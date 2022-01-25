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
/*lint -save -e537 -e718 -e746 -e734 -e958 -e713 -e740*/
#include "osm.h"
#include "gen_msg.h"
#include "msp_at.h"
#include "at_lte_common.h"
#include "ATCmdProc.h"
//#include "AtNdisInterface.h"
//#include "AtDataProc.h"
//#include "AtDataProc.h"


#include "AtParse.h"/*Ϊ����MnClient.h
 */
#include "gen_common.h"
#include "at_common.h"
#include "AtCtx.h"
#include "OmApi.h"
#include "AtCmdMiscProc.h"


/*lint -e767 ԭ��:Log��ӡ*/
#define    THIS_FILE_ID        MSP_FILE_ID_AT_LTE_EVENTREPORT_C
/*lint +e767 */



/******************************************************************************
 */
/* ��������: atCsqInfoIndProc
 */
/* ��������: ����CSQ�����ϱ���Ϣ
 */
/*
 */
/* ����˵��: pMsgBlock��MSP MSG��Ϣ�ṹ
 */
/*
 */
/* �� �� ֵ:
 */
/*
 */
/* ����Ҫ��: TODO: ...
 */
/* ���þ���: TODO: ...
 */

/******************************************************************************
 */
/*����V1�е�#define MSP_HAVE_AT_RSSI_REPORT�޸ĵ�
 */
VOS_UINT32 atCsqInfoIndProc(VOS_VOID *pMsgBlock)
{
    L4A_CSQ_INFO_IND_STRU* pCsqInfo = NULL;
    VOS_UINT16 usLength = 0;
    VOS_INT16 ulRssi = 0;

    pCsqInfo = (L4A_CSQ_INFO_IND_STRU*)pMsgBlock;

    if(ERR_MSP_SUCCESS == pCsqInfo->ulErrorCode)
    {
        if(AT_RSSI_UNKNOWN == pCsqInfo->sRssi)
        {
            ulRssi = AT_RSSI_UNKNOWN;
        }
        else if(pCsqInfo->sRssi >= AT_RSSI_HIGH)
        {
            ulRssi = AT_CSQ_RSSI_HIGH;
        }
        else if(pCsqInfo->sRssi <= AT_RSSI_LOW)
        {
            ulRssi = AT_CSQ_RSSI_LOW;
        }
        else
        {
            ulRssi = (pCsqInfo->sRssi - AT_RSSI_LOW) / 2;
        }

        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,
                            (VOS_CHAR *)pgucLAtSndCodeAddr + usLength,"%s%s%d%s",
                            gaucAtCrLf, "^RSSI:", ulRssi, gaucAtCrLf);
        At_SendResultData(AT_BROADCAST_CLIENT_INDEX_MODEM_0,pgucLAtSndCodeAddr,usLength);

    }


    return AT_FW_CLIENT_STATUS_READY;
}

#define AT_ANTENNA_LEVEL_NUM        3
VOS_UINT8         g_ATAntennaLevel[AT_ANTENNA_LEVEL_NUM];
VOS_UINT8         g_ulAntennaLevel = 0;

VOS_UINT32       g_ATE5Order = 0;

VOS_UINT8 AT_CalculateLTEAntennaLevel(
    VOS_INT16                           usRsrp
)
{
    VOS_INT16 usLevel = 0;
       g_ATE5Order =4;
    /*���߸�ʽ��ʾ����
 */
    if ( usRsrp <= g_stRsrpCfg.ssValue[3])
    {
        usLevel = AT_CMD_ANTENNA_LEVEL_4;
    }
    else if (( g_stRsrpCfg.ssValue[3] < usRsrp) && (usRsrp <= g_stRsrpCfg.ssValue[2]))
    {
        usLevel = AT_CMD_ANTENNA_LEVEL_3;
    }
    else if (( g_stRsrpCfg.ssValue[2] < usRsrp) && (usRsrp <= g_stRsrpCfg.ssValue[1]))
    {
        usLevel = AT_CMD_ANTENNA_LEVEL_2;
    }
    else
    {
        usLevel = AT_CMD_ANTENNA_LEVEL_1;
    }
    return (VOS_UINT8)usLevel;

}

VOS_UINT8 AT_GetSmoothLTEAntennaLevel(
    AT_CMD_ANTENNA_LEVEL_ENUM_UINT8     enLevel
)
{
    VOS_UINT8                               i;
    g_ATE5Order =5;
    /* ����ʱ ��������  */
    if ( AT_CMD_ANTENNA_LEVEL_0 == enLevel )
    {
        g_ulAntennaLevel = enLevel;

        MSP_MEMSET( g_ATAntennaLevel, (VOS_SIZE_T)sizeof(g_ATAntennaLevel), 0x00, (VOS_SIZE_T)sizeof(g_ATAntennaLevel) );
        return g_ulAntennaLevel;
    }

    /* �������з���״̬  ��������  */
    if ( AT_CMD_ANTENNA_LEVEL_0 == g_ulAntennaLevel )
    {
        g_ulAntennaLevel = enLevel;

        MSP_MEMSET( g_ATAntennaLevel, (VOS_SIZE_T)sizeof(g_ATAntennaLevel), 0x00, (VOS_SIZE_T)sizeof(g_ATAntennaLevel) );
        g_ATAntennaLevel[AT_ANTENNA_LEVEL_NUM-1] = enLevel;
        return g_ulAntennaLevel;
    }

    /* ���ϴε��źŸ����Ƚ�, �仯�Ƚϴ�(����1��)���������� */
    if ( enLevel > (g_ulAntennaLevel + 1) )
    {
        g_ulAntennaLevel = enLevel;

        MSP_MEMSET( g_ATAntennaLevel, (VOS_SIZE_T)sizeof(g_ATAntennaLevel), 0x00, (VOS_SIZE_T)sizeof(g_ATAntennaLevel) );
        g_ATAntennaLevel[AT_ANTENNA_LEVEL_NUM-1] = enLevel;
        return g_ulAntennaLevel;
    }
    else if (( enLevel + 1) < g_ulAntennaLevel )
    {
        g_ulAntennaLevel = enLevel;

        MSP_MEMSET( g_ATAntennaLevel, (VOS_SIZE_T)sizeof(g_ATAntennaLevel), 0x00, (VOS_SIZE_T)sizeof(g_ATAntennaLevel) );
        g_ATAntennaLevel[AT_ANTENNA_LEVEL_NUM-1] = enLevel;
        return g_ulAntennaLevel;
    }
    else
    {
      /* Do nothing... */
    }

    /* �Ƚ��ȳ������3�εĲ�ѯ��� */
    for ( i=0; i<AT_ANTENNA_LEVEL_NUM-1; i++ )
    {
         g_ATAntennaLevel[i] = g_ATAntennaLevel[i+1];
    }
    g_ATAntennaLevel[i] = enLevel;


    /* ���������򲻸��£��Դﵽƽ����Ч�� */
    for ( i=0; i<AT_ANTENNA_LEVEL_NUM-1; i++ )
    {
        if (g_ATAntennaLevel[i]  != g_ATAntennaLevel[i+1] )
        {
            return g_ulAntennaLevel;
        }
    }

    /* �źŸ����ȶ��� AT_ANTENNA_LEVEL_MAX_NUM ��ʱ�������� */
    g_ulAntennaLevel = enLevel;
    return g_ulAntennaLevel;
}

VOS_UINT32 at_CsqInfoProc(VOS_VOID *pMsgBlock,AT_ANLEVEL_INFO_CNF_STRU* pAnlevelAnqueryInfo)
{
    VOS_UINT32 ulResult = ERR_MSP_SUCCESS;
    L4A_CSQ_INFO_IND_STRU* pCsqInfo = NULL;
    VOS_UINT16                          usRsrp;
    VOS_UINT8                           enCurAntennaLevel;
    g_ATE5Order =2;

    if((NULL == pMsgBlock)||(NULL == pAnlevelAnqueryInfo))
    {
        HAL_SDMLOG("pMsgBlock is NULL!\n");
        return AT_ERROR;
    }
    pCsqInfo = (L4A_CSQ_INFO_IND_STRU*)pMsgBlock;
    pAnlevelAnqueryInfo->usClientId= pCsqInfo->usClientId;
    pAnlevelAnqueryInfo->ulErrorCode = pCsqInfo->ulErrorCode;

    /*RSSI ��ʽ��ʾ����
 */
    if(AT_RSSI_UNKNOWN == pCsqInfo->sRssi)
    {
        pAnlevelAnqueryInfo->sRssi = AT_RSSI_UNKNOWN;
    }
    else if(pCsqInfo->sRssi >= AT_RSSI_HIGH)
    {
        pAnlevelAnqueryInfo->sRssi = AT_CSQ_RSSI_HIGH;
    }
    else if(pCsqInfo->sRssi <= AT_RSSI_LOW)
    {
        pAnlevelAnqueryInfo->sRssi = AT_CSQ_RSSI_LOW;
    }
    else
    {
        pAnlevelAnqueryInfo->sRssi = ( VOS_UINT16)((pCsqInfo->sRssi - AT_RSSI_LOW) / 2);
    }

    /* �ϱ�����ת��:�� Rscp��Ecio��ʾΪ�Ǹ�ֵ����Rscp��EcioΪ-145��-32������rssiΪ99��
       ��ת��Ϊ0 */
    if ( (AT_RSSI_UNKNOWN  == pCsqInfo->sRsrp)|| (AT_RSSI_UNKNOWN == pCsqInfo->sRssi) )
    {
        /* ��������0, ��ӦӦ�õ�Ȧ�� */
        enCurAntennaLevel       = AT_CMD_ANTENNA_LEVEL_0;
    }
    else
    {
         /* ȡ����ֵ */
        usRsrp                    = ( VOS_UINT16)(-(pCsqInfo->sRsrp));
        /* ���ú���AT_CalculateAntennaLevel������D25�㷨������źŸ��� */
        enCurAntennaLevel = AT_CalculateLTEAntennaLevel((VOS_INT16)usRsrp);
    }

    /* �źŴ��ʹ��� */
    pAnlevelAnqueryInfo->sLevel = AT_GetSmoothLTEAntennaLevel( enCurAntennaLevel );

    pAnlevelAnqueryInfo->sRsrp = pCsqInfo->sRsrp;
    pAnlevelAnqueryInfo->sRsrq = pCsqInfo->sRsrq;

    return ulResult;
}

VOS_UINT32 atAnlevelInfoIndProc(VOS_VOID *pMsgBlock)
{
  VOS_UINT16 usLength = 0;
  VOS_UINT32 ulResult =0;
  VOS_UINT16 usRsrp=0;
  VOS_UINT16 usRsrq=0;
  AT_ANLEVEL_INFO_CNF_STRU stAnlevelAnqueryInfo = {0};
  g_ATE5Order =3;

  ulResult = at_CsqInfoProc(pMsgBlock,&stAnlevelAnqueryInfo);

  if (ulResult == ERR_MSP_SUCCESS)
  {
    usRsrp = (VOS_UINT16)((stAnlevelAnqueryInfo.sRsrp == 99)?99:(-(stAnlevelAnqueryInfo.sRsrp)));
    usRsrq = (VOS_UINT16)((stAnlevelAnqueryInfo.sRsrq == 99)?99:(-(stAnlevelAnqueryInfo.sRsrq)));

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,
                          (VOS_CHAR *)pgucLAtSndCodeAddr + usLength,"%s%s0,99,%d,%d,%d,%d%s",
                          gaucAtCrLf, "^ANLEVEL:",
                          stAnlevelAnqueryInfo.sRssi,
                          stAnlevelAnqueryInfo.sLevel,
                          usRsrp,
                          usRsrq,gaucAtCrLf);
    At_SendResultData(AT_CLIENT_TAB_APP_INDEX,pgucLAtSndCodeAddr,usLength);
  }
  else
  {
    HAL_SDMLOG("ulResult = %d,\n",(VOS_INT)ulResult);
  }

  return AT_FW_CLIENT_STATUS_READY;
}

VOS_UINT32 atSetAnlevelCnfSameProc(VOS_VOID *pMsgBlock)
{
    VOS_UINT16 usLength = 0;
    VOS_UINT32 ulResult =0;
    VOS_UINT16 usRsrp=0;
    VOS_UINT16 usRsrq=0;

    AT_ANLEVEL_INFO_CNF_STRU stAnlevelAnqueryInfo = {0};
     g_ATE5Order =1;

    ulResult = at_CsqInfoProc(pMsgBlock,&stAnlevelAnqueryInfo);

    if(ERR_MSP_SUCCESS == ulResult)
    {
        usRsrp = (VOS_UINT16)((stAnlevelAnqueryInfo.sRsrp == 99)?99:(-(stAnlevelAnqueryInfo.sRsrp)));
        usRsrq = (VOS_UINT16)((stAnlevelAnqueryInfo.sRsrq == 99)?99:(-(stAnlevelAnqueryInfo.sRsrq)));

        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,
                          (VOS_CHAR *)pgucLAtSndCodeAddr + usLength,"%s%s0,99,%d,%d,%d,%d%s",
                          gaucAtCrLf, "^ANQUERY:",
                          stAnlevelAnqueryInfo.sRssi,
                          stAnlevelAnqueryInfo.sLevel,
                          usRsrp,
                          usRsrq,gaucAtCrLf);
    }
    else
    {
          HAL_SDMLOG(" atSetAnlevelCnfSameProc ulResult = %d,\n",(VOS_INT)ulResult);
    }

    CmdErrProc((VOS_UINT8)(stAnlevelAnqueryInfo.usClientId), stAnlevelAnqueryInfo.ulErrorCode,
                        usLength, pgucLAtSndCodeAddr);

    return AT_FW_CLIENT_STATUS_READY;
}


/******************************************************************************
 */
/* ��������: atSetCsqCnfProc
 */
/* ��������: ����SET CSQ����ظ���Ϣ
 */
/*
 */
/* ����˵��: pMsgBlock��MSP MSG��Ϣ�ṹ
 */
/*
 */
/* �� �� ֵ:
 */
/*
 */
/* ����Ҫ��: TODO: ...
 */
/* ���þ���: TODO: ...
 */

/******************************************************************************
 */

/*���������AT������еĺ���������ͬ���ǲ���Ҫ���¸�����
 */
VOS_UINT32 atSetCsqCnfSameProc(VOS_VOID *pMsgBlock)
{
    L4A_CSQ_INFO_CNF_STRU * pstSetCsqCnf = NULL;
    VOS_UINT8 ucRssiValue = 0;
    VOS_UINT8 ucChannalQual = 0;

    VOS_UINT16 usLength = 0;

    pstSetCsqCnf = (L4A_CSQ_INFO_CNF_STRU*)pMsgBlock;

    if(ERR_MSP_SUCCESS == pstSetCsqCnf->ulErrorCode)
    {
        if(AT_RSSI_UNKNOWN == pstSetCsqCnf->sRssi)
        {
            ucRssiValue = AT_RSSI_UNKNOWN;
        }
        else if(pstSetCsqCnf->sRssi >= AT_RSSI_HIGH)
        {
            ucRssiValue = AT_CSQ_RSSI_HIGH;
        }
        else if(pstSetCsqCnf->sRssi <= AT_RSSI_LOW)
        {
            ucRssiValue = AT_CSQ_RSSI_LOW;
        }
        else
        {
            ucRssiValue = (VOS_UINT8)((pstSetCsqCnf->sRssi - AT_RSSI_LOW) / 2);
        }

        ucChannalQual = AT_BER_UNKNOWN;
    }
    else
    {
        ucRssiValue   = AT_RSSI_UNKNOWN;
        ucChannalQual = AT_BER_UNKNOWN;
    }

    usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                                    (VOS_CHAR *)pgucLAtSndCodeAddr,
                                                    (VOS_CHAR *)pgucLAtSndCodeAddr + usLength,
                                                    "%s: %d,%d%s",
                                                    "+CSQ",
                                                    ucRssiValue,
                                                    ucChannalQual,
                                                    gaucAtCrLf);

    CmdErrProc((VOS_UINT8)(pstSetCsqCnf->usClientId), ERR_MSP_SUCCESS, usLength, pgucLAtSndCodeAddr);

    return AT_FW_CLIENT_STATUS_READY;
}

VOS_UINT32 atCerssiInfoIndProc(VOS_VOID *pMsgBlock)
{
    L4A_CSQ_INFO_IND_STRU * pCerssi = NULL;
    VOS_UINT16 usLength = 0;

    pCerssi = (L4A_CSQ_INFO_IND_STRU*)pMsgBlock;

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,
                            (VOS_CHAR *)pgucLAtSndCodeAddr + usLength,"%s%s0,0,255,%d,%d,%d,%d,%d,%d%s",
                            gaucAtCrLf, "^CERSSI:",pCerssi->sRsrp,pCerssi->sRsrq,pCerssi->lSINR,
                            pCerssi->stCQI.usRI,pCerssi->stCQI.ausCQI[0],pCerssi->stCQI.ausCQI[1],gaucAtCrLf);

    At_SendResultData(AT_BROADCAST_CLIENT_INDEX_MODEM_0,pgucLAtSndCodeAddr,usLength);

    return AT_FW_CLIENT_STATUS_READY;
}

VOS_UINT32 atCerssiInfoCnfProc(VOS_VOID *pMsgBlock)
{
    L4A_CSQ_INFO_CNF_STRU * pCerssi = NULL;
    VOS_UINT16 usLength = 0;
    AT_MODEM_NET_CTX_STRU              *pstNetCtx = VOS_NULL_PTR;
    VOS_UINT8                           ucIndex;

    pCerssi = (L4A_CSQ_INFO_CNF_STRU*)pMsgBlock;

    /* ͨ��clientid��ȡindex */
    /* coverity[example_checked] */
    if (AT_FAILURE == At_ClientIdToUserId(pCerssi->usClientId,&ucIndex))
    {
       printk("atCerssiInfoCnfProc:WARNING:AT INDEX NOT FOUND!");

       return VOS_ERR;
    }

    pstNetCtx = AT_GetModemNetCtxAddrFromClientId(ucIndex);

    if(0 == pCerssi->ulErrorCode)
    {
        usLength = (VOS_UINT32)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,
            (VOS_CHAR *)pgucLAtSndCodeAddr,
            "%s%s%d,%d,0,0,255,%d,%d,%d,%d,%d,%d%s",
            gaucAtCrLf, "^CERSSI:",  pstNetCtx->ucCerssiReportType,
            pstNetCtx->ucCerssiMinTimerInterval,  pCerssi->sRsrp, pCerssi->sRsrq, pCerssi->lSINR,
            pCerssi->stCQI.usRI, pCerssi->stCQI.ausCQI[0], pCerssi->stCQI.ausCQI[1], gaucAtCrLf);
     }

    CmdErrProc((VOS_UINT8)(pCerssi->usClientId), pCerssi->ulErrorCode, usLength, pgucLAtSndCodeAddr);

    return AT_FW_CLIENT_STATUS_READY;
}

/******************************************************************************
 */
/* ��������: atSysModeIndProc
 */
/* ��������: ����SYSMODE�����ϱ���Ϣ
 */
/*
 */
/* ����˵��: pMsgBlock��MSP MSG��Ϣ�ṹ
 */
/*
 */
/* �� �� ֵ:
 */
/*
 */
/* ����Ҫ��: TODO: ...
 */
/* ���þ���: TODO: ...
 */

/******************************************************************************
 */

VOS_UINT32 atSysModeIndProc(VOS_VOID *pMsgBlock)
{
    return AT_FW_CLIENT_STATUS_READY;
}



VOS_UINT32 atLwclashCnfProc(VOS_VOID *pMsgBlock)
{
    L4A_READ_LWCLASH_CNF_STRU *pstLwclash = NULL;
    VOS_UINT16 usLength = 0;

    pstLwclash = (L4A_READ_LWCLASH_CNF_STRU *)pMsgBlock;

    usLength = (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                (VOS_CHAR *)pgucLAtSndCodeAddr,
                (VOS_CHAR *)pgucLAtSndCodeAddr,
                "^LWCLASH: %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
                pstLwclash->stLwclashInfo.enState,
                pstLwclash->stLwclashInfo.usUlFreq, pstLwclash->stLwclashInfo.usUlBandwidth,
                pstLwclash->stLwclashInfo.usDlFreq, pstLwclash->stLwclashInfo.usDlBandwidth,
                pstLwclash->stLwclashInfo.usBand,
                
                pstLwclash->ulScellNum ,
                
                pstLwclash->stScellInfo[0].ulUlFreq, pstLwclash->stScellInfo[0].usUlBandwidth,
                pstLwclash->stScellInfo[0].ulDlFreq, pstLwclash->stScellInfo[0].usDlBandwidth,
                
                pstLwclash->stScellInfo[1].ulUlFreq, pstLwclash->stScellInfo[1].usUlBandwidth,
                pstLwclash->stScellInfo[1].ulDlFreq, pstLwclash->stScellInfo[1].usDlBandwidth,
                
                pstLwclash->stScellInfo[2].ulUlFreq, pstLwclash->stScellInfo[2].usUlBandwidth,
                pstLwclash->stScellInfo[2].ulDlFreq, pstLwclash->stScellInfo[2].usDlBandwidth,
                
                pstLwclash->stScellInfo[3].ulUlFreq, pstLwclash->stScellInfo[3].usUlBandwidth,
                pstLwclash->stScellInfo[3].ulDlFreq, pstLwclash->stScellInfo[3].usDlBandwidth
                );

    CmdErrProc((VOS_UINT8)(pstLwclash->usClientId), pstLwclash->ulErrorCode, usLength, pgucLAtSndCodeAddr);

    return AT_FW_CLIENT_STATUS_READY;
}


/*****************************************************************************
 �� �� ��  : atLacellCnfProc
 ��������  : ^LCACELL����Ӧ����
 �������  : VOS_VOID *pMsgBlock
 �������  : ��
 �� �� ֵ  : VOS_UINT32
 ���ú���  :
 ��������  :

 �޸���ʷ  :
*****************************************************************************/
VOS_UINT32 atLcacellCnfProc(VOS_VOID *pMsgBlock)
{
    L4A_READ_LCACELL_CNF_STRU *pstLcacell = NULL;
    VOS_UINT16 usLength = 0;
    VOS_UINT32 ulStatCnt = 0;
    
    pstLcacell = (L4A_READ_LCACELL_CNF_STRU *)pMsgBlock;
    
    usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                (VOS_CHAR *)pgucLAtSndCodeAddr,
                (VOS_CHAR *)pgucLAtSndCodeAddr+usLength,
                "^LCACELL: ");

    for(ulStatCnt=0;ulStatCnt<CA_MAX_CELL_NUM;ulStatCnt++)
    {
        usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                (VOS_CHAR *)pgucLAtSndCodeAddr,
                (VOS_CHAR *)pgucLAtSndCodeAddr+usLength,
                "\"%d %d %d %d\",",ulStatCnt, pstLcacell->stLcacellInfo[ulStatCnt].ucUlConfigured,
                pstLcacell->stLcacellInfo[ulStatCnt].ucDlConfigured,
                pstLcacell->stLcacellInfo[ulStatCnt].ucActived);
    }

    usLength--;
    
    CmdErrProc((VOS_UINT8)(pstLcacell->usClientId), pstLcacell->ulErrorCode, usLength, pgucLAtSndCodeAddr);

    return AT_FW_CLIENT_STATUS_READY;
}


VOS_UINT32 atQryCellIdCnfProc(VOS_VOID *pMsgBlock)
{
    L4A_READ_CELL_ID_CNF_STRU *pstCnf = NULL;
    VOS_UINT16 usLength = 0;

    pstCnf = (L4A_READ_CELL_ID_CNF_STRU *)pMsgBlock;

    usLength = (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                (VOS_CHAR *)pgucLAtSndCodeAddr,
                (VOS_CHAR *)pgucLAtSndCodeAddr,
                ((2 == pstCnf->usMncNum)?"%s^CECELLID: %03x%02x,%d,%d,%d%s":"%s^CECELLID: %03x%03x,%d,%d,%d%s"),
                gaucAtCrLf,
                pstCnf->usMCC,
                (2 == pstCnf->usMncNum)?(pstCnf->usMNC&0xff):pstCnf->usMNC,
                pstCnf->ulCi,
                pstCnf->usPci,
                pstCnf->usTAC,
                gaucAtCrLf);

    CmdErrProc((VOS_UINT8)(pstCnf->usClientId), ERR_MSP_SUCCESS, usLength, pgucLAtSndCodeAddr);

    return AT_FW_CLIENT_STATUS_READY;
}

VOS_UINT32 g_ulCellNum =0;
VOS_UINT32 atQryCellInfoCnfProc(VOS_VOID *pMsgBlock)
{
    L4A_READ_CELL_INFO_CNF_STRU *pstcnf = NULL;
    VOS_UINT16 usLength = 0;
    VOS_UINT32 i =0;

    pstcnf = (L4A_READ_CELL_INFO_CNF_STRU *)pMsgBlock;

    if(0 != pstcnf->ulErrorCode)
    {
        CmdErrProc((VOS_UINT8)(pstcnf->usClientId), pstcnf->ulErrorCode, 0, NULL);

        return AT_FW_CLIENT_STATUS_READY;
    }

    if(0 == pstcnf->stNcellListInfo.enCellFlag)
    {
        usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                (VOS_CHAR *)pgucLAtSndCodeAddr,
                (VOS_CHAR *)pgucLAtSndCodeAddr+usLength,
                "%d:%d,%d,%d%s",
                1,
                EN_SERVICE_CELL_ID,
                pstcnf->stNcellListInfo.stSevCellInfo.stCellMeasInfo[0].stMeasRslt.usPhyCellId,
                pstcnf->stNcellListInfo.stSevCellInfo.stCellMeasInfo[0].stMeasRslt.sRssi,
                gaucAtCrLf);
    }
    else
    {
        /*ͬƵ*/
        for( i =0; i< pstcnf->stNcellListInfo.stIntraFreqNcellList.ulNCellNumber ;i++)
        {
            g_ulCellNum ++;
            usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                (VOS_CHAR *)pgucLAtSndCodeAddr,
                (VOS_CHAR *)pgucLAtSndCodeAddr+usLength,
                "%d:%d,%d,%d%s",
                g_ulCellNum,
                EN_SYN_FREQ_CELL_ID,
                pstcnf->stNcellListInfo.stIntraFreqNcellList.stCellMeasInfo[i].stMeasRslt.usPhyCellId,
                pstcnf->stNcellListInfo.stIntraFreqNcellList.stCellMeasInfo[i].stMeasRslt.sRssi,
                gaucAtCrLf);
        }
         /*��Ƶ*/
        for( i =0; i< pstcnf->stNcellListInfo.stInterFreqNcellList.ulNCellNumber ;i++)
        {
            g_ulCellNum ++;
            usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                (VOS_CHAR *)pgucLAtSndCodeAddr,
                (VOS_CHAR *)pgucLAtSndCodeAddr+usLength,
                "%d:%d,%d,%d%s",
                g_ulCellNum,
                EN_ASYN_FREQ_CELL_ID,
                pstcnf->stNcellListInfo.stInterFreqNcellList.stCellMeasInfo[i].stMeasRslt.usPhyCellId,
                pstcnf->stNcellListInfo.stInterFreqNcellList.stCellMeasInfo[i].stMeasRslt.sRssi,
                gaucAtCrLf);
        }
        /*W��Ƶ*/
        for( i =0; i< pstcnf->stNcellListInfo.stInterRATUMTSNcellList.ulNCellNumber ;i++)
        {
            g_ulCellNum ++;
            usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                (VOS_CHAR *)pgucLAtSndCodeAddr,
                (VOS_CHAR *)pgucLAtSndCodeAddr+usLength,
                "%d:%d,%d,%d%s",
                g_ulCellNum,
                EN_ASYN_UMTS_CELL_ID,
                pstcnf->stNcellListInfo.stInterRATUMTSNcellList.stUMTSNcellList[i].usPrimaryScramCode,
                pstcnf->stNcellListInfo.stInterRATUMTSNcellList.stUMTSNcellList[i].sCpichRscp,
                gaucAtCrLf);
        }
        /*G��Ƶ*/
        for( i =0; i< pstcnf->stNcellListInfo.stInterRATGeranNcellList.ulNCellNumber ;i++)
        {
            g_ulCellNum ++;
            usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                (VOS_CHAR *)pgucLAtSndCodeAddr,
                (VOS_CHAR *)pgucLAtSndCodeAddr+usLength,
                "%d:%d,%d,%d%s",
                g_ulCellNum,
                EN_ASYN_GSM_CELL_ID,
                (pstcnf->stNcellListInfo.stInterRATGeranNcellList.stGeranNcellList[i].stBSIC.usNcc) * 16 *16 +
                pstcnf->stNcellListInfo.stInterRATGeranNcellList.stGeranNcellList[i].stBSIC.usBcc,
                pstcnf->stNcellListInfo.stInterRATGeranNcellList.stGeranNcellList[i].sRSSI,
                gaucAtCrLf);
        }

        if(g_ulCellNum == 0)
        {
            usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                (VOS_CHAR *)pgucLAtSndCodeAddr,
                (VOS_CHAR *)pgucLAtSndCodeAddr+usLength,
                "%d%s",
                g_ulCellNum,
                gaucAtCrLf);
        }
    }

    g_ulCellNum = 0;
    CmdErrProc((VOS_UINT8)(pstcnf->usClientId), pstcnf->ulErrorCode, usLength, pgucLAtSndCodeAddr);

    return AT_FW_CLIENT_STATUS_READY;
}


VOS_VOID atLwclashInd(VOS_VOID *pMsgBlock)
{
    L4A_READ_LWCLASH_IND_STRU *pstLwclash = NULL;

    VOS_UINT16 usLength = 0;

    pstLwclash = (L4A_READ_LWCLASH_IND_STRU *)pMsgBlock;

        usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                    (VOS_CHAR *)pgucLAtSndCodeAddr,
                    (VOS_CHAR *)pgucLAtSndCodeAddr,
                    "%s^LWURC: %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d%s",
                    gaucAtCrLf, pstLwclash->stLwclashInfo.enState,
                    pstLwclash->stLwclashInfo.usUlFreq, pstLwclash->stLwclashInfo.usUlBandwidth,
                    pstLwclash->stLwclashInfo.usDlFreq, pstLwclash->stLwclashInfo.usDlBandwidth,
                    pstLwclash->stLwclashInfo.usBand, 
                    pstLwclash->ulScellNum ,
                   
                    pstLwclash->stScellInfo[0].ulUlFreq, pstLwclash->stScellInfo[0].usUlBandwidth,
                    pstLwclash->stScellInfo[0].ulDlFreq, pstLwclash->stScellInfo[0].usDlBandwidth,
                   
                    pstLwclash->stScellInfo[1].ulUlFreq, pstLwclash->stScellInfo[1].usUlBandwidth,
                    pstLwclash->stScellInfo[1].ulDlFreq, pstLwclash->stScellInfo[1].usDlBandwidth,
                   
                    pstLwclash->stScellInfo[2].ulUlFreq, pstLwclash->stScellInfo[2].usUlBandwidth,
                    pstLwclash->stScellInfo[2].ulDlFreq, pstLwclash->stScellInfo[2].usDlBandwidth,
                   
                    pstLwclash->stScellInfo[3].ulUlFreq, pstLwclash->stScellInfo[3].usUlBandwidth,
                    pstLwclash->stScellInfo[3].ulDlFreq, pstLwclash->stScellInfo[3].usDlBandwidth,
                    gaucAtCrLf);

    At_SendResultData(AT_BROADCAST_CLIENT_INDEX_MODEM_0, pgucLAtSndCodeAddr, usLength);
}

/*****************************************************************************
 �� �� ��  : atLcacellInd
 ��������  : ^LCACELLURC���������ϱ�����
 �������  : VOS_VOID *pMsgBlock
 �������  : ��
 �� �� ֵ  : VOS_VOID
 ���ú���  :
 ��������  :

 �޸���ʷ  :

*****************************************************************************/
VOS_VOID atLcacellInd(VOS_VOID *pMsgBlock)
{
    L4A_READ_LCACELL_IND_STRU *pstLcacell = NULL;
    VOS_UINT16 usLength = 0;
    VOS_UINT32 ulStatCnt = 0;

    pstLcacell = (L4A_READ_LCACELL_IND_STRU *)pMsgBlock;

    /* ������ͷβ���\r\n */
    usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                (VOS_CHAR *)pgucLAtSndCodeAddr,
                (VOS_CHAR *)pgucLAtSndCodeAddr+usLength,
                "%s^LCACELLURC: ",
                gaucAtCrLf);


    for(ulStatCnt=0;ulStatCnt<CA_MAX_CELL_NUM;ulStatCnt++)
    {
        if (0 != ulStatCnt)
        {
            usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                    (VOS_CHAR *)pgucLAtSndCodeAddr,
                    (VOS_CHAR *)pgucLAtSndCodeAddr+usLength,
                    ",");
        }

        usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                (VOS_CHAR *)pgucLAtSndCodeAddr,
                (VOS_CHAR *)pgucLAtSndCodeAddr+usLength,
                "\"%d %d %d %d\"",ulStatCnt, pstLcacell->stLcacellInfo[ulStatCnt].ucUlConfigured,
                pstLcacell->stLcacellInfo[ulStatCnt].ucDlConfigured,
                pstLcacell->stLcacellInfo[ulStatCnt].ucActived);
    }

    usLength += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                (VOS_CHAR *)pgucLAtSndCodeAddr,
                (VOS_CHAR *)pgucLAtSndCodeAddr+usLength,
                "%s",
                gaucAtCrLf);

    At_SendResultData(AT_BROADCAST_CLIENT_INDEX_MODEM_0, pgucLAtSndCodeAddr, usLength);
}


typedef VOS_UINT32 (*AT_L4A_MSG_FUN)(VOS_VOID* pMsgBlock);

/*lint -e958 ԭ��:ֻ��A��ʹ�� */
typedef struct
{
    VOS_UINT32 ulMsgId;
    AT_L4A_MSG_FUN atL4aMsgProc;
}AT_L4A_MSG_FUN_TABLE_STRU;
/*lint +e958 */
static const AT_L4A_MSG_FUN_TABLE_STRU g_astAtL4aCnfMsgFunTable[] = {

    {ID_MSG_L4A_CSQ_INFO_CNF        ,   atSetCsqCnfSameProc     },
    {ID_MSG_L4A_ANQUERY_INFO_CNF    ,   atSetAnlevelCnfSameProc },
    {ID_MSG_L4A_CELL_ID_CNF         ,   atQryCellIdCnfProc      },
    {ID_MSG_L4A_LWCLASHQRY_CNF      ,   atLwclashCnfProc        },
    {ID_MSG_L4A_RADVER_SET_CNF      ,   atSetRadverCnfProc      },
    {ID_MSG_L4A_CELL_INFO_CNF       ,   atQryCellInfoCnfProc    },
    {ID_MSG_L4A_CERSSI_INQ_CNF      ,   atCerssiInfoCnfProc     },
    {ID_MSG_L4A_LCACELLQRY_CNF      ,   atLcacellCnfProc        },
};

static const AT_L4A_MSG_FUN_TABLE_STRU g_astAtL4aIndMsgFunTable[] = {
    {ID_MSG_L4A_RSSI_IND            ,   atCsqInfoIndProc        },
    {ID_MSG_L4A_ANLEVEL_IND         ,   atAnlevelInfoIndProc    },
    {ID_MSG_L4A_CERSSI_IND          ,   atCerssiInfoIndProc     },
    {ID_MSG_L4A_MODE_IND            ,   atSysModeIndProc        },
    {ID_MSG_L4A_LWCLASH_IND         ,   (AT_L4A_MSG_FUN)atLwclashInd},
    {ID_MSG_L4A_LCACELL_IND         ,   (AT_L4A_MSG_FUN)atLcacellInd},
};

/******************************************************************************
 */
/* ��������: atL4aGetMsgFun
 */
/* ��������: ��ȡ����L4A�ظ���Ϣ�ĺ���
 */
/*
 */
/* ����˵��: ulMsgId����ϢID
 */
/*
 */
/* �� �� ֵ:
 */
/*
 */
/* ����Ҫ��: TODO: ...
 */
/* ���þ���: TODO: ...
 */

/******************************************************************************
 */
AT_L4A_MSG_FUN_TABLE_STRU* atL4aGetCnfMsgFun(VOS_UINT32 ulMsgId)
{
    VOS_UINT32 i = 0;
    VOS_UINT32 ulTableLen = 0;
    const AT_L4A_MSG_FUN_TABLE_STRU* pTable;

    pTable = g_astAtL4aCnfMsgFunTable;
    ulTableLen = sizeof(g_astAtL4aCnfMsgFunTable);

    for(i = 0; i < (ulTableLen/sizeof(AT_L4A_MSG_FUN_TABLE_STRU)); i++)
    {
        if(ulMsgId == (pTable + i)->ulMsgId)
        {
            return (AT_L4A_MSG_FUN_TABLE_STRU*)(pTable + i);
        }
    }

    /*����Ҳ���
 */
    return NULL;
}

/******************************************************************************
 */
/* ��������: atL4aGetIndMsgFun
 */
/* ��������: ��ȡ����L4A�ظ���Ϣ�ĺ���
 */
/*
 */
/* ����˵��: ulMsgId����ϢID
 */
/*
 */
/* �� �� ֵ:
 */
/*
 */
/* ����Ҫ��: TODO: ...
 */
/* ���þ���: TODO: ...
 */

/******************************************************************************
 */
AT_L4A_MSG_FUN_TABLE_STRU* atL4aGetIndMsgFun(VOS_UINT32 ulMsgId)
{
    VOS_UINT32 i = 0;
    VOS_UINT32 ulTableLen = 0;
    const AT_L4A_MSG_FUN_TABLE_STRU* pTable;

    pTable = g_astAtL4aIndMsgFunTable;
    ulTableLen = sizeof(g_astAtL4aIndMsgFunTable);

    for(i = 0; i < (ulTableLen/sizeof(AT_L4A_MSG_FUN_TABLE_STRU)); i++)
    {
        if(ulMsgId == (pTable + i)->ulMsgId)
        {
            return (AT_L4A_MSG_FUN_TABLE_STRU*)(pTable + i);
        }
    }

    /*����Ҳ���
 */
    return NULL;
}




/******************************************************************************
 */
/* ��������: at_L4aCnfProc
 */
/* ��������: ����L4A��CNF��IND��Ϣ�������Ҷ�Ӧ�������д���
 */
/*
 */
/* ����˵��: pMsgBlockTmp��VOS��Ϣ
 */
/*
 */
/* �� �� ֵ:
 */
/*
 */
/* ����Ҫ��: TODO: ...
 */
/* ���þ���: TODO: ...
 */

/******************************************************************************
 */
VOS_UINT32 at_L4aCnfProc(MsgBlock* pMsgBlockTmp)
{
    VOS_UINT32 ulRet = ERR_MSP_SUCCESS;
    VOS_UINT32 ulMsgId = 0;
    VOS_UINT16 usIndex = 0;
    AT_L4A_MSG_FUN_TABLE_STRU* pTable = NULL;
    AT_L4A_MSG_FUN pMsgProc = NULL;
    L4A_AT_CNF_HEADER_STRU    *pstMsgHeader;

    pstMsgHeader    = (L4A_AT_CNF_HEADER_STRU *)pMsgBlockTmp;
    ulMsgId = pstMsgHeader->enMsgId;

    /* coverity[check_return] */
    At_ClientIdToUserId((VOS_UINT16)pstMsgHeader->usClientId, (TAF_UINT8 *)&usIndex);

    pstMsgHeader->usClientId = usIndex;
    if( (pTable = atL4aGetCnfMsgFun(ulMsgId)) && (pTable->atL4aMsgProc))
    {
        AT_STOP_TIMER_CMD_READY(usIndex);
        pMsgProc = pTable->atL4aMsgProc;
    }
    else if( (pTable = atL4aGetIndMsgFun(ulMsgId)) && (pTable->atL4aMsgProc))
    {
        pMsgProc = pTable->atL4aMsgProc;
    }
    else
    {
        ulRet = ERR_MSP_FAILURE;
    }
    
    if(NULL != pMsgProc)
    {
        ulRet = pMsgProc(pMsgBlockTmp);
    }

    return ulRet;
}


AT_FTM_CNF_MSG_PROC_STRU g_astLteAtFtmCnfMsgTbl[] =
{
    { ID_MSG_SYM_SET_TMODE_CNF,         atSetTmodeParaCnfProc },
    { ID_MSG_FTM_SET_FCHAN_CNF,         atSetFCHANParaCnfProc },
    { ID_MSG_FTM_RD_FCHAN_CNF,          atQryFCHANParaCnfProc },
    { ID_MSG_FTM_SET_TXON_CNF,          atSetFTXONParaCnfProc },
    { ID_MSG_FTM_RD_TXON_CNF,           atQryFTXONParaCnfProc },
    { ID_MSG_FTM_SET_RXON_CNF,          atSetFRXONParaCnfProc },
    { ID_MSG_FTM_RD_RXON_CNF,           atQryFRXONParaCnfProc },
    { ID_MSG_FTM_SET_FWAVE_CNF,         atSetFWAVEParaCnfProc },
    { ID_MSG_FTM_SET_TSELRF_CNF,        atSetTselrfParaCnfProc},/*TSELRF ��ʵ��δ���� */
    { ID_MSG_FTM_SET_AAGC_CNF,          atSetFLNAParaCnfProc },
    { ID_MSG_FTM_RD_AAGC_CNF,           atQryFLNAParaCnfProc },
    { ID_MSG_FTM_FRSSI_CNF,             atQryFRSSIParaCnfProc },
    /* */
    { ID_MSG_FTM_RD_DLOADVER_CNF,       atQryDLoadVerCnf },
    { ID_MSG_FTM_RD_DLOADINFO_CNF,      atQryDLoadInfoCnf },
    { ID_MSG_FTM_RD_AUTHORITYVER_CNF,   atQryAuthorityVerCnf },
    { ID_MSG_FTM_RD_AUTHORITYID_CNF,    atQryAuthorityIDCnf },
    { ID_MSG_FTM_SET_NVBACKUP_CNF,      atSetNVBackupCnf },
    { ID_MSG_FTM_SET_NVRESTORE_CNF,     atSetNVRestoreCnf },
    { ID_MSG_FTM_SET_NVRSTSTTS_CNF,     atSetNVRstSTTSCnf },
    {ID_MSG_FTM_SET_TBAT_CNF,           atSetTbatCnf},
    {ID_MSG_FTM_RD_TBAT_CNF,            atRdTbatCnf},
    {ID_MSG_FTM_SET_SDLOAD_CNF,     atSetSdloadCnf },
    { ID_MSG_FTM_SET_LTCOMMCMD_CNF,     atSetLTCommCmdParaCnfProc},
    { ID_MSG_FTM_RD_LTCOMMCMD_CNF,      atQryLTCommCmdParaCnfProc},
    { ID_MSG_FTM_RD_FPLLSTATUS_CNF,     atQryFPllStatusParaCnfProc },

};


/******************************************************************************
 */
/* ��������: atGetFtmCnfMsgProc
 */
/* ��������: ���ݻظ�����Ϣ�ҵ�ƥ��Ĵ�����
 */
/*
 */
/* ����˵��:
 */
/*   MsgId  [in] ��ϢID
 */
/* �� �� ֵ:
 */
/*    ��NULL g_astLteAtFtmCnfMsgTbl Ԫ�ص�ַ�������˴�����
 */
/*    NULL ƥ��ʧ��
 */

/******************************************************************************
 */
AT_FTM_CNF_MSG_PROC_STRU* atGetFtmCnfMsgProc(VOS_UINT32 ulMsgId)
{
    VOS_UINT32 i = 0;
    VOS_UINT32 ulTableLen = sizeof(g_astLteAtFtmCnfMsgTbl)/sizeof(g_astLteAtFtmCnfMsgTbl[0]);

    for (i=0; i<ulTableLen; i++)
    {
        if (g_astLteAtFtmCnfMsgTbl[i].ulMsgId == ulMsgId)
        {
            return &(g_astLteAtFtmCnfMsgTbl[i]);
        }
    }

    return NULL;
}

/******************************************************************************
 */
/* ��������: atGetFtmCnfMsgProc
 */
/* ��������: AT����LTEװ��FTM�ظ���Ϣ���
 */
/*
 */
/* ����˵��:
 */
/*   pMsg  [in] �˼���Ϣ�ṹ
 */
/* �� �� ֵ:
 */
/*    ERR_MSP_SUCCESS �ɹ�
 */
/*    ERR_MSP_FAILURE ʧ��
 */

/******************************************************************************
 */
VOS_UINT32 At_FtmEventMsgProc(VOS_VOID* pMsg)
{
    VOS_UINT32 ulRet = ERR_MSP_SUCCESS;
    OS_MSG_STRU *pOsMsg = NULL;
    MsgBlock *pstMsgBlock = NULL;
    AT_FTM_CNF_MSG_PROC_STRU* pMsgProcItem = NULL;
    AT_FW_DATA_MSG_STRU *pDataMsg = (AT_FW_DATA_MSG_STRU*)pMsg;
    VOS_VOID * pTmp;


    pstMsgBlock = VOS_MemAlloc(WUEPS_PID_AT, (DYNAMIC_MEM_PT), (sizeof(MsgBlock)+sizeof(OS_MSG_STRU)-2));
    if (NULL == pstMsgBlock)
    {
        return ERR_MSP_FAILURE;
    }

    /* ��Ϣ�ṹת�� */
    pOsMsg = (OS_MSG_STRU *)(pstMsgBlock->aucValue);
    pOsMsg->ulMsgId = pDataMsg->ulMsgId;
    pOsMsg->ulParam2 = pDataMsg->ulLen;
    pOsMsg->ulParam3 = pDataMsg->ulReceiverPid;

    pTmp = VOS_MemAlloc(WUEPS_PID_AT, (DYNAMIC_MEM_PT), pDataMsg->ulLen);
    if (NULL == pTmp)
    {
        VOS_MemFree(WUEPS_PID_AT, pstMsgBlock);
        return ERR_MSP_FAILURE;
    }

    pOsMsg->ulParam1 = pTmp;


    MSP_MEMCPY((VOS_UINT8*)(pOsMsg->ulParam1), pDataMsg->ulLen, pDataMsg->pContext, pDataMsg->ulLen);

    /* ��Ϣ���� */
    pMsgProcItem = atGetFtmCnfMsgProc(pDataMsg->ulMsgId);

    if(NULL == pMsgProcItem)
    {
        ulRet = ERR_MSP_FAILURE;
    }
    else
    {
        AT_STOP_TIMER_CMD_READY(pDataMsg->ulClientId);
        ulRet = pMsgProcItem->pfnCnfMsgProc((VOS_UINT8)(pDataMsg->ulClientId), (VOS_VOID *)pstMsgBlock);
    }

    VOS_MemFree(WUEPS_PID_AT, pTmp);
    VOS_MemFree(WUEPS_PID_AT, pstMsgBlock);
    return ulRet;
}



VOS_UINT32    AT_ProcTempprtEventInd(
    TEMP_PROTECT_EVENT_AT_IND_STRU     *pstMsg
)
{
    VOS_UINT16                          usLength = 0;
    TEMP_PROTECT_EVENT_AT_IND_STRU     *pTempPrt = VOS_NULL;

    pTempPrt = (TEMP_PROTECT_EVENT_AT_IND_STRU*)pstMsg;

    HAL_SDMLOG("AT_ProcTempprtEventInd: Event %d Param %d\n", (VOS_INT)pTempPrt->ulTempProtectEvent, (VOS_INT)pTempPrt->ulTempProtectParam);

    usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucLAtSndCodeAddr,
                            (VOS_CHAR *)pgucLAtSndCodeAddr,"%s%s%d,%d%s",
                            gaucAtCrLf, "^TEMPPRT:", pTempPrt->ulTempProtectEvent,
                            pTempPrt->ulTempProtectParam, gaucAtCrLf);

    At_SendResultData(AT_CLIENT_TAB_APP_INDEX,pgucLAtSndCodeAddr,usLength);

    return VOS_OK;
}
/*lint -restore*/




