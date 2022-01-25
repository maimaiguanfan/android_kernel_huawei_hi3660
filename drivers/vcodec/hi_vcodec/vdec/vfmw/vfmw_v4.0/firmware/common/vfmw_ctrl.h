/*--------------------------------------------------------------------------------------------------------------------------*/
/*!!Warning: This is a key information asset of Huawei Tech Co.,Ltd                                                         */
/*CODEMARK:kOyQZYzjDpyGdBAEC2GaWinjiDDUykL9e8pckESWBbMVmSWkBuyJO01cTiy3TdzKxGk0oBQa
mSMf7J4FkTpfv/JzwTgKEHe/9N4yU5rqhEDGFnkG4/S4FxOOB/C/wNlmJJL1crdwYm/aqTyh
1nLqNLiWfZfPAW12dbT6stZWDDWKJoRnOvKlNbnridJHqzGLV/QsEBhxhLC9xnosVNT11er0
hCBdIEHWj9MnyfuwUv4/BlsfWNPrq6awrE3Dv4OYZbczDrCAZ1CF1Jg+QbD6FA==*/
/*--------------------------------------------------------------------------------------------------------------------------*/
#ifndef __VFMV_CTRL_H__
#define __VFMV_CTRL_H__
#include "vfmw.h"
#include "fsp.h"
#include "sysconfig.h"
#include "syntax.h"

/*======================================================================*/
/*  ����                                                                */
/*======================================================================*/
#define VCTRL_OK                0
#define VCTRL_ERR              -1
#define VCTRL_ERR_VDM_BUSY     -2
#define VCTRL_ERR_NO_FSTORE    -3
#define VCTRL_ERR_NO_STREAM    -4
#define MAX_FRAME_SIZE 			2048*2048

#define VCTRL_FIND_PTS          0
#define VCTRL_PTS_ILLEAGLE     -1
#define VCTRL_SEEKING_PTS      -2

#define VCTRL_USER_OPTION     0
#define VCTRL_ADD_EXTRA        1
#define VCTRL_IGNOR_EXTRA     2

#define MAX_USRDEC_FRAME_NUM        (16)
#define MAX_VID_PROTOCOL_NAME       (20)
#define MAX_DISPLAY_FRAME_NUM       (32)
#define MAX_RAW_PACKET_NUM          (6)
#define MAX_RAW_PACKET_SIZE         (6*1024*1024)
/*======================================================================*/
/*  �ṹ��ö��                                                          */
/*======================================================================*/
/* ֡�漯�ϣ����ڵ���: ��ѯ�ж���֡�棬�����ݷֱ�Ϊ�� */
typedef struct hiVFMWFrameARRAY
{
    SINT32       s32FrameNum;
    SINT32       s32PixWidth;
    SINT32       s32PixHeight;
    SINT32       s32Stride;
    SINT32       s32ChromOfst;
    UADDR        s32PhyAddr[64];
} VFMW_FRAME_ARRAY_S;

/* ͨ����ص�ͳ����Ϣ */
typedef struct hiVFMW_CHAN_STAT
{
    UINT32        u32FrBeginTime;   /* ����֡�ʵĿ�ʼʱ�� */
	UINT32        u32FrImgNum;      /* ֡�� */
	UINT32        u32FrFrameRate;   /* ֡�� */

	/* ��֡ͳ�� */
	UINT32        u32SkipFind;        /* �⵽��skip֡��Ŀ */
	UINT32        u32SkipDiscard;     /* ���������skip֡��Ŀ */
    UINT32        u32IsFieldFlag;     /* ��ǰ��������е�֡�ǳ�ͼ��־������PVR������˻�ȡ����֡������ */
	
    VFMW_FRAME_ARRAY_S stFrameArray;
} VFMW_CHAN_STAT_S;

typedef struct hiVFMW_GLOBAL_STAT
{
    /* VDMռ����ͳ�� */
    UINT32        u32VaBeginTime;     /* ͳ��VDMռ���ʵ���ʼʱ�� */
    UINT32        u32VaLastStartTime; /* ��һ������Ӳ����ʱ�� */
	UINT32        u32VaVdmWorkTime;   /* VDM�ۼƹ���ʱ�� */
	UINT32        u32VaVdmLoad;       /* VDMռ���� */
    
    /* VDM��������ͳ�� */
    UINT32        u32PicNum;
    UINT32        u32AccVdmKiloCycle;
    UINT32        u32VaVdmKiloCycle;
    UINT32        u32StatTime;

	UINT32 	      u32IsVdhEn;
} VFMW_GLOBAL_STAT_S;

/* ArrangeMem�������ô��ݵ�ָʾ������ */
typedef enum
{
    CAP_HEVC_SPECIFIC     = -2,
    INVALID_CHAN_ID       = -1,
    VALID_CHAN_ID_START   = 0,
} ARRANGE_FLAG_E;

typedef enum
{
    VDM_SUPPORT_VP6       = 1,   /* 1<<0 */
    VDM_SUPPORT_BPD       = 2,   /* 1<<1 */
    VDM_SUPPORT_VCMP      = 4,   /* 1<<2 */
    VDM_SUPPORT_GMC       = 8,   /* 1<<3 */
} VDM_CHARACTER_E;

/* �̵߳�״̬ */
typedef enum hiTASKSTATE_E
{
    TASK_STATE_EXIT = 0,        /* �˳�, �߳�δ������������ */
    TASK_STATE_STOP,            /* ֹͣ���߳��Ѵ��������ڿ�ת������ֹͣ */
    TASK_STATE_RUNNING,         /* ���� */
    TASK_STATE_BUTT
} TASK_STATE_E;

/* ���̷߳�����ָ�� */
typedef enum hiTASKCMD_E
{
    TASK_CMD_NONE = 0,        /* ��ָ�� */
    TASK_CMD_START,           /* ����ָ���������ֹͣ״̬���߳� */
    TASK_CMD_STOP,            /* ָֹͣ�ֹͣ��������״̬���߳� */
    TASK_CMD_KILL,            /* ����ָ�ʹ�������л�ֹͣ״̬���߳���Ȼ�˳� */
    TASK_CMD_BUTT
} TASK_CMD_E;

/* ���һ֡״̬���� */
typedef enum hiLastFrameState
{
    LAST_FRAME_INIT = 0,
    LAST_FRAME_RECEIVE,
    LAST_FRAME_REPORT_SUCCESS,
    LAST_FRAME_REPORT_FAILURE,
    LAST_FRAME_REPORT_FRAMEID,
	LAST_FRAME_BUTT
} LAST_FRAME_STATE_E;

/* ͨ������״̬ */
typedef enum
{
    CHAN_NORMAL,             //ͨ������Run
    CHAN_FRAME_STORE_ALLOC,  //ͨ�����ڷ���֡��
    CHAN_DESTORY,            //ͨ����������         
} VFMW_CHANNEL_CONTROL_STATE_E;

typedef enum
{
    FRAME_NODE_STATE_PRE = 0,
    FRAME_NODE_STATE_MMZ,
    FRAME_NODE_STATE_PRE_LOCK,
    FRAME_NODE_STATE_MMZ_LOCK,
    FRAME_NODE_STATE_FREE = 0xFF,
} CHAN_FRAME_NODE_STATE_E;

typedef struct hiDRV_MEM_S
{
    MEM_RECORD_S  stVdmHalMem[MAX_VDH_NUM]; /* VDM �����ڴ�  */
    MEM_RECORD_S  stScdCmnMem;              /* SCD �����ڴ�  */
    MEM_RECORD_S  stVdhReg;                 /* �������Ĵ���  */
    MEM_RECORD_S  stCrgReg;                 /* CRG�Ĵ���  */
    MEM_RECORD_S  stExtHalMem;              /* �ⲿ��HAL�ڴ� */
} DRV_MEM_S;

/* �������������ݼ� */
typedef struct hiVFMW_CTRL_DATA_S
{
    SINT32        s32IsVCTRLOpen;        /* ��־VCTRL�Ƿ�ȫ�ִ� */
    SINT32        s32ThisChanIDPlus1;    /* ��ǰ���ڽ����﷨�����ͨ���� */

    TASK_STATE_E  eTaskState;            /* �߳�״̬ */
    TASK_CMD_E    eTaskCommand;          /* �߳�ָ�� */
    OSAL_TASK     hThread;               /* �߳̾�� */
    DRV_MEM_S     stDrvMem;
    
    EXT_FN_EVENT_CALLBACK  pfnEventReport_Vdec;
    EXT_FN_EVENT_CALLBACK  pfnEventReport_OmxVdec;
    EXT_FN_POWER_ON        pfnPowerON;
    EXT_FN_POWER_OFF       pfnPowerOFF;
    EXT_FN_GET_CLK_RATE    pfnGetClkRate;
    EXT_FN_SET_CLK_RATE    pfnSetClkRate;
} VFMW_CTRL_DATA_S;

typedef struct hiVFMW_CHAN_FRM_BUF_S
{
    MEM_RECORD_S stFrameBuffer;
    CHAN_FRAME_NODE_STATE_E eBufferNodeState; //0:Ԥ����õ��Ľڵ�, 1:��̬����õ��Ľڵ�, 2:Ԥ�����ұ����ģ��ռ�õĽڵ㣬3:��̬�����ұ����ģ��ռ�õĽڵ�, 0xFF:���нڵ�
} VFMW_CHAN_FRM_BUF_S;

/* ����ͨ���ڴ��¼ */
typedef struct hiVFMW_CHAN_MEM_S
{
    UINT32         u32DynamicAllocEn;        /* �Ƿ�Ϊ��̬֡�淽ʽ */
    SINT32         s32SelfAllocChanMem_vdh;  /* ��ʶͨ�� vdh �Ƿ����Լ�����ģ�1: �Լ�����, 0: �ⲿ���� */
    SINT32         s32SelfAllocChanMem_scd;  /* ��ʶͨ�� scd �Ƿ����Լ�����ģ�1: �Լ�����, 0: �ⲿ���� */
    SINT32         s32SelfAllocChanMem_ctx;  /* ��ʶͨ�� ctx �Ƿ����Լ�����ģ�1: �Լ�����, 0: �ⲿ���� */
    MEM_RECORD_S   stChanMem;                /* ��ͨ���Ĵ洢��Դ */
    MEM_RECORD_S   stChanMem_vdh;            /* ��ͨ����֡��洢��Դ */
    MEM_RECORD_S   stChanMem_scd;            /* ��ͨ����SCD�洢��Դ */
    MEM_RECORD_S   stChanMem_ctx;            /* ��ͨ���������Ĵ洢��Դ */ 
    MEM_RECORD_S   stChanMem_dsp;            /* ��ͨ����dsp�����Ĵ洢��Դ */
    VFMW_CHAN_FRM_BUF_S* pstChanFrmBuf;
    VFMW_CHAN_FRM_BUF_S* pstChanPmvBuf;
} VFMW_CHAN_MEM_S;

typedef struct hiVFMW_MEM_ARRANGE_INFO_S
{
    VDMHAL_MEM_ARRANGE_S stMemArrange;
    SINT32 ImgSlotLen;
    SINT32 PmvSlotLen;
} VFMW_MEM_ARRANGE_INFO_S;

typedef struct hiVFMW_DYNAMIC_FS_TIMESTAMP_S
{
    UINT32 u32StartTime;
    UINT32 u32AllTime;
} VFMW_DYNAMIC_FS_TIMESTAMP_S;

typedef struct hiVFMW_RECENT_IMAGE_INFO
{
    UINT32                        DispWidth;
    UINT32                        DispHeight;
    UINT32                        ImgFormat;
    FRAME_PACKING_TYPE_E          ImgPackingType;
}VFMW_RECENT_IMAGE_INFO_S;

/* ����ͨ�� */
typedef struct hiVFMW_CHAN_S
{
    SINT32                        s32ChanID;                /* ͨ��ID */
    SINT32                        s32IsOpen;                /* 0: δ�򿪣�1: �� */
    SINT32                        s32IsRun;                 /* 0: ��������, 1: ���У��ɱ�����  */
    SINT32                        s32Priority;              /* ���ȼ���0��������ȼ�(�Ӳ�����) ~ 255(������ȼ��������ȵ���) */
    SINT32                        s32StopSyntax;            /* ͣ��syntax���룬������stopͨ����ʱ����ͣ���Σ��������������У���������
                                                               ��DecParam���ĸɾ����Ӷ�ʵ�ְ�ȫ��ͨ��stop��reset���� */
    VID_STD_E                     eVidStd;                  /* ͨ����Ӧ��Э������ */
    VDEC_ADAPTER_TYPE_E           eAdapterType;             /* ָʾ��ͨ������vdec/omxvdec���� */
    VDEC_CHAN_CAP_LEVEL_E         eChanCapLevel;            /* ͨ������������ */
    UINT32                        u32timeLastDecParamReady;
	
    VDEC_CHAN_CFG_S               stChanCfg;                /* �����û����ã����ڲ�ѯ�ӿ�ʵ�� */
    STREAM_INTF_S                 stStreamIntf;             /* �����ӿ� */

    SINT32                        s32SCDInstID;             /* ��ͨ����Ӧ��SCDʵ��ID */
    SINT32                        s32VDMInstID;             /* ��ͨ����Ӧ��VDMʵ��ID */

    SINT32                        s32OneChanMem;            /*��ͨ���ڴ����ⲿ����Ϊһ�飬ӳ��Ȳ���ʱ��Ҫ��Ϊ����������*/
    MEM_RECORD_S                  stChanMem;                /* ��ͨ���Ĵ洢��Դ */

    MEM_RECORD_S                  stChanMem_vdh;            /* ��ͨ����֡��洢��Դ */
    MEM_RECORD_S                  stChanMem_scd;            /* ��ͨ����SCD�洢��Դ */
    MEM_RECORD_S                  stChanMem_ctx;            /* ��ͨ���������Ĵ洢��Դ */ 
    
    SINT32                        s32SelfAllocChanMem_vdh;  /* ��ʶͨ�� vdh �Ƿ����Լ�����ģ�1: �Լ�����, 0: �ⲿ���� */
    SINT32                        s32SelfAllocChanMem_scd;  /* ��ʶͨ�� scd �Ƿ����Լ�����ģ�1: �Լ�����, 0: �ⲿ���� */
    SINT32                        s32SelfAllocChanMem_ctx;  /* ��ʶͨ�� ctx �Ƿ����Լ�����ģ�1: �Լ�����, 0: �ⲿ���� */
    UADDR                         s32VdmChanMemAddr;
    SINT32                        s32VdmChanMemSize;        /* VDMͨ����ռ�ݵĴ洢�ռ��С */
    UADDR                         s32ScdChanMemAddr;
    SINT32                        s32ScdChanMemSize;        /* SCDͨ����ռ�ݵĴ洢�ռ��С */
    UADDR                         s32BpdChanMemAddr;
    SINT32                        s32BpdChanMemSize;        /* BPDͨ����ռ�ݵĴ洢�ռ��С */
    UADDR                         s32Vp8SegIdChanMemAddr;
    SINT32                        s32Vp8SegIdChanMemSize;   /* SegIdͨ����ռ�ݵĴ洢�ռ��С */	
    SINT32                        s32NoStreamFlag;          /* ���ڸ�ͨ��û���㹻��������δ�����ɽ������decparam */
    VFMW_RECENT_IMAGE_INFO_S      stRecentImgInfo;
    LAST_FRAME_STATE_E            eLastFrameState;          /* �������һ֡��ǵ�״̬ */
	
    SYNTAX_EXTRA_DATA_S           stSynExtraData;
    SM_INSTANCE_S                 SmInstArray;
    FSP_INST_S                    FspInst;

    /* ��̬֡����� */
    UINT32                        u32CurFrameWidth; 
    UINT32                        u32CurFrameHeight; 
    SINT32                        s32RefFrameNum; 
    SINT32                        s32RefFrameSize;
    SINT32                        s32RefPmvSize; 
    SINT32                        s32ActualFrameNum;
    VFMW_CHAN_FRM_BUF_S           stFrmBuf[MAX_FRAME_NUM];
	VFMW_CHAN_FRM_BUF_S           stPmvBuf[MAX_FRAME_NUM];
    VFMW_MEM_ARRANGE_INFO_S       stMemArrangeInfo;
    VFMW_DYNAMIC_FS_TIMESTAMP_S   stDynamicFSTimestamp;
    
    
    SYNTAX_CTX_S                  stSynCtx;

} VFMW_CHAN_S;

/* �̵߳�״̬ */
typedef enum hiDSPSTATE_E
{
    DSP_STATE_NORMAL = 0,      /* DSP ��û�м����κδ���  */
    DSP_STATE_SCDLOWDLY,       /* DSP �Ѿ�������SCD���ӵĴ��룬˵������һ��ͨ�����ڵ��ӳ�ģʽ */
    DSP_STATE_AVS,             /* DSP �Ѿ�������AVS+�Ĵ��룬˵������һ��ͨ��������AVSЭ�� */
    DSP_STATE_BUTT
} DSP_STATE_E;

/*======================================================================*/
/*  ȫ�ֱ���                                                            */
/*======================================================================*/
extern SINT32             g_VdmCharacter;
extern VFMW_CHAN_STAT_S   g_VfmwChanStat[MAX_CHAN_NUM];
extern VFMW_GLOBAL_STAT_S g_VfmwGlobalStat[MAX_VDH_NUM];
extern VDM_VERSION_E      g_eVdmVersion;

/*======================================================================*/
/*  ��������                                                            */
/*======================================================================*/
SINT32 VCTRL_SetCallBack(VDEC_ADAPTER_TYPE_E type, INIT_INTF_S *pIntf);
VOID   VCTRL_PowerOn(VOID);
VOID   VCTRL_PowerOff(VOID);
SINT32 VCTRL_GetClkRate(CLK_RATE_E* pClkRate);
SINT32 VCTRL_SetClkRate(CLK_RATE_E eClkRate);
SINT32 VCTRL_StopTask(VOID);
SINT32 VCTRL_StartTask(VOID);
SINT32 VCTRL_OpenDrivers(DRV_MEM_S *pstDrvMem);
SINT32 VCTRL_CloseDrivers(DRV_MEM_S *pstDrvMem);
SINT32 VCTRL_OpenVfmw(VDEC_OPERATION_S *pArgs);
SINT32 VCTRL_CloseVfmw(VOID);
VOID   VCTRL_Suspend(VOID);
VOID   VCTRL_Resume(VOID);
SINT32 VCTRL_CreateChan(VDEC_CHAN_CAP_LEVEL_E eCapLevel, MEM_DESC_S *pChanMem);
SINT32 VCTRL_CreateChanWithOption(VDEC_CHAN_CAP_LEVEL_E eCapLevel, VDEC_CHAN_OPTION_S *pChanOption, SINT32 flag, SINT32 OneChanMemFlag);
SINT32 VCTRL_DestroyChan(SINT32 ChanID);
SINT32 VCTRL_DestroyChanWithOption(SINT32 ChanID);
SINT32 VCTRL_StartChan(SINT32 ChanID);
SINT32 VCTRL_StopChan(SINT32 ChanID);
SINT32 VCTRL_StopChanWithCheck(SINT32 ChanID);
SINT32 VCTRL_GetChanCfg(SINT32 ChanID, VDEC_CHAN_CFG_S *pstCfg);
SINT32 VCTRL_CmpConfigParameter(SINT32 ChanID, VDEC_CHAN_CFG_S *pstCfg);
VOID   VCTRL_GetVdecCapability(VDEC_CAP_S *pCap);
SINT32 VCTRL_ConfigChan(SINT32 ChanID, VDEC_CHAN_CFG_S *pstCfg);
SINT32 VCTRL_ResetChanWithOption(SINT32 ChanID, VDEC_CHAN_RESET_OPTION_S *pOption);
SINT32 VCTRL_ReleaseStream(SINT32 ChanID);
SINT32 VCTRL_ResetChan(SINT32 ChanID);
SINT32 VCTRL_GetChanMemSize(VDEC_CHAN_CAP_LEVEL_E eCapLevel, SINT32 *VdmMemSize, SINT32 *ScdMemSize);
VOID   VCTRL_GetChanState(SINT32 ChanID, VDEC_CHAN_STATE_S *pstChanState);
SINT32 VCTRL_SetStreamInterface( SINT32 ChanID, VOID *pIntf );
SINT32 VCTRL_GetChanMemSizeWithOption(VDEC_CHAN_CAP_LEVEL_E eCapLevel, VDEC_CHAN_OPTION_S *pChanOption, DETAIL_MEM_SIZE *pDetailMemSize,SINT32 flag);
VOID   VCTRL_GetChanCtxSize(VDEC_CHAN_CAP_LEVEL_E eCapLevel, SINT32* s32ChanCtxSize);
SINT32 VCTRL_GetStreamSize(SINT32 ChanID, SINT32 *pArgs);
VOID*  VCTRL_GetDecParam(SINT32 ChanId);
VOID   VCTRL_VdmPostProc( SINT32 ChanId, SINT32 ErrRatio, UINT32 Mb0QpInCurrPic, LUMA_INFO_S *pLumaInfo,SINT32 VdhId);
SINT32 VCTRL_GetChanImage( SINT32 ChanID, IMAGE *pImage );
SINT32 VCTRL_ReleaseChanImage( SINT32 ChanID, IMAGE *pImage );
SINT32 VCTRL_RunProcess(VOID);
VOID   VCTRL_InformVdmFree(SINT32 ChanID);
SINT32 VCTRL_GetImageBuffer( SINT32 ChanId );
SINT32 VCTRL_GetChanIDByCtx(VOID *pCtx);
SINT32 VCTRL_IsChanDecable( SINT32 ChanID );
SINT32 VCTRL_IsChanSegEnough( SINT32 ChanID );
SINT32 VCTRL_IsChanActive( SINT32 ChanID );
VOID   VCTRL_GetChanImgNum( SINT32 ChanID, SINT32 *pRefImgNum, SINT32 *pReadImgNum, SINT32 *pNewImgNum );
SINT32 VCTRL_SetMoreGapEnable(SINT32 ChanID, SINT32 MoreGapEnable);
SINT32 VCTRL_SetDbgOption ( UINT32 opt, UINT8* p_args );
SINT32 VCTRL_GetChanResolution(SINT32 ChanId, SINT32 *pChanW, SINT32 *pChanH);
SINT32 VCTRL_GetImageResolution(SINT32 ChanID, SINT32 *pDispW, SINT32 *pDispH);
VOID   VCTRL_SetLastFrameState(SINT32 ChanID, LAST_FRAME_STATE_E eState);       
SINT32 VCTRL_OutputLastFrame(SINT32 ChanId);  
SINT32 VCTRT_SetChanCtrlState(SINT32 ChanID, VFMW_CHANNEL_CONTROL_STATE_E ChanCtrlState);
SINT32 VCTRL_SetFsParamToChan(SINT32 ChanID, VDEC_CHAN_FRAME_STORES *stParams);
VOID   VCTRL_SetChanFsPartitionState(SINT32 ChanID, FSP_PARTITION_STATE_E state);
VOID   VCTRL_ReleaseDynamicNode(VFMW_CHAN_FRM_BUF_S *pstFrmBuf, VFMW_CHAN_FRM_BUF_S *pstPmvBuf);
VOID   VCTRL_OpenHardware(SINT32 ChanID);
VOID   VCTRL_CloseHardware(VOID);
VOID   VCTRL_STD_Enum2Str(VID_STD_E eVidStd, char *strVidStd);
VOID   VCTRL_EofState_Enum2Str(LAST_FRAME_STATE_E eState, char *strState);

VID_STD_E              VCTRL_GetVidStd(SINT32 ChanId);
STREAM_INTF_S*         VCTRL_GetStreamInterface(SINT32 ChanID);
VDEC_CHAN_CAP_LEVEL_E  VCTRL_GetChanCapLevel(SINT32 ChanID);
IMAGE_VO_QUEUE*        VCTRL_GetChanVoQue(SINT32 ChanID);

#if defined(VFMW_SCD_LOWDLY_SUPPORT) || defined(VFMW_AVSPLUS_SUPPORT)
SINT32 VCTRL_LoadDspCode(SINT32 ChanID);
#endif

#ifdef ENV_ARMLINUX_KERNEL
VOID   VCTRL_WriteYuv_Linear( OSAL_FILE *fpYuv, UADDR YPhyAddr, UADDR CPhyAddr, UINT32 Width, UINT32 Height,  UINT32 Stride, UINT32 chroma_idc);
VOID   VCTRL_WriteYuv_Tile( OSAL_FILE *fpYuv, UADDR YPhyAddr, UADDR CPhyAddr, UINT32 Width, UINT32 Height, UINT32 PicStructure, UINT32 Stride, UINT32 chroma_idc, UINT32 LeftOffset,UINT32 RightOffset,UINT32 TopOffset,UINT32 BottomOffset);
SINT32 VCTRL_CheckCSDCondition(SINT32 ChanId);
#endif

#endif

