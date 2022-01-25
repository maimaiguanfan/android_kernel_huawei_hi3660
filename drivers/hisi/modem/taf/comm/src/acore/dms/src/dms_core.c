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
#include "msp_errno.h"
#include <dms.h>
#include "dms_core.h"
#include "PsLib.h"
#include "TafNvInterface.h"
#include "NVIM_Interface.h"
#include <product_config.h>
#include "TafTypeDef.h"



/*****************************************************************************
    Э��ջ��ӡ��㷽ʽ�µ�.C�ļ��궨��
*****************************************************************************/

#define THIS_FILE_ID                    PS_FILE_ID_DMS_CORE_C


/*****************************************************************************
  2 ȫ�ֱ�������
*****************************************************************************/

DMS_MAIN_INFO                           g_stDmsMainInfo = {0};

static const struct file_operations     g_stPortCfgOps =
{
    .owner      = THIS_MODULE,
    .write      = DMS_WritePortCfgFile,
    .read       = DMS_ReadPortCfgFile,
};

static const struct file_operations g_stGetSliceOps      =
{
    .owner      = THIS_MODULE,
    .read       = DMS_ReadGetSliceFile,
};

static const struct file_operations     g_stModemStatus =
{
    .owner      = THIS_MODULE,
    .read       = DMS_ReadModemStatusFile,
};

DMS_NLK_ENTITY_STRU                     g_stDmsNlkEntity = {0};

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
static struct netlink_kernel_cfg        g_stDmsNlkCfg =
{
    .input      = DMS_NLK_Input,
};
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0) */

VOS_UINT8                               g_ucDmsPrintModemLogType = 0;


/*****************************************************************************
  3 �ⲿ��������
*****************************************************************************/

extern VOS_VOID At_MsgProc(MsgBlock* pMsg);
extern VOS_UINT32 At_PidInit(enum VOS_INIT_PHASE_DEFINE enPhase);
extern void *hisi_io_memcpy(void *dest, const void *src, unsigned int count);
/* ���pclint���� */

/*****************************************************************************
  3 ����ʵ��
*****************************************************************************/

VOS_VOID DMS_ReadPortDebugCfgNV(VOS_VOID)
{
    TAF_NV_PORT_DEBUG_CFG_STRU          stPortDebugNVCfg;
    VOS_UINT32                          ulAppVcomPortIdMask;
    VOS_UINT32                          ulDebugLevel;

    TAF_MEM_SET_S(&stPortDebugNVCfg, sizeof(stPortDebugNVCfg), 0x00, sizeof(TAF_NV_PORT_DEBUG_CFG_STRU));

    /* ��ȡNV�� */
    if (NV_OK != NV_ReadEx(MODEM_ID_0,
                           en_NV_Item_DMS_DEBUG_CFG,
                          &stPortDebugNVCfg,
                           sizeof(TAF_NV_PORT_DEBUG_CFG_STRU)))
    {
        return;
    }

    ulAppVcomPortIdMask = stPortDebugNVCfg.ulAppVcomPortIdMask;
    ulDebugLevel        = stPortDebugNVCfg.ulDebugLevel;

    APP_VCOM_SendDebugNvCfg(ulAppVcomPortIdMask, 0x00, ulDebugLevel);

    return;
}


VOS_VOID DMS_ReadGetModemLogCfgNV(VOS_VOID)
{
    TAF_NV_PRINT_MODEM_LOG_TYPE_STRU    stPrintModemLogType;

    TAF_MEM_SET_S(&stPrintModemLogType, sizeof(stPrintModemLogType), 0x00, sizeof(stPrintModemLogType));

    /* ��ȡNV�� */
    if (NV_OK != NV_Read(en_NV_Item_Print_Modem_Log_Type,
                         &stPrintModemLogType,
                         sizeof(TAF_NV_PRINT_MODEM_LOG_TYPE_STRU)))
    {
        return;
    }

    DMS_SET_PRINT_MODEM_LOG_TYPE(stPrintModemLogType.ucPrintModemLogType);

    return;
}


VOS_VOID DMS_Init(VOS_VOID)
{
    VOS_UINT32                          ulport;

    TAF_MEM_SET_S(g_astDmsSdmInfoTable, sizeof(g_astDmsSdmInfoTable), 0x00, sizeof(g_astDmsSdmInfoTable));

    g_stDmsMainInfo.pfnRdDataCallback  = NULL;
    g_stDmsMainInfo.pfnConnectCallBack = NULL;

    /*��ʼ�����ж˿ڵ�handle*/
    for (ulport = 0; ulport < DMS_PHY_BEAR_LAST; ulport++)
    {
        g_stDmsMainInfo.stPhyProperty[ulport].lPortHandle = UDI_INVALID_HANDLE;
    }

    /*����ͨ��Ĭ�����ԣ����ڶ������ͨ����������Ӧһ���߼�ͨ��*/
    g_stDmsMainInfo.stPhyProperty[DMS_PHY_BEAR_USB_PCUI].enLogicChan  = DMS_CHANNEL_AT;
    g_stDmsMainInfo.stPhyProperty[DMS_PHY_BEAR_USB_CTRL].enLogicChan  = DMS_CHANNEL_AT;
    g_stDmsMainInfo.stPhyProperty[DMS_PHY_BEAR_USB_PCUI2].enLogicChan = DMS_CHANNEL_AT;


    /* ��ʼ��ATͨ��ʹ�õľ�̬�ڴ� */
    Dms_StaticBufInit();


    DMS_ReadPortDebugCfgNV();

    mdrv_usb_reg_enablecb(DMS_UsbEnableCB);
    mdrv_usb_reg_disablecb(DMS_UsbDisableCB);

    wake_lock_init(&g_stDmsMainInfo.stwakelock, WAKE_LOCK_SUSPEND, "dms_wakelock");

    DMS_ReadGetModemLogCfgNV();

    return;
}



DMS_CONNECT_STA_PFN DMS_GetConnStaFun(VOS_VOID)
{
    return (DMS_GetMainInfo()->pfnConnectCallBack);
}


VOS_VOID DMS_SetConnStaCB(DMS_CONNECT_STA_PFN pfnReg)
{
    DMS_MAIN_INFO                      *pstMainInfo = DMS_GetMainInfo();

    if (NULL == pfnReg)
    {
        return;
    }

    pstMainInfo->pfnConnectCallBack = pfnReg;
    return;
}


DMS_READ_DATA_PFN DMS_GetDataReadFun(VOS_VOID)
{
    return g_stDmsMainInfo.pfnRdDataCallback;
}


DMS_MAIN_INFO* DMS_GetMainInfo(VOS_VOID)
{
    return &g_stDmsMainInfo;
}


 DMS_PHY_BEAR_PROPERTY_STRU* DMS_GetPhyBearProperty(DMS_PHY_BEAR_ENUM enPhyBear)
{
    return &(DMS_GetMainInfo()->stPhyProperty[enPhyBear]);
}


VOS_VOID DMS_UsbDisableCB(VOS_VOID)
{
    VOS_UINT32                          ulRet;

    DMS_DBG_SDM_FUN(DMS_SDM_USB_DISABLE, 0, 0, 0);

    /* nvֵΪ1��ʾ��at sever����1��ʾ��at sever����at sever�ر�pcui��ctrl�� */
    if (VOS_TRUE == g_stDmsMainInfo.bPortOpenFlg)
    {
        /* �ر�PCUIͨ�� */
        ulRet = DMS_VcomPcuiClose();
        if(ERR_MSP_SUCCESS!=ulRet)
        {
            DMS_DBG_SDM_FUN(DMS_SDM_USB_DISABLE_ERR,0, 0, 3);
        }

        /* �ر�CTRLͨ�� */
        ulRet = DMS_VcomCtrlClose();
        if(ERR_MSP_SUCCESS!=ulRet)
        {
            DMS_DBG_SDM_FUN(DMS_SDM_USB_DISABLE_ERR,0, 0, 5);
        }

        /* �ر�PCUI2ͨ�� */
        ulRet = DMS_VcomPcui2Close();
        if(ERR_MSP_SUCCESS != ulRet)
        {
            DMS_DBG_SDM_FUN(DMS_SDM_USB_DISABLE_ERR,0, 0, 6);
        }

    }

    /* �ر�NDIS CTRLͨ�� */
    ulRet =  DMS_NcmClose();
    if(ERR_MSP_SUCCESS!=ulRet)
    {
        DMS_DBG_SDM_FUN(DMS_SDM_USB_DISABLE_ERR,0, 0, 4);
    }

    return ;
}


VOS_VOID DMS_UsbEnableCB(VOS_VOID)
{
    VOS_UINT32                          ulRet;
    NVE_INFO_S                          stAtServerNv;

    TAF_MEM_SET_S(&stAtServerNv, sizeof(stAtServerNv), 0x00, sizeof(NVE_INFO_S));
    TAF_MEM_CPY_S(stAtServerNv.nv_name, sizeof(stAtServerNv.nv_name), "ATSERV", sizeof("ATSERV"));
    stAtServerNv.nv_number      = NVE_AT_SERVER_INDEX;
    stAtServerNv.nv_operation   = NVE_READ_OPERATE;
    stAtServerNv.valid_size     = 1;

    DMS_DBG_SDM_FUN(DMS_SDM_USB_ENABLE, 0, 0, 0);

    /* NVEֻ��ȡһ�Σ���ȡ���ٶ�ȡ */
    if (VOS_FALSE == g_stDmsMainInfo.bPortCfgFlg)
    {
        g_dms_debug_atserv_nv_info.ulNvValue        = stAtServerNv.nv_data[0];

        g_stDmsMainInfo.bPortCfgFlg                 = VOS_TRUE;

        /* nvֵΪ1��ʾ��at sever����1��ʾ��at sever����at sever��pcui��ctrl�� */
        if ((1 != stAtServerNv.nv_data[0])
            )
        {
            g_stDmsMainInfo.ulPortCfgValue = DMS_TEST_MODE;
        }
        else
        {
            g_stDmsMainInfo.ulPortCfgValue = DMS_NORMAL_MODE;
        }
    }

    if (DMS_TEST_MODE == g_stDmsMainInfo.ulPortCfgValue)
    {
        g_stDmsMainInfo.bPortOpenFlg = VOS_TRUE;

        /*�� AT PCUI ͨ��*/
        ulRet = DMS_VcomPcuiOpen();
        if (ERR_MSP_SUCCESS != ulRet)
        {
            DMS_DBG_SDM_FUN(DMS_SDM_USB_ENABLE_ERR, 0, 0, 3);
        }

        /*�� AT CTRL ͨ��*/
        ulRet = DMS_VcomCtrlOpen();
        if (ERR_MSP_SUCCESS != ulRet)
        {
            DMS_DBG_SDM_FUN(DMS_SDM_USB_ENABLE_ERR, 0, 0, 5);
        }

        /*�� AT PCUI2 ͨ��*/
        ulRet = DMS_VcomPcui2Open();
        if (ERR_MSP_SUCCESS != ulRet)
        {
            DMS_DBG_SDM_FUN(DMS_SDM_USB_ENABLE_ERR, 0, 0, 6);
        }
    }

    /*�� NDIS CTRL ͨ��*/
    ulRet = DMS_NcmOpen();
    if (ERR_MSP_SUCCESS != ulRet)
    {
        DMS_DBG_SDM_FUN(DMS_SDM_USB_ENABLE_ERR, 0, 0, 4);
    }

    g_ulNdisCfgFlag = 1;

    return ;
}


UDI_HANDLE DMS_GetPortHandle(DMS_PHY_BEAR_ENUM enPhyBear)
{
    return (DMS_GetMainInfo()->stPhyProperty[enPhyBear].lPortHandle);
}


VOS_UINT32 DMS_DsFidInit(enum VOS_INIT_PHASE_DEFINE ip)
{
    VOS_UINT32 ulRelVal = 0;

    switch (ip)
    {
    case VOS_IP_LOAD_CONFIG:

        DMS_Init();

        ulRelVal = VOS_RegisterPIDInfo(WUEPS_PID_AT, (Init_Fun_Type) At_PidInit, (Msg_Fun_Type) At_MsgProc);
        if (ulRelVal != VOS_OK)
        {
            return VOS_ERR;
        }

        ulRelVal = VOS_RegisterTaskPrio(MSP_APP_DS_FID, DMS_APP_DS_TASK_PRIORITY);
        if (ulRelVal != VOS_OK)
        {
            return VOS_ERR;
        }

        break;

    default:
        break;
    }

    return VOS_OK;
}


VOS_INT __init DMS_InitPorCfgFile(VOS_VOID)
{
    printk("DMS_InitPorCfgFile,entry,%u",VOS_GetSlice());

    g_stDmsMainInfo.bPortCfgFlg     = VOS_FALSE;
    g_stDmsMainInfo.bPortOpenFlg    = VOS_FALSE;
    g_stDmsMainInfo.ulPortCfgValue  = DMS_TEST_MODE;

    if (VOS_NULL_PTR == proc_create("portcfg", DMS_VFILE_CRT_LEVEL, VOS_NULL_PTR, &g_stPortCfgOps))
    {
        DMS_LOG_ERROR("DMS_InitPorCfgFile: proc_create return NULL.\n");
        return -ENOMEM;
    }

    printk("DMS_InitPorCfgFile,exit,%u",VOS_GetSlice());

    return 0;
}


ssize_t DMS_ReadPortCfgFile(
    struct file                        *file,
    char __user                        *buf,
    size_t                              len,
    loff_t                             *ppos
)
{
    VOS_CHAR                            acModeTemp[DMS_PORTCFG_FILE_LEN];
    VOS_UINT32                          ulLength;

    if (*ppos > 0)
    {
        return 0;
    }

    TAF_MEM_SET_S(acModeTemp, sizeof(acModeTemp), 0x00, DMS_PORTCFG_FILE_LEN);

    VOS_sprintf_s((VOS_CHAR *)acModeTemp, sizeof(acModeTemp), "%d", g_stDmsMainInfo.ulPortCfgValue);

    ulLength        = VOS_StrLen(acModeTemp);
    len             = PS_MIN(len, ulLength);

    /*�����ں˿ռ����ݵ��û��ռ�����*/
    if (0 == copy_to_user(buf,(VOS_VOID *)acModeTemp, (VOS_ULONG)len))
    {
        *ppos += (loff_t)len;

        return (ssize_t)len;
    }
    else
    {
        return -EPERM;
    }

}


ssize_t DMS_WritePortCfgFile(
    struct file                        *file,
    const char __user                  *buf,
    size_t                              len,
    loff_t                             *ppos
)
{
    VOS_CHAR                            acModeTemp[DMS_PORTCFG_FILE_LEN];
    VOS_UINT32                          ulStrLen;
    VOS_UINT32                          i;
    VOS_UINT32                          ulValue;

    ulValue = 0;
    TAF_MEM_SET_S(acModeTemp, sizeof(acModeTemp), 0x00, DMS_PORTCFG_FILE_LEN);

    if (NULL == buf)
    {
        return -EFAULT;
    }

    if (len >= DMS_PORTCFG_FILE_LEN)
    {
        return -ENOSPC;
    }

    /*�����û��ռ����ݵ��ں˿ռ�����*/
    if (copy_from_user((VOS_VOID *)acModeTemp, (VOS_VOID *)buf, (VOS_ULONG)len) > 0)
    {
        return -EFAULT;
    }

    acModeTemp[len] = '\0';

    ulStrLen = VOS_StrLen(acModeTemp);

    for ( i = 0; i < ulStrLen; i++ )
    {
        if ( (acModeTemp[i] >= '0') && (acModeTemp[i] <= '9') )
        {
            ulValue = (ulValue * 10) + (acModeTemp[i] - '0');
        }
    }

    g_stDmsMainInfo.ulPortCfgValue  = ulValue;

    /* ����Ѿ�д������ļ�������д��ֵΪ׼����������Ҫ�ٶ�NVE */
    g_stDmsMainInfo.bPortCfgFlg     = TRUE;

    return (ssize_t)len;
}

module_init(DMS_InitPorCfgFile);
module_init(DMS_InitGetSliceFile);


VOS_UINT32 DMS_RegOmChanDataReadCB(
    DMS_OM_CHAN_ENUM_UINT32             enChan,
    DMS_OM_CHAN_DATA_READ_CB_FUNC       pFunc
)
{
    DMS_NLK_OM_CHAN_PROPERTY_STRU      *pstOmChanProp = VOS_NULL_PTR;

    /* ���ͨ���źͺ���ָ�� */
    if ((enChan >= DMS_OM_CHAN_BUTT) || (VOS_NULL_PTR == pFunc))
    {
        printk(KERN_ERR "[%s][LINE: %d] Invalid channel %d.\n",
            __func__, __LINE__, (VOS_INT)enChan);
        return VOS_ERR;
    }

    /* ����ͨ�����ݻص����� */
    pstOmChanProp = DMS_GET_NLK_OM_CHAN_PROP(enChan);
    pstOmChanProp->pDataFunc = pFunc;

    return VOS_OK;
}


VOS_UINT32 DMS_RegOmChanEventCB(
    DMS_OM_CHAN_ENUM_UINT32             enChan,
    DMS_OM_CHAN_EVENT_CB_FUNC           pFunc
)
{
    DMS_NLK_OM_CHAN_PROPERTY_STRU      *pstOmChanProp = VOS_NULL_PTR;

    /* ���ͨ���� */
    if ((enChan >= DMS_OM_CHAN_BUTT) || (VOS_NULL_PTR == pFunc))
    {
        printk(KERN_ERR "[%s][LINE: %d] Invalid channel %d.\n",
            __func__, __LINE__, (VOS_INT)enChan);
        return VOS_ERR;
    }

    /* ����ͨ���¼��ص����� */
    pstOmChanProp = DMS_GET_NLK_OM_CHAN_PROP(enChan);
    pstOmChanProp->pEvtFunc = pFunc;

    return VOS_OK;
}


VOS_UINT32 DMS_WriteOmData(
    DMS_OM_CHAN_ENUM_UINT32             enChan,
    VOS_UINT8                          *pucData,
    VOS_UINT32                          ulLength
)
{
    VOS_UINT8                          *pucMem = VOS_NULL_PTR;
    VOS_UINT32                          ulMemNum;
    VOS_UINT32                          ulLastMemSize;
    VOS_UINT32                          ulCnt;

    if (DMS_GET_PRINT_MODEM_LOG_TYPE())
    {
        DMS_DBG_NLK_UL_UNSUPPORT_WRITE_LOG_NUM(1);
        return VOS_ERR;
    }

    DMS_DBG_NLK_DL_TOTAL_PKT_NUM(1);

    /* ���ͨ�� */
    if (enChan >= DMS_OM_CHAN_BUTT)
    {
        DMS_DBG_NLK_DL_ERR_CHAN_PKT_NUM(1);
        return VOS_ERR;
    }

    /* ������� */
    if ((VOS_NULL_PTR == pucData) || (0 == ulLength))
    {
        DMS_DBG_NLK_DL_ERR_PARA_PKT_NUM(1);
        return VOS_ERR;
    }

    DMS_DBG_NLK_DL_NORM_CHAN_PKT_NUM(enChan, 1);

    /* �����ݷֿ�, ����һ�η��͹������� */
    pucMem        = pucData;
    ulMemNum      = (ulLength >= DMS_GET_NLK_THRESH_SIZE())?(ulLength / DMS_GET_NLK_DATA_SIZE()):0;
    ulLastMemSize = (ulLength >= DMS_GET_NLK_THRESH_SIZE())?(ulLength % DMS_GET_NLK_DATA_SIZE()):ulLength;

    wake_lock(&g_stDmsMainInfo.stwakelock);

    /* ���͹̶���С���ݿ� */
    for (ulCnt = 0; ulCnt < ulMemNum; ulCnt++)
    {
        DMS_NLK_Send(DMS_GET_NLK_PHY_BEAR(enChan), DMS_GET_NLK_MSG_TYPE(enChan), pucMem, DMS_GET_NLK_DATA_SIZE());
        pucMem += DMS_GET_NLK_DATA_SIZE();
    }

    /* �������һ�����ݿ� */
    if (0 != ulLastMemSize)
    {
        DMS_NLK_Send(DMS_GET_NLK_PHY_BEAR(enChan), DMS_GET_NLK_MSG_TYPE(enChan), pucMem, ulLastMemSize);
    }

    wake_unlock(&g_stDmsMainInfo.stwakelock);

    return VOS_OK;
}


VOS_VOID DMS_NLK_InitEntity(VOS_VOID)
{
    DMS_NLK_ENTITY_STRU                *pstNlkEntity = VOS_NULL_PTR;

    pstNlkEntity = DMS_GET_NLK_ENTITY();

    /* netlink socket */
    pstNlkEntity->pstSock    = VOS_NULL_PTR;

    /* netlink ��Ϣ���ݿ��С */
    pstNlkEntity->ulDataSize   = DMS_NLK_DEFUALT_DATA_SIZE;

    /* netlink ��Ϣ���ݿ��С */
    pstNlkEntity->ulThreshSize = DMS_NLK_DEFUALT_DATA_SIZE * 2;

    /* netlink ������ؽ��̺� */
    pstNlkEntity->astPhyBearProp[DMS_NLK_PHY_BEAR_LTE].lPid     = DMS_NLK_INVALID_PID;
    pstNlkEntity->astPhyBearProp[DMS_NLK_PHY_BEAR_GU].lPid      = DMS_NLK_INVALID_PID;

    /* netlink �߼�ͨ������(LTE CLTR) */
    pstNlkEntity->astOmChanProp[DMS_OM_CHAN_LTE_CTRL].enPhyBear = DMS_NLK_PHY_BEAR_LTE;
    pstNlkEntity->astOmChanProp[DMS_OM_CHAN_LTE_CTRL].enMsgType = DMS_NLK_MSG_TYPE_LTE_CTRL;

    /* netlink �߼�ͨ������(LTE DATA) */
    pstNlkEntity->astOmChanProp[DMS_OM_CHAN_LTE_DATA].enPhyBear = DMS_NLK_PHY_BEAR_LTE;
    pstNlkEntity->astOmChanProp[DMS_OM_CHAN_LTE_DATA].enMsgType = DMS_NLK_MSG_TYPE_LTE_DATA;

    /* netlink �߼�ͨ������(GU DATA) */
    pstNlkEntity->astOmChanProp[DMS_OM_CHAN_GU_DATA].enPhyBear  = DMS_NLK_PHY_BEAR_GU;
    pstNlkEntity->astOmChanProp[DMS_OM_CHAN_GU_DATA].enMsgType  = DMS_NLK_MSG_TYPE_GU_DATA;

    return;
}


VOS_UINT32 DMS_NLK_CfgOpen(
    struct nlmsghdr                    *pstNlkHdr,
    DMS_NLK_PHY_BEAR_ENUM_UINT32        enPhyBear
)
{
    DMS_NLK_PHY_BEAR_PROPERTY_STRU     *pstPhyBearProp = VOS_NULL_PTR;
    DMS_OM_CHAN_EVENT_CB_FUNC           pEvtFunc       = VOS_NULL_PTR;
    DMS_OM_CHAN_ENUM_UINT32             enChan;

    /* �����غ� */
    if (enPhyBear >= DMS_NLK_PHY_BEAR_BUTT)
    {
        printk("[%s][LINE: %d] Invalid PHY bearer %d.\n",
            __func__, __LINE__, (VOS_INT)enPhyBear);
        return VOS_ERR;
    }

    /* ���ó���PID */
    pstPhyBearProp = DMS_GET_NLK_PHY_BEAR_PROP(enPhyBear);
    pstPhyBearProp->lPid = pstNlkHdr->nlmsg_pid;

    /* ����������ó��ع�����ͨ�� */
    for (enChan = 0; enChan < DMS_OM_CHAN_BUTT; enChan++)
    {
        pEvtFunc = DMS_GET_NLK_OM_CHAN_EVT_CB_FUNC(enChan);

        /* ֪ͨ���������ͬ���߼�ͨ��ʹ����ͨ���� */
        if ((enPhyBear == DMS_GET_NLK_PHY_BEAR(enChan)) && (VOS_NULL_PTR != pEvtFunc))
        {
            pEvtFunc(enChan, DMS_CHAN_EVT_OPEN);
        }
    }

    return VOS_OK;
}


VOS_UINT32 DMS_NLK_CfgClose(
    struct nlmsghdr                    *pstNlkHdr,
    DMS_NLK_PHY_BEAR_ENUM_UINT32        enBear
)
{
    DMS_NLK_PHY_BEAR_PROPERTY_STRU     *pstPhyBearProp = VOS_NULL_PTR;
    DMS_OM_CHAN_EVENT_CB_FUNC           pEvtFunc       = VOS_NULL_PTR;
    DMS_OM_CHAN_ENUM_UINT32             enChan;

    /* �����غ� */
    if (enBear >= DMS_NLK_PHY_BEAR_BUTT)
    {
        printk("[%s][LINE: %d] Invalid PHY bearer %d.\n",
            __func__, __LINE__, (VOS_INT)enBear);
        return VOS_ERR;
    }

    /* ���ó���PID */
    pstPhyBearProp = DMS_GET_NLK_PHY_BEAR_PROP(enBear);
    pstPhyBearProp->lPid = DMS_NLK_INVALID_PID;

    /* ����������ó��ع�����ͨ�� */
    for (enChan = 0; enChan < DMS_OM_CHAN_BUTT; enChan++)
    {
        pEvtFunc = DMS_GET_NLK_OM_CHAN_EVT_CB_FUNC(enChan);

        /* ֪ͨ���������ͬ���߼�ͨ��ʹ����ͨ���ر� */
        if ((enBear == DMS_GET_NLK_PHY_BEAR(enChan)) && (VOS_NULL_PTR != pEvtFunc))
        {
            pEvtFunc(enChan, DMS_CHAN_EVT_CLOSE);
        }
    }

    return VOS_OK;
}


VOS_VOID DMS_NLK_ProcLteCfgMsg(struct nlmsghdr *pstNlkHdr)
{
    DMS_NLK_CFG_STRU                   *pstMsg = VOS_NULL_PTR;

    pstMsg = nlmsg_data(pstNlkHdr);

    switch (pstMsg->enCfg)
    {
        case DMS_NLK_CFG_TYPE_OPEN:
            (VOS_VOID)DMS_NLK_CfgOpen(pstNlkHdr, DMS_NLK_PHY_BEAR_LTE);
            DMS_DBG_NLK_UL_SEND_MSG_NUM(pstNlkHdr->nlmsg_type, 1);
            break;

        case DMS_NLK_CFG_TYPE_CLOSE:
            (VOS_VOID)DMS_NLK_CfgClose(pstNlkHdr, DMS_NLK_PHY_BEAR_LTE);
            DMS_DBG_NLK_UL_SEND_MSG_NUM(pstNlkHdr->nlmsg_type, 1);
            break;

        default:
            DMS_DBG_NLK_UL_FREE_MSG_NUM(pstNlkHdr->nlmsg_type, 1);
            break;
    }

    return;
}


VOS_VOID DMS_NLK_ProcGuCfgMsg(struct nlmsghdr *pstNlkHdr)
{
    DMS_NLK_CFG_STRU                   *pstMsg = VOS_NULL_PTR;

    pstMsg = nlmsg_data(pstNlkHdr);

    switch (pstMsg->enCfg)
    {
        case DMS_NLK_CFG_TYPE_OPEN:
            (VOS_VOID)DMS_NLK_CfgOpen(pstNlkHdr, DMS_NLK_PHY_BEAR_GU);
            DMS_DBG_NLK_UL_SEND_MSG_NUM(pstNlkHdr->nlmsg_type, 1);
            break;

        case DMS_NLK_CFG_TYPE_CLOSE:
            (VOS_VOID)DMS_NLK_CfgClose(pstNlkHdr, DMS_NLK_PHY_BEAR_GU);
            DMS_DBG_NLK_UL_SEND_MSG_NUM(pstNlkHdr->nlmsg_type, 1);
            break;

        default:
            DMS_DBG_NLK_UL_FREE_MSG_NUM(pstNlkHdr->nlmsg_type, 1);
            break;
    }

    return;
}


VOS_VOID DMS_NLK_ProcLteCtrlMsg(struct nlmsghdr *pstNlkHdr)
{
    DMS_OM_CHAN_DATA_READ_CB_FUNC       pDataFunc  = VOS_NULL_PTR;
    DMS_NLK_PAYLOAD_STRU               *pstPayload = VOS_NULL_PTR;

    /* ��ȡͨ��ע��Ļص� */
    pDataFunc = DMS_GET_NLK_OM_CHAN_DATA_CB_FUNC(DMS_OM_CHAN_LTE_CTRL);
    if (VOS_NULL_PTR != pDataFunc)
    {
        /* ͸���������� */
        pstPayload = nlmsg_data(pstNlkHdr);
        (VOS_VOID)pDataFunc(DMS_OM_CHAN_LTE_CTRL, pstPayload->aucData, pstPayload->ulLength);
        DMS_DBG_NLK_UL_SEND_MSG_NUM(pstNlkHdr->nlmsg_type, 1);
    }
    else
    {
        DMS_DBG_NLK_UL_FREE_MSG_NUM(pstNlkHdr->nlmsg_type, 1);
    }

    return;
}


VOS_VOID DMS_NLK_ProcLteDataMsg(struct nlmsghdr *pstNlkHdr)
{
    DMS_OM_CHAN_DATA_READ_CB_FUNC       pDataFunc  = VOS_NULL_PTR;
    DMS_NLK_PAYLOAD_STRU               *pstPayload = VOS_NULL_PTR;

    /* ��ȡͨ��ע��Ļص� */
    pDataFunc = DMS_GET_NLK_OM_CHAN_DATA_CB_FUNC(DMS_OM_CHAN_LTE_DATA);
    if (VOS_NULL_PTR != pDataFunc)
    {
        /* ͸���������� */
        pstPayload = nlmsg_data(pstNlkHdr);
        (VOS_VOID)pDataFunc(DMS_OM_CHAN_LTE_DATA, pstPayload->aucData, pstPayload->ulLength);
        DMS_DBG_NLK_UL_SEND_MSG_NUM(pstNlkHdr->nlmsg_type, 1);
    }
    else
    {
        DMS_DBG_NLK_UL_FREE_MSG_NUM(pstNlkHdr->nlmsg_type, 1);
    }

    return;
}


VOS_VOID DMS_NLK_ProcGuDataMsg(struct nlmsghdr *pstNlkHdr)
{
    DMS_OM_CHAN_DATA_READ_CB_FUNC       pDataFunc = VOS_NULL_PTR;
    DMS_NLK_PAYLOAD_STRU               *pstPayload = VOS_NULL_PTR;

    /* ��ȡͨ��ע��Ļص� */
    pDataFunc = DMS_GET_NLK_OM_CHAN_DATA_CB_FUNC(DMS_OM_CHAN_GU_DATA);
    if (VOS_NULL_PTR != pDataFunc)
    {
        /* ͸���������� */
        pstPayload = nlmsg_data(pstNlkHdr);
        (VOS_VOID)pDataFunc(DMS_OM_CHAN_GU_DATA, pstPayload->aucData, pstPayload->ulLength);
        DMS_DBG_NLK_UL_SEND_MSG_NUM(pstNlkHdr->nlmsg_type, 1);
    }
    else
    {
        DMS_DBG_NLK_UL_FREE_MSG_NUM(pstNlkHdr->nlmsg_type, 1);
    }

    return;
}


VOS_INT DMS_NLK_Send(
    DMS_NLK_PHY_BEAR_ENUM_UINT32        enPhyBear,
    DMS_NLK_MSG_TYPE_ENUM_UINT32        enMsgType,
    VOS_UINT8                          *pucData,
    VOS_UINT32                          ulLength
)
{
    struct sk_buff                     *pstSkb      = VOS_NULL_PTR;
    struct nlmsghdr                    *pstNlkHdr   = VOS_NULL_PTR;
    DMS_NLK_PAYLOAD_STRU               *pstPlayload = VOS_NULL_PTR;
    VOS_UINT                            ulMsgSize;
    VOS_UINT                            ulPayloadSize;
    VOS_INT                             lRet;

    DMS_DBG_NLK_DL_TOTAL_MSG_NUM(1);

    /* ��� netlink socket */
    if (VOS_NULL_PTR == DMS_GET_NLK_SOCK())
    {
        DMS_DBG_NLK_DL_ERR_SOCK_MSG_NUM(1);
        return -EIO;
    }

    /* ����������״̬ */
    if (DMS_NLK_INVALID_PID == DMS_GET_NLK_PHY_PID(enPhyBear))
    {
        DMS_DBG_NLK_DL_ERR_PID_MSG_NUM(1);
        return -EINVAL;
    }

    /* ���� netlink ��Ϣ */
    ulPayloadSize = sizeof(DMS_NLK_PAYLOAD_STRU) + ulLength;
    ulMsgSize = NLMSG_SPACE(ulPayloadSize);

    pstSkb = nlmsg_new(ulPayloadSize, GFP_ATOMIC);
    if (VOS_NULL_PTR == pstSkb)
    {
        DMS_DBG_NLK_DL_ALLOC_MSG_FAIL_NUM(1);
        return -ENOBUFS;
    }

    /* ��� netlink ��Ϣͷ */
    /* Use "ulMsgSize - sizeof(*pstNlkHdr)" here (incluing align pads) */
    pstNlkHdr = nlmsg_put(pstSkb, 0, 0, (VOS_INT)enMsgType,
                    (VOS_INT)(ulMsgSize - sizeof(struct nlmsghdr)), 0);
    if (VOS_NULL_PTR == pstNlkHdr)
    {
        kfree_skb(pstSkb);
        DMS_DBG_NLK_DL_PUT_MSG_FAIL_NUM(1);
        return -EMSGSIZE;
    }

    /* ��� netlink ��Ϣ����PID */
#if ((LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0)) || (VOS_OS_VER == VOS_WIN32))
    NETLINK_CB(pstSkb).portid = DMS_GET_NLK_PHY_PID(enPhyBear);
#else
    NETLINK_CB(pstSkb).pid = DMS_GET_NLK_PHY_PID(enPhyBear);
#endif
    NETLINK_CB(pstSkb).dst_group = 0;

    /* ��� netlink ��Ϣ���� */
    pstPlayload = nlmsg_data(pstNlkHdr);
    pstPlayload->ulLength = ulLength;
    memset(pstPlayload->aucReserved, 0x00, sizeof(pstPlayload->aucReserved));/* unsafe_function_ignore: memset */

    memcpy(pstPlayload->aucData, pucData, ulLength);/* unsafe_function_ignore: memcpy */

    /* ���� netlink ��Ϣ */
#if ((LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0)) || (VOS_OS_VER == VOS_WIN32))
    lRet = netlink_unicast(DMS_GET_NLK_SOCK(), pstSkb, NETLINK_CB(pstSkb).portid, 0);
#else
    lRet = netlink_unicast(DMS_GET_NLK_SOCK(), pstSkb, NETLINK_CB(pstSkb).pid, 0);
#endif

    if (lRet < 0)
    {
        DMS_DBG_NLK_DL_UNICAST_MSG_FAIL_NUM(1);
        return lRet;
    }

    DMS_DBG_NLK_DL_UNICAST_MSG_SUCC_NUM(1);
    return 0;
}


VOS_VOID DMS_NLK_Input(struct sk_buff *pstSkb)
{
    struct nlmsghdr                    *pstNlkHdr = VOS_NULL_PTR;

    if (DMS_GET_PRINT_MODEM_LOG_TYPE())
    {
        DMS_DBG_NLK_UL_UNSUPPORT_INPUT_LOG_NUM(1);
        return;
    }

    DMS_DBG_NLK_UL_TOTAL_MSG_NUM(1);

    /* ��ȡ netlink ��Ϣ */
    pstNlkHdr = nlmsg_hdr(pstSkb);

    /* ��� netlink ��Ϣ�Ƿ�Ϸ� */
    if (!NLMSG_OK(pstNlkHdr, pstSkb->len))
    {
        DMS_DBG_NLK_UL_ERR_MSG_NUM(1);
        return;
    }

    /* ���� netlink ��Ϣ */
    switch (pstNlkHdr->nlmsg_type)
    {
        case DMS_NLK_MSG_TYPE_LTE_CFG:
            DMS_NLK_ProcLteCfgMsg(pstNlkHdr);
            break;

        case DMS_NLK_MSG_TYPE_LTE_CTRL:
            DMS_NLK_ProcLteCtrlMsg(pstNlkHdr);
            break;

        case DMS_NLK_MSG_TYPE_LTE_DATA:
            DMS_NLK_ProcLteDataMsg(pstNlkHdr);
            break;

        case DMS_NLK_MSG_TYPE_GU_CFG:
            DMS_NLK_ProcGuCfgMsg(pstNlkHdr);
            break;

        case DMS_NLK_MSG_TYPE_GU_DATA:
            DMS_NLK_ProcGuDataMsg(pstNlkHdr);
            break;

        default:
            DMS_DBG_NLK_UL_UNKNOWN_MSG_NUM(1);
            break;
    }

    return;
}


VOS_INT __init DMS_NLK_Init(VOS_VOID)
{
    struct sock                        *pstSock      = VOS_NULL_PTR;
    DMS_NLK_ENTITY_STRU                *pstNlkEntity = VOS_NULL_PTR;

    printk("DMS_NLK_Init,entry,%u",VOS_GetSlice());

    /* ��ʼ�� netlink ʵ�� */
    DMS_NLK_InitEntity();

    /* ���ں�̬����һ�� netlink socket */
#if ((LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0)) || (VOS_OS_VER == VOS_WIN32))
    pstSock = netlink_kernel_create(&init_net, NETLINK_HW_LOGCAT, &g_stDmsNlkCfg);
#else
    pstSock = netlink_kernel_create(&init_net, NETLINK_HW_LOGCAT, 0,
                                DMS_NLK_Input, NULL, THIS_MODULE);
#endif

    if (VOS_NULL_PTR == pstSock)
    {
        printk(KERN_ERR "[%s][LINE: %d] Fail to create netlink socket.\n",
            __func__, __LINE__);
        DMS_DBG_NLK_CREATE_SOCK_FAIL_NUM(1);
        return -ENOMEM;
    }

    /* ���� socket */
    pstNlkEntity = DMS_GET_NLK_ENTITY();
    pstNlkEntity->pstSock = pstSock;

    printk("DMS_NLK_Init,exit,%u",VOS_GetSlice());

    return 0;
}


VOS_VOID __exit DMS_NLK_Exit(VOS_VOID)
{
    /* �ͷ� netlink socket */
    netlink_kernel_release(DMS_GET_NLK_SOCK());
    DMS_NLK_InitEntity();
    return;
}


VOS_INT __init DMS_InitGetSliceFile(VOS_VOID)
{
    if (VOS_NULL_PTR == proc_create("getslice", DMS_VFILE_CRT_LEVEL, VOS_NULL_PTR, &g_stGetSliceOps))
    {
        DMS_LOG_ERROR("DMS_InitGetSliceFile: proc_create return NULL.\n");
        return VOS_ERR;
    }

    return VOS_OK;
}


ssize_t DMS_ReadGetSliceFile(
    struct file                        *file,
    char __user                        *buf,
    size_t                              len,
    loff_t                             *ppos
)
{
    VOS_CHAR                            acModeTemp[DMS_GET_SLICE_FILE_LEN];
    VOS_UINT32                          ulLength;
    VOS_UINT32                          ulHigh32bitValue;
    VOS_UINT32                          ulLow32bitValue;

    if (*ppos > 0)
    {
        return 0;
    }

    /* ��ʼ�� */
    ulLength            = 0;
    ulHigh32bitValue    = 0;
    ulLow32bitValue     = 0;

    TAF_MEM_SET_S(acModeTemp, sizeof(acModeTemp), 0x00, DMS_GET_SLICE_FILE_LEN);

    /* ��ȡʱ��  Seattle �� Portland��һ�� */
    (VOS_VOID)mdrv_timer_get_accuracy_timestamp(&ulHigh32bitValue, &ulLow32bitValue);

    if (ulHigh32bitValue != 0)
    {
        VOS_sprintf_s((VOS_CHAR *)acModeTemp, sizeof(acModeTemp), "%x%08x", ulHigh32bitValue, ulLow32bitValue);
    }
    else
    {
        VOS_sprintf_s((VOS_CHAR *)acModeTemp, sizeof(acModeTemp), "%x", ulLow32bitValue);
    }

    ulLength        = VOS_StrLen(acModeTemp);
    len             = PS_MIN(len, ulLength);

    /*�����ں˿ռ����ݵ��û��ռ�����*/
    if (0 == copy_to_user(buf,(VOS_VOID *)acModeTemp, (VOS_ULONG)len))
    {
        *ppos += (loff_t)len;

        return (ssize_t)len;
    }
    else
    {
        return -EPERM;
    }

}


VOS_INT __init DMS_InitModemStatusFile(VOS_VOID)
{
    DMS_LOG_INFO("DMS_InitModemStatusFile,entry,%u \n",VOS_GetSlice());

    DMS_InitModemStatus();

    if (VOS_NULL_PTR == proc_create("modemstatus", DMS_VFILE_CRT_LEVEL, VOS_NULL_PTR, &g_stModemStatus))
    {
        DMS_LOG_ERROR("DMS_InitModemStatusFile: proc_create return NULL.\n");
        return -ENOMEM;
    }

    DMS_LOG_INFO("DMS_InitModemStatusFile,exit,%u \n",VOS_GetSlice());

    return 0;
}


ssize_t DMS_ReadModemStatusFile(
    struct file                        *file,
    char __user                        *buf,
    size_t                              len,
    loff_t                             *ppos
)
{
    VOS_CHAR                            acStatus[DMS_MODEM_STATUS_FILE_LEN + 1];
    VOS_UINT32                          ulStatusLen;

    if (*ppos > 0)
    {
        return 0;
    }

    TAF_MEM_SET_S(acStatus, sizeof(acStatus), 0x00, DMS_MODEM_STATUS_FILE_LEN + 1);

    DMS_LOG_INFO("DMS_ReadModemStatusFile entry,time: %u, status: %d, %d, %d\n",
                 VOS_GetSlice(),
                 g_stDmsMainInfo.aucModemStatus[0],
                 g_stDmsMainInfo.aucModemStatus[1],
                 g_stDmsMainInfo.aucModemStatus[2]);

    /* ��ȡ���貦�ŵ�ģʽ�Լ�ʱ���ĵ�ַ */
    VOS_sprintf_s((VOS_CHAR *)acStatus, sizeof(acStatus), "%s", g_stDmsMainInfo.aucModemStatus);

    ulStatusLen = VOS_StrLen(acStatus);

    len = PS_MIN(len, ulStatusLen);

    /*�����ں˿ռ����ݵ��û��ռ�����*/
    if (0 == copy_to_user(buf,(VOS_VOID *)acStatus, (VOS_ULONG)len))
    {
        *ppos += (loff_t)len;

        return (ssize_t)len;
    }
    else
    {
        DMS_LOG_ERROR("DMS_ReadModemStatusFile copy to user fail %u \n",VOS_GetSlice());
        return -EPERM;
    }
}


VOS_VOID DMS_SetModemStatus(MODEM_ID_ENUM_UINT16 enModemId)
{
    if (enModemId >= MODEM_ID_BUTT)
    {
        DMS_LOG_ERROR("DMS_SetModemStatus ModemId error %d \n",enModemId);
        return;
    }

    g_stDmsMainInfo.aucModemStatus[enModemId] = VOS_TRUE;
}


VOS_UINT8 DMS_GetModemStatus(MODEM_ID_ENUM_UINT16 enModemId)
{
    if (enModemId >= MODEM_ID_BUTT)
    {
        DMS_LOG_ERROR("DMS_GetModemStatus ModemId error %d \n",enModemId);
        return g_stDmsMainInfo.aucModemStatus[0];
    }

    return g_stDmsMainInfo.aucModemStatus[enModemId];
}


VOS_VOID DMS_InitModemStatus(VOS_VOID)
{
    TAF_MEM_SET_S(g_stDmsMainInfo.aucModemStatus, sizeof(g_stDmsMainInfo.aucModemStatus), 0x00, sizeof(g_stDmsMainInfo.aucModemStatus));
}


/* This function is called on driver initialization and exit */
module_init(DMS_InitModemStatusFile);
module_init(DMS_NLK_Init);
module_exit(DMS_NLK_Exit);



