


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include <linux/mtd/hisi_nve_interface.h>
#include "hisi_ini.h"
#include "plat_debug.h"
#include "plat_firmware.h"
#include "platform_common_clk.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_PLATFORM_COMMON_CLK_C

typedef struct
{
    oal_int32   part_id;       // nv号
    oal_int32   total_size;    // nv项大小
    oal_int32   read_size;     // 存储实际有效内容的大小
    char*       name;
} nv_part_str;


/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
/*存储从NV中读取的参数信息，解析cfg文件时赋值一次，后续GNSS业务需要，也会更新该变量*/
oal_uint8          *g_pucDcxoDataBuf = NULL;
dcxo_manage_stru   g_st_dcxo_info    = {0};
dcxo_nv_info_stru  g_st_factory      = {0};

/*先读平台默认值，其次工厂刷入的nv，最后读自学习的nv，顺序不能反 */
nv_part_str gst_dcxo_nv_part[] = {
    {PLAT_DEF_ID, COEFF_NUM,    COEFF_NUM,                                                        PLAT_DEF_NAME},
    {NV_FAC_ID,   NV_FAC_SIZE,  OAL_SIZEOF(dcxo_nv_info_stru),                                     NV_FAC_NAME },
    {NV_GNSS_ID,  NV_GNSS_SIZE, OAL_SIZEOF(dcxo_nv_info_stru) - OAL_SIZEOF(dcxo_nv_coeff_th_stru), NV_GNSS_NAME}
};


oal_int32 g_default_coeff[COEFF_NUM] = {
     DEFAULT_A0,
     DEFAULT_A1,
     DEFAULT_A2,
     DEFAULT_A3,
     DEFAULT_T0
};

/*****************************************************************************
  3 函数实现
*****************************************************************************/
oal_int32 dcxo_data_buf_malloc(oal_void)
{
    oal_uint8 *buffer = NULL;

    buffer = OS_KZALLOC_GFP(DCXO_CALI_DATA_BUF_LEN);
    if (NULL == buffer)
    {
        return -EFAIL;
    }
    g_pucDcxoDataBuf = buffer;

    return SUCC;
}

oal_void *get_dcxo_data_buf_addr(oal_void)
{
    return g_pucDcxoDataBuf;
}

nv_part_str* get_nv_part_from_id(oal_int32 part_id)
{
    oal_int32 loop;
    for (loop = 0; loop < OAL_ARRAY_SIZE(gst_dcxo_nv_part); loop++)
    {
        if (gst_dcxo_nv_part[loop].part_id == part_id)
        {
            return &(gst_dcxo_nv_part[loop]);
        }
    }

    return NULL;
}

oal_int32 read_coeff_from_nvram(oal_uint8 *pc_out, nv_part_str *nv_para)
{
    struct hisi_nve_info_user  info;
    oal_int32 ret = INI_FAILED;

    if (nv_para->part_id < 0)       // 对于小于0的nv id为虚拟配置，目前用来配置dcxo默认值
    {
        OS_MEM_CPY(pc_out, g_default_coeff, nv_para->read_size);
        return INI_SUCC;
    }
    else
    {
        OS_MEM_SET(&info, 0, sizeof(info));
        OS_MEM_CPY(info.nv_name, nv_para->name, OAL_STRLEN(nv_para->name));
        info.nv_operation = NVRAM_READ;
        info.valid_size   = nv_para->total_size;
        info.nv_number    = nv_para->part_id;

        ret = hisi_nve_direct_access(&info);

        if (INI_SUCC == ret)
        {
            OS_MEM_CPY(pc_out, info.nv_data, nv_para->read_size);
            PS_PRINT_INFO("[dcxo] read_conf_from_nvram::nvram id[%d], nv name[%s], size[%d]\r\n!",
                            info.nv_number, info.nv_name, info.valid_size);
        }
        else
        {
            PS_PRINT_ERR("[dcxo] read nvm [%d] %s failed", info.nv_number, info.nv_name);
            return INI_FAILED;
        }
    }

    return INI_SUCC;
}

oal_void update_dcxo_coeff(oal_uint32 *coeff)
{
    oal_int32         loop;
    dcxo_dl_para_stru *pst_para;

    if (NULL == coeff)
        return;

    pst_para = get_dcxo_data_buf_addr();
    if (NULL == pst_para)
    {
        PS_PRINT_ERR("[dcxo] update coeff input pst_para is NULL\n");
        return;
    }

    OS_MEM_CPY(pst_para->coeff.para, coeff, GNSS_COEFF_NUM * OAL_SIZEOF(oal_int32));

    for (loop = 0; loop < COEFF_NUM; loop++)
    {
        PS_PRINT_INFO("[dcxo] update coeff  a[%d] = 0x%x \n", loop, pst_para->coeff.para[loop]);
    }

}

oal_int32 process_plat_cali(oal_int8 *pc_buffer, dcxo_dl_para_stru *pst_dl_para)
{
    oal_int32 loop;
    oal_int32 *pl_para;
    pl_para   = pst_dl_para->coeff.para;

    for (loop = 0; loop < COEFF_NUM; loop++)
    {
        *pl_para = g_default_coeff[loop];
        pl_para++;
    }

    pst_dl_para->check_data     = DCXO_CHECK_MAGIC;
    pst_dl_para->tcxo_dcxo_flag = read_tcxo_dcxo_mode();

    return INI_SUCC;
}

oal_int32 process_factory_cali(oal_int8 *pc_buffer, dcxo_dl_para_stru *pst_dl_para)
{
    oal_int32   loop;
    dcxo_coeff_stru *coeff_fac;

    OS_MEM_CPY(&g_st_factory, pc_buffer, OAL_SIZEOF(dcxo_nv_info_stru));
    if (DCXO_NV_CHK_OK != g_st_factory.valid)
    {
       PS_PRINT_ERR("[dcxo] read factory cali buffer error func [%s]\n", __func__);
        return INI_FAILED;
    }

    coeff_fac = &(g_st_factory.coeff_u.fac);
    OS_MEM_CPY(&(pst_dl_para->coeff), coeff_fac, OAL_SIZEOF(dcxo_coeff_stru));

    for (loop = 0; loop < COEFF_NUM; loop++)
    {
        PS_PRINT_INFO("[dcxo] read factory nv a[%d] = Dec(%d) Hex(0x%x)\n",
                        loop, (coeff_fac->para)[loop], (coeff_fac->para)[loop]);
    }

    // 此处检测到工厂分区t0使用的是10位，需要修正到16位
    if (T0_BIT_WIDTH_10 == g_st_factory.coeff_th.t0_bit_width_fix)
    {
        pst_dl_para->coeff.para[COEFF_NUM - 1] *= T0_WIDTH10_TO_WIDTH16;
    }

    return INI_SUCC;
}


oal_int32 process_gnss_self_study(oal_uint8 *pc_buffer, dcxo_dl_para_stru *pst_dl_para)
{
    oal_int32   l_invld_cnt;
    oal_int32   *pl_coeff;
    oal_int32   (*pl_check_coeff)[NV_CHK_PAIR];
    oal_int32   loop;
    dcxo_nv_info_stru *pst_info;

    pst_info = (dcxo_nv_info_stru *)pc_buffer;
    if (DCXO_NV_CHK_OK != pst_info->valid)
    {
        PS_PRINT_ERR("[dcxo] nv check  not valid \n");
        return INI_FAILED;
    }

    l_invld_cnt = 0;
    pl_coeff = pst_info->coeff_u.gnss.para;
    pl_check_coeff = g_st_factory.coeff_th.check_value;
    for (loop = 0; loop < GNSS_COEFF_NUM; loop++)
    {
        if ( (*pl_coeff < pl_check_coeff[loop][NV_MIN_PAIR_POS]) ||
                (*pl_coeff > pl_check_coeff[loop][NV_MAX_PAIR_POS]) )
        {
            l_invld_cnt++;
            PS_PRINT_ERR("[dcxo] get coeff [%d], value %d, check exceed min[%d] <-> max[%d] scope \n",
                loop, *pl_coeff, pl_check_coeff[loop][NV_MIN_PAIR_POS], pl_check_coeff[loop][NV_MAX_PAIR_POS]);
        }
        PS_PRINT_INFO("[dcxo] gnss self part a[%d]= Dec(%d) Hex(0x%x)\n", loop, *pl_coeff, *pl_coeff);

        pl_coeff++;
    }

    if (l_invld_cnt > 0)
    {
        return INI_FAILED;
    }

    OS_MEM_CPY(pst_dl_para->coeff.para, pst_info->coeff_u.gnss.para, OAL_SIZEOF(*pl_coeff)*GNSS_COEFF_NUM);

    return INI_SUCC;
}

oal_int32 process_coeff_nv_buffer(oal_int32 part_id, oal_int8 *buf, dcxo_dl_para_stru *pst_dl_para)
{
    oal_int32 ret = INI_FAILED;
    switch (part_id)
    {
        case PLAT_DEF_ID:
            ret = process_plat_cali(buf, pst_dl_para);
        break;

        case NV_FAC_ID:
            ret = process_factory_cali(buf, pst_dl_para);
        break;

        case NV_GNSS_ID:
            ret = process_gnss_self_study(buf, pst_dl_para);
        break;

        default:
        break;
    }

    if (INI_SUCC == ret)
        g_st_dcxo_info.use_part_id = part_id;

    return ret;
}

oal_int32 get_dcxo_coeff(dcxo_dl_para_stru *pst_dl_para)
{
    oal_uint8           *buffer = NULL;
    oal_int32           ret = INI_FAILED;
    oal_int32           loop;
    oal_int32           nv_size;

    if (NULL == pst_dl_para)
    {
        PS_PRINT_ERR("[dcxo] pst_dl_para input is NULL \n");
        return INI_FAILED;
    }

    buffer = OS_KZALLOC_GFP(MALLOC_LEN);
    if (NULL == buffer)
    {
        PS_PRINT_ERR("[dcxo] alloc coeff mem failed \n");
        return INI_FAILED;
    }

    nv_size = OAL_ARRAY_SIZE(gst_dcxo_nv_part);
    for (loop = 0; loop < nv_size; loop++)
    {
        if (INI_FAILED == read_coeff_from_nvram(buffer, &gst_dcxo_nv_part[loop]))
        {
             break;
        }

        if (INI_FAILED == process_coeff_nv_buffer(gst_dcxo_nv_part[loop].part_id, buffer, pst_dl_para))
        {
            break;
        }

        OS_MEM_SET(buffer, 0, MALLOC_LEN);
    }

    OS_MEM_KFREE(buffer);
    return ret;

}

oal_void show_dcxo_conf_info(dcxo_dl_para_stru *pst_dl_para)
{
    oal_int32          loop;
    nv_part_str*       nv_part;

    nv_part = get_nv_part_from_id(g_st_dcxo_info.use_part_id);

    PS_PRINT_INFO("[dcxo] check data = 0x%x use mode %s, nv id = %d, name = %s \n",
        pst_dl_para->check_data, (INI_TCXO_MODE == pst_dl_para->tcxo_dcxo_flag)? "tcxo" : "dcxo",
        g_st_dcxo_info.use_part_id, (NULL == nv_part)? NULL : nv_part->name);

    for (loop = 0; loop < COEFF_NUM; loop++)
    {
        PS_PRINT_INFO("[dcxo] a[%d] = 0x%x \n", loop, pst_dl_para->coeff.para[loop]);
    }

}

oal_int32 read_dcxo_cali_data(oal_void)
{
    dcxo_dl_para_stru       *pst_dl_para;

    pst_dl_para = get_dcxo_data_buf_addr();
    if (NULL == pst_dl_para)
    {
        PS_PRINT_ERR("[dcxo][read_dcxo_cali_data] pst_para is NULL\n");
        return INI_FAILED;
    }

    if (get_dcxo_coeff(pst_dl_para) == INI_FAILED)
    {
        PS_PRINT_ERR("[dcxo] not read coeff from gnss part\n");
    }

    show_dcxo_conf_info(pst_dl_para);

    return INI_SUCC;
}

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

