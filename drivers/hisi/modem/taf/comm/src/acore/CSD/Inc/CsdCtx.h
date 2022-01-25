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

/*****************************************************************************
  1 ͷ�ļ�����
*****************************************************************************/
#ifndef _CSD_CTX_H_
#define _CSD_CTX_H_


/*****************************************************************************
  1 ����ͷ�ļ�����
*****************************************************************************/
#include "vos.h"
#include "PsCommonDef.h"
#include "PsLogdef.h"
#include "ImmInterface.h"
#include "DiccInterface.h"
#include "AtCsdInterface.h"
#include "CsdLog.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif



#pragma pack(4)

#if( FEATURE_ON == FEATURE_CSD )


/*****************************************************************************
  2 �궨��
*****************************************************************************/

/*Ĭ��sliceֵ��ulTxSlice��ʼ��ʱʹ��*/
#define CSD_DEFAULT_TX_SLICE    (0)

/*The count number of the semaphore that create*/
#define CSD_SEMAPHORE_INIT_CNT  (0)
/*****************************************************************************
  3 ö�ٶ���
*****************************************************************************/

/*****************************************************************************
  2 ȫ�ֱ�������
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
    VOS_SEM                             hULdataSem;                             /*���������ź���*/
    VOS_SEM                             hDLdataSem;                             /*���������ź���*/
    VOS_UINT32                          ulLastDICCIsrSlice;                     /*��¼DICC�ж�SLICE*/
    AT_CSD_CALL_TYPE_STATE_ENUM_UINT16  enCallState;                            /*��¼��ǰͨ��״̬*/
    VOS_UINT8                           ucAtClientIndex;                        /* AT Client Index */
    VOS_UINT8                           aucReserved[1];
    IMM_ZC_HEAD_STRU                   *pstULQueue;                             /*���л������*/
    VOS_SPINLOCK                        stSpinLock;
}CSD_CTX_STRU;

/*****************************************************************************
  8 UNION����
*****************************************************************************/

/*****************************************************************************
  9 OTHERS����
*****************************************************************************/

/*****************************************************************************
  10 ��������
*****************************************************************************/

AT_CSD_CALL_TYPE_STATE_ENUM_UINT16 CSD_GetCallState(VOS_VOID);
VOS_SEM CSD_GetDownLinkDataSem(VOS_VOID);
VOS_UINT32 CSD_GetLastTxSlice(VOS_VOID);
VOS_SEM CSD_GetUpLinkDataSem(VOS_VOID);
VOS_UINT32 CSD_InitCtx(VOS_VOID);
VOS_UINT32 CSD_InitSem(VOS_VOID);
VOS_VOID CSD_SetCallState(AT_CSD_CALL_TYPE_STATE_ENUM_UINT16 enCallState);
VOS_VOID CSD_SetCurrTxSlice(VOS_UINT32 ulCurrSlice);
VOS_UINT32 CSD_UL_FreeQueue(IMM_ZC_HEAD_STRU *pstQueue);
IMM_ZC_HEAD_STRU *CSD_UL_GetQueue(VOS_VOID);
VOS_VOID CSD_UL_SetQueue(IMM_ZC_HEAD_STRU *pstULQueue);
VOS_UINT32 CSD_UL_InsertQueueTail(
    IMM_ZC_HEAD_STRU                    *pstQueueHead,
    IMM_ZC_STRU                         *pData
);
IMM_ZC_STRU  *CSD_UL_GetQueueFrontNode(IMM_ZC_HEAD_STRU *pstQueue);

VOS_UINT8 CSD_UL_GetAtClientIndex(VOS_VOID);
VOS_VOID CSD_UL_SetAtClientIndex(VOS_UINT8 ucIndex);

#endif /*FEATURE_ON == FEATURE_CSD*/

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

#endif

