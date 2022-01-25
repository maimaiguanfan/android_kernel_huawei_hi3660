



#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 ͷ�ļ�����
*****************************************************************************/
#include "wlan_spec.h"
#include "mac_board.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_MAC_BOARD_C


/*****************************************************************************
  2 ȫ�ֱ�������
*****************************************************************************/
/* HOST CRX�ӱ� */
frw_event_sub_table_item_stru g_ast_dmac_host_crx_table_etc[HMAC_TO_DMAC_SYN_BUTT];

/* DMACģ�飬HOST_DRX�¼�������ע��ṹ���� */
frw_event_sub_table_item_stru g_ast_dmac_tx_host_drx_etc[DMAC_TX_HOST_DRX_BUTT];

/* DMACģ�飬WLAN_DTX�¼�������ע��ṹ���� */
frw_event_sub_table_item_stru g_ast_dmac_tx_wlan_dtx_etc[DMAC_TX_WLAN_DTX_BUTT];

#ifndef _PRE_WLAN_PROFLING_MIPS
#if (_PRE_OS_VERSION_RAW == _PRE_OS_VERSION)  && defined (__CC_ARM)
#pragma arm section rwdata = "BTCM", code ="ATCM", zidata = "BTCM", rodata = "ATCM"
#endif
#endif
/* DMACģ�飬WLAN_CTX�¼�������ע��ṹ���� */
frw_event_sub_table_item_stru g_ast_dmac_wlan_ctx_event_sub_table_etc[DMAC_WLAN_CTX_EVENT_SUB_TYPE_BUTT];

/* DMACģ��,WLAN_DRX�¼�������ע��ṹ���� */
frw_event_sub_table_item_stru g_ast_dmac_wlan_drx_event_sub_table_etc[HAL_WLAN_DRX_EVENT_SUB_TYPE_BUTT];

#if ((_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV) || (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_HOST))
/* DMACģ�飬high prio�¼�������ע��ṹ�嶨�� */
frw_event_sub_table_item_stru g_ast_dmac_high_prio_event_sub_table_etc[HAL_EVENT_DMAC_HIGH_PRIO_SUB_TYPE_BUTT];
#else
/* DMACģ�飬ERROR_IRQ�¼�������ע��ṹ�嶨�� */
frw_event_sub_table_item_stru g_ast_dmac_high_prio_event_sub_table_etc[HAL_EVENT_ERROR_IRQ_SUB_TYPE_BUTT];
#endif

/* DMACģ��,WLAN_CRX�¼�������ע��ṹ���� */
frw_event_sub_table_item_stru g_ast_dmac_wlan_crx_event_sub_table_etc[HAL_WLAN_CRX_EVENT_SUB_TYPE_BUTT];

/* DMACģ�飬TX_COMP�¼�������ע��ṹ���� */
frw_event_sub_table_item_stru g_ast_dmac_tx_comp_event_sub_table_etc[HAL_TX_COMP_SUB_TYPE_BUTT];

/* DMACģ��, TBTT�¼����������� */
frw_event_sub_table_item_stru g_ast_dmac_tbtt_event_sub_table_etc[HAL_EVENT_TBTT_SUB_TYPE_BUTT];

/*DMACģ��, MISC�¼����������� */
frw_event_sub_table_item_stru g_ast_dmac_misc_event_sub_table_etc[HAL_EVENT_DMAC_MISC_SUB_TYPE_BUTT];

/* WLAN_DTX �¼������ͱ� */
frw_event_sub_table_item_stru g_ast_hmac_wlan_dtx_event_sub_table_etc[DMAC_TX_WLAN_DTX_BUTT];

/* HMACģ�� WLAN_DRX�¼�������ע��ṹ���� */
frw_event_sub_table_item_stru g_ast_hmac_wlan_drx_event_sub_table_etc[DMAC_WLAN_DRX_EVENT_SUB_TYPE_BUTT];

/* HMACģ�� WLAN_CRX�¼�������ע��ṹ���� */
frw_event_sub_table_item_stru g_ast_hmac_wlan_crx_event_sub_table_etc[DMAC_WLAN_CRX_EVENT_SUB_TYPE_BUTT];

/* HMACģ�� TBTT�¼�������ע��ṹ���� */
frw_event_sub_table_item_stru g_ast_hmac_tbtt_event_sub_table_etc[DMAC_TBTT_EVENT_SUB_TYPE_BUTT];

/* HMACģ�� ����HOST��������¼�������ע��ṹ���� */
frw_event_sub_table_item_stru g_ast_hmac_wlan_ctx_event_sub_table_etc[DMAC_TO_HMAC_SYN_BUTT];

/* HMACģ�� MISC��ɢ�¼�������ע��ṹ���� */
frw_event_sub_table_item_stru g_ast_hmac_wlan_misc_event_sub_table_etc[DMAC_MISC_SUB_TYPE_BUTT];

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
/* HMACģ�� IPC�¼�������ע��ṹ���� */
frw_event_sub_table_item_stru g_ast_hmac_host_drx_event_sub_table[DMAC_TX_HOST_DRX_BUTT];
#endif


#ifndef _PRE_WLAN_PROFLING_MIPS
#if (_PRE_OS_VERSION_RAW == _PRE_OS_VERSION)  && defined (__CC_ARM)
#pragma arm section rodata, code, rwdata, zidata  // return to default placement
#endif
#endif



/*****************************************************************************
  3 ����ʵ��
*****************************************************************************/


oal_void  event_fsm_table_register_etc(oal_void)
{
    /* Part1: ������Dmac�յ��¼�*/

    /* ע��DMACģ��HOST_CRX�¼� */
    frw_event_table_register_etc(FRW_EVENT_TYPE_HOST_CRX, FRW_EVENT_PIPELINE_STAGE_1, g_ast_dmac_host_crx_table_etc);

    /* ע��DMACģ��HOST_DRX�¼��������� */
    frw_event_table_register_etc(FRW_EVENT_TYPE_HOST_DRX, FRW_EVENT_PIPELINE_STAGE_1, g_ast_dmac_tx_host_drx_etc);

    /* ע��DMACģ��WLAN_DTX�¼��������� */
    frw_event_table_register_etc(FRW_EVENT_TYPE_WLAN_DTX, FRW_EVENT_PIPELINE_STAGE_1, g_ast_dmac_tx_wlan_dtx_etc);

    /* ע��DMACģ��WLAN_CTX�¼��������� */
    frw_event_table_register_etc(FRW_EVENT_TYPE_WLAN_CTX, FRW_EVENT_PIPELINE_STAGE_1, g_ast_dmac_wlan_ctx_event_sub_table_etc);

    /* ע��DMACģ��WLAN_DRX�¼��ӱ� */
    frw_event_table_register_etc(FRW_EVENT_TYPE_WLAN_DRX, FRW_EVENT_PIPELINE_STAGE_0, g_ast_dmac_wlan_drx_event_sub_table_etc);

    /* ע��DMACģ��WLAN_CRX�¼�pipeline 0�ӱ� */
    frw_event_table_register_etc(FRW_EVENT_TYPE_WLAN_CRX, FRW_EVENT_PIPELINE_STAGE_0, g_ast_dmac_wlan_crx_event_sub_table_etc);

    /* ע��DMACģ��TX_COMP�¼��ӱ� */
    frw_event_table_register_etc(FRW_EVENT_TYPE_WLAN_TX_COMP, FRW_EVENT_PIPELINE_STAGE_0, g_ast_dmac_tx_comp_event_sub_table_etc);

    /* ע��DMACģ��TBTT�¼��ֱ� */
    frw_event_table_register_etc(FRW_EVENT_TYPE_TBTT, FRW_EVENT_PIPELINE_STAGE_0, g_ast_dmac_tbtt_event_sub_table_etc);

#if ((_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV) || (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_HOST))
    /* ע��DMACģ��ʵʱ�¼��ӱ� */
#else
    /* ע��DMACģ��ERR�¼��ӱ� */
#endif
    frw_event_table_register_etc(FRW_EVENT_TYPE_HIGH_PRIO, FRW_EVENT_PIPELINE_STAGE_0, g_ast_dmac_high_prio_event_sub_table_etc);

    /* ע��DMACģ��MISC�¼��ֱ� */
    frw_event_table_register_etc(FRW_EVENT_TYPE_DMAC_MISC, FRW_EVENT_PIPELINE_STAGE_0, g_ast_dmac_misc_event_sub_table_etc);
    /* Part2: ������Hmac�յ��¼�*/

    /* ע��WLAN_DTX�¼��ӱ� */
    frw_event_table_register_etc(FRW_EVENT_TYPE_WLAN_DTX, FRW_EVENT_PIPELINE_STAGE_0, g_ast_hmac_wlan_dtx_event_sub_table_etc);

    /* ע��WLAN_DRX�¼��ӱ� */
    frw_event_table_register_etc(FRW_EVENT_TYPE_WLAN_DRX, FRW_EVENT_PIPELINE_STAGE_1, g_ast_hmac_wlan_drx_event_sub_table_etc);

    /* ע��HMACģ��WLAN_CRX�¼��ӱ� */
    frw_event_table_register_etc(FRW_EVENT_TYPE_WLAN_CRX, FRW_EVENT_PIPELINE_STAGE_1, g_ast_hmac_wlan_crx_event_sub_table_etc);

     /* ע��DMACģ��MISC�¼��ֱ� */
    frw_event_table_register_etc(FRW_EVENT_TYPE_DMAC_MISC, FRW_EVENT_PIPELINE_STAGE_1, g_ast_hmac_wlan_misc_event_sub_table_etc);

    /* ע��TBTT�¼��ӱ� */
    frw_event_table_register_etc(FRW_EVENT_TYPE_TBTT, FRW_EVENT_PIPELINE_STAGE_1, g_ast_hmac_tbtt_event_sub_table_etc);

    /* ע��ͳ�ƽ����ѯ�¼��ӱ� */
    frw_event_table_register_etc(FRW_EVENT_TYPE_HOST_SDT_REG, FRW_EVENT_PIPELINE_STAGE_1, g_ast_hmac_wlan_ctx_event_sub_table_etc);
}


oal_void  event_fsm_unregister_etc(oal_void)
{
    /* Part1: ������Dmac����¼�*/

    /* ȥע��DMACģ��HOST_CRX�¼� */
    OAL_MEMZERO(g_ast_dmac_host_crx_table_etc, OAL_SIZEOF(g_ast_dmac_host_crx_table_etc));

    /* ȥע��DMACģ��HOST_DRX�¼��������� */
    OAL_MEMZERO(g_ast_dmac_tx_host_drx_etc, OAL_SIZEOF(g_ast_dmac_tx_host_drx_etc));

    /* ȥע��DMACģ��WLAN_DTX�¼��������� */
    OAL_MEMZERO(g_ast_dmac_tx_wlan_dtx_etc, OAL_SIZEOF(g_ast_dmac_tx_wlan_dtx_etc));

    /* ȥע��DMACģ��WLAN_DRX�¼��ӱ� */
    OAL_MEMZERO(g_ast_dmac_wlan_drx_event_sub_table_etc, OAL_SIZEOF(g_ast_dmac_wlan_drx_event_sub_table_etc));

    /* ȥע��DMACģ��WLAN_CRX�¼��ӱ� */
    OAL_MEMZERO(g_ast_dmac_wlan_crx_event_sub_table_etc, OAL_SIZEOF(g_ast_dmac_wlan_crx_event_sub_table_etc));

    /* ȥע��DMACģ��TX_COMP�¼��ӱ� */
    OAL_MEMZERO(g_ast_dmac_tx_comp_event_sub_table_etc, OAL_SIZEOF(g_ast_dmac_tx_comp_event_sub_table_etc));

    /* ȥע��DMACģ��TBTT�¼��ֱ� */
    OAL_MEMZERO(g_ast_dmac_tbtt_event_sub_table_etc, OAL_SIZEOF(g_ast_dmac_tbtt_event_sub_table_etc));

#if ((_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV) || (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_HOST))
    /* ȥע��DMACģ��ʵʱ�¼��ӱ� */
#else
    /* ȥע��DMACģ��ERR�¼��ӱ� */
#endif
    OAL_MEMZERO(g_ast_dmac_high_prio_event_sub_table_etc, OAL_SIZEOF(g_ast_dmac_high_prio_event_sub_table_etc));

    /* ȥע��DMACģ����ɢ�¼��ӱ� */
    OAL_MEMZERO(g_ast_dmac_misc_event_sub_table_etc, OAL_SIZEOF(g_ast_dmac_misc_event_sub_table_etc));

    /* Part2: ������Hmac����¼�*/
    OAL_MEMZERO(g_ast_hmac_wlan_dtx_event_sub_table_etc, OAL_SIZEOF(g_ast_hmac_wlan_dtx_event_sub_table_etc));

    OAL_MEMZERO(g_ast_hmac_wlan_drx_event_sub_table_etc, OAL_SIZEOF(g_ast_hmac_wlan_drx_event_sub_table_etc));

    OAL_MEMZERO(g_ast_hmac_wlan_crx_event_sub_table_etc, OAL_SIZEOF(g_ast_hmac_wlan_crx_event_sub_table_etc));

    OAL_MEMZERO(g_ast_hmac_tbtt_event_sub_table_etc, OAL_SIZEOF(g_ast_hmac_tbtt_event_sub_table_etc));

    OAL_MEMZERO(g_ast_hmac_wlan_ctx_event_sub_table_etc, OAL_SIZEOF(g_ast_hmac_wlan_ctx_event_sub_table_etc));

    OAL_MEMZERO(g_ast_hmac_wlan_misc_event_sub_table_etc, OAL_SIZEOF(g_ast_hmac_wlan_misc_event_sub_table_etc));

}



/*lint -e19*/
oal_module_symbol(g_ast_dmac_host_crx_table_etc);
oal_module_symbol(g_ast_dmac_tx_host_drx_etc);
oal_module_symbol(g_ast_dmac_tx_wlan_dtx_etc);
oal_module_symbol(g_ast_dmac_wlan_ctx_event_sub_table_etc);
oal_module_symbol(g_ast_dmac_wlan_drx_event_sub_table_etc);
oal_module_symbol(g_ast_dmac_high_prio_event_sub_table_etc);
oal_module_symbol(g_ast_dmac_wlan_crx_event_sub_table_etc);
oal_module_symbol(g_ast_dmac_tx_comp_event_sub_table_etc);
oal_module_symbol(g_ast_dmac_tbtt_event_sub_table_etc);
oal_module_symbol(g_ast_dmac_misc_event_sub_table_etc);

oal_module_symbol(g_ast_hmac_wlan_dtx_event_sub_table_etc);
oal_module_symbol(g_ast_hmac_wlan_drx_event_sub_table_etc);
oal_module_symbol(g_ast_hmac_wlan_crx_event_sub_table_etc);
oal_module_symbol(g_ast_hmac_tbtt_event_sub_table_etc);
oal_module_symbol(g_ast_hmac_wlan_ctx_event_sub_table_etc);
oal_module_symbol(g_ast_hmac_wlan_misc_event_sub_table_etc);
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC != _PRE_MULTI_CORE_MODE)
oal_module_symbol(g_ast_hmac_host_drx_event_sub_table);
#endif


oal_module_symbol(event_fsm_table_register_etc);
oal_module_symbol(event_fsm_unregister_etc);

/*lint +e19*/


#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif


