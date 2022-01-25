/*
 *
 * All rights reserved.
 *
 * This software is available to you under a choice of one of two
 * licenses. You may choose this file to be licensed under the terms
 * of the GNU General Public License (GPL) Version 2 or the 2-clause
 * BSD license listed below:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */



/******************************************************************************
   1 ͷ�ļ�����
******************************************************************************/
#include "PPP/Inc/ppp_public.h"
#include "PPP/Inc/ppp_init.h"
#include "PPP/Inc/hdlc_interface.h"

/*****************************************************************************
    Э��ջ��ӡ��㷽ʽ�µ�.C�ļ��궨��
*****************************************************************************/
/*lint -e767  �����־�ļ���ID���� */
#define    THIS_FILE_ID                 PS_FILE_ID_PPP_PUBLIC_C
/*lint +e767   */


/******************************************************************************
   2 �ⲿ������������
******************************************************************************/



/******************************************************************************
   3 ˽�ж���
******************************************************************************/


/******************************************************************************
   4 ȫ�ֱ�������
******************************************************************************/
extern PPP_DATA_Q_CTRL_ST               g_PppDataQCtrl;

/******************************************************************************
   5 ����ʵ��
******************************************************************************/

PPP_ZC_STRU * PPP_MemAlloc(VOS_UINT16 usLen, VOS_UINT16 usReserveLen)
{
    /* �ýӿ���������ʱ��Ҫ����MACͷ���ȣ�
      ������ADS�շ���ΪIP����Ϊ��NDIS��E5�������ݽṹͳһ����Ҫ����MACͷ��
      �㿽��ָ���C�˷��ص�ʱ��ͳһƫ�ƹ̶��ֽڣ��ҵ��㿽��ͷ����
    */
    /*
       ��������ʱ����������0��������USB�շ�����Ϊ�ֽ�����ʽ��PPP֡����MACͷ
    */
    PPP_ZC_STRU *pstMem = PPP_ZC_MEM_ALLOC(usLen + usReserveLen);


    if (VOS_NULL_PTR != pstMem)
    {
        if ( usReserveLen > 0)
        {
            /* �ճ��������ȣ���PPPģ��������ݳ�����usLen���������������δ��ֵǰ���� */
            PPP_ZC_RESERVE(pstMem, usReserveLen);

            /* �������������ܴ��� */
            g_PppDataQCtrl.stStat.ulMemAllocUplinkCnt++;

            /* ��������Э�̽׶��ͷŵ�������Դ */
            PPP_ZC_SET_DATA_APP(pstMem, (VOS_UINT16)(1 << 8) | (VOS_UINT16)PPP_PULL_PACKET_TYPE);
        }
        else
        {
            /* �������������ܴ��� */
            g_PppDataQCtrl.stStat.ulMemAllocDownlinkCnt++;

            /* ��������Э�̽׶��ͷŵ�������Դ */
            PPP_ZC_SET_DATA_APP(pstMem, (VOS_UINT16)(1 << 8) | (VOS_UINT16)PPP_PUSH_PACKET_TYPE);
        }
    }
    else
    {
        if ( usReserveLen > 0)
        {
            /* ������������ʧ�ܴ��� */
            g_PppDataQCtrl.stStat.ulMemAllocUplinkFailCnt++;
        }
        else
        {
            /* ������������ʧ�ܴ��� */
            g_PppDataQCtrl.stStat.ulMemAllocDownlinkFailCnt++;
        }
    }

    return pstMem;
}


VOS_VOID PPP_MemWriteData(PPP_ZC_STRU *pstMem, VOS_UINT8 *pucSrc, VOS_UINT16 usLen)
{
    /* ���úý�Ҫд���㿽���ڴ��������ݳ��� */
    PPP_ZC_SET_DATA_LEN(pstMem, usLen);

    /* �������ڴ����ݲ��� */
    PPP_MemSingleCopy(PPP_ZC_GET_DATA_PTR(pstMem), pucSrc, usLen);

    return;
}


PPP_ZC_STRU * PPP_MemCopyAlloc(VOS_UINT8 *pSrc, VOS_UINT16 usLen, VOS_UINT16 usReserveLen)
{
    PPP_ZC_STRU                        *pstMem = VOS_NULL_PTR;


    pstMem = PPP_MemAlloc(usLen, usReserveLen);

    if ( VOS_NULL_PTR != pstMem )
    {
        /* �������ڴ����ݲ��� */
        PPP_MemWriteData(pstMem, pSrc, usLen);
    }

    return pstMem;
}


VOS_UINT32 PPP_MemCutTailData
(
    PPP_ZC_STRU **ppMemSrc,
    VOS_UINT8 *pucDest,
    VOS_UINT16 usLen,
    VOS_UINT16 usReserveLen
)
{
    PPP_ZC_STRU                        *pCurrMem;
    VOS_UINT16                          usCurrLen;
    VOS_UINT16                          usCurrOffset;


    /* ������� */
    if ( (VOS_NULL_PTR == ppMemSrc) ||
         (VOS_NULL_PTR == *ppMemSrc) ||
         (VOS_NULL_PTR == pucDest))
    {
        PPP_MNTN_LOG2(PS_PID_APP_PPP, 0, PS_PRINT_WARNING,
                      "PPP_MemCutTailData input parameters error, \
                      src addr'addr: 0X%p, dest addr: 0X%p\r\n",
                      (VOS_UINT_PTR)ppMemSrc, (VOS_UINT_PTR)pucDest);

        return PS_FAIL;
    }

    pCurrMem    = (PPP_ZC_STRU *)(*ppMemSrc);
    usCurrLen   = PPP_ZC_GET_DATA_LEN(pCurrMem);

    if ( ( 0 == usLen) || (usCurrLen < usLen) )
    {
        PPP_MNTN_LOG2(PS_PID_APP_PPP, 0, PS_PRINT_WARNING,
                      "PPP_MemCutTailData, Warning, usCurrLen %d Less Than usLen %d!\r\n",
                      usCurrLen, usLen);

        return PS_FAIL;
    }

    /* ��β�������������ݣ�ֻ����һ���ڵ� */
    usCurrOffset = usCurrLen - usLen;

    mdrv_memcpy(pucDest, &(PPP_ZC_GET_DATA_PTR(pCurrMem)[usCurrOffset]), (VOS_ULONG)usLen);

    if ( usCurrOffset > 0 )
    {
        /* ����ʣ�����ݣ�Ŀǰû�����㳤�Ȳ���Tailָ��ǰ�ƵĽӿڣ��������� */
        (*ppMemSrc) = PPP_MemCopyAlloc(PPP_ZC_GET_DATA_PTR(pCurrMem), usCurrOffset, usReserveLen);
    }
    else
    {
        (*ppMemSrc) = VOS_NULL_PTR;
    }

    /* �ͷ��ڴ� */
    PPP_MemFree(pCurrMem);

    return PS_SUCC;
}


VOS_UINT32 PPP_MemCutHeadData
(
    PPP_ZC_STRU **ppMemSrc,
    VOS_UINT8 *pucDest,
    VOS_UINT16 usDataLen
)
{
    PPP_ZC_STRU                        *pCurrMem;
    VOS_UINT16                          usMemSrcLen;


    if ( (VOS_NULL_PTR == ppMemSrc) ||
         (VOS_NULL_PTR == *ppMemSrc) ||
         (VOS_NULL_PTR == pucDest) )
    {
        PPP_MNTN_LOG2(PS_PID_APP_PPP, 0, LOG_LEVEL_WARNING,
                     "PPP_MemCutHeadData input parameters error, \
                     src addr'addr: 0x%p, dest addr: 0x%p\r\n",
                    (VOS_UINT_PTR)ppMemSrc, (VOS_UINT_PTR)pucDest);

        return PS_FAIL;
    }

    /* �ж�TTF�ڴ��ĳ����Ƿ����Ҫ�� */
    pCurrMem        = (PPP_ZC_STRU *)(*ppMemSrc);
    usMemSrcLen     = PPP_ZC_GET_DATA_LEN(pCurrMem);

    if ( ( 0 == usDataLen) || (usMemSrcLen < usDataLen) )
    {
        PPP_MNTN_LOG2(PS_PID_APP_PPP, 0, LOG_LEVEL_WARNING,
                      "PPP_MemCutHeadData, Warning: usMemSrcLen: %d Less Than usDataLen: %d!\r\n",
                      usMemSrcLen, usDataLen);

        return PS_FAIL;
    }

    /* ��ͷ�������������ݣ�ֻ����һ���ڵ� */
    mdrv_memcpy(pucDest, PPP_ZC_GET_DATA_PTR(pCurrMem), (VOS_ULONG)usDataLen);

    if ( usMemSrcLen >  usDataLen)
    {
        /* ����ʣ�����ݣ���������ָ��ͳ��� */
        PPP_ZC_REMOVE_HDR(pCurrMem, usDataLen);
    }
    else
    {
        /* �ͷ�ԭʼ�ڴ� */
        PPP_MemFree(pCurrMem);
        (*ppMemSrc) = VOS_NULL_PTR;
    }

    return PS_SUCC;
}


VOS_UINT32 PPP_MemGet(PPP_ZC_STRU *pMemSrc, VOS_UINT16 usOffset, VOS_UINT8 *pDest, VOS_UINT16 usLen)
{
    VOS_UINT16                          usMemSrcLen;


    /* ������� */
    if ( (VOS_NULL_PTR == pMemSrc)||(VOS_NULL_PTR == pDest) )
    {
        PPP_MNTN_LOG(PS_PID_APP_PPP, 0, PS_PRINT_WARNING,
                     "PPP_MemGet, Warning, Input Par pMemSrc Or pDest is Null!\r\n");

        return PS_FAIL;
    }

    if ( 0 == usLen )
    {
        PPP_MNTN_LOG(PS_PID_APP_PPP, 0, PS_PRINT_WARNING,
                     "PPP_MemGet, Warning, Input Par usLen is 0!\r\n");

        return PS_FAIL;
    }

    /* �ж�TTF�ڴ��ĳ����Ƿ����Ҫ�� */
    usMemSrcLen = PPP_ZC_GET_DATA_LEN(pMemSrc);

    if ( usMemSrcLen < (usOffset + usLen) )
    {
        PPP_MNTN_LOG2(PS_PID_APP_PPP, 0, PS_PRINT_WARNING,
                      "PPP_MemGet, Warning, MemSrcLen %d Less Than (Offset + Len) %d!\r\n",
                      usMemSrcLen, (usOffset + usLen));

        return PS_FAIL;
    }

    mdrv_memcpy(pDest, PPP_ZC_GET_DATA_PTR(pMemSrc) + usOffset, (VOS_ULONG)usLen);

    return PS_SUCC;
}


VOS_VOID PPP_MemFree(PPP_ZC_STRU *pstMem)
{
    /* �ͷ��㿽���ڴ� */
    PPP_ZC_MEM_FREE(pstMem);

    g_PppDataQCtrl.stStat.ulMemFreeCnt++;

    return;
}


VOS_VOID PPP_MemSingleCopy(VOS_UINT8 *pucDest, VOS_UINT8 *pucSrc, VOS_UINT32 ulLen)
{
    /* ���޸�ΪEDMA���� */
    mdrv_memcpy(pucDest, pucSrc, (VOS_ULONG)ulLen);

    return;
}


VOS_UINT32 PPP_GenerateSeed
(
    VOS_UINT8                           aucSerial[],
    VOS_UINT8                           ucSerialLen,
    VOS_UINT32                          ulStartIndex,
    VOS_UINT32                          ulDisturbaceFactor
)
{

    VOS_UINT32                          ulLoop;
    VOS_UINT32                          ulSeed;
    VOS_UINT8                          *pucSeed;
    VOS_UINT8                           ucTempValue = 0xFF;


    /* �����������ѡ�ĸ��ֽ���Ϊ���� */
    pucSeed = (VOS_UINT8 *)&ulSeed;
    for ( ulLoop = 0; ulLoop < sizeof(ulSeed); ulLoop++ )
    {
        *pucSeed++ = aucSerial[(VOS_UINT8)((ulStartIndex + ulLoop) % ucSerialLen)];
    }

    /* ���� */
    ulSeed += ulDisturbaceFactor;

    /* �滻���ֽ� */
    pucSeed = (VOS_UINT8 *)&ulSeed;
    for ( ulLoop = 0; ulLoop < sizeof(ulSeed); ulLoop++ )
    {
        if (0 == *pucSeed)
        {
            *pucSeed = ucTempValue--;
        }
        pucSeed++;
    }

    return ulSeed;
}


VOS_VOID PPP_Get16ByteSerial
(
    VOS_UINT8                           aucSerial[]
)
{
    VOS_UINT8                           aucSrcSerial[4][4];
    VOS_UINT32                          ulTick      = VOS_GetTick();
    VOS_UINT32                          ulSlice     = VOS_GetSlice();
    VOS_UINT32                          ulTaskId    = VOS_GetCurrentTaskID();
    VOS_UINT32                          ulStatSum;

    ulStatSum   = 0xECA8642F;

    /* ���ϵͳ����Ϊ0��������������� */
    ulTick = (0x00UL != ulTick) ? ulTick : 0x12345678;
    ulSlice = (0x00UL != ulSlice) ? ulSlice : 0x9ABCDEF1;
    ulTaskId = (0x00UL != ulTaskId) ? ulTaskId : 0xFDB97531;

    /* ��ϵͳ������ϳ�һ������ */
    PSACORE_MEM_CPY(&(aucSrcSerial[0][0]), sizeof(ulTick), &ulTick, sizeof(ulTick));
    PSACORE_MEM_CPY(&(aucSrcSerial[1][0]), sizeof(ulSlice), &ulSlice, sizeof(ulSlice));
    PSACORE_MEM_CPY(&(aucSrcSerial[2][0]), sizeof(ulTaskId), &ulTaskId, sizeof(ulTaskId));
    PSACORE_MEM_CPY(&(aucSrcSerial[3][0]), sizeof(ulStatSum), &ulStatSum, sizeof(ulStatSum));

    /* ���о���任�����µ��������*/
    PPP_MATRIX_TRANSFORM(aucSerial, aucSrcSerial);

    return;
}


VOS_VOID PPP_GetSecurityRand
(
    VOS_UINT8                           ucRandByteLen,
    VOS_UINT8                          *pucRand
)
{
    VOS_UINT8                           aucDictionary[256] = {0};
    VOS_UINT8                           aucSerial[16] = {0};
    VOS_UINT8                           ucValue = 0;
    VOS_UINT8                           ucTempValue;
    VOS_UINT32                          ulSeed;
    VOS_UINT32                          ulRandIndex;
    VOS_UINT32                          ulSwapIndex;
    VOS_UINT32                          ulLoop;
    VOS_UINT32                          ulStart;
    VOS_UINT32                          ulRandSum;
    const VOS_UINT32                    ulMax = 0x100;/* ���ڻ�ȡ�����, �����ȡֵ��Χ��[0x00..0xFF] */


    /***************************************************************************
     ����������ѧ�ڶ����ɵ�������ԭ����Ҫ��˼���ǽ����������ͨ��һ���Ľ�����
     ����������˼���������˿��Ƶ�ϴ�ơ������˿����Ǵ�С�����˳��źõģ�ͨ
     ����ε�ϴ��֮���˿��Ƶ�˳���Ծ�����ʧ�������������仰˵��ϵͳ����ֵ��
     �󡣶������ֹ����ǲ�����ģ�ֻ�ܴ����������򣬲��ܴ�����������ʹ��һ
     Щϵͳ���ݣ�ʱ��ֵ��TICKֵ���ڴ�ʹ������ϵͳ���á��жϡ���/�߳�ID��������
     ����ԣ�����һ����ʽ����ȡ����Ũ��������������������������������������
     �������������������
    ****************************************************************************/

    /***************************************************************************
     ���ɰ�ȫ�������Ϊ����:
     1�������ֵ�
     2�������ֵ�Ԫ��
    ****************************************************************************/

    /* ��ȡ16byte������� */
    PPP_Get16ByteSerial(aucSerial);

    /* �������� */
    ulSeed = PPP_GenerateSeed(aucSerial, (VOS_UINT8)sizeof(aucSerial), 0, 0);
    VOS_SetSeed(ulSeed);

    /***************************************************************************
     1�������ֵ�
    ****************************************************************************/
    /* a�����������ʼλ��, ���ɵķ�Χ:[0..0xFF] */
    ulStart = VOS_Rand(ulMax);
    ulRandSum = ulStart;

    /* b�������ֵ�ĺ�벿��: [ulStart��0xFF] */
    for (ulLoop = ulStart; ulLoop < ulMax; ulLoop++)
    {
        aucDictionary[ulLoop] = ucValue;
        ucValue++;
    }

    /* c�������ֵ��ǰ�벿��: [0, ulStart) */
    for (ulLoop = 0; ulLoop < ulStart; ulLoop++)
    {
        aucDictionary[ulLoop] = ucValue;
        ucValue++;
    }

    /***************************************************************************
     2�������ֵ�Ԫ��
       ����ucRandByteLen�ֽ���������У��������±�ulLoop = 0��ʼ��
       ��������ֵ�Ԫ��(ulLoop��[ulLoop, 0xFF]����)�������ֵ����С�
    ****************************************************************************/
    for (ulLoop = 0; ulLoop < ucRandByteLen; ulLoop++)
    {
        /* �������� */
        ulSeed = PPP_GenerateSeed(aucSerial, (VOS_UINT8)sizeof(aucSerial), ulLoop, ulRandSum);
        VOS_SetSeed(ulSeed);

        /* ���������, ���ɵķ�Χ:[0..0xFF] */
        ulRandIndex                 = VOS_Rand(ulMax);
        ulRandSum                  += ulRandIndex;

        /* ���㽻����λ�ã���Χ:[ulLoop..0xFF] */
        ulSwapIndex                 = (ulRandIndex % (ulMax - ulLoop)) + ulLoop;

        /* ����aucDictionary[ulLoop]��aucDictionary[ulSwapIndex] */
        ucTempValue                 = aucDictionary[ulLoop];
        aucDictionary[ulLoop]       = aucDictionary[ulSwapIndex];
        aucDictionary[ulSwapIndex]  = ucTempValue;
    }

    /* ��ȡucRandByteLen�ֽ�������� */
    PSACORE_MEM_CPY(pucRand, ucRandByteLen, &(aucDictionary[0]), ucRandByteLen);

    return;
}




