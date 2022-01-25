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
#include "product_config.h"

#include "Fc.h"
#include "FcInterface.h"
#include "FcIntraMsg.h"
#include "FcACoreCReset.h"




/*****************************************************************************
    Э��ջ��ӡ��㷽ʽ�µ�.C�ļ��궨��
*****************************************************************************/
#define    THIS_FILE_ID        PS_FILE_ID_ACORE_CRESET_FLOW_CTRL_C

/*****************************************************************************
  2 ȫ�ֱ�������
*****************************************************************************/
VOS_SEM         g_ulFcACoreCResetDoneSem;    /* FcACore��ɻص������ź��� */

/*****************************************************************************
  3 ����ʵ��
*****************************************************************************/


VOS_UINT32  FC_ACORE_CResetSendNotify(FC_MSG_TYPE_ENUM_UINT16     usMsgName)
{
    FC_ACORE_CRESET_IND_STRU    *pstMsg;

    /* ������Ϣ�ڴ� */
    pstMsg = (FC_ACORE_CRESET_IND_STRU *) VOS_AllocMsg( UEPS_PID_FLOWCTRL_A,
        (VOS_UINT32)sizeof(FC_ACORE_CRESET_IND_STRU) - VOS_MSG_HEAD_LENGTH );

    if ( VOS_NULL_PTR == pstMsg )
    {
        FC_LOG(PS_PRINT_ERROR,"FC_ACORE_CResetSendNotify, Alloc Msg Fail\n");
        return VOS_ERR;
    }

    /* ��д��Ϣ���� */
    pstMsg->ulReceiverCpuId = VOS_LOCAL_CPUID;
    pstMsg->ulReceiverPid   = UEPS_PID_FLOWCTRL_A;
    pstMsg->usMsgName       = usMsgName;

    /* ������Ϣ */
    (VOS_VOID)VOS_SendMsg(UEPS_PID_FLOWCTRL, pstMsg);

    return VOS_OK;

}



VOS_INT FC_ACORE_CResetCallback(DRV_RESET_CB_MOMENT_E enParam, VOS_INT userdata)
{
    VOS_UINT32                   ulResult;


    if ( MDRV_RESET_CB_BEFORE == enParam )      /* ��λ����ʱ���� */
    {
        FC_ACORE_CResetSendNotify(ID_FC_ACORE_CRESET_START_IND);

        ulResult = VOS_SmP(g_ulFcACoreCResetDoneSem, FC_ACORE_CRESET_TIMEOUT_LEN);
        if (VOS_OK != ulResult)
        {
            FC_LOG1(PS_PRINT_ERROR,
                          "FC_ACORE_CRESET_Callback, wait g_ulFcACoreResetDoneSem timeout! ulResult = %d\r\n", (VOS_INT32)ulResult);

            return VOS_ERR;
        }
    }
    else if ( MDRV_RESET_CB_AFTER == enParam )   /* ��λ�ɹ������ */
    {
        FC_ACORE_CResetSendNotify(ID_FC_ACORE_CRESET_END_IND);
    }
    else
    {
        FC_LOG(PS_PRINT_ERROR,"FC_ACORE_CResetCallback, enParam invalid !\n");
        return VOS_ERR;
    }

    /* for lint 715 */
    (VOS_VOID)userdata;

    return VOS_OK;
}


VOS_UINT32 FC_ACORE_CResetProc(FC_ACORE_CRESET_MOMENT_ENUM_UINT8 enCResetMoment)
{
    VOS_UINT32                          ulFcPointLoop;
    FC_POINT_STRU                      *pFcPoint;
    VOS_UINT32                          ulResult;
    FC_MSG_TYPE_ENUM_UINT16             usMsgName;
    FC_ACORE_CRESET_RSP_STRU           *pstMsg;
    VOS_UINT32                          ulRspResult;


    ulRspResult         = 0;

    if ( FC_ACORE_CRESET_BEFORE_RESET == enCResetMoment )
    {
        usMsgName           = ID_FC_ACORE_CRESET_START_RSP;

        /* ��λ����ʱ����ÿ�����ص㣬ִ�����ص����ⲿģ��ע���reset���� */
        for ( ulFcPointLoop = 0; ulFcPointLoop < g_stFcPointMgr.ulPointNum; ulFcPointLoop++ )
        {
            pFcPoint    = &g_stFcPointMgr.astFcPoint[ulFcPointLoop];

            if ( pFcPoint->pRstFunc != VOS_NULL_PTR )
            {
                ulResult     = pFcPoint->pRstFunc(pFcPoint->ulParam1, pFcPoint->ulParam2);
                ulRspResult |= ulResult ;
                FC_MNTN_TracePointFcEvent(ID_FC_MNTN_ACORE_CRESET_START_FC, pFcPoint, VOS_TRUE, ulResult);
            }
        }
    }
    else if ( FC_ACORE_CRESET_AFTER_RESET == enCResetMoment )
    {
        usMsgName           = ID_FC_ACORE_CRESET_END_RSP;
    }
    else
    {
        FC_LOG(PS_PRINT_ERROR,"FC_ACORE_CResetProc, enCResetMoment invalid !\n");
        return VOS_ERR;
    }

    /* ������Ϣ�ڴ�: */
    pstMsg = (FC_ACORE_CRESET_RSP_STRU *)(VOS_UINT_PTR)VOS_AllocMsg( UEPS_PID_FLOWCTRL_A, 
                               (VOS_UINT32)sizeof(FC_ACORE_CRESET_RSP_STRU) - VOS_MSG_HEAD_LENGTH);

    if(VOS_NULL_PTR == pstMsg)
    {
        FC_LOG(PS_PRINT_ERROR,"FC_ACORE_CResetProc, Alloc Msg Fail\n");
        return VOS_ERR;
    }

    /*��д��Ϣ����:*/
    pstMsg->ulReceiverCpuId = VOS_LOCAL_CPUID;
    pstMsg->ulReceiverPid   = UEPS_PID_FLOWCTRL_A;
    pstMsg->usMsgName       = usMsgName;
    pstMsg->ulResult        = ulRspResult;

    /*������Ϣ:*/
    (VOS_VOID)VOS_SendMsg(UEPS_PID_FLOWCTRL, pstMsg);

    return VOS_OK;
}



VOS_VOID FC_ACORE_CResetRcvStartRsp(VOS_VOID)
{
    VOS_SmV(g_ulFcACoreCResetDoneSem);

    return;
}


