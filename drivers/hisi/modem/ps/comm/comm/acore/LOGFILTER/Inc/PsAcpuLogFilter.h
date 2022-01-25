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


#ifndef __PSACPULOGFILTER_H__
#define __PSACPULOGFILTER_H__


/******************************************************************************
  1 ����ͷ�ļ�����
******************************************************************************/
#include "vos.h"
#include "PsLogFilterInterface.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


#pragma pack(4)


/******************************************************************************
  2 �궨��
******************************************************************************/
/* �����ע����Ϣ�滻�����͹��˺������������ɸ�����Ҫ������չ */
#define PS_OM_ACPU_LAYER_MSG_FILTER_ITEM_MAX_CNT            (4)
#define PS_OM_ACPU_LAYER_MSG_MATCH_ITEM_MAX_CNT             (4)
/******************************************************************************
  3 ö�ٶ���
******************************************************************************/


/******************************************************************************
  4 ȫ�ֱ�������
******************************************************************************/


/******************************************************************************
  5 ��Ϣͷ����
******************************************************************************/


/******************************************************************************
  6 ��Ϣ����
******************************************************************************/


/******************************************************************************
  7 STRUCT����
******************************************************************************/

/*****************************************************************************
 �ṹ��    : PS_OM_ACPU_LAYER_MSG_MATCH_CTRL_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : MsgMatch���ܵĿ��ƽṹ
*****************************************************************************/
typedef struct
{
    VOS_UINT32                          ulRegCnt;
    PS_OM_LAYER_MSG_MATCH_PFUNC         apfuncMatchEntry[PS_OM_ACPU_LAYER_MSG_MATCH_ITEM_MAX_CNT];
}PS_OM_ACPU_LAYER_MSG_MATCH_CTRL_STRU;


/*****************************************************************************
 �ṹ��    : PS_OM_ACPU_LAYER_MSG_FILTER_CTRL_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : MsgFilter���ܵĿ��ƽṹ
*****************************************************************************/
typedef struct
{
    VOS_UINT32                          ulRegCnt;
    PS_OM_LAYER_MSG_FILTER_PFUNC        apfuncFilterEntry[PS_OM_ACPU_LAYER_MSG_FILTER_ITEM_MAX_CNT];
}PS_OM_ACPU_LAYER_MSG_FILTER_CTRL_STRU;

/******************************************************************************
  8 UNION����
******************************************************************************/


/******************************************************************************
  9 OTHERS����
******************************************************************************/

#pragma pack()


#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* PsAcpuLogFilter.h */


