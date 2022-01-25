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

#ifndef __RNIC_NETMGR_INTERFACE_H__
#define __RNIC_NETMGR_INTERFACE_H__

/*****************************************************************************
  1 ����ͷ�ļ�����
*****************************************************************************/


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


#pragma pack(4)

/*****************************************************************************
  2 �궨��
*****************************************************************************/
#define   NM_MSG_BUFFER_SIZE       (sizeof(NM_MSG_STRU))   /* rnic��netmanager֮����Ϣbuffer��С */

#define   NM_IPV4_ADDR_LENGTH      (4)     /* IPV4��ַ���ȣ���λ�ֽ� */
#define   NM_IPV6_ADDR_LENGTH      (16)    /* IPV6��ַ����, ��λ�ֽ� */

/*****************************************************************************
  3 ö�ٶ���
*****************************************************************************/

enum NM_MODEMID_ENUM
{
    NM_MODEMID_0                  = 0x00,
    NM_MODEMID_1                  = 0x01,
    NM_MODEMID_2                  = 0x02,

    NM_MODEMID_BUTT
};
typedef unsigned short NM_MODEMID_ENUM_UINT16;


enum NM_IMS_RAT_ENUM
{
    NM_IMS_RAT_CELLULAR       = 0x00,
    NM_IMS_RAT_WIFI           = 0x01,
    NM_IMS_RAT_BUTT
};
typedef unsigned char NM_IMS_RAT_ENUM_UINT8;

/*****************************************************************************
  4 ȫ�ֱ�������
*****************************************************************************/


/*****************************************************************************
  5 ��Ϣͷ����
*****************************************************************************/


/*****************************************************************************
  6 ��Ϣ����
*****************************************************************************/

enum NM_MSG_ID_ENUM
{
    /* RNIC����NetManager����Ϣö�� */
    ID_NM_PDN_ACT_IND              = 0x0001,   /* PDN����ָʾ */
    ID_NM_PDN_DEACT_IND            = 0x0002,   /* PDNȥ����ָʾ */
    ID_NM_PDN_MODIFY_IND           = 0x0003,   /* PDN�޸�ָʾ */

    ID_NM_MODEM_RESET_IND          = 0x0004,   /* modem������λ */

    /* NetManager����RNIC����Ϣö�� */
    ID_NM_MSG_ID_ENUM_BUTT

};
typedef unsigned int NM_MSG_ID_ENUM_UINT32;

/*****************************************************************************
  7 STRUCT����
*****************************************************************************/

typedef struct
{
    unsigned int                        bitOpDnsPrim         :1;                     /* �ⲿģ����д��ָʾaucDnsPrimAddr�Ƿ���Ҫ���� */
    unsigned int                        bitOpDnsSec          :1;                     /* �ⲿģ����д��ָʾaucDnsSecAddr�Ƿ���Ҫ���� */
    unsigned int                        bitOpSpare           :30;

    unsigned char                       aucIpV4Addr[NM_IPV4_ADDR_LENGTH];       /* IP��ַ */
    unsigned char                       aucDnsPrimAddr[NM_IPV4_ADDR_LENGTH];    /* ��DNS������IP */
    unsigned char                       aucDnsSecAddr[NM_IPV4_ADDR_LENGTH];     /* ��DNS������IP */
}NM_IPV4_PDN_INFO_STRU;


typedef struct
{
    unsigned int                        bitOpDnsPrim         :1;                     /* �ⲿģ����д��ָʾaucDnsPrimAddr�Ƿ���Ҫ���� */
    unsigned int                        bitOpDnsSec          :1;                     /* �ⲿģ����д��ָʾaucDnsSecAddr�Ƿ���Ҫ���� */
    unsigned int                        bitOpSpare           :30;

    unsigned int                        ulBitPrefixLen;                              /* IPV6ǰ׺����,��λΪbit */
    unsigned char                       aucIpV6Addr[NM_IPV6_ADDR_LENGTH];       /* IP��ַ */
    unsigned char                       aucDnsPrimAddr[NM_IPV6_ADDR_LENGTH];    /* ��DNS������IP */
    unsigned char                       aucDnsSecAddr[NM_IPV6_ADDR_LENGTH];     /* ��DNS������IP */
}NM_IPV6_PDN_INFO_STRU;


typedef struct
{
    unsigned short                      usSockMinPort;      /* socket �˿ڷ�Χ���� */
    unsigned short                      usSockMaxPort;      /* socket �˿ڷ�Χ���� */
}NM_SOCK_PORT_INFO_STRU;


typedef struct
{
    unsigned int                        bitOpIpv4PdnInfo : 1;
    unsigned int                        bitOpIpv6PdnInfo : 1;
    unsigned int                        bitOpSockPortInfo: 1;
    unsigned int                        bitOpSpare       : 29;

    NM_MODEMID_ENUM_UINT16              enModemId;                  /* ��ǰModem�� */
    NM_IMS_RAT_ENUM_UINT8               enRatType;                  /* ע���� lte����wifi */
    unsigned char                       ucReserv;

    NM_IPV4_PDN_INFO_STRU               stIpv4PdnInfo;
    NM_IPV6_PDN_INFO_STRU               stIpv6PdnInfo;
    NM_SOCK_PORT_INFO_STRU              stSockPortInfo;

    unsigned int                        ulMtuSize;

}NM_PDN_INFO_CONFIG_STRU;


typedef struct
{
    NM_MODEMID_ENUM_UINT16              enModemId;                  /* ��ǰModem�� */
    NM_IMS_RAT_ENUM_UINT8               enRatType;                  /* ע���� lte����wifi */
    unsigned char                       ucReserv;
}NM_PDN_DEACT_IND_STRU;


typedef struct
{
    NM_MSG_ID_ENUM_UINT32               enMsgId;
    unsigned int                        ulMsgLen;

    /* ��ǰ����4����Ϣ������reset��Ϣ������Ҫ��ϢID
       active��modify��Ϣ����pdn cfg ind��Ϣ*/
    union
    {
        NM_PDN_INFO_CONFIG_STRU        stPdnCfgInfo;
        NM_PDN_DEACT_IND_STRU          stPdnDeactInd;
    } unMsgInfo;
}NM_MSG_STRU;

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

#endif /* end of RnicNMInterface.h */
