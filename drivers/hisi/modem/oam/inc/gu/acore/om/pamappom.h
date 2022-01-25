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




#ifndef __PAMAPPOM_H__
#define __PAMAPPOM_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/****************************************************************************
  1 ����ͷ�ļ�����
*****************************************************************************/
#include "vos.h"
#include "msp_diag_comm.h"
#include "mdrv.h"
#include "omprivate.h"


/*****************************************************************************
  2 �궨��
******************************************************************************/
#define  OM_ICC_LOG_PATH                "/data/modemlog/Log/Icc-log"
#define  OM_ICC_UNITARY_LOG_PATH        "/modem_log/Log/Icc-log"

#define OM_ICC_BUFFER_SIZE              (16*1024)

#define OM_DRV_MAX_IO_COUNT             (8)        /*һ���ύ������ӿڵ������Ŀ*/

/* ��si_pih.h�е�ö�ٶ���SI_PIH_REQ_ENUM_UINT32����һ�� */
#define OM_SI_PIH_GACCESS_REQ             (3)
#define OM_SI_PIH_ISDB_ACCESS_REQ         (7)
#define OM_SI_PIH_CGLA_SET_REQ            (16)
#define OM_SI_PIH_URSM_REQ                (19)
#define OM_SI_PIH_CRSM_SET_REQ            (21)
#define OM_SI_PIH_CRLA_SET_REQ            (22)
#define OM_SI_PIH_PRIVATECGLA_SET_REQ     (32)

#define OM_LOG1(Mod, SubMod, Level, String, Para1) \
           (VOS_VOID)DIAG_LogReport( DIAG_GEN_LOG_MODULE(VOS_GetModemIDFromPid(Mod), DIAG_MODE_UMTS, (Level)), \
                           (Mod), __FILE__, __LINE__, "%s, %d \r\n", (String), (VOS_INT32)(Para1) )

#define OM_NORMAL_LOG1(string, para1)  OM_LOG1(ACPU_PID_PAM_OM, 0, PS_LOG_LEVEL_NORMAL, string, para1)

/*******************************************************************************
  3 ö�ٶ���
*****************************************************************************/


enum OM_DATA_DESTINATION_ENUM
{
    OMRL_UART = 0,              /*������*/
    OMRL_USB,                   /*USBͨ��*/
    OMRL_FS,                    /*д�ļ�ϵͳ*/
    OMRL_WIFI,                  /*SOCKET WIFIͨ��*/
    OMRL_SD,                    /*дSD��*/
    OMRL_PORT_BUTT
};
typedef VOS_UINT16 OM_DATA_DESTINATION_ENUM_UIN16;
typedef VOS_UINT32 OM_DATA_DESTINATION_ENUM_UIN32;

enum OM_USB_PORT_ENUM
{
    OMRL_USB_OM = 1,            /*OM����˿�*/
    OMRL_USB_AT,                /*AT����˿�*/
    OMRL_USB_SHELL,             /*shell����˿�*/
    OMRL_USB_CONTROL = 5,
    OMRL_USB_BUTT
};
typedef VOS_UINT16 OM_USB_PORT_ENUM_UINT16;
typedef VOS_UINT32 OM_USB_PORT_ENUM_UINT32;

enum OM_ICC_CHANNEL_ENUM
{
    OM_OM_ICC_CHANNEL           = 0,    /*��ǰͨ�����ڴ���OM����*/
    OM_OSA_MSG_ICC_CHANNEL,             /*��ǰͨ�����ڴ���OSA����Ϣ����*/
    OM_ICC_CHANNEL_BUTT
};
typedef VOS_UINT32 OM_ICC_CHANNEL_ENUM_UINT32;

typedef struct
{
   VOS_UINT16                       usSendPid;
   VOS_UINT16                       usRcvPid;
   VOS_UINT32                       ulMsgName;
   VOS_UINT32                       ulSliceStart;
   VOS_UINT32                       ulSliceEnd;
}OM_RECORD_INFO_STRU;

typedef struct
{
   VOS_UINT8                       *pucBuf;
   VOS_UINT32                       ulLen;
   VOS_UINT8                        aucRsv[4];
}OM_RECORD_BUF_STRU;

typedef struct
{
    VOS_MSG_HEADER
}OM_FILTER_MSG_HEAD_STRU;

/*****************************************************************************
  4 STRUCT����
*****************************************************************************/


/*****************************************************************************
  5 �ӿڶ���
*****************************************************************************/
extern VOS_UINT V_ICC_OSAMsg_CB(VOS_UINT ulChannelID,VOS_INT lLen);

/* AT<->AT�����δ����Ƶ�GuNasLogFilter.c */

#if (VOS_WIN32 == VOS_OS_VER)
#define PS_LOG(ModulePID, SubMod, Level, String)\
        vos_printf("\r\n%s",   String)

#define PS_LOG1(ModulePID, SubMod, Level, String, Para1)\
        vos_printf("\r\n%s,para1=%d", String, Para1)

#define PS_LOG2(ModulePID, SubMod, Level, String, Para1, Para2)\
        vos_printf("\r\n%s,para1=%d,para2=%d", String, Para1, Para2)

#define PS_LOG3(ModulePID, SubMod, Level, String, Para1, Para2, Para3)\
        vos_printf("\r\n%s,para1=%d,para2=%d,para3=%d", String, Para1, Para3)

#define PS_LOG4(ModulePID, SubMod, Level, String, Para1, Para2, Para3, Para4)\
        vos_printf("\r\n%s,para1=%d,para2=%d,para3=%d,para4=%d", String, Para1, Para3, Para4)

#else
#define PS_LOG(ModulePID, SubMod, Level, String)

#define PS_LOG1(ModulePID, SubMod, Level, String, Para1)

#define PS_LOG2(ModulePID, SubMod, Level, String, Para1, Para2)

#define PS_LOG3(ModulePID, SubMod, Level, String, Para1, Para2, Para3)

#define PS_LOG4(ModulePID, SubMod, Level, String, Para1, Para2, Para3, Para4)

#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __PAMAPPOM_H__ */
