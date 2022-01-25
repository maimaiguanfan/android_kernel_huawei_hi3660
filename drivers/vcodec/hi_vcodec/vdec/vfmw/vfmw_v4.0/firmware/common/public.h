/*--------------------------------------------------------------------------------------------------------------------------*/
/*!!Warning: This is a key information asset of Huawei Tech Co.,Ltd                                                         */
/*CODEMARK:kOyQZYzjDpyGdBAEC2GaWinjiDDUykL9e8pckESWBbMVmSWkBuyJO01cTiy3TdzKxGk0oBQa
mSMf7J4FkTpfv/JzwTgKEHe/9N4yU5rqhEDlG50G/Vcvyq2txYFjqt/X5GLe/SnYbE4r/A06
+TaRl5lB3SEwfHkLGFw8eexRQh7/B/dgSgI8INThiyYKg+hZ04gu5VZxVbhWf2kaj0i7cXDV
Nv0DVJKNzjCoYhavLSDl24fCwqYgiOgWzxpZ/6lwJDnTKsRY4a5PQylkmHJMtw==*/
/*--------------------------------------------------------------------------------------------------------------------------*/

/***********************************************************************
*
* Copyright (c) 2006 HUAWEI - All Rights Reserved
*
* File: $public.h$
* Date: $2006/11/30$
* Revision: $v1.0$
* Purpose: header file for interface & interface & storage management 
*          module. this file only define internal information. the 
*          interface related data is defined in 'vfmw.h'
*
*
* Change History:
*
* Date             Author            Change
* ====             ======            ====== 
* 2006/11/30       z56361            Original
*
*
* Dependencies:
*
************************************************************************/
#ifndef __PUBLIC_H__
#define __PUBLIC_H__


#include "basedef.h"
#include "sysconfig.h"
#include "vfmw.h"
#include "scd_drv.h"
#include "vdm_hal.h"


/***********************************************************************
      constants 
 ***********************************************************************/
#define  SCD_SEG_BLANK_LEN      (128*1024)
#define  MIN_BOTTOM_SEG_LEN     (1*1024) //SCD_SEG_BLANK_LEN  //128K̫���˰ɣ���ô����������һ���������ʱ�ܳ���������ʵ�ִ����鷳
#define  SCD_AVS_SEG_BLANK_LEN  (1024*1024)
#define  MIN_BOTTOM_AVS_SEG_LEN (1023*1024)
#define  SM_MAX_SEG_BUF_SIZE    (2*1024*1024)
#define  SM_MIN_SEG_BUF_SIZE    (256*1024)

#define  VFMW_TRUE              (0)
#define  VFMW_FALSE             (1)

// debug message print control
#define  DEFAULT_PRINT_ENABLE   (0x0)          // Ĭ�� �ر�
#define  DEFAULT_PRINT_DEVICE   (DEV_SCREEN)   // Ĭ�� 'DEV_SCREEN'������ӡ����Ļ

#define  MAX_USERDATA_NUM       (4)            // ���֧��4�����豣����IMAGE�ṹ���Ӧ

/***********************************************************************
      enum  
 ***********************************************************************/
typedef enum
{
    DEV_SCREEN = 1,
    DEV_SYSLOG,
    DEV_FILE,
    DEV_MEM
} PRINT_DEVICE_TYPE;

typedef enum
{
    PRN_FATAL = 0,       // 0.  �����쳣��fatal error��������δ֪�쳣
    PRN_ERROR,           // 1.  һ���쳣��error���������﷨����
    PRN_CTRL,            // 2.  ������Ϣ, ����״̬�ƣ��ڲ�״̬����ת��Ϣ��
    PRN_VDMREG,          // 3.  VDM�Ĵ������������ƼĴ�����״̬�Ĵ���

    PRN_DNMSG,           // 4.  ����������Ϣ������Ϣ�ֵ�ֵ��ӡ
    PRN_RPMSG,           // 5.  �޲���Ϣ���޲���Ϣ������Ϣ��
    PRN_UPMSG,           // 6.  ������Ϣ��������Ϣ������Ϣ��
    PRN_STREAM,          // 7.  ������Ϣ����ÿ������������Ϣ����ַ�����ȵȣ�

    PRN_STR_HEAD,        // 8.  ����ͷ�ֽڣ���ǰ8�ֽڣ����ڲ��ұȶԣ�
    PRN_STR_TAIL,        // 9.  ����β�ֽڣ�����8�ֽڣ����ڲ��ұȶԣ�
    PRN_STR_BODY,        // 10. �����в��ֽڣ���ǰ��8�ֽ�֮���������
    PRN_IMAGE,           // 11. ����ͼ����Ϣ��ͼ���id����ַ�����Եȣ�

    PRN_QUEUE,           // 12. ����ͼ������Ϣ�����е�ͼ�������λ�õȣ�
    PRN_REF,             // 13. �ο�ͼ��Ϣ��H264��ӡLIST�����Ϣ��MPEG��ӡ�ο�ͼ����ά������Ϣ��
    PRN_DPB,             // 14. DPB����Ϣ����dpb��صĸ�����Ϣ��
    PRN_POC,             // 15. POC����Ϣ��POC�Ľ���ͼ��㣩

    PRN_MARK_MMCO,       // 16. MARK & MMCO������Ϣ��MARK��MMCO��������Ϣ���ɱ���ִ�еĹ��̣�
    PRN_SEQ,             // 17. ���м���Ҫ�﷨��Ϣ��H264��ӡSPS��MPEG2��ӡsequence��Ϣ��MPEG4��ӡVOL�����ϲ���Ϣ
    PRN_PIC,             // 18. ͼ����Ҫ�﷨��Ϣ��H264��ӡPPS��MPEG2��ӡpicture��MPEG4��ӡVOP/SHVOPͷ
    PRN_SLICE,           // 19. Slice����Ҫ�﷨��Ϣ

    PRN_SEI,             // 20. SEI��Ϣ
    PRN_SE,              // 21. �﷨��Ϣ
    PRN_DBG,             // 22. ������Ϣ
    PRN_BLOCK,           // 23. �����߳�������Ϣ

    PRN_SCD_REGMSG,      // 24. SCD�Ĵ�������Ϣ����Ϣ
    PRN_SCD_STREAM,      // 25. SCD������Ϣ
    PRN_SCD_INFO,        // 26. ��ӡSCDһЩ����״̬����Ϣ
    PRN_CRC,             // 27. CRC����ֵ
    
	PRN_POST,            // 28. ��ӡ�󼶼����������������Ϣ
	PRN_PTS,             // 29. ��ӡ��PTS������ص���Ϣ
    PRN_DEC_MODE,        // 30. ����ģʽ�л���Ϣ
    PRN_FS,              // 31. ��̬֡���ӡ
	
    PRN_ALWS = 32        // 32. ���ܿش�ӡ
} PRINT_MSG_TYPE;

/* state type */
typedef enum
{
    STATE_RCV_RAW = 0,           //����RAW����
    STATE_SCD_START,             //����SCD
    STATE_SCD_INTERRUPT,         //SCD�ж�

    STATE_DECSYNTAX_SEG,         //�﷨����SEG����
    STATE_GENERATE_DECPARAM,     //���ɽ������

    STATE_VDH_START,             //����VDH
    STATE_VDH_INTERRUPT,         //VDH�ж�
    STATE_VDH_REPAIR,            //VDH�޲�
    STATE_1D_TO_QUEUE,           //1Dͼ�����������

    STATE_2D_TO_QUEUE,           //2Dͼ�����������

    STATE_VO_RCV_IMG,            //VO��ȡͼ��
    STATE_VO_REL_IMG             //VO�ͷ�ͼ��
} VFMW_STATE_TYPE_E;

/***********************************************************************
      structures    
 ***********************************************************************/

/* MPEG2, MPEG4 and DIVX311 dedode interface need code stream info from control module.
   Struct 'STREAM_PARAM' defines the stream info format. */
typedef struct
{
    UINT8*    VirAddr;    // �����������ַ
    UADDR     PhyAddr;    // �����������ַ
    SINT32    Length;     // �������ȣ��ֽ�����
} STREAM_PARAM;

/* �ڴ��ӡ���ݽṹ  */
typedef struct 
{      
    SINT8*    buf_addr;
    SINT8*    cur_addr_offset;
    UINT32    buf_len;
    UINT32    total_msg_num;
} MEMPRINT;

/* MPEG2/4�������ݴ滺�� */
typedef struct
{
	UINT8*    buf_vir_addr;
	SINT8     filled_new_data_flag;
    UADDR     buf_phy_addr;
	SINT32    buf_size;
	SINT32    length;
} STRM_LOCAL_BUF;

/* VDM ��������Ϣ���� */
typedef struct
{
	SINT32    DecTaskState;
} VDMCMN_DECTASK_INFO_S;

/* ���ӳٸ�ģ��ʱ��ͳ����Ϣ */
typedef struct
{
    SINT32    lowdly_enable;             /* ���ӳ�ʹ�ܱ�־ */
    SINT32    chan_id;                   /* ͨ�� ID */
	SINT32    receive_raw;               /* ��¼�յ� RAW ��ʱ�� */
	SINT32    scd_start;                 /* SCD ����ʱ�� */
	SINT32    scd_return;                /* SCD �жϷ��� */
	SINT32    scd_count;                 /* SCD ����һ�������ĵ�ʱ�� */
	SINT32    vdh_start;                 /* VDH ����ʱ�� */
	SINT32    vdh_return;                /* VDH �жϷ��� */
	SINT32    vdh_count;                 /* VDH ����һ�������ĵ�ʱ�� */
	SINT32    vo_read_img;               /* VO �ɹ�����ͼ���ʱ�� */
	SINT32    raw_to_scd_return;         /* ��VFMW�յ����������һ֡ͼ���ʱ��, ֻ�а�֡������ʱ�Ž�Ϊ׼ȷ */
	SINT32    raw_to_img;                /* ��VFMW�յ����������һ֡ͼ���ʱ��, ֻ�а�֡������ʱ�Ž�Ϊ׼ȷ */
	SINT32    raw_to_vo;                 /* ��VFMW�յ����������һ֡ͼ���ʱ�䣬ֻ�а�֡������ʱ�Ž�Ϊ׼ȷ */
	SINT32    OneTimeFlag;
}LOWDLY_INFO_S;

/***********************************************************************
      global vars   
 ***********************************************************************/
extern  UINT32                 g_PrintEnable;
extern  UINT32                 g_PrintDevice;
extern  SINT32                 g_TraceCtrl;
extern  SINT32                 g_TraceBsPeriod;
extern  SINT32                 g_TraceFramePeriod;
extern  SINT32                 g_TraceImgPeriod;
extern  LOWDLY_INFO_S          g_stLowdlyInfo[MAX_CHAN_NUM];
extern  EXT_FN_EVENT_CALLBACK  g_event_report;

// register address offset from register base address
#define    REG_VDM_STATE                  (0x01C>>2)    // VDM_STATE
#define    REG_INTERPRT_CLEAN             (0X020>>2)    // INT
#define    REG_INTERPRT_MASK              (0x024>>2)    // INT_MASK

// map first. take care!!!!

// debug related state definition
#define    DSTATE_WAIT_STREAM             (1<<1)
#define    DSTATE_WAIT_VDMFINISH          (1<<2)
#define    DSTATE_WAIT_FRAMESTORE         (1<<3)
#define    DSTATE_WAIT_VDMPROPERTY        (1<<4)

/***********************************************************************
      macro 
 ***********************************************************************/
//ע��:�кܶຯ��ǰ��Կ�ָ����жϾͿ���ȥ����
//add for check null point parament
#define CHECK_NULL_PTR_ReturnValue(parPoint,ReturnValue)                        \
    do                                                  \
    {                                                   \
        if (NULL == parPoint)                           \
        {                                               \
            dprint(PRN_FATAL, "NULL pointer: %s, L%d\n", __FILE__, __LINE__);   \
            return ReturnValue;                         \
        }                                               \
    }while(0)

#define CHECK_NULL_PTR_Return(parPoint)                 \
    do                                                  \
    {                                                   \
        if (NULL == parPoint)                           \
        {                                               \
            dprint(PRN_FATAL,"NULL pointer: %s, L%d\n", __FILE__, __LINE__);    \
            return;                 \
        }                                               \
    }while(0)
//end

/*********************** ������Ϣ�ĺ��װ *************************/

/* type=1	ͼ�����ʾ�������˱仯 */
#define REPORT_DISP_AREA_CHANGE(chan_id,w,h,center_x,center_y)                  \
do{                                                         \
    if( NULL != g_event_report )                            \
    {                                                       \
        UINT16  para[4];                                    \
        para[0] = (UINT16)(w);                              \
        para[1] = (UINT16)(h);                              \
        para[2] = (UINT16)(center_x);                       \
        para[3] = (UINT16)(center_y);                       \
        g_event_report( chan_id, EVNT_DISP_EREA, (UINT8*)para, sizeof(para));   \
    }                                                       \
}while(0)

/* type=2, ͼ���߷����˱仯 */
#define REPORT_IMGSIZE_CHANGE( chan_id, oldw, oldh, neww, newh )	            \
do{                                                         \
    if( NULL != g_event_report )                    		\
    {                                                       \
        UINT16  para[4];                                    \
        para[0] = (UINT16)(oldw);                        	\
        para[1] = (UINT16)(oldh);                          	\
        para[2] = (UINT16)(neww);                          	\
        para[3] = (UINT16)(newh);                          	\
        g_event_report( chan_id, EVNT_IMG_SIZE_CHANGE, (UINT8*)para, sizeof(para)); \
    }                                                       \
}while(0)

/* type=3, ֡�ʷ����˱仯 */
#define REPORT_FRMRATE_CHANGE( chan_id, newfr )    		    \
do{                                                      	\
    if( NULL != g_event_report )                 		    \
    {                                                       \
        UINT32  para[1];                                    \
        para[0] = (UINT32)(newfr);                         	\
        g_event_report( chan_id, EVNT_FRMRATE_CHANGE, (UINT8*)para, sizeof(para)); \
    }                                                       \
}while(0)

/* type=4, ͼ�������/������Ϣ�仯���޲��� */
#define REPORT_SCAN_CHANGE( chan_id, newscan ) 			    \
do{                                                         \
    if( NULL != g_event_report )                    		\
    {                                                       \
        UINT32  para[1];                                    \
        para[0] = (UINT8)(newscan);                 		\
        g_event_report( chan_id, EVNT_SCAN_CHANGE, (UINT32*)para, sizeof(para));\
    }                                                       \
}while(0)

/* type=5, ��һ֡ͼ�������� */
#define REPORT_IMGRDY( chan_id )            		        \
do{                                                         \
    if( NULL != g_event_report )                    		\
    {                                                       \
        g_event_report( chan_id, EVNT_NEW_IMAGE, (UINT8*)0, 0);                 \
    }                                                       \
}while(0)

/* type=6, ������user data */
#define REPORT_USRDAT( chan_id, p_usrdat )              	\
do{                                                         \
    if( NULL != g_event_report )                    		\
    {                                                       \
        g_event_report( chan_id, EVNT_USRDAT, (VDEC_USRDAT_S*)p_usrdat, sizeof(VDEC_USRDAT_S));	 \
    }                                                       \
}while(0)

/* type=7  ���ֿ�߱ȷ����仯 */
#define REPORT_ASPR_CHANGE( chan_id, oldasp, newasp )  	    \
do{                                                         \
    if( NULL != g_event_report )                    		\
    {                                                       \
        UINT32  para[2];                                    \
        para[0] = (UINT8)(oldasp);                        	\
        para[1] = (UINT8)(newasp);                       	\
        g_event_report( chan_id, EVNT_ASPR_CHANGE, (UINT32*)para, sizeof(para));\
    }                                                       \
}while(0)

/* type=8  �������һ���ߴ���֮ǰ��ͬ��֡ */
#define REPORT_OUTPUT_IMG_SIZE_CHANGE( chan_id, oldw, oldh, neww, newh )	    \
    do{                                                     \
        if( NULL != g_event_report )                        \
        {                                                   \
            UINT16  para[4];                                \
            para[0] = (UINT16)(oldw);                       \
            para[1] = (UINT16)(oldh);                       \
            para[2] = (UINT16)(neww);                       \
            para[3] = (UINT16)(newh);                       \
            g_event_report( chan_id, EVNT_OUTPUT_IMG_SIZE_CHANGE, (UINT8*)para, sizeof(para)); \
        }                                                   \
    }while(0)


/* type=20  ����һ��I֡��׼�������I֡ */
#define REPORT_FIND_IFRAME( chan_id, stream_size )          \
do{                                                         \
    if( NULL != g_event_report )                    		\
    {                                                       \
        UINT32  para[1];                                    \
        para[0] = stream_size;                              \
        g_event_report( chan_id, EVNT_FIND_IFRAME, (UINT8*)para, sizeof(para)); \
    }                                                       \
}while(0)

/*--------------------------------------------------------------*/
/* type=100 ������������ p_args[3..0]��������*/
#define REPORT_STRMERR(chan_id, err_code )          		\
do{                                                         \
    if( NULL != g_event_report )                    		\
    {                                                       \
        UINT32  para[1];                                    \
        para[0] = (UINT32)(err_code);                     	\
        g_event_report( chan_id, EVNT_STREAM_ERR, (UINT8*)para, sizeof(para));	\
    }                                                       \
}while(0)

/* type=101 VDM����Ӧ */
#define REPORT_VDMERR(chan_id)                           	\
do{                                                         \
    if( NULL != g_event_report )                    		\
    {                                                       \
        g_event_report( chan_id, EVNT_VDM_ERR, (UINT8*)0, 0); 	                \
    }                                                       \
}while(0)

/* type=102 ���ֲ�֧�ֵĹ�� �� */
#define REPORT_UNSUPPORT(chan_id)                       	\
do{                                                         \
    if( NULL != g_event_report )                    		\
    {                                                       \
        g_event_report( chan_id, EVNT_UNSUPPORT, (UINT8*)0, 0);  	            \
    }                                                       \
}while(0)

/* type=103	�������﷨����	�� */
#define REPORT_SE_ERR(chan_id)                           	\
do{                                                         \
    if( NULL != g_event_report )                    		\
    {                                                       \
        g_event_report( chan_id, EVNT_SE_ERR, (UINT8*)0, 0); 		            \
    }                                                       \
}while(0)

/* type=104  ͼ������ʳ���ref_error_thr	�� */
#define REPORT_OVER_REFTHR(chan_id, CurrRate, RefRate)      \
do{ 														\
	if( NULL != g_event_report )							\
	{														\
		UINT32	para[2];									\
		para[0] = (UINT32)(CurrRate);						\
		para[1] = (UINT32)(RefRate);					    \
		g_event_report( chan_id, EVNT_OVER_REFTHR, (UINT8*)para, sizeof(para)); \
	}														\
}while(0)

/* type=105  ͼ������ʳ���out_error_thr	�� */
#define REPORT_OVER_OUTTHR(chan_id)                     	\
do{                                                         \
    if( NULL != g_event_report )                    		\
    {                                                       \
        g_event_report( chan_id, EVNT_OVER_OUTTHR, (UINT8*)0, 0);               \
    }                                                       \
}while(0)

/* type=106  �ο�֡�����������趨ֵ p_args[3..0]���ο�֡������ p_args[7..4]���趨ֵ */
#define REPORT_REF_NUM_OVER(chan_id, RefNum, MaxRefNum)    	\
do{                                                         \
    if( NULL != g_event_report )                    		\
    {                                                       \
        UINT32  para[2];                                    \
        para[0] = RefNum;                                   \
        para[1] = MaxRefNum;                                \
        g_event_report( chan_id, EVNT_REF_NUM_OVER, (UINT8*)para, sizeof(para));\
    }                                                       \
}while(0)

/* type=107, ͼ���߳�����ֵ */
#define REPORT_IMGSIZE_OVER( chan_id, w, h, maxw, maxh )	\
do{                                                         \
    if( NULL != g_event_report )                    		\
    {                                                       \
        UINT16  para[4];                                    \
        para[0] = (UINT16)(w);                        	    \
        para[1] = (UINT16)(h);                          	\
        para[2] = (UINT16)(maxw);                          	\
        para[3] = (UINT16)(maxh);                          	\
        g_event_report( chan_id, EVNT_SIZE_OVER, (UINT8*)para, sizeof(para));   \
    }                                                       \
}while(0)

/* type=108, slice����������ֵ */
#define REPORT_SLICE_NUM_OVER( chan_id, SliceNum, MaxSliceNum )	                \
do{                                                         \
    if( NULL != g_event_report )                    		\
    {                                                       \
        UINT32  para[2];                                    \
        para[0] = SliceNum;                         	    \
        para[1] = MaxSliceNum;                          	\
        g_event_report( chan_id, EVNT_SLICE_NUM_OVER, (UINT8*)para, sizeof(para)); \
    }                                                       \
}while(0)

/* type=109, sps����������ֵ */
#define REPORT_SPS_NUM_OVER( chan_id, SpsNum, MaxSpsNum )	\
do{                                                         \
    if( NULL != g_event_report )                    		\
    {                                                       \
        UINT32  para[2];                                    \
        para[0] = SpsNum;                           	    \
        para[1] = MaxSpsNum;                            	\
        g_event_report( chan_id, EVNT_SPS_NUM_OVER, (UINT8*)para, sizeof(para));\
    }                                                       \
}while(0)

/* type=110, pps����������ֵ */
#define REPORT_PPS_NUM_OVER( chan_id, PpsNum, MaxPpsNum )	\
do{                                                         \
    if( NULL != g_event_report )                    		\
    {                                                       \
        UINT32  para[2];                                    \
        para[0] = PpsNum;                            	    \
        para[1] = MaxPpsNum;                             	\
        g_event_report( chan_id, EVNT_PPS_NUM_OVER, (UINT8*)para, sizeof(para));\
    }                                                       \
}while(0)

/* type=111  ����I֡�������    �� */
#define REPORT_IFRAME_ERR(chan_id)                     	    \
do{                                                         \
    if( NULL != g_event_report )                            \
    {                                                       \
        g_event_report( chan_id, EVNT_IFRAME_ERR, (UINT8*)0, 0);                \
    }                                                       \
}while(0)

/* type=112  �����µ�֡������    �� */
#define REPORT_MEET_NEWFRM(chan_id)                     	\
do{                                                         \
    if( NULL != g_event_report )                            \
    {                                                       \
        g_event_report( chan_id, EVNT_MEET_NEWFRM, (UINT8*)0, 0);               \
    }                                                       \
}while(0)

/* type=113  ������������*/
#define REPORT_DECSYNTAX_ERR(chan_id)                     	\
do{ 														\
    if( NULL != g_event_report )							\
    {														\
        g_event_report( chan_id, EVNT_DECSYNTAX_ERR, (UINT8*)0, 0);             \
    }														\
}while(0)

/* type=114  �����ʷ����ϱ� */
#define REPORT_RATIO_NOTZERO(chan_id, ErrRatio)                                 \
do{ 														\
    if( NULL != g_event_report )							\
    {														\
        UINT32  para[1];                                    \
        para[0] = ErrRatio;                                 \
        g_event_report( chan_id, EVNT_RATIO_NOTZERO, (UINT8*)para, sizeof(para));  \
    }														\
}while(0)

/* type=115  ���һ֡�������ϱ� */
#define REPORT_LAST_FRAME(chan_id, flag)                    \
do{ 														\
    if( NULL != g_event_report )							\
    {														\
        UINT32  para[1];                                    \
        para[0] = (UINT32)(flag);                     	    \
        g_event_report( chan_id, EVNT_LAST_FRAME, (UINT8*)para, sizeof(para));  \
    }														\
}while(0)

/* type=116  ��ֱ������·ָ�֡���ϱ� */
#define REPORT_RESOLUTION_CHANGE(chan_id)                   \
do{ 														\
    if( NULL != g_event_report )							\
    {														\
        g_event_report( chan_id, EVNT_RESOLUTION_CHANGE, (UINT8*)0, 0);         \
    }														\
}while(0)
	
/* type=117  �����������ϱ� */
#define REPORT_STREAM_END_SYNTAX(chan_id)                   \
do{ 														\
    if( NULL != g_event_report )							\
    {														\
	    g_event_report( chan_id, EVNT_STREAM_END_SYNTAX, (UINT8*)0, 0);         \
	}														\
}while(0)

/* type=118 ������Ϣ�ϱ�����֡��*/
#define REPORT_NEED_ARRANGE(chan_id, FrmNum, FrmSize, PmvSize, DecWidth, DecHeight, Stride, DispWidth, DispHeight)                     	\
do{ 														\
	if( NULL != g_event_report )							\
	{														\
        UINT32  para[8];                                    \
        para[0] = (UINT32)(FrmNum);                         \
        para[1] = (UINT32)(FrmSize);                        \
        para[2] = (UINT32)(PmvSize);                        \
        para[3] = (UINT32)(DecWidth);                       \
        para[4] = (UINT32)(DecHeight);                      \
        para[5] = (UINT32)(Stride);                         \
        para[6] = (UINT32)(DispWidth);                      \
        para[7] = (UINT32)(DispHeight);                     \
        g_event_report(chan_id, EVNT_NEED_ARRANGE, (UINT8*)para, sizeof(para));  \
	}														\
}while(0)
    
    /* type=119 ���ֲ�֧�ֵĹ�� �ϱ����ͼ���ֵ */
#define REPORT_UNSUPPORT_SPEC(chan_id, unsupport_spec, data)                       \
do{                                                         \
    if( NULL != g_event_report )                            \
    {                                                       \
        UINT32  para[2];                                    \
        para[0] = unsupport_spec;                           \
        para[1] = data;                                     \
        g_event_report( chan_id, EVNT_UNSUPPORT_SPEC, (UINT8*)para, sizeof(para)); \
    }                                                       \
}while(0)
    
/* type=120  ��ֱ���ʱ�����֡�ú󼶻�֡ */
#define REPORT_FAKE_FRAME(chan_id)                     	    \
do{                                                         \
    if( NULL != g_event_report )                            \
    {                                                       \
        g_event_report( chan_id, EVNT_FAKE_FRAME, (UINT8*)0, 0);                \
    }                                                       \
}while(0)

/* type=122  Report color aspects info */
#define REPORT_COLOR_ASPECTS_INFO(chan_id, para, len)                  \
do {                                                        \
    if (NULL != g_event_report)                             \
    {                                                       \
        g_event_report(chan_id, EVNT_COLOR_ASPECTS_CHG, (UINT8*)para, len); \
    }                                                       \
} while (0)

#define dprint_sos_kernel(type, fmt, arg...)                        \
do{                                                                 \
    if (((PRN_ALWS == type) || (0 != (g_PrintEnable & (1LL << type)))) \
        && (DEV_SCREEN == g_PrintDevice))                            \
    {                                                               \
        VFMW_OSAL_Print("S: ");                                     \
        VFMW_OSAL_Print(fmt, ##arg);                                \
    }                                                               \
}while(0)


#ifdef HI_ADVCA_FUNCTION_RELEASE
// �߰����ܼ�⵽�ַ�����printk
#define dprint(type, fmt, arg...)  vfmw_dprint_nothing()
#else

#ifdef ENV_SOS_KERNEL
#define dprint(type, fmt, arg...)  dprint_sos_kernel(type, fmt, ##arg)
#else
#define dprint(type, fmt, arg...)  dprint_linux_kernel(type, fmt, ##arg)
#endif

#endif
 

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************
      functions 
 ***********************************************************************/
VOID   vfmw_dprint_nothing(VOID);
SINT32 IsDprintTypeEnable(UINT32 type);

#ifdef ENV_ARMLINUX_KERNEL
SINT32 dprint_linux_kernel( UINT32 type, const SINT8 *format, ... );
#endif

#ifdef VFMW_RECPOS_SUPPORT
#define  DBG_OK       (0)
#define  DBG_ERR     (-1)
SINT32   DBG_CreateTracer(VOID);
VOID     DBG_DestroyTracer(VOID);
VOID     DBG_AddTrace( SINT8 *pFuncName, SINT32 nLine, SINT32 Data );
VOID     DBG_GetTracerInfo( SINT32 *pMemPhyAddr, SINT32 *pMemLength, ULONG *pTracePhyAddr, SINT32 *pMaxNum, SINT32 *pCurIdx );
VOID     DBG_PrintTracer( SINT32 TraceDepth );
#endif

SINT32 CHECK_REC_POS_ENABLE(SINT32 type);

#ifdef VFMW_SCD_LOWDLY_SUPPORT
VOID DBG_CountTimeInfo(SINT32 ChanID, VFMW_STATE_TYPE_E SateType, SINT32 LowdlyFlag);
#endif

VOID     qsort( VOID *base, size_t num, size_t width, SINT32(*comp)(const VOID*, const VOID*) );

#ifdef __cplusplus
}
#endif

#endif

