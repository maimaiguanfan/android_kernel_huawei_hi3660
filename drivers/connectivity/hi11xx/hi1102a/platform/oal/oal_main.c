


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "oal_main.h"
#include "oal_workqueue.h"
#include "oal_mem.h"
#include "oal_schedule.h"
#include "oal_net.h"
#if defined(_PRE_PRODUCT_ID_HI110X_HOST)
#include "oal_hcc_host_if.h"
#endif
#include "oal_kernel_file.h"

#if defined(_PRE_PRODUCT_ID_HI110X_HOST)
#ifndef WIN32
#include "plat_firmware.h"
#endif
#endif

#ifdef _PRE_MEM_TRACE
#include "mem_trace.h"
#endif

#define OAL_MAX_TRACE_ENTRY 64

typedef struct
{
    oal_spin_lock_stru  st_lock;
    unsigned long       aul_trace_entries[OAL_MAX_TRACE_ENTRY];
}oal_stacktrace_stru;

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
void __iomem *g_l2cache_base;
#endif

/* 动态/静态DBDC，其中一个默认使能 */
oal_uint8  g_auc_wlan_service_device_per_chip[WLAN_SERVICE_DEVICE_MAX_NUM_PER_CHIP] = {WLAN_INIT_DEVICE_RADIO_CAP};

oal_uint8  g_uc_hitalk_status = 0;      /* 记录当前固件类型 */

/*****************************************************************************
  3 函数实现
*****************************************************************************/
/* 有些内核版本可能没开启CONFIG_STACKTRACE, 增加CONFIG_STACKTRACE的宏 */

#define oal_dump_stack_str_init()
oal_int32 oal_dump_stack_str(oal_uint8 *puc_str, oal_uint32 ul_max_size)
{
    return 0;
}

/*lint -save -e578 -e19 */
DEFINE_GET_BUILD_VERSION_FUNC(oal);
/*lint -restore*/


oal_int32  ATTR_OAL_NO_FUNC_TRACE oal_main_init(oal_void)
{
    oal_uint32  ul_rslt;

    // kernel symbol not exported, find manually
 #if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,34)) && ((_PRE_TARGET_PRODUCT_TYPE_ONT == _PRE_CONFIG_TARGET_PRODUCT))
    extern struct genl_family *g_pst_nl80211_fam;
    extern struct genl_multicast_group *g_pst_nl80211_mlme_mcgrp;

    g_pst_nl80211_fam = (struct genl_family *)kallsyms_lookup_name("nl80211_fam");
    g_pst_nl80211_mlme_mcgrp = (struct genl_multicast_group *)kallsyms_lookup_name("nl80211_mlme_mcgrp");
    if (!g_pst_nl80211_fam || !g_pst_nl80211_mlme_mcgrp)
    {
        OAL_IO_PRINT("find kernel symbol failed:fam=%p mcgrp=%p\n", g_pst_nl80211_fam, g_pst_nl80211_mlme_mcgrp);
        return -OAL_EFAIL;
    }
#endif

#ifdef _PRE_MEM_TRACE
    mem_trace_init();
#endif

#ifdef _PRE_CONFIG_CONN_HISI_SYSFS_SUPPORT
    if(NULL == oal_conn_sysfs_root_obj_init())
    {
        OAL_IO_PRINT("hisi root sysfs init failed\n");
    }
#endif

    /* 为了解各模块的启动时间，增加时间戳打印 */

#if defined(_PRE_PRODUCT_ID_HI110X_HOST) && !defined(CONFIG_HI110X_KERNEL_MODULES_BUILD_SUPPORT) && defined(_PRE_CONFIG_CONN_HISI_SYSFS_SUPPORT)
    /*110X 驱动build in，内存池初始化上移到内核完成，保证大片内存申请成功*/
#else
    /* 内存池初始化 */
    ul_rslt = oal_mem_init_pool();
    if (ul_rslt != OAL_SUCC)
    {
        OAL_IO_PRINT("oal_main_init: oal_mem_init_pool return error code: %d", ul_rslt);
        return -OAL_EFAIL;//lint !e527
    }
#endif

#if ((defined(_PRE_PRODUCT_ID_HI110X_HOST))||(defined(_PRE_PRODUCT_ID_HI110X_DEV)))
    /* Hi1102 SDIO总线初始化接口 TBD */

    /* 初始化: 总线上的chip数量增加1 */
    oal_bus_init_chip_num();
    ul_rslt = oal_bus_inc_chip_num();
    if(OAL_SUCC != ul_rslt)
    {
         OAL_IO_PRINT("oal_pci_probe: oal_bus_inc_chip_num failed!\n");
         return -OAL_EIO;
    }
#endif

    /* 启动成功 */
#if (_PRE_OS_VERSION_WIN32 == _PRE_OS_VERSION)
    OAL_MEMZERO(g_past_net_device, WLAN_VAP_SUPPORT_MAX_NUM_LIMIT * OAL_SIZEOF(oal_net_device_stru *));
#endif

#ifndef BFGX_UART_DOWNLOAD_SUPPORT
#if defined(_PRE_PRODUCT_ID_HI110X_HOST)
    /* HCC初始化 */
    if (OAL_UNLIKELY(OAL_SUCC !=hcc_dev_init()))
    {
        OAL_IO_PRINT("[ERROR]hcc_module_init return err null\n");
        return -OAL_EFAIL;
    }
#if defined(_PRE_PLAT_FEATURE_HI110X_PCIE) && defined(CONFIG_ARCH_SD56XX)
    /*5610 udp pcie chip test*/
    hcc_enable(hcc_get_110x_handler(), OAL_FALSE);
#endif
#endif

#ifdef _PRE_CONFIG_HISI_CONN_SOFTWDFT
    if(OAL_UNLIKELY(OAL_SUCC != oal_softwdt_init()))
    {
        OAL_IO_PRINT("oal_softwdt_init init failed!\n");
        return -OAL_EFAIL;
    }
#endif

#ifdef _PRE_OAL_FEATURE_KEY_PROCESS_TRACE
    if(OAL_UNLIKELY(OAL_SUCC != oal_dft_init()))
    {
        OAL_IO_PRINT("oal_dft_init init failed!\n");
        return -OAL_EFAIL;
    }
#endif
#else
    OAL_IO_PRINT("gnss only version not support wifi hcc\\n");
#endif

    oal_dump_stack_str_init();

    oal_workqueue_init();

#if ((_PRE_CONFIG_TARGET_PRODUCT == _PRE_TARGET_PRODUCT_TYPE_E5) && (_PRE_TEST_MODE != _PRE_TEST_MODE_UT))
    oal_register_syscore_ops();
#endif
    return OAL_SUCC;
}


oal_void  ATTR_OAL_NO_FUNC_TRACE oal_main_exit(oal_void)
{
#ifdef _PRE_OAL_FEATURE_KEY_PROCESS_TRACE
    oal_dft_exit();
#endif

#ifdef _PRE_CONFIG_HISI_CONN_SOFTWDFT
    oal_softwdt_exit();
#endif

#if ((defined(_PRE_PRODUCT_ID_HI110X_HOST))||(defined(_PRE_PRODUCT_ID_HI110X_DEV)))
    /* Hi1102 SDIO总线exit接口(不下电) TBD */

    /* chip num初始化:0 */
    oal_bus_init_chip_num();
#endif

#if defined(_PRE_PRODUCT_ID_HI110X_HOST) && !defined(CONFIG_HI110X_KERNEL_MODULES_BUILD_SUPPORT) && defined(_PRE_CONFIG_CONN_HISI_SYSFS_SUPPORT)
    /*110X 驱动build in，内存池初始化上移到内核完成，保证大片内存申请成功*/
#else
    /* 内存池卸载 */
    oal_mem_exit();
#endif

#ifdef _PRE_CONFIG_CONN_HISI_SYSFS_SUPPORT
    oal_conn_sysfs_root_boot_obj_exit();
    oal_conn_sysfs_root_obj_exit();
#endif
    oal_workqueue_exit();

#if ((_PRE_CONFIG_TARGET_PRODUCT == _PRE_TARGET_PRODUCT_TYPE_E5) && (_PRE_TEST_MODE != _PRE_TEST_MODE_UT))
    oal_unregister_syscore_ops();
#endif

#ifdef _PRE_MEM_TRACE
     mem_trace_exit();
#endif

    return ;
}

#if 0

void ATTR_OAL_NO_FUNC_TRACE __cyg_profile_func_enter(void *this_func, void *call_site)
{

}


void ATTR_OAL_NO_FUNC_TRACE __cyg_profile_func_exit(void *this_func, void *call_site)
{

}
#endif


OAL_STATIC oal_uint8  oal_device_check_enable_num(oal_void)
{
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    oal_uint8 uc_device_num = 0;
    oal_uint8 uc_device_id;

    for (uc_device_id = 0; uc_device_id < WLAN_SERVICE_DEVICE_MAX_NUM_PER_CHIP; uc_device_id++)
    {
        if (g_auc_wlan_service_device_per_chip[uc_device_id])
        {
            uc_device_num++;
        }
    }
    return uc_device_num;
#else
    return WLAN_SERVICE_DEVICE_MAX_NUM_PER_CHIP;
#endif //#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
}


oal_uint8 oal_chip_get_device_num(oal_uint32   ul_chip_ver)
{
    return oal_device_check_enable_num();
}

oal_uint8 oal_board_get_service_vap_start_id(oal_void)
{
    oal_uint8   uc_device_num_per_chip = oal_device_check_enable_num();

    /* 配置vap个数 = mac device个数,vap idx分配先配置vap,后业务vap */
    return (oal_uint8)(WLAN_CHIP_MAX_NUM_PER_BOARD * uc_device_num_per_chip);
}

#if defined(_PRE_PRODUCT_ID_HI110X_HOST)
#ifndef WIN32
oal_void hi_wlan_power_off(void)
{
#ifdef CONFIG_ARCH_SD56XX
#else
    hi_wlan_power_set(0);
#endif
}

oal_void save_nfc_lowpower_log(oal_void)
{
    /*读取nfc低电log数据,然后下电*/
    save_nfc_lowpower_log_2_sdt();
    hi_wlan_power_off();
}
#endif
#endif

/*lint -e578*//*lint -e19*/
oal_module_symbol(oal_dump_stack_str);
oal_module_symbol(oal_chip_get_device_num);
oal_module_symbol(oal_board_get_service_vap_start_id);
oal_module_symbol(oal_main_init);
oal_module_symbol(oal_main_exit);
oal_module_symbol(g_l2cache_base);
oal_module_symbol(g_auc_wlan_service_device_per_chip);
oal_module_symbol(g_uc_hitalk_status);
oal_module_license("GPL");

//oal_module_symbol(__cyg_profile_func_enter);
//oal_module_symbol(__cyg_profile_func_exit);




#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

