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

#ifndef _BSP_SOCP_H
#define _BSP_SOCP_H

#include "osl_common.h"
#include "mdrv_socp_common.h"
#include "bsp_trace.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**************************************************************************
  �����붨��
**************************************************************************/
#define BSP_ERR_SOCP_BASE            BSP_DEF_ERR(BSP_MODU_SOCP, 0)
#define BSP_ERR_SOCP_NULL            (BSP_ERR_SOCP_BASE + 0x1)
#define BSP_ERR_SOCP_NOT_INIT        (BSP_ERR_SOCP_BASE + 0x2)
#define BSP_ERR_SOCP_MEM_ALLOC       (BSP_ERR_SOCP_BASE + 0x3)
#define BSP_ERR_SOCP_SEM_CREATE      (BSP_ERR_SOCP_BASE + 0x4)
#define BSP_ERR_SOCP_TSK_CREATE      (BSP_ERR_SOCP_BASE + 0x5)
#define BSP_ERR_SOCP_INVALID_CHAN    (BSP_ERR_SOCP_BASE + 0x6)
#define BSP_ERR_SOCP_INVALID_PARA    (BSP_ERR_SOCP_BASE + 0x7)
#define BSP_ERR_SOCP_NO_CHAN         (BSP_ERR_SOCP_BASE + 0x8)
#define BSP_ERR_SOCP_SET_FAIL        (BSP_ERR_SOCP_BASE + 0x9)
#define BSP_ERR_SOCP_TIMEOUT         (BSP_ERR_SOCP_BASE + 0xa)
#define BSP_ERR_SOCP_NOT_8BYTESALIGN (BSP_ERR_SOCP_BASE + 0xb)
#define BSP_ERR_SOCP_CHAN_RUNNING    (BSP_ERR_SOCP_BASE + 0xc)
#define BSP_ERR_SOCP_CHAN_MODE       (BSP_ERR_SOCP_BASE + 0xd)
#define BSP_ERR_SOCP_DEST_CHAN       (BSP_ERR_SOCP_BASE + 0xe)
#define BSP_ERR_SOCP_DECSRC_SET      (BSP_ERR_SOCP_BASE + 0xf)

typedef u32 (*socp_compress_isr)     (void);
typedef u32 (*socp_compress_event_cb)(socp_event_cb EventCB);
typedef u32 (*socp_compress_read_cb) (socp_read_cb ReadCB);
typedef u32 (*socp_compress_enable)  (void);
typedef u32 (*socp_compress_disable) (void);
typedef u32 (*socp_compress_set)     (u32 chanid, SOCP_CODER_DEST_CHAN_S* attr);
typedef u32 (*socp_compress_getbuffer) (SOCP_BUFFER_RW_STRU *pRingBuffer);
typedef u32 (*socp_compress_readdone)(u32 u32Size);
typedef u32 (*socp_compress_clear)   (void);

typedef struct socp_compress_ops
{
    socp_compress_isr       isr;
    socp_compress_event_cb  register_Eventcb;
    socp_compress_read_cb   register_Readcb;
    socp_compress_enable    enable;
    socp_compress_disable   disable;
    socp_compress_set       set;
    socp_compress_getbuffer getbuffer;
    socp_compress_readdone  readdone;
    socp_compress_clear     clear;
}socp_compress_ops_stru;

#if (FEATURE_SOCP_DECODE_INT_TIMEOUT == FEATURE_ON)
typedef enum timeout_module
{
    DECODE_TIMEOUT_INT_TIMEOUT = 0,
    DECODE_TIMEOUT_DEC_INT_TIMEOUT = 1,
    DECODE_TIMEOUT_BUTTON = 2,

} DECODE_TIMEOUT_MODULE;

#endif

typedef struct
{
    SOCP_VOTE_TYPE_ENUM_U32 type;
}socp_vote_req_stru;

typedef struct
{
    u32 vote_rst;   /* 1:�ɹ�����1:ʧ�� */
}socp_vote_cnf_stru;

struct socp_enc_dst_log_cfg
{
    void*           pVirBuffer;      /* SOCP����Ŀ��ͨ����������BUFFER����32λϵͳ����4�ֽڣ���64λϵͳ����8�ֽ� */
    unsigned long   ulPhyBufferAddr; /* SOCP����Ŀ��ͨ����������BUFFER��ַ */
    unsigned int    BufferSize;      /* SOCP����Ŀ��ͨ������BUFFER��С */
    unsigned int    overTime;        /* NVE�����õĳ�ʱʱ�� */
    unsigned int    logOnFlag;       /* ��������buffer����������־(SOCP_DST_CHAN_CFG_TYPE_ENUM) */
    unsigned int    ulCurTimeout;    /* SOCP����Ŀ��ͨ�����ݴ��䳬ʱʱ�� */
    unsigned int    flushFlag;
	unsigned int    memLogCfg;
	unsigned int    currentMode;
    
};

/*****************************************************************************
* �� �� ��  : socp_init
*
* ��������  : ģ���ʼ������
*
* �������  : ��
*
* �������  : ��
*
* �� �� ֵ  : ��ʼ���ɹ��ı�ʶ��
*****************************************************************************/
s32 socp_init(void);

/*****************************************************************************
 �� �� ��  : bsp_socp_coder_set_src_chan
 ��������  : �˽ӿ����SOCP������Դͨ���ķ��䣬���ݱ�����Դͨ����������ͨ�����ԣ�����Ŀ��ͨ�������غ���ִ�н����
 �������  : pSrcAttr:������Դͨ�������ṹ��ָ�롣
             pSrcChanID:���뵽��Դͨ��ID��
 �������  : �ޡ�
 �� �� ֵ  : SOCP_OK:����Դͨ������ɹ���
             SOCP_ERROR:����Դͨ������ʧ�ܡ�
*****************************************************************************/
s32 bsp_socp_coder_set_src_chan(SOCP_CODER_SRC_ENUM_U32 enSrcChanID, SOCP_CODER_SRC_CHAN_S *pSrcAttr);

/*****************************************************************************
 �� �� ��  : bsp_socp_coder_set_dest_chan_attr
 ��������  : �˽ӿ����ĳһ����Ŀ��ͨ�������ã����غ���ִ�еĽ����
 �������  : u32DestChanID:SOCP��������Ŀ��ͨ��ID��
             pDestAttr:SOCP������Ŀ��ͨ�������ṹ��ָ�롣
 �������  : �ޡ�
 �� �� ֵ  : SOCP_OK:����Ŀ��ͨ�����óɹ���
             SOCP_ERROR:����Ŀ��ͨ������ʧ�ܡ�
*****************************************************************************/
s32 bsp_socp_coder_set_dest_chan_attr(u32 u32DestChanID, SOCP_CODER_DEST_CHAN_S *pDestAttr);

/*****************************************************************************
 �� �� ��      : bsp_socp_decoder_set_dest_chan
 ��������  :�˽ӿ����SOCP������Ŀ��ͨ���ķ��䣬
                ���ݽ���Ŀ��ͨ����������ͨ�����ԣ�
                ������Դͨ�������غ���ִ�н����
 �������  : pAttr:������Ŀ��ͨ�������ṹ��ָ��
                         pDestChanID:���뵽��Ŀ��ͨ��ID
 �������  : �ޡ�
 �� �� ֵ      : SOCP_OK:����Ŀ��ͨ������ɹ���
                             SOCP_ERROR:����Ŀ��ͨ������ʧ�ܡ�
*****************************************************************************/
s32 bsp_socp_decoder_set_dest_chan(SOCP_DECODER_DST_ENUM_U32 enDestChanID, SOCP_DECODER_DEST_CHAN_STRU *pAttr);

/*****************************************************************************
 �� �� ��      : bsp_socp_decoder_set_src_chan_attr
 ��������  :�˽ӿ����ĳһ����Դͨ�������ã����غ���ִ�еĽ����
 �������  : u32SrcChanID:������Դͨ��ID
                         pInputAttr:������Դͨ�������ṹ��ָ��
 �������  : �ޡ�
 �� �� ֵ      : SOCP_OK:����Դͨ�����óɹ�
                             SOCP_ERROR:����Դͨ������ʧ��
*****************************************************************************/
s32 bsp_socp_decoder_set_src_chan_attr ( u32 u32SrcChanID,SOCP_DECODER_SRC_CHAN_STRU *pInputAttr);

/*****************************************************************************
 �� �� ��      : bsp_socp_decoder_get_err_cnt
 ��������  :�˽ӿڸ�������ͨ���������쳣����ļ���ֵ��
 �������  : u32ChanID:������ͨ��ID
                         pErrCnt:�������쳣�����ṹ��ָ��
 �������  : �ޡ�
 �� �� ֵ      : SOCP_OK:�����쳣�����ɹ�
                             SOCP_ERROR:�����쳣����ʧ��
*****************************************************************************/
s32 bsp_socp_decoder_get_err_cnt (u32 u32ChanID, SOCP_DECODER_ERROR_CNT_STRU *pErrCnt);

/*****************************************************************************
 �� �� ��  : bsp_socp_free_channel
 ��������  : �˽ӿڸ���ͨ��ID�ͷŷ���ı����ͨ����
 �������  : u32ChanID:ͨ��ID��
 �������  : �ޡ�
 �� �� ֵ  : SOCP_OK:ͨ���ͷųɹ���
             SOCP_ERROR:ͨ���ͷ�ʧ�ܡ�
*****************************************************************************/
s32 bsp_socp_free_channel(u32 u32ChanID);

/*****************************************************************************
* �� �� ��  : socp_clean_encsrc_chan
*
* ��������  : ��ձ���Դͨ����ͬ��V9 SOCP�ӿ�
*
* �������  : enSrcChanID       ����ͨ����
*
* �������  : ��
*
* �� �� ֵ  : BSP_OK
*****************************************************************************/
u32 bsp_socp_clean_encsrc_chan(SOCP_CODER_SRC_ENUM_U32 enSrcChanID);

/*****************************************************************************
 �� �� ��  : bsp_socp_register_event_cb
 ��������  : �˽ӿ�Ϊ����ͨ��ע���¼��ص�������
 �������  : u32ChanID:ͨ��ID��
             EventCB:�¼��ص��������ο�socp_event_cb��������
 �������  : �ޡ�
 �� �� ֵ  : SOCP_OK:ע���¼��ص������ɹ���
             SOCP_ERROR:ע���¼��ص�����ʧ�ܡ�
*****************************************************************************/
s32 bsp_socp_register_event_cb(u32 u32ChanID, socp_event_cb EventCB);

/*****************************************************************************
 �� �� ��  : bsp_socp_start
 ��������  : �˽ӿ�����Դͨ��������������߽��롣
 �������  : u32SrcChanID:Դͨ��ID��
 �������  : �ޡ�
 �� �� ֵ  : SOCP_OK:�������������ɹ���
             SOCP_ERROR:������������ʧ�ܡ�
*****************************************************************************/
s32 bsp_socp_start(u32 u32SrcChanID);

/*****************************************************************************
 �� �� ��  : bsp_socp_stop
 ��������  : �˽ӿ�����Դͨ����ֹͣ������߽��롣
 �������  : u32SrcChanID:Դͨ��ID��
 �������  : �ޡ�
 �� �� ֵ  : SOCP_OK:��������ֹͣ�ɹ���
             SOCP_ERROR:��������ֹͣʧ�ܡ�
*****************************************************************************/
s32 bsp_socp_stop(u32 u32SrcChanID);

/*****************************************************************************
 �� �� ��      : bsp_socp_set_timeout
 ��������  :�˽ӿ����ó�ʱ��ֵ��
 �������  : u32Timeout:��ʱ��ֵ

 �������  : �ޡ�
 �� �� ֵ      : SOCP_OK:���ó�ʱʱ����ֵ�ɹ���
                             SOCP_ERROR:���ó�ʱʱ����ֵʧ��
*****************************************************************************/
s32 bsp_socp_set_timeout (SOCP_TIMEOUT_EN_ENUM_UIN32 eTmOutEn, u32 u32Timeout);

/*****************************************************************************
 �� �� ��   : bsp_socp_set_dec_pkt_lgth
 ��������  :���ý�������ȼ���ֵ
 �������  : pPktlgth:��������ȼ�ֵ

 �������  : �ޡ�
 �� �� ֵ      : SOCP_OK:���óɹ���
                    ����ֵ:����ʧ��
*****************************************************************************/
s32 bsp_socp_set_dec_pkt_lgth(SOCP_DEC_PKTLGTH_STRU *pPktlgth);

/*****************************************************************************
 �� �� ��   : bsp_socp_set_debug
 ��������  :���ý���Դͨ����debugģʽ
 �������  : u32ChanID:ͨ��ID
                  u32DebugEn: debug��ʶ
 �������  : �ޡ�
 �� �� ֵ      : SOCP_OK:���óɹ���
                     ����ֵ:����ʧ��
*****************************************************************************/
s32 bsp_socp_set_debug(u32 u32DestChanID, u32 u32DebugEn);

/*****************************************************************************
 �� �� ��      : bsp_socp_get_write_buff
 ��������  :�˽ӿ����ڻ�ȡд����buffer��
 �������  : u32SrcChanID:Դͨ��ID
                  pBuff:           :д����buffer

 �������  : �ޡ�
 �� �� ֵ      : SOCP_OK:��ȡд����buffer�ɹ���
                             SOCP_ERROR:��ȡд����bufferʧ��
*****************************************************************************/
s32 bsp_socp_get_write_buff( u32 u32SrcChanID, SOCP_BUFFER_RW_STRU *pBuff);

/*****************************************************************************
 �� �� ��      : bsp_socp_write_done
 ��������  :�ýӿ��������ݵ�д�������ṩд�����ݵĳ��ȡ�
 �������  : u32SrcChanID:Դͨ��ID
                  u32WrtSize:   ��д�����ݵĳ���
 �������  : �ޡ�
 �� �� ֵ      : SOCP_OK:д�����ݳɹ���
                             SOCP_ERROR:д������ʧ��
*****************************************************************************/
s32 bsp_socp_write_done(u32 u32SrcChanID, u32 u32WrtSize);

/*****************************************************************************
 �� �� ��      : bsp_socp_register_rd_cb
 ��������  :�ýӿ�����ע���RD�������ж�ȡ���ݵĻص�������
 �������  : u32SrcChanID:Դͨ��ID
                  RdCB:  �¼��ص�����
 �������  : �ޡ�
 �� �� ֵ      : SOCP_OK:ע��RD���λ����������ݻص������ɹ���
                             SOCP_ERROR:ע��RD���λ����������ݻص�����ʧ��
*****************************************************************************/
s32 bsp_socp_register_rd_cb(u32 u32SrcChanID, socp_rd_cb RdCB);

/*****************************************************************************
 �� �� ��      : bsp_socp_get_rd_buffer
 ��������  :�ô˽ӿ����ڻ�ȡRD buffer������ָ�롣
 �������  : u32SrcChanID:Դͨ��ID
                  pBuff:  RD buffer
 �������  : �ޡ�
 �� �� ֵ      : SOCP_OK:��ȡRD���λ������ɹ�
                             SOCP_ERROR:��ȡRD���λ�����ʧ��
*****************************************************************************/
s32 bsp_socp_get_rd_buffer( u32 u32SrcChanID,SOCP_BUFFER_RW_STRU *pBuff);

/*****************************************************************************
 �� �� ��      : bsp_socp_read_rd_done
 ��������  :�˽ӿ������ϲ�֪ͨSOCP��������RD buffer��ʵ�ʶ�ȡ�����ݡ�
 �������  : u32SrcChanID:Դͨ��ID
                  u32RDSize:  ��RD buffer��ʵ�ʶ�ȡ�����ݳ���
 �������  : �ޡ�
 �� �� ֵ      : SOCP_OK:��ȡRDbuffer�е����ݳɹ�
                             SOCP_ERROR:��ȡRDbuffer�е�����ʧ��
*****************************************************************************/
s32 bsp_socp_read_rd_done(u32 u32SrcChanID, u32 u32RDSize);

/*****************************************************************************
 �� �� ��      : bsp_socp_register_read_cb
 ��������  :�ýӿ�����ע������ݵĻص�������
 �������  : u32DestChanID:Ŀ��ͨ��ID
                  ReadCB: �¼��ص�����
 �������  : �ޡ�
 �� �� ֵ      : SOCP_OK:ע������ݻص������ɹ�
                             SOCP_ERROR:ע������ݻص�����ʧ��
*****************************************************************************/
s32 bsp_socp_register_read_cb( u32 u32DestChanID, socp_read_cb ReadCB);

/*****************************************************************************
 �� �� ��      : bsp_socp_register_read_cb
 ��������  :�ô˽ӿ����ڻ�ȡ�����ݻ�����ָ�롣
 �������  : u32DestChanID:Ŀ��ͨ��ID
                  ReadCB: ������buffer
 �������  : �ޡ�
 �� �� ֵ      : SOCP_OK:��ȡ�����ݻ������ɹ���
                             SOCP_ERROR:��ȡ�����ݻ������ɹ���
*****************************************************************************/
s32 bsp_socp_get_read_buff(u32 u32DestChanID,SOCP_BUFFER_RW_STRU *pBuffer);

/*****************************************************************************
 �� �� ��      : bsp_socp_read_data_done
 ��������  :�ýӿ������ϲ����SOCP��������Ŀ��ͨ���ж��ߵ�ʵ�����ݡ�
 �������  : u32DestChanID:Ŀ��ͨ��ID
                  u32ReadSize: �Ѷ������ݵĳ���
 �������  : �ޡ�
 �� �� ֵ      : SOCP_OK:�����ݳɹ���
                             SOCP_ERROR:������ʧ��
*****************************************************************************/
s32 bsp_socp_read_data_done(u32 u32DestChanID,u32 u32ReadSize);

/*****************************************************************************
 �� �� ��      : bsp_socp_set_bbp_enable
 ��������  :ʹ�ܻ�ֹͣBBPͨ����
 �������  : bEnable:ͨ��ID
 �������  : �ޡ�
 �� �� ֵ      : SOCP_OK:���óɹ���
                   ����ֵ:����ʧ��
*****************************************************************************/
s32 bsp_socp_set_bbp_enable(int bEnable);

/*****************************************************************************
 �� �� ��      : bsp_socp_set_bbp_ds_mode
 ��������  :����BBP DSͨ�������������ģʽ��
 �������  : eDsMode:DSͨ���������ʱ����ģʽ����
 �������  : �ޡ�
 �� �� ֵ      : SOCP_OK:���óɹ���
                   ����ֵ:����ʧ��
*****************************************************************************/
s32 bsp_socp_set_bbp_ds_mode(SOCP_BBP_DS_MODE_ENUM_UIN32 eDsMode);

/*****************************************************************************
* �� �� ��  : bsp_socp_get_state
*
* ��������  : ��ȡSOCP״̬
*
* �� �� ֵ  : SOCP_IDLE    ����
*             SOCP_BUSY    æµ
*****************************************************************************/
SOCP_STATE_ENUM_UINT32 bsp_socp_get_state(void);

s32 socp_init(void);

/*****************************************************************************
* �� �� ��  : bsp_socp_vote
* ��������  : SOCPͶƱ�ӿڣ�����ͶƱ�������SOCP�Ƿ�˯�ߣ��ýӿ�ֻ��A���ṩ
* �������  : id --- ͶƱ���ID��type --- ͶƱ����
* �������  : ��
* �� �� ֵ  : BSP_S32 0 --- ͶƱ�ɹ���0xFFFFFFFF --- ͶƱʧ��
*****************************************************************************/
BSP_S32 bsp_socp_vote(SOCP_VOTE_ID_ENUM_U32 id, SOCP_VOTE_TYPE_ENUM_U32 type);

/*****************************************************************************
* �� �� ��  : bsp_socp_vote_to_mcore
* ��������  : SOCPͶƱ�ӿڣ��ýӿ�ֻ��C���ṩ������LDSP�״μ��ص�SOCP�ϵ�����
* �������  : type --- ͶƱ����
* �������  : ��
* �� �� ֵ  : BSP_S32 0 --- ͶƱ�ɹ���0xFFFFFFFF --- ͶƱʧ��
*****************************************************************************/
BSP_S32 bsp_socp_vote_to_mcore(SOCP_VOTE_TYPE_ENUM_U32 type);

/*****************************************************************************
* �� �� ��  : bsp_socp_get_log_cfg
* ��������  : ��ȡlog����
* �������  :
* �������  :
* �� �� ֵ  :
*****************************************************************************/
struct socp_enc_dst_log_cfg * bsp_socp_get_log_cfg(void);
/*****************************************************************************
* �� �� ��  : bsp_socp_get_sd_logcfg
* ��������  : ��ȡ����
* �������  :
* �������  :
* �� �� ֵ  :
*****************************************************************************/
u32 bsp_socp_get_sd_logcfg(SOCP_ENC_DST_BUF_LOG_CFG_STRU* cfg);
/*****************************************************************************
* �� �� ��  : socp_set_clk_autodiv_enable
* ��������  : ����clk�ӿ�clk_disable_unprepare��bypass��0�������Զ���Ƶ
* �������  : ��
* �������  : ��
* �� �� ֵ  : ��
* ע    ��  :
              clk_prepare_enable �ӿ��� clk_disable_unprepare �ӿڱ���ɶ�ʹ��
              clk���Զ���ƵĬ�ϴ��ڴ�״̬������
              �����Ƚ��� clk_prepare_enable ���ܽ��� clk_disable_unprepare ����
*****************************************************************************/
void bsp_socp_set_clk_autodiv_enable(void);

/*****************************************************************************
* �� �� ��  : socp_set_clk_autodiv_disable
* ��������  : ����clk�ӿ�clk_prepare_enable��bypass��1�������Զ���Ƶ
* �������  : ��
* �������  : ��
* �� �� ֵ  : ��
* ע    ��  :
              clk_prepare_enable �ӿ��� clk_disable_unprepare �ӿڱ���ɶ�ʹ��
              clk���Զ���ƵĬ�ϴ��ڴ�״̬������
              �����Ƚ��� clk_prepare_enable ���ܽ��� clk_disable_unprepare ����
*****************************************************************************/
void bsp_socp_set_clk_autodiv_disable(void);


#if (FEATURE_SOCP_DECODE_INT_TIMEOUT == FEATURE_ON)
/*****************************************************************************
* �� �� ��  : bsp_socp_set_decode_timeout_register
* ��������  :ѡ����볬ʱ�Ĵ���
* �������  : ��
* �������  : ��
* �� �� ֵ  :
*****************************************************************************/
s32 bsp_socp_set_decode_timeout_register(DECODE_TIMEOUT_MODULE module);
#endif
/*****************************************************************************
* �� �� ��  : bsp_socp_set_enc_dst_threshold
* ��������  :
* �������  :
* �������  :
* �� �� ֵ  :
*****************************************************************************/
void bsp_socp_set_enc_dst_threshold(bool mode,u32 u32DestChanID);

/*****************************************************************************
* �� �� ��  : bsp_socp_set_ind_mode
*
* ��������  : �����ϱ�ģʽ�ӿ�
*
* �������  : ģʽ����
*
* �������  : ��
*
* �� �� ֵ  : BSP_S32 BSP_OK:�ɹ� BSP_ERROR:ʧ��
*****************************************************************************/
s32 bsp_socp_set_ind_mode(SOCP_IND_MODE_ENUM eMode);


/*****************************************************************************
 �� �� ��  : mdrv_socp_modify_write_read_ptr
 ��������  : �ô˽ӿ����ڶ�дָ����Ե�ַ��Ϊ��Ե�ַ
 �������  : ulSrcChanId:Դͨ��ID
             
 �������  : �ޡ�
 �� �� ֵ  : ��
*****************************************************************************/
void bsp_socp_modify_write_read_ptr(u32 ulSrcChanId);


#ifdef CONFIG_DEFLATE
s32 bsp_socp_compress_enable(u32 u32DestChanID);

s32 bsp_socp_compress_disable(u32 u32DestChanID);

s32 bsp_socp_register_compress(socp_compress_ops_stru *ops);
#endif
#ifdef __cplusplus
}
#endif

#endif /* end of _BSP_SOCP_H*/


