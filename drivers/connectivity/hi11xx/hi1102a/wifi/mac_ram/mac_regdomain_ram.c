


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "mac_regdomain.h"
#include "mac_device.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_MAC_REGDOMAIN_RAM_C

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
OAL_CONST mac_supp_mode_table_stru   g_bw_mode_table_2g[] =
{
    /* 1  */    { 2, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS  } },
    /* 2  */    { 2, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS  } },
    /* 3  */    { 2, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS  } },
    /* 4  */    { 2, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS  } },
    /* 5  */    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS, WLAN_BAND_WIDTH_40MINUS  } },
    /* 6  */    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS, WLAN_BAND_WIDTH_40MINUS  } },
    /* 7  */    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS, WLAN_BAND_WIDTH_40MINUS  } },
    /* 8  */    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS, WLAN_BAND_WIDTH_40MINUS  } },
    /* 9  */    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS, WLAN_BAND_WIDTH_40MINUS  } },
    /* 10 */    { 2, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40MINUS } },
    /* 11 */    { 2, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40MINUS } },
    /* 12 */    { 2, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40MINUS } },
    /* 13  */   { 2, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40MINUS } },
    /* 14  */   { 1, { WLAN_BAND_WIDTH_20M } },
};
// see http://support.huawei.com/ecommunity/bbs/10212257.html
OAL_CONST mac_supp_mode_table_stru   g_bw_mode_table_5g[] =
{
    /* 36  */   { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS,  WLAN_BAND_WIDTH_80PLUSPLUS  } },
    /* 40  */   { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40MINUS, WLAN_BAND_WIDTH_80MINUSPLUS } },
    /* 44  */   { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS,  WLAN_BAND_WIDTH_80PLUSMINUS } },
    /* 48  */   { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40MINUS, WLAN_BAND_WIDTH_80MINUSMINUS} },
    /* 52  */   { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS,  WLAN_BAND_WIDTH_80PLUSPLUS  } },
    /* 56  */   { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40MINUS, WLAN_BAND_WIDTH_80MINUSPLUS } },
    /* 60  */   { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS,  WLAN_BAND_WIDTH_80PLUSMINUS } },
    /* 64  */   { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40MINUS, WLAN_BAND_WIDTH_80MINUSMINUS} },

    /* 100 */   { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS,  WLAN_BAND_WIDTH_80PLUSPLUS  } },
    /* 104 */   { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40MINUS, WLAN_BAND_WIDTH_80MINUSPLUS } },
    /* 108 */   { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS,  WLAN_BAND_WIDTH_80PLUSMINUS } },
    /* 112 */   { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40MINUS, WLAN_BAND_WIDTH_80MINUSMINUS} },
    /* 116 */   { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS,  WLAN_BAND_WIDTH_80PLUSPLUS  } },
    /* 120 */   { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40MINUS, WLAN_BAND_WIDTH_80MINUSPLUS } },
    /* 124 */   { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS,  WLAN_BAND_WIDTH_80PLUSMINUS } },
    /* 128 */   { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40MINUS, WLAN_BAND_WIDTH_80MINUSMINUS} },

    /* 132 */   { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS,  WLAN_BAND_WIDTH_80PLUSPLUS} },
    /* 136 */   { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40MINUS, WLAN_BAND_WIDTH_80MINUSPLUS} },
    /* 140 */   { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS,  WLAN_BAND_WIDTH_80PLUSMINUS} },
    /* 144 */   { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40MINUS, WLAN_BAND_WIDTH_80MINUSMINUS} },

    /* 149 */   { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS,  WLAN_BAND_WIDTH_80PLUSPLUS  } },
    /* 153 */   { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40MINUS, WLAN_BAND_WIDTH_80MINUSPLUS } },
    /* 157 */   { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS,  WLAN_BAND_WIDTH_80PLUSMINUS } },
    /* 161 */   { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40MINUS, WLAN_BAND_WIDTH_80MINUSMINUS} },
    /* 165 */   { 1, { WLAN_BAND_WIDTH_20M } },
};

/*****************************************************************************
  3 函数实现
*****************************************************************************/
/*lint -save -e662 */

oal_uint8   mac_regdomain_get_channel_to_bw_mode_idx(oal_uint8 uc_channel_number)
{
    oal_uint8   uc_idx = 0;

    if(0 == uc_channel_number)
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY,"{mac_regdomain_get_channel_to_bw_mode_idx::unknow channel number=%d",uc_channel_number);
        return uc_idx;
    }

    if (uc_channel_number <= 14)
    {
        uc_idx =  uc_channel_number - 1;
    }
    else
    {
        if(uc_channel_number <= 64)
        {
            uc_idx =  (oal_uint8)((oal_uint32)(uc_channel_number - 36) >> 2); // [0,7]
        }
        else if(uc_channel_number <= 144)
        {
            uc_idx =  (oal_uint8)((oal_uint32)(uc_channel_number - 100) >> 2) + 8; //  [8, 19]
        }
        else if(uc_channel_number <= 165)
        {
            uc_idx =  (oal_uint8)((oal_uint32)(uc_channel_number - 149) >> 2) + 20; //  [20, 24]
        }
        else if(uc_channel_number <= 196)
        {
            uc_idx =  (oal_uint8)((oal_uint32)(uc_channel_number - 184) >> 2) + 25; //  [25, 28]
        }
        else
        {
            OAM_WARNING_LOG1(0, OAM_SF_ANY,"{mac_regdomain_get_channel_to_bw_mode_idx::unknow channel=%d, force uc_idx = chan 36",uc_channel_number);
            uc_idx =  0;
        }
    }

    return uc_idx;
}

/*lint -e19*/

oal_bool_enum mac_regdomain_channel_is_support_bw(wlan_channel_bandwidth_enum_uint8 en_cfg_bw, oal_uint8 uc_channel)
{
    oal_uint8                               uc_idx;
    oal_uint8                               uc_bw_loop;
    mac_supp_mode_table_stru                st_supp_mode_table;
    wlan_channel_band_enum_uint8            en_channel_band;

    if(0 == uc_channel)
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY,"{mac_regdomain_channel_is_support_bw::channel not set yet!");
        return OAL_FALSE;
    }

    if(WLAN_BAND_WIDTH_20M == en_cfg_bw)
    {
        return OAL_TRUE;
    }

    en_channel_band = mac_get_band_by_channel_num(uc_channel);
    /*lint -save -e661 */
    uc_idx = mac_regdomain_get_channel_to_bw_mode_idx(uc_channel);
    if((WLAN_BAND_2G == en_channel_band) && (uc_idx < OAL_SIZEOF(g_bw_mode_table_2g)/OAL_SIZEOF(g_bw_mode_table_2g[0])))
    {
        st_supp_mode_table = g_bw_mode_table_2g[uc_idx];
    }
    else if((WLAN_BAND_5G == en_channel_band) &&  (uc_idx < OAL_SIZEOF(g_bw_mode_table_5g)/OAL_SIZEOF(g_bw_mode_table_5g[0])))
    {
        st_supp_mode_table = g_bw_mode_table_5g[uc_idx];
    }
    else
    {
        return OAL_FALSE;
    }

    for(uc_bw_loop = 0; uc_bw_loop < st_supp_mode_table.uc_cnt ; uc_bw_loop++)
    {
        if(en_cfg_bw == st_supp_mode_table.aen_supp_bw[uc_bw_loop])
        {
            return OAL_TRUE;
        }
    }

    return OAL_FALSE;
}
/*lint +e19*/

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

