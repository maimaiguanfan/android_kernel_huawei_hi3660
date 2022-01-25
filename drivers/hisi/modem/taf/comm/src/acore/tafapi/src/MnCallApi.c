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
#include "vos.h"
#include "PsCommonDef.h"
#include "AtMnInterface.h"
#include "MnCallApi.h"
#include  "product_config.h"
#include "MnErrorCode.h"
#include "AtParse.h"
#include "ATCmdProc.h"

#include "TafAppCall.h"


#define    THIS_FILE_ID        PS_FILE_ID_MNCALL_API_C

/*****************************************************************************
   2 ����ʵ��
*****************************************************************************/


VOS_UINT32  MN_CALL_SendAppRequest(
    MN_CALL_APP_REQ_ENUM_UINT32         enReq,
    MN_CLIENT_ID_T                      clientId,
    MN_OPERATION_ID_T                   opId,
    MN_CALL_ID_T                        callId,
    const MN_CALL_APP_REQ_PARM_UNION   *punParam
)
{
    MN_CALL_APP_REQ_MSG_STRU *pstMsg =
        (MN_CALL_APP_REQ_MSG_STRU *)PS_ALLOC_MSG_WITH_HEADER_LEN(WUEPS_PID_AT,
                                              sizeof(MN_CALL_APP_REQ_MSG_STRU));
    if (VOS_NULL_PTR == pstMsg)
    {
        AT_ERR_LOG("MN_CALL_SendAppRequest: Failed to alloc VOS message.");
        return VOS_ERR;
    }

    TAF_MEM_SET_S((VOS_INT8*)pstMsg + VOS_MSG_HEAD_LENGTH,
               (VOS_SIZE_T)(sizeof(MN_CALL_APP_REQ_MSG_STRU) - VOS_MSG_HEAD_LENGTH),
                0x00,
               (VOS_SIZE_T)(sizeof(MN_CALL_APP_REQ_MSG_STRU) - VOS_MSG_HEAD_LENGTH));

    /* ��дVOS��Ϣͷ */
    pstMsg->ulSenderCpuId               = VOS_LOCAL_CPUID;
    pstMsg->ulSenderPid                 = WUEPS_PID_AT;
    pstMsg->ulReceiverCpuId             = VOS_LOCAL_CPUID;
    pstMsg->ulReceiverPid               = AT_GetDestPid(clientId, I0_WUEPS_PID_TAF);

    /* ��дԭ���ײ� */
    pstMsg->enReq = enReq;
    pstMsg->clientId = clientId;
    pstMsg->opId = opId;
    pstMsg->callId = callId;

    if (TAF_NULL_PTR != punParam)
    {
        TAF_MEM_CPY_S(&pstMsg->unParm, sizeof(pstMsg->unParm), punParam, sizeof(pstMsg->unParm));
    }

    /* ����VOS��Ϣ */
    if (VOS_OK != PS_SEND_MSG(WUEPS_PID_AT, pstMsg))
    {
        AT_ERR_LOG1("MN_CALL_SendAppRequest: Send Message Fail. reqtype:", (VOS_INT32)enReq);
        return VOS_ERR;
    }

    return VOS_OK;
}



VOS_UINT32  MN_CALL_Orig(
    MN_CLIENT_ID_T                      clientId,
    MN_OPERATION_ID_T                   opId,
    MN_CALL_ID_T                       *pCallId,
    const MN_CALL_ORIG_PARAM_STRU      *pstOrigParam
)
{
    VOS_UINT32                          ulResult;
    MN_CALL_ID_T                        callId;
    MN_CALL_APP_REQ_PARM_UNION          stAppReq;

    /* �ڸô����ڷ���CallId��ֱ�ӽ�callId��ֵΪ0
       CallId�ķ���ŵ�MN CALLģ�鴦�������ĺ����� */
    callId = 0;

    TAF_MEM_SET_S(&stAppReq, (VOS_UINT32)sizeof(stAppReq), 0x00, (VOS_UINT32)sizeof(stAppReq));
    TAF_MEM_CPY_S(&(stAppReq.stOrig), (VOS_UINT32)sizeof(stAppReq.stOrig), pstOrigParam, (VOS_UINT32)sizeof(MN_CALL_ORIG_PARAM_STRU));

    /* �����첽Ӧ������ */
    ulResult = MN_CALL_SendAppRequest(MN_CALL_APP_ORIG_REQ, clientId,
                                      opId, callId,
                                      &stAppReq);

    *pCallId = callId;

    return ulResult;

}



VOS_UINT32  MN_CALL_End(
    MN_CLIENT_ID_T                      clientId,
    MN_OPERATION_ID_T                   opId,
    MN_CALL_ID_T                        callId,
    const MN_CALL_END_PARAM_STRU       *pstEndParam
)
{
    MN_CALL_APP_REQ_PARM_UNION          stAppReq;
    VOS_UINT32                          ulResult;

    TAF_MEM_SET_S(&stAppReq, (VOS_UINT32)sizeof(stAppReq), 0x00, (VOS_UINT32)sizeof(stAppReq));

    if ( TAF_NULL_PTR == pstEndParam)
    {
        /* ���ع���һ��MN_CALL_END_REQ_PARAM_STRU�ṹ, ��дԭ��ֵΪ255 */
        stAppReq.stEnd.enEndCause = MN_CALL_INTERWORKING_UNSPECIFIED;
    }
    else
    {
        stAppReq.stEnd.enEndCause = pstEndParam->enEndCause;
    }

    /* �����첽Ӧ������ */
    ulResult = MN_CALL_SendAppRequest(MN_CALL_APP_END_REQ, clientId,
                                      opId, callId,
                                      &stAppReq);

    return ulResult;
}

/* Added by f62575 for AT Project, 2011-10-04,  Begin */

VOS_UINT32  MN_CALL_QryCdur(
    MN_CLIENT_ID_T                      clientId,
    MN_OPERATION_ID_T                   opId,
    MN_CALL_ID_T                        callId
)
{
    VOS_UINT32                          ulResult;

    /* �����첽Ӧ������ */
    ulResult = MN_CALL_SendAppRequest(MN_CALL_APP_GET_CDUR_REQ, clientId,
                                      opId, callId,
                                      VOS_NULL_PTR);

    return ulResult;
}
/* Added by f62575 for AT Project, 2011-10-04,  End */


VOS_UINT32  TAF_CALL_SendDtmf(
    AT_MN_MSGTYPE_ENUM_UINT16           enMsgType,
    MN_CLIENT_ID_T                      clientId,
    MN_OPERATION_ID_T                   opId,
    const TAF_CALL_DTMF_PARAM_STRU     *pstDtmfParam
)
{
    VOS_UINT32                          ulResult;
    MN_CALL_APP_REQ_PARM_UNION          stAppPara;

    /* ��ʼ���ֲ����� */
    TAF_MEM_SET_S(&stAppPara, sizeof(stAppPara), 0x00, sizeof(stAppPara));
    TAF_MEM_CPY_S(&stAppPara.stDtmf, sizeof(stAppPara.stDtmf), pstDtmfParam, sizeof(TAF_CALL_DTMF_PARAM_STRU));

    /* �����첽Ӧ������ */
    ulResult = MN_CALL_SendAppRequest(enMsgType, clientId, opId,
                                      pstDtmfParam->CallId,
                                      &stAppPara);

    return ulResult;
}


VOS_UINT32  MN_CALL_Sups(
    MN_CLIENT_ID_T                      clientId,
    MN_OPERATION_ID_T                   opId,
    const MN_CALL_SUPS_PARAM_STRU       *pstCallSupsParam
)
{
    VOS_UINT32                          ulResult;
    MN_CALL_APP_REQ_PARM_UNION          stAppPara;

    TAF_MEM_SET_S(&stAppPara, (VOS_UINT32)sizeof(stAppPara), 0x00, (VOS_UINT32)sizeof(stAppPara));
    TAF_MEM_CPY_S(&stAppPara.stCallMgmtCmd, (VOS_UINT32)sizeof(stAppPara.stCallMgmtCmd), pstCallSupsParam, (VOS_UINT32)sizeof(MN_CALL_SUPS_PARAM_STRU));

    /* �����첽Ӧ������ */
    /* ��������CallId���һ�� */
    ulResult = MN_CALL_SendAppRequest(MN_CALL_APP_SUPS_CMD_REQ, clientId,
                                      opId, pstCallSupsParam->callId,
                                      &stAppPara);

    return ulResult;
}



VOS_UINT32  MN_CALL_GetCallInfos(
    MN_CLIENT_ID_T                      clientId,
    MN_OPERATION_ID_T                   opId,
    MN_CALL_ID_T                        callId
)
{
    VOS_UINT32                          ulResult;

    /* �����첽Ӧ������ */
    ulResult = MN_CALL_SendAppRequest(MN_CALL_APP_GET_INFO_REQ, clientId,
                                      opId, callId,
                                      VOS_NULL_PTR);

    return ulResult;
}


VOS_UINT32 MN_CALL_SetAlsLineNo(
    TAF_UINT8                           ucIndex,
    MN_CALL_ALS_LINE_NO_ENUM_U8         enAlsLine
)
{
    VOS_UINT32                          ulRst;
    MN_CALL_APP_REQ_PARM_UNION          stAppReq;

    stAppReq.stSetAls.enAlsLine = enAlsLine;

    /*1.ͨ��TAF_MSG_ALS_LINE_NO_SET��Ϣ�������ṹMN_CALL_ALS_PARAM_STRU
        ֪ͨTAF��ALS�������á�*/
    ulRst = MN_CALL_SendAppRequest(MN_CALL_APP_SET_ALS_REQ,
                                   gastAtClientTab[ucIndex].usClientId,
                                   At_GetOpId(),
                                   0,
                                   &stAppReq);
    return ulRst;
}


VOS_UINT32 MN_CALL_CheckUus1ParmValid(
    MN_CALL_SET_UUS1_TYPE_ENUM_U32      enSetType,
    MN_CALL_UUS1_INFO_STRU              *pstUus1Info
)
{
    if ( VOS_NULL_PTR == pstUus1Info )
    {
        return MN_ERR_INVALIDPARM;
    }

    /*  У������ĺϷ���,�Ƿ�ֱ�ӷ��� */
    if ( ( enSetType >= MN_CALL_SET_UUS1_BUTT )
      || ( pstUus1Info->enMsgType > MN_CALL_UUS1_MSG_RELEASE_COMPLETE ))
    {
        return MN_ERR_INVALIDPARM;
    }


    /* ����UUIE�ļ�������һ���Ƿ���UUIE,�����ĳ��Ⱥ�PD�����м��,
       ��Ӧ�ñ�֤,������ڼ���UUS1ʱ��Ҫ���,ȥ������ĸ���  */
    if ( ( MN_CALL_SET_UUS1_ACT == enSetType)
      && ( MN_CALL_UUS_IEI != pstUus1Info->aucUuie[MN_CALL_IEI_POS]))
    {
        return MN_ERR_INVALIDPARM;
    }

    return MN_ERR_NO_ERROR;
}


VOS_UINT32  TAF_XCALL_SendFlashReq(
    MN_CLIENT_ID_T                      clientId,
    MN_OPERATION_ID_T                   opId,
    TAF_CALL_FLASH_PARA_STRU           *pstFlashPara
)
{
    TAF_CALL_APP_SEND_FLASH_REQ_STRU *pstMsg =
        (TAF_CALL_APP_SEND_FLASH_REQ_STRU *)PS_ALLOC_MSG_WITH_HEADER_LEN(WUEPS_PID_AT,
                                              sizeof(TAF_CALL_APP_SEND_FLASH_REQ_STRU));
    if (VOS_NULL_PTR == pstMsg)
    {
        AT_ERR_LOG("TAF_XCALL_SendFlashReq: Failed to alloc VOS message.");
        return VOS_ERR;
    }

    TAF_MEM_SET_S((VOS_INT8*)pstMsg + VOS_MSG_HEAD_LENGTH,
               (VOS_SIZE_T)(sizeof(TAF_CALL_APP_SEND_FLASH_REQ_STRU) - VOS_MSG_HEAD_LENGTH),
                0x00,
               (VOS_SIZE_T)(sizeof(TAF_CALL_APP_SEND_FLASH_REQ_STRU) - VOS_MSG_HEAD_LENGTH));

    /* ��дVOS��Ϣͷ */
    pstMsg->ulSenderCpuId               = VOS_LOCAL_CPUID;
    pstMsg->ulSenderPid                 = WUEPS_PID_AT;
    pstMsg->ulReceiverCpuId             = VOS_LOCAL_CPUID;
    pstMsg->ulReceiverPid               = AT_GetDestPid(clientId, I0_WUEPS_PID_TAF);

    /* ��д��Ϣ���� */
    pstMsg->usMsgId    = TAF_CALL_APP_SEND_FLASH_REQ;
    pstMsg->usClientId = clientId;
    pstMsg->ucOpId     = opId;
    TAF_MEM_CPY_S(&(pstMsg->stFlashPara), sizeof(pstMsg->stFlashPara), pstFlashPara, sizeof(TAF_CALL_FLASH_PARA_STRU));

    /* ����VOS��Ϣ */
    if (VOS_OK != PS_SEND_MSG(WUEPS_PID_AT, pstMsg))
    {
        AT_ERR_LOG("TAF_XCALL_SendFlashReq: Send TAF_CALL_APP_SEND_FLASH_REQ Message Fail");
        return VOS_ERR;
    }

    return VOS_OK;
}

/* Added by f279542 for CDMA 1X Iteration 4, 2014-11-10, begin */

VOS_UINT32  TAF_XCALL_SendBurstDtmf(
    MN_CLIENT_ID_T                      clientId,
    MN_OPERATION_ID_T                   opId,
    TAF_CALL_BURST_DTMF_PARA_STRU      *pstSndBurstDTMFPara
)
{
    TAF_CALL_BURST_DTMF_REQ_MSG_STRU *pstMsg =
        (TAF_CALL_BURST_DTMF_REQ_MSG_STRU *)PS_ALLOC_MSG_WITH_HEADER_LEN(WUEPS_PID_AT,
                                              sizeof(TAF_CALL_BURST_DTMF_REQ_MSG_STRU));
    if (VOS_NULL_PTR == pstMsg)
    {
        AT_ERR_LOG("TAF_XCALL_SendBurstDtmf: Failed to alloc VOS message.");
        return VOS_ERR;
    }

    TAF_MEM_SET_S((VOS_INT8*)pstMsg + VOS_MSG_HEAD_LENGTH,
               (VOS_SIZE_T)(sizeof(TAF_CALL_BURST_DTMF_REQ_MSG_STRU) - VOS_MSG_HEAD_LENGTH),
                0x00,
               (VOS_SIZE_T)(sizeof(TAF_CALL_BURST_DTMF_REQ_MSG_STRU) - VOS_MSG_HEAD_LENGTH));

    /* ��дVOS��Ϣͷ */
    pstMsg->ulSenderCpuId               = VOS_LOCAL_CPUID;
    pstMsg->ulSenderPid                 = WUEPS_PID_AT;
    pstMsg->ulReceiverCpuId             = VOS_LOCAL_CPUID;
    pstMsg->ulReceiverPid               = AT_GetDestPid(clientId, I0_WUEPS_PID_TAF);

    /* ��д��Ϣ���� */
    pstMsg->usMsgId    = TAF_CALL_APP_SEND_BURST_DTMF_REQ;
    pstMsg->usClientId = clientId;
    pstMsg->ucOpId     = opId;
    TAF_MEM_CPY_S(&(pstMsg->stBurstDTMFPara), sizeof(pstMsg->stBurstDTMFPara), pstSndBurstDTMFPara, sizeof(TAF_CALL_BURST_DTMF_PARA_STRU));

    /* ����VOS��Ϣ */
    if (VOS_OK != PS_SEND_MSG(WUEPS_PID_AT, pstMsg))
    {
        AT_ERR_LOG("TAF_XCALL_SendBurstDtmf: Send TAF_CALL_APP_SEND_BURST_DTMF_REQ Message Fail");
        return VOS_ERR;
    }

    return VOS_OK;
}
/* Added by f279542 for CDMA 1X Iteration 4, 2014-11-10, end */


VOS_UINT32  TAF_XCALL_SendCustomDialReq(
    MN_CLIENT_ID_T                      clientId,
    MN_OPERATION_ID_T                   opId,
    TAF_CALL_CUSTOM_DIAL_PARA_STRU     *pstCustomDialPara
)
{
    TAF_CALL_APP_SEND_CUSTOM_DIAL_REQ_STRU *pstMsg =
        (TAF_CALL_APP_SEND_CUSTOM_DIAL_REQ_STRU *)PS_ALLOC_MSG_WITH_HEADER_LEN(WUEPS_PID_AT,
                                              sizeof(TAF_CALL_APP_SEND_CUSTOM_DIAL_REQ_STRU));
    if (VOS_NULL_PTR == pstMsg)
    {
        AT_ERR_LOG("TAF_XCALL_SendCustomDialReq: Failed to alloc VOS message.");
        return VOS_FALSE;
    }

    TAF_MEM_SET_S((VOS_INT8*)pstMsg + VOS_MSG_HEAD_LENGTH,
               (VOS_SIZE_T)(sizeof(TAF_CALL_APP_SEND_CUSTOM_DIAL_REQ_STRU) - VOS_MSG_HEAD_LENGTH),
                0x00,
               (VOS_SIZE_T)(sizeof(TAF_CALL_APP_SEND_CUSTOM_DIAL_REQ_STRU) - VOS_MSG_HEAD_LENGTH));

    /* ��дVOS��Ϣͷ */
    pstMsg->ulSenderCpuId               = VOS_LOCAL_CPUID;
    pstMsg->ulSenderPid                 = WUEPS_PID_AT;
    pstMsg->ulReceiverCpuId             = VOS_LOCAL_CPUID;
    pstMsg->ulReceiverPid               = AT_GetDestPid(clientId, I0_WUEPS_PID_TAF);

    /* ��д��Ϣ���� */
    pstMsg->usMsgId    = TAF_CALL_APP_SEND_CUSTOM_DIAL_REQ;
    pstMsg->usClientId = clientId;
    pstMsg->ucOpId     = opId;
    TAF_MEM_CPY_S(&(pstMsg->stCustomDialPara), sizeof(pstMsg->stCustomDialPara), pstCustomDialPara, sizeof(pstMsg->stCustomDialPara));

    /* ����VOS��Ϣ */
    (VOS_VOID)PS_SEND_MSG(WUEPS_PID_AT, pstMsg);

    return VOS_TRUE;
}



VOS_UINT32  TAF_XCALL_SendContinuousDtmf(
    MN_CLIENT_ID_T                      clientId,
    MN_OPERATION_ID_T                   opId,
    TAF_CALL_CONT_DTMF_PARA_STRU       *pstSndContDTMFPara
)
{
    TAF_CALL_CONT_DTMF_REQ_MSG_STRU *pstMsg = VOS_NULL_PTR;
    pstMsg = (TAF_CALL_CONT_DTMF_REQ_MSG_STRU *)PS_ALLOC_MSG_WITH_HEADER_LEN(WUEPS_PID_AT,
                                              sizeof(TAF_CALL_CONT_DTMF_REQ_MSG_STRU));
    if (VOS_NULL_PTR == pstMsg)
    {
        AT_ERR_LOG("TAF_XCALL_SendContinuousDtmf: Failed to alloc VOS message.");
        return VOS_ERR;
    }

    TAF_MEM_SET_S((VOS_INT8*)pstMsg + VOS_MSG_HEAD_LENGTH,
               (VOS_SIZE_T)(sizeof(TAF_CALL_CONT_DTMF_REQ_MSG_STRU) - VOS_MSG_HEAD_LENGTH),
                0x00,
               (VOS_SIZE_T)(sizeof(TAF_CALL_CONT_DTMF_REQ_MSG_STRU) - VOS_MSG_HEAD_LENGTH));

    /* fill in VOS_MSG_HEAD */
    pstMsg->ulSenderCpuId               = VOS_LOCAL_CPUID;
    pstMsg->ulSenderPid                 = WUEPS_PID_AT;
    pstMsg->ulReceiverCpuId             = VOS_LOCAL_CPUID;
    pstMsg->ulReceiverPid               = AT_GetDestPid(clientId, I0_WUEPS_PID_TAF);

    /* fill in message content */
    pstMsg->usMsgId    = TAF_CALL_APP_SEND_CONT_DTMF_REQ;
    pstMsg->usClientId = clientId;
    pstMsg->ucOpId     = opId;
    TAF_MEM_CPY_S(&(pstMsg->stContDTMFPara), sizeof(pstMsg->stContDTMFPara), pstSndContDTMFPara, sizeof(TAF_CALL_CONT_DTMF_PARA_STRU));

    /* Send VOS message */
    if (VOS_OK != PS_SEND_MSG(WUEPS_PID_AT, pstMsg))
    {
        AT_ERR_LOG("TAF_XCALL_SendContinuousDtmf: Send TAF_CALL_APP_SEND_CONT_DTMF_REQ Message Fail");
        return VOS_ERR;
    }

    return VOS_OK;
}


VOS_UINT32  TAF_XCALL_SendCclpr(
    MN_CLIENT_ID_T                      clientId,
    MN_OPERATION_ID_T                   opId,
    VOS_UINT8                           ucCallId
)
{
    TAF_CALL_SND_CCLPR_REQ_MSG_STRU *pstMsg =
        (TAF_CALL_SND_CCLPR_REQ_MSG_STRU *)PS_ALLOC_MSG_WITH_HEADER_LEN(WUEPS_PID_AT,
                                              sizeof(TAF_CALL_SND_CCLPR_REQ_MSG_STRU));
    if (VOS_NULL_PTR == pstMsg)
    {
        AT_ERR_LOG("TAF_XCALL_SendCclpr: Failed to alloc VOS message.");
        return VOS_ERR;
    }

    TAF_MEM_SET_S((VOS_INT8*)pstMsg + VOS_MSG_HEAD_LENGTH,
               (VOS_SIZE_T)(sizeof(TAF_CALL_SND_CCLPR_REQ_MSG_STRU) - VOS_MSG_HEAD_LENGTH),
                0x00,
               (VOS_SIZE_T)(sizeof(TAF_CALL_SND_CCLPR_REQ_MSG_STRU) - VOS_MSG_HEAD_LENGTH));

    /* ��дVOS��Ϣͷ */
    pstMsg->ulSenderCpuId               = VOS_LOCAL_CPUID;
    pstMsg->ulSenderPid                 = WUEPS_PID_AT;
    pstMsg->ulReceiverCpuId             = VOS_LOCAL_CPUID;
    pstMsg->ulReceiverPid               = AT_GetDestPid(clientId, I0_WUEPS_PID_TAF);

    /* ��д��Ϣ���� */
    pstMsg->usMsgId    = TAF_CALL_APP_SEND_CCLPR_REQ;
    pstMsg->usClientId = clientId;
    pstMsg->ucOpId     = opId;
    pstMsg->ucCallId   = ucCallId;

    /* ����VOS��ϢTAF_CALL_APP_SEND_CCLPR_REQ */
    (VOS_VOID)PS_SEND_MSG(WUEPS_PID_AT, pstMsg);

    return VOS_OK;
}


VOS_UINT32 TAF_XCALL_SendEncryptCall(
    VOS_UINT32                                              ulModuleId,
    MN_CLIENT_ID_T                                          usClientId,
    MN_OPERATION_ID_T                                       opId,
    VOS_UINT32                                              ulEccVoiceType,
    TAF_ECC_CALL_BCD_NUM_STRU                              *pstDialNumber
)
{

    TAF_CALL_APP_ENCRYPT_VOICE_REQ_STRU                    *pstEncryptVoiceReq;
    VOS_UINT32                                              ulReceiverPid;
    VOS_UINT32                                              ulSenderPid;

    ulReceiverPid      = AT_GetDestPid(usClientId, WUEPS_PID_TAF);
    ulSenderPid        = AT_GetDestPid(usClientId, WUEPS_PID_TAF);

    pstEncryptVoiceReq = (TAF_CALL_APP_ENCRYPT_VOICE_REQ_STRU *)PS_ALLOC_MSG_WITH_HEADER_LEN(ulSenderPid,
                                          sizeof(TAF_CALL_APP_ENCRYPT_VOICE_REQ_STRU));
    if (VOS_NULL_PTR == pstEncryptVoiceReq)
    {
        AT_ERR_LOG("TAF_XCALL_SndEncryptCall: Failed to alloc VOS message.");
        return VOS_ERR;
    }

    TAF_MEM_SET_S((VOS_INT8*)pstEncryptVoiceReq + VOS_MSG_HEAD_LENGTH,
               (VOS_SIZE_T)(sizeof(TAF_CALL_APP_ENCRYPT_VOICE_REQ_STRU) - VOS_MSG_HEAD_LENGTH),
                0x00,
               (VOS_SIZE_T)(sizeof(TAF_CALL_APP_ENCRYPT_VOICE_REQ_STRU) - VOS_MSG_HEAD_LENGTH));

    /* ��дVOS��Ϣͷ */
    pstEncryptVoiceReq->ulSenderCpuId                       = VOS_LOCAL_CPUID;
    pstEncryptVoiceReq->ulSenderPid                         = ulSenderPid;
    pstEncryptVoiceReq->ulReceiverCpuId                     = VOS_LOCAL_CPUID;
    pstEncryptVoiceReq->ulReceiverPid                       = ulReceiverPid;
    pstEncryptVoiceReq->ulLength                            = sizeof(TAF_CALL_APP_ENCRYPT_VOICE_REQ_STRU) - VOS_MSG_HEAD_LENGTH;

    /* ��д��Ϣ���� */
    pstEncryptVoiceReq->enMsgName                           = ID_TAF_CALL_APP_ENCRYPT_VOICE_REQ;
    pstEncryptVoiceReq->stCtrl.usClientId                   = usClientId;
    pstEncryptVoiceReq->stCtrl.ucOpId                       = opId;
    pstEncryptVoiceReq->stCtrl.ulModuleId                   = ulModuleId;
    pstEncryptVoiceReq->enEccVoiceType                      = ulEccVoiceType;
    TAF_MEM_CPY_S(&pstEncryptVoiceReq->stDialNumber, (VOS_UINT32)sizeof(pstEncryptVoiceReq->stDialNumber), pstDialNumber, (VOS_UINT32)sizeof(TAF_ECC_CALL_BCD_NUM_STRU));

    /* ����VOS��ϢID_TAF_CALL_APP_ENCRYPT_VOICE_REQ */
	/*lint -e830 -e516 */
    (VOS_VOID)PS_SEND_MSG(ulSenderPid, pstEncryptVoiceReq);

    return VOS_OK;
}


VOS_UINT32 TAF_XCALL_SendEccCtrl(
    VOS_UINT32                                              ulModuleId,
    MN_CLIENT_ID_T                                          usClientId,
    MN_OPERATION_ID_T                                       opId,
    VOS_UINT32                                              ulRemoteCtrlEvtType,
    VOS_UINT32                                              ulResult

)
{
    TAF_CALL_APP_REMOTE_CTRL_ANSWER_REQ_STRU               *pstEccRemoteCtrlAnsReq;
    VOS_UINT32                                              ulReceiverPid;
    VOS_UINT32                                              ulSenderPid;

    ulReceiverPid          = AT_GetDestPid(usClientId, WUEPS_PID_TAF);
    ulSenderPid            = AT_GetDestPid(usClientId, WUEPS_PID_TAF);

    pstEccRemoteCtrlAnsReq = (TAF_CALL_APP_REMOTE_CTRL_ANSWER_REQ_STRU *)PS_ALLOC_MSG_WITH_HEADER_LEN(ulSenderPid,
                                          sizeof(TAF_CALL_APP_REMOTE_CTRL_ANSWER_REQ_STRU));
    if (VOS_NULL_PTR == pstEccRemoteCtrlAnsReq)
    {
        AT_ERR_LOG("TAF_XCALL_SndEccCtrl: Failed to alloc VOS message.");
        return VOS_ERR;
    }

    TAF_MEM_SET_S((VOS_INT8*)pstEccRemoteCtrlAnsReq + VOS_MSG_HEAD_LENGTH,
               (VOS_SIZE_T)(sizeof(TAF_CALL_APP_REMOTE_CTRL_ANSWER_REQ_STRU) - VOS_MSG_HEAD_LENGTH),
                0x00,
               (VOS_SIZE_T)(sizeof(TAF_CALL_APP_REMOTE_CTRL_ANSWER_REQ_STRU) - VOS_MSG_HEAD_LENGTH));

    /* ��дVOS��Ϣͷ */
    pstEccRemoteCtrlAnsReq->ulSenderCpuId                   = VOS_LOCAL_CPUID;
    pstEccRemoteCtrlAnsReq->ulSenderPid                     = ulSenderPid;
    pstEccRemoteCtrlAnsReq->ulReceiverCpuId                 = VOS_LOCAL_CPUID;
    pstEccRemoteCtrlAnsReq->ulReceiverPid                   = ulReceiverPid;
    pstEccRemoteCtrlAnsReq->ulLength                        = sizeof(TAF_CALL_APP_REMOTE_CTRL_ANSWER_REQ_STRU) - VOS_MSG_HEAD_LENGTH;

    /* ��д��Ϣ���� */
    pstEccRemoteCtrlAnsReq->enMsgName                       = ID_TAF_CALL_APP_REMOTE_CTRL_ANSWER_REQ;
    pstEccRemoteCtrlAnsReq->stCtrl.usClientId               = usClientId;
    pstEccRemoteCtrlAnsReq->stCtrl.ucOpId                   = opId;
    pstEccRemoteCtrlAnsReq->stCtrl.ulModuleId               = ulModuleId;
    pstEccRemoteCtrlAnsReq->enRemoteCtrlEvtType             = ulRemoteCtrlEvtType;
    pstEccRemoteCtrlAnsReq->enResult                        = ulResult;

    /* ����VOS��ϢID_TAF_CALL_APP_REMOTE_CTRL_ANSWER_REQ */
    (VOS_VOID)PS_SEND_MSG(ulSenderPid, pstEccRemoteCtrlAnsReq);

    return VOS_OK;
}


VOS_UINT32 TAF_XCALL_SetEccCap(
    VOS_UINT32                                              ulModuleId,
    MN_CLIENT_ID_T                                          usClientId,
    MN_OPERATION_ID_T                                       opId,
    VOS_UINT32                                              ulEccSrvCap,
    VOS_UINT32                                              ulEccSrvStatus
)
{
    TAF_CALL_APP_ECC_SRV_CAP_CFG_REQ_STRU                  *pstEccSrvCapReq;
    VOS_UINT32                                              ulReceiverPid;
    VOS_UINT32                                              ulSenderPid;

    ulReceiverPid   = AT_GetDestPid(usClientId, WUEPS_PID_TAF);
    ulSenderPid     = AT_GetDestPid(usClientId, WUEPS_PID_TAF);

    pstEccSrvCapReq = (TAF_CALL_APP_ECC_SRV_CAP_CFG_REQ_STRU *)PS_ALLOC_MSG_WITH_HEADER_LEN(ulSenderPid,
                                          sizeof(TAF_CALL_APP_ECC_SRV_CAP_CFG_REQ_STRU));
    if (VOS_NULL_PTR == pstEccSrvCapReq)
    {
        AT_ERR_LOG("TAF_XCALL_SetEccCap: Failed to alloc VOS message.");
        return VOS_ERR;
    }

    TAF_MEM_SET_S((VOS_INT8*)pstEccSrvCapReq + VOS_MSG_HEAD_LENGTH,
               (VOS_SIZE_T)(sizeof(TAF_CALL_APP_ECC_SRV_CAP_CFG_REQ_STRU) - VOS_MSG_HEAD_LENGTH),
                0x00,
               (VOS_SIZE_T)(sizeof(TAF_CALL_APP_ECC_SRV_CAP_CFG_REQ_STRU) - VOS_MSG_HEAD_LENGTH));

    /* ��дVOS��Ϣͷ */
    pstEccSrvCapReq->ulSenderCpuId                          = VOS_LOCAL_CPUID;
    pstEccSrvCapReq->ulSenderPid                            = ulSenderPid;
    pstEccSrvCapReq->ulReceiverCpuId                        = VOS_LOCAL_CPUID;
    pstEccSrvCapReq->ulReceiverPid                          = ulReceiverPid;
    pstEccSrvCapReq->ulLength                               = sizeof(TAF_CALL_APP_ECC_SRV_CAP_CFG_REQ_STRU) - VOS_MSG_HEAD_LENGTH;

    /* ��д��Ϣ���� */
    pstEccSrvCapReq->enMsgName                              = ID_TAF_CALL_APP_ECC_SRV_CAP_CFG_REQ;
    pstEccSrvCapReq->stCtrl.usClientId                      = usClientId;
    pstEccSrvCapReq->stCtrl.ucOpId                          = opId;
    pstEccSrvCapReq->stCtrl.ulModuleId                      = ulModuleId;
    pstEccSrvCapReq->enEccSrvCap                            = ulEccSrvCap;
    pstEccSrvCapReq->enEccSrvStatus                         = ulEccSrvStatus;

    /* ����VOS��ϢID_TAF_CALL_APP_ECC_SRV_CAP_CFG_REQ */
    (VOS_VOID)PS_SEND_MSG(ulSenderPid, pstEccSrvCapReq);

    return VOS_OK;
}

VOS_UINT32 TAF_XCALL_QryEncryptCallCap(
    VOS_UINT32                                              ulModuleId,
    MN_CLIENT_ID_T                                          usClientId,
    MN_OPERATION_ID_T                                       opId
)
{
    TAF_CALL_APP_ECC_SRV_CAP_QRY_REQ_STRU                  *pstQryEccCapReq;
    VOS_UINT32                                              ulReceiverPid;
    VOS_UINT32                                              ulSenderPid;

    ulReceiverPid   = AT_GetDestPid(usClientId, WUEPS_PID_TAF);
    ulSenderPid     = AT_GetDestPid(usClientId, WUEPS_PID_TAF);

    pstQryEccCapReq = (TAF_CALL_APP_ECC_SRV_CAP_QRY_REQ_STRU *)PS_ALLOC_MSG_WITH_HEADER_LEN(ulSenderPid,
                                          sizeof(TAF_CALL_APP_ECC_SRV_CAP_QRY_REQ_STRU));
    if (VOS_NULL_PTR == pstQryEccCapReq)
    {
        AT_ERR_LOG("TAF_XCALL_QryEncryptCallCap: Failed to alloc VOS message.");
        return VOS_ERR;
    }

    TAF_MEM_SET_S((VOS_INT8*)pstQryEccCapReq + VOS_MSG_HEAD_LENGTH,
               (VOS_SIZE_T)(sizeof(TAF_CALL_APP_ECC_SRV_CAP_QRY_REQ_STRU) - VOS_MSG_HEAD_LENGTH),
                0x00,
               (VOS_SIZE_T)(sizeof(TAF_CALL_APP_ECC_SRV_CAP_QRY_REQ_STRU) - VOS_MSG_HEAD_LENGTH));

    /* ��дVOS��Ϣͷ */
    pstQryEccCapReq->ulSenderCpuId                        = VOS_LOCAL_CPUID;
    pstQryEccCapReq->ulSenderPid                          = ulSenderPid;
    pstQryEccCapReq->ulReceiverCpuId                      = VOS_LOCAL_CPUID;
    pstQryEccCapReq->ulReceiverPid                        = ulReceiverPid;
    pstQryEccCapReq->ulLength                             = sizeof(TAF_CALL_APP_ECC_SRV_CAP_QRY_REQ_STRU) - VOS_MSG_HEAD_LENGTH;


    /* ��д��Ϣ���� */
    pstQryEccCapReq->enMsgName                            = ID_TAF_CALL_APP_ECC_SRV_CAP_QRY_REQ;
    pstQryEccCapReq->stCtrl.usClientId                    = usClientId;
    pstQryEccCapReq->stCtrl.ucOpId                        = opId;
    pstQryEccCapReq->stCtrl.ulModuleId                    = ulModuleId;

    /* ����VOS��ϢID_TAF_CALL_APP_ECC_SRV_CAP_QRY_REQ */
    (VOS_VOID)PS_SEND_MSG(ulSenderPid, pstQryEccCapReq);

    return VOS_OK;
}




VOS_UINT32 TAF_XCALL_SetPrivacyModePreferred(
    VOS_UINT32                                              ulModuleId,
    MN_CLIENT_ID_T                                          usClientId,
    MN_OPERATION_ID_T                                       opId,
    VOS_UINT32                                              ulPrivacyMode
)
{
    TAF_CALL_APP_PRIVACY_MODE_SET_REQ_STRU                 *pstPrivacyModeReq = VOS_NULL_PTR;
    VOS_UINT32                                              ulReceiverPid;
    VOS_UINT32                                              ulSenderPid;

    ulReceiverPid        = AT_GetDestPid(usClientId, WUEPS_PID_TAF);
    ulSenderPid          = AT_GetDestPid(usClientId, WUEPS_PID_TAF);


    pstPrivacyModeReq = (TAF_CALL_APP_PRIVACY_MODE_SET_REQ_STRU *)PS_ALLOC_MSG_WITH_HEADER_LEN(ulSenderPid,
                                                                                               sizeof(TAF_CALL_APP_PRIVACY_MODE_SET_REQ_STRU));
    if (VOS_NULL_PTR == pstPrivacyModeReq)
    {
        AT_ERR_LOG("TAF_XCALL_SetPrivacyModePreferred: Failed to alloc VOS message.");
        return VOS_ERR;
    }

    TAF_MEM_SET_S((VOS_INT8*)pstPrivacyModeReq + VOS_MSG_HEAD_LENGTH,
               (VOS_SIZE_T)(sizeof(TAF_CALL_APP_PRIVACY_MODE_SET_REQ_STRU) - VOS_MSG_HEAD_LENGTH),
                0x00,
               (VOS_SIZE_T)(sizeof(TAF_CALL_APP_PRIVACY_MODE_SET_REQ_STRU) - VOS_MSG_HEAD_LENGTH));


    /* ��дVOS��Ϣͷ */
    pstPrivacyModeReq->ulSenderCpuId                        = VOS_LOCAL_CPUID;
    pstPrivacyModeReq->ulSenderPid                          = ulSenderPid;
    pstPrivacyModeReq->ulReceiverCpuId                      = VOS_LOCAL_CPUID;
    pstPrivacyModeReq->ulReceiverPid                        = ulReceiverPid;
    pstPrivacyModeReq->ulLength                             = sizeof(TAF_CALL_APP_PRIVACY_MODE_SET_REQ_STRU) - VOS_MSG_HEAD_LENGTH;

    /* ��д��Ϣ���� */
    pstPrivacyModeReq->enMsgName                            = ID_TAF_CALL_APP_PRIVACY_MODE_SET_REQ;
    pstPrivacyModeReq->stCtrl.usClientId                    = usClientId;
    pstPrivacyModeReq->stCtrl.ucOpId                        = opId;
    pstPrivacyModeReq->stCtrl.ulModuleId                    = ulModuleId;

    pstPrivacyModeReq->enPrivacyMode                        = (TAF_CALL_PRIVACY_MODE_ENUM_UINT8)ulPrivacyMode;

    (VOS_VOID)PS_SEND_MSG(ulSenderPid, pstPrivacyModeReq);

    return VOS_OK;
}


VOS_UINT32 TAF_XCALL_QryPrivacyModePreferred(
    VOS_UINT32                                              ulModuleId,
    MN_CLIENT_ID_T                                          usClientId,
    MN_OPERATION_ID_T                                       opId
)
{
    TAF_CALL_APP_PRIVACY_MODE_QRY_REQ_STRU                 *pstPrivacyModeReq = VOS_NULL_PTR;
    VOS_UINT32                                              ulReceiverPid;
    VOS_UINT32                                              ulSenderPid;

    ulReceiverPid        = AT_GetDestPid(usClientId, WUEPS_PID_TAF);
    ulSenderPid          = AT_GetDestPid(usClientId, WUEPS_PID_TAF);


    pstPrivacyModeReq = (TAF_CALL_APP_PRIVACY_MODE_QRY_REQ_STRU *)PS_ALLOC_MSG_WITH_HEADER_LEN(ulSenderPid,
                                                                                               sizeof(TAF_CALL_APP_PRIVACY_MODE_QRY_REQ_STRU));
    if (VOS_NULL_PTR == pstPrivacyModeReq)
    {
        AT_ERR_LOG("TAF_XCALL_QryPrivacyModePreferred: Failed to alloc VOS message.");
        return VOS_ERR;
    }

    TAF_MEM_SET_S((VOS_INT8*)pstPrivacyModeReq + VOS_MSG_HEAD_LENGTH,
               (VOS_SIZE_T)(sizeof(TAF_CALL_APP_PRIVACY_MODE_QRY_REQ_STRU) - VOS_MSG_HEAD_LENGTH),
                0x00,
               (VOS_SIZE_T)(sizeof(TAF_CALL_APP_PRIVACY_MODE_QRY_REQ_STRU) - VOS_MSG_HEAD_LENGTH));


    /* ��дVOS��Ϣͷ */
    pstPrivacyModeReq->ulSenderCpuId                        = VOS_LOCAL_CPUID;
    pstPrivacyModeReq->ulSenderPid                          = ulSenderPid;
    pstPrivacyModeReq->ulReceiverCpuId                      = VOS_LOCAL_CPUID;
    pstPrivacyModeReq->ulReceiverPid                        = ulReceiverPid;
    pstPrivacyModeReq->ulLength                             = sizeof(TAF_CALL_APP_PRIVACY_MODE_QRY_REQ_STRU) - VOS_MSG_HEAD_LENGTH;

    /* ��д��Ϣ���� */
    pstPrivacyModeReq->enMsgName                            = ID_TAF_CALL_APP_PRIVACY_MODE_QRY_REQ;
    pstPrivacyModeReq->stCtrl.usClientId                    = usClientId;
    pstPrivacyModeReq->stCtrl.ucOpId                        = opId;
    pstPrivacyModeReq->stCtrl.ulModuleId                    = ulModuleId;

    (VOS_VOID)PS_SEND_MSG(ulSenderPid, pstPrivacyModeReq);

    return VOS_OK;
}


VOS_UINT32 TAF_CALL_QryCnap(
    VOS_UINT32                              ulModuleId,
    MN_CLIENT_ID_T                          usClientId,
    MN_OPERATION_ID_T                       opId
)
{
    TAF_CALL_APP_CNAP_QRY_REQ_STRU         *pstCnapQryReq;
    VOS_UINT32                              ulReceiverPid;
    VOS_UINT32                              ulSenderPid;

    ulReceiverPid     = AT_GetDestPid(usClientId, WUEPS_PID_TAF);
    ulSenderPid       = AT_GetDestPid(usClientId, WUEPS_PID_TAF);

    pstCnapQryReq = (TAF_CALL_APP_CNAP_QRY_REQ_STRU *)PS_ALLOC_MSG_WITH_HEADER_LEN(ulSenderPid,
                                sizeof(TAF_CALL_APP_CNAP_QRY_REQ_STRU));

    if (VOS_NULL_PTR == pstCnapQryReq)
    {
        AT_ERR_LOG("TAF_CALL_QryCnapPara: Failed to alloc VOS message.");
        return VOS_ERR;
    }

    TAF_MEM_SET_S((VOS_INT8 *)pstCnapQryReq + VOS_MSG_HEAD_LENGTH,
                  (VOS_SIZE_T)(sizeof(TAF_CALL_APP_CNAP_QRY_REQ_STRU) - VOS_MSG_HEAD_LENGTH),
                  0x00,
                  (VOS_SIZE_T)(sizeof(TAF_CALL_APP_CNAP_QRY_REQ_STRU) - VOS_MSG_HEAD_LENGTH));

    /* ��дVOS��Ϣͷ */
    pstCnapQryReq->ulSenderCpuId    = VOS_LOCAL_CPUID;
    pstCnapQryReq->ulSenderPid      = ulSenderPid;
    pstCnapQryReq->ulReceiverCpuId  = VOS_LOCAL_CPUID;
    pstCnapQryReq->ulReceiverPid    = ulReceiverPid;
    pstCnapQryReq->ulLength         = sizeof(TAF_CALL_APP_CNAP_QRY_REQ_STRU) - VOS_MSG_HEAD_LENGTH;

    /* ��д��Ϣ���� */
    pstCnapQryReq->enMsgName            = ID_TAF_CALL_APP_CNAP_QRY_REQ;
    pstCnapQryReq->stCtrl.usClientId    = usClientId;
    pstCnapQryReq->stCtrl.ucOpId        = opId;
    pstCnapQryReq->stCtrl.ulModuleId    = ulModuleId;

    /* ����VOS��ϢID_TAF_CALL_APP_CNAP_QRY_REQ */
    (VOS_VOID)PS_SEND_MSG(ulSenderPid, pstCnapQryReq);

    return VOS_OK;
}

