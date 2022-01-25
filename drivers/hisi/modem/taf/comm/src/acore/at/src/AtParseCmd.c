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
#include "ATCmdProc.h"
#include "AtCheckFunc.h"
#include "AtParseCmd.h"
#include "at_common.h"




/*****************************************************************************
    Э��ջ��ӡ��㷽ʽ�µ�.C�ļ��궨��
*****************************************************************************/
#define    THIS_FILE_ID        PS_FILE_ID_AT_PARSECMD_C

/*****************************************************************************
   2 ȫ�ֱ�������
*****************************************************************************/

/*****************************************************************************
   3 ��������������
*****************************************************************************/

/*****************************************************************************
   4 ����ʵ��
*****************************************************************************/


AT_STATE_TYPE_ENUM atFindNextSubState( AT_SUB_STATE_STRU *pSubStateTab,VOS_UINT8 ucInputChar)
{
    VOS_UINT16 usTabIndex = 0;                            /* ��״̬������ */

    /* ���αȽ���״̬��ÿһ��ֱ������ */
    while(AT_BUTT_STATE != pSubStateTab[usTabIndex].next_state)
    {
        if( AT_SUCCESS == pSubStateTab[usTabIndex].pFuncName(ucInputChar))    /* �ж������ַ��Ƿ�ƥ�� */
        {
            return pSubStateTab[usTabIndex].next_state;     /* ����ƥ�����״̬ */
        }
        usTabIndex++;                                               /* ��״̬���������� */
    }
    return AT_BUTT_STATE;
}


AT_STATE_TYPE_ENUM atFindNextMainState(AT_MAIN_STATE_STRU *pMainStateTab,
    VOS_UINT8 ucInputChar,  AT_STATE_TYPE_ENUM InputState)
{
    VOS_UINT16 usTabIndex = 0;                            /* ��״̬������ */

    /* ���αȽ���״̬��ÿһ��ֱ������ */
    while(AT_BUTT_STATE != pMainStateTab[usTabIndex].curr_state)
    {
        if( InputState == pMainStateTab[usTabIndex].curr_state)    /* �ж�����״̬�Ƿ�ƥ�� */
        {
            /* ���״̬ƥ��,����������ַ�Ѱ����һ����״̬ */
            return atFindNextSubState(pMainStateTab[usTabIndex].pSubStateTab,ucInputChar);
        }
        usTabIndex++;
    }
    return AT_BUTT_STATE;
}



TAF_UINT32 At_Auc2ul(TAF_UINT8 *nptr,TAF_UINT16 usLen,TAF_UINT32 *pRtn)
{
    TAF_UINT32 c     = 0;         /* current Char */
    TAF_UINT32 total = 0;         /* current total */
    TAF_UINT8 Length = 0;         /* current Length */

    c = (TAF_UINT32)*nptr++;

    while(Length++ < usLen)
    {
        if((c >= '0') && (c <= '9'))                /* �ַ���� */
        {
            /* 0xFFFFFFFF = 4294967295 */
            if(((total == 429496729) && (c > '5')) || (total > 429496729))
            {
                return AT_FAILURE;
            }
            total = (10 * total) + (c - '0');        /* accumulate digit */
            c = (TAF_UINT32)(TAF_UINT8)*nptr++;    /* get next Char */
        }
        else
        {
            return AT_FAILURE;
        }
    }

    *pRtn = total;   /* return result, negated if necessary */
    return AT_SUCCESS;
}

TAF_UINT32 At_String2Hex(TAF_UINT8 *nptr,TAF_UINT16 usLen,TAF_UINT32 *pRtn)
{
    TAF_UINT32 c     = 0;         /* current Char */
    TAF_UINT32 total = 0;         /* current total */
    TAF_UINT8 Length = 0;         /* current Length */

    c = (TAF_UINT32)*nptr++;

    while(Length++ < usLen)
    {
        if( (c  >= '0') && (c  <= '9') )
        {
            c  = c  - '0';
        }
        else if( (c  >= 'a') && (c  <= 'f') )
        {
            c  = (c  - 'a') + 10;
        }
        else if( (c  >= 'A') && (c  <= 'F') )
        {
            c  = (c  - 'A') + 10;
        }
        else
        {
            return AT_FAILURE;
        }

        if(total > 0x0FFFFFFF)              /* ������ת */
        {
            return AT_FAILURE;
        }
        else
        {
            total = (total << 4) + c;              /* accumulate digit */
            c = (TAF_UINT32)(TAF_UINT8)*nptr++;    /* get next Char */
        }
    }

    *pRtn = total;   /* return result, negated if necessary */
    return AT_SUCCESS;
}

TAF_UINT32 At_RangeToU32(TAF_UINT8 * pucBegain, TAF_UINT8 * pucEnd)
{
    TAF_UINT32 c;                                   /* current Char */
    TAF_UINT32 total = 0;                           /* current total */

    /* ���������� */
    if(pucBegain >= pucEnd)
    {
        return total;
    }

    /* �ӵ�һ���ַ���ʼ */
    c = (TAF_UINT32)*pucBegain;

    /* �����ۼ�*10���,ֱ������ */
    while( (pucBegain != pucEnd) && ( (c >= '0') && (c <= '9') ))
    {
        total = (10 * total) + (c - '0');             /* accumulate digit */
        pucBegain++;                                /* ע�⣬�����ڸ�ֵ֮ǰ��λ�����򣬱���ֵ���� */
        c = (TAF_UINT32)(TAF_UINT8)*pucBegain;      /* get next Char */

        if(total >= 0x19999998)                     /* �������0x19999998��ֱ�ӷ��أ�����ת */
        {
            return total;
        }
    }

    return total;
}
/*****************************************************************************
 Prototype      : At_RangeCopy
 Description    : ���ַ����е�ĳһ�ο�����ָ����ַ,pDstָʾĿ�ĵ�ַ,pucBegain
                  ָʾ��ʼ��ַ,pEndָʾ������ַ
 Input          : pucDst    --- Ŀ�ĵ�ַ
                  pucBegain --- ��ת���ִ��Ŀ�ʼ��ַ
                  pucEnd    --- ��ת���ִ��Ľ�����ַ
 Output         : ---
 Return Value   : ---
 Calls          : ---
 Called By      : ---

 History        : ---
  1.Date        : 2005-04-19
    Author      : ---
    Modification: Created function
*****************************************************************************/
TAF_VOID At_RangeCopy(TAF_UINT8 *pucDst,TAF_UINT8 * pucBegain, TAF_UINT8 * pucEnd)
{
    /* ���ο�����Ŀ�ĵ�ַ,ֱ������ */
    while(pucBegain < pucEnd)
    {
        *pucDst++ = *pucBegain++;
    }
}

TAF_UINT32 At_UpString(TAF_UINT8 *pData,TAF_UINT16 usLen)
{
    TAF_UINT8  *pTmp  = pData;                 /* current Char */
    TAF_UINT16 ChkLen = 0;

    if(0 == usLen)
    {
        return AT_FAILURE;
    }

    while(ChkLen++ < usLen)
    {
        if ( (*pTmp >= 'a') && (*pTmp <= 'z'))
        {
            *pTmp = *pTmp - 0x20;
        }
        pTmp++;
    }
    return AT_SUCCESS;
}



VOS_UINT32 atRangeToU32( VOS_UINT8 *pucBegain, VOS_UINT8 *pucEnd)
{
    VOS_UINT32 total = 0;                           /* current total */
    VOS_UINT32 ulRst;

    /* ���������� */
    if(pucBegain >= pucEnd)
    {
        return total;
    }

    ulRst = atAuc2ul(pucBegain, (VOS_UINT16)(pucEnd - pucBegain), &total);

    if(AT_SUCCESS != ulRst)
    {
        total = 0;
    }

    return total;
}


VOS_VOID atRangeCopy( VOS_UINT8 *pucDst, VOS_UINT8 * pucBegain, VOS_UINT8 * pucEnd)
{
    /* ���ο�����Ŀ�ĵ�ַ,ֱ������ */
    while(pucBegain < pucEnd)
    {
        *pucDst++ = *pucBegain++;
    }
}

/******************************************************************************
 ��������: ��ʮ�������ַ���ת���޷�������ֵ

 ����˵��:
   nptr [in/out] ������ַ�������ָ��
   usLen [in] ������ַ�������
   pRtn [in/out] ���ַ���ת����������ֵ

 �� �� ֵ:
    AT_FAILURE: �����ַ������з������ַ�������ֵ���
    AT_SUCCESS: �ɹ�
******************************************************************************/
static VOS_UINT32 auc2ulHex( VOS_UINT8 *nptr, VOS_UINT16 usLen,  VOS_UINT32 *pRtn)
{
    VOS_UINT8 c         = 0;         /* current Char */
    VOS_UINT32 total    = 0;         /* current total */
    VOS_UINT16 usLength = 2;         /* current Length */
    VOS_UINT8 *pcTmp    = nptr + 2;  /* ��0x��ʼ�Ƚ� */

    /* ����ָ���ɵ����߱�֤��ΪNULL, �ô������ж� */

    c = *pcTmp++;

    while(usLength++ < usLen)
    {
        /* 0xFFFFFFFF */
        if(total > 0xFFFFFFF)
        {
            return AT_FAILURE;
        }

        /* �ַ���� */
        if(isdigit(c))
        {
            total = AT_CHECK_BASE_HEX * total + (c - '0');        /* accumulate digit */
            c = *pcTmp++;    /* get next Char */
        }
        else if('A' <= c && 'F' >= c)
        {
            total = AT_CHECK_BASE_HEX * total + (c - 'A' + 10);        /* accumulate digit */
            c = *pcTmp++;    /* get next Char */
        }
        else if('a' <= c && 'f' >= c)
        {
            total = AT_CHECK_BASE_HEX * total + (c - 'a' + 10);        /* accumulate digit */
            c = *pcTmp++;    /* get next Char */
        }
        else
        {
            return AT_FAILURE;
        }
    }

    *pRtn = total;   /* return result, negated if necessary */

    return AT_SUCCESS;
}


/******************************************************************************
 ��������: ��ʮ�����ַ���ת���޷�������ֵ

 ����˵��:
   nptr [in/out] ������ַ�������ָ��
   usLen [in] ������ַ�������
   pRtn [in/out] ���ַ���ת����������ֵ

 �� �� ֵ:
    AT_FAILURE: �����ַ������з������ַ�������ֵ���
    AT_SUCCESS: �ɹ�
******************************************************************************/
static VOS_UINT32 auc2ulDec( VOS_UINT8 *nptr, VOS_UINT16 usLen,  VOS_UINT32 *pRtn)
{
    VOS_UINT32 c        = 0;         /* current Char */
    VOS_UINT32 total    = 0;         /* current total */
    VOS_UINT16 usLength = 0;         /* current Length */
    VOS_UINT8 *pcTmp    = nptr;      /* ��0x��ʼ�Ƚ� */

    /* ����ָ���ɵ����߱�֤��ΪNULL, �ô������ж� */

    c = (VOS_UINT32)*pcTmp++;

    while(usLength++ < usLen)
    {
        /* �ַ���� */
        if(isdigit(c))
        {
            /* 0xFFFFFFFF = 4294967295 */
            if(((total == 429496729) && (c > '5')) || (total > 429496729))
            {
                return AT_FAILURE;
            }

            total = AT_CHECK_BASE_DEC * total + (c - '0');        /* accumulate digit */
            c = (VOS_UINT32)(VOS_UINT8)*pcTmp++;    /* get next Char */
        }
        else
        {
            return AT_FAILURE;
        }
    }

    *pRtn = total;   /* return result, negated if necessary */

    return AT_SUCCESS;
}

/******************************************************************************
 ��������: ���ַ���ת���޷�������ֵ

 ����˵��:
   nptr [in/out] ������ַ�������ָ��
   usLen [in] ������ַ�������
   pRtn [in/out] ���ַ���ת����������ֵ

 �� �� ֵ:
    AT_FAILURE: �����ַ������з������ַ�������ֵ���
    AT_SUCCESS: �ɹ�
******************************************************************************/
VOS_UINT32 atAuc2ul( VOS_UINT8 *nptr,VOS_UINT16 usLen, VOS_UINT32 *pRtn)
{
    /* ����ú���ǰ�����в����ѽ��м�飬��֤��ΪNULL */

    if(NULL == nptr || 0 == usLen || NULL == pRtn)
    {
        return AT_FAILURE;
    }

    if('0' == *nptr)
    {
        if(2 < usLen && (('x' == *(nptr + 1)) || ('X' == *(nptr + 1))))
        {
            return auc2ulHex(nptr, usLen, pRtn);
        }
        else
        {
        }
    }

    return auc2ulDec(nptr, usLen, pRtn);
}


VOS_VOID At_ul2Auc(VOS_UINT32 ulValue,TAF_UINT16 usLen,VOS_UINT8 *pRtn)
{
    VOS_UINT32                          ulTempValue;
        
    if (0 == usLen)
    {
        return;
    }

    while(0 != ulValue)
    {
        ulTempValue = ulValue % 10;
        ulValue /=10;
        *(pRtn + usLen - 1) = '0' + (VOS_UINT8)ulTempValue;
        usLen--;

        if (0 == usLen)
        {
            return;
        }
    }

    while(0 < usLen)
    {
        *(pRtn + usLen - 1) = '0';
        usLen--;
    }
    
    return;
}


VOS_VOID* At_HeapAllocD(VOS_UINT32 ulSize)
{
    VOS_VOID* ret = NULL;

    if((ulSize == 0) || (ulSize > (1024*1024)))
    {
        return NULL;
    }

    ret = (VOS_VOID *)kmalloc(ulSize, GFP_KERNEL);

    return ret;
}


VOS_VOID At_HeapFreeD(VOS_VOID *pAddr)
{
    if(pAddr == NULL)
    {
        return ;
    }

    kfree(pAddr);

    return;
}

