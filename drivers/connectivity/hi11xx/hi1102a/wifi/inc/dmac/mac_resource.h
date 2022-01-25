

#ifndef __MAC_RESOURCE_H__
#define __MAC_RESOURCE_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "oal_ext_if.h"
#include "oal_queue.h"
#include "mac_device.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_MAC_RESOURCE_H

/*****************************************************************************
  2 宏定义
*****************************************************************************/
/* mac_vap_stru, hmac_vap_stru私有部分, dmac_vap_stru私有部分总规格 */
#ifdef _PRE_WLAN_FEATURE_CUSTOM_SECURITY
#ifdef _PRE_WLAN_FEATURE_HILINK
#define MAC_RES_VAP_SIZE        4332 //(2600 + 1232)     /* lm: hmac vap结构体中增加变量定时器，结果大小超出规格，因此扩充规格大小,0215 llz:hilink 需增加size */
#else
#define MAC_RES_VAP_SIZE        4232 //(2600 + 1232)     /* lm: hmac vap结构体中增加变量定时器，结果大小超出规格，因此扩充规格大小 */
#endif
#else
#define MAC_RES_VAP_SIZE        2532
#endif

        /* mac_user_stru, hmac_user_stru私有部分, dmac_user_stru私有部分总规格，必须四字节对齐 */
#ifdef _PRE_WLAN_FEATURE_TX_CLASSIFY_LAN_TO_WLAN
#define FEATURE_TX_CLASSIFY_LAN_TO_WLAN_RES_SIZE 1700 /* 增加业务识别功能后，hmac_user_stru结构体中增加用户业务信息，扩充规格为1700 */
#else
#define FEATURE_TX_CLASSIFY_LAN_TO_WLAN_RES_SIZE 0
#endif  /* end of _PRE_WLAN_FEATURE_TX_CLASSIFY_LAN_TO_WLAN */

#define MAC_RES_USER_SIZE       (3724 + FEATURE_TX_CLASSIFY_LAN_TO_WLAN_RES_SIZE + 240) /* 原来为3560,后增大结构体成员后为3640,预留60（原因：51双核加载dmac出现改结构体过大）*/

/* 最大dev数量 */
#define MAC_RES_MAX_DEV_NUM     (WLAN_CHIP_MAX_NUM_PER_BOARD * WLAN_DEVICE_MAX_NUM_PER_CHIP)

/* 最大的用户数(单播用户+组播用户) */
/*
1102软件规格:P2P_dev/CL以STA模式存在，P2P_GO以AP模式存在，用户数量(组播+单播用户)最大9个
    1)AP 模式:  1个组播用户 + 8个单播用户
    2)STA+P2P CL 共存模式:  2个组播用户 + 2个单播用户
    3)STA+P2P GO 共存模式:  3个组播用户 + 5个单播用户
    4)STA+Proxy STA共存模式:  1102没有proxy sta模式
*/
#define MAC_RES_MAX_USER_NUM    (WLAN_USER_MAX_USER_LIMIT)

/*****************************************************************************
  3 枚举定义
*****************************************************************************/


/*****************************************************************************
  4 全局变量声明
*****************************************************************************/
extern oal_uint16   g_us_max_asoc_user;


/*****************************************************************************
  5 消息头定义
*****************************************************************************/


/*****************************************************************************
  6 消息定义
*****************************************************************************/


/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
typedef struct
{
   oal_uint8       uc_vap[MAC_RES_VAP_SIZE];
}mac_res_mem_vap_stru;

typedef struct
{
   oal_uint8       uc_user[MAC_RES_USER_SIZE];
}mac_res_mem_user_stru;
#endif

typedef struct
{
    mac_device_stru     ast_dev_info[MAC_RES_MAX_DEV_NUM];
    oal_queue_stru      st_queue;
    oal_uint            aul_idx[MAC_RES_MAX_DEV_NUM];
    oal_uint8           auc_user_cnt[MAC_RES_MAX_DEV_NUM];
    oal_uint8           auc_resv[2];
}mac_res_device_stru;

typedef struct
{
    oal_void               *past_vap_info[WLAN_VAP_SUPPORT_MAX_NUM_LIMIT];
    oal_queue_stru          st_queue;
    oal_uint                aul_idx[WLAN_VAP_SUPPORT_MAX_NUM_LIMIT];
    oal_uint16              us_hmac_priv_size;
    oal_uint8               auc_user_cnt[WLAN_VAP_SUPPORT_MAX_NUM_LIMIT];
    oal_uint8               auc_resv[2];
}mac_res_vap_stru;

typedef struct
{
    oal_void               *past_user_info[MAC_RES_MAX_USER_NUM];
    oal_queue_stru          st_queue;
    oal_uint                *pul_idx;
    oal_uint16              us_hmac_priv_size;
    oal_uint8               auc_resv[2];
    oal_uint8              *puc_user_cnt;

}mac_res_user_stru;


typedef struct
{
    mac_res_device_stru st_dev_res;
    mac_res_vap_stru    st_vap_res;
    mac_res_user_stru   st_user_res;
}mac_res_stru;

/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/
extern mac_res_stru g_st_mac_res;

/*****************************************************************************
  10 函数声明
*****************************************************************************/
extern oal_uint32  mac_res_alloc_dmac_dev(oal_uint8    *puc_dev_idx);
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
extern oal_uint32  mac_res_alloc_hmac_dev(oal_uint8    *puc_dev_idx);
#else
extern oal_uint32  mac_res_alloc_hmac_dev(oal_uint32    ul_dev_idx);
#endif
extern oal_uint32  mac_res_free_dev(oal_uint32 ul_dev_idx);
extern oal_uint32  mac_res_free_mac_user(oal_uint16 us_idx);
extern oal_uint32  mac_res_free_mac_vap(oal_uint32 ul_idx);



OAL_STATIC OAL_INLINE oal_uint32  mac_res_alloc_hmac_vap(oal_uint8 *puc_idx, oal_uint16 us_hmac_priv_size)
{
    oal_uint  ul_idx_temp;

    if (OAL_UNLIKELY(OAL_PTR_NULL == puc_idx))
    {
        return OAL_FAIL;
    }

    ul_idx_temp = (oal_uint)oal_queue_dequeue(&(g_st_mac_res.st_vap_res.st_queue));

    /* 0为无效值 */
    if (0 == ul_idx_temp)
    {
        return OAL_FAIL;
    }

    *puc_idx = (oal_uint8)(ul_idx_temp - 1);

    g_st_mac_res.st_vap_res.us_hmac_priv_size = us_hmac_priv_size;

    (g_st_mac_res.st_vap_res.auc_user_cnt[ul_idx_temp - 1])++;

    return OAL_SUCC;
}


OAL_STATIC OAL_INLINE oal_uint32  mac_res_alloc_dmac_vap(oal_uint8 uc_idx)
{
    if (OAL_UNLIKELY(uc_idx >= WLAN_VAP_SUPPORT_MAX_NUM_LIMIT))
    {
        return OAL_FAIL;
    }

    (g_st_mac_res.st_vap_res.auc_user_cnt[uc_idx])++;

    return OAL_SUCC;
}


OAL_STATIC OAL_INLINE oal_uint32  mac_res_alloc_dmac_user(oal_uint16 us_idx)
{
    if (OAL_UNLIKELY((us_idx) >= MAC_RES_MAX_USER_NUM))
    {
        return OAL_FAIL;
    }

    /* DMAC仅需要设置为有效即可 */
    (g_st_mac_res.st_user_res.puc_user_cnt[us_idx])++;

    return OAL_SUCC;
}



OAL_STATIC OAL_INLINE mac_device_stru*  mac_res_get_dev(oal_uint32 ul_dev_idx)
{
    if (OAL_UNLIKELY(ul_dev_idx >= MAC_RES_MAX_DEV_NUM))
    {
        return OAL_PTR_NULL;
    }

    return &(g_st_mac_res.st_dev_res.ast_dev_info[ul_dev_idx]);
}


OAL_STATIC OAL_INLINE oal_void*  mac_res_get_hmac_vap(oal_uint32 ul_idx)
{
    if (OAL_UNLIKELY(ul_idx >= WLAN_VAP_SUPPORT_MAX_NUM_LIMIT))
    {
        return OAL_PTR_NULL;
    }

    return (oal_void *)(g_st_mac_res.st_vap_res.past_vap_info[ul_idx]);
}


OAL_STATIC OAL_INLINE oal_void*  mac_res_get_mac_vap(oal_uint8 uc_idx)
{
    if (OAL_UNLIKELY(uc_idx >= WLAN_VAP_SUPPORT_MAX_NUM_LIMIT))
    {
        return OAL_PTR_NULL;
    }

    /* 这里返回偏移内存空间 */
    return (oal_void *)((oal_uint8 *)(g_st_mac_res.st_vap_res.past_vap_info[uc_idx])
                        + g_st_mac_res.st_vap_res.us_hmac_priv_size);

}


OAL_STATIC OAL_INLINE oal_void*  mac_res_get_dmac_vap(oal_uint8 uc_idx)
{
    return mac_res_get_mac_vap(uc_idx);
}


OAL_STATIC OAL_INLINE oal_uint32  mac_res_alloc_hmac_user(oal_uint16 *pus_idx, oal_uint16 us_hmac_priv_size)
{
    oal_uint  ul_idx_temp;

    if (OAL_UNLIKELY(OAL_PTR_NULL == pus_idx))
    {
        return OAL_FAIL;
    }

    ul_idx_temp = (oal_uint)oal_queue_dequeue(&(g_st_mac_res.st_user_res.st_queue));

    /* 0为无效值 */
    if (0 == ul_idx_temp)
    {
        return OAL_FAIL;
    }

    *pus_idx = (oal_uint16)(ul_idx_temp - 1);

    g_st_mac_res.st_user_res.us_hmac_priv_size = us_hmac_priv_size;

    (g_st_mac_res.st_user_res.puc_user_cnt[ul_idx_temp - 1])++;

    return OAL_SUCC;
}



OAL_STATIC OAL_INLINE oal_void*  _mac_res_get_hmac_user(oal_uint16 us_idx)
{
    if (OAL_UNLIKELY(us_idx >= MAC_RES_MAX_USER_NUM))
    {
        return OAL_PTR_NULL;
    }

    return (oal_void *)(g_st_mac_res.st_user_res.past_user_info[us_idx]);
}


OAL_STATIC OAL_INLINE oal_void*  _mac_res_get_mac_user(oal_uint16 us_idx)
{
    if (OAL_UNLIKELY(us_idx >= MAC_RES_MAX_USER_NUM))
    {
        return OAL_PTR_NULL;
    }

    /* 这里偏移内存空间 */
    return (oal_void *)((oal_uint8 *)(g_st_mac_res.st_user_res.past_user_info[us_idx])
                        + g_st_mac_res.st_user_res.us_hmac_priv_size);
}


OAL_STATIC OAL_INLINE oal_uint16  mac_res_get_max_user_id(void)
{
    return (oal_uint16)(g_us_max_asoc_user * MAC_RES_MAX_DEV_NUM + WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE * MAC_RES_MAX_DEV_NUM);
}

extern oal_uint32 mac_res_init(oal_void);
extern oal_uint32 mac_res_exit(void);
extern oal_uint16 mac_res_get_max_asoc_user(oal_void);
extern oal_uint32 mac_res_set_max_asoc_user(oal_uint16 us_num);

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of mac_resource.h */
