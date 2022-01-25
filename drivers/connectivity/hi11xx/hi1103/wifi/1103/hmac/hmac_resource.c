


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 ͷ�ļ�����
*****************************************************************************/
#include "hmac_resource.h"


#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_RESOURCE_C


/*****************************************************************************
  2 ȫ�ֱ�������
*****************************************************************************/
hmac_res_stru    g_st_hmac_res_etc;


/*****************************************************************************
  3 ����ʵ��
*****************************************************************************/



oal_uint32  hmac_res_alloc_mac_dev_etc(oal_uint32    ul_dev_idx)
{
    if (OAL_UNLIKELY(ul_dev_idx >= MAC_RES_MAX_DEV_NUM))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{hmac_res_alloc_hmac_dev::invalid ul_dev_idx[%d].}", ul_dev_idx);

        return OAL_FAIL;
    }

    (g_st_hmac_res_etc.st_hmac_dev_res.auc_user_cnt[ul_dev_idx])++;

    return OAL_SUCC;
}


oal_uint32  hmac_res_free_mac_dev_etc(oal_uint32 ul_dev_idx)
{
    if (OAL_UNLIKELY(ul_dev_idx >= MAC_RES_MAX_DEV_NUM))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{mac_res_free_dev_etc::invalid ul_dev_idx[%d].}", ul_dev_idx);

        return OAL_FAIL;
    }

    (g_st_hmac_res_etc.st_hmac_dev_res.auc_user_cnt[ul_dev_idx])--;

    if (0 != g_st_hmac_res_etc.st_hmac_dev_res.auc_user_cnt[ul_dev_idx])
    {
        return OAL_SUCC;
    }

    /* �������ֵ��Ҫ��1���� */
    oal_queue_enqueue(&(g_st_hmac_res_etc.st_hmac_dev_res.st_queue), (oal_void *)((oal_uint)ul_dev_idx + 1));

    return OAL_SUCC;
}


hmac_device_stru  *hmac_res_get_mac_dev_etc(oal_uint32 ul_dev_idx)
{
    if (OAL_UNLIKELY(ul_dev_idx >= MAC_RES_MAX_DEV_NUM))
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{hmac_res_get_hmac_dev::invalid ul_dev_idx[%d].}", ul_dev_idx);

        return OAL_PTR_NULL;
    }

    return &(g_st_hmac_res_etc.st_hmac_dev_res.ast_hmac_dev_info[ul_dev_idx]);
}


mac_chip_stru  *hmac_res_get_mac_chip(oal_uint32 ul_chip_idx)
{
    return mac_res_get_mac_chip(ul_chip_idx);
}



oal_uint32  hmac_res_init_etc(oal_void)
{
    oal_uint32      ul_loop;

    OAL_MEMZERO(&g_st_hmac_res_etc, OAL_SIZEOF(g_st_hmac_res_etc));

    /***************************************************************************
            ��ʼ��HMAC DEV����Դ��������
    ***************************************************************************/
    oal_queue_set(&(g_st_hmac_res_etc.st_hmac_dev_res.st_queue),
                  g_st_hmac_res_etc.st_hmac_dev_res.aul_idx,
                  MAC_RES_MAX_DEV_NUM);

    for (ul_loop = 0; ul_loop < MAC_RES_MAX_DEV_NUM; ul_loop++)
    {
        /* ��ʼֵ������Ƕ�Ӧ�����±�ֵ��1 */
        oal_queue_enqueue(&(g_st_hmac_res_etc.st_hmac_dev_res.st_queue), (oal_void *)((oal_uint)ul_loop + 1));

        /* ��ʼ����Ӧ�����ü���ֵΪ0 */
        g_st_hmac_res_etc.st_hmac_dev_res.auc_user_cnt[ul_loop] = 0;
    }

    return OAL_SUCC;
}
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)

oal_uint32  hmac_res_exit_etc(mac_board_stru *pst_hmac_board)
{
    oal_uint8           uc_chip;
    oal_uint8           uc_device_max;
    oal_uint8           uc_device;
    oal_uint16          ul_chip_max_num;
    mac_chip_stru       *pst_chip;

    /* chip֧�ֵ��������PCIe���ߴ����ṩ; */
    ul_chip_max_num = oal_bus_get_chip_num_etc();

    for (uc_chip = 0; uc_chip < ul_chip_max_num; uc_chip++)
    {
        pst_chip = &pst_hmac_board->ast_chip[uc_chip];

        /* OAL�ӿڻ�ȡ֧��device���� */
        uc_device_max = oal_chip_get_device_num_etc(pst_chip->ul_chip_ver);

        for (uc_device = 0; uc_device < uc_device_max; uc_device++)
        {
            /* �ͷ�hmac res��Դ */
            hmac_res_free_mac_dev_etc(pst_chip->auc_device_id[uc_device]);
        }
    }
    return OAL_SUCC;
}
#endif
/*lint -e19*/
oal_module_symbol(hmac_res_get_mac_dev_etc);
/*lint +e19*/

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

