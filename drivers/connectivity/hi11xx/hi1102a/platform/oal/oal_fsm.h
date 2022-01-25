
#ifndef __OAL_FSM_H__
#define __OAL_FSM_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  1 ����ͷ�ļ�����
*****************************************************************************/
//#undef  THIS_FILE_ID
//#define THIS_FILE_ID OAM_FILE_ID_OAL_FSM_H
/*****************************************************************************
  2 �궨��
*****************************************************************************/
#define OAL_FSM_MAX_NAME    8  /*״̬������󳤶�*/
#define OAL_FSM_MAX_STATES 100
#define OAL_FSM_MAX_EVENTS 100
#define OAL_FSM_STATE_NONE 255 /* invalid state */
#define OAL_FSM_EVENT_NONE 255 /* invalid event */
/*****************************************************************************
  3 ö�ٶ���
*****************************************************************************/

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

/*״̬��Ϣ�ṹ����*/
typedef struct __oal_fsm_state_info{
    oal_uint32          state;                   /*״̬ID*/
    const oal_int8      *name;                   /*״̬��*/
    oal_void (*oal_fsm_entry)(oal_void *p_ctx);  /*���뱾״̬�Ĵ���ص�����ָ��*/
    oal_void (*oal_fsm_exit)(oal_void *p_ctx);   /*�˳���״̬�Ĵ���ص�����ָ��*/
                                                 /*��״̬�µ��¼�����ص�����ָ��*/
    oal_uint32 (*oal_fsm_event)(oal_void *p_ctx,oal_uint16 event,oal_uint16 event_data_len,oal_void *event_data);
} oal_fsm_state_info;

/*״̬���ṹ����*/
typedef struct  __oal_fsm {
    oal_uint8  uc_name[OAL_FSM_MAX_NAME];              /*״̬������ */
    oal_uint8  uc_cur_state;                           /*��ǰ״̬*/
    oal_uint8  uc_prev_state;                          /*ǰһ״̬������״̬�л��¼���״̬ */
    oal_uint8  uc_num_states;                          /*״̬����״̬����*/
    oal_uint8  uc_rsv[1];
    const oal_fsm_state_info *p_state_info;
    oal_void   *p_ctx;                                 /*�����ģ�ָ��״̬��ʵ��ӵ���� */
    oal_void   *p_oshandler;                           /*ownerָ�룬ָ��VAP����device,�ɾ����״̬������*/
    oal_uint16  us_last_event;                          /*�������¼�*/
    oal_uint8  uc_rsv1[2];
} oal_fsm_stru;


/*****************************************************************************
  8 UNION����
*****************************************************************************/


/*****************************************************************************
  9 OTHERS����
*****************************************************************************/


/*****************************************************************************
  10 ��������
*****************************************************************************/
extern oal_uint32  oal_fsm_create(oal_void            *p_oshandle,         /*״̬��owner��ָ�룬�Ե͹���״̬����ָ��VAP�ṹ*/
                                const oal_uint8          *p_name,             /*״̬��������*/
                                oal_void                 *p_ctx,              /*״̬��context*/
                                oal_fsm_stru             *pst_oal_fsm,          /* oal״̬������ */
                                oal_uint8                 uc_init_state,      /*��ʼ״̬*/
                                const oal_fsm_state_info *p_state_info,       /*״̬��ʵ��ָ��*/
                                oal_uint8                 uc_num_states     /*��״̬����״̬����*/
);

//extern oal_void oal_fsm_destroy(oal_fsm_stru* p_fsm);
extern oal_uint32 oal_fsm_trans_to_state(oal_fsm_stru* p_fsm,oal_uint8 uc_state);

extern oal_uint32 oal_fsm_event_dispatch(oal_fsm_stru* p_fsm ,oal_uint16 us_event,
                           oal_uint16 us_event_data_len, oal_void *p_event_data);

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif
