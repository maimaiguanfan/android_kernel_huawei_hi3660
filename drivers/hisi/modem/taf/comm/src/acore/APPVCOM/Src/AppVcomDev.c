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
#include "PsTypeDef.h"
#include "vos.h"
#include "PsCommonDef.h"
#include "AppVcomDev.h"
#include "TafTypeDef.h"
#include "AtTypeDef.h"
#include "AtMntn.h"





/*****************************************************************************
    Э��ջ��ӡ��㷽ʽ�µ�.C�ļ��궨��
*****************************************************************************/
#define    THIS_FILE_ID        PS_FILE_ID_APP_VCOM_DEV_C

/*****************************************************************************
   2 ȫ�ֱ�������
*****************************************************************************/

/* VCOM CTX,���ڱ���VCOM��ȫ�ֱ���*/
APP_VCOM_DEV_CTX_STRU                   g_astVcomCtx[APP_VCOM_MAX_NUM];

APP_VCOM_DEBUG_INFO_STRU                g_stAppVcomDebugInfo;


/* ���⴮���ļ������ӿ� */
static const struct file_operations     g_stOperations_Fops =
{
    .owner = THIS_MODULE,
    .read  = APP_VCOM_Read,
    .poll  = APP_VCOM_Poll,
    .write = APP_VCOM_Write,
    .open  = APP_VCOM_Open,
    .release = APP_VCOM_Release,
};


/* APPVCOM�Ĺ���Ӧ��
APPVCOM ID   �����С  ��;           �Ƿ�AT��CLIENT    ModemId
APPVCOM        4K      RIL(��)               ��         MODEM0
APPVCOM1       4K      RIL(����)             ��         MODEM0
APPVCOM2       4K      ���̲˵�              ��         MODEM0
APPVCOM3       8K      ����װ��(AT SERVER)   ��         MODEM0
APPVCOM4       4K      audio RIL             ��         MODEM0
APPVCOM5       4K      RIL(��)               ��         MODEM1
APPVCOM6       4K      RIL(����)             ��         MODEM1
APPVCOM7       8K      ����װ��(AT SERVER)   ��         MODEM1
APPVCOM8       4K      ���̲˵�/HIDP         ��         MODEM1
APPVCOM9       4K      AGPS                  ��         MODEM0
APPVCOM10      4K      NFC/BIP               ��         MODEM0
APPVCOM11      4K      ISDB                  ��         MODEM0
APPVCOM12      4K      AGPS                  ��         MODEM1
APPVCOM13      4K      RIL(��ѯ)             ��         MODEM0
APPVCOM14      4K      RIL(��ѯ)             ��         MODEM1
APPVCOM15      4K      NFC                   ��         MODEM1
APPVCOM16      4K      HIDP                  ��         MODEM0
APPVCOM17      4K      AGPS-AP               ��         MODEM0
APPVCOM18      4K      Ԥ��                  ��         MODEM0
APPVCOM19      4K      Ԥ��                  ��         MODEM0
APPVCOM20      4K      RIL(��)               ��         MODEM2
APPVCOM21      4K      RIL(����)             ��         MODEM2
APPVCOM22      4K      ���̲˵�              ��         MODEM2
APPVCOM23      8K      ����װ��(AT SERVER)   ��         MODEM2
APPVCOM24      4K      AGPS                  ��         MODEM2
APPVCOM25      4K      NFC                   ��         MODEM2
APPVCOM26      4K      RIL(��ѯ)             ��         MODEM2

APPVCOM27      4K      Ԥ��                  ��         MODEM0
APPVCOM28      4K      Ԥ��                  ��         MODEM0
APPVCOM29      4K      Ԥ��                  ��         MODEM0
APPVCOM30      4K      Ԥ��                  ��         MODEM0
APPVCOM31      4K      Ԥ��                  ��         MODEM0

APPVCOM32      4K      Ԥ��                  ��         MODEM0
APPVCOM33      4K      Ԥ��                  ��         MODEM0
APPVCOM34      4K      Ԥ��                  ��         MODEM0
APPVCOM35      4K      Ԥ��                  ��         MODEM0
APPVCOM36      4K      Ԥ��                  ��         MODEM0
APPVCOM37      4K      Ԥ��                  ��         MODEM0
APPVCOM38      4K      Ԥ��                  ��         MODEM0
APPVCOM39      4K      Ԥ��                  ��         MODEM0
APPVCOM40      4K      Ԥ��                  ��         MODEM0
APPVCOM41      4K      Ԥ��                  ��         MODEM0
APPVCOM42      4K      Ԥ��                  ��         MODEM0
APPVCOM43      4K      Ԥ��                  ��         MODEM0
APPVCOM44      4K      Ԥ��                  ��         MODEM0
APPVCOM45      4K      Ԥ��                  ��         MODEM0
APPVCOM46      4K      Ԥ��                  ��         MODEM0
APPVCOM47      4K      Ԥ��                  ��         MODEM0
APPVCOM48      4K      Ԥ��                  ��         MODEM0
APPVCOM49      4K      Ԥ��                  ��         MODEM0
APPVCOM50      4K      Ԥ��                  ��         MODEM0
APPVCOM51      4K      Ԥ��                  ��         MODEM0
APPVCOM52      4K      Ԥ��                  ��         MODEM0
APPVCOM53      128K    errlog                ��
APPVCOM54      4K      T/Lװ��               ��
APPVCOM55      2M      CBT                   ��         MODEM0
APPVCOM56      2M      log 3.5               ��
APPVCOM57      2M      log 3.5               ��
APPVCOM58      4K      Ԥ��                  ��         MODEM0
APPVCOM59      4K      Ԥ��                  ��         MODEM0
APPVCOM60      4K      Ԥ��                  ��         MODEM0
APPVCOM61      4K      Ԥ��                  ��         MODEM0
APPVCOM62      4K      Ԥ��                  ��         MODEM0
APPVCOM63      4K      Ԥ��                  ��         MODEM0
*/
const APP_VCOM_DEV_CONFIG_STRU g_astAppVcomCogfigTab[] =
{
    {APP_VCOM_DEV_NAME_0, APP_VCOM_SEM_NAME_0, 0x1000, 0},                      /* APPVCOM */
    {APP_VCOM_DEV_NAME_1, APP_VCOM_SEM_NAME_1, 0x1000, 0},                      /* APPVCOM1 */
    {APP_VCOM_DEV_NAME_2, APP_VCOM_SEM_NAME_2, 0x1000, 0},                      /* APPVCOM2 */
    {APP_VCOM_DEV_NAME_3, APP_VCOM_SEM_NAME_3, 0x2000, 0},                      /* APPVCOM3 */
    {APP_VCOM_DEV_NAME_4, APP_VCOM_SEM_NAME_4, 0x1000, 0},                      /* APPVCOM4 */
    {APP_VCOM_DEV_NAME_5, APP_VCOM_SEM_NAME_5, 0x1000, 0},                      /* APPVCOM5 */
    {APP_VCOM_DEV_NAME_6, APP_VCOM_SEM_NAME_6, 0x1000, 0},                      /* APPVCOM6 */
    {APP_VCOM_DEV_NAME_7, APP_VCOM_SEM_NAME_7, 0x2000, 0},                      /* APPVCOM7 */
    {APP_VCOM_DEV_NAME_8, APP_VCOM_SEM_NAME_8, 0x1000, 0},                      /* APPVCOM8 */
    {APP_VCOM_DEV_NAME_9, APP_VCOM_SEM_NAME_9, 0x5000, 0},                      /* APPVCOM9 */
    {APP_VCOM_DEV_NAME_10, APP_VCOM_SEM_NAME_10, 0x1000, 0},                    /* APPVCOM10 */
    {APP_VCOM_DEV_NAME_11, APP_VCOM_SEM_NAME_11, 0x1000, 0},                    /* APPVCOM11 */
    {APP_VCOM_DEV_NAME_12, APP_VCOM_SEM_NAME_12, 0x1000, 0},                    /* APPVCOM12 */
    {APP_VCOM_DEV_NAME_13, APP_VCOM_SEM_NAME_13, 0x1000, 0},                    /* APPVCOM13 */
    {APP_VCOM_DEV_NAME_14, APP_VCOM_SEM_NAME_14, 0x1000, 0},                    /* APPVCOM14 */
    {APP_VCOM_DEV_NAME_15, APP_VCOM_SEM_NAME_15, 0x1000, 0},                    /* APPVCOM15 */
    {APP_VCOM_DEV_NAME_16, APP_VCOM_SEM_NAME_16, 0x1000, 0},                    /* APPVCOM16 */
    {APP_VCOM_DEV_NAME_17, APP_VCOM_SEM_NAME_17, 0x1000, 0},                    /* APPVCOM17 */
    {APP_VCOM_DEV_NAME_18, APP_VCOM_SEM_NAME_18, 0x1000, 0},                    /* APPVCOM18 */
    {APP_VCOM_DEV_NAME_19, APP_VCOM_SEM_NAME_19, 0x1000, 0},                    /* APPVCOM19 */
    {APP_VCOM_DEV_NAME_20, APP_VCOM_SEM_NAME_20, 0x1000, 0},                    /* APPVCOM20 */
    {APP_VCOM_DEV_NAME_21, APP_VCOM_SEM_NAME_21, 0x1000, 0},                    /* APPVCOM21 */
    {APP_VCOM_DEV_NAME_22, APP_VCOM_SEM_NAME_22, 0x1000, 0},                    /* APPVCOM22 */
    {APP_VCOM_DEV_NAME_23, APP_VCOM_SEM_NAME_23, 0x2000, 0},                    /* APPVCOM23 */
    {APP_VCOM_DEV_NAME_24, APP_VCOM_SEM_NAME_24, 0x1000, 0},                    /* APPVCOM24 */
    {APP_VCOM_DEV_NAME_25, APP_VCOM_SEM_NAME_25, 0x1000, 0},                    /* APPVCOM25 */
    {APP_VCOM_DEV_NAME_26, APP_VCOM_SEM_NAME_26, 0x1000, 0},                    /* APPVCOM26 */

    {APP_VCOM_DEV_NAME_27, APP_VCOM_SEM_NAME_27, 0x1000, 0},                    /* APPVCOM27 */
    {APP_VCOM_DEV_NAME_28, APP_VCOM_SEM_NAME_28, 0x1000, 0},                    /* APPVCOM28 */
    {APP_VCOM_DEV_NAME_29, APP_VCOM_SEM_NAME_29, 0x1000, 0},                    /* APPVCOM29 */
    {APP_VCOM_DEV_NAME_30, APP_VCOM_SEM_NAME_30, 0x1000, 0},                    /* APPVCOM30 */
    {APP_VCOM_DEV_NAME_31, APP_VCOM_SEM_NAME_31, 0x1000, 0},                    /* APPVCOM31 */

    {APP_VCOM_DEV_NAME_32, APP_VCOM_SEM_NAME_32, 0x1000, 0},                    /* APPVCOM32 */
    {APP_VCOM_DEV_NAME_33, APP_VCOM_SEM_NAME_33, 0x1000, 0},                    /* APPVCOM33 */
    {APP_VCOM_DEV_NAME_34, APP_VCOM_SEM_NAME_34, 0x1000, 0},                    /* APPVCOM34 */
    {APP_VCOM_DEV_NAME_35, APP_VCOM_SEM_NAME_35, 0x1000, 0},                    /* APPVCOM35 */
    {APP_VCOM_DEV_NAME_36, APP_VCOM_SEM_NAME_36, 0x1000, 0},                    /* APPVCOM36 */
    {APP_VCOM_DEV_NAME_37, APP_VCOM_SEM_NAME_37, 0x1000, 0},                    /* APPVCOM37 */
    {APP_VCOM_DEV_NAME_38, APP_VCOM_SEM_NAME_38, 0x1000, 0},                    /* APPVCOM38 */
    {APP_VCOM_DEV_NAME_39, APP_VCOM_SEM_NAME_39, 0x1000, 0},                    /* APPVCOM39 */
    {APP_VCOM_DEV_NAME_40, APP_VCOM_SEM_NAME_40, 0x1000, 0},                    /* APPVCOM40 */
    {APP_VCOM_DEV_NAME_41, APP_VCOM_SEM_NAME_41, 0x1000, 0},                    /* APPVCOM41 */
    {APP_VCOM_DEV_NAME_42, APP_VCOM_SEM_NAME_42, 0x1000, 0},                    /* APPVCOM42 */
    {APP_VCOM_DEV_NAME_43, APP_VCOM_SEM_NAME_43, 0x1000, 0},                    /* APPVCOM43 */
    {APP_VCOM_DEV_NAME_44, APP_VCOM_SEM_NAME_44, 0x1000, 0},                    /* APPVCOM44 */
    {APP_VCOM_DEV_NAME_45, APP_VCOM_SEM_NAME_45, 0x1000, 0},                    /* APPVCOM45 */
    {APP_VCOM_DEV_NAME_46, APP_VCOM_SEM_NAME_46, 0x1000, 0},                    /* APPVCOM46 */
    {APP_VCOM_DEV_NAME_47, APP_VCOM_SEM_NAME_47, 0x1000, 0},                    /* APPVCOM47 */
    {APP_VCOM_DEV_NAME_48, APP_VCOM_SEM_NAME_48, 0x1000, 0},                    /* APPVCOM48 */
    {APP_VCOM_DEV_NAME_49, APP_VCOM_SEM_NAME_49, 0x1000, 0},                    /* APPVCOM49 */
    {APP_VCOM_DEV_NAME_50, APP_VCOM_SEM_NAME_50, 0x1000, 0},                    /* APPVCOM50 */
    {APP_VCOM_DEV_NAME_51, APP_VCOM_SEM_NAME_51, 0x1000, 0},                    /* APPVCOM51 */
    {APP_VCOM_DEV_NAME_52, APP_VCOM_SEM_NAME_52, 0x1000, 0},                    /* APPVCOM52 */

    {APP_VCOM_DEV_NAME_53, APP_VCOM_SEM_NAME_53, 0x20000, 0},                   /* APPVCOM53 */
    {APP_VCOM_DEV_NAME_54, APP_VCOM_SEM_NAME_54, 0x1000, 0},                    /* APPVCOM54 */
    {APP_VCOM_DEV_NAME_55, APP_VCOM_SEM_NAME_55, 0x200000, 0},                  /* APPVCOM55 */
    {APP_VCOM_DEV_NAME_56, APP_VCOM_SEM_NAME_56, 0x200000, 0},                  /* APPVCOM56 */
    {APP_VCOM_DEV_NAME_57, APP_VCOM_SEM_NAME_57, 0x200000, 0},                  /* APPVCOM57 */

    {APP_VCOM_DEV_NAME_58, APP_VCOM_SEM_NAME_58, 0x1000, 0},                    /* APPVCOM58 */
    {APP_VCOM_DEV_NAME_59, APP_VCOM_SEM_NAME_59, 0x1000, 0},                    /* APPVCOM59 */
    {APP_VCOM_DEV_NAME_60, APP_VCOM_SEM_NAME_60, 0x1000, 0},                    /* APPVCOM60 */
    {APP_VCOM_DEV_NAME_61, APP_VCOM_SEM_NAME_61, 0x1000, 0},                    /* APPVCOM61 */
    {APP_VCOM_DEV_NAME_62, APP_VCOM_SEM_NAME_62, 0x1000, 0},                    /* APPVCOM62 */
    {APP_VCOM_DEV_NAME_63, APP_VCOM_SEM_NAME_63, 0x1000, 0}                     /* APPVCOM63 */
};

APP_VCOM_DEBUG_CFG_STRU              g_stAppVcomDebugCfg;

/*****************************************************************************
   3 ��������������
*****************************************************************************/

/*****************************************************************************
   4 ����ʵ��
*****************************************************************************/

APP_VCOM_DEV_CTX_STRU* APP_VCOM_GetVcomCtxAddr(VOS_UINT8 ucIndex)
{
    return &(g_astVcomCtx[ucIndex]);
}


APP_VCOM_DEV_ENTITY_STRU* APP_VCOM_GetAppVcomDevEntity(VOS_UINT8 ucIndex)
{
    return (g_astVcomCtx[ucIndex].pstAppVcomDevEntity);
}


VOS_UINT32 APP_VCOM_RegDataCallback(VOS_UINT8 ucDevIndex, SEND_UL_AT_FUNC pFunc)
{
    APP_VCOM_DEV_CTX_STRU              *pstVcomCtx;

    /* �����Ŵ���*/
    if (ucDevIndex >= APP_VCOM_DEV_INDEX_BUTT)
    {
        return VOS_ERR;
    }

    pstVcomCtx = APP_VCOM_GetVcomCtxAddr(ucDevIndex);

    /* ����ָ�븳��ȫ�ֱ���*/
    pstVcomCtx->pSendUlAtFunc = pFunc;

    return VOS_OK;
}


VOS_UINT32 APP_VCOM_RegEvtCallback(VOS_UINT8 ucDevIndex, EVENT_FUNC pFunc)
{
    APP_VCOM_DEV_CTX_STRU              *pstVcomCtx;

    /* �����Ŵ���*/
    if (ucDevIndex >= APP_VCOM_DEV_INDEX_BUTT)
    {
        return VOS_ERR;
    }

    pstVcomCtx = APP_VCOM_GetVcomCtxAddr(ucDevIndex);

    /* ����ָ�븳��ȫ�ֱ���*/
    pstVcomCtx->pEventFunc = pFunc;

    return VOS_OK;
}


VOS_UINT8 APP_VCOM_GetIndexFromMajorDevId(VOS_UINT ulMajorDevId)
{
    VOS_UINT8                           ucLoop;
    APP_VCOM_DEV_CTX_STRU              *pstVcomCtx;

    for (ucLoop = 0; ucLoop < APP_VCOM_MAX_NUM; ucLoop++)
    {
        pstVcomCtx = APP_VCOM_GetVcomCtxAddr(ucLoop);

        if (pstVcomCtx->ulAppVcomMajorId == ulMajorDevId)
        {
            break;
        }
    }

    return ucLoop;
}


VOS_VOID  APP_VCOM_InitSpecCtx(VOS_UINT8 ucDevIndex)
{
    APP_VCOM_DEV_CTX_STRU              *pstVcomCtx;

    if (ucDevIndex >= APP_VCOM_MAX_NUM)
    {
        return;
    }

    pstVcomCtx = APP_VCOM_GetVcomCtxAddr(ucDevIndex);

    TAF_MEM_SET_S(pstVcomCtx->aucSendSemName, sizeof(pstVcomCtx->aucSendSemName), 0x00, APP_VCOM_SEM_NAME_MAX_LEN);
    TAF_MEM_SET_S(pstVcomCtx->aucAppVcomName, sizeof(pstVcomCtx->aucAppVcomName), 0x00, APP_VCOM_DEV_NAME_MAX_LEN);

    TAF_MEM_CPY_S(pstVcomCtx->aucAppVcomName,
               sizeof(pstVcomCtx->aucAppVcomName),
               g_astAppVcomCogfigTab[ucDevIndex].pcAppVcomName,
               VOS_StrLen(g_astAppVcomCogfigTab[ucDevIndex].pcAppVcomName));

    TAF_MEM_CPY_S(pstVcomCtx->aucSendSemName,
               sizeof(pstVcomCtx->aucSendSemName),
               g_astAppVcomCogfigTab[ucDevIndex].pcSendSemName,
               VOS_StrLen(g_astAppVcomCogfigTab[ucDevIndex].pcSendSemName));

    pstVcomCtx->ulAppVcomMajorId = APP_VCOM_MAJOR_DEV_ID + ucDevIndex;
}


VOS_VOID APP_VCOM_Setup(
    APP_VCOM_DEV_ENTITY_STRU *pstDev,
    VOS_UINT8                 ucIndex
)
{
    VOS_INT                             iErr;
    dev_t                               ulDevno;
    static struct class                *pstCom_class;
    APP_VCOM_DEV_CTX_STRU              *pstVcomCtx;

    pstVcomCtx = APP_VCOM_GetVcomCtxAddr(ucIndex);

    ulDevno = MKDEV(pstVcomCtx->ulAppVcomMajorId, ucIndex);

    cdev_init(&pstDev->stAppVcomDev, &g_stOperations_Fops);

    iErr = cdev_add(&pstDev->stAppVcomDev, ulDevno, 1);
    if (iErr)
    {
        APP_VCOM_TRACE_ERR(ucIndex, "APP_VCOM_Setup cdev_add error! ");
        return;
    }

    pstCom_class = class_create(THIS_MODULE, pstVcomCtx->aucAppVcomName);

    device_create(pstCom_class,
                  NULL,
                  MKDEV(pstVcomCtx->ulAppVcomMajorId, ucIndex),
                  "%s",
                  pstVcomCtx->aucAppVcomName);

    return;
}


VOS_INT __init APP_VCOM_Init(VOS_VOID)
{
    VOS_INT                             iResult1;
    VOS_INT                             iResult2;
    dev_t                               ulDevno;
    VOS_UINT8                           ucIndex;
    APP_VCOM_DEV_CTX_STRU              *pstVcomCtx;
    APP_VCOM_DEV_ENTITY_STRU           *pstVcomDevp;

    printk("APP_VCOM_Init entry,%u",VOS_GetSlice());

    pstVcomCtx = VOS_NULL_PTR;
    pstVcomDevp  = VOS_NULL_PTR;

    /* ��ʼ����ά�ɲ�ȫ�ֱ��� */
    TAF_MEM_SET_S(&g_stAppVcomDebugInfo, sizeof(g_stAppVcomDebugInfo), 0x00, sizeof(g_stAppVcomDebugInfo));

    TAF_MEM_SET_S(&g_stAppVcomDebugCfg, sizeof(g_stAppVcomDebugCfg), 0x00, sizeof(g_stAppVcomDebugCfg));

    /* ��ʼ�������豸 */
    for (ucIndex = 0; ucIndex < APP_VCOM_MAX_NUM; ucIndex++)
    {
        /* ��ʼ��ȫ�ֱ��� */
        APP_VCOM_InitSpecCtx(ucIndex);

        /* ��ȡȫ�ֱ���ָ�� */
        pstVcomCtx = APP_VCOM_GetVcomCtxAddr(ucIndex);

        /* ���豸��ת����dev_t ���� */
        ulDevno = MKDEV(pstVcomCtx->ulAppVcomMajorId, ucIndex);

        iResult1 = register_chrdev_region(ulDevno, 1, pstVcomCtx->aucAppVcomName);

        /* ע��ʧ����̬�����豸�� */
        if (iResult1 < 0)
        {
            iResult2 = alloc_chrdev_region(&ulDevno, 0, 1, pstVcomCtx->aucAppVcomName);

            if (iResult2 < 0 )
            {
                return VOS_ERROR;
            }

            pstVcomCtx->ulAppVcomMajorId = MAJOR(ulDevno);
        }

        /* ��̬�����豸�ṹ���ڴ� */
        pstVcomCtx->pstAppVcomDevEntity = kmalloc(sizeof(APP_VCOM_DEV_ENTITY_STRU) , GFP_KERNEL);

        if (VOS_NULL_PTR == pstVcomCtx->pstAppVcomDevEntity)
        {
            /* ȥע����豸�����ش��� */
            unregister_chrdev_region(ulDevno, 1);
            APP_VCOM_TRACE_ERR(ucIndex, "APP_VCOM_Init malloc device Entity fail. ");
            return VOS_ERROR;
        }

        /* ��ȡ�豸ʵ��ָ�� */
        pstVcomDevp = pstVcomCtx->pstAppVcomDevEntity;

        TAF_MEM_SET_S(pstVcomDevp, sizeof(APP_VCOM_DEV_ENTITY_STRU), 0x00, sizeof(APP_VCOM_DEV_ENTITY_STRU));

        if (APPVCOM_STATIC_MALLOC_MEMORY(ucIndex))
        {
            pstVcomDevp->pucAppVcomMem = kmalloc(g_astAppVcomCogfigTab[ucIndex].ulAppVcomMemSize, GFP_KERNEL);

            if (VOS_NULL_PTR == pstVcomDevp->pucAppVcomMem)
            {
                /* ȥע����豸�����ش��� */
                unregister_chrdev_region(ulDevno, 1);
                APP_VCOM_TRACE_ERR(ucIndex, "APP_VCOM_Init malloc device buff fail. ");
                kfree(pstVcomCtx->pstAppVcomDevEntity);
                pstVcomCtx->pstAppVcomDevEntity = VOS_NULL_PTR;
                return VOS_ERROR;
            }
        }

        init_waitqueue_head(&pstVcomDevp->Read_Wait);
        TAF_MEM_SET_S(pstVcomDevp->acWakeLockName, sizeof(pstVcomDevp->acWakeLockName), 0x00, APP_VCOM_RD_WAKE_LOCK_NAME_LEN);
        snprintf(pstVcomDevp->acWakeLockName, APP_VCOM_RD_WAKE_LOCK_NAME_LEN, "appvcom%d_rd_wake", ucIndex);
        pstVcomDevp->acWakeLockName[APP_VCOM_RD_WAKE_LOCK_NAME_LEN - 1] = '\0';
        wake_lock_init(&pstVcomDevp->stRdWakeLock, WAKE_LOCK_SUSPEND, pstVcomDevp->acWakeLockName);

        mutex_init(&pstVcomDevp->stMutex);

        APP_VCOM_Setup(pstVcomDevp, ucIndex);

        /* �����ź��� */
        sema_init(&pstVcomDevp->stMsgSendSem,1);
        sema_init(&pstVcomDevp->stWrtSem, 1);
    }

    printk("APP_VCOM_Init eixt,%u",VOS_GetSlice());

    return VOS_OK;
}


int APP_VCOM_Release(
    struct inode                       *inode,
    struct file                        *filp
)
{
    VOS_UINT                            ulDevMajor;
    VOS_UINT8                           ucIndex;
    APP_VCOM_DEV_CTX_STRU              *pstVcomCtx;
    APP_VCOM_DEV_ENTITY_STRU           *pstVcomDevp = VOS_NULL_PTR;

    if (VOS_NULL_PTR == inode || VOS_NULL_PTR == filp)
    {
        return VOS_ERROR;
    }

    /* ��ȡ���豸�� */
    ulDevMajor = imajor(inode);

    /* �������豸�ŵõ��豸��ȫ�ֱ����е�����ֵ */
    ucIndex = APP_VCOM_GetIndexFromMajorDevId(ulDevMajor);

    if (ucIndex >= APP_VCOM_MAX_NUM)
    {
        APP_VCOM_TRACE_ERR(ucIndex, "APP_VCOM_Release ucIndex is error. ");
        return VOS_ERROR;
    }

    /* ��ȡVCOMȫ�ֱ��� */
    pstVcomCtx = APP_VCOM_GetVcomCtxAddr(ucIndex);

    if (VOS_NULL_PTR == pstVcomCtx->pstAppVcomDevEntity)
    {
        APP_VCOM_TRACE_ERR(ucIndex, "APP_VCOM_Release VcomDevEntity is NULL. ");
        return VOS_ERROR;
    }

    if(APPVCOM_DYNAMIC_MALLOC_MEMORY(ucIndex))
    {
        pstVcomDevp = pstVcomCtx->pstAppVcomDevEntity;

        down(&pstVcomDevp->stMsgSendSem);

        if (VOS_NULL_PTR != pstVcomDevp->pucAppVcomMem)
        {
            kfree(pstVcomDevp->pucAppVcomMem);
            pstVcomDevp->pucAppVcomMem = VOS_NULL_PTR;
            APP_VCOM_TRACE_INFO(ucIndex, "APP_VCOM_Release free memory is ok. ");
        }

        up(&pstVcomDevp->stMsgSendSem);
    }

    /* ���豸�ṹ��ָ�븳ֵ���ļ�˽������ָ�� */
    filp->private_data = pstVcomCtx->pstAppVcomDevEntity;

    APP_VCOM_TRACE_INFO(ucIndex, "APP_VCOM_Release enter. ");

    if(NULL != pstVcomCtx->pEventFunc)
    {
        (VOS_VOID)(pstVcomCtx->pEventFunc(APP_VCOM_EVT_RELEASE));
    }

    pstVcomCtx->pstAppVcomDevEntity->ulIsDeviceOpen = VOS_FALSE;
    /* lint -e455 */
    wake_unlock(&pstVcomCtx->pstAppVcomDevEntity->stRdWakeLock);
    /* lint +e455 */

    return VOS_OK;
}


int APP_VCOM_Open(
    struct inode                       *inode,
    struct file                        *filp
)
{
    VOS_UINT                            ulDevMajor;
    VOS_UINT8                           ucIndex;
    APP_VCOM_DEV_CTX_STRU              *pstVcomCtx;
    APP_VCOM_DEV_ENTITY_STRU           *pstVcomDevp = VOS_NULL_PTR;

    if (VOS_NULL_PTR == inode || VOS_NULL_PTR == filp)
    {
        return VOS_ERROR;
    }

    /* ��ȡ���豸�� */
    ulDevMajor = imajor(inode);

    /* �������豸�ŵõ��豸��ȫ�ֱ����е�����ֵ */
    ucIndex = APP_VCOM_GetIndexFromMajorDevId(ulDevMajor);

    if (ucIndex >= APP_VCOM_MAX_NUM)
    {
        APP_VCOM_TRACE_ERR(ucIndex, "APP_VCOM_Open ucIndex is error. ");
        return VOS_ERROR;
    }

    /* ��ȡVCOMȫ�ֱ��� */
    pstVcomCtx = APP_VCOM_GetVcomCtxAddr(ucIndex);

    if (VOS_NULL_PTR == pstVcomCtx->pstAppVcomDevEntity)
    {
        APP_VCOM_TRACE_ERR(ucIndex, "APP_VCOM_Open VcomDevEntity is NULL. ");
        return VOS_ERROR;
    }

    if (APPVCOM_DYNAMIC_MALLOC_MEMORY(ucIndex))
    {
        /* ��ȡ�豸ʵ��ָ�� */
        pstVcomDevp = pstVcomCtx->pstAppVcomDevEntity;
        if (VOS_NULL_PTR == pstVcomDevp->pucAppVcomMem)
        {
            pstVcomDevp->pucAppVcomMem = kmalloc(g_astAppVcomCogfigTab[ucIndex].ulAppVcomMemSize, GFP_KERNEL);

            if (VOS_NULL_PTR == pstVcomDevp->pucAppVcomMem)
            {
                APP_VCOM_TRACE_ERR(ucIndex, "APP_VCOM_Open alloc memory is err. ");
                return VOS_ERROR;
            }
            APP_VCOM_TRACE_INFO(ucIndex, "APP_VCOM_Open alloc memory is ok. ");
        }
    }
    /* ���豸�ṹ��ָ�븳ֵ���ļ�˽������ָ�� */
    filp->private_data = pstVcomCtx->pstAppVcomDevEntity;

    APP_VCOM_TRACE_INFO(ucIndex, "APP_VCOM_Open enter. ");

    if(NULL != pstVcomCtx->pEventFunc)
    {
        (VOS_VOID)(pstVcomCtx->pEventFunc(APP_VCOM_EVT_OPEN));
    }

    pstVcomCtx->pstAppVcomDevEntity->ulIsDeviceOpen = VOS_TRUE;

    return VOS_OK;
}


ssize_t APP_VCOM_Read(
    struct file                        *stFilp,
    char __user                        *buf,
    size_t                              count,
    loff_t                             *ppos
)
{
    APP_VCOM_DEV_ENTITY_STRU           *pstVcomDev;
    struct cdev                        *pstCdev;
    VOS_UINT                            ulDevMajor;
    VOS_UINT8                           ucIndex;
    APP_VCOM_DEV_CTX_STRU              *pstVcomCtx;

    /* ����豸�ṹ��ָ�� */
    pstVcomDev = stFilp->private_data;

    /* ����豸���豸�� */
    pstCdev = &(pstVcomDev->stAppVcomDev);
    ulDevMajor = MAJOR(pstCdev->dev);

    /* ����豸��ȫ�ֱ����е�����ֵ */
    ucIndex = APP_VCOM_GetIndexFromMajorDevId(ulDevMajor);

    if (ucIndex >= APP_VCOM_MAX_NUM)
    {
        return APP_VCOM_ERROR;
    }

    pstVcomCtx = APP_VCOM_GetVcomCtxAddr(ucIndex);

    APP_VCOM_TRACE_INFO(ucIndex, "APP_VCOM_Read, read count:%d, current_len:%d. ", count, pstVcomDev->current_len);
    APP_VCOM_TRACE_INFO(ucIndex, "APP_VCOM_Read, f_flags:%d. ", stFilp->f_flags);
    APP_VCOM_TRACE_INFO(ucIndex, "APP_VCOM_Read, wait_event 111, flag:%d. ", pstVcomDev->ulReadWakeUpFlg);

    if (stFilp->f_flags & O_NONBLOCK)
    {
        return APP_VCOM_ERROR;
    }

    /*lint -e730 �޸���:l60609;������:z60575;ԭ��:�����̻߳�ͬʱд��ȫ�ֱ���  */
    if (wait_event_interruptible(pstVcomDev->Read_Wait, (pstVcomDev->current_len != 0)))
    {
        return -ERESTARTSYS;
    }
    /*lint +e730 �޸���:l60609;������:z60575;ԭ��:�����̻߳�ͬʱд��ȫ�ֱ���  */

    if (0 == pstVcomDev->current_len)
    {
        g_stAppVcomDebugInfo.ulReadLenErr[ucIndex]++;
    }

    APP_VCOM_TRACE_INFO(ucIndex, "APP_VCOM_Read, wait_event 222,flag:%d. ", pstVcomDev->ulReadWakeUpFlg);

    /* ��ȡ�ź��� */
    down(&pstVcomCtx->pstAppVcomDevEntity->stMsgSendSem);

    if (APPVCOM_DYNAMIC_MALLOC_MEMORY(ucIndex)&&(VOS_NULL_PTR == pstVcomDev->pucAppVcomMem))
    {
        up(&pstVcomCtx->pstAppVcomDevEntity->stMsgSendSem);
        return APP_VCOM_ERROR;
    }

    if (count > pstVcomDev->current_len)
    {
        count = pstVcomDev->current_len;
    }

    if (copy_to_user(buf, pstVcomDev->pucAppVcomMem, (VOS_ULONG)count))
    {
        APP_VCOM_TRACE_ERR(ucIndex, "APP_VCOM_Read, copy_to_user fail. ");

        /* �ͷ��ź��� */
        up(&pstVcomCtx->pstAppVcomDevEntity->stMsgSendSem);
        return APP_VCOM_ERROR;
    }

    if ((pstVcomDev->current_len - count) > 0)
    {
        /* FIFO����ǰ�� */
        memmove(pstVcomDev->pucAppVcomMem, (pstVcomDev->pucAppVcomMem + count), (pstVcomDev->current_len - count));
        APP_VCOM_TRACE_INFO(ucIndex, "APP_VCOM_Read, FIFO move. ");
    }

    /* ��Ч���ݳ��ȼ�С*/
    pstVcomDev->current_len -= count;

    APP_VCOM_TRACE_INFO(ucIndex, "APP_VCOM_Read, read %d bytes, current_len:%d. ", count, pstVcomDev->current_len);

    if (0 == pstVcomDev->current_len)
    {
        APP_VCOM_TRACE_NORM(ucIndex, "APP_VCOM_Send: read all data. ");
        /* lint -e455 */
        wake_unlock(&pstVcomDev->stRdWakeLock);
        /* lint +e455 */
    }

    /* �ͷ��ź��� */
    up(&pstVcomCtx->pstAppVcomDevEntity->stMsgSendSem);

    return (ssize_t)count;
}


ssize_t APP_VCOM_Write(
    struct file                        *stFilp,
    const char __user                  *buf,
    size_t                              count,
    loff_t                             *ppos
)
{
    VOS_UINT8                          *pucDataBuf;
    VOS_INT                             iRst;
    APP_VCOM_DEV_ENTITY_STRU           *pstVcomDev;
    struct cdev                        *pstCdev;
    VOS_UINT                            ulDevMajor;
    VOS_UINT8                           ucIndex;
    APP_VCOM_DEV_CTX_STRU              *pstVcomCtx;

    /* ����豸�ṹ��ָ�� */
    pstVcomDev = stFilp->private_data;

    /* ����豸���豸�� */
    pstCdev = &(pstVcomDev->stAppVcomDev);
    ulDevMajor = MAJOR(pstCdev->dev);

    /* ����豸��ȫ�ֱ����е�����ֵ */
    ucIndex = APP_VCOM_GetIndexFromMajorDevId(ulDevMajor);

    if(ucIndex >= APP_VCOM_MAX_NUM)
    {
        APP_VCOM_TRACE_ERR(ucIndex, "APP_VCOM_Write, ucIndex fail. ");
        return APP_VCOM_ERROR;
    }

    if (NULL == buf)
    {
        APP_VCOM_TRACE_ERR(ucIndex, "APP_VCOM_Write, buf is null ");
        return APP_VCOM_ERROR;
    }

    if ((count > APP_VCOM_MAX_DATA_LENGTH)||(0 == count))
    {
        APP_VCOM_TRACE_ERR(ucIndex, "APP_VCOM_Write, count is %d, it is error", count);
        return APP_VCOM_ERROR;
    }

    /* ���ȫ�ֱ�����ַ */
    pstVcomCtx = APP_VCOM_GetVcomCtxAddr(ucIndex);

    /* �����ڴ� */
    pucDataBuf = kmalloc(count, GFP_KERNEL);
    if (VOS_NULL_PTR == pucDataBuf )
    {
        APP_VCOM_TRACE_ERR(ucIndex, "APP_VCOM_Write, kmalloc fail. ");

        return APP_VCOM_ERROR;
    }

    /* buffer���� */
    TAF_MEM_SET_S(pucDataBuf, count, 0x00, (VOS_SIZE_T)count);

    if (copy_from_user(pucDataBuf, buf, (VOS_ULONG)count))
    {
        APP_VCOM_TRACE_ERR(ucIndex, "APP_VCOM_Write, copy_from_user fail. ");

        kfree(pucDataBuf);
        return APP_VCOM_ERROR;
    }

    if ((APP_VCOM_DEV_INDEX_9 == ucIndex) || (APP_VCOM_DEV_INDEX_12 == ucIndex))
    {
        down(&pstVcomCtx->pstAppVcomDevEntity->stWrtSem);

        if (VOS_NULL_PTR != pstVcomCtx->pstAppVcomDevEntity->pucWrtBuffer)
        {
            APP_VCOM_TRACE_NORM(ucIndex, "APP_VCOM_Write: free buff. ");

            kfree(pstVcomCtx->pstAppVcomDevEntity->pucWrtBuffer);
            pstVcomCtx->pstAppVcomDevEntity->pucWrtBuffer   = VOS_NULL_PTR;
            pstVcomCtx->pstAppVcomDevEntity->ulWrtBufferLen = 0;
        }

        if (VOS_NULL_PTR == pstVcomCtx->pSendUlAtFunc)
        {
            APP_VCOM_TRACE_NORM(ucIndex, "APP_VCOM_Write: save buff. ");

            pstVcomCtx->pstAppVcomDevEntity->pucWrtBuffer   = pucDataBuf;
            pstVcomCtx->pstAppVcomDevEntity->ulWrtBufferLen = count;
            up(&pstVcomCtx->pstAppVcomDevEntity->stWrtSem);
            return (ssize_t)count;
        }

        up(&pstVcomCtx->pstAppVcomDevEntity->stWrtSem);
    }

    /* ���ûص���������buf�е�AT����*/
    if (VOS_NULL_PTR == pstVcomCtx->pSendUlAtFunc)
    {
        APP_VCOM_TRACE_ERR(ucIndex, "APP_VCOM_Write, pSendUlAtFunc is null. ");
        kfree(pucDataBuf);
        return APP_VCOM_ERROR;
    }

    mutex_lock(&pstVcomDev->stMutex);

    iRst = pstVcomCtx->pSendUlAtFunc(ucIndex, pucDataBuf, (VOS_UINT32)count);
    if (VOS_OK != iRst)
    {
        g_stAppVcomDebugInfo.ulAtCallBackErr[ucIndex]++;
        APP_VCOM_TRACE_ERR(ucIndex, "APP_VCOM_Write, AT_RcvFromAppCom fail. ");
        mutex_unlock(&pstVcomDev->stMutex);
        kfree(pucDataBuf);

        return APP_VCOM_ERROR;
    }

    mutex_unlock(&pstVcomDev->stMutex);

    APP_VCOM_TRACE_INFO(ucIndex, "APP_VCOM_Write, write %d bytes, AT_RcvFromAppCom Success.",count);

    /* �ͷ��ڴ� */
    kfree(pucDataBuf);

    return (ssize_t)count;
}


unsigned int APP_VCOM_Poll(struct file *fp, struct poll_table_struct *wait)
{
    APP_VCOM_DEV_ENTITY_STRU           *pstVcomDev = VOS_NULL_PTR;
    unsigned int                        mask = 0;

    struct cdev                        *pstCdev;
    VOS_UINT                            ulDevMajor;
    VOS_UINT8                           ucIndex;

    pstVcomDev = fp->private_data;

    pstCdev = &(pstVcomDev->stAppVcomDev);
    ulDevMajor = MAJOR(pstCdev->dev);
    ucIndex = APP_VCOM_GetIndexFromMajorDevId(ulDevMajor);

    poll_wait(fp, &pstVcomDev->Read_Wait, wait);

    if (0 != pstVcomDev->current_len)
    {
        mask |= POLLIN | POLLRDNORM;
    }

    APP_VCOM_TRACE_INFO(ucIndex, "APP_VCOM_Poll, mask = %d. ", mask);

    return mask;
}


VOS_UINT32  APP_VCOM_Send (
    APP_VCOM_DEV_INDEX_UINT8            enDevIndex,
    VOS_UINT8                          *pData,
    VOS_UINT32                          uslength
)
{
    APP_VCOM_DEV_ENTITY_STRU           *pstVcomDev;
    APP_VCOM_DEV_CTX_STRU              *pstVcomCtx;

    if (enDevIndex >= APP_VCOM_MAX_NUM)
    {
        g_stAppVcomDebugInfo.ulDevIndexErr++;
        APP_VCOM_TRACE_ERR(enDevIndex, "APP_VCOM_Send, enDevIndex is error. ");
        return VOS_ERR;
    }

    pstVcomCtx = APP_VCOM_GetVcomCtxAddr(enDevIndex);
    if (VOS_NULL_PTR == pstVcomCtx)
    {
        APP_VCOM_TRACE_ERR(enDevIndex, "APP_VCOM_Send, pstVcomCtx is null. ");
        return VOS_ERR;
    }

    /* ����豸ʵ��ָ�� */
    pstVcomDev = APP_VCOM_GetAppVcomDevEntity(enDevIndex);
    if (VOS_NULL_PTR == pstVcomDev)
    {
        g_stAppVcomDebugInfo.ulVcomDevErr[enDevIndex]++;
        APP_VCOM_TRACE_ERR(enDevIndex, "APP_VCOM_Send, pstVcomDev is null. ");
        return VOS_ERR;
    }

    if ((APP_VCOM_DEV_INDEX_9 == enDevIndex) || (APP_VCOM_DEV_INDEX_12 == enDevIndex))
    {
        down(&pstVcomDev->stWrtSem);

        if (VOS_NULL_PTR != pstVcomDev->pucWrtBuffer)
        {
            if (VOS_NULL_PTR != pstVcomCtx->pSendUlAtFunc)
            {
                APP_VCOM_TRACE_NORM(enDevIndex, "APP_VCOM_Send: handle buff. ");

                (VOS_VOID)pstVcomCtx->pSendUlAtFunc(enDevIndex,
                                        pstVcomDev->pucWrtBuffer,
                                        pstVcomDev->ulWrtBufferLen);

                kfree(pstVcomDev->pucWrtBuffer);
                pstVcomDev->pucWrtBuffer   = VOS_NULL_PTR;
                pstVcomDev->ulWrtBufferLen = 0;
            }
        }

        up(&pstVcomDev->stWrtSem);
    }

    APP_VCOM_TRACE_INFO(enDevIndex, "APP_VCOM_Send, uslength:%d, current_len:%d. ", uslength, pstVcomDev->current_len);

    /* ��ȡ�ź��� */
    down(&pstVcomDev->stMsgSendSem);

    if (APPVCOM_DYNAMIC_MALLOC_MEMORY(enDevIndex)&&(VOS_NULL_PTR == pstVcomDev->pucAppVcomMem))
    {
        up(&pstVcomDev->stMsgSendSem);
        return VOS_ERR;
    }

    /* ��������ֱ�ӷ��� */
    /*lint -e661*/
    if (g_astAppVcomCogfigTab[enDevIndex].ulAppVcomMemSize == pstVcomDev->current_len)
    /*lint +e661*/
    {
        APP_VCOM_TRACE_NORM(enDevIndex, "APP_VCOM_Send: VCOM MEM FULL. ");

        if (VOS_TRUE == pstVcomDev->ulIsDeviceOpen)
        {
            APP_VCOM_ERR_LOG(enDevIndex, "APP_VCOM_Send: VCOM MEM FULL. ");
        }

        /*lint -e661*/
        g_stAppVcomDebugInfo.ulMemFullErr[enDevIndex]++;
        /*lint +e661*/

        up(&pstVcomDev->stMsgSendSem);
        return VOS_ERR;
    }

    /* �������ݴ���ʣ��Buffer��С */
    /*lint -e661*/
    if (uslength > (g_astAppVcomCogfigTab[enDevIndex].ulAppVcomMemSize - pstVcomDev->current_len))
    /*lint +e661*/
    {
        APP_VCOM_TRACE_NORM(enDevIndex, "APP_VCOM_Send: data more than Buffer. ");

        if (VOS_TRUE == pstVcomDev->ulIsDeviceOpen)
        {
            APP_VCOM_ERR_LOG(enDevIndex, "APP_VCOM_Send: VCOM MEM FULL. ");
        }

        /*lint -e661*/
        uslength = g_astAppVcomCogfigTab[enDevIndex].ulAppVcomMemSize - (VOS_UINT32)pstVcomDev->current_len;
        /*lint +e661*/
    }

    /* ���Ƶ�BUFFER */
    memcpy(pstVcomDev->pucAppVcomMem + pstVcomDev->current_len, pData, uslength);
    pstVcomDev->current_len += uslength;

    APP_VCOM_TRACE_INFO(enDevIndex, "APP_VCOM_Send, written %d byte(s), new len: %d. ", uslength, pstVcomDev->current_len);

    APP_VCOM_TRACE_INFO(enDevIndex, "APP_VCOM_Send, IsDeviceOpen: %d. ", pstVcomDev->ulIsDeviceOpen);
    if (VOS_TRUE == pstVcomDev->ulIsDeviceOpen)
    {
        wake_lock_timeout(&pstVcomDev->stRdWakeLock, (VOS_LONG)msecs_to_jiffies(APP_VCOM_READ_WAKE_LOCK_LEN));
    }

    /* �ͷ��ź��� */
    up(&pstVcomDev->stMsgSendSem);
    wake_up_interruptible(&pstVcomDev->Read_Wait);

    if (0 == pstVcomDev->current_len)
    {
        /*lint -e661*/
        g_stAppVcomDebugInfo.ulSendLenErr[enDevIndex]++;
        /*lint +e661*/
    }

    APP_VCOM_TRACE_INFO(enDevIndex, "APP_VCOM_Send, wakeup. ");
    return VOS_OK;
}


VOS_VOID APP_VCOM_ShowDebugInfo(VOS_VOID)
{
    int                                 i;
    (VOS_VOID)vos_printf("App Vcom Debug Info:");
    (VOS_VOID)vos_printf("Index Err: %d\r\n", g_stAppVcomDebugInfo.ulDevIndexErr);

    for (i = 0; i < APP_VCOM_MAX_NUM; i++)
    {
        (VOS_VOID)vos_printf("\r\n");
        (VOS_VOID)vos_printf("AppVcom[%d] Callback Function Return Err Num: %d\r\n", i, g_stAppVcomDebugInfo.ulAtCallBackErr[i]);
        (VOS_VOID)vos_printf("AppVcom[%d] Mem Full Num:                     %d\r\n", i, g_stAppVcomDebugInfo.ulMemFullErr[i]);
        (VOS_VOID)vos_printf("AppVcom[%d] Read Data Length = 0 Num:         %d\r\n", i, g_stAppVcomDebugInfo.ulReadLenErr[i]);
        (VOS_VOID)vos_printf("AppVcom[%d] Send Data Length = 0 Num:         %d\r\n", i, g_stAppVcomDebugInfo.ulSendLenErr[i]);
        (VOS_VOID)vos_printf("AppVcom[%d] Get App Vcom Dev Entity Err Num:  %d\r\n", i, g_stAppVcomDebugInfo.ulVcomDevErr[i]);
    }
}



VOS_VOID APP_VCOM_SendDebugNvCfg(
    VOS_UINT32                          ulPortIdMask1,
    VOS_UINT32                          ulPortIdMask2,
    VOS_UINT32                          ulDebugLevel
)
{
    g_stAppVcomDebugCfg.ulPortIdMask1 = ulPortIdMask1;
    g_stAppVcomDebugCfg.ulPortIdMask2 = ulPortIdMask2;
    g_stAppVcomDebugCfg.ulDebugLevel  = ulDebugLevel;
}


VOS_VOID APP_VCOM_MNTN_LogPrintf(VOS_CHAR *pcFmt, ...)
{
    VOS_CHAR                            acBuf[APP_VCOM_TRACE_BUF_LEN] = {0};
    VOS_UINT32                          ulPrintLength = 0;

    /* ��ʽ�����BUFFER */
    /*lint -e713 -e507*/
    APP_VCOM_LOG_FORMAT(ulPrintLength, acBuf, APP_VCOM_TRACE_BUF_LEN, pcFmt);
    /*lint +e713 +e507*/

    printk(KERN_ERR "%s", acBuf);
    return;
}

module_init(APP_VCOM_Init);



