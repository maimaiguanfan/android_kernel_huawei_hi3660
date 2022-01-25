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

#ifndef __MDRV_ACORE_WIFI_H__
#define __MDRV_ACORE_WIFI_H__

/*************************WIFI START**********************************/


extern unsigned short BSP_WifiCalDataLen(unsigned short usLen);
#define DRV_WifiCalDataLen(usLen) BSP_WifiCalDataLen(usLen)

/*****************************************************************************
 �� �� ��  : WIFI_TEST_CMD
 ��������  : ��������
 �������  : cmdStr�������ַ���
 �������  : ��
 ����ֵ    ����
*****************************************************************************/
extern void WIFI_TEST_CMD(char * cmdStr);

/*****************************************************************************
 �� �� ��  : WIFI_GET_TCMD_MODE
 ��������  : ��ȡ���������ģʽ
 �������  : ��
 �������  : 16��У׼����ģʽ
             17����ʾ���ڷ���ģʽ
             18����ʾ����ģʽ
 ����ֵ    ������ִ�е�״̬���ֵ
*****************************************************************************/
extern int WIFI_GET_TCMD_MODE(void);

/*****************************************************************************
 �� �� ��  : WIFI_POWER_START
 ��������  : WIFI�ϵ�
 �������  : ��
 �������  : ��
 ����ֵ    �� 0: execute ok
              1: execute failed
*****************************************************************************/
extern int WIFI_POWER_START(void);

/*****************************************************************************
 �� �� ��  : WIFI_POWER_SHUTDOWN
 ��������  : WIFI�µ�
 �������  : ��
 �������  : ��
 ����ֵ    �� 0: execute ok
              1: execute failed
*****************************************************************************/
extern int WIFI_POWER_SHUTDOWN(void);

/*****************************************************************************
 �� �� ��  : WIFI_GET_STATUS
 ��������  : WIFI״̬��ȡ
 �������  : ��
 �������  : ��
 ����ֵ    �� 0: wifi is off
              1: wifi is in normal mode
              2: wifi is in tcmd mode
*****************************************************************************/
extern int WIFI_GET_STATUS(void);

/*****************************************************************************
 �� �� ��  : WIFI_GET_RX_DETAIL_REPORT
 ��������  : get result of rx report: totalPkt, GoodPkt, ErrorPkt
 �������  : ��
 �������  : totalPkt��goodPkt��badPkt
 ����ֵ    ����
*****************************************************************************/
extern void WIFI_GET_RX_DETAIL_REPORT(int* totalPkt,int* goodPkt,int* badPkt);

/*****************************************************************************
 �� �� ��  : WIFI_GET_RX_PACKET_REPORT
 ��������  : get result of rx ucast&mcast packets
 �������  : ��
 �������  : ucastPkts��mcastPkts
 ����ֵ    ����
*****************************************************************************/
extern void  WIFI_GET_RX_PACKET_REPORT(unsigned int *ucastPkts, unsigned int *mcastPkts);

/*****************************************************************************
 �� �� ��  : WIFI_GET_PA_CUR_MODE
 ��������  : get the currrent PA mode of the wifi chip
 �������  : ��
 �������  : ��
 ����ֵ    ��0:  (WIFI_ONLY_PA_MODE) WIFI chip is in PA mode
             1:  (WIFI_ONLY_NOPA_MODE) WIFI chip is in no PA mode
             -1: wifi chip is in abnormal mode
*****************************************************************************/
extern int WIFI_GET_PA_CUR_MODE(void);

/*****************************************************************************
 �� �� ��  : WIFI_GET_PA_MODE
 ��������  : get the support PA mode of wifi chip
 �������  : ��
 �������  : ��
 ����ֵ    ��0:  (WIFI_ONLY_PA_MODE) WIFI suppport only PA mode
             1:  (WIFI_ONLY_NOPA_MODE) WIFI suppport only no PA mode
             2:  (WIFI_PA_NOPA_MODE) WIFI suppport both PA &  no PA mode
             -1: failed
*****************************************************************************/
extern int WIFI_GET_PA_MODE(void);

/*****************************************************************************
 �� �� ��  : WIFI_SET_PA_MODE
 ��������  : set the PA mode of wifi chip
 �������  : 0:  (WIFI_ONLY_PA_MODE) WIFI suppport only PA mode
             1:  (WIFI_ONLY_NOPA_MODE) WIFI suppport only no PA mode
             2:  (WIFI_PA_NOPA_MODE) WIFI suppport both PA &  no PA mode
 �������  : ��
 ����ֵ    ��0: success
             -1: failed
*****************************************************************************/
extern int WIFI_SET_PA_MODE(int wifiPaMode);

/*****************************************************************************
 �� �� ��  : DRV_WIFI_DATA_RESERVED_TAIL
 ��������  : WIFI������Ҫ���ݿ��ڴ�
 �������  : usLen - �û��������ݳ���Len
 �������  : ��
 �� �� ֵ  : ��������β��Ԥ���ĳ���
*****************************************************************************/
extern unsigned int DRV_WIFI_DATA_RESERVED_TAIL(unsigned int len);

/*****************************************************************************
 �� �� ��  : DRV_WIFI_SET_RX_FCTL
 ��������  : ����WIFI�������ر�ʶ
 �������  : para1��para2
 �������  : ��
 �� �� ֵ  : BSP_OK/BSP_ERROR
*****************************************************************************/
extern unsigned int DRV_WIFI_SET_RX_FCTL(unsigned int para1, unsigned int para2);

/*****************************************************************************
 �� �� ��  : DRV_WIFI_CLR_RX_FCTL
 ��������  : ���WIFI�������ر�ʶ
 �������  : para1��para2
 �������  : ��
 �� �� ֵ  : 1 : ��
             0 : ��
*****************************************************************************/
extern unsigned int DRV_WIFI_CLR_RX_FCTL(unsigned int para1, unsigned int para2);

/*****************************************************************************
 �� �� ��  : DRV_AT_GET_USER_EXIST_FLAG
 ��������  : ���ص�ǰ�Ƿ���USB���ӻ���WIFI�û�����(C�˵���)
 �������  : ��
 �������  : ��
 �� �� ֵ  : 1 : ��
             0 : ��
*****************************************************************************/
extern unsigned int  DRV_AT_GET_USER_EXIST_FLAG(void);


/*************************WIFI END************************************/

#endif

