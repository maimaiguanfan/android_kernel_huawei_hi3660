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

#ifndef __ADSDEVINTERFACE_H__
#define __ADSDEVINTERFACE_H__

/*****************************************************************************
  1 ͷ�ļ�����
*****************************************************************************/
#include "ImmInterface.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(4)


/*****************************************************************************
  2 �궨��
*****************************************************************************/
/* ����IPV6 MTU��󳤶�ֵ */
#define ADS_MTU_LEN_MAX                 (1500)

/* !!!!!!!!!!!��ʱ���壬���յ�ֵ�ɱ���ȷ�� */
#define ADS_NDIS_MSG_HDR                (0x00)
#define NDIS_ADS_MSG_HDR                (0x00)


/*****************************************************************************
  3 ö�ٶ���
*****************************************************************************/

enum ADS_PKT_TYPE_ENUM
{
    ADS_PKT_TYPE_IPV4 = 0x00,                                                   /* IPV4 */
    ADS_PKT_TYPE_IPV6 = 0x01,                                                   /* IPV6 */
    ADS_PKT_TYPE_BUTT
};
typedef VOS_UINT8 ADS_PKT_TYPE_ENUM_UINT8;


enum ADS_NDIS_IP_PACKET_TYPE_ENUM
{
    ADS_NDIS_IP_PACKET_TYPE_DHCPV4      = 0x00,
    ADS_NDIS_IP_PACKET_TYPE_DHCPV6      = 0x01,
    ADS_NDIS_IP_PACKET_TYPE_ICMPV6      = 0x02,
    ADS_NDIS_IP_PACKET_TYPE_LINK_FE80   = 0x03,
    ADS_NDIS_IP_PACKET_TYPE_LINK_FF     = 0x04,
    ADS_NDIS_IP_PACKET_TYPE_BUTT
};
typedef VOS_UINT8 ADS_NDIS_IP_PACKET_TYPE_ENUM_UINT8;


enum ADS_NDIS_MSG_ID_ENUM
{
    ID_ADS_NDIS_DATA_IND               = ADS_NDIS_MSG_HDR + 0x00,               /* ADS->CDS IP PACKET IND */
    ID_ADS_NDIS_MSG_ID_ENUM_BUTT
};
typedef VOS_UINT32  ADS_NDIS_MSG_ID_ENUM_UINT32;


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
    VOS_MSG_HEADER                                                              /*_H2ASN_Skip*/
    ADS_NDIS_MSG_ID_ENUM_UINT32         enMsgId;                                /*_H2ASN_Skip*/
    MODEM_ID_ENUM_UINT16                enModemId;
    VOS_UINT8                           ucRabId;                                /* RAB��ʶ��ȡֵ��Χ:[5,15] */
    ADS_NDIS_IP_PACKET_TYPE_ENUM_UINT8  enIpPacketType;
    VOS_UINT8                           aucReserved[4];
    IMM_ZC_STRU                        *pstSkBuff;                              /* ���ݰ��ṹָ�� */
} ADS_NDIS_DATA_IND_STRU;


typedef struct
{
    VOS_UINT32                          bitOpIpv4Addr       : 1;
    VOS_UINT32                          bitOpIpv6Addr       : 1;
    VOS_UINT32                          bitOpSpare          : 30;

    VOS_UINT8                           aucIpv4Addr[4];
    VOS_UINT8                           aucIpv6Addr[16];
} ADS_FILTER_IP_ADDR_INFO_STRU;


/*****************************************************************************
  8 UNION����
*****************************************************************************/


/*****************************************************************************
  9 OTHERS����
*****************************************************************************/

/*****************************************************************************
  10 ��������
*****************************************************************************/
/*****************************************************************************
 �� �� ��  : ADS_UL_SendPacket
 ��������  : ADS����Ϊ�ϲ�ģ���ṩ�����ݷ��ͺ��������ӿڲ��ͷ��ڴ档
            �ϲ�ģ����ݷ���ֵ�ж��Ƿ���Ҫ�ͷ��ڴ�
 �������  : pstImmZc  --- IMM����
             ucExRabId --- RABID [5, 15]
             (*****ucExRabIdΪ��չ��RabId����2bit��ʾModemId����6bit��ʾRabId*****)
 �������  : ��
 �� �� ֵ  : VOS_UINT32
*****************************************************************************/

VOS_UINT32 ADS_UL_SendPacket(
    IMM_ZC_STRU                        *pstImmZc,
    VOS_UINT8                           ucExRabId
);

typedef VOS_UINT32 (*RCV_DL_DATA_FUNC)(VOS_UINT8 ucExRabId, IMM_ZC_STRU *pData, ADS_PKT_TYPE_ENUM_UINT8 enPktType, VOS_UINT32 ulExParam);

/*****************************************************************************
 �� �� ��  : ADS_DL_RegDlDataCallback
 ��������  : ADS�������ݴ���ģ��Ϊ�ϲ�ģ���ṩ��ע���������ݽ��պ����ӿ�
 �������  : ucExRabId --- ��չ���غ�
             (*****ucExRabIdΪ��չ��RabId����2bit��ʾModemId����6bit��ʾRabId*****)
             pFunc     --- ���ݽ��ջص�����ָ��
             ulExParam --- ��չ����
 �������  : ��
 �� �� ֵ  : VOS_OK/VOS_ERR
*****************************************************************************/
VOS_UINT32 ADS_DL_RegDlDataCallback(
    VOS_UINT8                           ucExRabId,
    RCV_DL_DATA_FUNC                    pFunc,
    VOS_UINT32                          ulExParam
);

/*****************************************************************************
 �� �� ��  : ADS_DL_RegFilterDataCallback
 ��������  : ע���������ݹ��˻ص���չ
 �������  : ucRabId         - RABID [5, 15]
             pstFilterIpAddr - IP��ַ��Ϣ
             pFunc           - ����ָ��
 �������  : ��
 �� �� ֵ  : VOS_OK          - ע��ɹ�
             VOS_ERR         - ע��ʧ��
*****************************************************************************/
VOS_UINT32 ADS_DL_RegFilterDataCallback(
    VOS_UINT8                           ucRabId,
    ADS_FILTER_IP_ADDR_INFO_STRU       *pstFilterIpAddr,
    RCV_DL_DATA_FUNC                    pFunc
);

/*****************************************************************************
 �� �� ��  : ADS_DL_DeregFilterDataCallback
 ��������  : ȥע���������ݹ��˻ص���չ
 �������  : ucRabId - RABID [5, 15]
 �������  : ��
 �� �� ֵ  : VOS_OK  - ȥע��ɹ�
             VOS_ERR - ȥע��ʧ��
*****************************************************************************/
VOS_UINT32 ADS_DL_DeregFilterDataCallback(VOS_UINT8 ucRabId);

/*****************************************************************************
 �� �� ��  : ADS_UL_SendPacketEx
 ��������  : ���з���������չ�ӿ�, ʹ�øýӿڷ��͵�������ADS��¼����Ϣ, ����
             �������ݹ���ƥ��, �ýӿڱ����ADS_DL_RegFilterDataCallback���
             ʹ��, ֻ��ʹ��ADS_DL_RegFilterDataCallbackע������й��˻ص���,
             �������ݲ���Ҫ����ʹ�øýӿڼ�¼����Ϣ���й���
 �������  : pstImmZc  --- IMM����
             enIpType  --- IP����
             ucExRabId --- RABID [5, 15]
             (*****ucExRabIdΪ��չ��RabId����2bit��ʾModemId����6bit��ʾRabId*****)
 �������  : ��
 �� �� ֵ  : VOS_OK/VOS_ERR
*****************************************************************************/
VOS_UINT32 ADS_UL_SendPacketEx(
    IMM_ZC_STRU                        *pstImmZc,
    ADS_PKT_TYPE_ENUM_UINT8             enIpType,
    VOS_UINT8                           ucExRabId
);

#if (FEATURE_ON == FEATURE_RNIC_NAPI_GRO)
typedef VOS_VOID (*RCV_RD_LAST_DATA_FUNC)(VOS_UINT32 ulRmNetId);

/*********************************************************************************
 �� �� ��  : ADS_DL_RegRdLastDataCallback
 ��������  : ADS�������ݴ���ģ��Ϊ�ϲ�ģ���ṩ��ע������RD���һ�����ݴ���ص�����
 �������  : ucExRabId --- ��չ���غ�
             (*****ucExRabIdΪ��չ��RabId����2bit��ʾModemId����6bit��ʾRabId*****)
             pFunc     --- ���ݽ��ջص�����ָ��
             ulExParam --- ��չ����
 �������  : ��
 �� �� ֵ  : VOS_OK/VOS_ERR
*********************************************************************************/
VOS_UINT32 ADS_DL_RegRdLastDataCallback(
    VOS_UINT8                           ucExRabId,
    RCV_RD_LAST_DATA_FUNC               pFunc,
    VOS_UINT32                          ulExParam
);
#endif

#if (defined(CONFIG_BALONG_SPE))
/*****************************************************************************
 �� �� ��  : ADS_IPF_SpeIntWakeupADS
 ��������  : SPE�жϴ����¼�, ����ADS��������������
 �������  : VOS_VOID
 �������  : ��
 �� �� ֵ  : VOS_VOID
*****************************************************************************/
VOS_VOID ADS_IPF_SpeIntWakeupADS(VOS_VOID);

/*****************************************************************************
 �� �� ��  : ADS_IPF_RecycleMem
 ��������  : �����ڴ�
 �������  : pstImmZc --- imm memory
 �������  : ��
 �� �� ֵ  : VOS_OK/VOS_ERROR
*****************************************************************************/
VOS_INT ADS_IPF_RecycleMem(IMM_ZC_STRU *pstImmZc);

/*****************************************************************************
 �� �� ��  : ADS_IPF_RegSpeWPort
 ��������  : ע��SPE�˿�
 �������  : lPort --- �˿�
 �������  : ��
 �� �� ֵ  : VOS_VOID
*****************************************************************************/
VOS_VOID ADS_IPF_RegSpeWPort(VOS_INT32 lPort);

/*****************************************************************************
 �� �� ��  : ADS_IPF_GetSpeWPortTdDepth
 ��������  : ��ȡSPE WPORT�˿ڵ�TD�������
 �������  : VOS_VOID
 �������  : ��
 �� �� ֵ  : 0    --- ʹ��SPE��Ĭ������
             ���� --- ����ADSԤ�����ڴ��������������
*****************************************************************************/
VOS_UINT32 ADS_IPF_GetSpeWPortTdDepth(VOS_VOID);
#endif



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

