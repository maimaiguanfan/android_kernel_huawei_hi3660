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

#include <dms.h>
#include "dms_core.h"




/*****************************************************************************
    Э��ջ��ӡ��㷽ʽ�µ�.C�ļ��궨��
*****************************************************************************/

#define THIS_FILE_ID                    PS_FILE_ID_DMS_DEBUG_C


/*****************************************************************************
  2 ȫ�ֱ�������
*****************************************************************************/

DMS_DEBUG_INFO_TBL_STRU                 g_astDmsSdmInfoTable[DMS_SDM_DBG_INFO_MAX];
DMS_DEBUG_AT_SERV_NV_STRU               g_dms_debug_atserv_nv_info = {0};
VOS_UINT32                              g_ulDmsDebugLevel = DMS_LOG_LEVEL_NONE;
DMS_MNTN_NLK_STATS_STRU                 g_stDmsMntnNlkStats = {0};


/*****************************************************************************
  3 ����ʵ��
*****************************************************************************/

DMS_DEBUG_INFO_TBL_STRU* DMS_GetDebugInfo(VOS_VOID)
{
   return &(g_astDmsSdmInfoTable[0]);
}

VOS_VOID DMS_Debug(DMS_SDM_MSG_ID_ENUM ulType,
    VOS_UINT32 ulRserved1, VOS_UINT32 ulRserved2, VOS_UINT32 ulRserved3)
{
    g_astDmsSdmInfoTable[ulType].ulCalledNum += 1;
    g_astDmsSdmInfoTable[ulType].ulRserved1 = ulRserved1;
    g_astDmsSdmInfoTable[ulType].ulRserved2 = ulRserved2;
    g_astDmsSdmInfoTable[ulType].ulRserved3 = ulRserved3;
    g_astDmsSdmInfoTable[ulType].ulRtcTime  = VOS_GetTick();
    return;
}

VOS_VOID DMS_Debug_SetDataSize(VOS_UINT32 ullength)
{
    DMS_NLK_ENTITY_STRU                *pstNlkEntity = VOS_NULL_PTR;

    pstNlkEntity = DMS_GET_NLK_ENTITY();

    pstNlkEntity->ulThreshSize = ullength;

    return;
}


VOS_VOID DMS_SetLogLevel(VOS_UINT32 ulLvl)
{
    if (ulLvl >= DMS_LOG_LEVEL_TOP)
    {
        g_ulDmsDebugLevel = DMS_LOG_LEVEL_TOP;
    }
    else
    {
        g_ulDmsDebugLevel = ulLvl;
    }

    return;
}

VOS_VOID DMS_ShowDebugInfo(VOS_VOID)
{
    DMS_MAIN_INFO                      *pstMainInfo = DMS_GetMainInfo();

    (VOS_VOID)vos_printf("DMS_ShowDebugInfo: pfnAcmReadData = %p\n", pfnAcmReadData);
    (VOS_VOID)vos_printf("DMS_ShowDebugInfo: pstMainInfo->pfnRdDataCallback = %p\n", pstMainInfo->pfnRdDataCallback);
    return;
}

VOS_VOID DMS_ShowAtServerNvInfo(VOS_VOID)
{
    (VOS_VOID)vos_printf("Operation return value is %d.\n", g_dms_debug_atserv_nv_info.lOperatRet);
    (VOS_VOID)vos_printf("The nv value is %d.\n", g_dms_debug_atserv_nv_info.ulNvValue);
}


VOS_VOID  DMS_ShowPortCfgInfo(VOS_VOID)
{
    (VOS_VOID)vos_printf("ulPortCfgValue: %d\n", g_stDmsMainInfo.ulPortCfgValue);
    (VOS_VOID)vos_printf("bNveReadFlg: %d\n", g_stDmsMainInfo.bPortCfgFlg);
    (VOS_VOID)vos_printf("bPortOpenFlg: %d\n", g_stDmsMainInfo.bPortOpenFlg);

    return;
}

VOS_VOID DMS_ShowVcomStats(VOS_VOID)
{
    DMS_DEBUG_INFO_TBL_STRU* pstTable;

    /* ���ĳ����Ա��ĳ�����û��ʹ�ÿ��Բ���ӡ */

    pstTable = DMS_GetDebugInfo();


    (VOS_VOID)vos_printf("*------------ENABLE-------------------\n");
    (VOS_VOID)vos_printf("USB ENABLE counts  : %d \n USB ENABLE fail counts  : %d \n USB ENABLE fail branch  : %d \n\n",
        pstTable[DMS_SDM_USB_ENABLE].ulCalledNum,
        pstTable[DMS_SDM_USB_ENABLE_ERR].ulCalledNum,
        pstTable[DMS_SDM_USB_ENABLE_ERR].ulRserved3);
    (VOS_VOID)vos_printf("*------------DISABLE-------------------\n");
    (VOS_VOID)vos_printf("USB DISABLE counts  : %d \n USB DISABLE fail counts  : %d \n USB DISABLE fail branch  : %d \n\n",
        pstTable[DMS_SDM_USB_DISABLE].ulCalledNum,
        pstTable[DMS_SDM_USB_DISABLE_ERR].ulCalledNum,
        pstTable[DMS_SDM_USB_DISABLE_ERR].ulRserved3);

    (VOS_VOID)vos_printf("*------------OPEN-------------------\n");
    (VOS_VOID)vos_printf("VCOM_PCUI OPEN counts  : %d \n VCOM_PCUI OPEN fail counts  : %d \n  VCOM_PCUI OPEN fail value : %d\n\n",
        pstTable[DMS_SDM_VCOM_OPEN_PCUI].ulCalledNum,
        pstTable[DMS_SDM_VCOM_OPEN_ERR_PCUI].ulCalledNum,
        pstTable[DMS_SDM_VCOM_OPEN_ERR_PCUI].ulRserved1);
    (VOS_VOID)vos_printf("VCOM_CTRL OPEN counts  : %d \n VCOM_CTRL OPEN fail counts  : %d \n  VCOM_CTRL OPEN fail value : %d\n\n",
        pstTable[DMS_SDM_VCOM_OPEN_CTRL].ulCalledNum,
        pstTable[DMS_SDM_VCOM_OPEN_ERR_CTRL].ulCalledNum,
        pstTable[DMS_SDM_VCOM_OPEN_ERR_CTRL].ulRserved1);
    (VOS_VOID)vos_printf("VCOM_PCUI2 OPEN counts  : %d \n VCOM_PCUI2 OPEN fail counts  : %d \n  VCOM_PCUI2 OPEN fail value : %d\n\n",
        pstTable[DMS_SDM_VCOM_OPEN_PCUI2].ulCalledNum,
        pstTable[DMS_SDM_VCOM_OPEN_ERR_PCUI2].ulCalledNum,
        pstTable[DMS_SDM_VCOM_OPEN_ERR_PCUI2].ulRserved1);

    (VOS_VOID)vos_printf("*------------CLOSE-------------------\n");
    (VOS_VOID)vos_printf("VCOM_AT CLOSE counts  : %d \n VCOM_AT CLOSE fail counts  : %d \n  VCOM_AT CLOSE fail value : %d\n\n",
        pstTable[DMS_SDM_VCOM_CLOSE_PCUI].ulCalledNum,
        pstTable[DMS_SDM_VCOM_CLOSE_ERR_PCUI].ulCalledNum,
        pstTable[DMS_SDM_VCOM_CLOSE_ERR_PCUI].ulRserved1);
    (VOS_VOID)vos_printf("VCOM_CTRL CLOSE counts  : %d \n VCOM_CTRL CLOSE fail counts  : %d \n  VCOM_CTRL CLOSE fail value : %d\n\n",
        pstTable[DMS_SDM_VCOM_CLOSE_CTRL].ulCalledNum,
        pstTable[DMS_SDM_VCOM_CLOSE_ERR_CTRL].ulCalledNum,
        pstTable[DMS_SDM_VCOM_CLOSE_ERR_CTRL].ulRserved1);
    (VOS_VOID)vos_printf("VCOM_PCUI2 CLOSE counts  : %d \n VCOM_PCUI2 CLOSE fail counts  : %d \n  VCOM_PCUI2 CLOSE fail value : %d\n\n",
        pstTable[DMS_SDM_VCOM_CLOSE_PCUI2].ulCalledNum,
        pstTable[DMS_SDM_VCOM_CLOSE_ERR_PCUI2].ulCalledNum,
        pstTable[DMS_SDM_VCOM_CLOSE_ERR_PCUI2].ulRserved1);

    (VOS_VOID)vos_printf("*------------WRITE_�첽д-------------------\n");
    (VOS_VOID)vos_printf("VCOM_AT write counts  : %d \n VCOM_AT write OK counts  : %d \n  VCOM_AT Last write OK len : %d\n\n",
        pstTable[DMS_SDM_VCOM_WRT_PCUI].ulCalledNum,
        pstTable[DMS_SDM_VCOM_WRT_SUSS_PCUI].ulCalledNum,
        pstTable[DMS_SDM_VCOM_WRT_SUSS_PCUI].ulRserved1);
     (VOS_VOID)vos_printf("VCOM_CTRL write counts  : %d \n VCOM_CTRL write OK counts  : %d \n  VCOM_CTRL Last write OK len : %d\n\n",
        pstTable[DMS_SDM_VCOM_WRT_CTRL].ulCalledNum,
        pstTable[DMS_SDM_VCOM_WRT_SUSS_CTRL].ulCalledNum,
        pstTable[DMS_SDM_VCOM_WRT_SUSS_CTRL].ulRserved1);
     (VOS_VOID)vos_printf("VCOM_PCUI2 write counts  : %d \n VCOM_PCUI2 write OK counts  : %d \n  VCOM_PCUI2 Last write OK len : %d\n\n",
        pstTable[DMS_SDM_VCOM_WRT_PCUI2].ulCalledNum,
        pstTable[DMS_SDM_VCOM_WRT_SUSS_PCUI2].ulCalledNum,
        pstTable[DMS_SDM_VCOM_WRT_SUSS_PCUI2].ulRserved1);

    (VOS_VOID)vos_printf("*------------WRITE CALL BACK-------------------\n");
    (VOS_VOID)vos_printf("VCOM_AT WRITE CALL BACK counts  : %d \n VCOM_AT WRITE CALL BACK fail counts  : %d \n  VCOM_AT VCOM_AT fail addr : %d\nVCOM2 WRITE fail len :%d\n\n",
        pstTable[DMS_SDM_VCOM_WRT_CB_PCUI].ulCalledNum,
        pstTable[DMS_SDM_VCOM_WRT_CB_ERR_PCUI].ulCalledNum,
        pstTable[DMS_SDM_VCOM_WRT_CB_ERR_PCUI].ulRserved1,
        pstTable[DMS_SDM_VCOM_WRT_CB_ERR_PCUI].ulRserved2);
    (VOS_VOID)vos_printf("VCOM_CTRL WRITE CALL BACK counts  : %d \n VCOM_CTRL WRITE CALL BACK fail counts  : %d \n  VCOM_CTRL VCOM_AT fail addr : %d\nVCOM2 WRITE fail len :%d\n\n",
        pstTable[DMS_SDM_VCOM_WRT_CB_CTRL].ulCalledNum,
        pstTable[DMS_SDM_VCOM_WRT_CB_ERR_CTRL].ulCalledNum,
        pstTable[DMS_SDM_VCOM_WRT_CB_ERR_CTRL].ulRserved1,
        pstTable[DMS_SDM_VCOM_WRT_CB_ERR_CTRL].ulRserved2);
    (VOS_VOID)vos_printf("VCOM_PCUI2 WRITE CALL BACK counts  : %d \n VCOM_PCUI2 WRITE CALL BACK fail counts  : %d \n  VCOM_PCUI2 VCOM_AT fail addr : %d\nVCOM2 WRITE fail len :%d\n\n",
        pstTable[DMS_SDM_VCOM_WRT_CB_PCUI2].ulCalledNum,
        pstTable[DMS_SDM_VCOM_WRT_CB_ERR_PCUI2].ulCalledNum,
        pstTable[DMS_SDM_VCOM_WRT_CB_ERR_PCUI2].ulRserved1,
        pstTable[DMS_SDM_VCOM_WRT_CB_ERR_PCUI2].ulRserved2);

    (VOS_VOID)vos_printf("*------------EVT CALL BACK-------------------\n");
    (VOS_VOID)vos_printf("VCOM_PCUI EVT CALL BACK counts  : %d \n VCOM_PCUI EVT  type : %d\n\n",
        pstTable[DMS_SDM_VCOM_EVT_PCUI].ulCalledNum,
        pstTable[DMS_SDM_VCOM_EVT_PCUI].ulRserved1);
    (VOS_VOID)vos_printf("VCOM_CTRL EVT CALL BACK counts  : %d \n VCOM_CTRL EVT  type : %d\n\n",
        pstTable[DMS_SDM_VCOM_EVT_CTRL].ulCalledNum,
        pstTable[DMS_SDM_VCOM_EVT_CTRL].ulRserved1);
    (VOS_VOID)vos_printf("VCOM_PCUI2 EVT CALL BACK counts  : %d \n VCOM_PCUI2 EVT  type : %d\n\n",
        pstTable[DMS_SDM_VCOM_EVT_PCUI2].ulCalledNum,
        pstTable[DMS_SDM_VCOM_EVT_PCUI2].ulRserved1);

    (VOS_VOID)vos_printf("*------------NCM SEND TO DRV INFO------------------\n");
    (VOS_VOID)vos_printf("���͵���������   : %d\n ����buf: %x\n ���ͳ���:%d\n �������سɹ�����  : %d \n\n",
        pstTable[DMS_SDM_VCOM_WRT_NCM].ulCalledNum,
        pstTable[DMS_SDM_VCOM_WRT_NCM].ulRserved1,
        pstTable[DMS_SDM_VCOM_WRT_NCM].ulRserved2,
        pstTable[DMS_SDM_VCOM_WRT_SUSS_NCM].ulCalledNum);

    return;
}


VOS_VOID DMS_ShowNlkEntityInfo(VOS_VOID)
{
    (VOS_VOID)vos_printf("====================================\n");
    (VOS_VOID)vos_printf("DMS NLK CONTEXT                     \n");
    (VOS_VOID)vos_printf("====================================\n");
    (VOS_VOID)vos_printf("SOCKET:                         %p\n", g_stDmsNlkEntity.pstSock);
    (VOS_VOID)vos_printf("LTE CTRL Read CB:               %p\n", g_stDmsNlkEntity.astOmChanProp[DMS_OM_CHAN_LTE_CTRL].pDataFunc);
    (VOS_VOID)vos_printf("LTE CTRL MSG Type:              %d\n", g_stDmsNlkEntity.astOmChanProp[DMS_OM_CHAN_LTE_CTRL].enMsgType);
    (VOS_VOID)vos_printf("LTE CTRL PHY BEAR:              %d\n", g_stDmsNlkEntity.astOmChanProp[DMS_OM_CHAN_LTE_CTRL].enPhyBear);
    (VOS_VOID)vos_printf("LTE CTRL PHY PID:               %d\n", g_stDmsNlkEntity.astPhyBearProp[DMS_NLK_PHY_BEAR_LTE].lPid);
    (VOS_VOID)vos_printf("LTE DATA Read CB:               %p\n", g_stDmsNlkEntity.astOmChanProp[DMS_OM_CHAN_LTE_DATA].pDataFunc);
    (VOS_VOID)vos_printf("LTE DATA MSG Type:              %d\n", g_stDmsNlkEntity.astOmChanProp[DMS_OM_CHAN_LTE_DATA].enMsgType);
    (VOS_VOID)vos_printf("LTE DATA PHY BEAR:              %d\n", g_stDmsNlkEntity.astOmChanProp[DMS_OM_CHAN_LTE_DATA].enPhyBear);
    (VOS_VOID)vos_printf("LTE DATA PHY PID:               %d\n", g_stDmsNlkEntity.astPhyBearProp[DMS_NLK_PHY_BEAR_LTE].lPid);
    (VOS_VOID)vos_printf("GU Read CB:                     %p\n", g_stDmsNlkEntity.astOmChanProp[DMS_OM_CHAN_GU_DATA].pDataFunc);
    (VOS_VOID)vos_printf("GU DATA MSG Type:               %d\n", g_stDmsNlkEntity.astOmChanProp[DMS_OM_CHAN_GU_DATA].enMsgType);
    (VOS_VOID)vos_printf("GU DATA PHY BEAR:               %d\n", g_stDmsNlkEntity.astOmChanProp[DMS_OM_CHAN_GU_DATA].enPhyBear);
    (VOS_VOID)vos_printf("GU DATA PHY PID:                %d\n", g_stDmsNlkEntity.astPhyBearProp[DMS_NLK_PHY_BEAR_GU].lPid);

    return;
}


VOS_VOID DMS_ShowNlkUlStats(VOS_VOID)
{
    (VOS_VOID)vos_printf("====================================\n");
    (VOS_VOID)vos_printf("DMS NLK UL STATS                    \n");
    (VOS_VOID)vos_printf("====================================\n");
    (VOS_VOID)vos_printf("Create SOCK Fail NUM:           %u\n", g_stDmsMntnNlkStats.ulCreatSockFailNum);
    (VOS_VOID)vos_printf("Total Total MSG NUM:            %u\n", g_stDmsMntnNlkStats.ulUlTotalMsgNum);
    (VOS_VOID)vos_printf("ERR MSG NUM:                    %u\n", g_stDmsMntnNlkStats.ulUlErrMsgNum);
    (VOS_VOID)vos_printf("UnSupport Input Log Num:        %u\n", g_stDmsMntnNlkStats.ulUlUnSupportInputLogNum);
    (VOS_VOID)vos_printf("UnSupport Write Log Num:        %u\n", g_stDmsMntnNlkStats.ulUlUnSupportWriteLogNum);
    (VOS_VOID)vos_printf("UNKNOWN MSG NUM:                %u\n", g_stDmsMntnNlkStats.ulUlUnknownMsgNum);
    (VOS_VOID)vos_printf("Send LTE CFG MSG NUM:           %u\n", g_stDmsMntnNlkStats.aulUlSendMsgNum[DMS_NLK_MSG_TYPE_LTE_CFG]);
    (VOS_VOID)vos_printf("Send LTE CTRL MSG NUM:          %u\n", g_stDmsMntnNlkStats.aulUlSendMsgNum[DMS_NLK_MSG_TYPE_LTE_CTRL]);
    (VOS_VOID)vos_printf("Send LTE DATA MSG NUM:          %u\n", g_stDmsMntnNlkStats.aulUlSendMsgNum[DMS_NLK_MSG_TYPE_LTE_DATA]);
    (VOS_VOID)vos_printf("Send GU CFG MSG NUM:            %u\n", g_stDmsMntnNlkStats.aulUlSendMsgNum[DMS_NLK_MSG_TYPE_GU_CFG]);
    (VOS_VOID)vos_printf("Send GU DATA MSG NUM:           %u\n", g_stDmsMntnNlkStats.aulUlSendMsgNum[DMS_NLK_MSG_TYPE_GU_DATA]);
    (VOS_VOID)vos_printf("Free LTE CFG MSG NUM:           %u\n", g_stDmsMntnNlkStats.aulUlFreeMsgNum[DMS_NLK_MSG_TYPE_LTE_CFG]);
    (VOS_VOID)vos_printf("Free LTE CTRL MSG NUM:          %u\n", g_stDmsMntnNlkStats.aulUlFreeMsgNum[DMS_NLK_MSG_TYPE_LTE_CTRL]);
    (VOS_VOID)vos_printf("Free LTE DATA MSG NUM:          %u\n", g_stDmsMntnNlkStats.aulUlFreeMsgNum[DMS_NLK_MSG_TYPE_LTE_DATA]);
    (VOS_VOID)vos_printf("Free GU CFG MSG NUM:            %u\n", g_stDmsMntnNlkStats.aulUlFreeMsgNum[DMS_NLK_MSG_TYPE_GU_CFG]);
    (VOS_VOID)vos_printf("Free GU DATA MSG NUM:           %u\n", g_stDmsMntnNlkStats.aulUlFreeMsgNum[DMS_NLK_MSG_TYPE_GU_DATA]);

    return;
}


VOS_VOID DMS_ShowNlkDlStats(VOS_VOID)
{
    (VOS_VOID)vos_printf("====================================\n");
    (VOS_VOID)vos_printf("DMS NLK DL STATS                    \n");
    (VOS_VOID)vos_printf("====================================\n");
    (VOS_VOID)vos_printf("Total PKT NUM:                  %u\n", g_stDmsMntnNlkStats.ulDlTotalPktNum);
    (VOS_VOID)vos_printf("ERR PARA PKT NUM:               %u\n", g_stDmsMntnNlkStats.ulDlErrParaPktNum);
    (VOS_VOID)vos_printf("ERR CHAN PKT NUM:               %u\n", g_stDmsMntnNlkStats.ulDlErrChanPktNum);
    (VOS_VOID)vos_printf("LTE CTRL PKT NUM:               %u\n", g_stDmsMntnNlkStats.aulDlNormChanPktNum[DMS_OM_CHAN_LTE_CTRL]);
    (VOS_VOID)vos_printf("LTE DATA PKT NUM:               %u\n", g_stDmsMntnNlkStats.aulDlNormChanPktNum[DMS_OM_CHAN_LTE_DATA]);
    (VOS_VOID)vos_printf("GU DATA PKT NUM:                %u\n", g_stDmsMntnNlkStats.aulDlNormChanPktNum[DMS_OM_CHAN_GU_DATA]);
    (VOS_VOID)vos_printf("Total MSG NUM:                  %u\n", g_stDmsMntnNlkStats.ulDlTotalMsgNum);
    (VOS_VOID)vos_printf("ERR SOCK MSG NUM:               %u\n", g_stDmsMntnNlkStats.ulDlErrSockMsgNum);
    (VOS_VOID)vos_printf("ERR PID MSG NUM:                %u\n", g_stDmsMntnNlkStats.ulDlErrPidMsgNum);
    (VOS_VOID)vos_printf("Alloc MSG Fail NUM:             %u\n", g_stDmsMntnNlkStats.ulDlAllocMsgFailNum);
    (VOS_VOID)vos_printf("Put MSG Fail NUM:               %u\n", g_stDmsMntnNlkStats.ulDlPutMsgFailNum);
    (VOS_VOID)vos_printf("Unicast Fail NUM:               %u\n", g_stDmsMntnNlkStats.ulDlUnicastMsgFailNum);
    (VOS_VOID)vos_printf("Unicast Succ NUM:               %u\n", g_stDmsMntnNlkStats.ulDlUnicastMsgSuccNum);

    return;
}


VOS_VOID DMS_Help(VOS_VOID)
{
    (VOS_VOID)vos_printf("****************************************************\n");
    (VOS_VOID)vos_printf("DMS�����Ϣ                                         \n");
    (VOS_VOID)vos_printf("****************************************************\n");
    (VOS_VOID)vos_printf("DMS_SetLogLevel                                     \n");
    (VOS_VOID)vos_printf("DMS_ShowDebugInfo                                   \n");
    (VOS_VOID)vos_printf("DMS_ShowAtServerNvInfo                              \n");
    (VOS_VOID)vos_printf("DMS_ShowPortCfgInfo                                 \n");
    (VOS_VOID)vos_printf("DMS_ShowVcomStats                                   \n");
    (VOS_VOID)vos_printf("DMS_ShowNlkEntityInfo                               \n");
    (VOS_VOID)vos_printf("DMS_ShowNlkUlStats                                  \n");
    (VOS_VOID)vos_printf("DMS_ShowNlkDlStats                                  \n");

    return;
}



