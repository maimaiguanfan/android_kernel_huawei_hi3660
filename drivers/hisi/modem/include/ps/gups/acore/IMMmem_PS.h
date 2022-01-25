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


#ifndef __IMM_MEM_PS_H__
#define __IMM_MEM_PS_H__

/*****************************************************************************
  1 ����ͷ�ļ�����
*****************************************************************************/
#include "vos.h"
#include "product_config.h"
#if (VOS_LINUX == VOS_OS_VER)
#include <linux/skbuff.h>
#else
#include "skbuff.h"
#endif


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif




#pragma pack(4)

/*****************************************************************************
  2 �궨��
*****************************************************************************/
#define FEATURE_IMM_MEM_DEBUG           (FEATURE_TTF_MEM_DEBUG)


/*****************************************************************************
  3 ö�ٶ���
*****************************************************************************/
/*****************************************************************************
 ö����    : IMM_BOOL_ENUM
 Э����  :
 ASN.1���� :
 ö��˵��  : IMMͳһ��������ö�ٶ���
*****************************************************************************/
enum IMM_BOOL_ENUM
{
    IMM_FALSE                            = 0,
    IMM_TRUE                             = 1,
    IMM_BOOL_BUTT
};
typedef unsigned char   IMM_BOOL_ENUM_UINT8;

/*****************************************************************************
 ö����    : IMM_RSLT_CODE_ENUM
 Э����  :
 ASN.1���� :
 ö��˵��  : IMM ͳһ����ֵö�ٶ���
*****************************************************************************/
enum IMM_RSLT_CODE_ENUM
{
    IMM_SUCC                             = 0,
    IMM_FAIL                             = 1,
};
typedef unsigned int   IMM_RSLT_CODE_ENUM_UINT32;


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

enum MEM_BLK_STATE_ENUM
{
    MEM_BLK_STATE_FREE,       /*���ڴ�δ����������ͷ�*/
    MEM_BLK_STATE_ALLOC,      /*���ڴ�������*/
    MEM_BLK_STATE_BUTT
};
typedef unsigned int MEM_BLK_STATE_ENUM_UINT32;



typedef struct
{
    MEM_BLK_STATE_ENUM_UINT32           enMemStateFlag;
    unsigned int                        ulAllocTick;        /* CPU tick when alloc or free */
    unsigned short                      usAllocFileID;      /* File ID when alloc or free */
    unsigned short                      usAllocLineNum;     /* File Line when alloc or free */
    unsigned short                      usTraceFileID;      /* File ID when traced */
    unsigned short                      usTraceLineNum;     /* File Line when traced */
    unsigned int                        ulTraceTick;        /* CPU tick when traced */
} IMM_BLK_MEM_DEBUG_STRU;



enum IMM_MEM_POOL_ID_ENUM
{
    IMM_MEM_POOL_ID_SHARE = 0,              /* A�˹����ڴ��ID */
    IMM_MEM_POOL_ID_EXT,                    /* A���ⲿ�ڴ��ID */

    IMM_MEM_POOL_ID_BUTT
};
typedef unsigned char IMM_MEM_POOL_ID_ENUM_UINT8;



typedef struct
{
    IMM_MEM_POOL_ID_ENUM_UINT8          enPoolId;           /* ���ڴ�����������ĸ��ڴ�� */
    unsigned char                       ucClusterId;        /* ���ڴ���������һ������ */
    unsigned short                      usMagicNum;         /* �ڴ���ƿ�ħ���� */
#if(FEATURE_ON == FEATURE_IMM_MEM_DEBUG)
    IMM_BLK_MEM_DEBUG_STRU              stDbgInfo;         /* �ڴ���ƿ��DEBUG��Ϣ�洢�� */
#endif
    unsigned char                      *pstMemBlk;          /* ������ݵ�ָ�룬������ָ��ṹ��������ڴ� */
}IMM_MEM_STRU;


/* Ϊ�����β�ͬ�汾�Ĳ���, ��IMM_ZC_HEAD_STRU/IMM_ZC_STRU�ŵ�ͷ�ļ��� */
typedef struct sk_buff_head IMM_ZC_HEAD_STRU;

typedef struct sk_buff IMM_ZC_STRU;

typedef void (*IMM_MEM_EVENT_CALLBACK)(unsigned int ulMaxClusterFreeCnt);


/*****************************************************************************
  8 UNION����
*****************************************************************************/


/*****************************************************************************
  9 OTHERS����
*****************************************************************************/


/*****************************************************************************
  10 ��������
*****************************************************************************/

#pragma pack(0)



#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of IMM.h */

