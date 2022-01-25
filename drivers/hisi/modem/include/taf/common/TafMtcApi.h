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
#ifndef _TAF_MTC_API_H_
#define _TAF_MTC_API_H_


/*****************************************************************************
  1 ����ͷ�ļ�����
*****************************************************************************/
#include "vos.h"
#include "TafTypeDef.h"
#include "PsTypeDef.h"
#include "TafApi.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


#pragma pack(4)

/*****************************************************************************
  2 �궨��
*****************************************************************************/
#define TAF_MTC_MSG_ID_BASE             (0x1000)                               /* MTC �ṩ����ϢID����, ��0x1001��ʼ����ҪΪ�˱�����������Ϣ�غ� */

#define MTC_NONE_CS_VALUE                (0x00)                                /* �޵绰 */
#define MTC_GU_CS_VALUE                  (MTC_SET_BIT(MTC_CS_TYPE_GU))          /* GU�绰 */
#define MTC_IMS_CS_VALUE                 (MTC_SET_BIT(MTC_CS_TYPE_IMS))         /* IMS�绰 */
#define MTC_CDMA_CS_VALUE                (MTC_SET_BIT(MTC_CS_TYPE_CDMA))        /* CDMA�绰 */

#define MTC_CS_TYPE_ALL_VALUE            (MTC_GU_CS_VALUE | MTC_IMS_CS_VALUE | MTC_CDMA_CS_VALUE)

#define MTC_NONE_PS_VALUE                (0x00)
#define MTC_GU_PS_VALUE                  (MTC_SET_BIT(MTC_PS_TYPE_GU))
#define MTC_LTE_PS_VALUE                 (MTC_SET_BIT(MTC_PS_TYPE_LTE))
#define MTC_CDMA_PS_VALUE                (MTC_SET_BIT(MTC_PS_TYPE_CDMA))
#define MTC_PS_TYPE_ALL_VALUE            (MTC_GU_PS_VALUE | MTC_LTE_PS_VALUE | MTC_CDMA_PS_VALUE)

/*****************************************************************************
  3 ö�ٶ���
*****************************************************************************/


enum TAF_MTC_MSG_ID_ENUM
{
    /* ���CDMA����״̬ */
    ID_MSG_MTC_CDMA_CONN_STATE_IND      = TAF_MTC_MSG_ID_BASE + 0x0001,        /* _H2ASN_MsgChoice  TAF_MTC_CDMA_STATE_IND_STRU */
    ID_MSG_MTC_MODEM_SERVICE_CONN_STATE_IND,                                    /* _H2ASN_MsgChoice  TAF_MTC_MODEM_CONN_STATUS_IND_STRU */
    ID_MSG_MTC_USIMM_STATE_IND,                                                 /* _H2ASN_MsgChoice  TAF_MTC_USIMM_STATUS_IND_STRU */
    ID_MSG_MTC_BEGIN_SESSION_IND,                                               /* _H2ASN_MsgChoice  MTC_BEGIN_SESSION_IND_STRU */
    ID_MSG_MTC_END_SESSION_IND,                                                 /* _H2ASN_MsgChoice  MTC_END_SESSION_IND_STRU */

    ID_MSG_MTC_POWER_SAVE_IND,                                                  /* _H2ASN_MsgChoice  MTC_POWER_SAVE_IND_STRU */

    ID_MSG_MTC_RAT_MODE_IND,                                                    /* _H2ASN_MsgChoice  MTC_RAT_MODE_IND_STRU */

    ID_MSG_MTC_BUTT
};
typedef VOS_UINT32 TAF_MTC_MSG_ID_ENUM_UINT32;


enum TAF_MTC_SRV_CONN_STATE_ENUM
{
    TAF_MTC_SRV_NO_EXIST                = 0,                                    /* ������ */
    TAF_MTC_SRV_EXIST,                                                          /* ������ */

    TAF_MTC_SRV_CONN_STATE_BUTT
};
typedef VOS_UINT8 TAF_MTC_SRV_CONN_STATE_ENUM_UINT8;


enum TAF_MTC_POWER_STATE_ENUM
{
    TAF_MTC_POWER_OFF                  = 0,                                     /* �ػ� */
    TAF_MTC_POWER_ON,                                                           /* ���� */

    TAF_MTC_POWER_STATE_BUTT
};
typedef VOS_UINT8 TAF_MTC_POWER_STATE_ENUM_UINT8;



enum TAF_MTC_USIMM_CARD_SERVIC_ENUM
{
    TAF_MTC_USIMM_CARD_SERVIC_ABSENT        =0,                                 /* �޿� */
    TAF_MTC_USIMM_CARD_SERVIC_UNAVAILABLE   =1,                                 /* �п�,���񲻿��� */
    TAF_MTC_USIMM_CARD_SERVIC_SIM_PIN       =2,                                 /* SIM����������PIN��ԭ�򲻿��� */
    TAF_MTC_USIMM_CARD_SERVIC_SIM_PUK       =3,                                 /* SIM����������PUK��ԭ�򲻿��� */
    TAF_MTC_USIMM_CARD_SERVIC_NET_LCOK      =4,                                 /* SIM������������������ԭ�򲻿��� */
    TAF_MTC_USIMM_CARD_SERVIC_IMSI_LCOK     =5,                                 /* SIM����������IMSI����ԭ�򲻿��� */
    TAF_MTC_USIMM_CARD_SERVIC_AVAILABLE     =6,                                 /* ������� */

    TAF_MTC_USIMM_CARD_SERVIC_BUTT
};
typedef VOS_UINT16      TAF_MTC_USIMM_CARD_SERVIC_ENUM_UINT16;



enum TAF_MTC_CDMA_USIMM_CARD_ENUM
{
    TAF_MTC_CDMA_USIMM_CARD_UNVALID        =0,                                  /* �޿� */
    TAF_MTC_CDMA_USIMM_CARD_VALID,                                              /* ������� */

    TAF_MTC_CDMA_USIMM_CARD_BUTT
};
typedef VOS_UINT8  TAF_MTC_CDMA_USIMM_CARD_ENUM_UINT8;


enum MTC_SESSION_TYPE_ENUM
{
    MTC_SESSION_TYPE_CS_MO_NORMAL_CALL                      = 0,
    MTC_SESSION_TYPE_CS_MO_EMERGENCY_CALL                   = 1,
    MTC_SESSION_TYPE_CS_MO_SS                               = 2,
    MTC_SESSION_TYPE_CS_MO_SMS                              = 3,
    MTC_SESSION_TYPE_CS_MT_NORMAL_CALL                      = 4,
    MTC_SESSION_TYPE_CS_MT_EMERGENCY_CALLBACK               = 5,                /* ������eCall�Ļغ����� */
    MTC_SESSION_TYPE_CS_MT_SS                               = 6,
    MTC_SESSION_TYPE_CS_MT_SMS                              = 7,
    MTC_SESSION_TYPE_CS_LAU                                 = 8,
    MTC_SESSION_TYPE_CS_DETACH                              = 9,
    MTC_SESSION_TYPE_CS_MO_NORMAL_CSFB                      = 10,
    MTC_SESSION_TYPE_CS_MO_EMERGENCY_CSFB                   = 11,
    MTC_SESSION_TYPE_CS_MT_CSFB                             = 12,
    MTC_SESSION_TYPE_CS_LOOP_BACK                           = 13,               /* ����ģʽ */

    MTC_SESSION_TYPE_TAU_COMBINED                           = 14,               /* ����TAU */

    MTC_SESSION_TYPE_TAU_ONLY_EPS                           = 15,               /* PS ONLY TAU */
    MTC_SESSION_TYPE_TAU_PERIODIC                           = 16,               /* ������TAU */
    
    MTC_SESSION_TYPE_COMBINED_ATTACH                        = 17,
    MTC_SESSION_TYPE_COMBINED_RAU                           = 18,

    MTC_SESSION_TYPE_CS_BUTT                                = 19,               /* ���ö��֮ǰΪCS������� */

    MTC_SESSION_TYPE_PS_CONVERSAT_CALL                      = 20,
    MTC_SESSION_TYPE_PS_STREAM_CALL                         = 21,
    MTC_SESSION_TYPE_PS_INTERACT_CALL                       = 22,
    MTC_SESSION_TYPE_PS_BACKGROUND_CALL                     = 23,
    MTC_SESSION_TYPE_PS_SUBSCRIB_TRAFFIC_CALL               = 24,
    MTC_SESSION_TYPE_PS_MO_SMS                              = 25,
    MTC_SESSION_TYPE_PS_MT_SMS                              = 26,
    MTC_SESSION_TYPE_PS_ATTACH                              = 27,
    MTC_SESSION_TYPE_PS_RAU                                 = 28,
    MTC_SESSION_TYPE_PS_DETACH                              = 29,
    MTC_SESSION_TYPE_1X_PS_CALL                             = 30,               /* 1X�ϱ���PSҵ�� */
    MTC_SESSION_TYPE_BUTT                                   = 31                /* ��MTC_SESSION_TYPE_CS_BUTT�����ö��ΪPS������� */
};
typedef VOS_UINT8 MTC_SESSION_TYPE_ENUM_UINT8;                                  /* ���ö��������ʱ��ע�ⲻҪ����32���������¼��Խ�� */


enum MTC_CS_TYPE_ENUM
{
    MTC_CS_TYPE_GU                      = 0x00,                                 /* GU�绰 */
    MTC_CS_TYPE_IMS,                                                            /* IMS�绰 */
    MTC_CS_TYPE_CDMA,                                                           /* CDMA�绰 */

    MTC_CS_TYPE_BUTT
};
typedef VOS_UINT8 MTC_CS_TYPE_ENUM_UINT8;


enum MTC_PS_TYPE_ENUM
{
    MTC_PS_TYPE_GU                      = 0x00,                                /* PS */
    MTC_PS_TYPE_LTE,                                                           /* EPS */
    MTC_PS_TYPE_CDMA,                                                          /* CDMA */

    MTC_PS_TYPE_BUTT
};
typedef VOS_UINT8 MTC_PS_TYPE_ENUM_UINT8;


enum TAF_MTC_POWER_SAVE_ENUM
{
    TAF_MTC_POWER_SAVE                  = 0,                                    /* ����power save */
    TAF_MTC_POWER_SAVE_EXIT,                                                    /* �˳�power save */

    TAF_MTC_POWER_SAVE_BUTT
};
typedef VOS_UINT8 TAF_MTC_POWER_SAVE_ENUM_UINT8;


enum MTC_RATMODE_ENUM
{
    MTC_RATMODE_GSM                     = 0x00,
    MTC_RATMODE_WCDMA                   = 0x01,
    MTC_RATMODE_LTE                     = 0x02,
    MTC_RATMODE_TDS                     = 0x03,
    MTC_RATMODE_1X                      = 0x04,
    MTC_RATMODE_HRPD                    = 0x05,
    MTC_RATMODE_BUTT
};
typedef VOS_UINT8 MTC_RATMODE_ENUM_UINT8;

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

typedef struct
{
    TAF_MTC_SRV_CONN_STATE_ENUM_UINT8       enCsConnSt;                         /* CS����״̬ */
    TAF_MTC_SRV_CONN_STATE_ENUM_UINT8       enPsConnSt;                         /* PS����״̬ */
    TAF_MTC_POWER_STATE_ENUM_UINT8          enPowerState;                       /* ���ػ�״̬ */
    VOS_UINT8                               ucReserved1[1];
    TAF_MTC_USIMM_CARD_SERVIC_ENUM_UINT16   enUsimmState;                       /* ��״̬ */
    VOS_UINT8                               aucReserved2[2];
}TAF_MTC_CDMA_STATE_INFO_STRU;


typedef struct
{
    TAF_CTRL_STRU                       stCtrl;
    TAF_MTC_CDMA_STATE_INFO_STRU        stCdmaState;
}TAF_MTC_CDMA_STATE_IND_STRU;


typedef struct
{
    VOS_UINT32                          bitOpPsSrv      : 1;
    VOS_UINT32                          bitOpCsSrv      : 1;
    VOS_UINT32                          bitOpEpsSrv     : 1;
    VOS_UINT32                          bitSpare        : 29;
    TAF_MTC_SRV_CONN_STATE_ENUM_UINT8   enPsSrvConnState;
    TAF_MTC_SRV_CONN_STATE_ENUM_UINT8   enCsSrvConnState;
    TAF_MTC_SRV_CONN_STATE_ENUM_UINT8   enEpsSrvConnState;
    VOS_UINT8                           aucReserved[1];
}TAF_MTC_SRV_CONN_STATE_INFO_STRU;


typedef struct
{
    TAF_CTRL_STRU                       stCtrl;
    TAF_MTC_SRV_CONN_STATE_INFO_STRU    stModemConnStateInfo;
}TAF_MTC_MODEM_CONN_STATUS_IND_STRU;


typedef struct
{
    TAF_CTRL_STRU                           stCtrl;
    TAF_MTC_USIMM_CARD_SERVIC_ENUM_UINT16   enUsimState;       /* Usim��״̬ */
    TAF_MTC_USIMM_CARD_SERVIC_ENUM_UINT16   enCsimState;       /* Csim��״̬ */
}TAF_MTC_USIMM_STATUS_IND_STRU;


typedef struct
{
    TAF_CTRL_STRU                       stCtrl;
    MTC_RATMODE_ENUM_UINT8              enRatMode;
    VOS_UINT8                           aucReserved[3];
}MTC_RAT_MODE_IND_STRU;




typedef struct
{
    TAF_CTRL_STRU                       stCtrl;
    MTC_SESSION_TYPE_ENUM_UINT8         enSessionType;
    VOS_UINT8                           aucReserved[3];
}MTC_BEGIN_SESSION_IND_STRU;


typedef struct
{
    TAF_CTRL_STRU                       stCtrl;
    PS_BOOL_ENUM_UINT8                  enCsRelAll;
    PS_BOOL_ENUM_UINT8                  enPsRelAll;
    MTC_SESSION_TYPE_ENUM_UINT8         enSessionType;
    VOS_UINT8                           ucReserved;
}MTC_END_SESSION_IND_STRU;


typedef struct
{
    TAF_CTRL_STRU                       stCtrl;
    TAF_MTC_POWER_SAVE_ENUM_UINT8       enPowerSaveStatus;
    VOS_UINT8                           aucReserved[3];
}MTC_POWER_SAVE_IND_STRU;




/*****************************************************************************
  H2ASN������Ϣ�ṹ����
*****************************************************************************/
typedef struct
{
    TAF_MTC_MSG_ID_ENUM_UINT32          ulMsgId;                                /*_H2ASN_MsgChoice_Export TAF_MTC_MSG_ID_ENUM_UINT32*/
    VOS_UINT8                           aucMsgBlock[4];
    /***************************************************************************
        _H2ASN_MsgChoice_When_Comment          TAF_MTC_MSG_ID_ENUM_UINT32
    ****************************************************************************/
}TAF_MTC_MSG_REQ;
/*_H2ASN_Length UINT32*/

typedef struct
{
    VOS_MSG_HEADER
    TAF_MTC_MSG_REQ                     stMsgReq;
}TafMtcApi_MSG;

/*****************************************************************************
  8 UNION����
*****************************************************************************/


/*****************************************************************************
  9 OTHERS����
*****************************************************************************/



/*****************************************************************************
  10 ��������
*****************************************************************************/

VOS_UINT32 MTC_SndMsg(
    VOS_UINT32                          ulTaskId,
    VOS_UINT32                          ulMsgId,
    VOS_VOID                           *pData,
    VOS_UINT32                          ulLength
);


VOS_UINT32 MTC_SetCdmaServiceConnStateInfo(
    TAF_CTRL_STRU                      *pstCtrl,
    TAF_MTC_CDMA_STATE_INFO_STRU       *pstCdmsState
);


VOS_UINT32 MTC_SetModemServiceConnState(
    TAF_CTRL_STRU                      *pstCtrl,
    TAF_MTC_SRV_CONN_STATE_INFO_STRU   *pstModemConnSt
);


VOS_UINT32 MTC_SetModemUsimmState(
    TAF_CTRL_STRU                           *pstCtrl,
    TAF_MTC_USIMM_CARD_SERVIC_ENUM_UINT16    enUsimState,
    TAF_MTC_USIMM_CARD_SERVIC_ENUM_UINT16    enCsimState
);

VOS_VOID MTC_SetBeginSessionInfo(
    TAF_CTRL_STRU                      *pstCtrl,
    MTC_SESSION_TYPE_ENUM_UINT8         enSessionType
);

VOS_VOID MTC_SetEndSessionInfo(
    TAF_CTRL_STRU                      *pstCtrl,
    MTC_SESSION_TYPE_ENUM_UINT8         enSessionType
);

VOS_VOID MTC_SetPowerSaveInfo(
    VOS_UINT32                          ulSndpid,
    TAF_MTC_POWER_SAVE_ENUM_UINT8       enPowerSaveStatus
);

VOS_VOID MTC_SetRatModeInfo(
    TAF_CTRL_STRU                      *pstCtrl,
    MTC_RATMODE_ENUM_UINT8              enRatMode
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

#endif

