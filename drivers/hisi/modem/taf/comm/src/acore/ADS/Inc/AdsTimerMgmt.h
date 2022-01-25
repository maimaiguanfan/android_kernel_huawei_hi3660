/*
* Copyright (C) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
* foss@huawei.com
*
* If distributed as part of the Linux kernel, the following license terms
* apply:
*
* * This program is free software; you can redistribute it and/or modify
* * it under the terms of the GNU General Public License version 2 and
* * only version 2 as published by the Free Software Foundation.
* *
* * This program is distributed in the hope that it will be useful,
* * but WITHOUT ANY WARRANTY; without even the implied warranty of
* * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* * GNU General Public License for more details.
* *
* * You should have received a copy of the GNU General Public License
* * along with this program; if not, write to the Free Software
* * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
*
* Otherwise, the following license terms apply:
*
* * Redistribution and use in source and binary forms, with or without
* * modification, are permitted provided that the following conditions
* * are met:
* * 1) Redistributions of source code must retain the above copyright
* *    notice, this list of conditions and the following disclaimer.
* * 2) Redistributions in binary form must reproduce the above copyright
* *    notice, this list of conditions and the following disclaimer in the
* *    documentation and/or other materials provided with the distribution.
* * 3) Neither the name of Huawei nor the names of its contributors may
* *    be used to endorse or promote products derived from this software
* *    without specific prior written permission.
*
* * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
*/

#ifndef __ADSTIMERMGMT_H__
#define __ADSTIMERMGMT_H__

/*****************************************************************************
  1 ����ͷ�ļ�����
*****************************************************************************/
#include "vos.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


#pragma pack(4)

/*****************************************************************************
  2 �궨��
*****************************************************************************/
/* ADS��ͬʱ���еĶ�ʱ���������Ŀ,Ŀǰֻ��һ�� */
/* Modified by l60609 for DSDA Phase II, 2012-11-22, Begin */
#define ADS_MAX_TIMER_NUM               (TI_ADS_TIMER_BUTT)
/* Modified by l60609 for DSDA Phase II, 2012-11-22, End */

/* ��ʱ��TI_ADS_UL_SEND��ʱ�� */
#define TI_ADS_UL_SEND_LEN              (10)

/* ��ʱ��TI_ADS_DSFLOW_STATS��ʱ����1��ͳ��һ������ */
#define TI_ADS_DSFLOW_STATS_LEN         (1000)

/* ��ʱ��TI_ADS_DL_PROTECT��ʱ��, 100ms */
#define TI_ADS_DL_PROTECT_LEN           (100)

/* Added by l60609 for DSDA Phase II, 2012-12-20, Begin */
/* ��ʱ��TI_ADS_DL_ADQ_EMPTY��ʱ��, 10ms */
#define TI_ADS_DL_ADQ_EMPTY_LEN         (10)
/* Added by l60609 for DSDA Phase II, 2012-12-20, Begin */

/* ��ʱ��TI_ADS_RPT_STATS��ʱ����2���ϱ�һ��ͳ����Ϣ */
#define TI_ADS_RPT_STATS_LEN         (2000)

typedef VOS_VOID (*PFN_ADS_TIMER_CALL_BACK_FUN)(VOS_UINT32 ulParam, VOS_UINT32 ulTimerName);

/*****************************************************************************
  3 ö�ٶ���
*****************************************************************************/


enum ADS_TIMER_ID_ENUM
{
    TI_ADS_UL_SEND                      = 0x00,                                 /* ADS���з��Ͷ�ʱ�� */
    TI_ADS_DSFLOW_STATS                 = 0x01,                                 /* ����ͳ�ƶ�ʱ�� */
    TI_ADS_DL_ADQ_EMPTY                 = 0x02,                                 /* ����ADQ�ն�ʱ�� */
    TI_ADS_UL_DATA_STAT                 = 0x03,                                 /* ��������ͳ�ƶ�ʱ�� */

    TI_ADS_TIMER_BUTT
};
typedef VOS_UINT32  ADS_TIMER_ID_ENUM_UINT32;



enum ADS_TIMER_STATUS_ENUM
{
    ADS_TIMER_STATUS_STOP,                                                      /* ��ʱ��ֹͣ״̬ */
    ADS_TIMER_STATUS_RUNNING,                                                   /* ��ʱ������״̬ */
    ASD_TIMER_STATUS_BUTT
};
typedef VOS_UINT8 ADS_TIMER_STATUS_ENUM_UINT8;


enum ADS_TIMER_OPERATION_TYPE_ENUM
{
    ADS_TIMER_OPERATION_START,                                                  /*������ʱ��*/
    ADS_TIMER_OPERATION_STOP,                                                   /*ֹͣ��ʱ��*/
    ADS_TIMER_OPERATION_TYPE_ENUM_BUTT
};
typedef VOS_UINT8 ADS_TIMER_OPERATION_TYPE_ENUM_UINT8;


enum ADS_TIMER_STOP_CAUSE_ENUM
{
    ADS_TIMER_STOP_CAUSE_USER,                                                   /* �û�����ֹͣ�� */
    ADS_TIMER_STOP_CAUSE_TIMEOUT,                                                /* ��ʱ����ʱ��ʾֹͣ�� */
    ADS_TIMER_STOP_CAUSE_ENUM_BUTT
};
typedef VOS_UINT8 ADS_TIMER_STOP_CAUSE_ENUM_UINT8;

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
    HTIMER                              hTimer;                                 /* ��ʱ��������ָ�� */
    VOS_UINT8                           aucRsv[8];                              /* ����*/
} ADS_TIMER_CTX_STRU;

/*****************************************************************************
 �ṹ����: ADS_TIMER_INFO_STRU
 �ṹ˵��: ADS��ʱ����Ϣ�ṹ������SDT����ʾ
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER                                          /* _H2ASN_Skip */
    ADS_TIMER_ID_ENUM_UINT32            enTimerId;          /* _H2ASN_Skip */
    VOS_UINT32                          ulTimerLen;         /* ��ʱ������*/
    ADS_TIMER_OPERATION_TYPE_ENUM_UINT8 enTimerAction;      /* ��ʱ���������� */
    ADS_TIMER_STOP_CAUSE_ENUM_UINT8     enTimerStopCause;   /* ��ʱ��ֹͣ��ԭ�� */
    VOS_UINT8                           aucReserved[2];
} ADS_TIMER_INFO_STRU;

/*****************************************************************************
 �ṹ����: ADS_TIMER_OPERATE_STRU
 �ṹ˵��: ADS��ʱ�������ṹ
*****************************************************************************/
typedef struct
{
    VOS_UINT32                              ulPid;
    VOS_UINT32                              enTimerId;                          /* ��ʱ������ */
    VOS_TIMER_PRECISION_ENUM_UINT32         enPrecision;                        /* ��ʱ������ */
    VOS_UINT8                               aucReserved[4];
    PFN_ADS_TIMER_CALL_BACK_FUN             pfnTimerStartCallBack;              /* ��ʱ��ʹ��CALLBACK�ĺ��� */
} ADS_TIMER_OPERATE_STRU;



/*****************************************************************************
  8 UNION����
*****************************************************************************/


/*****************************************************************************
  9 OTHERS����
*****************************************************************************/


/*****************************************************************************
  10 ��������
*****************************************************************************/

VOS_VOID ADS_DL_StartProtectTimer(VOS_VOID);

VOS_VOID  ADS_StartTimer(
    ADS_TIMER_ID_ENUM_UINT32            enTimerId,
    VOS_UINT32                          ulLen
);

VOS_VOID ADS_StopTimer(
    VOS_UINT32                          ulPid,
    ADS_TIMER_ID_ENUM_UINT32            enTimerId,
    ADS_TIMER_STOP_CAUSE_ENUM_UINT8     enStopCause
);

ADS_TIMER_STATUS_ENUM_UINT8 ADS_GetTimerStatus(
    VOS_UINT32                          ulPid,
    ADS_TIMER_ID_ENUM_UINT32            enTimerId
);

#if (VOS_OS_VER == VOS_WIN32)
#pragma pack()
#else
#pragma pack(0)
#endif




#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of AdsTimerMgmt.h */
