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

#ifndef __ATCMDMISCPROC_H__
#define __ATCMDMISCPROC_H__

/*****************************************************************************
  1 ����ͷ�ļ�����
*****************************************************************************/
#include "AtCtx.h"
#include "AtParse.h"
#include "AtMtaInterface.h"
#include "at_lte_common.h"

#include "mdrv.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


#pragma pack(4)

/*****************************************************************************
  2 �궨��
*****************************************************************************/
#define AT_SECURE_STATE_NOT_SET                     (0)                         /* Secure State״̬δ���� */
#define AT_SECURE_STATE_SECURE                      (1)                         /* ��ǰSecure StateΪSecure */
#define AT_SECURE_STATE_RMA                         (2)                         /* ��ǰSecure StateΪRMA */

#define AT_EFUSE_OK                                 (0)                         /* efuse���سɹ� */
#define AT_EFUSE_REPEAT                             (1)                         /* efuse�����ظ����� */

#define AT_DRV_KCE_LEN                              (16)                        /* 128Bits��KCE�������� */
#define AT_KCE_PARA_LEN                             (32)                        /* KCE�ӽ���keyֵASKii��ʽ���� */

#define AT_DRV_SOCID_LEN                            (32)                        /* 256Bits��SOCID�������� */

#define AT_MODEM_YTD_LEN                            (10)                        /* �ַ��������յ��ܳ��� */
#define AT_MODEM_TIME_LEN                           (8)                         /* �ַ���Сʱ�����ӡ�����ܳ��� */
#define AT_MODEM_YEAR_LEN                           (4)                         /* �ַ�����ݵĳ��� */
#define AT_MODEM_MONTH_LEN                          (2)                         /* �ַ����·ݵĳ��� */
#define AT_MODEM_DATE_LEN                           (2)                         /* �ַ������ڵĳ��� */
#define AT_MODEM_HOUR_LEN                           (2)                         /* �ַ���Сʱ�ĳ��� */
#define AT_MODEM_MIN_LEN                            (2)                         /* �ַ������ӵĳ��� */
#define AT_MODEM_SEC_LEN                            (2)                         /* �ַ�����ĳ��� */
#define AT_MODEM_ZONE_LEN                           (3)                         /* �ַ���ʱ���ĳ��� */
#define AT_GET_MODEM_TIME_BUFF_LEN                  (5)

#define AT_MODEM_YEAR_MAX                           (2050)                      /* �����������ֵ */
#define AT_MODEM_YEAR_MIN                           (1970)                      /* ����������Сֵ */
#define AT_MODEM_MONTH_MAX                          (12)                        /* �����������ֵ */
#define AT_MODEM_MONTH_MIN                          (1)                         /* ����������Сֵ */
#define AT_MODEM_DAY_MAX                            (31)                        /* �����������ֵ */
#define AT_MODEM_DAY_MIN                            (1)                         /* ����������Сֵ */

#define AT_MODEM_HOUR_MAX                           (23)                        /* ������Сʱ���ֵ */
#define AT_MODEM_HOUR_MIN                           (0)                         /* ������Сʱ��Сֵ */
#define AT_MODEM_MIN_MAX                            (59)                        /* �����÷������ֵ */
#define AT_MODEM_MIN_MIN                            (0)                         /* �����÷�����Сֵ */
#define AT_MODEM_SEC_MAX                            (59)                        /* �������������ֵ */
#define AT_MODEM_SEC_MIN                            (0)                         /* ������������Сֵ */

#define AT_MODEM_ZONE_MAX                           (12)                        /* ������ʱ�����ֵ */
#define AT_MODEM_ZONE_MIN                           (-12)                       /* ������ʱ����Сֵ */


/*****************************************************************************
  3 ö�ٶ���
*****************************************************************************/


/*****************************************************************************
  4 ȫ�ֱ�������
*****************************************************************************/


/*****************************************************************************
  5 ��Ϣͷ����
*****************************************************************************/


/*****************************************************************************
  6 ��Ϣ����
*****************************************************************************/


/*****************************************************************************
  7 STRUCT����
*****************************************************************************/


/*****************************************************************************
  8 UNION����
*****************************************************************************/


/*****************************************************************************
  9 OTHERS����
*****************************************************************************/


/*****************************************************************************
  10 ��������
*****************************************************************************/

VOS_UINT32 AT_SetActiveModem(VOS_UINT8 ucIndex);

#if(FEATURE_ON == FEATURE_LTE)
#if(FEATURE_ON == FEATURE_LTE_MBMS)
VOS_UINT32 AT_SetMBMSServiceOptPara(VOS_UINT8 ucIndex);
VOS_UINT32 AT_SetMBMSServiceStatePara(VOS_UINT8 ucIndex);
VOS_UINT32 AT_SetMBMSPreferencePara(VOS_UINT8 ucIndex);
VOS_UINT32 AT_QryMBMSSib16NetworkTimePara(VOS_UINT8 ucIndex);
VOS_UINT32 AT_QryMBMSBssiSignalLevelPara(VOS_UINT8 ucIndex);
VOS_UINT32 AT_QryMBMSNetworkInfoPara(VOS_UINT8 ucIndex);
VOS_UINT32 AT_QryMBMSModemStatusPara(VOS_UINT8 ucIndex);
VOS_UINT32 AT_SetMBMSCMDPara(VOS_UINT8 ucIndex);
VOS_UINT32 AT_SetMBMSEVPara(VOS_UINT8 ucIndex);
VOS_UINT32 AT_SetMBMSInterestListPara(VOS_UINT8 ucIndex);
VOS_UINT32 AT_QryMBMSCmdPara(VOS_UINT8 ucIndex);
VOS_UINT32 AT_RcvMtaMBMSServiceOptSetCnf(
    VOS_VOID                           *pMsg
);
VOS_UINT32 AT_RcvMtaMBMSServiceStateSetCnf(
    VOS_VOID                           *pMsg
);
VOS_UINT32 AT_RcvMtaMBMSPreferenceSetCnf(
    VOS_VOID                           *pMsg
);
VOS_UINT32 AT_RcvMtaMBMSSib16NetworkTimeQryCnf(
    VOS_VOID                           *pMsg
);
VOS_UINT32 AT_RcvMtaMBMSBssiSignalLevelQryCnf(
    VOS_VOID                           *pMsg
);
VOS_UINT32 AT_RcvMtaMBMSNetworkInfoQryCnf(
    VOS_VOID                           *pMsg
);
VOS_UINT32 AT_RcvMtaMBMSModemStatusQryCnf(
    VOS_VOID                           *pMsg
);
VOS_UINT32 AT_RcvMtaMBMSEVSetCnf(
    VOS_VOID                           *pMsg
);
VOS_UINT32 AT_RcvMtaMBMSServiceEventInd(VOS_VOID *pstMsg);
VOS_UINT32 AT_RcvMtaMBMSInterestListSetCnf(
    VOS_VOID                           *pMsg
);
VOS_UINT32 AT_RcvMtaMBMSCmdQryCnf(
    VOS_VOID                           *pMsg
);
VOS_VOID AT_ReportMBMSCmdQryCnf(
    MTA_AT_MBMS_AVL_SERVICE_LIST_QRY_CNF_STRU      *pstLrrcCnf,
    VOS_UINT8                                       ucIndex
);
VOS_UINT32 At_TestMBMSCMDPara(VOS_UINT8 ucIndex);
#endif
VOS_UINT32 AT_RcvMtaLteLowPowerSetCnf(
    VOS_VOID                           *pMsg
);
VOS_UINT32 AT_SetLteLowPowerPara(VOS_UINT8 ucIndex);
VOS_UINT32 AT_CheckIsmCoexParaValue(VOS_INT32 ulVal, VOS_UINT32 ulParaNum);
VOS_UINT32 AT_SetIsmCoexPara(VOS_UINT8 ucIndex);
VOS_UINT32 AT_QryIsmCoexPara(VOS_UINT8 ucIndex);
VOS_UINT32 AT_RcvMtaIsmCoexSetCnf(
    VOS_VOID                           *pMsg
);
VOS_UINT32 AT_RcvL4AIsmCoexSetCnf(
    VOS_VOID                           *pMsg
);
VOS_UINT32 AT_RcvMtaIsmCoexQryCnf(
    VOS_VOID                           *pMsg
);
#endif

VOS_UINT32 AT_SetLogEnablePara(VOS_UINT8 ucIndex);
VOS_UINT32 AT_QryLogEnable(VOS_UINT8 ucIndex);

VOS_UINT32 AT_SetActPdpStubPara(VOS_UINT8 ucIndex);

extern VOS_UINT32 AT_SetNVCHKPara(VOS_UINT8 ucIndex);

#if ((FEATURE_ON == FEATURE_SC_DATA_STRUCT_EXTERN) || (FEATURE_ON == FEATURE_BOSTON_AFTER_FEATURE))
VOS_UINT32 AT_RcvDrvAgentSimlockWriteExSetCnf(VOS_VOID *pMsg);
VOS_UINT32 AT_RcvDrvAgentSimlockDataReadExReadCnf(VOS_VOID *pMsg);
VOS_UINT32 AT_SimLockDataReadExPara(VOS_UINT8 ucIndex);
VOS_UINT32 AT_SaveSimlockDataIntoCtx(
    AT_SIMLOCK_WRITE_EX_PARA_STRU *pstSimlockWriteExPara,
    VOS_UINT8                      ucIndex,
    VOS_UINT8                      ucNetWorkFlg);
VOS_UINT32 AT_SetSimlockDataWriteExPara(
    AT_SIMLOCK_WRITE_EX_PARA_STRU *pstSimlockWriteExPara,
    VOS_UINT8                      ucIndex,
    VOS_UINT8                      ucNetWorkFlg
);
VOS_UINT32  AT_ProcSimlockWriteExData(
    VOS_UINT8                          *pucSimLockData,
    VOS_UINT16                          usParaLen
);
#endif

VOS_UINT32 AT_RcvMtaAfcClkInfoCnf(
    VOS_VOID                           *pMsg
);

extern VOS_UINT32 AT_SetPdmCtrlPara(VOS_UINT8 ucIndex);

VOS_UINT32 AT_SetPhyComCfgPara(VOS_UINT8 ucIndex);
VOS_UINT32 AT_RcvMtaPhyComCfgSetCnf(VOS_VOID *pMsg);

#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
extern VOS_UINT32 AT_SetEvdoSysEvent(VOS_UINT8 ucIndex);
extern VOS_UINT32 AT_SetDoSigMask(VOS_UINT8 ucIndex);

extern VOS_UINT32 AT_RcvMtaEvdoSysEventSetCnf(
    VOS_VOID                           *pMsg
);

extern VOS_UINT32 AT_RcvMtaEvdoSigMaskSetCnf(
    VOS_VOID                           *pMsg
);

extern VOS_UINT32 AT_RcvMtaEvdoRevARLinkInfoInd(
    VOS_VOID                           *pMsg
);

extern VOS_UINT32 AT_RcvMtaEvdoSigExEventInd(
    VOS_VOID                           *pMsg
);
#endif

VOS_UINT32 AT_SetFratIgnitionPara(VOS_UINT8 ucIndex);

extern VOS_UINT32 AT_SetModemTimePara(VOS_UINT8 ucIndex);

extern VOS_UINT32 AT_SetRxTestModePara(VOS_UINT8 ucIndex);

extern VOS_UINT32 AT_RcvMtaSetRxTestModeCnf(
    VOS_VOID                           *pMsg
);

VOS_UINT32 AT_SetMipiWrParaEx(VOS_UINT8 ucIndex);
VOS_UINT32 AT_RcvMtaMipiWrEXCnf(VOS_VOID *pMsg);
VOS_UINT32 AT_SetMipiRdParaEx(VOS_UINT8 ucIndex);
VOS_UINT32 AT_RcvMtaMipiRdEXCnf(VOS_VOID *pMsg);

extern VOS_UINT32 AT_SetCrrconnPara(VOS_UINT8 ucIndex);

extern VOS_UINT32 AT_QryCrrconnPara(VOS_UINT8 ucIndex);

extern VOS_UINT32 AT_RcvMtaSetCrrconnCnf(
    VOS_VOID                        *pMsg
);

extern VOS_UINT32 AT_RcvMtaQryCrrconnCnf(
    VOS_VOID                        *pMsg
);

extern VOS_UINT32 AT_RcvMtaCrrconnStatusInd(
    VOS_VOID                           *pMsg
);

extern VOS_UINT32 AT_SetVtrlqualrptPara(VOS_UINT8 ucIndex);

extern VOS_UINT32 AT_RcvMtaSetVtrlqualrptCnf(
    VOS_VOID                        *pMsg
);

extern VOS_UINT32 AT_RcvMtaRlQualityInfoInd(
    VOS_VOID                           *pMsg
);

extern VOS_UINT32 AT_RcvMtaVideoDiagInfoRpt(
    VOS_VOID                           *pMsg
);


#if (VOS_OS_VER == VOS_WIN32)
#pragma pack()
#else
#pragma pack(0)
#endif




#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of AtCmdMiscProc.h */
