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

 

#ifndef __OM_HDLC_H__
#define __OM_HDLC_H__

/******************************************************************************
  1 ����ͷ�ļ�����
******************************************************************************/
#include "vos.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(4)
/******************************************************************************
  2 �궨��
******************************************************************************/
#define OM_HDLC_FRAME_FLAG  0x7e        /* HDLC֡��־λ���̶�Ϊ0x7e�����ֲ�ͬ��HDLC֡ */
#define OM_HDLC_ESC         0x7d        /* ת���ַ������������ַ���Ҫת�� */
#define OM_HDLC_ESC_MASK    0x20        /* ת���ַ����� */

#define OM_HDLC_INIT_FCS    0xffff      /* ����FCS�ĳ�ʼFCSֵ */
#define OM_HDLC_GOOD_FCS    0xf0b8      /* ������Ϊ��FCSֵʱ��˵��FCSУ����ȷ */

#define OM_HDLC_MODE_HUNT   0x1         /* ���װģʽ��bit0Ϊ1������δ�ҵ�֡��־��û�п�ʼ���װ */
#define OM_HDLC_MODE_ESC    0x2         /* ���װģʽ��bit1Ϊ1�������ϸ��ַ�Ϊת���ַ�����ǰ�ַ�Ϊ�������ݵ����� */

#define OM_HDLC_FCS_LEN     2           /* HDLC֡��FCSδת��ʱ�ĳ���(��λ: Byte) */


/******************************************************************************
  3 ö�ٶ���
******************************************************************************/
enum OM_HDLC_RESULT_ENUM
{
    HDLC_SUCC           = 0,                /* 0    HDLC����ɹ�           */
    HDLC_NOT_HDLC_FRAME = 1,                /* 1    û��������֡             */
    HDLC_FCS_ERROR,                         /* 2    ������У�����        */
    HDLC_FRAME_DISCARD,                     /* 3    �����쳣��������ǰHDLC֡ */
    HDLC_BUFF_FULL,                         /* 4    Ŀ�Ļ�����           */
    HDLC_PARA_ERROR,                        /* 5    ��������쳣           */
    HDLC_BUTT
};
typedef VOS_UINT32 OM_HDLC_RESULT_ENUM_UINT32;


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


typedef struct
{
    VOS_UINT32  ulMode;             /* HDLC����ʹ�õ��ڲ�������HDLC����ά�� */
    VOS_UINT32  ulLength;           /* HDLC����ʱ�ڲ�ά�����м���� */
    VOS_UINT32  ulDecapBuffSize;    /* HDLC�������ݴ��BUFFER�ĳ��ȣ������߸���ָ�� */
    VOS_UINT8  *pucDecapBuff;       /* HDLC�������ݴ�ŵ�ַ���ɵ����߸���ָ���ռ� */
    VOS_UINT32  ulInfoLen;          /* HDLC�����ɹ������ݳ��ȣ�HDLC����ά�� */
} OM_HDLC_STRU;


/******************************************************************************
  8 UNION����
******************************************************************************/


/******************************************************************************
  9 ��������
******************************************************************************/
extern VOS_UINT32 Om_HdlcEncap(
                const VOS_UINT8  *pucSrc,
                VOS_UINT16  usSrcLen,
                VOS_UINT8  *pucDest,
                VOS_UINT16  usDestBuffLen,
                VOS_UINT16 *pusDestLen );

extern VOS_VOID Om_HdlcInit( OM_HDLC_STRU *pstHdlc );

extern OM_HDLC_RESULT_ENUM_UINT32 Om_HdlcDecap( OM_HDLC_STRU *pstHdlc, VOS_UINT8 ucChar );



#if ((VOS_OS_VER == VOS_WIN32) || (VOS_OS_VER == VOS_NUCLEUS))
#pragma pack()
#else
#pragma pack(0)
#endif

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* OmHdlcInterface.h */




