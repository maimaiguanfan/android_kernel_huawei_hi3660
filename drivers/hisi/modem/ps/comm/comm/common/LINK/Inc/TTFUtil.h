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

#ifndef __TTF_UTIL_H__
#define __TTF_UTIL_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/******************************************************************************
  1 ����ͷ�ļ�����
******************************************************************************/
#include "vos.h"


/******************************************************************************
  2 �궨��
******************************************************************************/
#define TTF_Q_CNT(pQ)           ((pQ)->ulCnt)

#define TTF_MBUF_REMOVE_FROM_HDR(pMbuf, usNum)  \
{                                               \
    (pMbuf)->pData  += (usNum);                 \
    (pMbuf)->usLen  -= (usNum);                 \
}

#define TTF_MBUF_RESUME_TO_HDR(pMbuf, usNum)    \
{                                               \
    (pMbuf)->pData  -= (usNum);                 \
    (pMbuf)->usLen  += (usNum);                 \
}

#define TTF_MAX_IPV4_ADDR_LEN   15

#define TTF_REG_MAX_BIT         (31)

#define TTF_COPYLINK_UL_MAX_NUM    (100)
#define TTF_COPYLINK_DL_MAX_NUM    (250)

/* TTF SYS ERR ID */
#define TTF_MEM_RB_TASK_FAIL_SEM_ERR    (0x30000002)
#define TTF_CICOM_SYSTEM_ERROR_ID       (0x30000003)
#define TTF_MEM_ALLOC_FAIL_SYSTEM_ERROR (0x30000004)
#define TTF_MEM_COPY_ALLOC_FAIL_ERROR   (0x30000005)
#define TTF_MEM_SET_FAIL_ERROR          (0x30000006)  /* �����ڴ��쳣��MEM_SET��ȫ�������µĸ�λ */
#define TTF_MEM_CPY_FAIL_ERROR          (0x30000007)  /* �����ڴ��쳣��MEM_CPY��ȫ�������µĸ�λ */
#define TTF_MEM_MOVE_FAIL_ERROR         (0x30000008)  /* �����ڴ��쳣��MEM_MOVE��ȫ�������µĸ�λ */

/* CTTF SYS ERR ID */
#define TTF_NODE_ALLOC_SYSTEM_ERROR_ID  (0x3F000001)
#define PS_QNODE_GET_SYSTEM_ERROR_ID    (0x3F000002)
#define HDLC_DEF_SYSTEM_ERROR_ID        (0x3F000003)
#define HDLC_FRM_SYSTEM_ERROR_ID        (0x3F000004)
#define TTF_DDR_RAM_BASE_ADDR_ERROR_ID  (0x3F000005)
#define TTF_HRPD_EDMA_ERROR_ID          (0x3F000006)
#define TTF_MAC_LOW_POWER_ERROR         (0x3F000007)  /* �͹����·��ʵ��������¸�λ */
#define CTTF_TIMER_CHECK_ERROR_ID       (0x3F000008)
#define CTTF_MEM_OP_ERROR_ID            (0x3F000009)  /* �ڴ����ʧ�ܵ��¸�λ */


/* ǿ��ARMִ��ǰ������ָ����ARMָ����������
   ʹ��OM�ӿ�������L1,L2 Cache�򿪣���Ҫflush buffer������ */
#define TTF_FORCE_ARM_INSTUCTION()      VOS_FlushCpuWriteBuf()

/******************************************************************************
  3 ö�ٶ���
******************************************************************************/


/******************************************************************************
  4 ȫ�ֱ�������
******************************************************************************/


/******************************************************************************
  5 ��Ϣͷ����
******************************************************************************/


/******************************************************************************
  6 ��Ϣ����
******************************************************************************/


/******************************************************************************
  7 STRUCT����
******************************************************************************/
#pragma pack(4)

/*================================*/        /* TTF��̬���й����� */
typedef struct _TTF_Q_LINK_ST
{
    struct _TTF_Q_LINK_ST  *pNext;
} TTF_Q_LINK_ST;                            /* TTF���нڵ㶨��*/

typedef struct _TTF_Q_HDR_LINK_ST
{
    TTF_Q_LINK_ST          *pHead;          /* ����ͷָ�� */
    TTF_Q_LINK_ST          *pTail;          /* ����βָ�� */
} TTF_Q_HDR_LINK_ST;

typedef struct
{
    TTF_Q_HDR_LINK_ST       stHdr;
    VOS_UINT32              ulCnt;
} TTF_Q_ST;


/*================================*/        /* TTF�ڴ������ */
/*============================*/            /* TTF�ڴ浥Ԫ���� */
typedef struct _TTF_MBUF_ST
{
    TTF_Q_LINK_ST           stLink;         /* ���нڵ� */
    struct _TTF_MBUF_ST    *pNext;          /*�����ݵ���һ��*/
    VOS_UINT16              usLen;          /*���ݵ��ܳ��ȣ�����һ��ȫ����ʹ��*/
    VOS_UINT16              usUsed;         /*�Ѿ�ʹ�õ����ݳ���*/
    VOS_UINT8              *pData;          /*������ݵ�ָ�룬������ָ��ṹ��ĺ����ڴ�*/
} TTF_MBUF_ST;

/*TTF���ȼ��ṹ����*/
typedef struct
{
    VOS_UINT16              usMaxCnt;
    VOS_UINT16              usHighCnt;
    VOS_UINT16              usMidCnt;
    VOS_UINT16              usLowCnt;
    TTF_Q_ST                stQ;
} TTF_STREAM_ST;


/*****************************************************************************
 �ṹ��    : TTF_COPYLINK_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : ��ʽ�������ݽṹ
*****************************************************************************/
typedef struct _TTF_COPYLINK_NODE_STRU
{
    VOS_UINT8                          *pucSrcData;
    VOS_UINT8                          *pucDstData;
    VOS_UINT32                          ulDataLen;
} TTF_COPYLINK_NODE_STRU;

typedef struct
{
    TTF_COPYLINK_NODE_STRU              astCopyLinkNodes[TTF_COPYLINK_DL_MAX_NUM];
    VOS_UINT32                          ulCopyNodeCnt;
}TTF_COPYLINK_DL_STRU;

typedef struct
{
    TTF_COPYLINK_NODE_STRU              astCopyLinkNodes[TTF_COPYLINK_UL_MAX_NUM];
    VOS_UINT32                          ulCopyNodeCnt;
}TTF_COPYLINK_UL_STRU;



#define    TTF_CPY_LINK_REG_DL(ulPid, pucRegDataDst, pucRegDataSrc, usRegDataLen)        TTF_CopyLink_DL_Reg(ulPid, pucRegDataDst, pucRegDataSrc, usRegDataLen, &g_stTtfCopyLinkDL)
#define    TTF_CPY_LINK_REG_UL(ulPid, pucRegDataDst, pucRegDataSrc, usRegDataLen)        TTF_CopyLink_UL_Reg(ulPid, pucRegDataDst, pucRegDataSrc, usRegDataLen, &g_stTtfCopyLinkUL)

#define    TTF_CPY_LINK_EXECUTE_DL(ulPid)                                                TTF_CopyLink_DL_Execute(ulPid, &g_stTtfCopyLinkDL)
#define    TTF_CPY_LINK_EXECUTE_UL(ulPid)                                                TTF_CopyLink_UL_Execute(ulPid, &g_stTtfCopyLinkUL)

#define    TTF_CPY_LINK_DEL_DL()                                                        TTF_CopyLink_DL_Rel(&g_stTtfCopyLinkDL)
#define    TTF_CPY_LINK_DEL_UL()                                                        TTF_CopyLink_UL_Rel(&g_stTtfCopyLinkUL)

#define    TTF_CPY_LINK_INIT_UL()                                                       (VOS_VOID)TTF_CopyLink_UL_Init(&g_stTtfCopyLinkUL)
#define    TTF_CPY_LINK_INIT_DL()                                                       (VOS_VOID)TTF_CopyLink_DL_Init(&g_stTtfCopyLinkDL)

/*****************************************************************************
 �ṹ��    : TTF_ACCORE_MEM_SET_SOFT_REBOOT_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : TTF A�� PSACORE_MEM_SET ��ȫ������λ��Ϣ����ṹ
*****************************************************************************/
typedef struct
{
    VOS_UINT8           *pucToSet;
    VOS_UINT32          ulDestSize;
    VOS_INT8            cChar;
    VOS_UINT8           ucRsv[3];
    VOS_UINT32          ulCount;
}TTF_ACCORE_MEM_SET_SOFT_REBOOT_STRU;

/*****************************************************************************
 �ṹ��    : TTF_ACCORE_MEM_CPY_SOFT_REBOOT_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : TTF A�� PSACORE_MEM_CPY ��ȫ������λ��Ϣ����ṹ
*****************************************************************************/
typedef struct
{
    VOS_UINT8           *pucDest;
    VOS_UINT32          ulDestSize;
    VOS_UINT8           *pucSrc;
    VOS_UINT32          ulCount;
}TTF_ACCORE_MEM_CPY_SOFT_REBOOT_STRU;

/*****************************************************************************
 �ṹ��    : TTF_ACCORE_MEM_MOVE_SOFT_REBOOT_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : TTF A�� PSACORE_MEM_MOVE ��ȫ������λ��Ϣ����ṹ
*****************************************************************************/
typedef struct
{
    VOS_UINT8           *pucDest;
    VOS_UINT32          ulDestSize;
    VOS_UINT8           *pucSrc;
    VOS_UINT32          ulCount;
}TTF_ACCORE_MEM_MOVE_SOFT_REBOOT_STRU;



#define PSACORE_MEM_SET(ToSet, ulDestSize, Char, Count) \
                PSACORE_MEM_SET_EX(ToSet, (VOS_SIZE_T)(ulDestSize), Char, (VOS_SIZE_T)(Count), (THIS_FILE_ID), (__LINE__))


#define PSACORE_MEM_CPY(Dest, ulDestSize, Src, Count) \
                PSACORE_MEM_CPY_EX(Dest, (VOS_SIZE_T)(ulDestSize), Src, (VOS_SIZE_T)(Count), (THIS_FILE_ID), (__LINE__))


#define PSACORE_MEM_MOVE(Dest, ulDestSize, Src, Count) \
                PSACORE_MEM_MOVE_EX(Dest, (VOS_SIZE_T)(ulDestSize), Src, (VOS_SIZE_T)(Count), (THIS_FILE_ID), (__LINE__))

#pragma pack()

/******************************************************************************
  8 UNION����
******************************************************************************/


/******************************************************************************
  9 OTHERS����
******************************************************************************/
#ifndef PCLINT_MEM
/*����ָ�����ȵ��ڴ浥Ԫ�����ݸ���TTF_BLK_ST�ṹ֮��*/
extern TTF_MBUF_ST *TTF_MbufNew(VOS_UINT32 ulPid, VOS_UINT16 usLen);

/*�ͷ�ָ��TTF_MBUF_ST�ṹ���ڴ�*/
extern VOS_VOID     TTF_MbufFree(VOS_UINT32 ulPid, TTF_MBUF_ST *pMem);

extern VOS_UINT32   TTF_QIn(VOS_UINT32 ulPid, TTF_Q_ST *pQ, TTF_Q_LINK_ST *pLink);
extern VOS_VOID    *TTF_QOut(VOS_UINT32 ulPid, TTF_Q_ST *pQ);
#endif


extern VOS_UINT16   TTF_MbufGetLen(VOS_UINT32 ulPid, TTF_MBUF_ST *pMem);
extern VOS_VOID     TTF_QLink(VOS_UINT32 ulPid, VOS_VOID *pItem, TTF_Q_LINK_ST *pLink);
extern VOS_VOID     TTF_QInit(VOS_UINT32 ulPid, TTF_Q_ST *pQ);
extern VOS_UINT32   TTF_QIsEmpty(VOS_UINT32 ulPid, TTF_Q_ST *pQ);
extern VOS_UINT32   TTF_QCnt(VOS_UINT32 ulPid, TTF_Q_ST *pQ);
extern VOS_UINT32   TTF_LenStr2IpAddr(VOS_UINT8* pucStr, VOS_UINT8 *pucIpAddr);

extern VOS_VOID     TTF_Write32RegByBit(VOS_UINT32 ulRegAddr, VOS_UINT8 ucStartBit, VOS_UINT8 ucEndBit,VOS_UINT32 ulSetValue);
extern VOS_UINT32   TTF_Read32RegByBit(VOS_UINT32 ulRegAddr, VOS_UINT8 ucStartBit, VOS_UINT8 ucEndBit);

extern VOS_UINT32 TTF_CopyLink_DL_Init(TTF_COPYLINK_DL_STRU *pstTtfCpyLink);
extern VOS_UINT32 TTF_CopyLink_DL_Reg
(
    VOS_UINT32                          ulPid,
    VOS_UINT8                          *pucRegDataDst,
    VOS_UINT8                          *pucRegDataSrc,
    VOS_UINT16                          usRegDataLen,
    TTF_COPYLINK_DL_STRU                  *pstTtfCpyLink
);
extern VOS_UINT32 TTF_CopyLink_DL_Execute(VOS_UINT32 ulPid, TTF_COPYLINK_DL_STRU *pstTtfCpyLink);
extern VOS_VOID  TTF_CopyLink_DL_Rel(TTF_COPYLINK_DL_STRU *pstTtfCpyLink );

extern VOS_UINT32 TTF_CopyLink_UL_Init(TTF_COPYLINK_UL_STRU *pstTtfCpyLink);
extern VOS_UINT32 TTF_CopyLink_UL_Reg
(
    VOS_UINT32                          ulPid,
    VOS_UINT8                          *pucRegDataDst,
    VOS_UINT8                          *pucRegDataSrc,
    VOS_UINT16                          usRegDataLen,
    TTF_COPYLINK_UL_STRU                  *pstTtfCpyLink
);
extern VOS_UINT32 TTF_CopyLink_UL_Execute(VOS_UINT32 ulPid, TTF_COPYLINK_UL_STRU *pstTtfCpyLink);
extern VOS_VOID  TTF_CopyLink_UL_Rel(TTF_COPYLINK_UL_STRU *pstTtfCpyLink );


extern VOS_VOID TTF_InsertSortAsc16bit(VOS_UINT32 ulPid, VOS_UINT16    ausSortElement[], VOS_UINT32 ulElementCnt, VOS_UINT32 ulMaxCnt);
extern VOS_VOID TTF_RemoveDupElement16bit(VOS_UINT32 ulPid, VOS_UINT16    ausSortElement[], VOS_UINT32 *pulElementCnt, VOS_UINT32 ulMaxCnt);
extern VOS_VOID TTF_RingBufWrite(VOS_UINT32 ulPid, VOS_UINT32 ulDstRingBufBaseAddr,
    VOS_UINT16 usOffset, VOS_UINT8 *pucSrcData, VOS_UINT16 usDataLen, VOS_UINT16 usModLen);


extern TTF_COPYLINK_DL_STRU     g_stTtfCopyLinkDL;
extern TTF_COPYLINK_UL_STRU     g_stTtfCopyLinkUL;

extern VOS_VOID PSACORE_MEM_SET_EX(VOS_VOID *ToSet, VOS_SIZE_T ulDestSize, VOS_CHAR Char, VOS_SIZE_T Count, VOS_UINT32 ulFileNo, VOS_UINT32 ulLineNo);
extern VOS_VOID PSACORE_MEM_CPY_EX(VOS_VOID *Dest, VOS_SIZE_T ulDestSize, const VOS_VOID *Src, VOS_SIZE_T Count, VOS_UINT32 ulFileNo, VOS_UINT32 ulLineNo);
extern VOS_VOID PSACORE_MEM_MOVE_EX(VOS_VOID *Dest, VOS_SIZE_T ulDestSize, const VOS_VOID *Src, VOS_SIZE_T Count, VOS_UINT32 ulFileNo, VOS_UINT32 ulLineNo);

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* TTFUtil.h */

