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

#ifndef _ATTAFAGENTINTERFACE_H_
#define _ATTAFAGENTINTERFACE_H_

/*****************************************************************************
  1 ͷ�ļ�����
*****************************************************************************/
#include  "vos.h"
#include  "TafAgentInterface.h"

#pragma pack(4)

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  2 ȫ�ֱ�������
*****************************************************************************/



/*****************************************************************************
  3 ����ʵ��
*****************************************************************************/

/*****************************************************************************
 �� �� ��  : TAF_AGENT_FindCidForDial
 ��������  : ͨ��ͬ�����ƻ�ȡ���ڲ��ŵ�CID
 �������  : pCid    - CID
 �������  : ��
 �� �� ֵ  : VOS_OK  - �ɹ�
             VOS_ERR - ʧ��
 ���ú���  :
 ��������  :
*****************************************************************************/
VOS_UINT32 TAF_AGENT_FindCidForDial(
    MN_CLIENT_ID_T                      usClientId,
    VOS_UINT8                          *pCid
);


/*****************************************************************************
 �� �� ��  : TAF_AGENT_SetPdpCidPara
 ��������  : ����ָ��CID�Ĳ���
 �������  : usClientId    - �ͻ���ID
             pstPdpPriPara - PDP�����Ĳ���
 �������  : ��
 �� �� ֵ  : VOS_OK        - �ɹ�
             VOS_ERR       - ʧ��
 ���ú���  :
 ��������  :
*****************************************************************************/
VOS_UINT32 TAF_AGENT_SetPdpCidPara(
    MN_CLIENT_ID_T                      usClientId,
    TAF_PDP_PRIM_CONTEXT_EXT_STRU      *pstPdpPrimContextExt
);


/*****************************************************************************
 �� �� ��  : TAF_AGENT_GetPdpCidPara
 ��������  : ��ѯָ��CID�Ĳ���
 �������  : pstPdpPriPara - PDP�����Ĳ���
             ucCid         - CID
 �� �� ֵ  : VOS_OK        - �ɹ�
             VOS_ERR       - ʧ��
 ���ú���  :
 ��������  :
*****************************************************************************/
VOS_UINT32 TAF_AGENT_GetPdpCidPara(
    TAF_PDP_PRIM_CONTEXT_STRU          *pstPdpPriPara,
    MN_CLIENT_ID_T                      usClientId,
    VOS_UINT8                           ucCid
);


/*****************************************************************************
 �� �� ��  : TAF_AGENT_GetCallInfo
 ��������  : ��ȡͨ����Ϣͬ��API
 �������  : usClientId
             pucNumOfCalls
             pstCallInfos
 �������  : ��
 �� �� ֵ  : VOS_OK  - �ɹ�
             VOS_ERR - ʧ��
 ���ú���  :
 ��������  :
*****************************************************************************/
VOS_UINT32 TAF_AGENT_GetCallInfoReq(
    MN_CLIENT_ID_T                      usClientId,
    VOS_UINT8                          *pucCallNum,
    TAFAGERNT_MN_CALL_INFO_STRU        *pstCallInfos
);


VOS_UINT32 TAF_AGENT_GetSysMode(
    VOS_UINT16                                 usClientId,
    TAF_AGENT_SYS_MODE_STRU                   *pstSysMode
);

extern VOS_VOID TAF_AGENT_ClearAllSem(VOS_VOID);

VOS_UINT32 TAF_AGENT_SetPdpCidQosPara(
    MN_CLIENT_ID_T                      usClientId,
    TAF_PS_PDP_QOS_SET_PARA_STRU       *pstQosPara
);

VOS_UINT32 TAF_AGENT_GetPdpCidQosPara(
    MN_CLIENT_ID_T                      usClientId,
    VOS_UINT8                           ucCid,
    TAF_PS_PDP_QOS_QUERY_PARA_STRU     *pstQosPara
);

VOS_UINT32 TAF_AGENT_GetAntState(
    VOS_UINT16                                 usClientId,
    VOS_UINT16                                *pusAntState
);


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

