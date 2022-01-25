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
#ifndef _ACPU_RESET_H_
#define _ACPU_RESET_H_

/*****************************************************************************
  1 ����ͷ�ļ�����
*****************************************************************************/
#include "vos.h"
#include "mdrv.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(4)

/*****************************************************************************
  1 ȫ�ֱ�������
*****************************************************************************/

/*****************************************************************************
  2 �궨��
*****************************************************************************/
#define  RNIC_RESET_TIMEOUT_LEN                     (5000)                      /* RNIC��λ��ʱ�䳤�� */

#define  AT_RESET_TIMEOUT_LEN                       (6000)                      /* AT��λ��ʱ�䳤�� */

#define  ADS_RESET_TIMEOUT_LEN                      (5000)                      /* ADS��λ��ʱ�䳤�� */

#define  NAS_ADS_UL_FUNC_PROC_NAME                  "NAS_ADSUL"                 /* ADSULע������������ */
#define  NAS_ADS_DL_FUNC_PROC_NAME                  "NAS_ADSDL"                 /* ADSDLע������������ */
#define  NAS_RNIC_FUNC_PROC_NAME                    "NAS_RNIC"                  /* RNICע������������ */
#define  NAS_AT_FUNC_PROC_NAME                      "NAS_AT"                    /* ATע������������ */
/*****************************************************************************
  3 ö�ٶ���
*****************************************************************************/


enum ACPU_RESET_PRIORITY_ENUM
{
    ACPU_RESET_PRIORITY_RNIC            = 4,                                    /* RNIC */
    ACPU_RESET_PRIORITY_ADS_UL          = 20,                                   /* ADS UL */
    ACPU_RESET_PRIORITY_ADS_DL,                                                 /* ADS DL */
    ACPU_RESET_PRIORITY_AT,                                                     /* AT */
    ACPU_RESET_PRIORITY_BUTT
};
typedef VOS_INT32 ACPU_RESET_PRIORITY_ENUM_INT32;


enum AT_RESET_REPORT_CMD_ENUM
{
    AT_RESET_REPORT_CAUSE_CCPU_START = 0,
    AT_RESET_REPORT_CAUSE_CCPU_END,
    AT_RESET_REPORT_CAUSE_HIFI_START,
    AT_RESET_REPORT_CAUSE_HIFI_END,
    AT_RESET_REPORT_CMD_BUTT
};
typedef VOS_INT32 AT_RESET_REPORT_CAUSE_ENUM_UINT32;

/*****************************************************************************
  4 ȫ�ֱ�������
*****************************************************************************/


/*****************************************************************************
  5 ��Ϣͷ����
*****************************************************************************/


/*****************************************************************************
  6 ��Ϣ����
*****************************************************************************/



enum AT_RESET_MSG_ID_ENUM
{
    /* ��AT��AT������Ϣ�У�ʹ��(AT_MSG_STRU)ucType���б���ʲô��Ϣ�Ĵ���
       �Ѿ�ʹ�õ���:0x00��ʾAT�����·���0x01Ϊ����ϱ���0x02Ϊ���������
       ��������ӵ�AT��AT����ϢIDֵ��0x10��ʼ */
    ID_CCPU_AT_RESET_START_IND          = 0x10,                                 /* CCPU��λǰ��AT����Ϣָʾ */
    ID_CCPU_AT_RESET_END_IND            = 0x11,                                 /* CCPU��λ���AT����Ϣָʾ */
    ID_HIFI_AT_RESET_START_IND          = 0x12,                                 /* HIFI��λǰ��AT����Ϣָʾ */
    ID_HIFI_AT_RESET_END_IND            = 0x13,                                 /* HIFI��λ���AT����Ϣָʾ */

    ID_AT_RESET_MSG_ID_ENUM_BUTT
};
typedef VOS_UINT32  AT_RESET_MSG_ID_ENUM_UINT8;

/*****************************************************************************
  7 STRUCT����
*****************************************************************************/

/*****************************************************************************
  8 UNION����
*****************************************************************************/


/*****************************************************************************
  9 OTHERS����
*****************************************************************************/

/*****************************************************************************
  10 ��������
*****************************************************************************/

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

#endif /* end of AcpuReset.h */


