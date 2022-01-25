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




/******************************************************************************
   1 ͷ�ļ�����
******************************************************************************/
#include "PsTypeDef.h"
#include "TTFComm.h"
#include "TtfIpComm.h"

/*****************************************************************************
    Э��ջ��ӡ��㷽ʽ�µ�.C�ļ��궨��
*****************************************************************************/
/*lint -e767*/
#define    THIS_FILE_ID        PS_FILE_ID_TTF_IP_COMM_C
/*lint +e767*/


/******************************************************************************
   2 �ⲿ������������
******************************************************************************/

/******************************************************************************
   3 ˽�ж���
******************************************************************************/


/******************************************************************************
   4 ȫ�ֱ�������
******************************************************************************/


/******************************************************************************
   5 ����ʵ��
******************************************************************************/
/*lint -save -e958 */


 VOS_UINT32 TTF_CheckIpDataByProtocalType
(
    VOS_UINT8                   *pucData,
    VOS_UINT16                  usMemUsedLen,
    VOS_UINT16                  usIpHeadLen,
    VOS_UINT16                  usIpTotalLen,
    IP_DATA_PROTOCOL_ENUM_UINT8 enDataProtocalType
)
{
    VOS_UINT16                  usTcpHeadLen;
    VOS_UINT16                  usUdpLen;

    if (usMemUsedLen < (usIpTotalLen) || (usIpTotalLen < usIpHeadLen))
    {
        /* ���ݰ����ȣ�С��IP�ܳ���ֱ���˳� 
		   usIpTotalLen �ֶ��쳣��С��IP��ͷ��ֱ���˳� */
        return PS_FAIL;
    }

    switch (enDataProtocalType)
    {
        case IP_DATA_PROTOCOL_TCP:
            {
                if (usMemUsedLen < (usIpHeadLen+TTF_TCP_HEAD_NORMAL_LEN))
                {
                    /* ���ݰ����ȣ���������������TCPͷ�� */
                    return PS_FAIL;
                }

                usTcpHeadLen = (pucData[usIpHeadLen + TCP_LEN_POS] & TCP_LEN_MASK) >> 2;

                if ( usMemUsedLen < (usTcpHeadLen + usIpHeadLen) )
                {
                    /* ���ݰ����ȣ���������������TCPͷ�� */
                    return PS_FAIL;
                }
            }
            break;

        case IP_DATA_PROTOCOL_UDP:
            {
                if ( usMemUsedLen < (usIpHeadLen + UDP_HEAD_LEN))
                {
                    /* ���ݰ����ȣ���������������UDPͷ */
                    return PS_FAIL;
                }

                /* ��ȡ UDP���ܳ��� */
                usUdpLen  = IP_GET_VAL_NTOH_U16(pucData, (usIpHeadLen+TTF_UDP_LEN_POS));
                if ( usUdpLen < UDP_HEAD_LEN)
                {
                    /* UdpLen �ֶηǷ� */
                    return PS_FAIL;
                }
            }
            break;

        case IP_DATA_PROTOCOL_ICMPV4:
            {
                if (usMemUsedLen < (usIpHeadLen+ICMP_HEADER_LEN))
                {
                    /* ���ݰ����ȣ���������������ICMPV4ͷ */
                    return PS_FAIL;
                }
            }
            break;

        case IP_DATA_PROTOCOL_ICMPV6:
            {
                if (usMemUsedLen < (usIpHeadLen+ICMP_HEADER_LEN))
                {
                    /* ���ݰ����ȣ���������������ICMPV6ͷ */
                    return PS_FAIL;
                }
            }
            break;

        default:
            break;
    }


    return PS_SUCC;
}

IP_DATA_TYPE_ENUM_UINT8 TTF_ParseIpDataType
(
    VOS_UINT32                          ulPid,
    TTF_MEM_ST                         *pMemPt
)
{
    VOS_UINT16                                  usIpHeadLen;
    VOS_UINT16                                  usIpTotalLen;
    VOS_UINT16                                  usTcpHeadLen;
    IP_DATA_TYPE_ENUM_UINT8                     enDataType;
    IP_DATA_PROTOCOL_ENUM_UINT8                 enDataProtocalType;
    VOS_UINT16                                 *pusPort;
    VOS_UINT16                                 *pusFragmentOffset;
    VOS_UINT8                                   usTcpFlags;
    VOS_UINT8                                  *pData       = pMemPt->pData;

    /* ��ʼ������ΪNull */
    enDataType = IP_DATA_TYPE_NULL;

    /* �ڴ�������20�ֽڣ����ܽ���IPͷ��Э���ֶ� ROTOCOL_POS(9), PROTOCOL_POS(6)*/
    if (pMemPt->usUsed <= IPV4_HEAD_NORMAL_LEN)
    {
        TTF_LOG(ulPid, DIAG_MODE_COMM, PS_PRINT_WARNING, "TTF_ParseIpDataType IPHeadLen is exception.");
        return IP_DATA_TYPE_BUTT;
    }

    if ( IPV4_VER_VAL == (pData[0] & IP_VER_MASK) )
    {
        usIpHeadLen         = (pData[0] & IP_HEADER_LEN_MASK) << 2;
        usIpTotalLen        = IP_GET_VAL_NTOH_U16(pData, IP_IPV4_DATA_LEN_POS);
        enDataProtocalType  = pData[PROTOCOL_POS];
    }
    else if( IPV6_VER_VAL == (pData[0] & IP_VER_MASK) )
    {
        usIpHeadLen         = IPV6_HEAD_NORMAL_LEN;
        usIpTotalLen        = IP_GET_VAL_NTOH_U16(pData, IP_IPV6_DATA_LEN_POS) + IPV6_HEAD_NORMAL_LEN;
        enDataProtocalType  = pData[PROTOCOL_POS_V6];
    }
    else
    {
        TTF_LOG(ulPid, DIAG_MODE_COMM, PS_PRINT_WARNING, "TTF_ParseIpDataType Protocol is Null.");
        return IP_DATA_TYPE_BUTT;
    }

    if(TTF_PS_DATA_PRIORITY_HIGH == pMemPt->ucDataPriority)
    {
        TTF_LOG(ulPid, DIAG_MODE_COMM, PS_PRINT_WARNING, "TTF_ParseIpDataType user high priority data.");
        return IP_DATA_TYPE_USER_HIGH;
    }

    /* ��ȫ���: ������ݰ���С�Ƿ��ܹ����ɶ�ӦЭ���ͷ���������ɵ��쳣�����Ͳ��ü���������*/
    if (PS_FAIL == TTF_CheckIpDataByProtocalType(pData, pMemPt->usUsed, usIpHeadLen, usIpTotalLen, enDataProtocalType))
    {
        TTF_LOG2(ulPid, DIAG_MODE_COMM, PS_PRINT_WARNING, "TTF_ParseIpDataType datalen<1> ProtocalType<2> is exception.",pMemPt->usUsed,enDataProtocalType);
        return IP_DATA_TYPE_BUTT;
    }

    switch (enDataProtocalType)
    {
        case IP_DATA_PROTOCOL_TCP:
            {
                enDataType   = IP_DATA_TYPE_TCP;

                usTcpHeadLen = (pData[usIpHeadLen + TCP_LEN_POS] & TCP_LEN_MASK) >> 2;

                /* SDU���ݳ��ȵ���IP��ͷ���Ⱥ�TCP��ͷ������֮�ͣ�����TCP��FLAG��־�к���ACK */
                if ( usIpTotalLen == (usTcpHeadLen + usIpHeadLen) )
                {
                    usTcpFlags = pData[usIpHeadLen + TCP_FLAG_POS] & 0x3F;

                    if (TCP_SYN_MASK == (TCP_SYN_MASK & usTcpFlags))
                    {
                        enDataType = IP_DATA_TYPE_TCP_SYN;
                        break;
                    }

                    if (TCP_ACK_MASK == (TCP_ACK_MASK & usTcpFlags))
                    {
                        enDataType = IP_DATA_TYPE_TCP_ACK;
                    }
                }
                else
                {
                    pusPort = (VOS_UINT16 *)&pData[usIpHeadLen + TCP_DST_PORT_POS];
                    if (FTP_DEF_SERVER_SIGNALLING_PORT == ntohs(*pusPort))
                    {
                        enDataType = IP_DATA_TYPE_FTP_SIGNALLING;
                    }
                }
            }
            break;

        case IP_DATA_PROTOCOL_UDP:
            {
                enDataType = IP_DATA_TYPE_UDP;

                pusPort = (VOS_UINT16 *)&pData[usIpHeadLen + UDP_DST_PORT_POS];

                if (DNS_DEF_SERVER_PORT == ntohs(*pusPort))
                {
                    enDataType = IP_DATA_TYPE_UDP_DNS;
                }
            }
            break;

        case IP_DATA_PROTOCOL_ICMPV4:
            {
                pusFragmentOffset = (VOS_UINT16 *)&pData[IPV4_HEAD_FRAGMENT_OFFSET_POS];

                /* �ֶ� */
                if (ntohs(*pusFragmentOffset) & IPV4_HEAD_FRAGMENT_OFFSET_MASK)
                {
                    break;
                }

                /* ��ȡICMP���ĵ����� */
                if ((ICMP_TYPE_REQUEST == pData[usIpHeadLen]) || (ICMP_TYPE_REPLY == pData[usIpHeadLen]))
                {
                    enDataType = IP_DATA_TYPE_ICMP;
                }
            }
            break;

        case IP_DATA_PROTOCOL_ICMPV6:
            {
                /* ��ȡICMPV6���ĵ����� */
                if ((ICMPV6_TYPE_REQUEST == pData[usIpHeadLen]) || (ICMPV6_TYPE_REPLY == pData[usIpHeadLen]))
                {
                    enDataType = IP_DATA_TYPE_ICMP;
                }
            }
            break;

        case IP_DATA_PROTOCOL_IPV6_FRAGMENT:
            break;

        default:
            break;
    }

    return enDataType;
} /* TTF_ParseIpDataType */


VOS_UINT16 TTF_GetIpDataTraceLen
(
    VOS_UINT32                          ulPid,
    VOS_UINT8                          *pData,
    VOS_UINT16                          usSduLen
)
{
    VOS_UINT16                                  usIpHeadLen;
    VOS_UINT16                                  usIpTotalLen;
    VOS_UINT16                                  usTcpHeadLen;
    IP_DATA_PROTOCOL_ENUM_UINT8                 enDataProtocalType;
    VOS_UINT16                                 *pusSourcePort;
    VOS_UINT16                                 *pusDestPort;
    VOS_UINT16                                  usIpDataTraceLen;

    /* �ڴ�������20�ֽڣ����ܽ���IPͷ��Э���ֶ� ROTOCOL_POS(9), PROTOCOL_POS(6)*/
    if (usSduLen <= IPV4_HEAD_NORMAL_LEN)
    {
        TTF_LOG(ulPid, DIAG_MODE_COMM, PS_PRINT_WARNING, "TTF_ParseIpDataType IPHeadLen is exception.");
        return 0;
    }

    if ( IPV4_VER_VAL == (pData[0] & IP_VER_MASK) )
    {
        usIpHeadLen         = (pData[0] & IP_HEADER_LEN_MASK) << 2;
        usIpTotalLen        = IP_GET_VAL_NTOH_U16(pData, IP_IPV4_DATA_LEN_POS);
        enDataProtocalType  = pData[PROTOCOL_POS];
    }
    else if( IPV6_VER_VAL == (pData[0] & IP_VER_MASK) )
    {
        usIpHeadLen         = IPV6_HEAD_NORMAL_LEN;
        usIpTotalLen        = IP_GET_VAL_NTOH_U16(pData, IP_IPV6_DATA_LEN_POS) + IPV6_HEAD_NORMAL_LEN;
        enDataProtocalType  = pData[PROTOCOL_POS_V6];
    }
    else
    {
        TTF_LOG(ulPid, DIAG_MODE_COMM, PS_PRINT_WARNING, "TTF_GetIpDataTraceLen Protocol is Null.");
        return 0;
    }

    /* ��ȫ���: ������ݰ���С�Ƿ��ܹ����ɶ�ӦЭ���ͷ���������ɵ��쳣�����Ͳ��ü���������*/
    if (PS_FAIL == TTF_CheckIpDataByProtocalType(pData, usSduLen, usIpHeadLen, usIpTotalLen, enDataProtocalType))
    {
        TTF_LOG2(ulPid, DIAG_MODE_COMM, PS_PRINT_WARNING, "TTF_GetIpDataTraceLen datalen<1> ProtocalType<2> is exception.",usSduLen,enDataProtocalType);
        return 0;
    }

    usIpDataTraceLen = usIpHeadLen;

    switch (enDataProtocalType)
    {
        case IP_DATA_PROTOCOL_TCP:
            {
                usTcpHeadLen    = (pData[usIpHeadLen + TCP_LEN_POS] & TCP_LEN_MASK) >> 2;

                /* SDU���ݳ��ȵ���IP��ͷ���Ⱥ�TCP��ͷ������֮�ͣ�����TCP��FLAG��־�к���ACK */
                if ( usIpTotalLen == (usTcpHeadLen + usIpHeadLen) )
                {
                    usIpDataTraceLen = usIpTotalLen;
                }
                else
                {
                    pusSourcePort   = (VOS_UINT16 *)&pData[usIpHeadLen];
                    pusDestPort     = (VOS_UINT16 *)&pData[usIpHeadLen + TCP_DST_PORT_POS];

                    /* FTP����ȫ����ȡ��������TCPͷ */
                    if ((FTP_DEF_SERVER_SIGNALLING_PORT == ntohs(*pusSourcePort)) || (FTP_DEF_SERVER_SIGNALLING_PORT == ntohs(*pusDestPort)))
                    {
                        usIpDataTraceLen = usIpTotalLen;
                    }
                    else
                    {
                        usIpDataTraceLen = usIpHeadLen + usTcpHeadLen;
                    }
                }
            }
            break;

        case IP_DATA_PROTOCOL_UDP:
            {
                pusSourcePort   = (VOS_UINT16 *)&pData[usIpHeadLen];
                pusDestPort     = (VOS_UINT16 *)&pData[usIpHeadLen + UDP_DST_PORT_POS];

                /* DNSȫ����ȡ��������UDPͷ */
                if ((DNS_DEF_SERVER_PORT == ntohs(*pusSourcePort)) || (DNS_DEF_SERVER_PORT == ntohs(*pusDestPort)))
                {
                    usIpDataTraceLen = usIpTotalLen;
                }
                else
                {
                    usIpDataTraceLen = usIpHeadLen + UDP_HEAD_LEN;
                }
            }
            break;

        case IP_DATA_PROTOCOL_ICMPV4:
            {
                usIpDataTraceLen = usIpHeadLen + ICMP_HEADER_LEN;
            }
            break;

        case IP_DATA_PROTOCOL_ICMPV6:
            {
                usIpDataTraceLen = usIpHeadLen + ICMP_HEADER_LEN;
            }
            break;

        default:
            break;
    }

    return usIpDataTraceLen;
}


/*lint -restore */



