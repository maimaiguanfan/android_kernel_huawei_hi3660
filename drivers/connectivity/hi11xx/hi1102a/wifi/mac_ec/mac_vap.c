


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 ͷ�ļ�����
*****************************************************************************/
#include "oal_mem.h"
#include "wlan_spec.h"
#include "wlan_types.h"
#include "mac_vap.h"
#include "mac_device.h"
#include "mac_resource.h"
#include "mac_regdomain.h"

#include "dmac_ext_if.h"
#include "oal_main.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_MAC_VAP_EC_C

/*****************************************************************************
  2 ȫ�ֱ�������
*****************************************************************************/
#ifdef _PRE_WLAN_FEATURE_UAPSD
#if ((_PRE_OS_VERSION_WIN32 == _PRE_OS_VERSION)||(_PRE_OS_VERSION_WIN32_RAW == _PRE_OS_VERSION)) && (_PRE_TEST_MODE == _PRE_TEST_MODE_UT)
oal_uint8 g_uc_uapsd_cap = WLAN_FEATURE_UAPSD_IS_OPEN;
#else
oal_uint8 g_uc_uapsd_cap = OAL_FALSE;
#endif
#endif
#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
mac_tx_large_amsdu_ampdu_stru g_st_tx_large_amsdu     =  {1,1,1,1,1};
#endif

mac_tcp_ack_buf_switch_stru   g_st_tcp_ack_buf_switch =  {0};

/* WME��ʼ�������壬����OFDM��ʼ�� APģʽ ֵ������TGn 9 Appendix D: Default WMM AC Parameters */

mac_wme_param_stru g_ast_wmm_initial_params_ap[WLAN_WME_AC_BUTT] =
{
    /* BE */
    {
        /* AIFS, cwmin, cwmax, txop */
        3,          4,     6,     0,
    },

    /* BK */
    {
        /* AIFS, cwmin, cwmax, txop */
        7,          4,     10,    0,
    },

    /* VI */
    {
        /* AIFS, cwmin, cwmax, txop */
        1,          3,     4,     3008,
    },

    /* VO */
    {
        /* AIFS, cwmin, cwmax, txop */
        1,          2,     3,     1504,
    },
};

/* WMM��ʼ�������壬����OFDM��ʼ�� STAģʽ */
mac_wme_param_stru g_ast_wmm_initial_params_sta[WLAN_WME_AC_BUTT] =
{
    /* BE */
    {
        /* AIFS, cwmin, cwmax, txop */
        3,          3,     10,     0,
    },

    /* BK */
    {
        /* AIFS, cwmin, cwmax, txop */
        7,          4,     10,     0,
    },


    /* VI */
    {
        /* AIFS, cwmin, cwmax, txop */
        2,          3,     4,     3008,
    },

    /* VO */
    {
        /* AIFS, cwmin, cwmax, txop */
        2,          2,     3,     1504,
    },
};

/* WMM��ʼ�������壬aput������bss��STA��ʹ�õ�EDCA���� */
mac_wme_param_stru g_ast_wmm_initial_params_bss[WLAN_WME_AC_BUTT] =
{
    /* BE */
    {
        /* AIFS, cwmin, cwmax, txop */
        3,          4,     10,     0,
    },

    /* BK */
    {
        /* AIFS, cwmin, cwmax, txop */
        7,          4,     10,     0,
    },


    /* VI */
    {
        /* AIFS, cwmin, cwmax, txop */
        2,          3,     4,     3008,
    },

    /* VO */
    {
        /* AIFS, cwmin, cwmax, txop */
        2,          2,     3,     1504,
    },
};

#ifdef _PRE_WLAN_FEATURE_EDCA_MULTI_USER_MULTI_AC
/* ���û������ȼ�ʹ�õ�EDCA���� */
mac_wme_param_stru g_ast_wmm_multi_user_multi_ac_params_ap[WLAN_WME_AC_BUTT] =
{
    /* BE */
    {
        /* AIFS, cwmin, cwmax, txop */
        3,          5,     10,     0,
    },

    /* BK */
    {
        /* AIFS, cwmin, cwmax, txop */
        3,          5,     10,     0,
    },


    /* VI */
    {
        /* AIFS, cwmin, cwmax, txop */
        3,          5,     10,     0,
    },

    /* VO */
    {
        /* AIFS, cwmin, cwmax, txop */
        3,          5,     10,     0,
    },
};
#endif

#ifdef WIN32
mac_vap_rom_cb_stru g_st_mac_vap_rom_cb =
{
    mac_vap_init_cb,
    mac_vap_init_privacy_cb,
    mac_vap_set_beacon_cb,
    mac_vap_del_user_cb
};
#else
mac_vap_rom_cb_stru g_st_mac_vap_rom_cb =
{
    .p_mac_vap_init                     = mac_vap_init_cb,
    .p_mac_vap_init_privacy             = mac_vap_init_privacy_cb,
    .p_mac_vap_set_beacon               = mac_vap_set_beacon_cb,
    .p_mac_vap_del_user                 = mac_vap_del_user_cb
};
#endif

oal_void mac_vap_init_11ac_rates(mac_vap_stru *pst_mac_vap,mac_device_stru *pst_mac_dev);

/*****************************************************************************
  3 ����ʵ��
*****************************************************************************/


mac_wme_param_stru  *mac_get_wmm_cfg(wlan_vap_mode_enum_uint8 en_vap_mode)
{
    /* �ο���֤�����ã�û�а���Э�����ã�WLAN_VAP_MODE_BUTT��ʾ��ap�㲥��sta��edca���� */
    if (WLAN_VAP_MODE_BUTT == en_vap_mode)
    {
        return (mac_wme_param_stru  *)g_ast_wmm_initial_params_bss;
    }
    else if(WLAN_VAP_MODE_BSS_AP == en_vap_mode)
    {
       return (mac_wme_param_stru  *)g_ast_wmm_initial_params_ap;
    }
    return (mac_wme_param_stru  *)g_ast_wmm_initial_params_sta;

}

#ifdef _PRE_WLAN_FEATURE_EDCA_MULTI_USER_MULTI_AC

mac_wme_param_stru  *mac_get_wmm_cfg_multi_user_multi_ac(oal_traffic_type_enum_uint8 uc_traffic_type)
{
    /* ���û���ҵ�����Ͳ����²������������apģʽ�µ�Ĭ��ֵ */
    if (OAL_TRAFFIC_MULTI_USER_MULTI_AC == uc_traffic_type)
    {
        return (mac_wme_param_stru  *)g_ast_wmm_multi_user_multi_ac_params_ap;
    }

    return (mac_wme_param_stru  *)g_ast_wmm_initial_params_ap;

}
#endif


oal_uint32  mac_mib_set_station_id(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_cfg_staion_id_param_stru   *pst_param;

    pst_param = (mac_cfg_staion_id_param_stru *)puc_param;

    oal_set_mac_addr(pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.auc_dot11StationID,pst_param->auc_station_id);

    return OAL_SUCC;
}
#if 0

oal_uint32  mac_mib_get_station_id(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_len, oal_uint8 *puc_param)
{
    mac_cfg_staion_id_param_stru   *pst_param;

    pst_param = (mac_cfg_staion_id_param_stru *)puc_param;

    oal_memcopy(pst_param->auc_station_id,
                pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.auc_dot11StationID,
                WLAN_MAC_ADDR_LEN);

    *puc_len = OAL_SIZEOF(mac_cfg_staion_id_param_stru);

    return OAL_SUCC;
}
#endif

oal_uint32  mac_mib_set_bss_type(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    oal_int32       l_value;

    l_value = *((oal_int32 *)puc_param);

    pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11DesiredBSSType = (oal_uint8)l_value;

    return OAL_SUCC;
}


oal_uint32  mac_mib_get_bss_type(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_len, oal_uint8 *puc_param)
{
    *((oal_int32 *)puc_param) = pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11DesiredBSSType;
    *puc_len = OAL_SIZEOF(oal_int32);

    return OAL_SUCC;
}


oal_uint32  mac_mib_set_ssid(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    mac_cfg_ssid_param_stru    *pst_param;
    oal_uint8                   uc_ssid_len;
    oal_uint8                  *puc_mib_ssid;

    pst_param   = (mac_cfg_ssid_param_stru *)puc_param;
    uc_ssid_len = pst_param->uc_ssid_len;       /* ���Ȳ������ַ�����β'\0' */

    if (uc_ssid_len > WLAN_SSID_MAX_LEN - 1)
    {
        uc_ssid_len = WLAN_SSID_MAX_LEN - 1;
    }

    puc_mib_ssid = pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.auc_dot11DesiredSSID;

    oal_memcopy(puc_mib_ssid, pst_param->ac_ssid, uc_ssid_len);
    puc_mib_ssid[uc_ssid_len] = '\0';

    return OAL_SUCC;
}


oal_uint32  mac_mib_get_ssid(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_len, oal_uint8 *puc_param)
{
    mac_cfg_ssid_param_stru *pst_param;
    oal_uint8                uc_ssid_len;
    oal_uint8               *puc_mib_ssid;

    puc_mib_ssid = pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.auc_dot11DesiredSSID;
    uc_ssid_len  = (oal_uint8)OAL_STRLEN((oal_int8 *)puc_mib_ssid);

    pst_param = (mac_cfg_ssid_param_stru *)puc_param;

    pst_param->uc_ssid_len = uc_ssid_len;
    oal_memcopy(pst_param->ac_ssid, puc_mib_ssid, uc_ssid_len);

    *puc_len = OAL_SIZEOF(mac_cfg_ssid_param_stru);

    return OAL_SUCC;
}


oal_uint32  mac_mib_set_beacon_period(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    oal_uint32       ul_value;

    ul_value     = *((oal_uint32 *)puc_param);

    pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.ul_dot11BeaconPeriod = (oal_uint32)ul_value;

    return OAL_SUCC;
}


oal_uint32  mac_mib_get_beacon_period(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_len, oal_uint8 *puc_param)
{
    *((oal_uint32 *)puc_param) = pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.ul_dot11BeaconPeriod;

    *puc_len = OAL_SIZEOF(oal_uint32);

    return OAL_SUCC;
}



oal_uint32  mac_mib_set_dtim_period(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    oal_int32       l_value;

    l_value     = *((oal_int32 *)puc_param);

    pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.ul_dot11DTIMPeriod = (oal_uint32)l_value;

    return OAL_SUCC;
}


oal_uint32  mac_mib_get_dtim_period(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_len, oal_uint8 *puc_param)
{
    *((oal_uint32 *)puc_param) = pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.ul_dot11DTIMPeriod;

    *puc_len = OAL_SIZEOF(oal_uint32);

    return OAL_SUCC;
}


oal_uint32  mac_mib_set_shpreamble(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_param)
{
    oal_int32       l_value;

    l_value     = *((oal_int32 *)puc_param);

    if (0 != l_value)
    {
        mac_mib_set_ShortPreambleOptionImplemented(pst_mac_vap, OAL_TRUE);
    }
    else
    {
        mac_mib_set_ShortPreambleOptionImplemented(pst_mac_vap, OAL_FALSE);
    }

    return OAL_SUCC;
}


oal_uint32  mac_mib_get_shpreamble(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_len, oal_uint8 *puc_param)
{
    oal_int32       l_value;

    l_value = mac_mib_get_ShortPreambleOptionImplemented(pst_mac_vap);

    *((oal_int32 *)puc_param) = l_value;

    *puc_len = OAL_SIZEOF(l_value);

    return OAL_SUCC;

}
#if 0

oal_uint32  mac_mib_get_GroupReceivedFrameCount(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_len, oal_uint8 *puc_group_count)
{
    *((oal_uint32*)puc_group_count) = pst_mac_vap->pst_mib_info->st_wlan_mib_counters.ul_dot11GroupReceivedFrameCount;

    *puc_len = OAL_SIZEOF(oal_uint32);

    return OAL_SUCC;
}


oal_uint32  mac_mib_set_GroupReceivedFrameCount(mac_vap_stru *pst_mac_vap, oal_uint8 uc_len, oal_uint8 *puc_group_count)
{
    oal_uint32      ul_value;

    ul_value = *((oal_uint32 *)puc_group_count);

    pst_mac_vap->pst_mib_info->st_wlan_mib_counters.ul_dot11GroupReceivedFrameCount = ul_value;

    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_FEATURE_SMPS

wlan_mib_mimo_power_save_enum_uint8 mac_mib_get_smps(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.en_dot11MIMOPowerSave;
}


oal_uint32 mac_vap_get_smps_mode(mac_vap_stru *pst_mac_vap, oal_uint16 *pus_len, oal_uint8 *puc_param)
{
    if (OAL_TRUE != pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11HighThroughputOptionImplemented)
    {
        /* ��HT�û�����֧��SMPS ���طǷ���ϢWLAN_MIB_MIMO_POWER_SAVE_BUTT */
        *puc_param = WLAN_MIB_MIMO_POWER_SAVE_BUTT;
        *pus_len   = OAL_SIZEOF(oal_uint8);
        return OAL_SUCC;
    }

    *puc_param = pst_mac_vap->pst_mib_info->st_wlan_mib_ht_sta_cfg.en_dot11MIMOPowerSave;

    *pus_len   = OAL_SIZEOF(oal_uint8);

    return OAL_SUCC;
}


oal_uint32 mac_vap_get_smps_en(mac_vap_stru *pst_mac_vap, oal_uint16 *pus_len, oal_uint8 *puc_param)
{
    mac_device_stru   *pst_mac_device;

    /* ��ȡdevice */
    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);

    if (OAL_PTR_NULL == pst_mac_device)
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_SMPS, "{mac_vap_get_smps_en::pst_mac_device[%d] null.}", pst_mac_vap->uc_device_id);

        return OAL_ERR_CODE_PTR_NULL;
    }

    *puc_param = pst_mac_device->en_smps;

    *pus_len   = OAL_SIZEOF(oal_uint8);

    return OAL_SUCC;
}


oal_void mac_vap_set_smps(mac_vap_stru *pst_vap, oal_uint8 uc_value)
{
    pst_vap->st_cap_flag.bit_smps = uc_value;
}
#endif

#ifdef _PRE_WLAN_FEATURE_UAPSD


oal_uint32  mac_vap_set_uapsd_en(mac_vap_stru *pst_mac_vap, oal_uint8 uc_value)
{
    pst_mac_vap->st_cap_flag.bit_uapsd = (OAL_TRUE == uc_value)?1:0;

    return OAL_SUCC;
}


oal_uint8  mac_vap_get_uapsd_en(mac_vap_stru *pst_mac_vap)
{
// *puc_param  = pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11APSDOptionImplemented;

    return pst_mac_vap->st_cap_flag.bit_uapsd;
}

#endif



oal_uint32 mac_vap_user_exist(oal_dlist_head_stru *pst_new, oal_dlist_head_stru *pst_head)
{
    oal_dlist_head_stru      *pst_user_list_head;
    oal_dlist_head_stru      *pst_member_entry;


    OAL_DLIST_SEARCH_FOR_EACH_SAFE(pst_member_entry,pst_user_list_head,pst_head)
    {
        if(pst_new == pst_member_entry)
        {
            OAM_ERROR_LOG0(0, OAM_SF_ASSOC, "{oal_dlist_check_head:dmac user doule add.}");
            return OAL_SUCC;
        }
    }

    return OAL_FAIL;
}


oal_uint32  mac_vap_add_assoc_user(mac_vap_stru *pst_vap, oal_uint16 us_user_idx)
{
    mac_user_stru              *pst_user;
    oal_dlist_head_stru        *pst_dlist_head;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_vap))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ASSOC, "{mac_vap_add_assoc_user::pst_vap null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_user = (mac_user_stru *)mac_res_get_mac_user(us_user_idx);

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_user))
    {
        OAM_ERROR_LOG1(pst_vap->uc_vap_id, OAM_SF_ASSOC, "{mac_vap_add_assoc_user::pst_user[%d] null.}", us_user_idx);

        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_user->us_user_hash_idx = MAC_CALCULATE_HASH_VALUE(pst_user->auc_user_mac_addr);

    if(OAL_SUCC == mac_vap_user_exist(&(pst_user->st_user_dlist), &(pst_vap->st_mac_user_list_head)))
    {
        OAM_ERROR_LOG1(pst_vap->uc_vap_id, OAM_SF_ASSOC, "{mac_vap_add_assoc_user::user[%d] already exist.}", us_user_idx);
        return OAL_ERR_CODE_PTR_NULL;
    }
    oal_spin_lock_bh(&pst_vap->st_cache_user_lock);

    pst_dlist_head = &(pst_vap->ast_user_hash[pst_user->us_user_hash_idx]);
#ifdef _PRE_WLAN_DFT_STAT
    (pst_vap->ul_hash_cnt)++;
#endif
    /* ����˫��hash�����ͷ */
    oal_dlist_add_head(&(pst_user->st_user_hash_dlist), pst_dlist_head);

    /* ����˫�������ͷ */
    pst_dlist_head = &(pst_vap->st_mac_user_list_head);
    oal_dlist_add_head(&(pst_user->st_user_dlist), pst_dlist_head);
#ifdef _PRE_WLAN_DFT_STAT
    (pst_vap->ul_dlist_cnt)++;
#endif

    /* ����cache user */
    oal_set_mac_addr(pst_vap->auc_cache_user_mac_addr, pst_user->auc_user_mac_addr);
    pst_vap->us_cache_user_id = us_user_idx;


    /* ��¼STAģʽ�µ���֮������VAP��id */
    if (WLAN_VAP_MODE_BSS_STA == pst_vap->en_vap_mode)
    {
        mac_vap_set_assoc_id(pst_vap, (oal_uint8)us_user_idx);
    }

    /* vap�ѹ��� user����++ */
    pst_vap->us_user_nums++;
    oal_spin_unlock_bh(&pst_vap->st_cache_user_lock);

    return OAL_SUCC;
}



oal_uint32  mac_vap_del_user(mac_vap_stru *pst_vap, oal_uint16 us_user_idx)
{
    mac_user_stru          *pst_user;
    mac_user_stru          *pst_user_temp;
    oal_dlist_head_stru    *pst_hash_head;
    oal_dlist_head_stru    *pst_entry;
    oal_dlist_head_stru    *pst_dlist_tmp       = OAL_PTR_NULL;
    oal_uint32              ul_ret              = OAL_FAIL;
    oal_uint8               uc_txop_ps_user_cnt = 0;


    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_vap))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ASSOC, "{mac_vap_del_user::pst_vap null,us_user_idx is %d}", us_user_idx);

        return OAL_ERR_CODE_PTR_NULL;
    }


    oal_spin_lock_bh(&pst_vap->st_cache_user_lock);

    /* ��cache user id�Ա� , ��������cache user*/
    if (us_user_idx == pst_vap->us_cache_user_id)
    {
        oal_set_mac_addr_zero(pst_vap->auc_cache_user_mac_addr);
        pst_vap->us_cache_user_id = MAC_INVALID_USER_ID;
    }

    if (OAL_RETURN == g_st_mac_vap_rom_cb.p_mac_vap_del_user(pst_vap,us_user_idx,&ul_ret))
    {
        oal_spin_unlock_bh(&pst_vap->st_cache_user_lock);
        return ul_ret;
    }


    pst_user = (mac_user_stru *)mac_res_get_mac_user(us_user_idx);

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_user))
    {
        oal_spin_unlock_bh(&pst_vap->st_cache_user_lock);
        OAM_ERROR_LOG1(pst_vap->uc_vap_id, OAM_SF_ASSOC, "{mac_vap_del_user::pst_user null,us_user_idx is %d}", us_user_idx);
        return OAL_ERR_CODE_PTR_NULL;
    }

    mac_user_set_asoc_state(pst_user, MAC_USER_STATE_BUTT);

    if(pst_user->us_user_hash_idx >= MAC_VAP_USER_HASH_MAX_VALUE)
    {
        oal_spin_unlock_bh(&pst_vap->st_cache_user_lock);
        /*ADD USER���ʧ�������ظ�ɾ��User�����ܽ���˷�֧��*/
        OAM_ERROR_LOG1(pst_vap->uc_vap_id, OAM_SF_ASSOC, "{mac_vap_del_user::hash idx invaild %u}", pst_user->us_user_hash_idx);
        return OAL_FAIL;
    }

    pst_hash_head = &(pst_vap->ast_user_hash[pst_user->us_user_hash_idx]);

    OAL_DLIST_SEARCH_FOR_EACH_SAFE(pst_entry, pst_dlist_tmp, pst_hash_head)
    {
        pst_user_temp = (mac_user_stru *)OAL_DLIST_GET_ENTRY(pst_entry, mac_user_stru, st_user_hash_dlist);

        if (OAL_PTR_NULL == pst_user_temp)/*lint !e774*/
        {
            OAM_ERROR_LOG1(pst_vap->uc_vap_id, OAM_SF_ASSOC, "{mac_vap_del_user::pst_user_temp null,us_user_idx is %d}", us_user_idx);

            continue;
        }
        if (pst_user_temp->st_vht_hdl.bit_vht_txop_ps)
        {
            uc_txop_ps_user_cnt++;
        }

        if (!oal_compare_mac_addr(pst_user->auc_user_mac_addr, pst_user_temp->auc_user_mac_addr))
        {
            oal_dlist_delete_entry(pst_entry);

            /* ��˫�������в�� */
            oal_dlist_delete_entry(&(pst_user->st_user_dlist));

            oal_dlist_delete_entry(&(pst_user->st_user_hash_dlist));
            ul_ret = OAL_SUCC;

#ifdef _PRE_WLAN_DFT_STAT
            (pst_vap->ul_hash_cnt)--;
            (pst_vap->ul_dlist_cnt)--;
#endif
            /* ��ʼ����Ӧ��Ա */
            pst_user->us_user_hash_idx = 0xffff;
            pst_user->us_assoc_id      = us_user_idx;
            pst_user->en_is_multi_user = OAL_FALSE;
            OAL_MEMZERO(pst_user->auc_user_mac_addr, WLAN_MAC_ADDR_LEN);
            pst_user->uc_vap_id        = 0xff;
            pst_user->uc_device_id     = 0xff;
            pst_user->uc_chip_id       = 0xff;
            pst_user->en_user_asoc_state = MAC_USER_STATE_BUTT;
        }
    }

    if (0 == uc_txop_ps_user_cnt)
    {
        pst_vap->st_cap_flag.bit_txop_ps = OAL_FALSE;
    }

    if (OAL_SUCC == ul_ret)
    {
        /* vap�ѹ��� user����-- */
        if (pst_vap->us_user_nums)
        {
            pst_vap->us_user_nums--;
        }
        /* STAģʽ�½�������VAP��id��Ϊ�Ƿ�ֵ */
        if (WLAN_VAP_MODE_BSS_STA == pst_vap->en_vap_mode)
        {
            mac_vap_set_assoc_id(pst_vap, 0xff);
        }
        oal_spin_unlock_bh(&pst_vap->st_cache_user_lock);
        return OAL_SUCC;
    }
    oal_spin_unlock_bh(&pst_vap->st_cache_user_lock);
    OAM_WARNING_LOG1(pst_vap->uc_vap_id, OAM_SF_ASSOC, "{mac_vap_del_user::delete user failed,user idx is %d.}", us_user_idx);

    return OAL_FAIL;
}


oal_uint32  mac_vap_find_user_by_macaddr( mac_vap_stru *pst_vap, oal_uint8 *puc_sta_mac_addr, oal_uint16 *pus_user_idx)
{
    mac_user_stru              *pst_mac_user;
    oal_uint32                  ul_user_hash_value;
    oal_dlist_head_stru        *pst_entry;

    if (OAL_UNLIKELY((OAL_PTR_NULL == pst_vap)
                  || (OAL_PTR_NULL == puc_sta_mac_addr)
                  || ((OAL_PTR_NULL == pus_user_idx))))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{mac_vap_find_user_by_macaddr::param null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    if (WLAN_VAP_MODE_BSS_STA == pst_vap->en_vap_mode)
    {
        pst_mac_user = (mac_user_stru *)mac_res_get_mac_user(pst_vap->uc_assoc_vap_id);
        if (OAL_PTR_NULL == pst_mac_user)
        {
            return OAL_FAIL;
        }

        if (!oal_compare_mac_addr(pst_mac_user->auc_user_mac_addr, puc_sta_mac_addr))
        {
            *pus_user_idx = pst_vap->uc_assoc_vap_id;
            return (*pus_user_idx != (oal_uint16)MAC_INVALID_USER_ID) ? OAL_SUCC : OAL_FAIL;/* [false alarm]:����ֵΪ����ֵ0����1����Ӱ��*/
        }
        return OAL_FAIL;
    }

    oal_spin_lock_bh(&pst_vap->st_cache_user_lock);
    /* ��cache user�Ա� , �����ֱ�ӷ���cache user id*/
    if (!oal_compare_mac_addr(pst_vap->auc_cache_user_mac_addr, puc_sta_mac_addr))
    {
        /* �û�ɾ����user macaddr��cache user macaddr��ַ��Ϊ0����ʵ�����û��Ѿ�ɾ������ʱuser id��Ч */
        *pus_user_idx = pst_vap->us_cache_user_id;
        oal_spin_unlock_bh(&pst_vap->st_cache_user_lock);
        return (*pus_user_idx != (oal_uint16)MAC_INVALID_USER_ID) ? OAL_SUCC : OAL_FAIL;/* [false alarm]:����ֵΪ����ֵ0����1����Ӱ��*/
    }

    ul_user_hash_value = MAC_CALCULATE_HASH_VALUE(puc_sta_mac_addr);

    OAL_DLIST_SEARCH_FOR_EACH(pst_entry, &(pst_vap->ast_user_hash[ul_user_hash_value]))
    {
        pst_mac_user = (mac_user_stru *)OAL_DLIST_GET_ENTRY(pst_entry, mac_user_stru, st_user_hash_dlist);

        if (OAL_PTR_NULL == pst_mac_user)/*lint !e774*/
        {
            OAM_ERROR_LOG0(pst_vap->uc_vap_id, OAM_SF_ANY, "{mac_vap_find_user_by_macaddr::pst_mac_user null.user idx}");
            continue;
        }

        /* ��ͬ��MAC��ַ */
        if (!oal_compare_mac_addr(pst_mac_user->auc_user_mac_addr, puc_sta_mac_addr))
        {
            *pus_user_idx = pst_mac_user->us_assoc_id;
            /*����cache user*/
            oal_set_mac_addr(pst_vap->auc_cache_user_mac_addr, pst_mac_user->auc_user_mac_addr);
            pst_vap->us_cache_user_id = pst_mac_user->us_assoc_id;
            oal_spin_unlock_bh(&pst_vap->st_cache_user_lock);
            return (*pus_user_idx != (oal_uint16)MAC_INVALID_USER_ID) ? OAL_SUCC : OAL_FAIL;/* [false alarm]:����ֵΪ����ֵ0����1����Ӱ��*/
        }
    }
    oal_spin_unlock_bh(&pst_vap->st_cache_user_lock);
    return OAL_FAIL;
}


oal_uint32  mac_device_find_user_by_macaddr(
                mac_vap_stru        *pst_vap,
                oal_uint8           *puc_sta_mac_addr,
                oal_uint16          *pus_user_idx)
{
    mac_device_stru            *pst_device;
    mac_vap_stru               *pst_mac_vap;
    oal_uint8                   uc_vap_id;
    oal_uint8                   uc_vap_idx;
    oal_uint32                  ul_ret;

    /* ��ȡdevice */
    pst_device = mac_res_get_dev(pst_vap->uc_device_id);
    if (OAL_PTR_NULL == pst_device)
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "mac_res_get_dev[%d] return null ", pst_vap->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* ��device�µ�����vap���б��� */
    for (uc_vap_idx = 0; uc_vap_idx < pst_device->uc_vap_num; uc_vap_idx++)
    {
        uc_vap_id = pst_device->auc_vap_id[uc_vap_idx];

        /* ����vap����Ҫ���� */
        if (uc_vap_id == pst_device->uc_cfg_vap_id)
        {
            continue;
        }

        /* ��vap����Ҫ���� */
        if (uc_vap_id == pst_vap->uc_vap_id)
        {
            continue;
        }

        pst_mac_vap = (mac_vap_stru *)mac_res_get_mac_vap(uc_vap_id);
        if (OAL_PTR_NULL == pst_mac_vap)
        {
            continue;
        }

        /* ֻ����APģʽ */
        if (WLAN_VAP_MODE_BSS_AP != pst_mac_vap->en_vap_mode)
        {
            continue;
        }

        ul_ret = mac_vap_find_user_by_macaddr(pst_mac_vap, puc_sta_mac_addr, pus_user_idx);
        if (OAL_SUCC == ul_ret)
        {
            return OAL_SUCC;
        }
    }

    return OAL_FAIL;
}
#if 0

oal_uint32  mac_vap_update_user(mac_vap_stru *pst_vap, oal_uint32   ul_user_dix)
{
    /* ��OFFLOAD�����²���Ҫʵ�� */

    return OAL_SUCC;
}
#endif

oal_uint32  mac_vap_init_wme_param(mac_vap_stru *pst_mac_vap)
{
    OAL_CONST mac_wme_param_stru   *pst_wmm_param;
    OAL_CONST mac_wme_param_stru   *pst_wmm_param_sta;
    oal_uint8                       uc_ac_type;

    pst_wmm_param = mac_get_wmm_cfg(pst_mac_vap->en_vap_mode);
    if(OAL_PTR_NULL == pst_wmm_param)
    {
        return OAL_ERR_CODE_PTR_NULL;
    }

    for (uc_ac_type = 0; uc_ac_type < WLAN_WME_AC_BUTT; uc_ac_type++)
    {
        /* VAP�����EDCA���� */
        pst_mac_vap->pst_mib_info->st_wlan_mib_qap_edac[uc_ac_type].ul_dot11QAPEDCATableIndex   = uc_ac_type + 1;
        pst_mac_vap->pst_mib_info->st_wlan_mib_qap_edac[uc_ac_type].ul_dot11QAPEDCATableAIFSN   = pst_wmm_param[uc_ac_type].ul_aifsn;
        pst_mac_vap->pst_mib_info->st_wlan_mib_qap_edac[uc_ac_type].ul_dot11QAPEDCATableCWmin   = pst_wmm_param[uc_ac_type].ul_logcwmin;
        pst_mac_vap->pst_mib_info->st_wlan_mib_qap_edac[uc_ac_type].ul_dot11QAPEDCATableCWmax   = pst_wmm_param[uc_ac_type].ul_logcwmax;
        pst_mac_vap->pst_mib_info->st_wlan_mib_qap_edac[uc_ac_type].ul_dot11QAPEDCATableTXOPLimit = pst_wmm_param[uc_ac_type].ul_txop_limit;
    }

    if (WLAN_VAP_MODE_BSS_AP == pst_mac_vap->en_vap_mode)
    {
        /* APģʽʱ�㲥��STA��EDCA������ֻ��APģʽ��Ҫ��ʼ����ֵ��ʹ��WLAN_VAP_MODE_BUTT�� */
        pst_wmm_param_sta = mac_get_wmm_cfg(WLAN_VAP_MODE_BUTT);

        for (uc_ac_type = 0; uc_ac_type < WLAN_WME_AC_BUTT; uc_ac_type++)
        {
            pst_mac_vap->pst_mib_info->ast_wlan_mib_edca[uc_ac_type].ul_dot11EDCATableIndex     = uc_ac_type + 1;  /* ע: Э��涨ȡֵ1 2 3 4 */
            pst_mac_vap->pst_mib_info->ast_wlan_mib_edca[uc_ac_type].ul_dot11EDCATableAIFSN     = pst_wmm_param_sta[uc_ac_type].ul_aifsn;
            pst_mac_vap->pst_mib_info->ast_wlan_mib_edca[uc_ac_type].ul_dot11EDCATableCWmin     = pst_wmm_param_sta[uc_ac_type].ul_logcwmin;
            pst_mac_vap->pst_mib_info->ast_wlan_mib_edca[uc_ac_type].ul_dot11EDCATableCWmax     = pst_wmm_param_sta[uc_ac_type].ul_logcwmax;
            pst_mac_vap->pst_mib_info->ast_wlan_mib_edca[uc_ac_type].ul_dot11EDCATableTXOPLimit = pst_wmm_param_sta[uc_ac_type].ul_txop_limit;
        }
    }

    return OAL_SUCC;
}


oal_void mibset_RSNAClearWpaPairwiseCipherImplemented(mac_vap_stru *pst_vap)
{
    oal_uint8 uc_index;

    for (uc_index = 0; uc_index < WLAN_PAIRWISE_CIPHER_SUITES; uc_index++)
    {
        pst_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.auc_wpa_pair_suites[uc_index] = 0;
    }
}


oal_void mibset_RSNAClearWpa2PairwiseCipherImplemented(mac_vap_stru *pst_vap)
{
    oal_uint8 uc_index;

    for (uc_index = 0; uc_index < WLAN_PAIRWISE_CIPHER_SUITES; uc_index++)
    {
        pst_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.auc_rsn_pair_suites[uc_index] = 0;
    }
}


oal_void  mac_vap_init_mib_11n(mac_vap_stru  *pst_mac_vap)
{
    wlan_mib_ieee802dot11_stru    *pst_mib_info;
    mac_device_stru               *pst_dev;

    pst_dev = mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (OAL_PTR_NULL == pst_dev)
    {
        MAC_WARNING_LOG(pst_mac_vap->uc_vap_id, "mac_vap_init_mib_11n: pst_dev is null ptr");
        OAM_WARNING_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{mac_vap_init_mib_11n::pst_dev null.}");

        return;
    }

    pst_mib_info = pst_mac_vap->pst_mib_info;

    pst_mib_info->st_wlan_mib_sta_config.en_dot11HighThroughputOptionImplemented = OAL_FALSE;

    pst_mib_info->st_phy_ht.en_dot11LDPCCodingOptionImplemented         = OAL_TRUE;
    pst_mib_info->st_phy_ht.en_dot11LDPCCodingOptionActivated           = OAL_TRUE;  /* Ĭ��ldpc && stbc���ܴ򿪣�����STA��Э��Э�� */
    pst_mib_info->st_phy_ht.en_dot11TxSTBCOptionActivated               = HT_TX_STBC_DEFAULT_VALUE;
    /* pst_mib_info->st_phy_ht.en_dot11FortyMHzOperationImplemented        = OAL_FALSE; */
    pst_mib_info->st_phy_ht.en_dot112GFortyMHzOperationImplemented      = !pst_mac_vap->st_cap_flag.bit_disable_2ght40;
    pst_mib_info->st_phy_ht.en_dot115GFortyMHzOperationImplemented      = OAL_TRUE;

    /* SMPS���Ժ겻����ʱĬ�Ͼ�ΪPOWER_SAVE_MIMO */
    pst_mib_info->st_wlan_mib_ht_sta_cfg.en_dot11MIMOPowerSave          = WLAN_MIB_MIMO_POWER_SAVE_MIMO;

    pst_mib_info->st_phy_ht.en_dot11HTGreenfieldOptionImplemented       = HT_GREEN_FILED_DEFAULT_VALUE;
    pst_mib_info->st_phy_ht.en_dot11ShortGIOptionInTwentyImplemented    = OAL_TRUE;
    pst_mib_info->st_phy_ht.en_dot112GShortGIOptionInFortyImplemented   = !pst_mac_vap->st_cap_flag.bit_disable_2ght40;
    pst_mib_info->st_phy_ht.en_dot115GShortGIOptionInFortyImplemented   = OAL_TRUE;
    pst_mib_info->st_phy_ht.en_dot11TxSTBCOptionImplemented             = pst_dev->bit_tx_stbc;
    pst_mib_info->st_phy_ht.en_dot11RxSTBCOptionImplemented             = (pst_dev->bit_rx_stbc == 0) ? OAL_FALSE : OAL_TRUE;
    pst_mib_info->st_phy_ht.ul_dot11NumberOfSpatialStreamsImplemented   = 2;
    pst_mib_info->st_wlan_mib_ht_sta_cfg.en_dot11MaxAMSDULength         = 0;
#if (defined(_PRE_PRODUCT_ID_HI110X_DEV) || defined(_PRE_PRODUCT_ID_HI110X_HOST))
    pst_mib_info->st_wlan_mib_ht_sta_cfg.en_dot11LsigTxopProtectionOptionImplemented = OAL_FALSE;
#else
    pst_mib_info->st_wlan_mib_ht_sta_cfg.en_dot11LsigTxopProtectionOptionImplemented = OAL_TRUE;
#endif

    pst_mib_info->st_wlan_mib_ht_sta_cfg.ul_dot11MaxRxAMPDUFactor       = 3;
    pst_mib_info->st_wlan_mib_ht_sta_cfg.ul_dot11MinimumMPDUStartSpacing= 5;
    pst_mib_info->st_wlan_mib_ht_sta_cfg.en_dot11PCOOptionImplemented   = OAL_FALSE;
    pst_mib_info->st_wlan_mib_ht_sta_cfg.ul_dot11TransitionTime         = 3;
    pst_mib_info->st_wlan_mib_ht_sta_cfg.en_dot11MCSFeedbackOptionImplemented = OAL_FALSE;
    pst_mib_info->st_wlan_mib_ht_sta_cfg.en_dot11HTControlFieldSupported= OAL_FALSE;
    pst_mib_info->st_wlan_mib_ht_sta_cfg.en_dot11RDResponderOptionImplemented = OAL_FALSE;
#ifdef _PRE_WLAN_FEATURE_TXBF
    /* txbf������Ϣ ע:11n bfee����Ŀǰȫ��0���պ�����ʵ����Ի�Ϊ�豸����,C01���� */
    pst_mib_info->st_wlan_mib_txbf_config.en_dot11TransmitStaggerSoundingOptionImplemented = pst_dev->bit_su_bfmer;
#else
    pst_mib_info->st_wlan_mib_txbf_config.en_dot11TransmitStaggerSoundingOptionImplemented = 0;
#endif
    pst_mib_info->st_wlan_mib_txbf_config.en_dot11ReceiveStaggerSoundingOptionImplemented  = 0;
    pst_mib_info->st_wlan_mib_txbf_config.en_dot11ReceiveNDPOptionImplemented      = OAL_FALSE;
    pst_mib_info->st_wlan_mib_txbf_config.en_dot11TransmitNDPOptionImplemented       = OAL_FALSE;
    pst_mib_info->st_wlan_mib_txbf_config.en_dot11ImplicitTransmitBeamformingOptionImplemented = OAL_FALSE;
    pst_mib_info->st_wlan_mib_txbf_config.uc_dot11CalibrationOptionImplemented      = 0;
    pst_mib_info->st_wlan_mib_txbf_config.en_dot11ExplicitCSITransmitBeamformingOptionImplemented  = OAL_FALSE;
    pst_mib_info->st_wlan_mib_txbf_config.en_dot11ExplicitNonCompressedBeamformingMatrixOptionImplemented = OAL_FALSE;
    pst_mib_info->st_wlan_mib_txbf_config.uc_dot11ExplicitTransmitBeamformingCSIFeedbackOptionImplemented = 0;
    pst_mib_info->st_wlan_mib_txbf_config.uc_dot11ExplicitNonCompressedBeamformingFeedbackOptionImplemented = 0;
    pst_mib_info->st_wlan_mib_txbf_config.uc_dot11ExplicitCompressedBeamformingFeedbackOptionImplemented = 0;
    pst_mib_info->st_wlan_mib_txbf_config.ul_dot11NumberBeamFormingCSISupportAntenna              = 0;
    pst_mib_info->st_wlan_mib_txbf_config.ul_dot11NumberNonCompressedBeamformingMatrixSupportAntenna     = 0;
    pst_mib_info->st_wlan_mib_txbf_config.ul_dot11NumberCompressedBeamformingMatrixSupportAntenna        = 0;

    /* ����ѡ��������Ϣ */
    pst_mib_info->st_wlan_mib_phy_antenna.en_dot11AntennaSelectionOptionImplemented              = 0;
    pst_mib_info->st_wlan_mib_phy_antenna.en_dot11TransmitExplicitCSIFeedbackASOptionImplemented = 0;
    pst_mib_info->st_wlan_mib_phy_antenna.en_dot11TransmitIndicesFeedbackASOptionImplemented     = 0;
    pst_mib_info->st_wlan_mib_phy_antenna.en_dot11ExplicitCSIFeedbackASOptionImplemented         = 0;
    pst_mib_info->st_wlan_mib_phy_antenna.en_dot11TransmitExplicitCSIFeedbackASOptionImplemented = 0;
    pst_mib_info->st_wlan_mib_phy_antenna.en_dot11ReceiveAntennaSelectionOptionImplemented       = 0;
    pst_mib_info->st_wlan_mib_phy_antenna.en_dot11TransmitSoundingPPDUOptionImplemented          = 0;

    /* obss��Ϣ */
    mac_mib_init_obss_scan(pst_mac_vap);

    /*Ĭ��ʹ��2040����*/
     mac_mib_init_2040(pst_mac_vap);
}


oal_void  mac_vap_init_11ac_mcs_singlenss(
                wlan_mib_ieee802dot11_stru          *pst_mib_info,
                wlan_channel_bandwidth_enum_uint8    en_bandwidth)
{
    mac_tx_max_mcs_map_stru         *pst_tx_max_mcs_map;
    mac_rx_max_mcs_map_stru         *pst_rx_max_mcs_map;

    /* ��ȡmibֵָ�� */
    pst_rx_max_mcs_map = (mac_tx_max_mcs_map_stru *)(&(pst_mib_info->st_wlan_mib_vht_sta_config.us_dot11VHTRxMCSMap));
    pst_tx_max_mcs_map = (mac_tx_max_mcs_map_stru *)(&(pst_mib_info->st_wlan_mib_vht_sta_config.us_dot11VHTTxMCSMap));

    /* 20MHz���������£�֧��MCS0-MCS8 */
    if (WLAN_BAND_WIDTH_20M == en_bandwidth)
    {
#ifdef _PRE_WLAN_FEATURE_11AC_20M_MCS9
        pst_rx_max_mcs_map->us_max_mcs_1ss = MAC_MAX_SUP_MCS9_11AC_EACH_NSS;
        pst_tx_max_mcs_map->us_max_mcs_1ss = MAC_MAX_SUP_MCS9_11AC_EACH_NSS;
#else
        pst_rx_max_mcs_map->us_max_mcs_1ss = MAC_MAX_SUP_MCS8_11AC_EACH_NSS;
        pst_tx_max_mcs_map->us_max_mcs_1ss = MAC_MAX_SUP_MCS8_11AC_EACH_NSS;
#endif
        pst_mib_info->st_wlan_mib_vht_sta_config.ul_dot11VHTRxHighestDataRateSupported = MAC_MAX_RATE_SINGLE_NSS_20M_11AC;
        pst_mib_info->st_wlan_mib_vht_sta_config.ul_dot11VHTTxHighestDataRateSupported = MAC_MAX_RATE_SINGLE_NSS_20M_11AC;
    }

    /* 40MHz���������£�֧��MCS0-MCS9 */
    else if ((WLAN_BAND_WIDTH_40MINUS == en_bandwidth) || (WLAN_BAND_WIDTH_40PLUS == en_bandwidth))
    {
        pst_rx_max_mcs_map->us_max_mcs_1ss = MAC_MAX_SUP_MCS9_11AC_EACH_NSS;
        pst_tx_max_mcs_map->us_max_mcs_1ss = MAC_MAX_SUP_MCS9_11AC_EACH_NSS;
        pst_mib_info->st_wlan_mib_vht_sta_config.ul_dot11VHTRxHighestDataRateSupported = MAC_MAX_RATE_SINGLE_NSS_40M_11AC;
        pst_mib_info->st_wlan_mib_vht_sta_config.ul_dot11VHTTxHighestDataRateSupported = MAC_MAX_RATE_SINGLE_NSS_40M_11AC;
    }

    /* 80MHz���������£�֧��MCS0-MCS9 */
    else if ((WLAN_BAND_WIDTH_80MINUSMINUS == en_bandwidth)
             || (WLAN_BAND_WIDTH_80MINUSPLUS == en_bandwidth)
             || (WLAN_BAND_WIDTH_80PLUSMINUS == en_bandwidth)
             || (WLAN_BAND_WIDTH_80PLUSPLUS == en_bandwidth))
    {
        pst_rx_max_mcs_map->us_max_mcs_1ss = MAC_MAX_SUP_MCS9_11AC_EACH_NSS;
        pst_tx_max_mcs_map->us_max_mcs_1ss = MAC_MAX_SUP_MCS9_11AC_EACH_NSS;
        pst_mib_info->st_wlan_mib_vht_sta_config.ul_dot11VHTRxHighestDataRateSupported = MAC_MAX_RATE_SINGLE_NSS_80M_11AC;
        pst_mib_info->st_wlan_mib_vht_sta_config.ul_dot11VHTTxHighestDataRateSupported = MAC_MAX_RATE_SINGLE_NSS_80M_11AC;
    }
}


oal_void  mac_vap_init_11ac_mcs_doublenss(
                wlan_mib_ieee802dot11_stru          *pst_mib_info,
                wlan_channel_bandwidth_enum_uint8    en_bandwidth)
{
    mac_tx_max_mcs_map_stru         *pst_tx_max_mcs_map;
    mac_rx_max_mcs_map_stru         *pst_rx_max_mcs_map;

    /* ��ȡmibֵָ�� */
    pst_rx_max_mcs_map = (mac_tx_max_mcs_map_stru *)(&(pst_mib_info->st_wlan_mib_vht_sta_config.us_dot11VHTRxMCSMap));
    pst_tx_max_mcs_map = (mac_tx_max_mcs_map_stru *)(&(pst_mib_info->st_wlan_mib_vht_sta_config.us_dot11VHTTxMCSMap));

    /* 20MHz���������£�֧��MCS0-MCS8 */
    if (WLAN_BAND_WIDTH_20M == en_bandwidth)
    {
        pst_rx_max_mcs_map->us_max_mcs_1ss = MAC_MAX_SUP_MCS8_11AC_EACH_NSS;
        pst_rx_max_mcs_map->us_max_mcs_2ss = MAC_MAX_SUP_MCS8_11AC_EACH_NSS;
        pst_tx_max_mcs_map->us_max_mcs_1ss = MAC_MAX_SUP_MCS8_11AC_EACH_NSS;
        pst_tx_max_mcs_map->us_max_mcs_2ss = MAC_MAX_SUP_MCS8_11AC_EACH_NSS;
        pst_mib_info->st_wlan_mib_vht_sta_config.ul_dot11VHTRxHighestDataRateSupported = MAC_MAX_RATE_DOUBLE_NSS_20M_11AC;
        pst_mib_info->st_wlan_mib_vht_sta_config.ul_dot11VHTTxHighestDataRateSupported = MAC_MAX_RATE_DOUBLE_NSS_20M_11AC;
    }

    /* 40MHz���������£�֧��MCS0-MCS9 */
    else if ((WLAN_BAND_WIDTH_40MINUS == en_bandwidth) || (WLAN_BAND_WIDTH_40PLUS == en_bandwidth))
    {
        pst_rx_max_mcs_map->us_max_mcs_1ss = MAC_MAX_SUP_MCS9_11AC_EACH_NSS;
        pst_rx_max_mcs_map->us_max_mcs_2ss = MAC_MAX_SUP_MCS9_11AC_EACH_NSS;
        pst_tx_max_mcs_map->us_max_mcs_1ss = MAC_MAX_SUP_MCS9_11AC_EACH_NSS;
        pst_tx_max_mcs_map->us_max_mcs_2ss = MAC_MAX_SUP_MCS9_11AC_EACH_NSS;
        pst_mib_info->st_wlan_mib_vht_sta_config.ul_dot11VHTRxHighestDataRateSupported = MAC_MAX_RATE_DOUBLE_NSS_40M_11AC;
        pst_mib_info->st_wlan_mib_vht_sta_config.ul_dot11VHTTxHighestDataRateSupported = MAC_MAX_RATE_DOUBLE_NSS_40M_11AC;
    }

    /* 80MHz���������£�֧��MCS0-MCS9 */
    else if ((WLAN_BAND_WIDTH_80MINUSMINUS == en_bandwidth)
             || (WLAN_BAND_WIDTH_80MINUSPLUS== en_bandwidth)
             || (WLAN_BAND_WIDTH_80PLUSMINUS== en_bandwidth)
             || (WLAN_BAND_WIDTH_80PLUSPLUS== en_bandwidth))
    {
        pst_rx_max_mcs_map->us_max_mcs_1ss = MAC_MAX_SUP_MCS9_11AC_EACH_NSS;
        pst_rx_max_mcs_map->us_max_mcs_2ss = MAC_MAX_SUP_MCS9_11AC_EACH_NSS;
        pst_tx_max_mcs_map->us_max_mcs_1ss = MAC_MAX_SUP_MCS9_11AC_EACH_NSS;
        pst_tx_max_mcs_map->us_max_mcs_2ss = MAC_MAX_SUP_MCS9_11AC_EACH_NSS;
        pst_mib_info->st_wlan_mib_vht_sta_config.ul_dot11VHTRxHighestDataRateSupported = MAC_MAX_RATE_DOUBLE_NSS_80M_11AC;
        pst_mib_info->st_wlan_mib_vht_sta_config.ul_dot11VHTTxHighestDataRateSupported = MAC_MAX_RATE_DOUBLE_NSS_80M_11AC;
    }
}


oal_void  mac_vap_init_mib_11ac(mac_vap_stru  *pst_mac_vap)
{
    wlan_mib_ieee802dot11_stru      *pst_mib_info;
    mac_device_stru                 *pst_mac_dev;

    pst_mac_dev = mac_res_get_dev(pst_mac_vap->uc_device_id);

    if (OAL_PTR_NULL == pst_mac_dev)
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{mac_vap_init_mib_11ac::pst_mac_dev[%d] null.}", pst_mac_vap->uc_device_id);

        return;
    }

    pst_mib_info = pst_mac_vap->pst_mib_info;
    pst_mib_info->st_wlan_mib_sta_config.en_dot11VHTOptionImplemented = OAL_TRUE;

    pst_mib_info->st_wlan_mib_vht_sta_config.ul_dot11MaxMPDULength                      = WLAN_MIB_VHT_MPDU_11454;
    pst_mib_info->st_wlan_mib_phy_vht.uc_dot11VHTChannelWidthOptionImplemented          = WLAN_MIB_VHT_SUPP_WIDTH_80;
    pst_mib_info->st_wlan_mib_phy_vht.en_dot11VHTLDPCCodingOptionImplemented            = pst_mac_dev->bit_ldpc_coding;
    pst_mib_info->st_wlan_mib_phy_vht.en_dot11VHTShortGIOptionIn80Implemented           = OAL_TRUE;
    pst_mib_info->st_wlan_mib_phy_vht.en_dot11VHTShortGIOptionIn160and80p80Implemented  = OAL_FALSE;
    pst_mib_info->st_wlan_mib_phy_vht.en_dot11VHTTxSTBCOptionImplemented                = pst_mac_dev->bit_tx_stbc;
    pst_mib_info->st_wlan_mib_phy_vht.en_dot11VHTRxSTBCOptionImplemented                = (pst_mac_dev->bit_rx_stbc == 0) ? OAL_FALSE : OAL_TRUE;
    pst_mib_info->st_wlan_mib_vht_txbf_config.en_dot11VHTSUBeamformerOptionImplemented  = pst_mac_dev->bit_su_bfmer;
    pst_mib_info->st_wlan_mib_vht_txbf_config.en_dot11VHTSUBeamformeeOptionImplemented  = pst_mac_dev->bit_su_bfmee;

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
    pst_mib_info->st_wlan_mib_vht_txbf_config.ul_dot11VHTNumberSoundingDimensions       = pst_mac_dev->uc_tx_chain - 1;
#else
    pst_mib_info->st_wlan_mib_vht_txbf_config.ul_dot11VHTNumberSoundingDimensions       = 0;
#endif
    pst_mib_info->st_wlan_mib_vht_txbf_config.en_dot11VHTMUBeamformerOptionImplemented  = OAL_FALSE;
    pst_mib_info->st_wlan_mib_vht_txbf_config.en_dot11VHTMUBeamformeeOptionImplemented  = pst_mac_dev->bit_mu_bfmee;
    pst_mib_info->st_wlan_mib_vht_txbf_config.ul_dot11VHTBeamformeeNTxSupport           = 1;
#ifdef _PRE_WLAN_FEATURE_TXOPPS
    pst_mib_info->st_wlan_mib_vht_sta_config.en_dot11VHTTXOPPowerSaveOptionImplemented  = OAL_TRUE;
#endif
    pst_mib_info->st_wlan_mib_vht_sta_config.en_dot11VHTControlFieldSupported           = OAL_FALSE;
    pst_mib_info->st_wlan_mib_vht_sta_config.ul_dot11VHTMaxRxAMPDUFactor                = 7;            /* 2^(13+1)-1�ֽ� */
#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
    pst_mib_info->st_wlan_mib_sta_config.en_dot11OperatingModeNotificationImplemented   = OAL_TRUE;
#endif
}

oal_void  mac_vap_init_mib_11i(mac_vap_stru  *pst_vap)
{
    mac_mib_set_rsnaactivated(pst_vap, OAL_FALSE);
    mac_mib_set_dot11RSNAMFPR(pst_vap, OAL_FALSE);
    mac_mib_set_dot11RSNAMFPC(pst_vap, OAL_FALSE);
    mac_mib_set_pre_auth_actived(pst_vap, OAL_FALSE);
    mac_mib_set_privacyinvoked(pst_vap, OAL_FALSE);
    mac_mib_init_rsnacfg_suites(pst_vap);
    mac_mib_set_rsnacfg_gtksareplaycounters(pst_vap, 0);
    mac_mib_set_rsnacfg_ptksareplaycounters(pst_vap, 0);
}

#ifdef _PRE_WLAN_FEATURE_11K
oal_void  mac_vap_init_mib_11k(mac_vap_stru  *pst_vap)
{
    pst_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11RadioMeasurementActivated = OAL_TRUE;

    pst_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11RMBeaconActiveMeasurementActivated    = OAL_TRUE;
    pst_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11RMBeaconPassiveMeasurementActivated   = OAL_TRUE;
    pst_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11RMBeaconTableMeasurementActivated     = OAL_TRUE;
    pst_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11RMLinkMeasurementActivated            = OAL_TRUE;

    pst_vap->pst_mib_info->st_wlan_mib_sta_config.ul_dot11RMMaxMeasurementDuration              = 1200; //1.2s
}
#endif

#if defined(_PRE_WLAN_FEATURE_11V) || defined(_PRE_WLAN_FEATURE_11V_ENABLE)
/* Ĭ��֧��11v ����ر����ϲ���ýӿ� */
OAL_STATIC oal_void  mac_vap_init_mib_11v(mac_vap_stru  *pst_vap)
{
    /*en_dot11MgmtOptionBSSTransitionActivated ��ʼ��ʱΪTRUE,�ɶ��ƻ��������or�ر� */
    mac_mib_set_MgmtOptionBSSTransitionActivated(pst_vap, OAL_TRUE);
    mac_mib_set_MgmtOptionBSSTransitionImplemented(pst_vap, OAL_TRUE);
    mac_mib_set_WirelessManagementImplemented(pst_vap, OAL_TRUE);
}
#endif




oal_void mac_vap_init_legacy_rates(mac_vap_stru *pst_vap,mac_data_rate_stru *pst_rates)
{
    oal_uint8                      uc_rate_index;
    oal_uint8                      uc_curr_rate_index = 0;
    mac_data_rate_stru            *puc_orig_rate;
    mac_data_rate_stru            *puc_curr_rate;
    oal_uint8                      uc_rates_num;

    /* ��ʼ�����ʼ� */
    uc_rates_num = MAC_DATARATES_PHY_80211G_NUM;

    /* ��ʼ�����ʸ������������ʸ������ǻ������ʸ��� */
    pst_vap->st_curr_sup_rates.st_rate.uc_rs_nrates = MAC_NUM_DR_802_11A;
    pst_vap->st_curr_sup_rates.uc_br_rate_num       = MAC_NUM_BR_802_11A;
    pst_vap->st_curr_sup_rates.uc_nbr_rate_num      = MAC_NUM_NBR_802_11A;
    pst_vap->st_curr_sup_rates.uc_min_rate          = 6;
    pst_vap->st_curr_sup_rates.uc_max_rate          = 24;

    /* �����ʿ�����VAP�ṹ���µ����ʼ��� */
    for (uc_rate_index = 0; uc_rate_index < uc_rates_num; uc_rate_index++)
    {
        puc_orig_rate = &pst_rates[uc_rate_index];
        puc_curr_rate = &(pst_vap->st_curr_sup_rates.st_rate.ast_rs_rates[uc_curr_rate_index]);

        /* Basic Rates */
        if ((puc_orig_rate->uc_mbps == 6)
            || (puc_orig_rate->uc_mbps == 12)
            || (puc_orig_rate->uc_mbps == 24))
        {
            oal_memcopy(puc_curr_rate,puc_orig_rate,sizeof(mac_data_rate_stru));
            puc_curr_rate->uc_mac_rate |= 0x80;
            uc_curr_rate_index++;
        }

        /* Non-basic rates */
        else if ((puc_orig_rate->uc_mbps== 9)
                || (puc_orig_rate->uc_mbps == 18)
                || (puc_orig_rate->uc_mbps == 36)
                || (puc_orig_rate->uc_mbps == 48)
                || (puc_orig_rate->uc_mbps == 54))
        {
            oal_memcopy(puc_curr_rate,puc_orig_rate,sizeof(mac_data_rate_stru));
            uc_curr_rate_index++;
        }

        if (uc_curr_rate_index == pst_vap->st_curr_sup_rates.st_rate.uc_rs_nrates)
        {
            break;
        }
    }
}


oal_void mac_vap_init_11b_rates(mac_vap_stru *pst_vap,mac_data_rate_stru *pst_rates)
{
    oal_uint8                      uc_rate_index;
    oal_uint8                      uc_curr_rate_index = 0;
    mac_data_rate_stru            *puc_orig_rate;
    mac_data_rate_stru            *puc_curr_rate;
    oal_uint8                      uc_rates_num;

    /* ��ʼ�����ʼ� */
    uc_rates_num = MAC_DATARATES_PHY_80211G_NUM;

    /* ��ʼ�����ʸ������������ʸ������ǻ������ʸ��� */
    pst_vap->st_curr_sup_rates.st_rate.uc_rs_nrates = MAC_NUM_DR_802_11B;
    pst_vap->st_curr_sup_rates.uc_br_rate_num       = 0;
    pst_vap->st_curr_sup_rates.uc_nbr_rate_num      = MAC_NUM_NBR_802_11B;
    pst_vap->st_curr_sup_rates.uc_min_rate          = 1;
    pst_vap->st_curr_sup_rates.uc_max_rate          = 2;

    /* �����ʿ�����VAP�ṹ���µ����ʼ��� */
    for (uc_rate_index = 0; uc_rate_index < uc_rates_num; uc_rate_index++)
    {
        puc_orig_rate = &pst_rates[uc_rate_index];
        puc_curr_rate = &(pst_vap->st_curr_sup_rates.st_rate.ast_rs_rates[uc_curr_rate_index]);

        /*  Basic Rates  */
        if ((puc_orig_rate->uc_mbps == 1) || (puc_orig_rate->uc_mbps == 2)
           || ((WLAN_VAP_MODE_BSS_STA == pst_vap->en_vap_mode) && ((puc_orig_rate->uc_mbps == 5) || (puc_orig_rate->uc_mbps == 11))))
        {
            pst_vap->st_curr_sup_rates.uc_br_rate_num++;
            oal_memcopy(puc_curr_rate,puc_orig_rate,sizeof(mac_data_rate_stru));
            puc_curr_rate->uc_mac_rate |= 0x80;
            uc_curr_rate_index++;
        }

        /* Non-basic rates */
        else if ((WLAN_VAP_MODE_BSS_AP == pst_vap->en_vap_mode)
                && ((puc_orig_rate->uc_mbps == 5) || (puc_orig_rate->uc_mbps == 11)))
        {
            oal_memcopy(puc_curr_rate,puc_orig_rate,sizeof(mac_data_rate_stru));
            uc_curr_rate_index++;
        }
        else
        {
            continue;
        }

        if (uc_curr_rate_index == pst_vap->st_curr_sup_rates.st_rate.uc_rs_nrates)
        {
            break;
        }
    }
}


oal_void mac_vap_init_11g_mixed_one_rates(mac_vap_stru *pst_vap,mac_data_rate_stru *pst_rates)
{
    oal_uint8                      uc_rate_index;
    mac_data_rate_stru            *puc_orig_rate;
    mac_data_rate_stru            *puc_curr_rate;
    oal_uint8                      uc_rates_num;

    /* ��ʼ�����ʼ� */
    uc_rates_num = MAC_DATARATES_PHY_80211G_NUM;

    /* ��ʼ�����ʸ������������ʸ������ǻ������ʸ��� */
    pst_vap->st_curr_sup_rates.st_rate.uc_rs_nrates = MAC_NUM_DR_802_11G_MIXED;
    pst_vap->st_curr_sup_rates.uc_br_rate_num       = MAC_NUM_BR_802_11G_MIXED_ONE;
    pst_vap->st_curr_sup_rates.uc_nbr_rate_num      = MAC_NUM_NBR_802_11G_MIXED_ONE;
    pst_vap->st_curr_sup_rates.uc_min_rate          = 1;
    pst_vap->st_curr_sup_rates.uc_max_rate          = 11;

    /* �����ʿ�����VAP�ṹ���µ����ʼ��� */
    for (uc_rate_index = 0; uc_rate_index < uc_rates_num; uc_rate_index++)
    {
        puc_orig_rate = &pst_rates[uc_rate_index];
        puc_curr_rate = &(pst_vap->st_curr_sup_rates.st_rate.ast_rs_rates[uc_rate_index]);

        oal_memcopy(puc_curr_rate,puc_orig_rate,sizeof(mac_data_rate_stru));

        /* Basic Rates */
        if ((puc_orig_rate->uc_mbps== 1)
            || (puc_orig_rate->uc_mbps == 2)
            || (puc_orig_rate->uc_mbps == 5)
            || (puc_orig_rate->uc_mbps == 11))
        {
            puc_curr_rate->uc_mac_rate |= 0x80;
        }
    }
}


oal_void mac_vap_init_11g_mixed_two_rates(mac_vap_stru *pst_vap,mac_data_rate_stru *pst_rates)
{
    oal_uint8                      uc_rate_index;
    mac_data_rate_stru            *puc_orig_rate;
    mac_data_rate_stru            *puc_curr_rate;
    oal_uint8                      uc_rates_num;

    /* ��ʼ�����ʼ� */
    uc_rates_num = MAC_DATARATES_PHY_80211G_NUM;

    /* ��ʼ�����ʸ������������ʸ������ǻ������ʸ��� */
    pst_vap->st_curr_sup_rates.st_rate.uc_rs_nrates = MAC_NUM_DR_802_11G_MIXED;
    pst_vap->st_curr_sup_rates.uc_br_rate_num       = MAC_NUM_BR_802_11G_MIXED_TWO;
    pst_vap->st_curr_sup_rates.uc_nbr_rate_num      = MAC_NUM_NBR_802_11G_MIXED_TWO;
    pst_vap->st_curr_sup_rates.uc_min_rate          = 1;
    pst_vap->st_curr_sup_rates.uc_max_rate          = 24;

    /* �����ʿ�����VAP�ṹ���µ����ʼ��� */
    for (uc_rate_index = 0; uc_rate_index < uc_rates_num; uc_rate_index++)
    {
        puc_orig_rate = &pst_rates[uc_rate_index];
        puc_curr_rate = &(pst_vap->st_curr_sup_rates.st_rate.ast_rs_rates[uc_rate_index]);

        oal_memcopy(puc_curr_rate,puc_orig_rate,sizeof(mac_data_rate_stru));

        /* Basic Rates */
        if ((puc_orig_rate->uc_mbps== 1)
            || (puc_orig_rate->uc_mbps == 2)
            || (puc_orig_rate->uc_mbps == 5)
            || (puc_orig_rate->uc_mbps == 11)
            || (puc_orig_rate->uc_mbps == 6)
            || (puc_orig_rate->uc_mbps == 12)
            || (puc_orig_rate->uc_mbps == 24))
        {
            puc_curr_rate->uc_mac_rate |= 0x80;
        }
    }
}


oal_void mac_vap_init_11n_rates(mac_vap_stru *pst_mac_vap,mac_device_stru *pst_mac_dev)
{
    wlan_mib_ieee802dot11_stru    *pst_mib_info;

    pst_mib_info = pst_mac_vap->pst_mib_info;
    /* ��ʼ�����ʼ� */

    /* MCS���MIBֵ��ʼ�� */
    pst_mib_info->st_phy_ht.en_dot11TxMCSSetDefined              = OAL_TRUE;
    pst_mib_info->st_phy_ht.en_dot11TxRxMCSSetNotEqual           = OAL_FALSE;
    pst_mib_info->st_phy_ht.en_dot11TxUnequalModulationSupported = OAL_FALSE;

    /* ��MIBֵ��MCS MAP���� */
    OAL_MEMZERO(pst_mib_info->st_supported_mcsrx.auc_dot11SupportedMCSRxValue, WLAN_HT_MCS_BITMASK_LEN);

    /* 1���ռ��� */
    if (WLAN_SINGLE_NSS == pst_mac_dev->en_nss_num)
    {
        pst_mib_info->st_phy_ht.ul_dot11TxMaximumNumberSpatialStreamsSupported = 1;
        pst_mib_info->st_supported_mcsrx.auc_dot11SupportedMCSRxValue[0] = 0xFF; /* ֧�� RX MCS 0-7��8λȫ��Ϊ1*/
        pst_mib_info->st_supported_mcstx.auc_dot11SupportedMCSTxValue[0] = 0xFF; /* ֧�� TX MCS 0-7��8λȫ��Ϊ1*/

        pst_mib_info->st_phy_ht.ul_dot11HighestSupportedDataRate = MAC_MAX_RATE_SINGLE_NSS_20M_11N;

        if ((WLAN_BAND_WIDTH_40MINUS == pst_mac_vap->st_channel.en_bandwidth) || (WLAN_BAND_WIDTH_40PLUS == pst_mac_vap->st_channel.en_bandwidth))
        {
            /* 40M ֧��MCS32 */
            pst_mib_info->st_supported_mcsrx.auc_dot11SupportedMCSRxValue[4] = (oal_uint8)0x01;  /* ֧�� RX MCS 32,���һλΪ1 */
            pst_mib_info->st_supported_mcstx.auc_dot11SupportedMCSTxValue[4] = (oal_uint8)0x01;  /* ֧�� RX MCS 32,���һλΪ1 */
            pst_mib_info->st_phy_ht.ul_dot11HighestSupportedDataRate = MAC_MAX_RATE_SINGLE_NSS_40M_11N;
        }
    }

    /* 2���ռ��� */
    else if (WLAN_DOUBLE_NSS == pst_mac_dev->en_nss_num)
    {
        pst_mib_info->st_phy_ht.ul_dot11TxMaximumNumberSpatialStreamsSupported = 2;
        pst_mib_info->st_supported_mcsrx.auc_dot11SupportedMCSRxValue[0] = 0xFF;  /* ֧�� RX MCS 0-7��8λȫ��Ϊ1*/
        pst_mib_info->st_supported_mcsrx.auc_dot11SupportedMCSRxValue[1] = 0xFF;  /* ֧�� RX MCS 8-15��8λȫ��Ϊ1*/

        pst_mib_info->st_supported_mcstx.auc_dot11SupportedMCSTxValue[0] = 0xFF;  /* ֧�� TX MCS 0-7��8λȫ��Ϊ1*/
        pst_mib_info->st_supported_mcstx.auc_dot11SupportedMCSTxValue[1] = 0xFF;  /* ֧�� TX MCS 8-15��8λȫ��Ϊ1*/

        pst_mib_info->st_phy_ht.ul_dot11HighestSupportedDataRate = MAC_MAX_RATE_DOUBLE_NSS_20M_11N;

        if ((WLAN_BAND_WIDTH_40MINUS == pst_mac_vap->st_channel.en_bandwidth) || (WLAN_BAND_WIDTH_40PLUS == pst_mac_vap->st_channel.en_bandwidth))
        {
            /* 40M ֧�ֵ��������Ϊ300M */
            pst_mib_info->st_supported_mcsrx.auc_dot11SupportedMCSRxValue[4] = (oal_uint8)0x01;  /* ֧�� RX MCS 32,���һλΪ1 */
            pst_mib_info->st_supported_mcstx.auc_dot11SupportedMCSTxValue[4] = (oal_uint8)0x01;  /* ֧�� RX MCS 32,���һλΪ1 */
            pst_mib_info->st_phy_ht.ul_dot11HighestSupportedDataRate = MAC_MAX_RATE_DOUBLE_NSS_40M_11N;
        }
    }
#ifdef _PRE_WLAN_FEATURE_11AC2G
    if((WLAN_HT_MODE == pst_mac_vap->en_protocol)
        && (OAL_TRUE == pst_mac_vap->st_cap_flag.bit_11ac2g)
        && (WLAN_BAND_2G == pst_mac_vap->st_channel.en_band))
    {
       mac_vap_init_11ac_rates(pst_mac_vap, pst_mac_dev);
    }
#endif
}


oal_void mac_vap_init_11ac_rates(mac_vap_stru *pst_mac_vap,mac_device_stru *pst_mac_dev)
{
    wlan_mib_ieee802dot11_stru    *pst_mib_info;

    pst_mib_info = pst_mac_vap->pst_mib_info;

    /* �Ƚ�TX RX MCSMAP��ʼ��Ϊ���пռ�������֧�� 0xFFFF*/
    pst_mib_info->st_wlan_mib_vht_sta_config.us_dot11VHTRxMCSMap = 0xFFFF;
    pst_mib_info->st_wlan_mib_vht_sta_config.us_dot11VHTTxMCSMap = 0xFFFF;

    if (WLAN_SINGLE_NSS == pst_mac_dev->en_nss_num)
    {
        /* 1���ռ�������� */
        mac_vap_init_11ac_mcs_singlenss(pst_mib_info, pst_mac_vap->st_channel.en_bandwidth);
    }
    else if (WLAN_DOUBLE_NSS == pst_mac_dev->en_nss_num)
    {
        /* 2���ռ�������� */
        mac_vap_init_11ac_mcs_doublenss(pst_mib_info, pst_mac_vap->st_channel.en_bandwidth);
    }
    else
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{mac_vap_init_11ac_rates::invalid en_nss_num[%d].}", pst_mac_dev->en_nss_num);
    }
}


oal_void mac_vap_init_p2p_rates(mac_vap_stru *pst_vap, wlan_protocol_enum_uint8 en_vap_protocol, mac_data_rate_stru  *pst_rates)
{
    mac_device_stru               *pst_mac_dev;

    pst_mac_dev = mac_res_get_dev(pst_vap->uc_device_id);
    if (OAL_PTR_NULL == pst_mac_dev)
    {
        OAM_ERROR_LOG1(pst_vap->uc_vap_id, OAM_SF_ANY, "{mac_vap_init_p2p_rates::pst_mac_dev[%d] null.}", pst_vap->uc_device_id);

        return;
    }

    mac_vap_init_legacy_rates(pst_vap, pst_rates);

    oal_memcopy(&pst_vap->ast_sta_sup_rates_ie[WLAN_BAND_5G], &pst_vap->st_curr_sup_rates, OAL_SIZEOF(pst_vap->st_curr_sup_rates));
    oal_memcopy(&pst_vap->ast_sta_sup_rates_ie[WLAN_BAND_2G], &pst_vap->st_curr_sup_rates, OAL_SIZEOF(pst_vap->st_curr_sup_rates));

    if (WLAN_VHT_MODE == en_vap_protocol)
    {
        mac_vap_init_11n_rates(pst_vap, pst_mac_dev);
        mac_vap_init_11ac_rates(pst_vap, pst_mac_dev);
    }
    else
    {
        mac_vap_init_11n_rates(pst_vap, pst_mac_dev);
    }
}

oal_void mac_vap_init_rates_by_protocol(mac_vap_stru *pst_vap, wlan_protocol_enum_uint8 en_vap_protocol, mac_data_rate_stru *pst_rates)
{
    mac_device_stru *pst_mac_dev;

    pst_mac_dev = (mac_device_stru *)mac_res_get_dev(pst_vap->uc_device_id);
    if (OAL_PTR_NULL == pst_mac_dev)
    {
        OAM_ERROR_LOG1(pst_vap->uc_vap_id, OAM_SF_ANY, "{mac_vap_init_rates_by_protocol::mac_res_get_dev fail.device_id:[%d].}", pst_vap->uc_device_id);
        return;
    }

    /* STAģʽĬ��Э��ģʽ��11ac����ʼ�����ʼ�Ϊ�������ʼ� */
#ifdef _PRE_WLAN_FEATURE_P2P
    if (!IS_LEGACY_VAP(pst_vap))
    {
        mac_vap_init_p2p_rates(pst_vap, en_vap_protocol, pst_rates);
        return;
    }
#endif
    if (WLAN_VAP_MODE_BSS_STA == pst_vap->en_vap_mode && WLAN_VHT_MODE == en_vap_protocol)
    {
        /* ����STAȫ�ŵ�ɨ�� 5Gʱ ��д֧�����ʼ�ie */
        mac_vap_init_legacy_rates(pst_vap, pst_rates);
        oal_memcopy(&pst_vap->ast_sta_sup_rates_ie[WLAN_BAND_5G], &pst_vap->st_curr_sup_rates, OAL_SIZEOF(pst_vap->st_curr_sup_rates));

        /* ����STAȫ�ŵ�ɨ�� 2Gʱ ��д֧�����ʼ�ie */
        mac_vap_init_11g_mixed_one_rates(pst_vap, pst_rates);
        oal_memcopy(&pst_vap->ast_sta_sup_rates_ie[WLAN_BAND_2G], &pst_vap->st_curr_sup_rates, OAL_SIZEOF(pst_vap->st_curr_sup_rates));

        mac_vap_init_11n_rates(pst_vap, pst_mac_dev);
        mac_vap_init_11ac_rates(pst_vap, pst_mac_dev);
    }
    else if ((WLAN_VHT_ONLY_MODE == en_vap_protocol) || (WLAN_VHT_MODE == en_vap_protocol))
    {
#ifdef _PRE_WLAN_FEATURE_11AC2G
        if (WLAN_BAND_2G == pst_vap->st_channel.en_band)
        {
            mac_vap_init_11g_mixed_one_rates(pst_vap, pst_rates);
        }
        else
        {
            mac_vap_init_legacy_rates(pst_vap, pst_rates);
        }
#else
        mac_vap_init_legacy_rates(pst_vap, pst_rates);
#endif
        mac_vap_init_11n_rates(pst_vap, pst_mac_dev);
        mac_vap_init_11ac_rates(pst_vap, pst_mac_dev);
    }
    else if ((WLAN_HT_ONLY_MODE == en_vap_protocol) || (WLAN_HT_MODE == en_vap_protocol))
    {
        if (WLAN_BAND_5G == pst_vap->st_channel.en_band)
        {
            mac_vap_init_legacy_rates(pst_vap, pst_rates);
        }
        else if (WLAN_BAND_2G == pst_vap->st_channel.en_band)
        {
            mac_vap_init_11g_mixed_one_rates(pst_vap, pst_rates);
        }

        mac_vap_init_11n_rates(pst_vap, pst_mac_dev);
    }
    else if ((WLAN_LEGACY_11A_MODE == en_vap_protocol) || (WLAN_LEGACY_11G_MODE == en_vap_protocol))
    {
        mac_vap_init_legacy_rates(pst_vap, pst_rates);
    }
    else if (WLAN_LEGACY_11B_MODE == en_vap_protocol)
    {
        mac_vap_init_11b_rates(pst_vap, pst_rates);
    }
    else if (WLAN_MIXED_ONE_11G_MODE == en_vap_protocol)
    {
        mac_vap_init_11g_mixed_one_rates(pst_vap, pst_rates);
    }
    else if (WLAN_MIXED_TWO_11G_MODE == en_vap_protocol)
    {
        mac_vap_init_11g_mixed_two_rates(pst_vap, pst_rates);
    }
    else
    {
        /* ��ʱ������ */
    }
}


oal_void mac_vap_init_rates(mac_vap_stru *pst_vap)
{
    mac_device_stru               *pst_mac_dev;
    wlan_protocol_enum_uint8       en_vap_protocol;
    mac_data_rate_stru            *pst_rates;

    pst_mac_dev = mac_res_get_dev(pst_vap->uc_device_id);

    if (OAL_PTR_NULL == pst_mac_dev)
    {
        OAM_ERROR_LOG1(pst_vap->uc_vap_id, OAM_SF_ANY, "{mac_vap_init_rates::pst_mac_dev[%d] null.}", pst_vap->uc_device_id);

        return;
    }

    /* ��ʼ�����ʼ� */
    pst_rates   = mac_device_get_all_rates(pst_mac_dev);

    en_vap_protocol = pst_vap->en_protocol;

    mac_vap_init_rates_by_protocol(pst_vap, en_vap_protocol, pst_rates);

}


oal_void mac_sta_init_bss_rates(mac_vap_stru *pst_vap, oal_void *pst_bss_dscr)
{
    mac_device_stru               *pst_mac_dev;
    wlan_protocol_enum_uint8       en_vap_protocol;
    mac_data_rate_stru            *pst_rates;
    oal_uint32                     i,j;
    mac_bss_dscr_stru             *pst_bss = (mac_bss_dscr_stru *)pst_bss_dscr;

    pst_mac_dev = mac_res_get_dev(pst_vap->uc_device_id);

    if (OAL_PTR_NULL == pst_mac_dev)
    {
        OAM_ERROR_LOG1(pst_vap->uc_vap_id, OAM_SF_ANY, "{mac_vap_init_rates::pst_mac_dev[%d] null.}", pst_vap->uc_device_id);

        return;
    }


    /* ��ʼ�����ʼ� */
    pst_rates   = mac_device_get_all_rates(pst_mac_dev);
    if(OAL_PTR_NULL != pst_bss)
    {
        for(i = 0; i < pst_bss->uc_num_supp_rates; i++)
        {
            for(j = 0; j < MAC_DATARATES_PHY_80211G_NUM; j++)
            {
                if((pst_rates[j].uc_mac_rate & 0x7f) == (pst_bss->auc_supp_rates[i] & 0x7f))
                {
                    pst_rates[j].uc_mac_rate = pst_bss->auc_supp_rates[i];
                    break;
                }
            }
        }
    }

    en_vap_protocol = pst_vap->en_protocol;

    mac_vap_init_rates_by_protocol(pst_vap, en_vap_protocol, pst_rates);
}

#if 0
#ifdef _PRE_WLAN_DFT_STAT

OAL_STATIC oal_uint32  mac_vap_dft_stat_init(mac_vap_stru *pst_vap)
{
    OAL_MEMZERO(&pst_vap->st_vap_dft, OAL_SIZEOF(mac_vap_dft_stru));

    return OAL_SUCC;
}
#endif

oal_uint32  mac_vap_dft_stat_clear(mac_vap_stru *pst_vap)
{
    pst_vap->st_vap_dft.ul_flg = 0;

    if (OAL_TRUE == pst_vap->st_vap_dft.st_vap_dft_timer.en_is_registerd)
    {
        FRW_TIMER_IMMEDIATE_DESTROY_TIMER(&pst_vap->st_vap_dft.st_vap_dft_timer);
    }

    if (OAL_PTR_NULL != pst_vap->st_vap_dft.pst_vap_dft_stats)
    {
        OAL_MEM_FREE(pst_vap->st_vap_dft.pst_vap_dft_stats, OAL_TRUE);
    }

    return OAL_SUCC;
}

#endif


oal_void mac_vap_set_tx_power(mac_vap_stru *pst_vap, oal_uint8 uc_tx_power)
{
    pst_vap->uc_tx_power = uc_tx_power;
}


oal_void mac_vap_set_aid(mac_vap_stru *pst_vap, oal_uint16 us_aid)
{
    pst_vap->us_sta_aid = us_aid;
}


oal_void mac_vap_set_assoc_id(mac_vap_stru *pst_vap, oal_uint8 uc_assoc_vap_id)
{
    pst_vap->uc_assoc_vap_id = uc_assoc_vap_id;
}


oal_void mac_vap_set_uapsd_cap(mac_vap_stru *pst_vap, oal_uint8 uc_uapsd_cap)
{
    pst_vap->uc_uapsd_cap = uc_uapsd_cap;
}


oal_void mac_vap_set_p2p_mode(mac_vap_stru *pst_vap, wlan_p2p_mode_enum_uint8 en_p2p_mode)
{
    pst_vap->en_p2p_mode = en_p2p_mode;
}


oal_void mac_vap_set_multi_user_idx(mac_vap_stru *pst_vap, oal_uint16 us_multi_user_idx)
{
    pst_vap->us_multi_user_idx = us_multi_user_idx;
}


oal_void mac_vap_set_rx_nss(mac_vap_stru *pst_vap, oal_uint8 uc_rx_nss)
{
    pst_vap->en_vap_rx_nss = uc_rx_nss;
}


oal_void mac_vap_set_al_tx_payload_flag(mac_vap_stru *pst_vap, oal_uint8 uc_paylod)
{
#ifdef _PRE_WLAN_FEATURE_ALWAYS_TX
    pst_vap->bit_payload_flag = uc_paylod;
#endif
}


oal_void mac_vap_set_al_tx_flag(mac_vap_stru *pst_vap, oal_bool_enum_uint8 en_flag)
{
#ifdef _PRE_WLAN_FEATURE_ALWAYS_TX
    pst_vap->bit_al_tx_flag = en_flag;
#endif
}


oal_void mac_vap_set_al_tx_first_run(mac_vap_stru *pst_vap, oal_bool_enum_uint8 en_flag)
{
#ifdef _PRE_WLAN_FEATURE_ALWAYS_TX
    pst_vap->bit_first_run = en_flag;
#endif
}

#ifdef _PRE_WLAN_FEATURE_STA_PM

oal_void mac_vap_set_uapsd_para(mac_vap_stru *pst_mac_vap, mac_cfg_uapsd_sta_stru *pst_uapsd_info)
{
    oal_uint8                 uc_ac;

    pst_mac_vap->st_sta_uapsd_cfg.uc_max_sp_len = pst_uapsd_info->uc_max_sp_len;

    for (uc_ac = 0; uc_ac < WLAN_WME_AC_BUTT; uc_ac++)
    {
        pst_mac_vap->st_sta_uapsd_cfg.uc_delivery_enabled[uc_ac] = pst_uapsd_info->uc_delivery_enabled[uc_ac];
        pst_mac_vap->st_sta_uapsd_cfg.uc_trigger_enabled[uc_ac]  = pst_uapsd_info->uc_trigger_enabled[uc_ac];
    }
}
#endif


oal_void mac_vap_set_wmm_params_update_count(mac_vap_stru *pst_vap, oal_uint8 uc_update_count)
{
    pst_vap->uc_wmm_params_update_count = uc_update_count;
}


oal_void mac_vap_set_rifs_tx_on(mac_vap_stru *pst_vap, oal_uint8 uc_value)
{
    pst_vap->st_cap_flag.bit_rifs_tx_on = uc_value;
}
#if 0

oal_void mac_vap_set_tdls_prohibited(mac_vap_stru *pst_vap, oal_uint8 uc_value)
{
    pst_vap->st_cap_flag.bit_tdls_prohibited = uc_value;
}


oal_void mac_vap_set_tdls_channel_switch_prohibited(mac_vap_stru *pst_vap, oal_uint8 uc_value)
{
    pst_vap->st_cap_flag.bit_tdls_channel_switch_prohibited = uc_value;
}
#endif
#ifdef _PRE_WLAN_FEATURE_VOWIFI

oal_void mac_vap_vowifi_init(mac_vap_stru *pst_mac_vap)
{
    if (WLAN_VAP_MODE_BSS_STA != pst_mac_vap->en_vap_mode)
    {
        return;
    }
    if (OAL_PTR_NULL == pst_mac_vap->pst_vowifi_cfg_param)
    {
        pst_mac_vap->pst_vowifi_cfg_param = OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL, OAL_SIZEOF(mac_vowifi_param_stru), OAL_TRUE);
        if (OAL_PTR_NULL == pst_mac_vap->pst_vowifi_cfg_param)
        {
            OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_VOWIFI, "{mac_vap_vowifi_init::pst_vowifi_cfg_param alloc null, size[%d].}", OAL_SIZEOF(mac_vowifi_param_stru));
            return ;
        }
    }
    OAL_MEMZERO(pst_mac_vap->pst_vowifi_cfg_param, OAL_SIZEOF(mac_vowifi_param_stru));
    pst_mac_vap->pst_vowifi_cfg_param->en_vowifi_mode         = MAC_VAP_VOWIFI_MODE_DEFAULT;
    pst_mac_vap->pst_vowifi_cfg_param->uc_trigger_count_thres = MAC_VAP_VOWIFI_TRIGGER_COUNT_DEFAULT;
    pst_mac_vap->pst_vowifi_cfg_param->us_rssi_period_ms      = MAC_VAP_VOWIFI_PERIOD_DEFAULT_MS;
    pst_mac_vap->pst_vowifi_cfg_param->c_rssi_high_thres      = MAC_VAP_VOWIFI_HIGH_THRES_DEFAULT;
    pst_mac_vap->pst_vowifi_cfg_param->c_rssi_low_thres       = MAC_VAP_VOWIFI_LOW_THRES_DEFAULT;
}

oal_void mac_vap_vowifi_exit(mac_vap_stru *pst_mac_vap)
{
    mac_vowifi_param_stru   *pst_vowifi_cfg_param;

    if (OAL_PTR_NULL == pst_mac_vap->pst_vowifi_cfg_param)
    {
        return;
    }

    pst_vowifi_cfg_param = pst_mac_vap->pst_vowifi_cfg_param;

    /*���ÿ����ͷ�*/
    pst_mac_vap->pst_vowifi_cfg_param = OAL_PTR_NULL;
    OAL_MEM_FREE(pst_vowifi_cfg_param, OAL_TRUE);
}
#endif /* #ifdef _PRE_WLAN_FEATURE_VOWIFI */


oal_void mac_vap_set_11ac2g(mac_vap_stru *pst_vap, oal_uint8 uc_value)
{
    pst_vap->st_cap_flag.bit_11ac2g = uc_value;
}


oal_void mac_vap_set_hide_ssid(mac_vap_stru *pst_vap, oal_uint8 uc_value)
{
    pst_vap->st_cap_flag.bit_hide_ssid = uc_value;
}



oal_uint8 mac_vap_get_peer_obss_scan(mac_vap_stru *pst_vap)
{
    return pst_vap->st_cap_flag.bit_peer_obss_scan;
}


oal_void mac_vap_set_peer_obss_scan(mac_vap_stru *pst_vap, oal_uint8 uc_value)
{
    pst_vap->st_cap_flag.bit_peer_obss_scan = uc_value;
}


wlan_p2p_mode_enum_uint8  mac_get_p2p_mode(mac_vap_stru *pst_vap)
{
    return (pst_vap->en_p2p_mode);
}



oal_void mac_dec_p2p_num(mac_vap_stru *pst_vap)
{
    mac_device_stru               *pst_device;

    pst_device     = mac_res_get_dev(pst_vap->uc_device_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_device))
    {
        OAM_ERROR_LOG1(pst_vap->uc_vap_id, OAM_SF_ANY, "{mac_p2p_dec_num::pst_device[%d] null.}", pst_vap->uc_device_id);
        return;
    }

    if (IS_P2P_DEV(pst_vap))
    {
        pst_device->st_p2p_info.uc_p2p_device_num--;
    }
    else if (IS_P2P_GO(pst_vap) || IS_P2P_CL(pst_vap))
    {
        pst_device->st_p2p_info.uc_p2p_goclient_num--;
    }
}

oal_void mac_inc_p2p_num(mac_vap_stru *pst_vap)
{
    mac_device_stru               *pst_dev;

    pst_dev = mac_res_get_dev(pst_vap->uc_device_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_dev))
    {
        OAM_ERROR_LOG1(pst_vap->uc_vap_id, OAM_SF_CFG, "{hmac_inc_p2p_num::pst_dev[%d] null.}", pst_vap->uc_device_id);
        return;
    }

    if (IS_P2P_DEV(pst_vap))
    {
        /* device��sta������1 */
        pst_dev->st_p2p_info.uc_p2p_device_num++;
    }
    else if (IS_P2P_GO(pst_vap))
    {
        pst_dev->st_p2p_info.uc_p2p_goclient_num++;
    }
    else if (IS_P2P_CL(pst_vap))
    {
        pst_dev->st_p2p_info.uc_p2p_goclient_num++;
    }
}

oal_bool_enum_uint8  mac_is_wep_enabled(mac_vap_stru *pst_mac_vap)
{
    if(OAL_PTR_NULL == pst_mac_vap)
    {
        return OAL_FALSE;
    }

    if ((mac_mib_get_privacyinvoked(pst_mac_vap) == OAL_FALSE)||(mac_mib_get_rsnaactivated(pst_mac_vap) == OAL_TRUE))
    {
        return OAL_FALSE;
    }

    return OAL_TRUE;
}

oal_uint32 mac_vap_save_app_ie(mac_vap_stru *pst_mac_vap, oal_app_ie_stru *pst_app_ie, en_app_ie_type_uint8 en_type)
{
    oal_uint8           *puc_ie = OAL_PTR_NULL;
    oal_uint32           ul_ie_len;
    oal_app_ie_stru      st_tmp_app_ie;

    OAL_MEMZERO(&st_tmp_app_ie, OAL_SIZEOF(st_tmp_app_ie));

    if (en_type >= OAL_APP_IE_NUM)
    {
        OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{mac_vap_save_app_ie::invalid en_type[%d].}", en_type);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    ul_ie_len = pst_app_ie->ul_ie_len;

    /* �������WPS ����Ϊ0�� ��ֱ���ͷ�VAP ����Դ */
    if (0 == ul_ie_len)
    {
        if (OAL_PTR_NULL != pst_mac_vap->ast_app_ie[en_type].puc_ie)
        {
            OAL_MEM_FREE(pst_mac_vap->ast_app_ie[en_type].puc_ie, OAL_TRUE);
        }

        pst_mac_vap->ast_app_ie[en_type].puc_ie         = OAL_PTR_NULL;
        pst_mac_vap->ast_app_ie[en_type].ul_ie_len      = 0;
        pst_mac_vap->ast_app_ie[en_type].ul_ie_max_len  = 0;

        return OAL_SUCC;
    }

    /* �������͵�IE�Ƿ���Ҫ�����ڴ� */
    if ((pst_mac_vap->ast_app_ie[en_type].ul_ie_max_len < ul_ie_len) || (NULL == pst_mac_vap->ast_app_ie[en_type].puc_ie))
    {
        /* ���������Ӧ�ó��֣�ά����Ҫ */
        if (NULL == pst_mac_vap->ast_app_ie[en_type].puc_ie && pst_mac_vap->ast_app_ie[en_type].ul_ie_max_len != 0  )
        {
            OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{mac_vap_set_app_ie::invalid len[%d].}",
                          pst_mac_vap->ast_app_ie[en_type].ul_ie_max_len);
        }

        /* �����ǰ���ڴ�ռ�С������ϢԪ����Ҫ�ĳ��ȣ�����Ҫ���������ڴ� */
        puc_ie = OAL_MEM_ALLOC(OAL_MEM_POOL_ID_LOCAL, (oal_uint16)(ul_ie_len), OAL_TRUE);
        if (OAL_PTR_NULL == puc_ie)
        {
            OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{mac_vap_set_app_ie::LOCAL_MEM_POOL is empty!,len[%d], en_type[%d].}",
                              pst_app_ie->ul_ie_len, en_type);
            return OAL_ERR_CODE_ALLOC_MEM_FAIL;
        }

        OAL_MEM_FREE(pst_mac_vap->ast_app_ie[en_type].puc_ie, OAL_TRUE);

        pst_mac_vap->ast_app_ie[en_type].puc_ie = puc_ie;
        pst_mac_vap->ast_app_ie[en_type].ul_ie_max_len = ul_ie_len;
    }

    oal_memcopy((oal_void*)pst_mac_vap->ast_app_ie[en_type].puc_ie, (oal_void*)pst_app_ie->auc_ie, ul_ie_len);
    pst_mac_vap->ast_app_ie[en_type].ul_ie_len = ul_ie_len;

    return OAL_SUCC;
}
 
 oal_uint32 mac_vap_clear_app_ie(mac_vap_stru *pst_mac_vap, en_app_ie_type_uint8 en_type)
 {
     if (en_type < OAL_APP_IE_NUM)
     {
         if (OAL_PTR_NULL != pst_mac_vap->ast_app_ie[en_type].puc_ie)
         {
             OAL_MEM_FREE(pst_mac_vap->ast_app_ie[en_type].puc_ie, OAL_TRUE);
             pst_mac_vap->ast_app_ie[en_type].puc_ie    = OAL_PTR_NULL;
         }
         pst_mac_vap->ast_app_ie[en_type].ul_ie_len     = 0;
         pst_mac_vap->ast_app_ie[en_type].ul_ie_max_len = 0;
     }

     return OAL_SUCC;
 }

 oal_void mac_vap_free_mib(mac_vap_stru   *pst_vap)
{
    if(NULL == pst_vap)
        return;

    if (OAL_PTR_NULL != pst_vap->pst_mib_info)
    {
        wlan_mib_ieee802dot11_stru  *pst_mib_info = pst_vap->pst_mib_info;
        /*���ÿ����ͷ�*/
        pst_vap->pst_mib_info = OAL_PTR_NULL;
        OAL_MEM_FREE(pst_mib_info, OAL_TRUE);
    }
}

 
oal_uint32 mac_vap_exit(mac_vap_stru *pst_vap)
{
    mac_device_stru               *pst_device;
    oal_uint8                      uc_index;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_vap))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{mac_vap_exit::pst_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* ��ֹ���뵼�º�����uc_vap_num����uc_sta_num�ȼ���ֵ�ظ�ִ�м����� */
    if (MAC_VAP_STATE_BUTT == pst_vap->en_vap_state)
    {
        OAM_WARNING_LOG0(pst_vap->uc_vap_id, OAM_SF_ANY, "{mac_vap_exit::vap_state is already MAC_VAP_STATE_BUTT, and then return!! }");
        return OAL_SUCC;
    }

    pst_vap->uc_init_flag = MAC_VAP_INVAILD;

    /* �ͷ���hmac�йص��ڴ� */
    mac_vap_free_mib(pst_vap);
#ifdef _PRE_WLAN_FEATURE_VOWIFI
    /* �ͷ�vowifi����ڴ� */
    mac_vap_vowifi_exit(pst_vap);
#endif /* _PRE_WLAN_FEATURE_VOWIFI */
    /* �ͷ�WPS��ϢԪ���ڴ� */
    for (uc_index = 0; uc_index < OAL_APP_IE_NUM; uc_index++)
    {
        mac_vap_clear_app_ie(pst_vap, uc_index);
    }

    /* ҵ��vap��ɾ������device��ȥ�� */
    pst_device     = mac_res_get_dev(pst_vap->uc_device_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_device))
    {
        OAM_ERROR_LOG1(pst_vap->uc_vap_id, OAM_SF_ANY, "{mac_vap_exit::pst_device[%d] null.}", pst_vap->uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* ҵ��vap�Ѿ�ɾ������device��ȥ�� */
    for (uc_index = 0; uc_index < pst_device->uc_vap_num; uc_index++)
    {
        /* ��device���ҵ�vap id */
        if (pst_device->auc_vap_id[uc_index] == pst_vap->uc_vap_id)
        {
            /* ����������һ��vap��������һ��vap id�ƶ������λ�ã�ʹ�ø������ǽ��յ� */
            if(uc_index < (pst_device->uc_vap_num - 1))
            {
                pst_device->auc_vap_id[uc_index] = pst_device->auc_vap_id[pst_device->uc_vap_num - 1];
                break;
            }
        }
    }

    if (pst_device->uc_vap_num != 0)
    {
        /* device�µ�vap������1 */
        pst_device->uc_vap_num--;
    }
    else
    {
        OAM_ERROR_LOG1(pst_vap->uc_vap_id, OAM_SF_CFG,
                            "{mac_vap_exit::mac_device's vap_num is zero. sta_num = %d}",
                            pst_device->uc_sta_num);
    }

    /* �����������ɾ����vap id����֤��������Ԫ�ؾ�Ϊδɾ��vap */
    pst_device->auc_vap_id[pst_device->uc_vap_num] = 0;

    /* device��sta������1 */
    if (WLAN_VAP_MODE_BSS_STA == pst_vap->en_vap_mode)
    {
        if (pst_device->uc_sta_num != 0)
        {
            pst_device->uc_sta_num--;
        }
        else
        {
            OAM_ERROR_LOG1(pst_vap->uc_vap_id, OAM_SF_CFG,
                            "{mac_vap_exit::mac_device's sta_num is zero. vap_num = %d}",
                            pst_device->uc_vap_num);
        }
    }
#ifdef _PRE_WLAN_FEATURE_P2P
    mac_dec_p2p_num(pst_vap);
#endif

    pst_vap->en_protocol  = WLAN_PROTOCOL_BUTT;

    /* ���1��vapɾ��ʱ�����device��������Ϣ */
    if (0 == pst_device->uc_vap_num)
    {
        pst_device->uc_max_channel   = 0;
        pst_device->en_max_band      = WLAN_BAND_BUTT;
        pst_device->en_max_bandwidth = WLAN_BAND_WIDTH_BUTT;
    }

    /* ɾ��֮��vap��״̬��λ�Ƿ� */
    mac_vap_state_change(pst_vap, MAC_VAP_STATE_BUTT);

    return OAL_SUCC;
}


oal_uint32 mac_vap_check_signal_bridge(mac_vap_stru *pst_mac_vap)
{
    mac_device_stru *pst_mac_device;
    mac_vap_stru    *pst_other_vap;
    oal_uint8        uc_vap_idx;

    pst_mac_device = (mac_device_stru *)mac_res_get_dev(pst_mac_vap->uc_device_id);
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_device))
    {
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* check all vap state in case other vap is signal bridge GO mode */
    for (uc_vap_idx = 0; uc_vap_idx < pst_mac_device->uc_vap_num; uc_vap_idx++)
    {
        pst_other_vap = mac_res_get_mac_vap(pst_mac_device->auc_vap_id[uc_vap_idx]);
        if (OAL_PTR_NULL == pst_other_vap)
        {
            continue;
        }

#ifdef _PRE_WLAN_FEATURE_P2P
        /* �ն�����: ���ź��ţ���ֹ���� */
        if (WLAN_P2P_GO_MODE == pst_other_vap->en_p2p_mode)
        {
            /* �����P2P GOģʽ��Beacon֡������P2P ie��Ϊ�ź��� */
            if (mac_find_vendor_ie(MAC_WLAN_OUI_WFA, MAC_WLAN_OUI_TYPE_WFA_P2P, pst_mac_vap->ast_app_ie[OAL_APP_BEACON_IE].puc_ie,
                    (oal_int32) pst_mac_vap->ast_app_ie[OAL_APP_BEACON_IE].ul_ie_len) == OAL_PTR_NULL)
            {
                return OAL_FAIL;
            }
        }
#endif
    }

    return OAL_SUCC;
}



oal_void  mac_init_mib(mac_vap_stru *pst_mac_vap)
{
    wlan_mib_ieee802dot11_stru *pst_mib_info;
    oal_uint8 uc_idx;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_vap))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{mac_init_mib::pst_mac_vap null.}");

        return;
    }

    pst_mib_info = pst_mac_vap->pst_mib_info;

    /* ��������mibֵ��ʼ�� */
    pst_mib_info->st_wlan_mib_sta_config.ul_dot11DTIMPeriod             = WLAN_DTIM_DEFAULT;
    pst_mib_info->st_wlan_mib_operation.ul_dot11RTSThreshold            = WLAN_RTS_MAX;
    pst_mib_info->st_wlan_mib_operation.ul_dot11FragmentationThreshold  = WLAN_FRAG_THRESHOLD_MAX;
    pst_mib_info->st_wlan_mib_sta_config.en_dot11DesiredBSSType         = WLAN_MIB_DESIRED_BSSTYPE_INFRA;
    pst_mib_info->st_wlan_mib_sta_config.ul_dot11BeaconPeriod           = WLAN_BEACON_INTVAL_DEFAULT;


    mac_mib_set_ShortPreambleOptionImplemented(pst_mac_vap, WLAN_LEGACY_11B_MIB_SHORT_PREAMBLE);
    pst_mib_info->st_phy_hrdsss.en_dot11PBCCOptionImplemented           = OAL_FALSE;
    pst_mib_info->st_phy_hrdsss.en_dot11ChannelAgilityPresent           = OAL_FALSE;
    pst_mib_info->st_wlan_mib_sta_config.en_dot11MultiDomainCapabilityActivated = OAL_TRUE;
    pst_mib_info->st_wlan_mib_sta_config.en_dot11SpectrumManagementRequired = OAL_TRUE;
    pst_mib_info->st_wlan_mib_sta_config.en_dot11ExtendedChannelSwitchActivated = OAL_FALSE;
    pst_mib_info->st_wlan_mib_sta_config.en_dot11QosOptionImplemented   = OAL_TRUE;
    pst_mib_info->st_wlan_mib_sta_config.en_dot11APSDOptionImplemented  = OAL_FALSE;
    pst_mib_info->st_wlan_mib_sta_config.en_dot11QBSSLoadImplemented    = OAL_TRUE;
    pst_mib_info->st_phy_erp.en_dot11ShortSlotTimeOptionImplemented     = OAL_TRUE;
    pst_mib_info->st_phy_erp.en_dot11ShortSlotTimeOptionActivated       = OAL_TRUE;
    pst_mib_info->st_wlan_mib_sta_config.en_dot11RadioMeasurementActivated = OAL_FALSE;


    pst_mib_info->st_phy_erp.en_dot11DSSSOFDMOptionActivated            = OAL_FALSE;
    pst_mib_info->st_wlan_mib_sta_config.en_dot11ImmediateBlockAckOptionImplemented = OAL_TRUE;
    pst_mib_info->st_wlan_mib_sta_config.en_dot11DelayedBlockAckOptionImplemented   = OAL_FALSE;
    pst_mib_info->st_wlan_mib_sta_config.ul_dot11AuthenticationResponseTimeOut = WLAN_AUTH_TIMEOUT;

    mac_mib_set_HtProtection(pst_mac_vap, WLAN_MIB_HT_NO_PROTECTION);
    mac_mib_set_RifsMode(pst_mac_vap, OAL_TRUE);
    mac_mib_set_NonGFEntitiesPresent(pst_mac_vap, OAL_FALSE);
    mac_mib_set_LsigTxopFullProtectionActivated(pst_mac_vap, OAL_TRUE);

    pst_mib_info->st_wlan_mib_operation.en_dot11DualCTSProtection       = OAL_FALSE;
    pst_mib_info->st_wlan_mib_operation.en_dot11PCOActivated            = OAL_FALSE;

    pst_mib_info->st_wlan_mib_sta_config.ul_dot11AssociationResponseTimeOut = WLAN_ASSOC_TIMEOUT;
    pst_mib_info->st_wlan_mib_sta_config.ul_dot11AssociationSAQueryMaximumTimeout = WLAN_SA_QUERY_MAXIMUM_TIME_FIXED;
    pst_mib_info->st_wlan_mib_sta_config.ul_dot11AssociationSAQueryRetryTimeout   = WLAN_SA_QUERY_RETRY_TIME_FIXED;

    /* ��֤�㷨���ʼ�� */

    /* WEP ȱʡKey���ʼ�� */
    for(uc_idx = 0; uc_idx < WLAN_NUM_DOT11WEPDEFAULTKEYVALUE; uc_idx++)
    {
        oal_memset(pst_mib_info->ast_wlan_mib_wep_dflt_key[uc_idx].auc_dot11WEPDefaultKeyValue, 0, WLAN_MAX_WEP_STR_SIZE);
        /* ��С��ʼ��Ϊ WEP-40  */
        pst_mib_info->ast_wlan_mib_wep_dflt_key[uc_idx].auc_dot11WEPDefaultKeyValue[WLAN_WEP_SIZE_OFFSET] = 40;
    }



    /*    ���˽�б��ʼ��  */
    mac_mib_set_privacyinvoked(pst_mac_vap, OAL_FALSE);
    pst_mib_info->st_wlan_mib_privacy.uc_dot11WEPDefaultKeyID                = 0;

    /* ����wmm������ʼֵ */
    mac_vap_init_wme_param(pst_mac_vap);

    /* 11i */
    mac_vap_init_mib_11i(pst_mac_vap);

    /* Ĭ��11n 11acʹ�ܹرգ�����Э��ģʽʱ�� */
    mac_vap_init_mib_11n(pst_mac_vap);
    mac_vap_init_mib_11ac(pst_mac_vap);

    /* staut�͹���mib���ʼ�� */
    pst_mib_info->st_wlan_mib_sta_config.uc_dot11PowerManagementMode         = WLAN_MIB_PWR_MGMT_MODE_ACTIVE;

#ifdef _PRE_WLAN_FEATURE_11K
    /* 11k */
    mac_vap_init_mib_11k(pst_mac_vap);
#endif

#ifdef _PRE_WLAN_FEATURE_11V_ENABLE
    mac_vap_init_mib_11v(pst_mac_vap);
#endif
}


oal_void  mac_vap_cap_init_legacy(mac_vap_stru *pst_mac_vap)
{
    pst_mac_vap->st_cap_flag.bit_rifs_tx_on = OAL_FALSE;
    pst_mac_vap->st_cap_flag.bit_smps       = WLAN_MIB_MIMO_POWER_SAVE_MIMO;
    return;
}


oal_uint32  mac_vap_cap_init_htvht(mac_vap_stru *pst_mac_vap)
{
    pst_mac_vap->st_cap_flag.bit_rifs_tx_on = OAL_FALSE;

#ifdef _PRE_WLAN_FEATURE_TXOPPS
    if(OAL_PTR_NULL == pst_mac_vap->pst_mib_info)
    {
        OAM_ERROR_LOG3(pst_mac_vap->uc_vap_id, OAM_SF_ASSOC, "{mac_vap_cap_init_htvht::pst_mib_info null,vap mode[%d] state[%d] user num[%d].}",
                                     pst_mac_vap->en_vap_mode ,pst_mac_vap->en_vap_state, pst_mac_vap->us_user_nums);
        return OAL_FAIL;
    }
    if (pst_mac_vap->en_protocol == WLAN_VHT_MODE ||
         pst_mac_vap->en_protocol == WLAN_VHT_ONLY_MODE)
    {
        pst_mac_vap->st_cap_flag.bit_txop_ps   = OAL_FALSE;
        pst_mac_vap->pst_mib_info->st_wlan_mib_vht_sta_config.en_dot11VHTTXOPPowerSaveOptionImplemented  = OAL_TRUE;
    }
    else
    {
        pst_mac_vap->pst_mib_info->st_wlan_mib_vht_sta_config.en_dot11VHTTXOPPowerSaveOptionImplemented  = OAL_FALSE;
        pst_mac_vap->st_cap_flag.bit_txop_ps    = OAL_FALSE;
    }
#endif

#ifdef _PRE_WLAN_FEATURE_SMPS
    pst_mac_vap->st_cap_flag.bit_smps = mac_mib_get_smps(pst_mac_vap);
#else
    pst_mac_vap->st_cap_flag.bit_smps = WLAN_MIB_MIMO_POWER_SAVE_MIMO;
#endif

    return OAL_SUCC;
}



//l00311403TODO
oal_uint32 mac_vap_config_vht_ht_mib_by_protocol(mac_vap_stru *pst_mac_vap)
{

    if(OAL_PTR_NULL == pst_mac_vap->pst_mib_info)
    {
        OAM_ERROR_LOG3(pst_mac_vap->uc_vap_id, OAM_SF_ASSOC, "{mac_vap_config_vht_ht_mib_by_protocol::pst_mib_info null,vap mode[%d] state[%d] user num[%d].}",
                                     pst_mac_vap->en_vap_mode ,pst_mac_vap->en_vap_state, pst_mac_vap->us_user_nums);
        return OAL_FAIL;
    }
    /* ����Э��ģʽ���� HT/VHT mibֵ */
    if (WLAN_HT_MODE == pst_mac_vap->en_protocol || WLAN_HT_ONLY_MODE == pst_mac_vap->en_protocol)
    {
        pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11HighThroughputOptionImplemented = OAL_TRUE;
        pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11VHTOptionImplemented = OAL_FALSE;
    }
    else if (WLAN_VHT_MODE == pst_mac_vap->en_protocol || WLAN_VHT_ONLY_MODE == pst_mac_vap->en_protocol)
    {
        pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11HighThroughputOptionImplemented = OAL_TRUE;
        pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11VHTOptionImplemented = OAL_TRUE;
    }
    else
    {
        pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11HighThroughputOptionImplemented = OAL_FALSE;
        pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11VHTOptionImplemented = OAL_FALSE;
    }
#ifdef _PRE_WLAN_FEATURE_11AC2G
    if((WLAN_HT_MODE == pst_mac_vap->en_protocol)
        && (OAL_TRUE == pst_mac_vap->st_cap_flag.bit_11ac2g)
        && (WLAN_BAND_2G == pst_mac_vap->st_channel.en_band))
    {
        pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11VHTOptionImplemented = OAL_TRUE;
    }
#endif

    if (g_uc_hitalk_status & NARROW_BAND_ON_MASK)
    {
        pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11HighThroughputOptionImplemented = OAL_FALSE;
        pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.en_dot11VHTOptionImplemented = OAL_FALSE;
    }

    return OAL_SUCC;
}


oal_void  mac_vap_init_rx_nss_by_protocol(mac_vap_stru *pst_mac_vap)
{
    wlan_protocol_enum_uint8 en_protocol;
    mac_device_stru         *pst_mac_device;

    en_protocol   = pst_mac_vap->en_protocol;

    pst_mac_device = mac_res_get_dev(pst_mac_vap->uc_device_id);

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_device))
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{mac_vap_init_rx_nss_by_protocol::pst_mac_device[%d] null.}", pst_mac_vap->uc_device_id);
        return;
    }

    switch(en_protocol)
    {
        case WLAN_HT_MODE:
        case WLAN_VHT_MODE:
        case WLAN_HT_ONLY_MODE:
        case WLAN_VHT_ONLY_MODE:
        case WLAN_HT_11G_MODE:
             pst_mac_vap->en_vap_rx_nss  = WLAN_DOUBLE_NSS;
             break;
        case WLAN_PROTOCOL_BUTT:
             pst_mac_vap->en_vap_rx_nss  = WLAN_NSS_BUTT;
             return;

        default:
             pst_mac_vap->en_vap_rx_nss  = WLAN_SINGLE_NSS;
             break;
    }

    pst_mac_vap->en_vap_rx_nss = OAL_MIN(pst_mac_vap->en_vap_rx_nss, pst_mac_device->en_nss_num);
}


oal_uint32  mac_vap_init_by_protocol(mac_vap_stru *pst_mac_vap, wlan_protocol_enum_uint8 en_protocol)
{
    pst_mac_vap->en_protocol          = en_protocol;

    if (en_protocol < WLAN_HT_MODE)
    {
        mac_vap_cap_init_legacy(pst_mac_vap);
    }
    else
    {
        if(OAL_SUCC != mac_vap_cap_init_htvht(pst_mac_vap))
        {
            return OAL_FAIL;
        }
    }

    /* ����Э��ģʽ����mibֵ */
    if(OAL_SUCC != mac_vap_config_vht_ht_mib_by_protocol(pst_mac_vap))
    {
        return OAL_FAIL;
    }

    /* ����Э����³�ʼ���ռ������� */
    mac_vap_init_rx_nss_by_protocol(pst_mac_vap);

    return OAL_SUCC;
}


oal_void  mac_vap_change_mib_by_bandwidth(mac_vap_stru *pst_mac_vap, wlan_channel_bandwidth_enum_uint8 en_bandwidth)
{
    wlan_mib_ieee802dot11_stru *pst_mib_info;

    pst_mib_info = pst_mac_vap->pst_mib_info;

    if (OAL_PTR_NULL == pst_mib_info)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{mac_vap_change_mib_by_bandwidth::pst_mib_info null.}");

        return;
    }

    /* ����40Mʹ��mib, Ĭ��ʹ�� */
    mac_mib_set_FortyMHzOperationImplemented(pst_mac_vap, OAL_TRUE);

    /* ����short giʹ��mib, Ĭ��ȫʹ�ܣ����ݴ�����Ϣ���� */
    pst_mib_info->st_phy_ht.en_dot11ShortGIOptionInTwentyImplemented           = OAL_TRUE;
    mac_mib_set_ShortGIOptionInFortyImplemented(pst_mac_vap, OAL_TRUE);
    pst_mib_info->st_wlan_mib_phy_vht.en_dot11VHTShortGIOptionIn80Implemented  = OAL_TRUE;

    if (WLAN_BAND_WIDTH_20M == en_bandwidth)
    {
        mac_mib_set_FortyMHzOperationImplemented(pst_mac_vap, OAL_FALSE);
        mac_mib_set_ShortGIOptionInFortyImplemented(pst_mac_vap, OAL_FALSE);
        pst_mib_info->st_wlan_mib_phy_vht.en_dot11VHTShortGIOptionIn80Implemented  = OAL_FALSE;
    }
    else if (WLAN_BAND_WIDTH_20M < en_bandwidth && en_bandwidth < WLAN_BAND_WIDTH_80PLUSPLUS)
    {
        pst_mib_info->st_wlan_mib_phy_vht.en_dot11VHTShortGIOptionIn80Implemented  = OAL_FALSE;
    }
}


#ifdef _PRE_WLAN_FEATURE_PROXY_ARP

oal_err_code_enum_uint32 mac_proxy_init_vap(mac_vap_stru  *pst_mac_vap)
{
    pst_mac_vap->pst_vap_proxyarp = OAL_PTR_NULL;
    return OAL_SUCC;
}
#endif



oal_uint32 mac_vap_init(
                mac_vap_stru               *pst_vap,
                oal_uint8                   uc_chip_id,
                oal_uint8                   uc_device_id,
                oal_uint8                   uc_vap_id,
                mac_cfg_add_vap_param_stru *pst_param)
{
    oal_uint32                     ul_loop;
    wlan_mib_ieee802dot11_stru    *pst_mib_info;
    mac_device_stru               *pst_mac_device = mac_res_get_dev(uc_device_id);
    oal_uint32                     ul_cb_ret = OAL_SUCC;/* rom cb��������ֵ */

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_device))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{mac_vap_init::pst_mac_device[%d] null!}", uc_device_id);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_vap->uc_chip_id     = uc_chip_id;
    pst_vap->uc_device_id   = uc_device_id;
    pst_vap->uc_vap_id      = uc_vap_id;
    pst_vap->en_vap_mode    = pst_param->en_vap_mode;
    pst_vap->ul_core_id     = pst_mac_device->ul_core_id;

    pst_vap->bit_has_user_bw_limit  = OAL_FALSE;
    pst_vap->bit_vap_bw_limit   = 0;
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
    pst_vap->bit_voice_aggr     = OAL_TRUE;
#else
    pst_vap->bit_voice_aggr     = OAL_FALSE;
#endif
    pst_vap->bit_one_tx_tcp_be  = OAL_FALSE;

    oal_set_mac_addr_zero(pst_vap->auc_bssid);

    for (ul_loop = 0; ul_loop < MAC_VAP_USER_HASH_MAX_VALUE; ul_loop++)
    {
        oal_dlist_init_head(&(pst_vap->ast_user_hash[ul_loop]));
    }

    /* cache user ����ʼ�� */
    oal_spin_lock_init(&pst_vap->st_cache_user_lock);

    oal_dlist_init_head(&pst_vap->st_mac_user_list_head);

    /* ��ʼ��֧��2.4G 11ac˽����ǿ */
#ifdef _PRE_WLAN_FEATURE_11AC2G
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    pst_vap->st_cap_flag.bit_11ac2g = pst_param->bit_11ac2g_enable;
#else
    pst_vap->st_cap_flag.bit_11ac2g = OAL_TRUE;
#endif
#endif

#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    /* ���ݶ��ƻ�ˢ��2g ht40���� */
    pst_vap->st_cap_flag.bit_disable_2ght40 = pst_param->bit_disable_capab_2ght40;
#else
    pst_vap->st_cap_flag.bit_disable_2ght40 = OAL_FALSE;
#endif

#ifdef _PRE_WLAN_FEATURE_IP_FILTER
    if (IS_STA(pst_vap) && (WLAN_LEGACY_VAP_MODE == pst_param->en_p2p_mode))
    {
        /* ��LEGACY_STA֧�� */
        pst_vap->st_cap_flag.bit_ip_filter = OAL_TRUE;
    }
    else
#endif /* _PRE_WLAN_FEATURE_IP_FILTER */
    {
        pst_vap->st_cap_flag.bit_ip_filter = OAL_FALSE;
    }

    switch(pst_vap->en_vap_mode)
    {
        case WLAN_VAP_MODE_CONFIG :
            return OAL_SUCC;
        case WLAN_VAP_MODE_BSS_STA:
        case WLAN_VAP_MODE_BSS_AP:
            /* ����vap����Ĭ��ֵ */
            pst_vap->uc_assoc_vap_id = 0xff;
            pst_vap->uc_tx_power     = WLAN_MAX_TXPOWER;
            pst_vap->st_protection.en_protection_mode    = WLAN_PROT_NO;

            pst_vap->st_cap_flag.bit_dsss_cck_mode_40mhz = OAL_TRUE;

            /* ��ʼ�����Ա�ʶ */
            pst_vap->st_cap_flag.bit_uapsd      = WLAN_FEATURE_UAPSD_IS_OPEN;
       #ifdef _PRE_WLAN_FEATURE_UAPSD
       #if defined(_PRE_PRODUCT_ID_HI110X_HOST) || defined(_PRE_PRODUCT_ID_HI110X_DEV)
            if (WLAN_VAP_MODE_BSS_AP == pst_vap->en_vap_mode)
            {
                pst_vap->st_cap_flag.bit_uapsd      = g_uc_uapsd_cap;
            }
       #endif
       #endif
            /* ��ʼ��dpd���� */
            pst_vap->st_cap_flag.bit_dpd_enbale = OAL_TRUE;

            pst_vap->st_cap_flag.bit_dpd_done   = OAL_FALSE;
            /* ��ʼ��TDLS prohibited�ر� */
            pst_vap->st_cap_flag.bit_tdls_prohibited                = OAL_FALSE;
            /* ��ʼ��TDLS channel switch prohibited�ر� */
            pst_vap->st_cap_flag.bit_tdls_channel_switch_prohibited = OAL_FALSE;

            /* ��ʼ��KeepALive���� */
            pst_vap->st_cap_flag.bit_keepalive   = OAL_TRUE;
            /* ��ʼ����ȫ����ֵ */
            //pst_vap->uc_80211i_mode              = OAL_FALSE;
            pst_vap->st_cap_flag.bit_wpa         = OAL_FALSE;
            pst_vap->st_cap_flag.bit_wpa2        = OAL_FALSE;

            mac_vap_set_peer_obss_scan(pst_vap, OAL_FALSE);

            //OAL_MEMZERO(&(pst_vap->st_key_mgmt), sizeof(mac_key_mgmt_stru));

            /* ��ʼ��Э��ģʽ�����Ϊ�Ƿ�ֵ����ͨ�������������� */
            pst_vap->st_channel.en_band         = WLAN_BAND_BUTT;
            pst_vap->st_channel.en_bandwidth    = WLAN_BAND_WIDTH_BUTT;
            pst_vap->st_channel.uc_chan_number  = 0;
            pst_vap->en_protocol  = WLAN_PROTOCOL_BUTT;

            /*�����Զ���������*/
            pst_vap->st_protection.bit_auto_protection = OAL_SWITCH_ON;

            OAL_MEMZERO(pst_vap->ast_app_ie, OAL_SIZEOF(mac_app_ie_stru) * OAL_APP_IE_NUM);

            /*���ó�ʼ��rx nssֵ,֮��Э���ʼ�� */
            pst_vap->en_vap_rx_nss = WLAN_NSS_BUTT;

            /* ����VAP״̬Ϊ��ʼ״̬INIT */
            mac_vap_state_change(pst_vap, MAC_VAP_STATE_INIT);

#ifdef _PRE_WLAN_FEATURE_PROXY_ARP
            mac_proxy_init_vap(pst_vap);
#endif /* #ifdef _PRE_WLAN_FEATURE_PROXY_ARP */

/* ��mac vap�µ�uapsd��״̬,����״̬���в���������host device uapsd��Ϣ��ͬ�� */
#ifdef _PRE_WLAN_FEATURE_STA_PM
        OAL_MEMZERO(&(pst_vap->st_sta_uapsd_cfg),OAL_SIZEOF(mac_cfg_uapsd_sta_stru));
#endif/* #ifdef _PRE_WLAN_FEATURE_STA_PM */

#ifdef _PRE_WLAN_FEATURE_P2P
            //pst_vap->uc_p2p0_hal_vap_id     = 0xff;
            //pst_vap->uc_p2p_gocl_hal_vap_id = 0xff;
#endif  /* _PRE_WLAN_FEATURE_P2P */
            break;

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
        case WLAN_VAP_MODE_WDS:
            /* TBD ��ʼ��wds���Ա�ʶ */
            break;
        case WLAN_VAP_MODE_MONITOER :
            /* TBD */
            break;
        case WLAN_VAP_HW_TEST:
            /* TBD */
            break;
#endif
        default :
            OAM_WARNING_LOG1(uc_vap_id, OAM_SF_ANY, "{mac_vap_init::invalid vap mode[%d].}", pst_vap->en_vap_mode);

            return OAL_ERR_CODE_INVALID_CONFIG;
    }
    /* ����MIB�ڴ�ռ䣬����VAPû��MIB */
    if ((WLAN_VAP_MODE_BSS_STA == pst_vap->en_vap_mode) ||
        (WLAN_VAP_MODE_BSS_AP == pst_vap->en_vap_mode)
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
        ||(WLAN_VAP_MODE_WDS == pst_vap->en_vap_mode)
#endif
        )
    {
        pst_vap->pst_mib_info = OAL_MEM_ALLOC(OAL_MEM_POOL_ID_MIB, OAL_SIZEOF(wlan_mib_ieee802dot11_stru), OAL_TRUE);
        if (OAL_PTR_NULL == pst_vap->pst_mib_info)
        {
            OAM_ERROR_LOG1(pst_vap->uc_vap_id, OAM_SF_ANY, "{mac_vap_init::pst_mib_info alloc null, size[%d].}", OAL_SIZEOF(wlan_mib_ieee802dot11_stru));
            return OAL_ERR_CODE_ALLOC_MEM_FAIL;
        }

        pst_mib_info = pst_vap->pst_mib_info;
        OAL_MEMZERO(pst_mib_info, OAL_SIZEOF(wlan_mib_ieee802dot11_stru));

        /* ����mac��ַ */
        oal_set_mac_addr(pst_mib_info->st_wlan_mib_sta_config.auc_dot11StationID, pst_mac_device->auc_hw_addr);
        pst_mib_info->st_wlan_mib_sta_config.auc_dot11StationID[WLAN_MAC_ADDR_LEN - 1] += uc_vap_id;

        /* ��ʼ��mibֵ */
        mac_init_mib(pst_vap);
#ifdef _PRE_WLAN_FEATURE_VOWIFI
        if (WLAN_LEGACY_VAP_MODE == pst_param->en_p2p_mode)
        {
            mac_vap_vowifi_init(pst_vap);
            if (OAL_PTR_NULL != pst_vap->pst_vowifi_cfg_param)
            {
                pst_vap->pst_vowifi_cfg_param->en_vowifi_mode = VOWIFI_MODE_BUTT;
            }
        }
#endif /* _PRE_WLAN_FEATURE_VOWIFI */
#ifdef _PRE_WLAN_FEATURE_TXBF
        pst_vap->st_txbf_add_cap.bit_imbf_receive_cap  = 0;
        pst_vap->st_txbf_add_cap.bit_exp_comp_txbf_cap = OAL_TRUE;
        pst_vap->st_txbf_add_cap.bit_min_grouping      = 0;
        pst_vap->st_txbf_add_cap.bit_csi_bfee_max_rows = 0;
        pst_vap->st_txbf_add_cap.bit_channel_est_cap   = 0;
        pst_vap->bit_ap_11ntxbf                        = 0;
        pst_vap->st_cap_flag.bit_11ntxbf               = 0;
#endif

        /* sta������������� */
        if (WLAN_VAP_MODE_BSS_STA == pst_vap->en_vap_mode)
        {
            /* ��ʼ��staЭ��ģʽΪ11ac */
            switch(pst_mac_device->en_protocol_cap)
            {
                case WLAN_PROTOCOL_CAP_LEGACY:
                case WLAN_PROTOCOL_CAP_HT:
                     pst_vap->en_protocol                = WLAN_HT_MODE;
                     break;

                case WLAN_PROTOCOL_CAP_VHT:
                     pst_vap->en_protocol                = WLAN_VHT_MODE;
                     break;

                default:
                     OAM_WARNING_LOG1(pst_vap->uc_vap_id, OAM_SF_CFG, "{mac_vap_init::en_protocol_cap[%d] is not supportted.}", pst_mac_device->en_protocol_cap);
                     return OAL_ERR_CODE_CONFIG_UNSUPPORT;
            }

            switch(pst_mac_device->en_bandwidth_cap)
            {
                case WLAN_BW_CAP_20M:
                     pst_vap->st_channel.en_bandwidth    = WLAN_BAND_WIDTH_20M;
                     break;

                case WLAN_BW_CAP_40M:
                     pst_vap->st_channel.en_bandwidth    = WLAN_BAND_WIDTH_40MINUS;
                     break;

                case WLAN_BW_CAP_80M:
                case WLAN_BW_CAP_160M:
                     pst_vap->st_channel.en_bandwidth    = WLAN_BAND_WIDTH_80PLUSMINUS;
                     break;

                default:
                     OAM_WARNING_LOG1(pst_vap->uc_vap_id, OAM_SF_CFG, "{mac_vap_init::en_bandwidth_cap[%d] is not supportted.}", pst_mac_device->en_bandwidth_cap);
                     return OAL_ERR_CODE_CONFIG_UNSUPPORT;

            }

            switch(pst_mac_device->en_band_cap)
            {
                case WLAN_BAND_CAP_2G:
                     pst_vap->st_channel.en_band         = WLAN_BAND_2G;
                     break;

                case WLAN_BAND_CAP_5G:
                case WLAN_BAND_CAP_2G_5G:
                     pst_vap->st_channel.en_band         = WLAN_BAND_5G;
                     break;

                default:
                     OAM_WARNING_LOG1(pst_vap->uc_vap_id, OAM_SF_CFG, "{mac_vap_init::en_band_cap[%d] is not supportted.}", pst_mac_device->en_band_cap);
                     return OAL_ERR_CODE_CONFIG_UNSUPPORT;

            }

            if(OAL_SUCC != mac_vap_init_by_protocol(pst_vap, WLAN_VHT_MODE))
            {
                mac_vap_free_mib(pst_vap);
                return OAL_ERR_CODE_INVALID_CONFIG;
            }
            mac_vap_init_rates(pst_vap);
        }
    }

#ifdef _PRE_WLAN_NARROW_BAND
    /* ��ʼ��Ӳ��խ������ */
    pst_vap->st_cap_flag.bit_nb = pst_mac_device->bit_nb_is_supp;
#endif

    if (OAL_RETURN == g_st_mac_vap_rom_cb.p_mac_vap_init(pst_vap, uc_chip_id, uc_device_id, uc_vap_id, pst_param, &ul_cb_ret))
    {
        return ul_cb_ret;
    }

    pst_vap->uc_init_flag = MAC_VAP_VAILD;

    return OAL_SUCC;
}


oal_uint32 mac_vap_set_bssid(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_bssid)
{
    oal_memcopy (pst_mac_vap->auc_bssid, puc_bssid, WLAN_MAC_ADDR_LEN);
    return OAL_SUCC;
}


oal_uint32 mac_vap_set_current_channel(mac_vap_stru *pst_vap, wlan_channel_band_enum_uint8 en_band, oal_uint8 uc_channel)
{
    oal_uint8  uc_channel_idx = 0;
    oal_uint32 ul_ret;

    /* ����ŵ��� */
    ul_ret = mac_is_channel_num_valid(en_band, uc_channel);
    if(OAL_SUCC != ul_ret)
    {
        return ul_ret;
    }

    /* �����ŵ����ҵ������� */
    ul_ret = mac_get_channel_idx_from_num(en_band, uc_channel, &uc_channel_idx);
    if (OAL_SUCC != ul_ret)
    {
        return ul_ret;
    }

    pst_vap->st_channel.uc_chan_number = uc_channel;
    pst_vap->st_channel.en_band        = en_band;
    pst_vap->st_channel.uc_idx         = uc_channel_idx;

    pst_vap->pst_mib_info->st_wlan_mib_phy_dsss.ul_dot11CurrentChannel = uc_channel_idx;
    return OAL_SUCC;
}


oal_uint8  mac_vap_get_curr_baserate(mac_vap_stru *pst_mac_ap,oal_uint8 uc_br_idx)
{
    oal_uint8          uc_loop;
    oal_uint8          uc_found_br_num = 0;
    oal_uint8          uc_rate_num;
    mac_rateset_stru  *pst_rate;

    if (OAL_PTR_NULL == pst_mac_ap)
    {
        return (oal_uint8)OAL_ERR_CODE_PTR_NULL;
    }
    pst_rate = &(pst_mac_ap->st_curr_sup_rates.st_rate);

    uc_rate_num = pst_rate->uc_rs_nrates;

    /* ����base rate ����¼���ҵ��ĸ����������ԱȽϲ����� */
    for (uc_loop = 0; uc_loop < uc_rate_num; uc_loop++)
    {
        if (0 != ((pst_rate->ast_rs_rates[uc_loop].uc_mac_rate) & 0x80))
        {
            if (uc_br_idx == uc_found_br_num)
            {
                return pst_rate->ast_rs_rates[uc_loop].uc_mac_rate;
            }

            uc_found_br_num++;
        }
    }

    /* δ�ҵ������ش��� */
    return OAL_FALSE;
}


oal_void  mac_vap_state_change(mac_vap_stru *pst_mac_vap, mac_vap_state_enum_uint8 en_vap_state)
{
#if IS_HOST
    OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{mac_vap_state_change:from[%d]to[%d]}",pst_mac_vap->en_vap_state,en_vap_state);
#endif
    pst_mac_vap->en_vap_state = en_vap_state;
}


oal_bool_enum_uint8  mac_vap_check_bss_cap_info_phy_ap(oal_uint16 us_cap_info,mac_vap_stru *pst_mac_vap)
{
    mac_cap_info_stru  *pst_cap_info = (mac_cap_info_stru *)(&us_cap_info);

    if (WLAN_BAND_2G != pst_mac_vap->st_channel.en_band)
    {
        return OAL_TRUE;
    }

    /* PBCC */
    if ((OAL_FALSE == pst_mac_vap->pst_mib_info->st_phy_hrdsss.en_dot11PBCCOptionImplemented) &&
       (1 == pst_cap_info->bit_pbcc))
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{mac_vap_check_bss_cap_info_phy_ap::PBCC is different.}");
    }

    /* Channel Agility */
    if ((OAL_FALSE == pst_mac_vap->pst_mib_info->st_phy_hrdsss.en_dot11ChannelAgilityPresent) &&
       (1 == pst_cap_info->bit_channel_agility))
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{mac_vap_check_bss_cap_info_phy_ap::Channel Agility is different.}");
    }

    /* DSSS-OFDM Capabilities */
    if ((OAL_FALSE == pst_mac_vap->pst_mib_info->st_phy_erp.en_dot11DSSSOFDMOptionActivated) &&
       (1 == pst_cap_info->bit_dsss_ofdm))
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "{mac_vap_check_bss_cap_info_phy_ap::DSSS-OFDM Capabilities is different.}");
    }

    return OAL_TRUE;
}


oal_void  mac_vap_get_bandwidth_cap(mac_vap_stru *pst_mac_vap, wlan_bw_cap_enum_uint8 *pen_cap)
{
    mac_channel_stru            *pst_channel;
    wlan_bw_cap_enum_uint8       en_band_cap = WLAN_BW_CAP_20M;

    pst_channel = &(pst_mac_vap->st_channel);

    if (WLAN_BAND_WIDTH_40PLUS == pst_channel->en_bandwidth || WLAN_BAND_WIDTH_40MINUS == pst_channel->en_bandwidth)
    {
        en_band_cap = WLAN_BW_CAP_40M;
    }
    else if (pst_channel->en_bandwidth >= WLAN_BAND_WIDTH_80PLUSPLUS)
    {
        en_band_cap = WLAN_BW_CAP_80M;
    }

    *pen_cap = en_band_cap;
}

oal_uint32 mac_dump_protection(mac_vap_stru *pst_mac_vap, oal_uint8* puc_param)
{
    mac_h2d_protection_stru     *pst_h2d_prot;
    mac_protection_stru         *pst_protection;

    if (OAL_PTR_NULL == puc_param)
    {
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_h2d_prot = (mac_h2d_protection_stru *)puc_param;
    pst_protection = &pst_h2d_prot->st_protection;

    OAM_INFO_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
            "\r\nul_sync_mask=0x%x us_user_idx=%d bit_barker_preamble_mode=%d en_dot11HTProtection=%d\r\n",
            pst_h2d_prot->ul_sync_mask, pst_h2d_prot->us_user_idx, pst_h2d_prot->st_user_cap_info.bit_barker_preamble_mode,
            pst_h2d_prot->en_dot11HTProtection);
    OAM_INFO_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
            "en_dot11RIFSMode=%d en_dot11LSIGTXOPFullProtectionActivated=%d en_dot11NonGFEntitiesPresent=%d en_protection_mode=%d\r\n",
            pst_h2d_prot->en_dot11RIFSMode, pst_h2d_prot->en_dot11LSIGTXOPFullProtectionActivated,
            pst_h2d_prot->en_dot11NonGFEntitiesPresent, pst_protection->en_protection_mode);
    OAM_INFO_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
            "uc_obss_non_erp_aging_cnt=%d uc_obss_non_ht_aging_cnt=%d bit_auto_protection=%d bit_obss_non_erp_present=%d\r\n",
            pst_protection->uc_obss_non_erp_aging_cnt, pst_protection->uc_obss_non_ht_aging_cnt,
            pst_protection->bit_auto_protection, pst_protection->bit_obss_non_erp_present);
    OAM_INFO_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
            "bit_obss_non_ht_present=%d bit_rts_cts_protect_mode=%d bit_lsig_txop_protect_mode=%d uc_sta_no_short_slot_num=%d\r\n",
            pst_protection->bit_obss_non_ht_present, pst_protection->bit_rts_cts_protect_mode,
            pst_protection->bit_lsig_txop_protect_mode, pst_protection->uc_sta_no_short_slot_num);
    OAM_INFO_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
            "uc_sta_no_short_preamble_num=%d uc_sta_non_erp_num=%d uc_sta_non_ht_num=%d uc_sta_non_gf_num=%d\r\n",
            pst_protection->uc_sta_no_short_preamble_num, pst_protection->uc_sta_non_erp_num,
            pst_protection->uc_sta_non_ht_num, pst_protection->uc_sta_non_gf_num);
    OAM_INFO_LOG3(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
            "uc_sta_20M_only_num=%d uc_sta_no_40dsss_cck_num=%d uc_sta_no_lsig_txop_num=%d\r\n",
            pst_protection->uc_sta_20M_only_num, pst_protection->uc_sta_no_40dsss_cck_num,
            pst_protection->uc_sta_no_lsig_txop_num);

    return OAL_SUCC;
}


#if 0
oal_void  mac_vap_set_p2p_channel(mac_vap_stru *pst_mac_vap, mac_channel_stru * pst_p2p_home_channel)
{
    /*
    TBD
    */
}
#endif

wlan_prot_mode_enum_uint8 mac_vap_get_user_protection_mode(mac_vap_stru *pst_mac_vap_sta, mac_user_stru *pst_mac_user)
{
    wlan_prot_mode_enum_uint8           en_protection_mode = WLAN_PROT_NO;

    if ((OAL_PTR_NULL == pst_mac_vap_sta) || (OAL_PTR_NULL == pst_mac_user))
    {
        return en_protection_mode;
    }

    /*��2GƵ���£����AP���͵�beacon֡ERP ie��Use Protection bit��Ϊ1���򽫱�����������ΪERP����*/
    if ((WLAN_BAND_2G == pst_mac_vap_sta->st_channel.en_band) && (OAL_TRUE == pst_mac_user->st_cap_info.bit_erp_use_protect))
    {
        en_protection_mode = WLAN_PROT_ERP;
    }

    /*���AP���͵�beacon֡ht operation ie��ht protection�ֶ�Ϊmixed��non-member���򽫱�����������ΪHT����*/
    else if ((WLAN_MIB_HT_NON_HT_MIXED == pst_mac_user->st_ht_hdl.bit_HT_protection)
              ||(WLAN_MIB_HT_NONMEMBER_PROTECTION == pst_mac_user->st_ht_hdl.bit_HT_protection))
    {
        en_protection_mode = WLAN_PROT_HT;
    }

    /*���AP���͵�beacon֡ht operation ie��non-gf sta present�ֶ�Ϊ1���򽫱�����������ΪGF����*/
    else if (OAL_TRUE == pst_mac_user->st_ht_hdl.bit_nongf_sta_present)
    {
        en_protection_mode = WLAN_PROT_GF;
    }

    /*ʣ�µ������������*/
    else
    {
        en_protection_mode = WLAN_PROT_NO;
    }

    return en_protection_mode;
}

oal_bool_enum mac_protection_lsigtxop_check(mac_vap_stru *pst_mac_vap)
{
    mac_user_stru  *pst_mac_user = OAL_PTR_NULL;

    /*�������11nվ�㣬��֧��lsigtxop����*/
    if ((WLAN_HT_MODE != pst_mac_vap->en_protocol)
        && (WLAN_HT_ONLY_MODE != pst_mac_vap->en_protocol)
        && (WLAN_HT_11G_MODE != pst_mac_vap->en_protocol))
    {
        return OAL_FALSE;
    }

    if (WLAN_VAP_MODE_BSS_STA == pst_mac_vap->en_vap_mode)
    {
        pst_mac_user = (mac_user_stru *)mac_res_get_mac_user(pst_mac_vap->uc_assoc_vap_id); /*user�������AP����Ϣ*/
        if ((OAL_PTR_NULL == pst_mac_user) || (OAL_FALSE == pst_mac_user->st_ht_hdl.bit_lsig_txop_protection_full_support))
        {
            return OAL_FALSE;
        }
        else
        {
            return OAL_TRUE;
        }
    }
    /*lint -e644*/
    /*BSS ������վ�㶼֧��Lsig txop protection, ��ʹ��Lsig txop protection���ƣ�����С, AP��STA���ò�ͬ���ж�*/
    if ((WLAN_VAP_MODE_BSS_AP == pst_mac_vap->en_vap_mode) && (OAL_TRUE == mac_mib_get_LsigTxopFullProtectionActivated(pst_mac_vap)))
    {
        return OAL_TRUE;
    }
    else
    {
        return OAL_FALSE;
    }
    /*lint +e644*/
}

oal_void mac_protection_set_lsig_txop_mechanism(mac_vap_stru *pst_mac_vap, oal_switch_enum_uint8 en_flag)
{
    /*����֡/����֡����ʱ����Ҫ����bit_lsig_txop_protect_modeֵ��д�����������е�L-SIG TXOP enableλ*/
    pst_mac_vap->st_protection.bit_lsig_txop_protect_mode = en_flag;
    OAM_WARNING_LOG1(0,OAM_SF_PWR,"lzhqi mac_protection_set_lsig_txop_mechanism:on[%d]?",en_flag);
}

oal_void mac_protection_set_rts_tx_param(mac_vap_stru *pst_mac_vap,oal_switch_enum_uint8 en_flag,
                                        wlan_prot_mode_enum_uint8 en_prot_mode,mac_cfg_rts_tx_param_stru *pst_rts_tx_param)
{
    if ((OAL_PTR_NULL == pst_mac_vap) || (OAL_PTR_NULL == pst_rts_tx_param))
    {
        OAM_ERROR_LOG2(0, OAM_SF_ASSOC, "{mac_protection_set_rts_tx_param::param null,pst_mac_vap[%p] pst_rts_tx_param[%p]}",
                                     pst_mac_vap ,pst_rts_tx_param);
        return;
    }
    /*ֻ������erp����ʱ��RTS[0~2]���ʲ���Ϊ11Mpbs(11b), ����ʱ��Ϊ24Mpbs(leagcy ofdm) */
    if ((WLAN_PROT_ERP == en_prot_mode) && (OAL_SWITCH_ON == en_flag))
    {
        pst_rts_tx_param->en_band = WLAN_BAND_2G;

        /* RTS[0~2]��Ϊ11Mbps, RTS[3]��Ϊ1Mbps */
        pst_rts_tx_param->auc_protocol_mode[0]    = WLAN_11B_PHY_PROTOCOL_MODE;
        pst_rts_tx_param->auc_rate[0]             = WLAN_SHORT_11b_11_M_BPS;
        pst_rts_tx_param->auc_protocol_mode[1]    = WLAN_11B_PHY_PROTOCOL_MODE;
        pst_rts_tx_param->auc_rate[1]             = WLAN_SHORT_11b_11_M_BPS;
        pst_rts_tx_param->auc_protocol_mode[2]    = WLAN_11B_PHY_PROTOCOL_MODE;
        pst_rts_tx_param->auc_rate[2]             = WLAN_SHORT_11b_11_M_BPS;
        pst_rts_tx_param->auc_protocol_mode[3]    = WLAN_11B_PHY_PROTOCOL_MODE;
        pst_rts_tx_param->auc_rate[3]             = WLAN_LONG_11b_1_M_BPS;
    }
    else
    {
        pst_rts_tx_param->en_band = pst_mac_vap->st_channel.en_band;

        /* RTS[0~2]��Ϊ24Mbps */
        pst_rts_tx_param->auc_protocol_mode[0]    = WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE;
        pst_rts_tx_param->auc_rate[0]             = WLAN_LEGACY_OFDM_24M_BPS;
        pst_rts_tx_param->auc_protocol_mode[1]    = WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE;
        pst_rts_tx_param->auc_rate[1]             = WLAN_LEGACY_OFDM_24M_BPS;
        pst_rts_tx_param->auc_protocol_mode[2]    = WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE;
        pst_rts_tx_param->auc_rate[2]             = WLAN_LEGACY_OFDM_24M_BPS;

        /* 2G��RTS[3]��Ϊ1Mbps */
        if (WLAN_BAND_2G == pst_rts_tx_param->en_band)
        {
            pst_rts_tx_param->auc_protocol_mode[3]    = WLAN_11B_PHY_PROTOCOL_MODE;
            pst_rts_tx_param->auc_rate[3]             = WLAN_LONG_11b_1_M_BPS;
        }
        /* 5G��RTS[3]��Ϊ24Mbps */
        else
        {
            pst_rts_tx_param->auc_protocol_mode[3]    = WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE;
            pst_rts_tx_param->auc_rate[3]             = WLAN_LEGACY_OFDM_24M_BPS;
        }
    }
}
#if (_PRE_WLAN_FEATURE_PMF != _PRE_PMF_NOT_SUPPORT)

OAL_STATIC oal_uint32 mac_vap_init_pmf(mac_vap_stru  *pst_mac_vap,
                                       mac_cfg80211_connect_security_stru *pst_mac_security_param)
{
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_vap) || OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_security_param))
    {
        OAM_ERROR_LOG2(0, OAM_SF_PMF,"mac_11w_init_privacy::Null input,pst_mac_vap[%d],security_param[%d]!!",
                        pst_mac_vap,pst_mac_security_param );
        return OAL_ERR_CODE_PTR_NULL;
    }
    if (OAL_TRUE != mac_mib_get_rsnaactivated(pst_mac_vap))
    {
        return OAL_SUCC;
    }

    switch(pst_mac_security_param->en_pmf_cap)
    {
        case MAC_PMF_DISABLED:
        {
            mac_mib_set_dot11RSNAMFPC(pst_mac_vap, OAL_FALSE);
            mac_mib_set_dot11RSNAMFPR(pst_mac_vap, OAL_FALSE);
        }
        break;
        case MAC_PMF_ENABLED:
        {
            mac_mib_set_dot11RSNAMFPC(pst_mac_vap, OAL_TRUE);
            mac_mib_set_dot11RSNAMFPR(pst_mac_vap, OAL_FALSE);
        }
        break;
        case MAC_PME_REQUIRED:
        {
            mac_mib_set_dot11RSNAMFPC(pst_mac_vap, OAL_TRUE);
            mac_mib_set_dot11RSNAMFPR(pst_mac_vap, OAL_TRUE);
        }
        break;
        default:
        {
            return OAL_FAIL;
        }
    }

    if (MAC_NL80211_MFP_REQUIRED == pst_mac_security_param->en_mgmt_proteced)
    {
        pst_mac_vap->en_user_pmf_cap = OAL_TRUE;
    }
    else
    {
        pst_mac_vap->en_user_pmf_cap = OAL_FALSE;
    }

    return OAL_SUCC;
}

#endif


oal_uint32 mac_vap_add_wep_key(mac_vap_stru *pst_mac_vap, oal_uint16 us_len, oal_uint8 *puc_param)
{
    mac_wep_key_param_stru          *pst_wep_addkey_params = OAL_PTR_NULL;
    mac_user_stru                   *pst_multi_user        = OAL_PTR_NULL;
    wlan_priv_key_param_stru        *pst_wep_key           = OAL_PTR_NULL;
    oal_uint32                       ul_cipher_type        = WLAN_CIPHER_SUITE_WEP40;
    oal_uint8                        uc_wep_cipher_type    = WLAN_80211_CIPHER_SUITE_WEP_40;

    pst_wep_addkey_params = (mac_wep_key_param_stru*)puc_param;

    /*wep ��Կ���Ϊ4��*/
    if(pst_wep_addkey_params->uc_key_index >= WLAN_MAX_WEP_KEY_COUNT)
    {
        return OAL_ERR_CODE_SECURITY_KEY_ID;
    }

    switch (pst_wep_addkey_params->uc_key_len)
    {
        case WLAN_WEP40_KEY_LEN:
            uc_wep_cipher_type = WLAN_80211_CIPHER_SUITE_WEP_40;
            ul_cipher_type     = WLAN_CIPHER_SUITE_WEP40;
            break;
        case WLAN_WEP104_KEY_LEN:
            uc_wep_cipher_type = WLAN_80211_CIPHER_SUITE_WEP_104;
            ul_cipher_type     = WLAN_CIPHER_SUITE_WEP104;
            break;
        default:
            return OAL_ERR_CODE_SECURITY_KEY_LEN;
    }

    /* WEP��Կ��Ϣ��¼���鲥�û���*/
    pst_multi_user = mac_res_get_mac_user(pst_mac_vap->us_multi_user_idx);
    if (OAL_PTR_NULL == pst_multi_user)
    {
        return OAL_ERR_CODE_SECURITY_USER_INVAILD;
    }
    mac_mib_set_privacyinvoked(pst_mac_vap, OAL_TRUE);
    /* ��ʼ��WEP�鲥�����׼� */
    //mac_mib_set_RSNAGroupCipherSelected(pst_mac_vap, uc_wep_cipher_type);

    /*��ʼ���鲥�û��İ�ȫ��Ϣ*/
    if (pst_wep_addkey_params->en_default_key == OAL_TRUE)
    {
        pst_multi_user->st_key_info.en_cipher_type     = uc_wep_cipher_type;
        pst_multi_user->st_key_info.uc_default_index   = pst_wep_addkey_params->uc_key_index;
        pst_multi_user->st_key_info.uc_igtk_key_index  = 0xff;/* wepʱ����Ϊ��Ч */
        pst_multi_user->st_key_info.bit_gtk            = 0;
    }

    pst_wep_key   = &pst_multi_user->st_key_info.ast_key[pst_wep_addkey_params->uc_key_index];

    pst_wep_key->ul_cipher        = ul_cipher_type;
    pst_wep_key->ul_key_len       = (oal_uint32)pst_wep_addkey_params->uc_key_len;

    OAL_MEMZERO(pst_wep_key->auc_key, WLAN_WPA_KEY_LEN);
    oal_memcopy(pst_wep_key->auc_key, pst_wep_addkey_params->auc_wep_key, pst_wep_addkey_params->uc_key_len);

    /* TBD Ų��ȥ ��ʼ���鲥�û��ķ�����Ϣ */
    pst_multi_user->st_user_tx_info.st_security.en_cipher_key_type      = pst_wep_addkey_params->uc_key_index + HAL_KEY_TYPE_PTK;
    pst_multi_user->st_user_tx_info.st_security.en_cipher_protocol_type = uc_wep_cipher_type;
    //OAM_INFO_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_WPA,
    //              "{mac_config_11i_add_wep_key::set multiuser ciphertype[%d] ok}", uc_wep_cipher_type);
    return OAL_SUCC;
}


oal_uint32 mac_vap_init_privacy(mac_vap_stru *pst_mac_vap, mac_cfg80211_connect_security_stru *pst_mac_security_param)
{
    mac_wep_key_param_stru              st_wep_key = {0};
    mac_cfg80211_crypto_settings_stru  *pst_crypto;
    oal_uint32                          ul_ret  = OAL_SUCC;

    mac_mib_set_privacyinvoked(pst_mac_vap, OAL_FALSE);
#if defined (_PRE_WLAN_FEATURE_WPA) || defined(_PRE_WLAN_FEATURE_WPA2)
    /* ��ʼ�� RSNActive ΪFALSE */
    mac_mib_set_rsnaactivated(pst_mac_vap, OAL_FALSE);
#endif
    /* ��������׼���Ϣ */
    mibset_RSNAClearWpaPairwiseCipherImplemented(pst_mac_vap);
    mibset_RSNAClearWpa2PairwiseCipherImplemented(pst_mac_vap);

    pst_mac_vap->st_cap_flag.bit_wpa  = OAL_FALSE;
    pst_mac_vap->st_cap_flag.bit_wpa2 = OAL_FALSE;

    /* ������ */
    if (OAL_FALSE == pst_mac_security_param->en_privacy)
    {
        return OAL_SUCC;
    }

    /* WEP���� */
    if (0 != pst_mac_security_param->uc_wep_key_len)
    {
        st_wep_key.uc_key_len   = pst_mac_security_param->uc_wep_key_len;
        st_wep_key.uc_key_index = pst_mac_security_param->uc_wep_key_index;
        st_wep_key.en_default_key = OAL_TRUE;
        oal_memcopy(st_wep_key.auc_wep_key, pst_mac_security_param->auc_wep_key, WLAN_WEP104_KEY_LEN);
        ul_ret = mac_vap_add_wep_key(pst_mac_vap, OAL_SIZEOF(mac_wep_key_param_stru), (oal_uint8 *)&st_wep_key);
        if (OAL_SUCC != ul_ret)
        {
            OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_WPA,
                           "{mac_11i_init_privacy::mac_config_11i_add_wep_key failed[%d].}", ul_ret);
        }
        return ul_ret;
    }

    /* WPA/WPA2���� */
    pst_crypto = &(pst_mac_security_param->st_crypto);

    if ((pst_crypto->n_ciphers_pairwise > WLAN_PAIRWISE_CIPHER_SUITES) ||
        (pst_crypto->n_akm_suites > WLAN_AUTHENTICATION_SUITES))
    {
        OAM_ERROR_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_WPA,
                       "{mac_11i_init_privacy::cipher_num[%d] akm_num[%d] unexpected.}",
                       pst_crypto->n_ciphers_pairwise, pst_crypto->n_akm_suites);
        return OAL_ERR_CODE_SECURITY_CHIPER_TYPE;
    }

    /* ��ʼ��RSNA mib Ϊ TRUR */
    mac_mib_set_privacyinvoked(pst_mac_vap, OAL_TRUE);
    mac_mib_set_rsnaactivated(pst_mac_vap, OAL_TRUE);

#if (_PRE_WLAN_FEATURE_PMF != _PRE_PMF_NOT_SUPPORT)
    ul_ret = mac_vap_init_pmf(pst_mac_vap, pst_mac_security_param);
    if (OAL_SUCC != ul_ret)
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_WPA,
                       "{mac_11i_init_privacy::mac_11w_init_privacy failed[%d].}", ul_ret);
        return ul_ret;
    }
#endif
    /* ���ü����׼� */
    if (pst_crypto->wpa_versions == WITP_WPA_VERSION_1)
    {
        pst_mac_vap->st_cap_flag.bit_wpa = OAL_TRUE;
        mac_mib_set_wpa_pair_suites(pst_mac_vap, pst_crypto->ciphers_pairwise, MAC_PAIRWISE_CIPHER_SUITES_NUM);
        mac_mib_set_wpa_akm_suites(pst_mac_vap, pst_crypto->akm_suites, MAC_AUTHENTICATION_SUITE_NUM);
        mac_mib_set_wpa_group_suite(pst_mac_vap, pst_crypto->cipher_group);
    }
    else if (pst_crypto->wpa_versions == WITP_WPA_VERSION_2)
    {
        pst_mac_vap->st_cap_flag.bit_wpa2 = OAL_TRUE;
        mac_mib_set_rsn_pair_suites(pst_mac_vap, pst_crypto->ciphers_pairwise, MAC_PAIRWISE_CIPHER_SUITES_NUM);
        mac_mib_set_rsn_akm_suites(pst_mac_vap, pst_crypto->akm_suites, MAC_AUTHENTICATION_SUITE_NUM);
        mac_mib_set_rsn_group_suite(pst_mac_vap, pst_crypto->cipher_group);
        mac_mib_set_rsn_group_mgmt_suite(pst_mac_vap, pst_crypto->uc_group_mgmt_suite);
    }

    OAM_INFO_LOG4(pst_mac_vap->uc_vap_id, OAM_SF_WPA,
                  "{mac_11i_init_privacy::wpa_version[%d], group %d, n_pairwise %d, n_akm %d.}",
                  pst_crypto->wpa_versions,
                  pst_crypto->cipher_group,
                  pst_crypto->n_ciphers_pairwise,
                  pst_crypto->n_akm_suites);
    if (OAL_RETURN == g_st_mac_vap_rom_cb.p_mac_vap_init_privacy(pst_mac_vap, pst_mac_security_param, &ul_ret))
    {
        return ul_ret;
    }
    return OAL_SUCC;
}

oal_void mac_mib_set_wep(mac_vap_stru *pst_mac_vap, oal_uint8 uc_key_id)
{
    /*��ʼ��wep���MIB��Ϣ*/
    if (OAL_PTR_NULL != pst_mac_vap->pst_mib_info)
    {
        oal_memset(pst_mac_vap->pst_mib_info->ast_wlan_mib_wep_dflt_key[uc_key_id].auc_dot11WEPDefaultKeyValue, 0, WLAN_MAX_WEP_STR_SIZE);
        pst_mac_vap->pst_mib_info->ast_wlan_mib_wep_dflt_key[uc_key_id].auc_dot11WEPDefaultKeyValue[WLAN_WEP_SIZE_OFFSET] = 40;
    }
}


oal_uint32 mac_check_group_policy(mac_vap_stru *pst_mac_vap,
                                                  oal_uint8 uc_grp_policy,
                                                  oal_uint8 uc_80211i_mode)
{
    if (DMAC_WPA_802_11I == uc_80211i_mode)
    {
        if (uc_grp_policy != mac_mib_get_wpa_group_suite(pst_mac_vap))
        {
            return OAL_FAIL;
        }
    }
    else if (DMAC_RSNA_802_11I == uc_80211i_mode)
    {
        if (uc_grp_policy != mac_mib_get_rsn_group_suite(pst_mac_vap))
        {
            return OAL_FAIL;
        }
    }
    else
    {
        return OAL_FAIL;
    }

    return OAL_SUCC;
}



oal_uint32 mac_check_auth_policy(mac_vap_stru *pst_mac_vap,
                                                  oal_uint8 *auc_auth_policy,
                                                  oal_uint8 uc_80211i_mode)
{
    if (DMAC_WPA_802_11I == uc_80211i_mode)
    {
        if (0 == mac_mib_wpa_akm_match_suites(pst_mac_vap, auc_auth_policy, WLAN_AUTHENTICATION_SUITES))
        {
            return OAL_FAIL;
        }
    }
    else if (DMAC_RSNA_802_11I == uc_80211i_mode)
    {
        if (0 == mac_mib_rsn_akm_match_suites(pst_mac_vap, auc_auth_policy, WLAN_AUTHENTICATION_SUITES))
        {
            return OAL_FAIL;
        }
    }
    else
    {
        return OAL_FAIL;
    }

    return OAL_SUCC;
}


mac_user_stru *mac_vap_get_user_by_addr(mac_vap_stru *pst_mac_vap, oal_uint8  *puc_mac_addr)
{
    oal_uint32              ul_ret;
    oal_uint16              us_user_idx   = 0xffff;
    mac_user_stru          *pst_mac_user = OAL_PTR_NULL;

    /*����mac addr�ҵ�sta����*/
    ul_ret = mac_vap_find_user_by_macaddr(pst_mac_vap, puc_mac_addr, &us_user_idx);
    if(OAL_SUCC != ul_ret)
    {
        OAM_WARNING_LOG1(0, OAM_SF_ANY, "{mac_vap_get_user_by_addr::find_user_by_macaddr failed[%d].}", ul_ret);
        if(OAL_PTR_NULL != puc_mac_addr)
        {
            OAM_WARNING_LOG3(0, OAM_SF_ANY, "{mac_vap_get_user_by_addr::mac[%x:XX:XX:XX:%x:%x] cant be found!}",
                                    puc_mac_addr[0], puc_mac_addr[4], puc_mac_addr[5]);
        }
        return OAL_PTR_NULL;
    }

    /*����sta�����ҵ�user�ڴ�����*/
    pst_mac_user = (mac_user_stru *)mac_res_get_mac_user(us_user_idx);
    if (OAL_PTR_NULL == pst_mac_user)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{mac_vap_get_user_by_addr::user ptr null.}");
    }

    return pst_mac_user;
}


oal_uint32 mac_vap_add_key(mac_vap_stru *pst_mac_vap, mac_user_stru *pst_mac_user, oal_uint8 uc_key_id, mac_key_params_stru *pst_key)
{
    oal_uint32                            ul_ret;

    switch ((oal_uint8)pst_key->cipher)
    {
        case WLAN_80211_CIPHER_SUITE_WEP_40:
        case WLAN_80211_CIPHER_SUITE_WEP_104:
            /* ����mib */
            mac_mib_set_privacyinvoked(pst_mac_vap, OAL_TRUE);
            mac_mib_set_rsnaactivated(pst_mac_vap, OAL_FALSE);
            //TBD �����鲥��Կ�׼�Ӧ�÷���set default key
            ul_ret = mac_user_add_wep_key(pst_mac_user, uc_key_id, pst_key);
            break;
        case WLAN_80211_CIPHER_SUITE_TKIP:
        case WLAN_80211_CIPHER_SUITE_CCMP:
        case WLAN_80211_CIPHER_SUITE_GCMP:
        case WLAN_80211_CIPHER_SUITE_GCMP_256:
        case WLAN_80211_CIPHER_SUITE_CCMP_256:
            ul_ret = mac_user_add_rsn_key(pst_mac_user, uc_key_id, pst_key);
            break;
        case WLAN_80211_CIPHER_SUITE_BIP:
        case WLAN_80211_CIPHER_SUITE_BIP_GMAC_128:
        case WLAN_80211_CIPHER_SUITE_BIP_GMAC_256:
        case WLAN_80211_CIPHER_SUITE_BIP_CMAC_256:
            ul_ret = mac_user_add_bip_key(pst_mac_user, uc_key_id, pst_key);
            break;
        default:
            return OAL_ERR_CODE_SECURITY_CHIPER_TYPE;
    }

    return ul_ret;
}


oal_uint8 mac_vap_get_default_key_id(mac_vap_stru *pst_mac_vap)
{
    mac_user_stru                *pst_multi_user;
    oal_uint8                     uc_default_key_id;

    /* �������������鲥�û���Կ��Ϣ�в�����Կ */
    pst_multi_user = mac_res_get_mac_user(pst_mac_vap->us_multi_user_idx);
    if (OAL_PTR_NULL == pst_multi_user)
    {
        /* TBD ���ñ������ĵط���û�д��󷵻ش��� */
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_WPA, "{mac_vap_get_default_key_id::multi_user[%d] NULL}", pst_mac_vap->us_multi_user_idx);
        return 0;
    }

    if ((pst_multi_user->st_key_info.en_cipher_type != WLAN_80211_CIPHER_SUITE_WEP_40) &&
        (pst_multi_user->st_key_info.en_cipher_type != WLAN_80211_CIPHER_SUITE_WEP_104))
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_WPA,
                       "{mac_vap_get_default_key_id::unexpectd cipher_type[%d]}", pst_multi_user->st_key_info.en_cipher_type);
        return 0;
    }
    uc_default_key_id = pst_multi_user->st_key_info.uc_default_index;
    if (uc_default_key_id >= WLAN_NUM_TK)
    {
        OAM_ERROR_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_WPA, "{mac_vap_get_default_key_id::unexpectd keyid[%d]}", uc_default_key_id);
        return 0;
    }
    return uc_default_key_id;
}


oal_uint32 mac_vap_set_default_key(mac_vap_stru *pst_mac_vap, oal_uint8  uc_key_index)
{
    wlan_priv_key_param_stru     *pst_wep_key = OAL_PTR_NULL;
    mac_user_stru                *pst_multi_user;

    /* 1.1 �����wep ���ܣ���ֱ�ӷ��� */
    if (OAL_TRUE != mac_is_wep_enabled(pst_mac_vap))
    {
        return OAL_SUCC;
    }

    /* 2.1 �������������鲥�û���Կ��Ϣ�в�����Կ */
    pst_multi_user = mac_res_get_mac_user(pst_mac_vap->us_multi_user_idx);
    if (OAL_PTR_NULL == pst_multi_user)
    {
        return OAL_ERR_CODE_SECURITY_USER_INVAILD;
    }
    pst_wep_key   = &pst_multi_user->st_key_info.ast_key[uc_key_index];

    if (pst_wep_key->ul_cipher != WLAN_CIPHER_SUITE_WEP40 &&
        pst_wep_key->ul_cipher != WLAN_CIPHER_SUITE_WEP104)
    {
        return OAL_ERR_CODE_SECURITY_CHIPER_TYPE;
    }

    /* 3.1 ������Կ���ͼ�default id */
    pst_multi_user->st_key_info.en_cipher_type     = (oal_uint8)(pst_wep_key->ul_cipher);
    pst_multi_user->st_key_info.uc_default_index   = uc_key_index;

    /* 4.1 ����mib���� */
    mac_set_wep_default_keyid(pst_mac_vap, uc_key_index);

    return OAL_SUCC;
}


oal_uint32 mac_vap_set_default_mgmt_key(mac_vap_stru *pst_mac_vap, oal_uint8 uc_key_index)
{
    mac_user_stru                *pst_multi_user;

    /* ����֡������Ϣ�������鲥�û��� */
    pst_multi_user = mac_res_get_mac_user(pst_mac_vap->us_multi_user_idx);
    if (OAL_PTR_NULL == pst_multi_user)
    {
        return OAL_ERR_CODE_SECURITY_USER_INVAILD;
    }

    /* keyidУ�� */
    if (uc_key_index < WLAN_NUM_TK || uc_key_index > WLAN_MAX_IGTK_KEY_INDEX)
    {
        return OAL_ERR_CODE_SECURITY_KEY_ID;
    }

    switch ((oal_uint8)pst_multi_user->st_key_info.ast_key[uc_key_index].ul_cipher)
    {
        case WLAN_80211_CIPHER_SUITE_BIP:
        case WLAN_80211_CIPHER_SUITE_BIP_GMAC_128:
        case WLAN_80211_CIPHER_SUITE_BIP_GMAC_256:
        case WLAN_80211_CIPHER_SUITE_BIP_CMAC_256:
            /* ����IGTK��keyid */
            pst_multi_user->st_key_info.uc_igtk_key_index   = uc_key_index;
            break;
        default:
            return OAL_ERR_CODE_SECURITY_CHIPER_TYPE;
    }
    return OAL_SUCC;
}


void mac_vap_init_user_security_port(mac_vap_stru  *pst_mac_vap,
                                        mac_user_stru *pst_mac_user)
{
    oal_uint8 auc_auth_policy[1] = {WLAN_AUTH_SUITE_1X};
    mac_user_set_port(pst_mac_user, OAL_TRUE);

    if (OAL_TRUE != mac_mib_get_rsnaactivated(pst_mac_vap))
    {
        return;
    }

    /* �Ƿ����WPA ��WPA2 ���ܷ�ʽ */
    if ((OAL_TRUE != pst_mac_vap->st_cap_flag.bit_wpa)
        && (OAL_TRUE != pst_mac_vap->st_cap_flag.bit_wpa2))
    {
        return;
    }

    /* STA��Ҫ����Ƿ����802.1X ��֤��ʽ */
    if ((WLAN_VAP_MODE_BSS_STA == pst_mac_vap->en_vap_mode
        && OAL_TRUE == mac_check_auth_policy(pst_mac_vap, auc_auth_policy, DMAC_RSNA_802_11I))
        || (WLAN_VAP_MODE_BSS_AP == pst_mac_vap->en_vap_mode))
    {
        mac_user_set_port(pst_mac_user, OAL_FALSE);
    }

    //mac_user_init_key(pst_mac_user);
}


oal_uint32 mac_vap_set_beacon(mac_vap_stru *pst_mac_vap, mac_beacon_param_stru *pst_beacon_param)
{
    mac_user_stru *pst_multi_user;
    oal_uint32     ul_ret = OAL_SUCC;

    if ((OAL_PTR_NULL == pst_mac_vap) || (OAL_PTR_NULL == pst_beacon_param))
    {
        OAM_ERROR_LOG0(0, OAM_SF_WPA, "{mac_vap_set_beacon::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* ���֮ǰ�ļ���������Ϣ */
    mac_mib_set_privacyinvoked(pst_mac_vap, OAL_FALSE);
    mac_mib_set_rsnaactivated(pst_mac_vap, OAL_FALSE);
    pst_mac_vap->st_cap_flag.bit_wpa  = OAL_FALSE;
    pst_mac_vap->st_cap_flag.bit_wpa2 = OAL_FALSE;
    mac_mib_set_dot11RSNAMFPR(pst_mac_vap, OAL_FALSE);
    mac_mib_set_dot11RSNAMFPC(pst_mac_vap, OAL_FALSE);
    mac_mib_init_rsnacfg_suites(pst_mac_vap);


    /* ����鲥��Կ��Ϣ */
    pst_multi_user = mac_res_get_mac_user(pst_mac_vap->us_multi_user_idx);
    if (OAL_PTR_NULL == pst_multi_user)
    {
        OAM_ERROR_LOG0(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{mac_vap_set_beacon::pst_multi_user null .}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (OAL_FALSE == pst_beacon_param->en_privacy)
    {
        /* ֻ�ڷǼ��ܳ�������������ܳ������������ø��� */
        mac_user_init_key(pst_multi_user);
        pst_multi_user->st_user_tx_info.st_security.en_cipher_key_type = WLAN_KEY_TYPE_TX_GTK;
        return OAL_SUCC;
    }

    /* ʹ�ܼ��� */
    mac_mib_set_privacyinvoked(pst_mac_vap, OAL_TRUE);

    if (pst_beacon_param->uc_crypto_mode & WLAN_WPA_BIT)
    {
        /* ʹ��WPA */
        pst_mac_vap->st_cap_flag.bit_wpa = OAL_TRUE;
        mac_mib_set_rsnaactivated(pst_mac_vap, OAL_TRUE);

        /* ����WPA���������׼� */
        mac_mib_set_wpa_pair_suites(pst_mac_vap, pst_beacon_param->auc_pairwise_crypto_wpa, MAC_PAIRWISE_CIPHER_SUITES_NUM);

        /* ����WPA�鲥�����׼� */
        mac_mib_set_wpa_group_suite(pst_mac_vap, pst_beacon_param->uc_group_crypto);

        /* ����WPA��֤�׼� */
        mac_mib_set_wpa_akm_suites(pst_mac_vap, pst_beacon_param->auc_auth_type, MAC_AUTHENTICATION_SUITE_NUM);
    }

    if (pst_beacon_param->uc_crypto_mode & WLAN_WPA2_BIT)
    {
        /* ʹ��RSN */
        pst_mac_vap->st_cap_flag.bit_wpa2 = OAL_TRUE;
        mac_mib_set_rsnaactivated(pst_mac_vap, OAL_TRUE);

        /* ����RSN���������׼� */
        mac_mib_set_rsn_pair_suites(pst_mac_vap, pst_beacon_param->auc_pairwise_crypto_wpa2, MAC_PAIRWISE_CIPHER_SUITES_NUM);

        /* ����RSN�鲥�����׼� */
        mac_mib_set_rsn_group_suite(pst_mac_vap, pst_beacon_param->uc_group_crypto);

        /* ����RSN��֤�׼� */
        mac_mib_set_rsn_akm_suites(pst_mac_vap, pst_beacon_param->auc_auth_type, MAC_AUTHENTICATION_SUITE_NUM);

        /* ����RSN�鲥����֡�����׼� */
        mac_mib_set_rsn_group_mgmt_suite(pst_mac_vap, pst_beacon_param->uc_group_mgmt_cipher);

        /* ����RSN���� */
        mac_mib_set_dot11RSNAMFPR(pst_mac_vap, (pst_beacon_param->us_rsn_capability & BIT6) ? OAL_TRUE : OAL_FALSE);
        mac_mib_set_dot11RSNAMFPC(pst_mac_vap, (pst_beacon_param->us_rsn_capability & BIT7) ? OAL_TRUE : OAL_FALSE);
        mac_mib_set_pre_auth_actived(pst_mac_vap, pst_beacon_param->us_rsn_capability & BIT0);
        mac_mib_set_rsnacfg_ptksareplaycounters(pst_mac_vap, (pst_beacon_param->us_rsn_capability & 0x0C) >> 2);
        mac_mib_set_rsnacfg_gtksareplaycounters(pst_mac_vap, (pst_beacon_param->us_rsn_capability & 0x30) >> 4);
    }

    if (OAL_RETURN == g_st_mac_vap_rom_cb.p_mac_vap_set_beacon(pst_mac_vap, pst_beacon_param, &ul_ret))
    {
        return ul_ret;
    }

    return OAL_SUCC;
}


oal_uint8* mac_vap_get_mac_addr(mac_vap_stru *pst_mac_vap)
{
#ifdef _PRE_WLAN_FEATURE_P2P
    if (IS_P2P_DEV(pst_mac_vap))
    {
        /* ��ȡP2P DEV MAC ��ַ����ֵ��probe req ֡�� */
        return pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.auc_p2p0_dot11StationID;
    }
    else
#endif /* _PRE_WLAN_FEATURE_P2P */
    {
        /* ���õ�ַ2Ϊ�Լ���MAC��ַ */
        return pst_mac_vap->pst_mib_info->st_wlan_mib_sta_config.auc_dot11StationID;
    }
}
#ifdef _PRE_WLAN_FEATURE_11R

oal_uint32 mac_mib_init_ft_cfg(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_mde)
{

    if (OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_WPA, "{mac_mib_init_ft_cfg::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (OAL_PTR_NULL == pst_mac_vap->pst_mib_info)
    {
        OAM_ERROR_LOG0(0, OAM_SF_WPA, "{mac_mib_init_ft_cfg::pst_mib_info null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if ((OAL_PTR_NULL == puc_mde) || (puc_mde[0] != MAC_EID_MOBILITY_DOMAIN) || (puc_mde[1] != 3))
    {
        pst_mac_vap->pst_mib_info->st_wlan_mib_fast_bss_trans_cfg.en_dot11FastBSSTransitionActivated = OAL_FALSE;
        return OAL_SUCC;
    }

    pst_mac_vap->pst_mib_info->st_wlan_mib_fast_bss_trans_cfg.en_dot11FastBSSTransitionActivated = OAL_TRUE;
    pst_mac_vap->pst_mib_info->st_wlan_mib_fast_bss_trans_cfg.auc_dot11FTMobilityDomainID[0] = puc_mde[2];
    pst_mac_vap->pst_mib_info->st_wlan_mib_fast_bss_trans_cfg.auc_dot11FTMobilityDomainID[1] = puc_mde[3];
    if (puc_mde[4] & 1)
    {
        pst_mac_vap->pst_mib_info->st_wlan_mib_fast_bss_trans_cfg.en_dot11FTOverDSActivated = OAL_TRUE;
    }
    else
    {
        pst_mac_vap->pst_mib_info->st_wlan_mib_fast_bss_trans_cfg.en_dot11FTOverDSActivated = OAL_FALSE;
    }
    if (puc_mde[4] & 2)
    {
        pst_mac_vap->pst_mib_info->st_wlan_mib_fast_bss_trans_cfg.en_dot11FTResourceRequestSupported = OAL_TRUE;
    }
    else
    {
        pst_mac_vap->pst_mib_info->st_wlan_mib_fast_bss_trans_cfg.en_dot11FTResourceRequestSupported = OAL_FALSE;
    }
    return OAL_SUCC;
}

oal_uint32 mac_mib_get_md_id(mac_vap_stru *pst_mac_vap, oal_uint16 *pus_mdid)
{

    if (OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_WPA, "{mac_mib_init_ft_cfg::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (OAL_PTR_NULL == pst_mac_vap->pst_mib_info)
    {
        OAM_ERROR_LOG0(0, OAM_SF_WPA, "{mac_mib_init_ft_cfg::pst_mib_info null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (OAL_FALSE == pst_mac_vap->pst_mib_info->st_wlan_mib_fast_bss_trans_cfg.en_dot11FastBSSTransitionActivated)
    {
        return OAL_FAIL;
    }

    pst_mac_vap->pst_mib_info->st_wlan_mib_fast_bss_trans_cfg.en_dot11FastBSSTransitionActivated = OAL_TRUE;

    *pus_mdid = *(oal_uint16*)(pst_mac_vap->pst_mib_info->st_wlan_mib_fast_bss_trans_cfg.auc_dot11FTMobilityDomainID);

    return OAL_SUCC;
}
#endif //_PRE_WLAN_FEATURE_11R


#ifdef _PRE_WLAN_FEATURE_VOWIFI

oal_uint32 mac_vap_set_vowifi_param(mac_vap_stru *pst_mac_vap, mac_vowifi_cmd_enum_uint8 en_vowifi_cfg_cmd, oal_uint8 uc_value)
{
    oal_int8 c_value;

    if (OAL_PTR_NULL == pst_mac_vap)
    {
        OAM_ERROR_LOG0(0, OAM_SF_VOWIFI, "{mac_vap_set_vowifi_param::pst_mac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    switch (en_vowifi_cfg_cmd)
    {
        case VOWIFI_SET_MODE:
        {
            /* �쳣ֵ */
            if (uc_value >= VOWIFI_MODE_BUTT)
            {
                OAM_ERROR_LOG1(0, OAM_SF_VOWIFI, "{mac_vap_set_vowifi_param::MODE Value[%d] error!}", uc_value);
                return OAL_ERR_CODE_VOWIFI_SET_INVALID;
            }

            /*  MODE
                0: disable report of rssi change
                1: enable report when rssi lower than threshold(vowifi_low_thres)
                2: enable report when rssi higher than threshold(vowifi_high_thres)
            */
            pst_mac_vap->pst_vowifi_cfg_param->en_vowifi_mode  = uc_value;
            pst_mac_vap->pst_vowifi_cfg_param->uc_cfg_cmd_cnt |= BIT0;

            OAM_WARNING_LOG1(0, OAM_SF_VOWIFI, "{mac_vap_set_vowifi_param::Set vowifi_mode=[%d]!}",
            pst_mac_vap->pst_vowifi_cfg_param->en_vowifi_mode);
            break;
        }
        case VOWIFI_SET_PERIOD:
        {
            /* �쳣ֵ */
            if ((uc_value < MAC_VOWIFI_PERIOD_MIN)||(uc_value > MAC_VOWIFI_PERIOD_MAX))
            {
                OAM_ERROR_LOG1(0, OAM_SF_VOWIFI, "{mac_vap_set_vowifi_param::PERIOD Value[%d] error!}", uc_value);
                return OAL_ERR_CODE_VOWIFI_SET_INVALID;
            }

            /*��λms,��Χ��1s��30s��, the period of monitor the RSSI when host suspended */
            pst_mac_vap->pst_vowifi_cfg_param->us_rssi_period_ms = (oal_uint16)(uc_value *  1000);
            pst_mac_vap->pst_vowifi_cfg_param->uc_cfg_cmd_cnt   |= BIT1;

            break;
        }
        case VOWIFI_SET_LOW_THRESHOLD:
        {
            c_value = (oal_int8)uc_value;
            /* �쳣ֵ */
            if ((c_value < MAC_VOWIFI_LOW_THRESHOLD_MIN)||(c_value > MAC_VOWIFI_LOW_THRESHOLD_MAX))
            {
                OAM_ERROR_LOG1(0, OAM_SF_VOWIFI, "{mac_vap_set_vowifi_param::LOW_THRESHOLD Value[%d] error!}", c_value);
                return OAL_ERR_CODE_VOWIFI_SET_INVALID;
            }

            /* [-1, -100],vowifi_low_thres */
            pst_mac_vap->pst_vowifi_cfg_param->c_rssi_low_thres  = c_value;
            pst_mac_vap->pst_vowifi_cfg_param->uc_cfg_cmd_cnt   |= BIT2;

            break;
        }
        case VOWIFI_SET_HIGH_THRESHOLD:
        {
            c_value = (oal_int8)uc_value;
            /* �쳣ֵ */
            if ((c_value < MAC_VOWIFI_HIGH_THRESHOLD_MIN)||(c_value > MAC_VOWIFI_HIGH_THRESHOLD_MAX))
            {
                OAM_ERROR_LOG1(0, OAM_SF_VOWIFI, "{mac_vap_set_vowifi_param::HIGH_THRESHOLD Value[%d] error!}", c_value);
                return OAL_ERR_CODE_VOWIFI_SET_INVALID;
            }

            /* [-1, -100],vowifi_high_thres */
            pst_mac_vap->pst_vowifi_cfg_param->c_rssi_high_thres = c_value;
            pst_mac_vap->pst_vowifi_cfg_param->uc_cfg_cmd_cnt   |= BIT3;

            break;
        }
        case VOWIFI_SET_TRIGGER_COUNT:
        {
            /* �쳣ֵ */
            if ((uc_value < MAC_VOWIFI_TRIGGER_COUNT_MIN)||(uc_value > MAC_VOWIFI_TRIGGER_COUNT_MAX))
            {
                OAM_ERROR_LOG1(0, OAM_SF_VOWIFI, "{mac_vap_set_vowifi_param::TRIGGER_COUNT Value[%d] error!}", uc_value);
                return OAL_ERR_CODE_VOWIFI_SET_INVALID;
            }

            /*��1��100��, the continuous counters of lower or higher than threshold which will trigger the report to host */
            pst_mac_vap->pst_vowifi_cfg_param->uc_trigger_count_thres = uc_value;
            pst_mac_vap->pst_vowifi_cfg_param->uc_cfg_cmd_cnt        |= BIT4;

            break;
        }
        default:
            OAM_WARNING_LOG1(pst_mac_vap->uc_vap_id, OAM_SF_CFG, "{mac_vap_set_vowifi_param::invalid cmd = %d!!}", en_vowifi_cfg_cmd);

            break;
    }

    /* ���������ռ���ϣ���ʼ��vowifi����ϱ�״̬ */
    if (BIT0 & pst_mac_vap->pst_vowifi_cfg_param->uc_cfg_cmd_cnt)
    {
        pst_mac_vap->pst_vowifi_cfg_param->uc_cfg_cmd_cnt     = 0;
        pst_mac_vap->pst_vowifi_cfg_param->en_vowifi_reported = OAL_FALSE;
    }

    return OAL_SUCC;
}
#endif /* _PRE_WLAN_FEATURE_VOWIFI */


oal_switch_enum_uint8 mac_vap_protection_autoprot_is_enabled(mac_vap_stru *pst_mac_vap)
{
    return pst_mac_vap->st_protection.bit_auto_protection;
}


oal_uint8 mac_vap_get_bandwith(wlan_bw_cap_enum_uint8 en_dev_cap, wlan_channel_bandwidth_enum_uint8 en_bss_cap)
{
    wlan_channel_bandwidth_enum_uint8 en_band_with = WLAN_BAND_WIDTH_20M;

    if (WLAN_BAND_WIDTH_BUTT <= en_bss_cap)
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "mac_vap_get_bandwith:bss cap is invaild en_dev_cap[%d] to en_bss_cap[%d]", en_dev_cap, en_bss_cap);
        return en_band_with;
    }

    switch (en_dev_cap)
    {
        case WLAN_BW_CAP_20M:
            break;

        case WLAN_BW_CAP_40M:
            if (WLAN_BAND_WIDTH_40MINUS >= en_bss_cap)
            {
                en_band_with = en_bss_cap;
            }
            else if ((WLAN_BAND_WIDTH_80PLUSPLUS <= en_bss_cap) && (WLAN_BAND_WIDTH_80PLUSMINUS >= en_bss_cap))
            {
                en_band_with = WLAN_BAND_WIDTH_40PLUS;
            }
            else if ((WLAN_BAND_WIDTH_80MINUSPLUS <= en_bss_cap) && (WLAN_BAND_WIDTH_80MINUSMINUS >= en_bss_cap))
            {
                en_band_with = WLAN_BAND_WIDTH_40MINUS;
            }
            break;

        case WLAN_BW_CAP_80M:
            if (WLAN_BAND_WIDTH_80MINUSMINUS >= en_bss_cap)
            {
                en_band_with = en_bss_cap;
            }
            break;
        default:
            OAM_ERROR_LOG2(0, OAM_SF_ANY, "mac_vap_get_bandwith: bandwith en_dev_cap[%d] to en_bss_cap[%d]", en_dev_cap, en_bss_cap);
            break;
    }

    return en_band_with;
}

#ifdef _PRE_WLAN_FEATURE_FTM

mac_ftm_mode_enum_uint8 mac_check_ftm_enable(mac_vap_stru *pst_mac_vap)
{
    /* �ж���κϷ��� */
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_mac_vap))
    {
        OAM_ERROR_LOG0(0, OAM_SF_FTM, "{mac_check_ftm_enable: input pointer is null!}");
        return MAC_FTM_MODE_BUTT;
    }

    if ((OAL_FALSE == mac_mib_get_FineTimingMsmtInitActivated(pst_mac_vap))
        &&(OAL_FALSE == mac_mib_get_FineTimingMsmtRespActivated(pst_mac_vap)))
    {
        return MAC_FTM_DISABLE_MODE;
    }
    else if((OAL_FALSE == mac_mib_get_FineTimingMsmtInitActivated(pst_mac_vap))
        &&(OAL_TRUE == mac_mib_get_FineTimingMsmtRespActivated(pst_mac_vap)))
    {
        return MAC_FTM_RESPONDER_MODE;
    }
    else if((OAL_TRUE == mac_mib_get_FineTimingMsmtInitActivated(pst_mac_vap))
        &&(OAL_FALSE == mac_mib_get_FineTimingMsmtRespActivated(pst_mac_vap)))
    {
        return MAC_FTM_INITIATOR_MODE;
    }
    else
    {
        return MAC_FTM_MIX_MODE;
    }
}
#endif


oal_void mac_set_wep_key_value(mac_vap_stru *pst_mac_vap, oal_uint8 uc_idx, OAL_CONST oal_uint8 *puc_key, oal_uint8 uc_size)
{
    oal_uint8 *puc_dot11WEPDefaultKeyValue;

    puc_dot11WEPDefaultKeyValue = pst_mac_vap->pst_mib_info->ast_wlan_mib_wep_dflt_key[uc_idx].auc_dot11WEPDefaultKeyValue;
    puc_dot11WEPDefaultKeyValue[WLAN_WEP_SIZE_OFFSET] = uc_size;

    switch(uc_size)
    {
        case 40:
            uc_size = 5;
            break;
        case 104:
            uc_size = 13;
            break;
        default:
            uc_size = 5;
            break;
    }

    oal_memcopy(&puc_dot11WEPDefaultKeyValue[WLAN_WEP_KEY_VALUE_OFFSET], puc_key, uc_size);
}


wlan_ciper_protocol_type_enum_uint8 mac_get_wep_type(mac_vap_stru *pst_mac_vap, oal_uint8 uc_key_id)
{
    wlan_ciper_protocol_type_enum_uint8 en_cipher_type = WLAN_80211_CIPHER_SUITE_NO_ENCRYP;

    switch(mac_get_wep_keysize(pst_mac_vap,uc_key_id))
    {
        case 40:
            en_cipher_type = WLAN_80211_CIPHER_SUITE_WEP_40;
            break;
        case 104:
            en_cipher_type = WLAN_80211_CIPHER_SUITE_WEP_104;
            break;
        default:
            en_cipher_type = WLAN_80211_CIPHER_SUITE_WEP_40;
            break;
    }
    return en_cipher_type;
}


oal_void mac_mib_init_2040(mac_vap_stru *pst_mac_vap)
{
    mac_mib_set_FortyMHzIntolerant(pst_mac_vap, OAL_FALSE);
    mac_mib_set_SpectrumManagementImplemented(pst_mac_vap, OAL_TRUE);
    mac_mib_set_2040BSSCoexistenceManagementSupport(pst_mac_vap, OAL_TRUE);
}


oal_void mac_mib_init_obss_scan(mac_vap_stru *pst_mac_vap)
{
    mac_mib_set_OBSSScanPassiveDwell(pst_mac_vap, 20);
    mac_mib_set_OBSSScanActiveDwell(pst_mac_vap, 10);
    mac_mib_set_BSSWidthTriggerScanInterval(pst_mac_vap, 300);
    mac_mib_set_OBSSScanPassiveTotalPerChannel(pst_mac_vap, 200);
    mac_mib_set_OBSSScanActiveTotalPerChannel(pst_mac_vap, 20);
    mac_mib_set_BSSWidthChannelTransitionDelayFactor(pst_mac_vap, 5);
    mac_mib_set_OBSSScanActivityThreshold(pst_mac_vap, 25);
}


oal_void mac_mib_init_rsnacfg_suites(mac_vap_stru *pst_mac_vap)
{
    wlan_mib_dot11RSNAConfigEntry_stru *pst_wlan_mib_rsna_cfg;

    pst_wlan_mib_rsna_cfg = &(pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg);

    pst_wlan_mib_rsna_cfg->uc_wpa_group_suite      = WLAN_80211_CIPHER_SUITE_TKIP;
    pst_wlan_mib_rsna_cfg->uc_rsn_group_suite      = WLAN_80211_CIPHER_SUITE_CCMP;
    pst_wlan_mib_rsna_cfg->uc_rsn_group_mgmt_suite = WLAN_80211_CIPHER_SUITE_BIP;

    OAL_MEMZERO(pst_wlan_mib_rsna_cfg->auc_wpa_pair_suites, OAL_SIZEOF(pst_wlan_mib_rsna_cfg->auc_wpa_pair_suites));
    pst_wlan_mib_rsna_cfg->auc_wpa_pair_suites[0]  = WLAN_80211_CIPHER_SUITE_CCMP;
    pst_wlan_mib_rsna_cfg->auc_wpa_pair_suites[1]  = WLAN_80211_CIPHER_SUITE_TKIP;

    OAL_MEMZERO(pst_wlan_mib_rsna_cfg->auc_wpa_akm_suites, OAL_SIZEOF(pst_wlan_mib_rsna_cfg->auc_wpa_akm_suites));
    pst_wlan_mib_rsna_cfg->auc_wpa_akm_suites[0]   = WLAN_AUTH_SUITE_PSK;
    pst_wlan_mib_rsna_cfg->auc_wpa_akm_suites[1]   = WLAN_AUTH_SUITE_PSK_SHA256;

    OAL_MEMZERO(pst_wlan_mib_rsna_cfg->auc_rsn_pair_suites, OAL_SIZEOF(pst_wlan_mib_rsna_cfg->auc_rsn_pair_suites));
    pst_wlan_mib_rsna_cfg->auc_rsn_pair_suites[0]  = WLAN_80211_CIPHER_SUITE_CCMP;
    pst_wlan_mib_rsna_cfg->auc_rsn_pair_suites[1]  = WLAN_80211_CIPHER_SUITE_TKIP;

    OAL_MEMZERO(pst_wlan_mib_rsna_cfg->auc_rsn_akm_suites, OAL_SIZEOF(pst_wlan_mib_rsna_cfg->auc_rsn_akm_suites));
    pst_wlan_mib_rsna_cfg->auc_rsn_akm_suites[0]   = WLAN_AUTH_SUITE_PSK;
    pst_wlan_mib_rsna_cfg->auc_rsn_akm_suites[1]   = WLAN_AUTH_SUITE_PSK_SHA256;
}

oal_void mac_mib_set_wpa_pair_suites(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_suites, oal_uint8 uc_cipher_num)
{
    oal_uint8 uc_loop;

    uc_cipher_num = (oal_uint8) OAL_MIN(uc_cipher_num, MAC_PAIRWISE_CIPHER_SUITES_NUM);

    for (uc_loop = 0; uc_loop < uc_cipher_num; uc_loop++)
    {
        pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.auc_wpa_pair_suites[uc_loop] = puc_suites[uc_loop];
    }
}

oal_void mac_mib_set_rsn_pair_suites(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_suites, oal_uint8 uc_cipher_num)
{
    oal_uint8 uc_loop;

    uc_cipher_num = (oal_uint8) OAL_MIN(uc_cipher_num, MAC_PAIRWISE_CIPHER_SUITES_NUM);

    for (uc_loop = 0; uc_loop < uc_cipher_num; uc_loop++)
    {
        pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.auc_rsn_pair_suites[uc_loop] = puc_suites[uc_loop];
    }
}

oal_void mac_mib_set_wpa_akm_suites(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_suites, oal_uint8 uc_akm_num)
{
    oal_uint8 uc_loop;

    uc_akm_num = (oal_uint8) OAL_MIN(uc_akm_num, MAC_AUTHENTICATION_SUITE_NUM);

    for (uc_loop = 0; uc_loop < uc_akm_num; uc_loop++)
    {
        pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.auc_wpa_akm_suites[uc_loop] = puc_suites[uc_loop];
    }
}

oal_void mac_mib_set_rsn_akm_suites(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_suites, oal_uint8 uc_akm_num)
{
    oal_uint8 uc_loop;

    uc_akm_num = (oal_uint8) OAL_MIN(uc_akm_num, MAC_AUTHENTICATION_SUITE_NUM);

    for (uc_loop = 0; uc_loop < uc_akm_num; uc_loop++)
    {
        pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.auc_rsn_akm_suites[uc_loop] = puc_suites[uc_loop];
    }
}

oal_uint8 mac_mib_wpa_pair_match_suites(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_suites, oal_uint8 uc_suite_num)
{
    oal_uint8 uc_idx_local;
    oal_uint8 uc_idx_peer;

    for (uc_idx_local = 0; uc_idx_local < WLAN_PAIRWISE_CIPHER_SUITES; uc_idx_local++)
    {
        for (uc_idx_peer = 0; uc_idx_peer < uc_suite_num; uc_idx_peer++)
        {
            if (pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.auc_wpa_pair_suites[uc_idx_local] == puc_suites[uc_idx_peer])
            {
                return puc_suites[uc_idx_peer];
            }
        }
    }
    return 0;
}

oal_uint8 mac_mib_rsn_pair_match_suites(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_suites, oal_uint8 uc_suite_num)
{
    oal_uint8 uc_idx_local;
    oal_uint8 uc_idx_peer;

    for (uc_idx_local = 0; uc_idx_local < WLAN_PAIRWISE_CIPHER_SUITES; uc_idx_local++)
    {
        for (uc_idx_peer = 0; uc_idx_peer < uc_suite_num; uc_idx_peer++)
        {
            if (pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.auc_rsn_pair_suites[uc_idx_local] == puc_suites[uc_idx_peer])
            {
                return puc_suites[uc_idx_peer];
            }
        }
    }
    return 0;
}

oal_uint8 mac_mib_wpa_akm_match_suites(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_suites, oal_uint8 uc_suite_num)
{
    oal_uint8 uc_idx_local;
    oal_uint8 uc_idx_peer;

    for (uc_idx_local = 0; uc_idx_local < WLAN_AUTHENTICATION_SUITES; uc_idx_local++)
    {
        for (uc_idx_peer = 0; uc_idx_peer < uc_suite_num; uc_idx_peer++)
        {
            if (pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.auc_wpa_akm_suites[uc_idx_local] == puc_suites[uc_idx_peer])
            {
                return puc_suites[uc_idx_peer];
            }
        }
    }
    return 0;
}

/*lint -e661 */
oal_uint8 mac_mib_rsn_akm_match_suites(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_suites, oal_uint8 uc_suite_num)
{
    oal_uint8 uc_idx_local;
    oal_uint8 uc_idx_peer;

    for (uc_idx_local = 0; uc_idx_local < WLAN_AUTHENTICATION_SUITES; uc_idx_local++)
    {
        for (uc_idx_peer = 0; uc_idx_peer < uc_suite_num; uc_idx_peer++)
        {
            if (pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.auc_rsn_akm_suites[uc_idx_local] == puc_suites[uc_idx_peer])
            {
                return puc_suites[uc_idx_peer];
            }
        }
    }
    return 0;
}
/*lint +e661 */

oal_uint8 mac_mib_get_wpa_pair_suites(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_suites)
{
    oal_uint8   uc_loop;
    oal_uint8   uc_num = 0;

    for (uc_loop = 0; uc_loop < WLAN_PAIRWISE_CIPHER_SUITES; uc_loop++)
    {
        if (0 != pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.auc_wpa_pair_suites[uc_loop])
        {
            puc_suites[uc_num++] = pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.auc_wpa_pair_suites[uc_loop];
        }
    }
    return uc_num;
}

oal_uint8  mac_mib_get_rsn_pair_suites(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_suites)
{
    oal_uint8   uc_loop;
    oal_uint8   uc_num = 0;

    for (uc_loop = 0; uc_loop < WLAN_PAIRWISE_CIPHER_SUITES; uc_loop++)
    {
        if (0 != pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.auc_rsn_pair_suites[uc_loop])
        {
            puc_suites[uc_num++] = pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.auc_rsn_pair_suites[uc_loop];
        }
    }
    return uc_num;
}

oal_uint8 mac_mib_get_wpa_akm_suites(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_suites)
{
    oal_uint8   uc_num = 0;
    oal_uint8   uc_loop;

    for (uc_loop = 0; uc_loop < WLAN_AUTHENTICATION_SUITES; uc_loop++)
    {
        if (0 != pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.auc_wpa_akm_suites[uc_loop])
        {
            puc_suites[uc_num++] = pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.auc_wpa_akm_suites[uc_loop];
        }
    }

    return uc_num;
}

oal_uint8 mac_mib_get_rsn_akm_suites(mac_vap_stru *pst_mac_vap, oal_uint8 *puc_suites)
{
    oal_uint8   uc_num = 0;
    oal_uint8   uc_loop;

    for (uc_loop = 0; uc_loop < WLAN_AUTHENTICATION_SUITES; uc_loop++)
    {
        if (0 != pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.auc_rsn_akm_suites[uc_loop])
        {
            puc_suites[uc_num++] = pst_mac_vap->pst_mib_info->st_wlan_mib_privacy.st_wlan_mib_rsna_cfg.auc_rsn_akm_suites[uc_loop];
        }
    }

    return uc_num;
}

/*lint -e19*/
//oal_module_symbol(mac_vap_set_p2p_channel);
#ifdef _PRE_WLAN_FEATURE_UAPSD
oal_module_symbol(g_uc_uapsd_cap);
#endif
oal_module_symbol(mac_vap_set_bssid);
oal_module_symbol(mac_vap_set_current_channel);
oal_module_symbol(mac_vap_init_wme_param);
oal_module_symbol(mac_mib_set_station_id);
oal_module_symbol(mac_vap_state_change);
#if 0
oal_module_symbol(mac_mib_get_station_id);
#endif
oal_module_symbol(mac_mib_set_bss_type);
oal_module_symbol(mac_get_bss_type);
oal_module_symbol(mac_mib_set_ssid);
oal_module_symbol(mac_get_ssid);
oal_module_symbol(mac_mib_set_beacon_period);
oal_module_symbol(mac_get_beacon_period);
oal_module_symbol(mac_mib_set_dtim_period);
#ifdef _PRE_WLAN_FEATURE_UAPSD
oal_module_symbol(mac_vap_set_uapsd_en);
oal_module_symbol(mac_vap_get_uapsd_en);
#endif
#if 0
oal_module_symbol(mac_mib_get_GroupReceivedFrameCount);
oal_module_symbol(mac_mib_set_GroupReceivedFrameCount);
oal_module_symbol(mac_vap_get_user_wme_info);
oal_module_symbol(mac_vap_set_user_wme_info);
#endif
oal_module_symbol(mac_mib_set_shpreamble);
oal_module_symbol(mac_mib_get_shpreamble);
oal_module_symbol(mac_vap_add_assoc_user);
oal_module_symbol(mac_vap_del_user);
oal_module_symbol(mac_vap_init);
oal_module_symbol(mac_vap_exit);
oal_module_symbol(mac_init_mib);
oal_module_symbol(mac_mib_get_ssid);
oal_module_symbol(mac_mib_get_bss_type);
oal_module_symbol(mac_mib_get_beacon_period);
oal_module_symbol(mac_mib_get_dtim_period);
oal_module_symbol(mac_vap_init_rates);
oal_module_symbol(mac_vap_init_by_protocol);
oal_module_symbol(mibset_RSNAClearWpaPairwiseCipherImplemented);
oal_module_symbol(mibset_RSNAClearWpa2PairwiseCipherImplemented);
oal_module_symbol(mac_vap_config_vht_ht_mib_by_protocol);
//oal_module_symbol(mac_mib_set_RSNAAuthenticationSuiteSelected);
oal_module_symbol(mac_vap_check_bss_cap_info_phy_ap);
oal_module_symbol(mac_get_wmm_cfg);
#ifdef _PRE_WLAN_FEATURE_EDCA_MULTI_USER_MULTI_AC
oal_module_symbol(mac_get_wmm_cfg_multi_user_multi_ac);
#endif
oal_module_symbol(mac_vap_get_bandwidth_cap);
oal_module_symbol(mac_vap_change_mib_by_bandwidth);
oal_module_symbol(mac_vap_init_rx_nss_by_protocol);
oal_module_symbol(mac_dump_protection);
oal_module_symbol(mac_vap_set_aid);
oal_module_symbol(mac_vap_set_al_tx_payload_flag);
oal_module_symbol(mac_vap_set_assoc_id);
oal_module_symbol(mac_vap_set_al_tx_flag);
oal_module_symbol(mac_vap_set_tx_power);
oal_module_symbol(mac_vap_set_uapsd_cap);
oal_module_symbol(mac_vap_set_al_tx_first_run);
oal_module_symbol(mac_vap_set_multi_user_idx);
oal_module_symbol(mac_vap_set_wmm_params_update_count);
oal_module_symbol(mac_vap_set_rifs_tx_on);
#if 0
oal_module_symbol(mac_vap_set_tdls_prohibited);
oal_module_symbol(mac_vap_set_tdls_channel_switch_prohibited);
#endif
oal_module_symbol(mac_vap_set_11ac2g);
oal_module_symbol(mac_vap_set_hide_ssid);
oal_module_symbol(mac_get_p2p_mode);
oal_module_symbol(mac_vap_get_peer_obss_scan);
oal_module_symbol(mac_vap_set_peer_obss_scan);
oal_module_symbol(mac_vap_clear_app_ie);
oal_module_symbol(mac_vap_save_app_ie);
oal_module_symbol(mac_vap_set_rx_nss);
oal_module_symbol(mac_vap_find_user_by_macaddr);
oal_module_symbol(mac_vap_get_curr_baserate);

oal_module_symbol(mac_sta_init_bss_rates);

#ifdef _PRE_WLAN_FEATURE_SMPS
oal_module_symbol(mac_mib_get_smps);
oal_module_symbol(mac_vap_get_smps_mode);
oal_module_symbol(mac_vap_get_smps_en);
oal_module_symbol(mac_vap_set_smps);
#endif

oal_module_symbol(mac_device_find_user_by_macaddr);

oal_module_symbol(mac_vap_init_privacy);

oal_module_symbol(mac_mib_set_wep);
oal_module_symbol(mac_check_auth_policy);
oal_module_symbol(mac_vap_get_user_by_addr);
oal_module_symbol(mac_vap_add_key);
oal_module_symbol(mac_vap_get_default_key_id);
oal_module_symbol(mac_vap_set_default_key);
oal_module_symbol(mac_vap_set_default_mgmt_key);
oal_module_symbol(mac_vap_init_user_security_port);
oal_module_symbol(mac_vap_set_beacon);
oal_module_symbol(mac_protection_lsigtxop_check);
oal_module_symbol(mac_protection_set_lsig_txop_mechanism);
oal_module_symbol(mac_vap_get_user_protection_mode);
oal_module_symbol(mac_vap_protection_autoprot_is_enabled);
oal_module_symbol(mac_protection_set_rts_tx_param);
oal_module_symbol(mac_vap_get_bandwith);

#ifdef _PRE_WLAN_FEATURE_VOWIFI
oal_module_symbol(mac_vap_set_vowifi_param);
#endif /* _PRE_WLAN_FEATURE_VOWIFI */


/*lint +e19*/


#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif


