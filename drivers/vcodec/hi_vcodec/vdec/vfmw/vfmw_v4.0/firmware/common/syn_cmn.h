/*--------------------------------------------------------------------------------------------------------------------------*/
/*!!Warning: This is a key information asset of Huawei Tech Co.,Ltd                                                         */
/*CODEMARK:kOyQZYzjDpyGdBAEC2GaWinjiDDUykL9e8pckESWBbMVmSWkBuyJO01cTiy3TdzKxGk0oBQa
mSMf7J4FkTpfv/JzwTgKEHe/9N4yU5rqhEDGFnkG4/S4FxOOB/C/wNlmJJL1crdwYm/aqTyh
1nLqNAW+9yG4/77/JxFk/VGVJGjlQUgnE3bgUeAWPAyzTmTw3cXhZo0edE8dli7BK93chb75
hM8q2dhunY8jp/u6QpgzJvwo7rSN1KKZTEW7P0aDZsZKvGq0bsnXO4Q55MsTZw==*/
/*--------------------------------------------------------------------------------------------------------------------------*/

#ifndef __SYNTAX_COMMON_HEADER__
#define __SYNTAX_COMMON_HEADER__

#include "basedef.h"
#include "vfmw.h"
#include "scd_drv.h"


/* ����ֵ�����������޸ģ������Ϊ��2�������ݣ�����С��64�������ܳ����� */
#define MAX_QUEUE_IMAGE_NUM    (64)   

/* �����������ѡ��, ΪDecOrderOutput����ȡֵ */
#define NOT_DEC_ORDER          (0)
#define DEC_ORDER_NORMAL_DPB   (1)
#define DEC_ORDER_SIMPLE_DPB   (2)
#define DEC_ORDER_SKYPLAY      (3)

#define IMG_ID_MAGIC_STEP      (100)

/* MPEG2, MPEG4����ƴ����������ݽṹ */
typedef struct
{
     STREAM_SEG_S    StreamPack[2];  // for vfmw decode
}DEC_STREAM_PACKET_S;

typedef struct
{
     UINT8           FoundPicFlag;
	 UINT8           IsCurrSegWaitDec;
     UINT8           ConvolutionPacketNum;
     STREAM_SEG_S    CurrSeg;
     DEC_STREAM_PACKET_S    Pack;  // for vfmw decode
     DEC_STREAM_PACKET_S    LastPack;
     UINT8                  u32SCDWrapAround;
}STREAM_SEG_PACKET_S;

typedef struct hiIMAGE_SIDE_INFO_S
{
    UINT32     OldAspectRatio;
    UINT32     OldScanType;
    UINT32     OldSeqCnt;
    UINT32     OldFrameRate;
    UINT32     OldDispSize;
    UINT32     OldDispCenter;
    UINT32     LastImageIdPlus2;
    UINT32     ImageWidth;
    UINT32     ImageHeight;
    UINT32     VideoFormat;
    UINT32     ImageFormat;
    UINT32     SeqImgCnt;
    UINT32     SeqCnt;

    /* time stamp calculation */
	UINT64     PrevTR;	        /* recorded RV pre frame time reference value */
	UINT64     PrevPts;
	UINT64     AveragePtsStep;  /* smoothed by a FIR filter */
} IMAGE_SIDE_INFO_S;

/* video output queue(VO queue)  */
typedef struct
{
    UINT16     max_queue_size;
    UINT16     queue_size;
    UINT16     history;
    UINT16     head;
    UINT16     tail;
    IMAGE      *image_ptr[MAX_QUEUE_IMAGE_NUM];
    IMAGE_SIDE_INFO_S  SideInfo;
    UINT32     ResetMagicWord;
} IMAGE_VO_QUEUE;

typedef struct hiSYNTAX_EXTRA_DATA_S
{
    SINT8      s8SpecMode;                         /* 0: normal mode, 1: CRC check mode */
    SINT32     s32RefErrThr;
    SINT32     s32OutErrThr;
    SINT32     s32DecMode;                         /* 0,1,2 = IPB,IP,I */
    SINT32     s32DecOrderOutput;                  /* 0:display order, 1:decode order */
    UADDR      s32SyntaxMemAddr;
    SINT32     s32SyntaxMemSize;
    VDEC_CHAN_CAP_LEVEL_E eCapLevel;
    UADDR      s32BpdMemAddr;
    SINT32     s32BpdMemSize;  
    UADDR      s32Vp8SegIdMemAddr;
    SINT32     s32Vp8SegIdMemSize;
    UINT64     pts;
    UINT64     NextPts;
    UINT64     Usertag;
    UINT64     NextUsertag;
    UINT64     DispTime;
    UINT64     NextDispTime;	
    UINT32     DispEnableFlag;       
    UINT32     DispFrameDistance;   
    UINT32     DistanceBeforeFirstFrame;   
    UINT32     NextDispEnableFlag;       
    UINT32     NextDispFrameDistance;   
    UINT32     NextDistanceBeforeFirstFrame;   
    UINT32     GopNum;       
    UINT32     NextGopNum;  
    SINT32     s32NewPicSegDetector;   

    STREAM_SEG_PACKET_S stSegPacket;   
    STD_EXTENSION_U     StdExt;    	
    VDEC_CHAN_STATE_S   stChanState;
    SINT32     s32BasicCtxSize;
    UADDR      s32ContextMemAddr;
    SINT32     s32ContextMemSize;	
	SINT32     s32Compress_en;
    SINT32     s32MoreGapEnable;
    SINT32     s32FastForwardBackwardSpeed;
    SINT32     s32KeepSPSPPS;                      /* keep global info in ctx for seek reset */
    SINT32     s32SupportAllP;                     /* support stream of all p frames */
	SINT32     s32ModuleLowlyEnable;               /* ģ������ʱʹ�ܱ�־ */
    VFMW_CONTROLINFO_S  stPvrInfo;
    VDEC_CHAN_OPTION_S  stChanOption;              /* ͨ������ΪCAP_LEVEL_USER_DEFINE_WITH_OPTIONʱ���û�ָ��ѡ������ */	
    VDEC_DISPIC_PARAM_S stDisParams;               /* ��֡���� */
    SINT32 s32WaitFsFlag;                          /* �ȴ�����֡���־ */
} SYNTAX_EXTRA_DATA_S;


VOID   ReleasePacket(VID_STD_E VidStd, VOID *pCtx);
SINT32 OpenUsdPool(SINT32 ChanID);
VOID   CloseUsdPool(SINT32 ChanID);
SINT32 InitUsdPool(SINT32 ChanID);
VOID   FreeUsdByDec(SINT32 ChanID, VDEC_USRDAT_S *pUsrdat);
VOID   MarkUsdByDnr(SINT32 ChanID, VDEC_USRDAT_S *pUsrdat);
VOID   UnMarkUsdByDnr(SINT32 ChanID, VDEC_USRDAT_S *pUsrdat);
VDEC_USRDAT_S *GetUsd(SINT32 ChanID);

#endif


