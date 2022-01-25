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
#include "AdsCtx.h"
#include "AdsUpLink.h"
#include "AdsDownLink.h"
#include "AdsFilter.h"
#include "AdsDebug.h"
#include "mdrv.h"
#include "NasNvInterface.h"
#include "TafNvInterface.h"



/*****************************************************************************
    Э��ջ��ӡ��㷽ʽ�µ�.C�ļ��궨��
*****************************************************************************/
#define    THIS_FILE_ID                 PS_FILE_ID_ADS_CTX_C

/*****************************************************************************
  2 ȫ�ֱ�������
*****************************************************************************/

VOS_UINT32                              g_ulAdsULTaskId        = 0;  /* ADS��������ID */
VOS_UINT32                              g_ulAdsDLTaskId        = 0;  /* ADS��������ID */
VOS_UINT32                              g_ulAdsULTaskReadyFlag = 0;  /* ADS������������״̬ */
VOS_UINT32                              g_ulAdsDLTaskReadyFlag = 0;  /* ADS������������״̬ */

/* ADSģ��������� */
ADS_CTX_STRU                            g_stAdsCtx;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 9, 0))
VOS_UINT64                              g_ullAdsDmaMask = 0xffffffffffffffffULL;
#else
struct device                          *g_pstDmaDev;
#endif
/*****************************************************************************
  3 ����ʵ��
*****************************************************************************/


VOS_UINT32 ADS_UL_CheckAllQueueEmpty(VOS_UINT8 ucInstanceIndex)
{
    VOS_UINT32                          i;
    ADS_UL_CTX_STRU                    *pstAdsUlCtx = VOS_NULL_PTR;

    pstAdsUlCtx = ADS_GetUlCtx(ucInstanceIndex);

    for (i = ADS_RAB_ID_MIN; i < ADS_RAB_ID_MAX + 1; i++)
    {
        if (VOS_FALSE != pstAdsUlCtx->astAdsUlQueue[i].ucIsQueueValid)
        {
            break;
        }
    }

    /* ���е�PDP��ȥ���*/
    if ((ADS_RAB_ID_MAX + 1) != i)
    {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}


VOS_VOID ADS_UL_SetProtectTmrLen(VOS_UINT32 ulTimerLen)
{
    g_stAdsCtx.stAdsIpfCtx.ulProtectTmrLen = ulTimerLen;
    return;
}


VOS_UINT32 ADS_UL_IsQueueExistent(
    VOS_UINT8                           ucInstanceIndex,
    VOS_UINT8                           ucRabId
)
{
    /* ����Ϊ�� */
    if (VOS_NULL_PTR == ADS_UL_GET_QUEUE_LINK_PTR(ucInstanceIndex, ucRabId))
    {
        return VOS_ERR;
    }
    else
    {
        return VOS_OK;
    }
}


VOS_UINT32 ADS_UL_IsAnyQueueExist(VOS_VOID)
{
    VOS_UINT8                           ucInstance;
    VOS_UINT8                           ucRabId;

    for (ucInstance = 0; ucInstance < ADS_INSTANCE_MAX_NUM; ucInstance++)
    {
        for (ucRabId = ADS_RAB_ID_MIN; ucRabId <= ADS_RAB_ID_MAX; ucRabId++)
        {
            if (VOS_OK == ADS_UL_IsQueueExistent(ucInstance, ucRabId))
            {
                return VOS_TRUE;
            }
        }
    }

    return VOS_FALSE;
}


VOS_UINT32 ADS_UL_InsertQueue(
    VOS_UINT8                           ucInstance,
    IMM_ZC_STRU                        *pstImmZc,
    VOS_UINT8                           ucRabId
)
{
    VOS_UINT32                          ulNonEmptyEvent;
    VOS_UINT32                          ulAllUlQueueDataNum;
    VOS_UINT32                          ulSlice;
    VOS_UINT                            ulQueueLen;
    VOS_ULONG                           ulLockLevel;
    ADS_CDS_IPF_PKT_TYPE_ENUM_UINT8     enPktType;

    ulNonEmptyEvent = VOS_FALSE;

    /* �˽ӿڲ��ͷ�pstData�����ϲ�ģ����ݺ�������ֵ�ж��Ƿ���Ҫ�ͷ��ڴ� */

    /* ���м��� */
    /*lint -e571*/
    VOS_SpinLockIntLock(ADS_UL_GET_QUEUE_LINK_SPINLOCK(ucInstance, ucRabId), ulLockLevel);
    /*lint +e571*/

    /* �����ڣ������в����� */
    if (VOS_OK != ADS_UL_IsQueueExistent(ucInstance, ucRabId))
    {
        /* ���в�����ɽ��� */
        VOS_SpinUnlockIntUnlock(ADS_UL_GET_QUEUE_LINK_SPINLOCK(ucInstance, ucRabId), ulLockLevel);
        ADS_WARNING_LOG(ACPU_PID_ADS_UL, "ADS_UL_InsertQueue:the queue is not ext!");
        ADS_DBG_UL_PKT_ENQUE_FAIL_NUM(1);
        return VOS_ERR;
    }

    ulQueueLen = IMM_ZcQueueLen(ADS_UL_GET_QUEUE_LINK_PTR(ucInstance, ucRabId));
    if (ulQueueLen >= ADS_UL_GET_MAX_QUEUE_LENGTH(ucInstance))
    {
        /* ���в�����ɽ��� */
        VOS_SpinUnlockIntUnlock(ADS_UL_GET_QUEUE_LINK_SPINLOCK(ucInstance, ucRabId), ulLockLevel);
        ADS_DBG_UL_PKT_ENQUE_FAIL_NUM(1);
        return VOS_ERR;
    }

    /* ����ModemId/PktType/RabId��IMM */
    enPktType = ADS_UL_GET_QUEUE_PKT_TYPE(ucInstance, ucRabId);
    ADS_UL_SAVE_MODEMID_PKTTYEP_RABID_TO_IMM(pstImmZc, ucInstance, enPktType, ucRabId);

    /* �������ǰ�����ݴ���ʱ��� */
    ulSlice = VOS_GetSlice();
    ADS_UL_SAVE_SLICE_TO_IMM(pstImmZc, ulSlice);

    /* ������� */
    IMM_ZcQueueTail(ADS_UL_GET_QUEUE_LINK_PTR(ucInstance, ucRabId), pstImmZc);
    ADS_DBG_UL_PKT_ENQUE_SUCC_NUM(1);

    /* �����ɿձ�Ϊ�ǿ� */
    if (1 == IMM_ZcQueueLen(ADS_UL_GET_QUEUE_LINK_PTR(ucInstance, ucRabId)))
    {
        ulNonEmptyEvent = VOS_TRUE;
    }

    /* ���в�����ɽ��� */
    VOS_SpinUnlockIntUnlock(ADS_UL_GET_QUEUE_LINK_SPINLOCK(ucInstance, ucRabId), ulLockLevel);

    ulAllUlQueueDataNum = ADS_UL_GetAllQueueDataNum();

    if (VOS_TRUE == ADS_UL_GET_THRESHOLD_ACTIVE_FLAG())
    {
        /* (1).jiffies��ʱ,��ֱ�Ӵ�������,��ֹͣ������ʱ��
           (2).�����������ѵ��ܰ������ҵ�ǰû���ڴ�������,�������л��滺�洦���¼�
           (3).�����ɿձ�Ϊ�ǿ�ʱ��������ͳ�ƶ�ʱ���Լ�������ʱ��
         */
        ADS_UL_ADD_STAT_PKT_NUM(1);

        /* ��ʱ����Ϊ��������Ҫ������������ */
        if (0 != ADS_UL_GET_JIFFIES_EXP_TMR_LEN())
        {
            if (ADS_TIME_AFTER_EQ(ADS_GET_CURR_KERNEL_TIME(),
                                  (ADS_UL_GET_JIFFIES_TMR_CNT() + ADS_UL_GET_JIFFIES_EXP_TMR_LEN())))
            {
                ADS_UL_SET_JIFFIES_TMR_CNT(ADS_GET_CURR_KERNEL_TIME());
                ADS_UL_SndEvent(ADS_UL_EVENT_DATA_PROC);
                ADS_DBG_UL_TMR_HIT_THRES_TRIG_EVENT(1);
                return VOS_OK;
            }
        }

        if (ADS_UL_IS_REACH_THRESHOLD(ulAllUlQueueDataNum, ADS_UL_GET_SENDING_FLAG()))
        {
            ADS_UL_SndEvent(ADS_UL_EVENT_DATA_PROC);
            ADS_DBG_UL_QUE_HIT_THRES_TRIG_EVENT(1);
        }

        /* �����ɿձ�Ϊ�ǿ� */
        if (VOS_TRUE == ulNonEmptyEvent)
        {
            ADS_StartTimer(TI_ADS_UL_DATA_STAT, ADS_UL_GET_STAT_TIMER_LEN());
            ADS_StartTimer(TI_ADS_UL_SEND, ADS_UL_GET_PROTECT_TIMER_LEN());
        }
    }
    else
    {
        /* (1).�����ɿձ�Ϊ�ǿ�ʱ�������л��洦���¼�
           (2).�����������ѵ��ܰ����޵��������ҵ�ǰû���ڴ�������
               �������л��滺�洦���¼�
         */
        if (VOS_TRUE == ulNonEmptyEvent)
        {
            ADS_UL_SndEvent(ADS_UL_EVENT_DATA_PROC);
            ADS_DBG_UL_QUE_NON_EMPTY_TRIG_EVENT(1);
        }
        else
        {
            if (ADS_UL_IS_REACH_THRESHOLD(ulAllUlQueueDataNum, ADS_UL_GET_SENDING_FLAG()))
            {
                ADS_UL_SndEvent(ADS_UL_EVENT_DATA_PROC);
                ADS_DBG_UL_QUE_HIT_THRES_TRIG_EVENT(1);
            }
        }
    }

    return VOS_OK;
}


VOS_UINT32 ADS_UL_GetInstanceAllQueueDataNum(VOS_UINT8 ucInstanceIndex)
{
    VOS_UINT32                          i;
    VOS_UINT32                          ulTotalNum;
    ADS_SPEC_CTX_STRU                  *pstAdsSpecCtx = VOS_NULL_PTR;

    pstAdsSpecCtx = &(g_stAdsCtx.astAdsSpecCtx[ucInstanceIndex]);

    ulTotalNum = 0;

    for (i = ADS_RAB_ID_MIN; i < ADS_RAB_ID_MAX + 1; i++)
    {
        if (VOS_NULL_PTR != pstAdsSpecCtx->stAdsUlCtx.astAdsUlQueue[i].pstAdsUlLink)
        {
            ulTotalNum += pstAdsSpecCtx->stAdsUlCtx.astAdsUlQueue[i].pstAdsUlLink->qlen;
        }
    }

    return ulTotalNum;
}


VOS_UINT32 ADS_UL_GetAllQueueDataNum(VOS_VOID)
{
    VOS_UINT32                          ulTotalNum;
    VOS_UINT8                           i;

    ulTotalNum = 0;

    for (i = 0; i < ADS_INSTANCE_MAX_NUM; i++)
    {
        ulTotalNum = ulTotalNum + ADS_UL_GetInstanceAllQueueDataNum(i);
    }

    return ulTotalNum;
}


/*lint -sem(ADS_UL_SetQueue,custodial(4))*/
VOS_UINT32 ADS_UL_CreateQueue(
    VOS_UINT8                           ucInstanceIndex,
    VOS_UINT8                           ucRabId,
    ADS_QCI_TYPE_ENUM_UINT8             enPrio,
    ADS_CDS_IPF_PKT_TYPE_ENUM_UINT8     enPktType,
    VOS_UINT8                           uc1XorHrpdUlIpfFlag
)
{
    IMM_ZC_HEAD_STRU                   *pstUlQueue;
    ADS_UL_CTX_STRU                    *pstAdsUlCtx;

    pstAdsUlCtx = ADS_GetUlCtx(ucInstanceIndex);

    /* RabId�Ķ����Ѿ����� */
    if (VOS_OK == ADS_UL_IsQueueExistent(ucInstanceIndex, ucRabId))
    {
        /* ��Ӧ�ĵ������ȼ�Ҳһ�������Ǳ�֮ǰ��Ҫ�ͣ�������QCIֱ�ӷ���OK */
        if (enPrio >= pstAdsUlCtx->astAdsUlQueue[ucRabId].enPrio)
        {
            return VOS_OK;
        }
        /* �����Ӧ�ĵ������ȼ���֮ǰ��Ҫ�ߣ���Ҫ���¸�PDP�Ķ������ȼ������Զ��й���������� */
        else
        {
            ADS_UL_UpdateQueueInPdpModified(ucInstanceIndex, enPrio, ucRabId);
            return VOS_OK;
        }
    }

    /* ucRabID�Ķ��в�����, ��Ҫ��������ͷ��� */
    pstUlQueue = (IMM_ZC_HEAD_STRU *)PS_MEM_ALLOC(ACPU_PID_ADS_UL, sizeof(IMM_ZC_HEAD_STRU));

    if (VOS_NULL_PTR == pstUlQueue)
    {
        ADS_ERROR_LOG(ACPU_PID_ADS_UL, "ADS_UL_CreateQueue: pstUlQueue is null");
        return VOS_ERR;
    }

    /* ���г�ʼ�� */
    IMM_ZcQueueHeadInit(pstUlQueue);

    /* ��������Ϣ���µ����������� */
    ADS_UL_SetQueue(ucInstanceIndex,
                    ucRabId,
                    VOS_TRUE,
                    pstUlQueue,
                    enPrio,
                    enPktType,
                    uc1XorHrpdUlIpfFlag);

    /* ���в����ܱ��þ���һ��RABID��Ӧһ�����У�����Ч���Ѿ�����Ϣ�������Σ�
       �ʲ���Ҫ�ж��Ƿ���������ֱ���������� */
    ADS_UL_OrderQueueIndex(ucInstanceIndex, ucRabId);

    return VOS_OK;
}


VOS_VOID ADS_UL_ClearQueue(
    VOS_UINT8                           ucRabId,
    IMM_ZC_HEAD_STRU                   *pstQueue
)
{
    VOS_UINT32                          i;
    VOS_UINT32                          ulQueueCnt;
    IMM_ZC_STRU                        *pstNode;

    ulQueueCnt = IMM_ZcQueueLen(pstQueue);

    for (i = 0; i < ulQueueCnt; i++)
    {
        pstNode = IMM_ZcDequeueHead(pstQueue);

        /* �ͷŽ������ */
        if (VOS_NULL_PTR != pstNode)
        {
            IMM_ZcFreeAny(pstNode);
        }
    }
}


VOS_VOID ADS_UL_DestroyQueue(
    VOS_UINT8                           ucInstanceIndex,
    VOS_UINT8                           ucRabId
)
{
    VOS_ULONG                           ulLockLevel;

    /* �����ڵ�����Ϊ�� */
    if (VOS_ERR == ADS_UL_IsQueueExistent(ucInstanceIndex, ucRabId))
    {
        /* Rab Id�Լ����ȼ���Ϊ��Чֵ */
        ADS_UL_SetQueue(ucInstanceIndex,
                        ucRabId,
                        VOS_FALSE,
                        VOS_NULL_PTR,
                        ADS_QCI_TYPE_BUTT,
                        ADS_PDP_TYPE_BUTT,
                        VOS_FALSE);

        /* �������µĶ��й���������� */
        ADS_UL_UpdateQueueInPdpDeactived(ucInstanceIndex, ucRabId);

        return;
    }

    /* ���м��� */
    /*lint -e571*/
    VOS_SpinLockIntLock(ADS_UL_GET_QUEUE_LINK_SPINLOCK(ucInstanceIndex, ucRabId), ulLockLevel);
    /*lint +e571*/

    /* Modified by l60609 for AP������Ŀ ��2012-09-10 Begin */
    /* ���ٶ����е����� */
    ADS_UL_ClearQueue(ucRabId,
                      ADS_UL_GET_QUEUE_LINK_PTR(ucInstanceIndex, ucRabId));
    /* Modified by l60609 for AP������Ŀ ��2012-09-10 End */

    /* ���ٶ���ͷ���*/
    PS_MEM_FREE(ACPU_PID_ADS_DL, ADS_UL_GET_QUEUE_LINK_PTR(ucInstanceIndex, ucRabId));

    /* ��������Ϣ���µ����������� */
    ADS_UL_SetQueue(ucInstanceIndex,
                    ucRabId,
                    VOS_FALSE,
                    VOS_NULL_PTR,
                    ADS_QCI_TYPE_BUTT,
                    ADS_PDP_TYPE_BUTT,
                    VOS_FALSE);

    /* ���в�����ɽ��� */
    VOS_SpinUnlockIntUnlock(ADS_UL_GET_QUEUE_LINK_SPINLOCK(ucInstanceIndex, ucRabId), ulLockLevel);

    /* �������µĶ��й���������� */
    ADS_UL_UpdateQueueInPdpDeactived(ucInstanceIndex, ucRabId);

}


VOS_UINT32 ADS_UL_GetInsertIndex(
    VOS_UINT8                           ucInstanceIndex,
    VOS_UINT8                           ucRabId
)
{
    VOS_UINT32                          i;
    ADS_UL_CTX_STRU                    *pstAdsUlCtx;

    pstAdsUlCtx = ADS_GetUlCtx(ucInstanceIndex);

    /* �������ȼ�����ȡ�����������ж��е�Indexֵ */
    for (i = 0; i < ADS_RAB_NUM_MAX; i++)
    {
        if (pstAdsUlCtx->aucPrioIndex[i] == ucRabId)
        {
            break;
        }
    }

    return i;

}


VOS_VOID ADS_UL_OrderQueueIndex(
    VOS_UINT8                           ucInstanceIndex,
    VOS_UINT8                           ucIndex
)
{
    VOS_UINT32                          i;
    VOS_UINT32                          j;
    ADS_UL_CTX_STRU                    *pstAdsUlCtx;

    pstAdsUlCtx = ADS_GetUlCtx(ucInstanceIndex);

    /* �����PDP�����ȼ��Ƚϸߣ���Ҫ���뵽����PDP��ǰ�棬�������ȼ��͵���Ҫ�����һλ */
    for (i = 0; i < ADS_RAB_NUM_MAX; i++)
    {
        if (pstAdsUlCtx->astAdsUlQueue[ucIndex].enPrio < pstAdsUlCtx->astAdsUlQueue[pstAdsUlCtx->aucPrioIndex[i]].enPrio)
        {
            for (j = ADS_RAB_NUM_MAX - 1; j > i; j--)
            {
                pstAdsUlCtx->aucPrioIndex[j] = pstAdsUlCtx->aucPrioIndex[j - 1];
            }
            pstAdsUlCtx->aucPrioIndex[i] = ucIndex;

            break;
        }
    }
}


VOS_VOID ADS_UL_UpdateQueueInPdpModified(
    VOS_UINT8                           ucInstanceIndex,
    ADS_QCI_TYPE_ENUM_UINT8             enPrio,
    VOS_UINT8                           ucRabId
)
{
    VOS_UINT32                          i;
    VOS_UINT32                          ulIndex;
    ADS_UL_CTX_STRU                    *pstAdsUlCtx;

    pstAdsUlCtx = ADS_GetUlCtx(ucInstanceIndex);

    /* ���޸ĵ����ȼ����µ����ڵĶ��й����� */
    pstAdsUlCtx->astAdsUlQueue[ucRabId].enPrio = enPrio;

    /* ��ȡ�޸ĵ�RABID��aucPrioIndex������λ�� */
    ulIndex = ADS_UL_GetInsertIndex(ucInstanceIndex, ucRabId);

    /* û���ҵ����������� */
    if (ulIndex >= ADS_RAB_NUM_MAX)
    {
        return;
    }

    /* �Ƚ��޸Ķ�Ӧλ�������ǰ�ƶ�һλ */
    for (i = ulIndex; i <  ADS_RAB_NUM_MAX - 1; i++)
    {
        pstAdsUlCtx->aucPrioIndex[i] = pstAdsUlCtx->aucPrioIndex[i + 1UL];
    }

    pstAdsUlCtx->aucPrioIndex[ADS_RAB_NUM_MAX - 1] = 0;

    /* �ƶ����൱�������²��뵽���й����� */
    ADS_UL_OrderQueueIndex(ucInstanceIndex, ucRabId);

}


VOS_VOID ADS_UL_UpdateQueueInPdpDeactived(
    VOS_UINT8                           ucInstanceIndex,
    VOS_UINT8                           ucRabId
)
{
    VOS_UINT32                          i;
    VOS_UINT32                          ulIndex;
    ADS_UL_CTX_STRU                    *pstAdsUlCtx;

    pstAdsUlCtx = ADS_GetUlCtx(ucInstanceIndex);

    /* ����ɾ����PDP��������������Ԫ����ǰ�ƶ�һλ */
    ulIndex = ADS_UL_GetInsertIndex(ucInstanceIndex, ucRabId);

    if (ulIndex >= ADS_RAB_NUM_MAX)
    {
        return;
    }

    for (i = ulIndex; i < ADS_RAB_NUM_MAX - 1; i++)
    {
        pstAdsUlCtx->aucPrioIndex[i] = pstAdsUlCtx->aucPrioIndex[i + 1UL];
    }

    pstAdsUlCtx->aucPrioIndex[ADS_RAB_NUM_MAX - 1] = 0;

}


VOS_VOID ADS_UL_SetQueue(
    VOS_UINT8                           ucInstanceIndex,
    VOS_UINT8                           ucRabId,
    VOS_UINT8                           ucIsQueueValid,
    IMM_ZC_HEAD_STRU                   *pstUlQueue,
    ADS_QCI_TYPE_ENUM_UINT8             enPrio,
    ADS_CDS_IPF_PKT_TYPE_ENUM_UINT8     enPktType,
    VOS_UINT8                           uc1XorHrpdUlIpfFlag
)
{
    g_stAdsCtx.astAdsSpecCtx[ucInstanceIndex].stAdsUlCtx.astAdsUlQueue[ucRabId].pstAdsUlLink   = pstUlQueue;
    g_stAdsCtx.astAdsSpecCtx[ucInstanceIndex].stAdsUlCtx.astAdsUlQueue[ucRabId].ucIsQueueValid = ucIsQueueValid;
    g_stAdsCtx.astAdsSpecCtx[ucInstanceIndex].stAdsUlCtx.astAdsUlQueue[ucRabId].enPrio         = enPrio;
    g_stAdsCtx.astAdsSpecCtx[ucInstanceIndex].stAdsUlCtx.astAdsUlQueue[ucRabId].usRecordNum    = 0;
    g_stAdsCtx.astAdsSpecCtx[ucInstanceIndex].stAdsUlCtx.astAdsUlQueue[ucRabId].enPktType      = enPktType;
    g_stAdsCtx.astAdsSpecCtx[ucInstanceIndex].stAdsUlCtx.astAdsUlQueue[ucRabId].uc1XorHrpdUlIpfFlag = uc1XorHrpdUlIpfFlag;
}


VOS_VOID ADS_UL_SndEvent(VOS_UINT32 ulEvent)
{
    if (1 == g_ulAdsULTaskReadyFlag)
    {
        (VOS_VOID)VOS_EventWrite(g_ulAdsULTaskId, ulEvent);
    }

    return;
}


VOS_VOID ADS_UL_ProcEvent(VOS_UINT32 ulEvent)
{
    if (ulEvent & ADS_UL_EVENT_DATA_PROC)
    {
        ADS_UL_WakeLock();
        ADS_UL_ProcLinkData();
        ADS_UL_WakeUnLock();
        ADS_DBG_UL_PROC_EVENT_NUM(1);
    }

    return;
}


VOS_VOID ADS_DL_SndEvent(VOS_UINT32 ulEvent)
{
    if (1 == g_ulAdsDLTaskReadyFlag)
    {
        (VOS_VOID)VOS_EventWrite(g_ulAdsDLTaskId, ulEvent);
    }

    return;
}


VOS_VOID ADS_DL_ProcEvent(VOS_UINT32 ulEvent)
{

    if (ulEvent & ADS_DL_EVENT_IPF_RD_INT)
    {
        ADS_DL_WakeLock();
        ADS_DL_ProcIpfResult();
        ADS_DL_WakeUnLock();
        ADS_DBG_DL_PROC_IPF_RD_EVENT_NUM(1);
    }

    if (ulEvent & ADS_DL_EVENT_IPF_ADQ_EMPTY_INT)
    {
        ADS_DL_AllocMemForAdq();
        ADS_DBG_DL_PROC_IPF_AD_EVENT_NUM(1);
    }

    if (ulEvent & ADS_DL_EVENT_IPF_FILTER_DATA_PROC)
    {
        ADS_DL_ProcIpfFilterQue();
    }

    return;
}


VOS_VOID ADS_DL_InitFcAssemParamInfo(VOS_VOID)
{
    ADS_DL_FC_ASSEM_STRU               *pstFcAssemInfo;

    pstFcAssemInfo = ADS_DL_GET_FC_ASSEM_INFO_PTR(ADS_INSTANCE_INDEX_0);

    pstFcAssemInfo->ulEnableMask     = VOS_FALSE;
    pstFcAssemInfo->ulFcActiveFlg    = VOS_FALSE;
    pstFcAssemInfo->ulTmrCnt         = ADS_GET_CURR_KERNEL_TIME();
    pstFcAssemInfo->ulRdCnt          = 0;
    pstFcAssemInfo->ulRateUpLev      = 0;
    pstFcAssemInfo->ulRateDownLev    = 0;
    pstFcAssemInfo->ulExpireTmrLen   = 0;
    pstFcAssemInfo->pFcAssemTuneFunc = VOS_NULL_PTR;

    return;
}


VOS_VOID ADS_DL_ResetFcAssemParamInfo(VOS_VOID)
{
    ADS_DL_FC_ASSEM_STRU               *pstFcAssemInfo;

    pstFcAssemInfo = ADS_DL_GET_FC_ASSEM_INFO_PTR(ADS_INSTANCE_INDEX_0);

    pstFcAssemInfo->ulFcActiveFlg    = VOS_FALSE;
    pstFcAssemInfo->ulRdCnt          = 0;

    return;
}


VOS_UINT32 ADS_UL_EnableRxWakeLockTimeout(VOS_UINT32 ulValue)
{
    ADS_IPF_CTX_STRU                   *pstIpfCntxt = VOS_NULL_PTR;

    pstIpfCntxt = ADS_GET_IPF_CTX_PTR();
    if (ulValue > pstIpfCntxt->ulRxWakeLockTimeout)
    {
        pstIpfCntxt->ulRxWakeLockTimeout = ulValue;
    }

    return 0;
}


VOS_UINT32 ADS_UL_WakeLockTimeout(VOS_VOID)
{
    ADS_IPF_CTX_STRU                   *pstIpfCntxt = VOS_NULL_PTR;
    VOS_UINT32                          ulRet;

    pstIpfCntxt = ADS_GET_IPF_CTX_PTR();
    ulRet       = pstIpfCntxt->ulRxWakeLockTimeout;

    if (0 != pstIpfCntxt->ulRxWakeLockTimeout)
    {
        wake_lock_timeout(&(pstIpfCntxt->stRxWakeLock),
                          (VOS_LONG)msecs_to_jiffies(pstIpfCntxt->ulRxWakeLockTimeout));
    }

    pstIpfCntxt->ulRxWakeLockTimeout = 0;

    return ulRet;
}


VOS_UINT32 ADS_UL_WakeLock(VOS_VOID)
{
    ADS_IPF_CTX_STRU                   *pstIpfCntxt = VOS_NULL_PTR;
    VOS_UINT32                          ulRet;

    pstIpfCntxt = ADS_GET_IPF_CTX_PTR();
    if (VOS_FALSE == pstIpfCntxt->ulWakeLockEnable)
    {
        return 0;
    }

    wake_lock(&(pstIpfCntxt->stUlBdWakeLock));
    pstIpfCntxt->ulUlBdWakeLockCnt++;

    ulRet = pstIpfCntxt->ulUlBdWakeLockCnt;
    return ulRet;
}


VOS_UINT32 ADS_UL_WakeUnLock(VOS_VOID)
{
    ADS_IPF_CTX_STRU                   *pstIpfCntxt = VOS_NULL_PTR;
    VOS_UINT32                          ulRet;

    pstIpfCntxt = ADS_GET_IPF_CTX_PTR();
    if (VOS_FALSE == pstIpfCntxt->ulWakeLockEnable)
    {
        return 0;
    }

    ADS_UL_WakeLockTimeout();

    wake_unlock(&(pstIpfCntxt->stUlBdWakeLock));
    pstIpfCntxt->ulUlBdWakeLockCnt--;

    ulRet = pstIpfCntxt->ulUlBdWakeLockCnt;
    return ulRet;
}


VOS_UINT32 ADS_DL_EnableTxWakeLockTimeout(VOS_UINT32 ulValue)
{
    ADS_IPF_CTX_STRU                   *pstIpfCntxt = VOS_NULL_PTR;

    pstIpfCntxt = ADS_GET_IPF_CTX_PTR();
    if (ulValue > pstIpfCntxt->ulTxWakeLockTimeout)
    {
        pstIpfCntxt->ulTxWakeLockTimeout = ulValue;
    }

    return 0;
}


VOS_UINT32 ADS_DL_WakeLockTimeout(VOS_VOID)
{
    ADS_IPF_CTX_STRU                   *pstIpfCntxt = VOS_NULL_PTR;
    VOS_UINT32                          ulRet;

    pstIpfCntxt = ADS_GET_IPF_CTX_PTR();
    ulRet       = pstIpfCntxt->ulTxWakeLockTimeout;

    if (0 != pstIpfCntxt->ulTxWakeLockTimeout)
    {
        wake_lock_timeout(&(pstIpfCntxt->stTxWakeLock),
                          (VOS_LONG)msecs_to_jiffies(pstIpfCntxt->ulTxWakeLockTimeout));
    }

    pstIpfCntxt->ulTxWakeLockTimeout = 0;

    return ulRet;
}


VOS_UINT32 ADS_DL_WakeLock(VOS_VOID)
{
    ADS_IPF_CTX_STRU                   *pstIpfCntxt = VOS_NULL_PTR;
    VOS_UINT32                          ulRet;

    pstIpfCntxt = ADS_GET_IPF_CTX_PTR();
    if (VOS_FALSE == pstIpfCntxt->ulWakeLockEnable)
    {
        return 0;
    }

    wake_lock(&(pstIpfCntxt->stDlRdWakeLock));
    pstIpfCntxt->ulDlRdWakeLockCnt++;

    ulRet = pstIpfCntxt->ulDlRdWakeLockCnt;
    return ulRet;
}


VOS_UINT32 ADS_DL_WakeUnLock(VOS_VOID)
{
    ADS_IPF_CTX_STRU                   *pstIpfCntxt = VOS_NULL_PTR;
    VOS_UINT32                          ulRet;

    pstIpfCntxt = ADS_GET_IPF_CTX_PTR();
    if (VOS_FALSE == pstIpfCntxt->ulWakeLockEnable)
    {
        return 0;
    }

    ADS_DL_WakeLockTimeout();

    wake_unlock(&(pstIpfCntxt->stDlRdWakeLock));
    pstIpfCntxt->ulDlRdWakeLockCnt--;

    ulRet = pstIpfCntxt->ulDlRdWakeLockCnt;
    return ulRet;
}


VOS_VOID ADS_IPF_MemMapRequset(
    IMM_ZC_STRU                        *pstImmZc,
    VOS_UINT32                          ulLen,
    VOS_UINT8                           ucIsIn
)
{
    VOS_UINT8                          *pucData = VOS_NULL_PTR;
    dma_addr_t                          ulDmaAddr;

    pucData   = IMM_ZcGetDataPtr(pstImmZc);
    ulDmaAddr = dma_map_single(ADS_GET_IPF_DEV(), pucData, ulLen,
                        (ucIsIn) ? DMA_TO_DEVICE : DMA_FROM_DEVICE);
    ADS_IPF_SetMemDma(pstImmZc, ulDmaAddr);
    return;
}


VOS_VOID ADS_IPF_MemMapByDmaRequset(
    IMM_ZC_STRU                        *pstImmZc,
    VOS_UINT32                          ulLen,
    VOS_UINT8                           ucIsIn
)
{
    VOS_UINT8                          *pucData = VOS_NULL_PTR;
    dma_addr_t                          ulDmaAddr;

    ulDmaAddr = ADS_IPF_GetMemDma(pstImmZc);
    pucData   = phys_to_virt(ulDmaAddr);
    dma_map_single(ADS_GET_IPF_DEV(), pucData, ulLen,
                        (ucIsIn) ? DMA_TO_DEVICE : DMA_FROM_DEVICE);
    return;
}


VOS_VOID ADS_IPF_MemUnmapRequset(
    IMM_ZC_STRU                        *pstImmZc,
    VOS_UINT32                          ulLen,
    VOS_UINT8                           ucIsIn
)
{
    dma_addr_t                          ulDmaAddr;

    ulDmaAddr = ADS_IPF_GetMemDma(pstImmZc);
    dma_unmap_single(ADS_GET_IPF_DEV(), ulDmaAddr, ulLen,
                        (ucIsIn) ? DMA_TO_DEVICE : DMA_FROM_DEVICE);
    return;
}


VOS_UINT32 ADS_IPF_IsSpeMem(IMM_ZC_STRU *pstImmZc)
{
    return VOS_FALSE;
}


VOS_VOID ADS_IPF_SetMemDma(IMM_ZC_STRU *pstImmZc, dma_addr_t ulDmaAddr)
{
    ADS_IMM_MEM_CB(pstImmZc)->ulDmaAddr = ulDmaAddr;
    return;
}


dma_addr_t ADS_IPF_GetMemDma(IMM_ZC_STRU *pstImmZc)
{
    return ADS_IMM_MEM_CB(pstImmZc)->ulDmaAddr;
}



IMM_ZC_STRU* ADS_IPF_AllocMem(VOS_UINT32 ulPoolId, VOS_UINT32 ulLen, VOS_UINT32 ulReserveLen)
{
    IMM_ZC_STRU                        *pstImmZc = VOS_NULL_PTR;

    pstImmZc = (IMM_ZC_STRU *)IMM_ZcStaticAlloc(ulLen);
    if (VOS_NULL_PTR == pstImmZc)
    {
        ADS_DBG_DL_ADQ_ALLOC_SYS_MEM_FAIL_NUM(1);
        return VOS_NULL_PTR;
    }

    ADS_DBG_DL_ADQ_ALLOC_SYS_MEM_SUCC_NUM(1);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 9, 0))
    /* ����˳��: ˢCACHE, Ԥ��ͷ���ռ� */
    ADS_IPF_DL_MEM_MAP(pstImmZc, ulLen);
    IMM_ZcReserve(pstImmZc, ulReserveLen);
#else
    /* ����˳��: Ԥ��ͷ���ռ䣬ˢCACHE  */
    IMM_ZcReserve(pstImmZc, ulReserveLen);
    ADS_IPF_DL_MEM_MAP(pstImmZc, ulLen);
#endif
    return pstImmZc;
}


VOS_SEM ADS_GetULResetSem(VOS_VOID)
{
    return g_stAdsCtx.hULResetSem;
}


VOS_SEM ADS_GetDLResetSem(VOS_VOID)
{
    return g_stAdsCtx.hDLResetSem;
}


VOS_UINT32 ADS_GetCurrentRate(
    VOS_UINT32                         *pulUlBpsRate,
    VOS_UINT32                         *pulDlBpsRate
)
{
    /* FCֻ��stick�ã�ֻ����ʵ��0�� */
    *pulUlBpsRate = g_stAdsCtx.stDsFlowStatsCtx.stULDataStats.ulULCurDataRate << 3;
    *pulDlBpsRate = g_stAdsCtx.stDsFlowStatsCtx.stDLDataStats.ulDLCurDataRate << 3;

    return VOS_OK;
}


VOS_UINT8 ADS_GetUlResetFlag(VOS_VOID)
{
    return g_stAdsCtx.ucUlResetFlag;
}


VOS_VOID ADS_SetUlResetFlag(VOS_UINT8 ucFlag)
{
    g_stAdsCtx.ucUlResetFlag = ucFlag;

    return;
}




ADS_UL_CTX_STRU* ADS_GetUlCtx(VOS_UINT8 ucInstanceIndex)
{
    ADS_SPEC_CTX_STRU                  *pstAdsSpecCtx = VOS_NULL_PTR;

    pstAdsSpecCtx = &(g_stAdsCtx.astAdsSpecCtx[ucInstanceIndex]);

    return &(pstAdsSpecCtx->stAdsUlCtx);
}


ADS_DL_CTX_STRU* ADS_GetDlCtx(VOS_UINT8 ucInstanceIndex)
{
    ADS_SPEC_CTX_STRU                  *pstAdsSpecCtx = VOS_NULL_PTR;

    pstAdsSpecCtx = &(g_stAdsCtx.astAdsSpecCtx[ucInstanceIndex]);

    return &(pstAdsSpecCtx->stAdsDlCtx);
}


ADS_TIMER_CTX_STRU* ADS_GetTiCtx(VOS_VOID)
{
    return g_stAdsCtx.astAdsTiCtx;
}


ADS_CTX_STRU* ADS_GetAllCtx(VOS_VOID)
{
    return &g_stAdsCtx;
}


VOS_VOID ADS_InitUlCtx(VOS_UINT8 ucInstanceIndex)
{
    VOS_UINT32                          i;
    VOS_UINT32                          ulRst;
    ADS_SPEC_CTX_STRU                  *pstAdsSpecCtx = VOS_NULL_PTR;

    pstAdsSpecCtx = &(g_stAdsCtx.astAdsSpecCtx[ucInstanceIndex]);

    /* Ĭ�ϴӵ�һ�����п�ʼ���� */
    pstAdsSpecCtx->stAdsUlCtx.ucAdsUlCurIndex      = 0;

    for (i = 0; i < ADS_RAB_ID_MAX + 1; i++)
    {
        pstAdsSpecCtx->stAdsUlCtx.astAdsUlQueue[i].pstAdsUlLink    = VOS_NULL_PTR;
        pstAdsSpecCtx->stAdsUlCtx.astAdsUlQueue[i].ucIsQueueValid  = VOS_FALSE;
        pstAdsSpecCtx->stAdsUlCtx.astAdsUlQueue[i].enPrio          = ADS_QCI_TYPE_BUTT;
        pstAdsSpecCtx->stAdsUlCtx.astAdsUlQueue[i].usRecordNum     = 0;
        pstAdsSpecCtx->stAdsUlCtx.astAdsUlQueue[i].enPktType       = ADS_CDS_IPF_PKT_TYPE_IP;

        /* Added by l60609 for AP������Ŀ ��2012-09-10 Begin */
        /* ����ʼ�� */
        VOS_SpinLockInit(&(pstAdsSpecCtx->stAdsUlCtx.astAdsUlQueue[i].stSpinLock));
        /* Added by l60609 for AP������Ŀ ��2012-09-10 End */
    }

    TAF_MEM_SET_S(pstAdsSpecCtx->stAdsUlCtx.aucPrioIndex, sizeof(pstAdsSpecCtx->stAdsUlCtx.aucPrioIndex), 0x00, ADS_RAB_NUM_MAX);

    /* ��NV�������ȼ���Ȩ����д��ADS�������� */
    ulRst = NV_ReadEx(ucInstanceIndex,
                      en_NV_Item_ADS_Queue_Scheduler_Pri,
                      &(pstAdsSpecCtx->stAdsUlCtx.stQueuePriNv),
                      sizeof(ADS_UL_QUEUE_SCHEDULER_PRI_NV_STRU));
    if(NV_OK != ulRst)
    {
        pstAdsSpecCtx->stAdsUlCtx.stQueuePriNv.ulStatus = VOS_FALSE;

        for (i = 0; i < ADS_UL_QUEUE_SCHEDULER_PRI_MAX; i++)
        {
            pstAdsSpecCtx->stAdsUlCtx.stQueuePriNv.ausPriWeightedNum[i] = ADS_UL_DEFAULT_PRI_WEIGHTED_NUM;
        }

        ADS_ERROR_LOG(ACPU_PID_ADS_UL, "ADS_InitUlCtx: NV read failed !");
    }

    if (VOS_FALSE == pstAdsSpecCtx->stAdsUlCtx.stQueuePriNv.ulStatus)
    {
        for (i = 0; i < ADS_UL_QUEUE_SCHEDULER_PRI_MAX; i++)
        {
            pstAdsSpecCtx->stAdsUlCtx.stQueuePriNv.ausPriWeightedNum[i] = ADS_UL_DEFAULT_PRI_WEIGHTED_NUM;
        }
    }

    /* Added by l60609 for AP������Ŀ ��2012-08-31 Begin */
    pstAdsSpecCtx->stAdsUlCtx.ulUlMaxQueueLength     = ADS_UL_MAX_QUEUE_LENGTH;
    /* Added by l60609 for AP������Ŀ ��2012-08-31 End */

    return;
}


VOS_VOID ADS_InitDlCtx(VOS_UINT8 ucInstance)
{
    ADS_SPEC_CTX_STRU                  *pstAdsSpecCtx = VOS_NULL_PTR;
    VOS_UINT32                          i;

    pstAdsSpecCtx = &(g_stAdsCtx.astAdsSpecCtx[ucInstance]);

    /* ��ʼ�����е�RAB��Ϣ */
    for (i = 0; i < ADS_RAB_NUM_MAX; i++)
    {
        pstAdsSpecCtx->stAdsDlCtx.astAdsDlRabInfo[i].ucRabId              = ADS_RAB_ID_INVALID;
        pstAdsSpecCtx->stAdsDlCtx.astAdsDlRabInfo[i].enPktType            = ADS_CDS_IPF_PKT_TYPE_IP;
        pstAdsSpecCtx->stAdsDlCtx.astAdsDlRabInfo[i].ulExParam            = 0;
        pstAdsSpecCtx->stAdsDlCtx.astAdsDlRabInfo[i].pRcvDlDataFunc       = VOS_NULL_PTR;
        pstAdsSpecCtx->stAdsDlCtx.astAdsDlRabInfo[i].pRcvDlFilterDataFunc = VOS_NULL_PTR;
        pstAdsSpecCtx->stAdsDlCtx.astAdsDlRabInfo[i].pstLstPkt            = VOS_NULL_PTR;
    }

    return;
}


VOS_VOID ADS_InitStatsInfoCtx(VOS_VOID)
{
    ADS_STATS_INFO_CTX_STRU            *pstDsFlowStatsCtx = VOS_NULL_PTR;

    pstDsFlowStatsCtx = ADS_GET_DSFLOW_STATS_CTX_PTR();

    pstDsFlowStatsCtx->stULDataStats.ulULPeriodSndBytes = 0;
    pstDsFlowStatsCtx->stULDataStats.ulULCurDataRate    = 0;
    pstDsFlowStatsCtx->stDLDataStats.ulDLPeriodRcvBytes = 0;
    pstDsFlowStatsCtx->stDLDataStats.ulDLCurDataRate    = 0;
}


VOS_VOID ADS_InitSpecCtx(VOS_VOID)
{
    VOS_UINT8                           i;

    for (i = 0; i < ADS_INSTANCE_MAX_NUM; i++)
    {
        /* ��ʼ������������ */
        ADS_InitUlCtx(i);

        /* ��ʼ������������ */
        ADS_InitDlCtx(i);
    }

    /* ��ʼ������̬������� */
    ADS_DL_InitFcAssemParamInfo();
}


VOS_VOID ADS_ResetSpecUlCtx(VOS_UINT8 ucInstance)
{
    VOS_UINT32                          i;
    VOS_UINT32                          ulRst;
    ADS_SPEC_CTX_STRU                  *pstAdsSpecCtx = VOS_NULL_PTR;

    pstAdsSpecCtx = &(g_stAdsCtx.astAdsSpecCtx[ucInstance]);

    /* Ĭ�ϴӵ�һ�����п�ʼ���� */
    pstAdsSpecCtx->stAdsUlCtx.ucAdsUlCurIndex      = 0;

    for (i = 0; i < ADS_RAB_ID_MAX + 1; i++)
    {
        pstAdsSpecCtx->stAdsUlCtx.astAdsUlQueue[i].pstAdsUlLink    = VOS_NULL_PTR;
        pstAdsSpecCtx->stAdsUlCtx.astAdsUlQueue[i].ucIsQueueValid  = VOS_FALSE;
        pstAdsSpecCtx->stAdsUlCtx.astAdsUlQueue[i].enPrio          = ADS_QCI_TYPE_BUTT;
        pstAdsSpecCtx->stAdsUlCtx.astAdsUlQueue[i].usRecordNum     = 0;
        pstAdsSpecCtx->stAdsUlCtx.astAdsUlQueue[i].enPktType       = ADS_CDS_IPF_PKT_TYPE_IP;
    }

    TAF_MEM_SET_S(pstAdsSpecCtx->stAdsUlCtx.aucPrioIndex, sizeof(pstAdsSpecCtx->stAdsUlCtx.aucPrioIndex), 0x00, ADS_RAB_NUM_MAX);

    /* ��NV�������ȼ���Ȩ����д��ADS�������� */
    ulRst = NV_ReadEx(ucInstance,
                      en_NV_Item_ADS_Queue_Scheduler_Pri,
                      &(pstAdsSpecCtx->stAdsUlCtx.stQueuePriNv),
                      sizeof(ADS_UL_QUEUE_SCHEDULER_PRI_NV_STRU));
    if(NV_OK != ulRst)
    {
        pstAdsSpecCtx->stAdsUlCtx.stQueuePriNv.ulStatus = VOS_FALSE;

        for (i = 0; i < ADS_UL_QUEUE_SCHEDULER_PRI_MAX; i++)
        {
            pstAdsSpecCtx->stAdsUlCtx.stQueuePriNv.ausPriWeightedNum[i] = ADS_UL_DEFAULT_PRI_WEIGHTED_NUM;
        }

        ADS_ERROR_LOG(ACPU_PID_ADS_UL, "ADS_InitUlCtx: NV read failed !");
    }

    if (VOS_FALSE == pstAdsSpecCtx->stAdsUlCtx.stQueuePriNv.ulStatus)
    {
        for (i = 0; i < ADS_UL_QUEUE_SCHEDULER_PRI_MAX; i++)
        {
            pstAdsSpecCtx->stAdsUlCtx.stQueuePriNv.ausPriWeightedNum[i] = ADS_UL_DEFAULT_PRI_WEIGHTED_NUM;
        }
    }

    pstAdsSpecCtx->stAdsUlCtx.ulUlMaxQueueLength     = ADS_UL_MAX_QUEUE_LENGTH;

    return;
}


VOS_VOID ADS_ResetSpecDlCtx(VOS_UINT8 ucInstance)
{
    ADS_SPEC_CTX_STRU                  *pstAdsSpecCtx  = VOS_NULL_PTR;
    VOS_UINT32                          i;

    pstAdsSpecCtx = &(g_stAdsCtx.astAdsSpecCtx[ucInstance]);

    /* �������е�RAB��Ϣ */
    for (i = 0; i < ADS_RAB_NUM_MAX; i++)
    {
        pstAdsSpecCtx->stAdsDlCtx.astAdsDlRabInfo[i].ucRabId              = ADS_RAB_ID_INVALID;
        pstAdsSpecCtx->stAdsDlCtx.astAdsDlRabInfo[i].enPktType            = ADS_CDS_IPF_PKT_TYPE_IP;
        pstAdsSpecCtx->stAdsDlCtx.astAdsDlRabInfo[i].ulExParam            = 0;
        pstAdsSpecCtx->stAdsDlCtx.astAdsDlRabInfo[i].pRcvDlDataFunc       = VOS_NULL_PTR;
        pstAdsSpecCtx->stAdsDlCtx.astAdsDlRabInfo[i].pRcvDlFilterDataFunc = VOS_NULL_PTR;
        pstAdsSpecCtx->stAdsDlCtx.astAdsDlRabInfo[i].pstLstPkt            = VOS_NULL_PTR;
    }

    return;
}


VOS_VOID ADS_ResetUlCtx(VOS_VOID)
{
    VOS_UINT8                           i;

    for (i = 0; i < ADS_INSTANCE_MAX_NUM; i++)
    {
        /* ��ʼ������������ */
        ADS_ResetSpecUlCtx(i);
    }

    /* ��ʼ������ͳ�Ƶ������� */
    ADS_InitStatsInfoCtx();

    return;
}


VOS_VOID ADS_ResetDlCtx(VOS_VOID)
{
    VOS_UINT8                           i;

    for (i = 0; i < ADS_INSTANCE_MAX_NUM; i++)
    {
        /* ��ʼ������������ */
        ADS_ResetSpecDlCtx(i);
    }

    /* ���õ���̬������� */
    ADS_DL_ResetFcAssemParamInfo();

    return;
}


VOS_VOID ADS_ResetIpfCtx(VOS_VOID)
{
    /* Ĭ���������ݷ��ͱ�����ʱ��ʱ��Ϊ10ms */
    g_stAdsCtx.stAdsIpfCtx.ulProtectTmrLen   = 10;

    /* Ĭ����������ͳ�ƶ�ʱ��ʱ��Ϊ100ms */
    g_stAdsCtx.stAdsIpfCtx.stUlAssemParmInfo.stThresholdStatInfo.ulStatTmrLen = 100;
    g_stAdsCtx.stAdsIpfCtx.stUlAssemParmInfo.stThresholdStatInfo.ulStatPktNum = 0;

    /* Ĭ���ܰ��������� */
    if (VOS_TRUE == g_stAdsCtx.stAdsIpfCtx.stUlAssemParmInfo.ulActiveFlag)
    {
        g_stAdsCtx.stAdsIpfCtx.ulThredHoldNum = ADS_UL_DATA_THRESHOLD_ONE;
    }
    else
    {
        g_stAdsCtx.stAdsIpfCtx.ulThredHoldNum = 32;
    }

    /* Ĭ�����ݲ��ڷ��� */
    g_stAdsCtx.stAdsIpfCtx.ucSendingFlg = VOS_FALSE;
}


VOS_VOID ADS_InitIpfCtx(VOS_VOID)
{
    ADS_UL_DYNAMIC_ASSEM_INFO_STRU     *pstUlAssemParmInfo = VOS_NULL_PTR;
    ADS_NV_DYNAMIC_THRESHOLD_STRU       stThreshold;
    TAF_NV_ADS_WAKE_LOCK_CFG_STRU       stWakeLockCfg;
    TAF_NV_ADS_IPF_MODE_CFG_STRU        stIpfMode;
    VOS_UINT32                          ulRet;
    VOS_UINT8                           i;

    for (i = 0; i < ADS_DL_ADQ_MAX_NUM; i++)
    {
        TAF_MEM_SET_S(g_stAdsCtx.stAdsIpfCtx.astIpfDlAdDesc[i], (VOS_SIZE_T)(IPF_DLAD0_DESC_SIZE * sizeof(IPF_AD_DESC_S)), 0x00, (VOS_SIZE_T)(IPF_DLAD0_DESC_SIZE * sizeof(IPF_AD_DESC_S)));
    }

    /* ��ʼ������Դ�ڴ��ͷŶ��� */
    IMM_ZcQueueHeadInit(&g_stAdsCtx.stAdsIpfCtx.stUlSrcMemFreeQue);

    /* ��ʼ������BD BUFF*/
    TAF_MEM_SET_S(g_stAdsCtx.stAdsIpfCtx.astIpfUlBdCfgParam, sizeof(g_stAdsCtx.stAdsIpfCtx.astIpfUlBdCfgParam), 0x00, (VOS_SIZE_T)(IPF_ULBD_DESC_SIZE * sizeof(IPF_CONFIG_ULPARAM_S)));

    /* ��ʼ������RD BUFF*/
    TAF_MEM_SET_S(g_stAdsCtx.stAdsIpfCtx.astIpfDlRdDesc, sizeof(g_stAdsCtx.stAdsIpfCtx.astIpfDlRdDesc), 0x00, (VOS_SIZE_T)(IPF_DLRD_DESC_SIZE * sizeof(IPF_RD_DESC_S)));

    TAF_MEM_SET_S(&stWakeLockCfg, sizeof(stWakeLockCfg), 0x00, sizeof(TAF_NV_ADS_WAKE_LOCK_CFG_STRU));

    /* Ĭ���������ݷ��ͱ�����ʱ��ʱ��Ϊ10ms */
    g_stAdsCtx.stAdsIpfCtx.ulProtectTmrLen   = 10;

    pstUlAssemParmInfo = &g_stAdsCtx.stAdsIpfCtx.stUlAssemParmInfo;

    TAF_MEM_SET_S(&stThreshold, sizeof(stThreshold), 0x00, (VOS_SIZE_T)sizeof(ADS_NV_DYNAMIC_THRESHOLD_STRU));

    ulRet = NV_ReadEx(MODEM_ID_0,
                      en_NV_Item_ADS_DYNAMIC_THRESHOLD_CFG,
                      &stThreshold,
                      sizeof(ADS_NV_DYNAMIC_THRESHOLD_STRU));
    if(NV_OK != ulRet)
    {
        pstUlAssemParmInfo->ulActiveFlag                      = VOS_FALSE;
        pstUlAssemParmInfo->ulProtectTmrExpCnt                = 0;
        pstUlAssemParmInfo->stWaterMarkLevel.ulWaterLevel1    = 80;
        pstUlAssemParmInfo->stWaterMarkLevel.ulWaterLevel2    = 150;
        pstUlAssemParmInfo->stWaterMarkLevel.ulWaterLevel3    = 500;
        pstUlAssemParmInfo->stWaterMarkLevel.ulWaterLevel4    = 0xFFFFFFFF;

        pstUlAssemParmInfo->stThresholdLevel.ulThreshold1     = 1;
        pstUlAssemParmInfo->stThresholdLevel.ulThreshold2     = 13;
        pstUlAssemParmInfo->stThresholdLevel.ulThreshold3     = 60;
        pstUlAssemParmInfo->stThresholdLevel.ulThreshold4     = 64;
        ADS_ERROR_LOG(ACPU_PID_ADS_UL, "ADS_InitIpfCtx: NV read failed !");
    }

    pstUlAssemParmInfo->ulActiveFlag                      = stThreshold.ulActiveFlag;
    pstUlAssemParmInfo->ulProtectTmrExpCnt                = stThreshold.ulProtectTmrExpCnt;
    pstUlAssemParmInfo->stWaterMarkLevel.ulWaterLevel1    = stThreshold.stWaterMarkLevel.ulWaterLevel1;
    pstUlAssemParmInfo->stWaterMarkLevel.ulWaterLevel2    = stThreshold.stWaterMarkLevel.ulWaterLevel2;
    pstUlAssemParmInfo->stWaterMarkLevel.ulWaterLevel3    = stThreshold.stWaterMarkLevel.ulWaterLevel3;
    pstUlAssemParmInfo->stWaterMarkLevel.ulWaterLevel4    = stThreshold.stWaterMarkLevel.ulWaterLevel4;

    pstUlAssemParmInfo->stThresholdLevel.ulThreshold1     = stThreshold.stThresholdLevel.ulThreshold1;
    pstUlAssemParmInfo->stThresholdLevel.ulThreshold2     = stThreshold.stThresholdLevel.ulThreshold2;
    pstUlAssemParmInfo->stThresholdLevel.ulThreshold3     = stThreshold.stThresholdLevel.ulThreshold3;
    pstUlAssemParmInfo->stThresholdLevel.ulThreshold4     = stThreshold.stThresholdLevel.ulThreshold4;

    /* Ĭ����������ͳ�ƶ�ʱ��ʱ��Ϊ100ms */
    pstUlAssemParmInfo->stThresholdStatInfo.ulStatTmrLen = 100;
    pstUlAssemParmInfo->stThresholdStatInfo.ulStatPktNum = 0;

    /* ��ʱʱ�����������Ҫ����jiffies������ʱ�� */
    if (0 != pstUlAssemParmInfo->ulProtectTmrExpCnt)
    {
        pstUlAssemParmInfo->ulProtectTmrCnt = ADS_GET_CURR_KERNEL_TIME();
    }

    /* Ĭ���ܰ��������� */
    if (VOS_TRUE == pstUlAssemParmInfo->ulActiveFlag)
    {
        g_stAdsCtx.stAdsIpfCtx.ulThredHoldNum = ADS_UL_DATA_THRESHOLD_ONE;
    }
    else
    {
        g_stAdsCtx.stAdsIpfCtx.ulThredHoldNum = 32;
    }

    /* Ĭ�����ݲ��ڷ��� */
    g_stAdsCtx.stAdsIpfCtx.ucSendingFlg = VOS_FALSE;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 9, 0))
    TAF_MEM_SET_S(&(g_stAdsCtx.stAdsIpfCtx.stDev), sizeof(g_stAdsCtx.stAdsIpfCtx.stDev), 0x00, (VOS_SIZE_T)sizeof(struct device));
    g_stAdsCtx.stAdsIpfCtx.stDev.dma_mask = &g_ullAdsDmaMask;
#endif

    wake_lock_init(&g_stAdsCtx.stAdsIpfCtx.stUlBdWakeLock, WAKE_LOCK_SUSPEND, "ipf_bd_wake");
    wake_lock_init(&g_stAdsCtx.stAdsIpfCtx.stDlRdWakeLock, WAKE_LOCK_SUSPEND, "ipf_rd_wake");

    wake_lock_init(&g_stAdsCtx.stAdsIpfCtx.stRxWakeLock, WAKE_LOCK_SUSPEND, "ads_rx_wake");
    wake_lock_init(&g_stAdsCtx.stAdsIpfCtx.stTxWakeLock, WAKE_LOCK_SUSPEND, "ads_tx_wake");

    g_stAdsCtx.stAdsIpfCtx.ulWakeLockEnable         = VOS_FALSE;

    g_stAdsCtx.stAdsIpfCtx.ulUlBdWakeLockCnt        = 0;
    g_stAdsCtx.stAdsIpfCtx.ulDlRdWakeLockCnt        = 0;

    g_stAdsCtx.stAdsIpfCtx.ulRxWakeLockTimeout      = 0;
    g_stAdsCtx.stAdsIpfCtx.ulTxWakeLockTimeout      = 0;

    g_stAdsCtx.stAdsIpfCtx.ulTxWakeLockTmrLen       = 500;
    g_stAdsCtx.stAdsIpfCtx.ulRxWakeLockTmrLen       = 500;

    ulRet = NV_ReadEx(MODEM_ID_0, en_NV_Item_ADS_WAKE_LOCK_CFG,
                      &stWakeLockCfg, sizeof(TAF_NV_ADS_WAKE_LOCK_CFG_STRU));
    if (NV_OK == ulRet)
    {
        g_stAdsCtx.stAdsIpfCtx.ulWakeLockEnable     = stWakeLockCfg.ulEnable;
        g_stAdsCtx.stAdsIpfCtx.ulTxWakeLockTmrLen   = stWakeLockCfg.ulTxWakeTimeout;
        g_stAdsCtx.stAdsIpfCtx.ulRxWakeLockTmrLen   = stWakeLockCfg.ulRxWakeTimeout;
    }

    ulRet = NV_ReadEx(MODEM_ID_0, en_NV_Item_ADS_IPF_MODE_CFG,
                      &stIpfMode, (VOS_UINT32)sizeof(TAF_NV_ADS_IPF_MODE_CFG_STRU));
    if (NV_OK == ulRet)
    {
        g_stAdsCtx.stAdsIpfCtx.ucIpfMode = stIpfMode.ucIpfMode;
    }

    IMM_ZcQueueHeadInit(ADS_GET_IPF_FILTER_QUE());


    return;
}


VOS_VOID ADS_InitTiCtx(VOS_VOID)
{
    VOS_UINT32                          i;

    for (i = 0; i < ADS_MAX_TIMER_NUM; i++)
    {
        g_stAdsCtx.astAdsTiCtx[i].hTimer        = VOS_NULL_PTR;
    }

    return;
}


VOS_VOID ADS_InitResetSem(VOS_VOID)
{
    g_stAdsCtx.hULResetSem  = VOS_NULL_PTR;
    g_stAdsCtx.hDLResetSem  = VOS_NULL_PTR;

    /* ����������ź��� */
    if (VOS_OK != VOS_SmBCreate( "UL", 0, VOS_SEMA4_FIFO, &g_stAdsCtx.hULResetSem))
    {
        ADS_TRACE_HIGH("Create ADS acpu UL_CNF sem failed!\n");
        ADS_DBG_UL_RESET_CREATE_SEM_FAIL_NUM(1);
        return;
    }

    if (VOS_OK != VOS_SmBCreate( "DL", 0, VOS_SEMA4_FIFO, &g_stAdsCtx.hDLResetSem))
    {
        ADS_TRACE_HIGH("Create ADS acpu DL_CNF sem failed!\n");
        ADS_DBG_DL_RESET_CREATE_SEM_FAIL_NUM(1);
        return;
    }

    return;
}


VOS_VOID ADS_InitCtx(VOS_VOID)
{
    TAF_MEM_SET_S(&g_stAdsStats, sizeof(g_stAdsStats), 0x00, sizeof(g_stAdsStats));

    /* ��ʼ��ÿ��ʵ���������� */
    ADS_InitSpecCtx();

    /* ��ʼ������ͳ�Ƶ������� */
    ADS_InitStatsInfoCtx();

    /* ��ʼ��IPF��ص������� */
    ADS_InitIpfCtx();

    /* ��ʼ����ʱ�������� */
    ADS_InitTiCtx();

    /* ��ʼ����λ�ź��� */
    ADS_InitResetSem();

    /* ��ʼ��ADS������������ */
    ADS_FILTER_InitCtx();

    /* ��ʼ����ǰʵ������ֵ */
    g_stAdsCtx.ucAdsCurInstanceIndex = ADS_INSTANCE_INDEX_0;

    return;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 0))

VOS_INT32 ADS_PlatDevProbe(struct platform_device *pstDev)
{
    g_pstDmaDev = &(pstDev->dev);
    dma_set_mask_and_coherent(g_pstDmaDev, DMA_BIT_MASK(64));

    printk(KERN_ERR"ADS_PlatDevProbe: dma mask = 0x%llx, coherent_dma_mask = 0x%llx, archdata.dma_coherent = %d.\r\n",
           *(g_pstDmaDev->dma_mask), g_pstDmaDev->coherent_dma_mask, g_pstDmaDev->archdata.dma_coherent);

    return 0;
}


VOS_INT32 ADS_PlatDevRemove(struct platform_device *pstDev)
{
    return 0;
}

static const struct of_device_id g_stAdsPlatDevOfMatch[] = {
	{
		.compatible = "hisilicon,hisi-ads",
	},
	{ },
};

static struct platform_driver g_stAdsPlatDevDriver = {
	.probe	= ADS_PlatDevProbe,
	.remove	= ADS_PlatDevRemove,
	.driver	= {
		.name = "hisi-ads",
		.of_match_table = of_match_ptr(g_stAdsPlatDevOfMatch),
	},
};


VOS_INT32 __init ADS_PlatDevInit(void)
{
	return platform_driver_register(&g_stAdsPlatDevDriver);
}


VOS_VOID __exit ADS_PlatDevExit(void)
{
	platform_driver_unregister(&g_stAdsPlatDevDriver);
}

module_init(ADS_PlatDevInit);
module_exit(ADS_PlatDevExit);
#endif


