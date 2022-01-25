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
#include "diag_mem.h"
#include "diag_debug.h"
#include "SCMProc.h"
#include "soc_socp_adapter.h"
#include "cpm.h"
#include "LNvCommon.h"
#include "LPsNvInterface.h"
#include "SCMSoftDecode.h"
#include "OmCommonPpm.h"


#define    THIS_FILE_ID        PS_FILE_ID_SCM_PROC_C

/* ****************************************************************************
  2 ȫ�ֱ�������
**************************************************************************** */
extern SCM_SOFTDECODE_INFO_STRU   g_stScmSoftDecodeInfo;
extern OM_ACPU_DEBUG_INFO         g_stAcpuDebugInfo;



/* ����Ŀ��ͨ���ص����� */
SCM_DECODERDESTFUCN         g_astSCMDecoderCbFunc[SOCP_DECODER_DST_CB_BUTT]={VOS_NULL_PTR, VOS_NULL_PTR, VOS_NULL_PTR};

SCM_CODER_SRC_CFG_STRU      g_astSCMCoderSrcCfg[SCM_CODER_SRC_NUM]=
{
    {SCM_CHANNEL_UNINIT, SOCP_CODER_SRC_LOM_CNF1, SOCP_CODER_DST_OM_CNF,   SOCP_DATA_TYPE_0, SOCP_ENCSRC_CHNMODE_CTSPACKET, SOCP_CHAN_PRIORITY_2, SCM_CODER_SRC_BDSIZE, SCM_CODER_SRC_RDSIZE, VOS_NULL_PTR, VOS_NULL_PTR, VOS_NULL_PTR, VOS_NULL_PTR},
    {SCM_CHANNEL_UNINIT, SOCP_CODER_SRC_LOM_IND1, SOCP_CODER_DST_OM_IND,   SOCP_DATA_TYPE_0, SOCP_ENCSRC_CHNMODE_CTSPACKET, SOCP_CHAN_PRIORITY_2, SCM_CODER_SRC_BDSIZE, SCM_CODER_SRC_RDSIZE, VOS_NULL_PTR, VOS_NULL_PTR, VOS_NULL_PTR, VOS_NULL_PTR}
};

SCM_CODER_DEST_CFG_STRU     g_astSCMCoderDstCfg[SCM_CODER_DST_NUM]=
{
    {SCM_CHANNEL_UNINIT, SOCP_CODER_DST_OM_CNF, SCM_CODER_DST_CNF_SIZE, SCM_CODER_DST_THRESHOLD,  VOS_NULL_PTR, VOS_NULL_PTR,  VOS_NULL_PTR},
    {SCM_CHANNEL_UNINIT, SOCP_CODER_DST_OM_IND, SCM_CODER_DST_IND_SIZE, SCM_CODER_DST_THRESHOLD,  VOS_NULL_PTR, VOS_NULL_PTR,  VOS_NULL_PTR},
};

SCM_DECODER_SRC_CFG_STRU    g_astSCMDecoderSrcCfg[SCM_DECODER_SRC_NUM]=
{
    {SCM_CHANNEL_UNINIT, SOCP_DECODER_SRC_LOM, SCM_DECODER_SRC_SIZE, VOS_NULL, VOS_NULL_PTR, VOS_NULL_PTR}
};

SCM_DECODER_DEST_CFG_STRU   g_astSCMDecoderDstCfg[SCM_DECODER_DST_NUM]=
{
    {SCM_CHANNEL_UNINIT, SOCP_DECODER_DST_LOM, SOCP_DECODER_SRC_LOM, SOCP_DATA_TYPE_0, SCM_DECODER_DST_SIZE, SCM_DECDOER_DST_THRESHOLD, VOS_NULL_PTR, VOS_NULL_PTR, VOS_NULL_PTR}
};



SCM_INFODATA_STRU           g_stSCMInfoData;    /* ���ڱ���log��Ϣ */


/* ������������������Դbuff���ٽ���Դ���� */
VOS_SPINLOCK             g_stScmGetCoderSrcBuffSpinLock;


SCM_CODER_SRC_DEBUG_STRU g_astScmCoderSrcDebugInfo[SCM_CODER_SRC_NUM] = {{0}};


/*****************************************************************************
  3 �ⲿ��������
*****************************************************************************/

/*****************************************************************************
  4 ����ʵ��
*****************************************************************************/


VOS_UINT32  SCM_FindChannelCfg(VOS_UINT32           ulChannelID,
                                    VOS_UINT32              ulChMax,
                                    SCM_CHANNEL_CFG_HEAD    *pstCfg,
                                    VOS_UINT32               ulStruLen,
                                    VOS_UINT32              *pulNum)
{
    VOS_UINT32                          i;
    SCM_CHANNEL_CFG_HEAD               *pstTmpCfg = pstCfg;

    for(i=0; i<ulChMax; i++)
    {
        if((pstTmpCfg->ulChannelID == ulChannelID)
            &&(SCM_CHANNEL_INIT_SUCC == pstTmpCfg->enInitState))
        {
            *pulNum = i;

            return VOS_OK;/* ���سɹ� */
        }

        pstTmpCfg = (SCM_CHANNEL_CFG_HEAD *)((VOS_UINT8 *)pstTmpCfg + ulStruLen);
    }

    return VOS_ERR;/* ����ʧ�� */
}


VOS_UINT32 SCM_CoderSrcChannelCfg(SCM_CODER_SRC_CFG_STRU *pstCfg)
{
    SOCP_CODER_SRC_CHAN_S               stChannle;          /* ��ǰͨ����������Ϣ */

    stChannle.u32DestChanID = pstCfg->enDstCHID;            /*  Ŀ��ͨ��ID */
    stChannle.eDataType     = pstCfg->enDataType;           /*  �������ͣ�ָ�����ݷ�װЭ�飬���ڸ��ö�ƽ̨ */
    stChannle.eMode         = pstCfg->enCHMode;             /*  ͨ������ģʽ */
    stChannle.ePriority     = pstCfg->enCHLevel;            /*  ͨ�����ȼ� */
    stChannle.u32BypassEn   = SOCP_HDLC_ENABLE;             /*  ͨ��bypassʹ�� */
    stChannle.eDataTypeEn   = SOCP_DATA_TYPE_EN;            /*  ��������ʹ��λ */
    stChannle.eDebugEn      = SOCP_ENC_DEBUG_DIS;           /*  ����λʹ�� */

    stChannle.sCoderSetSrcBuf.pucInputStart  = pstCfg->pucSrcPHY;                             /*  ����ͨ����ʼ��ַ */
    stChannle.sCoderSetSrcBuf.pucInputEnd    = (pstCfg->pucSrcPHY + pstCfg->ulSrcBufLen)-1;   /*  ����ͨ��������ַ */
    stChannle.sCoderSetSrcBuf.pucRDStart     = pstCfg->pucRDPHY;                              /* RD buffer��ʼ��ַ */
    stChannle.sCoderSetSrcBuf.pucRDEnd       = (pstCfg->pucRDPHY + pstCfg->ulRDBufLen)-1;     /*  RD buffer������ַ */
    stChannle.sCoderSetSrcBuf.u32RDThreshold = SCM_CODER_SRC_RD_THRESHOLD;                    /* RD buffer�����ϱ���ֵ */

    if (VOS_OK != mdrv_socp_corder_set_src_chan(pstCfg->enChannelID, &stChannle))
    {
        SCM_CODER_SRC_ERR("SCM_CoderSrcChannelCfg: Search Channel ID Error", pstCfg->enChannelID, 0);/* ��ӡʧ�� */

        return VOS_ERR;/* ���ش��� */
    }

    pstCfg->enInitState = SCM_CHANNEL_INIT_SUCC; /* ��¼ͨ����ʼ�����ô��� */

    return VOS_OK;/* ���سɹ� */
}


VOS_UINT32 SCM_CoderSrcChannelInit(VOS_VOID)
{
    VOS_UINT32                          i;

    VOS_SpinLockInit(&g_stScmGetCoderSrcBuffSpinLock);

    for (i = 0; i < SCM_CODER_SRC_NUM; i++)
    {
        /* TODO: CJQ */
        if(g_astSCMCoderSrcCfg[i].enChannelID == SOCP_CODER_SRC_LOM_IND3)
        {
            g_astSCMCoderSrcCfg[i].enInitState = SCM_CHANNEL_CFG_FAIL;
            continue;
        }

        if (VOS_OK != SCM_CoderSrcChannelCfg(&g_astSCMCoderSrcCfg[i]))
        {
            g_astSCMCoderSrcCfg[i].enInitState = SCM_CHANNEL_CFG_FAIL;  /* ��¼ͨ����ʼ�����ô��� */

            return VOS_ERR;/* ����ʧ�� */
        }

        if(VOS_OK != mdrv_socp_start(g_astSCMCoderSrcCfg[i].enChannelID))
        {
            g_astSCMCoderSrcCfg[i].enInitState = SCM_CHANNEL_START_FAIL;  /* ��¼ͨ���������ô��� */

            return VOS_ERR;/* ����ʧ�� */
        }

        g_astSCMCoderSrcCfg[i].enInitState = SCM_CHANNEL_INIT_SUCC;     /* ��¼ͨ����ʼ�����ô��� */
    }

    return VOS_OK;/* ���سɹ� */
}


VOS_UINT32 SCM_RlsSrcRDAll(SOCP_CODER_SRC_ENUM_U32 enChanlID, VOS_UINT_PTR *pDataPhyAddr, VOS_UINT32 *pulDataLen)
{
    SOCP_BUFFER_RW_STRU                 stSrcChanRD;
    SOCP_RD_DATA_STRU                  *pstRDData;
    VOS_UINT32                          ulRDNum;
    VOS_UINT32                          ulRDTotalNum= 0;
    VOS_UINT32                          ulTotalLen  = 0;
    VOS_UINT32                          ulFirstAddr = 0;
    VOS_UINT32                          ulCfgNum;
    VOS_UINT32                          i;
    SCM_CHANNEL_ENUM_U32                enLteMark   = SCM_CHANNEL_BUTT;/*��־ͨ������*/
    VOS_UINT32                          ulen = 0;

    if((SOCP_CODER_SRC_LOM_CNF1 == enChanlID)||(SOCP_CODER_SRC_LOM_CNF2 == enChanlID)
        ||(SOCP_CODER_SRC_LOM_IND1 == enChanlID)||(SOCP_CODER_SRC_LOM_IND2 == enChanlID)
        ||(SOCP_CODER_SRC_LOM_IND3 == enChanlID))
    {
          enLteMark = SCM_LTE_CHANNEL;
    }
    (VOS_VOID)VOS_MemSet_s(&stSrcChanRD, (VOS_UINT32)sizeof(stSrcChanRD), 0, sizeof(stSrcChanRD));

    if ( VOS_OK != SCM_FindChannelCfg(enChanlID,
                                        SCM_CODER_SRC_NUM,
                                        (SCM_CHANNEL_CFG_HEAD *)g_astSCMCoderSrcCfg,
                                        sizeof(SCM_CODER_SRC_CFG_STRU),
                                        &ulCfgNum))/* �ж�ͨ������ */
    {
        SCM_CODER_SRC_ERR("SCM_RlsSrcRDAll: Find Channel Error", enChanlID, 0);/* ��¼Log */
        return VOS_ERR;/* ����ʧ�� */
    }

    SCM_CODER_SRC_LOG("SCM_RlsSrcRDAll: Release Channel Data", enChanlID, 0);

    if (VOS_OK != mdrv_socp_get_rd_buffer(enChanlID, &stSrcChanRD))
    {
        SCM_CODER_SRC_ERR("SCM_RlsSrcRDAll: Get RD Info Error", enChanlID, 0);/* ��¼Log */
        return VOS_ERR;/* ����ʧ�� */
    }

    if (VOS_NULL_PTR == stSrcChanRD.pBuffer)        /* �������� */
    {
        SCM_CODER_SRC_ERR("SCM_RlsSrcRDAll: RD Info is Error", enChanlID, 0);/* ��¼Log */
        return VOS_ERR;/* ����ʧ�� */
    }

    if ((0 == stSrcChanRD.u32Size) && (0 == stSrcChanRD.u32RbSize)) /* ��������Ҫ�ͷ� */
    {
        *pDataPhyAddr   = 0;
        *pulDataLen     = 0;

        return VOS_OK;/* ���سɹ� */
    }

    /* RD������ȡ�Ƿ� */
    if(((stSrcChanRD.u32Size + stSrcChanRD.u32RbSize) / sizeof(SOCP_RD_DATA_STRU)) > SCM_CODE_SRC_RD_NUM)
    {
        SCM_CODER_SRC_ERR("SCM_RlsSrcRDFirst: Get RD Data Error", enChanlID, 0);/* ��¼Log */
        SCM_CODER_SRC_ERR("SCM_RlsSrcRDFirst: Get RD PTR Error", stSrcChanRD.u32Size, stSrcChanRD.u32RbSize);/* ��¼Log */

        return VOS_ERR;/* ����ʧ�� */
    }

    /* ����RD���� */
    ulRDNum = stSrcChanRD.u32Size / sizeof(SOCP_RD_DATA_STRU);

    if (0 != ulRDNum)
    {
        stSrcChanRD.pBuffer = (VOS_CHAR*)VOS_UncacheMemPhyToVirt((VOS_UINT8*)stSrcChanRD.pBuffer,
                                    g_astSCMCoderSrcCfg[ulCfgNum].pucRDPHY,
                                    g_astSCMCoderSrcCfg[ulCfgNum].pucRDBuf,
                                    g_astSCMCoderSrcCfg[ulCfgNum].ulRDBufLen);

        ulRDTotalNum = ulRDNum;

        /* ��ȡRD���ݵĵ�ַ�ͳ��� */
        pstRDData   = (SOCP_RD_DATA_STRU*)stSrcChanRD.pBuffer;

        /* ��¼��ǰ��һ���ͷŵ����ݵ�ַ,������Ҫ���ظ��ϲ� */
        ulFirstAddr = pstRDData->ulDataAddr;

        for (i = 0; i < ulRDNum; i++)
        {
            ulen    = 0;

            /* �ۼ�RD���ݳ��� */
            if(SCM_LTE_CHANNEL == enLteMark)
            {
                ulen = ALIGN_DDR_WITH_4BYTE(pstRDData->usMsgLen);
            }
            else
            {
                ulen = pstRDData->usMsgLen;
            }
            ulTotalLen += ulen;
            pstRDData++;
        }
    }

    /* ����ؾ�RD���� */
    ulRDNum = stSrcChanRD.u32RbSize / sizeof(SOCP_RD_DATA_STRU);

    if (0 != ulRDNum)
    {
        stSrcChanRD.pRbBuffer = (VOS_CHAR*)VOS_UncacheMemPhyToVirt((VOS_UINT8*)stSrcChanRD.pRbBuffer,
                                    g_astSCMCoderSrcCfg[ulCfgNum].pucRDPHY,
                                    g_astSCMCoderSrcCfg[ulCfgNum].pucRDBuf,
                                    g_astSCMCoderSrcCfg[ulCfgNum].ulRDBufLen);

        ulRDTotalNum += ulRDNum;

        /* ��ȡRD���ݻؾ�ĵ�ַ�ͳ��� */
        pstRDData   = (SOCP_RD_DATA_STRU*)stSrcChanRD.pRbBuffer;

        if (0 == ulFirstAddr)/* ��¼��ǰ��һ���ͷŵ����ݵ�ַ,������Ҫ���ظ��ϲ� */
        {
            ulFirstAddr = pstRDData->ulDataAddr;
        }

        for (i = 0; i < ulRDNum; i++)
        {
            ulen    = 0;

            /* �ۼ�RD���ݳ��� */
            if(SCM_LTE_CHANNEL == enLteMark)
            {
                ulen = ALIGN_DDR_WITH_4BYTE((pstRDData->usMsgLen));
            }
            else
            {
                ulen = pstRDData->usMsgLen;
            }
            ulTotalLen += ulen;

            pstRDData++;
        }
    }

    if (VOS_OK != mdrv_socp_read_rd_done(enChanlID, (stSrcChanRD.u32Size+stSrcChanRD.u32RbSize)))
    {
        SCM_CODER_SRC_ERR("SCM_RlsSrcRDAll: Write RD Done is Error", enChanlID, (stSrcChanRD.u32Size+stSrcChanRD.u32RbSize));/* ��¼Log */
        return VOS_ERR;/* ���ش��� */
    }

    /* ��ȡ��RD���ֵ��¼��ȫ�ֱ����� */
    if (ulRDTotalNum > g_stSCMInfoData.aulRDUsedMax[ulCfgNum])
    {
        g_stSCMInfoData.aulRDUsedMax[ulCfgNum] = ulRDTotalNum;
    }

    *pDataPhyAddr   = ulFirstAddr;

    *pulDataLen     = ulTotalLen;

    return VOS_OK;/* ���ؽ��OK */
}


VOS_UINT32 SCM_RlsSrcRDFirst(SOCP_CODER_SRC_ENUM_U32 enChanlID,
                                    VOS_UINT_PTR *pDataPhyAddr,
                                    VOS_UINT32   *pulDataLen)
{
    SOCP_BUFFER_RW_STRU                 stSrcChanRD;
    SOCP_RD_DATA_STRU                  *pstRDData;
    VOS_UINT32                          ulRDTotalNum = 0;
    VOS_UINT32                          ulCfgNum;
    VOS_UINT32                          ulResult;

    (VOS_VOID)VOS_MemSet_s(&stSrcChanRD, (VOS_UINT32)sizeof(stSrcChanRD), 0, sizeof(stSrcChanRD));

    if (VOS_OK != SCM_FindChannelCfg(enChanlID, SCM_CODER_SRC_NUM,
                                (SCM_CHANNEL_CFG_HEAD *)g_astSCMCoderSrcCfg,
                                sizeof(SCM_CODER_SRC_CFG_STRU),
                                &ulCfgNum))/* �ж�ͨ������ */
    {
        SCM_CODER_SRC_ERR("SCM_RlsSrcRDFirst: Find Channel Error", enChanlID, 0);/* ��¼Log */
        return VOS_ERR;/* ����ʧ�� */
    }

    SCM_CODER_SRC_LOG("SCM_RlsSrcRDFirst: Release Channel Data", enChanlID, 0);

    if (VOS_OK != mdrv_socp_get_rd_buffer(enChanlID, &stSrcChanRD))
    {
        SCM_CODER_SRC_ERR("SCM_RlsSrcRDFirst: Get RD Buffer Error", enChanlID, 0);/* ��¼Log */
        return VOS_ERR;/* ����ʧ�� */
    }

    if (VOS_NULL_PTR == stSrcChanRD.pBuffer)        /* �������� */
    {
        SCM_CODER_SRC_ERR("SCM_RlsSrcRDFirst: RD Buffer is  Error", enChanlID, 0);/* ��¼Log */
        return VOS_ERR;/* ����ʧ�� */
    }

    /* RD������ȡ�Ƿ� */
    if(((stSrcChanRD.u32Size + stSrcChanRD.u32RbSize) / sizeof(SOCP_RD_DATA_STRU)) > SCM_CODE_SRC_RD_NUM)
    {
        SCM_CODER_SRC_ERR("SCM_RlsSrcRDFirst: Get RD Data Error", enChanlID, 0);/* ��¼Log */
        SCM_CODER_SRC_ERR("SCM_RlsSrcRDFirst: Get RD PTR Error", stSrcChanRD.u32Size, stSrcChanRD.u32RbSize);/* ��¼Log */

        return VOS_ERR;/* ����ʧ�� */
    }

    if (0 != stSrcChanRD.u32Size)      /* RD������ */
    {
        stSrcChanRD.pBuffer = (VOS_CHAR*)VOS_UncacheMemPhyToVirt((VOS_UINT8*)stSrcChanRD.pBuffer,
                                    g_astSCMCoderSrcCfg[ulCfgNum].pucRDPHY,
                                    g_astSCMCoderSrcCfg[ulCfgNum].pucRDBuf,
                                    g_astSCMCoderSrcCfg[ulCfgNum].ulRDBufLen);

        /* ��ȡRD��һ�������׵�ַ�ͳ��� */
        pstRDData = (SOCP_RD_DATA_STRU *)stSrcChanRD.pBuffer;

        *pDataPhyAddr   = (VOS_UINT_PTR)(pstRDData->ulDataAddr);
        *pulDataLen     = (VOS_UINT32)pstRDData->usMsgLen;

        ulResult = (VOS_UINT32)mdrv_socp_read_rd_done(enChanlID, (VOS_UINT32)sizeof(SOCP_RD_DATA_STRU));  /* �ͷ�һ������ */
    }
    else if (0 != stSrcChanRD.u32RbSize)/* RD�ؾ������� */
    {
        stSrcChanRD.pRbBuffer = (VOS_CHAR*)VOS_UncacheMemPhyToVirt((VOS_UINT8*)stSrcChanRD.pRbBuffer,
                                    g_astSCMCoderSrcCfg[ulCfgNum].pucRDPHY,
                                    g_astSCMCoderSrcCfg[ulCfgNum].pucRDBuf,
                                    g_astSCMCoderSrcCfg[ulCfgNum].ulRDBufLen);

        /* ��ȡ�ؾ�RD��һ�������׵�ַ�ͳ��� */
        pstRDData = (SOCP_RD_DATA_STRU *)stSrcChanRD.pRbBuffer;

        *pDataPhyAddr   = (VOS_UINT_PTR)(pstRDData->ulDataAddr);
        *pulDataLen     = (VOS_UINT32)pstRDData->usMsgLen;

        ulResult = (VOS_UINT32)mdrv_socp_read_rd_done(enChanlID, (VOS_UINT32)sizeof(SOCP_RD_DATA_STRU));  /* �ͷ�һ������ */
    }
    else         /* ��������Ҫ�ͷ� */
    {
        *pDataPhyAddr   = 0;
        *pulDataLen     = 0;
        ulResult        = VOS_OK;
    }

    if (VOS_OK != ulResult)
    {
        SCM_CODER_SRC_ERR("SCM_RlsSrcRDFirst: Read RD Done is  Error", enChanlID, 0);/* ��¼Log */
        return VOS_ERR;/* ���ش��� */
    }

    /* ����RD�ܴ�С����¼ȫ�ֱ����� */
    ulRDTotalNum = (stSrcChanRD.u32Size + stSrcChanRD.u32RbSize) / sizeof(SOCP_RD_DATA_STRU);

    if (ulRDTotalNum > g_stSCMInfoData.aulRDUsedMax[ulCfgNum])
    {
        g_stSCMInfoData.aulRDUsedMax[ulCfgNum] = ulRDTotalNum;
    }

    return VOS_OK;/* ���سɹ� */
}



VOS_UINT32 SCM_GetBDFreeNum(SOCP_CODER_SRC_ENUM_U32 enChanlID, VOS_UINT32 *pulBDNum)
{
    SOCP_BUFFER_RW_STRU                 stRwBuf;
    VOS_UINT32                          ulBDNum;
    VOS_UINT32                          ulBDTotal;
    VOS_UINT32                          ulCfgNum;

    /* �ж�ָ�����ȷ */
    if (VOS_NULL_PTR == pulBDNum)
    {
        return VOS_ERR;
    }

    if (VOS_OK != SCM_FindChannelCfg(enChanlID, SCM_CODER_SRC_NUM,
                                (SCM_CHANNEL_CFG_HEAD *)g_astSCMCoderSrcCfg, sizeof(SCM_CODER_SRC_CFG_STRU), &ulCfgNum))/* �ж�ͨ������ */
    {
        SCM_CODER_SRC_ERR("SCM_GetBDFreeNum: Find Channel Error", enChanlID, 0);/* ��¼Log */
        return VOS_ERR;/* ����ʧ�� */
    }

    SCM_CODER_SRC_LOG("SCM_GetBDFreeNum: Get BD Number", enChanlID, 0);

    if (VOS_OK != mdrv_socp_get_write_buff(enChanlID, &stRwBuf))
    {
        SCM_CODER_SRC_ERR("SCM_GetBDFreeNum: Get Write Buffer Error", enChanlID, 0);/* ��¼Log */
        return VOS_ERR;/* ����ʧ�� */
    }

    /* ����BD��ֵ */
    ulBDNum = (stRwBuf.u32Size + stRwBuf.u32RbSize) / sizeof(SOCP_BD_DATA_STRU);

    /* BD������ȡ�Ƿ� */
    if(ulBDNum > SCM_CODE_SRC_BD_NUM)
    {
        SCM_CODER_SRC_ERR("SCM_GetBDFreeNum: Get BD Data Error", enChanlID, 0);/* ��¼Log */
        SCM_CODER_SRC_ERR("SCM_GetBDFreeNum: Get BD PTR Error", stRwBuf.u32Size, stRwBuf.u32RbSize);/* ��¼Log */

        return VOS_ERR;/* ����ʧ�� */
    }

    /* ����Ҫ����һ������BD����֤ͨ�����ᱻд�����쳣 */
    if (ulBDNum <= 1)
    {
        *pulBDNum = 0;
    }
    else
    {
        *pulBDNum = (ulBDNum - 1);
    }

    /* ����ͨ��ȫ��BD�ĸ��� */
    ulBDTotal = g_astSCMCoderSrcCfg[ulCfgNum].ulSrcBufLen / sizeof(SOCP_BD_DATA_STRU);

    /* ��ȫ�ֱ����бȽϼ�¼���ֵ */
    if ((ulBDTotal- ulBDNum) > g_stSCMInfoData.aulBDUsedMax[ulCfgNum])
    {
        g_stSCMInfoData.aulBDUsedMax[ulCfgNum] = (ulBDTotal- ulBDNum);
    }

    return VOS_OK;
}



VOS_VOID SCM_CoderSrcMemcpy(SOCP_CODER_SRC_ENUM_U32 enChanlID, SCM_CODER_SRC_MEMCPY_STRU *pInfo, SOCP_BUFFER_RW_STRU *pstSocpBuf)
{
    VOS_VOID    *pDst;
    VOS_UINT32  ulCfgNum;

    if (VOS_OK != SCM_FindChannelCfg(enChanlID, SCM_CODER_SRC_NUM,
                                     (SCM_CHANNEL_CFG_HEAD *)g_astSCMCoderSrcCfg,
                                     sizeof(SCM_CODER_SRC_CFG_STRU), &ulCfgNum))/* �ж�ͨ������ */
    {
        return ;
    }

    /* �������Ŀ����������̲����Խ��������ɵ��õĵط���֤ */
    if(pInfo->uloffset < pstSocpBuf->u32Size)
    {
        if((pInfo->uloffset + pInfo->ulLen) <= pstSocpBuf->u32Size)
        {
            (VOS_VOID)VOS_MemCpy_s(((VOS_UINT8*)pInfo->pHeader + pInfo->uloffset), pInfo->ulLen, pInfo->pSrc, pInfo->ulLen);          
            VOS_FlushCpuWriteBuf();
        }
        else
        {
            (VOS_VOID)VOS_MemCpy_s(((VOS_UINT8*)pInfo->pHeader + pInfo->uloffset), (pstSocpBuf->u32Size - pInfo->uloffset), pInfo->pSrc, (pstSocpBuf->u32Size - pInfo->uloffset));
            VOS_FlushCpuWriteBuf();
            
            pDst = g_astSCMCoderSrcCfg[ulCfgNum].pucSrcBuf;

            (VOS_VOID)VOS_MemCpy_s(pDst,
                (pInfo->uloffset + pInfo->ulLen - pstSocpBuf->u32Size),
                ((VOS_UINT8*)pInfo->pSrc + (pstSocpBuf->u32Size - pInfo->uloffset)),
                (pInfo->uloffset + pInfo->ulLen - pstSocpBuf->u32Size));           
            VOS_FlushCpuWriteBuf();
            
        }
    }
    else
    {
        pDst = g_astSCMCoderSrcCfg[ulCfgNum].pucSrcBuf;

        pDst = (VOS_UINT8*)pDst + (pInfo->uloffset - pstSocpBuf->u32Size);

        (VOS_VOID)VOS_MemCpy_s(pDst, pInfo->ulLen, pInfo->pSrc, pInfo->ulLen);
        VOS_FlushCpuWriteBuf();
        
    }
}



VOS_UINT32 SCM_GetCoderChnSrcBuff(SOCP_CODER_SRC_ENUM_U32 enChanlID,
                                            VOS_UINT32 ulDataLen,
                                            SCM_CODER_SRC_PACKET_HEADER_STRU** pstCoderHeader,
                                            SOCP_BUFFER_RW_STRU *pstSocpBuf)
{
    SOCP_BUFFER_RW_STRU                 stRwBuf;
    VOS_UINT32                          ulCfgNum;
    SCM_CODER_SRC_PACKET_HEADER_STRU    *pstBuff;
    VOS_UINT32                          *pstBuftmp;
    VOS_UINT32                          ulTrueLen;

    /* �ж����ݲ��ܴ���4K */
    if ((0 == ulDataLen) || (ulDataLen > SCM_CODER_SRC_MAX_LEN))
    {
        (VOS_VOID)vos_printf("%s : ulDataLen %d.\n", __FUNCTION__, ulDataLen);
        return VOS_ERR;
    }

    if (VOS_OK != SCM_FindChannelCfg(enChanlID, SCM_CODER_SRC_NUM,
                                     (SCM_CHANNEL_CFG_HEAD *)g_astSCMCoderSrcCfg,
                                     sizeof(SCM_CODER_SRC_CFG_STRU), &ulCfgNum))/* �ж�ͨ������ */
    {
        return VOS_ERR;/* ����ʧ�� */
    }

    if(SOCP_ENCSRC_CHNMODE_LIST == g_astSCMCoderSrcCfg[ulCfgNum].enCHMode)
    {
        return VOS_ERR;/* ����ʧ�� */
    }

    if(VOS_OK != mdrv_socp_get_write_buff(enChanlID, &stRwBuf))
    {
        g_astScmCoderSrcDebugInfo[ulCfgNum].ulGetWriteBufErr ++;
        return VOS_ERR;/* ����ʧ�� */
    }

    ulTrueLen = ALIGN_DDR_WITH_8BYTE(ulDataLen);
    if((stRwBuf.u32Size + stRwBuf.u32RbSize) >= (ulTrueLen + SCM_HISI_HEADER_LENGTH))
    {
        /*��Ҫ���������ַ���ϲ�*/
        pstBuff = (SCM_CODER_SRC_PACKET_HEADER_STRU*)VOS_UncacheMemPhyToVirt((VOS_UINT8*)stRwBuf.pBuffer,
                                    g_astSCMCoderSrcCfg[ulCfgNum].pucSrcPHY,
                                    g_astSCMCoderSrcCfg[ulCfgNum].pucSrcBuf,
                                    g_astSCMCoderSrcCfg[ulCfgNum].ulSrcBufLen);

        if(stRwBuf.u32Size >= SCM_HISI_HEADER_LENGTH)
        {
            pstBuff->ulHisiMagic = SCM_HISI_HEADER_MAGIC;
            pstBuff->ulDataLen   = ulDataLen;
        }
        else if(stRwBuf.u32Size >= 4)
        {
            pstBuff->ulHisiMagic = SCM_HISI_HEADER_MAGIC;

            pstBuftmp = (VOS_UINT32*)g_astSCMCoderSrcCfg[ulCfgNum].pucSrcBuf;
            *pstBuftmp      = ulDataLen;
        }
        else    /* TODO: Ӧ��û��stRwBuf.u32SizeΪ0�ĳ��� */
        {
            pstBuftmp = (VOS_UINT32*)g_astSCMCoderSrcCfg[ulCfgNum].pucSrcBuf;

            *(pstBuftmp++)  = SCM_HISI_HEADER_MAGIC;
            *pstBuftmp      = ulDataLen;
        }

        *pstCoderHeader     = pstBuff;

        (VOS_VOID)VOS_MemCpy_s(pstSocpBuf, (VOS_UINT32)sizeof(stRwBuf), &stRwBuf, sizeof(stRwBuf));

        return VOS_OK;
    }
    else
    {
        g_astScmCoderSrcDebugInfo[ulCfgNum].ulGetCoherentBuffErr++;
        return VOS_ERR;
    }

}


VOS_UINT32 SCM_SendCoderSrc(SOCP_CODER_SRC_ENUM_U32 enChanlID, VOS_UINT8 *pucSendDataAddr, VOS_UINT32 ulSendLen)
{
    SOCP_BUFFER_RW_STRU                 stRwBuf;
    VOS_UINT32                          ulBDNum;
    VOS_UINT32                          ulCfgNum;
    SOCP_BD_DATA_STRU                   stBDData;
    SCM_CODER_SRC_PACKET_HEADER_STRU*   pstCoderHeader;

    /* �ж�����ָ��ͳ��ȵ���ȷ�����Ȳ��ܴ���4K */
    if ((VOS_NULL_PTR == pucSendDataAddr)
        ||(0 == ulSendLen)
        /*||(SCM_CODER_SRC_MAX_LEN < ulSendLen)*/)
    {
        return VOS_ERR;
    }

    if (VOS_OK != SCM_FindChannelCfg(enChanlID, SCM_CODER_SRC_NUM,
                                     (SCM_CHANNEL_CFG_HEAD *)g_astSCMCoderSrcCfg,
                                     sizeof(SCM_CODER_SRC_CFG_STRU), &ulCfgNum))/* �ж�ͨ������ */
    {
        return VOS_ERR;/* ����ʧ�� */
    }

    if (VOS_OK != mdrv_socp_get_write_buff(g_astSCMCoderSrcCfg[ulCfgNum].enChannelID, &stRwBuf))
    {
        g_astScmCoderSrcDebugInfo[ulCfgNum].ulGetWriteBufErr ++;
        return VOS_ERR;/* ����ʧ�� */
    }


    if(SOCP_ENCSRC_CHNMODE_LIST == g_astSCMCoderSrcCfg[ulCfgNum].enCHMode)
    {
        /* �������BD��ֵ */
        ulBDNum = (stRwBuf.u32Size + stRwBuf.u32RbSize) / sizeof(SOCP_BD_DATA_STRU);

        /* �ж��Ƿ��пռ� */
        if (1 >= ulBDNum)
        {
            return VOS_ERR;
        }

        stRwBuf.pBuffer = (VOS_CHAR*)VOS_UncacheMemPhyToVirt((VOS_UINT8*)stRwBuf.pBuffer,
                                        g_astSCMCoderSrcCfg[ulCfgNum].pucSrcPHY,
                                        g_astSCMCoderSrcCfg[ulCfgNum].pucSrcBuf,
                                        g_astSCMCoderSrcCfg[ulCfgNum].ulSrcBufLen);

        stBDData.ulDataAddr = (VOS_UINT32)((VOS_UINT_PTR)pucSendDataAddr);
        stBDData.usMsgLen   = (VOS_UINT16)ulSendLen;
        stBDData.enDataType = SOCP_BD_DATA;

        (VOS_VOID)VOS_MemCpy_s(stRwBuf.pBuffer, (VOS_UINT32)sizeof(stBDData), &stBDData, sizeof(stBDData));    /* �������ݵ�ָ���ĵ�ַ */
        VOS_FlushCpuWriteBuf();

        if (VOS_OK != mdrv_socp_write_done(enChanlID, sizeof(stBDData)))   /* ��ǰ����д����� */
        {
            SCM_CODER_SRC_ERR("SCM_SendCoderSrc: Write Buffer is Error", enChanlID, 0);/* ��¼Log */
            return VOS_ERR;/* ����ʧ�� */
        }
    }
    else if(SOCP_ENCSRC_CHNMODE_CTSPACKET == g_astSCMCoderSrcCfg[ulCfgNum].enCHMode)
    {
        if(stRwBuf.u32Size < SCM_HISI_HEADER_LENGTH)
        {
            g_astScmCoderSrcDebugInfo[ulCfgNum].ulSendFirstNotEnough ++;
            return VOS_ERR;
        }

        stRwBuf.pBuffer = (VOS_CHAR*)VOS_UncacheMemPhyToVirt((VOS_UINT8*)stRwBuf.pBuffer,
                                        g_astSCMCoderSrcCfg[ulCfgNum].pucSrcPHY,
                                        g_astSCMCoderSrcCfg[ulCfgNum].pucSrcBuf,
                                        g_astSCMCoderSrcCfg[ulCfgNum].ulSrcBufLen);
        if(stRwBuf.pBuffer != (VOS_CHAR*)pucSendDataAddr)
        {
            g_astScmCoderSrcDebugInfo[ulCfgNum].ulSendAddrErr++;
            return VOS_ERR;
        }

        pstCoderHeader = (SCM_CODER_SRC_PACKET_HEADER_STRU*)pucSendDataAddr;
        if((pstCoderHeader->ulDataLen != ulSendLen)||(pstCoderHeader->ulHisiMagic != SCM_HISI_HEADER_MAGIC))
        {
            g_astScmCoderSrcDebugInfo[ulCfgNum].ulSendHeaderErr++;
            return VOS_ERR;
        }
        VOS_FlushCpuWriteBuf();
        /*��һ�������ռ䲻��HISI��ͷ����*/
        ulSendLen = ALIGN_DDR_WITH_8BYTE(ulSendLen);
        if(VOS_OK != mdrv_socp_write_done(enChanlID, (ulSendLen + SCM_HISI_HEADER_LENGTH)))
        {
            g_astScmCoderSrcDebugInfo[ulCfgNum].ulSendWriteDoneErr ++;
            return VOS_ERR;
        }
        g_astScmCoderSrcDebugInfo[ulCfgNum].ulSendDataLen += ulSendLen;
        g_astScmCoderSrcDebugInfo[ulCfgNum].ulSendPacketNum ++;
    }
    else
    {
        return VOS_ERR;
    }
    return VOS_OK;
}


VOS_UINT32 SCM_CoderDstChanMemAlloc(VOS_VOID)
{
    VOS_UINT32                          i = 0;
    VOS_UINT_PTR                        ulPHYAddr;

    for (i = 0; i < SCM_CODER_DST_NUM; i++)
    {
        /* �������Ŀ�Ŀռ� */
        g_astSCMCoderDstCfg[i].pucBuf = (VOS_UINT8*)VOS_UnCacheMemAlloc(g_astSCMCoderDstCfg[i].ulBufLen, &ulPHYAddr);

        /* ����ռ���� */
        if (VOS_NULL_PTR == g_astSCMCoderDstCfg[i].pucBuf)
        {
            /* ��¼ͨ����ʼ�����Ϊ�ڴ������쳣 */
            g_astSCMCoderDstCfg[i].enInitState = SCM_CHANNEL_MEM_FAIL;

            return VOS_ERR;/* ���ش��� */
        }

        g_astSCMCoderDstCfg[i].pucBufPHY = (VOS_UINT8*)ulPHYAddr;
    }

    return VOS_OK;
}


VOS_UINT32 SCM_CoderDstChanMemInit(VOS_VOID)
{
    VOS_UINT_PTR                        ulPHYAddr;
    VOS_UINT32                          ulBufLen;
    SOCP_ENC_DST_BUF_LOG_CFG_STRU       stLogCfg;

    VOS_MemSet_s(&stLogCfg,(VOS_UINT32)sizeof(stLogCfg),0,sizeof(stLogCfg));
    if(VOS_OK != mdrv_socp_get_sd_logcfg(&stLogCfg))
    {
        vos_printf("!!!! No code dest channel config from SOCP.\n");

        return SCM_CoderDstChanMemAlloc();
    }

    /* δ��log buffer�µĴ��� */
    if ((SOCP_DST_CHAN_NOT_CFG == stLogCfg.logOnFlag)
        || (VOS_NULL == stLogCfg.ulPhyBufferAddr))
    {
        vos_printf("!!!! No code dest channel config from SOCP, flag %d, addr 0x%x.\n",
            stLogCfg.logOnFlag, stLogCfg.ulPhyBufferAddr);

        return SCM_CoderDstChanMemAlloc();
    }

    /* INDͨ����Ҫ���ӳ�д�룬BUFFER��С50M(Ĭ��ֵ)��ˮ������Ϊˮ������Ϊ75%���ڴ��ڳ�ʼ���Ѿ������ */
    g_astSCMCoderDstCfg[SCM_CODER_DST_IND_CHANNEL].pucBufPHY    = (VOS_UINT8*)(stLogCfg.ulPhyBufferAddr);
    g_astSCMCoderDstCfg[SCM_CODER_DST_IND_CHANNEL].ulBufLen     = stLogCfg.BufferSize;
    g_astSCMCoderDstCfg[SCM_CODER_DST_IND_CHANNEL].pucBuf       = stLogCfg.pVirBuffer;
    /* ��Ϊ�ɰ汾��SOCP�ĵ�λ��KB�µİ汾��B,Ϊ�˼���SOCP�����г���1024,����������Ҫ����1024 */
    g_astSCMCoderDstCfg[SCM_CODER_DST_IND_CHANNEL].ulThreshold  = 0x4;

    /* CNFͨ������Ҫ���ӳ�д�� */
    ulBufLen = g_astSCMCoderDstCfg[SCM_CODER_DST_CNF_CHANNEL].ulBufLen;

    g_astSCMCoderDstCfg[SCM_CODER_DST_CNF_CHANNEL].pucBuf = (VOS_UINT8*)VOS_UnCacheMemAlloc(ulBufLen, &ulPHYAddr);

    /* ����ռ���� */
    if (VOS_NULL_PTR == g_astSCMCoderDstCfg[SCM_CODER_DST_CNF_CHANNEL].pucBuf)
    {
        /* ��¼ͨ����ʼ�����Ϊ�ڴ������쳣 */
        g_astSCMCoderDstCfg[SCM_CODER_DST_CNF_CHANNEL].enInitState = SCM_CHANNEL_MEM_FAIL;

        return VOS_ERR;
    }

    g_astSCMCoderDstCfg[SCM_CODER_DST_CNF_CHANNEL].pucBufPHY = (VOS_UINT8*)ulPHYAddr;

    return VOS_OK;
}


VOS_UINT32 SCM_RlsDestBuf(VOS_UINT32 ulChanlID, VOS_UINT32 ulReadSize)
{
    VOS_UINT32                          ulDataLen;
    SOCP_BUFFER_RW_STRU                 stBuffer;

    if(0 == ulReadSize) /*�ͷ�ͨ����������*/
    {
        if (VOS_OK != BSP_SOCP_GetReadBuff(ulChanlID, &stBuffer))
        {
            SCM_CODER_DST_ERR("SCM_RlsDestBuf: Get Read Buffer is Error", ulChanlID, 0);/* ��¼Log */
            return VOS_ERR;
        }

        ulDataLen = stBuffer.u32Size + stBuffer.u32RbSize;

        if (SOCP_CODER_DST_OM_IND == ulChanlID)
        {
            diag_ReportLost(EN_DIAG_LOST_CPMCB);
        }

        SCM_CODER_DST_LOG("SCM_RlsDestBuf: Relese All Data", ulChanlID, ulDataLen);
    }
    else
    {
        /* ��¼����ʱ�� */
        SCM_CODER_DST_LOG("SCM_RlsDestBuf: Relese Read Data", ulChanlID, ulReadSize);

        ulDataLen = ulReadSize;
    }

    if (VOS_OK != BSP_SOCP_ReadDataDone(ulChanlID, ulDataLen))
    {
        SCM_CODER_DST_ERR("SCM_RlsDestBuf: Read Data Done is Error", ulChanlID, ulDataLen);/* ��¼Log */

        return VOS_ERR;
    }
    OM_ACPU_DEBUG_CHANNEL_TRACE(ulChanlID, NULL, ulDataLen, OM_ACPU_READ_DONE, OM_ACPU_DATA);
    return VOS_OK;
}


VOS_VOID SCM_CoderDestReadCB(VOS_UINT32 ulDstChID)
{
    VOS_UINT32                          ulChType;
    VOS_UINT32                          ulCfgNum;
    SOCP_BUFFER_RW_STRU                 stBuffer;
    VOS_UINT32                          ulTimerIn;
    VOS_UINT32                          ulTimerOut;
    VOS_UINT_PTR                        ulVirtAddr;

    if(SOCP_CODER_DST_OM_CNF == ulDstChID)
    {
        diag_PTR(EN_DIAG_PTR_SCM_CODER_DST_CB);
    }

    ulChType = SOCP_REAL_CHAN_TYPE(ulDstChID);

    if (SOCP_CODER_DEST_CHAN != ulChType)
    {
        SCM_CODER_DST_ERR("SCM_CoderDestReadCB: Channel Type is Error", ulDstChID, ulChType);/* ��¼Log */
        return;
    }

    if (VOS_OK != BSP_SOCP_GetReadBuff(ulDstChID, &stBuffer))
    {
        SCM_CODER_DST_ERR("SCM_CoderDestReadCB: Get Read Buffer is Error", ulDstChID, 0);/* ��¼Log */
        return;
    }

    if (VOS_OK != SCM_FindChannelCfg(ulDstChID, SCM_CODER_DST_NUM,
                                (SCM_CHANNEL_CFG_HEAD *)g_astSCMCoderDstCfg, sizeof(SCM_CODER_DEST_CFG_STRU), &ulCfgNum))
    {
        BSP_SOCP_ReadDataDone(ulDstChID, stBuffer.u32Size + stBuffer.u32RbSize);  /* ������� */

        if(SOCP_CODER_DST_OM_IND == ulDstChID)
        {
            diag_ReportLost(EN_DIAG_LOST_BRANCH);
        }

        SCM_CODER_DST_ERR("SCM_CoderDestReadCB: Find Channel is Error", ulDstChID, 0);/* ��¼Log */

        return;
    }

     /* ����log���ܣ�INDͨ���ϱ�����Ϊ�գ�ʹlog�����ڱ��� */
    if(VOS_NULL_PTR == g_astSCMCoderDstCfg[ulCfgNum].pfunc)
    {
        return;
    }

    if((0 == (stBuffer.u32Size + stBuffer.u32RbSize))||(VOS_NULL_PTR == stBuffer.pBuffer))
    {
        BSP_SOCP_ReadDataDone(ulDstChID, stBuffer.u32Size + stBuffer.u32RbSize);  /* ������� */

        if(SOCP_CODER_DST_OM_IND == ulDstChID)
        {
            diag_ReportLost(EN_DIAG_LOST_BRANCH);
        }

        SCM_CODER_DST_ERR("SCM_CoderDestReadCB: Get RD error ", ulDstChID,0);/* ��¼Log */
        return;
    }

    if(0 == stBuffer.u32Size)
    {
        return;
    }

    /* �������� */
    ulVirtAddr = VOS_UncacheMemPhyToVirt((VOS_UINT8 *)stBuffer.pBuffer,
                                g_astSCMCoderDstCfg[ulCfgNum].pucBufPHY,
                                g_astSCMCoderDstCfg[ulCfgNum].pucBuf,
                                g_astSCMCoderDstCfg[ulCfgNum].ulBufLen);
    if(VOS_NULL_PTR == ulVirtAddr)
    {
        BSP_SOCP_ReadDataDone(ulDstChID, stBuffer.u32Size + stBuffer.u32RbSize);  /* ������� */

        if(SOCP_CODER_DST_OM_IND == ulDstChID)
        {
            diag_ReportLost(EN_DIAG_LOST_BRANCH);
        }

        SCM_CODER_DST_ERR("SCM_CoderDestReadCB:  stBuffer.pBuffer == VOS_NULL", ulDstChID, 0);/* ��¼Log */
        return;
    }
    ulTimerIn = mdrv_timer_get_normal_timestamp();

    g_astSCMCoderDstCfg[ulCfgNum].pfunc(ulDstChID, (VOS_UINT8*)ulVirtAddr, (VOS_UINT8*)stBuffer.pBuffer,(VOS_UINT32)stBuffer.u32Size);
    ulTimerOut = mdrv_timer_get_normal_timestamp();
    /* ��¼�ص�������ִ��ʱ�� */
    SCM_CODER_DST_LOG("SCM_CoderDestReadCB: Call channel Func Proc time", ulDstChID, (ulTimerIn-ulTimerOut));

    return;
}

/* ****************************************************************************
 �� �� ��  : SCM_CoderDestEventCB
 ��������  : �������Ŀ��ͨ����Event�¼��ص�
 �������  : ulDstChID Ŀ��ͨ��ID
 �������  : ��
 �� �� ֵ  : ��
 ���ú���  :
 ��������  :
 �޸���ʷ  :

**************************************************************************** */
VOS_VOID SCM_CoderDestEventCB(VOS_UINT32 ulDstChID,SOCP_EVENT_ENUM_UIN32 u32Event, VOS_UINT32 u32Param)
{
    
    if(SOCP_EVENT_OUTBUFFER_OVERFLOW == u32Event)
    {
        diag_ReportLost(EN_DIAG_LOST_OVERFLOW);
    }
    else
    {
        /* do nothing */
    }
}


VOS_UINT32 SCM_CoderDstChannelInit(VOS_VOID)
{
    VOS_UINT32                          i;
    SOCP_CODER_DEST_CHAN_S              stChannel;

    for (i = 0; i < SCM_CODER_DST_NUM; i++)
    {
        if (0 == i)
        {
            stChannel.u32EncDstThrh = SCM_CODER_DST_GTHRESHOLD;
        }
        else
        {
            /* �������Ŀ��ͨ��1��ֵ���� */
            stChannel.u32EncDstThrh = 2 * SCM_CODER_DST_GTHRESHOLD;
        }

        stChannel.sCoderSetDstBuf.pucOutputStart    = g_astSCMCoderDstCfg[i].pucBufPHY;

        stChannel.sCoderSetDstBuf.pucOutputEnd
            = (g_astSCMCoderDstCfg[i].pucBufPHY + g_astSCMCoderDstCfg[i].ulBufLen)-1;

        stChannel.sCoderSetDstBuf.u32Threshold      = g_astSCMCoderDstCfg[i].ulThreshold;

        if (VOS_OK != mdrv_socp_coder_set_dest_chan_attr(g_astSCMCoderDstCfg[i].enChannelID, &stChannel))
        {
            g_astSCMCoderDstCfg[i].enInitState = SCM_CHANNEL_CFG_FAIL;  /* ��¼ͨ����ʼ�����ô��� */

            return VOS_ERR;/* ����ʧ�� */
        }

        g_astSCMCoderDstCfg[i].enInitState = SCM_CHANNEL_INIT_SUCC;     /* ��¼ͨ����ʼ�����ô��� */

        BSP_SOCP_RegisterReadCB(g_astSCMCoderDstCfg[i].enChannelID, (socp_read_cb)SCM_CoderDestReadCB);

        DRV_SOCP_REGISTER_EVENT_CB(g_astSCMCoderDstCfg[i].enChannelID, (socp_event_cb)SCM_CoderDestEventCB);
    }

    return VOS_OK;
}




VOS_UINT32 SCM_RegCoderDestProc(SOCP_CODER_DST_ENUM_U32 enChanlID, SCM_CODERDESTFUCN func)
{
    VOS_UINT32                          ulCgfNum;

    if (VOS_OK != SCM_FindChannelCfg(enChanlID,
                                SCM_CODER_DST_NUM,
                                (SCM_CHANNEL_CFG_HEAD *)g_astSCMCoderDstCfg,
                                sizeof(SCM_CODER_DEST_CFG_STRU),
                                &ulCgfNum))
    {
        return VOS_ERR;/* ����ʧ�� */
    }

    g_astSCMCoderDstCfg[ulCgfNum].pfunc = func;

    return VOS_OK;/* ���سɹ� */
}



VOS_UINT32 SCM_RegDecoderDestProc(SOCP_DECODER_DST_ENUM_U32 enChanlID, SCM_DECODERDESTFUCN func)
{
    VOS_UINT32                          ulOffset;

    if (enChanlID >= SOCP_DECODER_DST_BUTT)
    {
        return VOS_ERR;
    }

    if (VOS_NULL_PTR == func)
    {
        return VOS_ERR;
    }

    if (SOCP_DECODER_DST_LOM == enChanlID)
    {
        ulOffset = SOCP_DECODER_DST_CB_TL_OM;
    }
    else if (SOCP_DECODER_DST_GUOM == enChanlID)
    {
        ulOffset = SOCP_DECODER_DST_CB_GU_OM;
    }
    else if (SOCP_DECODER_CBT == enChanlID)
    {
        ulOffset = SOCP_DECODER_DST_CB_GU_CBT;
    }
    else
    {
        return VOS_ERR;
    }

    g_astSCMDecoderCbFunc[ulOffset] = func;

    return VOS_OK;
}

VOS_UINT32 SCM_CoderDstCompressCfg(VOS_BOOL enable)
{
    if (enable)
    {
        mdrv_socp_compress_enable(SOCP_CODER_DST_OM_IND);
    }
    else
    {
        mdrv_socp_compress_disable(SOCP_CODER_DST_OM_IND);
    }
    return VOS_OK;
}



VOS_UINT32 SCM_CreateSrcBuffer(VOS_UINT8 **pBufVir, VOS_UINT8 **pBufPhy, VOS_UINT32 ulLen)
{
    VOS_UINT_PTR ulRealAddr;

    /*����uncache�Ķ�̬�ڴ���*/
    *pBufVir = (VOS_UINT8*)VOS_UnCacheMemAlloc(ulLen, &ulRealAddr);

    /* �����ڴ�ʧ�� */
    if (VOS_NULL_PTR == *pBufVir)
    {
        return VOS_ERR;
    }

    /* ����bufʵ��ַ */
    *pBufPhy = (VOS_UINT8*)ulRealAddr;

    return VOS_OK;
}



VOS_UINT32 SCM_CoderSrcChanMemInit(VOS_VOID)
{
    VOS_UINT32                           i;
    VOS_UINT32                          ulRet;

    for (i=0; i<SCM_CODER_SRC_NUM; i++)
    {
        if(g_astSCMCoderSrcCfg[i].enChannelID == SCM_CODER_SRC_LOM_CNF)
        {
            ulRet = SCM_CreateSrcBuffer(&g_astSCMCoderSrcCfg[i].pucSrcBuf,
                                        &g_astSCMCoderSrcCfg[i].pucSrcPHY,
                                        SCM_CODER_SRC_CNF_BUFFER_SIZE);
            if(VOS_OK != ulRet)
            {
                return VOS_ERR;
            }
            g_astSCMCoderSrcCfg[i].ulSrcBufLen  = SCM_CODER_SRC_CNF_BUFFER_SIZE;
        }
        else if(g_astSCMCoderSrcCfg[i].enChannelID == SCM_CODER_SRC_LOM_IND)
        {
            ulRet = SCM_CreateSrcBuffer(&g_astSCMCoderSrcCfg[i].pucSrcBuf,
                                        &g_astSCMCoderSrcCfg[i].pucSrcPHY,
                                        SCM_CODER_SRC_IND_BUFFER_SIZE);
            if(VOS_OK != ulRet)
            {
                return VOS_ERR;
            }
            g_astSCMCoderSrcCfg[i].ulSrcBufLen  = SCM_CODER_SRC_IND_BUFFER_SIZE;

        }
        else
        {
            /* TODO: CJQ */
            /* SOCP_CODER_SRC_LOM_IND3 ͨ����δʹ�ã��ݲ������ڴ� */
        }
    }

    return VOS_OK;

}


VOS_UINT32 SCM_ChannelMemInit(VOS_VOID)
{
    VOS_UINT32  ulRet;
    NV_POWER_ON_LOG_SWITCH_STRU     stPowerOnLog;
    SOCP_ENC_DST_BUF_LOG_CFG_STRU   stLogCfg;

    if(VOS_OK != SCM_CoderSrcChanMemInit())
    {
        return VOS_ERR;
    }
    
    VOS_MemSet_s(&stLogCfg,(VOS_UINT32)sizeof(stLogCfg),0,sizeof(stLogCfg));
    if(VOS_OK == mdrv_socp_get_sd_logcfg(&stLogCfg))
    {
        ulRet = NV_Read(EN_NV_ID_POWER_ON_LOG_SWITCH, (u8*)&stPowerOnLog, sizeof(stPowerOnLog));
        
        if(VOS_OK != ulRet)
        {
            (VOS_VOID)vos_printf("Read nv 0x%x fail.\n", EN_NV_ID_POWER_ON_LOG_SWITCH);
        }
        else
        {
            stPowerOnLog.cPowerOnlogA = (stLogCfg.BufferSize < 50*1024*1024) ? 0 : 1;
            if(VOS_OK != NV_Write(EN_NV_ID_POWER_ON_LOG_SWITCH, (u8*)&stPowerOnLog, sizeof(stPowerOnLog)))
            {
                (VOS_VOID)vos_printf("Write nv 0x%x fail.\n", EN_NV_ID_POWER_ON_LOG_SWITCH);
            }
            else
            {
                (VOS_VOID)vos_printf("Write power on log nv 0x%x success.\n", EN_NV_ID_POWER_ON_LOG_SWITCH);
            }
        }
    }

    return VOS_OK;
}



VOS_UINT32 SCM_ChannelInit(VOS_VOID)
{
    VOS_MemSet_s(&g_stSCMInfoData, (VOS_UINT32)sizeof(g_stSCMInfoData), 0, sizeof(g_stSCMInfoData));

    if (VOS_OK != SCM_CoderSrcChannelInit()) /* ����Դͨ����ʼ�� */
    {
        return VOS_ERR;/* ���ش��� */
    }

    return VOS_OK;
}


VOS_VOID SCM_ChannelInfoShow(VOS_VOID)
{
    VOS_UINT32      i;

    for(i=0; i<SCM_CODER_SRC_NUM; i++)
    {
        vos_printf("\r\n The Channle 0x%x info is :", g_astSCMCoderSrcCfg[i].enChannelID);

        vos_printf("\r\n The Max BD num is %d", g_stSCMInfoData.aulBDUsedMax[i]);

        vos_printf("\r\n The Max rD num is %d \r\n", g_stSCMInfoData.aulRDUsedMax[i]);
    }

    return;
}



VOS_VOID SCM_CoderSrcCHShow(VOS_UINT32 ulCfgNum)
{
    vos_printf("\r\n CH id         is 0x%x", g_astSCMCoderSrcCfg[ulCfgNum].enChannelID);
    vos_printf("\r\n CH init state is   %d", g_astSCMCoderSrcCfg[ulCfgNum].enInitState);
    vos_printf("\r\n CH type       is   %d", g_astSCMCoderSrcCfg[ulCfgNum].enCHMode);
    vos_printf("\r\n CH Dst        is 0x%x", g_astSCMCoderSrcCfg[ulCfgNum].enDstCHID);
    vos_printf("\r\n CH data type  is   %d", g_astSCMCoderSrcCfg[ulCfgNum].enDataType);
    vos_printf("\r\n CH Level      is   %d", g_astSCMCoderSrcCfg[ulCfgNum].enCHLevel);
    vos_printf("\r\n CH BD VirtAddris 0x%p", g_astSCMCoderSrcCfg[ulCfgNum].pucSrcBuf);
    vos_printf("\r\n CH BD PHYAddr is 0x%p", g_astSCMCoderSrcCfg[ulCfgNum].pucSrcPHY);
    vos_printf("\r\n CH BD BufLen  is 0x%x", g_astSCMCoderSrcCfg[ulCfgNum].ulSrcBufLen);
    vos_printf("\r\n CH RD VirtAddris 0x%p", g_astSCMCoderSrcCfg[ulCfgNum].pucRDBuf);
    vos_printf("\r\n CH RD PHYAddr is 0x%p", g_astSCMCoderSrcCfg[ulCfgNum].pucRDPHY);
    vos_printf("\r\n CH RD BufLen  is 0x%x", g_astSCMCoderSrcCfg[ulCfgNum].ulRDBufLen);

    vos_printf("\r\n CH SEND data   LENGTH          0x%x", g_astScmCoderSrcDebugInfo[ulCfgNum].ulSendDataLen);
    vos_printf("\r\n CH SEND packet NUM             0x%x", g_astScmCoderSrcDebugInfo[ulCfgNum].ulSendPacketNum);
    vos_printf("\r\n CH SEND addr   ERR             0x%x", g_astScmCoderSrcDebugInfo[ulCfgNum].ulSendAddrErr);
    vos_printf("\r\n CH SEND head   ERR             0x%x", g_astScmCoderSrcDebugInfo[ulCfgNum].ulSendHeaderErr);
    vos_printf("\r\n CH SEND write  ERR             0x%x", g_astScmCoderSrcDebugInfo[ulCfgNum].ulSendWriteDoneErr);
    vos_printf("\r\n CH SEND first buff not enough  0x%x", g_astScmCoderSrcDebugInfo[ulCfgNum].ulSendFirstNotEnough);

    vos_printf("\r\n CH SEND get buffer ERR         0x%x", g_astScmCoderSrcDebugInfo[ulCfgNum].ulGetCoderBuffErr);
    vos_printf("\r\n CH SEND get coherent buff ERR  0x%x", g_astScmCoderSrcDebugInfo[ulCfgNum].ulGetCoherentBuffErr);

    vos_printf("\r\n CH SEND fill max buff  len     0x%x", g_astScmCoderSrcDebugInfo[ulCfgNum].ulFillFirstBuffMax);
    vos_printf("\r\n CH SEND fill num               0x%x", g_astScmCoderSrcDebugInfo[ulCfgNum].ulFillFirstBuffNum);
    vos_printf("\r\n CH SEND after fill not enough  0x%x", g_astScmCoderSrcDebugInfo[ulCfgNum].ulAfterFillNotEnough);
    vos_printf("\r\n CH SEND fill not enough        0x%x", g_astScmCoderSrcDebugInfo[ulCfgNum].ulFillNotEnough);


    return;
}



VOS_VOID SCM_CoderDstCHShow(VOS_UINT32 ulCfgNum)
{
    vos_printf("\r\n CH id         is 0x%x", g_astSCMCoderDstCfg[ulCfgNum].enChannelID);
    vos_printf("\r\n CH init state is   %d", g_astSCMCoderDstCfg[ulCfgNum].enInitState);
    vos_printf("\r\n CH BD VirtAddris 0x%x", g_astSCMCoderDstCfg[ulCfgNum].pucBuf);
    vos_printf("\r\n CH BD PHYAddr is 0x%x", g_astSCMCoderDstCfg[ulCfgNum].pucBufPHY);
    vos_printf("\r\n CH BD BufLen  is 0x%x", g_astSCMCoderDstCfg[ulCfgNum].ulBufLen);
    vos_printf("\r\n CH threshold  is 0x%x", g_astSCMCoderDstCfg[ulCfgNum].ulThreshold);
    vos_printf("\r\n CH CB func    is 0x%x", g_astSCMCoderDstCfg[ulCfgNum].pfunc);

    return;
}


SCM_INFODATA_STRU* SCM_GetDebugLogInfo(VOS_VOID)
{
    return &g_stSCMInfoData;
}


VOS_UINT32 SCM_GetDebugLogInfoLen(VOS_VOID)
{
    return (VOS_UINT32)sizeof(g_stSCMInfoData);
}



VOS_VOID SCM_SocpSendDataToUDISucc(
    SOCP_CODER_DST_ENUM_U32             enChanID,
    CPM_PHY_PORT_ENUM_UINT32            enPhyport,
    OM_SOCP_CHANNEL_DEBUG_INFO         *pstDebugInfo,
    VOS_UINT32                         *pulSendDataLen
)
{
    if ((SOCP_CODER_DST_OM_CNF == enChanID) && (CPM_CFG_PORT == enPhyport))
    {
        if ((0 == g_stUsbCfgPseudoSync.ulLen) || (*pulSendDataLen != g_stUsbCfgPseudoSync.ulLen))
        {
            pstDebugInfo->ulUSBSendCBAbnormalNum++;
            pstDebugInfo->ulUSBSendCBAbnormalLen += *pulSendDataLen;
        }

        *pulSendDataLen = g_stUsbCfgPseudoSync.ulLen;
    }
    else if ((SOCP_CODER_DST_OM_IND == enChanID) && (CPM_IND_PORT == enPhyport))
    {
        if ((0 == g_stUsbIndPseudoSync.ulLen) || (*pulSendDataLen != g_stUsbIndPseudoSync.ulLen))
        {
            pstDebugInfo->ulUSBSendCBAbnormalNum++;
            pstDebugInfo->ulUSBSendCBAbnormalLen += *pulSendDataLen;
        }

        *pulSendDataLen = g_stUsbIndPseudoSync.ulLen;
    }
    else
    {
        ;
    }

    return;
}


VOS_VOID SCM_SocpSendDataToUDI(SOCP_CODER_DST_ENUM_U32 enChanID, VOS_UINT8 *pucVirData, VOS_UINT8 *pucPHYData, VOS_UINT32 ulDataLen)
{
    VOS_UINT32                  ulResult;
    VOS_UINT32                  ulRet = VOS_ERR;
    CPM_PHY_PORT_ENUM_UINT32    enPhyport;
    VOS_UINT32                  ulSendDataLen;
    VOS_BOOL                    bUsbSendSucFlag = VOS_FALSE;
    VOS_BOOL                    bUsbSendFlag = VOS_FALSE;
    OM_SOCP_CHANNEL_DEBUG_INFO  *pstDebugInfo = VOS_NULL_PTR;
    CPM_LOGIC_PORT_ENUM_UINT32  enLogicPort;

    if(SOCP_CODER_DST_OM_CNF == enChanID)
    {
        diag_PTR(EN_DIAG_PTR_SCM_SENDTOUDI);
    }

    if (SOCP_CODER_DST_OM_CNF == enChanID)
    {
        pstDebugInfo = &g_stAcpuDebugInfo.stCnfDebugInfo;
        enLogicPort  = CPM_OM_CFG_COMM;
    }
    else if (SOCP_CODER_DST_OM_IND == enChanID)
    {
        pstDebugInfo = &g_stAcpuDebugInfo.stIndDebugInfo;
        enLogicPort  = CPM_OM_IND_COMM;
    }
    else
    {
        g_stAcpuDebugInfo.ulInvaldChannel++;

        return;
    }

    /*�������*/
    SOCP_SEND_DATA_PARA_CHECK(pstDebugInfo, ulDataLen, pucVirData);

    PPM_GetSendDataLen(enChanID, ulDataLen, &ulSendDataLen, &enPhyport);

    if(SOCP_CODER_DST_OM_CNF == enChanID)
    {
        OM_ACPU_DEBUG_CHANNEL_TRACE(enChanID, pucVirData, ulSendDataLen, OM_ACPU_SEND_USB, OM_ACPU_DATA);
    }
    else if(SOCP_CODER_DST_OM_IND == enChanID)
    {
        OM_ACPU_DEBUG_CHANNEL_TRACE(enChanID, pucVirData, ulSendDataLen, OM_ACPU_SEND_USB_IND, OM_ACPU_DATA);
    }

    /* ͳ������ͨ���������� */
    if(CPM_OM_IND_COMM == enLogicPort)
    {
        diag_ThroughputSave(EN_DIAG_THRPUT_DATA_CHN_PHY, ulSendDataLen);
    }
    ulResult = CPM_ComSend(enLogicPort, pucVirData, pucPHYData, ulSendDataLen);

    if(CPM_SEND_ERR == ulResult)  /*��ǰͨ���Ѿ�����ʧ�ܣ�����SOCPͨ�������ݰ���*/
    {
        pstDebugInfo->ulUSBSendErrNum++;
        pstDebugInfo->ulUSBSendErrLen += ulSendDataLen;

        if (SOCP_CODER_DST_OM_IND == enChanID)
        {
            diag_ReportLost(EN_DIAG_LOST_CPMWR);
        }
    }
    else if(CPM_SEND_FUNC_NULL == ulResult)   /*��ǰͨ���쳣���ӵ���������*/
    {
        pstDebugInfo->ulOmDiscardNum++;
        pstDebugInfo->ulOmDiscardLen += ulDataLen;

        if (SOCP_CODER_DST_OM_IND == enChanID)
        {
            diag_ReportLost(EN_DIAG_LOST_CPMWR);
        }
    }
    else if(CPM_SEND_PARA_ERR == ulResult)   /* �������ݻ�ȡʵ��ַ�쳣 */
    {
        pstDebugInfo->ulOmGetVirtErr++;
        pstDebugInfo->ulOmGetVirtSendLen += ulDataLen;

        if (SOCP_CODER_DST_OM_IND == enChanID)
        {
            diag_ReportLost(EN_DIAG_LOST_CPMWR);
        }
    }
    else if(CPM_SEND_AYNC == ulResult) //����cpm������
    {
        bUsbSendSucFlag = VOS_TRUE;
        bUsbSendFlag    = VOS_TRUE;
        ulRet           = VOS_OK;
    }
    else if(CPM_SEND_OK == ulResult)
    {
        SCM_SocpSendDataToUDISucc(enChanID, enPhyport, pstDebugInfo, &ulSendDataLen);

        bUsbSendSucFlag = VOS_TRUE;
    }
    else
    {
        vos_printf("PPM_SocpSendDataToUDI: CPM_ComSend return Error %d", (VOS_INT)ulResult);
    }

    if(bUsbSendFlag != VOS_TRUE)
    {
        ulRet = SCM_RlsDestBuf(enChanID, ulSendDataLen);

        if(VOS_OK != ulRet)
        {
            pstDebugInfo->ulSocpReadDoneErrNum++;
            pstDebugInfo->ulSocpReadDoneErrLen += ulSendDataLen;

            vos_printf("PPM_SocpSendDataToUDI: SCM_RlsDestBuf return Error %d", (VOS_INT)ulRet);
        }
    }

    if ((VOS_OK == ulRet) && (VOS_TRUE == bUsbSendSucFlag))
    {
        pstDebugInfo->ulUSBSendNum++;
        pstDebugInfo->ulUSBSendRealLen += ulSendDataLen;
    }

    return;
}


VOS_VOID SCM_RegCoderDestIndChan(VOS_VOID)
{
    vos_printf("SCM_RegCoderDestIndChan.\n");

    /* ��OM SOCPĿ��ͨ��������ע���SCM */
    if (VOS_OK != SCM_RegCoderDestProc(SOCP_CODER_DST_OM_IND, (SCM_CODERDESTFUCN)SCM_SocpSendDataToUDI))
    {
        vos_printf("SCM_RegCoderDestIndChan Reg OM IND Fail.\n");

        return ;
    }
}

VOS_VOID SCM_unRegCoderDestIndChan(VOS_VOID)
{
    vos_printf("SCM_unRegCoderDestIndChan.\n");

    /* ��OM SOCPĿ��ͨ��������ע���SCM */
    if (VOS_OK != SCM_RegCoderDestProc(SOCP_CODER_DST_OM_IND, (SCM_CODERDESTFUCN)VOS_NULL))
    {
        vos_printf("SCM_unRegCoderDestIndChan Reg OM IND Fail.\n");

        return ;
    }
}


VOS_UINT32 SCM_Init(VOS_VOID)
{
   if (VOS_OK != SCM_ChannelMemInit())/* ͨ���ڴ��ʼ�� */
   {
       return VOS_ERR;
   }

   if(VOS_OK != SCM_ChannelInit())
   {
        return VOS_ERR;
   }
   /* ��OM SOCPĿ��ͨ��������ע���SCM */
   if (VOS_OK != SCM_RegCoderDestProc(SOCP_CODER_DST_OM_CNF, (SCM_CODERDESTFUCN)SCM_SocpSendDataToUDI))
   {
        vos_printf("SCM_Init:SCM_RegCoderDestProc Reg OM CNF Fail.\n");

        return VOS_ERR;
    }

    CPM_LogicRcvReg(CPM_OM_CFG_COMM, SCM_SoftDecodeCfgDataRcv);

    g_stScmSoftDecodeInfo.ulCpmRegLogicRcvSuc++;

    return VOS_OK;
}


VOS_UINT32 COMM_Init(VOS_VOID)
{
    if (VOS_OK != SCM_Init())
    {
        return VOS_ERR;
    }

    /* ��ʼ������ͨ�� */
    if (VOS_OK != PPM_InitPhyPort())
    {
        return VOS_ERR;
    }

    return VOS_OK;
}


unsigned long long g_scm_dma_mask = (unsigned long long) (-1);


VOS_VOID SCM_FlushCpuCache( VOS_VOID *pAddress, VOS_INT lSize )
{
    struct device dev = {0};

    dev.dma_mask = (unsigned long long *)(&g_scm_dma_mask);
    dma_map_single(&dev, pAddress, lSize, DMA_TO_DEVICE);
}



VOS_VOID SCM_InvalidCpuCache( VOS_VOID *pAddress, VOS_INT lSize )
{
    struct device dev = {0};

    dev.dma_mask = (unsigned long long *)(&g_scm_dma_mask);
    dma_map_single(&dev, pAddress, lSize, DMA_FROM_DEVICE);
}





