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

#ifndef __MDRV_COMMON_PM_H__
#define __MDRV_COMMON_PM_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "mdrv_om_common.h"

struct mdrv_pm_profile
{
	unsigned int max_profile;
	unsigned int min_profile;
};

typedef enum tagPWC_COMM_MODEM_E
{
    PWC_COMM_MODEM_0 = 0,
    PWC_COMM_MODEM_1 = 1,
    PWC_COMM_MODEM_2 = 2,
    PWC_COMM_MODEM_BUTT
}PWC_COMM_MODEM_E;


/******************************************************
 * *˯��ͶƱID ,��0��ʼ�����32��
 * *�漰ͶƱ�������Ҫ�ڴ����LOCK ID
 * *��ͬ���޸�wakelock_balong.c�е�debug_wakelock
 * *******************************************************/
typedef enum tagPWC_CLIENT_ID_E
{
    PWRCTRL_SLEEP_BEGIN=0X100,
    PWRCTRL_SLEEP_TLPHY = PWRCTRL_SLEEP_BEGIN,   /*MSP--fuxin*/
    PWRCTRL_SLEEP_PS_G0,        /*GU--ganlan*/
    PWRCTRL_SLEEP_PS_W0,       /*GU--ganlan*/
    PWRCTRL_SLEEP_PS_G1,        /*GU--ganlan*/
    PWRCTRL_SLEEP_PS_W1,       /*GU--ganlan*/
    PWRCTRL_SLEEP_FTM,           /*GU--zhangyizhan*/
    PWRCTRL_SLEEP_FTM_1,       /*GU--zhangyizhan*/
    PWRCTRL_SLEEP_NAS,           /*GU--zhangyizhan*/
    PWRCTRL_SLEEP_NAS_1,       /*GU--zhangyizhan*/
    PWRCTRL_SLEEP_OAM,          /* GU--zhangyizhan */
    PWRCTRL_SLEEP_SCI0,            /* LTE --yangzhi */
    PWRCTRL_SLEEP_SCI1,            /* LTE --yangzhi */
    PWRCTRL_SLEEP_TLPS,            /* lte ps */
    PWRCTRL_SLEEP_TLPS1,           /* lte ps1 */
    PWRCTRL_SLEEP_DSFLOW,          /* NAS --zhangyizhan */
    PWRCTRL_SLEEP_TEST,            /* PM  ---shangmianyou */
    PWRCTRL_SLEEP_UART0,        /*UART0 -zhangliangdong */
    PWRCTRL_SLEEP_TDS,         /*TRRC&TL2----leixiantiao*/

    PWRCTRL_SLEEP_CDMAUART,         /*drv cdma uart ����*/
    PWRCTRL_SLEEP_USIM,             /*oam*/
    PWRCTRL_SLEEP_DSPPOWERON,       /*v8r1 ccore �ṩ��GUTL DSP��Ϊc���ϵ��ʼ��ͶƱ��*/
    PWRCTRL_SLEEP_RESET,            /* RESET  ---nieluhua */
    PWRCTRL_SLEEP_PS_G2,        /*GU--ganlan*/
    PWRCTRL_SLEEP_FTM_2,       /*GU--zhangyizhan*/
    PWRCTRL_SLEEP_NAS_2,       /*GU--zhangyizhan*/
    PWRCTRL_SLEEP_1X,          /*CDMA--ganlan*/
    PWRCTRL_SLEEP_HRPD,        /*CDMA--ganlan;0x11A*/
    PWRCTRL_SLEEP_MSP,        /*cuijunqiang*/
    PWRCTRL_SLEEP_VOWIFI,        /*zhangdongfeng,xiamiaofang*/
    /*���²��ֵ�ID�Ѿ���ʹ���ˣ����ڻ�ɾ��*/
    PWRCTRL_SLEEP_RNIC,
    PWRCTRL_TEST_DEEPSLEEP  = 0x11f,
    PWRCTRL_SLEEP_END =0x120
}PWC_CLIENT_ID_E;

typedef enum tagPWC_COMM_MODE_E
{
    PWC_COMM_MODE_GSM=0,
    PWC_COMM_MODE_WCDMA=1,
    PWC_COMM_MODE_LTE=2,
    PWC_COMM_MODE_TDS=3,
    PWC_COMM_MODE_CDMA_1X=4,
    PWC_COMM_MODE_CDMA_HRPD=5,
    PWC_COMM_MODE_NUMBER,
    PWC_COMM_MODE_COMMON=PWC_COMM_MODE_NUMBER,
	PWC_COMM_MODE_REMOTE_CLK_W,
	PWC_COMM_MODE_REMOTE_CLK_G,
    PWC_COMM_MODE_BUTT
}PWC_COMM_MODE_E;
/*ע��:ö������ʱ����PWC_COMM_MODEM_BUTT*PWC_COMM_MODE_BUTT > 32ʱ��Ҫ֪���ͶƱ����޸�ͶƱ����
       ��Ҫ��PWC_COMM_MODE_CDMA_HRPD֮ǰ������������*/


/*
 * PM_OM_MOD_ID_ENUM - ʹ��pmom(pm dump/pm log/pm info)��ģ��ID
 */
enum PM_OM_MOD_ID_ENUM
{
    PM_MOD_BEGIN     =  32,
    PM_MOD_AP_OSA  =  OM_AP_OSA,
    PM_MOD_CP_OSA  =  OM_CP_OSA,
    PM_MOD_CP_MSP  =  OM_CP_MSP_SLEEP,
    PM_OM_CAS_1X     =  33,
    PM_OM_CPROC_1X   =  34,
    PM_OM_CAS_EVDO   =  35,
    PM_OM_CPROC_EVDO =  36,
    PM_OM_TLPHY      =  37,
    PM_OM_TRRC       =  38,
    PM_OM_LRRC       =  39,
    PM_OM_WPHY       =  40,
    PM_OM_GPHY       =  41,
    PM_MOD_END       =  43,
	/* �ϲ�ģ��ID�����Գ���48 */
    PM_OM_MOD_ID_ENUM_MAX = 48
};

/*
 * PM_OM_MAGIC_ENUM - ��PM_OM_MOD_ID_ENUM��Ӧ��ģ���ħ��
 * ���ĸ��ַ���ascii���ʶ, С���ֽ���
 */
enum PM_OM_MAGIC_ENUM
{
	PM_OM_MAGIC_AOSA = 0x41534F41, /*AOSA : OSA ACORE */
	PM_OM_MAGIC_COSA = 0x41534F43, /*COSA : OSA CCORE */
	PM_OM_MAGIC_CMSP = 0x50534D43, /*CMSP : MSP CCORE */
	PM_OM_MAGIC_CASX = 0x58534143, /*CASX : CAS 1X    */
	PM_OM_MAGIC_CPRX = 0x58525043, /*CPRX : CPROC 1X  */
	PM_OM_MAGIC_CASE = 0x45534143, /*CASE : CAS EVDO  */
	PM_OM_MAGIC_CPRE = 0x45525043, /*CPRE : CPROC EVDO*/
	PM_OM_MAGIC_TLPY = 0x48504C54, /*TLPH : TL PHY    */
	PM_OM_MAGIC_TRRC = 0x43525254, /*TRRC : T RRC     */
	PM_OM_MAGIC_LRRC = 0x4352524C, /*LRRC : L RRC     */
	PM_OM_MAGIC_WPHY = 0x59485057, /*WPHY : W PHY     */
	PM_OM_MAGIC_GPHY = 0x59485047  /*GPHY : G PHY     */
};

/*
 * struct pm_info_usr_data - ��¼����״̬��Ϣ�Ľṹ�嶨��
 * @mod_id	: ���ID, ��ӦPM_OM_MOD_ID_ENUM
 * @magic	: ���magic, ������ı�ʶ, ��ӦPM_OM_MAGIC_ENUM
 * @buf		: �����¼����״̬��Ϣ�ĵ��ڴ���ʼ��ַ
 * @buf_len	: �����¼����״̬��Ϣ�ĵ��ڴ��С
 * @private	: �û�˽��������
 */
struct pm_info_usr_data
{
	unsigned int mod_id;
	unsigned int magic;
	void         *buf;
	unsigned int buf_len;
	void         *context;
};

/* modem����״̬��Ϣ�ռ��ص�����ָ�� */
typedef int (*pm_info_cbfun)(struct pm_info_usr_data *usr_data);

/*****************************************************************************
* �� �� ��  : mdrv_pm_info_stat_register
*
* ��������  : ע��modem����״̬��Ϣ�ռ��ص��������û���Ϣ
*
* �������  : pcbfun   modem����״̬��Ϣ�ռ��ص�����, �����mdrv_pm_log�������ϱ�
*             usr_data �û�����, ��������NULLָ��
* �������  :
*
* �� �� ֵ  : 0��ִ�гɹ������㣬ʧ��
*
* ����˵��  :
*
*****************************************************************************/
int mdrv_pm_info_stat_register(pm_info_cbfun pcbfun, struct pm_info_usr_data *usr_data);

/*****************************************************************************
* �� �� ��  : mdrv_pm_log
*
* ��������  : ���������Ϣ��log��
*
* �������  : mod_id	ģ��id��ʹ��enum PM_OM_MOD_ID_ENUMö�����Ͷ����ֵ
*            data_len	������ݳ���
*            data 		�������
* �������  :
*
* �� �� ֵ  : 0��ִ�гɹ������㣬ʧ��
*
* ����˵��  : 
*
*****************************************************************************/
int mdrv_pm_log(int mod_id,  unsigned int data_len , void *data);

#ifdef __cplusplus
}
#endif
#endif
