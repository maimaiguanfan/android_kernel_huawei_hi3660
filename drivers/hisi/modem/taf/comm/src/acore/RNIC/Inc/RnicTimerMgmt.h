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

#ifndef __RNIC_TIMERMGMT_H__
#define __RNIC_TIMERMGMT_H__

/*****************************************************************************
  1 ����ͷ�ļ�����
*****************************************************************************/
#include "vos.h"
#include "product_config.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


#pragma pack(4)

/*****************************************************************************
  2 �궨��
*****************************************************************************/

/* RNIC��ͬʱ���еĶ�ʱ���������Ŀ */
#define RNIC_MAX_TIMER_NUM                      (TI_RNIC_TIMER_BUTT)

#define TI_RNIC_UNIT                            (1000)                          /* 1s = 1000ms */

#define TI_RNIC_DSFLOW_STATS_LEN                (2000)                          /* 2��ͳ��һ������ */
#define TI_RNIC_DEMAND_DIAL_DISCONNECT_LEN      (10000)                         /* 10�벦�ŶϿ���ʱ�� */
#define TI_RNIC_DEMAND_DIAL_PROTECT_LEN         (1000)                          /* 1�벦�ű�����ʱ�� */

#if (FEATURE_ON == FEATURE_MULTI_MODEM)
#if  (MULTI_MODEM_NUMBER == 3)
#define RNIC_IS_DSFLOW_TIMER_ID(enTimerId)          \
            ((TI_RNIC_DSFLOW_STATS_0 == (enTimerId))   \
          || (TI_RNIC_DSFLOW_STATS_1 == (enTimerId))   \
          || (TI_RNIC_DSFLOW_STATS_2 == (enTimerId))   \
          || (TI_RNIC_DSFLOW_STATS_3 == (enTimerId))   \
          || (TI_RNIC_DSFLOW_STATS_4 == (enTimerId))   \
          || (TI_RNIC_DSFLOW_STATS_5 == (enTimerId))   \
          || (TI_RNIC_DSFLOW_STATS_6 == (enTimerId)))
#else
#define RNIC_IS_DSFLOW_TIMER_ID(enTimerId)          \
            ((TI_RNIC_DSFLOW_STATS_0 == (enTimerId))   \
          || (TI_RNIC_DSFLOW_STATS_1 == (enTimerId))   \
          || (TI_RNIC_DSFLOW_STATS_2 == (enTimerId))   \
          || (TI_RNIC_DSFLOW_STATS_3 == (enTimerId))   \
          || (TI_RNIC_DSFLOW_STATS_4 == (enTimerId)))
#endif /* #if (MULTI_MODEM_NUMBER == 3) */
#else
#define RNIC_IS_DSFLOW_TIMER_ID(enTimerId)          \
            ((TI_RNIC_DSFLOW_STATS_0 == (enTimerId))   \
          || (TI_RNIC_DSFLOW_STATS_1 == (enTimerId))   \
          || (TI_RNIC_DSFLOW_STATS_2 == (enTimerId)))
#endif

/*****************************************************************************
  3 ö�ٶ���
*****************************************************************************/


enum RNIC_TIMER_ID_TYPE_ENUM
{
    TI_RNIC_DEMAND_DIAL_DISCONNECT,                                             /* ���貦�ŶϿ���ʱ�� */
    TI_RNIC_DEMAND_DIAL_PROTECT,                                                /* ���貦�ű�����ʱ�� */
    TI_RNIC_DSFLOW_STATS_0,                                                     /* ����0��2�������ͳ�ƶ�ʱ�� */
    TI_RNIC_DSFLOW_STATS_1,                                                     /* ����1��2�������ͳ�ƶ�ʱ�� */
    TI_RNIC_DSFLOW_STATS_2,                                                     /* ����2��2�������ͳ�ƶ�ʱ�� */
#if (FEATURE_ON == FEATURE_MULTI_MODEM)
    TI_RNIC_DSFLOW_STATS_3,                                                     /* ����3��2�������ͳ�ƶ�ʱ�� */
    TI_RNIC_DSFLOW_STATS_4,                                                     /* ����4��2�������ͳ�ƶ�ʱ�� */
#if  (MULTI_MODEM_NUMBER == 3)
    TI_RNIC_DSFLOW_STATS_5,                                                     /* ����5��2�������ͳ�ƶ�ʱ�� */
    TI_RNIC_DSFLOW_STATS_6,                                                     /* ����6��2�������ͳ�ƶ�ʱ�� */
#endif /* #if (MULTI_MODEM_NUMBER == 3) */
#endif
    TI_RNIC_TIMER_BUTT
};
typedef VOS_UINT16 RNIC_TIMER_ID_ENUM_UINT16;


enum RNIC_TIMER_STATUS_ENUM
{
    RNIC_TIMER_STATUS_STOP,                                                     /* ��ʱ��ֹͣ״̬ */
    RNIC_TIMER_STATUS_RUNING,                                                   /* ��ʱ������״̬ */
    RNIC_TIMER_STATUS_BUTT
};
typedef VOS_UINT8 RNIC_TIMER_STATUS_ENUM_UINT8;


enum RNIC_TIMER_OPERATION_TYPE_ENUM
{
    RNIC_TIMER_OPERATION_START,                                                 /* ������ʱ�� */
    RNIC_TIMER_OPERATION_STOP,                                                  /* ֹͣ��ʱ�� */
    RNIC_TIMER_OPERATION_TYPE_ENUM_BUTT
};
typedef VOS_UINT8  RNIC_TIMER_OPERATION_TYPE_ENUM_UINT8;


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
    RNIC_TIMER_STATUS_ENUM_UINT8        enTimerStatus;                          /* ��ʱ��������״̬,������ֹͣ */
    VOS_UINT8                           aucRsv[7];                              /* ���� */
} RNIC_TIMER_CTX_STRU;



typedef struct
{
    VOS_MSG_HEADER
    RNIC_TIMER_ID_ENUM_UINT16               enTimerId;                          /* ��ʱ��ID */
    RNIC_TIMER_OPERATION_TYPE_ENUM_UINT8    enTimeAction;                       /* ��ʱ���������� */
    VOS_UINT8                               aucRsv[1];                          /* ���� */
    VOS_UINT32                              ulTimeLen;                          /* ��ʱ������ */
}RNIC_TIMER_INFO_STRU;

/*****************************************************************************
  8 UNION����
*****************************************************************************/


/*****************************************************************************
  9 OTHERS����
*****************************************************************************/


/*****************************************************************************
  10 ��������
*****************************************************************************/
VOS_VOID  RNIC_InitAllTimers(
    RNIC_TIMER_CTX_STRU                 *pstRnicTimerCtx
);

VOS_VOID  RNIC_StartTimer(
    RNIC_TIMER_ID_ENUM_UINT16           enTimerId,
    VOS_UINT32                          ulLen
);

VOS_VOID  RNIC_StopTimer(
    RNIC_TIMER_ID_ENUM_UINT16           enTimerId
);

VOS_VOID  RNIC_StopAllTimer( VOS_VOID );

RNIC_TIMER_STATUS_ENUM_UINT8  RNIC_GetTimerStatus(
    RNIC_TIMER_ID_ENUM_UINT16           enTimerId
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

#endif /* end of RnicTimerMgmt.h */
