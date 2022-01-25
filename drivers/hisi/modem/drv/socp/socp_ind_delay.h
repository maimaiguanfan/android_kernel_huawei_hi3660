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

#ifndef _SOCP_IND_DELAY_H
#define _SOCP_IND_DELAY_H
#include <product_config.h>

#include "osl_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define INDDELAY_NULL                        (void*)0
#define SOCP_MAX_MEM_SIZE                          50 *1024 *1024
#define SOCP_MIN_MEM_SIZE                          1 *1024 *1024
#define SOCP_MAX_TIMEOUT                           1200     /*MS*/
#define SOCP_MIN_TIMEOUT                           10       /*MS*/
#define SOCP_RESERVED_TRUE                          1
#define SOCP_RESERVED_FALSE                         0
typedef struct _socp_dst_early_cfg
{
    void*           pVirBuffer;      /* fastbootԤ����buffer����BUFFER����32λϵͳ����4�ֽڣ���64λϵͳ����8�ֽ� */
    unsigned long   ulPhyBufferAddr; /* fastbootԤ����buffer�����ַ */
    unsigned int    ulBufferSize;    /* fastbootԤ����buffer��С */
    unsigned int    ulTimeout;       /* fastbootָ�������ݴ��䳬ʱʱ��, ֻ����fastbootԤ���ڴ����ʱ����Ч */
    unsigned int    ulBufUsable;     /* fastbootԤ����buffer�Ƿ���õı�־ */
	unsigned int    ulLogCfg;        /* SOCP����Ŀ��ͨ������buffer�Ƿ��ں�Ԥ���ڴ淽ʽ*/
}socp_early_cfg_stru;

typedef struct _socp_mem_reserve_stru
{
    void*           pVirBuffer;      /* SOCP����Ŀ��ͨ����������BUFFER����32λϵͳ����4�ֽڣ���64λϵͳ����8�ֽ� */
    unsigned long   ulPhyBufferAddr; /* SOCP����Ŀ��ͨ����������BUFFER��ַ */
    unsigned int    ulBufferSize;    /* SOCP����Ŀ��ͨ������BUFFER��С */
    unsigned int    ulTimeout;       /* SOCP����Ŀ��ͨ�����ݴ��䳬ʱʱ�� */
    unsigned int    ulBufUsable;     /* Ԥ����kernel buffer�Ƿ���õı�־ */
}socp_mem_reserve_stru;



s32  bsp_socp_ind_delay_init(void);
s32  bsp_socp_dst_init(void);
u32 bsp_socp_read_cur_mode(u32 chanid);
#ifdef __cplusplus
}
#endif

#endif

