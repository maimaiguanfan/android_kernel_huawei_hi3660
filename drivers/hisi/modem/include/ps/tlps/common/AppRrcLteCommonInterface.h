/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2012-2018. All rights reserved.
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

#ifndef __APPRRCLTECOMMONINTERFACE_H__
#define __APPRRCLTECOMMONINTERFACE_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 Include Headfile
*****************************************************************************/
#include  "vos.h"

#if (VOS_OS_VER != VOS_WIN32)
#pragma pack(4)
#else
#pragma pack(push, 4)
#endif

#define LMAX_NEIGHBOR_CELL_NUM    (16)
#ifndef VOS_MSG_HEADER
#define VOS_MSG_HEADER  VOS_UINT32 uwSenderCpuId;  \
                        VOS_UINT32 uwSenderPid;    \
                        VOS_UINT32 uwReceiverCpuId;\
                        VOS_UINT32 uwReceiverPid;  \
                        VOS_UINT32 uwLength;
#endif
#ifndef APP_MSG_HEADER
#define APP_MSG_HEADER                  VOS_UINT16   usOriginalId;\
                                        VOS_UINT16   usTerminalId;\
                                        VOS_UINT32   ulTimeStamp; \
                                        VOS_UINT32   ulSN;
#endif

enum RRC_OM_GET_CELL_INFO_FLAG_ENUM
{
    EN_GET_SERVICE_CELL_INFO = 0,
    EN_GET_NEIGHBOR_CELL_INFO = 1
};

typedef VOS_UINT32 RRC_OM_GET_CELL_INFO_FLAG_ENUM_UINT32;


/*****************************************************************************
 �ṹ��    : APP_LRRC_GET_NCELL_INFO_REQ_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : Э��ջ��APP��Ľӿ���Ϣ�Ľṹ��
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER                       /*_H2ASN_Skip*/
    VOS_UINT32             ulMsgId;        /*_H2ASN_Skip*/
    APP_MSG_HEADER
    VOS_UINT32             ulOpId;
    RRC_OM_GET_CELL_INFO_FLAG_ENUM_UINT32   enCellFlag;   /*=0��ʾ��ȡ����С����=1��ʾ��ȡ����*/
}APP_LRRC_GET_NCELL_INFO_REQ_STRU;


/*****************************************************************************
 �ṹ��    : LRRC_APP_SRVING_CELL_MEAS_RESULT_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : ����С����ͬƵ����ƵС����������ṹ��
*****************************************************************************/
typedef struct
{
    VOS_UINT16                          usPhyCellId;/* С��id */
    VOS_INT16                           sRsrp;/* RSRPֵ */
    VOS_INT16                           sRsrq;/* RSRqֵ */
    VOS_INT16                           sRssi;/* RSSIֵ */
}LRRC_APP_SRVING_CELL_MEAS_RESULT_STRU;


/*****************************************************************************
 �ṹ��    : LRRC_APP_CELL_INFO_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : ����С����ͬƵ����ƵС����Ϣ�ṹ��
*****************************************************************************/
typedef struct
{
    VOS_UINT16                          usFreqInfo;/* ����С��Ƶ�� */
    VOS_UINT16                          enBandInd;/* Ƶ��ָʾ */
    LRRC_APP_SRVING_CELL_MEAS_RESULT_STRU     stMeasRslt;
}LRRC_APP_CELL_INFO_STRU;

/*****************************************************************************
 �ṹ��    : LRRC_APP_CELL_MEAS_INFO_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : ����С����ͬƵ����ƵС����Ϣ�ṹ��
*****************************************************************************/
typedef struct
{
    VOS_UINT32   ulNCellNumber;
    LRRC_APP_CELL_INFO_STRU stCellMeasInfo[LMAX_NEIGHBOR_CELL_NUM];
}LRRC_APP_CELL_MEAS_INFO_STRU;

typedef LRRC_APP_CELL_MEAS_INFO_STRU LRRC_APP_SRV_CELL_MEAS_INFO_STRU;
typedef LRRC_APP_CELL_MEAS_INFO_STRU LRRC_APP_INTRA_FREQ_NCELL_MEAS_INFO_STRU;
typedef LRRC_APP_CELL_MEAS_INFO_STRU LRRC_APP_INTER_FREQ_NCELL_MEAS_INFO_STRU;


/*****************************************************************************
 �ṹ��    : LRRC_APP_BSIC_INFO_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : BSC��Ϣ
*****************************************************************************/
typedef struct
{
    VOS_UINT16                                              usNcc;
    VOS_UINT16                                              usBcc;
}LRRC_APP_BSIC_INFO_STRU;
/*****************************************************************************
 �ṹ��    : LRRC_GERAN_NCELL_INFO_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : L��ģ�£�GSM ������Ϣ
*****************************************************************************/

typedef struct
{
    VOS_UINT16    usArfcn;
    VOS_INT16     sRSSI;
    LRRC_APP_BSIC_INFO_STRU     stBSIC;
}LRRC_GERAN_NCELL_INFO_STRU;

/*****************************************************************************
 �ṹ��    : LRRC_UMTS_NCELL_INFO_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : L��ģ�£�UMTS������Ϣ�ṹ��
*****************************************************************************/
typedef struct
{
    VOS_UINT16    usARFCN;
    VOS_UINT16    usPrimaryScramCode;
    VOS_INT16     sCpichRscp;
    VOS_INT16     sCpichEcN0;
}LRRC_UMTS_NCELL_INFO_STRU;

/*****************************************************************************
 �ṹ��    : LRRC_APP_INTER_RAT_UMTS_NCELL_LIST_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : L��ģ�£�UMTS������Ϣ�б�
*****************************************************************************/

typedef struct
{
    VOS_UINT32 ulNCellNumber;
    LRRC_UMTS_NCELL_INFO_STRU stUMTSNcellList[LMAX_NEIGHBOR_CELL_NUM];
}LRRC_APP_INTER_RAT_UMTS_NCELL_LIST_STRU;


/*****************************************************************************
 �ṹ��    : LRRC_APP_INTER_RAT_GERAN_NCELL_LIST_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : L��ģ�£�GSM ������Ϣ�б�
*****************************************************************************/
typedef struct
{
    VOS_UINT32    ulNCellNumber;
    LRRC_GERAN_NCELL_INFO_STRU stGeranNcellList[LMAX_NEIGHBOR_CELL_NUM];
}LRRC_APP_INTER_RAT_GERAN_NCELL_LIST_STRU;


/*****************************************************************************
 �ṹ��    : LRRC_APP_NCELL_LIST_INFO_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : ����С����ͬƵ����Ƶ�Լ���ϵͳ�������
*****************************************************************************/
typedef struct
{
    RRC_OM_GET_CELL_INFO_FLAG_ENUM_UINT32      enCellFlag;   /*=0��ʾ��ȡ����С����=1��ʾ��ȡ����*/
    LRRC_APP_SRV_CELL_MEAS_INFO_STRU          stSevCellInfo; /*����С����Ϣ������Ƶ�㡢band��Phycial ID,RSRP,RSRQ,RSSI*/
    LRRC_APP_INTRA_FREQ_NCELL_MEAS_INFO_STRU  stIntraFreqNcellList;/* ͬƵ������Ϣ*/
    LRRC_APP_INTER_FREQ_NCELL_MEAS_INFO_STRU  stInterFreqNcellList;/* ��Ƶ������Ϣ*/
    LRRC_APP_INTER_RAT_UMTS_NCELL_LIST_STRU    stInterRATUMTSNcellList; /* WCDMA/TDSCDMA��ϵͳ����*/
    LRRC_APP_INTER_RAT_GERAN_NCELL_LIST_STRU  stInterRATGeranNcellList; /* GERAN��ϵͳ���� */
}LRRC_APP_NCELL_LIST_INFO_STRU;

/*****************************************************************************
 �ṹ��    : LRRC_APP_GET_NCELL_INFO_CNF_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : ����С����ͬƵ����Ƶ�Լ���ϵͳ������Ϣ
*****************************************************************************/

typedef struct
{
    VOS_MSG_HEADER                       /*_H2ASN_Skip*/
    VOS_UINT32             ulMsgId;        /*_H2ASN_Skip*/
    APP_MSG_HEADER
    VOS_UINT32             ulOpId;
    VOS_UINT32             enResult;
    LRRC_APP_NCELL_LIST_INFO_STRU stNcellListInfo;
}LRRC_APP_GET_NCELL_INFO_CNF_STRU;



#if (VOS_OS_VER != VOS_WIN32)
#pragma pack()
#else
#pragma pack(pop)
#endif




#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of AppRrcLteCommonInterface.h */
