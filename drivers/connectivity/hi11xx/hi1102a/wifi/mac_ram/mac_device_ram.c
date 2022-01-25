

#ifdef __cplusplus
#if __cplusplus
    extern "C" {
#endif
#endif

/*****************************************************************************
  1 ͷ�ļ�����
*****************************************************************************/
#include "mac_device.h"

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102A_HOST) && (_PRE_OS_VERSION == _PRE_OS_VERSION_LINUX)
#include "board.h"
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_MAC_DEVICE_RAM_C

/*****************************************************************************
  2 ȫ�ֱ�������
*****************************************************************************/
mac_phy_debug_switch_stru  g_st_mac_phy_debug_switch = {0};

/*****************************************************************************
  3 ����ʵ��
*****************************************************************************/


oal_rom_cb_result_enum_uint8 mac_device_init_cb(mac_device_stru *pst_mac_device,
                                                        oal_uint32       ul_chip_ver,
                                                        oal_uint8        uc_chip_id,
                                                        oal_uint8        uc_device_id,
                                                        oal_uint32      *pul_cb_ret)
{
    *pul_cb_ret = OAL_SUCC;

    return OAL_CONTINUE;
}

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102A_HOST) && (_PRE_OS_VERSION == _PRE_OS_VERSION_LINUX)

wlan_bw_cap_enum_uint8 mac_device_max_band_cb(oal_void)
{
    /* Host��ֻ��ʹ�ú����ж�FPGA/ASIC, Device����patch�ж� */
    if (VERSION_ASIC == isAsic())
    {
        return WLAN_BW_CAP_80M;
    }
    else if (VERSION_FPGA == isAsic())
    {
        return WLAN_BW_CAP_40M;
    }
    else
    {
        return WLAN_BW_CAP_80M;
    }
}
#else

wlan_bw_cap_enum_uint8 mac_device_max_band_cb(oal_void)
{
    /* Device���ʹ��ASIC���ж� */
#if (_PRE_WLAN_CHIP_ASIC != _PRE_WLAN_CHIP_VERSION)
    return WLAN_BW_CAP_40M;
#else
    return WLAN_BW_CAP_80M;
#endif
}
#endif

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif


