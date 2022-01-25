


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
#include "hal_ext_if.h"
#include "mac_device.h"
#include "mac_resource.h"
#include "mac_regdomain.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_MAC_DEVICE_C

/*****************************************************************************
  2 ȫ�ֱ�������
*****************************************************************************/
/* macģ����ӵ�ȫ�ֿ��Ʊ��� */
mac_board_stru g_st_mac_board;

mac_device_voe_custom_stru   g_st_mac_voe_custom_param;

/* ÿ��chip�µ�mac device�����������ƻ� */
/* 02��51ÿ��chip��ֻ��1��device��51˫оƬʱÿ��chip����������ȵ� */
/* 03 DBDC����ʱmac device����ȡ2(���Ǿ�̬DBDC), ����ȡ1 */
/* Ĭ�ϳ�ʼ��ֵΪ��̬DBDC��ȡHAL Device0��������ֵ */
mac_device_capability_stru g_st_mac_device_capability[WLAN_SERVICE_DEVICE_MAX_NUM_PER_CHIP] = {
    {
        /* nss num  */                      WLAN_HAL0_NSS_NUM ,  /* ��ֵ����hal device����һ�£����ƻ��򿪺���ͳһˢ��һ�� */
        /* mib_bw_supp_width */             WLAN_HAL0_BW_MAX_WIDTH,  /* TBD:03 ini FPGA 20_40M,ASIC 160M */
        /* NB */                            WLAN_HAL0_NB_IS_EN,
        /* 1024QAM */                       WLAN_HAL0_1024QAM_IS_EN,

        /* 80211 MC */                      WLAN_HAL0_11MC_IS_EN,
        /* ldpc coding */                   WLAN_HAL0_LDPC_IS_EN,
        /* tx stbc */                       WLAN_HAL0_TX_STBC_IS_EN,    /* ��ֵ����hal device����һ�£����ƻ��򿪺���ͳһˢ��һ�� */
        /* rx stbc */                       WLAN_HAL0_RX_STBC_IS_EN,  /* ��ֵ����hal device����һ�£����ƻ��򿪺���ͳһˢ��һ�� */

        /* su bfer */                       WLAN_HAL0_SU_BFER_IS_EN,          /* ��ֵ����hal device����һ�£����ƻ��򿪺���ͳһˢ��һ�� */
        /* su bfee */                       WLAN_HAL0_SU_BFEE_IS_EN,          /* ��ֵ����hal device����һ�£����ƻ��򿪺���ͳһˢ��һ�� */
        /* mu bfer */                       WLAN_HAL0_MU_BFER_IS_EN,          /* ��ֵ����hal device����һ�£����ƻ��򿪺���ͳһˢ��һ�� */
        /* mu bfee */                       WLAN_HAL0_MU_BFEE_IS_EN,          /* ��ֵ����hal device����һ�£����ƻ��򿪺���ͳһˢ��һ�� */
        /* 11ax*/                           WLAN_HAL0_11AX_IS_EN,
    },
#if (WLAN_SERVICE_DEVICE_MAX_NUM_PER_CHIP > 1)
    {
        0,
    }
#endif
};
#if (_PRE_WLAN_FEATURE_BLACKLIST_LEVEL == _PRE_WLAN_FEATURE_BLACKLIST_BOARD)
mac_blacklist_info_stru g_ast_blacklist;
#elif (_PRE_WLAN_FEATURE_BLACKLIST_LEVEL == _PRE_WLAN_FEATURE_BLACKLIST_CHIP)
mac_blacklist_info_stru g_ast_blacklist[WLAN_CHIP_MAX_NUM_PER_BOARD];
#elif (_PRE_WLAN_FEATURE_BLACKLIST_LEVEL == _PRE_WLAN_FEATURE_BLACKLIST_DEVICE)
mac_blacklist_info_stru g_ast_blacklist[WLAN_SERVICE_DEVICE_SUPPORT_MAX_NUM_SPEC];
#elif (_PRE_WLAN_FEATURE_BLACKLIST_LEVEL == _PRE_WLAN_FEATURE_BLACKLIST_VAP)

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
mac_blacklist_info_stru *g_pst_blacklist = OAL_PTR_NULL;
oal_uint8 g_auc_valid_blacklist_idx[WLAN_SERVICE_DEVICE_SUPPORT_MAX_NUM_SPEC]; /* bitmap */
#else
mac_blacklist_info_stru g_ast_blacklist[WLAN_SERVICE_DEVICE_SUPPORT_MAX_NUM_SPEC * WLAN_VAP_SUPPORT_MAX_NUM_LIMIT];
#endif

#endif
#ifdef _PRE_WLAN_FEATURE_NEGTIVE_DET
mac_cfg_pk_mode_stru g_st_wifi_pk_mode_status = {0};   /* PK modeģʽ*/
#endif
/*****************************************************************************
  3 ����ʵ��
*****************************************************************************/
#ifdef _PRE_WLAN_FEATURE_DFS

oal_void  mac_dfs_init(mac_device_stru *pst_mac_device)
{
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    /* ����״���Ĭ�Ϲر� */
    mac_dfs_set_dfs_enable(pst_mac_device, OAL_TRUE);
    /* debugĬ�Ϲر� */
    mac_dfs_set_debug_level(pst_mac_device, 0);
#else
    /* ����״���Ĭ��ʹ�� */
    mac_dfs_set_dfs_enable(pst_mac_device, OAL_TRUE);
    /* debugĬ�ϴ� */
    mac_dfs_set_debug_level(pst_mac_device, 1);
#endif

}
#endif /* #ifdef _PRE_WLAN_FEATURE_DFS */
oal_uint32  mac_board_init_etc(void)
{
    OAL_MEMZERO(g_pst_mac_board, OAL_SIZEOF(mac_board_stru));

#if (_PRE_WLAN_FEATURE_BLACKLIST_LEVEL == _PRE_WLAN_FEATURE_BLACKLIST_VAP) && (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
    g_pst_blacklist = (mac_blacklist_info_stru *)oal_memalloc(WLAN_SERVICE_DEVICE_SUPPORT_MAX_NUM_SPEC * WLAN_SERVICE_AP_MAX_NUM_PER_DEVICE * OAL_SIZEOF(mac_blacklist_info_stru));
    if(OAL_PTR_NULL == g_pst_blacklist)
    {
        OAL_IO_PRINT("[file = %s, line = %d], mac_board_init_etc, memory allocation g_pst_blacklist %u bytes fail!\n",
                        __FILE__, __LINE__, (oal_uint32)(WLAN_SERVICE_DEVICE_SUPPORT_MAX_NUM_SPEC * WLAN_SERVICE_AP_MAX_NUM_PER_DEVICE * sizeof(mac_blacklist_info_stru)));
        return OAL_ERR_CODE_PTR_NULL;
    }
    oal_memset(g_pst_blacklist, 0, WLAN_SERVICE_DEVICE_SUPPORT_MAX_NUM_SPEC * WLAN_SERVICE_AP_MAX_NUM_PER_DEVICE * OAL_SIZEOF(mac_blacklist_info_stru));
#endif

    return OAL_SUCC;
}

oal_uint32  mac_device_init_etc(mac_device_stru *pst_mac_device, oal_uint32 ul_chip_ver, oal_uint8 uc_chip_id, oal_uint8 uc_device_id)
{
    oal_uint8         uc_device_id_per_chip;

    if (OAL_PTR_NULL == pst_mac_device)
    {
       OAM_ERROR_LOG0(0, OAM_SF_ANY, "{mac_device_init_etc::pst_mac_device null.}");

       return OAL_ERR_CODE_PTR_NULL;
    }

    OAL_MEMZERO(pst_mac_device, OAL_SIZEOF(mac_device_stru));

    /* ��ʼ��device������ */
    pst_mac_device->uc_chip_id   = uc_chip_id;
    pst_mac_device->uc_device_id = uc_device_id;

    /* ��ʼ��device�����һЩ���� */
    pst_mac_device->en_max_bandwidth = WLAN_BAND_WIDTH_BUTT;
    pst_mac_device->en_max_band      = WLAN_BAND_BUTT;
    pst_mac_device->uc_max_channel   = 0;
    pst_mac_device->ul_beacon_interval = WLAN_BEACON_INTVAL_DEFAULT;
    pst_mac_device->en_delayed_shift = OAL_FALSE;
    pst_mac_device->en_delayed_shift_down_prot = OAL_FALSE;

#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
    pst_mac_device->st_bss_id_list.us_num_networks = 0;
#endif

    /* 03����ҵ��device,00 01,ȡ��ͬ���ƻ�,51˫оƬ00 11,ȡͬһ�����ƻ�*/
    uc_device_id_per_chip = uc_device_id - uc_chip_id;
    if (uc_device_id_per_chip >= WLAN_SERVICE_DEVICE_MAX_NUM_PER_CHIP)
    {
        OAM_ERROR_LOG3(0, OAM_SF_ANY, "{mac_device_init_etc::pst_mac_device device id[%d] chip id[%d] >support[%d].}",uc_device_id, uc_chip_id, WLAN_SERVICE_DEVICE_MAX_NUM_PER_CHIP);
        uc_device_id_per_chip = 0;
    }

    /* �����ƻ�����Ϣ���浽mac device�ṹ���� */
    /* ��ʼ��mac device������ */
    oal_memcopy(&pst_mac_device->st_device_cap, &g_pst_mac_device_capability[uc_device_id_per_chip], OAL_SIZEOF(mac_device_capability_stru));

#ifdef _PRE_WLAN_FEATURE_SMPS
    /* ����03��˵����һ��deviceֻ֧��SISO,����ΪMIMO SAVE״̬�Ƿ�û��Ӱ�죬��ȷ��; ��Щ������m2s�л��лᶯ̬�仯 */
    MAC_DEVICE_GET_MODE_SMPS(pst_mac_device)  = WLAN_MIB_MIMO_POWER_SAVE_MIMO;
#endif

    pst_mac_device->en_device_state = OAL_TRUE;

#ifdef _PRE_WALN_FEATURE_LUT_RESET
    pst_mac_device->en_reset_switch = OAL_TRUE;
#else
    pst_mac_device->en_reset_switch = OAL_FALSE;
#endif

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
    /* ���ݳ�ʼ��ͨ������ˢ��֧�ֵĿռ����� */
    if ((WLAN_RF_CHANNEL_NUMS == g_l_rf_channel_num) && (WLAN_RF_CHANNEL_ZERO == g_l_rf_single_tran))
    {
        MAC_DEVICE_GET_NSS_NUM(pst_mac_device)  = WLAN_DOUBLE_NSS;
    }
    else
    {
        MAC_DEVICE_GET_NSS_NUM(pst_mac_device) = WLAN_SINGLE_NSS;
    }
#endif

    /* Ĭ�Ϲر�wmm,wmm��ʱ��������Ϊ0 */
    pst_mac_device->en_wmm = OAL_TRUE;

#ifdef _PRE_WLAN_FEATURE_PROXYSTA
    OAL_MEMZERO(&pst_mac_device->st_psta, OAL_SIZEOF(pst_mac_device->st_psta));
#endif

    /* ����оƬ�汾��ʼ��device������Ϣ */
   switch(ul_chip_ver)
   {
        case WLAN_CHIP_VERSION_HI1151V100H:
        #ifdef _PRE_WLAN_FEATURE_11AX
            pst_mac_device->en_protocol_cap  = WLAN_PROTOCOL_CAP_HE ;
            pst_mac_device->en_band_cap      = mac_device_check_5g_enable(uc_device_id) ? WLAN_BAND_CAP_2G_5G : WLAN_BAND_CAP_2G;
        #else
            pst_mac_device->en_protocol_cap  = WLAN_PROTOCOL_CAP_VHT;
            pst_mac_device->en_band_cap      = mac_device_check_5g_enable(uc_device_id) ? WLAN_BAND_CAP_2G_5G : WLAN_BAND_CAP_2G;
            #endif
            break;
        case WLAN_CHIP_VERSION_HI1151V100L:
        #ifdef _PRE_WLAN_FEATURE_11AX
            pst_mac_device->en_protocol_cap  = WLAN_PROTOCOL_CAP_HE ;
            pst_mac_device->en_band_cap      = mac_device_check_5g_enable(uc_device_id) ? WLAN_BAND_CAP_2G_5G : WLAN_BAND_CAP_2G;
        #else
            pst_mac_device->en_protocol_cap  = WLAN_PROTOCOL_CAP_VHT;
            pst_mac_device->en_band_cap      = WLAN_BAND_CAP_2G;
            #endif
            break;

        default:
            OAM_WARNING_LOG1(0, OAM_SF_CFG, "{mac_device_init_etc::ul_chip_ver is not supportted[0x%x].}", ul_chip_ver);
            return OAL_ERR_CODE_CONFIG_UNSUPPORT;
    }

    /* ��ʼ��vap numͳ����Ϣ */
    pst_mac_device->uc_vap_num = 0;
    pst_mac_device->uc_sta_num = 0;
#ifdef _PRE_WLAN_FEATURE_P2P
    pst_mac_device->st_p2p_info.uc_p2p_device_num   = 0;
    pst_mac_device->st_p2p_info.uc_p2p_goclient_num = 0;
    pst_mac_device->st_p2p_info.pst_primary_net_device = OAL_PTR_NULL;/* ��ʼ����net_device Ϊ��ָ�� */
#endif

    /* ��ʼ��Ĭ�Ϲ����� */
    mac_init_regdomain_etc();

    /* ��ʼ���ŵ��б� */
    mac_init_channel_list_etc();

    /* ��ʼ����λ״̬*/
    MAC_DEV_RESET_IN_PROGRESS(pst_mac_device, OAL_FALSE);
    pst_mac_device->us_device_reset_num = 0;

    /* Ĭ�Ϲر�DBAC���� */
#ifdef _PRE_WLAN_FEATURE_DBAC
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
    pst_mac_device->en_dbac_enabled = OAL_FALSE;
#else
    pst_mac_device->en_dbac_enabled = OAL_TRUE;
#endif
#endif

    pst_mac_device->en_dbdc_running = OAL_FALSE;

#ifdef _PRE_SUPPORT_ACS
    oal_memset(&pst_mac_device->st_acs_switch, 0, OAL_SIZEOF(pst_mac_device->st_acs_switch));
#endif

#ifdef _PRE_WLAN_FEATURE_20_40_80_COEXIST
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
    mac_set_2040bss_switch(pst_mac_device, OAL_TRUE);
#else
    mac_set_2040bss_switch(pst_mac_device, OAL_FALSE);
#endif
#endif
    pst_mac_device->uc_in_suspend       = OAL_FALSE;
    pst_mac_device->uc_arpoffload_switch   = OAL_FALSE;

    pst_mac_device->uc_wapi = OAL_FALSE;

    /* AGC��ͨ��Ĭ��Ϊ����Ӧ   */
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
    pst_mac_device->uc_lock_channel = 0x02;
#endif
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
    pst_mac_device->uc_scan_count    = 0;
#endif

    /* ��ʼ�����mac ouiΪ0(3���ֽڶ���0),ȷ��ֻ��Android�·���Чmac oui�Ž������mac��ַɨ��(�����macɨ�迪�ش򿪵������) */
    pst_mac_device->en_is_random_mac_addr_scan = OAL_FALSE;
    pst_mac_device->auc_mac_oui[0] = 0x0;
    pst_mac_device->auc_mac_oui[1] = 0x0;
    pst_mac_device->auc_mac_oui[2] = 0x0;

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
    // FCSԤ���ڴ棬��Ҫ�Ҹ�Ӳ��������ʹ�ö�̬�ڴ棬�Լ���1151
    pst_mac_device->pst_fcs_cfg = oal_memalloc(OAL_SIZEOF(mac_fcs_cfg_stru));
    if (!pst_mac_device->pst_fcs_cfg)
    {
        OAM_WARNING_LOG0(0, OAM_SF_ANY, "{mac_device_init_etc::alloc fcs_cfg failed, abort.}");
        return OAL_FAIL;
    }
#endif

#ifdef _PRE_WLAN_FEATURE_DFS
    mac_dfs_init(pst_mac_device);
#endif /* #ifdef _PRE_WLAN_FEATURE_DFS */

    return OAL_SUCC;
}


oal_uint32  mac_device_exit_etc(mac_device_stru *pst_device)
{
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_device))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{dmac_device_exit::pst_device null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_device->uc_vap_num = 0;
    pst_device->uc_sta_num = 0;
    pst_device->st_p2p_info.uc_p2p_device_num   = 0;
    pst_device->st_p2p_info.uc_p2p_goclient_num = 0;

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
    if (pst_device->pst_fcs_cfg)
    {
        oal_free(pst_device->pst_fcs_cfg);
        pst_device->pst_fcs_cfg = OAL_PTR_NULL;
    }
#endif

#if (_PRE_WLAN_FEATURE_BLACKLIST_LEVEL == _PRE_WLAN_FEATURE_BLACKLIST_DEVICE)
    OAL_MEMZERO(&(g_ast_blacklist[pst_device->uc_device_id]), OAL_SIZEOF(mac_blacklist_info_stru));
#endif

    mac_res_free_dev_etc(pst_device->uc_device_id);

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
    mac_device_set_state_etc(pst_device, OAL_FALSE);
#endif

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_11AX

oal_uint8  mac_device_trans_bandwith_to_he_capinfo(wlan_bw_cap_enum_uint8 en_max_op_bd)
{
    oal_uint8   uc_he_band_width_set = 0;
    switch (en_max_op_bd)
    {
        case WLAN_BW_CAP_20M:
            return uc_he_band_width_set;
        case WLAN_BW_CAP_40M:
            uc_he_band_width_set |= 0x1;/*Bit0:ָʾ2.4G֧��40MHz */
            return uc_he_band_width_set;
        case WLAN_BW_CAP_80M:
        case WLAN_BW_CAP_160M:
        case WLAN_BW_CAP_80PLUS80:
            uc_he_band_width_set |= 0x1;/*Bit0:ָʾ2.4G֧��40MHz */
            uc_he_band_width_set |= 0x2;/*Bit1:ָʾ5G֧��80MHz */
                                        /*160MHz ax ��֧��*/
            return uc_he_band_width_set;
        default:
            OAM_ERROR_LOG1(0, OAM_SF_ANY, "{mac_device_trans_bandwith_to_he_capinfo::bandwith[%d] is invalid.}", en_max_op_bd);
            return WLAN_MIB_VHT_SUPP_WIDTH_BUTT;
    }
}
#endif


oal_uint32  mac_chip_exit_etc(mac_board_stru *pst_board, mac_chip_stru *pst_chip)
{
#if 0
    mac_device_stru   *pst_dev;
    oal_uint32         ul_ret;
    oal_uint8          uc_device;
#endif
    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_chip || OAL_PTR_NULL == pst_board))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{hmac_chip_init::param null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }

#if (_PRE_WLAN_FEATURE_BLACKLIST_LEVEL == _PRE_WLAN_FEATURE_BLACKLIST_CHIP)
    OAL_MEMZERO(&(g_ast_blacklist[pst_chip->uc_chip_id]), OAL_SIZEOF(mac_blacklist_info_stru));
#endif

    /*����Device����ṹ�ͷ�*/
#if 0
    for (uc_device = 0; uc_device < pst_chip->uc_device_nums; uc_device++)
    {
         pst_dev = mac_res_get_dev_etc(pst_chip->auc_device_id[uc_device]);
         ul_ret = pst_board->p_device_destroy_fun(pst_dev);
         if (OAL_SUCC != ul_ret)
         {
             OAM_WARNING_LOG1(0, OAM_SF_ANY, "{mac_chip_exit_etc::p_device_destroy_fun failed[%d].}", ul_ret);

             return ul_ret;
         }
    }
#endif

#ifdef _PRE_WLAN_FEATURE_USER_EXTEND
    if (OAL_TRUE == pst_chip->st_active_user_timer.en_is_registerd)
    {
        FRW_TIMER_IMMEDIATE_DESTROY_TIMER(&pst_chip->st_active_user_timer);
    }
#endif
#ifdef  _PRE_WLAN_FEATURE_RX_AGGR_EXTEND
    if(OAL_PTR_NULL != pst_chip->pst_rx_aggr_extend)
    {
        oal_free(pst_chip->pst_rx_aggr_extend);
        pst_chip->pst_rx_aggr_extend = OAL_PTR_NULL;
    }
#endif
    pst_chip->uc_device_nums = 0;

    /* destroy�������״̬��ΪFALSE */
    pst_chip->en_chip_state  = OAL_FALSE;

    return OAL_SUCC;
}



oal_uint32  mac_board_exit_etc(mac_board_stru *pst_board)
{
#if 0
    oal_uint8  uc_chip_idx;
    oal_uint32 ul_ret;
#endif

#if (_PRE_WLAN_FEATURE_BLACKLIST_LEVEL == _PRE_WLAN_FEATURE_BLACKLIST_VAP) && (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
    if(g_pst_blacklist != OAL_PTR_NULL)
    {
        oal_free(g_pst_blacklist);
        g_pst_blacklist = OAL_PTR_NULL;
    }
#endif

    if (OAL_UNLIKELY(OAL_PTR_NULL == pst_board))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{mac_board_exit_etc::pst_board null.}");

        return OAL_ERR_CODE_PTR_NULL;
    }
#if (_PRE_WLAN_FEATURE_BLACKLIST_LEVEL == _PRE_WLAN_FEATURE_BLACKLIST_BOARD)
    OAL_MEMZERO(&g_ast_blacklist, OAL_SIZEOF(mac_blacklist_info_stru));
#endif
#if 0
    while (0 != pst_board->uc_chip_id_bitmap)
    {
        /* ��ȡ���ұ�һλΪ1��λ������ֵ��Ϊchip�������±� */
        uc_chip_idx = oal_bit_find_first_bit_one_byte(pst_board->uc_chip_id_bitmap);
        if (OAL_UNLIKELY(uc_chip_idx >= WLAN_CHIP_MAX_NUM_PER_BOARD))
        {
            MAC_ERR_LOG2(0, "uc_chip_idx is exceeded support spec.", uc_chip_idx, pst_board->uc_chip_id_bitmap);
            OAM_ERROR_LOG2(0, OAM_SF_ANY, "{mac_board_exit_etc::invalid uc_chip_idx[%d] uc_chip_id_bitmap=%d.}",
                           uc_chip_idx, pst_board->uc_chip_id_bitmap);

            return OAL_ERR_CODE_ARRAY_OVERFLOW;
        }

        ul_ret = mac_chip_exit_etc(pst_board, &pst_board->ast_chip[uc_chip_idx]);
        if (OAL_SUCC != ul_ret)
        {
            MAC_WARNING_LOG1(0, "mac_chip_destroy return fail.", ul_ret);
            OAM_WARNING_LOG1(0, OAM_SF_ANY, "{mac_board_exit_etc::mac_chip_exit_etc failed[%d].}", ul_ret);

            return ul_ret;
        }

        /* �����Ӧ��bitmapλ */
        oal_bit_clear_bit_one_byte(&pst_board->uc_chip_id_bitmap, uc_chip_idx);
    }
#endif

    return OAL_SUCC;
}
oal_void mac_device_set_dfr_reset_etc(mac_device_stru *pst_mac_device, oal_uint8 uc_device_reset_in_progress)
{
    pst_mac_device->uc_device_reset_in_progress = uc_device_reset_in_progress;
}

oal_void  mac_device_set_state_etc(mac_device_stru *pst_mac_device, oal_uint8 en_device_state)
{
    pst_mac_device->en_device_state = en_device_state;
}

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)

oal_uint8 mac_get_valid_blacklist_index(oal_uint8 uc_bit_map)
{
    oal_uint8 uc_index = 0;
    oal_uint8 uc_tmp = uc_bit_map;

    while (uc_tmp & BIT0)
    {
        uc_tmp = uc_tmp >> 1;
        uc_index++;
    }

    return uc_index;
}
#endif

#if (_PRE_WLAN_FEATURE_BLACKLIST_LEVEL != _PRE_WLAN_FEATURE_BLACKLIST_NONE)

oal_void mac_blacklist_get_pointer(wlan_vap_mode_enum_uint8 en_vap_mode, oal_uint8 uc_chip_id, oal_uint8 uc_dev_id, oal_uint8 uc_vap_id, mac_blacklist_info_stru **pst_blacklist_info)
{
#if (_PRE_WLAN_FEATURE_BLACKLIST_LEVEL == _PRE_WLAN_FEATURE_BLACKLIST_VAP)
    oal_uint8      uc_device_index;
    oal_uint8      uc_vap_index;
    oal_uint16     us_array_index = 0;
#endif

#if (_PRE_WLAN_FEATURE_BLACKLIST_LEVEL == _PRE_WLAN_FEATURE_BLACKLIST_BOARD)
    *pst_blacklist_info = &g_ast_blacklist;
#elif (_PRE_WLAN_FEATURE_BLACKLIST_LEVEL == _PRE_WLAN_FEATURE_BLACKLIST_CHIP)
    *pst_blacklist_info = &g_ast_blacklist[uc_chip_id];
#elif (_PRE_WLAN_FEATURE_BLACKLIST_LEVEL == _PRE_WLAN_FEATURE_BLACKLIST_DEVICE)
    *pst_blacklist_info = &g_ast_blacklist[uc_dev_id];
#elif (_PRE_WLAN_FEATURE_BLACKLIST_LEVEL == _PRE_WLAN_FEATURE_BLACKLIST_VAP)
    uc_device_index = uc_dev_id;

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
    uc_vap_index = mac_get_valid_blacklist_index(g_auc_valid_blacklist_idx[uc_device_index]);
#else
    uc_vap_index = uc_vap_id;
#endif

    if (WLAN_VAP_MODE_BSS_AP == en_vap_mode || WLAN_VAP_MODE_BSS_STA == en_vap_mode)
    {
        us_array_index = uc_device_index * WLAN_VAP_SUPPORT_MAX_NUM_LIMIT + uc_vap_index;
        if (us_array_index >= WLAN_SERVICE_DEVICE_SUPPORT_MAX_NUM_SPEC * WLAN_VAP_SUPPORT_MAX_NUM_LIMIT)
        {
            OAM_ERROR_LOG2(uc_vap_index, OAM_SF_ANY, "{mac_blacklist_get_pointer::en_vap_mode=%d, vap index=%d is wrong.}", en_vap_mode, us_array_index);
            return;
        }
    }
    else
    {
        OAM_ERROR_LOG1(uc_vap_index, OAM_SF_ANY, "{mac_blacklist_get_pointer::en_vap_mode=%d is wrong.}", en_vap_mode);
        return;
    }

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
    *pst_blacklist_info = &g_pst_blacklist[us_array_index];
    g_auc_valid_blacklist_idx[uc_device_index] |= (oal_uint8)(1U << uc_vap_index);
    g_pst_blacklist[us_array_index].uc_blacklist_vap_index = uc_vap_index;
    g_pst_blacklist[us_array_index].uc_blacklist_device_index = uc_device_index;
#else
    *pst_blacklist_info = &g_ast_blacklist[us_array_index];
    //g_auc_valid_blacklist_idx[uc_device_index] |= (oal_uint8)(1U << uc_vap_index);
    g_ast_blacklist[us_array_index].uc_blacklist_vap_index = uc_vap_index;
    g_ast_blacklist[us_array_index].uc_blacklist_device_index = uc_device_index;
#endif

#endif
}
#endif

/*lint -e19*/
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
#if (_PRE_WLAN_FEATURE_BLACKLIST_LEVEL == _PRE_WLAN_FEATURE_BLACKLIST_VAP)
oal_module_symbol(g_pst_blacklist);
#elif (_PRE_WLAN_FEATURE_BLACKLIST_LEVEL != _PRE_WLAN_FEATURE_BLACKLIST_NONE)
oal_module_symbol(g_ast_blacklist);
#endif

#if (_PRE_WLAN_FEATURE_BLACKLIST_LEVEL == _PRE_WLAN_FEATURE_BLACKLIST_VAP)
oal_module_symbol(g_auc_valid_blacklist_idx);
#endif
#endif
#if (_PRE_WLAN_FEATURE_BLACKLIST_LEVEL != _PRE_WLAN_FEATURE_BLACKLIST_NONE)
oal_module_symbol(mac_blacklist_get_pointer);
#endif
#ifdef _PRE_WLAN_FEATURE_DFS
oal_module_symbol(mac_dfs_init);
#endif /* #ifdef _PRE_WLAN_FEATURE_DFS */
oal_module_symbol(mac_device_exit_etc);
oal_module_symbol(mac_chip_exit_etc);
oal_module_symbol(mac_board_exit_etc);
oal_module_symbol(mac_device_set_dfr_reset_etc);
oal_module_symbol(mac_device_set_state_etc);
oal_module_symbol(mac_board_init_etc);
oal_module_symbol(mac_device_init_etc);

/*lint +e19*/

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

