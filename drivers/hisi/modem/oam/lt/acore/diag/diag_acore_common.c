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
  1 Include HeadFile
*****************************************************************************/
#include "vos.h"
#include "vos_Id.h"
#include <linux/debugfs.h>
#include "mdrv.h"
#include "msp_errno.h"
#include "msp_service.h"
#include "diag_common.h"
#include "SCMProc.h"
#include "SCMSoftDecode.h"
#include "diag_msgbsp.h"
#include "diag_msgbbp.h"
#include "diag_msgphy.h"
#include "diag_msgps.h"
#include "diag_msghifi.h"
#include "diag_msgapplog.h"
#include "diag_acore_common.h"
#include "omerrorlog.h"
#include "diag_api.h"
#include "diag_cfg.h"
#include "diag_debug.h"
#include "cpm.h"
#include "SysNvId.h"
#include "soc_socp_adapter.h"



/*****************************************************************************
  2 Declare the Global Variable
*****************************************************************************/

#define    THIS_FILE_ID        MSP_FILE_ID_DIAG_ACORE_COMMON_C

DRV_RESET_CB_MOMENT_E g_DiagResetingCcore = MDRV_RESET_CB_INVALID;


VOS_UINT32 g_ulDebugCfg = 0;

DIAG_DUMP_INFO_STRU g_stDumpInfo = {0};


extern DIAG_TRANS_HEADER_STRU g_stBbpTransHead;
extern DIAG_TRANS_HEADER_STRU g_stPSTransHead;

struct wake_lock diag_wakelock;

/*****************************************************************************
  3 Function
*****************************************************************************/

extern VOS_VOID SCM_StopAllSrcChan(VOS_VOID);



VOS_VOID diag_SocpVoteMsgProc(MsgBlock* pMsgBlock)
{
}


VOS_VOID diag_AgentVoteToSocp(SOCP_VOTE_TYPE_ENUM_U32 voteType)
{
}

/*****************************************************************************
 Function Name   : diag_ResetCcoreCB
 Description     : ���modem������λ�ص�����
 Input           : enParam
 Output          : None
 Return          : VOS_VOID
*****************************************************************************/
VOS_INT diag_ResetCcoreCB(DRV_RESET_CB_MOMENT_E enParam, int userdata)
{
    VOS_INT ret = ERR_MSP_SUCCESS;

    DIAG_CMD_TRANS_IND_STRU stTransInfo = {0};
    DIAG_MSG_REPORT_HEAD_STRU stDiagHead;

    if(enParam == MDRV_RESET_CB_BEFORE)
    {
        diag_printf("Diag receive ccore reset Callback.\n");

        g_DiagResetingCcore = MDRV_RESET_CB_BEFORE;

        if(!DIAG_IS_CONN_ON)
        {
            return ERR_MSP_SUCCESS;
        }

        stTransInfo.ulModule = MSP_PID_DIAG_APP_AGENT;
        stTransInfo.ulMsgId  = DIAG_CMD_MODEM_WILL_RESET;
        stTransInfo.ulNo     = (g_DiagLogPktNum.ulTransNum)++;

        (VOS_VOID)VOS_MemSet_s(&stDiagHead, sizeof(stDiagHead), 0, sizeof(DIAG_MSG_REPORT_HEAD_STRU));

        stDiagHead.u.ulID           = DIAG_CMD_MODEM_WILL_RESET;
        stDiagHead.ulSsid           = DIAG_SSID_CPU;
        stDiagHead.ulModemId        = 0;
        stDiagHead.ulDirection      = DIAG_MT_IND;
        stDiagHead.ulMsgTransId     = g_ulTransId++;
        stDiagHead.ulChanId         = SCM_CODER_SRC_LOM_IND;

        stDiagHead.ulHeaderSize     = sizeof(stTransInfo);
        stDiagHead.pHeaderData      = &stTransInfo;

        stDiagHead.ulDataSize       = sizeof(VOS_UINT32);
        stDiagHead.pData            = &enParam;

        ret = diag_ServicePackData(&stDiagHead);
        if(ret)
        {
            diag_printf("Report ccore reset fail\n");
        }

        diag_printf("Diag report ccore reset to HIDP,and reset SOCP timer.\n");

        /* modem������λʱ�����жϳ�ʱʱ��ָ�ΪĬ��ֵ����HIDP�����յ���λ��Ϣ */
        mdrv_socp_set_ind_mode(SOCP_IND_MODE_DIRECT);
    }
    else if(enParam == MDRV_RESET_CB_AFTER)
    {
        g_DiagResetingCcore = MDRV_RESET_CB_AFTER;

    }
    else
    {
        diag_printf("diag_ResetCcoreCB enParam error\n");
    }
    return ERR_MSP_SUCCESS;
}


VOS_UINT32 diag_AppAgentMsgProcInit(enum VOS_INIT_PHASE_DEFINE ip)
{
    VOS_UINT32 ret = ERR_MSP_SUCCESS;
    VOS_CHAR * resetName = "DIAG";  /*C�˵�����λ������*/
    VOS_INT    resetLevel = 49;

    if(ip == VOS_IP_LOAD_CONFIG)
    {
    	 wake_lock_init(&diag_wakelock,WAKE_LOCK_SUSPEND,"diag_wakelock");
        ret = (VOS_UINT32)mdrv_sysboot_register_reset_notify(resetName, (pdrv_reset_cbfun)diag_ResetCcoreCB, 0, resetLevel);
        if(ret)
        {
            diag_printf("diag register ccore reset fail\n");
        }

        diag_MspMsgInit();
        diag_BspMsgInit();
        diag_DspMsgInit();
        diag_BbpMsgInit();
        diag_PsMsgInit();
        diag_HifiMsgInit();
        diag_MessageInit();
        diag_ServiceInit();
        msp_ServiceInit();
        diag_AppLogMsgInit();

        VOS_SpinLockInit(&g_DiagLogPktNum.ulPrintLock);
        VOS_SpinLockInit(&g_DiagLogPktNum.ulAirLock);
        VOS_SpinLockInit(&g_DiagLogPktNum.ulVoLTELock);
        VOS_SpinLockInit(&g_DiagLogPktNum.ulLayerLock);
        VOS_SpinLockInit(&g_DiagLogPktNum.ulUserLock);
        VOS_SpinLockInit(&g_DiagLogPktNum.ulEventLock);
        VOS_SpinLockInit(&g_DiagLogPktNum.ulTransLock);

        diag_AgentVoteToSocp(SOCP_VOTE_FOR_SLEEP);
    }
    else if(ip == VOS_IP_RESTART)
    {
        diag_InitAuthVariable();

        if (VOS_OK != CPM_PortAssociateInit())
        {
            return VOS_ERR;
        }

        /* ����ʼ�������������ÿ��� */
        diag_CfgResetAllSwt();

        if(VOS_TRUE == diag_IsPowerOnLogOpen())
        {
            g_ulDiagCfgInfo |= DIAG_CFG_POWERONLOG;
        }
        else
        {
            SCM_RegCoderDestIndChan();
        }

        diag_printf("Diag PowerOnLog is %s.\n", (g_ulDiagCfgInfo&DIAG_CFG_POWERONLOG) ? "open" : "close");
    }

    return ret;
}



VOS_VOID diag_DumpMsgInfo(VOS_UINT32 ulSenderPid, VOS_UINT32 ulMsgId, VOS_UINT32 ulSize)
{
    VOS_UINT32 ulPtr = g_stDumpInfo.ulMsgCur;

    if(g_stDumpInfo.pcMsgAddr)
    {
        *((VOS_UINT32*)(&g_stDumpInfo.pcMsgAddr[ulPtr]))    = ulSenderPid;
        ulPtr = ulPtr + sizeof(VOS_UINT32);
        *((VOS_UINT32*)(&g_stDumpInfo.pcMsgAddr[ulPtr]))  = ulMsgId;
        ulPtr = ulPtr + sizeof(VOS_UINT32);
        *((VOS_UINT32*)(&g_stDumpInfo.pcMsgAddr[ulPtr]))  = ulSize;
        ulPtr = ulPtr + sizeof(VOS_UINT32);
        *((VOS_UINT32*)(&g_stDumpInfo.pcMsgAddr[ulPtr])) = mdrv_timer_get_normal_timestamp();

        g_stDumpInfo.ulMsgCur = (g_stDumpInfo.ulMsgCur + 16);
        if(g_stDumpInfo.ulMsgCur >= g_stDumpInfo.ulMsgLen)
        {
            g_stDumpInfo.ulMsgCur = 0;
        }
    }
}

/* DUMP�洢����Ϣ����󳤶ȣ�����64��ʾ����0xaa5555aa��֡ͷ����Ϣ���ݵ��ܵ���󳤶� */
#define DIAG_DUMP_MAX_FRAME_LEN          (80)


VOS_VOID diag_DumpDFInfo(DIAG_FRAME_INFO_STRU * pFrame)
{
    VOS_UINT32 ulPtr;
    VOS_UINT32 ulLen;
    VOS_UINT32 tempLen;

    if(g_stDumpInfo.pcDFAddr)
    {
        ulPtr = g_stDumpInfo.ulDFCur;

        *((VOS_INT32*)(&g_stDumpInfo.pcDFAddr[ulPtr])) =(VOS_INT32)0xaa5555aa;
        *((VOS_INT32*)(&g_stDumpInfo.pcDFAddr[ulPtr+sizeof(VOS_UINT32)])) = mdrv_timer_get_normal_timestamp();

        /* ÿ�����ݶ���16�ֽڶ��룬����ؾ� */
        g_stDumpInfo.ulDFCur = g_stDumpInfo.ulDFCur + 8;

        ulPtr = g_stDumpInfo.ulDFCur;

        ulLen = 8 + sizeof(DIAG_FRAME_INFO_STRU) + pFrame->ulMsgLen;
        if(ulLen > DIAG_DUMP_MAX_FRAME_LEN)
        {
            ulLen = DIAG_DUMP_MAX_FRAME_LEN;
        }

        ulLen = ((ulLen + 0xf) & (~0xf)) - 8;

        if((ulPtr + ulLen) <= g_stDumpInfo.ulDFLen)
        {
            tempLen = ulLen;

            (VOS_VOID)VOS_MemCpy_s(&g_stDumpInfo.pcDFAddr[ulPtr], tempLen, (VOS_VOID*)pFrame, ulLen);

            /* ����Ϊ0����Ҫȡ�� */
            g_stDumpInfo.ulDFCur = (g_stDumpInfo.ulDFCur + ulLen) % g_stDumpInfo.ulDFLen;
        }
        else
        {
            tempLen = g_stDumpInfo.ulDFLen - ulPtr;

            (VOS_VOID)VOS_MemCpy_s(&g_stDumpInfo.pcDFAddr[ulPtr], tempLen, (VOS_VOID*)pFrame, (g_stDumpInfo.ulDFLen - ulPtr));

            ulLen = ulLen - (g_stDumpInfo.ulDFLen - ulPtr);     /* δ�������������� */
            ulPtr = g_stDumpInfo.ulDFLen - ulPtr;               /* �ѿ����ĳ��� */

            tempLen = ulLen;
            (VOS_VOID)VOS_MemCpy_s(&g_stDumpInfo.pcDFAddr[0], tempLen,(((VOS_UINT8*)pFrame) + ulPtr), ulLen);

            /* ulLenǰ���Ѿ��������ƣ�����ؾ� */
            g_stDumpInfo.ulDFCur = ulLen;
        }
    }
}


VOS_VOID diag_AppAgentMsgProc(MsgBlock* pMsgBlock)
{
    VOS_UINT32  ulErrorLog = ERR_MSP_CONTINUE; /* ������ͷ�е�ע����������� */
    REL_TIMER_MSG *pTimer =NULL;
    DIAG_DATA_MSG_STRU* pMsgTmp;

    /*����ж�*/
    if (NULL == pMsgBlock)
    {
        return;
    }

    /*����ʼ����������˯��*/
    wake_lock(&diag_wakelock);

    diag_DumpMsgInfo(pMsgBlock->ulSenderPid, (*(VOS_UINT32*)pMsgBlock->aucValue), pMsgBlock->ulLength);

    /*���ݷ���PID��ִ�в�ͬ����*/
    switch(pMsgBlock->ulSenderPid)
    {
        /*��ʱ��Ϣ�����ճ�ʱ����ʽ������ظ�*/
        case DOPRA_PID_TIMER:

            pTimer   = (REL_TIMER_MSG*)pMsgBlock;

            if((DIAG_ERRORLOG_TIMER_NAME == pTimer->ulName) && (DIAG_ERRORLOG_TIMER_PARA == pTimer->ulPara))
            {
                OM_AcpuErrLogTimeoutProc();
            }
            else if((DIAG_CLTINFO_TIMER_NAME == pTimer->ulName) && (DIAG_CLTINFO_TIMER_PARA == pTimer->ulPara))
            {
                OM_AcpuCltInfoCnfNotNeedProcessSetFlag();
                OM_AcpuRcvCltInfoFinish();
            }
            else if((DIAG_DEBUG_TIMER_NAME == pTimer->ulName) && (DIAG_DEBUG_TIMER_PARA == pTimer->ulPara))
            {
                diag_ReportMntn();
            }
            else
            {
                diag_TransTimeoutProc(pTimer);
            }
            ulErrorLog = VOS_OK;
            break;

        case WUEPS_PID_REG:
        case CCPU_PID_PAM_OM:

            ulErrorLog = diag_TransCnfProc((VOS_UINT8*)pMsgBlock, (pMsgBlock->ulLength + VOS_MSG_HEAD_LENGTH),
                                            DIAG_MSG_TYPE_BBP, &g_stBbpTransHead);
            break;

        case MSP_PID_DIAG_APP_AGENT:

            pMsgTmp = (DIAG_DATA_MSG_STRU*)pMsgBlock;
            if(ID_MSG_DIAG_HSO_DISCONN_IND == pMsgTmp->ulMsgId)
            {
                (VOS_VOID)diag_SetChanDisconn(pMsgBlock);
            }
            else if(ID_MSG_DIAG_CHR_REQ == pMsgTmp->ulMsgId)
            {
                OM_AcpuErrLogReqMsgProc(pMsgBlock);
            }
            else
            {
            }

            ulErrorLog = VOS_OK;
            break;
        /*lint -save -e826*/
        /*ͶƱ��Ϣ*/
        case MSP_PID_DIAG_AGENT:

            pMsgTmp = (DIAG_DATA_MSG_STRU*)pMsgBlock;
            if(DIAG_MSG_BSP_CMD_LIST_REQ == pMsgTmp->ulMsgId)
            {
                diag_BspRecvCmdList(pMsgBlock);
                ulErrorLog = VOS_OK;
            }
            else
            {
                ulErrorLog = ERR_MSP_CONTINUE;
            }
            break;

        case I0_UEPS_PID_MTA:

            pMsgTmp = (DIAG_DATA_MSG_STRU*)pMsgBlock;
            if(ID_MTA_DIAG_RSA_VERIFY_CNF == pMsgTmp->ulMsgId)
            {
                diag_AuthNvCfg(pMsgBlock);
                ulErrorLog = VOS_OK;
            }
            else
            {
                ulErrorLog = ERR_MSP_CONTINUE;
            }
            break;
        /*lint -restore +e826*/
        case DSP_PID_APM:

            {
                ulErrorLog = ERR_MSP_CONTINUE;
            }
            break;

        default:
            /* ��ͨ��PID�ж��Ƿ�͸���Ļظ���ͨ��ƥ������ʱ����Ľڵ�ָ����ȷ���Ƿ�͸���Ļظ� */
            ulErrorLog = diag_TransCnfProc((VOS_UINT8*)pMsgBlock, (pMsgBlock->ulLength + VOS_MSG_HEAD_LENGTH),
                                            DIAG_MSG_TYPE_PS, &g_stPSTransHead);
            break;

    }

    /* ������ֵΪERR_MSP_CONTINUEʱ����ʾ��Ҫ��������Ƿ���errorlog����Ϣ */
    if(ERR_MSP_CONTINUE == ulErrorLog)
    {
        OM_AcpuErrLogMsgProc(pMsgBlock);
        OM_AcpuCltInfoCnfMsgProc(pMsgBlock);
    }

   /*����ʼ����������˯��*/
  wake_unlock(&diag_wakelock);

   return ;
}

/*****************************************************************************
 Function Name   : diag_AddTransInfoToList
 Description     : ��ӽ�����͸���������ݵ�������
*****************************************************************************/
DIAG_TRANS_NODE_STRU* diag_AddTransInfoToList(VOS_UINT8 * pstReq, VOS_UINT32 ulRcvlen, DIAG_TRANS_HEADER_STRU *pstHead)
{
    DIAG_TRANS_NODE_STRU* pNewNode = NULL;
    VOS_UINT32 ret, ulHigh32, ulLow32;
    VOS_UINT32 ulNodeSize = 0;
    VOS_UINT32 ulTempLen;
    VOS_UINT_PTR ullAddr;

    ulNodeSize = sizeof(DIAG_TRANS_NODE_STRU) + ulRcvlen;

    /*����һ���ڵ��С*/
    pNewNode = VOS_MemAlloc(MSP_PID_DIAG_APP_AGENT, DYNAMIC_MEM_PT, ulNodeSize);
    if (NULL == pNewNode)
    {
        return NULL;
    }

    ulTempLen = ulNodeSize;

    (VOS_VOID)VOS_MemSet_s(pNewNode, ulTempLen, 0, ulNodeSize);

    ulTempLen = ulRcvlen;
    /*������������浽�ڵ���*/
    (VOS_VOID)VOS_MemCpy_s(pNewNode->ucRcvData, ulTempLen, pstReq, ulRcvlen);

    ulLow32 = (uintptr_t)pNewNode;
    {
        ullAddr = (VOS_UINT_PTR)pNewNode;
        ulHigh32 = (VOS_UINT32)(ullAddr>>32);
    }

    /* ������ʱ�����Ա�û�лظ�ʱ�ܹ���ʱɾ���ڵ� */
    ret = VOS_StartRelTimer(&pNewNode->Timer, MSP_PID_DIAG_APP_AGENT, DIAG_TRANS_TIMEOUT_LEN, ulHigh32, \
                            ulLow32, VOS_RELTIMER_NOLOOP, VOS_TIMER_NO_PRECISION);
    if(ret != ERR_MSP_SUCCESS)
    {
        diag_printf("VOS_StartRelTimer fail [%s]\n", __FUNCTION__);
    }

    /* ����ź������� */
    (VOS_VOID)VOS_SmP(pstHead->TransSem, 0);

    /* ����ڵ㵽����β�� */
    blist_add_tail(&pNewNode->DiagList, &pstHead->TransHead);

    pNewNode->pSem       = &pstHead->TransSem;  /* ���ڳ�ʱʱ�Ļ��Ᵽ�� */
    pNewNode->ulMagicNum = DIAG_TRANS_MAGIC_NUM;

    (VOS_VOID)VOS_SmV(pstHead->TransSem);

    return pNewNode;
}



VOS_UINT32 diag_TransReqProcEntry(DIAG_FRAME_INFO_STRU *pstReq, DIAG_TRANS_HEADER_STRU *pstHead)
{
    VOS_UINT32              ret = ERR_MSP_FAILURE;
    VOS_UINT32              ulSize;
    VOS_UINT32              ulCmdParasize;
    VOS_UINT32              ulTempLen;
    DIAG_TRANS_MSG_STRU     *pstSendReq = NULL;
    DIAG_TRANS_NODE_STRU    *pNode;
    VOS_UINT_PTR            ullAddr;
    DIAG_OSA_MSG_STRU       *pstMsg = NULL;

    diag_PTR(EN_DIAG_PTR_MSGMSP_TRANS);

    ulCmdParasize = pstReq->ulMsgLen - sizeof(MSP_DIAG_DATA_REQ_STRU);

     /* ���͸������*/
    pstSendReq = (DIAG_TRANS_MSG_STRU*)(pstReq->aucData + sizeof(MSP_DIAG_DATA_REQ_STRU));

    diag_LNR(EN_DIAG_LNR_PS_TRANS, pstReq->ulCmdId, VOS_GetSlice());

    if(VOS_PID_AVAILABLE != VOS_CheckPidValidity(pstSendReq->ulReceiverPid))
    {
        diag_FailedCmdCnf(pstReq, ERR_MSP_DIAG_ERRPID_CMD);
        return ERR_MSP_FAILURE;
    }

    ulSize = sizeof(DIAG_FRAME_INFO_STRU) + pstReq->ulMsgLen;

    pNode = diag_AddTransInfoToList((VOS_UINT8*)pstReq, ulSize, pstHead);
    if(VOS_NULL == pNode)
    {
        diag_printf("%s diag_AddTransCmdToList failed.\n", __FUNCTION__);
        return ERR_MSP_FAILURE;
    }

    /* д��32λ */
    pstSendReq->ulSN = (uintptr_t)pNode;

    /* �����64λCPU����Ҫ�Ѹ�32λҲ����ȥ */
    {
        ullAddr = (VOS_UINT_PTR)pNode;
        pstSendReq->usOriginalId    = (VOS_UINT16)((ullAddr>>32)&0x0000FFFF);
        pstSendReq->usTerminalId    = (VOS_UINT16)((ullAddr>>48)&0x0000FFFF);
    }

    if(DIAG_DEBUG_TRANS & g_ulDebugCfg)
    {
        diag_printf("[debug] trans req : cmdid 0x%x, pid %d, msgid 0x%x.\n",
           pstReq->ulCmdId, pstSendReq->ulReceiverPid, pstSendReq->ulMsgId);
    }

    pstMsg = (DIAG_OSA_MSG_STRU *)VOS_AllocMsg(MSP_PID_DIAG_APP_AGENT, (ulCmdParasize - VOS_MSG_HEAD_LENGTH));

    if (pstMsg != NULL)
    {
        pstMsg->ulReceiverPid   = pstSendReq->ulReceiverPid;

        ulTempLen = ulCmdParasize - VOS_MSG_HEAD_LENGTH;

        (VOS_VOID)VOS_MemCpy_s(&pstMsg->ulMsgId, ulTempLen, &pstSendReq->ulMsgId, (ulCmdParasize - VOS_MSG_HEAD_LENGTH));

        ret = VOS_SendMsg(MSP_PID_DIAG_APP_AGENT, pstMsg);
        if (ret != VOS_OK)
        {
			diag_printf("diag_TransReqProcEntry VOS_SendMsg failed!\n");
        }
        else
        {
            ret = ERR_MSP_SUCCESS;
        }
    }

    return ret;
}


/*****************************************************************************
 Function Name   : diag_DelTransCmdNode
 Description     : ɾ���Ѿ�������Ľڵ�
*****************************************************************************/
VOS_VOID diag_DelTransCmdNode(DIAG_TRANS_NODE_STRU* pTempNode)
{
    VOS_UINT32 ulNodeSize = 0;

    if(DIAG_TRANS_MAGIC_NUM != pTempNode->ulMagicNum)
    {
        return;
    }

    /*����ź�������*/
    (VOS_VOID)VOS_SmP((*pTempNode->pSem),0);

    /*ɾ���ڵ�*/
    if((VOS_NULL != pTempNode->DiagList.next) && (VOS_NULL != pTempNode->DiagList.prev))
    {
        blist_del(&pTempNode->DiagList);
    }

    (VOS_VOID)VOS_SmV((*pTempNode->pSem));

    ulNodeSize = sizeof(DIAG_TRANS_NODE_STRU);

    (VOS_VOID)VOS_MemSet_s(pTempNode, ulNodeSize, 0, sizeof(DIAG_TRANS_NODE_STRU));

    /*�ͷ��ڴ�*/
    VOS_MemFree(MSP_PID_DIAG_APP_AGENT, pTempNode);

    return ;
}


/*****************************************************************************
 Function Name   : diag_TransTimeoutProc
 Description     : ͸������ĳ�ʱ����
*****************************************************************************/
VOS_VOID diag_TransTimeoutProc(REL_TIMER_MSG *pTimer)
{
    DIAG_TRANS_NODE_STRU *pNode;
    DIAG_FRAME_INFO_STRU *pFrame;
    VOS_UINT_PTR ullAddr;

    /* ����64λ */
    {
        ullAddr = (VOS_UINT_PTR)pTimer->ulName;
        ullAddr = (ullAddr<<32) | pTimer->ulPara;

        pNode = (DIAG_TRANS_NODE_STRU *)ullAddr;
    }

    pFrame = (DIAG_FRAME_INFO_STRU *)pNode->ucRcvData;

    if(DIAG_DEBUG_TRANS & g_ulDebugCfg)
    {
        diag_printf("[debug] trans timeout : cmdid 0x%x.\n", pFrame->ulCmdId);
    }

    /* ɾ���ڵ� */
    diag_DelTransCmdNode(pNode);
}


DIAG_TRANS_NODE_STRU * diag_IsTransCnf(DIAG_TRANS_MSG_STRU* pstPsCnf, DIAG_TRANS_HEADER_STRU *pstHead)
{
    DIAG_TRANS_NODE_STRU    *pNode;
    DIAG_TRANS_NODE_STRU    *pTempNode;
    LIST_S                  *me = NULL;
    VOS_UINT_PTR ullAddr;

    /* ����64λ */
    ullAddr = (VOS_UINT_PTR)pstPsCnf->usTerminalId;
    ullAddr = (ullAddr<<16) | pstPsCnf->usOriginalId;
    ullAddr = (ullAddr<<32) | pstPsCnf->ulSN;

    pNode = (DIAG_TRANS_NODE_STRU *)ullAddr;

    /*����ź�������*/
    (VOS_VOID)VOS_SmP(pstHead->TransSem,0);

    /* �������в���ÿ����������*/
    blist_for_each(me, &pstHead->TransHead)
    {
        pTempNode = blist_entry(me, DIAG_TRANS_NODE_STRU, DiagList);

        if(pTempNode == pNode)
        {
            (VOS_VOID)VOS_SmV(pstHead->TransSem);

            /* ulMagicNum�Ƿ���ʾ�ڵ��ѳ�ʱɾ�� */
            if(DIAG_TRANS_MAGIC_NUM != pNode->ulMagicNum)
            {
                return VOS_NULL;
            }

            return pNode;
        }
    }

    (VOS_VOID)VOS_SmV(pstHead->TransSem);

    return VOS_NULL;
}

/*****************************************************************************
 Function Name   : diag_GetTransInfo
 Description     : ��ȡ͸���������Ϣ����ɾ���ڵ�
*****************************************************************************/
VOS_VOID diag_GetTransInfo(MSP_DIAG_CNF_INFO_STRU *pstInfo,
                             DIAG_TRANS_CNF_STRU    *pstDiagCnf,
                             DIAG_TRANS_MSG_STRU    *pstPsCnf,
                             DIAG_TRANS_NODE_STRU   *pNode)
{
    DIAG_FRAME_INFO_STRU    *pFrame;
    MSP_DIAG_DATA_REQ_STRU  *pDiagData;
    APP_OM_MSG_STRU         *pstOmMsg;

    /*����ź�������*/
    (VOS_VOID)VOS_SmP((*pNode->pSem),0);

    /* ɾ����ʱ�� */
    (VOS_VOID)VOS_StopRelTimer(&pNode->Timer);

    pFrame = (DIAG_FRAME_INFO_STRU *)pNode->ucRcvData;
    pstInfo->ulTransId    = pFrame->stService.ulMsgTransId;
    pstInfo->ulMsgId      = pFrame->ulCmdId;
    pstInfo->ulMode       = pFrame->stID.mode4b;
    pstInfo->ulSubType    = pFrame->stID.sec5b;

    pDiagData           = (MSP_DIAG_DATA_REQ_STRU *)pFrame->aucData;
    pstDiagCnf->ulAuid  = pDiagData->ulAuid;
    pstDiagCnf->ulSn    = pDiagData->ulSn;

    pstOmMsg = (APP_OM_MSG_STRU *)pDiagData->ucData;

    pstPsCnf->usOriginalId  = pstOmMsg->usOriginalId;
    pstPsCnf->usTerminalId  = pstOmMsg->usTerminalId;
    pstPsCnf->ulSN          = pstOmMsg->ulSN;

    (VOS_VOID)VOS_SmV((*pNode->pSem));

    diag_DelTransCmdNode(pNode);

    return ;
}


VOS_UINT32 diag_TransCnfProc(VOS_UINT8* pstCnf ,VOS_UINT32 ulLen, DIAG_MESSAGE_TYPE_U32 ulGroupId, DIAG_TRANS_HEADER_STRU *pstHead)
{
    VOS_UINT32              ret = 0;
    VOS_UINT32              ulTempLen;
    DIAG_TRANS_CNF_STRU     *pstDiagCnf;
    DIAG_TRANS_MSG_STRU     *pstPsCnf;
    MSP_DIAG_CNF_INFO_STRU  stDiagInfo = {0};
    DIAG_TRANS_NODE_STRU    *pNode;

    if(VOS_NULL_PTR == pstCnf)
    {
        return ERR_MSP_FAILURE;
    }
    pstPsCnf = (DIAG_TRANS_MSG_STRU *)pstCnf;

    if(pstPsCnf->ulLength < (sizeof(DIAG_TRANS_MSG_STRU) - VOS_MSG_HEAD_LENGTH - sizeof(pstPsCnf->aucPara)))
    {
        /* �������С��͸������Ļظ����ȿ϶�����͸������ */
        return ERR_MSP_CONTINUE;
    }

    pNode = diag_IsTransCnf(pstPsCnf, pstHead);
    if(VOS_NULL == pNode)
    {
        return ERR_MSP_CONTINUE;
    }

    diag_LNR(EN_DIAG_LNR_PS_TRANS, ((pstPsCnf->usOriginalId<<16) | pstPsCnf->usTerminalId), pstPsCnf->ulSN);

    if(DIAG_DEBUG_TRANS & g_ulDebugCfg)
    {
        diag_printf("[debug] trans cnf : cmdid 0x%x, pid %d, msgid 0x%x.\n",
            stDiagInfo.ulMsgId, pstPsCnf->ulSenderPid, pstPsCnf->ulMsgId);
    }

    pstDiagCnf = VOS_MemAlloc(MSP_PID_DIAG_APP_AGENT, DYNAMIC_MEM_PT, (ulLen + sizeof(DIAG_TRANS_CNF_STRU)));
    if(VOS_NULL_PTR == pstDiagCnf)
    {
        return ERR_MSP_FAILURE;
    }

    stDiagInfo.ulSSId       = DIAG_SSID_APP_CPU;
    stDiagInfo.ulMsgType    = ulGroupId;
    stDiagInfo.ulDirection  = DIAG_MT_CNF;
    stDiagInfo.ulModemid    = 0;

    diag_GetTransInfo(&stDiagInfo, pstDiagCnf, pstPsCnf, pNode);

    diag_LNR(EN_DIAG_LNR_PS_TRANS, (stDiagInfo.ulMsgId), VOS_GetSlice());

    ulTempLen = ulLen;

    (VOS_VOID)VOS_MemCpy_s(pstDiagCnf->aucPara, ulTempLen, pstPsCnf, ulLen);

    ret = DIAG_MsgReport(&stDiagInfo, pstDiagCnf, (ulLen + sizeof(DIAG_TRANS_CNF_STRU)));

    VOS_MemFree(MSP_PID_DIAG_APP_AGENT, pstDiagCnf);

    return ret;
}



VOS_VOID DIAG_LogShowToFile(VOS_BOOL bIsSendMsg)
{
}


int diag_debug_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static ssize_t diag_debug_read(struct file *file, char __user *ubuf, size_t cnt, loff_t *ppos)
{
    printk("usage:\n");
    printk("\t echo cmd [param1] [param2] ... [paramN] > /sys/kernel/debug/modem_diag/diag\n");
    printk("cmd list:\n");
    printk("\t DIAG_ALL --- save all diag debug infomation.\n");
    return 0;
}

static ssize_t diag_debug_write(struct file *filp, const char __user *ubuf, size_t cnt, loff_t *ppos)
{
    char buf[128] = {0};
    ssize_t ret = cnt;
    DIAG_A_DEBUG_C_REQ_STRU *pstFlag = NULL;
    VOS_UINT32              ulret = ERR_MSP_FAILURE;

    cnt = (cnt > 127) ? 127 : cnt;
    if(ubuf == NULL)
    {
        diag_printf("user buf is NULL\n");
        goto out;
    }

    if(copy_from_user(buf, ubuf, cnt))
    {
        diag_printf("copy from user fail\n");
        ret = -EFAULT;
        goto out;
    }
    buf[cnt] = 0;

    /* �����쳣Ŀ¼���� */
    if(0 == strncmp(buf, "DIAG_ALL", strlen("DIAG_ALL")))
    {
        DIAG_DebugCommon();
        DIAG_DebugNoIndLog();
        DIAG_DebugDFR();
        DIAG_Throughput();

        pstFlag = (DIAG_A_DEBUG_C_REQ_STRU *)VOS_AllocMsg(MSP_PID_DIAG_APP_AGENT, (sizeof(DIAG_A_DEBUG_C_REQ_STRU) - VOS_MSG_HEAD_LENGTH));

        if (pstFlag != NULL)
        {
            pstFlag->ulReceiverPid  = MSP_PID_DIAG_AGENT;

            pstFlag->ulMsgId        = DIAG_MSG_MSP_A_DEBUG_C_REQ;
            pstFlag->ulFlag         = DIAG_DEBUG_DFR_BIT | DIAG_DEBUG_NIL_BIT;

            ulret = VOS_SendMsg(MSP_PID_DIAG_APP_AGENT, pstFlag);
            if (ulret != VOS_OK)
            {
                diag_printf("diag_debug_write VOS_SendMsg failed!\n");
            }
        }
    }

out:
    return ret;
}

long diag_debug_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    return 0;
}
const struct file_operations diag_debug_fops = {
    .open       = diag_debug_open,
    .read       = diag_debug_read,
    .write	    = diag_debug_write,
    .unlocked_ioctl = diag_debug_ioctl,
};



VOS_UINT32 MSP_AppDiagFidInit(enum VOS_INIT_PHASE_DEFINE ip)
{
    VOS_UINT32 ulRelVal = 0;
    struct dentry * d_file;

    switch (ip)
    {
        case VOS_IP_LOAD_CONFIG:

            ulRelVal = VOS_RegisterPIDInfo(MSP_PID_DIAG_APP_AGENT, (Init_Fun_Type) diag_AppAgentMsgProcInit, (Msg_Fun_Type) diag_AppAgentMsgProc);

            if (ulRelVal != VOS_OK)
            {
                return VOS_ERR;
            }

            ulRelVal = VOS_RegisterMsgTaskPrio(MSP_FID_DIAG_ACPU, VOS_PRIORITY_M2);
            if (ulRelVal != VOS_OK)
            {
                return VOS_ERR;
            }

            /* coverity[var_deref_model] */
            d_file = debugfs_create_dir("modem_diag", NULL);
            if(!d_file)
            {
                diag_printf("create debugfs dir modem_diag fail\n");
                return VOS_ERR;
            }
            /* ����8K��dump�ռ� */
            g_stDumpInfo.pcDumpAddr = (VOS_VOID * )mdrv_om_register_field(OM_AP_DIAG, "ap_diag", (void*)0, (void*)0, DIAG_DUMP_LEN, 0);

            if(VOS_NULL != g_stDumpInfo.pcDumpAddr)
            {
                g_stDumpInfo.pcMsgAddr = g_stDumpInfo.pcDumpAddr;
                g_stDumpInfo.ulMsgCur  = 0;
                g_stDumpInfo.ulMsgLen  = DIAG_DUMP_MSG_LEN;

                g_stDumpInfo.pcDFAddr  = g_stDumpInfo.pcDumpAddr + DIAG_DUMP_MSG_LEN;
                g_stDumpInfo.ulDFCur   = 0;
                g_stDumpInfo.ulDFLen   = DIAG_DUMP_DF_LEN;
            }

            SCM_SoftDecodeCfgRcvTaskInit();

            VOS_RegisterMsgGetHook((VOS_MSG_HOOK_FUNC)DIAG_TraceReport);

            return COMM_Init();
            // break;

        case VOS_IP_RESTART:
            // return COMM_Init();

        default:
            break;
    }

    return VOS_OK;
}


VOS_VOID DIAG_DebugTransOn(VOS_UINT32 ulOn)
{
    if(0 == ulOn)
    {
        g_ulDebugCfg &= (~DIAG_DEBUG_TRANS);
    }
    else
    {
        g_ulDebugCfg |= DIAG_DEBUG_TRANS;
    }
}




