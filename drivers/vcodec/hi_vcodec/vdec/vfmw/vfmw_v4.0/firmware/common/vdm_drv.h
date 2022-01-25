/*--------------------------------------------------------------------------------------------------------------------------*/
/*!!Warning: This is a key information asset of Huawei Tech Co.,Ltd                                                         */
/*CODEMARK:kOyQZYzjDpyGdBAEC2GaWinjiDDUykL9e8pckESWBbMVmSWkBuyJO01cTiy3TdzKxGk0oBQa
mSMf7J4FkTpfv/JzwTgKEHe/9N4yU5rqhEDGFnkG4/S4FxOOB/C/wNlmJJL1crdwYm/aqTyh
1nLqNKmrus/Fmm0BALAnCBLI/o9lsmOngR+oaESiVF4tAAaBiSgyCBrt1jqY+1dMZQGO79Vv
iUuE6IIWAMymzsapnNhLNA48hh3b+prrZETi0raYtzCm5UEoQTZQxhbBlWsayQ==*/
/*--------------------------------------------------------------------------------------------------------------------------*/

/*********************************************************************** 
*
* Copyright (c) 2009 Hisilicon - All Rights Reserved
*
* File: $vdm_drv.h$
* Date: $2009/05/08$
* Revision: $v1.0$
* Purpose: VDEC driver interface.
*
* Change History:
*
* Date       Author            Change
* ======     ======            ======
* 20090508   q45134            Original
*
************************************************************************/

#ifndef __VDM_DRV_HEADER__
#define __VDM_DRV_HEADER__
#include "vfmw.h"
#include "sysconfig.h"

#ifdef __cplusplus
extern "C" {
#endif

#define VDH_IDLE                  (0)
#define VDH_BUSY                  (1)

#define VDM_TIME_OUT              (500)        // ms
#define VDM_FPGA_TIME_OUT         (500000)     // ms

#define COUNT_WAIT_NO_ISR         (400)        // x10ms
#define MAX_TIME_WAIT_MORE_START  (200)        // ms

#define VDMDRV_OK                 (0)
#define VDMDRV_ERR                (-1)

#define MSG_SLOT_SIZE             (256)        //һ��SLOT��С, WORD
#define UP_MSG_SLOT_NUM           (2)
#define MAX_UP_MSG_SLICE_NUM      (200)        //���֧���ϱ���slice������final 200. vfmw test 40000

#define UP_MSG_SIZE               (MAX_UP_MSG_SLICE_NUM*4)  //һ��slice��Ϣ��ռ4word
#define LUMA_HISTORGAM_NUM        (32)

typedef enum
{
    VDH_STATE_REG        = 1,
    INT_STATE_REG        = 2,
    INT_MASK_REG         = 3,
    VCTRL_STATE_REG      = 4,
} REG_ID_E;

typedef enum
{
    VDM_NULL_STATE       = 0,
    VDM_DECODE_STATE     = 1,
    VDM_REPAIR_STATE_0   = 2,  // First repair
    VDM_REPAIR_STATE_1   = 3   // Second repair, only used for AVS field mode repair
} VDMDRV_STATEMACHINE_E;

typedef enum
{
    VDMDRV_SLEEP_STAGE_NONE = 0,      // δ����
    VDMDRV_SLEEP_STAGE_PREPARE,       // �յ������������δ�������
    VDMDRV_SLEEP_STAGE_SLEEP          // ������
} VDMDRV_SLEEP_STAGE_E;

typedef struct
{
    VDMDRV_STATEMACHINE_E VdmStateMachine;
    SINT32 ErrRatio;
    SINT32 ChanId;
    VID_STD_E VidStd;
    UINT32 StartTime;  // ms
    UINT32 CurrTime;   // ms
    SINT32 VdmTimeOut;
    UINT32 LastWaitMoreStartTime;
    SINT32 ChanResetFlag;
#ifdef VFMW_MODULE_LOWDLY_SUPPORT
	SINT32 ChanIntState;/*������ʾvdh�ж�״̬�ı�־��1:ͬʱ��⵽�к��жϺͽ�������ж�*/
#endif
    VOID *pDecParam;
} VDMDRV_PARAM_S;

typedef struct 
{
    SINT32 PriorByChanId[MAX_CHAN_NUM];/*��ͨ����˳���ͨ�������ȼ���Ϣ*/
    SINT32 ChanIdTabByPrior[MAX_CHAN_NUM]; /* -1: ��λֵ��û��Ҫ�����ͨ��*/
    SINT32 ChanDecByVdhPlusOne[MAX_CHAN_NUM]; /* 0: δ��VDH���룬n: ���ڱ�(VDH_id + 1)����  5: ���ڱ�VEDU���� */
} CHAN_CTX;

/* VDM�����ڴ��ַ */
typedef struct
{
    // vdm register base vir addr
    SINT32    *pVdmRegVirAddr;
    SINT32    *pPERICRGVirAddr;
    SINT32    *pBpdRegVirAddr;
	
    // vdm hal base addr
    UADDR     HALMemBaseAddr;
    SINT32    HALMemSize;
    SINT32    VahbStride;   

    /* message pool */
    UADDR     MsgSlotAddr[256];
    SINT32    ValidMsgSlotNum;

    /* vlc code table */
    UADDR     H264TabAddr;    /* 32 Kbyte */    
    UADDR     MPEG2TabAddr;   /* 32 Kbyte */
    UADDR     MPEG4TabAddr;   /* 32 Kbyte */
    UADDR     AVSTabAddr;     /* 32 Kbyte */
    UADDR     VC1TabAddr;
    /* cabac table */
    UADDR     H264MnAddr;
    /* nei info for vdh for hevc  */
    UADDR     sed_top_phy_addr;
    UADDR     pmv_top_phy_addr;
    UADDR     pmv_left_phy_addr;	
    UADDR     rcn_top_phy_addr;
    UADDR     mn_phy_addr;
    UADDR     tile_segment_info_phy_addr;
    UADDR     dblk_left_phy_addr;
    UADDR     dblk_top_phy_addr;
    UADDR     sao_left_phy_addr;
    UADDR     sao_top_phy_addr;                              
    UADDR     ppfd_phy_addr; 
	SINT32    ppfd_buf_len; 

    /*nei info for vdh */
    UADDR     SedTopAddr;     /* len = 64*4*x*/
    UADDR     PmvTopAddr;     /* len = 64*4*x*/
    UADDR     RcnTopAddr;     /* len = 64*4*x*/
    UADDR     ItransTopAddr;
    UADDR     DblkTopAddr;
    UADDR     PpfdBufAddr;
    UADDR     PpfdBufLen;

    UADDR     IntensityConvTabAddr;
    UADDR     BitplaneInfoAddr;
    UADDR     Vp6TabAddr;
    UADDR     Vp8TabAddr;
	
    UINT8*    luma_2d_vir_addr;
    UADDR     luma_2d_phy_addr;
    UINT8*    chrom_2d_vir_addr;
    UADDR     chrom_2d_phy_addr;
#ifdef VFMW_MODULE_LOWDLY_SUPPORT
	/*ģ������ʱʹ�õ��к�*/
    UADDR     line_num_stAddr;
#endif
    
} VDMHAL_HWMEM_S;

typedef struct
{
	UINT32    LumaSumHigh;
	UINT32    LumaSumLow;
	UINT32    LumaHistorgam[LUMA_HISTORGAM_NUM];
}LUMA_INFO_S;
/* ���뱨�����ݽṹ */
typedef struct
{
    UINT32    BasicCfg1;
    UINT32    VdmState;
    UINT32    Mb0QpInCurrPic;
    UINT32    SwitchRounding;
    UINT32    SedSta;
    UINT32    SedEnd0;

    UINT32    DecCyclePerPic;
    UINT32    RdBdwidthPerPic;
    UINT32    WrBdWidthPerPic;
    UINT32    RdReqPerPic;
    UINT32    WrReqPerPic;
	UINT32    LumaSumHigh;
	UINT32    LumaSumLow;
	UINT32    LumaHistorgam[LUMA_HISTORGAM_NUM];
} VDMHAL_BACKUP_S;

/* �޲��������ݽṹ */
typedef struct
{
    VID_STD_E VidStd;
    UADDR     ImageAddr;
    UADDR     Image2DAddr;
	SINT32    VahbStride;
    UADDR     RefImageAddr;
    UADDR     CurrPmvAddr;
    SINT32    ImageWidth;
    SINT32    ImageHeight;
    SINT32    IsFrame;        // 0:fld, 1:frm
    SINT32    ImageCSP;       // 0:420, 1:400
    struct
    {
        SINT16    StartMbn;
        SINT16    EndMbn;
    } MbGroup[MAX_UP_MSG_SLICE_NUM];  
    SINT32     ValidGroupNum;
    SINT32     rpr_ref_pic_type;
	SINT32     Compress_en;
	SINT32     Pic_type;
	SINT32     FullRepair;
    UINT32     level;
	UINT32 	   CtbSize;            // ctbsize for hevc
	UINT32 	   tiles_enabled_flag; // for hevc
} VDMHAL_REPAIR_PARAM_S;
/* ���뱨�����ݽṹ */
typedef struct
{
    UINT32    RetType;
    UINT32    ImgStride;
    UINT32    DecSliceNum;
    UINT16    SliceMbRange[MAX_UP_MSG_SLICE_NUM+1][2];    
} VDMHAL_DEC_REPORT_S;

typedef struct
{
    UADDR  MemBaseAddr;
    SINT32 Size;
    SINT32 VdhId;
}VDMHAL_OPENPARAM_S;

typedef struct
{
	UINT32 *pMb0QpInCurrPic;
    SINT32 VdhId;
}BACKUP_INFO_S;
    
typedef struct
{
    VID_STD_E VidStd; 
    VDMHAL_HWMEM_S *pHwMem; 
    VOID *pDecParam; 
    SINT32 VdhId;
}CFG_REPAIR_REG_S;
    
typedef struct
{
    VDMHAL_REPAIR_PARAM_S *pRepairParam; 
    VDMHAL_HWMEM_S *pHwMem; 
    SINT32 VdhId;
}CFG_REPAIR_MSG_S;

typedef struct
{
    VID_STD_E VidStd;
    VDMHAL_DEC_REPORT_S *pDecReport;
    SINT32 VdhId;
}MAKE_DEC_REPORT_S;

/*#########################################################################################
       ȫ�ֱ�������
 ##########################################################################################*/
extern VDMHAL_HWMEM_S        g_HwMem[MAX_VDH_NUM];
extern UINT32                g_UpMsg[MAX_VDH_NUM][UP_MSG_SIZE];   //������Ϣ����
extern VDMHAL_REPAIR_PARAM_S g_RepairParam[MAX_VDH_NUM][2];
extern VDMHAL_DEC_REPORT_S   g_DecReport[MAX_VDH_NUM];
extern VDMHAL_BACKUP_S       g_BackUp[MAX_VDH_NUM];
extern VDMDRV_PARAM_S        g_VdmDrvParam[MAX_VDH_NUM];
extern CHAN_CTX              g_ChanCtx;
extern UINT32                USE_FF_APT_EN;

VOID   VDMDRV_Init( SINT32 VdhId );
VOID   VDMDRV_Reset( SINT32 VdhId );
VOID   VDMDRV_SetPriority(SINT32 *pPrioArray);
VOID   VDMDRV_VdmIntServProc( SINT32 VdhId );
VOID   VDMDRV_WakeUpVdm( VOID );
SINT32 VDMDRV_IsVdmIdle( SINT32 VdhId );
SINT32 VDMDRV_IsChanInDec( SINT32 ChanID );
SINT32 VDMDRV_WaitVdmReadyIfNoIsr( SINT32 VdhId );
SINT32 VDMDRV_GetCurrChanID(SINT32 VdhId);
SINT32 VDMDRV_IsVdmInISR(SINT32 VdhId);
SINT32 VDMDRV_PrepareSleep(SINT32 VdhId);
VOID   VDMDRV_ForceSleep(SINT32 VdhId);
VOID   VDMDRV_ExitSleep(SINT32 VdhId);
SINT32 VDMDRV_AvsFirstFldNeedRepair(VID_STD_E VidStd, VOID *pDecParam, SINT32 VdhId);
SINT32 VDMDRV_AvsSecondFldNeedRepair(VID_STD_E VidStd, VOID *pDecParam,SINT32 VdhId);
SINT32 VDMDRV_IsMpeg4NvopCopy(VID_STD_E VidStd, VOID *pDecParam);
SINT32 VDMDRV_IsVc1SkpicCopy(VID_STD_E VidStd, VOID *pDecParam);
SINT32 VDMDRV_IsVp6NvopCopy(VID_STD_E VidStd, VOID *pDecParam);
VOID   VDMDRV_SetChanIsDec( SINT32 ChanID, SINT32 VdhID );
VOID   VDMDRV_ClearChanIsDec( SINT32 ChanID );
SINT32 VDMDRV_IsChanDec( SINT32 ChanID );
VOID   VDMDRV_ResetVDH(SINT32 ChanID);
VOID   VDMDRV_InitConfig(VOID);
VOID   VDMDRV_PowerOn(VOID);
VOID   VDMDRV_PowerOff(VOID);
VDMDRV_SLEEP_STAGE_E VDMDRV_GetSleepStage(SINT32 VdhId);

#ifdef __cplusplus
}
#endif


#endif  //__VDM_DRV_H__
