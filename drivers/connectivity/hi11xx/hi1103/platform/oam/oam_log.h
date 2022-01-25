

#ifndef __OAM_LOG_H__
#define __OAM_LOG_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 ����ͷ�ļ�����
*****************************************************************************/
#include "oal_ext_if.h"

/*****************************************************************************
  2 �궨��
*****************************************************************************/
#define OAM_LOG_PARAM_MAX_NUM           4                                       /* �ɴ�ӡ���Ĳ������� */
#define OAM_LOG_PRINT_DATA_LENGTH       512                                     /* ÿ��д���ļ�����󳤶� */

#ifdef _PRE_WLAN_REPORT_PRODUCT_LOG
#define OAM_ONT_LOG_STRING_LIMIT        200
#define OAM_ONT_LOG_DEFAULT_EVENT       HW_KER_WIFI_LOG_BUTT        //Ĭ�ϲ����뵽ont proc log
#define OAM_ONT_LOG_MAX_COUNT           50
#define OAM_REPORT_PRODUCT_LOG_FUN(_uiChipIndex, _uiEvent, _uilevel, fmt, arg...)      hw_wifi_log_dbg(_uiChipIndex, _uiEvent, _uilevel, fmt, ##arg)

typedef struct
{
    oal_dlist_head_stru     st_list_entry;
    oal_uint8               uc_chip_id;
    oal_uint8               uc_event_id;
    oal_uint8               auc_resv[2];
    oal_int8                auc_log_string[OAM_ONT_LOG_STRING_LIMIT];
}oam_pdt_log_rpt_stru;
//ont log�ڵ���Ϣ
typedef struct
{
    oal_spin_lock_stru                       st_spin_lock;
    oal_dlist_head_stru                      st_pdt_used_list;
    oal_dlist_head_stru                      st_pdt_free_list;
    oal_work_stru                            st_pdt_log_work;
    oam_pdt_log_rpt_stru                    *pst_log_mem;
}oam_pdt_log_stru;
extern oal_void oam_pdt_log_init(oal_void);
extern oal_void oam_pdt_log_exit(oal_void);
#endif

/*****************************************************************************
  3 ö�ٶ���
*****************************************************************************/
#ifdef _PRE_WLAN_REPORT_PRODUCT_LOG
typedef enum
{
    HW_KER_WIFI_CHIP_2G_LOG = 0,
    HW_KER_WIFI_CHIP_5G_LOG,
    HW_KER_WIFI_CHIP_ALL_LOG,
    HW_KER_WIFI_CHIP_LOG_BUTT
} HW_KER_WIFI_CHIP_LOG_E; //chipIndex
typedef oal_uint32 oam_ONT_CHIP_LOG_E_enum_uint32;

typedef enum
{
    HW_KER_WIFI_LOG_CONFIG = 0, /*���ò���*/
    HW_KER_WIFI_LOG_CONNECT,    /*���Ӳ���*/
    HW_KER_WIFI_LOG_CMDOUT,     /*�洢�������*/
    HW_KER_WIFI_LOG_CHANNEL,    /*�洢�ŵ�ɨ�裬��¼�ŵ�ɨ��������һ���ŵ��뵱ǰѡ����ŵ� */
    HW_KER_WIFI_LOG_COLLISION,  /*BSSID ��ͻɨ��*/
    HW_KER_WIFI_LOG_BUTT
} HW_KER_WIFI_LOG_E;
typedef oal_uint8 oam_ont_log_enum_uint8;
#endif
/*****************************************************************************
  4 ȫ�ֱ�������
*****************************************************************************/

/*****************************************************************************
  5 ��Ϣͷ����
*****************************************************************************/


/*****************************************************************************
  6 ��Ϣ����
*****************************************************************************/


/*****************************************************************************
  7 STRUCT����
*****************************************************************************/
typedef struct
{
    oal_uint32  bit16_file_id:   16;
    oal_uint32  bit8_feature_id: 8;
    oal_uint32  bit4_vap_id:     4;
    oal_uint32  bit4_log_level:  4;
}om_log_wifi_para_stru;
/*****************************************************************************
  8 UNION����
*****************************************************************************/


/*****************************************************************************
  9 OTHERS����
*****************************************************************************/


/*****************************************************************************
  10 ��������
*****************************************************************************/
extern oal_uint32  oam_log_init_etc(oal_void);
#ifdef _PRE_WLAN_REPORT_PRODUCT_LOG
extern void hw_wifi_log_dbg(oal_uint32 uiChipIndex, oal_uint32 uiEvent, oal_uint32 uilevel, char* fmt, ...);
#endif

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of oam_log.h */
