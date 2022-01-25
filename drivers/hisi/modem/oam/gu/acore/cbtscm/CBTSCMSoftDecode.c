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
**************************************************************************** */
#include "omprivate.h"
#include "PamOamSpecTaskDef.h"
#include "ombufmngr.h"
#include "OmHdlcInterface.h"
#include "CBTSCMSoftDecode.h"



#define    THIS_FILE_ID        PS_FILE_ID_CBT_SCM_SOFT_DECODE_C

/* ****************************************************************************
  2 ȫ�ֱ�������
**************************************************************************** */
/* ��������������CBT���ݽ��յ��ٽ���Դ���� */
VOS_SPINLOCK             g_stCbtScmDataRcvSpinLock;

/* HDLC���ƽṹ */
OM_HDLC_STRU             g_stCbtScmHdlcSoftDecodeEntity;

/* SCM���ݽ������ݻ����� */
VOS_CHAR                 g_aucCbtScmDataRcvBuffer[CBTSCM_DATA_RCV_PKT_SIZE];

/* SCM���ݽ���������ƽṹ */
CBTSCM_DATA_RCV_CTRL_STRU   g_stCbtScmDataRcvTaskCtrlInfo;

CBTSCM_SOFTDECODE_INFO_STRU   g_stCbtScmSoftDecodeInfo;


/*****************************************************************************
  3 �ⲿ��������
*****************************************************************************/
extern VOS_UINT32 CBT_AcpuRcvData(VOS_UINT8 *pucData, VOS_UINT32 ulSize);

/*****************************************************************************
  4 ����ʵ��
*****************************************************************************/


VOS_UINT32 CBTSCM_SoftDecodeDataRcvProc(VOS_UINT8 *pucBuffer, VOS_UINT32 ulLen)
{
    VOS_INT32                           sRet;

    if (ulLen > (VOS_UINT32)OM_RingBufferFreeBytes(g_stCbtScmDataRcvTaskCtrlInfo.rngOmRbufId))
    {
        g_stCbtScmSoftDecodeInfo.stRbInfo.ulBufferNotEnough++;

        return VOS_ERR;
    }

    sRet = OM_RingBufferPut(g_stCbtScmDataRcvTaskCtrlInfo.rngOmRbufId,
                            (VOS_CHAR *)pucBuffer,
                            (VOS_INT)ulLen);



    if (ulLen == (VOS_UINT32)sRet)
    {
        if (VOS_OK == VOS_SmV(g_stCbtScmDataRcvTaskCtrlInfo.SmID))
        {
            g_stCbtScmSoftDecodeInfo.stPutInfo.ulDataLen += sRet;
            g_stCbtScmSoftDecodeInfo.stPutInfo.ulNum++;

            return VOS_OK;
        }
    }

    return VOS_ERR;

}


VOS_UINT32 CBTSCM_SoftDecodeDataRcv(VOS_UINT8 *pucBuffer, VOS_UINT32 ulLen)
{
    VOS_UINT32                          ulRstl;
    VOS_ULONG                           ulLockLevel;

    VOS_SpinLockIntLock(&g_stCbtScmDataRcvSpinLock, ulLockLevel);

    ulRstl = CBTSCM_SoftDecodeDataRcvProc(pucBuffer, ulLen);

    VOS_SpinUnlockIntUnlock(&g_stCbtScmDataRcvSpinLock, ulLockLevel);

    return ulRstl;
}


VOS_UINT32 CBTSCM_SoftDecodeAcpuRcvData(
    OM_HDLC_STRU                       *pstHdlcCtrl,
    VOS_UINT8                          *pucData,
    VOS_UINT32                          ulLen)
{
    VOS_UINT32                          i;
    VOS_UINT32                          ulResult;
    VOS_UINT8                           ucChar;

    ulResult = VOS_ERR;

    for( i = 0; i < ulLen; i++ )
    {
        ucChar = (VOS_UINT8)pucData[i];

        ulResult = Om_HdlcDecap(pstHdlcCtrl, ucChar);

        if ( HDLC_SUCC == ulResult )
        {
            g_stCbtScmSoftDecodeInfo.stHdlcDecapData.ulDataLen += pstHdlcCtrl->ulInfoLen;
            g_stCbtScmSoftDecodeInfo.stHdlcDecapData.ulNum++;

            /* GU CBT���ݲ���ҪDATATYPE�ֶΣ�ɾ�� */
            if (VOS_OK != CBT_AcpuRcvData(pstHdlcCtrl->pucDecapBuff + 1,
                            pstHdlcCtrl->ulInfoLen - 1))
            {
            }
        }
        else if (HDLC_NOT_HDLC_FRAME == ulResult)
        {
            /*����������֡,����HDLC���װ*/
        }
        else
        {
            g_stCbtScmSoftDecodeInfo.ulFrameDecapErr++;
        }
    }

    return VOS_OK;
}


VOS_UINT32 CBTSCM_SoftDecodeHdlcInit(OM_HDLC_STRU *pstHdlc)
{
    /* ��������HDLC���װ�Ļ��� */
    pstHdlc->pucDecapBuff    = (VOS_UINT8 *)VOS_MemAlloc(ACPU_PID_CBT, STATIC_MEM_PT, CBTSCM_DATA_RCV_PKT_SIZE);

    if (VOS_NULL_PTR == pstHdlc->pucDecapBuff)
    {
        (VOS_VOID)vos_printf("CBTSCM_SoftDecodeHdlcInit: Alloc Decapsulate buffer fail!");

        return VOS_ERR;
    }

    /* HDLC���װ���泤�ȸ�ֵ */
    pstHdlc->ulDecapBuffSize = CBTSCM_DATA_RCV_PKT_SIZE;

    /* ��ʼ��HDLC���װ���������� */
    Om_HdlcInit(pstHdlc);

    return VOS_OK;
}


VOS_VOID CBTSCM_SoftDecodeReqRcvSelfTask(
    VOS_UINT32                          ulPara1,
    VOS_UINT32                          ulPara2,
    VOS_UINT32                          ulPara3,
    VOS_UINT32                          ulPara4)
{
    VOS_INT32                           sRet;
    VOS_INT32                           lLen;
    VOS_INT32                           lRemainlen;
    VOS_INT32                           lReadLen;
    VOS_UINT32                          ulPktNum;
    VOS_UINT32                          i;
    VOS_ULONG                           ulLockLevel;


    ulPara1 = ulPara1;
    ulPara2 = ulPara2;
    ulPara3 = ulPara3;
    ulPara4 = ulPara4;

    for (;;)
    {
        if (VOS_OK != VOS_SmP(g_stCbtScmDataRcvTaskCtrlInfo.SmID, 0))
        {
            continue;
        }

        lLen = OM_RingBufferNBytes(g_stCbtScmDataRcvTaskCtrlInfo.rngOmRbufId);

        if (lLen <= 0)
        {
            continue;
        }

        ulPktNum = (VOS_UINT32)((lLen + CBTSCM_DATA_RCV_PKT_SIZE - 1) / CBTSCM_DATA_RCV_PKT_SIZE);

        lRemainlen = lLen;

        for (i = 0; i < ulPktNum; i++)
        {
            if (CBTSCM_DATA_RCV_PKT_SIZE < lRemainlen)
            {
                lReadLen = CBTSCM_DATA_RCV_PKT_SIZE;

                sRet = OM_RingBufferGet(g_stCbtScmDataRcvTaskCtrlInfo.rngOmRbufId,
                                        g_stCbtScmDataRcvTaskCtrlInfo.pucBuffer,
                                        CBTSCM_DATA_RCV_PKT_SIZE);
            }
            else
            {
                lReadLen = lRemainlen;

                sRet = OM_RingBufferGet(g_stCbtScmDataRcvTaskCtrlInfo.rngOmRbufId,
                                        g_stCbtScmDataRcvTaskCtrlInfo.pucBuffer,
                                        lRemainlen);
            }

            if (sRet != lReadLen)
            {
                VOS_SpinLockIntLock(&g_stCbtScmDataRcvSpinLock, ulLockLevel);

                OM_RingBufferFlush(g_stCbtScmDataRcvTaskCtrlInfo.rngOmRbufId);

                VOS_SpinUnlockIntUnlock(&g_stCbtScmDataRcvSpinLock, ulLockLevel);

                g_stCbtScmSoftDecodeInfo.stRbInfo.ulRingBufferFlush++;

                continue;
            }

            lRemainlen -= lReadLen;

            /* ����HDLC���װ���� */
            if (VOS_OK != CBTSCM_SoftDecodeAcpuRcvData(&g_stCbtScmHdlcSoftDecodeEntity,
                                                        (VOS_UINT8 *)g_stCbtScmDataRcvTaskCtrlInfo.pucBuffer,
                                                        (VOS_UINT32)lReadLen))
            {
                (VOS_VOID)vos_printf("CBTSCM_SoftDecodeReqRcvSelfTask: CBTSCM_SoftDecodeAcpuRcvData Fail");

                g_stCbtScmSoftDecodeInfo.stGetInfo.ulDataLen += lReadLen;
            }
        }
    }
}


VOS_UINT32 CBTSCM_SoftDecodeReqRcvTaskInit(VOS_VOID)
{
    VOS_UINT32                              ulRslt;

    /* ע��CBT���ݽ����Դ������� */
    ulRslt = VOS_RegisterSelfTaskPrio(ACPU_FID_CBT,
                                      (VOS_TASK_ENTRY_TYPE)CBTSCM_SoftDecodeReqRcvSelfTask,
                                      SCM_DATA_RCV_SELFTASK_PRIO,
                                      SCM_CBT_REQ_TASK_STACK_SIZE);
    if ( VOS_NULL_BYTE == ulRslt )
    {
        return VOS_ERR;
    }

    PAM_MEM_SET_S(&g_stCbtScmSoftDecodeInfo, sizeof(g_stCbtScmSoftDecodeInfo), 0, sizeof(g_stCbtScmSoftDecodeInfo));

    if (VOS_OK != VOS_SmCCreate("CBT", 0, VOS_SEMA4_FIFO, &(g_stCbtScmDataRcvTaskCtrlInfo.SmID)))
    {
        (VOS_VOID)vos_printf("CBTSCM_SoftDecodeReqRcvTaskInit: Error, CBT semCCreate Fail");

        g_stCbtScmSoftDecodeInfo.stRbInfo.ulSemCreatErr++;

        return VOS_ERR;
    }

    if (VOS_OK != CBTSCM_SoftDecodeHdlcInit(&g_stCbtScmHdlcSoftDecodeEntity))
    {
        (VOS_VOID)vos_printf("CBTSCM_SoftDecodeReqRcvTaskInit: Error, HDLC Init Fail");

        return VOS_ERR;
    }

    g_stCbtScmDataRcvTaskCtrlInfo.rngOmRbufId = OM_RingBufferCreate(CBTSCM_DATA_RCV_BUFFER_SIZE);

    if (VOS_NULL_PTR == g_stCbtScmDataRcvTaskCtrlInfo.rngOmRbufId)
    {
        (VOS_VOID)vos_printf("CBTSCM_SoftDecodeReqRcvTaskInit: Error, Creat CBT ringBuffer Fail");

        g_stCbtScmSoftDecodeInfo.stRbInfo.ulRingBufferCreatErr++;

        (VOS_VOID)VOS_MemFree(ACPU_PID_CBT, g_stCbtScmHdlcSoftDecodeEntity.pucDecapBuff);

        return VOS_ERR;
    }

    g_stCbtScmDataRcvTaskCtrlInfo.pucBuffer = &g_aucCbtScmDataRcvBuffer[0];

    VOS_SpinLockInit(&g_stCbtScmDataRcvSpinLock);

    return VOS_OK;
}

VOS_VOID CBTSCM_SoftDecodeInfoShow(VOS_VOID)
{
    (VOS_VOID)vos_printf("\r\nSCM_SoftDecodeInfoShow:\r\n");

    (VOS_VOID)vos_printf("\r\nSem Creat Error %d:\r\n",                   g_stCbtScmSoftDecodeInfo.stRbInfo.ulSemCreatErr);
    (VOS_VOID)vos_printf("\r\nSem Give Error %d:\r\n",                    g_stCbtScmSoftDecodeInfo.stRbInfo.ulSemGiveErr);
    (VOS_VOID)vos_printf("\r\nRing Buffer Creat Error %d:\r\n",           g_stCbtScmSoftDecodeInfo.stRbInfo.ulRingBufferCreatErr);
    (VOS_VOID)vos_printf("\r\nTask Id Error %d:\r\n",                     g_stCbtScmSoftDecodeInfo.stRbInfo.ulTaskIdErr);
    (VOS_VOID)vos_printf("\r\nRing Buffer not Enough %d:\r\n",            g_stCbtScmSoftDecodeInfo.stRbInfo.ulBufferNotEnough);
    (VOS_VOID)vos_printf("\r\nRing Buffer Flush %d:\r\n",                 g_stCbtScmSoftDecodeInfo.stRbInfo.ulRingBufferFlush);
    (VOS_VOID)vos_printf("\r\nRing Buffer Put Error %d:\r\n",             g_stCbtScmSoftDecodeInfo.stRbInfo.ulRingBufferPutErr);

    (VOS_VOID)vos_printf("\r\nRing Buffer Put Success Times %d:\r\n",     g_stCbtScmSoftDecodeInfo.stPutInfo.ulNum);
    (VOS_VOID)vos_printf("\r\nRing Buffer Put Success Bytes %d:\r\n",     g_stCbtScmSoftDecodeInfo.stPutInfo.ulDataLen);

    (VOS_VOID)vos_printf("\r\nRing Buffer Get Success Times %d:\r\n",     g_stCbtScmSoftDecodeInfo.stGetInfo.ulNum);
    (VOS_VOID)vos_printf("\r\nRing Buffer Get Success Bytes %d:\r\n",     g_stCbtScmSoftDecodeInfo.stGetInfo.ulDataLen);

    (VOS_VOID)vos_printf("\r\nHDLC Decode Success Times %d:\r\n",         g_stCbtScmSoftDecodeInfo.stHdlcDecapData.ulNum);
    (VOS_VOID)vos_printf("\r\nHDLC Decode Success Bytes %d:\r\n",         g_stCbtScmSoftDecodeInfo.stHdlcDecapData.ulDataLen);

    (VOS_VOID)vos_printf("\r\nHDLC Decode Error Times %d:\r\n",           g_stCbtScmSoftDecodeInfo.ulFrameDecapErr);

    (VOS_VOID)vos_printf("\r\nHDLC Init Error Times %d:\r\n",             g_stCbtScmSoftDecodeInfo.ulHdlcInitErr);

    (VOS_VOID)vos_printf("\r\nHDLC Decode Data Type Error Times %d:\r\n", g_stCbtScmSoftDecodeInfo.ulDataTypeErr);
}





