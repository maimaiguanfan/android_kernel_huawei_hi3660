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

#ifndef _CBTSCM_SOFTDECODE_H_
#define _CBTSCM_SOFTDECODE_H_

/*****************************************************************************
  1 ����ͷ�ļ�����
*****************************************************************************/
#include "vos.h"
#include "omringbuffer.h"
#include "msp_debug.h"


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif

/**************************************************************************
  2 �궨��
**************************************************************************/
#define CBTSCM_DATA_RCV_BUFFER_SIZE    (100*1024)      /* SCM���ݽ���buffer��С */
#define CBTSCM_DATA_RCV_PKT_SIZE       (8*1024)        /* SCM���ݽ���PKT��С */


/**************************************************************************
  3 ö�ٶ���
**************************************************************************/

/**************************************************************************
  4 �ṹ�嶨��
**************************************************************************/

/*****************************************************************************
�ṹ��    : CBTSCM_DATE_RCV_CTRL_STRU
�ṹ˵��  : CBTSCM���ݽ��յĿ��ƽṹ
*****************************************************************************/
typedef struct
{
    VOS_SEM                     SmID;
    OM_RING_ID                  rngOmRbufId;
    VOS_CHAR                   *pucBuffer;
}CBTSCM_DATA_RCV_CTRL_STRU;

/*****************************************************************************
�ṹ��    : CBTSCM_MAINTENANCE_INFO_STRU
�ṹ˵��  : CBTSCM�����ģ���ά�ɲ���Ϣ��¼�ṹ
*****************************************************************************/
typedef struct
{
    VOS_UINT32                          ulDataLen; /* ���ջ������ݳ��� */
    VOS_UINT32                          ulNum;     /* ���ջ������ݴ��� */
}CBTSCM_MAINTENANCE_INFO_STRU;

typedef struct
{
    VOS_UINT32                          ulSemCreatErr;
    VOS_UINT32                          ulSemGiveErr;
    VOS_UINT32                          ulRingBufferCreatErr;
    VOS_UINT32                          ulTaskIdErr;
    VOS_UINT32                          ulBufferNotEnough;
    VOS_UINT32                          ulRingBufferFlush;
    VOS_UINT32                          ulRingBufferPutErr;
}CBTSCM_SOFTDECODE_RB_INFO_STRU;

/*****************************************************************************
�ṹ��    : OM_ACPU_PC_UE_SUC_STRU
�ṹ˵��  : OMģ���������տ�ά�ɲ���Ϣ��¼�ṹ
*****************************************************************************/
typedef struct
{
    CBTSCM_SOFTDECODE_RB_INFO_STRU         stRbInfo;
    CBTSCM_MAINTENANCE_INFO_STRU           stPutInfo;
    CBTSCM_MAINTENANCE_INFO_STRU           stGetInfo;
    CBTSCM_MAINTENANCE_INFO_STRU           stHdlcDecapData;
    VOS_UINT32                             ulFrameDecapErr;
    VOS_UINT32                             ulHdlcInitErr;
    VOS_UINT32                             ulDataTypeErr;
    VOS_UINT32                             ulCpmRegLogicRcvSuc;
}CBTSCM_SOFTDECODE_INFO_STRU;

/**************************************************************************
  6 ��������
**************************************************************************/


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif


