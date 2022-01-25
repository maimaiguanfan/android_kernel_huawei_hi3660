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


#include <linux/sched.h>
#include <linux/timer.h>
#include <linux/thread_info.h>
#include <linux/syslog.h>
#include <linux/errno.h>
#include <linux/kthread.h>
#include <linux/semaphore.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/notifier.h>
#include <linux/kdebug.h>
#include <linux/reboot.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/delay.h>
#include <linux/wakelock.h>
#include <asm/string.h>
#include <asm/traps.h>
#include "product_config.h"
#include <linux/syscalls.h>
#include "osl_types.h"
#include "osl_io.h"
#include "osl_bio.h"
#include "osl_malloc.h"
#include "bsp_dump.h"
#include "bsp_nvim.h"
#include "bsp_dump_mem.h"
#include "drv_nv_def.h"
#include "mdrv_om.h"
#include "drv_comm.h"
#include <gunas_errno.h>
#include "dump_modem_area.h"
#include "dump_apr.h"
#include "dump_print.h"
#include "dump_file.h"
#include "dump_config.h"
#include "dump_exc_ctrl.h"



/*�洢cp�ж���Ϣ �жϺ� �������*/
interupt_table_t *g_cp_interupt_table = NULL;

/*�洢cp���� ���������Ϣ*/
task_table_t *g_cp_task_table = NULL;
/*�洢modid��Χ ���������Ϣ*/
modid_info_t *g_p_modid_table = NULL;

/*****************************************************************************
* �� �� ��  : bsp_dump_parse_apr_dts_info
*
* ��������  : ��ȡdts��ʼ��������Ϣ���ж���Ϣ �����Ϣ
*
* �������  :
*
*
* �������  :��
*
* �� �� ֵ  : ��
*****************************************************************************/
s32 dump_apr_init(void)
{

    return BSP_OK;
}
/*****************************************************************************
* �� �� ��  : bsp_dump_parse_reset_info
*
* ��������  : ���쳣��Ϣ����
*
* �������  :  reset_info:�洢��������쳣��Ϣ��������
               size
*
*
* �������  :��
*
* �� �� ֵ  : ��
*****************************************************************************/
void dump_apr_parse_reset_info( dump_reset_log_t *dump_reset_info, dump_except_info_t dump_except_info)
{

}
/*****************************************************************************
* �� �� ��  : bsp_dump_get_reset_module
*
* ��������  : ������λ���������
*
* �������  :
*
*
* �������  :��
*
* �� �� ֵ  : ��
*****************************************************************************/
void dump_apr_get_reset_module(dump_except_info_t dump_except_info, u8* task_name,u8 * module)
{

}
/*****************************************************************************
* �� �� ��  : bsp_dump_get_cp_reset_reason
*
* ��������  : ��ȡCP��λ��ԭ��
*
* �������  :
*
*
* �������  :��
*
* �� �� ֵ  : ��
*****************************************************************************/
void dump_apr_get_reset_task(dump_except_info_t dump_except_info,  u8 * task_name, u32 * reboot_int)
{

}

/*****************************************************************************
* �� �� ��  : bsp_dump_search_taskid
*
* ��������  : ���Ҹ�λ��task id
*
* �������  :
*
*
* �������  :��
*
* �� �� ֵ  : ��
*****************************************************************************/
s32 dump_apr_get_reset_taskid(dump_queue_t *Q)
{

    return BSP_ERROR;
}

/*****************************************************************************
* �� �� ��  : bsp_dump_get_reset_context_and_taskid
*
* ��������  :��ȡ��λΪ�жϸ�λ��������λ,���Ϊ����λ,������λ����id�������λΪ�жϣ�������λ�жϺ�
*
* �������  :
*
*
* �������  :��
*
* �� �� ֵ  : ��
*****************************************************************************/
void dump_apr_get_reset_context_and_id(u32 *reboot_context, u32 *reboot_task,u32 *reboot_int, struct dump_global_struct_s* dump_head)
{


}
/*****************************************************************************
* �� �� ��  : bsp_dump_get_reset_voice
*
* ��������  : ������λʱ�Ƿ���������
*
* �������  :
*
*
* �������  :��
*
* �� �� ֵ  : ��
*****************************************************************************/
void dump_apr_get_reset_voice(u32 voice, u8 * reboot_voice)
{

}
/*****************************************************************************
* �� �� ��  : bsp_dump_get_reset_modid
*
* ��������  : ��ȡ��λ��modid
*
* �������  :
*
*
* �������  :��
*
* �� �� ֵ  : ��
*****************************************************************************/
void dump_apr_get_reset_modid(u32 reason, u32 reboot_modid, u32 * modId)
{

}
/*****************************************************************************
* �� �� ��  : bsp_dump_get_cp_reset_reason
*
* ��������  : ��ȡ��λ��ԭ��
*
* �������  :
*
*
* �������  :��
*
* �� �� ֵ  : ��
*****************************************************************************/
void dump_apr_get_reset_reason(u32 reason, u8 * reboot_reason)
{

}
/*****************************************************************************
* �� �� ��  : bsp_dump_get_reset_ccore
*
* ��������  : ��ȡCP��λ��ԭ��
*
* �������  :
*
*
* �������  :��
*
* �� �� ֵ  : ��
*****************************************************************************/
void dump_apr_get_reset_ccore(u32 core, char *reboot_core)
{

}


/*****************************************************************************
* �� �� ��  : bsp_dump_show_apr_cfg
*
* ��������  : ��ʾapr������
*
* �������  :
*
*
* �������  :��
*
* �� �� ֵ  : ��
*****************************************************************************/
s32  dump_apr_show_cfg(u32 type)
{

    return BSP_OK;
}
/*****************************************************************************
* �� �� ��  : om_get_reset_info
*
* ��������  : ��ȡ��λ��Ϣ
*
* �������  :
*
*
* �������  :��
*
* �� �� ֵ  : ��
*****************************************************************************/
void dump_apr_get_reset_info(char * reset_info, u32 size)
{

}
/*****************************************************************************
* �� �� ��  : bsp_apr_save_data
*
* ��������  : ����apr����
*
* �������  :
*
*
* �������  :��
*
* �� �� ֵ  : ��
*****************************************************************************/
void dump_save_apr_data(char* dir_name)
{

}

