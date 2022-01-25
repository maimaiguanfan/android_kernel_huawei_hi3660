

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 ͷ�ļ�����
*****************************************************************************/
#include "oam_event.h"
#include "oam_main.h"
#include "oam_ext_if.h"


#undef THIS_FILE_ID
#define THIS_FILE_ID   OAM_FILE_ID_OAM_EVENT_C

/*****************************************************************************
  2 ȫ�ֱ�������
*****************************************************************************/

/*****************************************************************************
  3 ����ʵ��
*****************************************************************************/

oal_switch_enum_uint8  oam_report_data_get_global_switch(oam_ota_frame_direction_type_enum_uint8 en_direction)
{
    if (OAL_UNLIKELY(OAM_OTA_FRAME_DIRECTION_TYPE_BUTT <= en_direction))
    {
        return OAL_ERR_CODE_ARRAY_OVERFLOW;
    }

    return g_st_oam_mng_ctx.st_user_track_ctx.aen_data_global_switch[en_direction];
}


OAL_STATIC oal_uint32  oam_report_data_set_global_switch(
                                        oam_ota_frame_direction_type_enum_uint8 en_direction)
{
    oal_uint16          us_usr_idx;
    oal_switch_enum_uint8 en_mcast_switch = OAL_SWITCH_OFF;
    oal_switch_enum_uint8 en_ucast_switch = OAL_SWITCH_OFF;

    if (OAL_UNLIKELY(OAM_OTA_FRAME_DIRECTION_TYPE_BUTT <= en_direction))
    {
        return OAL_ERR_CODE_ARRAY_OVERFLOW;
    }

    /* �ȼ�鵥������֡�Ƿ����û��Ѿ����� */
    for (us_usr_idx = 0; us_usr_idx < WLAN_USER_MAX_USER_LIMIT; us_usr_idx++)
    {
        if ((OAL_SWITCH_ON == g_st_oam_mng_ctx.st_user_track_ctx.ast_80211_ucast_data_ctx[us_usr_idx][en_direction].en_frame_cb_switch)
            || (OAL_SWITCH_ON == g_st_oam_mng_ctx.st_user_track_ctx.ast_80211_ucast_data_ctx[us_usr_idx][en_direction].en_frame_content_switch)
            || (OAL_SWITCH_ON == g_st_oam_mng_ctx.st_user_track_ctx.ast_80211_ucast_data_ctx[us_usr_idx][en_direction].en_frame_dscr_switch))
        {
            en_ucast_switch = OAL_SWITCH_ON;
            break;
        }
    }

    /* �ټ���鲥����֡�Ƿ񿪹ش��� */
    if ((OAL_SWITCH_ON == g_st_oam_mng_ctx.st_user_track_ctx.ast_80211_mcast_data_ctx[en_direction].en_frame_cb_switch)
        || (OAL_SWITCH_ON == g_st_oam_mng_ctx.st_user_track_ctx.ast_80211_mcast_data_ctx[en_direction].en_frame_content_switch)
        || (OAL_SWITCH_ON == g_st_oam_mng_ctx.st_user_track_ctx.ast_80211_mcast_data_ctx[en_direction].en_frame_dscr_switch))
    {
        en_mcast_switch = OAL_SWITCH_ON;
    }

    g_st_oam_mng_ctx.st_user_track_ctx.aen_data_global_switch[en_direction] = (en_ucast_switch | en_mcast_switch);

    return OAL_SUCC;
}


oal_uint32 oam_report_eth_frame_set_switch(oal_uint16               us_user_idx,
                                                   oal_switch_enum_uint8    en_switch,
                                       oam_ota_frame_direction_type_enum_uint8 en_eth_direction)
{
    if (us_user_idx >= WLAN_USER_MAX_USER_LIMIT)
    {
        OAL_IO_PRINT("oam_report_eth_frame_set_switch::user_idx exceeds!\n");
        return OAL_ERR_CODE_OAM_EVT_USER_IDX_EXCEED;
    }

    if (en_eth_direction >= OAM_OTA_FRAME_DIRECTION_TYPE_BUTT)
    {
        OAL_IO_PRINT("oam_report_eth_frame_set_switch::eth_direction exceeds!\n");
        return OAL_ERR_CODE_OAM_EVT_FRAME_DIR_INVALID;
    }

    g_st_oam_mng_ctx.st_user_track_ctx.aen_eth_data_ctx[us_user_idx][en_eth_direction] = en_switch;

    return OAL_SUCC;
}


oal_uint32  oam_report_eth_frame_get_switch(
                                 oal_uint16             us_user_idx,
                                 oam_ota_frame_direction_type_enum_uint8 en_eth_direction,
                                 oal_switch_enum_uint8  *pen_eth_switch)
{
    if (us_user_idx >= WLAN_USER_MAX_USER_LIMIT)
    {
        return OAL_ERR_CODE_OAM_EVT_USER_IDX_EXCEED;
    }

    if (en_eth_direction >= OAM_OTA_FRAME_DIRECTION_TYPE_BUTT)
    {
        return OAL_ERR_CODE_OAM_EVT_FRAME_DIR_INVALID;
    }

    if (OAL_PTR_NULL == pen_eth_switch)
    {
        return OAL_ERR_CODE_PTR_NULL;
    }

    *pen_eth_switch = g_st_oam_mng_ctx.st_user_track_ctx.aen_eth_data_ctx[us_user_idx][en_eth_direction];

    return OAL_SUCC;
}


oal_uint32  oam_report_80211_mcast_set_switch(
                                        oam_ota_frame_direction_type_enum_uint8 en_mcast_direction,
                                        oam_user_track_frame_type_enum_uint8    en_frame_type,
                                        oal_switch_enum_uint8                   en_frame_switch,
                                        oal_switch_enum_uint8                   en_cb_switch,
                                        oal_switch_enum_uint8                   en_dscr_switch)
{
    if (en_mcast_direction >= OAM_OTA_FRAME_DIRECTION_TYPE_BUTT)
    {
        return OAL_ERR_CODE_OAM_EVT_FRAME_DIR_INVALID;
    }

    if (en_frame_type >= OAM_USER_TRACK_FRAME_TYPE_BUTT)
    {
        return OAL_ERR_CODE_CONFIG_UNSUPPORT;
    }

    if (OAL_UNLIKELY(en_frame_switch >= OAL_SWITCH_BUTT)
        || OAL_UNLIKELY(en_cb_switch >= OAL_SWITCH_BUTT)
        || OAL_UNLIKELY(en_dscr_switch >= OAL_SWITCH_BUTT))
    {
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /* Ϊ�鲥����֡���ÿ��� */
    if (OAM_USER_TRACK_FRAME_TYPE_MGMT == en_frame_type)
    {
        g_st_oam_mng_ctx.st_user_track_ctx.ast_80211_mcast_mgmt_ctx[en_mcast_direction].en_frame_content_switch = en_frame_switch;
        g_st_oam_mng_ctx.st_user_track_ctx.ast_80211_mcast_mgmt_ctx[en_mcast_direction].en_frame_cb_switch = en_cb_switch;
        g_st_oam_mng_ctx.st_user_track_ctx.ast_80211_mcast_mgmt_ctx[en_mcast_direction].en_frame_dscr_switch = en_dscr_switch;
    }
    else
    {
        g_st_oam_mng_ctx.st_user_track_ctx.ast_80211_mcast_data_ctx[en_mcast_direction].en_frame_content_switch = en_frame_switch;
        g_st_oam_mng_ctx.st_user_track_ctx.ast_80211_mcast_data_ctx[en_mcast_direction].en_frame_cb_switch = en_cb_switch;
        g_st_oam_mng_ctx.st_user_track_ctx.ast_80211_mcast_data_ctx[en_mcast_direction].en_frame_dscr_switch = en_dscr_switch;

        /* �����ܿ��� */
        oam_report_data_set_global_switch(en_mcast_direction);
    }

    return OAL_SUCC;
}


oal_uint32  oam_report_80211_mcast_get_switch(
                                        oam_ota_frame_direction_type_enum_uint8 en_mcast_direction,
                                        oam_user_track_frame_type_enum_uint8    en_frame_type,
                                        oal_switch_enum_uint8                  *pen_frame_switch,
                                        oal_switch_enum_uint8                  *pen_cb_switch,
                                        oal_switch_enum_uint8                  *pen_dscr_switch)
{
    if (en_mcast_direction >= OAM_OTA_FRAME_DIRECTION_TYPE_BUTT)
    {
        return OAL_ERR_CODE_OAM_EVT_FRAME_DIR_INVALID;
    }

    if (en_frame_type >= OAM_USER_TRACK_FRAME_TYPE_BUTT)
    {
        return OAL_ERR_CODE_CONFIG_UNSUPPORT;
    }

    if (OAL_UNLIKELY(OAL_PTR_NULL == pen_frame_switch)
        || OAL_UNLIKELY(OAL_PTR_NULL == pen_cb_switch)
        || OAL_UNLIKELY(OAL_PTR_NULL == pen_dscr_switch))
    {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (OAM_USER_TRACK_FRAME_TYPE_MGMT == en_frame_type)
    {
        *pen_frame_switch = g_st_oam_mng_ctx.st_user_track_ctx.ast_80211_mcast_mgmt_ctx[en_mcast_direction].en_frame_content_switch;
        *pen_cb_switch    = g_st_oam_mng_ctx.st_user_track_ctx.ast_80211_mcast_mgmt_ctx[en_mcast_direction].en_frame_cb_switch;
        *pen_dscr_switch  = g_st_oam_mng_ctx.st_user_track_ctx.ast_80211_mcast_mgmt_ctx[en_mcast_direction].en_frame_dscr_switch;
    }
    else
    {
        *pen_frame_switch = g_st_oam_mng_ctx.st_user_track_ctx.ast_80211_mcast_data_ctx[en_mcast_direction].en_frame_content_switch;
        *pen_cb_switch    = g_st_oam_mng_ctx.st_user_track_ctx.ast_80211_mcast_data_ctx[en_mcast_direction].en_frame_cb_switch;
        *pen_dscr_switch  = g_st_oam_mng_ctx.st_user_track_ctx.ast_80211_mcast_data_ctx[en_mcast_direction].en_frame_dscr_switch;
    }

    return OAL_SUCC;
}


oal_uint32  oam_report_80211_ucast_set_switch(
                                        oam_ota_frame_direction_type_enum_uint8 en_ucast_direction,
                                        oam_user_track_frame_type_enum_uint8    en_frame_type,
                                        oal_switch_enum_uint8                   en_frame_switch,
                                        oal_switch_enum_uint8                   en_cb_switch,
                                        oal_switch_enum_uint8                   en_dscr_switch,
                                        oal_uint16                              us_user_idx)
{
    if (en_ucast_direction >= OAM_OTA_FRAME_DIRECTION_TYPE_BUTT)
    {
        return OAL_ERR_CODE_OAM_EVT_FRAME_DIR_INVALID;
    }

    if (en_frame_type >= OAM_USER_TRACK_FRAME_TYPE_BUTT)
    {
        return OAL_ERR_CODE_CONFIG_UNSUPPORT;
    }

    if (us_user_idx >= WLAN_USER_MAX_USER_LIMIT)
    {
        return OAL_ERR_CODE_OAM_EVT_USER_IDX_EXCEED;
    }

    if (OAL_UNLIKELY(en_frame_switch >= OAL_SWITCH_BUTT)
        || OAL_UNLIKELY(en_cb_switch >= OAL_SWITCH_BUTT)
        || OAL_UNLIKELY(en_dscr_switch  >= OAL_SWITCH_BUTT))
    {
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    if (OAM_USER_TRACK_FRAME_TYPE_MGMT == en_frame_type)
    {
        g_st_oam_mng_ctx.st_user_track_ctx.ast_80211_ucast_mgmt_ctx[us_user_idx][en_ucast_direction].en_frame_content_switch = en_frame_switch;/* [false alarm]:���������жϣ�����Խ��*/
        g_st_oam_mng_ctx.st_user_track_ctx.ast_80211_ucast_mgmt_ctx[us_user_idx][en_ucast_direction].en_frame_cb_switch = en_cb_switch;/* [false alarm]:���������жϣ�����Խ��*/
        g_st_oam_mng_ctx.st_user_track_ctx.ast_80211_ucast_mgmt_ctx[us_user_idx][en_ucast_direction].en_frame_dscr_switch = en_dscr_switch;/* [false alarm]:���������жϣ�����Խ��*/
    }
    else
    {
        g_st_oam_mng_ctx.st_user_track_ctx.ast_80211_ucast_data_ctx[us_user_idx][en_ucast_direction].en_frame_content_switch = en_frame_switch;/* [false alarm]:���������жϣ�����Խ��*/
        g_st_oam_mng_ctx.st_user_track_ctx.ast_80211_ucast_data_ctx[us_user_idx][en_ucast_direction].en_frame_cb_switch = en_cb_switch;/* [false alarm]:���������жϣ�����Խ��*/
        g_st_oam_mng_ctx.st_user_track_ctx.ast_80211_ucast_data_ctx[us_user_idx][en_ucast_direction].en_frame_dscr_switch = en_dscr_switch;/* [false alarm]:���������жϣ�����Խ��*/

        /* �����ܿ��� */
        oam_report_data_set_global_switch(en_ucast_direction);
    }
    return OAL_SUCC;
}


oal_uint32  oam_report_80211_ucast_get_switch(
                                        oam_ota_frame_direction_type_enum_uint8 en_ucast_direction,
                                        oam_user_track_frame_type_enum_uint8    en_frame_type,
                                        oal_switch_enum_uint8                  *pen_frame_switch,
                                        oal_switch_enum_uint8                  *pen_cb_switch,
                                        oal_switch_enum_uint8                  *pen_dscr_switch,
                                        oal_uint16                              us_user_idx)
{
    if (en_ucast_direction >= OAM_OTA_FRAME_DIRECTION_TYPE_BUTT)
    {
        return OAL_ERR_CODE_OAM_EVT_FRAME_DIR_INVALID;
    }

    if (en_frame_type >= OAM_USER_TRACK_FRAME_TYPE_BUTT)
    {
        return OAL_ERR_CODE_CONFIG_UNSUPPORT;
    }

    if (us_user_idx >= WLAN_USER_MAX_USER_LIMIT)
    {
        return OAL_ERR_CODE_OAM_EVT_USER_IDX_EXCEED;
    }

    if (OAL_UNLIKELY(OAL_PTR_NULL == pen_frame_switch)
        || OAL_UNLIKELY(OAL_PTR_NULL == pen_cb_switch)
        || OAL_UNLIKELY(OAL_PTR_NULL == pen_dscr_switch))
    {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (OAM_USER_TRACK_FRAME_TYPE_MGMT == en_frame_type)
    {
        *pen_frame_switch = g_st_oam_mng_ctx.st_user_track_ctx.ast_80211_ucast_mgmt_ctx[us_user_idx][en_ucast_direction].en_frame_content_switch;/* [false alarm]:���������жϣ�����Խ��*/
        *pen_cb_switch    = g_st_oam_mng_ctx.st_user_track_ctx.ast_80211_ucast_mgmt_ctx[us_user_idx][en_ucast_direction].en_frame_cb_switch;/* [false alarm]:���������жϣ�����Խ��*/
        *pen_dscr_switch  = g_st_oam_mng_ctx.st_user_track_ctx.ast_80211_ucast_mgmt_ctx[us_user_idx][en_ucast_direction].en_frame_dscr_switch;/* [false alarm]:���������жϣ�����Խ��*/
    }
    else
    {
        *pen_frame_switch = g_st_oam_mng_ctx.st_user_track_ctx.ast_80211_ucast_data_ctx[us_user_idx][en_ucast_direction].en_frame_content_switch;/* [false alarm]:���������жϣ�����Խ��*/
        *pen_cb_switch    = g_st_oam_mng_ctx.st_user_track_ctx.ast_80211_ucast_data_ctx[us_user_idx][en_ucast_direction].en_frame_cb_switch;/* [false alarm]:���������жϣ�����Խ��*/
        *pen_dscr_switch  = g_st_oam_mng_ctx.st_user_track_ctx.ast_80211_ucast_data_ctx[us_user_idx][en_ucast_direction].en_frame_dscr_switch;/* [false alarm]:���������жϣ�����Խ��*/
    }

    return OAL_SUCC;
}


oal_uint32  oam_report_80211_probe_set_switch(
                                        oam_ota_frame_direction_type_enum_uint8 en_probe_direction,
                                        oal_switch_enum_uint8                   en_frame_switch,
                                        oal_switch_enum_uint8                   en_cb_switch,
                                        oal_switch_enum_uint8                   en_dscr_switch)
{
    if (en_probe_direction >= OAM_OTA_FRAME_DIRECTION_TYPE_BUTT)
    {
        return OAL_ERR_CODE_OAM_EVT_FRAME_DIR_INVALID;
    }

    if (OAL_UNLIKELY(en_frame_switch >= OAL_SWITCH_BUTT)
        || OAL_UNLIKELY(en_cb_switch >= OAL_SWITCH_BUTT)
        || OAL_UNLIKELY(en_dscr_switch >= OAL_SWITCH_BUTT))
    {
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    g_st_oam_mng_ctx.st_user_track_ctx.aen_80211_probe_switch[en_probe_direction].en_frame_content_switch = en_frame_switch;
    g_st_oam_mng_ctx.st_user_track_ctx.aen_80211_probe_switch[en_probe_direction].en_frame_cb_switch = en_cb_switch;
    g_st_oam_mng_ctx.st_user_track_ctx.aen_80211_probe_switch[en_probe_direction].en_frame_dscr_switch = en_dscr_switch;

    return OAL_SUCC;
}


oal_uint32  oam_report_80211_probe_get_switch(
                                        oam_ota_frame_direction_type_enum_uint8 en_probe_direction,
                                        oal_switch_enum_uint8                  *pen_frame_switch,
                                        oal_switch_enum_uint8                  *pen_cb_switch,
                                        oal_switch_enum_uint8                  *pen_dscr_switch)
{
    if (en_probe_direction >= OAM_OTA_FRAME_DIRECTION_TYPE_BUTT)
    {
        return OAL_ERR_CODE_OAM_EVT_FRAME_DIR_INVALID;
    }

    if (OAL_UNLIKELY(OAL_PTR_NULL == pen_frame_switch)
        || OAL_UNLIKELY(OAL_PTR_NULL == pen_cb_switch)
        || OAL_UNLIKELY(OAL_PTR_NULL == pen_dscr_switch))
    {
        return OAL_ERR_CODE_PTR_NULL;
    }

    *pen_frame_switch = g_st_oam_mng_ctx.st_user_track_ctx.aen_80211_probe_switch[en_probe_direction].en_frame_content_switch;
    *pen_cb_switch    = g_st_oam_mng_ctx.st_user_track_ctx.aen_80211_probe_switch[en_probe_direction].en_frame_cb_switch;
    *pen_dscr_switch  = g_st_oam_mng_ctx.st_user_track_ctx.aen_80211_probe_switch[en_probe_direction].en_frame_dscr_switch;

    return OAL_SUCC;
}


oal_uint32  oam_report_dhcp_arp_set_switch(oal_switch_enum_uint8 en_switch)
{
    g_st_oam_mng_ctx.st_user_track_ctx.en_tx_mcast_dhcp_arp_switch = en_switch;

    return OAL_SUCC;
}


oal_switch_enum_uint8  oam_report_dhcp_arp_get_switch(oal_void)
{
    return g_st_oam_mng_ctx.st_user_track_ctx.en_tx_mcast_dhcp_arp_switch;
}
#if 0

oal_switch_enum_uint8 oam_report_get_switch(oam_ota_type_enum_uint8 en_type)
{
    if (en_type >= OAM_OTA_TYPE_BUTT)
    {
        return OAL_SWITCH_OFF;
    }

    return g_st_oam_mng_ctx.ast_ota_ctx[0].aen_ota_switch[en_type];
}
#endif

oal_uint32  oam_event_get_switch(
                oal_uint8              uc_vap_id,
                oal_switch_enum_uint8 *pen_switch_type)
{
    if (OAL_PTR_NULL == pen_switch_type)
    {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (uc_vap_id >= WLAN_VAP_SUPPORT_MAX_NUM_LIMIT)
    {
        return OAL_ERR_CODE_ARRAY_OVERFLOW;
    }

    *pen_switch_type = g_st_oam_mng_ctx.ast_event_ctx[uc_vap_id].en_event_switch;

    return OAL_SUCC;
}


oal_uint32  oam_event_set_switch(
                oal_uint8               uc_vap_id,
                oal_switch_enum_uint8   en_switch_type)
{
    if (en_switch_type >= OAL_SWITCH_BUTT)
    {
        OAL_IO_PRINT("oam_event_set_switch::event_type[%d] invalid. \n", en_switch_type);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    if (uc_vap_id >= WLAN_VAP_SUPPORT_MAX_NUM_LIMIT)
    {
        OAL_IO_PRINT("oam_event_set_switch::vap_id[%d] >= WLAN_VAP_SUPPORT_MAX_NUM_LIMIT.\n", uc_vap_id);
        return OAL_ERR_CODE_ARRAY_OVERFLOW;
    }

    g_st_oam_mng_ctx.ast_event_ctx[uc_vap_id].en_event_switch = en_switch_type;

    return OAL_SUCC;
}


oal_uint32  oam_event_set_specific_type_switch(
                            oal_uint8                   uc_vap_id,
                            oal_switch_enum_uint8       en_switch_type,
                            oam_event_type_enum_uint16  en_event_type)
{
    if (en_switch_type >= OAL_SWITCH_BUTT)
    {
        OAL_IO_PRINT("oam_event_set_specific_type_switch::en_switch_type[%d] invalid. \n", en_switch_type);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    if (uc_vap_id >= WLAN_VAP_SUPPORT_MAX_NUM_LIMIT)
    {
        OAL_IO_PRINT("oam_event_set_specific_type_switch::vap_id[%d] >= WLAN_VAP_SUPPORT_MAX_NUM_LIMIT. \n", uc_vap_id);
        return OAL_ERR_CODE_ARRAY_OVERFLOW;
    }

    g_st_oam_mng_ctx.ast_specific_event_ctx[uc_vap_id].aen_specific_event_switch[en_event_type] = en_switch_type;

    return OAL_SUCC;
}


oal_uint32  oam_ota_set_beacon_switch(
                  oal_uint8                                   uc_vap_id,
                  oam_sdt_print_beacon_rxdscr_type_enum_uint8 en_switch_type)
{
    if (en_switch_type >= OAM_SDT_PRINT_BEACON_RXDSCR_TYPE_BUTT)
    {
        OAL_IO_PRINT("oam_ota_set_beacon_switch::event_type[%d] exceeds! \n", en_switch_type);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    if (uc_vap_id >= WLAN_VAP_SUPPORT_MAX_NUM_LIMIT)
    {
        OAL_IO_PRINT("oam_ota_set_beacon_switch::vap_id[%d] exceeds! \n", uc_vap_id);
        return OAL_ERR_CODE_ARRAY_OVERFLOW;
    }

    g_st_oam_mng_ctx.ast_ota_ctx[uc_vap_id].en_beacon_switch = en_switch_type;

    return OAL_SUCC;
}


oam_sdt_print_beacon_rxdscr_type_enum_uint8  oam_ota_get_beacon_switch(oal_void)
{
    return g_st_oam_mng_ctx.ast_ota_ctx[0].en_beacon_switch;
}


oal_uint32  oam_ota_set_rx_dscr_switch(
                                                  oal_uint8             uc_vap_id,
                                                  oal_switch_enum_uint8 en_switch_type)
{
    if (en_switch_type >= OAM_PROFILING_SWITCH_BUTT)
    {
        OAL_IO_PRINT("oam_ota_set_rx_dscr_switch::event_type[%d] exceeds! \n", en_switch_type);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    if (uc_vap_id >= WLAN_VAP_SUPPORT_MAX_NUM_LIMIT)
    {
        OAL_IO_PRINT("oam_ota_set_rx_dscr_switch::vap_id[%d] exceeds! \n", uc_vap_id);
        return OAL_ERR_CODE_ARRAY_OVERFLOW;
    }

    g_st_oam_mng_ctx.ast_ota_ctx[uc_vap_id].en_rx_dscr_switch = en_switch_type;

    return OAL_SUCC;
}

oal_switch_enum_uint8  oam_ota_get_rx_dscr_switch(oal_void)
{
    return g_st_oam_mng_ctx.ast_ota_ctx[0].en_rx_dscr_switch;
}


OAL_STATIC oal_uint32  oam_event_format_string(
                oal_int8                       *pac_output_data,
                oal_uint16                      ul_data_len,
                oal_uint8                       uc_vap_id,
                oam_module_id_enum_uint16       en_mod,
                oam_event_type_enum_uint16      en_event_type)
{
    oal_uint32          ul_tick;

    if (OAL_PTR_NULL == pac_output_data)
    {
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* ��ȡϵͳTICKֵ */
    ul_tick = (oal_uint32)OAL_TIME_GET_STAMP_MS();

    /* ��ʽ��������� */
    OAL_SPRINTF(pac_output_data,
                ul_data_len,
                "��EVENT��:Tick=%u, VAP=%d, ModId=%d, EVENT TYPE=%u\r\n",
                ul_tick,
                uc_vap_id,
                en_mod,
                en_event_type);

    return OAL_SUCC;
}


OAL_STATIC oal_uint32  oam_event_print_to_std(
                oal_uint8                   uc_vap_id,
                oam_module_id_enum_uint16   en_mod,
                oam_event_type_enum_uint16  en_event_type)
{
    oal_int8   ac_output_data[OAM_PRINT_FORMAT_LENGTH];    /* ���ڱ���д�뵽�ļ��е����� */
    oal_uint32 ul_rslt;

    ul_rslt = oam_event_format_string(ac_output_data,
                                      OAM_PRINT_FORMAT_LENGTH,
                                      uc_vap_id,
                                      en_mod,
                                      en_event_type);

    if (OAL_SUCC != ul_rslt)
    {
        return ul_rslt;
    }

    ul_rslt = oam_print_to_console(ac_output_data);

    if (OAL_SUCC != ul_rslt)
    {
        return ul_rslt;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32  oam_event_print_to_file(
                oal_uint8                   uc_vap_id,
                oam_module_id_enum_uint16   en_mod,
                oam_event_type_enum_uint16  en_event_type)
{
#ifdef _PRE_WIFI_DMT
    oal_int8   ac_output_data[OAM_PRINT_FORMAT_LENGTH];  /* ���ڱ���д�뵽�ļ��е����� */
    oal_uint32 ul_rslt;

    ul_rslt = oam_event_format_string(ac_output_data,
                                      OAM_PRINT_FORMAT_LENGTH,
                                      uc_vap_id,
                                      en_mod,
                                      en_event_type);

    if (OAL_SUCC != ul_rslt)
    {
        return ul_rslt;
    }

    ul_rslt = oam_print_to_file(ac_output_data);

    if (OAL_SUCC != ul_rslt)
    {
        return ul_rslt;
    }
#endif
    return OAL_SUCC;
}


OAL_STATIC oal_uint32  oam_event_print_to_sdt(
                                    oal_uint8                 *puc_mac_hdr_addr,
                                    oal_uint8                   uc_vap_id,
                                    oam_module_id_enum_uint16   en_mod,
                                    oam_event_type_enum_uint16  en_event_type,
                                    oal_uint8                  *puc_string)
{
    oam_event_stru      st_event;
    oal_uint32          ul_tick;
    oal_netbuf_stru    *pst_netbuf;
    oal_uint32          ul_ret          = OAL_SUCC;

    if (OAL_UNLIKELY(OAL_PTR_NULL == g_st_oam_sdt_func_hook.p_sdt_report_data_func))
    {
        return OAL_ERR_CODE_PTR_NULL;
    }

    OAL_MEMZERO(&st_event, OAL_SIZEOF(oam_event_stru));

    /* ��ȡϵͳʱ�� */
    ul_tick = (oal_uint32)OAL_TIME_GET_STAMP_MS();

    /* ��дevent�ṹ�� */
    st_event.st_event_hdr.ul_tick        = ul_tick;
    st_event.st_event_hdr.uc_vap_id      = uc_vap_id;
    st_event.st_event_hdr.en_module      = en_mod;
    st_event.st_event_hdr.en_event_type  = en_event_type;
    oal_set_mac_addr(st_event.st_event_hdr.auc_user_macaddr, puc_mac_hdr_addr);

    oal_memcopy((oal_void *)st_event.auc_event_info,
                (const oal_void *)puc_string,
                 OAM_EVENT_INFO_MAX_LEN);

    /* ��event��Ϣ���Ƶ�netbuf�ϱ���SDT,ͷ��Ԥ��8�ֽڣ�β��Ԥ��1�ֽڣ���sdt_drv�� */
    pst_netbuf = oam_alloc_data2sdt(OAM_EVENT_STRU_SIZE);
    if (OAL_PTR_NULL == pst_netbuf)
    {
        return OAL_ERR_CODE_PTR_NULL;
    }

    oal_memcopy((oal_void *)oal_netbuf_data(pst_netbuf),
                (const oal_void *)&st_event,
                OAM_EVENT_STRU_SIZE);

    /* �ж�sdt������Ϣ�����Ƿ�������������������� */
    ul_ret = oam_report_data2sdt(pst_netbuf, OAM_DATA_TYPE_EVENT, OAM_PRIMID_TYPE_OUTPUT_CONTENT);

    return ul_ret;
}


oal_uint32  oam_event_report(
                        oal_uint8                  *puc_mac_hdr_addr,
                        oal_uint8                   uc_vap_id,
                        oam_module_id_enum_uint16   en_mod,
                        oam_event_type_enum_uint16  en_event_type,
                        oal_uint8                  *puc_string)
{
    oal_uint32          ul_rslt;

    if (en_event_type >= OAM_EVENT_TYPE_BUTT)
    {
        return OAL_FAIL;
    }

    if (uc_vap_id >= WLAN_VAP_SUPPORT_MAX_NUM_LIMIT)
    {
        return OAL_ERR_CODE_ARRAY_OVERFLOW;
    }

    if ((OAL_PTR_NULL == puc_string) || (OAL_PTR_NULL == puc_mac_hdr_addr))
    {
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* ���event�ϱ��ܿ����Ƿ�� */
    if (OAL_SWITCH_OFF == g_st_oam_mng_ctx.ast_event_ctx[uc_vap_id].en_event_switch)
    {
        return OAL_SUCC;
    }

    /* ������ĳһ��event�ϱ������Ƿ�� */
    if (OAL_SWITCH_OFF == g_st_oam_mng_ctx.ast_specific_event_ctx[uc_vap_id].aen_specific_event_switch[en_event_type])
    {
        return OAL_SUCC;
    }

    switch (g_st_oam_mng_ctx.en_output_type)
    {
        /* ���������̨ */
        case OAM_OUTPUT_TYPE_CONSOLE:
            ul_rslt = oam_event_print_to_std(uc_vap_id, en_mod, en_event_type);

            break;

        /* ������ļ�ϵͳ�� */
        case OAM_OUTPUT_TYPE_FS:
            ul_rslt = oam_event_print_to_file(uc_vap_id, en_mod, en_event_type);
            break;

        /* �����PC����⹤��ƽ̨ */
        case OAM_OUTPUT_TYPE_SDT:
            ul_rslt = oam_event_print_to_sdt(puc_mac_hdr_addr,
                                             uc_vap_id, en_mod,
                                             en_event_type,
                                             puc_string);

            break;

        /* ��Ч���� */
        default:
            ul_rslt = OAL_ERR_CODE_INVALID_CONFIG;

            break;
    }

    if (OAL_SUCC != ul_rslt)
    {
        return ul_rslt;
    }

    return OAL_SUCC;
}


oal_uint32  oam_event_init(oal_void)
{
    oal_uint32 ul_rslt;
    oal_uint32 ul_vapid_loop;
    oal_uint32 ul_eventtype_loop;
#if 0
    oal_uint32 ul_otatype_loop;
#endif
    /* ��ʼ������VAPĬ��EVENT����Ϊ�� */
    for (ul_vapid_loop = 0; ul_vapid_loop < WLAN_VAP_SUPPORT_MAX_NUM_LIMIT; ul_vapid_loop++)
    {
        /* ����EVENT�ܿ��� */
        ul_rslt = oam_event_set_switch((oal_uint8)ul_vapid_loop, OAL_SWITCH_ON);

        if (OAL_SUCC != ul_rslt)
        {
            return ul_rslt;
        }

        /* ���þ���ĳһ��EVENT�Ŀ��� */
        for (ul_eventtype_loop = 0; ul_eventtype_loop < OAM_EVENT_TYPE_BUTT; ul_eventtype_loop++)
        {
            oam_event_set_specific_type_switch((oal_uint8)ul_vapid_loop, OAL_SWITCH_ON, (oal_uint16)ul_eventtype_loop);
        }

        /* �ڲ����¼�event��Ҫ�ر� */
        oam_event_set_specific_type_switch((oal_uint8)ul_vapid_loop, OAL_SWITCH_OFF, OAM_EVENT_INTERNAL);
        oam_event_set_specific_type_switch((oal_uint8)ul_vapid_loop, OAL_SWITCH_OFF, OAM_EVENT_USER_INFO_CHANGE);

        /* ����beacon��ӡ����Ϊ�ر� */
        oam_ota_set_beacon_switch((oal_uint8)ul_vapid_loop, OAL_SWITCH_OFF);
        /* ����rx��������ӡ����Ϊ�ر� */
        oam_ota_set_rx_dscr_switch((oal_uint8)ul_vapid_loop, OAL_SWITCH_OFF);
    }

    /* ���õ��û�������ص����п��� */
    oam_report_set_all_switch(OAL_SWITCH_OFF);

    oam_report_dhcp_arp_set_switch(OAL_SWITCH_OFF);

    return OAL_SUCC;
}


OAL_STATIC oal_uint32  oam_ota_report_to_std(oal_uint8     *puc_param_one_addr,
                                       oal_uint16      us_param_one_len,
                                       oal_uint8      *puc_param_two_addr,
                                       oal_uint16      us_param_two_len,
                                       oam_ota_type_enum_uint8   en_ota_type)
{
    if (0 == us_param_two_len)
    {
        OAL_IO_PRINT("\n\nOTA TYPE is--> %d and OTA DATA is:\n", en_ota_type);
        oam_dump_buff_by_hex(puc_param_one_addr, us_param_one_len, 4);
    }
    else
    {
        /* ��ӡ��һ������ */
        OAL_IO_PRINT("\n\nOTA TYPE is--> %d and OTA DATA the first part is:\n", en_ota_type);
        oam_dump_buff_by_hex(puc_param_one_addr, us_param_one_len, OAM_PRINT_CRLF_NUM);

        /* ��ӡ�ڶ������� */
        OAL_IO_PRINT("\nOTA DATA tht second part is:\n");

        if (OAM_OTA_TYPE_80211_FRAME == en_ota_type)
        {
            us_param_two_len = OAL_MIN(us_param_two_len, OAM_OTA_DATA_TO_STD_MAX_LEN);
        }

        oam_dump_buff_by_hex(puc_param_two_addr, us_param_two_len, OAM_PRINT_CRLF_NUM);
    }

    return OAL_SUCC;
}


/*lint -e662*/
oal_uint32  oam_ota_report_to_sdt(oal_uint8    *puc_param_one_addr,
                                       oal_uint16     us_param_one_len,
                                       oal_uint8     *puc_param_two_addr,
                                       oal_uint16     us_param_two_len,
                                       oam_ota_type_enum_uint8  en_ota_type)
{
    oal_uint32           ul_ret             = OAL_SUCC;
#if ((_PRE_OS_VERSION_RAW != _PRE_OS_VERSION) && (_PRE_OS_VERSION_WIN32_RAW != _PRE_OS_VERSION))
    oal_uint32           ul_tick;
    oal_uint16           us_skb_len;
    oal_netbuf_stru     *pst_netbuf;
    oam_ota_stru        *pst_ota_data;


    if (OAL_UNLIKELY(OAL_PTR_NULL == g_st_oam_sdt_func_hook.p_sdt_report_data_func))
    {
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* Ϊ�ϱ�OTA��������ռ䣬�������������us_param_two_lenΪ0 */
    us_skb_len = us_param_one_len + us_param_two_len + OAL_SIZEOF(oam_ota_hdr_stru);
    if (us_skb_len > WLAN_SDT_NETBUF_MAX_PAYLOAD)
    {
        us_skb_len = WLAN_SDT_NETBUF_MAX_PAYLOAD;
        if ((us_param_one_len + OAL_SIZEOF(oam_ota_hdr_stru)) < us_skb_len)
        {
            us_param_two_len = us_skb_len - us_param_one_len - (oal_uint16)OAL_SIZEOF(oam_ota_hdr_stru);
        }
        else
        {
            us_param_one_len = us_skb_len - OAL_SIZEOF(oam_ota_hdr_stru);
            us_param_two_len = 0;
        }
    }

    pst_netbuf = oam_alloc_data2sdt(us_skb_len);
    if (OAL_PTR_NULL == pst_netbuf)
    {
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_ota_data = (oam_ota_stru *)oal_netbuf_data(pst_netbuf);

    /* ��ȡϵͳTICKֵ */
    ul_tick = (oal_uint32)OAL_TIME_GET_STAMP_MS();

    pst_ota_data->st_ota_hdr.ul_tick     = ul_tick;
    pst_ota_data->st_ota_hdr.en_ota_type = en_ota_type;

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_HOST)
    pst_ota_data->st_ota_hdr.auc_resv[0]    = OAM_OTA_TYPE_1102_HOST;
#elif (_PRE_PRODUCT_ID ==_PRE_PRODUCT_ID_HI1103_HOST)
    pst_ota_data->st_ota_hdr.auc_resv[0]    = OAM_OTA_TYPE_1103_HOST;
#elif (_PRE_PRODUCT_ID ==_PRE_PRODUCT_ID_HI1102A_HOST)
    pst_ota_data->st_ota_hdr.auc_resv[0]    = OAM_OTA_TYPE_1102A_HOST;
#else
    pst_ota_data->st_ota_hdr.auc_resv[0]    = OAM_OTA_TYPE_1151_HOST;
#endif


    switch (en_ota_type)
    {
        /* ��װ���ջ��߷�����������ӦOTA�ṹ���������Ա */
        case OAM_OTA_TYPE_RX_DSCR:
        case OAM_OTA_TYPE_TX_DSCR:
        case OAM_OTA_TYPE_IRQ:
        case OAM_OTA_TYPE_EVENT_QUEUE:
        case OAM_OTA_TYPE_TIMER:
        case OAM_OTA_TYPE_MEMPOOL:
        case OAM_OTA_TYPE_HMAC_VAP:
        case OAM_OTA_TYPE_DMAC_VAP:
        case OAM_OTA_TYPE_HMAC_USER:
        case OAM_OTA_TYPE_DMAC_USER:
        case OAM_OTA_TYPE_HMAC_VAP_MEMBER_SIZE:
        case OAM_OTA_TYPE_DMAC_VAP_MEMBER_SIZE:
        case OAM_OTA_TYPE_RX_DSCR_PILOT:
        case OAM_OTA_TYPE_TX_DSCR_PILOT:
            pst_ota_data->st_ota_hdr.us_ota_data_len = us_param_one_len;
            oal_memcopy((oal_void *)pst_ota_data->auc_ota_data,
                        (const oal_void *)puc_param_one_addr,
                        (oal_uint32)us_param_one_len);

            break;

        /* ��װ֡��ӦOTA�ṹ���������Ա */
        case OAM_OTA_TYPE_80211_FRAME:
        case OAM_OTA_TYPE_MEMBLOCK:
            pst_ota_data->st_ota_hdr.uc_frame_hdr_len = (oal_uint8)us_param_one_len;
            pst_ota_data->st_ota_hdr.us_ota_data_len  = us_param_one_len + us_param_two_len;

            /* ����֡ͷ */
            oal_memcopy((oal_void *)pst_ota_data->auc_ota_data,
                        (const oal_void *)puc_param_one_addr,
                        (oal_uint32)us_param_one_len);

            /* ����֡�� */
            oal_memcopy((oal_void *)(pst_ota_data->auc_ota_data + us_param_one_len),
                        (const oal_void *)puc_param_two_addr,
                        (oal_uint32)us_param_two_len);

            break;

        /* ��Чֵ */
        default:
            oal_mem_sdt_netbuf_free(pst_netbuf, OAL_TRUE);
            //oal_netbuf_free(pst_netbuf);
            return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /* �ж�sdt������Ϣ�����Ƿ�������������������� */
    ul_ret = oam_report_data2sdt(pst_netbuf, OAM_DATA_TYPE_OTA, OAM_PRIMID_TYPE_OUTPUT_CONTENT);
#endif
    return ul_ret;
}



oal_uint32  oam_ota_report(
                           oal_uint8     *puc_param_one_addr,
                           oal_uint16     us_param_one_len,
                           oal_uint8     *puc_param_two_addr,
                           oal_uint16     us_param_two_len,
                           oam_ota_type_enum_uint8  en_ota_type)
{
    oal_uint32          ul_rslt = OAL_ERR_CODE_BUTT;
#if 0
    /* ���Ҫ��ӡ��ota���Ϳ����Ƿ�� */
    if (OAL_SWITCH_OFF == g_st_oam_mng_ctx.ast_ota_ctx[0].aen_ota_switch[en_ota_type])
    {
        return OAL_SUCC;
    }
#endif
    switch (g_st_oam_mng_ctx.en_output_type)
    {
        /* ���������̨ */
        case OAM_OUTPUT_TYPE_CONSOLE:
        ul_rslt = oam_ota_report_to_std(puc_param_one_addr,
                                        us_param_one_len,
                                        puc_param_two_addr,
                                        us_param_two_len,
                                        en_ota_type);

        break;

        /* �����SDT���� */
        case OAM_OUTPUT_TYPE_SDT:
        ul_rslt = oam_ota_report_to_sdt(puc_param_one_addr,
                                        us_param_one_len,
                                        puc_param_two_addr,
                                        us_param_two_len,
                                        en_ota_type);

        break;

        default:
        ul_rslt = OAL_ERR_CODE_INVALID_CONFIG;

        break;
    }

    if (OAL_SUCC != ul_rslt)
    {
        return ul_rslt;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_uint32  oam_report_80211_frame_to_console(
                        oal_uint8     *puc_mac_hdr_addr,
                        oal_uint8      uc_mac_hdr_len,
                        oal_uint8     *puc_mac_body_addr,
                        oal_uint16     us_mac_frame_len,
                        oam_ota_frame_direction_type_enum_uint8  en_frame_direction)
{
    oal_uint16      us_80211_frame_body_len;

    if (OAM_OTA_FRAME_DIRECTION_TYPE_TX == en_frame_direction)
    {
        OAL_IO_PRINT("oam_report_80211_frame_to_console::tx_80211_frame header:\n");
    }
    else
    {
        OAL_IO_PRINT("oam_report_80211_frame_to_console::rx_80211_frame header:\n");
    }

    oam_dump_buff_by_hex(puc_mac_hdr_addr, uc_mac_hdr_len, OAM_PRINT_CRLF_NUM);

    if (uc_mac_hdr_len > us_mac_frame_len)
    {
        OAL_IO_PRINT("oam_report_80211_frame_to_console::rx_80211_frame invalid frame\n");
        return OAL_FAIL;
    }

    us_80211_frame_body_len   = us_mac_frame_len - uc_mac_hdr_len;

    OAL_IO_PRINT("oam_report_80211_frame_to_console::80211_frame body:\n");
    oam_dump_buff_by_hex(puc_mac_body_addr, us_80211_frame_body_len, OAM_PRINT_CRLF_NUM);

    return OAL_SUCC;
}

OAL_STATIC oal_void oam_hide_mac_addr(oal_uint8 *puc_mac_hdr, oal_uint8 uc_beacon_hdr_len)
{
    if (OAL_PTR_NULL == puc_mac_hdr || WLAN_MGMT_FRAME_HEADER_LEN > uc_beacon_hdr_len)
    {
        return;
    }
    /* addr1 */
    puc_mac_hdr[5] = 0xff;    /* [false alarm]:fortify��  */
    puc_mac_hdr[6] = 0xff;    /* [false alarm]:fortify��  */
    puc_mac_hdr[7] = 0xff;    /* [false alarm]:fortify��  */

    /* addr2 */
    puc_mac_hdr[11] = 0xff;   /* [false alarm]:fortify��  */
    puc_mac_hdr[12] = 0xff;   /* [false alarm]:fortify��  */
    puc_mac_hdr[13] = 0xff;   /* [false alarm]:fortify��  */

    /* addr3 */
    puc_mac_hdr[17] = 0xff;   /* [false alarm]:fortify��  */
    puc_mac_hdr[18] = 0xff;   /* [false alarm]:fortify��  */
    puc_mac_hdr[19] = 0xff;   /* [false alarm]:fortify��  */

}


OAL_STATIC oal_uint32  oam_report_80211_frame_to_sdt(
                        oal_uint8     *puc_user_macaddr,
                        oal_uint8     *puc_mac_hdr_addr,
                        oal_uint8      uc_mac_hdr_len,
                        oal_uint8     *puc_mac_body_addr,
                        oal_uint16     us_mac_frame_len,
                        oam_ota_frame_direction_type_enum_uint8  en_frame_direction)
{
    oal_uint32           ul_tick;
    oal_uint16           us_skb_len;
    oal_netbuf_stru     *pst_netbuf;
    oam_ota_stru        *pst_ota_data;
    oal_uint32           ul_ret             = OAL_SUCC;

    if (OAL_UNLIKELY(OAL_PTR_NULL == g_st_oam_sdt_func_hook.p_sdt_report_data_func))
    {
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* Ϊ�ϱ�80211֡����ռ� */
    us_skb_len = us_mac_frame_len + OAL_SIZEOF(oam_ota_hdr_stru);
    if (us_skb_len > WLAN_SDT_NETBUF_MAX_PAYLOAD)
    {
        us_skb_len = WLAN_SDT_NETBUF_MAX_PAYLOAD;
        us_mac_frame_len = WLAN_SDT_NETBUF_MAX_PAYLOAD - OAL_SIZEOF(oam_ota_hdr_stru);
    }

    pst_netbuf = oam_alloc_data2sdt(us_skb_len);
    if (OAL_PTR_NULL == pst_netbuf)
    {
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_ota_data = (oam_ota_stru *)OAL_NETBUF_HEADER(pst_netbuf);

    /* ��ȡϵͳTICKֵ */
    ul_tick = (oal_uint32)OAL_TIME_GET_STAMP_MS();

    /* ��дota��Ϣͷ�ṹ�� */
    pst_ota_data->st_ota_hdr.ul_tick            = ul_tick;
    pst_ota_data->st_ota_hdr.en_ota_type        = OAM_OTA_TYPE_80211_FRAME;
    pst_ota_data->st_ota_hdr.uc_frame_hdr_len   = uc_mac_hdr_len;
    pst_ota_data->st_ota_hdr.us_ota_data_len    = us_mac_frame_len;
    pst_ota_data->st_ota_hdr.en_frame_direction = en_frame_direction;
    oal_set_mac_addr(pst_ota_data->st_ota_hdr.auc_user_macaddr, puc_user_macaddr);
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_HOST)
    pst_ota_data->st_ota_hdr.auc_resv[0]    = OAM_OTA_TYPE_1102_HOST;
#elif (_PRE_PRODUCT_ID ==_PRE_PRODUCT_ID_HI1103_HOST)
    pst_ota_data->st_ota_hdr.auc_resv[0]    = OAM_OTA_TYPE_1103_HOST;
#elif (_PRE_PRODUCT_ID ==_PRE_PRODUCT_ID_HI1102A_HOST)
    pst_ota_data->st_ota_hdr.auc_resv[0]    = OAM_OTA_TYPE_1102A_HOST;
#else
    pst_ota_data->st_ota_hdr.auc_resv[0]    = OAM_OTA_TYPE_1151_HOST;
#endif

    /* ����֡ͷ */
    oal_memcopy((oal_void *)pst_ota_data->auc_ota_data,
                (const oal_void *)puc_mac_hdr_addr,
                (oal_uint32)uc_mac_hdr_len);
    /* ����֡�� */
    oal_memcopy((oal_void *)(pst_ota_data->auc_ota_data + uc_mac_hdr_len),
                (const oal_void *)puc_mac_body_addr,
                (oal_uint32)(us_mac_frame_len - uc_mac_hdr_len));

    oam_hide_mac_addr(pst_ota_data->auc_ota_data, uc_mac_hdr_len);

    /* �ж�sdt������Ϣ�����Ƿ�������������������� */
    ul_ret = oam_report_data2sdt(pst_netbuf, OAM_DATA_TYPE_OTA, OAM_PRIMID_TYPE_OUTPUT_CONTENT);

    return ul_ret;
}



oal_uint32  oam_report_80211_frame(
                        oal_uint8     *puc_user_macaddr,
                        oal_uint8     *puc_mac_hdr_addr,
                        oal_uint8      uc_mac_hdr_len,
                        oal_uint8     *puc_mac_body_addr,
                        oal_uint16     us_mac_frame_len,
                        oam_ota_frame_direction_type_enum_uint8  en_frame_direction)
{
    oal_uint32      ul_ret      = OAL_SUCC;
    oal_uint32      ul_oam_ret  = OAL_SUCC;
    oal_uint32      ul_return_addr = 0;

    /* Less IO_print, less oam_report_80211_frame_to_console, use oam_report_80211_frame_to_sdt for log.
       Suggested by gongxiangling & wangzhenzhong */

#if (_PRE_OS_VERSION_RAW == _PRE_OS_VERSION)
    ul_return_addr  = __return_address();
#endif

    if (OAL_UNLIKELY(OAL_PTR_NULL == puc_mac_hdr_addr
        || OAL_PTR_NULL == puc_mac_body_addr
        || OAL_PTR_NULL == puc_user_macaddr))
    {
        OAM_ERROR_LOG4(0, OAM_SF_ANY, "{oam_report_80211_frame:[device] puc_mac_hdr_addr = 0x%X, puc_mac_body_addr = 0x%X, puc_user_macaddr = 0x%X, __return_address = 0x%X}",
                        puc_mac_hdr_addr,puc_mac_body_addr,puc_user_macaddr, ul_return_addr);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* ���֡ͷ���ȺϷ��� */
    if ((WLAN_MAX_FRAME_HEADER_LEN < uc_mac_hdr_len)
        || (WLAN_MIN_FRAME_HEADER_LEN > uc_mac_hdr_len))
    {
        oam_report_dft_params(BROADCAST_MACADDR,puc_mac_hdr_addr,OAM_OTA_FRAME_TO_SDT_MAX_LEN,OAM_OTA_TYPE_80211_FRAME);
        OAM_WARNING_LOG4(0, OAM_SF_ANY, "{oam_report_80211_frame:HEAD CHECK! hearder_len = %d, frame_len = %d, en_frame_direction = %d, return_addres = 0x%X}",
                        uc_mac_hdr_len,us_mac_frame_len,en_frame_direction,ul_return_addr);
        return OAL_ERR_CODE_OAM_EVT_FR_HDR_LEN_INVALID;
    }

    /* ���mac֡�ܳ��ȺϷ��� */
    if (uc_mac_hdr_len > us_mac_frame_len)
    {
        oam_report_dft_params(BROADCAST_MACADDR,puc_mac_hdr_addr,OAM_OTA_FRAME_TO_SDT_MAX_LEN,OAM_OTA_TYPE_80211_FRAME);
        OAM_WARNING_LOG4(0, OAM_SF_ANY, "{oam_report_80211_frame:HEAD/FRAME CHECK! hearder_len = %d, frame_len = %d, en_frame_direction = %d, return_addres = 0x%X}",
                        uc_mac_hdr_len,us_mac_frame_len,en_frame_direction,ul_return_addr);
        return OAL_ERR_CODE_OAM_EVT_FR_LEN_INVALID;
    }

    us_mac_frame_len = (us_mac_frame_len > WLAN_MAX_FRAME_LEN) ? WLAN_MAX_FRAME_LEN : us_mac_frame_len;

    if (OAL_UNLIKELY(en_frame_direction >= OAM_OTA_FRAME_DIRECTION_TYPE_BUTT))
    {
        return OAL_ERR_CODE_OAM_EVT_FRAME_DIR_INVALID;
    }

    switch (g_st_oam_mng_ctx.en_output_type)
    {
        /* ���������̨ */
        case OAM_OUTPUT_TYPE_CONSOLE:
            ul_ret = oam_report_80211_frame_to_console(puc_mac_hdr_addr,
                                                       uc_mac_hdr_len,
                                                       puc_mac_body_addr,
                                                       us_mac_frame_len,
                                                       en_frame_direction);
            break;

        /* �����SDT���� */
        case OAM_OUTPUT_TYPE_SDT:
            /* �����ж� */
            if (OAM_RATELIMIT_NOT_OUTPUT == oam_log_ratelimit(OAM_RATELIMIT_TYPE_FRAME_WLAN))
            {
                ul_oam_ret = OAL_SUCC;
            }
            else
            {
                ul_oam_ret = oam_report_80211_frame_to_sdt(puc_user_macaddr,
                                                       puc_mac_hdr_addr,
                                                       uc_mac_hdr_len,
                                                       puc_mac_body_addr,
                                                       us_mac_frame_len,
                                                       en_frame_direction);
            }
            break;

        default:
            ul_oam_ret = OAL_ERR_CODE_INVALID_CONFIG;
            break;
    }

    if ((OAL_SUCC != ul_oam_ret) ||
        (OAL_SUCC != ul_ret) )
    {
        OAM_WARNING_LOG4(0, OAM_SF_ANY, "{oam_report_80211_frame:[device] en_frame_direction = %d, ul_ret = %d, ul_oam_ret = %d, return_addres = 0x%X}",
                        en_frame_direction, ul_ret, ul_oam_ret, ul_return_addr);
    }

    return ((OAL_SUCC != ul_ret)?(ul_ret):(ul_oam_ret));
}


OAL_STATIC oal_uint32  oam_report_dscr_to_console(
                                    oal_uint8  *puc_dscr_addr,
                                    oal_uint16  us_dscr_len,
                                    oam_ota_type_enum_uint8  en_ota_type)
{
    if ((OAM_OTA_TYPE_RX_DSCR == en_ota_type)||(OAM_OTA_TYPE_RX_DSCR_PILOT== en_ota_type))
    {
        OAL_IO_PRINT("oam_report_dscr_to_console::rx_dscr info:\n\n");
    }

    else
    {
        OAL_IO_PRINT("oam_report_dscr_to_console::tx_dscr info:\n\n");
    }

    oam_dump_buff_by_hex(puc_dscr_addr, us_dscr_len, 4);

    return OAL_SUCC;
}


OAL_STATIC oal_uint32  oam_report_dscr_to_sdt(
                                        oal_uint8  *puc_user_macaddr,
                                        oal_uint8  *puc_dscr_addr,
                                        oal_uint16  us_dscr_len,
                                        oam_ota_type_enum_uint8  en_ota_type)
{
    oal_uint32           ul_tick;
    oal_uint16           us_skb_len;
    oal_netbuf_stru     *pst_netbuf;
    oam_ota_stru        *pst_ota_data;
    oal_uint32           ul_ret         = OAL_SUCC;

    if (OAL_UNLIKELY(OAL_PTR_NULL == g_st_oam_sdt_func_hook.p_sdt_report_data_func))
    {
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* Ϊ�ϱ�����������ռ� */
    us_skb_len = us_dscr_len + OAL_SIZEOF(oam_ota_hdr_stru);
    if (us_skb_len > WLAN_SDT_NETBUF_MAX_PAYLOAD)
    {
        us_skb_len = WLAN_SDT_NETBUF_MAX_PAYLOAD;
        us_dscr_len = WLAN_SDT_NETBUF_MAX_PAYLOAD - OAL_SIZEOF(oam_ota_hdr_stru);
    }

    pst_netbuf = oam_alloc_data2sdt(us_skb_len);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_netbuf))
    {
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_ota_data = (oam_ota_stru *)oal_netbuf_data(pst_netbuf);

    /* ��ȡϵͳTICKֵ */
    ul_tick = (oal_uint32)OAL_TIME_GET_STAMP_MS();

    /* ��дota��Ϣͷ�ṹ�� */
    pst_ota_data->st_ota_hdr.ul_tick     = ul_tick;
    pst_ota_data->st_ota_hdr.en_ota_type = en_ota_type;
    pst_ota_data->st_ota_hdr.us_ota_data_len = us_dscr_len;
    pst_ota_data->st_ota_hdr.uc_frame_hdr_len = 0;
    oal_set_mac_addr(pst_ota_data->st_ota_hdr.auc_user_macaddr, puc_user_macaddr);
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_HOST)
    pst_ota_data->st_ota_hdr.auc_resv[0]    = OAM_OTA_TYPE_1102_HOST;
#elif (_PRE_PRODUCT_ID ==_PRE_PRODUCT_ID_HI1103_HOST)
    pst_ota_data->st_ota_hdr.auc_resv[0]    = OAM_OTA_TYPE_1103_HOST;
#elif (_PRE_PRODUCT_ID ==_PRE_PRODUCT_ID_HI1102A_HOST)
    pst_ota_data->st_ota_hdr.auc_resv[0]    = OAM_OTA_TYPE_1102A_HOST;
#else
    pst_ota_data->st_ota_hdr.auc_resv[0]    = OAM_OTA_TYPE_1151_HOST;
#endif

    /* ��������,��дota���� */
    oal_memcopy((oal_void *)pst_ota_data->auc_ota_data,
                (const oal_void *)puc_dscr_addr,
                (oal_uint32)us_dscr_len);

    /* �ж�sdt������Ϣ�����Ƿ�������������������� */
    ul_ret = oam_report_data2sdt(pst_netbuf, OAM_DATA_TYPE_OTA, OAM_PRIMID_TYPE_OUTPUT_CONTENT);

    return ul_ret;
}



oal_uint32 oam_report_dscr(oal_uint8  *puc_user_macaddr,
                               oal_uint8  *puc_dscr_addr,
                               oal_uint16  us_dscr_len,
                               oam_ota_type_enum_uint8  en_ota_type)
{
    oal_uint32   ul_ret;

    if (OAL_UNLIKELY((OAL_PTR_NULL == puc_user_macaddr) || (OAL_PTR_NULL == puc_dscr_addr)))
    {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if ((WLAN_MEM_SHARED_TX_DSCR_SIZE2 < us_dscr_len)
        || (us_dscr_len == 0))
    {
        oam_dump_buff_by_hex(puc_dscr_addr, us_dscr_len, OAM_PRINT_CRLF_NUM);
        return OAL_ERR_CODE_OAM_EVT_DSCR_LEN_INVALID;
    }

#ifdef _PRE_WLAN_1103_PILOT
    if (OAL_UNLIKELY((OAM_OTA_TYPE_RX_DSCR_PILOT != en_ota_type) && (OAM_OTA_TYPE_TX_DSCR_PILOT != en_ota_type)))
    {
        return OAL_ERR_CODE_CONFIG_UNSUPPORT;
    }
#else
    if (OAL_UNLIKELY((OAM_OTA_TYPE_RX_DSCR != en_ota_type) && (OAM_OTA_TYPE_TX_DSCR != en_ota_type)))
    {
        return OAL_ERR_CODE_CONFIG_UNSUPPORT;
    }
#endif

    switch (g_st_oam_mng_ctx.en_output_type)
    {
        /* ���������̨ */
        case OAM_OUTPUT_TYPE_CONSOLE:
            ul_ret = oam_report_dscr_to_console(puc_dscr_addr, us_dscr_len, en_ota_type);

            break;

        /* �����SDT���� */
        case OAM_OUTPUT_TYPE_SDT:
            /* �����ж� */
            if (OAM_RATELIMIT_NOT_OUTPUT == oam_log_ratelimit(OAM_RATELIMIT_TYPE_DSCR))
            {
                return OAL_SUCC;
            }
            ul_ret = oam_report_dscr_to_sdt(puc_user_macaddr, puc_dscr_addr, us_dscr_len, en_ota_type);

            break;

        default:
            ul_ret = OAL_ERR_CODE_INVALID_CONFIG;

            break;
    }

    return ul_ret;
}


OAL_STATIC oal_uint32  oam_report_beacon_to_console(
                        oal_uint8               *puc_beacon_hdr_addr,
                        oal_uint16               us_beacon_len,
                        oam_ota_frame_direction_type_enum_uint8 en_beacon_direction)
{
    oal_uint8      *puc_beacon_body_addr;
    oal_uint16      us_beacon_body_len;

    if (OAM_OTA_FRAME_DIRECTION_TYPE_TX == en_beacon_direction)
    {
        OAL_IO_PRINT("oam_report_beacon_to_console::tx_beacon info:\n");
    }
    else
    {
        OAL_IO_PRINT("oam_report_beacon_to_console::rx_beacon info:\n");
    }
    OAL_IO_PRINT("oam_report_beacon_to_console::beacon_header:\n");

    oam_dump_buff_by_hex(puc_beacon_hdr_addr, OAM_BEACON_HDR_LEN, OAM_PRINT_CRLF_NUM);

    puc_beacon_body_addr = puc_beacon_hdr_addr + OAM_BEACON_HDR_LEN;
    us_beacon_body_len   = us_beacon_len - OAM_BEACON_HDR_LEN;

    OAL_IO_PRINT("oam_report_beacon_to_console::beacon_body:\n");
    oam_dump_buff_by_hex(puc_beacon_body_addr, us_beacon_body_len, OAM_PRINT_CRLF_NUM);

    return OAL_SUCC;
}


OAL_STATIC oal_uint32  oam_report_beacon_to_sdt(
                                oal_uint8     *puc_beacon_hdr_addr,
                                oal_uint8      uc_beacon_hdr_len,
                                oal_uint8     *puc_beacon_body_addr,
                                oal_uint16     us_beacon_len,
                                oam_ota_frame_direction_type_enum_uint8 en_beacon_direction)
{
    oal_uint32           ul_tick;
    oal_uint16           us_skb_len;
    oal_netbuf_stru     *pst_netbuf;
    oam_ota_stru        *pst_ota_data;
    oal_uint32           ul_ret         = OAL_SUCC;

    if (OAL_UNLIKELY(OAL_PTR_NULL == g_st_oam_sdt_func_hook.p_sdt_report_data_func))
    {
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* Ϊ�ϱ�beacon֡����ռ� */
    us_skb_len = us_beacon_len + OAL_SIZEOF(oam_ota_hdr_stru);
    if (us_skb_len > WLAN_SDT_NETBUF_MAX_PAYLOAD)
    {
        us_skb_len = WLAN_SDT_NETBUF_MAX_PAYLOAD;
        us_beacon_len = WLAN_SDT_NETBUF_MAX_PAYLOAD - OAL_SIZEOF(oam_ota_hdr_stru);
    }

    pst_netbuf = oam_alloc_data2sdt(us_skb_len);
    if (OAL_PTR_NULL == pst_netbuf)
    {
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_ota_data = (oam_ota_stru *)oal_netbuf_data(pst_netbuf);

    /* ��ȡϵͳTICKֵ */
    ul_tick = (oal_uint32)OAL_TIME_GET_STAMP_MS();

    /* ��дota��Ϣͷ�ṹ�� */
    pst_ota_data->st_ota_hdr.ul_tick            = ul_tick;
    pst_ota_data->st_ota_hdr.en_ota_type        = OAM_OTA_TYPE_BEACON;
    pst_ota_data->st_ota_hdr.uc_frame_hdr_len   = OAM_BEACON_HDR_LEN;
    pst_ota_data->st_ota_hdr.us_ota_data_len    = us_beacon_len;
    pst_ota_data->st_ota_hdr.en_frame_direction = en_beacon_direction;
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_HOST)
    pst_ota_data->st_ota_hdr.auc_resv[0]    = OAM_OTA_TYPE_1102_HOST;
#elif (_PRE_PRODUCT_ID ==_PRE_PRODUCT_ID_HI1103_HOST)
    pst_ota_data->st_ota_hdr.auc_resv[0]    = OAM_OTA_TYPE_1103_HOST;
#elif (_PRE_PRODUCT_ID ==_PRE_PRODUCT_ID_HI1102A_HOST)
    pst_ota_data->st_ota_hdr.auc_resv[0]    = OAM_OTA_TYPE_1102A_HOST;
#else
    pst_ota_data->st_ota_hdr.auc_resv[0]    = OAM_OTA_TYPE_1151_HOST;
#endif

    /* ��������,��дota���� */
    oal_memcopy((oal_void *)pst_ota_data->auc_ota_data,
                (const oal_void *)puc_beacon_hdr_addr,
                (oal_uint32)uc_beacon_hdr_len);
    oal_memcopy((oal_void *)(pst_ota_data->auc_ota_data + uc_beacon_hdr_len),
                (const oal_void *)puc_beacon_body_addr,
                (oal_uint32)(us_beacon_len - uc_beacon_hdr_len));

    oam_hide_mac_addr(pst_ota_data->auc_ota_data, uc_beacon_hdr_len);

    /* �·���sdt���ն��У����������򴮿���� */
    ul_ret = oam_report_data2sdt(pst_netbuf, OAM_DATA_TYPE_OTA, OAM_PRIMID_TYPE_OUTPUT_CONTENT);

    return ul_ret;
}


oal_uint32  oam_report_beacon(
                                    oal_uint8     *puc_beacon_hdr_addr,
                                    oal_uint8      uc_beacon_hdr_len,
                                    oal_uint8     *puc_beacon_body_addr,
                                    oal_uint16     us_beacon_len,
                                    oam_ota_frame_direction_type_enum_uint8 en_beacon_direction)
{
    oal_uint32   ul_ret;

    if (en_beacon_direction >= OAM_OTA_FRAME_DIRECTION_TYPE_BUTT)
    {
        return OAL_ERR_CODE_OAM_EVT_FRAME_DIR_INVALID;
    }

    if ((OAM_SDT_PRINT_BEACON_RXDSCR_TYPE_BEACON != g_st_oam_mng_ctx.ast_ota_ctx[0].en_beacon_switch)
        && (OAM_SDT_PRINT_BEACON_RXDSCR_TYPE_BOTH != g_st_oam_mng_ctx.ast_ota_ctx[0].en_beacon_switch))
    {
        return OAL_SUCC;
    }

    if ((OAL_PTR_NULL == puc_beacon_hdr_addr)||(OAL_PTR_NULL == puc_beacon_body_addr))
    {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if ((WLAN_MAX_FRAME_LEN < us_beacon_len)
        || (us_beacon_len <= WLAN_MGMT_FRAME_HEADER_LEN))
    {
        oam_dump_buff_by_hex(puc_beacon_hdr_addr, us_beacon_len, OAM_PRINT_CRLF_NUM);
        return OAL_ERR_CODE_OAM_EVT_FR_LEN_INVALID;
    }


    switch (g_st_oam_mng_ctx.en_output_type)
    {
        /* ���������̨ */
        case OAM_OUTPUT_TYPE_CONSOLE:
            ul_ret = oam_report_beacon_to_console(puc_beacon_hdr_addr,
                                                  us_beacon_len,
                                                  en_beacon_direction);

            break;

        /* �����SDT���� */
        case OAM_OUTPUT_TYPE_SDT:
            ul_ret = oam_report_beacon_to_sdt(puc_beacon_hdr_addr,
                                              uc_beacon_hdr_len,
                                              puc_beacon_body_addr,
                                              us_beacon_len,
                                              en_beacon_direction);

            break;

        default:
            ul_ret = OAL_ERR_CODE_INVALID_CONFIG;

            break;
    }

    return ul_ret;
}


OAL_STATIC oal_uint32  oam_report_eth_frame_to_console(
                        oal_uint8               *puc_eth_frame_hdr_addr,
                        oal_uint16               us_eth_frame_len,
                        oam_ota_frame_direction_type_enum_uint8 en_eth_frame_direction)
{
    oal_uint8      *puc_eth_frame_body_addr;
    oal_uint16      us_eth_frame_body_len;

    if (OAM_OTA_FRAME_DIRECTION_TYPE_TX == en_eth_frame_direction)
    {
        OAL_IO_PRINT("oam_report_eth_frame_to_console::recv frame from eth:\n");
    }
    else
    {
        OAL_IO_PRINT("oam_report_eth_frame_to_console::report frame to eth:\n");
    }

    OAL_IO_PRINT("oam_report_eth_frame_to_console::eth_frame header:\n");
    oam_dump_buff_by_hex(puc_eth_frame_hdr_addr, ETHER_HDR_LEN, OAM_PRINT_CRLF_NUM);

    puc_eth_frame_body_addr = puc_eth_frame_hdr_addr + ETHER_HDR_LEN;
    us_eth_frame_body_len   = us_eth_frame_len - ETHER_HDR_LEN;

    OAL_IO_PRINT("oam_report_eth_frame_to_console::eth_frame body:\n");
    oam_dump_buff_by_hex(puc_eth_frame_body_addr, us_eth_frame_body_len, OAM_PRINT_CRLF_NUM);

    return OAL_SUCC;
}


OAL_STATIC oal_uint32  oam_report_eth_frame_to_sdt(
                        oal_uint8               *puc_user_mac_addr,
                        oal_uint8               *puc_eth_frame_hdr_addr,
                        oal_uint16               us_eth_frame_len,
                        oam_ota_frame_direction_type_enum_uint8 en_eth_frame_direction)
{
    oal_uint32           ul_tick;
    oal_uint16           us_skb_len;
    oal_netbuf_stru     *pst_netbuf;
    oam_ota_stru        *pst_ota_data;
    oal_uint32           ul_ret     = OAL_SUCC;

    if (OAL_UNLIKELY(OAL_PTR_NULL == g_st_oam_sdt_func_hook.p_sdt_report_data_func))
    {
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* Ϊ�ϱ�beacon֡����ռ� */
    us_skb_len = us_eth_frame_len + OAL_SIZEOF(oam_ota_hdr_stru);
    if (us_skb_len > WLAN_SDT_NETBUF_MAX_PAYLOAD)
    {
        us_skb_len = WLAN_SDT_NETBUF_MAX_PAYLOAD;
        us_eth_frame_len = WLAN_SDT_NETBUF_MAX_PAYLOAD - OAL_SIZEOF(oam_ota_hdr_stru);
    }

    pst_netbuf = oam_alloc_data2sdt(us_skb_len);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_netbuf))
    {
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_ota_data = (oam_ota_stru *)oal_netbuf_data(pst_netbuf);

    /* ��ȡϵͳTICKֵ */
    ul_tick = (oal_uint32)OAL_TIME_GET_STAMP_MS();

    /* ��дota��Ϣͷ�ṹ�� */
    pst_ota_data->st_ota_hdr.ul_tick            = ul_tick;
    pst_ota_data->st_ota_hdr.en_ota_type        = OAM_OTA_TYPE_ETH_FRAME;
    pst_ota_data->st_ota_hdr.uc_frame_hdr_len   = ETHER_HDR_LEN;
    pst_ota_data->st_ota_hdr.us_ota_data_len    = us_eth_frame_len;
    pst_ota_data->st_ota_hdr.en_frame_direction = en_eth_frame_direction;
    oal_set_mac_addr(pst_ota_data->st_ota_hdr.auc_user_macaddr, puc_user_mac_addr);
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_HOST)
    pst_ota_data->st_ota_hdr.auc_resv[0]    = OAM_OTA_TYPE_1102_HOST;
#elif (_PRE_PRODUCT_ID ==_PRE_PRODUCT_ID_HI1103_HOST)
    pst_ota_data->st_ota_hdr.auc_resv[0]    = OAM_OTA_TYPE_1103_HOST;
#elif (_PRE_PRODUCT_ID ==_PRE_PRODUCT_ID_HI1102A_HOST)
    pst_ota_data->st_ota_hdr.auc_resv[0]    = OAM_OTA_TYPE_1102A_HOST;
#else
    pst_ota_data->st_ota_hdr.auc_resv[0]    = OAM_OTA_TYPE_1151_HOST;
#endif

    /* ��������,��дota���� */
    oal_memcopy((oal_void *)pst_ota_data->auc_ota_data,
                (const oal_void *)puc_eth_frame_hdr_addr,
                (oal_uint32)us_eth_frame_len);

    /* �·���sdt���ն��У����������򴮿���� */
    ul_ret = oam_report_data2sdt(pst_netbuf, OAM_DATA_TYPE_OTA, OAM_PRIMID_TYPE_OUTPUT_CONTENT);

    return ul_ret;
}


oal_uint32  oam_report_eth_frame(oal_uint8               *puc_user_mac_addr,
                                       oal_uint8               *puc_eth_frame_hdr_addr,
                                       oal_uint16               us_eth_frame_len,
                                       oam_ota_frame_direction_type_enum_uint8 en_eth_frame_direction)
{
    oal_uint32      ul_ret;

    if (OAL_UNLIKELY((OAL_PTR_NULL == puc_user_mac_addr) || (OAL_PTR_NULL == puc_eth_frame_hdr_addr)))
    {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if ((ETHER_MAX_LEN < us_eth_frame_len)
        || (us_eth_frame_len < ETHER_HDR_LEN))
    {
        oam_dump_buff_by_hex(puc_eth_frame_hdr_addr, us_eth_frame_len, OAM_PRINT_CRLF_NUM);

        return OAL_ERR_CODE_OAM_EVT_FR_LEN_INVALID;
    }

    if (OAL_UNLIKELY(en_eth_frame_direction >= OAM_OTA_FRAME_DIRECTION_TYPE_BUTT))
    {
        return OAL_ERR_CODE_OAM_EVT_FRAME_DIR_INVALID;
    }

    switch (g_st_oam_mng_ctx.en_output_type)
    {
        /* ���������̨ */
        case OAM_OUTPUT_TYPE_CONSOLE:
            ul_ret = oam_report_eth_frame_to_console(puc_eth_frame_hdr_addr,
                                                     us_eth_frame_len,
                                                     en_eth_frame_direction);

            break;

        /* �����SDT���� */
        case OAM_OUTPUT_TYPE_SDT:
            /* �����ж� */
            if (OAM_RATELIMIT_NOT_OUTPUT == oam_log_ratelimit(OAM_RATELIMIT_TYPE_FRAME_ETH))
            {
                return OAL_SUCC;
            }
            ul_ret = oam_report_eth_frame_to_sdt(puc_user_mac_addr,
                                                 puc_eth_frame_hdr_addr,
                                                 us_eth_frame_len,
                                                 en_eth_frame_direction);
            break;

        default:
            ul_ret = OAL_ERR_CODE_INVALID_CONFIG;

            break;
    }

    return ul_ret;
}


OAL_STATIC oal_uint32  oam_report_netbuf_cb_to_sdt(
                                oal_uint8  *puc_user_mac_addr,
                                oal_uint8  *puc_netbuf_cb,
                                oam_ota_type_enum_uint8 en_ota_type)
{
    oal_uint32           ul_ret     = OAL_SUCC;
#if ((_PRE_OS_VERSION_RAW != _PRE_OS_VERSION) && (_PRE_OS_VERSION_WIN32_RAW != _PRE_OS_VERSION))
    oal_uint32           ul_tick;
    oal_uint16           us_ota_data_len;
    oal_uint16           us_skb_len;
    oal_netbuf_stru     *pst_netbuf;
    oam_ota_stru        *pst_ota_data;

    if (OAL_UNLIKELY(OAL_PTR_NULL == g_st_oam_sdt_func_hook.p_sdt_report_data_func))
    {
        return OAL_ERR_CODE_PTR_NULL;
    }

    us_skb_len = OAM_SKB_CB_LEN + OAL_SIZEOF(oam_ota_hdr_stru) + OAM_RESERVE_SKB_LEN;
    us_ota_data_len = OAM_SKB_CB_LEN + OAL_SIZEOF(oam_ota_hdr_stru);

    pst_netbuf = oal_mem_sdt_netbuf_alloc(us_skb_len, OAL_TRUE);
    if (OAL_PTR_NULL == pst_netbuf)
    {
        return OAL_ERR_CODE_PTR_NULL;
    }
    oal_netbuf_reserve(pst_netbuf, OAM_RESERVE_SKB_HEADROOM_LEN);

    oal_netbuf_put(pst_netbuf, us_ota_data_len);
    pst_ota_data = (oam_ota_stru *)oal_netbuf_data(pst_netbuf);

    /* ��ȡϵͳTICKֵ */
    ul_tick = (oal_uint32)OAL_TIME_GET_STAMP_MS();

    /* ��дota��Ϣͷ�ṹ�� */
    pst_ota_data->st_ota_hdr.ul_tick     = ul_tick;
    pst_ota_data->st_ota_hdr.en_ota_type = en_ota_type;
    pst_ota_data->st_ota_hdr.us_ota_data_len = OAM_SKB_CB_LEN;
    pst_ota_data->st_ota_hdr.uc_frame_hdr_len = 0;
    oal_set_mac_addr(pst_ota_data->st_ota_hdr.auc_user_macaddr, puc_user_mac_addr);
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_HOST)
    pst_ota_data->st_ota_hdr.auc_resv[0]    = OAM_OTA_TYPE_1102_HOST;
#elif (_PRE_PRODUCT_ID ==_PRE_PRODUCT_ID_HI1103_HOST)
    pst_ota_data->st_ota_hdr.auc_resv[0]    = OAM_OTA_TYPE_1103_HOST;
#elif (_PRE_PRODUCT_ID ==_PRE_PRODUCT_ID_HI1102A_HOST)
    pst_ota_data->st_ota_hdr.auc_resv[0]    = OAM_OTA_TYPE_1102A_HOST;
#else
    pst_ota_data->st_ota_hdr.auc_resv[0]    = OAM_OTA_TYPE_1151_HOST;
#endif

    /* ��������,��дota���� */
    oal_memcopy((oal_void *)pst_ota_data->auc_ota_data,
                (const oal_void *)puc_netbuf_cb,
                OAM_SKB_CB_LEN);

    /* �ϱ�SDT */
    ul_ret = oam_report_data2sdt(pst_netbuf, OAM_DATA_TYPE_OTA, OAM_PRIMID_TYPE_OUTPUT_CONTENT);
#endif
    return ul_ret;
}



oal_uint32  oam_report_netbuf_cb(oal_uint8  *puc_user_mac_addr,
                                       oal_uint8  *puc_netbuf_cb,
                                       oam_ota_type_enum_uint8 en_ota_type)
{
    oal_uint32      ul_ret;

    if (OAL_PTR_NULL == puc_user_mac_addr || OAL_PTR_NULL == puc_netbuf_cb)
    {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (en_ota_type >= OAM_OTA_TYPE_BUTT)
    {
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /* �����ж� */
    if (OAM_RATELIMIT_NOT_OUTPUT == oam_log_ratelimit(OAM_RATELIMIT_TYPE_CB))
    {
        return OAL_SUCC;
    }
    ul_ret = oam_report_netbuf_cb_to_sdt(puc_user_mac_addr, puc_netbuf_cb, en_ota_type);

    return ul_ret;
}
#if 0

OAL_STATIC oal_uint32  oam_report_timer_track_to_sdt(
                                        oal_uint32 ul_file_id,
                                        oal_uint32 ul_line_num,
                                        oam_timer_track_type_enum_uint8 en_type)
{
    oal_uint32                  ul_tick;
    oal_uint16                  us_ota_data_len;
    oal_uint16                  us_skb_len;
    oal_netbuf_stru            *pst_netbuf;
    oam_ota_stru               *pst_ota_data;
    oal_uint32                  ul_ret     = OAL_SUCC;
    oam_timer_track_info_stru   st_timer_track_info;
    oal_uint8                   auc_macaddr[WLAN_MAC_ADDR_LEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    if (OAL_UNLIKELY(OAL_PTR_NULL == g_st_oam_sdt_func_hook.p_sdt_report_data_func))
    {
        return OAL_ERR_CODE_PTR_NULL;
    }

    us_skb_len = OAL_SIZEOF(st_timer_track_info)
               + OAL_SIZEOF(oam_ota_hdr_stru) + OAM_RESERVE_SKB_LEN;
    us_ota_data_len = OAL_SIZEOF(st_timer_track_info) + OAL_SIZEOF(oam_ota_hdr_stru);

    pst_netbuf = oal_mem_sdt_netbuf_alloc(us_skb_len, OAL_TRUE);
    if (OAL_PTR_NULL == pst_netbuf)
    {
        return OAL_ERR_CODE_PTR_NULL;
    }
    oal_netbuf_reserve(pst_netbuf, OAM_RESERVE_SKB_HEADROOM_LEN);

    oal_netbuf_put(pst_netbuf, us_ota_data_len);
    pst_ota_data = (oam_ota_stru *)oal_netbuf_data(pst_netbuf);

    /* ��ȡϵͳTICKֵ */
    ul_tick = (oal_uint32)OAL_TIME_GET_STAMP_MS();

    /* ��дota��Ϣͷ�ṹ�� */
    pst_ota_data->st_ota_hdr.ul_tick     = ul_tick;
    pst_ota_data->st_ota_hdr.en_ota_type = OAM_OTA_TYPE_TIMER_TRACK;
    pst_ota_data->st_ota_hdr.us_ota_data_len = OAL_SIZEOF(st_timer_track_info);
    pst_ota_data->st_ota_hdr.uc_frame_hdr_len = 0;
    oal_set_mac_addr(pst_ota_data->st_ota_hdr.auc_user_macaddr, auc_macaddr);
#if defined(_PRE_PRODUCT_ID_HI110X_HOST)
    pst_ota_data->st_ota_hdr.auc_resv[0]    = 2;
#else
    pst_ota_data->st_ota_hdr.auc_resv[0]    = 3;
#endif

    st_timer_track_info.ul_file_id  = ul_file_id;
    st_timer_track_info.ul_line_num = ul_line_num;
    st_timer_track_info.en_type     = en_type;

    /* ��������,��дota���� */
    oal_memcopy((oal_void *)pst_ota_data->auc_ota_data,
                (const oal_void *)&st_timer_track_info,
                OAL_SIZEOF(st_timer_track_info));

    /* �ϱ�SDT */
    ul_ret = oam_report_data2sdt(pst_netbuf, OAM_DATA_TYPE_OTA, OAM_PRIMID_TYPE_OUTPUT_CONTENT);

    return ul_ret;
}


oal_uint32  oam_report_timer_track(oal_uint32 ul_file_id,
                                        oal_uint32 ul_line_num,
                                        oam_timer_track_type_enum_uint8 en_type)
{
    if (OAL_UNLIKELY(ul_file_id >= OAM_FILE_ID_BUTT || en_type >= OAM_TIMER_TRACK_TYPE_BUTT))
    {
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /* ��鿪�� */
    if (OAL_SWITCH_OFF == oam_report_get_switch(OAM_OTA_TYPE_TIMER_TRACK))
    {
        return OAL_SUCC;
    }

    return oam_report_timer_track_to_sdt(ul_file_id, ul_line_num, en_type);
}
#endif

OAL_STATIC oal_uint32  oam_report_mpdu_num_to_sdt(
                                        oal_uint8  *puc_user_mac_addr,
                                        oam_report_mpdu_num_stru *pst_mpdu_num)
{
    oal_uint32                  ul_tick;
    oal_uint16                  us_ota_data_len;
    oal_uint16                  us_skb_len;
    oal_netbuf_stru            *pst_netbuf;
    oam_ota_stru               *pst_ota_data;
    oal_uint32                  ul_ret     = OAL_SUCC;

    if (OAL_UNLIKELY(OAL_PTR_NULL == g_st_oam_sdt_func_hook.p_sdt_report_data_func))
    {
        return OAL_ERR_CODE_PTR_NULL;
    }

    us_skb_len = OAL_SIZEOF(oam_report_mpdu_num_stru)
               + OAL_SIZEOF(oam_ota_hdr_stru) + OAM_RESERVE_SKB_LEN;
    us_ota_data_len = OAL_SIZEOF(oam_report_mpdu_num_stru) + OAL_SIZEOF(oam_ota_hdr_stru);

    pst_netbuf = oal_mem_sdt_netbuf_alloc(us_skb_len, OAL_TRUE);
    if (OAL_PTR_NULL == pst_netbuf)
    {
        return OAL_ERR_CODE_PTR_NULL;
    }
    oal_netbuf_reserve(pst_netbuf, OAM_RESERVE_SKB_HEADROOM_LEN);

    oal_netbuf_put(pst_netbuf, us_ota_data_len);
    pst_ota_data = (oam_ota_stru *)oal_netbuf_data(pst_netbuf);

    /* ��ȡϵͳTICKֵ */
    ul_tick = (oal_uint32)OAL_TIME_GET_STAMP_MS();

    /* ��дota��Ϣͷ�ṹ�� */
    pst_ota_data->st_ota_hdr.ul_tick     = ul_tick;
    pst_ota_data->st_ota_hdr.en_ota_type = OAM_OTA_TYPE_MPDU_NUM;
    pst_ota_data->st_ota_hdr.us_ota_data_len = OAL_SIZEOF(oam_report_mpdu_num_stru);
    pst_ota_data->st_ota_hdr.uc_frame_hdr_len = 0;
    oal_set_mac_addr(pst_ota_data->st_ota_hdr.auc_user_macaddr, puc_user_mac_addr);
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_HOST)
    pst_ota_data->st_ota_hdr.auc_resv[0]    = OAM_OTA_TYPE_1102_HOST;
#elif (_PRE_PRODUCT_ID ==_PRE_PRODUCT_ID_HI1103_HOST)
    pst_ota_data->st_ota_hdr.auc_resv[0]    = OAM_OTA_TYPE_1103_HOST;
#elif (_PRE_PRODUCT_ID ==_PRE_PRODUCT_ID_HI1102A_HOST)
    pst_ota_data->st_ota_hdr.auc_resv[0]    = OAM_OTA_TYPE_1102A_HOST;
#else
    pst_ota_data->st_ota_hdr.auc_resv[0]    = OAM_OTA_TYPE_1151_HOST;
#endif

    /* ��������,��дota���� */
    oal_memcopy((oal_void *)pst_ota_data->auc_ota_data,
                (const oal_void *)pst_mpdu_num,
                OAL_SIZEOF(oam_report_mpdu_num_stru));

    /* �ϱ�SDT */
    ul_ret = oam_report_data2sdt(pst_netbuf, OAM_DATA_TYPE_OTA, OAM_PRIMID_TYPE_OUTPUT_CONTENT);

    return ul_ret;
}



oal_uint32  oam_report_mpdu_num(oal_uint8  *puc_user_mac_addr,
                                        oam_report_mpdu_num_stru *pst_mpdu_num)
{
    if (OAL_UNLIKELY(OAL_PTR_NULL == puc_user_mac_addr || OAL_PTR_NULL == pst_mpdu_num))
    {
        return OAL_ERR_CODE_PTR_NULL;
    }

    return oam_report_mpdu_num_to_sdt(puc_user_mac_addr, pst_mpdu_num);
}


oal_uint32  oam_report_dft_params_to_sdt(
                                        oal_uint8  *puc_user_mac_addr,
                                        oal_uint8  *puc_param,
                                        oal_uint16  us_param_len,
                                        oam_ota_type_enum_uint8 en_type)
{
    oal_uint32                  ul_tick;
    oal_uint16                  us_ota_data_len;
    oal_uint16                  us_skb_len;
    oal_netbuf_stru            *pst_netbuf;
    oam_ota_stru               *pst_ota_data;
    oal_uint32                  ul_ret     = OAL_SUCC;

    if (OAL_UNLIKELY(OAL_PTR_NULL == g_st_oam_sdt_func_hook.p_sdt_report_data_func))
    {
        return OAL_ERR_CODE_PTR_NULL;
    }

    us_ota_data_len = us_param_len + OAL_SIZEOF(oam_ota_hdr_stru);
    us_skb_len = us_ota_data_len + OAM_RESERVE_SKB_LEN;

    if (us_skb_len > WLAN_SDT_NETBUF_MAX_PAYLOAD)
    {
        return OAL_FAIL;
    }

    pst_netbuf = oal_mem_sdt_netbuf_alloc(us_skb_len, OAL_TRUE);
    if (OAL_PTR_NULL == pst_netbuf)
    {
        return OAL_ERR_CODE_PTR_NULL;
    }
    oal_netbuf_reserve(pst_netbuf, OAM_RESERVE_SKB_HEADROOM_LEN);

    oal_netbuf_put(pst_netbuf, us_ota_data_len);
    pst_ota_data = (oam_ota_stru *)oal_netbuf_data(pst_netbuf);

    /* ��ȡϵͳTICKֵ */
    ul_tick = (oal_uint32)OAL_TIME_GET_STAMP_MS();

    /* ��дota��Ϣͷ�ṹ�� */
    pst_ota_data->st_ota_hdr.ul_tick     = ul_tick;
    pst_ota_data->st_ota_hdr.en_ota_type = en_type;
    pst_ota_data->st_ota_hdr.us_ota_data_len = us_param_len;
    pst_ota_data->st_ota_hdr.uc_frame_hdr_len = 0;
    oal_set_mac_addr(pst_ota_data->st_ota_hdr.auc_user_macaddr, puc_user_mac_addr);
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_HOST)
    pst_ota_data->st_ota_hdr.auc_resv[0]    = OAM_OTA_TYPE_1102_HOST;
#elif (_PRE_PRODUCT_ID ==_PRE_PRODUCT_ID_HI1103_HOST)
    pst_ota_data->st_ota_hdr.auc_resv[0]    = OAM_OTA_TYPE_1103_HOST;
#elif (_PRE_PRODUCT_ID ==_PRE_PRODUCT_ID_HI1102A_HOST)
    pst_ota_data->st_ota_hdr.auc_resv[0]    = OAM_OTA_TYPE_1102A_HOST;
#else
    pst_ota_data->st_ota_hdr.auc_resv[0]    = OAM_OTA_TYPE_1151_HOST;
#endif

    /* ��������,��дota���� */
    oal_memcopy((oal_void *)pst_ota_data->auc_ota_data,
                (const oal_void *)puc_param,
                us_param_len);

    /* �ϱ�SDT */
    ul_ret = oam_report_data2sdt(pst_netbuf, OAM_DATA_TYPE_OTA, OAM_PRIMID_TYPE_OUTPUT_CONTENT);

    return ul_ret;
}



oal_uint32  oam_report_dft_params(oal_uint8  *puc_user_mac_addr,
                                        oal_uint8  *puc_param,
                                        oal_uint16  us_param_len,
                                        oam_ota_type_enum_uint8 en_type)
{
    if (OAL_UNLIKELY(OAL_PTR_NULL == puc_user_mac_addr || OAL_PTR_NULL == puc_param))
    {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (en_type >= OAM_OTA_TYPE_BUTT)
    {
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    if (0 != us_param_len)
    {
        return oam_report_dft_params_to_sdt(puc_user_mac_addr, puc_param, us_param_len, en_type);
    }

    return OAL_ERR_CODE_INVALID_CONFIG;
}




oal_uint32  oam_report_set_all_switch(oal_switch_enum_uint8 en_switch)
{
    oal_uint8   uc_vapid_loop;

    if (OAL_SWITCH_OFF == en_switch)
    {
        OAL_MEMZERO(&g_st_oam_mng_ctx.st_user_track_ctx, OAL_SIZEOF(oam_user_track_ctx_stru));

        for (uc_vapid_loop = 0; uc_vapid_loop < WLAN_VAP_SUPPORT_MAX_NUM_LIMIT; uc_vapid_loop++)
        {
            /*beacon֡����*/
            oam_ota_set_beacon_switch(uc_vapid_loop, OAL_SWITCH_OFF);
            /* rx���������� */
            oam_ota_set_rx_dscr_switch(uc_vapid_loop, OAL_SWITCH_OFF);
        }
    }
    else
    {
        oal_memset(&g_st_oam_mng_ctx.st_user_track_ctx, OAL_SWITCH_ON, OAL_SIZEOF(oam_user_track_ctx_stru));

        for (uc_vapid_loop = 0; uc_vapid_loop < WLAN_VAP_SUPPORT_MAX_NUM_LIMIT; uc_vapid_loop++)
        {
            /*beacon֡����*/
            oam_ota_set_beacon_switch(uc_vapid_loop, OAL_SWITCH_ON);
            /* rx���������� */
            oam_ota_set_rx_dscr_switch(uc_vapid_loop, OAL_SWITCH_ON);
        }
    }

    return OAL_SUCC;
}


oal_void oam_report_backtrace(oal_void)
{
    oal_uint8  *puc_buff = (oal_uint8 *)OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL, OAM_REPORT_MAX_STRING_LEN, OAL_TRUE);

    if (OAL_PTR_NULL == puc_buff)
    {
        OAM_ERROR_LOG0(0, OAM_SF_CFG, "{oam_report_backtrace::pc_print_buff null.}");
        return;
    }

    if (oal_dump_stack_str(puc_buff, OAM_REPORT_MAX_STRING_LEN) > 0)
    {
        oam_print((oal_int8*)puc_buff);
    }
    else
    {
        OAM_WARNING_LOG0(0, OAM_SF_CFG, "{oam_report_backtrace::dump stack str failed.}");
    }

    OAL_MEM_FREE(puc_buff, OAL_TRUE);
}

/*lint -e19*/
oal_module_symbol(oam_report_backtrace);
oal_module_symbol(oam_event_set_switch);
oal_module_symbol(oam_event_get_switch);
oal_module_symbol(oam_event_report);
oal_module_symbol(oam_ota_report);
oal_module_symbol(oam_event_set_specific_type_switch);
oal_module_symbol(oam_ota_set_beacon_switch);
oal_module_symbol(oam_ota_set_rx_dscr_switch);
oal_module_symbol(oam_ota_report_to_std);
oal_module_symbol(oam_report_dscr);
oal_module_symbol(oam_report_beacon);
oal_module_symbol(oam_report_eth_frame);
oal_module_symbol(oam_report_80211_frame);
oal_module_symbol(oam_ota_get_beacon_switch);
oal_module_symbol(oam_ota_get_rx_dscr_switch);
oal_module_symbol(oam_report_eth_frame_set_switch);
oal_module_symbol(oam_report_eth_frame_get_switch);
oal_module_symbol(oam_report_80211_mcast_set_switch);
oal_module_symbol(oam_report_80211_mcast_get_switch);
oal_module_symbol(oam_report_80211_ucast_set_switch);
oal_module_symbol(oam_report_80211_ucast_get_switch);
oal_module_symbol(oam_report_80211_probe_set_switch);
oal_module_symbol(oam_report_80211_probe_get_switch);
oal_module_symbol(oam_report_netbuf_cb);
#if 0
oal_module_symbol(oam_report_get_switch);
#endif
#if 0
oal_module_symbol(oam_report_timer_track);
#endif
oal_module_symbol(oam_report_mpdu_num);
oal_module_symbol(oam_report_set_all_switch);
oal_module_symbol(oam_report_dhcp_arp_get_switch);
oal_module_symbol(oam_report_dhcp_arp_set_switch);
oal_module_symbol(oam_report_dft_params);
oal_module_symbol(oam_report_data_get_global_switch);






















#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

