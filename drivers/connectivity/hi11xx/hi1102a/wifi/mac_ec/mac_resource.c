


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 ͷ�ļ�����
*****************************************************************************/
#include "wlan_spec.h"
#include "mac_resource.h"
#if (defined(_PRE_PRODUCT_ID_HI110X_DEV) || (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151))
#include "dmac_vap.h"
#include "dmac_user.h"
#endif
#if (defined(_PRE_PRODUCT_ID_HI110X_HOST) || (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151))
#include "hmac_vap.h"
#include "hmac_user.h"
#endif


#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_MAC_RESOURCE_C


/*****************************************************************************
  2 ȫ�ֱ�������
*****************************************************************************/
#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
extern dmac_vap_stru  g_ast_dmac_vap[WLAN_VAP_SUPPORT_MAX_NUM_LIMIT];
extern dmac_user_stru g_ast_dmac_user[MAC_RES_MAX_USER_NUM];
extern oal_uint32     g_aul_user_idx[MAC_RES_MAX_USER_NUM];
extern oal_uint8      g_auc_user_cnt[MAC_RES_MAX_USER_NUM];

dmac_vap_stru  g_ast_dmac_vap[WLAN_VAP_SUPPORT_MAX_NUM_LIMIT];
dmac_user_stru g_ast_dmac_user[MAC_RES_MAX_USER_NUM];
oal_uint32     g_aul_user_idx[MAC_RES_MAX_USER_NUM];
oal_uint8      g_auc_user_cnt[MAC_RES_MAX_USER_NUM];
#endif


#if defined(_PRE_PRODUCT_ID_HI110X_HOST)
extern hmac_vap_stru  g_ast_hmac_vap[WLAN_VAP_SUPPORT_MAX_NUM_LIMIT];
extern hmac_user_stru g_ast_hmac_user[MAC_RES_MAX_USER_NUM];

hmac_user_stru *g_pst_hmac_user= &g_ast_hmac_user[0];
#endif

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
mac_res_mem_vap_stru g_ast_mac_res_vap[WLAN_VAP_SUPPORT_MAX_NUM_LIMIT];
#endif


/* 1��device֧�ֵ��������û��� */
oal_uint16   g_us_max_asoc_user = WLAN_ASSOC_USER_MAX_NUM;
mac_res_stru g_st_mac_res;

oal_uint16 g_us_mac_res_stru_size = OAL_SIZEOF(mac_res_stru);

/*****************************************************************************
  3 ����ʵ��
*****************************************************************************/

oal_uint32  mac_res_check_spec(oal_void)
{
    oal_uint32  ul_ret = OAL_SUCC;
    /* ���MAC VAP��С */
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
    if ((OAL_SIZEOF(hmac_vap_stru) + OAL_SIZEOF(dmac_vap_stru) - OAL_SIZEOF(mac_vap_stru)) > MAC_RES_VAP_SIZE)
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{mac_res_check_spec::vap_stru is over limit! vap_stru[%d], MAC_RES_VAP_SIZE[%d]}",
                      (OAL_SIZEOF(hmac_vap_stru) + OAL_SIZEOF(dmac_vap_stru) - OAL_SIZEOF(mac_vap_stru)),
                       MAC_RES_VAP_SIZE);
        OAL_IO_PRINT("{mac_res_check_spec::vap_stru is over limit! hmac_vap[%d], dmac_vap[%d], mac_vap[%d], MAC_RES_VAP_SIZE[%d].\r\n}",
                     OAL_SIZEOF(hmac_vap_stru), OAL_SIZEOF(dmac_vap_stru), OAL_SIZEOF(mac_vap_stru), MAC_RES_VAP_SIZE);
        ul_ret = OAL_FAIL;
    }

    if ((OAL_SIZEOF(hmac_user_stru) + OAL_SIZEOF(dmac_user_stru) - OAL_SIZEOF(mac_user_stru)) > MAC_RES_USER_SIZE)
    {
        OAM_ERROR_LOG2(0, OAM_SF_ANY, "{mac_res_check_spec::user_stru is over limit! user_stru[%d], MAC_RES_USER_SIZE[%d]}",
                       (OAL_SIZEOF(hmac_user_stru) + OAL_SIZEOF(dmac_user_stru) - OAL_SIZEOF(mac_user_stru)), MAC_RES_USER_SIZE);
        OAL_IO_PRINT("{mac_res_check_spec::user_stru is over limit! hmac_user[%d], dmac_user[%d], mac_user[%d], MAC_RES_USER_SIZE[%d].\r\n}",
                     OAL_SIZEOF(hmac_user_stru), OAL_SIZEOF(dmac_user_stru), OAL_SIZEOF(mac_user_stru), MAC_RES_USER_SIZE);
        ul_ret = OAL_FAIL;
    }
#endif
    return ul_ret;
}


#if (defined(_PRE_PRODUCT_ID_HI110X_DEV))

oal_void  mac_res_vap_init(oal_void)
{
    oal_uint32 ul_loop;
    oal_uint ul_one_vap_size = 0;

    g_st_mac_res.st_vap_res.us_hmac_priv_size = 0;

    for (ul_loop = 0; ul_loop < WLAN_VAP_SUPPORT_MAX_NUM_LIMIT; ul_loop++)
    {
        g_st_mac_res.st_vap_res.past_vap_info[ul_loop] = (oal_uint8 *)g_ast_dmac_vap + ul_one_vap_size;
        OAL_MEMZERO(g_st_mac_res.st_vap_res.past_vap_info[ul_loop], OAL_SIZEOF(dmac_vap_stru));

        ul_one_vap_size += OAL_SIZEOF(dmac_vap_stru);

        /* ��ʼ����Ӧ�����ü���ֵΪ0 */
        g_st_mac_res.st_vap_res.auc_user_cnt[ul_loop] = 0;
    }
}


oal_uint32  mac_res_user_init(oal_void)
{
    oal_uint32      ul_loop;
    oal_void       *p_user_info = OAL_PTR_NULL;
    oal_void       *p_idx       = OAL_PTR_NULL;
    oal_void       *p_user_cnt  = OAL_PTR_NULL;
    oal_uint        ul_one_user_info_size = 0;

    /***************************************************************************
            ��ʼ��USER����Դ��������
    ***************************************************************************/
    p_user_info = (oal_void *)g_ast_dmac_user;
    p_idx       = (oal_void *)g_aul_user_idx;
    p_user_cnt  = (oal_void *)g_auc_user_cnt;

    /* �ڴ��ʼ��0 */
    OAL_MEMZERO(p_user_info, (OAL_SIZEOF(dmac_user_stru) * MAC_RES_MAX_USER_NUM));
    OAL_MEMZERO(p_idx,       (OAL_SIZEOF(oal_uint) * MAC_RES_MAX_USER_NUM));
    OAL_MEMZERO(p_user_cnt,  (OAL_SIZEOF(oal_uint8) * MAC_RES_MAX_USER_NUM));

    g_st_mac_res.st_user_res.pul_idx        = p_idx;
    g_st_mac_res.st_user_res.puc_user_cnt   = p_user_cnt;

    g_st_mac_res.st_user_res.us_hmac_priv_size = 0;

    for (ul_loop = 0; ul_loop < MAC_RES_MAX_USER_NUM; ul_loop++)
    {
        g_st_mac_res.st_user_res.past_user_info[ul_loop]  = (oal_uint8 *)p_user_info + ul_one_user_info_size;
        ul_one_user_info_size += OAL_SIZEOF(dmac_user_stru);

        /* ��ʼ����Ӧ�����ü���ֵΪ0 */
        g_st_mac_res.st_user_res.puc_user_cnt[ul_loop] = 0;
    }

    return OAL_SUCC;
}


oal_uint32  mac_res_exit(void)
{
    oal_uint ul_loop;

    OAL_MEM_FREE((g_st_mac_res.st_user_res.past_user_info[0]), OAL_TRUE);

    for (ul_loop = 0; ul_loop < MAC_RES_MAX_USER_NUM; ul_loop++)
    {
       g_st_mac_res.st_user_res.past_user_info[ul_loop]      = OAL_PTR_NULL;
    }

    g_st_mac_res.st_user_res.pul_idx            = OAL_PTR_NULL;
    g_st_mac_res.st_user_res.puc_user_cnt       = OAL_PTR_NULL;

    return OAL_SUCC;
}


oal_uint32  mac_res_init(oal_void)
{
    oal_uint        ul_loop;
    oal_uint32      ul_ret;

    OAL_MEMZERO(&g_st_mac_res, g_us_mac_res_stru_size);
    /***************************************************************************
            ��ʼ��DEV����Դ��������
    ***************************************************************************/
    oal_queue_set(&(g_st_mac_res.st_dev_res.st_queue),
                  g_st_mac_res.st_dev_res.aul_idx,
                  MAC_RES_MAX_DEV_NUM);

    ul_ret = mac_res_check_spec();
    if (OAL_SUCC != ul_ret)
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{mac_res_init::mac_res_init failed[%d].}", ul_ret);
        return ul_ret;
    }

    for (ul_loop = 0; ul_loop < MAC_RES_MAX_DEV_NUM; ul_loop++)
    {
        /* ��ʼֵ������Ƕ�Ӧ�����±�ֵ��1 */
        oal_queue_enqueue(&(g_st_mac_res.st_dev_res.st_queue), (oal_void *)(ul_loop + 1));

        /* ��ʼ����Ӧ�����ü���ֵΪ0 */
        g_st_mac_res.st_dev_res.auc_user_cnt[ul_loop] = 0;
    }


    /***************************************************************************
            ��ʼ��VAP����Դ��������
    ***************************************************************************/
    mac_res_vap_init();

    /***************************************************************************
            ��ʼ��USER����Դ��������
    ***************************************************************************/
    ul_ret = mac_res_user_init();
    if (OAL_SUCC != ul_ret)
    {
        MAC_ERR_LOG1(0, "mac_res_init: mac_res_user_init return err code", ul_ret);
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{mac_res_init::mac_res_user_init failed[%d].}", ul_ret);

        return ul_ret;
    }

    return OAL_SUCC;
}


oal_uint32  mac_res_free_mac_user(oal_uint16 us_idx)
{
    if (OAL_UNLIKELY(us_idx >= MAC_RES_MAX_USER_NUM))
    {
        return OAL_FAIL;
    }

    if(0 == g_st_mac_res.st_user_res.puc_user_cnt[us_idx])
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY,"mac_res_free_mac_user::cnt==0! idx:%d",us_idx);
        oal_dump_stack();
        return OAL_ERR_CODE_USER_RES_CNT_ZERO;
    }

    (g_st_mac_res.st_user_res.puc_user_cnt[us_idx])--;

    if (0 != g_st_mac_res.st_user_res.puc_user_cnt[us_idx])
    {
        return OAL_SUCC;
    }

    return OAL_SUCC;
}


oal_uint32  mac_res_free_mac_vap(oal_uint32 ul_idx)
{
    if (OAL_UNLIKELY(ul_idx >= WLAN_VAP_SUPPORT_MAX_NUM_LIMIT))
    {
        return OAL_FAIL;
    }

    if(0 == g_st_mac_res.st_vap_res.auc_user_cnt[ul_idx])
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY,"mac_res_free_mac_vap::cnt==0! idx:%d",ul_idx);
        oal_dump_stack();
        return OAL_SUCC;
    }

    (g_st_mac_res.st_vap_res.auc_user_cnt[ul_idx])--;

    if (0 != g_st_mac_res.st_vap_res.auc_user_cnt[ul_idx])
    {
        return OAL_SUCC;
    }

    return OAL_SUCC;
}

#else


oal_void  mac_res_vap_init(oal_void)
{
    oal_uint ul_loop;

    oal_queue_set(&(g_st_mac_res.st_vap_res.st_queue),
                  g_st_mac_res.st_vap_res.aul_idx,
                  (oal_uint8)WLAN_VAP_SUPPORT_MAX_NUM_LIMIT);
    /*lint -e413*/
    g_st_mac_res.st_vap_res.us_hmac_priv_size = (oal_uint16)OAL_OFFSET_OF(hmac_vap_stru, st_vap_base_info);
    /*lint +e413*/

    for (ul_loop = 0; ul_loop < WLAN_VAP_SUPPORT_MAX_NUM_LIMIT; ul_loop++)
    {
    #if (defined(_PRE_PRODUCT_ID_HI110X_HOST))
        g_st_mac_res.st_vap_res.past_vap_info[ul_loop] = (oal_void*)&g_ast_hmac_vap[ul_loop];
        OAL_MEMZERO(g_st_mac_res.st_vap_res.past_vap_info[ul_loop], OAL_SIZEOF(hmac_vap_stru));
    #else
        g_st_mac_res.st_vap_res.past_vap_info[ul_loop] = (oal_void*)&g_ast_mac_res_vap[ul_loop];
        OAL_MEMZERO(g_st_mac_res.st_vap_res.past_vap_info[ul_loop], OAL_SIZEOF(mac_res_mem_vap_stru));
    #endif

        /* ��ʼֵ������Ƕ�Ӧ�����±�ֵ��1 */
        oal_queue_enqueue(&(g_st_mac_res.st_vap_res.st_queue), (oal_void *)(ul_loop + 1));
        /* ��ʼ����Ӧ�����ü���ֵΪ0 */
        g_st_mac_res.st_vap_res.auc_user_cnt[ul_loop] = 0;
     }

}


oal_uint32  mac_res_user_init(oal_void)
{
    oal_uint        ul_loop;
    oal_void       *p_user_info = OAL_PTR_NULL;
    oal_void       *p_idx       = OAL_PTR_NULL;
    oal_void       *p_user_cnt  = OAL_PTR_NULL;
    oal_uint        ul_one_user_info_size;

    /***************************************************************************
            ��ʼ��USER����Դ��������
    ***************************************************************************/
    /*
    �������û� = 1��device֧�ֵ��������û��� * board�����device��Ŀ;
    ����鲥�û� = 1��device֧�ֵ����ҵ��vap�� * board�����device��Ŀ;
    ����û��� = �������û� + �鲥�û����� */

    /* ��̬�����û���Դ������ڴ� */
#if (defined(_PRE_PRODUCT_ID_HI110X_HOST))
    p_user_info = (oal_void *)g_pst_hmac_user;
#else
    p_user_info = oal_memalloc(OAL_SIZEOF(mac_res_mem_user_stru) * MAC_RES_MAX_USER_NUM);
#endif
    p_idx       = oal_memalloc(OAL_SIZEOF(oal_uint) * MAC_RES_MAX_USER_NUM);
    p_user_cnt  = oal_memalloc(OAL_SIZEOF(oal_uint8) * MAC_RES_MAX_USER_NUM);
    if ((OAL_PTR_NULL == p_user_info) || (OAL_PTR_NULL == p_idx) || (OAL_PTR_NULL == p_user_cnt))
    {
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{mac_res_user_init::param null.}");

#if (defined(_PRE_PRODUCT_ID_HI110X_HOST))
#else
        if (OAL_PTR_NULL != p_user_info)
        {
            oal_free(p_user_info);
        }
#endif

        if (OAL_PTR_NULL != p_idx)
        {
            oal_free(p_idx);
        }

        if (OAL_PTR_NULL != p_user_cnt)
        {
            oal_free(p_user_cnt);
        }

        return OAL_ERR_CODE_ALLOC_MEM_FAIL;
    }


    /* �ڴ��ʼ��0 */
#if (defined(_PRE_PRODUCT_ID_HI110X_HOST))
    OAL_MEMZERO(p_user_info, (OAL_SIZEOF(hmac_user_stru) * MAC_RES_MAX_USER_NUM));
#else
    OAL_MEMZERO(p_user_info, (OAL_SIZEOF(mac_res_mem_user_stru) * MAC_RES_MAX_USER_NUM));
#endif
    OAL_MEMZERO(p_idx,       (OAL_SIZEOF(oal_uint) * MAC_RES_MAX_USER_NUM));
    OAL_MEMZERO(p_user_cnt,  (OAL_SIZEOF(oal_uint8) * MAC_RES_MAX_USER_NUM));

    g_st_mac_res.st_user_res.pul_idx        = p_idx;
    g_st_mac_res.st_user_res.puc_user_cnt   = p_user_cnt;

    oal_queue_set(&(g_st_mac_res.st_user_res.st_queue),
                  g_st_mac_res.st_user_res.pul_idx,
                  (oal_uint8)MAC_RES_MAX_USER_NUM);
    g_st_mac_res.st_user_res.us_hmac_priv_size = 0;

    ul_one_user_info_size = 0;

    for (ul_loop = 0; ul_loop < MAC_RES_MAX_USER_NUM; ul_loop++)
    {
        /* ��ʼֵ������Ƕ�Ӧ�����±�ֵ��1 */
        oal_queue_enqueue(&(g_st_mac_res.st_user_res.st_queue), (oal_void *)(ul_loop + 1));

        /* ��ʼ����Ӧ������λ�� */
        g_st_mac_res.st_user_res.past_user_info[ul_loop]  = (oal_uint8 *)p_user_info + ul_one_user_info_size;
    #if (defined(_PRE_PRODUCT_ID_HI110X_HOST))
        ul_one_user_info_size += OAL_SIZEOF(hmac_user_stru);
    #else
        ul_one_user_info_size += OAL_SIZEOF(mac_res_mem_user_stru);
    #endif

        /* ��ʼ����Ӧ�����ü���ֵΪ0 */
        g_st_mac_res.st_user_res.puc_user_cnt[ul_loop] = 0;

    }

    return OAL_SUCC;
}

oal_uint32  mac_res_exit(void)
{
    oal_uint ul_loop;
#if (defined(_PRE_PRODUCT_ID_HI110X_HOST))
#else
    oal_free(g_st_mac_res.st_user_res.past_user_info[0]);
#endif
    oal_free(g_st_mac_res.st_user_res.pul_idx);
    oal_free(g_st_mac_res.st_user_res.puc_user_cnt);

    for (ul_loop = 0; ul_loop < MAC_RES_MAX_USER_NUM; ul_loop++)
    {
       g_st_mac_res.st_user_res.past_user_info[ul_loop]      = OAL_PTR_NULL;
    }
    g_st_mac_res.st_user_res.pul_idx            = OAL_PTR_NULL;
    g_st_mac_res.st_user_res.puc_user_cnt       = OAL_PTR_NULL;

    return OAL_SUCC;
}


oal_uint32  mac_res_init(oal_void)
{
    oal_uint        ul_loop;
    oal_uint32      ul_ret;

    OAL_MEMZERO(&g_st_mac_res, g_us_mac_res_stru_size);
    /***************************************************************************
            ��ʼ��DEV����Դ��������
    ***************************************************************************/
    oal_queue_set(&(g_st_mac_res.st_dev_res.st_queue),
                  g_st_mac_res.st_dev_res.aul_idx,
                  MAC_RES_MAX_DEV_NUM);

    ul_ret = mac_res_check_spec();
    if (OAL_SUCC != ul_ret)
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{mac_res_init::mac_res_user_init failed[%d].}", ul_ret);
        return ul_ret;
    }

    for (ul_loop = 0; ul_loop < MAC_RES_MAX_DEV_NUM; ul_loop++)
    {
        /* ��ʼֵ������Ƕ�Ӧ�����±�ֵ��1 */
        oal_queue_enqueue(&(g_st_mac_res.st_dev_res.st_queue), (oal_void *)(ul_loop + 1));

        /* ��ʼ����Ӧ�����ü���ֵΪ0 */
        g_st_mac_res.st_dev_res.auc_user_cnt[ul_loop] = 0;
    }

    /***************************************************************************
            ��ʼ��VAP����Դ��������
    ***************************************************************************/
    mac_res_vap_init();

    /***************************************************************************
            ��ʼ��USER����Դ��������
    ***************************************************************************/
    ul_ret = mac_res_user_init();
    if (OAL_SUCC != ul_ret)
    {
        MAC_ERR_LOG1(0, "mac_res_init: mac_res_user_init return err code", ul_ret);
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{mac_res_init::mac_res_user_init failed[%d].}", ul_ret);

        return ul_ret;
    }

    return OAL_SUCC;
}


oal_uint32  mac_res_free_mac_user(oal_uint16 us_idx)
{
    if (OAL_UNLIKELY(us_idx >= MAC_RES_MAX_USER_NUM))
    {
        return OAL_FAIL;
    }

    if(0 == g_st_mac_res.st_user_res.puc_user_cnt[us_idx])
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY,"mac_res_free_mac_user::cnt==0! idx:%d",us_idx);
        oal_dump_stack();
        return OAL_ERR_CODE_USER_RES_CNT_ZERO;
    }

    (g_st_mac_res.st_user_res.puc_user_cnt[us_idx])--;

    if (0 != g_st_mac_res.st_user_res.puc_user_cnt[us_idx])
    {
        return OAL_SUCC;
    }

    /* �������ֵ��Ҫ��1���� */
    oal_queue_enqueue(&(g_st_mac_res.st_user_res.st_queue), (oal_void *)((oal_uint)us_idx + 1));

    return OAL_SUCC;
}


oal_uint32  mac_res_free_mac_vap(oal_uint32 ul_idx)
{
    if (OAL_UNLIKELY(ul_idx >= WLAN_VAP_SUPPORT_MAX_NUM_LIMIT))
    {
        return OAL_FAIL;
    }

    if(0 == g_st_mac_res.st_vap_res.auc_user_cnt[ul_idx])
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY,"mac_res_free_mac_vap::cnt==0! idx:%d",ul_idx);
        oal_dump_stack();
        return OAL_SUCC;
    }

    (g_st_mac_res.st_vap_res.auc_user_cnt[ul_idx])--;

    if (0 != g_st_mac_res.st_vap_res.auc_user_cnt[ul_idx])
    {
        return OAL_SUCC;
    }

    /* �������ֵ��Ҫ��1���� */
    oal_queue_enqueue(&(g_st_mac_res.st_vap_res.st_queue), (oal_void *)((oal_uint)ul_idx + 1));

    return OAL_SUCC;
}

#endif



oal_uint32  mac_res_set_max_asoc_user(oal_uint16 us_num)
{
    g_us_max_asoc_user = us_num;

    /* ��Ҫϵͳ��λ */

    return OAL_SUCC;
}


 oal_uint32  mac_res_alloc_dmac_dev(oal_uint8    *puc_dev_idx)
{
    oal_uint  ul_dev_idx_temp;

    if (OAL_UNLIKELY(OAL_PTR_NULL == puc_dev_idx))
    {
        OAL_IO_PRINT("mac_res_alloc_dmac_dev: OAL_PTR_NULL == pul_dev_idx");
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{mac_res_alloc_dmac_dev::puc_dev_idx null.}");

        return OAL_FAIL;
    }

    ul_dev_idx_temp = (oal_uint)oal_queue_dequeue(&(g_st_mac_res.st_dev_res.st_queue));

    /* 0Ϊ��Чֵ */
    if (0 == ul_dev_idx_temp)
    {
        OAL_IO_PRINT("mac_res_alloc_dmac_dev: 0 == ul_dev_idx_temp");
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{mac_res_alloc_dmac_dev::ul_dev_idx_temp=0.}");

        return OAL_FAIL;
    }

    *puc_dev_idx = (oal_uint8)(ul_dev_idx_temp - 1);

    (g_st_mac_res.st_dev_res.auc_user_cnt[ul_dev_idx_temp - 1])++;

    return OAL_SUCC;
}
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)

 oal_uint32  mac_res_alloc_hmac_dev(oal_uint8    *puc_dev_idx)
{
    oal_uint  ul_dev_idx_temp;

    if (OAL_UNLIKELY(OAL_PTR_NULL == puc_dev_idx))
    {
        OAL_IO_PRINT("mac_res_alloc_hmac_dev: OAL_PTR_NULL == pul_dev_idx");
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{mac_res_alloc_hmac_dev::puc_dev_idx null.}");

        return OAL_FAIL;
    }

    ul_dev_idx_temp = (oal_uint)oal_queue_dequeue(&(g_st_mac_res.st_dev_res.st_queue));

    /* 0Ϊ��Чֵ */
    if (0 == ul_dev_idx_temp)
    {
        OAL_IO_PRINT("mac_res_alloc_hmac_dev: 0 == ul_dev_idx_temp");
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "{mac_res_alloc_hmac_dev::ul_dev_idx_temp=0.}");

        return OAL_FAIL;
    }

    *puc_dev_idx = (oal_uint8)(ul_dev_idx_temp - 1);

    (g_st_mac_res.st_dev_res.auc_user_cnt[ul_dev_idx_temp - 1])++;

    return OAL_SUCC;
}
#else

oal_uint32  mac_res_alloc_hmac_dev(oal_uint32    ul_dev_idx)
{
    if (OAL_UNLIKELY(ul_dev_idx >= MAC_RES_MAX_DEV_NUM))
    {
        MAC_ERR_LOG(0, "mac_res_alloc_hmac_dev: ul_dev_idx >= MAC_RES_MAX_DEV_NUM");
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{mac_res_alloc_hmac_dev::invalid ul_dev_idx[%d].}", ul_dev_idx);

        return OAL_FAIL;
    }

    (g_st_mac_res.st_dev_res.auc_user_cnt[ul_dev_idx])++;

    return OAL_SUCC;
}
#endif

oal_uint32  mac_res_free_dev(oal_uint32 ul_dev_idx)
{
    if (OAL_UNLIKELY(ul_dev_idx >= MAC_RES_MAX_DEV_NUM))
    {
        MAC_ERR_LOG(0, "mac_res_free_dev: ul_dev_idx >= MAC_RES_MAX_DEV_NUM");
        OAM_ERROR_LOG1(0, OAM_SF_ANY, "{mac_res_free_dev::invalid ul_dev_idx[%d].}", ul_dev_idx);

        return OAL_FAIL;
    }

    if(0 == g_st_mac_res.st_dev_res.auc_user_cnt[ul_dev_idx])
    {
        OAM_ERROR_LOG1(0, OAM_SF_ANY,"mac_res_free_dev::cnt==0! idx:%d",ul_dev_idx);
        oal_dump_stack();
        return OAL_SUCC;
    }

    (g_st_mac_res.st_dev_res.auc_user_cnt[ul_dev_idx])--;

    if (0 != g_st_mac_res.st_dev_res.auc_user_cnt[ul_dev_idx])
    {
        return OAL_SUCC;
    }

    /* �������ֵ��Ҫ��1���� */
    oal_queue_enqueue(&(g_st_mac_res.st_dev_res.st_queue), (oal_void *)((oal_uint)ul_dev_idx + 1));

    return OAL_SUCC;
}


#if (!defined(_PRE_PRODUCT_ID_HI110X_DEV))
    OAL_INLINE oal_uint16  mac_res_get_max_asoc_user(oal_void)
#else
    oal_uint16  mac_res_get_max_asoc_user(oal_void)
#endif
{
    return g_us_max_asoc_user;
}

/*lint -e19*/
oal_module_symbol(g_st_mac_res);
oal_module_symbol(mac_res_get_max_asoc_user);
oal_module_symbol(mac_res_set_max_asoc_user);
oal_module_symbol(g_us_max_asoc_user);
oal_module_symbol(mac_res_free_dev);
oal_module_symbol(mac_res_alloc_hmac_dev);
oal_module_symbol(mac_res_get_dev);
oal_module_symbol(mac_res_free_mac_user);
oal_module_symbol(mac_res_free_mac_vap);
/*lint +e19*/


#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

