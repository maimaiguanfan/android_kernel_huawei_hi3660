


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 ͷ�ļ�����
*****************************************************************************/
#include "oam_ext_if.h"
#include "frw_ext_if.h"
#include "wlan_spec.h"
#include "mac_ie.h"
#include "mac_frame.h"
#include "mac_vap.h"
#include "mac_device.h"
#include "mac_resource.h"
#include "mac_regdomain.h"
#include "dmac_ext_if.h"

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
#include "hisi_customize_wifi.h"
#endif
#endif


#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_MAC_FRAME_ROM_C


/*****************************************************************************
  2 ����ԭ������
*****************************************************************************/
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
mac_frame_cb g_st_mac_frame_rom_cb = {
    .set_ext_cap_ie_cb        = mac_set_ext_capabilities_ie_rom_cb,
    .set_ht_cap_ie_cb         = mac_set_ht_cap_ie_rom_cb,//set_ht_cap_ie
    .set_ht_opern_ie_cb       = OAL_PTR_NULL,//set_ht_opern_ie
    .set_rsn_ie_cb            = OAL_PTR_NULL,//set_rsn_ie
    .set_vht_cap_ie_cb        = OAL_PTR_NULL,//set_vht_cap_ie_cb
    .set_vht_opern_ie_cb      = OAL_PTR_NULL,//set_vht_opern_ie_cb
    .set_wpa_ie_cb            = OAL_PTR_NULL,//set_wpa_ie_cb
    .set_nb_ie_cb             = OAL_PTR_NULL,//set_nb_ie_cb
    .set_vht_capinfo_field_cb = mac_set_vht_capinfo_field_cb//set_vht_capinfo
    };
#else
mac_frame_cb g_st_mac_frame_rom_cb = {
    .set_ext_cap_ie_cb        = OAL_PTR_NULL,
    .set_ht_cap_ie_cb         = OAL_PTR_NULL,//set_ht_cap_ie
    .set_ht_opern_ie_cb       = OAL_PTR_NULL,//set_ht_opern_ie
    .set_rsn_ie_cb            = OAL_PTR_NULL,//set_rsn_ie
    .set_vht_cap_ie_cb        = OAL_PTR_NULL,//set_vht_cap_ie_cb
    .set_vht_opern_ie_cb      = OAL_PTR_NULL,//set_vht_opern_ie_cb
    .set_wpa_ie_cb            = OAL_PTR_NULL,//set_wpa_ie_cb
    .set_nb_ie_cb             = OAL_PTR_NULL,//set_nb_ie_cb
    .set_vht_capinfo_field_cb = OAL_PTR_NULL//set_vht_capinfo
    };
#endif



/*****************************************************************************
  3 ȫ�ֱ�������
*****************************************************************************/
/* WMM OUI���� */
OAL_CONST oal_uint8    g_auc_wmm_oui_etc[MAC_OUI_LEN] = {0x00, 0x50, 0xF2};

/* WPA OUI ���� */
OAL_CONST oal_uint8    g_auc_wpa_oui_etc[MAC_OUI_LEN] = {0x00, 0x50, 0xF2};

/* WFA TPC RPT OUI ���� */
OAL_CONST oal_uint8    g_auc_wfa_oui_etc[MAC_OUI_LEN] = {0x00, 0x50, 0xF2};

/* P2P OUI ���� */
OAL_CONST oal_uint8    g_auc_p2p_oui_etc[MAC_OUI_LEN] = {0x50, 0x6F, 0x9A};

/* RSNA OUI ���� */
OAL_CONST oal_uint8    g_auc_rsn_oui_etc[MAC_OUI_LEN] = {0x00, 0x0F, 0xAC};

/* WPS OUI ���� */
OAL_CONST oal_uint8    g_auc_wps_oui_etc[MAC_OUI_LEN] = {0x00, 0x50, 0xF2};

/* խ�� OUI ���� */
OAL_CONST oal_uint8    g_auc_huawei_oui[MAC_OUI_LEN] = {0xac, 0x85, 0x3d};

/*****************************************************************************
  4 ����ʵ��
*****************************************************************************/


oal_void  mac_report_beacon(mac_rx_ctl_stru *pst_rx_cb,oal_netbuf_stru *pst_netbuf)
{
    oal_uint32    ul_ret;

#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
    oal_uint8     *puc_beacon_payload_addr = OAL_PTR_NULL;

    puc_beacon_payload_addr = oal_netbuf_payload(pst_netbuf);
    ul_ret = oam_report_beacon_etc((oal_uint8 *)mac_get_rx_cb_mac_hdr(pst_rx_cb),
                               pst_rx_cb->uc_mac_header_len,
                               puc_beacon_payload_addr,
                               pst_rx_cb->us_frame_len,
                               OAM_OTA_FRAME_DIRECTION_TYPE_RX);
#else
    ul_ret = oam_report_beacon_etc((oal_uint8 *)MAC_GET_RX_CB_MAC_HEADER_ADDR(pst_rx_cb),
                               pst_rx_cb->uc_mac_header_len,
                               (oal_uint8 *)MAC_GET_RX_CB_MAC_HEADER_ADDR(pst_rx_cb) + pst_rx_cb->uc_mac_header_len,
                               pst_rx_cb->us_frame_len,
                               OAM_OTA_FRAME_DIRECTION_TYPE_RX);
#endif
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_WIFI_BEACON, "{mac_report_beacon::oam_report_beacon_etc return err: 0x%x.}\r\n", ul_ret);
    }
}


oal_uint32  mac_report_80211_get_switch(
                            mac_vap_stru               *pst_mac_vap,
                            mac_rx_ctl_stru            *pst_rx_cb,
                            oal_switch_enum_uint8      *pen_frame_switch,
                            oal_switch_enum_uint8      *pen_cb_switch,
                            oal_switch_enum_uint8      *pen_dscr_switch)
{
    mac_ieee80211_frame_stru         *pst_frame_hdr;
    oal_uint8                         uc_frame_type = 0;
    oal_uint16                        us_user_idx = 0xffff;
    oal_uint8                        *puc_da;
    oal_uint32                        ul_ret;

    pst_frame_hdr = (mac_ieee80211_frame_stru *)(mac_get_rx_cb_mac_hdr(pst_rx_cb));
    if (OAL_PTR_NULL == pst_frame_hdr)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{mac_report_80211_get_switch::pst_frame_hdr null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    if ((WLAN_CONTROL == pst_frame_hdr->st_frame_control.bit_type)
        || (WLAN_MANAGEMENT == pst_frame_hdr->st_frame_control.bit_type))
    {
        uc_frame_type = OAM_USER_TRACK_FRAME_TYPE_MGMT;
    }

    if (WLAN_DATA_BASICTYPE == pst_frame_hdr->st_frame_control.bit_type)
    {
        uc_frame_type = OAM_USER_TRACK_FRAME_TYPE_DATA;
    }

    /* probe request �� probe response̫�࣬��������һ�� */
    if (WLAN_MANAGEMENT == pst_frame_hdr->st_frame_control.bit_type)
    {
        if (WLAN_PROBE_REQ == pst_frame_hdr->st_frame_control.bit_sub_type
        || WLAN_PROBE_RSP == pst_frame_hdr->st_frame_control.bit_sub_type)
        {
            ul_ret = oam_report_80211_probe_get_switch_etc(OAM_OTA_FRAME_DIRECTION_TYPE_RX,
                                                       pen_frame_switch,
                                                       pen_cb_switch,
                                                       pen_dscr_switch);
            if (OAL_SUCC != ul_ret)
            {
                OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{mac_report_80211_get_switch::oam_report_80211_probe_get_switch_etc failed.}");

                return ul_ret;
            }

            return OAL_SUCC;
        }
        else if(WLAN_DEAUTH == pst_frame_hdr->st_frame_control.bit_sub_type
                || WLAN_DISASOC == pst_frame_hdr->st_frame_control.bit_sub_type)
        {
                *pen_cb_switch    = 1;
                *pen_dscr_switch  = 1;
                *pen_frame_switch = 1;
                return OAL_SUCC;
        }
        else if(WLAN_ACTION == pst_frame_hdr->st_frame_control.bit_sub_type
                || WLAN_ACTION_NO_ACK == pst_frame_hdr->st_frame_control.bit_sub_type)
        {
                *pen_cb_switch    = 1;
                *pen_dscr_switch  = 1;
                *pen_frame_switch = 1;
                return OAL_SUCC;
        }

    }

    mac_rx_get_da(pst_frame_hdr, &puc_da);

    if (ETHER_IS_MULTICAST(puc_da))
    {
        ul_ret = oam_report_80211_mcast_get_switch_etc(OAM_OTA_FRAME_DIRECTION_TYPE_RX,
                                                   uc_frame_type,
                                                   pen_frame_switch,
                                                   pen_cb_switch,
                                                   pen_dscr_switch);
        if (OAL_SUCC != ul_ret)
        {
            OAM_WARNING_LOG2(0, OAM_SF_RX,
                             "{mac_report_80211_get_switch::oam_report_80211_mcast_get_switch_etc failed! ul_ret=[%d],frame_type=[%d]}",
                             ul_ret, uc_frame_type);
            return ul_ret;
        }
    }
    else
    {
        ul_ret = mac_vap_find_user_by_macaddr_etc(pst_mac_vap,
                                              pst_frame_hdr->auc_address2,
                                              &us_user_idx);
        if (OAL_ERR_CODE_PTR_NULL == ul_ret)
        {
            MAC_ERR_LOG(0, "mac_vap_find_user_by_macaddr_etc return null ptr!");
            OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{mac_report_80211_get_switch::mac_vap_find_user_by_macaddr_etc failed.}");
            return ul_ret;
        }
        if (OAL_FAIL == ul_ret)
        {
            *pen_cb_switch    = 0;
            *pen_dscr_switch  = 0;
            *pen_frame_switch = 0;

            return OAL_FAIL;
        }
        ul_ret = oam_report_80211_ucast_get_switch_etc(OAM_OTA_FRAME_DIRECTION_TYPE_RX,
                                                   uc_frame_type,
                                                   pen_frame_switch,
                                                   pen_cb_switch,
                                                   pen_dscr_switch,
                                                   us_user_idx);
        if (OAL_SUCC != ul_ret)
        {
            OAM_WARNING_LOG3(pst_mac_vap->uc_vap_id, OAM_SF_RX,
                             "{mac_report_80211_get_switch::oam_report_80211_ucast_get_switch_etc failed! ul_ret=[%d],frame_type=[%d], user_idx=[%d]}",
                             ul_ret, uc_frame_type, us_user_idx);
            OAM_WARNING_LOG3(pst_mac_vap->uc_vap_id, OAM_SF_RX, "{oam_report_80211_ucast_get_switch_etc::frame_switch=[%d], cb_switch=[%d], dscr_switch=[%d]",
                             pen_frame_switch, pen_cb_switch, pen_dscr_switch);

            return ul_ret;
        }
    }

    return OAL_SUCC;
}


oal_uint32  mac_report_80211_get_user_macaddr(
                                            mac_rx_ctl_stru *pst_rx_cb,
                                            oal_uint8        auc_user_macaddr[])
{
    mac_ieee80211_frame_stru *pst_frame_hdr;
    oal_uint8                *puc_da;

    pst_frame_hdr = (mac_ieee80211_frame_stru *)(mac_get_rx_cb_mac_hdr(pst_rx_cb));
    if (OAL_PTR_NULL == pst_frame_hdr)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{mac_report_80211_get_user_macaddr::pst_frame_hdr null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    mac_rx_get_da(pst_frame_hdr, &puc_da);

    if (ETHER_IS_MULTICAST(puc_da))
    {
        oal_set_mac_addr(auc_user_macaddr, BROADCAST_MACADDR);
    }
    else
    {
        oal_set_mac_addr(auc_user_macaddr, pst_frame_hdr->auc_address2);
    }

    return OAL_SUCC;
}


oal_uint32  mac_report_80211_frame(oal_uint8      *puc_mac_vap,
                                           oal_uint8       *puc_rx_cb,
                                           oal_netbuf_stru *pst_netbuf,
                                           oal_uint8       *puc_des_addr,
                                           oam_ota_type_enum_uint8 en_ota_type)
{
    oal_switch_enum_uint8           en_frame_switch = OAL_SWITCH_OFF;
    oal_switch_enum_uint8           en_cb_switch    = OAL_SWITCH_OFF;
    oal_switch_enum_uint8           en_dscr_switch  = OAL_SWITCH_OFF;
    oal_uint32                      ul_ret ;
    oal_uint8                       auc_user_macaddr[WLAN_MAC_ADDR_LEN] = {0};
    mac_vap_stru                   *pst_mac_vap     = (mac_vap_stru*)puc_mac_vap;
    mac_rx_ctl_stru                *pst_rx_cb       = (mac_rx_ctl_stru*)puc_rx_cb;
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
    oal_uint8                      *puc_mac_payload_addr = OAL_PTR_NULL;
#endif

    /* ��ȡ��ӡ���� */
    ul_ret = mac_report_80211_get_switch(pst_mac_vap,
                                         pst_rx_cb,
                                         &en_frame_switch,
                                         &en_cb_switch,
                                         &en_dscr_switch);
    if (OAL_ERR_CODE_PTR_NULL == ul_ret)
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{mac_report_80211_frame::mac_report_80211_get_switch failed.}");

        return ul_ret;
    }

    if (OAL_FAIL == ul_ret)
    {
        return ul_ret;
    }

    /* ��ȡ���Ͷ��û���ַ���û�SDT����,������鲥\�㲥֡�����ַ��ΪȫF */
    ul_ret = mac_report_80211_get_user_macaddr(pst_rx_cb, auc_user_macaddr);
    if (OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{mac_report_80211_frame::mac_report_80211_get_user_macaddr failed.}");

        return ul_ret;
    }

    if ((OAM_OTA_TYPE_RX_DSCR == en_ota_type)||(OAM_OTA_TYPE_RX_DSCR_PILOT== en_ota_type))
    {
         /* �ϱ�����֡�Ľ���������*/
        if (OAL_SWITCH_ON == en_dscr_switch)
        {
            ul_ret = oam_report_dscr_etc(auc_user_macaddr, puc_des_addr,(oal_uint16)WLAN_RX_DSCR_SIZE, en_ota_type);
            if (OAL_SUCC != ul_ret)
            {
                OAM_WARNING_LOG1(0, OAM_SF_RX, "{mac_report_80211_frame::oam_report_dscr_etc return err: 0x%x.}\r\n", ul_ret);
            }
        }
    }
    else
    {
        /* �ϱ����յ���֡ */
        if (OAL_SWITCH_ON == en_frame_switch)
        {
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
            puc_mac_payload_addr = oal_netbuf_payload(pst_netbuf);

            ul_ret = oam_report_80211_frame_etc(auc_user_macaddr,
                                   (oal_uint8 *)mac_get_rx_cb_mac_hdr(pst_rx_cb),
                                   pst_rx_cb->uc_mac_header_len,
                                   puc_mac_payload_addr,
                                   pst_rx_cb->us_frame_len,
                                   OAM_OTA_FRAME_DIRECTION_TYPE_RX);
#else
            ul_ret = oam_report_80211_frame_etc(auc_user_macaddr,
                                   (oal_uint8 *)MAC_GET_RX_CB_MAC_HEADER_ADDR(pst_rx_cb),
                                   pst_rx_cb->uc_mac_header_len,
                                   (oal_uint8 *)MAC_GET_RX_CB_MAC_HEADER_ADDR(pst_rx_cb) + pst_rx_cb->uc_mac_header_len,
                                   pst_rx_cb->us_frame_len,
                                   OAM_OTA_FRAME_DIRECTION_TYPE_RX);
#endif
            if (OAL_SUCC != ul_ret)
            {
                OAM_WARNING_LOG1(0, OAM_SF_RX, "{mac_report_80211_frame::oam_report_80211_frame_etc return err: 0x%x.}\r\n", ul_ret);
            }
        }

        /* �ϱ�����֡��CB�ֶ� */
        if (OAL_SWITCH_ON == en_cb_switch)
        {
            ul_ret = oam_report_netbuf_cb_etc(auc_user_macaddr, (oal_uint8 *)pst_rx_cb, en_ota_type);
            if (OAL_SUCC != ul_ret)
            {
                OAM_WARNING_LOG1(0, OAM_SF_RX, "{mac_report_80211_frame::oam_report_netbuf_cb_etc return err: 0x%x.}\r\n", ul_ret);
            }
        }
    }

    return OAL_SUCC;
}


oal_uint32  mac_rx_report_80211_frame_etc(oal_uint8 *pst_vap,
                                             oal_uint8 *pst_rx_cb,
                                             oal_netbuf_stru *pst_netbuf,
                                             oam_ota_type_enum_uint8 en_ota_type)
{
    oal_uint8            uc_sub_type;
    mac_vap_stru        *pst_mac_vap;
    mac_rx_ctl_stru     *pst_mac_rx_cb;

    if (OAL_PTR_NULL == pst_rx_cb || OAL_PTR_NULL == pst_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_RX, "{mac_rx_report_80211_frame_etc::param null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_vap = (mac_vap_stru *)pst_vap;
    pst_mac_rx_cb = (mac_rx_ctl_stru *)pst_rx_cb;

    uc_sub_type = mac_get_frame_type_and_subtype((oal_uint8 *)mac_get_rx_cb_mac_hdr(pst_mac_rx_cb));

    if ((WLAN_FC0_SUBTYPE_BEACON|WLAN_FC0_TYPE_MGT) == uc_sub_type)
    {
        mac_report_beacon(pst_mac_rx_cb,pst_netbuf);
    }
    else
    {
        mac_report_80211_frame((oal_uint8 *)pst_mac_vap, (oal_uint8 *)pst_mac_rx_cb, pst_netbuf, OAL_PTR_NULL, en_ota_type);
    }

    return OAL_SUCC;
}
#ifdef _PRE_WLAN_FEATURE_P2P

oal_uint8 *mac_find_p2p_attribute_etc(oal_uint8 uc_eid, oal_uint8 *puc_ies, oal_int32 l_len)
{
    oal_int32 l_ie_len = 0;

    if (OAL_PTR_NULL == puc_ies)
    {
        return OAL_PTR_NULL;
    }

    /* ����P2P IE���������ֱ������һ�� */
    while (l_len > MAC_P2P_ATTRIBUTE_HDR_LEN && puc_ies[0] != uc_eid)
    {
        l_ie_len = (oal_int32)((puc_ies[2] << 8) + puc_ies[1]);
        l_len   -= l_ie_len + MAC_P2P_ATTRIBUTE_HDR_LEN;
        puc_ies += l_ie_len + MAC_P2P_ATTRIBUTE_HDR_LEN;
    }

    /* ���ҵ�P2P IE��ʣ�೤�Ȳ�ƥ��ֱ�ӷ��ؿ�ָ�� */
    l_ie_len = (oal_int32)((puc_ies[2] << 8) + puc_ies[1]);
    if ((l_len < MAC_P2P_ATTRIBUTE_HDR_LEN) || (l_len < (MAC_P2P_ATTRIBUTE_HDR_LEN + l_ie_len)))
    {
        return OAL_PTR_NULL;
    }

    return puc_ies;
}
#endif

oal_uint8 *mac_find_ie_etc(oal_uint8 uc_eid, oal_uint8 *puc_ies, oal_int32 l_len)
{
    if (OAL_PTR_NULL == puc_ies)
    {
        return OAL_PTR_NULL;
    }

    /* buffer���ȳ���1500�ֽ���Ϊ����쳣���������Ҳ��� */
    if (l_len > 1500)
    {
        return OAL_PTR_NULL;
    }

    while (l_len > MAC_IE_HDR_LEN && puc_ies[0] != uc_eid)
    {
        l_len   -= puc_ies[1] + MAC_IE_HDR_LEN;
        puc_ies += puc_ies[1] + MAC_IE_HDR_LEN;
    }

    if ((l_len < MAC_IE_HDR_LEN) || (l_len < (MAC_IE_HDR_LEN + puc_ies[1]))
        || ((l_len == MAC_IE_HDR_LEN) && (puc_ies[0] != uc_eid)))
    {
        return OAL_PTR_NULL;
    }

    return puc_ies;
}


oal_uint8 *mac_find_vendor_ie_etc(oal_uint32      ul_oui,
                                oal_uint8     uc_oui_type,
                                oal_uint8    *puc_ies,
                                oal_int32     l_len)
{
    struct mac_ieee80211_vendor_ie *pst_ie;
    oal_uint8 *puc_pos;
    oal_uint8 *puc_end;
    oal_uint32 ul_ie_oui;

    if (OAL_PTR_NULL == puc_ies)
    {
        return OAL_PTR_NULL;
    }

    puc_pos = puc_ies;
    puc_end = puc_ies + l_len;
    while (puc_pos < puc_end)
    {
        puc_pos = mac_find_ie_etc(MAC_EID_VENDOR, puc_pos, (oal_int32)(puc_end - puc_pos));
        if (OAL_PTR_NULL == puc_pos)
        {
            return OAL_PTR_NULL;
        }

        pst_ie = (struct mac_ieee80211_vendor_ie *)puc_pos;
        if (pst_ie->uc_len >= (sizeof(*pst_ie) - MAC_IE_HDR_LEN))
        {
            ul_ie_oui = pst_ie->auc_oui[0] << 16 | pst_ie->auc_oui[1] << 8 | pst_ie->auc_oui[2];
            if ((ul_ie_oui == ul_oui) && (pst_ie->uc_oui_type == uc_oui_type))
            {
                return puc_pos;
            }
        }
        puc_pos += 2 + pst_ie->uc_len;
    }
    return OAL_PTR_NULL;
}


oal_void  mac_set_beacon_interval_field_etc(oal_void *pst_vap, oal_uint8 *puc_buffer)
{
    oal_uint16   *pus_bcn_int;
    oal_uint32    ul_bcn_int;
    mac_vap_stru *pst_mac_vap = (mac_vap_stru *)pst_vap;

    /*****************************************************************************
                |Beacon interval|
        Octets:        2
    *****************************************************************************/
    pus_bcn_int = (oal_uint16 *)puc_buffer;

    ul_bcn_int = mac_mib_get_BeaconPeriod(pst_mac_vap);

    *pus_bcn_int = (oal_uint16)OAL_BYTEORDER_TO_LE32(ul_bcn_int);
}


oal_void  mac_set_cap_info_ap_etc(oal_void *pst_vap, oal_uint8 *puc_cap_info)
{
    mac_cap_info_stru  *pst_cap_info = (mac_cap_info_stru *)puc_cap_info;
    mac_vap_stru       *pst_mac_vap  = (mac_vap_stru *)pst_vap;

    /**************************************************************************
         -------------------------------------------------------------------
         |B0 |B1  |B2        |B3    |B4     |B5      |B6  |B7     |B8      |
         -------------------------------------------------------------------
         |ESS|IBSS|CFPollable|CFPReq|Privacy|Preamble|PBCC|Agility|SpecMgmt|
         -------------------------------------------------------------------
         |B9 |B10      |B11 |B12     |B13      |B14        |B15            |
         -------------------------------------------------------------------
         |QoS|ShortSlot|APSD|RM      |DSSS-OFDM|Delayed BA |Immediate BA   |
         -------------------------------------------------------------------
    ***************************************************************************/
    /* ��ʼ���� */
    puc_cap_info[0] = 0;
    puc_cap_info[1] = 0;

    if (WLAN_MIB_DESIRED_BSSTYPE_INDEPENDENT == mac_mib_get_DesiredBSSType(pst_mac_vap))
    {
        pst_cap_info->bit_ibss = 1;
    }
    else if (WLAN_MIB_DESIRED_BSSTYPE_INFRA == mac_mib_get_DesiredBSSType(pst_mac_vap))
    {
        pst_cap_info->bit_ess = 1;
    }

    /* The Privacy bit is set if WEP is enabled */
    pst_cap_info->bit_privacy = mac_mib_get_privacyinvoked(pst_mac_vap);

    /* preamble */
    pst_cap_info->bit_short_preamble = mac_mib_get_ShortPreambleOptionImplemented(pst_mac_vap);

    /* packet binary convolutional code (PBCC) modulation */
    pst_cap_info->bit_pbcc = mac_mib_get_PBCCOptionImplemented(pst_mac_vap);

    /* Channel Agility */
    pst_cap_info->bit_channel_agility = mac_mib_get_ChannelAgilityPresent(pst_mac_vap);

    /* Spectrum Management */
    pst_cap_info->bit_spectrum_mgmt = mac_mib_get_dot11SpectrumManagementRequired(pst_mac_vap);

    /* QoS subfield */
    //pst_cap_info->bit_qos = mac_mib_get_dot11QosOptionImplemented(pst_mac_vap);
    pst_cap_info->bit_qos = 0;

    /* short slot */
    pst_cap_info->bit_short_slot_time = mac_mib_get_ShortSlotTimeOptionImplemented(pst_mac_vap) & mac_mib_get_ShortSlotTimeOptionActivated(pst_mac_vap);

    /* APSD */
    pst_cap_info->bit_apsd = mac_mib_get_dot11APSDOptionImplemented(pst_mac_vap);

    /* Radio Measurement */
    pst_cap_info->bit_radio_measurement = mac_mib_get_dot11RadioMeasurementActivated(pst_mac_vap);

    /* DSSS-OFDM */
    pst_cap_info->bit_dsss_ofdm = mac_mib_get_DSSSOFDMOptionActivated(pst_mac_vap);

    /* Delayed BA */
    pst_cap_info->bit_delayed_block_ack = mac_mib_get_dot11DelayedBlockAckOptionImplemented(pst_mac_vap);

    /* Immediate Block Ack �ο�STA��AP��ˣ�������һֱΪ0,ʵ��ͨ��addbaЭ�̡��˴��޸�Ϊ���һ�¡�mibֵ���޸� */
    /*pst_cap_info->bit_immediate_block_ack = pst_mib->st_wlan_mib_sta_config.en_dot11ImmediateBlockAckOptionImplemented;*/
    pst_cap_info->bit_immediate_block_ack = 0;

}


oal_void  mac_set_cap_info_sta_etc(oal_void *pst_vap, oal_uint8 *puc_cap_info)
{
    mac_cap_info_stru   *pst_cap_info = (mac_cap_info_stru *)puc_cap_info;
    mac_vap_stru        *pst_mac_vap  = (mac_vap_stru *)pst_vap;

    /**************************************************************************
         -------------------------------------------------------------------
         |B0 |B1  |B2        |B3    |B4     |B5      |B6  |B7     |B8      |
         -------------------------------------------------------------------
         |ESS|IBSS|CFPollable|CFPReq|Privacy|Preamble|PBCC|Agility|SpecMgmt|
         -------------------------------------------------------------------
         |B9 |B10      |B11 |B12     |B13      |B14        |B15            |
         -------------------------------------------------------------------
         |QoS|ShortSlot|APSD|RM      |DSSS-OFDM|Delayed BA |Immediate BA   |
         -------------------------------------------------------------------
    ***************************************************************************/
    /* ѧϰ�Զ˵�������Ϣ */
    oal_memcopy(puc_cap_info, (oal_uint8 *)(&pst_mac_vap->us_assoc_user_cap_info), OAL_SIZEOF(mac_cap_info_stru));

    /* ��������λ��ѧϰ������Ĭ��ֵ */
    pst_cap_info->bit_ibss              = 0;
    pst_cap_info->bit_cf_pollable       = 0;
    pst_cap_info->bit_cf_poll_request   = 0;
#if defined(_PRE_WLAN_FEATURE_11K) || defined(_PRE_WLAN_FEATURE_11K_EXTERN)
    pst_cap_info->bit_radio_measurement = (mac_mib_get_dot11RadioMeasurementActivated(pst_mac_vap) & (pst_cap_info->bit_radio_measurement));
#endif
}


oal_void  mac_set_ssid_ie_etc(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len, oal_uint16 us_frm_type)
{
    oal_uint8    *puc_ssid;
    oal_uint8     uc_ssid_len;
    mac_vap_stru *pst_mac_vap = (mac_vap_stru *)pst_vap;

    /***************************************************************************
                    ----------------------------
                    |Element ID | Length | SSID|
                    ----------------------------
           Octets:  |1          | 1      | 0~32|
                    ----------------------------
    ***************************************************************************/
    /***************************************************************************
      A SSID  field  of length 0 is  used  within Probe
      Request management frames to indicate the wildcard SSID.
    ***************************************************************************/
    /* ֻ��beacon������ssid */
    if((pst_mac_vap->st_cap_flag.bit_hide_ssid) && (WLAN_FC0_SUBTYPE_BEACON == us_frm_type))
    {
        /* ssid ie */
        *puc_buffer = MAC_EID_SSID;
        /* ssid len */
        *(puc_buffer + 1) = 0;
        *puc_ie_len = MAC_IE_HDR_LEN;
        return;
    }

    *puc_buffer = MAC_EID_SSID;

    puc_ssid = mac_mib_get_DesiredSSID(pst_mac_vap);

    uc_ssid_len = (oal_uint8)OAL_STRLEN((oal_int8 *)puc_ssid);   /* ������'\0'*/

    *(puc_buffer + 1) = uc_ssid_len;

    oal_memcopy(puc_buffer + MAC_IE_HDR_LEN, puc_ssid, uc_ssid_len);

    *puc_ie_len = uc_ssid_len + MAC_IE_HDR_LEN;

}

#ifdef _PRE_WLAN_NARROW_BAND

oal_void  mac_set_nb_ie( oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{

    oal_uint8            uc_index;

    /* ----------------------------------------------------------------- */
    /* NB Information/Parameter Element Format                           */
    /* ----------------------------------------------------------------- */
    /* EID | IE LEN | OUI | OUIType | Narrow Band|                       */
    /* ----------------------------------------------------------------- */
    /* 1   |   1    |  3  | 1       |3           |                       */
    /* ----------------------------------------------------------------- */

    /* ��дEID, ��������� */
    puc_buffer[0] = MAC_EID_VENDOR;

    /* ��ʼ����дbuffer��λ�� */
    uc_index = MAC_IE_HDR_LEN;

    oal_memcopy(puc_buffer + uc_index, g_auc_huawei_oui, MAC_OUI_LEN);
    uc_index += MAC_OUI_LEN;

    puc_buffer[uc_index++] = MAC_HISI_NB_IE; /* oui_type */

    puc_buffer[uc_index++] = NARROW_BW_1M;
    puc_buffer[uc_index++] = NARROW_BW_5M;
    puc_buffer[uc_index++] = NARROW_BW_10M;

    /* ������ϢԪ�س��� */
    puc_buffer[1] = uc_index - MAC_IE_HDR_LEN;
    *puc_ie_len = uc_index;

    if (g_st_mac_frame_rom_cb.set_nb_ie_cb != OAL_PTR_NULL)
    {
        g_st_mac_frame_rom_cb.set_nb_ie_cb(puc_buffer, puc_ie_len);
    }
}
#endif


oal_void  mac_set_supported_rates_ie_etc(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
    mac_vap_stru     *pst_mac_vap   = (mac_vap_stru *)pst_vap;
    mac_rateset_stru *pst_rates_set;
    oal_uint8         uc_nrates;
    oal_uint8         uc_idx;

    pst_rates_set = &(pst_mac_vap->st_curr_sup_rates.st_rate);

    /* STAȫ�ŵ�ɨ��ʱ����Ƶ������supported rates */
    if (WLAN_VAP_MODE_BSS_STA == pst_mac_vap->en_vap_mode &&
#ifdef _PRE_WLAN_FEATURE_11AX
       (WLAN_VHT_MODE == pst_mac_vap->en_protocol || WLAN_HE_MODE == pst_mac_vap->en_protocol))
#else
        WLAN_VHT_MODE == pst_mac_vap->en_protocol)
#endif
    {
        if (pst_mac_vap->st_channel.en_band < WLAN_BAND_BUTT)
        {
            pst_rates_set = &(pst_mac_vap->ast_sta_sup_rates_ie[pst_mac_vap->st_channel.en_band].st_rate);
        }
    }


    /**************************************************************************
                        ---------------------------------------
                        |Element ID | Length | Supported Rates|
                        ---------------------------------------
             Octets:    |1          | 1      | 1~8            |
                        ---------------------------------------
    The Information field is encoded as 1 to 8 octets, where each octet describes a single Supported
    Rate or BSS membership selector.
    **************************************************************************/
    puc_buffer[0] = MAC_EID_RATES;

    uc_nrates = pst_rates_set->uc_rs_nrates;

    if (uc_nrates > MAC_MAX_SUPRATES)
    {
        uc_nrates = MAC_MAX_SUPRATES;
    }

    for (uc_idx = 0; uc_idx < uc_nrates; uc_idx++)
    {
        puc_buffer[MAC_IE_HDR_LEN + uc_idx] = pst_rates_set->ast_rs_rates[uc_idx].uc_mac_rate;
    }

    puc_buffer[1] = uc_nrates;

    *puc_ie_len = MAC_IE_HDR_LEN + uc_nrates;
}


oal_void mac_set_dsss_params_etc(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len,oal_uint8 uc_chan_num)
{
    /***************************************************************************
                        ----------------------------------------
                        | Element ID  | Length |Current Channel|
                        ----------------------------------------
              Octets:   | 1           | 1      | 1             |
                        ----------------------------------------
    The DSSS Parameter Set element contains information to allow channel number identification for STAs.
    ***************************************************************************/

    
#if 0
    if (WLAN_BAND_2G != pst_mac_vap->st_channel.en_band)
    {
        /* �������2.4GHzƵ�Σ���û��dsss���� */
        *puc_ie_len = 0;

        return;
    }
#endif

    puc_buffer[0] = MAC_EID_DSPARMS;
    puc_buffer[1] = MAC_DSPARMS_LEN;
    puc_buffer[2] = uc_chan_num;

    *puc_ie_len = MAC_IE_HDR_LEN + MAC_DSPARMS_LEN;
}

#ifdef _PRE_WLAN_FEATURE_11D


oal_void mac_set_country_ie_etc(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
    mac_vap_stru                *pst_mac_vap = (mac_vap_stru *)pst_vap;
    mac_regdomain_info_stru     *pst_rd_info;
    oal_uint8                    uc_band;
    oal_uint8                    uc_index;
    oal_uint32                   ul_ret;
    oal_uint8                    uc_len = 0;

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    if(OAL_TRUE != hwifi_get_init_value_etc(CUS_TAG_INI, WLAN_CFG_INIT_BEACON_COUNTRY_IE_SWITCH))
    {
        *puc_ie_len = 0;
        return;
    }
#endif
#endif

    if (OAL_TRUE != mac_mib_get_dot11MultiDomainCapabilityActivated(pst_mac_vap)
        && OAL_TRUE != mac_mib_get_dot11SpectrumManagementRequired(pst_mac_vap)
        && OAL_TRUE != mac_mib_get_dot11RadioMeasurementActivated(pst_mac_vap))
    {
        /* û��ʹ�ܹ�����ie */
        *puc_ie_len = 0;

        return;
    }

    /***************************************************************************
                               |....These three fields are repeated...|
    -------------------------------------------------------------------------------
    |EID | Len | CountryString | First Channel |Number of |Maximum    | Pad       |
    |    |     |               |   Number/     |Channels/ |Transmit   |(if needed)|
    |    |     |               |   Operating   | Operating|Power Level|           |
    |    |     |               |   Extension   | Class    |/Coverage  |           |
    |    |     |               |   Identifier  |          |Class      |           |
    -------------------------------------------------------------------------------
    |1   |1    |3              |1              |1         |1          |0 or 1     |
    -------------------------------------------------------------------------------
    ***************************************************************************/
    /* ��ȡ��������Ϣ */
    mac_get_regdomain_info_etc(&pst_rd_info);

    /* ��ȡ��ǰ����Ƶ�� */
    uc_band = pst_mac_vap->st_channel.en_band;

    /* ��дEID, ��������� */
    puc_buffer[0] = MAC_EID_COUNTRY;

    /* ��ʼ����дbuffer��λ�� */
    uc_index = MAC_IE_HDR_LEN;

    /* ������ */
    puc_buffer[uc_index++] = (oal_uint8)(pst_rd_info->ac_country[0]);
    puc_buffer[uc_index++] = (oal_uint8)(pst_rd_info->ac_country[1]);
    puc_buffer[uc_index++] = ' ';     /* 0��ʾ��������涨��ͬ */

    if (WLAN_BAND_2G == uc_band)
    {
        ul_ret = mac_set_country_ie_2g_etc(pst_rd_info, &(puc_buffer[uc_index]), &uc_len);
    }
    else if (WLAN_BAND_5G == uc_band)
    {
        ul_ret = mac_set_country_ie_5g_etc(pst_rd_info, &(puc_buffer[uc_index]), &uc_len);
    }
    else
    {
        ul_ret = OAL_FAIL;
    }

    if (OAL_SUCC != ul_ret)
    {
        *puc_ie_len = 0;
        return;
    }

    if (0 == uc_len)
    {
        /* �޹��������� */
        *puc_ie_len = 0;

        return;
    }

    uc_index += uc_len;

    /* ����ܳ���Ϊ��������1�ֽ�pad */
    if (1 == (uc_index & BIT0))
    {
        puc_buffer[uc_index] = 0;
        uc_index += 1;
    }

    /* ������ϢԪ�س��� */
    puc_buffer[1] = uc_index - MAC_IE_HDR_LEN;
    *puc_ie_len = uc_index;
}
#endif

oal_void mac_set_11ntxbf_vendor_ie_etc(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
    mac_vap_stru                        *pst_mac_vap = (mac_vap_stru *)pst_vap;
    mac_11ntxbf_vendor_ie_stru          *pst_vendor_ie;
    if (OAL_TRUE != pst_mac_vap->st_cap_flag.bit_11ntxbf)
    {
        *puc_ie_len = 0;
        return;
    }

    pst_vendor_ie = (mac_11ntxbf_vendor_ie_stru *)puc_buffer;
    pst_vendor_ie->uc_id = MAC_EID_VENDOR;
    pst_vendor_ie->uc_len = sizeof(mac_11ntxbf_vendor_ie_stru) - MAC_IE_HDR_LEN;
    /* ��ֵΪCCB���� */
    pst_vendor_ie->uc_ouitype = MAC_EID_11NTXBF;

    /*lint -e572*/ /*lint -e778*/
    pst_vendor_ie->auc_oui[0] = (oal_uint8)((MAC_HUAWEI_VENDER_IE >> 16) & 0xff);
    pst_vendor_ie->auc_oui[1] = (oal_uint8)((MAC_HUAWEI_VENDER_IE >> 8) & 0xff);
    pst_vendor_ie->auc_oui[2] = (oal_uint8)((MAC_HUAWEI_VENDER_IE) & 0xff);
    /*lint +e572*/ /*lint +e778*/

    OAL_MEMZERO(&pst_vendor_ie->st_11ntxbf, OAL_SIZEOF(mac_11ntxbf_info_stru));
    pst_vendor_ie->st_11ntxbf.bit_11ntxbf = pst_mac_vap->st_cap_flag.bit_11ntxbf;
    *puc_ie_len = OAL_SIZEOF(mac_11ntxbf_vendor_ie_stru);

}


oal_void mac_set_pwrconstraint_ie_etc(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
    mac_vap_stru *pst_mac_vap = (mac_vap_stru *)pst_vap;

    /***************************************************************************
                   -------------------------------------------
                   |ElementID | Length | LocalPowerConstraint|
                   -------------------------------------------
       Octets:     |1         | 1      | 1                   |
                   -------------------------------------------

    ����վ�����������������书�ʣ�����ϢԪ�ؼ�¼�涨���ֵ
    ��ȥʵ��ʹ��ʱ�����ֵ
    ***************************************************************************/
    if (OAL_FALSE == mac_mib_get_dot11SpectrumManagementRequired(pst_mac_vap))
    {
        *puc_ie_len = 0;

        return;
    }

    *puc_buffer       = MAC_EID_PWRCNSTR;
    *(puc_buffer + 1) = MAC_PWR_CONSTRAINT_LEN;

    /* Note that this field is always set to 0 currently. Ideally            */
    /* this field can be updated by having an algorithm to decide transmit   */
    /* power to be used in the BSS by the AP.                                */
    /* TBD,  */
    *(puc_buffer + MAC_IE_HDR_LEN) = 0;

    *puc_ie_len = MAC_IE_HDR_LEN + MAC_PWR_CONSTRAINT_LEN;
}


oal_void mac_set_quiet_ie_etc(
                void      *pst_vap,    oal_uint8 *puc_buffer, oal_uint8  uc_qcount,
                oal_uint8  uc_qperiod, oal_uint16 us_qduration,    oal_uint16 us_qoffset,
                oal_uint8 *puc_ie_len)
{
    /* ��������� tbd, ��Ҫ11h���Խ�һ��������ie������ */

    mac_quiet_ie_stru *pst_quiet;
    mac_vap_stru      *pst_mac_vap = (mac_vap_stru *)pst_vap;

    if ((OAL_TRUE != mac_mib_get_dot11SpectrumManagementRequired(pst_mac_vap))
        && (OAL_TRUE != mac_mib_get_dot11RadioMeasurementActivated(pst_mac_vap)))
    {
        *puc_ie_len = 0;

        return;
    }

    /***************************************************************************
    -----------------------------------------------------------------------------
    |ElementID | Length | QuietCount | QuietPeriod | QuietDuration | QuietOffset|
    -----------------------------------------------------------------------------
    |1         | 1      | 1          | 1           | 2             | 2          |
    -----------------------------------------------------------------------------
    ***************************************************************************/
    if (0 == us_qduration || 0 == uc_qcount)
    {
        *puc_ie_len = 0;

        return;
    }

    *puc_buffer = MAC_EID_QUIET;

    *(puc_buffer + 1) = MAC_QUIET_IE_LEN;

    pst_quiet = (mac_quiet_ie_stru *)(puc_buffer + MAC_IE_HDR_LEN);

    pst_quiet->quiet_count    = uc_qcount;
    pst_quiet->quiet_period   = uc_qperiod;
    pst_quiet->quiet_duration = OAL_BYTEORDER_TO_LE16(us_qduration);
    pst_quiet->quiet_offset   = OAL_BYTEORDER_TO_LE16(us_qoffset);

    *puc_ie_len = MAC_IE_HDR_LEN + MAC_QUIET_IE_LEN;
}


oal_void mac_set_tpc_report_ie_etc(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
    mac_vap_stru *pst_mac_vap = (mac_vap_stru *)pst_vap;

    /***************************************************************************
                -------------------------------------------------
                |ElementID  |Length  |TransmitPower  |LinkMargin|
                -------------------------------------------------
       Octets:  |1          |1       |1              |1         |
                -------------------------------------------------

    TransimitPower, ��֡�Ĵ��͹��ʣ���dBmΪ��λ
    ***************************************************************************/
    if (OAL_FALSE == mac_mib_get_dot11SpectrumManagementRequired(pst_mac_vap)
        && OAL_FALSE == mac_mib_get_dot11RadioMeasurementActivated(pst_mac_vap))
    {
        *puc_ie_len = 0;

        return;
    }

    *puc_buffer       = MAC_EID_TPCREP;
    *(puc_buffer + 1) = MAC_TPCREP_IE_LEN;
    *(puc_buffer + 2) = pst_mac_vap->uc_tx_power;
    *(puc_buffer + 3) = 0;                          /* ���ֶι���֡�в��� */

    *puc_ie_len = MAC_IE_HDR_LEN + MAC_TPCREP_IE_LEN;
}


oal_void mac_set_erp_ie_etc(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
    mac_vap_stru         *pst_mac_vap    = (mac_vap_stru *)pst_vap;
    mac_erp_params_stru  *pst_erp_params;

    /***************************************************************************
    --------------------------------------------------------------------------
    |EID  |Len  |NonERP_Present|Use_Protection|Barker_Preamble_Mode|Reserved|
    --------------------------------------------------------------------------
    |B0-B7|B0-B7|B0            |B1            |B2                  |B3-B7   |
    --------------------------------------------------------------------------
    ***************************************************************************/
    if ((WLAN_BAND_5G == pst_mac_vap->st_channel.en_band) || (WLAN_LEGACY_11B_MODE == pst_mac_vap->en_protocol))
    {
        *puc_ie_len = 0;

        return;     /* 5GƵ�κ�11bЭ��ģʽ û��erp��Ϣ */
    }

    *puc_buffer       = MAC_EID_ERP;
    *(puc_buffer + 1) = MAC_ERP_IE_LEN;
    *(puc_buffer + 2) = 0;  /* ��ʼ��0 */

    pst_erp_params = (mac_erp_params_stru *)(puc_buffer + MAC_IE_HDR_LEN);

    /*�������non erpվ����ap������ ����obss�д���non erpվ��*/
    if ((0 != pst_mac_vap->st_protection.uc_sta_non_erp_num) || ( OAL_TRUE == pst_mac_vap->st_protection.bit_obss_non_erp_present))
    {
        pst_erp_params->bit_non_erp = 1;
    }
    else
    {
        pst_erp_params->bit_non_erp = 0;
    }

    /*���ap�Ѿ�����erp����*/
    if (WLAN_PROT_ERP == pst_mac_vap->st_protection.en_protection_mode)
    {
        pst_erp_params->bit_use_protection = 1;
    }
    else
    {
        pst_erp_params->bit_use_protection = 0;
    }

    /*������ڲ�֧��short preamble��վ����ap������ ����ap����֧��short preamble*/
    if ((0 != pst_mac_vap->st_protection.uc_sta_no_short_preamble_num)
        || (OAL_FALSE == mac_mib_get_ShortPreambleOptionImplemented(pst_mac_vap)))
    {
        pst_erp_params->bit_preamble_mode = 1;
    }

    *puc_ie_len = MAC_IE_HDR_LEN + MAC_ERP_IE_LEN;

}


oal_void mac_set_rsn_ie_etc(oal_void *pst_vap, oal_uint8 *puc_pmkid, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
    mac_vap_stru     *pst_mac_vap = (mac_vap_stru *)pst_vap;
    mac_rsn_cap_stru *pst_rsn_cap;
    oal_uint8         uc_index;
    oal_uint8         uc_pair_suites_num;
    oal_uint8         uc_akm_suites_num;
    oal_uint8         uc_loop = 0;
    oal_uint32        ul_group_suit;
    oal_uint32        ul_group_mgmt_suit = 0;
    oal_uint32        aul_pcip[WLAN_PAIRWISE_CIPHER_SUITES] = {0};
    oal_uint32        aul_akm[WLAN_AUTHENTICATION_SUITES] = {0};

    if (OAL_TRUE != mac_mib_get_rsnaactivated(pst_mac_vap) || (OAL_TRUE != pst_mac_vap->st_cap_flag.bit_wpa2))
    {
        *puc_ie_len = 0;
        return;
    }

    ul_group_suit            = mac_mib_get_rsn_group_suite(pst_mac_vap);
    ul_group_mgmt_suit       = mac_mib_get_rsn_group_mgmt_suite(pst_mac_vap);
    uc_pair_suites_num       = mac_mib_get_rsn_pair_suites(pst_mac_vap, aul_pcip);
    uc_akm_suites_num        = mac_mib_get_rsn_akm_suites(pst_mac_vap, aul_akm);

    if ((0 == uc_pair_suites_num) || (0 == uc_akm_suites_num))
    {
        *puc_ie_len = 0;
        return;
    }

    /*************************************************************************/
    /*                  RSN Element Format              */
    /* --------------------------------------------------------------------- */
    /* |Element ID | Length | Version | Group Cipher Suite | Pairwise Cipher */
    /* --------------------------------------------------------------------- */
    /* | 1         | 1      | 2       |      4             |     2           */
    /* --------------------------------------------------------------------- */
    /* --------------------------------------------------------------------- */
    /* Suite | Pairwise Cipher Suite List | AKM Suite Count | AKM Suite List */
    /* --------------------------------------------------------------------- */
    /*       | 4-m                        |     2          | 4-n             */
    /* --------------------------------------------------------------------- */
    /* --------------------------------------------------------------------- */
    /* |RSN Capabilities|PMKID Count|PMKID List|Group Management Cipher Suite*/
    /* --------------------------------------------------------------------- */
    /* |    2           |    2      |16 -s     |         4                 | */
    /* --------------------------------------------------------------------- */
    /*                                                                       */
    /*************************************************************************/
    uc_index = MAC_IE_HDR_LEN;

    /* ����RSN ie��EID */
    puc_buffer[0] = MAC_EID_RSN;

    /* ����version�ֶ� */
    puc_buffer[uc_index++] = MAC_RSN_IE_VERSION;
    puc_buffer[uc_index++] = 0;

    oal_memcopy(&puc_buffer[uc_index], (oal_uint8 *)(&ul_group_suit), 4);
    uc_index += 4;

    /* ���óɶԼ����׼� */
    puc_buffer[uc_index++] = uc_pair_suites_num;
    puc_buffer[uc_index++] = 0;

    for (uc_loop = 0; uc_loop < uc_pair_suites_num; uc_loop++)
    {
        oal_memcopy(&puc_buffer[uc_index], (oal_uint8 *)(&aul_pcip[uc_loop]), 4);
        uc_index += 4;
    }

    /* ������֤�׼��� */
    puc_buffer[uc_index++] = uc_akm_suites_num;
    puc_buffer[uc_index++] = 0;

    /* ����MIB ֵ��������֤�׼����� */
    for (uc_loop = 0; uc_loop < uc_akm_suites_num; uc_loop++)
    {
        oal_memcopy(&puc_buffer[uc_index], (oal_uint8 *)(&aul_akm[uc_loop]), 4);
        uc_index += 4;
    }

    /* ���� RSN Capabilities�ֶ� */
    /*************************************************************************/
    /* --------------------------------------------------------------------- */
    /* | B15 - B6  |  B5 - B4      | B3 - B2     |       B1    |     B0    | */
    /* --------------------------------------------------------------------- */
    /* | Reserved  |  GTSKA Replay | PTSKA Replay| No Pairwise | Pre - Auth| */
    /* |           |    Counter    |   Counter   |             |           | */
    /* --------------------------------------------------------------------- */
    /*                                                                       */
    /*************************************************************************/

    /* ����RSN Capabilities ֵ������Pre_Auth, no_pairwise,  */
    /* Replay counters (PTKSA and GTKSA)                    */
    /* WPA ����Ҫ��дRSN Capabilities �ֶ�                  */
    pst_rsn_cap = (mac_rsn_cap_stru *)(puc_buffer + uc_index);
    OAL_MEMZERO(pst_rsn_cap, OAL_SIZEOF(mac_rsn_cap_stru));
    uc_index += MAC_RSN_CAP_LEN;

    pst_rsn_cap->bit_mfpr        = mac_mib_get_dot11RSNAMFPR(pst_mac_vap);
    pst_rsn_cap->bit_mfpc        = mac_mib_get_dot11RSNAMFPC(pst_mac_vap);
    pst_rsn_cap->bit_pre_auth    = mac_mib_get_pre_auth_actived(pst_mac_vap);
    pst_rsn_cap->bit_no_pairwise = 0;
    pst_rsn_cap->bit_ptska_relay_counter = mac_mib_get_rsnacfg_ptksareplaycounters(pst_mac_vap);
    pst_rsn_cap->bit_gtska_relay_counter = mac_mib_get_rsnacfg_gtksareplaycounters(pst_mac_vap);

    /* ���� PMKID ��Ϣ */
    if (puc_pmkid)
    {
        puc_buffer[uc_index++] = 0x01;
        puc_buffer[uc_index++] = 0x00;
        oal_memcopy(&(puc_buffer[uc_index]), puc_pmkid, WLAN_PMKID_LEN);
        uc_index += WLAN_PMKID_LEN;
    }

    if ((OAL_TRUE == mac_mib_get_dot11RSNAMFPC((mac_vap_stru *)pst_mac_vap) && ul_group_mgmt_suit))
    {
        /* ����Ѿ����pmkid��Ϣ������Ҫ���������Ҫ��дһ���յ�PMKID */
        if (OAL_PTR_NULL == puc_pmkid)
        {
            puc_buffer[uc_index++] = 0x00;
            puc_buffer[uc_index++] = 0x00;
        }
        oal_memcopy(&puc_buffer[uc_index], (oal_uint8 *)(&ul_group_mgmt_suit), 4);
        uc_index += 4;

    }

    /* ����RSN element�ĳ��� */
    puc_buffer[1] = uc_index - MAC_IE_HDR_LEN;

    *puc_ie_len = uc_index;

    if (OAL_PTR_NULL != g_st_mac_frame_rom_cb.set_rsn_ie_cb)
    {
        g_st_mac_frame_rom_cb.set_rsn_ie_cb(pst_mac_vap, puc_pmkid, puc_buffer, puc_ie_len);
    }

}


oal_void mac_set_wpa_ie_etc(oal_void  *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
    mac_vap_stru     *pst_mac_vap = (mac_vap_stru *)pst_vap;
    oal_uint8         uc_index;
    oal_uint8         uc_pair_suites_num;
    oal_uint8         uc_akm_suites_num;
    oal_uint8         uc_loop = 0;
    oal_uint32        ul_group_suit;
    oal_uint32        aul_pcip[WLAN_PAIRWISE_CIPHER_SUITES] = {0};
    oal_uint32        aul_akm[WLAN_AUTHENTICATION_SUITES] = {0};

    if (OAL_TRUE != mac_mib_get_rsnaactivated(pst_mac_vap) || (OAL_TRUE != pst_mac_vap->st_cap_flag.bit_wpa))
    {
        *puc_ie_len = 0;
        return;
    }

    ul_group_suit      = mac_mib_get_wpa_group_suite(pst_mac_vap);
    uc_pair_suites_num = mac_mib_get_wpa_pair_suites(pst_mac_vap, aul_pcip);
    uc_akm_suites_num  = mac_mib_get_wpa_akm_suites(pst_mac_vap, aul_akm);

    if ((0 == uc_pair_suites_num) || (0 == uc_akm_suites_num))
    {
        *puc_ie_len = 0;
        return;
    }

    /*************************************************************************/
    /*                  RSN Element Format              */
    /* --------------------------------------------------------------------- */
    /* |Element ID | Length | Version | Group Cipher Suite | Pairwise Cipher */
    /* --------------------------------------------------------------------- */
    /* | 1         | 1      | 2       |      4             |     2           */
    /* --------------------------------------------------------------------- */
    /* --------------------------------------------------------------------- */
    /* Suite | Pairwise Cipher Suite List | AKM Suite Count | AKM Suite List */
    /* --------------------------------------------------------------------- */
    /*       | 4-m                        |     2          | 4-n             */
    /* --------------------------------------------------------------------- */
    /* --------------------------------------------------------------------- */
    /* |RSN Capabilities|PMKID Count|PMKID List|Group Management Cipher Suite*/
    /* --------------------------------------------------------------------- */
    /* |    2           |    2      |16 -s     |         4                 | */
    /* --------------------------------------------------------------------- */
    /*                                                                       */
    /*************************************************************************/
    uc_index = MAC_IE_HDR_LEN;

    /* ����RSN ie��EID */
    puc_buffer[0] = MAC_EID_WPA;

    oal_memcopy(puc_buffer + uc_index, g_auc_wpa_oui_etc, MAC_OUI_LEN);

    uc_index += MAC_OUI_LEN;

    puc_buffer[uc_index++] = MAC_OUITYPE_WPA;/* ���WPA ��OUI ���� */


    /* ����version�ֶ� */
    puc_buffer[uc_index++] = MAC_RSN_IE_VERSION;
    puc_buffer[uc_index++] = 0;

    /* ����Group Cipher Suite */
    /*************************************************************************/
    /*                  Group Cipher Suite                                   */
    /* --------------------------------------------------------------------- */
    /*                  | OUI | Suite type |                                 */
    /* --------------------------------------------------------------------- */
    /*          Octets: |  3  |     1      |                                 */
    /* --------------------------------------------------------------------- */
    /*************************************************************************/
    oal_memcopy(&puc_buffer[uc_index], (oal_uint8 *)(&ul_group_suit), 4);
    uc_index += 4;

    /* ���óɶԼ����׼� */
    puc_buffer[uc_index++] = uc_pair_suites_num;
    puc_buffer[uc_index++] = 0;

    for (uc_loop = 0; uc_loop < uc_pair_suites_num; uc_loop++)
    {
        oal_memcopy(&puc_buffer[uc_index], (oal_uint8 *)(&aul_pcip[uc_loop]), 4);
        uc_index += 4;
    }

    /* ������֤�׼��� */
    puc_buffer[uc_index++] = uc_akm_suites_num;
    puc_buffer[uc_index++] = 0;

    for (uc_loop = 0; uc_loop < uc_akm_suites_num; uc_loop++)
    {
        oal_memcopy(&puc_buffer[uc_index], (oal_uint8 *)(&aul_akm[uc_loop]), 4);
        uc_index += 4;
    }

    /* ����wpa element�ĳ��� */
    puc_buffer[1] = uc_index - MAC_IE_HDR_LEN;

    *puc_ie_len = uc_index;

    if (OAL_PTR_NULL != g_st_mac_frame_rom_cb.set_wpa_ie_cb)
    {
        g_st_mac_frame_rom_cb.set_wpa_ie_cb(pst_mac_vap, puc_buffer, puc_ie_len);
    }

}
#ifdef _PRE_WLAN_FEATURE_STA_UAPSD

oal_uint8 mac_get_uapsd_config_max_sp_len_etc(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->st_sta_uapsd_cfg.uc_max_sp_len;
}

oal_uint8 mac_get_uapsd_config_ac_etc(mac_vap_stru *pst_mac_vap, oal_uint8 uc_ac)
{
    if (uc_ac < WLAN_WME_AC_BUTT)
    {
        return pst_mac_vap->st_sta_uapsd_cfg.uc_trigger_enabled[uc_ac];
    }

    return 0;
}

oal_void  mac_set_qos_info_wmm_sta_etc(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_buffer)
{
    oal_uint8                   uc_qos_info = 0;
    oal_uint8                   uc_max_sp_bits;
    oal_uint8                   uc_max_sp_length;

    /* QoS Information field                                          */
    /* -------------------------------------------------------------- */
    /* | B0    | B1    | B2    | B3    | B4      | B5:B6 | B7       | */
    /* -------------------------------------------------------------- */
    /* | AC_VO | AC_VI | AC_BK | AC_BE |         | Max SP|          | */
    /* | U-APSD| U-APSD| U-APSD| U-APSD| Reserved| Length| Reserved | */
    /* | Flag  | Flag  | Flag  | Flag  |         |       |          | */
    /* -------------------------------------------------------------- */

    /* Set the UAPSD configuration information in the QoS info field if the  */
    /* BSS type is Infrastructure and the AP supports UAPSD.                 */
    if(OAL_TRUE == pst_mac_vap->uc_uapsd_cap)
    {
        uc_max_sp_length  = mac_get_uapsd_config_max_sp_len_etc(pst_mac_vap);
        /*lint -e734*/
        uc_qos_info |= (mac_get_uapsd_config_ac_etc(pst_mac_vap, WLAN_WME_AC_VO) << 0);
        uc_qos_info |= (mac_get_uapsd_config_ac_etc(pst_mac_vap, WLAN_WME_AC_VI) << 1);
        uc_qos_info |= (mac_get_uapsd_config_ac_etc(pst_mac_vap, WLAN_WME_AC_BK) << 2);
        uc_qos_info |= (mac_get_uapsd_config_ac_etc(pst_mac_vap, WLAN_WME_AC_BE) << 3);
        /*lint +e734*/
       if(uc_max_sp_length <= 6)
       {
           uc_max_sp_bits = uc_max_sp_length >> 1;

           uc_qos_info |= ((uc_max_sp_bits & 0x03) << 5);
        }
    }

    puc_buffer[0] = uc_qos_info;
}
#endif

oal_void  mac_set_qos_info_field(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_buffer)
{
    mac_qos_info_stru *pst_qos_info = (mac_qos_info_stru *)puc_buffer;

    /* QoS Information field  (AP MODE)            */
    /* ------------------------------------------- */
    /* | B0:B3               | B4:B6    | B7     | */
    /* ------------------------------------------- */
    /* | Parameter Set Count | Reserved | U-APSD | */

    if (WLAN_VAP_MODE_BSS_AP == pst_mac_vap->en_vap_mode)
    {
        pst_qos_info->bit_params_count = pst_mac_vap->uc_wmm_params_update_count;
        pst_qos_info->bit_uapsd        = pst_mac_vap->st_cap_flag.bit_uapsd;
        pst_qos_info->bit_resv         = 0;
    }


    /* QoS Information field  (STA MODE)           */
    /* ---------------------------------------------------------------------------------------------------------- */
    /* | B0              | B1              | B2              | B3              | B4      |B5   B6      | B7     | */
    /* ---------------------------------------------------------------------------------------------------------- */
    /* |AC_VO U-APSD Flag|AC_VI U-APSD Flag|AC_BK U-APSD Flag|AC_BE U-APSD Flag|Reserved |Max SP Length|Reserved| */
    /* ---------------------------------------------------------------------------------------------------------- */
    if (WLAN_VAP_MODE_BSS_STA == pst_mac_vap->en_vap_mode)
    {
    #ifdef _PRE_WLAN_FEATURE_STA_UAPSD
        mac_set_qos_info_wmm_sta_etc(pst_mac_vap, puc_buffer);
	#else
		puc_buffer[0] = 0;
		puc_buffer[0] |= 0x0;
    #endif
    }
}


oal_void  mac_set_wmm_ac_params(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_buffer, wlan_wme_ac_type_enum_uint8 en_ac)
{
    mac_wmm_ac_params_stru *pst_ac_params = (mac_wmm_ac_params_stru *)puc_buffer;

    /* AC_** Parameter Record field               */
    /* ------------------------------------------ */
    /* | Byte 1    | Byte 2        | Byte 3:4   | */
    /* ------------------------------------------ */
    /* | ACI/AIFSN | ECWmin/ECWmax | TXOP Limit | */
    /* ------------------------------------------ */

    /* ACI/AIFSN Field                    */
    /* ---------------------------------- */
    /* | B0:B3 | B4  | B5:B6 | B7       | */
    /* ---------------------------------- */
    /* | AIFSN | ACM | ACI   | Reserved | */
    /* ---------------------------------- */
    /* AIFSN */
    pst_ac_params->bit_aifsn = mac_mib_get_EDCATableAIFSN(pst_mac_vap, en_ac);

    /* ACM */
#if 0 // TBD:APUT WMMAC
    if ((OAL_TRUE == g_en_wmmac_switch_etc) && ((WLAN_WME_AC_VO == en_ac) || (WLAN_WME_AC_VI == en_ac)))
    {
        pst_ac_params->bit_acm = OAL_TRUE;
    }
    else
#endif //_PRE_WLAN_FEATURE_WMMAC
    {
        pst_ac_params->bit_acm = mac_mib_get_EDCATableMandatory(pst_mac_vap, en_ac);
    }

    /* ACI */
    pst_ac_params->bit_aci = mac_mib_get_EDCATableIndex(pst_mac_vap, en_ac) - 1;

    pst_ac_params->bit_resv = 0;

    /* ECWmin/ECWmax Field */
    /* ------------------- */
    /* | B0:B3  | B4:B7  | */
    /* ------------------- */
    /* | ECWmin | ECWmax | */
    /* ------------------- */
    /* ECWmin */
    pst_ac_params->bit_ecwmin = mac_mib_get_EDCATableCWmin(pst_mac_vap, en_ac);

    /* ECWmax */
    pst_ac_params->bit_ecwmax = mac_mib_get_EDCATableCWmax(pst_mac_vap, en_ac);

    /* TXOP Limit. The value saved in MIB is in usec while the value to be   */
    /* set in this element should be in multiple of 32us                     */
    pst_ac_params->us_txop = (oal_uint16)(mac_mib_get_EDCATableTXOPLimit(pst_mac_vap, en_ac) >> 5);

}


oal_void  mac_set_wmm_params_ie_etc(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_bool_enum_uint8 en_is_qos, oal_uint8 *puc_ie_len)
{
    oal_uint8     uc_index;
    mac_vap_stru *pst_mac_vap = (mac_vap_stru *)pst_vap;

//    if (OAL_FALSE == pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11QosOptionImplemented)
    if(OAL_FALSE == en_is_qos)
    {
        *puc_ie_len = 0;

        return;
    }

    /* WMM Parameter Element Format                                          */
    /* --------------------------------------------------------------------- */
    /* | 3Byte | 1        | 1           | 1             | 1        | 1     | */
    /* --------------------------------------------------------------------- */
    /* | OUI   | OUI Type | OUI Subtype | Version field | QoS Info | Resvd | */
    /* --------------------------------------------------------------------- */
    /* | 4              | 4              | 4              | 4              | */
    /* --------------------------------------------------------------------- */
    /* | AC_BE ParamRec | AC_BK ParamRec | AC_VI ParamRec | AC_VO ParamRec | */
    /* --------------------------------------------------------------------- */
    puc_buffer[0] = MAC_EID_WMM;
    puc_buffer[1] = MAC_WMM_PARAM_LEN;

    uc_index = MAC_IE_HDR_LEN;

    /* OUI */
    oal_memcopy(&puc_buffer[uc_index], g_auc_wmm_oui_etc, MAC_OUI_LEN);
    uc_index += MAC_OUI_LEN;

    /* OUI Type */
    puc_buffer[uc_index++] = MAC_OUITYPE_WMM;

    /* OUI Subtype */
    puc_buffer[uc_index++] = MAC_OUISUBTYPE_WMM_PARAM;

    /* Version field */
    puc_buffer[uc_index++] = MAC_OUI_WMM_VERSION;

    /* QoS Information Field */
    mac_set_qos_info_field(pst_mac_vap, &puc_buffer[uc_index]);
    uc_index += MAC_QOS_INFO_LEN;

    /* Reserved */
    puc_buffer[uc_index++] = 0;

    /* Set the AC_BE, AC_BK, AC_VI, AC_VO Parameter Record fields */
    mac_set_wmm_ac_params(pst_mac_vap, &puc_buffer[uc_index], WLAN_WME_AC_BE);
    uc_index += MAC_AC_PARAM_LEN;

    mac_set_wmm_ac_params(pst_mac_vap, &puc_buffer[uc_index], WLAN_WME_AC_BK);
    uc_index += MAC_AC_PARAM_LEN;

    mac_set_wmm_ac_params(pst_mac_vap, &puc_buffer[uc_index], WLAN_WME_AC_VI);
    uc_index += MAC_AC_PARAM_LEN;

    mac_set_wmm_ac_params(pst_mac_vap, &puc_buffer[uc_index], WLAN_WME_AC_VO);

    *puc_ie_len = MAC_IE_HDR_LEN + MAC_WMM_PARAM_LEN;
}


oal_void mac_set_exsup_rates_ie_etc(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
    mac_vap_stru     *pst_mac_vap   = (mac_vap_stru *)pst_vap;
    mac_rateset_stru *pst_rates_set;
    oal_uint8         uc_nrates;
    oal_uint8         uc_idx;

    pst_rates_set = &(pst_mac_vap->st_curr_sup_rates.st_rate);

    /* STAȫ�ŵ�ɨ��ʱ����Ƶ������supported rates */
    if (WLAN_VAP_MODE_BSS_STA == pst_mac_vap->en_vap_mode &&
#ifdef _PRE_WLAN_FEATURE_11AX
       (WLAN_VHT_MODE == pst_mac_vap->en_protocol || WLAN_HE_MODE == pst_mac_vap->en_protocol))
#else
        WLAN_VHT_MODE == pst_mac_vap->en_protocol)
#endif
    {
        pst_rates_set = &(pst_mac_vap->ast_sta_sup_rates_ie[pst_mac_vap->st_channel.en_band].st_rate);
    }

    /***************************************************************************
                   -----------------------------------------------
                   |ElementID | Length | Extended Supported Rates|
                   -----------------------------------------------
       Octets:     |1         | 1      | 1-255                   |
                   -----------------------------------------------
    ***************************************************************************/
    if (pst_rates_set->uc_rs_nrates <= MAC_MAX_SUPRATES)
    {
        *puc_ie_len = 0;

        return;
    }

    puc_buffer[0] = MAC_EID_XRATES;
    uc_nrates     = pst_rates_set->uc_rs_nrates - MAC_MAX_SUPRATES;
    puc_buffer[1] = uc_nrates;

    for (uc_idx = 0; uc_idx < uc_nrates; uc_idx++)
    {
        puc_buffer[MAC_IE_HDR_LEN + uc_idx] = pst_rates_set->ast_rs_rates[uc_idx + MAC_MAX_SUPRATES].uc_mac_rate;
    }

    *puc_ie_len = MAC_IE_HDR_LEN + uc_nrates;
}
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)

oal_void mac_set_bssload_ie(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
    mac_bss_load_stru  *pst_bss_load;
    mac_vap_stru       *pst_mac_vap  = (mac_vap_stru *)pst_vap;
#ifdef _PRE_WLAN_FEATURE_11K_EXTERN
    mac_device_stru         *pst_mac_device;
    hal_to_dmac_device_stru *pst_hal_device;
#endif

    if (OAL_FALSE == mac_mib_get_dot11QosOptionImplemented(pst_mac_vap) ||
        OAL_FALSE == mac_mib_get_dot11QBSSLoadImplemented(pst_mac_vap))
    {
        *puc_ie_len = 0;

        return;
    }

#ifdef _PRE_WLAN_FEATURE_11K_EXTERN
    /*��ȡmac_device_stru*/
    pst_mac_device = mac_res_get_dev_etc(pst_mac_vap->uc_device_id);
    if ( OAL_PTR_NULL == pst_mac_device)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_RRM, "mac_set_bssload_ie::pst_mac_device null");
        return;
    }
#endif
    /***************************************************************************
    ------------------------------------------------------------------------
    |EID |Len |StationCount |ChannelUtilization |AvailableAdmissionCapacity|
    ------------------------------------------------------------------------
    |1   |1   |2            |1                  |2                         |
    ------------------------------------------------------------------------
    ***************************************************************************/
    puc_buffer[0] = MAC_EID_QBSS_LOAD;
    puc_buffer[1] = MAC_BSS_LOAD_IE_LEN;

    pst_bss_load = (mac_bss_load_stru *)(puc_buffer + MAC_IE_HDR_LEN);

    pst_bss_load->us_sta_count = OAL_BYTEORDER_TO_LE16(pst_mac_vap->us_user_nums);

    pst_bss_load->uc_chan_utilization = pst_mac_vap->uc_channel_utilization;
#ifdef _PRE_WLAN_FEATURE_11K_EXTERN
    pst_hal_device = (MAC_GET_DMAC_VAP(pst_mac_vap))->pst_hal_device;
    if (WLAN_BAND_WIDTH_20M == pst_mac_vap->st_channel.en_bandwidth)
    {
        pst_bss_load->uc_chan_utilization = pst_hal_device->st_chan_ratio.us_chan_ratio_20M*255/1000;
    }
    else if ( WLAN_BAND_WIDTH_40PLUS == pst_mac_vap->st_channel.en_bandwidth
        || WLAN_BAND_WIDTH_40MINUS == pst_mac_vap->st_channel.en_bandwidth)
    {
        pst_bss_load->uc_chan_utilization = pst_hal_device->st_chan_ratio.us_chan_ratio_40M*255/1000;
    }
    else if ( pst_mac_vap->st_channel.en_bandwidth >= WLAN_BAND_WIDTH_80PLUSPLUS
        && pst_mac_vap->st_channel.en_bandwidth <= WLAN_BAND_WIDTH_80MINUSMINUS)
    {
        pst_bss_load->uc_chan_utilization = pst_hal_device->st_chan_ratio.us_chan_ratio_80M*255/1000;
    }
    else
    {
        pst_bss_load->uc_chan_utilization = 0;
    }
#endif
    pst_bss_load->us_aac = 0;   /* TBD */

    *puc_ie_len = MAC_IE_HDR_LEN + MAC_BSS_LOAD_IE_LEN;
}
#endif
#else

oal_void mac_set_bssload_ie(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
    mac_bss_load_stru  *pst_bss_load;
    mac_vap_stru       *pst_mac_vap  = (mac_vap_stru *)pst_vap;
#ifdef _PRE_WLAN_FEATURE_11K_EXTERN
    mac_device_stru     *pst_mac_device;
#endif

    if (OAL_FALSE == mac_mib_get_dot11QosOptionImplemented(pst_mac_vap) ||
        OAL_FALSE == mac_mib_get_dot11QBSSLoadImplemented(pst_mac_vap))
    {
        *puc_ie_len = 0;

        return;
    }

#ifdef _PRE_WLAN_FEATURE_11K_EXTERN
    /*��ȡmac_device_stru*/
    pst_mac_device = mac_res_get_dev_etc(pst_mac_vap->uc_device_id);
    if ( OAL_PTR_NULL == pst_mac_device)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_RRM, "mac_set_bssload_ie::pst_mac_device null");
        return;
    }
#endif
    /***************************************************************************
    ------------------------------------------------------------------------
    |EID |Len |StationCount |ChannelUtilization |AvailableAdmissionCapacity|
    ------------------------------------------------------------------------
    |1   |1   |2            |1                  |2                         |
    ------------------------------------------------------------------------
    ***************************************************************************/
    puc_buffer[0] = MAC_EID_QBSS_LOAD;
    puc_buffer[1] = MAC_BSS_LOAD_IE_LEN;

    pst_bss_load = (mac_bss_load_stru *)(puc_buffer + MAC_IE_HDR_LEN);

    pst_bss_load->us_sta_count = OAL_BYTEORDER_TO_LE16(pst_mac_vap->us_user_nums);

    pst_bss_load->uc_chan_utilization = pst_mac_vap->uc_channel_utilization;
#ifdef _PRE_WLAN_FEATURE_11K_EXTERN
    if (WLAN_BAND_WIDTH_20M == pst_mac_vap->st_channel.en_bandwidth)
    {
        pst_bss_load->uc_chan_utilization = pst_mac_device->st_chan_ratio.us_chan_ratio_20M*255/1000;
    }
    else if ( WLAN_BAND_WIDTH_40PLUS == pst_mac_vap->st_channel.en_bandwidth
        || WLAN_BAND_WIDTH_40MINUS == pst_mac_vap->st_channel.en_bandwidth)
    {
        pst_bss_load->uc_chan_utilization = pst_mac_device->st_chan_ratio.us_chan_ratio_40M*255/1000;
    }
    else if ( pst_mac_vap->st_channel.en_bandwidth >= WLAN_BAND_WIDTH_80PLUSPLUS
        && pst_mac_vap->st_channel.en_bandwidth <= WLAN_BAND_WIDTH_80MINUSMINUS)
    {
        pst_bss_load->uc_chan_utilization = pst_mac_device->st_chan_ratio.us_chan_ratio_80M*255/1000;
    }
    else
    {
        pst_bss_load->uc_chan_utilization = 0;
    }
#endif
    pst_bss_load->us_aac = 0;   /* TBD */

    *puc_ie_len = MAC_IE_HDR_LEN + MAC_BSS_LOAD_IE_LEN;
}
#endif

#ifdef _PRE_WLAN_FEATURE_SMPS

oal_uint8 mac_calc_smps_field(oal_uint8 en_smps)
{

    if (WLAN_MIB_MIMO_POWER_SAVE_DYNAMIC == en_smps)
    {
        return MAC_SMPS_DYNAMIC_MODE;
    }
    else if (WLAN_MIB_MIMO_POWER_SAVE_STATIC == en_smps)
    {
        return MAC_SMPS_STATIC_MODE;
    }

    return MAC_SMPS_MIMO_MODE;
}
#endif


oal_void mac_set_ht_capinfo_field(oal_void *pst_vap, oal_uint8 *puc_buffer)
{
    mac_vap_stru                   *pst_mac_vap = (mac_vap_stru *)pst_vap;

    mac_frame_ht_cap_stru *pst_ht_capinfo = (mac_frame_ht_cap_stru *)puc_buffer;

    /*********************** HT Capabilities Info field*************************
    ----------------------------------------------------------------------------
     |-------------------------------------------------------------------|
     | LDPC   | Supp    | SM    | Green- | Short  | Short  |  Tx  |  Rx  |
     | Coding | Channel | Power | field  | GI for | GI for | STBC | STBC |
     | Cap    | Wth Set | Save  |        | 20 MHz | 40 MHz |      |      |
     |-------------------------------------------------------------------|
     |   B0   |    B1   |B2   B3|   B4   |   B5   |    B6  |  B7  |B8  B9|
     |-------------------------------------------------------------------|

     |-------------------------------------------------------------------|
     |    HT     |  Max   | DSS/CCK | Reserved | 40 MHz     | L-SIG TXOP |
     |  Delayed  | AMSDU  | Mode in |          | Intolerant | Protection |
     | Block-Ack | Length | 40MHz   |          |            | Support    |
     |-------------------------------------------------------------------|
     |    B10    |   B11  |   B12   |   B13    |    B14     |    B15     |
     |-------------------------------------------------------------------|
    ***************************************************************************/
    /* ��ʼ��0 */
    puc_buffer[0] = 0;
    puc_buffer[1] = 0;

    pst_ht_capinfo->bit_ldpc_coding_cap = mac_mib_get_LDPCCodingOptionImplemented(pst_mac_vap);

    /* ������֧�ֵ��ŵ���ȼ�"��0:��20MHz����; 1:20MHz��40MHz���� */
    pst_ht_capinfo->bit_supported_channel_width = (MAC_VAP_GET_CAP_BW(pst_mac_vap) > WLAN_BAND_WIDTH_20M) ? OAL_TRUE : OAL_FALSE;

#ifdef _PRE_WLAN_FEATURE_SMPS
    pst_ht_capinfo->bit_sm_power_save = mac_calc_smps_field(mac_mib_get_smps(pst_mac_vap));
#else
    pst_ht_capinfo->bit_sm_power_save = MAC_SMPS_MIMO_MODE;
#endif

    pst_ht_capinfo->bit_ht_green_field = mac_mib_get_HTGreenfieldOptionImplemented(pst_mac_vap);

    pst_ht_capinfo->bit_short_gi_20mhz = mac_mib_get_ShortGIOptionInTwentyImplemented(pst_mac_vap);
    pst_ht_capinfo->bit_short_gi_40mhz = mac_mib_get_ShortGIOptionInFortyImplemented(pst_mac_vap);

    pst_ht_capinfo->bit_tx_stbc = mac_mib_get_TxSTBCOptionImplemented(pst_mac_vap);
    pst_ht_capinfo->bit_rx_stbc = mac_mib_get_RxSTBCOptionImplemented(pst_mac_vap);

    pst_ht_capinfo->bit_ht_delayed_block_ack = mac_mib_get_dot11DelayedBlockAckOptionImplemented(pst_mac_vap);

    pst_ht_capinfo->bit_max_amsdu_length = mac_mib_get_max_amsdu_length(pst_mac_vap);

    /* �Ƿ��ھ���40MHz��������������20/40MHzģʽ��BSS��ʹ��DSSS/CCK */
    if (WLAN_BAND_2G == pst_mac_vap->st_channel.en_band)
    {
        if ((WLAN_LEGACY_11B_MODE    == pst_mac_vap->en_protocol) ||
            (WLAN_MIXED_ONE_11G_MODE == pst_mac_vap->en_protocol) ||
            (WLAN_MIXED_TWO_11G_MODE == pst_mac_vap->en_protocol) ||
            (WLAN_HT_MODE            == pst_mac_vap->en_protocol))
        {
            pst_ht_capinfo->bit_dsss_cck_mode_40mhz = pst_mac_vap->st_cap_flag.bit_dsss_cck_mode_40mhz;
        }
        else
        {
            pst_ht_capinfo->bit_dsss_cck_mode_40mhz = 0;
        }
    }
    else
    {
        pst_ht_capinfo->bit_dsss_cck_mode_40mhz = 0;
    }

    /* ����"40MHz������"��ֻ��2.4GHz����Ч */
    if (WLAN_BAND_2G == pst_mac_vap->st_channel.en_band)
    {
        pst_ht_capinfo->bit_forty_mhz_intolerant = mac_mib_get_FortyMHzIntolerant(pst_mac_vap);
    }
    else
    {
        /*5G 40MHz����������Ϊ0*/
        pst_ht_capinfo->bit_forty_mhz_intolerant = OAL_FALSE;
    }

    pst_ht_capinfo->bit_lsig_txop_protection = mac_mib_get_LsigTxopFullProtectionActivated(pst_mac_vap);
}


oal_void mac_set_ampdu_params_field(oal_void * pst_vap, oal_uint8 *puc_buffer)
{
    mac_vap_stru *pst_mac_vap                = (mac_vap_stru *)pst_vap;
    mac_ampdu_params_stru *pst_ampdu_params  = (mac_ampdu_params_stru *)puc_buffer;

     /******************** AMPDU Parameters Field ******************************
      |-----------------------------------------------------------------------|
      | Maximum AMPDU Length Exponent | Minimum MPDU Start Spacing | Reserved |
      |-----------------------------------------------------------------------|
      | B0                         B1 | B2                      B4 | B5     B7|
      |-----------------------------------------------------------------------|
     **************************************************************************/
    /* ��ʼ��0 */
    puc_buffer[0] = 0;

    pst_ampdu_params->bit_max_ampdu_len_exponent = mac_mib_get_max_ampdu_len_exponent(pst_mac_vap);
    pst_ampdu_params->bit_min_mpdu_start_spacing = mac_mib_get_min_mpdu_start_spacing(pst_mac_vap);
}


oal_void mac_set_sup_mcs_set_field(oal_void * pst_vap, oal_uint8 *puc_buffer)
{
    mac_vap_stru *pst_mac_vap              = (mac_vap_stru *)pst_vap;
    mac_sup_mcs_set_stru *pst_sup_mcs_set  = (mac_sup_mcs_set_stru *)puc_buffer;

    /************************* Supported MCS Set Field **********************
    |-------------------------------------------------------------------|
    | Rx MCS Bitmask | Reserved | Rx Highest    | Reserved |  Tx MCS    |
    |                |          | Supp Data Rate|          |Set Defined |
    |-------------------------------------------------------------------|
    | B0         B76 | B77  B79 | B80       B89 | B90  B95 |    B96     |
    |-------------------------------------------------------------------|
    | Tx Rx MCS Set  | Tx Max Number     |   Tx Unequal     | Reserved  |
    |  Not Equal     | Spat Stream Supp  | Modulation Supp  |           |
    |-------------------------------------------------------------------|
    |      B97       | B98           B99 |       B100       | B101 B127 |
    |-------------------------------------------------------------------|
    *************************************************************************/

    /* ��ʼ���� */
    OAL_MEMZERO(puc_buffer, OAL_SIZEOF(mac_sup_mcs_set_stru));

    oal_memcopy(pst_sup_mcs_set->auc_rx_mcs, mac_mib_get_SupportedMCSRx(pst_mac_vap), WLAN_HT_MCS_BITMASK_LEN);

    pst_sup_mcs_set->bit_rx_highest_rate = mac_mib_get_HighestSupportedDataRate(pst_mac_vap);

    if (OAL_TRUE == mac_mib_get_TxMCSSetDefined(pst_mac_vap))
    {
        pst_sup_mcs_set->bit_tx_mcs_set_def = 1;

        if (OAL_TRUE == mac_mib_get_TxRxMCSSetNotEqual(pst_mac_vap))
        {
            pst_sup_mcs_set->bit_tx_rx_not_equal = 1;

            pst_sup_mcs_set->bit_tx_max_stream = mac_mib_get_TxMaximumNumberSpatialStreamsSupported(pst_mac_vap);
            if (OAL_TRUE == mac_mib_get_TxUnequalModulationSupported(pst_mac_vap))
            {
                pst_sup_mcs_set->bit_tx_unequal_modu = 1;
            }

        }
    }

    /* reserveλ��0 */
    pst_sup_mcs_set->bit_resv1 = 0;
    pst_sup_mcs_set->bit_resv2 = 0;

}

oal_void mac_set_ht_extcap_field(oal_void * pst_vap, oal_uint8 *puc_buffer)
{
    mac_vap_stru *pst_mac_vap      = (mac_vap_stru *)pst_vap;
    mac_ext_cap_stru *pst_ext_cap  = (mac_ext_cap_stru *)puc_buffer;

    /***************** HT Extended Capabilities Field **********************
      |-----------------------------------------------------------------|
      | PCO | PCO Trans | Reserved | MCS  |  +HTC   |  RD    | Reserved |
      |     |   Time    |          | Fdbk | Support | Resp   |          |
      |-----------------------------------------------------------------|
      | B0  | B1     B2 | B3    B7 | B8 B9|   B10   |  B11   | B12  B15 |
      |-----------------------------------------------------------------|
    ***********************************************************************/
    /* ��ʼ��0 */
    puc_buffer[0] = 0;
    puc_buffer[1] = 0;

    if(OAL_TRUE == mac_mib_get_pco_option_implemented(pst_mac_vap))
    {
        pst_ext_cap->bit_pco = 1;
        pst_ext_cap->bit_pco_trans_time = mac_mib_get_transition_time(pst_mac_vap);
    }

    pst_ext_cap->bit_mcs_fdbk = mac_mib_get_mcs_fdbk(pst_mac_vap);

    pst_ext_cap->bit_htc_sup = mac_mib_get_htc_sup(pst_mac_vap);

    pst_ext_cap->bit_rd_resp = mac_mib_get_rd_rsp(pst_mac_vap);
}

oal_void mac_set_txbf_cap_field(oal_void * pst_vap, oal_uint8 *puc_buffer)
{
    mac_vap_stru *pst_mac_vap        = (mac_vap_stru *)pst_vap;
    mac_txbf_cap_stru *pst_txbf_cap  = (mac_txbf_cap_stru *)puc_buffer;

    /*************** Transmit Beamforming Capability Field *********************
     |-------------------------------------------------------------------------|
     |   Implicit | Rx Stagg | Tx Stagg  | Rx NDP   | Tx NDP   | Implicit      |
     |   TxBF Rx  | Sounding | Sounding  | Capable  | Capable  | TxBF          |
     |   Capable  | Capable  | Capable   |          |          | Capable       |
     |-------------------------------------------------------------------------|
     |      B0    |     B1   |    B2     |   B3     |   B4     |    B5         |
     |-------------------------------------------------------------------------|
     |              | Explicit | Explicit Non- | Explicit      | Explicit      |
     |  Calibration | CSI TxBF | Compr Steering| Compr steering| TxBF CSI      |
     |              | Capable  | Cap.          | Cap.          | Feedback      |
     |-------------------------------------------------------------------------|
     |  B6       B7 |   B8     |       B9      |       B10     | B11  B12      |
     |-------------------------------------------------------------------------|
     | Explicit Non- | Explicit | Minimal  | CSI Num of | Non-Compr Steering   |
     | Compr BF      | Compr BF | Grouping | Beamformer | Num of Beamformer    |
     | Fdbk Cap.     | Fdbk Cap.|          | Ants Supp  | Ants Supp            |
     |-------------------------------------------------------------------------|
     | B13       B14 | B15  B16 | B17  B18 | B19    B20 | B21        B22       |
     |-------------------------------------------------------------------------|
     | Compr Steering    | CSI Max Num of     |   Channel     |                |
     | Num of Beamformer | Rows Beamformer    | Estimation    | Reserved       |
     | Ants Supp         | Supported          | Capability    |                |
     |-------------------------------------------------------------------------|
     | B23           B24 | B25            B26 | B27       B28 | B29  B31       |
     |-------------------------------------------------------------------------|
    ***************************************************************************/
    /* ��ʼ���� */
    puc_buffer[0] = 0;
    puc_buffer[1] = 0;
    puc_buffer[2] = 0;
    puc_buffer[3] = 0;

    /* ָʾSTA�Ƿ���Խ���staggered sounding֡ */
    pst_txbf_cap->bit_rx_stagg_sounding = mac_mib_get_ReceiveStaggerSoundingOptionImplemented(pst_mac_vap);

    /* ָʾSTA�Ƿ���Է���staggered sounding֡.*/
    pst_txbf_cap->bit_tx_stagg_sounding = mac_mib_get_TransmitStaggerSoundingOptionImplemented(pst_mac_vap);

    pst_txbf_cap->bit_rx_ndp = mac_mib_get_ReceiveNDPOptionImplemented(pst_mac_vap);

    pst_txbf_cap->bit_tx_ndp = mac_mib_get_TransmitNDPOptionImplemented(pst_mac_vap);

    pst_txbf_cap->bit_implicit_txbf = mac_mib_get_ImplicitTransmitBeamformingOptionImplemented(pst_mac_vap);

    pst_txbf_cap->bit_calibration = mac_mib_get_CalibrationOptionImplemented(pst_mac_vap);

    pst_txbf_cap->bit_explicit_csi_txbf = mac_mib_get_ExplicitCSITransmitBeamformingOptionImplemented(pst_mac_vap);

    pst_txbf_cap->bit_explicit_noncompr_steering = mac_mib_get_ExplicitNonCompressedBeamformingMatrixOptionImplemented(pst_mac_vap);


    /* Indicates if this STA can apply transmit beamforming using compressed */
    /* beamforming feedback matrix explicit feedback in its tranmission.     */
    /*************************************************************************/
    /*************************************************************************/
    /* No MIB exists, not clear what needs to be set    B10                  */
    /*************************************************************************/
    /*************************************************************************/

    /* Indicates if this receiver can return CSI explicit feedback */
    pst_txbf_cap->bit_explicit_txbf_csi_fdbk = mac_mib_get_ExplicitTransmitBeamformingCSIFeedbackOptionImplemented(pst_mac_vap);

    /* Indicates if this receiver can return non-compressed beamforming      */
    /* feedback matrix explicit feedback.                                    */
    pst_txbf_cap->bit_explicit_noncompr_bf_fdbk = mac_mib_get_ExplicitNonCompressedBeamformingFeedbackOptionImplemented(pst_mac_vap);

    /* Indicates if this STA can apply transmit beamforming using explicit   */
    /* compressed beamforming feedback matrix.                               */
    pst_txbf_cap->bit_explicit_compr_bf_fdbk = mac_mib_get_ExplicitCompressedBeamformingFeedbackOptionImplemented(pst_mac_vap);

    /* Indicates the minimal grouping used for explicit feedback reports */
    /*************************************************************************/
    /*************************************************************************/
    /*  No MIB exists, not clear what needs to be set       B17              */
    /*************************************************************************/
    /*************************************************************************/
    /* Indicates the maximum number of beamformer antennas the beamformee    */
    /* can support when CSI feedback is required.                            */
    pst_txbf_cap->bit_csi_num_bf_antssup = mac_mib_get_NumberBeamFormingCSISupportAntenna(pst_mac_vap);

    /* Indicates the maximum number of beamformer antennas the beamformee    */
    /* can support when non-compressed beamforming feedback matrix is        */
    /* required                                                              */
    pst_txbf_cap->bit_noncompr_steering_num_bf_antssup = mac_mib_get_NumberNonCompressedBeamformingMatrixSupportAntenna(pst_mac_vap);

    /* Indicates the maximum number of beamformer antennas the beamformee   */
    /* can support when compressed beamforming feedback matrix is required  */
    pst_txbf_cap->bit_compr_steering_num_bf_antssup = mac_mib_get_NumberCompressedBeamformingMatrixSupportAntenna(pst_mac_vap) - 1;

    /* Indicates the maximum number of rows of CSI explicit feedback from    */
    /* beamformee that the beamformer can support when CSI feedback is       */
    /* required                                                              */
    /*************************************************************************/
    /*************************************************************************/
    /*  No MIB exists, not clear what needs to be set     B25                */
    /*************************************************************************/
    /*************************************************************************/

    /* Indicates maximum number of space time streams (columns of the MIMO   */
    /* channel matrix) for which channel dimensions can be simultaneously    */
    /* estimated. When staggered sounding is supported this limit applies    */
    /* independently to both the data portion and to the extension portion   */
    /* of the long training fields.                                          */
    /*************************************************************************/
    /*************************************************************************/
    /*      No MIB exists, not clear what needs to be set          B27       */
    /*************************************************************************/
    /*************************************************************************/
#ifdef _PRE_WLAN_FEATURE_TXBF
	pst_txbf_cap->bit_explicit_compr_Steering = pst_mac_vap->st_txbf_add_cap.bit_exp_comp_txbf_cap;
	pst_txbf_cap->bit_chan_estimation = pst_mac_vap->st_txbf_add_cap.bit_channel_est_cap;
    pst_txbf_cap->bit_minimal_grouping = pst_mac_vap->st_txbf_add_cap.bit_min_grouping;
    pst_txbf_cap->bit_csi_maxnum_rows_bf_sup = pst_mac_vap->st_txbf_add_cap.bit_csi_bfee_max_rows;
	pst_txbf_cap->bit_implicit_txbf_rx = pst_mac_vap->st_txbf_add_cap.bit_imbf_receive_cap;
#endif
}

oal_void mac_set_asel_cap_field(oal_void * pst_vap, oal_uint8 *puc_buffer)
{
    mac_vap_stru *pst_mac_vap        = (mac_vap_stru *)pst_vap;
    mac_asel_cap_stru *pst_asel_cap  = (mac_asel_cap_stru *)puc_buffer;

    /************** Antenna Selection Capability Field *************************
     |-------------------------------------------------------------------|
     |  Antenna  | Explicit CSI  | Antenna Indices | Explicit | Antenna  |
     | Selection | Fdbk based TX | Fdbk based TX   | CSI Fdbk | Indices  |
     |  Capable  | ASEL Capable  | ASEL Capable    | Capable  | Fdbk Cap.|
     |-------------------------------------------------------------------|
     |    B0     |     B1        |      B2         |    B3    |    B4    |
     |-------------------------------------------------------------------|

     |------------------------------------|
     |  RX ASEL |   Transmit   |          |
     |  Capable |   Sounding   | Reserved |
     |          | PPDU Capable |          |
     |------------------------------------|
     |    B5    |     B6       |    B7    |
     |------------------------------------|
    ***************************************************************************/
    /* ��ʼ��0 */
    puc_buffer[0] = 0;

    /* ָʾSTA�Ƿ�֧������ѡ�� */
    pst_asel_cap->bit_asel = mac_mib_get_AntennaSelectionOptionImplemented(pst_mac_vap);

    /* ָʾSTA�Ƿ���л�����ʾCSI(�ŵ�״̬��Ϣ)�����ķ�������ѡ������ */
    pst_asel_cap->bit_explicit_sci_fdbk_tx_asel = mac_mib_get_TransmitExplicitCSIFeedbackASOptionImplemented(pst_mac_vap);

    /* ָʾSTA�Ƿ���л�������ָ�������ķ�������ѡ������ */
    pst_asel_cap->bit_antenna_indices_fdbk_tx_asel = mac_mib_get_TransmitIndicesFeedbackASOptionImplemented(pst_mac_vap);

    /* ָʾSTA������ѡ���֧�����Ƿ��ܹ�����CSI(�ŵ�״̬��Ϣ)���ṩCSI���� */
    pst_asel_cap->bit_explicit_csi_fdbk = mac_mib_get_ExplicitCSIFeedbackASOptionImplemented(pst_mac_vap);

    /* Indicates whether or not this STA can conduct antenna indices */
    /* selection computation and feedback the results in support of  */
    /* Antenna Selection.*/
    pst_asel_cap->bit_antenna_indices_fdbk = mac_mib_get_TransmitExplicitCSIFeedbackASOptionImplemented(pst_mac_vap);

    /* ָʾSTA�Ƿ���н�������ѡ������ */
    pst_asel_cap->bit_rx_asel = mac_mib_get_ReceiveAntennaSelectionOptionImplemented(pst_mac_vap);

    /* ָʾSTA�Ƿ��ܹ���ÿһ�������ж�����Ϊ����ѡ�����з���̽��PPDU */
    pst_asel_cap->bit_trans_sounding_ppdu = mac_mib_get_TransmitSoundingPPDUOptionImplemented(pst_mac_vap);
}


oal_void mac_set_timeout_interval_ie_etc(oal_void *pst_vap,
                                           oal_uint8 *puc_buffer,
                                           oal_uint8 *puc_ie_len,
                                           oal_uint32 ul_type,
                                           oal_uint32 ul_timeout)
{
#if (_PRE_WLAN_FEATURE_PMF != _PRE_PMF_NOT_SUPPORT)
    mac_Timeout_Interval_type_enum en_tie_type;

    en_tie_type = (mac_Timeout_Interval_type_enum)ul_type;
    *puc_ie_len = 0;

    /* �ж��Ƿ���Ҫ����timeout_interval IE */
    if (MAC_TIE_BUTT <= en_tie_type)
    {
        return;
    }


    /* Timeout Interval Parameter Element Format
    -----------------------------------------------------------------------
    |ElementID | Length | Timeout Interval Type| Timeout Interval Value  |
    -----------------------------------------------------------------------
    |1         | 1      | 1                    |  4                      |
    -----------------------------------------------------------------------
    */
    puc_buffer[0] = MAC_EID_TIMEOUT_INTERVAL;
    puc_buffer[1] = MAC_TIMEOUT_INTERVAL_INFO_LEN;
    puc_buffer[2] = en_tie_type;

    /* ����Timeout Interval Value */
    puc_buffer[3] = ul_timeout & 0x000000FF;
    puc_buffer[4] = (ul_timeout & 0x0000FF00)>>8;
    puc_buffer[5] = (ul_timeout & 0x00FF0000)>>16;
    puc_buffer[6] = (ul_timeout & 0xFF000000)>>24;

    *puc_ie_len = MAC_IE_HDR_LEN + MAC_TIMEOUT_INTERVAL_INFO_LEN;
#else
    *puc_ie_len = 0;
#endif
    return;
}



 oal_void mac_set_ht_capabilities_ie_etc(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
 {
     mac_vap_stru *pst_mac_vap        = (mac_vap_stru *)pst_vap;

     if ((OAL_TRUE != mac_mib_get_HighThroughputOptionImplemented(pst_mac_vap))||
         (((OAL_TRUE == mac_is_wep_enabled(pst_mac_vap))||
         (OAL_TRUE == mac_is_tkip_only(pst_mac_vap))) && ( WLAN_VAP_MODE_BSS_AP == pst_mac_vap->en_vap_mode)))
     {
         *puc_ie_len = 0;
         return;
     }

    /***************************************************************************
    -------------------------------------------------------------------------
    |EID |Length |HT Capa. Info |A-MPDU Parameters |Supported MCS Set|
    -------------------------------------------------------------------------
    |1   |1      |2             |1                 |16               |
    -------------------------------------------------------------------------
    |HT Extended Cap. |Transmit Beamforming Cap. |ASEL Cap.          |
    -------------------------------------------------------------------------
    |2                |4                         |1                  |
    -------------------------------------------------------------------------
    ***************************************************************************/
    *puc_buffer       = MAC_EID_HT_CAP;
    *(puc_buffer + 1) = MAC_HT_CAP_LEN;

    puc_buffer += MAC_IE_HDR_LEN;

    /* ���ht capabilities information����Ϣ */
    mac_set_ht_capinfo_field(pst_vap, puc_buffer);
    puc_buffer += MAC_HT_CAPINFO_LEN;

    /* ���A-MPDU parameters����Ϣ */
    mac_set_ampdu_params_field(pst_vap, puc_buffer);
    puc_buffer += MAC_HT_AMPDU_PARAMS_LEN;

    /* ���supported MCS set����Ϣ */
    mac_set_sup_mcs_set_field(pst_vap, puc_buffer);
    puc_buffer += MAC_HT_SUP_MCS_SET_LEN;

    /* ���ht extended capabilities����Ϣ */
    mac_set_ht_extcap_field(pst_vap, puc_buffer);
    puc_buffer += MAC_HT_EXT_CAP_LEN;

    /* ��� transmit beamforming capabilities����Ϣ */
    mac_set_txbf_cap_field(pst_vap, puc_buffer);
    puc_buffer += MAC_HT_TXBF_CAP_LEN;

    /* ���asel(antenna selection) capabilities����Ϣ*/
    mac_set_asel_cap_field(pst_vap, puc_buffer);

    *puc_ie_len = MAC_IE_HDR_LEN + MAC_HT_CAP_LEN;

    if (OAL_PTR_NULL != g_st_mac_frame_rom_cb.set_ht_cap_ie_cb)
    {
        g_st_mac_frame_rom_cb.set_ht_cap_ie_cb(pst_mac_vap, puc_buffer, puc_ie_len);
    }
}


oal_void mac_set_ht_opern_ie_etc(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
    mac_vap_stru        *pst_mac_vap     = (mac_vap_stru *)pst_vap;
    mac_ht_opern_stru   *pst_ht_opern;
    oal_uint8           uc_obss_non_ht = 0;

    if ((OAL_TRUE != mac_mib_get_HighThroughputOptionImplemented(pst_mac_vap))||
        (((OAL_TRUE == mac_is_wep_enabled(pst_mac_vap))||
         (OAL_TRUE == mac_is_tkip_only(pst_mac_vap))) && ( WLAN_VAP_MODE_BSS_AP == pst_mac_vap->en_vap_mode)))
    {
        *puc_ie_len = 0;
         return;
    }

    /***************************************************************************
      ----------------------------------------------------------------------
      |EID |Length |PrimaryChannel |HT Operation Information |Basic MCS Set|
      ----------------------------------------------------------------------
      |1   |1      |1              |5                        |16           |
      ----------------------------------------------------------------------
    ***************************************************************************/

    /************************ HT Information Field ****************************
     |--------------------------------------------------------------------|
     | Primary | Seconday  | STA Ch | RIFS |           reserved           |
     | Channel | Ch Offset | Width  | Mode |                              |
     |--------------------------------------------------------------------|
     |    1    | B0     B1 |   B2   |  B3  |    B4                     B7 |
     |--------------------------------------------------------------------|

     |----------------------------------------------------------------|
     |     HT     | Non-GF STAs | resv      | OBSS Non-HT  | Reserved |
     | Protection |   Present   |           | STAs Present |          |
     |----------------------------------------------------------------|
     | B0     B1  |     B2      |    B3     |     B4       | B5   B15 |
     |----------------------------------------------------------------|

     |-------------------------------------------------------------|
     | Reserved |  Dual  |  Dual CTS  | Seconday | LSIG TXOP Protn |
     |          | Beacon | Protection |  Beacon  | Full Support    |
     |-------------------------------------------------------------|
     | B0    B5 |   B6   |     B7     |     B8   |       B9        |
     |-------------------------------------------------------------|

     |---------------------------------------|
     |  PCO   |  PCO  | Reserved | Basic MCS |
     | Active | Phase |          |    Set    |
     |---------------------------------------|
     |  B10   |  B11  | B12  B15 |    16     |
     |---------------------------------------|
    **************************************************************************/

    *puc_buffer = MAC_EID_HT_OPERATION;

    *(puc_buffer + 1) = MAC_HT_OPERN_LEN;
    pst_ht_opern = (mac_ht_opern_stru *)(puc_buffer + MAC_IE_HDR_LEN);

    /* ���ŵ���� */
    pst_ht_opern->uc_primary_channel = pst_mac_vap->st_channel.uc_chan_number;

    /* ����"���ŵ�ƫ����" */
    switch (pst_mac_vap->st_channel.en_bandwidth)
    {
        case WLAN_BAND_WIDTH_40PLUS:
        case WLAN_BAND_WIDTH_80PLUSPLUS:
        case WLAN_BAND_WIDTH_80PLUSMINUS:
            pst_ht_opern->bit_secondary_chan_offset = MAC_SCA;
            break;

        case WLAN_BAND_WIDTH_40MINUS:
        case WLAN_BAND_WIDTH_80MINUSPLUS:
        case WLAN_BAND_WIDTH_80MINUSMINUS:
            pst_ht_opern->bit_secondary_chan_offset = MAC_SCB;
            break;

#ifdef _PRE_WLAN_FEATURE_160M
        case WLAN_BAND_WIDTH_160PLUSPLUSPLUS:
        case WLAN_BAND_WIDTH_160PLUSPLUSMINUS:
        case WLAN_BAND_WIDTH_160PLUSMINUSPLUS:
        case WLAN_BAND_WIDTH_160PLUSMINUSMINUS:
            pst_ht_opern->bit_secondary_chan_offset = MAC_SCA;
            break;

        case WLAN_BAND_WIDTH_160MINUSPLUSPLUS:
        case WLAN_BAND_WIDTH_160MINUSPLUSMINUS:
        case WLAN_BAND_WIDTH_160MINUSMINUSPLUS:
        case WLAN_BAND_WIDTH_160MINUSMINUSMINUS:
            pst_ht_opern->bit_secondary_chan_offset = MAC_SCB;
            break;
#endif

        default:
            pst_ht_opern->bit_secondary_chan_offset = MAC_SCN;
            break;
    }

    /* ����"STA�ŵ����"����BSS�����ŵ���� >= 40MHzʱ����Ҫ����field����Ϊ1 */
    pst_ht_opern->bit_sta_chan_width = (pst_mac_vap->st_channel.en_bandwidth > WLAN_BAND_WIDTH_20M) ? 1 : 0;

    /* ָʾ�����������Ƿ�����ʹ�ü�С��֡��� */
    pst_ht_opern->bit_rifs_mode = mac_mib_get_RifsMode(pst_mac_vap);

    /* B4-B7���� */
    pst_ht_opern->bit_resv1 = 0;

    /* ָʾht����ı���Ҫ�� */
    pst_ht_opern->bit_HT_protection = mac_mib_get_HtProtection(pst_mac_vap);

    /* Non-GF STAs */
    pst_ht_opern->bit_nongf_sta_present = mac_mib_get_NonGFEntitiesPresent(pst_mac_vap);

    /* B3 resv */
    pst_ht_opern->bit_resv2 = 0;

    /* B4  obss_nonht_sta_present */
    if ((0 != pst_mac_vap->st_protection.bit_obss_non_ht_present) ||
         (0 != pst_mac_vap->st_protection.uc_sta_non_ht_num))
    {
        uc_obss_non_ht = 1;
    }
    pst_ht_opern->bit_obss_nonht_sta_present = uc_obss_non_ht;

    /* B5-B15 ���� */
    pst_ht_opern->bit_resv3 = 0;
    pst_ht_opern->bit_resv4 = 0;

    /* B0-B5 ���� */
    pst_ht_opern->bit_resv5 = 0;

    /* B6  dual_beacon */
    pst_ht_opern->bit_dual_beacon = 0;

    /* Dual CTS protection */
    pst_ht_opern->bit_dual_cts_protection = mac_mib_get_DualCTSProtection(pst_mac_vap);

    /* secondary_beacon: Set to 0 in a primary beacon */
    pst_ht_opern->bit_secondary_beacon = 0;

    /* BSS support L-SIG TXOP Protection */
    pst_ht_opern->bit_lsig_txop_protection_full_support = mac_mib_get_LsigTxopFullProtectionActivated(pst_mac_vap);

    /* PCO active */
    pst_ht_opern->bit_pco_active = mac_mib_get_PCOActivated(pst_mac_vap);

    /* PCO phase */
    pst_ht_opern->bit_pco_phase = 0;

    /* B12-B15  ���� */
    pst_ht_opern->bit_resv6 = 0;

    /* Basic MCS Set: set all bit zero,Indicates the MCS values that are supported by all HT STAs in the BSS. */
    OAL_MEMZERO(pst_ht_opern->auc_basic_mcs_set, MAC_HT_BASIC_MCS_SET_LEN);

    *puc_ie_len = MAC_IE_HDR_LEN + MAC_HT_OPERN_LEN;

    if (OAL_PTR_NULL != g_st_mac_frame_rom_cb.set_ht_opern_ie_cb)
    {
        g_st_mac_frame_rom_cb.set_ht_opern_ie_cb(pst_mac_vap, puc_buffer, puc_ie_len);
    }

}


oal_void mac_set_obss_scan_params_etc(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
    mac_vap_stru                *pst_mac_vap = (mac_vap_stru *)pst_vap;
    mac_obss_scan_params_stru   *pst_obss_scan;
    oal_uint32                  ul_ret;

    if (OAL_TRUE != mac_mib_get_HighThroughputOptionImplemented(pst_mac_vap))
    {
        *puc_ie_len = 0;
         return;
    }

    if ((WLAN_BAND_2G != pst_mac_vap->st_channel.en_band) ||
        (OAL_TRUE != mac_mib_get_FortyMHzOperationImplemented(pst_mac_vap)))
    {
        *puc_ie_len = 0;
        return;
    }

    /***************************************************************************
     |ElementID |Length |OBSS    |OBSS   |BSS Channel   |OBSS Scan  |OBSS Scan   |
     |          |       |Scan    |Scan   |Width Trigger |Passive    |Active Total|
     |          |       |Passive |Active |Scan Interval |Total Per  |Per         |
     |          |       |Dwell   |Dwell  |              |Channel    |Channel     |
     ----------------------------------------------------------------------------
     |1         |1      |2       |2      |2             |2          |2           |
     ----------------------------------------------------------------------------
     |BSS Width   |OBSS Scan|
     |Channel     |Activity |
     |Transition  |Threshold|
     |Delay Factor|         |
     ------------------------
     |2           |2        |
    ***************************************************************************/
    puc_buffer[0] = MAC_EID_OBSS_SCAN;
    puc_buffer[1] = MAC_OBSS_SCAN_IE_LEN;

    pst_obss_scan = (mac_obss_scan_params_stru *)(puc_buffer + MAC_IE_HDR_LEN);

    ul_ret = mac_mib_get_OBSSScanPassiveDwell(pst_mac_vap);
    pst_obss_scan->us_passive_dwell = (oal_uint16)(OAL_BYTEORDER_TO_LE32(ul_ret));

    ul_ret = mac_mib_get_OBSSScanActiveDwell(pst_mac_vap);
    pst_obss_scan->us_active_dwell  = (oal_uint16)(OAL_BYTEORDER_TO_LE32(ul_ret));

    ul_ret = mac_mib_get_BSSWidthTriggerScanInterval(pst_mac_vap);
    pst_obss_scan->us_scan_interval = (oal_uint16)(OAL_BYTEORDER_TO_LE32(ul_ret));

    ul_ret = mac_mib_get_OBSSScanPassiveTotalPerChannel(pst_mac_vap);
    pst_obss_scan->us_passive_total_per_chan  = (oal_uint16)(OAL_BYTEORDER_TO_LE32(ul_ret));

    ul_ret = mac_mib_get_OBSSScanActiveTotalPerChannel(pst_mac_vap);
    pst_obss_scan->us_active_total_per_chan   = (oal_uint16)(OAL_BYTEORDER_TO_LE32(ul_ret));

    ul_ret = mac_mib_get_BSSWidthChannelTransitionDelayFactor(pst_mac_vap);
    pst_obss_scan->us_transition_delay_factor = (oal_uint16)(OAL_BYTEORDER_TO_LE32(ul_ret));

    ul_ret = mac_mib_get_OBSSScanActivityThreshold(pst_mac_vap);
    pst_obss_scan->us_scan_activity_thresh    = (oal_uint16)(OAL_BYTEORDER_TO_LE32(ul_ret));

    *puc_ie_len = MAC_IE_HDR_LEN + MAC_OBSS_SCAN_IE_LEN;
}


oal_void mac_set_ext_capabilities_ie_etc(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
    mac_vap_stru            *pst_mac_vap = (mac_vap_stru *)pst_vap;
    mac_ext_cap_ie_stru     *pst_ext_cap;
#ifdef _PRE_WLAN_FEATURE_11KV_INTERFACE
    oal_uint8               uc_ie_set_len = 0;
#endif

    if ((OAL_TRUE != mac_mib_get_HighThroughputOptionImplemented(pst_mac_vap))||
        (((OAL_TRUE == mac_is_wep_enabled(pst_mac_vap))||
         (OAL_TRUE == mac_is_tkip_only(pst_mac_vap))) && ( WLAN_VAP_MODE_BSS_AP == pst_mac_vap->en_vap_mode)))
    {
        *puc_ie_len = 0;
        /* �������������ext cap ie ��ʹ��hera��Ʒ�ӿڴ���������Ϣ���Ӹ�IE */
#ifdef _PRE_WLAN_FEATURE_11KV_INTERFACE
        mac_set_ie_field((oal_void *)(pst_mac_vap->pst_excap_ie_info), puc_buffer, &uc_ie_set_len);
        *puc_ie_len += uc_ie_set_len;
#endif
        return;
    }

    /***************************************************************************
                         ----------------------------------
                         |Element ID |Length |Capabilities|
                         ----------------------------------
          Octets:        |1          |1      |n           |
                         ----------------------------------
    -------------------------------------------------------------------------------------------------------------------
    |  B0       | B1 | B2             | B3   | B4   |  B5  |  B6    |  B7   | ...|  B38    |   B39      |...|  B62
    ----------------------------------------------------------------------------
    |20/40 coex |resv|extended channel| resv | PSMP | resv | S-PSMP | Event |    |TDLS Pro-  TDLS Channel     Operating
                                                                                             Switching        mode
    |mgmt supp  |    |switching       |      |      |      |        |       | ...| hibited | Prohibited |...| notify
    -------------------------------------------------------------------------------------------------------------------
    ***************************************************************************/
    puc_buffer[0] = MAC_EID_EXT_CAPS;
    puc_buffer[1] = MAC_XCAPS_EX_LEN;

    /* ��ʼ���� */
    OAL_MEMZERO(puc_buffer + MAC_IE_HDR_LEN, OAL_SIZEOF(mac_ext_cap_ie_stru));

    pst_ext_cap = (mac_ext_cap_ie_stru *)(puc_buffer + MAC_IE_HDR_LEN);

    /* ����20/40 BSS Coexistence Management Support fieid */
    if ((OAL_TRUE == mac_mib_get_2040BSSCoexistenceManagementSupport(pst_mac_vap)) &&
        (WLAN_BAND_2G == pst_mac_vap->st_channel.en_band) &&
        (OAL_TRUE == mac_mib_get_FortyMHzOperationImplemented(pst_mac_vap)))
    {
        pst_ext_cap->bit_2040_coexistence_mgmt = 1;
    }

    /* ����TDLS prohibited */
    pst_ext_cap->bit_tdls_prhibited =  pst_mac_vap->st_cap_flag.bit_tdls_prohibited;

    /* ����TDLS channel switch prohibited */
    pst_ext_cap->bit_tdls_channel_switch_prhibited = pst_mac_vap->st_cap_flag.bit_tdls_channel_switch_prohibited;

#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
    /* �����11ac վ�� ����OPMODE NOTIFY��־ */
    if ((OAL_TRUE == mac_mib_get_VHTOptionImplemented(pst_mac_vap))&&
        (((OAL_FALSE == mac_is_wep_enabled(pst_mac_vap))&&
        (OAL_FALSE == mac_is_tkip_only(pst_mac_vap))) || ( WLAN_VAP_MODE_BSS_AP != pst_mac_vap->en_vap_mode)))
    {
        pst_ext_cap->bit_operating_mode_notification = mac_mib_get_OperatingModeNotificationImplemented(pst_mac_vap);
    }
#endif

#ifdef _PRE_WLAN_FEATURE_PROXY_ARP
    if ((OAL_PTR_NULL != pst_mac_vap->pst_vap_proxyarp)
        && (OAL_TRUE == pst_mac_vap->pst_vap_proxyarp->en_is_proxyarp)
        && (WLAN_VAP_MODE_BSS_AP == pst_mac_vap->en_vap_mode))
    {
        pst_ext_cap->bit_proxyarp = 1;
    }
    else
    {
        pst_ext_cap->bit_proxyarp = 0;
    }
#endif
#ifdef _PRE_WLAN_FEATURE_HS20
    /*  ���֧��Hotspot2.0��Interwoking��־  */
    pst_ext_cap->bit_interworking = 1;
#else
    pst_ext_cap->bit_interworking = 0;
#endif

    *puc_ie_len = MAC_IE_HDR_LEN + MAC_XCAPS_EX_LEN;
    /* �����������ext cap ie ��ʹ��hera��Ʒ�ӿڴ���������Ϣ�޸ĸ�IE */
#ifdef _PRE_WLAN_FEATURE_11KV_INTERFACE
    uc_ie_set_len = *puc_ie_len;    /* IE�Ѿ����� uc_ie_set_len�ں����ڲ�����0 */
    mac_set_ie_field((oal_void *)(pst_mac_vap->pst_excap_ie_info), puc_buffer, &uc_ie_set_len);
#endif

    if (OAL_PTR_NULL != g_st_mac_frame_rom_cb.set_ext_cap_ie_cb)
    {
        g_st_mac_frame_rom_cb.set_ext_cap_ie_cb(pst_mac_vap, puc_buffer, puc_ie_len);
    }
}


oal_void  mac_set_vht_capinfo_field_etc(oal_void *pst_vap, oal_uint8 *puc_buffer)
{
    mac_vap_stru           *pst_mac_vap     = (mac_vap_stru *)pst_vap;
    mac_vht_cap_info_stru  *pst_vht_capinfo = (mac_vht_cap_info_stru *)puc_buffer;
#ifdef _PRE_WLAN_FEATURE_M2S
    mac_user_stru          *pst_mac_user;
#endif

    /*********************** VHT ������Ϣ�� ************************************
    ----------------------------------------------------------------------------
     |-----------------------------------------------------------------------|
     | Max    | Supp    | RX   | Short GI| Short  | Tx   |  Rx  |  SU        |
     | MPDU   | Channel | LDPC | for 80  | GI for | STBC | STBC | Beamformer |
     | Length | Wth Set |      |         | 160MHz |      |      | Capable    |
     |-----------------------------------------------------------------------|
     | B0 B1  | B2 B3   | B4   |   B5    |   B6   |  B7  |B8 B10|   B11      |
     |-----------------------------------------------------------------------|
     |-----------------------------------------------------------------------|
     | SU         | Compressed   | Num of    | MU        | MU        | VHT   |
     | Beamformee | Steering num | Sounding  | Beamformer| Beamformee| TXOP  |
     | Capable    | of bf ant sup| Dimensions| Capable   | Capable   | PS    |
     |-----------------------------------------------------------------------|
     |    B12     | B13      B15 | B16    B18|   B19     |    B20    | B21   |
     |-----------------------------------------------------------------------|
     |-----------------------------------------------------------------------|
     | +HTC   | Max AMPDU| VHT Link  | Rx ANT     | Tx ANT     |   Resv      |
     | VHT    | Length   | Adaptation| Pattern    | Pattern    |             |
     | Capable| Exponent | Capable   | Consistency| Consistency|             |
     |-----------------------------------------------------------------------|
     | B22    | B23  B25 | B26   B27 |   B28      |   B29      |  B30 B31    |
     |-----------------------------------------------------------------------|
    ***************************************************************************/
    pst_vht_capinfo->bit_max_mpdu_length         = mac_mib_get_maxmpdu_length(pst_mac_vap);

    /* ����"��֧�ֵ��ŵ���ȼ�"��0:neither 160 nor 80+80:; 1:160MHz; 2:160/80+80MHz */
    pst_vht_capinfo->bit_supported_channel_width = mac_mib_get_VHTChannelWidthOptionImplemented(pst_mac_vap);

    pst_vht_capinfo->bit_rx_ldpc                 = mac_mib_get_VHTLDPCCodingOptionImplemented(pst_mac_vap);
    pst_vht_capinfo->bit_short_gi_80mhz          = mac_mib_get_VHTShortGIOptionIn80Implemented(pst_mac_vap);
    pst_vht_capinfo->bit_short_gi_160mhz         = mac_mib_get_VHTShortGIOptionIn160and80p80Implemented(pst_mac_vap);
    pst_vht_capinfo->bit_tx_stbc                 = mac_mib_get_VHTTxSTBCOptionImplemented(pst_mac_vap);
    pst_vht_capinfo->bit_rx_stbc                 = (OAL_TRUE == mac_mib_get_VHTRxSTBCOptionImplemented(pst_mac_vap)) ? 1 : 0;
    pst_vht_capinfo->bit_su_beamformer_cap       = mac_mib_get_VHTSUBeamformerOptionImplemented(pst_mac_vap);
    pst_vht_capinfo->bit_su_beamformee_cap       = mac_mib_get_VHTSUBeamformeeOptionImplemented(pst_mac_vap);
    pst_vht_capinfo->bit_num_bf_ant_supported    = mac_mib_get_VHTBeamformeeNTxSupport(pst_mac_vap) - 1;

#ifdef _PRE_WLAN_FEATURE_M2S
    /* �ο����,���ֶθ��ݶԶ�num of sounding���Լ�������ȡ����*/
    pst_mac_user = mac_res_get_mac_user_etc(pst_mac_vap->us_assoc_vap_id);
    if(WLAN_VAP_MODE_BSS_STA == pst_mac_vap->en_vap_mode && OAL_PTR_NULL != pst_mac_user)
    {
        pst_vht_capinfo->bit_num_bf_ant_supported = OAL_MIN(pst_vht_capinfo->bit_num_bf_ant_supported,
                                                                pst_mac_user->st_vht_hdl.bit_num_sounding_dim);
    }
#endif

    /* sounding dim��bfer������ */
#if (WLAN_MAX_NSS_NUM >= WLAN_DOUBLE_NSS)
    pst_vht_capinfo->bit_num_sounding_dim        = mac_mib_get_VHTNumberSoundingDimensions(pst_mac_vap);
#else
    pst_vht_capinfo->bit_num_sounding_dim        = 0;
#endif

    pst_vht_capinfo->bit_mu_beamformer_cap       = mac_mib_get_VHTMUBeamformerOptionImplemented(pst_mac_vap);
    pst_vht_capinfo->bit_mu_beamformee_cap       = mac_mib_get_VHTMUBeamformeeOptionImplemented(pst_mac_vap);
    pst_vht_capinfo->bit_vht_txop_ps             = mac_mib_get_txopps(pst_mac_vap);
    pst_vht_capinfo->bit_htc_vht_capable         = mac_mib_get_vht_ctrl_field_cap(pst_mac_vap);
    pst_vht_capinfo->bit_max_ampdu_len_exp       = mac_mib_get_vht_max_rx_ampdu_factor(pst_mac_vap);


     pst_vht_capinfo->bit_vht_link_adaptation    = 0;
     pst_vht_capinfo->bit_rx_ant_pattern         = 0;   /* �ڸù����в��ı�����ģʽ����Ϊ1,���ı�����Ϊ0 */
     pst_vht_capinfo->bit_tx_ant_pattern         = 0;   /* �ڸù����в��ı�����ģʽ����Ϊ1,���ı�����Ϊ0 */

    /* resvλ��0 */
    pst_vht_capinfo->bit_resv = 0;

    if (OAL_PTR_NULL != g_st_mac_frame_rom_cb.set_vht_capinfo_field_cb)
    {
        g_st_mac_frame_rom_cb.set_vht_capinfo_field_cb(pst_mac_vap, puc_buffer);
    }

}


oal_void  mac_set_vht_supported_mcsset_field_etc(oal_void *pst_vap, oal_uint8 *puc_buffer)
{
    mac_vap_stru              *pst_mac_vap    = (mac_vap_stru *)pst_vap;
    mac_vht_sup_mcs_set_stru  *pst_vht_mcsset = (mac_vht_sup_mcs_set_stru *)puc_buffer;

    /*********************** VHT ֧�ֵ�MCS�� ************************************
    ----------------------------------------------------------------------------
     |-----------------------------------------------------------------------|
     | Rx MCS Map | Rx Highest Supported | Resv    | Tx MCS Map  |
     |            | Long gi Data Rate    |         |             |
     |-----------------------------------------------------------------------|
     | B0     B15 | B16              B28 | B29 B31 | B32     B47 |
     |-----------------------------------------------------------------------|
     |-----------------------------------------------------------------------|
     | Tx Highest Supported |  Resv   |
     | Long gi Data Rate    |         |
     |-----------------------------------------------------------------------|
     |  B48             B60 | B61 B63 |
     |-----------------------------------------------------------------------|
    ***************************************************************************/
    pst_vht_mcsset->bit_rx_mcs_map      = mac_mib_get_vht_rx_mcs_map(pst_mac_vap);
    pst_vht_mcsset->bit_rx_highest_rate = mac_mib_get_us_rx_highest_rate(pst_mac_vap);
    pst_vht_mcsset->bit_tx_mcs_map      = mac_mib_get_vht_tx_mcs_map(pst_mac_vap);
    pst_vht_mcsset->bit_tx_highest_rate = mac_mib_get_us_tx_highest_rate(pst_mac_vap);

    /* resv��0 */
    pst_vht_mcsset->bit_resv  = 0;
    pst_vht_mcsset->bit_resv2 = 0;

}


oal_void  mac_set_vht_capabilities_ie_etc(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
    mac_vap_stru  *pst_mac_vap = (mac_vap_stru *)pst_vap;
    if ((OAL_TRUE != mac_mib_get_VHTOptionImplemented(pst_mac_vap))||
        (((OAL_TRUE == mac_is_wep_enabled(pst_mac_vap))||
         (OAL_TRUE == mac_is_tkip_only(pst_mac_vap))) && ( WLAN_VAP_MODE_BSS_AP == pst_mac_vap->en_vap_mode))
    #ifdef _PRE_WLAN_FEATURE_11AC2G
        || ((OAL_FALSE == pst_mac_vap->st_cap_flag.bit_11ac2g) && (WLAN_BAND_2G == pst_mac_vap->st_channel.en_band))
    #endif /* _PRE_WLAN_FEATURE_11AC2G */
        )
    {
        *puc_ie_len = 0;
        return;
    }

    /***************************************************************************
    -------------------------------------------------------------------------
    |EID |Length |VHT Capa. Info |VHT Supported MCS Set|
    -------------------------------------------------------------------------
    |1   |1      | 4             | 8                   |
    -------------------------------------------------------------------------
    ***************************************************************************/
    puc_buffer[0] = MAC_EID_VHT_CAP;
    puc_buffer[1] = MAC_VHT_CAP_IE_LEN;

    puc_buffer += MAC_IE_HDR_LEN;

    mac_set_vht_capinfo_field_etc(pst_vap, puc_buffer);

    puc_buffer += MAC_VHT_CAP_INFO_FIELD_LEN;

    mac_set_vht_supported_mcsset_field_etc(pst_vap, puc_buffer);

    *puc_ie_len = MAC_IE_HDR_LEN + MAC_VHT_CAP_IE_LEN;

    if (OAL_PTR_NULL != g_st_mac_frame_rom_cb.set_vht_cap_ie_cb)
    {
        g_st_mac_frame_rom_cb.set_vht_cap_ie_cb(pst_mac_vap, puc_buffer, puc_ie_len);
    }

}


oal_void  mac_set_vht_opern_ie_etc(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
    mac_vap_stru        *pst_mac_vap = (mac_vap_stru *)pst_vap;
    mac_vht_opern_stru  *pst_vht_opern;

    if ((OAL_TRUE != mac_mib_get_VHTOptionImplemented(pst_mac_vap))||
        (((OAL_TRUE == mac_is_wep_enabled(pst_mac_vap))||
         (OAL_TRUE == mac_is_tkip_only(pst_mac_vap))) && ( WLAN_VAP_MODE_BSS_AP == pst_mac_vap->en_vap_mode))
#ifdef _PRE_WLAN_FEATURE_11AC2G
        || ((OAL_FALSE == pst_mac_vap->st_cap_flag.bit_11ac2g) && (WLAN_BAND_2G == pst_mac_vap->st_channel.en_band))
#endif /* _PRE_WLAN_FEATURE_11AC2G */
        )
    {
        *puc_ie_len = 0;
        return;
    }

    /***********************VHT Operation element*******************************
    -------------------------------------------------------------------------
            |EID |Length |VHT Opern Info |VHT Basic MCS Set|
    -------------------------------------------------------------------------
    Octes:  |1   |1      | 3             | 2               |
    -------------------------------------------------------------------------
    ***************************************************************************/
    puc_buffer[0] = MAC_EID_VHT_OPERN;
    puc_buffer[1] = MAC_VHT_INFO_IE_LEN;

    puc_buffer += MAC_IE_HDR_LEN;

    /**********************VHT Opern Info***************************************
    -------------------------------------------------------------------------
            | Channel Width | Channel Center | Channel Center |
            |               | Freq Seg0      | Freq Seg1      |
    -------------------------------------------------------------------------
    Octes:  |       1       |       1        |       1        |
    -------------------------------------------------------------------------
    ***************************************************************************/
    pst_vht_opern = (mac_vht_opern_stru *)puc_buffer;

    /*
        uc_channel_width��ȡֵ��0 -- 20/40M, 1 -- 80M, 2 -- 160M, 3 -- 80+80M
    */
    if ((pst_mac_vap->st_channel.en_bandwidth >= WLAN_BAND_WIDTH_80PLUSPLUS) && (pst_mac_vap->st_channel.en_bandwidth <= WLAN_BAND_WIDTH_80MINUSMINUS))
    {
        pst_vht_opern->uc_channel_width = WLAN_MIB_VHT_OP_WIDTH_80;
    }
#ifdef _PRE_WLAN_FEATURE_160M
    else if ((pst_mac_vap->st_channel.en_bandwidth >= WLAN_BAND_WIDTH_160PLUSPLUSPLUS) && (pst_mac_vap->st_channel.en_bandwidth <= WLAN_BAND_WIDTH_160MINUSMINUSMINUS))
    {
        pst_vht_opern->uc_channel_width = WLAN_MIB_VHT_OP_WIDTH_80;
    }
#endif
    else
    {
         pst_vht_opern->uc_channel_width = WLAN_MIB_VHT_OP_WIDTH_20_40;
    }

    switch (pst_mac_vap->st_channel.en_bandwidth)
    {

#ifdef _PRE_WLAN_FEATURE_160M
        /* ��20�ŵ�+1, ��40�ŵ�+1, ��80�ŵ�+1 */
        case WLAN_BAND_WIDTH_160PLUSPLUSPLUS:
            pst_vht_opern->uc_channel_center_freq_seg0 = pst_mac_vap->st_channel.uc_chan_number + 6;
            pst_vht_opern->uc_channel_center_freq_seg1 = pst_mac_vap->st_channel.uc_chan_number + 14;
        break;

        /* ��20�ŵ�+1, ��40�ŵ�+1, ��80�ŵ�-1 */
        case WLAN_BAND_WIDTH_160PLUSPLUSMINUS:
            pst_vht_opern->uc_channel_center_freq_seg0 = pst_mac_vap->st_channel.uc_chan_number + 6;
            pst_vht_opern->uc_channel_center_freq_seg1 = pst_mac_vap->st_channel.uc_chan_number - 2;
        break;

        /* ��20�ŵ�+1, ��40�ŵ�-1, ��80�ŵ�+1 */
        case WLAN_BAND_WIDTH_160PLUSMINUSPLUS:
            pst_vht_opern->uc_channel_center_freq_seg0 = pst_mac_vap->st_channel.uc_chan_number - 2;
            pst_vht_opern->uc_channel_center_freq_seg1 = pst_mac_vap->st_channel.uc_chan_number + 6;
        break;

        /* ��20�ŵ�+1, ��40�ŵ�-1, ��80�ŵ�-1 */
        case WLAN_BAND_WIDTH_160PLUSMINUSMINUS:
            pst_vht_opern->uc_channel_center_freq_seg0 = pst_mac_vap->st_channel.uc_chan_number - 2;
            pst_vht_opern->uc_channel_center_freq_seg1 = pst_mac_vap->st_channel.uc_chan_number - 10;
        break;

        /* ��20�ŵ�-1, ��40�ŵ�+1, ��80�ŵ�+1 */
        case WLAN_BAND_WIDTH_160MINUSPLUSPLUS:
            pst_vht_opern->uc_channel_center_freq_seg0 = pst_mac_vap->st_channel.uc_chan_number + 2;
            pst_vht_opern->uc_channel_center_freq_seg1 = pst_mac_vap->st_channel.uc_chan_number + 10;
        break;

        /* ��20�ŵ�-1, ��40�ŵ�+1, ��80�ŵ�-1 */
        case WLAN_BAND_WIDTH_160MINUSPLUSMINUS:
            pst_vht_opern->uc_channel_center_freq_seg0 = pst_mac_vap->st_channel.uc_chan_number + 2;
            pst_vht_opern->uc_channel_center_freq_seg1 = pst_mac_vap->st_channel.uc_chan_number - 6;
        break;

        /* ��20�ŵ�-1, ��40�ŵ�-1, ��80�ŵ�+1 */
        case WLAN_BAND_WIDTH_160MINUSMINUSPLUS:
            pst_vht_opern->uc_channel_center_freq_seg0 = pst_mac_vap->st_channel.uc_chan_number - 6;
            pst_vht_opern->uc_channel_center_freq_seg1 = pst_mac_vap->st_channel.uc_chan_number + 2;
        break;

        /* ��20�ŵ�-1, ��40�ŵ�-1, ��80�ŵ�-1 */
        case WLAN_BAND_WIDTH_160MINUSMINUSMINUS:
            pst_vht_opern->uc_channel_center_freq_seg0 = pst_mac_vap->st_channel.uc_chan_number - 6;
            pst_vht_opern->uc_channel_center_freq_seg1 = pst_mac_vap->st_channel.uc_chan_number - 14;
        break;
#endif
        case WLAN_BAND_WIDTH_80PLUSPLUS:
            /***********************************************************************
            | ��20 | ��20 | ��40       |
                          |
                          |����Ƶ���������20ƫ6���ŵ�
            ************************************************************************/
            pst_vht_opern->uc_channel_center_freq_seg0 = pst_mac_vap->st_channel.uc_chan_number + 6;
            break;

        case WLAN_BAND_WIDTH_80PLUSMINUS:
            /***********************************************************************
            | ��40        | ��20 | ��20 |
                          |
                          |����Ƶ���������20ƫ-2���ŵ�
            ************************************************************************/
            pst_vht_opern->uc_channel_center_freq_seg0 = pst_mac_vap->st_channel.uc_chan_number - 2;
            break;

        case WLAN_BAND_WIDTH_80MINUSPLUS:
            /***********************************************************************
            | ��20 | ��20 | ��40       |
                          |
                          |����Ƶ���������20ƫ2���ŵ�
            ************************************************************************/
            pst_vht_opern->uc_channel_center_freq_seg0 = pst_mac_vap->st_channel.uc_chan_number + 2;
            break;

        case WLAN_BAND_WIDTH_80MINUSMINUS:
            /***********************************************************************
            | ��40        | ��20 | ��20 |
                          |
                          |����Ƶ���������20ƫ-6���ŵ�
            ************************************************************************/
            pst_vht_opern->uc_channel_center_freq_seg0 = pst_mac_vap->st_channel.uc_chan_number - 6;
            break;

        case WLAN_BAND_WIDTH_40MINUS:
            /***********************************************************************
            | ��20 | ��20 |
            |
            | ����Ƶ���������20ƫ-2���ŵ�
            ************************************************************************/
            pst_vht_opern->uc_channel_center_freq_seg0 = pst_mac_vap->st_channel.uc_chan_number - 2;
            break;

        case WLAN_BAND_WIDTH_40PLUS:
            /***********************************************************************
            | ��20 | ��20 |
            |
            | ����Ƶ���������20ƫ+2���ŵ�
            ************************************************************************/
            pst_vht_opern->uc_channel_center_freq_seg0 = pst_mac_vap->st_channel.uc_chan_number + 2;
            break;

        case WLAN_BAND_WIDTH_20M:
            /* ����Ƶ�ʾ������ŵ�Ƶ��   */
            pst_vht_opern->uc_channel_center_freq_seg0 = pst_mac_vap->st_channel.uc_chan_number;
            break;

        default:
            break;

    }

    if ((pst_mac_vap->st_channel.en_bandwidth < WLAN_BAND_WIDTH_160PLUSPLUSPLUS) || (pst_mac_vap->st_channel.en_bandwidth > WLAN_BAND_WIDTH_160MINUSMINUSMINUS))
    {
        pst_vht_opern->uc_channel_center_freq_seg1 = 0;
    }
    pst_vht_opern->us_basic_mcs_set            = mac_mib_get_vht_rx_mcs_map(pst_mac_vap);

    *puc_ie_len = MAC_IE_HDR_LEN + MAC_VHT_INFO_IE_LEN;

    if (OAL_PTR_NULL != g_st_mac_frame_rom_cb.set_vht_opern_ie_cb)
    {
        g_st_mac_frame_rom_cb.set_vht_opern_ie_cb(pst_mac_vap, puc_buffer, puc_ie_len);
    }

}


oal_uint32  mac_set_csa_ie_etc(oal_uint8 uc_mode,oal_uint8 uc_channel, oal_uint8 uc_csa_cnt, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
    if (OAL_UNLIKELY((OAL_PTR_NULL == puc_buffer) || (OAL_PTR_NULL == puc_ie_len)))
    {
        return OAL_ERR_CODE_PTR_NULL;
    }

    /*  Channel Switch Announcement Information Element Format               */
    /* --------------------------------------------------------------------- */
    /* | Element ID | Length | Chnl Switch Mode | New Chnl | Ch Switch Cnt | */
    /* --------------------------------------------------------------------- */
    /* | 1          | 1      | 1                | 1        | 1             | */
    /* --------------------------------------------------------------------- */

   /* ����Channel Switch Announcement Element */
    puc_buffer[0] = MAC_EID_CHANSWITCHANN;
    puc_buffer[1] = MAC_CHANSWITCHANN_LEN;
    puc_buffer[2] = uc_mode;                       /* ask all associated STAs to stop transmission */
    puc_buffer[3] = uc_channel;
    puc_buffer[4] = uc_csa_cnt;

    *puc_ie_len = MAC_IE_HDR_LEN + MAC_CHANSWITCHANN_LEN;

    return OAL_SUCC;
}


oal_uint32  mac_set_csa_bw_ie(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
    mac_vap_stru                      *pst_mac_vap = (mac_vap_stru *)pst_vap;
    wlan_channel_bandwidth_enum_uint8  en_bw;
    oal_uint8                          uc_len;
    oal_uint8                          uc_channel;

    if (OAL_UNLIKELY((OAL_PTR_NULL == puc_buffer) || (OAL_PTR_NULL == puc_ie_len)|| (OAL_PTR_NULL == pst_mac_vap)))
    {
        return OAL_ERR_CODE_PTR_NULL;
    }

    en_bw       = pst_mac_vap->st_ch_switch_info.en_announced_bandwidth;
    uc_channel  = pst_mac_vap->st_ch_switch_info.uc_announced_channel;
    uc_len      = 0;
    /* ��װSecond channel offset IE */
    if (OAL_SUCC != mac_set_second_channel_offset_ie_etc(en_bw, puc_buffer, &uc_len))
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CSA, "{mac_set_csa_bw_ie::mac_set_second_channel_offset_ie_etc failed}");
        return 0;
    }

    puc_buffer += uc_len;
    *puc_ie_len = uc_len;

    if (OAL_TRUE == mac_mib_get_VHTOptionImplemented(pst_mac_vap))
    {
      /* 11AC Wide Bandwidth Channel Switch IE */
        uc_len = 0;
        if (OAL_SUCC != mac_set_11ac_wideband_ie_etc(uc_channel, en_bw, puc_buffer, &uc_len))
        {
            OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CSA, "{mac_set_csa_bw_ie::mac_set_11ac_wideband_ie_etc failed}");
            return 0;
        }
        *puc_ie_len += uc_len;
    }

    return OAL_SUCC;
}


oal_uint8*  mac_get_ssid_etc(oal_uint8 *puc_beacon_body, oal_int32 l_frame_body_len, oal_uint8 *puc_ssid_len)
{
    const oal_uint8   *puc_ssid_ie;
    oal_uint16         us_offset =  MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;

    /*************************************************************************/
    /*                       Beacon Frame - Frame Body                       */
    /* --------------------------------------------------------------------- */
    /* |Timestamp |BeaconInt |CapInfo |SSID |SupRates |DSParSet |TIM elm   | */
    /* --------------------------------------------------------------------- */
    /* |8         |2         |2       |2-34 |3-10     |3        |4-256     | */
    /* --------------------------------------------------------------------- */
    /*                                                                       */
    /*************************************************************************/

    /***************************************************************************
                    ----------------------------
                    |Element ID | Length | SSID|
                    ----------------------------
           Octets:  |1          | 1      | 0~32|
                    ----------------------------
    ***************************************************************************/
    /* ssid�ĳ��ȳ�ʼ��ֵΪ0 */
    *puc_ssid_len = 0;

    /* ���beacon֡����probe rsp֡�ĳ��ȵĺϷ��� */
    if (l_frame_body_len <= us_offset)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{mac_get_ssid_etc:: the length of beacon/probe rsp frame body is invalid.}");
        return OAL_PTR_NULL;
    }

    /* ����ssid��ie */
    puc_ssid_ie = mac_find_ie_etc(MAC_EID_SSID, (puc_beacon_body + us_offset), (oal_int32)(l_frame_body_len - us_offset));
    if ((OAL_PTR_NULL != puc_ssid_ie) && (puc_ssid_ie[1] < WLAN_SSID_MAX_LEN))
    {
        /* ��ȡssid ie�ĳ��� */
        *puc_ssid_len = puc_ssid_ie[1];

        return (oal_uint8 *)(puc_ssid_ie + MAC_IE_HDR_LEN);
    }

    OAM_WARNING_LOG0(0, OAM_SF_ANY, "{mac_get_ssid_etc:: ssid ie isn't found.}");
    return OAL_PTR_NULL;
}


oal_uint16  mac_get_beacon_period_etc(oal_uint8 *puc_beacon_body)
{
    /*************************************************************************/
    /*                       Beacon Frame - Frame Body                       */
    /* --------------------------------------------------------------------- */
    /* |Timestamp |BeaconInt |CapInfo |SSID |SupRates |DSParSet |TIM elm   | */
    /* --------------------------------------------------------------------- */
    /* |8         |2         |2       |2-34 |3-10     |3        |4-256     | */
    /* --------------------------------------------------------------------- */
    /*                                                                       */
    /*************************************************************************/
    return *((oal_uint16 *)(puc_beacon_body + MAC_TIME_STAMP_LEN));
}


oal_uint8  mac_get_dtim_period_etc(oal_uint8 *puc_frame_body, oal_uint16 us_frame_body_len)
{
    oal_uint8   *puc_ie;

    oal_uint16   us_offset;

    us_offset = MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;

    //puc_ie = mac_get_tim_ie(puc_frame_body, us_frame_body_len, us_offset);
    puc_ie = mac_find_ie_etc(MAC_EID_TIM, puc_frame_body + us_offset, us_frame_body_len - us_offset);
    if ((OAL_PTR_NULL != puc_ie) && (puc_ie[1] >= MAC_MIN_TIM_LEN))
    {
        return puc_ie[3];
    }

    return 0;
}



oal_uint8  mac_get_dtim_cnt_etc(oal_uint8 *puc_frame_body, oal_uint16 us_frame_body_len)
{
    oal_uint8   *puc_ie;

    oal_uint16   us_offset;

    us_offset = MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;

    //puc_ie = mac_get_tim_ie(puc_frame_body, us_frame_body_len, us_offset);
    puc_ie = mac_find_ie_etc(MAC_EID_TIM, puc_frame_body + us_offset, us_frame_body_len - us_offset);
    if ((OAL_PTR_NULL != puc_ie) && (puc_ie[1] >= MAC_MIN_TIM_LEN))
    {
        return puc_ie[2];
    }

    return 0;
}


oal_bool_enum_uint8  mac_is_wmm_ie_etc(oal_uint8 *puc_ie)
{
    /* --------------------------------------------------------------------- */
    /* WMM Information/Parameter Element Format                              */
    /* --------------------------------------------------------------------- */
    /* | OUI | OUIType | OUISubtype | Version | QoSInfo | OUISubtype based | */
    /* --------------------------------------------------------------------- */
    /* |3    | 1       | 1          | 1       | 1       | ---------------- | */
    /* --------------------------------------------------------------------- */
    if((puc_ie[0] == MAC_EID_WMM) &&
       (puc_ie[2] == MAC_WMM_OUI_BYTE_ONE)
        && (puc_ie[3] == MAC_WMM_OUI_BYTE_TWO)
        && (puc_ie[4] == MAC_WMM_OUI_BYTE_THREE) && /* OUI */
       (puc_ie[5] == MAC_OUITYPE_WMM) && /* OUI Type     */
       ((puc_ie[6] == MAC_OUISUBTYPE_WMM_INFO) || (puc_ie[6] == MAC_OUISUBTYPE_WMM_PARAM)) && /* OUI Sub Type */
       (puc_ie[7] == MAC_OUI_WMM_VERSION)) /* Version field */
    {
        return OAL_TRUE;
    }

    return OAL_FALSE;
}


oal_uint8*  mac_get_wmm_ie_etc(oal_uint8 *puc_beacon_body, oal_uint16 us_frame_len)
{
    oal_uint8 *puc_wmmie = OAL_PTR_NULL;

    puc_wmmie = mac_find_vendor_ie_etc(MAC_WLAN_OUI_MICROSOFT, MAC_WLAN_OUI_TYPE_MICROSOFT_WMM,
                                   puc_beacon_body, us_frame_len);
    if(OAL_PTR_NULL == puc_wmmie)
    {
        return OAL_PTR_NULL;
    }

    return mac_is_wmm_ie_etc(puc_wmmie) ? puc_wmmie : OAL_PTR_NULL;
}


oal_uint16 mac_get_rsn_capability_etc(const oal_uint8 *puc_rsn_ie)
{
    oal_uint16      us_pairwise_count      = 0;
    oal_uint16      us_akm_count           = 0;
    oal_uint16      us_rsn_capability      = 0;
    oal_uint8       uc_ie_len;
    oal_uint16      us_suite_idx;
    oal_uint16      us_index;

    if (OAL_PTR_NULL == puc_rsn_ie)
    {
        return 0;
    }

    /*************************************************************************/
    /*                  RSN Element Format                                   */
    /* --------------------------------------------------------------------- */
    /* |Element ID | Length | Version | Group Cipher Suite | Pairwise Cipher */
    /* --------------------------------------------------------------------- */
    /* |     1     |    1   |    2    |         4          |       2         */
    /* --------------------------------------------------------------------- */
    /* --------------------------------------------------------------------- */
    /* Suite Count| Pairwise Cipher Suite List | AKM Suite Count | AKM Suite List */
    /* --------------------------------------------------------------------- */
    /*            |         4*m                |     2           |   4*n     */
    /* --------------------------------------------------------------------- */
    /* --------------------------------------------------------------------- */
    /* |RSN Capabilities|PMKID Count|PMKID List|Group Management Cipher Suite*/
    /* --------------------------------------------------------------------- */
    /* |        2       |    2      |   16 *s  |               4           | */
    /* --------------------------------------------------------------------- */
    /*                                                                       */
    /*************************************************************************/
    uc_ie_len  = puc_rsn_ie[1];
    if (uc_ie_len < MAC_MIN_RSN_LEN)
    {
        OAM_WARNING_LOG1(0, OAM_SF_WPA, "{hmac_get_rsn_capability::invalid rsn ie len[%d].}", puc_rsn_ie[1]);
        return 0;
    }

    us_index          = 8;
    /*��ȡPairwise Suite Count*/
    us_pairwise_count = OAL_MAKE_WORD16(puc_rsn_ie[us_index] ,puc_rsn_ie[us_index + 1]);
    if (us_pairwise_count > MAC_PAIRWISE_CIPHER_SUITES_NUM)
    {
        OAM_WARNING_LOG1(0, OAM_SF_WPA, "{hmac_get_rsn_capability::invalid us_pairwise_count[%d].}", us_pairwise_count);
        return 0;
    }
    us_index += 2;

    /*Pairwise Cipher Suite List */
    for (us_suite_idx = 0; us_suite_idx < us_pairwise_count; us_suite_idx++)
    {
        if(OAL_FALSE == MAC_IE_REAMIN_LEN_IS_ENOUGH(&puc_rsn_ie[2], &puc_rsn_ie[us_index], uc_ie_len, 4))
        {
            return 0;
        }
        us_index += 4;
    }

    /*AKM Suite Count*/
    if (OAL_FALSE == MAC_IE_REAMIN_LEN_IS_ENOUGH(&puc_rsn_ie[2], &puc_rsn_ie[us_index], uc_ie_len, 2))
    {
        return 0;
    }
    us_akm_count = OAL_MAKE_WORD16(puc_rsn_ie[us_index] ,puc_rsn_ie[us_index + 1]);
    if (us_akm_count > WLAN_AUTHENTICATION_SUITES)
    {
        OAM_WARNING_LOG1(0, OAM_SF_WPA, "{hmac_get_rsn_capability::invalid us_akm_count[%d].}", us_akm_count);
        return 0;
    }
    us_index += 2;

    /* AKM Suite List*/
    for (us_suite_idx = 0; us_suite_idx < us_akm_count; us_suite_idx++)
    {
        if (OAL_FALSE ==  MAC_IE_REAMIN_LEN_IS_ENOUGH(&puc_rsn_ie[2], &puc_rsn_ie[us_index], uc_ie_len, 4))
        {
            return 0;
        }
        us_index += 4;
    }

    /* Խ��RSN Capabilities */
    if(OAL_FALSE == MAC_IE_REAMIN_LEN_IS_ENOUGH(&puc_rsn_ie[2], &puc_rsn_ie[us_index], uc_ie_len, 2))
    {
        return 0;
    }

    us_rsn_capability = OAL_MAKE_WORD16(puc_rsn_ie[us_index] ,puc_rsn_ie[us_index + 1]);

    return us_rsn_capability;

}


oal_void mac_set_power_cap_ie_etc(oal_uint8 *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
    mac_vap_stru            *pst_mac_vap        = (mac_vap_stru *)pst_vap;
    mac_regclass_info_stru  *pst_regclass_info;

    if (OAL_FALSE == mac_mib_get_dot11SpectrumManagementRequired(pst_mac_vap)
        && OAL_FALSE == mac_mib_get_dot11RadioMeasurementActivated(pst_mac_vap))
    {
        OAM_WARNING_LOG0(0, OAM_SF_TPC, "{mac_set_power_cap_ie_etc::en_dot11SpectrumManagementRequired and en_dot11RadioMeasurementActivated are FALSE!}");
        *puc_ie_len = 0;
        return;
    }

    /********************************************************************************************
            ------------------------------------------------------------------------------------
            |ElementID | Length | MinimumTransmitPowerCapability| MaximumTransmitPowerCapability|
            ------------------------------------------------------------------------------------
    Octets: |1         | 1      | 1                             | 1                             |
            -------------------------------------------------------------------------------------

    *********************************************************************************************/

    *puc_buffer       = MAC_EID_PWRCAP;
    *(puc_buffer + 1) = MAC_PWR_CAP_LEN;

    /* �ɹ���ȡ��������Ϣ����ݹ������TPC����������С���书�ʣ�����Ĭ��Ϊ0 */
    pst_regclass_info = mac_get_channel_num_rc_info_etc(pst_mac_vap->st_channel.en_band, pst_mac_vap->st_channel.uc_chan_number);

    if (OAL_PTR_NULL != pst_regclass_info)
    {
        *(puc_buffer + 2) = (oal_uint8)((pst_mac_vap->st_channel.en_band == WLAN_BAND_2G) ? 4 : 3);
        *(puc_buffer + 3) = (oal_uint8)OAL_MIN(pst_regclass_info->uc_max_reg_tx_pwr, pst_regclass_info->us_max_tx_pwr/10);
    }
    else
    {
        *(puc_buffer + 2) = 0;
        *(puc_buffer + 3) = 0;
    }
    *puc_ie_len = MAC_IE_HDR_LEN + MAC_PWR_CAP_LEN;
}


oal_void mac_set_supported_channel_ie_etc(oal_uint8 *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
    oal_uint8            uc_channel_max_num;
    oal_uint8            uc_channel_idx;
    oal_uint8            us_channel_ie_len = 0;
    oal_uint8           *puc_ie_len_buffer = 0;
    mac_vap_stru        *pst_mac_vap       = (mac_vap_stru *)pst_vap;
    oal_uint8            uc_channel_idx_cnt = 0;

    if (OAL_FALSE == mac_mib_get_dot11SpectrumManagementRequired(pst_mac_vap)
        || OAL_TRUE == mac_mib_get_dot11ExtendedChannelSwitchActivated(pst_mac_vap))
    {
        *puc_ie_len = 0;
        return;
    }

    /********************************************************************************************
            ���Ȳ������ŵ������ŵ����ɶԳ���
            ------------------------------------------------------------------------------------
            |ElementID | Length | Fisrt Channel Number| Number of Channels|
            ------------------------------------------------------------------------------------
    Octets: |1         | 1      | 1                   | 1                 |
            -------------------------------------------------------------------------------------

    *********************************************************************************************/
    /* ����֧�ֵ�Ƶ�λ�ȡ����ŵ����� */
    if (WLAN_BAND_2G == pst_mac_vap->st_channel.en_band)
    {
        uc_channel_max_num = (oal_uint8)MAC_CHANNEL_FREQ_2_BUTT;
    }
    else if (WLAN_BAND_5G == pst_mac_vap->st_channel.en_band)
    {
        uc_channel_max_num = (oal_uint8)MAC_CHANNEL_FREQ_5_BUTT;
    }
    else
    {
        *puc_ie_len = 0;
        return;
    }

    *puc_buffer = MAC_EID_SUPPCHAN;
     puc_buffer++;
     puc_ie_len_buffer = puc_buffer;

    /* ��д�ŵ���Ϣ */
    for (uc_channel_idx = 0; uc_channel_idx < uc_channel_max_num; uc_channel_idx++)
    {
        /* �޸Ĺ�����ṹ�����Ҫ���Ӹ��Ƿ�֧���źŵ��ж� */
        if (OAL_SUCC == mac_is_channel_idx_valid_etc(pst_mac_vap->st_channel.en_band, uc_channel_idx))
        {
            uc_channel_idx_cnt++;
            /*uc_channel_idx_cntΪ1��ʱ���ʾ�ǵ�һ�������ŵ�����Ҫд��Fisrt Channel Number*/
            if(1 == uc_channel_idx_cnt)
            {
                puc_buffer++;

                mac_get_channel_num_from_idx_etc(pst_mac_vap->st_channel.en_band, uc_channel_idx, puc_buffer);

            }
            else if((uc_channel_max_num - 1) == uc_channel_idx)
            {
                /*��Number of Channelsд��֡����*/
                puc_buffer++;
               *puc_buffer = uc_channel_idx_cnt;

                us_channel_ie_len += 2;
            }
        }
        else
        {
            /*uc_channel_idx_cnt��Ϊ0��ʱ���ʾ֮ǰ�п����ŵ�����Ҫ�������ŵ��ĳ���д��֡����*/
            if(0 != uc_channel_idx_cnt)
            {
                /*��Number of Channelsд��֡����*/
                puc_buffer++;
               *puc_buffer = uc_channel_idx_cnt;

                us_channel_ie_len += 2;
            }
            /*��Number of Channelsͳ������*/
            uc_channel_idx_cnt = 0;
        }
    }

    *puc_ie_len_buffer = us_channel_ie_len;
    *puc_ie_len = us_channel_ie_len + MAC_IE_HDR_LEN;

}


oal_void mac_set_wmm_ie_sta_etc(oal_uint8 *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
    oal_uint8            uc_index;
    mac_vap_stru        *pst_mac_vap  = (mac_vap_stru *)pst_vap;

    /* WMM Information Element Format                                */
    /* ------------------------------------------------------------- */
    /* | 3     | 1        | 1           | 1             | 1        | */
    /* ------------------------------------------------------------- */
    /* | OUI   | OUI Type | OUI Subtype | Version field | QoS Info | */
    /* ------------------------------------------------------------- */

    /* �ж�STA�Ƿ�֧��WMM */
    if (OAL_TRUE != mac_mib_get_dot11QosOptionImplemented(pst_mac_vap))
    {
        *puc_ie_len = 0;
         return;
    }

    puc_buffer[0]        = MAC_EID_WMM;
    puc_buffer[1]        = MAC_WMM_INFO_LEN;

    uc_index             = MAC_IE_HDR_LEN;

    /* OUI */
    oal_memcopy(&puc_buffer[uc_index], g_auc_wmm_oui_etc, MAC_OUI_LEN);
    uc_index += MAC_OUI_LEN;

    /* OUI Type */
    puc_buffer[uc_index++] = MAC_OUITYPE_WMM;

    /* OUI Subtype */
    puc_buffer[uc_index++] = MAC_OUISUBTYPE_WMM_INFO;

    /* Version field */
    puc_buffer[uc_index++] = MAC_OUI_WMM_VERSION;

    /* QoS Information Field */
    mac_set_qos_info_field(pst_mac_vap, &puc_buffer[uc_index]);
    uc_index += MAC_QOS_INFO_LEN;

    /* Reserved */
    puc_buffer[uc_index++] = 0;

    *puc_ie_len = MAC_IE_HDR_LEN + MAC_WMM_INFO_LEN;
}

#ifdef _PRE_WLAN_FEATURE_WMMAC

oal_void mac_set_tspec_info_field(oal_uint8 *pst_vap, mac_wmm_tspec_stru *pst_addts_args, oal_uint8 *puc_buffer)
{
    mac_wmm_tspec_stru    *pst_tspec_info;
    /**************************************************************************************************/

    /* TSPEC�ֶ�:
              ----------------------------------------------------------------------------------------
              |TS Info|Nominal MSDU Size|Max MSDU Size|Min Serv Itvl|Max Serv Itvl|
              ----------------------------------------------------------------------------------------
     Octets:  | 3     |  2              |   2         |4            |4            |
              ----------------------------------------------------------------------------------------
              | Inactivity Itvl | Suspension Itvl | Serv Start Time |Min Data Rate | Mean Data Rate |
              ----------------------------------------------------------------------------------------
     Octets:  |4                | 4               | 4               |4             |  4             |
              ----------------------------------------------------------------------------------------
              |Peak Data Rate|Burst Size|Delay Bound|Min PHY Rate|Surplus BW Allowance  |Medium Time|
              ----------------------------------------------------------------------------------------
     Octets:  |4             |4         | 4         | 4          |  2                   |2          |
              ----------------------------------------------------------------------------------------

     TS info�ֶ�:
              ----------------------------------------------------------------------------------------
              |Reserved |TSID |Direction |1 |0 |Reserved |PSB |UP |Reserved |Reserved |Reserved |
              ----------------------------------------------------------------------------------------
       Bits:  |1        |4    |2         |  2  |1        |1   |3  |2        |1        |7        |
              ----------------------------------------------------------------------------------------
   ***************************************************************************************************/

    /* ��ʼ��TSPEC�ṹ�ڴ���Ϣ */
    OAL_MEMZERO(puc_buffer, MAC_WMMAC_TSPEC_LEN);

    pst_tspec_info = (mac_wmm_tspec_stru *)(puc_buffer); //TSPEC Body

    pst_tspec_info->ts_info.bit_tsid       = pst_addts_args->ts_info.bit_tsid;
    pst_tspec_info->ts_info.bit_direction  = pst_addts_args->ts_info.bit_direction;
    pst_tspec_info->ts_info.bit_acc_policy = 1;           /* bit7 = 1, bit8 = 0 */
    pst_tspec_info->ts_info.bit_apsd       = pst_addts_args->ts_info.bit_apsd;
    pst_tspec_info->ts_info.bit_user_prio  = pst_addts_args->ts_info.bit_user_prio;

    pst_tspec_info->us_norminal_msdu_size  = pst_addts_args->us_norminal_msdu_size;
    pst_tspec_info->us_max_msdu_size       = pst_addts_args->us_max_msdu_size;
    pst_tspec_info->ul_min_data_rate       = pst_addts_args->ul_min_data_rate;
    pst_tspec_info->ul_mean_data_rate      = pst_addts_args->ul_mean_data_rate;
    pst_tspec_info->ul_peak_data_rate      = pst_addts_args->ul_peak_data_rate;
    pst_tspec_info->ul_min_phy_rate        = pst_addts_args->ul_min_phy_rate;
    pst_tspec_info->us_surplus_bw          = pst_addts_args->us_surplus_bw;
}


oal_uint16 mac_set_wmmac_ie_sta_etc(oal_uint8 *pst_vap, oal_uint8 *puc_buffer, mac_wmm_tspec_stru *pst_addts_args)
{
    oal_uint8            uc_index;

    /************************************************************************************/
    /*                                Set WMM TSPEC ��Ϣ:                               */
    /*       ---------------------------------------------------------------------------
             |ID | Length| OUI |OUI Type| OUI subtype| Version| TSPEC body|
             ---------------------------------------------------------------------------
    Octets:  |1  | 1     | 3   |1       | 1          | 1      | 55        |
             ---------------------------------------------------------------------------
    *************************************************************************************/

    puc_buffer[0]        = MAC_EID_WMM;
    puc_buffer[1]        = MAC_WMMAC_INFO_LEN;

    uc_index             = MAC_IE_HDR_LEN;

    /* OUI */
    oal_memcopy(&puc_buffer[uc_index], g_auc_wmm_oui_etc, MAC_OUI_LEN);
    uc_index += MAC_OUI_LEN;

    /* OUI Type */
    puc_buffer[uc_index++] = MAC_OUITYPE_WMM;

    /* OUI Subtype */
    puc_buffer[uc_index++] = MAC_OUISUBTYPE_WMMAC_TSPEC;

    /* Version field */
    puc_buffer[uc_index++] = MAC_OUI_WMM_VERSION;

    /* wmmac tspec Field */
    mac_set_tspec_info_field(pst_vap, pst_addts_args, &puc_buffer[uc_index]);

    return (MAC_IE_HDR_LEN + MAC_WMMAC_INFO_LEN);
}

#endif //_PRE_WLAN_FEATURE_WMMAC


oal_void mac_set_listen_interval_ie_etc(oal_uint8 *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
    puc_buffer[0] = 0x0a;
    puc_buffer[1] = 0x00;
    *puc_ie_len   = MAC_LIS_INTERVAL_IE_LEN;
}


oal_void mac_set_status_code_ie_etc(oal_uint8 *puc_buffer, mac_status_code_enum_uint16 en_status_code)
{
    puc_buffer[0] = (oal_uint8)(en_status_code & 0x00FF);
    puc_buffer[1] = (oal_uint8)((en_status_code & 0xFF00) >> 8);
}


oal_void mac_set_aid_ie_etc(oal_uint8 *puc_buffer, oal_uint16 uc_aid)
{
    /*The 2 MSB bits of Association ID is set to 1 as required by the standard.*/
    uc_aid |= 0xC000;
    puc_buffer[0] = (uc_aid & 0x00FF);
    puc_buffer[1] = (uc_aid & 0xFF00) >> 8;
}


oal_uint8  mac_get_bss_type_etc(oal_uint16 us_cap_info)
{
    mac_cap_info_stru *pst_cap_info = (mac_cap_info_stru *)&us_cap_info;

    if (0 != pst_cap_info->bit_ess)
    {
        return (oal_uint8)WLAN_MIB_DESIRED_BSSTYPE_INFRA;
    }

    if (0 != pst_cap_info->bit_ibss)
    {
        return (oal_uint8)WLAN_MIB_DESIRED_BSSTYPE_INDEPENDENT;
    }

    return (oal_uint8)WLAN_MIB_DESIRED_BSSTYPE_ANY;
}


oal_uint32  mac_check_mac_privacy_etc(oal_uint16 us_cap_info,oal_uint8 *pst_vap)
{
    mac_vap_stru       *pst_mac_vap;
    mac_cap_info_stru  *pst_cap_info = (mac_cap_info_stru *)&us_cap_info;

    if (OAL_PTR_NULL == pst_vap)
    {
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mac_vap = (mac_vap_stru *)pst_vap;

    if (OAL_TRUE == mac_mib_get_privacyinvoked(pst_mac_vap))
    {
        /* ��VAP��Privacy invoked������VAPû�� */
        if (pst_cap_info->bit_privacy == 0)
        {
            return (oal_uint32)OAL_FALSE;
        }
    }
    /* ���Ǽ����ԣ���vap��֧�ּ���ʱ��������û�������*/

    return (oal_uint32)OAL_TRUE;

}


oal_void mac_add_app_ie_etc(oal_void *pst_vap,
                            oal_uint8 *puc_buffer,
                            oal_uint16 *pus_ie_len,
                            en_app_ie_type_uint8 en_type)
{
    mac_vap_stru    *pst_mac_vap;
    oal_uint8       *puc_app_ie;
    oal_uint32       ul_app_ie_len;

    pst_mac_vap   = (mac_vap_stru *)pst_vap;
    puc_app_ie    = pst_mac_vap->ast_app_ie[en_type].puc_ie;
    ul_app_ie_len = pst_mac_vap->ast_app_ie[en_type].ul_ie_len;

    if (0 == ul_app_ie_len)
    {
        *pus_ie_len = 0;
        return;
    }
    else
    {
        oal_memcopy(puc_buffer, puc_app_ie, ul_app_ie_len);
        *pus_ie_len = (oal_uint16)ul_app_ie_len;
    }

    return;
}


oal_void mac_add_wps_ie_etc(oal_void *pst_vap,
                        oal_uint8 *puc_buffer,
                        oal_uint16 *pus_ie_len,
                        en_app_ie_type_uint8 en_type)
{
    mac_vap_stru    *pst_mac_vap;
    oal_uint8       *puc_app_ie;
    oal_uint8       *puc_wps_ie;
    oal_uint32       ul_app_ie_len;

    pst_mac_vap   = (mac_vap_stru *)pst_vap;
    puc_app_ie    = pst_mac_vap->ast_app_ie[en_type].puc_ie;
    ul_app_ie_len = pst_mac_vap->ast_app_ie[en_type].ul_ie_len;

    if (0 == ul_app_ie_len)
    {
        *pus_ie_len = 0;
        return;
    }

    //puc_wps_ie = mac_get_wps_ie(puc_app_ie, (oal_uint16)ul_app_ie_len, 0);
    puc_wps_ie = mac_find_vendor_ie_etc(MAC_WLAN_OUI_MICROSOFT, MAC_WLAN_OUI_TYPE_MICROSOFT_WPS, puc_app_ie, (oal_int32)ul_app_ie_len);
    if ((puc_wps_ie == OAL_PTR_NULL) || (puc_wps_ie[1] < MAC_MIN_WPS_IE_LEN))
    {
        *pus_ie_len = 0;
        return;
    }

    /* ��WPS ie ��Ϣ������buffer �� */
    oal_memcopy(puc_buffer, puc_wps_ie, puc_wps_ie[1] + MAC_IE_HDR_LEN);
    *pus_ie_len = puc_wps_ie[1] + MAC_IE_HDR_LEN;

    return;
}

#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY

oal_void mac_set_opmode_field_etc(oal_uint8 *pst_vap, oal_uint8 *puc_buffer, wlan_nss_enum_uint8 en_nss)
{
    mac_vap_stru           *pst_mac_vap       = (mac_vap_stru *)pst_vap;
    mac_opmode_notify_stru *pst_opmode_notify = (mac_opmode_notify_stru *)puc_buffer;
    wlan_bw_cap_enum_uint8  en_cp_bw          = WLAN_BW_CAP_BUTT;

    /****************************************************************/
    /*--------------------------------------------------------------*/
    /*|B0-B1         |B2-B3                   |B4-B6   |B7         |*/
    /*--------------------------------------------------------------*/
    /*|Channel Width |Dynamic Extended NSS BW |Rx Nss  |Rx Nss Type|*/
    /*--------------------------------------------------------------*/
    /****************************************************************/
    /* bit_rx_nss_typeĿǰд��0���ɣ����������������ϣ�����txbf��С������ʱ������Ҫ����1���� ����չTBD */
    OAL_MEMZERO(pst_opmode_notify, OAL_SIZEOF(mac_opmode_notify_stru));

    mac_vap_get_bandwidth_cap_etc(pst_mac_vap, &en_cp_bw);

    pst_opmode_notify->bit_channel_width = en_cp_bw;

    /* Ҫ�л�������������vap�µ�nss�����Ļ�����Ҫ��ǰ��vap��nss�������ӿڲ���������Σ���߳�������չ�� */
    pst_opmode_notify->bit_rx_nss        = en_nss;
    pst_opmode_notify->bit_rx_nss_type   = 0;
}


oal_void mac_set_opmode_notify_ie_etc(oal_uint8 *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
    mac_vap_stru *pst_mac_vap = (mac_vap_stru *)pst_vap;

    /********************************************
            -------------------------------------
            |ElementID | Length | Operating Mode|
            -------------------------------------
    Octets: |1         | 1      | 1             |
            -------------------------------------

    ********************************************/

    if ((OAL_FALSE == mac_mib_get_VHTOptionImplemented(pst_mac_vap))
        || (OAL_FALSE == mac_mib_get_OperatingModeNotificationImplemented(pst_mac_vap)))
    {
        *puc_ie_len = 0;
        return;
    }

    puc_buffer[0] = MAC_EID_OPMODE_NOTIFY;
    puc_buffer[1] = MAC_OPMODE_NOTIFY_LEN;

    mac_set_opmode_field_etc((oal_void*)pst_vap, (puc_buffer + MAC_IE_HDR_LEN), pst_mac_vap->en_vap_rx_nss);

    *puc_ie_len = MAC_IE_HDR_LEN + MAC_OPMODE_NOTIFY_LEN;
}
#endif

#if (_PRE_WLAN_FEATURE_PMF != _PRE_PMF_NOT_SUPPORT)


wlan_pmf_cap_status_uint8 mac_get_pmf_cap_etc(oal_uint8 *puc_ie, oal_uint32 ul_ie_len)
{
    oal_uint8  *puc_rsn_ie;
    oal_uint16  us_rsn_cap;

    if (OAL_UNLIKELY(OAL_PTR_NULL == puc_ie))
    {
        return MAC_PMF_DISABLED;
    }

    /* ����RSN��ϢԪ��,���û��RSN��ϢԪ��,���ղ�֧�ִ���*/
    //puc_rsn_ie = mac_get_rsn_ie(puc_ie, ul_ie_len, 0);
    puc_rsn_ie = mac_find_ie_etc(MAC_EID_RSN, puc_ie, (oal_int32)(ul_ie_len));
    if (OAL_PTR_NULL == puc_rsn_ie)
    {
        return MAC_PMF_DISABLED;
    }

    /* ����RSN��ϢԪ��, �ж�RSN�����Ƿ�ƥ��*/
    us_rsn_cap = mac_get_rsn_capability_etc(puc_rsn_ie);
    if ((us_rsn_cap & BIT6) && (us_rsn_cap & BIT7))
    {
        return MAC_PMF_REQUIRED;
    }

    if (us_rsn_cap & BIT7)
    {
        return MAC_PMF_ENABLED;
    }
    return MAC_PMF_DISABLED;
}
#endif
#if defined(_PRE_WLAN_FEATURE_11R) || (defined(_PRE_WLAN_FEATURE_11R_AP))

oal_void mac_set_md_ie_etc(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
    mac_vap_stru *pst_mac_vap = (mac_vap_stru *)pst_vap;

    if (OAL_TRUE != mac_mib_get_ft_trainsistion(pst_mac_vap))
    {
        *puc_ie_len = 0;
        return;
    }

    /***************************************************************************
                    ---------------------------------------------------
                    |Element ID | Length | MDID| FT Capbility & Policy|
                    ---------------------------------------------------
           Octets:  |1          | 1      | 2   |  1                   |
                    ---------------------------------------------------
    ***************************************************************************/

    *puc_buffer       = MAC_EID_MOBILITY_DOMAIN;
    *(puc_buffer + 1) = 3;
    *(puc_buffer + 2) = mac_mib_get_ft_mdid(pst_mac_vap) & 0xFF;
    *(puc_buffer + 3) = (mac_mib_get_ft_mdid(pst_mac_vap) >> 8) & 0xFF;
    *(puc_buffer + 4) = 0;

    /***************************************************************************
                    ------------------------------------------
                    |FT over DS| RRP Capability | Reserved   |
                    ------------------------------------------
             Bits:  |1         | 1              | 6          |
                    ------------------------------------------
    ***************************************************************************/
    if (OAL_TRUE == mac_mib_get_ft_over_ds(pst_mac_vap))
    {
        *(puc_buffer + 4) += 1;
    }
    if (OAL_TRUE == mac_mib_get_ft_resource_req(pst_mac_vap))
    {
        *(puc_buffer + 4) += 2;
    }
    *puc_ie_len = 5;
}


oal_void mac_set_ft_ie_etc(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint16 *pus_ie_len)
{
    mac_vap_stru *pst_mac_vap = (mac_vap_stru *)pst_vap;

    if (OAL_TRUE != mac_mib_get_ft_trainsistion(pst_mac_vap))
    {
        *pus_ie_len = 0;
        return;
    }

    /*********************************************************************************************
              ------------------------------------------------------------------------------
              |Element ID | Length | MIC Control | MIC | ANonce | SNonce | Optional Params |
              ------------------------------------------------------------------------------
     Octets:  |1          | 1      | 2           |  16 |   32   |   32   |   variable      |
              ------------------------------------------------------------------------------
    ***********************************************************************************************/

    *puc_buffer       = MAC_EID_FT;

    *pus_ie_len = 84;
}
#ifdef _PRE_WLAN_FEATURE_11R
oal_void mac_set_rde_ie_etc(oal_void *pst_mac_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
    /*********************************************************************************************
              -------------------------------------------------------------------
              |Element ID | Length | RDE Identifier | Res Dscr Cnt| Status Code |
              -------------------------------------------------------------------
     Octets:  |1          | 1      | 1              |  1          |   2         |
              -------------------------------------------------------------------
    ***********************************************************************************************/

    *puc_buffer       = MAC_EID_RDE;
    *(puc_buffer + 1) = 4;
    *(puc_buffer + 2) = 0;
    /*TBD:count��ӦRDE������RIC descriptor IE����*/
    *(puc_buffer + 3) = 0;
    *(puc_buffer + 4) = 0;
    *(puc_buffer + 5) = 0;

    *puc_ie_len = 6;
}

oal_void mac_set_tspec_ie_etc(oal_void *pst_mac_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len, oal_uint8 uc_tid)
{
    oal_uint8           uc_len = 0;
    mac_ts_info_stru    *pst_ts_info;

    /***********************************************************************************************
              --------------------------------------------------------------------------------------
              |Element ID|Length|TS Info|Nominal MSDU Size|Max MSDU Size|Min Serv Itvl|Max Serv Itvl|
              ---------------------------------------------------------------------------------------
     Octets:  |1         | 1(55)| 3     |  2              |   2         |4            |4            |
              ---------------------------------------------------------------------------------------
              | Inactivity Itvl | Suspension Itvl | Serv Start Time |Min Data Rate | Mean Data Rate |
              ---------------------------------------------------------------------------------------
     Octets:  |4                | 4               | 4               |4             |  4             |
              ---------------------------------------------------------------------------------------
              |Peak Data Rate|Burst Size|Delay Bound|Min PHY Rate|Surplus BW Allowance  |Medium Time|
              ---------------------------------------------------------------------------------------
     Octets:  |4             |4         | 4         | 4          |  2                   |2          |
              ---------------------------------------------------------------------------------------

    ***********************************************************************************************/

    *puc_buffer       = MAC_EID_TSPEC;
    *(puc_buffer + 1) = 55;
    uc_len += 2;

    OAL_MEMZERO(puc_buffer + uc_len, *(puc_buffer + 1));

    pst_ts_info = (mac_ts_info_stru *)(puc_buffer + uc_len); //TS Info
    uc_len += 3;

    pst_ts_info->bit_tsid       = uc_tid;
    pst_ts_info->bit_direction  = 3;  //bidirection
    pst_ts_info->bit_apsd       = 1;
    pst_ts_info->bit_user_prio  = uc_tid;

    *(oal_uint16 *)(puc_buffer + uc_len) =  0x812c;//Nominal MSDU Size
    uc_len += 28;

    *(oal_uint32 *)(puc_buffer + uc_len) = 0x0001D4C0;//Mean Data Rate
    uc_len += 16;

    *(oal_uint16 *)(puc_buffer + uc_len) = 0x3000;//Surplus BW Allowance

    *puc_ie_len = 57;

}
#endif
#endif //_PRE_WLAN_FEATURE_11R

#if defined(_PRE_WLAN_FEATURE_11K) || defined(_PRE_WLAN_FEATURE_11K_EXTERN) || defined(_PRE_WLAN_FEATURE_FTM) || defined(_PRE_WLAN_FEATURE_11KV_INTERFACE)

oal_void mac_set_rrm_enabled_cap_field_etc(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
    mac_vap_stru                *pst_mac_vap = (mac_vap_stru *)pst_vap;
    oal_rrm_enabled_cap_ie_stru *pst_rrm_enabled_cap_ie;
#ifdef _PRE_WLAN_FEATURE_11KV_INTERFACE
    oal_uint8                   uc_ie_set_len = 0;
#endif

#ifdef _PRE_WLAN_FEATURE_11K_EXTERN
    if(OAL_FALSE == mac_mib_get_dot11RadioMeasurementActivated(pst_mac_vap))
    {
        *puc_ie_len = 0;
        /* �������������rrm enabled ie ��ʹ��hera��Ʒ�ӿڴ���������Ϣ���Ӹ�IE */
#ifdef _PRE_WLAN_FEATURE_11KV_INTERFACE
        mac_set_ie_field((oal_void *)(pst_mac_vap->pst_rrm_ie_info), puc_buffer, &uc_ie_set_len);
        *puc_ie_len += uc_ie_set_len;
#endif
        return;
    }
#endif

    puc_buffer[0] = MAC_EID_RRM;
    puc_buffer[1] = MAC_RRM_ENABLE_CAP_IE_LEN;

    pst_rrm_enabled_cap_ie = (oal_rrm_enabled_cap_ie_stru *)(puc_buffer + MAC_IE_HDR_LEN);

    OAL_MEMZERO(pst_rrm_enabled_cap_ie, OAL_SIZEOF(oal_rrm_enabled_cap_ie_stru));

#if defined(_PRE_WLAN_FEATURE_11K) || defined(_PRE_WLAN_FEATURE_11K_EXTERN)
    /* ֻ��bit0 4 5 6λ��1 */
    pst_rrm_enabled_cap_ie->bit_link_cap        = mac_mib_get_dot11RMLinkMeasurementActivated(pst_mac_vap);
    pst_rrm_enabled_cap_ie->bit_bcn_passive_cap = mac_mib_get_dot11RMBeaconPassiveMeasurementActivated(pst_vap);
    pst_rrm_enabled_cap_ie->bit_bcn_active_cap  = mac_mib_get_dot11RMBeaconActiveMeasurementActivated(pst_vap);
    pst_rrm_enabled_cap_ie->bit_bcn_table_cap   = mac_mib_get_dot11RMBeaconTableMeasurementActivated(pst_vap);
#endif

#ifdef _PRE_WLAN_FEATURE_11K_EXTERN
    pst_rrm_enabled_cap_ie->bit_chn_load_cap    = mac_mib_get_dot11RMChannelLoadMeasurementActivated(pst_vap);
    pst_rrm_enabled_cap_ie->bit_neighbor_rpt_cap= mac_mib_get_dot11RMNeighborReportActivated(pst_vap);
#endif
#if (defined(_PRE_WLAN_FEATURE_11K) && (defined(_PRE_PRODUCT_ID_HI110X_HOST) || defined(_PRE_PRODUCT_ID_HI110X_DEV)))
    /*st_wlan_mib_sta_config.en_dot11RMNeighborReportActivated ROM��,����03,
      ���11k��,ֱ������Ϊ 1  */
    if(IS_LEGACY_STA(pst_mac_vap))
    {
        pst_rrm_enabled_cap_ie->bit_neighbor_rpt_cap = 1;
    }
#endif

#if defined(_PRE_WLAN_FEATURE_11V) || (defined(_PRE_WLAN_FEATURE_FTM)) || defined(_PRE_WLAN_FEATURE_11V_ENABLE)
    pst_rrm_enabled_cap_ie->bit_ftm_range_report_cap = mac_mib_get_FineTimingMsmtRangeRepActivated(pst_mac_vap);
#endif

    *puc_ie_len = MAC_IE_HDR_LEN + MAC_RRM_ENABLE_CAP_IE_LEN;
    /* �����������rrm enabled ie ��ʹ��hera��Ʒ�ӿڴ���������Ϣ�޸ĸ�IE */
#ifdef _PRE_WLAN_FEATURE_11KV_INTERFACE
    uc_ie_set_len = *puc_ie_len;    /* IE�Ѿ����� uc_ie_set_len�ں����ڲ�����0 */
    mac_set_ie_field((oal_void *)(pst_mac_vap->pst_rrm_ie_info), puc_buffer, &uc_ie_set_len);
#endif
}
#endif

#ifdef _PRE_WLAN_FEATURE_11K

oal_void mac_set_wfa_tpc_report_ie_etc(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
    oal_uint8      uc_index = 0;
    mac_vap_stru  *pst_mac_vap = (mac_vap_stru *)pst_vap;

    /***************************************************************************
       -------------------------------------------------------------------------
       |EID |Length |OUI   |OUI Type|OUI Subtype |Transmit Power |Link Margin |
       -------------------------------------------------------------------------
       |1   |1      | 3    |1       |1           |1              |1           |
       -------------------------------------------------------------------------
       ***************************************************************************/

    puc_buffer[0] = MAC_EID_WFA_TPC_RPT;
    puc_buffer[1] = MAC_WFA_TPC_RPT_LEN;
    uc_index += MAC_IE_HDR_LEN;

    oal_memcopy(puc_buffer + uc_index, g_auc_wfa_oui_etc, MAC_OUI_LEN);
    uc_index += MAC_OUI_LEN;

    puc_buffer[uc_index++] = MAC_OUITYPE_WFA;
    puc_buffer[uc_index++] = MAC_OUISUBTYPE_WFA;
    puc_buffer[uc_index++] = pst_mac_vap->uc_tx_power;
    puc_buffer[uc_index] = 0;

    *puc_ie_len = MAC_IE_HDR_LEN + MAC_WFA_TPC_RPT_LEN;
}

#endif //_PRE_WLAN_FEATURE_11K

#ifdef _PRE_WLAN_FEATURE_HISTREAM

oal_void mac_set_histream_ie(oal_void *pst_vap, oal_uint8 *puc_buffer, oal_uint8 *puc_ie_len)
{
    puc_buffer[0] = MAC_EID_VENDOR;
    puc_buffer[1] = MAC_OUI_LEN + 1;
    /*lint -e572*/ /*lint -e778*/
    puc_buffer[2] = (oal_uint8)((MAC_HUAWEI_VENDER_IE >> 16) & 0xff);
    puc_buffer[3] = (oal_uint8)((MAC_HUAWEI_VENDER_IE >> 8) & 0xff);
    puc_buffer[4] = (oal_uint8)((MAC_HUAWEI_VENDER_IE) & 0xff);
    puc_buffer[5] = MAC_HISI_HISTREAM_IE;

    /*lint +e572*/ /*lint +e778*/
    *puc_ie_len = MAC_IE_HDR_LEN + puc_buffer[1];
}
#endif //_PRE_WLAN_FEATURE_HISTREAM

oal_uint16  mac_encap_2040_coext_mgmt_etc(oal_void *pst_vap, oal_netbuf_stru *pst_buffer, oal_uint8 uc_coext_info, oal_uint32 ul_chan_report)
{
    oal_uint8                     *puc_mac_header          = oal_netbuf_header(pst_buffer);
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
    oal_uint8                     *puc_payload_addr        = oal_netbuf_data(pst_buffer);
#else
    oal_uint8                     *puc_payload_addr        = puc_mac_header + MAC_80211_FRAME_LEN;
#endif
    oal_uint8                      uc_chan_idx     = 0;
    oal_uint16                     us_ie_len_idx   = 0;
    oal_uint16                     us_index        = 0;
    mac_vap_stru                  *pst_mac_vap     = (mac_vap_stru *)pst_vap;
    wlan_channel_band_enum_uint8   en_band         = pst_mac_vap->st_channel.en_band;
    oal_uint8                      uc_max_num_chan = mac_get_num_supp_channel(en_band);
    oal_uint8                      uc_channel_num;

    /*************************************************************************/
    /*                        Management Frame Format                        */
    /* --------------------------------------------------------------------  */
    /* |Frame Control|Duration|DA|SA|BSSID|Sequence Control|Frame Body|FCS|  */
    /* --------------------------------------------------------------------  */
    /* | 2           |2       |6 |6 |6    |2               |0 - 2312  |4  |  */
    /* --------------------------------------------------------------------  */
    /*                                                                       */
    /*************************************************************************/

    /* ���� Frame Control field */
    mac_hdr_set_frame_control(puc_mac_header, WLAN_PROTOCOL_VERSION| WLAN_FC0_TYPE_MGT | WLAN_FC0_SUBTYPE_ACTION);

    /* ���÷�Ƭ���Ϊ0 */
    mac_hdr_set_fragment_number(puc_mac_header, 0);

    /* ���� address1(���ն�): AP MAC��ַ (BSSID)*/
    oal_set_mac_addr(puc_mac_header + WLAN_HDR_ADDR1_OFFSET, pst_mac_vap->auc_bssid);

    /* ���� address2(���Ͷ�): dot11StationID */
    oal_set_mac_addr(puc_mac_header + WLAN_HDR_ADDR2_OFFSET, mac_mib_get_StationID(pst_mac_vap));

    /* ���� address3: AP MAC��ַ (BSSID) */
    oal_set_mac_addr(puc_mac_header + WLAN_HDR_ADDR3_OFFSET, pst_mac_vap->auc_bssid);

    /*************************************************************************************/
    /*                 20/40 BSS Coexistence Management frame - Frame Body               */
    /* --------------------------------------------------------------------------------- */
    /* |Category |Public Action |20/40 BSS Coex IE| 20/40 BSS Intolerant Chan Report IE| */
    /* --------------------------------------------------------------------------------- */
    /* |1        |1             |3                |Variable                            | */
    /* --------------------------------------------------------------------------------- */
    /*                                                                                   */
    /*************************************************************************************/

    puc_payload_addr[us_index++] = MAC_ACTION_CATEGORY_PUBLIC;           /* Category */
    puc_payload_addr[us_index++] = MAC_PUB_COEXT_MGMT;                   /* Public Action */

    /* ��װ20/40 BSS Coexistence element */
    puc_payload_addr[us_index++] = MAC_EID_2040_COEXT;                   /* Element ID */
    puc_payload_addr[us_index++] = MAC_2040_COEX_LEN;                    /* Length */
    puc_payload_addr[us_index++] = uc_coext_info;                        /* 20/40 BSS Coexistence Information field */

    /* ��װ20/40 BSS Intolerant Channel Report element */
    /* ֻ�е�STA��⵽Trigger Event Aʱ���Ű���Operating Class���μ�802.11n 10.15.12 */
    puc_payload_addr[us_index++] = MAC_EID_2040_INTOLCHREPORT;       /* Element ID */
    us_ie_len_idx          = us_index;
    puc_payload_addr[us_index++] = MAC_2040_INTOLCHREPORT_LEN_MIN;   /* Length */
    puc_payload_addr[us_index++] = 0;                               /* Operating Class */
    if (ul_chan_report > 0)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ASSOC,
                      "{mac_encap_2040_coext_mgmt_etc::Channel List = 0x%x.}", ul_chan_report);
        /* Channel List */
        for (uc_chan_idx = 0; uc_chan_idx < uc_max_num_chan; uc_chan_idx++)
        {
            if (0 != ((ul_chan_report >> uc_chan_idx) & BIT0))
            {
                mac_get_channel_num_from_idx_etc(en_band, uc_chan_idx, &uc_channel_num);
                puc_payload_addr[us_index++] = uc_channel_num;
                puc_payload_addr[us_ie_len_idx]++;
            }
        }
    }

    return (oal_uint16)(us_index + MAC_80211_FRAME_LEN);      /* [false alarm]:fortify��,�������޷�����  */
}

oal_bool_enum_uint8 mac_frame_is_null_data(oal_netbuf_stru *pst_net_buf)
{
    oal_uint8                      *pst_mac_header;
    oal_uint8                       uc_frame_type;
    oal_uint8                       uc_frame_subtype;
    mac_tx_ctl_stru                *pst_tx_ctl;

    pst_tx_ctl = (mac_tx_ctl_stru *)oal_netbuf_cb(pst_net_buf);
    pst_mac_header = (oal_uint8 *)(MAC_GET_CB_FRAME_HEADER_ADDR(pst_tx_ctl));
    uc_frame_type    = mac_frame_get_type_value((oal_uint8 *)pst_mac_header);
    uc_frame_subtype = mac_frame_get_subtype_value((oal_uint8 *)pst_mac_header);

    if ((WLAN_DATA_BASICTYPE == uc_frame_type)
         && ((WLAN_NULL_FRAME == uc_frame_subtype) || (WLAN_QOS_NULL_FRAME == uc_frame_subtype)))
    {
        return OAL_TRUE;
    }

    return OAL_FALSE;
}

/*lint -e19*/
oal_module_symbol(mac_find_ie_etc);
oal_module_symbol(mac_find_vendor_ie_etc);
oal_module_symbol(mac_set_aid_ie_etc);
oal_module_symbol(mac_set_supported_rates_ie_etc);
oal_module_symbol(mac_set_ext_capabilities_ie_etc);
oal_module_symbol(mac_set_ssid_ie_etc);
oal_module_symbol(mac_set_exsup_rates_ie_etc);
oal_module_symbol(mac_set_cap_info_ap_etc);
oal_module_symbol(mac_set_cap_info_sta_etc);
oal_module_symbol(mac_set_wmm_params_ie_etc);
oal_module_symbol(mac_set_wmm_ie_sta_etc);
#ifdef _PRE_WLAN_FEATURE_WMMAC
oal_module_symbol(mac_set_wmmac_ie_sta_etc);
#endif //_PRE_WLAN_FEATURE_WMMAC
oal_module_symbol(mac_set_ht_capabilities_ie_etc);
oal_module_symbol(mac_set_obss_scan_params_etc);
oal_module_symbol(mac_set_ht_opern_ie_etc);
oal_module_symbol(mac_set_listen_interval_ie_etc);
oal_module_symbol(mac_set_status_code_ie_etc);
oal_module_symbol(mac_set_power_cap_ie_etc);
oal_module_symbol(mac_set_supported_channel_ie_etc);
oal_module_symbol(mac_set_rsn_ie_etc);
oal_module_symbol(mac_set_wpa_ie_etc);
oal_module_symbol(mac_set_vht_capabilities_ie_etc);
oal_module_symbol(mac_set_vht_opern_ie_etc);
oal_module_symbol(mac_get_dtim_period_etc);
oal_module_symbol(mac_get_dtim_cnt_etc);
oal_module_symbol(mac_is_wmm_ie_etc);
oal_module_symbol(mac_check_mac_privacy_etc);
oal_module_symbol(g_auc_rsn_oui_etc);
oal_module_symbol(g_auc_wmm_oui_etc);
oal_module_symbol(mac_set_timeout_interval_ie_etc);
oal_module_symbol(mac_add_app_ie_etc);
oal_module_symbol(mac_get_bss_type_etc);
oal_module_symbol(mac_get_beacon_period_etc);

#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
oal_module_symbol(mac_set_opmode_notify_ie_etc);
#endif
oal_module_symbol(mac_report_80211_frame);
oal_module_symbol(mac_rx_report_80211_frame_etc);
oal_module_symbol(mac_set_11ntxbf_vendor_ie_etc);
oal_module_symbol(mac_get_wmm_ie_etc);
oal_module_symbol(mac_get_rsn_capability_etc);

#if (_PRE_WLAN_FEATURE_PMF != _PRE_PMF_NOT_SUPPORT)
oal_module_symbol(mac_get_pmf_cap_etc);
#endif

oal_module_symbol(mac_encap_2040_coext_mgmt_etc);
#ifdef _PRE_WLAN_FEATURE_HISTREAM
oal_module_symbol(mac_set_histream_ie);
#endif //_PRE_WLAN_FEATURE_HISTREAM

#if defined(_PRE_WLAN_FEATURE_11K) || defined(_PRE_WLAN_FEATURE_11K_EXTERN) || defined(_PRE_WLAN_FEATURE_11KV_INTERFACE)
oal_module_symbol(mac_set_rrm_enabled_cap_field_etc);
#endif
/*lint +e19*/

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif


