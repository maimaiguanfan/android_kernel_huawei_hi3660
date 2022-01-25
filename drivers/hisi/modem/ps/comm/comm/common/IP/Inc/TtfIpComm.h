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

#ifndef __TTFIPCOMM_H__
#define __TTFIPCOMM_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 ����ͷ�ļ�����
*****************************************************************************/
#include "vos.h"
#include "TTFComm.h"
#include "TtfDrvInterface.h"

#pragma pack(4)

/*****************************************************************************
  2 ö�ٶ���
*****************************************************************************/

/*****************************************************************************
 ö����    : IP_HEAD_PROTOCOL_ENUM
 Э����  :
 ASN.1���� : IP��Э�����
 ö��˵��  : IPͷ����Э����������ĸ�ʽ
*****************************************************************************/
enum IP_DATA_PROTOCOL_ENUM
{
    IP_DATA_PROTOCOL_ICMPV4                 = 0x01,         /* ICMPV4Э���ʶ��ֵ */
    IP_DATA_PROTOCOL_IGMP                   = 0x02,         /* IGMPЭ���ʶ��ֵ */
    IP_DATA_PROTOCOL_IPINIP                 = 0x04,         /* IP in IP (encapsulation)Э���ʶ��ֵ */
    IP_DATA_PROTOCOL_TCP                    = 0x06,         /* TCPЭ���ʶ��ֵ */
    IP_DATA_PROTOCOL_UDP                    = 0x11,         /* UDPЭ���ʶ��ֵ */
    IP_DATA_PROTOCOL_IPV6_ROUTE             = 0x2b,         /* Routing Header for IPv6Э���ʶ��ֵ */
    IP_DATA_PROTOCOL_IPV6_FRAGMENT          = 0x2c,         /* Fragment Header for IPv6Э���ʶ��ֵ */
    IP_DATA_PROTOCOL_ESP                    = 0x32,         /* Encapsulating Security PayloadЭ���ʶ��ֵ */
    IP_DATA_PROTOCOL_AH                     = 0x33,         /* Authentication HeaderЭ���ʶ��ֵ */
    IP_DATA_PROTOCOL_ICMPV6                 = 0x3a,         /* ICMPV6Э���ʶ��ֵ */
    IP_DATA_PROTOCOL_IPV6_NONEXT            = 0x3b,         /* No Next Header for IPv6Э���ʶ��ֵ */
    IP_DATA_PROTOCOL_IPV6_OPTS              = 0x3c,         /* Destination Options for IPv6Э���ʶ��ֵ */
    IP_DATA_PROTOCOL_IPV6_IPIP              = 0x5e,         /* IP-within-IP Encapsulation ProtocolЭ���ʶ��ֵ */
    IP_DATA_PROTOCOL_IPCOMP                 = 0x6c,         /* IP Payload Compression ProtocolЭ���ʶ��ֵ */

    IP_DATA_PROTOCOL_BUTT
};
typedef VOS_UINT8 IP_DATA_PROTOCOL_ENUM_UINT8;

/* IP���ݳ���Э�� */
/*****************************************************************************
 ö����    : IP_DATA_PROTOCOL_TYPE_ENUM
 Э����  :
 ASN.1���� : ���ݳ���Э��
 ö��˵��  : TTF�ڲ�ʵ��ö�ٵ���������
*****************************************************************************/
enum IP_DATA_TYPE_ENUM
{
    IP_DATA_TYPE_NULL                   = 0,    /* NULL */
    IP_DATA_TYPE_ICMP                   = 1,    /* ICMP */
    IP_DATA_TYPE_IGMP                   = 2,    /* IGMP */
    IP_DATA_TYPE_TCP                    = 3,    /* TCP */
    IP_DATA_TYPE_TCP_SYN                = 4,    /* TCP ACK */
    IP_DATA_TYPE_TCP_ACK                = 5,    /* TCP ACK */
    IP_DATA_TYPE_FTP_SIGNALLING         = 6,    /* FTP SIGNALLING */
    IP_DATA_TYPE_UDP                    = 7,    /* UDP */
    IP_DATA_TYPE_UDP_DNS                = 8,    /* UDP DNS */
    IP_DATA_TYPE_USER_HIGH              = 9,    /* user high data */

    IP_DATA_TYPE_BUTT
};
typedef VOS_UINT8 IP_DATA_TYPE_ENUM_UINT8;

/*****************************************************************************
  3 �궨��
*****************************************************************************/
#ifndef ntohs
#define ntohs(x)                            ((((VOS_UINT16)(x) & 0x00ff) << 8) | (((VOS_UINT16)(x) & 0xff00) >> 8))
#endif

#ifndef ntohl
#define ntohl(x)                            ((((VOS_UINT32)(x) & 0x000000ff) << 24) | (((VOS_UINT32)(x) & 0x0000ff00) <<  8) | \
                                             (((VOS_UINT32)(x) & 0x00ff0000) >>  8) | (((VOS_UINT32)(x) & 0xff000000) >> 24))
#endif


#define TTF_IP_MAX_OPT                  (40)
#define TTF_TCP_MAX_OPT                 (40)
#define TTF_TCP_HEAD_NORMAL_LEN         (20)
#define TTF_UDP_LEN_POS                 (4)
#define TTF_IP_FRAG_POS_IN_HEAD         (0x3)

#define TTF_IP_HEAD_TO_REAL_LEN         (0x2)
#define TTF_IP_FRAG_TO_REAL_LEN         (0x3)

/* IP flags. */
#define TTF_IP_CE                       (0x8000)  /* Flag: "Congestion"       */
#define TTF_IP_DF                       (0x4000)  /* Flag: "Don't Fragment"   */
#define TTF_IP_MF                       (0x2000)  /* Flag: "More Fragments"   */
#define TTF_IP_OFFSET                   (0x1FFF)  /* "Fragment Offset" part   */


#define TTF_TCP_HEAD_TO_REAL_LEN        (0x2)

#define TTF_IP_CONGEST_MASK             (0x8000)
#define TTF_IP_DF_MASK                  (0x4000)
#define TTF_IP_MF_MASK                  (0x2000)
#define TTF_IP_FRAGMENT_OFFSET_MASK     (0x1FFF)

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
    VOS_UINT8   ihl:4,
                ver:4;
    VOS_UINT8   tos;
    VOS_UINT16  tot_len;
    VOS_UINT16  id;
    VOS_UINT16  frag_off;
    VOS_UINT8   ttl;
    VOS_UINT8   protocol;
    VOS_UINT16  csum;
    VOS_UINT32  saddr;
    VOS_UINT32  daddr;
    /*The options start here. */
}TTF_IP_HDR;

typedef struct
{
    VOS_UINT16  sPort;
    VOS_UINT16  dPort;
    VOS_UINT32  seq;
    VOS_UINT32  ack_seq;
    VOS_UINT16  doff:4,
                res:4,
                cwr:1,
                ece:1,
                urg:1,
                ack:1,
                psh:1,
                rst:1,
                syn:1,
                fin:1;
    VOS_UINT16  windowsize;
    VOS_UINT16  csum;
    VOS_UINT16  urg_ptr;
}TTF_TCP_HDR;

typedef struct
{
    VOS_UINT16  sPort;
    VOS_UINT16  dPort;
    VOS_UINT16  len;
    VOS_UINT16  csum;
}TTF_UDP_HDR;

typedef struct
{
    IP_DATA_TYPE_ENUM_UINT8                 enParseDataType;
    VOS_UINT8                               ucIpVer;            /* IP Э��汾 */
    IP_DATA_PROTOCOL_ENUM_UINT8             enProto;            /* IPv4 ProtoЭ�����ͣ�IPv6ΪNext Header */
    VOS_UINT8                               ucFragmentFlag;
    VOS_UINT8                               aucRsv[4];

    VOS_UINT32                              ulSrcIpAddr;        /* IPv4 ԴIP ADDR */
    VOS_UINT32                              ulDstIpAddr;        /* IPv4 Ŀ��IP ADDR */

    VOS_UINT16                              usSrcPort;          /* IP Դ�˿ں� */
    VOS_UINT16                              usDstPort;          /* IP Ŀ�Ķ˿ں� */
    VOS_UINT8                               ucIcmpType;         /* ICMP�������� */
    VOS_UINT8                               ucIcmpCode;         /* ICMP����CODE */
    VOS_UINT16                              usIdentify;         /* IPv4 IDENTIFY�ֶ� */

    VOS_UINT32                              ulTcpSeq;           /* TCP SEQ */
    VOS_UINT32                              ulTcpAckSeq;        /* TCP ACK SEQ */
}TTF_IP_DATA_PARAM_STRU;

/*****************************************************************************
  8 UNION����
*****************************************************************************/


/*****************************************************************************
  9 OTHERS����
*****************************************************************************/


/*****************************************************************************
  10 ��������
*****************************************************************************/
IP_DATA_TYPE_ENUM_UINT8 TTF_ParseIpDataType(VOS_UINT32 ulPid, TTF_MEM_ST *pMemPt);
VOS_UINT16 TTF_GetIpDataTraceLen
(
    VOS_UINT32                          ulPid,
    VOS_UINT8                          *pData,
    VOS_UINT16                          usSduLen
);

#pragma pack()


#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of TtfIpComm.h */

