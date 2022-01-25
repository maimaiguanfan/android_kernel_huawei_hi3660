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

#ifndef __ATCMDPACKETPROC_H__
#define __ATCMDPACKETPROC_H__

/*****************************************************************************
  1 ����ͷ�ļ�����
*****************************************************************************/
#include "AtCtx.h"
#include "AtParse.h"
#include "AtRnicInterface.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


#pragma pack(4)

/*****************************************************************************
  2 �궨��
*****************************************************************************/


/*****************************************************************************
  3 ö�ٶ���
*****************************************************************************/


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


/*****************************************************************************
  8 UNION����
*****************************************************************************/


/*****************************************************************************
  9 OTHERS����
*****************************************************************************/


/*****************************************************************************
  10 ��������
*****************************************************************************/
VOS_UINT32 AT_SetChdataPara(VOS_UINT8 ucIndex);
VOS_UINT32 AT_QryChdataPara(VOS_UINT8 ucIndex);

VOS_UINT32 At_SetDhcpPara(VOS_UINT8 ucIndex);
VOS_UINT32 At_QryDhcpPara_AppUser(VOS_UINT8 ucIndex);
VOS_UINT32 At_QryDhcpPara_NdisUser(VOS_UINT8 ucIndex);
VOS_UINT32 At_QryDhcpPara_UsbComUser(VOS_UINT8 ucIndex);
VOS_UINT32 At_QryDhcpPara(TAF_UINT8 ucIndex);
VOS_UINT32 AT_TestDhcpPara(VOS_UINT8 ucIndex);

#if (FEATURE_ON == FEATURE_IPV6)
VOS_UINT32 AT_SetDhcpv6Para(VOS_UINT8 ucIndex);
VOS_UINT32 AT_QryDhcpV6Para_AppUser(VOS_UINT8 ucIndex);
VOS_UINT32 AT_QryDhcpV6Para_NdisUser(VOS_UINT8 ucIndex);
VOS_UINT32 At_QryDhcpV6Para_UsbComUser(TAF_UINT8 ucIndex);
VOS_UINT32 AT_QryDhcpv6Para(VOS_UINT8 ucIndex);
VOS_UINT32 AT_TestDhcpv6Para(VOS_UINT8 ucIndex);

VOS_UINT32 AT_SetApRaInfoPara(VOS_UINT8 ucIndex);
VOS_UINT32 AT_QryApRaInfoPara(VOS_UINT8 ucIndex);
VOS_UINT32 AT_TestApRaInfoPara(VOS_UINT8 ucIndex);

VOS_UINT32 AT_SetApLanAddrPara(VOS_UINT8 ucIndex);
VOS_UINT32 AT_QryApLanAddrPara(VOS_UINT8 ucIndex);
VOS_UINT32 AT_TestApLanAddrPara(VOS_UINT8 ucIndex);

#endif

VOS_UINT32  At_SetApConnStPara(
    VOS_UINT8                           ucIndex
);
VOS_UINT32  At_QryApConnStPara(
    VOS_UINT8                           ucIndex
);
VOS_UINT32 AT_TestApConnStPara(VOS_UINT8 ucIndex);

VOS_UINT32 At_SetApThroughputPara(
    VOS_UINT8                           ucIndex
);
VOS_UINT32 At_QryApThroughputPara(
    VOS_UINT8                           ucIndex
);
VOS_UINT32 AT_TestApThroughputPara(VOS_UINT8 ucIndex);

VOS_UINT32 AT_SetApEndPppPara(VOS_UINT8 ucIndex);

VOS_UINT32 AT_SetApDsFlowRptCfgPara(VOS_UINT8 ucIndex);
VOS_UINT32 AT_QryApDsFlowRptCfgPara(VOS_UINT8 ucIndex);

VOS_UINT32 AT_SetDsFlowNvWriteCfgPara(VOS_UINT8 ucIndex);
VOS_UINT32 AT_QryDsFlowNvWriteCfgPara(VOS_UINT8 ucIndex);

VOS_UINT32 AT_SetImsPdpCfg(VOS_UINT8 ucIndex);

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

#endif /* end of AtCmdPacketProc.h */
