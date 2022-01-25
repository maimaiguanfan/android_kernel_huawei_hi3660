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

#include <product_config.h>
#include "bsp_dump_mem.h"
#include "dump_modem_field.h"
#include "dump_modem_area.h"
#include "dump_print.h"

struct dump_field_ctrl_info_s   g_st_field_ctrl;
u32    g_dump_mem_init = 0;

/*****************************************************************************
* �� �� ��  : dump_get_area_info
* ��������  : ��֤field_id�Ƿ�Ϸ����쳣id����BSP_ERROR������id����area id
*
* �������  :
* �������  :

* �� �� ֵ  :

*
* �޸ļ�¼  : 2016��1��4��17:05:33   lixiaofan  creat
*
*****************************************************************************/
 u32 dump_get_areaid_by_fieldid(u32 field_id)
{
    return DUMP_AREA_CP;
}
/*****************************************************************************
* �� �� ��  : dump_get_cp_save_done
* ��������  : �ж�c��log�Ƿ񱣴����
*
* �������  :
* �������  :

* �� �� ֵ  :

*
* �޸ļ�¼  : 2016��1��4��17:05:33   lixiaofan  creat
*
*****************************************************************************/
s32 dump_get_cp_save_done(void)
{
    return BSP_OK;
}

/*****************************************************************************
* �� �� ��  : bsp_dump_get_field_addr
* ��������  : ��ȡfield�ĵ�ַ
*
* �������  :field_id
* �������  :

* �� �� ֵ  :

*
* �޸ļ�¼  : 2016��1��4��17:05:33   lixiaofan  creat
*
*****************************************************************************/
u8 * bsp_dump_get_field_addr(u32 field_id)
{

    return NULL;
}
/*****************************************************************************
* �� �� ��  : bsp_dump_get_field_map
* ��������  : ��ȡfield��field��Ϣ
*
* �������  :field_id
* �������  :

* �� �� ֵ  :

*
* �޸ļ�¼  : 2016��1��4��17:05:33   lixiaofan  creat
*
*****************************************************************************/
u8 * bsp_dump_get_field_map(u32 field_id)
{

    return NULL;
}

/*****************************************************************************
* �� �� ��  : bsp_dump_register_field
* ��������  : ����field��Ϣ
            1. ������ַע�ᣬ�������ʱvirt_addr,phy_addr���봫0���ɹ�����dumpע���ַ
            2. �Դ���ַע�ᣬ�������ʱphy_addrΪ�Դ������ַ��virt_addrΪ�����ַ��ͬʱ��dump�ڴ��з�����ͬ��С�ڴ棬�ɹ�������virt_addr

            3. ����ע�᷽ʽ��������dump�����ڴ棬�����Դ���ַ��ע�᷽ʽ����ϵͳ�쳣ʱ����dumpģ�������ݿ���
            4. ÿ��ע��������Ҫ��ʹ���ߴ����Ӧ�İ汾�ţ���8λΪ���汾�ţ���8λΪ�ΰ汾��
* �������  :field_id
* �������  :

* �� �� ֵ  :

*
* �޸ļ�¼  : 2016��1��4��17:05:33   lixiaofan  creat
*
*****************************************************************************/
u8 * bsp_dump_register_field(u32 field_id, char * name, void * virt_addr, void * phy_addr, u32 length, u16 version)
{

    return NULL;
}

/*****************************************************************************
* �� �� ��  : bsp_dump_save_self_addr
* ��������  : ������ע��Ŀռ�
*
* �������  :field_id
* �������  :

* �� �� ֵ  :

*
* �޸ļ�¼  : 2016��1��4��17:05:33   lixiaofan  creat
*
*****************************************************************************/
void bsp_dump_save_self_addr(void)
{


    return ;
}

/*****************************************************************************
* �� �� ��  : bsp_dump_field_init
* ��������  : cp dump fileld��ʼ��
*
* �������  :field_id
* �������  :

* �� �� ֵ  :

*
* �޸ļ�¼  : 2016��1��4��17:05:33   lixiaofan  creat
*
*****************************************************************************/
s32 bsp_dump_field_init(void)
{

    return BSP_OK;
}

/*****************************************************************************
* �� �� ��  : bsp_dump_get_field_phy_addr
* ��������  : ��ȡfield�������ַ
*
* �������  :field_id
* �������  :

* �� �� ֵ  :

*
* �޸ļ�¼  : 2016��1��4��17:05:33   lixiaofan  creat
*
*****************************************************************************/
u8 * bsp_dump_get_field_phy_addr(u32 field_id)
{

    return NULL;
}



/*****************************************************************************
* �� �� ��  : modem_dump_field_init
* ��������  : ��ʼ��modem ap��Ҫʹ��filed �ռ�
*
* �������  :
* �������  :

* �� �� ֵ  :

*
* �޸ļ�¼  : 2016��1��4��17:05:33   lixiaofan  creat
*
*****************************************************************************/

void dump_mdmap_field_init(void)
{

}

/*****************************************************************************
* �� �� ��  : bsp_dump_mem_init
* ��������  : modem ap ��ά�ɲ�ռ��ʼ��
*
* �������  :
* �������  :

* �� �� ֵ  :

*
* �޸ļ�¼  : 2016��1��4��17:05:33   lixiaofan  creat
*
*****************************************************************************/
s32 bsp_dump_mem_init(void)
{
    if(g_dump_mem_init == 1)
    {
        return BSP_OK;
    }
    if(dump_area_init())
    {
        return BSP_ERROR;
    }

    if(bsp_dump_field_init())
    {
        return BSP_ERROR;
    }

    dump_fetal("bsp_dump_mem_init finish\n");
    g_dump_mem_init = 1;

    return BSP_OK;
}

/*****************************************************************************
* �� �� ��  : dump_show_field
* ��������  : ���Խӿڣ���ʾ��ǰ�Ѿ�ע��fieldʹ�����
*
* �������  :
* �������  :

* �� �� ֵ  :

*
* �޸ļ�¼  : 2016��1��4��17:05:33   lixiaofan  creat
*
*****************************************************************************/
void dump_show_field(void)
{

}

arch_initcall(bsp_dump_mem_init);

