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
#ifndef _ATAPPVCOMINTERFACE_H_
#define _ATAPPVCOMINTERFACE_H_


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
/*�ṩ��OM ���豸����*/
#if (FEATURE_ON == FEATURE_VCOM_EXT)
#define APP_VCOM_DEV_INDEX_ERRLOG         (APP_VCOM_DEV_INDEX_53)
#define APP_VCOM_DEV_INDEX_TLLOG          (APP_VCOM_DEV_INDEX_54)
#define APP_VCOM_DEV_INDEX_CBT            (APP_VCOM_DEV_INDEX_55)
#define APP_VCOM_DEV_INDEX_LOG            (APP_VCOM_DEV_INDEX_56)
#define APP_VCOM_DEV_INDEX_LOG1           (APP_VCOM_DEV_INDEX_57)
#else
#define APP_VCOM_DEV_INDEX_CBT            (APP_VCOM_DEV_INDEX_BUTT)
#define APP_VCOM_DEV_INDEX_TLLOG          (APP_VCOM_DEV_INDEX_BUTT)
#define APP_VCOM_DEV_INDEX_ERRLOG         (APP_VCOM_DEV_INDEX_BUTT)
#define APP_VCOM_DEV_INDEX_LOG            (APP_VCOM_DEV_INDEX_BUTT)
#define APP_VCOM_DEV_INDEX_LOG1           (APP_VCOM_DEV_INDEX_BUTT)
#endif
/*****************************************************************************
  3 ö�ٶ���
*****************************************************************************/


enum APP_VCOM_DEV_INDEX
{
    APP_VCOM_DEV_INDEX_0                 = 0,
    APP_VCOM_DEV_INDEX_1                 = 1,
#if (FEATURE_ON == FEATURE_VCOM_EXT)
    APP_VCOM_DEV_INDEX_2                 = 2,
    APP_VCOM_DEV_INDEX_3                 = 3,
    APP_VCOM_DEV_INDEX_4                 = 4,
    APP_VCOM_DEV_INDEX_5                 = 5,
    APP_VCOM_DEV_INDEX_6                 = 6,
    APP_VCOM_DEV_INDEX_7                 = 7,
    APP_VCOM_DEV_INDEX_8                 = 8,
    APP_VCOM_DEV_INDEX_9                 = 9,
    APP_VCOM_DEV_INDEX_10                = 10,
    APP_VCOM_DEV_INDEX_11                = 11,
    APP_VCOM_DEV_INDEX_12                = 12,
    APP_VCOM_DEV_INDEX_13                = 13,
    APP_VCOM_DEV_INDEX_14                = 14,
    APP_VCOM_DEV_INDEX_15                = 15,
    APP_VCOM_DEV_INDEX_16                = 16,
    APP_VCOM_DEV_INDEX_17                = 17,
    APP_VCOM_DEV_INDEX_18                = 18,
    APP_VCOM_DEV_INDEX_19                = 19,
    APP_VCOM_DEV_INDEX_20                = 20,
    APP_VCOM_DEV_INDEX_21                = 21,
    APP_VCOM_DEV_INDEX_22                = 22,
    APP_VCOM_DEV_INDEX_23                = 23,
    APP_VCOM_DEV_INDEX_24                = 24,
    APP_VCOM_DEV_INDEX_25                = 25,
    APP_VCOM_DEV_INDEX_26                = 26,
    APP_VCOM_DEV_INDEX_27                = 27,
    APP_VCOM_DEV_INDEX_28                = 28,
    APP_VCOM_DEV_INDEX_29                = 29,
    APP_VCOM_DEV_INDEX_30                = 30,
    APP_VCOM_DEV_INDEX_31                = 31,

    APP_VCOM_DEV_INDEX_32                = 32,
    APP_VCOM_DEV_INDEX_33                = 33,
    APP_VCOM_DEV_INDEX_34                = 34,
    APP_VCOM_DEV_INDEX_35                = 35,
    APP_VCOM_DEV_INDEX_36                = 36,
    APP_VCOM_DEV_INDEX_37                = 37,
    APP_VCOM_DEV_INDEX_38                = 38,
    APP_VCOM_DEV_INDEX_39                = 39,
    APP_VCOM_DEV_INDEX_40                = 40,
    APP_VCOM_DEV_INDEX_41                = 41,
    APP_VCOM_DEV_INDEX_42                = 42,
    APP_VCOM_DEV_INDEX_43                = 43,
    APP_VCOM_DEV_INDEX_44                = 44,
    APP_VCOM_DEV_INDEX_45                = 45,
    APP_VCOM_DEV_INDEX_46                = 46,
    APP_VCOM_DEV_INDEX_47                = 47,
    APP_VCOM_DEV_INDEX_48                = 48,
    APP_VCOM_DEV_INDEX_49                = 49,
    APP_VCOM_DEV_INDEX_50                = 50,
    APP_VCOM_DEV_INDEX_51                = 51,
    APP_VCOM_DEV_INDEX_52                = 52,
    APP_VCOM_DEV_INDEX_53                = 53,
    APP_VCOM_DEV_INDEX_54                = 54,
    APP_VCOM_DEV_INDEX_55                = 55,
    APP_VCOM_DEV_INDEX_56                = 56,
    APP_VCOM_DEV_INDEX_57                = 57,
    APP_VCOM_DEV_INDEX_58                = 58,
    APP_VCOM_DEV_INDEX_59                = 59,
    APP_VCOM_DEV_INDEX_60                = 60,
    APP_VCOM_DEV_INDEX_61                = 61,
    APP_VCOM_DEV_INDEX_62                = 62,
    APP_VCOM_DEV_INDEX_63                = 63,
#endif
    APP_VCOM_DEV_INDEX_BUTT
};
typedef VOS_UINT8 APP_VCOM_DEV_INDEX_UINT8;

enum APP_VCOM_EVT
{
    APP_VCOM_EVT_RELEASE   = 0,     /* �˿ڹر��¼� */
    APP_VCOM_EVT_OPEN      = 1,     /* �˿ڴ��¼� */
    APP_VCOM_EVT_BUTT
};
typedef VOS_UINT32 APP_VCOM_EVT_UINT32;
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

/*****************************************************************************
  8 UNION����
*****************************************************************************/

/*****************************************************************************
  9 OTHERS����
*****************************************************************************/

/*****************************************************************************
  10 ��������
*****************************************************************************/


typedef int (*SEND_UL_AT_FUNC)(VOS_UINT8 ucDevIndex, VOS_UINT8 *pData, VOS_UINT32 uslength);
typedef int (*EVENT_FUNC)(APP_VCOM_EVT_UINT32 event);

/*****************************************************************************
 �� �� ��  : APP_VCOM_RegDataCallback
 ��������  : �ṩ�ϲ�Ӧ�ó���ע�����ݽ��ջص�����ָ���API�ӿں�����
 �������  : uPortNo������ʵ���š�
             pCallback���ص�����ָ�롣
 �������  : �ޡ�
 �� �� ֵ  : 0:  �����ɹ���
             -1������ʧ�ܡ�
*****************************************************************************/
extern VOS_UINT32 APP_VCOM_RegDataCallback(VOS_UINT8 ucDevIndex, SEND_UL_AT_FUNC pFunc);
/*****************************************************************************
 �� �� ��  : APP_VCOM_RegEvtCallback
 ��������  : VCOMΪ�ⲿģ���ṩ��ע��˿��¼��������ӿڡ�
 �������  : uPortNo������ʵ���š�
             pCallback���ص�����ָ�롣
 �������  : �ޡ�
 �� �� ֵ  : 0:  �����ɹ���
             -1������ʧ�ܡ�
*****************************************************************************/
extern VOS_UINT32 APP_VCOM_RegEvtCallback(VOS_UINT8 ucDevIndex, EVENT_FUNC pFunc);
/*****************************************************************************
 �� �� ��  : APP_VCOM_Send
 ��������  : ʵ�����⴮��ͨ�������ݷ��͹��ܣ����ϲ�������ݵĴ���������
              ֱ�Ӵ�����APP��
 �������  : uPortNo�����⴮��ʵ���š�
             pData��������ָ�롣
             uslength�������ֽ�����
 �������  : �ޡ�
 �� �� ֵ  : 0�������ɹ���
             ����ֵ������ʧ��

*****************************************************************************/
extern VOS_UINT32 APP_VCOM_Send (VOS_UINT8 ucDevIndex, VOS_UINT8 *pData, VOS_UINT32 uslength);
#define APP_VCOM_SEND(ucDevIndex,pData,uslength) APP_VCOM_Send(ucDevIndex,pData,uslength)


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

