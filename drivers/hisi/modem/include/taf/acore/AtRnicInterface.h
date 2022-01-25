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
#ifndef __AT_RNIC_INTERFACE_H__
#define __AT_RNIC_INTERFACE_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 ����ͷ�ļ�����
*****************************************************************************/
#include "vos.h"
#include "PsCommonDef.h"
#include "TafTypeDef.h"

#include "NasNvInterface.h"

#include "RnicInterface.h"

#pragma pack(4)

/*****************************************************************************
  2 �궨��
*****************************************************************************/
/*================================================*/
/* ��ֵ�궨�� */
/*================================================*/
#if (VOS_OS_VER == VOS_WIN32)
/* ����ST���������IP���ݰ����ȣ���λ�ֽ� */
#define RNIC_IP_DATA_LEN                (12)
#endif
/* APP���ź�Ĭ�Ͽ���ʱ�� */
#define AT_APP_IDLE_TIME                (600)

/* ���IPV6��ַ���ȣ�������":" */
#define RNICITF_MAX_IPV6_ADDR_LEN       (16)

/* RABIDȡֵ��Χ[5 15] */
#define RNICITF_MIN_RAB_ID              (5)
#define RNICITF_MAX_RAB_ID              (15)

#define RNIC_MODEM_0_RMNET_ID_IS_VALID(ucRmNetId)\
    (((ucRmNetId) == RNIC_RMNET_ID_0)\
  || ((ucRmNetId) == RNIC_RMNET_ID_1)\
  || ((ucRmNetId) == RNIC_RMNET_ID_2))

/*================================================*/
/* ���ܺ����궨�� */
/*================================================*/

/*******************************************************************************
  3 ö�ٶ���
*******************************************************************************/

/*****************************************************************************
  4 ȫ�ֱ�������
*****************************************************************************/

/*****************************************************************************
  5 ��Ϣͷ����
*****************************************************************************/

/*****************************************************************************
  6 ��Ϣ����
*****************************************************************************/

enum AT_RNIC_MSG_ID_ENUM
{
    /* AT����RNIC����Ϣö�� */
    ID_AT_RNIC_DIAL_MODE_IND            = 0x0001,   /* ����ģʽָʾ */          /* _H2ASN_MsgChoice AT_RNIC_DIAL_MODE_IND_STRU */
    ID_AT_RNIC_DIAL_MODE_REQ            = 0x0002,   /* ����ģʽ��ѯ */          /* _H2ASN_MsgChoice AT_RNIC_DIAL_MODE_REQ_STRU */
    ID_AT_RNIC_IPV4_PDP_ACT_IND         = 0x0003,   /* IPV4 PDP����ָʾ */      /* _H2ASN_MsgChoice AT_RNIC_IPV4_PDP_ACT_IND_STRU */
    ID_AT_RNIC_IPV6_PDP_ACT_IND         = 0x0004,   /* IPV6 PDP����ָʾ */      /* _H2ASN_MsgChoice AT_RNIC_IPV6_PDP_ACT_IND_STRU */
    ID_AT_RNIC_IPV4V6_PDP_ACT_IND       = 0x0005,   /* IPV4V6 PDP����ָʾ */    /* _H2ASN_MsgChoice AT_RNIC_IPV4V6_PDP_ACT_IND_STRU */
    ID_AT_RNIC_PDP_DEACT_IND            = 0x0006,   /* PDPȥ����ָʾ */         /* _H2ASN_MsgChoice AT_RNIC_PDP_DEACT_IND_STRU */
    ID_AT_RNIC_DSFLOW_IND               = 0x0007,   /* �����ϱ�ָʾ */          /* _H2ASN_MsgChoice AT_RNIC_DSFLOW_IND_STRU */
    ID_AT_RNIC_MTU_CHANGE_IND           = 0x0008,   /* MTU����ָʾ */           /* _H2ASN_MsgChoice AT_RNIC_MTU_CHANGE_IND_STRU */
    /* Added by L47619 for V3R3 Share-PDP Project, 2013-6-3, begin */
    ID_AT_RNIC_PDN_INFO_CFG_IND         = 0x0009,   /* Share-PDP����ָʾ */      /* _H2ASN_MsgChoice AT_RNIC_PDN_INFO_CFG_IND_STRU */
    ID_AT_RNIC_PDN_INFO_REL_IND         = 0x000A,   /* Share-PDP�ͷ�ָʾ */      /* _H2ASN_MsgChoice AT_RNIC_PDN_INFO_REL_IND_STRU */
    /* Added by L47619 for V3R3 Share-PDP Project, 2013-6-3, end */

    /* RNIC����AT����Ϣö�� */
    ID_RNIC_AT_PDP_OPERATE_IND          = 0x1001,   /* PDP�������ȥ����ָʾ */ /* _H2ASN_MsgChoice RNIC_AT_PDP_OPERATE_IND_STRU */
    ID_RNIC_AT_DSFLOW_RSP               = 0x1002,   /* �����ظ� */              /* _H2ASN_MsgChoice RNIC_AT_DSFLOW_RSP_STRU */
    ID_RNIC_AT_DIAL_MODE_CNF            = 0x1003,   /* ����ģʽ��ѯ�ظ� */      /* _H2ASN_MsgChoice RNIC_AT_DIAL_MODE_CNF_STRU */
    ID_RNIC_AT_MSG_ID_ENUM_BUTT

};
typedef VOS_UINT32 AT_RNIC_MSG_ID_ENUM_UINT32;

/*****************************************************************************
  7 STRUCT����
*****************************************************************************/


typedef struct
{
    VOS_MSG_HEADER                                                                  /* _H2ASN_Skip */
    AT_RNIC_MSG_ID_ENUM_UINT32          enMsgId;                    /* ��ϢID */    /* _H2ASN_Skip */
    MN_CLIENT_ID_T                      clientId;                   /* Client ID */
    VOS_UINT8                           aucRsv[2];                  /* ���� */
}AT_RNIC_DIAL_MODE_REQ_STRU;


typedef struct
{
    VOS_MSG_HEADER                                                                  /* _H2ASN_Skip */
    AT_RNIC_MSG_ID_ENUM_UINT32          enMsgId;                    /* ��ϢID */    /* _H2ASN_Skip */
    MN_CLIENT_ID_T                      clientId;                   /* Client ID */
    VOS_UINT8                           aucRsv[2];                  /* ���� */
    VOS_UINT32                          ulDialMode;                 /* Dialģʽ */
    VOS_UINT32                          ulIdleTime;                 /* ��ʱ������ */
    VOS_UINT32                          ulEventReportFlag;          /*�Ƿ��Ӧ���ϱ���ʶ*/
}RNIC_AT_DIAL_MODE_CNF_STRU;


typedef struct
{
    VOS_MSG_HEADER                                                                  /* _H2ASN_Skip */
    AT_RNIC_MSG_ID_ENUM_UINT32          enMsgId;                    /* ��ϢID */    /* _H2ASN_Skip */
    MN_CLIENT_ID_T                      clientId;                   /* Client ID */
    /* Modified by l60609 for V9R1 IPv6&TAF/SM Project, 2013-4-27, begin */
    RNIC_RMNET_ID_ENUM_UINT8            enRnicRmNetId;              /* RNIC����ID */
    VOS_UINT8                           aucRsv[1];                  /* ���� */
    /* Modified by l60609 for V9R1 IPv6&TAF/SM Project, 2013-4-27, end */
}AT_RNIC_DSFLOW_IND_STRU;


typedef struct RNIC_DATA_RATE_STRU
{
    VOS_UINT32                          ulDLDataRate;                           /* ��ǰ�������� */
    VOS_UINT32                          ulULDataRate;                           /* ��ǰ�������� */
}RNIC_DATA_RATE_STRU;




typedef struct
{
    VOS_MSG_HEADER                                                  /* ��Ϣͷ */    /* _H2ASN_Skip */
    AT_RNIC_MSG_ID_ENUM_UINT32          enMsgId;                    /* ��Ϣ���� */  /* _H2ASN_Skip */
    MN_CLIENT_ID_T                      clientId;                   /* Client ID */
    VOS_UINT8                           aucRsv[2];                  /* ���� */
    RNIC_DATA_RATE_STRU                 stRnicDataRate;             /* ��ǰ�������� */
}RNIC_AT_DSFLOW_RSP_STRU;


typedef struct
{
    VOS_MSG_HEADER                                                  /* ��Ϣͷ */    /* _H2ASN_Skip */
    AT_RNIC_MSG_ID_ENUM_UINT32          enMsgId;                    /* ��Ϣ���� */  /* _H2ASN_Skip */
    VOS_UINT8                           ucRabId;                    /* RAB��ʶ��ȡֵ��Χ:[5,15] */
    VOS_UINT8                           ucRmNetId;                  /* ����ID */

    VOS_UINT8                           aucRsv[2];

    VOS_UINT32                          ulIpv4Addr;                 /* IPV4��IP��ַ�������� */
    VOS_UINT32                          ulNetMask;                  /* IPV4�����룬������ */
    VOS_UINT32                          ulGateWay;                  /* IPV4�����ص�ַ�������� */

    VOS_UINT32                          OP_PriDns   : 1;
    VOS_UINT32                          OP_SecDns   : 1;
    VOS_UINT32                          OP_PriWINNS : 1;
    VOS_UINT32                          OP_SecWINNS : 1;
    VOS_UINT32                          OP_Spare    : 28;
    VOS_UINT32                          ulPrimDNS;                  /* IPV4����DNS�������� */
    VOS_UINT32                          ulSecDNS;                   /* IPV4����DNS�������� */
    VOS_UINT32                          ulPrimWINNS;                /* IPV4����WINNS�������� */
    VOS_UINT32                          ulSecWINNS;                 /* IPV4�ĸ�WINNS�������� */
}AT_RNIC_IPV4_PDP_ACT_IND_STRU;


typedef struct
{
    VOS_MSG_HEADER                                                              /* ��Ϣͷ */    /* _H2ASN_Skip */
    AT_RNIC_MSG_ID_ENUM_UINT32          enMsgId;                                /* ��Ϣ���� */  /* _H2ASN_Skip */
    VOS_UINT8                           ucRabId;                                /* RAB��ʶ��ȡֵ��Χ:[5,15] */
    VOS_UINT8                           ucRmNetId;                              /* ����ID */

    VOS_UINT8                           aucRsv[2];

    VOS_UINT8                           aucIpv6Addr[RNICITF_MAX_IPV6_ADDR_LEN]; /* �� PDP�����Ĵ�����IPV6��ַ���ȣ�������":" */

    VOS_UINT32                          OP_PriDns   : 1;
    VOS_UINT32                          OP_SecDns   : 1;
    VOS_UINT32                          OP_Spare    : 30;
    VOS_UINT8                           aucPrimDNS[RNICITF_MAX_IPV6_ADDR_LEN];  /* �� PDP�����Ĵ�����IPV6��DNS���ȣ�������":" */
    VOS_UINT8                           aucSecDNS[RNICITF_MAX_IPV6_ADDR_LEN];   /* �� PDP�����Ĵ�����IPV6��DNS���ȣ�������":" */
}AT_RNIC_IPV6_PDP_ACT_IND_STRU;


typedef struct
{
    VOS_MSG_HEADER                                                              /* ��Ϣͷ */    /* _H2ASN_Skip */
    AT_RNIC_MSG_ID_ENUM_UINT32          enMsgId;                                /* ��Ϣ���� */  /* _H2ASN_Skip */

    VOS_UINT32                          OP_IPv4PriDns   : 1;
    VOS_UINT32                          OP_IPv4SecDns   : 1;
    VOS_UINT32                          OP_IPv4PriWINNS : 1;
    VOS_UINT32                          OP_IPv4SecWINNS : 1;
    VOS_UINT32                          OP_IPv6PriDns   : 1;
    VOS_UINT32                          OP_IPv6SecDns   : 1;
    VOS_UINT32                          OP_Spare    : 26;

    VOS_UINT8                           ucRabId;                                /* RAB��ʶ��ȡֵ��Χ:[5,15] */
    VOS_UINT8                           ucRmNetId;                              /* ����ID */

    VOS_UINT8                           aucRsv[2];


    VOS_UINT32                          ulIpv4Addr;                             /* IPV4��IP��ַ�������� */
    VOS_UINT32                          ulIPv4NetMask;                              /* IPV4�����룬������ */
    VOS_UINT32                          ulIPv4GateWay;                              /* IPV4�����ص�ַ�������� */
    VOS_UINT32                          ulIPv4PrimDNS;                              /* IPV4����DNS�������� */
    VOS_UINT32                          ulIPv4SecDNS;                               /* IPV4����DNS�������� */
    VOS_UINT32                          ulIPv4PrimWINNS;                            /* IPV4����WINNS�������� */
    VOS_UINT32                          ulIPv4SecWINNS;                             /* IPV4�ĸ�WINNS�������� */

    VOS_UINT8                           aucIpv6Addr[RNICITF_MAX_IPV6_ADDR_LEN];     /* �� PDP�����Ĵ�����IPV6��ַ���ȣ�������":" */
    VOS_UINT8                           aucIpv6PrimDNS[RNICITF_MAX_IPV6_ADDR_LEN];  /* �� PDP�����Ĵ�����IPV6��DNS���ȣ�������":" */
    VOS_UINT8                           aucIpv6SecDNS[RNICITF_MAX_IPV6_ADDR_LEN];   /* �� PDP�����Ĵ�����IPV6��DNS���ȣ�������":" */

}AT_RNIC_IPV4V6_PDP_ACT_IND_STRU;


typedef struct
{
    VOS_MSG_HEADER                                                              /* ��Ϣͷ */    /* _H2ASN_Skip */
    AT_RNIC_MSG_ID_ENUM_UINT32          enMsgId;                                /* ��Ϣ���� */  /* _H2ASN_Skip */
    VOS_UINT8                           ucRabId;                                /* RAB��ʶ��ȡֵ��Χ:[5,15] */
    VOS_UINT8                           ucRmNetId;                              /* ����ID */
    VOS_UINT8                           aucRsv[2];
}AT_RNIC_PDP_DEACT_IND_STRU;


typedef struct
{
    VOS_MSG_HEADER                                                              /* ��Ϣͷ */    /* _H2ASN_Skip */
    AT_RNIC_MSG_ID_ENUM_UINT32          enMsgId;                                /* ��Ϣ���� */  /* _H2ASN_Skip */
    VOS_UINT32                          ulMtuValue;                             /* MTUֵ */
    VOS_UINT8                           ucRmNetId;                              /* ����ID */
    VOS_UINT8                           aucRsv[3];                              /* ���� */
}AT_RNIC_MTU_CHANGE_IND_STRU;


/* Added by L47619 for V3R3 Share-PDP Project, 2013-6-3, begin */

typedef struct
{
    VOS_UINT32                          ulPdnAddr;                              /* IP��ַ */
    VOS_UINT32                          ulSubnetMask;                           /* ���������ַ */
    VOS_UINT32                          ulGateWayAddr;                          /* ���ص�ַ */
    VOS_UINT32                          ulDnsPrimAddr;                          /* ��DNS��ַ */
    VOS_UINT32                          ulDnsSecAddr;                           /* ��DNS��ַ */
    VOS_UINT32                          ulWinnsPrimAddr;                         /* ��WINNS��ַ */
    VOS_UINT32                          ulWinnsSecAddr;                          /* ��WINNS��ַ */
    VOS_UINT32                          ulPcscfPrimAddr;                        /* ��P-CSCF��ַ */
    VOS_UINT32                          ulPcscfSecAddr;                         /* ��P-CSCF��ַ */
    VOS_UINT32                          ulPcscfThiAddr;                         /* ����P-CSCF��ַ */
}AT_RNIC_IPV4_PDN_INFO_STRU;


typedef struct
{
    VOS_UINT8                           aucPdnAddr[RNICITF_MAX_IPV6_ADDR_LEN];      /* IP��ַ */
    VOS_UINT8                           aucDnsPrimAddr[RNICITF_MAX_IPV6_ADDR_LEN];  /* ��DNS��ַ */
    VOS_UINT8                           aucDnsSecAddr[RNICITF_MAX_IPV6_ADDR_LEN];   /* ��DNS��ַ */
    VOS_UINT8                           aucPcscfPrimAddr[RNICITF_MAX_IPV6_ADDR_LEN];/* ��P-CSCF��ַ */
    VOS_UINT8                           aucPcscfSecAddr[RNICITF_MAX_IPV6_ADDR_LEN]; /* ��P-CSCF��ַ */
    VOS_UINT8                           aucPcscfThiAddr[RNICITF_MAX_IPV6_ADDR_LEN]; /* ����P-CSCF��ַ */
} AT_RNIC_IPV6_PDN_INFO_STRU;


typedef struct
{
    VOS_MSG_HEADER                                                              /* ��Ϣͷ   */      /* _H2ASN_Skip */
    AT_RNIC_MSG_ID_ENUM_UINT32          enMsgId;                                /* ��Ϣ���� */      /* _H2ASN_Skip */

    VOS_UINT32                          bitOpIpv4PdnInfo    : 1;
    VOS_UINT32                          bitOpIpv6PdnInfo    : 1;
    VOS_UINT32                          bitOpSpare          : 30;

    VOS_UINT8                           ucRabId;                                /* ���غ�, ��Χ[5, 15] */
    VOS_UINT8                           ucRmNetId;                              /* ����ID */
    VOS_UINT8                           aucRev[2];

    AT_RNIC_IPV4_PDN_INFO_STRU          stIpv4PdnInfo;                          /* IPv4 PDN������Ϣ */
    AT_RNIC_IPV6_PDN_INFO_STRU          stIpv6PdnInfo;                          /* IPv6 PDN������Ϣ */

} AT_RNIC_PDN_INFO_CFG_IND_STRU;


typedef struct
{
    VOS_MSG_HEADER                                                              /* ��Ϣͷ   */      /* _H2ASN_Skip */
    AT_RNIC_MSG_ID_ENUM_UINT32          enMsgId;                                /* ��Ϣ���� */      /* _H2ASN_Skip */

    VOS_UINT8                           ucRabId;                                /* ��Χ[5, 15] */
    VOS_UINT8                           ucRmNetId;                              /* ����ID */
    VOS_UINT8                           aucRsv[2];

} AT_RNIC_PDN_INFO_REL_IND_STRU;
/* Added by L47619 for V3R3 Share-PDP Project, 2013-6-3, end */

/*****************************************************************************
  8 UNION����
*****************************************************************************/

/*****************************************************************************
  9 OTHERS����
*****************************************************************************/

/*****************************************************************************
  H2ASN������Ϣ�ṹ����
*****************************************************************************/
typedef struct
{
    AT_RNIC_MSG_ID_ENUM_UINT32          enMsgId;            /* _H2ASN_MsgChoice_Export AT_RNIC_MSG_ID_ENUM_UINT32 */
    VOS_UINT8                           aucMsg[4];
    /***************************************************************************
        _H2ASN_MsgChoice_When_Comment          AT_RNIC_MSG_ID_ENUM_UINT32
    ****************************************************************************/
} AT_RNIC_INTERFACE_MSG_DATA;
/* _H2ASN_Length UINT32 */

typedef struct
{
    VOS_MSG_HEADER
    AT_RNIC_INTERFACE_MSG_DATA          stMsgData;
} AtRnicInterface_MSG;

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

#endif /* AtRnicInterface.h */
