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
#ifndef  MN_CALL_API_H
#define  MN_CALL_API_H

/*****************************************************************************
  1 ͷ�ļ�����
*****************************************************************************/
#include "TafTypeDef.h"
#include "TafAppSsa.h"
#include "vos.h"

#include  "AtMnInterface.h"
#include "product_config.h"
#include "TafNvInterface.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


#pragma pack(4)

/*****************************************************************************
  2 ��������
*****************************************************************************/
/* �����и��� */
#define    MN_CALL_MAX_NUM              (7)

/* ��ѡ���Ƿ���ڵı�־ */
#define MN_CALL_OPTION_EXIST            (1)
#define MN_CALL_OPTION_NOT_EXIST        (0)

#define MN_CALL_MAX_EMC_NUM             (80)
#define MN_CALL_ECC_NUM_VALID_SIM_PRESENT (1)
#define MN_CALL_ECC_NUM_VALID_SIM_ABSENT  (0)
#define MN_CALL_ECC_NUM_INVALID_CATEGORY  (0)

/* 10.5.4.8 Called party subaddress */
#define MN_CALL_MAX_SUBADDR_INFO_LEN     (20)

#define MN_CALL_RPT_CFG_MAX_SIZE        (8)
#define     MN_CALL_NET_RAT_TYPE_WCDMA      (0)           /* ��ǰפ��ΪW */
#define     MN_CALL_NET_RAT_TYPE_GSM        (1)           /* ��ǰפ��ΪG */
#define     MN_CALL_NET_RAT_TYPE_LTE        (2)           /* ��ǰפ��ģΪL */

#define TAF_CALL_DTMF_MIN_ONLENGTH              (65)                            /* DTMF����Сʱ�� */
#define TAF_CALL_DTMF_DEFAULT_ONLENGTH          (60000)                         /* Ĭ��DTMF��ʱ��60s */

#define TAF_CALL_DTMF_DEFAULT_OFFLENGTH          (75)                           /* stop dtmf ack��start dtmf req֮��ʱ��������70����С��600����,Ĭ��75���� */

#define TAF_CALL_MAX_BC_NUM                             (2)                     /* ���BC���� */
#define TAF_CALL_MIN_BC_NUM                             (1)                     /* ��СBC���� */

#if (FEATURE_ON == FEATURE_ECALL)
#define  TAF_ECALL_REDIAL_PERIOD_TIMER_LENGTH           (120000)                /* 120s */
#define  TAF_ECALL_REDIAL_INTERVAL_TIMER_LENGTH         (5000)                  /* 5s */
#define  TAF_CALL_T2_TIMER_LENGTH                       (3600000)               /* ʱ��Ϊ1Сʱ */
/* T9��ʱ��ʱ�������ã�Ĭ������Ϊ1Сʱ��������ʱ����Χ: 1 hour<= T9 <= 12 hours */
#define  TAF_CALL_ONE_HOUR_TIMER_LENGTH                 (3600000)
#define  TAF_CALL_T9_MIN_TIMER_LENGTH                   (1)                     /* T9��Сʱ��Ϊ1Сʱ */
#define  TAF_CALL_T9_MAX_TIMER_LENGTH                   (12)                    /* T9���ʱ��Ϊ12��Ϣ */
#endif

/* Added by l60609 for CDMA 1X Iteration 2, 2014-9-3, begin */
#define TAF_CALL_MAX_NUM_DIGIT_LEN                          (64)
#define TAF_CALL_MAX_FLASH_DIGIT_LEN                        (32)
#define TAF_CALL_MAX_BURST_DTMF_NUM                         (255)

/* Added by l60609 for CDMA 1X Iteration 2, 2014-9-3, end */

#define TAF_CALL_MAX_ECONF_CALLED_NUM        (5)
#define TAF_IMSA_ALPHA_STRING_SZ            (129)

#define TAF_CALL_MAX_DISPALY_CHARI_OCTET_NUM                (64)
#define TAF_CALL_MAX_CALLED_NUMBER_CHARI_OCTET_NUM          (64)
#define TAF_CALL_MAX_CALLING_NUMBER_CHARI_OCTET_NUM         (64)
#define TAF_CALL_MAX_CONNECTED_NUMBER_CHARI_OCTET_NUM       (64)
#define TAF_CALL_MAX_REDIRECTING_NUMBER_CHARI_OCTET_NUM     (64)
#define TAF_CALL_MAX_EXT_DISPLAY_DATA_NUM                   (3)
#define TAF_CALL_MAX_EXTENDED_DISPALY_CHARI_OCTET_NUM       (64)

#define MN_CALL_EMER_CATEGORG_VAL_MAX_SUSPORT_ECALL         ( ((VOS_UINT8) MN_CALL_EMER_CATEGORG_POLICE) \
                                                            | ((VOS_UINT8) MN_CALL_EMER_CATEGORG_AMBULANCE) \
                                                            | ((VOS_UINT8) MN_CALL_EMER_CATEGORG_FIRE_BRIGADE) \
                                                            | ((VOS_UINT8) MN_CALL_EMER_CATEGORG_MARINE_GUARD) \
                                                            | ((VOS_UINT8) MN_CALL_EMER_CATEGORG_MOUNTAIN_RESCUE) \
                                                            | ((VOS_UINT8) MN_CALL_EMER_CATEGORG_MAN_INIT_ECALL) \
                                                            | ((VOS_UINT8) MN_CALL_EMER_CATEGORG_AUTO_INIT_ECALL) )


#define MN_CALL_EMER_CATEGORG_VAL_MAX_NOT_SUSPORT_ECALL     ( ((VOS_UINT8) MN_CALL_EMER_CATEGORG_POLICE) \
                                                            | ((VOS_UINT8) MN_CALL_EMER_CATEGORG_AMBULANCE) \
                                                            | ((VOS_UINT8) MN_CALL_EMER_CATEGORG_FIRE_BRIGADE) \
                                                            | ((VOS_UINT8) MN_CALL_EMER_CATEGORG_MARINE_GUARD) \
                                                            | ((VOS_UINT8) MN_CALL_EMER_CATEGORG_MOUNTAIN_RESCUE) )

#define TAF_CALL_APP_EC_RANDOM_NUM                          (32)
#define TAF_CALL_APP_EC_RANDOM_DATA_LENGTH                  (33)
#define TAF_CALL_ECC_KMC_PUBLIC_KEY_MAX_HEX_LENGTH          (48)
#define TAF_CALL_ECC_KMC_PUBLIC_KEY_MAX_ASCII_LENGTH        (96)
#define TAF_CALL_APP_ENCRYPTED_VOICE_DATA_MAX_LENGTH        (1024)
#define TAF_CALL_APP_EC_RANDOM_MAX_GROUP                    (8)

#define MN_CALL_DISPLAY_NAME_STRING_SZ                      (129)

#define TAF_CALL_ERROR_INFO_TEXT_STRING_SZ                 (129)

#define   MN_CALL_FILL_MSG_HEADER(pSendMsg, RecvPid)    \
        (pSendMsg)->ulSenderCpuId   = VOS_LOCAL_CPUID; \
        (pSendMsg)->ulSenderPid     = WUEPS_PID_TAF; \
        (pSendMsg)->ulReceiverCpuId = VOS_LOCAL_CPUID; \
        (pSendMsg)->ulReceiverPid   = RecvPid;

#define TAF_CALL_CNAP_NAME_STR_MAX_LENGTH                   (161)               /* CNAP���������ֶ���󳤶� */

#define TAF_CALL_MAX_CC_TIMEROUT_NUM                        (1)                 /* ����ܼǵ�Cc��ʱ����ʱԭ������ */
                                                                                /* Ŀǰֻ��310��ʱ�ز�ʹ�� */

/*****************************************************************************
  3���Ͷ���
*****************************************************************************/
/* Added by f62575 for AT Project, 2011-11-1, begin */


/*****************************************************************************
 ö����    : NAS_CC_STATE_INFO_STRU
 �ṹ˵��  : CC�ڲ�״̬�ṹ
*****************************************************************************/
enum NAS_CC_CALL_STATE_ENUM                                                     /* Call state value (octet 2)               */
{                                                                               /* Bits 6 5 4 3 2 1                         */
    NAS_CC_CALL_STATE_U0                                    =  0,               /*      0 0 0 0 0 0 UO - null               */
    NAS_CC_CALL_STATE_U0_1                                  =  2,               /*      0 0 0 0 1 0 U0.1- MM con pending    */
    NAS_CC_CALL_STATE_U0_2                                  = 34,               /*      1 0 0 0 1 0 U0.2- CC prompt present */
    NAS_CC_CALL_STATE_U0_3                                  = 35,               /*      1 0 0 0 1 1 U0.3- Wait for network  */
    NAS_CC_CALL_STATE_U0_4                                  = 36,               /*      1 0 0 1 0 0 U0.4- CC-Est present    */
    NAS_CC_CALL_STATE_U0_5                                  = 37,               /*      1 0 0 1 0 1 U0.5- CC-Est confirmed  */
    NAS_CC_CALL_STATE_U0_6                                  = 38,               /*      1 0 0 1 1 0 U0.6- Recall present    */
    NAS_CC_CALL_STATE_U1                                    =  1,               /*      0 0 0 0 0 1 U1 - call initiated     */
    NAS_CC_CALL_STATE_U3                                    =  3,               /*      0 0 0 0 1 1 U3 - MO call proc       */
    NAS_CC_CALL_STATE_U4                                    =  4,               /*      0 0 0 1 0 0 U4 - call delivered     */
    NAS_CC_CALL_STATE_U6                                    =  6,               /*      0 0 0 1 1 0 U6 - call present       */
    NAS_CC_CALL_STATE_U7                                    =  7,               /*      0 0 0 1 1 1 U7 - call received      */
    NAS_CC_CALL_STATE_U8                                    =  8,               /*      0 0 1 0 0 0 U8 - connect request    */
    NAS_CC_CALL_STATE_U9                                    =  9,               /*      0 0 1 0 0 1 U9 - MT call Cnf        */
    NAS_CC_CALL_STATE_U10                                   = 10,               /*      0 0 1 0 1 0 U10- active             */
    NAS_CC_CALL_STATE_U11                                   = 11,               /*      0 0 1 0 1 1 U11- disconnect REQ     */
    NAS_CC_CALL_STATE_U12                                   = 12,               /*      0 0 1 1 0 0 U12- disconnect IND     */
    NAS_CC_CALL_STATE_U19                                   = 19,               /*      0 1 0 0 1 1 U19- release REQ        */
    NAS_CC_CALL_STATE_U26                                   = 26,               /*      0 1 1 0 1 0 U26- MO modify          */
    NAS_CC_CALL_STATE_U27                                   = 27,               /*      0 1 1 0 1 1 U27- MT modify          */
    NAS_CC_CALL_STATE_BUTT
};
typedef VOS_UINT8   NAS_CC_CALL_STATE_ENUM_U8;

enum MN_CALL_ALS_LINE_NO_ENUM
{
    MN_CALL_ALS_LINE_NO_1               = 1,
    MN_CALL_ALS_LINE_NO_2               = 2,
    MN_CALL_ALS_LINE_NO_MAX,
    MN_CALL_ALS_LINE_NO_BUTT
};
typedef VOS_UINT8 MN_CALL_ALS_LINE_NO_ENUM_U8;

/* Table 10.5.100/3GPP TS 24.008: Auxiliary states information element        */
enum NAS_CC_HOLD_AUX_STATE_ENUM                                                 /* Hold aux state (octet 3)          */
{                                                                               /* Bits 4 3                          */
    NAS_CC_HOLD_AUX_S_IDLE                                  = 0,                /*      0 0 idle                     */
    NAS_CC_HOLD_AUX_S_HOLD_REQ                              = 1,                /*      0 1 hold request             */
    NAS_CC_HOLD_AUX_S_CALL_HELD                             = 2,                /*      1 0 call held                */
    NAS_CC_HOLD_AUX_S_RETRIEVE_REQ                          = 3,                /*      1 1 retrieve request         */
    NAS_CC_HOLD_AUX_S_BUTT
};                                                                              /* defined in 24.083 [27].           */
typedef VOS_UINT8   NAS_CC_HOLD_AUX_STATE_ENUM_U8;

/* Table 10.5.101/3GPP TS 24.008: Auxiliary states information element        */
enum NAS_CC_MPTY_AUX_STATE_ENUM                                                 /* Multi party aux state (octet 3)   */
{                                                                               /* Bits 2 1                          */
    NAS_CC_MPTY_AUX_S_IDLE                                  = 0,                /*      0 0 idle                     */
    NAS_CC_MPTY_AUX_S_MPTY_REQ                              = 1,                /*      0 1 MPTY request             */
    NAS_CC_MPTY_AUX_S_CALL_IN_MPTY                          = 2,                /*      1 0 call in MPTY             */
    NAS_CC_MPTY_AUX_S_SPLIT_REQ                             = 3,                /*      1 1 split request            */
    NAS_CC_MPTY_AUX_S_BUTT
};                                                                              /* defined in 24.084 [28].           */
typedef VOS_UINT8   NAS_CC_MPTY_AUX_STATE_ENUM_U8;



enum NAS_CC_CALL_TYPE
{
    NAS_CC_CALL_TYPE_MO_NORMAL_CALL,                    /* ���������� */
    NAS_CC_CALL_TYPE_EMERGENCY_CALL,                    /* ���н����� */
    NAS_CC_CALL_TYPE_MT_NORMAL_CALL,                    /* ������ͨ�� */
    NAS_CC_CALL_TYPE_BUTT
};
typedef VOS_UINT8  NAS_CC_CALL_TYPE_ENUM_U8;

/* Added by f62575 for AT Project, 2011-11-1, end */


enum TAF_CALL_VOICE_DOMAIN_ENUM
{
    TAF_CALL_VOICE_DOMAIN_3GPP   = 0,
    TAF_CALL_VOICE_DOMAIN_IMS    = 1,
    TAF_CALL_VOICE_DOMAIN_3GPP2  = 2,

    TAF_CALL_VOICE_DOMAIN_AUTO   = 3,

    TAF_CALL_VOICE_DOMAIN_BUTT
};
typedef VOS_UINT8   TAF_CALL_VOICE_DOMAIN_ENUM_UINT8;


enum TAF_CALL_EMC_DOMAIN_ENUM
{
    TAF_CALL_EMC_DOMAIN_CS                  = 0,                            /* CS������� */
    TAF_CALL_EMC_DOMAIN_IMS_LTE             = 1,                            /* IMS LTE������� */
    TAF_CALL_EMC_DOMAIN_IMS_WIFI            = 2,                            /* IMS WIFI������� */
    TAF_CALL_EMC_DOMAIN_BUTT
};
typedef VOS_UINT8   TAF_CALL_EMC_DOMAIN_ENUM_UINT8;


enum TAF_CALL_SERVICE_TYPE_ENUM
{
    TAF_CALL_SERVICE_TYPE_NONE          = 0,                              /* ��WPS CALL */
    TAF_CALL_SERVICE_TYPE_WPS           = 1,                              /* WPS CALL */
    TAF_CALL_SERVICE_TYPE_BUTT
};
typedef VOS_UINT8 TAF_CALL_SERVICE_TYPE_ENUM_UINT8;


enum TAF_CALL_PEER_VIDEO_SUPPORT_ENUM
{
    TAF_CALL_PEER_VIDEO_UNSUPPORT           = 0,                                /* �Զ���Ƶ������֧�� */
    TAF_CALL_PEER_VIDEO_SUPPORT             = 1,                                /* �Զ���Ƶ����֧�� */
    TAF_CALL_PEER_VIDEO_BUTT
};
typedef VOS_UINT8 TAF_CALL_PEER_VIDEO_SUPPORT_ENUM_UINT8;



typedef struct
{
    VOS_UINT8                           ucCallId;           /* ����ID */
    VOS_UINT8                           ucTi;               /* ����ID */
    NAS_CC_CALL_STATE_ENUM_U8           enCallState;        /* ����Э��״̬ */
    NAS_CC_HOLD_AUX_STATE_ENUM_U8       enHoldAuxState;     /* ���б��ָ���״̬ */
    NAS_CC_MPTY_AUX_STATE_ENUM_U8       enMptyAuxState;     /* �෽���и���״̬ */
    VOS_UINT8                           aucReserve1[3];
} NAS_CC_STATE_INFO_STRU;

/*****************************************************************************
 ö����    : AT_CC_STATE_QRY_CNF_STRU
 �ṹ˵��  : CC�ظ�AT״̬��ѯ�ظ��ӿ�
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;                              /* ��Ϣ�� */
    AT_APPCTRL_STRU                     stAtAppCtrl;
    VOS_UINT8                           ucCallNum;                              /* ���и��� */
    VOS_UINT8                           aucCallExist[MN_CALL_MAX_NUM];
    NAS_CC_STATE_INFO_STRU              astCcStateInfoList[MN_CALL_MAX_NUM];                          /* ��Ϣ���� */
} AT_CC_STATE_QRY_CNF_MSG_STRU;


enum MN_CALL_380_CS_CALL_TYPE_ENUM
{
    MN_CALL_380_CS_CALL_TYPE_NORMAL             = 0x0,
    MN_CALL_380_CS_CALL_TYPE_EMERGENCY          = 0x1,

    /* ���IMSA��ֵ��ΪBUTTʱ����ʾ���ֶ���Ч��SPM����Ҫ�����л����������ز����� */
    MN_CALL_380_CS_CALL_TYPE_BUTT
};

typedef VOS_UINT8 MN_CALL_380_CS_CALL_TYPE_ENUM_UINT8;



typedef VOS_UINT8  MN_CALL_ID_T;

enum MN_CALL_TYPE_ENUM
{
    MN_CALL_TYPE_VOICE,                                                         /* normal voice call */
    MN_CALL_TYPE_VIDEO_TX,                                                      /* Tx video call, Two way voice */
    MN_CALL_TYPE_VIDEO_RX,                                                      /* Rx video call, Two way voice */
    MN_CALL_TYPE_VIDEO,                                                         /* video call */
    MN_CALL_TYPE_FAX,                                                           /* group 3 fax */
    MN_CALL_TYPE_CS_DATA,                                                       /* CS data call*/
    MN_CALL_TYPE_EMERGENCY = 9,                                                 /* emergency call */
    MN_CALL_TYPE_MIEC,                                                          /* manually initiated ecall */
    MN_CALL_TYPE_AIEC,                                                          /* automatic initiated ecall */
    MN_CALL_TYPE_TEST,                                                          /* test ecall */
    MN_CALL_TYPE_RECFGURATION,                                                  /* reconfiguration call */
    MN_CALL_TYPE_PSAP_ECALL,                                                    /* ����PSAP�غ��ĺ������� */
    MN_CALL_TYPE_BUTT
};
typedef VOS_UINT8  MN_CALL_TYPE_ENUM_U8;

/* �������е�Category,3gpp 31102�ж���*/
typedef enum MN_CALL_EMER_CATEGORY_TYPE
{
    MN_CALL_EMER_CATEGORG_POLICE            = 0x01,
    MN_CALL_EMER_CATEGORG_AMBULANCE         = 0x02,
    MN_CALL_EMER_CATEGORG_FIRE_BRIGADE      = 0x04,
    MN_CALL_EMER_CATEGORG_MARINE_GUARD      = 0x08,
    MN_CALL_EMER_CATEGORG_MOUNTAIN_RESCUE   = 0x10,
    MN_CALL_EMER_CATEGORG_MAN_INIT_ECALL    = 0x20,
    MN_CALL_EMER_CATEGORG_AUTO_INIT_ECALL   = 0x40,

    MN_CALL_EMER_EXT_CATEGORG_NATIONAL_INTELLIGENCE_SERVICE     = 0x06,
    MN_CALL_EMER_EXT_CATEGORG_SPY_REPORT                        = 0x03,
    MN_CALL_EMER_EXT_CATEGORG_SCHOOL_VIOLENCE_REPORT            = 0x12,
    MN_CALL_EMER_EXT_CATEGORG_CYBER_TERRORISM_REPORT            = 0x13,
    MN_CALL_EMER_EXT_CATEGORG_CONTRABANDS_REPORT                = 0x09,

    MN_CALL_EMER_CATEGORG_MAX               = 0x80,

    MN_CALL_EMER_CATEGORG_ERR_VALUE         = 0xFF,

}MN_CALL_EMER_CATEGORY_TYPE_ENUM;
typedef VOS_UINT8 MN_CALL_EMER_CATEGORG_TYPE_ENUM_U8;

enum MN_CALL_EX_STATE_ENUM
{
    MN_CALL_S_ACTIVE,                                                           /* active */
    MN_CALL_S_HELD,                                                             /* held */
    MN_CALL_S_DIALING,                                                          /* dialing (MO call) */
    MN_CALL_S_ALERTING,                                                         /* alerting (MO call) */
    MN_CALL_S_INCOMING,                                                         /* incoming (MT call) */
    MN_CALL_S_WAITING,                                                          /* waiting (MT call) */
    MN_CALL_S_IDLE,                                                             /* idle */
    MN_CALL_S_CCBS_WAITING_ACTIVE,                                              /*CCBS�ȴ�����̬*/
    MN_CALL_S_CCBS_WAITING_RECALL,                                              /*CCBS�ȴ��غ�̬*/
    MN_CALL_S_UNKNOWN,                                                          /* unknown state */

    MN_CALL_S_WAITING_ACCEPT,                                                   /* ��״̬ΪSRVCC����ǰ�û�����ʧ�ܣ��ɹ�����GU�·���connect */
    MN_CALL_S_WAITING_SRV_ACQ,                                                  /* �ȴ�ҵ�񲶻�ȷ�� */
    MN_CALL_S_WAITING_RF_AVAIL,                                                 /* �ȴ�RF��Դ���� */

    MN_CALL_S_BUTT
};
typedef VOS_UINT8  MN_CALL_STATE_ENUM_U8;



enum TAF_CALL_SUB_STATE_ENUM
{
    TAF_CALL_SUB_STATE_NULL,
    TAF_CALL_SUB_STATE_INCOMING_WAIT_CONNECT_ACK,                                    /* ata�������ϱ�ok��������connect ack��״̬*/
    TAF_CALL_SUB_STATE_DISCONNECT_WAIT_EVT_REL,
    TAF_CALL_SUB_STATE_BUTT
};
typedef VOS_UINT8 TAF_CALL_SUB_STATE_ENUM_UINT8;


enum MN_CALL_MPTY_STATE_ENUM
{
    MN_CALL_NOT_IN_MPTY,
    MN_CALL_IN_MPTY,
    MN_CALL_MPYT_STATE_BUTT
};
typedef VOS_UINT8  MN_CALL_MPTY_STATE_ENUM_U8;


/* call direction */
enum MN_CALL_DIR_ENUM
{
    MN_CALL_DIR_MO,                                                             /* MO CALL */
    MN_CALL_DIR_MT,                                                             /* MT CALL */
    MN_CALL_DIR_CCBS,                                                            /*CCBS CALL*/
    MN_CALL_DIR_BUTT
};
typedef VOS_UINT8  MN_CALL_DIR_ENUM_U8;

/* see 3GPP 27.007 +CBST, para speed */
enum MN_CALL_CS_DATA_SPEED_ENUM
{
    MN_CALL_CSD_SPD_AUTOBAUD                                = 0,
    MN_CALL_CSD_SPD_300_V21                                 = 1,
    MN_CALL_CSD_SPD_1K2_V22                                 = 2,
    MN_CALL_CSD_SPD_2K4_V22BIS                              = 4,
    MN_CALL_CSD_SPD_2K4_V26TER                              = 5,
    MN_CALL_CSD_SPD_4K8_V32                                 = 6,
    MN_CALL_CSD_SPD_9K6_V32                                 = 7,
    MN_CALL_CSD_SPD_9K6_V34                                 = 12,
    MN_CALL_CSD_SPD_14K4_V34                                = 14,
    MN_CALL_CSD_SPD_19K2_V34                                = 15,
    MN_CALL_CSD_SPD_28K8_V34                                = 16,
    MN_CALL_CSD_SPD_33K6_V34                                = 17,
    MN_CALL_CSD_SPD_1K2_V120                                = 34,
    MN_CALL_CSD_SPD_2K4_V120                                = 36,
    MN_CALL_CSD_SPD_4K8_V120                                = 38,
    MN_CALL_CSD_SPD_9K6_V120                                = 39,
    MN_CALL_CSD_SPD_14K4_V120                               = 43,
    MN_CALL_CSD_SPD_19K2_V120                               = 47,
    MN_CALL_CSD_SPD_28K8_V120                               = 48,
    MN_CALL_CSD_SPD_38K4_V120                               = 49,
    MN_CALL_CSD_SPD_48K_V120                                = 50,
    MN_CALL_CSD_SPD_56K_V120                                = 51,
    MN_CALL_CSD_SPD_300_V110                                = 65,
    MN_CALL_CSD_SPD_1K2_V110                                = 66,
    MN_CALL_CSD_SPD_2K4_V110                                = 68,
    MN_CALL_CSD_SPD_4K8_V110                                = 70,
    MN_CALL_CSD_SPD_9K6_V110                                = 71,
    MN_CALL_CSD_SPD_14K4_V110                               = 75,
    MN_CALL_CSD_SPD_19K2_V110                               = 79,
    MN_CALL_CSD_SPD_28K8_V110                               = 80,
    MN_CALL_CSD_SPD_38K4_V110                               = 81,
    MN_CALL_CSD_SPD_48K_V110                                = 82,
    MN_CALL_CSD_SPD_56K_FTM                                 = 83,
    MN_CALL_CSD_SPD_64K_FTM                                 = 84,
    MN_CALL_CSD_SPD_56K_BT                                  = 115,
    MN_CALL_CSD_SPD_64K_BT                                  = 116,
    MN_CALL_CSD_SPD_32K_PIAFS                               = 120,
    MN_CALL_CSD_SPD_64K_PIAFS                               = 121,
    MN_CALL_CSD_SPD_64K_MULTI                               = 134,
    MN_CALL_CSD_SPD_BUTT
};
typedef VOS_UINT8   MN_CALL_CS_DATA_SPEED_ENUM_U8;


/* see 3GPP 27.007 +CBST, para name */
enum MN_CALL_CS_DATA_NAME_ENUM
{
    MN_CALL_CSD_NAME_ASYNC_UDI,                                                 /* data circuit asynchronous (UDI or 3.1 kHz modem) */
    MN_CALL_CSD_NAME_SYNC_UDI,                                                  /* data circuit synchronous (UDI or 3.1 kHz modem)  */
    MN_CALL_CSD_NAME_ASYNC_RDI,                                                 /* data circuit asynchronous (RDI) */
    MN_CALL_CSD_NAME_SYNC_RDI,                                                   /* data circuit synchronous (RDI) */
    MN_CALL_CSD_NAME_BUTT
};
typedef VOS_UINT8   MN_CALL_CS_DATA_NAME_ENUM_U8;


/* see 3GPP 27.007 +CBST, para CE(Connection element) */
enum MN_CALL_CS_DATA_CE_ENUM
{
    MN_CALL_CSD_CE_T,                                                           /* transparent  */
    MN_CALL_CSD_CE_NT,                                                          /* non-transparent  */
    MN_CALL_CSD_CE_BOTH_T,                                                      /* both, transparent preferred  */
    MN_CALL_CSD_CE_BOTH_NT,                                                      /* both, non-transparent preferred  */
    MN_CALL_CSD_CE_BUTT
};
typedef VOS_UINT8   MN_CALL_CS_DATA_CE_ENUM_U8;

/* for CS DATA service configration */
typedef struct
{
    MN_CALL_CS_DATA_SPEED_ENUM_U8       enSpeed;
    MN_CALL_CS_DATA_NAME_ENUM_U8        enName;
    MN_CALL_CS_DATA_CE_ENUM_U8          enConnElem;
    VOS_UINT8                           aucReserved[5];
} MN_CALL_CS_DATA_CFG_STRU;


enum MN_CALL_CS_DATA_SPD_SIMPLE_ENUM
{
    MN_CALL_CSD_SPD_300,
    MN_CALL_CSD_SPD_1K2,
    MN_CALL_CSD_SPD_2K4,
    MN_CALL_CSD_SPD_4K8,
    MN_CALL_CSD_SPD_9K6,
    MN_CALL_CSD_SPD_12K,
    MN_CALL_CSD_SPD_14K4,
    MN_CALL_CSD_SPD_19K2,
    MN_CALL_CSD_SPD_28K8,
    MN_CALL_CSD_SPD_31K2,
    MN_CALL_CSD_SPD_32K,
    MN_CALL_CSD_SPD_33K6,
    MN_CALL_CSD_SPD_38K4,
    MN_CALL_CSD_SPD_48K,
    MN_CALL_CSD_SPD_56K,
    MN_CALL_CSD_SPD_64K,
    MN_CALL_CSD_SPD_SIMPLE_BUTT
};
typedef VOS_UINT8   MN_CALL_CS_DATA_SPD_SIMPLE_ENUM_U8;

 /* for report CS DATA service configration */
 typedef struct
 {
    MN_CALL_CS_DATA_SPD_SIMPLE_ENUM_U8 enSpeed;
    MN_CALL_CS_DATA_NAME_ENUM_U8       enName;
    MN_CALL_CS_DATA_CE_ENUM_U8         enConnElem;
    VOS_UINT8                          aucReserved1[5];
 } MN_CALL_CS_DATA_CFG_INFO_STRU;


/* 3GPP 27.007 +CMOD */
enum MN_CALL_MODE_ENUM
{
    MN_CALL_MODE_SINGLE,                                                        /* single mode  */
    MN_CALL_MODE_AVF,                                                            /* alternating voice/fax */
    MN_CALL_MODE_BUTT
};
typedef VOS_UINT8   MN_CALL_MODE_ENUM_U8;


enum MN_CALL_CLIR_CFG_ENUM
{
    MN_CALL_CLIR_AS_SUBS,                                                       /* CLIR AS SUBSCRIBE */
    MN_CALL_CLIR_INVOKE,                                                        /* CLIR invocation */
    MN_CALL_CLIR_SUPPRESS,                                                       /* CLIR suppression  */
    MN_CALL_CLIR_BUTT
};
typedef VOS_UINT8 MN_CALL_CLIR_CFG_ENUM_U8;


typedef struct
{
    VOS_BOOL                            bEnable;                                /* if enable cug */
    VOS_BOOL                            bSuppressPrefer;                        /* suppress preferential CUG */
    VOS_BOOL                            bSuppressOA;                            /* suppress OA(Outgoing Access) */
    VOS_BOOL                            bIndexPresent;
    VOS_UINT32                          ulIndex;                                /* CUG Index */
    VOS_UINT8                           aucReserved[4];
} MN_CALL_CUG_CFG_STRU;


/* 3GPP 27.007 +CHLD */
enum MN_CALL_SUPS_CMD_ENUM
{
    MN_CALL_SUPS_CMD_REL_HELD_OR_UDUB,                                          /* Releases all held calls or sets User Determined User Busy (UDUB) for a waiting call */
    MN_CALL_SUPS_CMD_REL_ACT_ACPT_OTH,                                          /* Releases all active calls (if any exist) and accepts the other (held or waiting) call */
    MN_CALL_SUPS_CMD_REL_CALL_X,                                                /* Releases a specific active call X */
    MN_CALL_SUPS_CMD_REL_ALL_CALL,                                              /* Releases all active call and held call and waiting call */
    MN_CALL_SUPS_CMD_HOLD_ACT_ACPT_OTH,                                         /* Places all active calls (if any exist) on hold and accepts the other (held or waiting) call */
    MN_CALL_SUPS_CMD_HOLD_ALL_EXCPT_X,                                          /* Places all active calls on hold except call X with which communication shall be supported */
    MN_CALL_SUPS_CMD_BUILD_MPTY,                                                /* Adds a held call to the conversation */
    MN_CALL_SUPS_CMD_ECT,                                                       /* Connects the two calls and disconnects the subscriber from both calls (ECT) */
    MN_CALL_SUPS_CMD_DEFLECT_CALL,                                              /* Redirect an incoming or a waiting call to the specified followed by SEND directory number */
    MN_CALL_SUPS_CMD_ACT_CCBS,                                                  /* Activates the Completion of Calls to Busy Subscriber Request */
    MN_CALL_SUPS_CMD_REL_ALL_EXCEPT_WAITING_CALL,                               /* Releases all calls (if any exist) except waiting call */
    MN_CALL_SUPS_CMD_REL_HELD,                                                  /* Releases all held calls*/
    MN_CALL_SUPS_CMD_REL_ACTIVE,                                                /* Releases all active calls */
    MN_CALL_SUPS_CMD_REL_ECALL,                                                 /* Releases eCall*/
    MN_CALL_SUPS_CMD_ECONF_REL_USER,                                            /* Releases member in econf  */
    MN_CALL_SUPS_CMD_ECONF_MERGE_CALL,                                          /* Merge econf and normal call */
    MN_CALL_SUPS_CMD_ACPT_WAITING_CALL,
    MN_CALL_SUPS_CMD_REL_INCOMING_OR_WAITING,

    MN_CALL_SUPS_CMD_BUTT
};
typedef VOS_UINT8   MN_CALL_SUPS_CMD_ENUM_U8;

/* Call relate SS Command Execute Result */
enum MN_CALL_SS_RESULT_ENUM
{
    MN_CALL_SS_RES_SUCCESS,
    MN_CALL_SS_RES_FAIL,
    MN_CALL_SS_RES_BUTT
};
typedef VOS_UINT8   MN_CALL_SS_RESULT_ENUM_U8;

/* Call relate SS Error Code */
enum MN_CALL_SS_ERR_CODE_ENUM
{
    MN_CALL_ERR_SS_UNSPECIFIC                      = 0,                         /* UNSPECIFIC Error*/
    MN_CALL_ERR_SS_ILLEGAL_SUBSCRIBER              = 9,                         /* IllegalSubscriber */
    MN_CALL_ERR_SS_ILLEGAL_EQUIPMENT               = 12,                        /* IllegalEquipment */
    MN_CALL_ERR_SS_CALL_BARRED                     = 13,                        /* CallBarred */
    MN_CALL_ERR_SS_FORWARDING_VIOLATION            = 14,                        /* ForwardingViolation */
    MN_CALL_ERR_SS_ILLEGALSS_OPERATION             = 16,                        /* IllegalSS-Operation */
    MN_CALL_ERR_SS_ERROR_STATUS                    = 17,                        /* SS-ErrorStatus */
    MN_CALL_ERR_SS_NOT_AVAILABLE                   = 18,                        /* SS-NotAvailable */
    MN_CALL_ERR_SS_SUBSCRIPTION_VIOLATION          = 19,                        /* SS-SubscriptionViolation */
    MN_CALL_ERR_SS_INCOMPATIBILITY                 = 20,                        /* SS-Incompatibility */
    MN_CALL_ERR_SS_FACILITY_NOT_SUPPORT            = 21,                        /* FacilityNotSupported */
    MN_CALL_ERR_SS_ABSENT_SUBSCRIBER               = 27,                        /* AbsentSubscriber */
    MN_CALL_ERR_SS_INCOMPATIBLE_TERMINAL           = 28,                        /* IncompatibleTerminal */
    MN_CALL_ERR_SS_SHORT_TERM_DENIAL               = 29,                        /* ShortTermDenial */
    MN_CALL_ERR_SS_LONG_TERM_DENIAL                = 30,                        /* LongTermDenial */
    MN_CALL_ERR_SS_SYSTEM_FAILURE                  = 34,                        /* SystemFailure */
    MN_CALL_ERR_SS_DATA_MISSING                    = 35,                        /* DataMissing */
    MN_CALL_ERR_SS_UNEXPECTED_DATA_VALUE           = 36,                        /* SystemFailure */
    MN_CALL_ERR_SS_FORWARDING_FAILED               = 47,                        /* ForwardingFailed */
    MN_CALL_ERR_SS_RESOURCE_LIMITATION             = 51,                        /* ResourceLimitation */
    MN_CALL_ERR_SS_DEFLECTION_TO_SERVED_SUBSCRIBER = 123,                       /* DeflectionToServedSubscriber  */
    MN_CALL_ERR_SS_SPECIAL_SERVICE_CODE            = 124,                       /* specialServiceCode */
    MN_CALL_ERR_SS_INVALID_DEFLECTED_TO_NUM        = 125,                       /* InvalidDeflectedToNumber */
    MN_CALL_ERR_SS_MAX_NUM_MPTY_EXCEEDED           = 126,                       /* MaxNumberOfMPTY-ParticipantsExceeded */
    MN_CALL_ERR_SS_RESOURCE_NOT_AVAILABLE          = 127,                        /* resourcesNotAvailable */
    MN_CALL_ERR_SS_BUTT
};
typedef VOS_UINT8   MN_CALL_SS_ERR_CODE_ENUM_U8;

/* Call Control Protocol Cause Value */
enum MN_CALL_CC_CAUSE_ENUM
{
    MN_CALL_INVALID_CAUSE                                   = 0,                /*   0. invalid cause value*/
    MN_CALL_UNASSIGNED_CAUSE                                = 1,                /*   1. Unassigned (unallocated) number            */
    MN_CALL_NO_ROUTE_TO_DEST                                = 3,                /*   3. No route to destination                    */
    MN_CALL_CHANNEL_UNACCEPTABLE                            = 6,                /*   6. Channel unacceptable                       */
    MN_CALL_OPERATOR_DETERMINED_BARRING                     = 8,                /*   8. Operator determined barring                */
    MN_CALL_NORMAL_CALL_CLEARING                            = 16,               /*  16. Normal call clearing                       */
    MN_CALL_USER_BUSY                                       = 17,               /*  17. User busy                                  */
    MN_CALL_NO_USER_RESPONDING                              = 18,               /*  18. No user responding                         */
    MN_CALL_USER_ALERTING_NO_ANSWER                         = 19,               /*  19. User alerting, no answer                   */
    MN_CALL_CALL_REJECTED                                   = 21,               /*  21. Call rejected                              */
    MN_CALL_NUMBER_CHANGED                                  = 22,               /*  22. Number changed                             */
    MN_CALL_CALL_REJECTED_DUE_TO_FEATURE_AT_THE_DESTINATION = 24,               /*  24. Call rejected due to feature at the destination */
    MN_CALL_PRE_EMPTION                                     = 25,               /*  25. Pre-emption                                */
    MN_CALL_NON_SELECTED_USER_CLEARING                      = 26,               /*  26. Non selected user clearing                 */
    MN_CALL_DESTINATION_OUT_OF_ORDER                        = 27,               /*  27. Destination out of order                   */
    MN_CALL_INVALID_NUMBER_FORMAT                           = 28,               /*  28. Invalid number format                      */
    MN_CALL_FACILITY_REJECTED                               = 29,               /*  29. Facility rejected                          */
    MN_CALL_RESPONSE_TO_STATUS_ENQUIRY                      = 30,               /*  30. Response to STATUS ENQUIRY                 */
    MN_CALL_NORMAL_UNSPECIFIED                              = 31,               /*  31. Normal, unspecified                        */
    MN_CALL_NO_CIRCUIT_CHANNEL_AVAILABLE                    = 34,               /*  34. No circuit/channel available               */
    MN_CALL_NETWORK_OUT_OF_ORDER                            = 38,               /*  38. Network out of order                       */
    MN_CALL_TEMPORARY_FAILURE                               = 41,               /*  41. Temporary failure                          */
    MN_CALL_SWITCHING_EQUIPMENT_CONGESTION                  = 42,               /*  42. Switching equipment congestion             */
    MN_CALL_ACCESS_INFORMATION_DISCARDED                    = 43,               /*  43. Access information discarded               */
    MN_CALL_REQUESTED_CIRCUIT_CHANNEL_NOT_AVAILABLE         = 44,               /*  44. requested circuit/channel not available    */
    MN_CALL_RESOURCES_UNAVAILABLE_UNSPECIFIED               = 47,               /*  47. Resources unavailable, unspecified         */
    MN_CALL_QUALITY_OF_SERVICE_UNAVAILABLE                  = 49,               /*  49. Quality of service unavailable             */
    MN_CALL_REQUESTED_FACILITY_NOT_SUBSCRIBED               = 50,               /*  50. Requested facility not subscribed          */
    MN_CALL_INCOMING_CALL_BARRED_WITHIN_CUG                 = 55,               /*  55. Incoming calls barred within the CUG       */
    MN_CALL_BEARER_CAPABILITY_NOT_AUTHORISED                = 57,               /*  57. Bearer capability not authorized           */
    MN_CALL_BEARER_CAPABILITY_NOT_PRESENTLY_AVAILABLE       = 58,               /*  58. Bearer capability not presently available  */
    MN_CALL_SERVICE_OR_OPTION_NOT_AVAILABLE                 = 63,               /*  63. Service or option not available, unspec    */
    MN_CALL_BEARER_SERVICE_NOT_IMPLEMENTED                  = 65,               /*  65. Bearer service not implemented             */
    MN_CALL_ACM_GEQ_ACMMAX                                  = 68,               /*  68. ACM equal to or greater than ACMmax        */
    MN_CALL_REQUESTED_FACILITY_NOT_IMPLEMENTED              = 69,               /*  69. Requested facility not implemented         */
    MN_CALL_ONLY_RESTRICTED_DIGITAL_INFO_BC_AVAILABLE       = 70,               /*  70. Only restricted digital info BC available  */
    MN_CALL_SERVICE_OR_OPTION_NOT_IMPLEMENTED               = 79,               /*  79. Service or option not implemented, unspec  */
    MN_CALL_INVALID_TRANSACTION_ID_VALUE                    = 81,               /*  81. Invalid transaction identifier value       */
    MN_CALL_USER_NOT_MEMBER_OF_CUG                          = 87,               /*  87. User not member of CUG                     */
    MN_CALL_INCOMPATIBLE_DESTINATION                        = 88,               /*  88. Incompatible destination Incompatible para */
    MN_CALL_INVALID_TRANSIT_NETWORK_SELECTION               = 91,               /*  91. Invalid transit network selection          */
    MN_CALL_SEMANTICALLY_INCORRECT_MESSAGE                  = 95,               /*  95. Semantically incorrect message             */
    MN_CALL_INVALID_MANDATORY_INFORMATION                   = 96,               /*  96. Invalid mandatory information              */
    MN_CALL_MESSAGE_TYPE_NON_EXISTENT                       = 97,               /*  97. Msg type non-existent or not implemented   */
    MN_CALL_MESSAGE_TYPE_NOT_COMPATIBLE_WITH_PROT_STATE     = 98,               /*  98. Msg type not compatible with protocol state*/
    MN_CALL_IE_NON_EXISTENT_OR_NOT_IMPLEMENTED              = 99,               /*  99. IE non-existent or not implemented         */
    MN_CALL_CONDITIONAL_IE_ERROR                            = 100,              /* 100. Conditional IE error                       */
    MN_CALL_MESSAGE_NOT_COMPATIBLE_WITH_PROTOCOL_STATE      = 101,              /* 101. Message not compatible with protocol state */
    MN_CALL_RECOVERY_ON_TIMER_EXPIRY                        = 102,              /* 102. Recovery on timer expiry Timer number      */
    MN_CALL_PROTOCOL_ERROR_UNSPECIFIED                      = 111,              /* 111. Protocol error, unspecified                */
    MN_CALL_INTERWORKING_UNSPECIFIED                        = 127,               /* 127. Interworking, unspecified                  */
    MN_CALL_CC_CAUSE_BUTT
};
typedef VOS_UINT8   MN_CALL_CC_CAUSE_ENUM_U8;


/* the reason for no num of the incoming call */
enum MN_CALL_NO_CLI_CAUSE_ENUM
{
    MN_CALL_NO_CLI_UNAVAL,                                                      /* num unavailable */
    MN_CALL_NO_CLI_USR_REJ,                                                     /* user reject to provide num */
    MN_CALL_NO_CLI_INTERACT,                                                    /* interact by other service */
    MN_CALL_NO_CLI_PAYPHONE,                                                     /* Pay Phone */
    MN_CALL_NO_CLI_BUTT
};
typedef VOS_UINT8  MN_CALL_NO_CLI_CAUSE_ENUM_U8;

/* 3GPP 27.007 +CSSI/+CSSU */
enum MN_CALL_SS_NOTIFY_CODE_ENUM
{
    MN_CALL_SS_NTFY_UNCOND_FWD_ACTIVE,                                          /* unconditional call forwarding is active  */
    MN_CALL_SS_NTFY_COND_FWD_ACTIVE,                                            /* some of the conditional call forwardings are active  */
    MN_CALL_SS_NTFY_BE_FORWORDED,                                               /* call has been forwarded  */
    MN_CALL_SS_NTFY_IS_WAITING,                                                 /* call is waiting */
    MN_CALL_SS_NTFY_MO_CUG_INFO,                                                /* this is a CUG call (also <index> present)  */
    MN_CALL_SS_NTFY_OUTGOING_BARRED,                                            /* outgoing calls are barred */
    MN_CALL_SS_NTFY_INCOMING_BARRED,                                            /* incoming calls are barred  */
    MN_CALL_SS_NTFY_CLIR_SUPPRESS_REJ,                                          /* CLIR suppression rejected */
    MN_CALL_SS_NTFY_BE_DEFLECTED,                                               /* call has been deflected  */
    MN_CALL_SS_NTFY_FORWORDED_CALL,                                             /* this is a forwarded call (MT call setup)  */
    MN_CALL_SS_NTFY_MT_CUG_INFO,                                                /* this is a CUG call (also <index> present) (MT call setup)  */
    MN_CALL_SS_NTFY_ON_HOLD,                                                    /* call has been put on hold (during a voice call)  */
    MN_CALL_SS_NTFY_RETRIEVED,                                                  /* call has been retrieved (during a voice call) */
    MN_CALL_SS_NTFY_ENTER_MPTY,                                                 /* multiparty call entered (during a voice call) */
    MN_CALL_SS_NTFY_DEFLECTED_CALL,                                             /* this is a deflected call (MT call setup)  */
    MN_CALL_SS_NTFY_EXPLICIT_CALL_TRANSFER,                                     /* Explicit Call Transfer (ECT) Supplementary Service*/
    MN_CALL_SS_NTFY_CCBS_RECALL,                                                /*this is a CCBS recall*/
    MN_CALL_SS_NTFY_CCBS_BE_RECALLED,                                            /*call has been CCBS recalled*/
    MN_CALL_SS_NTFY_BUTT
};
typedef  VOS_UINT8  MN_CALL_SS_NOTIFY_CODE_ENUM_U8;


/* Call Number Type  */
/* ��䷽ʽ����
  ----------------------------------------------------------------
  |   8   |   7   |   6   |   5   |   4   |   3   |   2   |   1   |
  ----------------------------------------------------------------
  |   1   |    Type of number     | Numbering plan identification |
  ----------------------------------------------------------------
*/
enum MN_CALL_EXIT_ENUM
{
    MN_CALL_IS_EXIT     =0x80,                                                        /*000: unknown */
    MN_CALL_EXIT_BUTT
};
typedef VOS_UINT8   MN_CALL_EXIT_ENUM_U8;

/* Type of Number,5,6,7bit of the MN_CALL_NUM_TYPE_ENUM_U8 */
enum MN_CALL_TON_ENUM
{
    MN_CALL_TON_UNKNOWN,                                                        /*000: unknown */
    MN_CALL_TON_INTERNATIONAL,                                                  /*001: international number */
    MN_CALL_TON_NATIONAL,                                                       /*010: national number */
    MN_CALL_TON_NETWORK_SPEC,                                                   /*011: network specific number */
    MN_CALL_TON_SHORT_CODE,                                                      /*100: dedicated access, short code */
    MN_CALL_TON_BUTT
};
typedef VOS_UINT8   MN_CALL_TON_ENUM_U8;

/* Numbering plan identification,1,2,3,4bit fo the MN_CALL_NUM_TYPE_ENUM_U8 */
enum MN_CALL_NPI_ENUM
{
    MN_CALL_NPI_UNKNOWN = 0,                                                    /*0000: unknown */
    MN_CALL_NPI_ISDN,                                                           /*0001: ISDN/telephony numbering plan  */
    MN_CALL_NPI_DATA = 3,                                                       /*0011: data numbering plan  */
    MN_CALL_NPI_TELEX = 4,                                                      /*0100: telex numbering plan  */
    MN_CALL_NPI_NATIONAL = 8,                                                   /*1000: national numbering plane */
    MN_CALL_NPI_PRIVATE,                                                         /*1001: private numbering plan */
    MN_CALL_NPI_BUTT
};
typedef VOS_UINT8   MN_CALL_NPI_ENUM_U8;
typedef VOS_UINT8   MN_CALL_NUM_TYPE_ENUM_U8;

/* Call Number Structure */
#define MN_CALL_MAX_BCD_NUM_LEN         20
typedef struct
{
    MN_CALL_NUM_TYPE_ENUM_U8            enNumType;
    VOS_UINT8                           ucNumLen;
    VOS_UINT8                           ucReserved[2];
    VOS_UINT8                           aucBcdNum[MN_CALL_MAX_BCD_NUM_LEN];
} MN_CALL_BCD_NUM_STRU;



#define TAF_ECC_CALL_MAX_BCD_NUM_LEN         6
typedef struct
{
    MN_CALL_NUM_TYPE_ENUM_U8            enNumType;
    VOS_UINT8                           ucNumLen;
    VOS_UINT8                           aucBcdNum[TAF_ECC_CALL_MAX_BCD_NUM_LEN];
} TAF_ECC_CALL_BCD_NUM_STRU;


enum MN_CALL_ECT_CALL_STATE_ENUM
{
    MN_CALL_ECT_STATE_ALERTING,
    MN_CALL_ECT_STATE_ACTIVE,
    MN_CALL_ECT_STATE_BUTT
};
typedef VOS_UINT8  MN_CALL_ECT_CALL_STATE_ENUM_U8;

enum MN_CALL_ECT_RDN_ENUM
{
    MN_CALL_ECT_RDN_PRESENTATION_ALLOWED_ADDR,                                  /* presentationAllowedAddress */
    MN_CALL_ECT_RDN_PRESENTATION_RESTRICTED,                                    /* presentationRestricted */
    MN_CALL_ECT_RDN_NUM_NOT_AVAILABLE,                                          /* numberNotAvailableDueToInterworking */
    MN_CALL_ECT_RDN_PRESENTATION_RESTRICTED_ADDR,                                /* presentationRestrictedAddress */
    MN_CALL_ECT_RDN_BUTT
};
typedef VOS_UINT8  MN_CALL_ECT_RDN_ENUM_U8;

typedef struct
{

    MN_CALL_ECT_CALL_STATE_ENUM_U8      enEctCallState;
    MN_CALL_ECT_RDN_ENUM_U8             enEctRdnType;
    VOS_UINT8                           aucReserved[2];

    union
    {
        MN_CALL_BCD_NUM_STRU            stPresentationAllowedAddr;
        MN_CALL_BCD_NUM_STRU            stPresentationRestrictedAddr;
    }rdn;

}MN_CALL_ECT_IND_STRU;

typedef struct
{
    MN_CALL_SS_NOTIFY_CODE_ENUM_U8      enCode;
    VOS_UINT8                           aucReserve1[3];
    VOS_UINT32                          ulCugIndex;                             /* CUG Index��is valid while Code is MO_CUG_INFO or MT_CUG_INFO */
    MN_CALL_ECT_IND_STRU                stEctIndicator;                         /* */
} MN_CALL_SS_NOTIFY_STRU;

/* ����CSSN������,�����ȥ���� */
enum MN_CALL_SET_CSSN_TYPE_ENUM
{
    MN_CALL_SET_CSSN_DEACT,
    MN_CALL_SET_CSSN_ACT,
    MN_CALL_SET_CSSN_BUTT
};
typedef VOS_UINT32 MN_CALL_SET_CSSN_TYPE_ENUM_U32;

enum MN_CALL_CUUS1_FLG_ENUM
{
    MN_CALL_CUUS1_DISABLE,
    MN_CALL_CUUS1_ENABLE,
    MN_CALL_CUUS1_BUTT
};
typedef VOS_UINT32 MN_CALL_CUUS1_FLG_ENUM_U32;


/* ����UUS1������,�����ȥ���� */
enum MN_CALL_SET_UUS1_TYPE_ENUM
{
    MN_CALL_SET_UUS1_ACT,
    MN_CALL_SET_UUS1_DEACT,
    MN_CALL_SET_UUS1_BUTT
};
typedef VOS_UINT32 MN_CALL_SET_UUS1_TYPE_ENUM_U32;

/* ���û��ϱ�UUS1����Ϣ����,�������м�����Ϣ,�������ANY,��Ĭ��Ϊ
����SETUP,PROGRESS��Ϣֻ���ϱ�,�������� */
enum MN_CALL_UUS1_MSG_TYPE_ENUM
{
    MN_CALL_UUS1_MSG_ANY,
    MN_CALL_UUS1_MSG_SETUP,
    MN_CALL_UUS1_MSG_ALERT,
    MN_CALL_UUS1_MSG_CONNECT,
    MN_CALL_UUS1_MSG_DISCONNECT,
    MN_CALL_UUS1_MSG_RELEASE,
    MN_CALL_UUS1_MSG_RELEASE_COMPLETE,
    MN_CALL_UUS1_MSG_PROGRESS,
    MN_CALL_UUS1_MSG_BUTT
};
typedef VOS_UINT32 MN_CALL_UUS1_MSG_TYPE_ENUM_U32;

#define MN_CALL_MAX_UUS1_MSG_NUM        (7)
#define MN_CALL_MAX_CSSN_MSG_NUM        (2)

/*The user-user is a type 4 information element with a minimum length of 3 octets
and a maximum length of either 35 or 131 octets. In the SETUP message the
user-user information element has a maximum size of 35 octets in a
GSM PLMN. In the USER INFORMATION, ALERTING, CONNECT, DISCONNECT,
PROGRESS, RELEASE and RELEASE COMPLETE messages the user-user information
element has a maximum size of 131 octets in a GSM PLMN.*/

#define MN_CALL_MIN_UUIE_LEN                    (3)
#define MN_CALL_MAX_UUIE_LEN                    (131)
#define MN_CALL_MAX_SETUP_UUIE_CONTEND_LEN      (33)
#define MN_CALL_MAX_OTHER_UUIE_CONTEND_LEN      (129)
#define MN_CALL_MIN_UUIE_CONTEND_LEN            (1)
#define MN_CALL_UUIE_HEADER_LEN                 (2)

/* �˴���UUIE������Э���й涨��UUIE����,������������ */
/********************************************************
  |  8 |  7  | 6 |  5 |  4 |  3 |  2 |  1   |
  |    |     User-user IEI                  | octet 1
  |         Length of user-user contents    | octet 2                           |
  |      User-user protocol discriminator   | octet 3
  |     User-user information               | octet 4*
  |       ...............                   |
  |       ...............                   | octet N*
*********************************************************/
#define MN_CALL_UUS_IEI                         (0x7E)
#define MN_CALL_IEI_POS                         (0x00)
#define MN_CALL_LEN_POS                         (0x01)
#define MN_CALL_PD_POS                          (0x02)
#define MN_CALL_CONTENT_POS                     (0x03)

typedef struct
{
    MN_CALL_UUS1_MSG_TYPE_ENUM_U32      enMsgType;
    VOS_UINT8                           aucUuie[MN_CALL_MAX_UUIE_LEN];
    VOS_UINT8                           aucReserved1;
}MN_CALL_UUS1_INFO_STRU;


/* Call Event */
enum MN_CALL_EVENT_ENUM
{
    MN_CALL_EVT_ORIG                = 0x00,                                     /* originate a MO Call */
    MN_CALL_EVT_CALL_PROC           = 0x01,                                     /* Call is Proceeding */
    MN_CALL_EVT_ALERTING            = 0x02,                                     /* Alerting,MO Call */
    MN_CALL_EVT_CONNECT             = 0x03,                                     /* Call Connect */
    MN_CALL_EVT_RELEASED            = 0x04,                                     /* Call Released */
    MN_CALL_EVT_INCOMING            = 0x05,                                     /* Incoming Call */
    MN_CALL_EVT_SS_CMD_PROGRESS     = 0x06,                                     /* Call Related Command is in progress */
    MN_CALL_EVT_SS_CMD_RSLT         = 0x07,                                     /* Call Related Command is Completed */
    MN_CALL_EVT_SS_NOTIFY           = 0x08,                                     /* SS Notify */
    MN_CALL_EVT_START_DTMF_CNF      = 0x09,                                     /* Start DTMF tempooralily response */
    MN_CALL_EVT_STOP_DTMF_CNF       = 0x0b,                                     /* Stop DTMF tempooralily response */
    MN_CALL_EVT_CCBS_POSSIBLE       = 0x0d,                                     /* ���Լ���CCBS*/
    MN_CALL_EVT_TIME_EXPIRED        = 0x0e,                                     /*��ʱ*/
    MN_CALL_EVT_UUS1_INFO           = 0x0f,                                     /* UUS1��Ϣ�ϱ� */
    MN_CALL_EVT_GET_CDUR_CNF        = 0x10,                                     /* ͨ��ʱ�� */
    MN_CALL_EVT_CLCC_INFO           = 0x11,                                     /* ��ǰ���к��е���Ϣ */
    MN_CALL_EVT_ALL_RELEASED        = 0x12,                                     /* Call All Released */
    MN_CALL_EVT_SET_UUS1_INFO_CNF   = 0x13,                                     /* �����û���������1��Ϣ */
    MN_CALL_EVT_QRY_UUS1_INFO_CNF   = 0x14,                                     /* ��ѯ�û���������1��Ϣ */
    MN_CALL_EVT_SET_ALS_CNF         = 0x15,                                     /* ���õ�ǰ��·�� */
    MN_CALL_EVT_QRY_ALS_CNF         = 0x16,                                     /* ��ѯ��ǰ��·�� */

    MN_CALL_EVT_ECC_NUM_IND         = 0x17,

    MN_CALL_EVT_HOLD                = 0x18,                                     /* Call Hold ���б��� */
    MN_CALL_EVT_RETRIEVE            = 0x19,                                     /* Call Retrieve ���лָ� */

    MN_CALL_EVT_CLPR_SET_CNF        = 0x1a,

    MN_CALL_EVT_CHANNEL_INFO_IND    = 0x1c,

    MN_CALL_EVT_XLEMA_CNF           = 0x1d,

    MN_CALL_EVT_SET_CSSN_CNF        = 0x1e,

    MN_CALL_EVT_CALL_ORIG_CNF       = 0x1f,
    MN_CALL_EVT_SUPS_CMD_CNF        = 0x20,

    MN_CALL_EVT_START_DTMF_RSLT     = 0x22,
    MN_CALL_EVT_STOP_DTMF_RSLT      = 0x23,

    MN_CALL_EVT_QRY_ECALL_INFO_CNF  = 0x24,

    MN_CALL_EVT_CALL_MODIFY_CNF     = 0x25,
    MN_CALL_EVT_CALL_ANSWER_REMOTE_MODIFY_CNF   = 0x26,
    MN_CALL_EVT_CALL_MODIFY_STATUS_IND          = 0x27,

    /* Added by l60609 for CDMA 1X Iteration 2, 2014-9-3, begin */
    TAF_CALL_EVT_SEND_FLASH_RSLT,                           /* _H2ASN_MsgChoice TAF_CALL_EVT_SEND_FLASH_RSLT_STRU */
    TAF_CALL_EVT_CALLED_NUM_INFO_IND,                       /* _H2ASN_MsgChoice TAF_CALL_EVT_CALLED_NUM_INFO_IND_STRU */
    TAF_CALL_EVT_CALLING_NUM_INFO_IND,                      /* _H2ASN_MsgChoice TAF_CALL_EVT_CALLING_NUM_INFO_IND_STRU */
    TAF_CALL_EVT_DISPLAY_INFO_IND,                          /* _H2ASN_MsgChoice TAF_CALL_EVT_DISPLAY_INFO_IND_STRU */
    TAF_CALL_EVT_EXT_DISPLAY_INFO_IND,                      /* _H2ASN_MsgChoice TAF_CALL_EVT_EXT_DISPLAY_INFO_IND_STRU */
    TAF_CALL_EVT_CONN_NUM_INFO_IND,                         /* _H2ASN_MsgChoice TAF_CALL_EVT_CONN_NUM_INFO_IND_STRU */
    TAF_CALL_EVT_REDIR_NUM_INFO_IND,                        /* _H2ASN_MsgChoice TAF_CALL_EVT_REDIR_NUM_INFO_IND_STRU */
    TAF_CALL_EVT_SIGNAL_INFO_IND,                           /* _H2ASN_MsgChoice TAF_CALL_EVT_SIGNAL_INFO_IND_STRU */
    TAF_CALL_EVT_LINE_CTRL_INFO_IND,                        /* _H2ASN_MsgChoice TAF_CALL_EVT_LINE_CTRL_INFO_IND_STRU */
    TAF_CALL_EVT_CALL_WAITING_IND,                          /* _H2ASN_MsgChoice TAF_CALL_EVT_CALL_WAITING_IND_STRU */
    /* Added by l60609 for CDMA 1X Iteration 2, 2014-9-3, end */

    TAF_CALL_EVT_ECONF_DIAL_CNF,
    TAF_CALL_EVT_ECONF_NOTIFY_IND,                               /* imsa֪ͨSPMģ����ǿ�Ͷ෽ͨ�������ߵ�״̬���ͱ仯  */
    TAF_CALL_EVT_CLCCECONF_INFO,

    /* Added by f279542 for CDMA 1X Iteration 4, 2014-11-10, begin */
    TAF_CALL_EVT_SEND_BURST_DTMF_CNF,                            /* _H2ASN_MsgChoice TAF_CALL_EVT_SEND_BURST_DTMF_CNF_STRU */
    TAF_CALL_EVT_SEND_BURST_DTMF_RSLT,                           /* _H2ASN_MsgChoice TAF_CALL_EVT_SEND_BURST_DTMF_RSLT_STRU */
    /* Added by f279542 for CDMA 1X Iteration 4, 2014-11-10, end */

    TAF_CALL_EVT_CCWAC_INFO_IND,                            /* _H2ASN_MsgChoice TAF_CALL_EVT_CCWAC_INFO_IND_STRU */

    TAF_CALL_EVT_SEND_CONT_DTMF_CNF,                            /* _H2ASN_MsgChoice TAF_CALL_EVT_SEND_CONT_DTMF_CNF_STRU */
    TAF_CALL_EVT_SEND_CONT_DTMF_RSLT,                           /* _H2ASN_MsgChoice TAF_CALL_EVT_SEND_CONT_DTMF_RSLT_STRU */
    TAF_CALL_EVT_RCV_CONT_DTMF_IND,                             /* _H2ASN_MsgChoice TAF_CALL_EVT_RCV_CONT_DTMF_IND_STRU */
    TAF_CALL_EVT_RCV_BURST_DTMF_IND,                            /* _H2ASN_MsgChoice TAF_CALL_EVT_RCV_BURST_DTMF_IND_STRU */

    TAF_CALL_EVT_CCLPR_SET_CNF,                                /* _H2ASN_MsgChoice TAF_CALL_EVT_CCLPR_GET_CNF_STRU */

    TAF_CALL_EVT_CCWAI_SET_CNF,                                 /* _H2ASN_MsgChoice TAF_CALL_EVT_CCWAI_SET_CNF */

    TAF_CALL_EVT_CHANNEL_QRY_CNF,
    MN_CALL_EVT_BUTT
};
typedef  VOS_UINT32  MN_CALL_EVENT_ENUM_U32;

/* ��ԭ�е�ö�ٶ�����е��� */
/* TAF_CS_CAUSE����NAS_CC_CAUSE��Ӧ ���ʱע�� */
#define TAF_CS_CAUSE_CC_NW_SECTION_BEGIN    (0x0000)
#define TAF_CS_CAUSE_CM_SRV_REJ_BEGIN       (0x0100)
#define TAF_CS_CAUSE_CSFB_SRV_REJ_BEGIN     (0x0200)
#define TAF_CS_CAUSE_SMS_CP_ERR_BEGIN       (0x0300)
#define TAF_CS_CAUSE_SMS_RP_ERR_BEGIN       (0x0400)
#define TAF_CS_CAUSE_SS_NW_REJ_BEGIN        (0x0500)
#define TAF_CS_CAUSE_RR_CONN_FAIL_BEGIN     (0x0600)
#define TAF_CS_CAUSE_RR_REL_BEGIN           (0x0700)
#define TAF_CS_CAUSE_CSFB_LMM_FAIL_BEGIN    (0x0800)
#define TAF_CS_CAUSE_MM_INTER_ERR_BEGIN     (0x0900)
#define TAF_CS_CAUSE_CALL_BEGIN             (0x0A00)
#define TAF_CS_CAUSE_MSG_INTER_ERR_BEGIN    (0x0B00)
#define TAF_CS_CAUSE_SS_INTER_ERR_BEGIN     (0x0C00)
#define TAF_CS_CAUSE_VC_BEGIN               (0x0D00)
#define TAF_CS_CAUSE_IMS_BEGIN              (0x4800)
#define TAF_CS_CAUSE_IMSA_BEGIN             (0x4C00)
/* Added by l60609 for CDMA 1X Iteration 2, 2014-9-3, begin */
#define TAF_CS_CAUSE_XCALL_BEGIN            (0x9000)
/* Added by l60609 for CDMA 1X Iteration 2, 2014-9-3, end */



enum TAF_CS_CAUSE_ENUM
{
    TAF_CS_CAUSE_SUCCESS                                                        = (TAF_CS_CAUSE_CC_NW_SECTION_BEGIN + 0),

    /* CS�������ϱ��Ĵ���ԭ��ֵ */
    TAF_CS_CAUSE_CC_NW_UNASSIGNED_CAUSE                                         = (TAF_CS_CAUSE_CC_NW_SECTION_BEGIN + 1),   /*   1. Unassigned (unallocated) number            */
    TAF_CS_CAUSE_CC_NW_NO_ROUTE_TO_DEST                                         = (TAF_CS_CAUSE_CC_NW_SECTION_BEGIN + 3),   /*   3. No route to destination                    */
    TAF_CS_CAUSE_CC_NW_CHANNEL_UNACCEPTABLE                                     = (TAF_CS_CAUSE_CC_NW_SECTION_BEGIN + 6),   /*   6. Channel unacceptable                       */
    TAF_CS_CAUSE_CC_NW_OPERATOR_DETERMINED_BARRING                              = (TAF_CS_CAUSE_CC_NW_SECTION_BEGIN + 8),   /*   8. Operator determined barring                */
    TAF_CS_CAUSE_CC_NW_NORMAL_CALL_CLEARING                                     = (TAF_CS_CAUSE_CC_NW_SECTION_BEGIN + 16),  /*  16. Normal call clearing                       */
    TAF_CS_CAUSE_CC_NW_USER_BUSY                                                = (TAF_CS_CAUSE_CC_NW_SECTION_BEGIN + 17),  /*  17. User busy                                  */
    TAF_CS_CAUSE_CC_NW_NO_USER_RESPONDING                                       = (TAF_CS_CAUSE_CC_NW_SECTION_BEGIN + 18),  /*  18. No user responding                         */
    TAF_CS_CAUSE_CC_NW_USER_ALERTING_NO_ANSWER                                  = (TAF_CS_CAUSE_CC_NW_SECTION_BEGIN + 19),  /*  19. User alerting, no answer                   */
    TAF_CS_CAUSE_CC_NW_CALL_REJECTED                                            = (TAF_CS_CAUSE_CC_NW_SECTION_BEGIN + 21),  /*  21. Call rejected                              */
    TAF_CS_CAUSE_CC_NW_NUMBER_CHANGED                                           = (TAF_CS_CAUSE_CC_NW_SECTION_BEGIN + 22),  /*  22. Number changed                             */
    TAF_CS_CAUSE_CC_NW_CALL_REJECTED_DUE_TO_FEATURE_AT_THE_DESTINATION          = (TAF_CS_CAUSE_CC_NW_SECTION_BEGIN + 24),  /*  24. Call rejected due to feature at the destination */
    TAF_CS_CAUSE_CC_NW_PRE_EMPTION                                              = (TAF_CS_CAUSE_CC_NW_SECTION_BEGIN + 25),  /*  25. Pre-emption                                */
    TAF_CS_CAUSE_CC_NW_NON_SELECTED_USER_CLEARING                               = (TAF_CS_CAUSE_CC_NW_SECTION_BEGIN + 26),  /*  26. Non selected user clearing                 */
    TAF_CS_CAUSE_CC_NW_DESTINATION_OUT_OF_ORDER                                 = (TAF_CS_CAUSE_CC_NW_SECTION_BEGIN + 27),  /*  27. Destination out of order                   */
    TAF_CS_CAUSE_CC_NW_INVALID_NUMBER_FORMAT                                    = (TAF_CS_CAUSE_CC_NW_SECTION_BEGIN + 28),  /*  28. Invalid number format                      */
    TAF_CS_CAUSE_CC_NW_FACILITY_REJECTED                                        = (TAF_CS_CAUSE_CC_NW_SECTION_BEGIN + 29),  /*  29. Facility rejected                          */
    TAF_CS_CAUSE_CC_NW_RESPONSE_TO_STATUS_ENQUIRY                               = (TAF_CS_CAUSE_CC_NW_SECTION_BEGIN + 30),  /*  30. Response to STATUS ENQUIRY                 */
    TAF_CS_CAUSE_CC_NW_NORMAL_UNSPECIFIED                                       = (TAF_CS_CAUSE_CC_NW_SECTION_BEGIN + 31),  /*  31. Normal, unspecified                        */
    TAF_CS_CAUSE_CC_NW_NO_CIRCUIT_CHANNEL_AVAILABLE                             = (TAF_CS_CAUSE_CC_NW_SECTION_BEGIN + 34),  /*  34. No circuit/channel available               */
    TAF_CS_CAUSE_CC_NW_NETWORK_OUT_OF_ORDER                                     = (TAF_CS_CAUSE_CC_NW_SECTION_BEGIN + 38),  /*  38. Network out of order                       */
    TAF_CS_CAUSE_CC_NW_TEMPORARY_FAILURE                                        = (TAF_CS_CAUSE_CC_NW_SECTION_BEGIN + 41),  /*  41. Temporary failure                          */
    TAF_CS_CAUSE_CC_NW_SWITCHING_EQUIPMENT_CONGESTION                           = (TAF_CS_CAUSE_CC_NW_SECTION_BEGIN + 42),  /*  42. Switching equipment congestion             */
    TAF_CS_CAUSE_CC_NW_ACCESS_INFORMATION_DISCARDED                             = (TAF_CS_CAUSE_CC_NW_SECTION_BEGIN + 43),  /*  43. Access information discarded               */
    TAF_CS_CAUSE_CC_NW_REQUESTED_CIRCUIT_CHANNEL_NOT_AVAILABLE                  = (TAF_CS_CAUSE_CC_NW_SECTION_BEGIN + 44),  /*  44. requested circuit/channel not available    */
    TAF_CS_CAUSE_CC_NW_RESOURCES_UNAVAILABLE_UNSPECIFIED                        = (TAF_CS_CAUSE_CC_NW_SECTION_BEGIN + 47),  /*  47. Resources unavailable, unspecified         */
    TAF_CS_CAUSE_CC_NW_QUALITY_OF_SERVICE_UNAVAILABLE                           = (TAF_CS_CAUSE_CC_NW_SECTION_BEGIN + 49),  /*  49. Quality of service unavailable             */
    TAF_CS_CAUSE_CC_NW_REQUESTED_FACILITY_NOT_SUBSCRIBED                        = (TAF_CS_CAUSE_CC_NW_SECTION_BEGIN + 50),  /*  50. Requested facility not subscribed          */
    TAF_CS_CAUSE_CC_NW_INCOMING_CALL_BARRED_WITHIN_CUG                          = (TAF_CS_CAUSE_CC_NW_SECTION_BEGIN + 55),  /*  55. Incoming calls barred within the CUG       */
    TAF_CS_CAUSE_CC_NW_BEARER_CAPABILITY_NOT_AUTHORISED                         = (TAF_CS_CAUSE_CC_NW_SECTION_BEGIN + 57),  /*  57. Bearer capability not authorized           */
    TAF_CS_CAUSE_CC_NW_BEARER_CAPABILITY_NOT_PRESENTLY_AVAILABLE                = (TAF_CS_CAUSE_CC_NW_SECTION_BEGIN + 58),  /*  58. Bearer capability not presently available  */
    TAF_CS_CAUSE_CC_NW_SERVICE_OR_OPTION_NOT_AVAILABLE                          = (TAF_CS_CAUSE_CC_NW_SECTION_BEGIN + 63),  /*  63. Service or option not available, unspec    */
    TAF_CS_CAUSE_CC_NW_BEARER_SERVICE_NOT_IMPLEMENTED                           = (TAF_CS_CAUSE_CC_NW_SECTION_BEGIN + 65),  /*  65. Bearer service not implemented             */
    TAF_CS_CAUSE_CC_NW_ACM_GEQ_ACMMAX                                           = (TAF_CS_CAUSE_CC_NW_SECTION_BEGIN + 68),  /*  68. ACM equal to or greater than ACMmax        */
    TAF_CS_CAUSE_CC_NW_REQUESTED_FACILITY_NOT_IMPLEMENTED                       = (TAF_CS_CAUSE_CC_NW_SECTION_BEGIN + 69),  /*  69. Requested facility not implemented         */
    TAF_CS_CAUSE_CC_NW_ONLY_RESTRICTED_DIGITAL_INFO_BC_AVAILABLE                = (TAF_CS_CAUSE_CC_NW_SECTION_BEGIN + 70),  /*  70. Only restricted digital info BC available  */
    TAF_CS_CAUSE_CC_NW_SERVICE_OR_OPTION_NOT_IMPLEMENTED                        = (TAF_CS_CAUSE_CC_NW_SECTION_BEGIN + 79),  /*  79. Service or option not implemented, unspec  */
    TAF_CS_CAUSE_CC_NW_INVALID_TRANSACTION_ID_VALUE                             = (TAF_CS_CAUSE_CC_NW_SECTION_BEGIN + 81),  /*  81. Invalid transaction identifier value       */
    TAF_CS_CAUSE_CC_NW_USER_NOT_MEMBER_OF_CUG                                   = (TAF_CS_CAUSE_CC_NW_SECTION_BEGIN + 87),  /*  87. User not member of CUG                     */
    TAF_CS_CAUSE_CC_NW_INCOMPATIBLE_DESTINATION                                 = (TAF_CS_CAUSE_CC_NW_SECTION_BEGIN + 88),  /*  88. Incompatible destination Incompatible para */
    TAF_CS_CAUSE_CC_NW_INVALID_TRANSIT_NETWORK_SELECTION                        = (TAF_CS_CAUSE_CC_NW_SECTION_BEGIN + 91),  /*  91. Invalid transit network selection          */
    TAF_CS_CAUSE_CC_NW_SEMANTICALLY_INCORRECT_MESSAGE                           = (TAF_CS_CAUSE_CC_NW_SECTION_BEGIN + 95),  /*  95. Semantically incorrect message             */
    TAF_CS_CAUSE_CC_NW_INVALID_MANDATORY_INFORMATION                            = (TAF_CS_CAUSE_CC_NW_SECTION_BEGIN + 96),  /*  96. Invalid mandatory information              */
    TAF_CS_CAUSE_CC_NW_MESSAGE_TYPE_NON_EXISTENT                                = (TAF_CS_CAUSE_CC_NW_SECTION_BEGIN + 97),  /*  97. Msg type non-existent or not implemented   */
    TAF_CS_CAUSE_CC_NW_MESSAGE_TYPE_NOT_COMPATIBLE_WITH_PROT_STATE              = (TAF_CS_CAUSE_CC_NW_SECTION_BEGIN + 98),  /*  98. Msg type not compatible with protocol state*/
    TAF_CS_CAUSE_CC_NW_IE_NON_EXISTENT_OR_NOT_IMPLEMENTED                       = (TAF_CS_CAUSE_CC_NW_SECTION_BEGIN + 99),  /*  99. IE non-existent or not implemented         */
    TAF_CS_CAUSE_CC_NW_CONDITIONAL_IE_ERROR                                     = (TAF_CS_CAUSE_CC_NW_SECTION_BEGIN + 100), /* 100. Conditional IE error                       */
    TAF_CS_CAUSE_CC_NW_MESSAGE_NOT_COMPATIBLE_WITH_PROTOCOL_STATE               = (TAF_CS_CAUSE_CC_NW_SECTION_BEGIN + 101), /* 101. Message not compatible with protocol state */
    TAF_CS_CAUSE_CC_NW_RECOVERY_ON_TIMER_EXPIRY                                 = (TAF_CS_CAUSE_CC_NW_SECTION_BEGIN + 102), /* 102. Recovery on timer expiry Timer number      */
    TAF_CS_CAUSE_CC_NW_PROTOCOL_ERROR_UNSPECIFIED                               = (TAF_CS_CAUSE_CC_NW_SECTION_BEGIN + 111), /* 111. Protocol error, unspecified                */
    TAF_CS_CAUSE_CC_NW_INTERWORKING_UNSPECIFIED                                 = (TAF_CS_CAUSE_CC_NW_SECTION_BEGIN + 127), /* 127. Interworking, unspecified                  */

    /* CM SERVICE REJECT */
    TAF_CS_CAUSE_CM_SRV_REJ_IMSI_UNKNOWN_IN_HLR                                 = (TAF_CS_CAUSE_CM_SRV_REJ_BEGIN + 2),
    TAF_CS_CAUSE_CM_SRV_REJ_ILLEGAL_MS                                          = (TAF_CS_CAUSE_CM_SRV_REJ_BEGIN + 3),
    TAF_CS_CAUSE_CM_SRV_REJ_IMSI_UNKNOWN_IN_VLR                                 = (TAF_CS_CAUSE_CM_SRV_REJ_BEGIN + 4),
    TAF_CS_CAUSE_CM_SRV_REJ_IMEI_NOT_ACCEPTED                                   = (TAF_CS_CAUSE_CM_SRV_REJ_BEGIN + 5),
    TAF_CS_CAUSE_CM_SRV_REJ_ILLEGAL_ME                                          = (TAF_CS_CAUSE_CM_SRV_REJ_BEGIN + 6),
    TAF_CS_CAUSE_CM_SRV_REJ_PLMN_NOT_ALLOWED                                    = (TAF_CS_CAUSE_CM_SRV_REJ_BEGIN + 11),
    TAF_CS_CAUSE_CM_SRV_REJ_LOCATION_AREA_NOT_ALLOWED                           = (TAF_CS_CAUSE_CM_SRV_REJ_BEGIN + 12),
    TAF_CS_CAUSE_CM_SRV_REJ_ROAMING_NOT_ALLOWED_IN_THIS_LOCATION_AREA           = (TAF_CS_CAUSE_CM_SRV_REJ_BEGIN + 13),
    TAF_CS_CAUSE_CM_SRV_REJ_NO_SUITABLE_CELLS_IN_LOCATION_AREA                  = (TAF_CS_CAUSE_CM_SRV_REJ_BEGIN + 15),
    TAF_CS_CAUSE_CM_SRV_REJ_NETWORK_FAILURE                                     = (TAF_CS_CAUSE_CM_SRV_REJ_BEGIN + 17),
    TAF_CS_CAUSE_CM_SRV_REJ_MAC_FAILURE                                         = (TAF_CS_CAUSE_CM_SRV_REJ_BEGIN + 20),
    TAF_CS_CAUSE_CM_SRV_REJ_SYNCH_FAILURE                                       = (TAF_CS_CAUSE_CM_SRV_REJ_BEGIN + 21),
    TAF_CS_CAUSE_CM_SRV_REJ_CONGESTION                                          = (TAF_CS_CAUSE_CM_SRV_REJ_BEGIN + 22),
    TAF_CS_CAUSE_CM_SRV_REJ_GSM_AUTHENTICATION_UNACCEPTABLE                     = (TAF_CS_CAUSE_CM_SRV_REJ_BEGIN + 23),
    TAF_CS_CAUSE_CM_SRV_REJ_NOT_AUTHORIZED_FOR_THIS_CSG                         = (TAF_CS_CAUSE_CM_SRV_REJ_BEGIN + 25),
    TAF_CS_CAUSE_CM_SRV_REJ_SERVICE_OPTION_NOT_SUPPORTED                        = (TAF_CS_CAUSE_CM_SRV_REJ_BEGIN + 32),
    TAF_CS_CAUSE_CM_SRV_REJ_REQUESTED_SERVICE_OPTION_NOT_SUBSCRIBED             = (TAF_CS_CAUSE_CM_SRV_REJ_BEGIN + 33),
    TAF_CS_CAUSE_CM_SRV_REJ_SERVICE_OPTION_TEMPORARILY_OUT_OF_ORDER             = (TAF_CS_CAUSE_CM_SRV_REJ_BEGIN + 34),
    TAF_CS_CAUSE_CM_SRV_REJ_CALL_CANNOT_BE_IDENTIFIED                           = (TAF_CS_CAUSE_CM_SRV_REJ_BEGIN + 38),
    TAF_CS_CAUSE_CM_SRV_REJ_RETRY_UPON_ENTRY_INTO_A_NEW_CELL_0                  = (TAF_CS_CAUSE_CM_SRV_REJ_BEGIN + 48),
    TAF_CS_CAUSE_CM_SRV_REJ_RETRY_UPON_ENTRY_INTO_A_NEW_CELL_1                  = (TAF_CS_CAUSE_CM_SRV_REJ_BEGIN + 49),
    TAF_CS_CAUSE_CM_SRV_REJ_RETRY_UPON_ENTRY_INTO_A_NEW_CELL_2                  = (TAF_CS_CAUSE_CM_SRV_REJ_BEGIN + 50),
    TAF_CS_CAUSE_CM_SRV_REJ_RETRY_UPON_ENTRY_INTO_A_NEW_CELL_3                  = (TAF_CS_CAUSE_CM_SRV_REJ_BEGIN + 51),
    TAF_CS_CAUSE_CM_SRV_REJ_RETRY_UPON_ENTRY_INTO_A_NEW_CELL_4                  = (TAF_CS_CAUSE_CM_SRV_REJ_BEGIN + 52),
    TAF_CS_CAUSE_CM_SRV_REJ_RETRY_UPON_ENTRY_INTO_A_NEW_CELL_5                  = (TAF_CS_CAUSE_CM_SRV_REJ_BEGIN + 53),
    TAF_CS_CAUSE_CM_SRV_REJ_RETRY_UPON_ENTRY_INTO_A_NEW_CELL_6                  = (TAF_CS_CAUSE_CM_SRV_REJ_BEGIN + 54),
    TAF_CS_CAUSE_CM_SRV_REJ_RETRY_UPON_ENTRY_INTO_A_NEW_CELL_7                  = (TAF_CS_CAUSE_CM_SRV_REJ_BEGIN + 55),
    TAF_CS_CAUSE_CM_SRV_REJ_RETRY_UPON_ENTRY_INTO_A_NEW_CELL_8                  = (TAF_CS_CAUSE_CM_SRV_REJ_BEGIN + 56),
    TAF_CS_CAUSE_CM_SRV_REJ_RETRY_UPON_ENTRY_INTO_A_NEW_CELL_9                  = (TAF_CS_CAUSE_CM_SRV_REJ_BEGIN + 57),
    TAF_CS_CAUSE_CM_SRV_REJ_RETRY_UPON_ENTRY_INTO_A_NEW_CELL_10                 = (TAF_CS_CAUSE_CM_SRV_REJ_BEGIN + 58),
    TAF_CS_CAUSE_CM_SRV_REJ_RETRY_UPON_ENTRY_INTO_A_NEW_CELL_11                 = (TAF_CS_CAUSE_CM_SRV_REJ_BEGIN + 59),
    TAF_CS_CAUSE_CM_SRV_REJ_RETRY_UPON_ENTRY_INTO_A_NEW_CELL_12                 = (TAF_CS_CAUSE_CM_SRV_REJ_BEGIN + 60),
    TAF_CS_CAUSE_CM_SRV_REJ_RETRY_UPON_ENTRY_INTO_A_NEW_CELL_13                 = (TAF_CS_CAUSE_CM_SRV_REJ_BEGIN + 61),
    TAF_CS_CAUSE_CM_SRV_REJ_RETRY_UPON_ENTRY_INTO_A_NEW_CELL_14                 = (TAF_CS_CAUSE_CM_SRV_REJ_BEGIN + 62),
    TAF_CS_CAUSE_CM_SRV_REJ_RETRY_UPON_ENTRY_INTO_A_NEW_CELL_15                 = (TAF_CS_CAUSE_CM_SRV_REJ_BEGIN + 63),
    TAF_CS_CAUSE_CM_SRV_REJ_SEMANTICALLY_INCORRECT_MESSAGE                      = (TAF_CS_CAUSE_CM_SRV_REJ_BEGIN + 95),
    TAF_CS_CAUSE_CM_SRV_REJ_INVALID_MANDATORY_INFORMATION                       = (TAF_CS_CAUSE_CM_SRV_REJ_BEGIN + 96),
    TAF_CS_CAUSE_CM_SRV_REJ_MESSAGE_TYPE_NONEXISTENT_OR_NOT_IMPLEMENTED         = (TAF_CS_CAUSE_CM_SRV_REJ_BEGIN + 97),
    TAF_CS_CAUSE_CM_SRV_REJ_MESSAGE_TYPE_NOT_COMPATIBLE_WITH_THE_PROTOCAL_STATE = (TAF_CS_CAUSE_CM_SRV_REJ_BEGIN + 98),
    TAF_CS_CAUSE_CM_SRV_REJ_INFOMATION_ELEMENT_NONEXISTENT_OR_NOT_IMPLEMENTED   = (TAF_CS_CAUSE_CM_SRV_REJ_BEGIN + 99),
    TAF_CS_CAUSE_CM_SRV_REJ_CONDITIONAL_IE_ERROR                                = (TAF_CS_CAUSE_CM_SRV_REJ_BEGIN + 100),
    TAF_CS_CAUSE_CM_SRV_REJ_MESSAGE_NOT_COMPATIBLE_WITH_THE_PROTOCOL_STATE      = (TAF_CS_CAUSE_CM_SRV_REJ_BEGIN + 101),
    TAF_CS_CAUSE_CM_SRV_REJ_PROTOCOL_ERROR_UNSPECIFIED                          = (TAF_CS_CAUSE_CM_SRV_REJ_BEGIN + 111),

    TAF_CS_CAUSE_CM_SRV_REJ_CONGESTION_WITH_T3246                               = (TAF_CS_CAUSE_CM_SRV_REJ_BEGIN + 127),

    /* CSFB SERVICE REJECT */
    TAF_CS_CAUSE_CSFB_SRV_REJ_ILLEGAL_UE                                        = (TAF_CS_CAUSE_CSFB_SRV_REJ_BEGIN + 3),
    TAF_CS_CAUSE_CSFB_SRV_REJ_ILLEGAL_ME                                        = (TAF_CS_CAUSE_CSFB_SRV_REJ_BEGIN + 6),
    TAF_CS_CAUSE_CSFB_SRV_REJ_EPS_SERVICES_NOT_ALLOWED                          = (TAF_CS_CAUSE_CSFB_SRV_REJ_BEGIN + 7),
    TAF_CS_CAUSE_CSFB_SRV_REJ_EPS_SERVICES_AND_NONEPS_SERVICES_NOT_ALLOWED      = (TAF_CS_CAUSE_CSFB_SRV_REJ_BEGIN + 8),
    TAF_CS_CAUSE_CSFB_SRV_REJ_UE_IDENTITY_CANNOT_BE_DERIVED_BY_NW               = (TAF_CS_CAUSE_CSFB_SRV_REJ_BEGIN + 9),
    TAF_CS_CAUSE_CSFB_SRV_REJ_IMPLICITLY_DETACHED                               = (TAF_CS_CAUSE_CSFB_SRV_REJ_BEGIN + 10),
    TAF_CS_CAUSE_CSFB_SRV_REJ_PLMN_NOT_ALLOWED                                  = (TAF_CS_CAUSE_CSFB_SRV_REJ_BEGIN + 11),
    TAF_CS_CAUSE_CSFB_SRV_REJ_TRACKING_AREA_NOT_ALLOWED                         = (TAF_CS_CAUSE_CSFB_SRV_REJ_BEGIN + 12),
    TAF_CS_CAUSE_CSFB_SRV_REJ_ROAMING_NOT_ALLOWED_IN_THIS_TA                    = (TAF_CS_CAUSE_CSFB_SRV_REJ_BEGIN + 13),
    TAF_CS_CAUSE_CSFB_SRV_REJ_NO_SUITABLE_CELLS_IN_TRACKING_AREA                = (TAF_CS_CAUSE_CSFB_SRV_REJ_BEGIN + 15),
    TAF_CS_CAUSE_CSFB_SRV_REJ_CS_DOMAIN_NOT_AVAILABLE                           = (TAF_CS_CAUSE_CSFB_SRV_REJ_BEGIN + 18),
    TAF_CS_CAUSE_CSFB_SRV_REJ_CONGESTION                                        = (TAF_CS_CAUSE_CSFB_SRV_REJ_BEGIN + 22),
    TAF_CS_CAUSE_CSFB_SRV_REJ_NOT_AUTHORIZED_FOR_THIS_CSG                       = (TAF_CS_CAUSE_CSFB_SRV_REJ_BEGIN + 25),
    TAF_CS_CAUSE_CSFB_SRV_REJ_REQ_SER_OPTION_NOT_AUTHORIZED_IN_PLMN             = (TAF_CS_CAUSE_CSFB_SRV_REJ_BEGIN + 35),
    TAF_CS_CAUSE_CSFB_SRV_REJ_CS_SERVICE_TEMPORARILY_NOT_AVAILABLE              = (TAF_CS_CAUSE_CSFB_SRV_REJ_BEGIN + 39),
    TAF_CS_CAUSE_CSFB_SRV_REJ_NO_EPS_BEARER_CONTEXT_ACTIVATED                   = (TAF_CS_CAUSE_CSFB_SRV_REJ_BEGIN + 40),
    TAF_CS_CAUSE_CSFB_SRV_REJ_SEVERE_NETWORK_FAILURE                            = (TAF_CS_CAUSE_CSFB_SRV_REJ_BEGIN + 42),


    /* AS rr connection fail  */
    TAF_CS_CAUSE_RR_CONN_FAIL_CONGESTION                            = (TAF_CS_CAUSE_RR_CONN_FAIL_BEGIN + 1),
    TAF_CS_CAUSE_RR_CONN_FAIL_UNSPECIFIED                           = (TAF_CS_CAUSE_RR_CONN_FAIL_BEGIN + 2),
    TAF_CS_CAUSE_RR_CONN_FAIL_ACCESS_BAR                            = (TAF_CS_CAUSE_RR_CONN_FAIL_BEGIN + 3),
    TAF_CS_CAUSE_RR_CONN_FAIL_EST_CONN_FAIL                         = (TAF_CS_CAUSE_RR_CONN_FAIL_BEGIN + 4),
    TAF_CS_CAUSE_RR_CONN_FAIL_IMMEDIATE_ASSIGN_REJECT               = (TAF_CS_CAUSE_RR_CONN_FAIL_BEGIN + 5),
    TAF_CS_CAUSE_RR_CONN_FAIL_RANDOM_ACCESS_REJECT                  = (TAF_CS_CAUSE_RR_CONN_FAIL_BEGIN + 6),
    TAF_CS_CAUSE_RR_CONN_FAIL_T3122_RUNING                          = (TAF_CS_CAUSE_RR_CONN_FAIL_BEGIN + 7),
    TAF_CS_CAUSE_RR_CONN_FAIL_NO_RF                                 = (TAF_CS_CAUSE_RR_CONN_FAIL_BEGIN + 8),
    TAF_CS_CAUSE_RR_CONN_FAIL_LOW_LEVEL_SEARCHING_NETWORK           = (TAF_CS_CAUSE_RR_CONN_FAIL_BEGIN + 9),
    TAF_CS_CAUSE_RR_CONN_FAIL_RANDOM_ACCESS_SEND_FAIL               = (TAF_CS_CAUSE_RR_CONN_FAIL_BEGIN + 10),
    TAF_CS_CAUSE_RR_CONN_FAIL_NO_VALID_INFO                         = (TAF_CS_CAUSE_RR_CONN_FAIL_BEGIN + 11),
    TAF_CS_CAUSE_RR_CONN_FAIL_UE_NOT_ALLOW                          = (TAF_CS_CAUSE_RR_CONN_FAIL_BEGIN + 12),
    TAF_CS_CAUSE_RR_CONN_FAIL_TIME_OUT                              = (TAF_CS_CAUSE_RR_CONN_FAIL_BEGIN + 13),
    TAF_CS_CAUSE_RR_CONN_FAIL_NO_RANDOM_ACCESS_RESOURCE             = (TAF_CS_CAUSE_RR_CONN_FAIL_BEGIN + 14),
    TAF_CS_CAUSE_RR_CONN_FAIL_INVALID_IMMEDIATE_ASSIGNED_MSG        = (TAF_CS_CAUSE_RR_CONN_FAIL_BEGIN + 15),
    TAF_CS_CAUSE_RR_CONN_FAIL_ACTIVE_PHYSICAL_CHANNEL_FAIL          = (TAF_CS_CAUSE_RR_CONN_FAIL_BEGIN + 16),
    TAF_CS_CAUSE_RR_CONN_FAIL_AIR_MSG_DECODE_ERROR                  = (TAF_CS_CAUSE_RR_CONN_FAIL_BEGIN + 17),
    TAF_CS_CAUSE_RR_CONN_FAIL_CURRENT_PROTOCOL_NOT_SUPPORT          = (TAF_CS_CAUSE_RR_CONN_FAIL_BEGIN + 18),
    TAF_CS_CAUSE_RR_CONN_FAIL_INVALID_UE_STATE                      = (TAF_CS_CAUSE_RR_CONN_FAIL_BEGIN + 19),
    TAF_CS_CAUSE_RR_CONN_FAIL_CELL_BARRED                           = (TAF_CS_CAUSE_RR_CONN_FAIL_BEGIN + 20),
    TAF_CS_CAUSE_RR_CONN_FAIL_FAST_RETURN_TO_LTE                    = (TAF_CS_CAUSE_RR_CONN_FAIL_BEGIN + 21),
    TAF_CS_CAUSE_RR_CONN_FAIL_RA_FAIL_NO_VALID_INFO                 = (TAF_CS_CAUSE_RR_CONN_FAIL_BEGIN + 22),
    TAF_CS_CAUSE_RR_CONN_FAIL_RJ_INTER_RAT                          = (TAF_CS_CAUSE_RR_CONN_FAIL_BEGIN + 23),
    TAF_CS_CAUSE_RR_CONN_FAIL_CSFB_NO_RF                            = (TAF_CS_CAUSE_RR_CONN_FAIL_BEGIN + 24),

    /* AS Rel ind */
    TAF_CS_CAUSE_RR_REL_AUTH_REJ                                    = (TAF_CS_CAUSE_RR_REL_BEGIN + 0),
    TAF_CS_CAUSE_RR_REL_NORMAL_EVENT                                = (TAF_CS_CAUSE_RR_REL_BEGIN + 1),
    TAF_CS_CAUSE_RR_REL_NORMAL_UNSPECIFIED                          = (TAF_CS_CAUSE_RR_REL_BEGIN + 2),
    TAF_CS_CAUSE_RR_REL_PREEMPTIVE_RELEASE                          = (TAF_CS_CAUSE_RR_REL_BEGIN + 3),
    TAF_CS_CAUSE_RR_REL_CONGESTION                                  = (TAF_CS_CAUSE_RR_REL_BEGIN + 4),
    TAF_CS_CAUSE_RR_REL_REEST_REJ                                   = (TAF_CS_CAUSE_RR_REL_BEGIN + 5),
    TAF_CS_CAUSE_RR_REL_DIRECTED_SIGNAL_CONN_REEST                  = (TAF_CS_CAUSE_RR_REL_BEGIN + 6),
    TAF_CS_CAUSE_RR_REL_USER_INACTIVE                               = (TAF_CS_CAUSE_RR_REL_BEGIN + 7),
    TAF_CS_CAUSE_RR_REL_UTRAN_RELEASE                               = (TAF_CS_CAUSE_RR_REL_BEGIN + 8),
    TAF_CS_CAUSE_RR_REL_RRC_ERROR                                   = (TAF_CS_CAUSE_RR_REL_BEGIN + 9),
    TAF_CS_CAUSE_RR_REL_RL_FAILURE                                  = (TAF_CS_CAUSE_RR_REL_BEGIN + 10),
    TAF_CS_CAUSE_RR_REL_OTHER_REASON                                = (TAF_CS_CAUSE_RR_REL_BEGIN + 11),
    TAF_CS_CAUSE_RR_REL_NO_RF                                       = (TAF_CS_CAUSE_RR_REL_BEGIN + 12),
    TAF_CS_CAUSE_RR_REL_RLC_ERR_OR                                  = (TAF_CS_CAUSE_RR_REL_BEGIN + 13),
    TAF_CS_CAUSE_RR_REL_CELL_UP_DATE_FAIL                           = (TAF_CS_CAUSE_RR_REL_BEGIN + 14),
    TAF_CS_CAUSE_RR_REL_NAS_REL_REQ                                 = (TAF_CS_CAUSE_RR_REL_BEGIN + 15),
    TAF_CS_CAUSE_RR_REL_CONN_FAIL                                   = (TAF_CS_CAUSE_RR_REL_BEGIN + 16),     /* redial */
    TAF_CS_CAUSE_RR_REL_NAS_DATA_ABSENT                             = (TAF_CS_CAUSE_RR_REL_BEGIN + 17),     /* redial */
    TAF_CS_CAUSE_RR_REL_T314_EXPIRED                                = (TAF_CS_CAUSE_RR_REL_BEGIN + 18),
    TAF_CS_CAUSE_RR_REL_W_RL_FAIL                                   = (TAF_CS_CAUSE_RR_REL_BEGIN + 19),

    TAF_CS_CAUSE_RR_REL_G_RL_FAIL                                   = (TAF_CS_CAUSE_RR_REL_BEGIN + 20),
    TAF_CS_CAUSE_RR_REL_OTHER_REASON_NO_EST_CNF                     = (TAF_CS_CAUSE_RR_REL_BEGIN + 21),

    /* CSFB LMM Error */
    TAF_CS_CAUSE_CSFB_LMM_FAIL_LMM_LOCAL_FAIL                       = (TAF_CS_CAUSE_CSFB_LMM_FAIL_BEGIN) + 0,
    TAF_CS_CAUSE_CSFB_LMM_FAIL_AUTH_REJ                             = (TAF_CS_CAUSE_CSFB_LMM_FAIL_BEGIN) + 1,
    TAF_CS_CAUSE_CSFB_LMM_FAIL_T3417_TIME_OUT                       = (TAF_CS_CAUSE_CSFB_LMM_FAIL_BEGIN) + 2,
    TAF_CS_CAUSE_CSFB_LMM_FAIL_APP_DETACH_SERVICE                   = (TAF_CS_CAUSE_CSFB_LMM_FAIL_BEGIN) + 3,
    TAF_CS_CAUSE_CSFB_LMM_FAIL_CN_DETACH_SERVICE                    = (TAF_CS_CAUSE_CSFB_LMM_FAIL_BEGIN) + 4,
    TAF_CS_CAUSE_CSFB_LMM_FAIL_FOR_OTHERS                           = (TAF_CS_CAUSE_CSFB_LMM_FAIL_BEGIN) + 5,
    TAF_CS_CAUSE_CSFB_LMM_FAIL_FOR_EMM_STATE                        = (TAF_CS_CAUSE_CSFB_LMM_FAIL_BEGIN) + 6,
    TAF_CS_CAUSE_CSFB_LMM_FAIL_SMS_ONLY                             = (TAF_CS_CAUSE_CSFB_LMM_FAIL_BEGIN) + 7,
    TAF_CS_CAUSE_CSFB_LMM_FAIL_PS_ONLY                              = (TAF_CS_CAUSE_CSFB_LMM_FAIL_BEGIN) + 8,
    TAF_CS_CAUSE_CSFB_LMM_FAIL_TAU_COLL_ABNORMAL                    = (TAF_CS_CAUSE_CSFB_LMM_FAIL_BEGIN) + 9,

    /* MM Inter Error */
    TAF_CS_CAUSE_MM_INTER_ERR_FORB_LA                               = (TAF_CS_CAUSE_MM_INTER_ERR_BEGIN + 0),
    TAF_CS_CAUSE_MM_INTER_ERR_FORB_OTHER                            = (TAF_CS_CAUSE_MM_INTER_ERR_BEGIN + 1),
    TAF_CS_CAUSE_MM_INTER_ERR_CS_ACCESS_BAR                         = (TAF_CS_CAUSE_MM_INTER_ERR_BEGIN + 2),
    TAF_CS_CAUSE_MM_INTER_ERR_CS_DETACH                             = (TAF_CS_CAUSE_MM_INTER_ERR_BEGIN + 3),
    TAF_CS_CAUSE_MM_INTER_ERR_CS_SIM_INVALID                        = (TAF_CS_CAUSE_MM_INTER_ERR_BEGIN + 4),
    TAF_CS_CAUSE_MM_INTER_ERR_CS_SERVICE_CONGESTION                 = (TAF_CS_CAUSE_MM_INTER_ERR_BEGIN + 5),
    TAF_CS_CAUSE_MM_INTER_ERR_T3230_TIMER_OUT                       = (TAF_CS_CAUSE_MM_INTER_ERR_BEGIN + 6),
    TAF_CS_CAUSE_MM_INTER_ERR_SEARCHING_NETWORK                     = (TAF_CS_CAUSE_MM_INTER_ERR_BEGIN + 7),
    TAF_CS_CAUSE_MM_INTER_ERR_INTER_RAT_SYSTEM_CHANGE               = (TAF_CS_CAUSE_MM_INTER_ERR_BEGIN + 8),
    TAF_CS_CAUSE_MM_INTER_ERR_NOT_SUPPORT_CS_CALL_S1_MODE_ONLY      = (TAF_CS_CAUSE_MM_INTER_ERR_BEGIN + 9),
    TAF_CS_CAUSE_MM_INTER_ERR_OUT_OF_COVERAGE                       = (TAF_CS_CAUSE_MM_INTER_ERR_BEGIN + 10),
    TAF_CS_CAUSE_MM_INTER_ERR_ATTEMPTING_TO_UPDATE                  = (TAF_CS_CAUSE_MM_INTER_ERR_BEGIN + 11),
    TAF_CS_CAUSE_MM_INTER_ERR_CSFB_ALREADY_EXISTS                   = (TAF_CS_CAUSE_MM_INTER_ERR_BEGIN + 12),
    TAF_CS_CAUSE_MM_INTER_ERR_RESUME_TO_GU_FAIL                     = (TAF_CS_CAUSE_MM_INTER_ERR_BEGIN + 13),
    TAF_CS_CAUSE_MM_INTER_ERR_TI_INVALID                            = (TAF_CS_CAUSE_MM_INTER_ERR_BEGIN + 14),
    TAF_CS_CAUSE_MM_INTER_ERR_WAIT_EST_CNF_TIME_OUT                 = (TAF_CS_CAUSE_MM_INTER_ERR_BEGIN + 15),   /* redial */
    TAF_CS_CAUSE_MM_INTER_ERR_CC_CONN_REQ_EXIST                     = (TAF_CS_CAUSE_MM_INTER_ERR_BEGIN + 16),
    TAF_CS_CAUSE_MM_INTER_ERR_UE_INVALID_STATE                      = (TAF_CS_CAUSE_MM_INTER_ERR_BEGIN + 17),
    TAF_CS_CAUSE_MM_INTER_ERR_WAIT_CC_REEST_TIME_OUT                = (TAF_CS_CAUSE_MM_INTER_ERR_BEGIN + 18),
    TAF_CS_CAUSE_MM_INTER_ERR_BACK_TO_LTE                           = (TAF_CS_CAUSE_MM_INTER_ERR_BEGIN + 19),
    TAF_CS_CAUSE_MM_INTER_ERR_RESUME_TO_EHRPD                       = (TAF_CS_CAUSE_MM_INTER_ERR_BEGIN + 20),
    TAF_CS_CAUSE_MM_INTER_ERR_SND_SAPI3_FAIL                        = (TAF_CS_CAUSE_MM_INTER_ERR_BEGIN + 21),   /* ����ҵ��SAPI3����ʧ��,���ز� */
    TAF_CS_CAUSE_MM_INTER_ERR_EST_SAPI3_FAIL                        = (TAF_CS_CAUSE_MM_INTER_ERR_BEGIN + 22),   /* ����ҵ��SAPI3����ʱGAS�ظ�ʧ��,�ز� */
    TAF_CS_CAUSE_MM_INTER_ERR_ECALL_INACTIVE                        = (TAF_CS_CAUSE_MM_INTER_ERR_BEGIN + 23),   /* eCall Inactive״̬,���ز� */
    TAF_CS_CAUSE_MM_INTER_ERR_REEST_FAIL                            = (TAF_CS_CAUSE_MM_INTER_ERR_BEGIN + 24),   /* MM reest״̬�յ�est_cnf(ʧ��)��rel ind,���ز� */
    TAF_CS_CAUSE_MM_INTER_ERR_CC_REL_REQ                            = (TAF_CS_CAUSE_MM_INTER_ERR_BEGIN + 25),
    TAF_CS_CAUSE_MM_INTER_ERR_LTE_LIMITED_SERVICE                   = (TAF_CS_CAUSE_MM_INTER_ERR_BEGIN + 26),


    /* ƽ̨�ڲ��Ĵ���ԭ��ֵ */
    TAF_CS_CAUSE_INVALID_PARAMETER                                  = (TAF_CS_CAUSE_CALL_BEGIN + 1),            /* �������� */
    TAF_CS_CAUSE_SIM_NOT_EXIST                                      = (TAF_CS_CAUSE_CALL_BEGIN + 2),            /* USIM�������� */
    TAF_CS_CAUSE_SIM_PIN_NEED                                       = (TAF_CS_CAUSE_CALL_BEGIN + 3),            /* ��ҪSIM����PIN�� */
    TAF_CS_CAUSE_NO_CALL_ID                                         = (TAF_CS_CAUSE_CALL_BEGIN + 4),            /* Call Id ����ʧ�� */
    TAF_CS_CAUSE_NOT_ALLOW                                          = (TAF_CS_CAUSE_CALL_BEGIN + 5),            /* ���б���ֹ */
    TAF_CS_CAUSE_STATE_ERROR                                        = (TAF_CS_CAUSE_CALL_BEGIN + 6),            /* ��ǰ����״̬�쳣 */
    TAF_CS_CAUSE_FDN_CHECK_FAILURE                                  = (TAF_CS_CAUSE_CALL_BEGIN + 8),            /* FDN���ʧ�� */
    TAF_CS_CAUSE_CALL_CTRL_BEYOND_CAPABILITY                        = (TAF_CS_CAUSE_CALL_BEGIN + 9),            /* CALL CONTROLҵ���޸��˺��в�����UE��֧���޸ĺ�Ĳ��� */
    TAF_CS_CAUSE_CALL_CTRL_TIMEOUT                                  = (TAF_CS_CAUSE_CALL_BEGIN + 10),           /* CALL CONTROLҵ��ȴ�USIM����Ӧ��ʱ */
    TAF_CS_CAUSE_CALL_CTRL_NOT_ALLOWED                              = (TAF_CS_CAUSE_CALL_BEGIN + 11),           /* CALL CONTROLҵ��USIM��ֹ���� */
    TAF_CS_CAUSE_CALL_CTRL_INVALID_PARAMETER                        = (TAF_CS_CAUSE_CALL_BEGIN + 13),           /* CALL CTRLҵ�����ʧ�ܻ�����Ϣʧ�ܶ���Ϊ�������� */
    TAF_CS_CAUSE_DTMF_BUF_FULL                                      = (TAF_CS_CAUSE_CALL_BEGIN + 14),           /* DTMF�������� */
    TAF_CS_CAUSE_DTMF_REPEAT_STOP                                   = (TAF_CS_CAUSE_CALL_BEGIN + 15),           /* �ظ���STOP DTMF���� */
    TAF_CS_CAUSE_DTMF_REJ                                           = (TAF_CS_CAUSE_CALL_BEGIN + 16),           /* ����ܾ�START DTMF���� */
    TAF_CS_CAUSE_CALL_ON_HOLD                                       = (TAF_CS_CAUSE_CALL_BEGIN + 17),           /* ���б����� */
    TAF_CS_CAUSE_CALL_RELEASE                                       = (TAF_CS_CAUSE_CALL_BEGIN + 18),           /* �����ͷ� */
    TAF_CS_CAUSE_POWER_OFF                                          = (TAF_CS_CAUSE_CALL_BEGIN + 19),           /* ��ػ� */
    TAF_CS_CAUSE_NOT_IN_SPEECH_CALL                                 = (TAF_CS_CAUSE_CALL_BEGIN + 20),           /* ��ǰû�п���ͨ�� */
    TAF_CS_CAUSE_SIM_INVALID                                        = (TAF_CS_CAUSE_CALL_BEGIN + 21),           /* ����Ч */
    TAF_CS_CAUSE_DOMAIN_SELECTION_FAILURE                           = (TAF_CS_CAUSE_CALL_BEGIN + 22),           /* ҵ����ѡ��ʧ�� */
    TAF_CS_CAUSE_DOMAIN_SELECTION_TIMER_EXPIRED                     = (TAF_CS_CAUSE_CALL_BEGIN + 23),           /* ҵ����ѡ�񻺴泬ʱ */
    TAF_CS_CAUSE_MODEM_POWER_OFF                                    = (TAF_CS_CAUSE_CALL_BEGIN + 24),           /* MODEM�ػ� */
    TAF_CS_CAUSE_CS_NOT_SUPPORT_ENCRYPT                             = (TAF_CS_CAUSE_CALL_BEGIN + 25),           /* CS��֧���������� */

    TAF_CS_CAUSE_ECALL_MODE_NOT_ALLOWED                             = (TAF_CS_CAUSE_CALL_BEGIN + 26),           /* ECALL����ģʽ������ */
    TAF_CS_CAUSE_ECALL_CACHE_MSG_FAIL                               = (TAF_CS_CAUSE_CALL_BEGIN + 27),           /* ������Ϣʧ�� */
    TAF_CS_CAUSE_ECALL_LOWERPOWER_POWER_ON_FAIL                     = (TAF_CS_CAUSE_CALL_BEGIN + 28),           /* �±�״̬�¿���ʧ�� */
    TAF_CS_CAUSE_CALL_NUMBER_NOT_EXIST                              = (TAF_CS_CAUSE_CALL_BEGIN + 29),

    TAF_CS_CAUSE_NO_RF                                              = (TAF_CS_CAUSE_CALL_BEGIN + 30),

    /* CC Inter Error */
    TAF_CS_CAUSE_CC_INTER_ERR_T303_TIME_OUT                         = (TAF_CS_CAUSE_CALL_BEGIN + 31),
    TAF_CS_CAUSE_CC_INTER_ERR_T335_TIME_OUT                         = (TAF_CS_CAUSE_CALL_BEGIN + 32),
    TAF_CS_CAUSE_CC_INTER_ERR_WAIT_RAB_TIME_OUT                     = (TAF_CS_CAUSE_CALL_BEGIN + 33),
    TAF_CS_CAUSE_CC_INTER_ERR_NO_TCH                                = (TAF_CS_CAUSE_CALL_BEGIN + 34),

    TAF_CS_CAUSE_SYSCFG_MODE_CHANGE                                 = (TAF_CS_CAUSE_CALL_BEGIN + 35),
    TAF_CS_CAUSE_LTE_ONLY_BAND12                                    = (TAF_CS_CAUSE_CALL_BEGIN + 36),

    TAF_CS_CAUSE_CALL_SRV_ACQ_FAIL                                  = (TAF_CS_CAUSE_CALL_BEGIN + 37),

    TAF_CS_CAUSE_EMC_EXIST_OPT_NOT_ALLOW                            = (TAF_CS_CAUSE_CALL_BEGIN + 38),           /* �������д��� */
    TAF_CS_CAUSE_CS_CALL_EXIST_OPT_NOT_ALLOW                        = (TAF_CS_CAUSE_CALL_BEGIN + 39),           /* CS���д��� */
    TAF_CS_CAUSE_IMS_CALL_EXIST_OPT_NOT_ALLOW                       = (TAF_CS_CAUSE_CALL_BEGIN + 40),           /* IMS���д��� */
    TAF_CS_CAUSE_NV_NOT_SUPPORT_OPT_NOT_ALLOW                       = (TAF_CS_CAUSE_CALL_BEGIN + 41),           /* NV���ò�֧�ֺ��� */
    TAF_CS_CAUSE_NEW_CALL_STATE_INCORRECT_OPT_NOT_ALLOW             = (TAF_CS_CAUSE_CALL_BEGIN + 42),           /* �·����CALL״̬���� */
    TAF_CS_CAUSE_ECALL_SYS_MODE_NOT_ALLOW                           = (TAF_CS_CAUSE_CALL_BEGIN + 43),           /* ECALL��ϵͳģʽ���� */
    TAF_CS_CAUSE_ECALL_CALL_MODE_NOT_ALLOW                          = (TAF_CS_CAUSE_CALL_BEGIN + 44),           /* ECALL�ĺ���ģʽ���� */
    TAF_CS_CAUSE_ECONF_CALL_TYPE_NOT_ALLOW                          = (TAF_CS_CAUSE_CALL_BEGIN + 45),           /* ��ǿ��ͨ���ĺ������Ͳ��� */
    TAF_CS_CAUSE_REPEAT_ASYNC_ANS_NOT_ALLOW                         = (TAF_CS_CAUSE_CALL_BEGIN + 46),           /* �ظ��첽���� */

    TAF_CS_CAUSE_CC_INTER_ERR_T310_TIME_OUT                         = (TAF_CS_CAUSE_CALL_BEGIN + 47),

    TAF_CS_CAUSE_CALL_SRV_ACQ_FAIL_NO_RF                            = (TAF_CS_CAUSE_CALL_BEGIN + 48),

    TAF_CS_CAUSE_UNKNOWN                                            = (TAF_CS_CAUSE_CALL_BEGIN + 0xFF),         /* δ֪���� */



    /* VC Error */
    TAF_CS_CAUSE_VC_ERR_STARTED                                     =  (TAF_CS_CAUSE_VC_BEGIN + 1),             /* HIFI�Ѿ����� */
    TAF_CS_CAUSE_VC_ERR_PORT_CFG_FAIL                               =  (TAF_CS_CAUSE_VC_BEGIN + 2),             /* �˿�����ʧ�� */
    TAF_CS_CAUSE_VC_ERR_SET_DEVICE_FAIL                             =  (TAF_CS_CAUSE_VC_BEGIN + 3),             /* ����Deviceʧ�� */
    TAF_CS_CAUSE_VC_ERR_SET_START_FAIL                              =  (TAF_CS_CAUSE_VC_BEGIN + 4),             /* startʧ�� */
    TAF_CS_CAUSE_VC_ERR_SET_VOLUME_FAIL                             =  (TAF_CS_CAUSE_VC_BEGIN + 5),             /* ��������ʧ�� */
    TAF_CS_CAUSE_VC_ERR_SAMPLE_RATE_FAIL                            =  (TAF_CS_CAUSE_VC_BEGIN + 6),             /* ���ʲ���ʧ�� */
    TAF_CS_CAUSE_VC_ERR_TI_START_EXPIRED                            =  (TAF_CS_CAUSE_VC_BEGIN + 7),             /* start������ʱ����ʱ */

    /* IMS�Ĵ���ԭ��ֵ */
    TAF_CS_CAUSE_IMS_OK                                             = (TAF_CS_CAUSE_IMS_BEGIN + 200),
    TAF_CS_CAUSE_IMS_MULTIPLE_CHOICES                               = (TAF_CS_CAUSE_IMS_BEGIN + 300),
    TAF_CS_CAUSE_IMS_MOVED_PERMANENTLY                              = (TAF_CS_CAUSE_IMS_BEGIN + 301),
    TAF_CS_CAUSE_IMS_MOVED_TEMPORARILY                              = (TAF_CS_CAUSE_IMS_BEGIN + 302),
    TAF_CS_CAUSE_IMS_USE_PROXY                                      = (TAF_CS_CAUSE_IMS_BEGIN + 305),
    TAF_CS_CAUSE_IMS_ALTERNATIVE_SERVICE                            = (TAF_CS_CAUSE_IMS_BEGIN + 380),
    TAF_CS_CAUSE_IMS_BAD_REQUEST                                    = (TAF_CS_CAUSE_IMS_BEGIN + 400),
    TAF_CS_CAUSE_IMS_UNAUTHORIZED                                   = (TAF_CS_CAUSE_IMS_BEGIN + 401),
    TAF_CS_CAUSE_IMS_PAYMENT_REQUIRED                               = (TAF_CS_CAUSE_IMS_BEGIN + 402),
    TAF_CS_CAUSE_IMS_FORBIDDEN                                      = (TAF_CS_CAUSE_IMS_BEGIN + 403),
    TAF_CS_CAUSE_IMS_NOT_FOUND                                      = (TAF_CS_CAUSE_IMS_BEGIN + 404),
    TAF_CS_CAUSE_IMS_METHOD_NOT_ALLOWED                             = (TAF_CS_CAUSE_IMS_BEGIN + 405),
    TAF_CS_CAUSE_IMS_NOT_ACCEPTABLE                                 = (TAF_CS_CAUSE_IMS_BEGIN + 406),
    TAF_CS_CAUSE_IMS_PROXY_AUTHENTICATION_REQUIRED                  = (TAF_CS_CAUSE_IMS_BEGIN + 407),
    TAF_CS_CAUSE_IMS_REQUEST_TIMEOUT                                = (TAF_CS_CAUSE_IMS_BEGIN + 408),
    TAF_CS_CAUSE_IMS_CONFLICT_DEPRECATED                            = (TAF_CS_CAUSE_IMS_BEGIN + 409),
    TAF_CS_CAUSE_IMS_GONE                                           = (TAF_CS_CAUSE_IMS_BEGIN + 410),
    TAF_CS_CAUSE_IMS_CONDITIONAL_REQUEST_FAILED                     = (TAF_CS_CAUSE_IMS_BEGIN + 412),
    TAF_CS_CAUSE_IMS_REQUEST_ENTITY_TOO_LARGE                       = (TAF_CS_CAUSE_IMS_BEGIN + 413),
    TAF_CS_CAUSE_IMS_REQUEST_URI_TOO_LONG                           = (TAF_CS_CAUSE_IMS_BEGIN + 414),
    TAF_CS_CAUSE_IMS_UNSUPPORTED_MEDIA_TYPE                         = (TAF_CS_CAUSE_IMS_BEGIN + 415),
    TAF_CS_CAUSE_IMS_UNSUPPORTED_URI_SCHEME                         = (TAF_CS_CAUSE_IMS_BEGIN + 416),
    TAF_CS_CAUSE_IMS_UNKNOWN_RESOURCE_PRIORITY                      = (TAF_CS_CAUSE_IMS_BEGIN + 417),
    TAF_CS_CAUSE_IMS_BAD_EXTENSION                                  = (TAF_CS_CAUSE_IMS_BEGIN + 420),
    TAF_CS_CAUSE_IMS_EXTENSION_REQUIRED                             = (TAF_CS_CAUSE_IMS_BEGIN + 421),
    TAF_CS_CAUSE_IMS_SESSION_INTERVAL_TOO_SMALL                     = (TAF_CS_CAUSE_IMS_BEGIN + 422),
    TAF_CS_CAUSE_IMS_INTERVAL_TOO_BRIEF                             = (TAF_CS_CAUSE_IMS_BEGIN + 423),
    TAF_CS_CAUSE_IMS_BAD_LOCATION_INFORMATION                       = (TAF_CS_CAUSE_IMS_BEGIN + 424),
    TAF_CS_CAUSE_IMS_USE_IDENTITY_HEADER                            = (TAF_CS_CAUSE_IMS_BEGIN + 428),
    TAF_CS_CAUSE_IMS_PROVIDE_REFERRER_IDENTITY                      = (TAF_CS_CAUSE_IMS_BEGIN + 429),
    TAF_CS_CAUSE_IMS_FLOW_FAILED                                    = (TAF_CS_CAUSE_IMS_BEGIN + 430),
    TAF_CS_CAUSE_IMS_ANONYMITY_DISALLOWED                           = (TAF_CS_CAUSE_IMS_BEGIN + 433),
    TAF_CS_CAUSE_IMS_BAD_IDENTITY_INFO                              = (TAF_CS_CAUSE_IMS_BEGIN + 436),
    TAF_CS_CAUSE_IMS_UNSUPPORTED_CERTIFICATE                        = (TAF_CS_CAUSE_IMS_BEGIN + 437),
    TAF_CS_CAUSE_IMS_INVALID_IDENTITY_HEADER                        = (TAF_CS_CAUSE_IMS_BEGIN + 438),
    TAF_CS_CAUSE_IMS_FIRST_HOP_LACKS_OUTBOUND_SUPPORT               = (TAF_CS_CAUSE_IMS_BEGIN + 439),
    TAF_CS_CAUSE_IMS_MAX_BREADTH_EXCEEDED                           = (TAF_CS_CAUSE_IMS_BEGIN + 440),
    TAF_CS_CAUSE_IMS_BAD_INFO_PACKEAGE                              = (TAF_CS_CAUSE_IMS_BEGIN + 469),
    TAF_CS_CAUSE_IMS_CONSENT_NEEDED                                 = (TAF_CS_CAUSE_IMS_BEGIN + 470),
    TAF_CS_CAUSE_IMS_TEMPORARILY_UNAVAILABLE                        = (TAF_CS_CAUSE_IMS_BEGIN + 480),
    TAF_CS_CAUSE_IMS_CALL_TRANSACTION_DOES_NOT_EXIST                = (TAF_CS_CAUSE_IMS_BEGIN + 481),
    TAF_CS_CAUSE_IMS_LOOP_DETECTED                                  = (TAF_CS_CAUSE_IMS_BEGIN + 482),
    TAF_CS_CAUSE_IMS_TOO_MANY_HOPS                                  = (TAF_CS_CAUSE_IMS_BEGIN + 483),
    TAF_CS_CAUSE_IMS_ADDRESS_INCOMPLETE                             = (TAF_CS_CAUSE_IMS_BEGIN + 484),
    TAF_CS_CAUSE_IMS_AMBIGUOUS                                      = (TAF_CS_CAUSE_IMS_BEGIN + 485),
    TAF_CS_CAUSE_IMS_BUSY_HERE                                      = (TAF_CS_CAUSE_IMS_BEGIN + 486),
    TAF_CS_CAUSE_IMS_REQUEST_TERMINATED                             = (TAF_CS_CAUSE_IMS_BEGIN + 487),
    TAF_CS_CAUSE_IMS_NOT_ACCEPTABLE_HERE                            = (TAF_CS_CAUSE_IMS_BEGIN + 488),
    TAF_CS_CAUSE_IMS_BAD_EVENT                                      = (TAF_CS_CAUSE_IMS_BEGIN + 489),
    TAF_CS_CAUSE_IMS_REQUEST_PENDING                                = (TAF_CS_CAUSE_IMS_BEGIN + 491),
    TAF_CS_CAUSE_IMS_UNDECIPHERABLE                                 = (TAF_CS_CAUSE_IMS_BEGIN + 493),
    TAF_CS_CAUSE_IMS_SECURITY_AGREEMENT_REQUIRED                    = (TAF_CS_CAUSE_IMS_BEGIN + 494),
    TAF_CS_CAUSE_IMS_SERVER_INTERNAL_ERROR                          = (TAF_CS_CAUSE_IMS_BEGIN + 500),
    TAF_CS_CAUSE_IMS_NOT_IMPLEMENTED                                = (TAF_CS_CAUSE_IMS_BEGIN + 501),
    TAF_CS_CAUSE_IMS_BAD_GATEWAY                                    = (TAF_CS_CAUSE_IMS_BEGIN + 502),
    TAF_CS_CAUSE_IMS_SERVICE_UNAVAILABLE                            = (TAF_CS_CAUSE_IMS_BEGIN + 503),
    TAF_CS_CAUSE_IMS_SERVER_TIME_OUT                                = (TAF_CS_CAUSE_IMS_BEGIN + 504),
    TAF_CS_CAUSE_IMS_VERSION_NOT_SUPPORTED                          = (TAF_CS_CAUSE_IMS_BEGIN + 505),
    TAF_CS_CAUSE_IMS_MESSAGE_TOO_LARGE                              = (TAF_CS_CAUSE_IMS_BEGIN + 513),
    TAF_CS_CAUSE_IMS_PRECONDITION_FAILURE                           = (TAF_CS_CAUSE_IMS_BEGIN + 580),
    TAF_CS_CAUSE_IMS_BUSY_EVERYWHERE                                = (TAF_CS_CAUSE_IMS_BEGIN + 600),
    TAF_CS_CAUSE_IMS_DECLINE                                        = (TAF_CS_CAUSE_IMS_BEGIN + 603),
    TAF_CS_CAUSE_IMS_DOES_NOT_EXIST_ANYWHERE                        = (TAF_CS_CAUSE_IMS_BEGIN + 604),
    TAF_CS_CAUSE_IMS_GLOBAL_NOT_ACCEPTABLE                          = (TAF_CS_CAUSE_IMS_BEGIN + 606),
    /* ����ԭ��ֵ, ��ԭ��ֵ���ڲ���ȥCS���ز�ʱ��IMS SIPԭ��ֵת�� */
    TAF_CS_CAUSE_IMS_OTHERS                                         = (TAF_CS_CAUSE_IMS_BEGIN + 699),

    /* IMSA�Ĵ���ԭ��ֵ */
    TAF_CS_CAUSE_IMSA_ERROR                                         = (TAF_CS_CAUSE_IMSA_BEGIN + 1),            /* һ����� */
    TAF_CS_CAUSE_IMSA_NOT_SUPPORTED_CMD                             = (TAF_CS_CAUSE_IMSA_BEGIN + 2),            /* IMSA��֧�ָ����� */
    TAF_CS_CAUSE_IMSA_ALLOC_ENTITY_FAIL                             = (TAF_CS_CAUSE_IMSA_BEGIN + 3),            /* IMSA�������ʵ��ʧ�� */
    TAF_CS_CAUSE_IMSA_RETRY_VIA_CS                                  = (TAF_CS_CAUSE_IMSA_BEGIN + 4),            /* ͨ��CS���ٳ��Դ˺��� */
    TAF_CS_CAUSE_IMSA_TIMEOUT                                       = (TAF_CS_CAUSE_IMSA_BEGIN + 5),            /* ����ִ�г�ʱ */
    TAF_CS_CAUSE_IMSA_NOT_SUPPORTED_CALL_TYPE                       = (TAF_CS_CAUSE_IMSA_BEGIN + 6),            /* ��֧�ֵĺ������� */
    TAF_CS_CAUSE_IMSA_SERVICE_NOT_AVAILABLE                         = (TAF_CS_CAUSE_IMSA_BEGIN + 7),            /* ���ж�Ӧ�ķ��񲻿��� */
    TAF_CS_CAUSE_IMSA_SRVCCING_BUFF_MSG_FAIL                        = (TAF_CS_CAUSE_IMSA_BEGIN + 8),            /* SRVCC�����л�����Ϣʧ�� */
    TAF_CS_CAUSE_IMSA_SRVCC_SUCC                                    = (TAF_CS_CAUSE_IMSA_BEGIN + 9),            /* SRVCC�ɹ� */
    TAF_CS_CAUSE_IMSA_SRVCC_ABNORMAL                                = (TAF_CS_CAUSE_IMSA_BEGIN + 10),           /* SRVCC�쳣������ػ��� */
    TAF_CS_CAUSE_IMSA_SRVCC_ABNORMAL_DEREG                          = (TAF_CS_CAUSE_IMSA_BEGIN + 11),           /* SRVCC�����г���DEREG */
    TAF_CS_CAUSE_IMSA_CMCC_TCALL_TIMEOUT                            = (TAF_CS_CAUSE_IMSA_BEGIN + 12),           /* �й��ƶ�����, Tcall��ʱ��CS���ز�*/
    TAF_CS_CAUSE_IMSA_CMCC_TQOS_TIMEOUT                             = (TAF_CS_CAUSE_IMSA_BEGIN + 13),           /* �й��ƶ�����, Tqos��ʱ��CS���ز�*/
    TAF_CS_CAUSE_IMSA_STRM_RTP_BREAK                                = (TAF_CS_CAUSE_IMSA_BEGIN + 14),           /* ͨ����û�������� */
    /* ����ԭ��ֵ, ��ԭ��ֵ���ڲ���ȥCS���ز�ʱ��IMS�ڲ�ԭ��ֵת�� */
    TAF_CS_CAUSE_IMSA_IMS_ERROR                                     = (TAF_CS_CAUSE_IMSA_BEGIN + 15),
    TAF_CS_CAUSE_IMSA_NO_RF                                         = (TAF_CS_CAUSE_IMSA_BEGIN + 16),           /* DSDSû�����뵽��Դ */
    TAF_CS_CAUSE_IMSA_EMC_DOMAIN_TMP_SEL_FAIL                       = (TAF_CS_CAUSE_IMSA_BEGIN + 17),
    TAF_CS_CAUSE_IMSA_SERVICE_LTE_SIGNAL_BAD                        = (TAF_CS_CAUSE_IMSA_BEGIN + 18),

    /* XCALL ����ԭ��ֵ */
    TAF_CS_CAUSE_XCALL_NO_SERVICE                                   = (TAF_CS_CAUSE_XCALL_BEGIN + 1),           /* No Service\x{ff0c}Call Redial */
    TAF_CS_CAUSE_XCALL_MAX_ACCESS_PROBES,                                                                       /* Max Access Probes, Call Redial */
    TAF_CS_CAUSE_XCALL_REORDER,                                                                                 /* Reorder Order, Call Redial */
    TAF_CS_CAUSE_XCALL_INTERCEPT,                                                                               /* Intercept Order, Call Redial */
    TAF_CS_CAUSE_XCALL_ACCESS_DENYIED,                                                                          /* Access Denied, Call Redial */
    TAF_CS_CAUSE_XCALL_LOCK,                                                                                    /* Lock, Call not Redial */
    TAF_CS_CAUSE_XCALL_ACCT_BLOCK,                                                                              /* Acct Block, Call Redial */
    TAF_CS_CAUSE_XCALL_NDSS,                                                                                    /* Access Control based on Call Type, redial */
    TAF_CS_CAUSE_XCALL_REDIRECTION,                                                                             /* Redirection, Call Redial */
    TAF_CS_CAUSE_XCALL_NOT_ACCEPT_BY_BS,                                                                        /* BS Reject, used in Register process, no need redial */
    TAF_CS_CAUSE_XCALL_ACCESS_IN_PROGRESS,                                                                      /* Access In Progress, Call Redial */
    TAF_CS_CAUSE_XCALL_ACCESS_FAIL,                                                                             /* Access fail, Call Redial */
    TAF_CS_CAUSE_XCALL_ABORT,                                                                                   /* State can not Process Call, Call Redial */


    TAF_CS_CAUSE_XCALL_SIGNAL_FADE_IN_ACH,                                                                             /* Signal Fade, Call Redial */
    TAF_CS_CAUSE_XCALL_CHANNEL_ASSIGN_TIMEOUT,                                                                  /* Access Time Out, Call Redial */
    TAF_CS_CAUSE_XCALL_BS_RELEASE,                                                                              /* BS, Call Redial */
    TAF_CS_CAUSE_XCALL_OTHER_SERVICE_IN_TCH,                                                                    /* Already In Traffic Channel, no need dial */
    TAF_CS_CAUSE_XCALL_CCS_NOT_SUPPORT,                                                                         /* Concurrent Service not Support, Call Redial */
    TAF_CS_CAUSE_XCALL_MS_NORMAL_RELEASE,                                                                          /* Normal Call end, Call not Redial */
    TAF_CS_CAUSE_XCALL_SO_REJ,                                                                                  /* CAS_CNAS_1X_TERMINATE_REASON_SO_REJ */
    TAF_CS_CAUSE_XCALL_RELEASE_TIME_OUT,                                                                        /* CAS_CNAS_1X_TERMINATE_REASON_RELEASE_TIME_OUT */
    TAF_CS_CAUSE_XCALL_CONNECT_ORDER_ACK_FAILURE,                                                               /* L2 ACK FAILURE */
    TAF_CS_CAUSE_XCALL_INCOMING_RSP_TIME_OUT,                                                                   /* TiWaitForXcallIncomingRsp TIME OUT */
    TAF_CS_CAUSE_XCALL_L2_ACK_TIME_OUT,                                                                         /* TiWaitL2Ack TIME OUT */
    TAF_CS_CAUSE_XCALL_POWER_DOWN_IND,                                                                          /* �ػ�ԭ���µ�ԭ�� */

    TAF_CS_CAUSE_XCALL_CONNID_NOT_FOUND,
    TAF_CS_CAUSE_XCALL_APS_TIMEOUT,
    TAF_CS_CAUSE_XCALL_ACCESS_CNF_TIMEOUT,
    TAF_CS_CAUSE_XCALL_ACCESS_IND_TIMEOUT,

    TAF_CS_CAUSE_XCALL_UNKNOWN,
    TAF_CS_CAUSE_XCALL_CALL_NOT_ALLOWED,

    TAF_CS_CAUSE_XCALL_XCALL_HANGUP,
    TAF_CS_CAUSE_XCALL_ACCESS_REL_IND,


    TAF_CS_CAUSE_XCALL_INTERNAL_NO_SERVICE,                                                                     /* ��ǰ�޷��� */
    TAF_CS_CAUSE_XCALL_WAIT_L2_ACK_RELEASE,
    TAF_CS_CAUSE_XCALL_EMERGENCY_CALL_ABORT,                                                                    /* ����������ֹ������״̬�����ز� */
    TAF_CS_CAUSE_XCALL_EMERGENCY_CALL_FLASHED,                                                                  /* ���������ڱ��д���ʱ��flash��Ϣ����ȥ */

    TAF_CS_CAUSE_XCALL_NDSS_REDIAL_IND,                                                                         /* ���й����з����ض�����Ҫ�ز� */

    TAF_CS_CAUSE_XCALL_MT_SMS_IN_TCH,
    TAF_CS_CAUSE_XCALL_NW_NORMAL_RELEASE,

    TAF_CS_CAUSE_XCALL_SIGNAL_FADE_IN_TCH,

    TAF_CS_CAUSE_XCALL_EXIST_VOICE_CALL,
    TAF_CS_CAUSE_XCALL_CLIENT_END,
    TAF_CS_CAUSE_XCALL_INCOMING_CALL,
    TAF_CS_CAUSE_XCALL_REDIAL_PERIOD_TIMEOUT,
    TAF_CS_CAUSE_XCALL_CAS_MS_END,
    TAF_CS_CAUSE_XCALL_NO_RF,
    TAF_CS_CAUSE_XCALL_RETRY_FAIL,

    TAF_CS_CAUSE_XCALL_SRV_ACQ_CNF_FAIL,

    TAF_CS_CAUSE_XCALL_TCH_LOST,
    TAF_CS_CAUSE_XCALL_T50_EXPIRED,
    TAF_CS_CAUSE_XCALL_T51_EXPIRED,
    TAF_CS_CAUSE_XCALL_TCH_WAIT_BS_ACK_EXPIRED,
    TAF_CS_CAUSE_BUTT                                               = (0xFFFFFFFF)
};
typedef  VOS_UINT32  TAF_CS_CAUSE_ENUM_UINT32;

/* [10.5.4.8] Called party subaddress */
/*============================================================================*/
/* Table 10.5.119/3GPP TS 24.008: Called party subaddress */
enum MN_CALL_TYPE_OF_SUBADDR_ENUM                                                                   /* Type of subaddress (octet 3)      */
{                                                                               /* Bits 7 6 5                        */
    MN_CALL_SUBADDR_NSAP                = 0,                                    /*      0 0 0 NSAP                   */
    MN_CALL_USER_SPEC                   = 2                                     /*      0 1 0 User specified         */
};                                                  /* All other values are reserved     */
typedef VOS_UINT8   MN_CALL_TYPE_OF_SUBADDR_ENUM_UINT8;
/* [10.5.4.9] Calling party BCD number */
/*============================================================================*/
/* Table 10.5.120/3GPP TS 24.008: Calling party BCD number */
enum MN_CALL_PRESENTATION_IND_ENUM                                                                   /* Presentation indicator (octet 3a) */
{                                                                               /* Bits 7 6                          */
    MN_CALL_PRESENTATION_ALLOWED                             = 0,               /*      0 0 Presentation allowed     */
    MN_CALL_PRESENTATION_RESTRICTED                          = 1,               /*      0 1 Presentation restricted  */
    MN_CALL_PRESENTATION_NOT_AVAILABLE                       = 2,               /*      1 0 Number not available     */
    MN_CALL_PRESENTATION_RESERVED                            = 3                /*      1 1 Reserved                 */
};
typedef VOS_UINT8   MN_CALL_PRESENTATION_IND_ENUM_UINT8;


/*============================================================================*/
/* Table 2.7.4.41. /3GPP2 C.S005-A v6.0: Calling party number */
enum TAF_CALL_PRESENTATION_IND_ENUM                                                                   /* Presentation indicator (octet 3a) */
{                                                                               /* Bits 7 6                          */
    TAF_CALL_PRESENTATION_ALLOWED                             = 0,               /*      0 0 Presentation allowed     */
    TAF_CALL_PRESENTATION_RESTRICTED                          = 1,               /*      0 1 Presentation restricted  */
    TAF_CALL_PRESENTATION_NOT_AVAILABLE                       = 2,               /*      1 0 Number not available     */
    TAF_CALL_PRESENTATION_RESERVED                            = 3,               /*      1 1 Reserved                 */
    TAF_CALL_PRESENTATION_BUTT                                = 4
};

typedef VOS_UINT8   TAF_CALL_PRESENTATION_IND_ENUM_UINT8;

/* Added by l60609 for CDMA 1X Iteration 2, 2014-9-3, begin */

/* ��ö�ٶ��嵽tafAppCall.h�ļ��� */

enum TAF_CALL_DIGIT_MODE_ENUM
{
    TAF_CALL_DIGIT_MODE_DTMF            = 0x00,
    TAF_CALL_DIGIT_MODE_ASCII           = 0x01,
    TAF_CALL_DIGIT_MODE_BUTT            = 0x02
};
typedef VOS_UINT8 TAF_CALL_DIGIT_MODE_ENUM_UINT8;


enum TAF_CALL_NUMBER_PLAN_ENUM
{
    TAF_CALL_NUMBER_PLAN_UNKNOW         = 0x00,
    TAF_CALL_NUMBER_PLAN_ISDN           = 0x01,
    TAF_CALL_NUMBER_PLAN_DATA           = 0x03,
    TAF_CALL_NUMBER_PLAN_TELEX          = 0x04,
    TAF_CALL_NUMBER_PLAN_PRIVATE        = 0x09,
    TAF_CALL_NUMBER_PLAN_BUTT           = 0x0A
};
typedef VOS_UINT8 TAF_CALL_NUMBER_PLAN_ENUM_UINT8;


enum TAF_CALL_NUMBER_TYPE_ENUM
{
    TAF_CALL_NUMBER_TYPE_UNKNOW                             = 0x00,
    TAF_CALL_NUMBER_TYPE_INTERNATIONAL                      = 0x01,
    TAF_CALL_NUMBER_TYPE_NATIONAL                           = 0x02,
    TAF_CALL_NUMBER_TYPE_NETWORK_SPEC                       = 0x03,
    TAF_CALL_NUMBER_TYPE_SUBSCRIBER                         = 0x04,
    TAF_CALL_NUMBER_TYPE_RESERVED                           = 0x05,
    TAF_CALL_NUMBER_TYPE_ABBREVIATED                        = 0x06,
    TAF_CALL_NUMBER_TYPE_RESERVED_FOR_EXT                   = 0x07,
    TAF_CALL_NUMBER_TYPE_BUTT                               = 0x08
};
typedef VOS_UINT8 TAF_CALL_NUMBER_TYPE_ENUM_UINT8;
/* Added by l60609 for CDMA 1X Iteration 2, 2014-9-3, end */


enum MN_CALL_MODIFY_STATUS_ENUM
{
    MN_CALL_MODIFY_REMOTE_USER_REQUIRE_TO_MODIFY,                               /* Զ���û������modify */
    MN_CALL_MODIFY_PROC_BEGIN,                                                  /* modify���̿�ʼ */
    MN_CALL_MODIFY_PROC_END,                                                    /* modify���̽��� */
    MN_CALL_MODIFY_STATUS_BUTT
};
typedef  VOS_UINT8  MN_CALL_MODIFY_STATUS_ENUM_UINT8;



enum TAF_CALL_SEND_BURST_DTMF_CNF_RESULT_ENUM
{
    TAF_CALL_SEND_BURST_DTMF_CNF_RESULT_SUCCESS         = 0,
    TAF_CALL_SEND_BURST_DTMF_CNF_RESULT_ABNORMAL_STATE,
    TAF_CALL_SEND_BURST_DTMF_CNF_RESULT_BUFFER_FULL,
    TAF_CALL_SEND_BURST_DTMF_CNF_RESULT_ALLOC_MSG_FAIL,
    TAF_CALL_SEND_BURST_DTMF_CNF_RESULT_ALLOC_MEM_FAIL,

    TAF_CALL_SEND_BURST_DTMF_CNF_RESULT_BUTT
};
typedef VOS_UINT8   TAF_CALL_SEND_BURST_DTMF_CNF_RESULT_ENUM_U8;


enum TAF_CALL_REJ_CAUSE_ENUM
{
    TAF_CALL_REJ_CAUSE_USER_BUSY        = 0,        /* CS��Ҷϵ绰ԭ��ֵΪ#17 User Busy, IMS��Ҷϵ绰ԭ��Ϊ486 Busy Here */
    TAF_CALL_REJ_CAUSE_CALL_REJECTION   = 1,        /* CS��Ҷϵ绰ԭ��ֵΪ#21 Call Rejection, IMS��Ҷϵ绰ԭ��Ϊ603 Decline */

    TAF_CALL_REJ_CAUSE_BUTT
};
typedef VOS_UINT8 TAF_CALL_REJ_CAUSE_ENUM_UINT8;

/* Added by f62575 for STK&DCM Project, 2012/09/18, begin */


enum MN_CALL_CF_CAUSE_ENUM
{
    MN_CALL_CF_CAUSE_NONE               = 0,
    MN_CALL_CF_CAUSE_ALWAYS,
    MN_CALL_CF_CAUSE_BUSY,
    MN_CALL_CF_CAUSE_POWER_OFF,
    MN_CALL_CF_CAUSE_NO_ANSWER,
    MN_CALL_CF_CAUSE_SHADOW_ZONE,
    MN_CALL_CF_CAUSE_DEFLECTION_480,
    MN_CALL_CF_CAUSE_DEFLECTION_487,

    MN_CALL_CF_CAUSE_BUTT
};
typedef VOS_UINT8  MN_CALL_CF_CAUSE_ENUM_UINT8;


enum MN_CALL_HOLD_TONE_ENUM
{
    MN_CALL_HOLD_TONE_DISABLE                   = 0,
    MN_CALL_HOLD_TONE_ENABLE,

    MN_CALL_HOLD_TONE_BUTT
};
typedef VOS_UINT8  MN_CALL_HOLD_TONE_ENUM_UINT8;


enum TAF_CALL_CNI_VALIDITY_ENUM
{
    TAF_CALL_CNI_VALIDITY_VALID             = 0,                                /* CNI Valid */
    TAF_CALL_CNI_VALIDITY_WITHHELD,                                             /* CNI has been withheld by the originator */
    TAF_CALL_CNI_VALIDITY_NOT_AVAILABLE,                                        /* CNI is not available due to interworking problems or limitations of originating network */
    TAF_CALL_CNI_VALIDITY_BUTT
};
typedef VOS_UINT8 TAF_CALL_CNI_VALIDITY_ENUM_UINT8;


enum TAF_CALL_DCS_ENUM
{
    TAF_CALL_DCS_7BIT                   = 0,                                    /* 7 bit */
    TAF_CALL_DCS_8BIT,                                                          /* 8 bit */
    TAF_CALL_DCS_UCS2,                                                          /* UCS-2 */
    TAF_CALL_DCS_BUTT
};
typedef VOS_UINT8 TAF_CALL_DCS_ENUM_UINT8;


typedef struct
{
    VOS_UINT8                           ucExistFlag;                            /* �����Ƿ�Я��CNAP��Ϣ��־ */
    TAF_CALL_CNI_VALIDITY_ENUM_UINT8    enCniValidity;                          /* <CNI_Validity>���� */
    TAF_CALL_DCS_ENUM_UINT8             enDcs;                                  /* <name>������Data Coding Scheme */
    VOS_UINT8                           ucLength;
    VOS_UINT8                           aucNameStr[TAF_CALL_CNAP_NAME_STR_MAX_LENGTH];
    VOS_UINT8                           aucReserved[3];
} TAF_CALL_CNAP_STRU;



typedef struct
{
    VOS_UINT8                           IsExist;
    VOS_UINT8                           LastOctOffset;
    VOS_UINT8                           Octet3;
    VOS_UINT8                           SubAddrInfo[MN_CALL_MAX_SUBADDR_INFO_LEN];
    VOS_UINT8                           ucReserved;
}MN_CALL_SUBADDR_STRU;
/* Added by f62575 for STK&DCM Project, 2012/09/18, end */

#define MN_CALL_MAX_CALLED_BCD_NUM_LEN      (40)                                /* 3GPP TS 24.008 10.5.4.7 */
#define MN_CALL_MAX_CALLED_ASCII_NUM_LEN    (MN_CALL_MAX_CALLED_BCD_NUM_LEN * 2)

typedef struct
{
    MN_CALL_NUM_TYPE_ENUM_U8            enNumType;
    VOS_UINT8                           ucNumLen;
    VOS_UINT8                           aucBcdNum[MN_CALL_MAX_CALLED_BCD_NUM_LEN];
    VOS_UINT8                           aucReserved[6];
}MN_CALL_CALLED_NUM_STRU;


typedef struct
{
    VOS_BOOL                            bExist;
    VOS_UINT8                           ucEmergencyCat;
    VOS_UINT8                           aucReserve[3];
}MN_CALL_EMERGENCY_CAT_STRU;





typedef struct
{
    VOS_UINT8                           ucIsUser;
    VOS_UINT8                           ucDiscCallFlag;
    VOS_UINT8                           aucReserved[2];
}MN_CALL_DISC_DIR;


typedef struct
{
    VOS_UINT8                           ucNumLen;
    VOS_CHAR                            acDisplayName[MN_CALL_DISPLAY_NAME_STRING_SZ];
    VOS_UINT8                           aucReserved[2];
}MN_CALL_DISPLAY_NAME_STRU;

typedef struct
{
    MN_CLIENT_ID_T                              usClientId;
    MN_OPERATION_ID_T                           ucOpId;
    MN_CALL_ID_T                                ucCallId;
    TAF_CALL_SEND_BURST_DTMF_CNF_RESULT_ENUM_U8 enResult;
    VOS_UINT8                                   aucReserved[3];
}TAF_CALL_EVT_SEND_BURST_DTMF_CNF_STRU;


typedef struct
{
    MN_CLIENT_ID_T                      usClientId;
    MN_OPERATION_ID_T                   ucOpId;
    VOS_UINT8                           ucResult;
}TAF_CALL_EVT_SEND_BURST_DTMF_RSLT_STRU;


typedef struct
{
    VOS_UINT8                           ucTextLen;
    VOS_CHAR                            acErrInfoText[TAF_CALL_ERROR_INFO_TEXT_STRING_SZ];
    VOS_UINT8                           aucReserved[2];
}TAF_CALL_ERROR_INFO_TEXT_STRU;



typedef struct
{
    VOS_UINT8                           ucRecordNum;                            /* ��ʱ����ʱ���� */
    VOS_UINT8                           aucReserved[3];
    VOS_UINT32                          aulCause[TAF_CALL_MAX_CC_TIMEROUT_NUM]; /* ��¼Cc�ඨʱ����ʱԭ�� */
}TAF_CALL_CC_TIMEROUT_RECORD_STRU;

/* Call Info Structure */
typedef struct
{
    MN_CLIENT_ID_T                      clientId;                               /* Client ID */
    MN_OPERATION_ID_T                   opId;                                   /* op ID */
    MN_CALL_ID_T                        callId;                                 /* Call ID */
    MN_CALL_STATE_ENUM_U8               enCallState;                            /* Call State */
    MN_CALL_MPTY_STATE_ENUM_U8          enMptyState;                            /* MPTY State */
    MN_CALL_TYPE_ENUM_U8                enCallType;                             /* Call Type */
    MN_CALL_MODE_ENUM_U8                enCallMode;                             /* Call Mode */
    MN_CALL_DIR_ENUM_U8                 enCallDir;                              /* Call Direction(MO/MT) */
    MN_CALL_PRESENTATION_IND_ENUM_UINT8 enCallingPI;
    TAF_CALL_SUB_STATE_ENUM_UINT8       enCallSubState;
    VOS_UINT8                           ucEconfFlag;
    MN_CALL_CS_DATA_CFG_INFO_STRU       stDataCfgInfo;                          /* Data Configuration */
    MN_CALL_BCD_NUM_STRU                stCallNumber;                           /* Call Number */
    MN_CALL_SUBADDR_STRU                stSubCallNumber;                        /* Call SubAddress */
    MN_CALL_CALLED_NUM_STRU             stCalledNumber;                         /* Called Number */
    MN_CALL_SUBADDR_STRU                stCalledSubAddr;                        /* Called SubAddress */
    MN_CALL_BCD_NUM_STRU                stRedirectNumber;                       /* Redirect Number */
    MN_CALL_SUBADDR_STRU                stRedirectSubaddr;                      /* Redirect Call SubAddress */
    MN_CALL_BCD_NUM_STRU                stConnectNumber;                        /* Connect Number */
    MN_CALL_SUPS_CMD_ENUM_U8            enCallSupsCmd;                          /* Call Management Command */
    MN_CALL_SS_RESULT_ENUM_U8           enSsResult;                             /* the result of the last Call Management Command */
    MN_CALL_SS_ERR_CODE_ENUM_U8         enSsErrCode;                            /* SS Error Code */
    VOS_UINT8                           ucRabId;                                /* RAB ID */
    MN_CALL_NO_CLI_CAUSE_ENUM_U8        enNoCliCause;                           /* Cause of the NO CLI */
    MN_CALL_ALS_LINE_NO_ENUM_U8         enAlsLineNo;                            /* ALS Line number: 1 or 2,default is 1*/

    VOS_UINT8                           ucAtaReportOkAsyncFlag;                 /* VOS_TRUE:ata�첽�ϱ�ok������connect���ϱ�ok����������connect ack��
                                                                                   VOS_FALSE:ataͬ���ϱ�������connect����յ�����connect ack�����ϱ�ok */

    TAF_CALL_VOICE_DOMAIN_ENUM_UINT8    enVoiceDomain;                          /* VOICE DOMAIN */
    MN_CALL_SS_NOTIFY_STRU              stSsNotify;                             /* SS Notify */
    TAF_CS_CAUSE_ENUM_UINT32            enCause;                                /* Cause Value of the Call Control */
    TAF_SS_CCBS_FEATURE_STRU            stCcbsFeature;                          /* CCBS feature info*/
    MN_CALL_UUS1_INFO_STRU              stUusInfo;
    VOS_UINT32                          ulCurCallTime;
    VOS_UINT32                          ulTotalCallTime;                        /*�ۼ�ͨ��ʱ��*/
    VOS_UINT32                          ulPreCallTime;                          /*�ϴ�ͨ��ʱ��*/
    MN_CALL_DISC_DIR                    stDiscDir;                              /* �Ҷϵ绰�ķ��� */
    VOS_UINT8                           aucCurcRptCfg[MN_CALL_RPT_CFG_MAX_SIZE];        /* CURC���õ������ϱ���ʶ */
    VOS_UINT8                           aucUnsolicitedRptCfg[MN_CALL_RPT_CFG_MAX_SIZE]; /* �����������õ������ϱ���ʶ */
    VOS_UINT8                           ucEncryptFlag;                          /* ���ܱ�� */
    VOS_UINT8                           ucSrvccSuccFlg;                         /* ��¼����ͨ���Ƿ���ͨ��SRVCC������ */
    MN_CALL_CF_CAUSE_ENUM_UINT8         enCallForwardCause;
    MN_CALL_HOLD_TONE_ENUM_UINT8        enHoldToneType;

    MN_CALL_DISPLAY_NAME_STRU           stDisplayName;

    TAF_CALL_EVT_SEND_BURST_DTMF_RSLT_STRU stSendBurstDTMFRslt;
    TAF_CALL_EVT_SEND_BURST_DTMF_CNF_STRU  stSendBurstDTMFCnf;

    MN_CALL_EMERGENCY_CAT_STRU          stEmergencyCat;                         /* Emergency Catory */

    TAF_CALL_CNAP_STRU                  stNameIndicator;                        /* Name Indicator */

    TAF_CALL_ERROR_INFO_TEXT_STRU       stErrInfoText;                          /* �쳣��Ϣ�ı� */
    VOS_UINT8                           ucStartHifiFlag;                        /* IMSAָʾ�Ƿ���HIFI�ı�ʶ */
    TAF_CALL_SERVICE_TYPE_ENUM_UINT8    enServiceType;
    VOS_UINT8                           aucReserved[2];
} MN_CALL_INFO_STRU;


typedef struct
{
    MN_CALL_TYPE_ENUM_U8                enCallType;                             /* Call Type */
    MN_CALL_CLIR_CFG_ENUM_U8            enClirCfg;                              /* CLIR Configuration */
    MN_CALL_MODE_ENUM_U8                enCallMode;                             /* Call Mode */
    VOS_UINT8                           ucCsCallRetryFlg;                       /* CS CALL�Ƿ���ٳ��Ա��:��CS����ã���CS��δ���Թ�����ʱ����ʾCS������ٳ��� VOS_TRUE�������������ʾCS�򲻿����ٳ��� VOS_FALSE */
    MN_CALL_CALLED_NUM_STRU             stDialNumber;                           /* Call Number */
    MN_CALL_CUG_CFG_STRU                stCugCfg;                               /* CUG Configuration */
    MN_CALL_CS_DATA_CFG_STRU            stDataCfg;                              /* Data Configuration */
    MN_CALL_SUBADDR_STRU                stSubaddr;                              /* Subaddress */

    MN_CALL_EMERGENCY_CAT_STRU          stEmergencyCat;                         /* Emergency Catory */

    VOS_UINT8                           ucImsInvitePtptFlag;                    /* IMS ���������ͨ����� */

    TAF_CALL_VOICE_DOMAIN_ENUM_UINT8    enVoiceDomain;                          /* VOICE DOMAIN */

    VOS_UINT8                           ucEncryptFlag;                          /* ���ܱ�� */

    VOS_UINT8                           ucDomainRedialFlag;                     /* ��ǰ�����ʧ�ܺ��������Ա�� */
    VOS_UINT8                           ucHangupImsCallInCsRedialFlg;                   /* ucDomainRedialFlagΪtrue��IMS��ͨ�����յ�380ʱ���Ƿ�Ҫ�Ҷϵ�ǰ���������ͨ���б�־ */

    TAF_CALL_EMC_DOMAIN_ENUM_UINT8      enEmcDomain;

    TAF_CALL_SERVICE_TYPE_ENUM_UINT8    enServiceType;                          /* ҵ������ */

    VOS_UINT8                           aucReserv[1];
} MN_CALL_ORIG_PARAM_STRU;


typedef struct
{
    TAF_CS_CAUSE_ENUM_UINT32            enEndCause;                             /* End Cause */
} MN_CALL_END_PARAM_STRU;


typedef struct
{
    VOS_UINT32                          ulReserved;
} MN_CALL_ANS_PARAM_STRU;

/* Modified by f62575 for V9R1 STK����, 2013-6-26, begin */

typedef struct
{
    MN_CALL_ID_T                        CallId;                                 /* ����ID��0: ����ǰĬ�Ϻ��� */
    VOS_CHAR                            cKey;                                   /* DTMF Key ֻ��START_DTMFʱ��ע */
    VOS_UINT16                          usOnLength;                             /* DTMF����ʱ��(��λms)��0: ����ֹͣDTMF */

    VOS_UINT16                          usOffLength;                            /* stop dtmf req��start dtmf req���͵�ʱ����(��λms)
                                                                                 ����70msС��600ms */
    VOS_UINT8                           aucReserved[2];
} TAF_CALL_DTMF_PARAM_STRU;
/* Modified by f62575 for V9R1 STK����, 2013-6-26, end */

typedef struct
{
    MN_CALL_SUPS_CMD_ENUM_U8            enCallSupsCmd;                          /* Call Management Command */
    MN_CALL_ID_T                        callId;                                 /* Call ID */
    MN_CALL_TYPE_ENUM_U8                enCallType;                             /* ����Խ�������·���в������� */
    TAF_CALL_REJ_CAUSE_ENUM_UINT8       enCallRejCause;                         /* ��������ΪMN_CALL_SUPS_CMD_REL_INCOMING_OR_WAITINGʱ����IE����Ч */
    MN_CALL_BCD_NUM_STRU                stRedirectNum;                          /* Redirect Number */
    MN_CALL_CALLED_NUM_STRU             stRemoveNum;                            /* ��ǿ�ͻ�������Ҫ�Ƴ�������ߺ��� */
} MN_CALL_SUPS_PARAM_STRU;


typedef struct
{
    VOS_UINT32                          ulActNum;                               /* ��Ҫ���õ�CSSN�ĸ��� */
    MN_CALL_SET_CSSN_TYPE_ENUM_U32      aenSetType[MN_CALL_MAX_CSSN_MSG_NUM];   /* aenSetType[0]:CSSI�Ƿ���Ҫ���enSetType[1]:CSSU�Ƿ���Ҫ���� */
}MN_CALL_SET_CSSN_REQ_STRU;



typedef struct
{
    MN_CALL_TYPE_ENUM_U8                enCurrCallType;                         /* ��ǰ�������� */
    MN_CALL_TYPE_ENUM_U8                enExpectCallType;                       /* ������������ */
    VOS_UINT8                           aucReserved[2];
}MN_CALL_MODIFY_REQ_STRU;


typedef struct
{
    MN_CALL_EVENT_ENUM_U32              enEvent;                                /* �ظ����¼����� */
    MN_CLIENT_ID_T                      usClientId;                             /* �˿ں� */
    MN_OPERATION_ID_T                   ucOpId;                                 /* ����ID */
    MN_CALL_ID_T                        ucCallId;                               /* Call ID */
    TAF_CS_CAUSE_ENUM_UINT32            enCause;                                /* ������ */
}MN_CALL_MODIFY_CNF_STRU;


typedef struct
{
    MN_CALL_EVENT_ENUM_U32                          enEvent;                    /* �¼��� */
    MN_CLIENT_ID_T                                  usClientId;
    MN_OPERATION_ID_T                               ucOpId;                     /* ����ID */
    MN_CALL_ID_T                                    ucCallId;                   /* Call ID */
    MN_CALL_MODIFY_STATUS_ENUM_UINT8                enModifyStatus;             /* ��ǰ��MODIFY�Ĺ���״̬ */
    TAF_CALL_VOICE_DOMAIN_ENUM_UINT8                enVoiceDomain;              /* VOICE DOMAIN������ʼ����IMS�� */
    MN_CALL_TYPE_ENUM_U8                            enCurrCallType;             /* ��ǰ�������� */
    MN_CALL_TYPE_ENUM_U8                            enExpectCallType;           /* ������������ */
    TAF_CS_CAUSE_ENUM_UINT32                        enCause;                    /* �����룬����MODIFY_PROC_END���쳣ʱ��ʹ�� */
} MN_CALL_EVT_MODIFY_STATUS_IND_STRU;



typedef struct
{
    MN_CALL_CUUS1_FLG_ENUM_U32          enCuus1IFlg;                            /* CUUS1I��־ */
    MN_CALL_CUUS1_FLG_ENUM_U32          enCuus1UFlg;                            /* CUUS1U��־ */
    VOS_UINT32                          ulActNum;                             /*��Ҫ���õ�UUS1�ĸ���*/
    MN_CALL_SET_UUS1_TYPE_ENUM_U32      enSetType[MN_CALL_MAX_UUS1_MSG_NUM];  /*Ҫ���õ��û���������1����*/
    MN_CALL_UUS1_INFO_STRU              stUus1Info[MN_CALL_MAX_UUS1_MSG_NUM]; /*���õ��û�����������Ϣ*/
}MN_CALL_UUS1_PARAM_STRU;


typedef struct
{
    MN_CALL_EVENT_ENUM_U32  enEvent;    /*�ظ����¼�����*/
    MN_CLIENT_ID_T          ClientId;   /*�˿ں�*/
    MN_OPERATION_ID_T       OpId;       /*����ID*/
    VOS_UINT8               ucReserved; /* �����ֶ� */
    VOS_UINT32              ulRet;      /*�ظ����*/
}TAF_PH_SET_UUS1_INFO_CNF_STRU;


typedef struct
{
    MN_CALL_EVENT_ENUM_U32  enEvent;                                /*�ظ����¼�����*/
    MN_CLIENT_ID_T          ClientId;                               /*�˿ں�*/
    MN_OPERATION_ID_T       OpId;                                   /*����ID*/
    VOS_UINT8               ucReserved;                             /* �����ֶ� */
    VOS_UINT32              ulRet;                                  /*�ظ����*/
    VOS_UINT32              ulActNum;                               /*�û���������1�ĸ���*/
    MN_CALL_UUS1_INFO_STRU  stUus1Info[MN_CALL_MAX_UUS1_MSG_NUM];   /*�û���������1����Ϣ*/
    MN_CALL_SET_UUS1_TYPE_ENUM_U32      aenSetType[MN_CALL_MAX_UUS1_MSG_NUM];  /*Ҫ���õ��û���������1���� */
}TAF_PH_QRY_UUS1_INFO_CNF_STRU;


typedef struct
{
    MN_CALL_ALS_LINE_NO_ENUM_U8         enAlsLine;          /*Ҫʹ�õ���·��*/
    VOS_UINT8                           aucReserved[3];     /*�����ֶ�*/
}MN_CALL_SET_ALS_PARAM_STRU;


typedef struct
{
    MN_CALL_EVENT_ENUM_U32  enEvent;    /*�ظ����¼�����*/
    MN_CLIENT_ID_T          ClientId;   /*�˿ں�*/
    MN_OPERATION_ID_T       OpId;       /*����ID*/
    VOS_UINT8               ucReserved; /* �����ֶ� */
    VOS_UINT32              ulRet;      /*�ظ����*/
}MN_CALL_SET_ALS_CNF_STRU;


typedef struct
{
    MN_CALL_EVENT_ENUM_U32          enEvent;                                    /*�ظ����¼�����*/
    MN_CLIENT_ID_T                  ClientId;                                   /*�˿ں�*/
    MN_OPERATION_ID_T               OpId;                                       /*����ID*/
    VOS_UINT8                       ucReserved1;                                /* �����ֶ� */
    VOS_UINT32                      ulRet;                                      /*�ظ����*/
    MN_CALL_ALS_LINE_NO_ENUM_U8     enAlsLine;                                  /*��ǰʹ�õ���·��*/
    VOS_UINT8                       aucReserved2[3];                            /* �����ֶ� */
}MN_CALL_QRY_ALS_CNF_STRU;


typedef struct
{
    VOS_UINT8                           ucIndex;                                /* ���ƽ������������� */
    VOS_UINT8                           ucTotal;                                /* ���ƽ������������� */
    VOS_UINT8                           ucCategory;                             /* �������������� */
    VOS_UINT8                           ucSimPresent;                           /* �������������п�ʱ��Ч�����޿�ʱ��Ч��1�п�ʱ��Ч��0�޿�ʱ��Ч */
    MN_CALL_BCD_NUM_STRU                stEccNum;                               /* ���������� */
    VOS_UINT32                          ulMcc;                                  /* MCC,3 bytes */
} MN_CALL_ECC_LIST_INFO_STRU;


typedef struct
{
    VOS_UINT32                          bitOpPeerVideoSupport       : 1;
    VOS_UINT32                          bitOpSpare                  : 31;

    MN_CALL_ID_T                        callId;                                 /* Call ID */
    MN_CALL_DIR_ENUM_U8                 enCallDir;                              /* Call Direction(MO/MT) */
    MN_CALL_STATE_ENUM_U8               enCallState;                            /* Call State */
    MN_CALL_MPTY_STATE_ENUM_U8          enMptyState;                            /* MPTY State */
    MN_CALL_TYPE_ENUM_U8                enCallType;                             /* Call Type */
    TAF_CALL_VOICE_DOMAIN_ENUM_UINT8    enVoiceDomain;                          /* VOICE DOMAIN */
    VOS_UINT8                           ucEConferenceFlag;                      /* Is Econf:true, Is not Econf:false */
    VOS_UINT8                           ucEncryptFlag;                          /* Is Encrypt:true, Is not Encrypt:false */
    MN_CALL_BCD_NUM_STRU                stCallNumber;                           /* Call Number */
    MN_CALL_CALLED_NUM_STRU             stCalledNumber;                         /* Called Number */
    MN_CALL_BCD_NUM_STRU                stConnectNumber;                        /* Connect Number */

    MN_CALL_DISPLAY_NAME_STRU           stDisplayName;

    TAF_CALL_PEER_VIDEO_SUPPORT_ENUM_UINT8  enPeerVideoSupport;
    VOS_UINT8                               aucReserved[3];
}MN_CALL_INFO_PARAM_STRU;


typedef struct
{
    VOS_UINT32                          ulCallNum;
    MN_CALL_CALLED_NUM_STRU             astDialNumber[TAF_CALL_MAX_ECONF_CALLED_NUM];
    MN_CALL_SUBADDR_STRU                astSubaddr[TAF_CALL_MAX_ECONF_CALLED_NUM];
}TAF_CALL_ECONF_CALL_LIST_STRU;


typedef struct
{
    MN_CALL_TYPE_ENUM_U8                enCallType;                             /* Call Type */
    MN_CALL_CLIR_CFG_ENUM_U8            enClirCfg;                              /* CLIR Configuration */
    MN_CALL_MODE_ENUM_U8                enCallMode;                             /* Call Mode */
    TAF_CALL_VOICE_DOMAIN_ENUM_UINT8    enVoiceDomain;                          /* VOICE DOMAIN */
    VOS_UINT8                           ucImsInvitePtptFlag;                    /* IMS ���������ͨ����� */
    VOS_UINT8                           aucReserved[3];
    MN_CALL_CUG_CFG_STRU                stCugCfg;                               /* CUG Configuration */
    MN_CALL_CS_DATA_CFG_STRU            stDataCfg;                              /* Data Configuration */
    TAF_CALL_ECONF_CALL_LIST_STRU       stEconfCalllist;                        /* ����μӶ෽ͨ�������ߵ���Ϣ */
} TAF_CALL_ECONF_DIAL_REQ_STRU;


typedef struct
{
    MN_CALL_EVENT_ENUM_U32              enEvent;                                /* �ظ����¼����� */
    MN_CLIENT_ID_T                      usClientId;                             /* �˿ں� */
    MN_OPERATION_ID_T                   ucOpId;                                 /* ����ID */
    MN_CALL_ID_T                        ucCallId;                               /* Call ID */
    TAF_CS_CAUSE_ENUM_UINT32            enCause;                                /* ������ */
}TAF_CALL_ECONF_DIAL_CNF_STRU;


enum TAF_CALL_ECONF_STATE_ENUM
{
    TAF_CALL_ECONF_STATE_DISCONNECTED,
    TAF_CALL_ECONF_STATE_CONNECTED,
    TAF_CALL_ECONF_STATE_ONHOLD,
    TAF_CALL_ECONF_STATE_PREDIALING,
    TAF_CALL_ECONF_STATE_DIALING,
    TAF_CALL_ECONF_STATE_BUTT
};
typedef VOS_UINT8  TAF_CALL_ECONF_STATE_ENUM_U8;


enum CALL_IMSA_GET_CALL_INFO_REQ_TYPE_ENMU
{
    CALL_IMSA_GET_CALL_INFO_REQ_TYPE_CLCC        = 0,     /* CLCC��ѯ�����ȡCALL��Ϣ */
    CALL_IMSA_GET_CALL_INFO_REQ_TYPE_ATA         = 1,     /* ATA��ѯ�����ȡCALL��Ϣ */
    CALL_IMSA_GET_CALL_INFO_REQ_TYPE_ENMU_BUTT
};
typedef VOS_UINT8  CALL_IMSA_GET_CALL_INFO_REQ_TYPE_ENMU_UINT8;


typedef struct
{
    TAF_CALL_ECONF_STATE_ENUM_U8   enCallState;                                 /* Call State */
    VOS_UINT8                      auReserve1[3];
    TAF_CS_CAUSE_ENUM_UINT32       enCause;                                     /* disconnecʱ����ҪЯ��ԭ��ֵ */
    MN_CALL_CALLED_NUM_STRU        stCallNumber;                                /* Call Number */
    VOS_CHAR                       aucDisplaytext[TAF_IMSA_ALPHA_STRING_SZ];    /*display-text*/
    VOS_UINT8                      auReserve2[7];
}TAF_CALL_ECONF_INFO_PARAM_STRU;


typedef struct
{
    MN_CALL_EVENT_ENUM_U32                                  enEvent;            /* �¼��� */
    MN_CLIENT_ID_T                                          usClientId;         /* �û�ID */
    VOS_UINT8                                               ucNumOfMaxCalls;
    VOS_UINT8                                               ucNumOfCalls;       /* �෽ͨ������߸��� */
    TAF_CALL_ECONF_INFO_PARAM_STRU                          astCallInfo[TAF_CALL_MAX_ECONF_CALLED_NUM];     /* ��������ͨ������Ϣ */
}TAF_CALL_ECONF_INFO_QRY_CNF_STRU;


typedef struct
{
    MN_CALL_EVENT_ENUM_U32                          enEvent;                    /* �¼��� */
    MN_CLIENT_ID_T                                  usClientId;
    MN_OPERATION_ID_T                               ucOpId;                     /* ����ID */
    VOS_UINT8                                       ucNumOfCalls;               /* ��������ͨ���ĸ��� */
    TAF_CALL_ECONF_INFO_PARAM_STRU                  astCallInfo[TAF_CALL_MAX_ECONF_CALLED_NUM];     /* ��������ͨ������Ϣ */
} TAF_CALL_EVT_ECONF_NOTIFY_IND_STRU;


typedef struct
{
    MN_CALL_EVENT_ENUM_U32              enEvent;                                /* �¼��� */
    MN_CLIENT_ID_T                      clientId;                               /* �û�ID */
    VOS_UINT8                           ucNumOfCalls;                           /* ��������ͨ���ĸ��� */
    VOS_CHAR                            ucReserved;                             /* �����ֶ� */
    MN_CALL_INFO_PARAM_STRU             astCallInfos[MN_CALL_MAX_NUM];           /* ��������ͨ������Ϣ */
} MN_CALL_INFO_QRY_CNF_STRU;


typedef struct
{
    VOS_UINT8                           ucIndex;                                /* ���ƽ������������� */
    VOS_UINT8                           ucTotal;                                /* ���ƽ������������� */
    VOS_UINT8                           ucCategory;                             /* �������������� */
    VOS_UINT8                           ucSimPresent;                           /* �������������п�ʱ��Ч�����޿�ʱ��Ч��1�п�ʱ��Ч��0�޿�ʱ��Ч */
    VOS_UINT32                          ulMcc;                                  /* MCC,3 bytes */
    MN_CALL_BCD_NUM_STRU                stEccNum;                               /* ���������� */
    VOS_UINT8                           ucAbnormalServiceFlg;                   /* �����������п�ʱ�Ƿ���ڷ���������ʱ��Ч��0:�κη���ʱ����Ч��1:������������ʱ��Ч */
    VOS_UINT8                           aucReserved[3];
}MN_CALL_APP_CUSTOM_ECC_NUM_REQ_STRU;


typedef struct
{
    VOS_UINT8                           ucCategory;                             /* �������������� */
    VOS_UINT8                           ucValidSimPresent;                      /* �������������п�ʱ��Ч�����޿�ʱ��Ч��1�п�ʱ��Ч��0�޿�ʱ��Ч */
    VOS_UINT8                           ucAbnormalServiceFlg;                   /* �����������п�ʱ�Ƿ���ڷ���������ʱ��Ч��0:�κη���ʱ����Ч��1:������������ʱ��Ч */
    VOS_UINT8                           ucEccNumLen;
    VOS_UINT8                           aucEccNum[MN_CALL_MAX_BCD_NUM_LEN];
    VOS_UINT32                          ulMcc;                                  /* MCC,3 bytes */
} MN_CALL_CUSTOM_ECC_NUM_STRU;


typedef struct
{
    MN_CALL_EVENT_ENUM_U32              enEvent;                                /* �¼��� */
    MN_CLIENT_ID_T                      usClientId;
    VOS_UINT8                           aucReserve[2];
    VOS_UINT32                          ulEccNumCount;
    MN_CALL_CUSTOM_ECC_NUM_STRU         astCustomEccNumList[MN_CALL_MAX_EMC_NUM];
} MN_CALL_ECC_NUM_INFO_STRU;





enum MN_CALL_CODEC_TYPE_ENUM
{
    MN_CALL_CODEC_TYPE_AMR,                 /* amr */
    MN_CALL_CODEC_TYPE_EFR,                 /* enhanced full rate */
    MN_CALL_CODEC_TYPE_FR,                  /* full rate */
    MN_CALL_CODEC_TYPE_HR,                  /* half rate */
    MN_CALL_CODEC_TYPE_AMR2,
    /* Added by z40661 for AMR-WB , 2012-02-09, begin */
    MN_CALL_CODEC_TYPE_AMRWB,
    /* Added by z40661 for AMR-WB , 2012-02-09, end */
    MN_CALL_CODEC_TYPE_G711,
    MN_CALL_CODEC_TYPE_EVS,
    MN_CALL_CODEC_TYPE_BUTT                 /* invalid value */
};
typedef VOS_UINT8  MN_CALL_CODEC_TYPE_ENUM_U8;

enum MN_CALL_RADIO_MODE_ENUM
{
    MN_CALL_RADIO_MODE_GSM,
    MN_CALL_RADIO_MODE_WCDMA,
    MN_CALL_RADIO_MODE_IMS_EUTRAN,
    MN_CALL_RADIO_MODE_BUTT
};
typedef VOS_UINT8  MN_CALL_RADIO_MODE_ENUM_U8;

enum MN_CALL_CHANNEL_RATE_ENUM
{
    MN_CALL_CHANNEL_RATE_475,                   /* rate 4.75kbps/s */
    MN_CALL_CHANNEL_RATE_515,                   /* rate 5.15kbps/s */
    MN_CALL_CHANNEL_RATE_59,                    /* rate 5.9kbps/s */
    MN_CALL_CHANNEL_RATE_67,                    /* rate 6.7kbps/s */
    MN_CALL_CHANNEL_RATE_74,                    /* rate 7.4kbps/s */
    MN_CALL_CHANNEL_RATE_795,                   /* rate 7.95kbps/s */
    MN_CALL_CHANNEL_RATE_102,                   /* rate 10.2kbps/s */
    MN_CALL_CHANNEL_RATE_122,                   /* rate 12.2kbps/s */
    MN_CALL_CHANNEL_RATE_BUTT                   /* invalid value */
};
typedef VOS_UINT8  MN_CALL_CHANNEL_RATE_ENUM_U8;


enum TAF_CALL_TYPE_ENUM
{
    TAF_CALL_TYPE_NORMAL_CALL,
    TAF_CALL_TYPE_ECALL,
    TAF_CALL_TYPE_BUTT
};
typedef VOS_UINT16 TAF_CALL_TYPE_ENUM_UINT16;

typedef struct
{
    MN_CALL_RADIO_MODE_ENUM_U8          enMode;       /*GSM|WCDMA*/
    MN_CALL_CODEC_TYPE_ENUM_U8          enCodecType;  /* codec type */
    TAF_CALL_TYPE_ENUM_UINT16           enCallType;   /* eCall type */
} MN_CALL_CHANNEL_PARAM_STRU;

typedef struct
{
    VOS_BOOL                             bChannelEnable;    /*GSM|WCDMA*/
    MN_CALL_CHANNEL_PARAM_STRU           stChannelParam;        /* channel rate */
} MN_CALL_CHANNEL_INFO_STRU;

enum MN_CALL_CHANNEL_EVENT_ENUM
{
    MN_CALL_EVT_CHANNEL_OPEN,                                                   /* open codec channel */
    MN_CALL_EVT_CHANNEL_CLOSE,                                                  /* close codec channel */
    MN_CALL_EVT_CHANNEL_PARA_CHANGE,

    MN_CALL_EVT_CHANNEL_LOCAL_ALERTING,
    MN_CALL_EVT_TYPE_BUFF                                                       /* reserved value */
};
typedef  VOS_UINT32  MN_CALL_CHANNEL_EVENT_ENUM_U32;

/** ****************************************************************************
 * Name        : MN_CALL_APP_EC_KMC_DATA_STRU
 *
 * Description :
 *******************************************************************************/
typedef struct
{
    VOS_UINT8                                               ucEccKmcVer;
    VOS_UINT8                                               aucReserved[1];
    VOS_UINT16                                              usEccKmcLength;
    VOS_UINT8                                               aucEccKmcData[TAF_CALL_ECC_KMC_PUBLIC_KEY_MAX_HEX_LENGTH];
} MN_CALL_APP_EC_KMC_DATA_STRU;



enum TAF_CALL_CCWAI_SRV_CLASS_ENUM
{
    TAF_CALL_CCWAI_SRV_CLASS_VOICE               = 1,
    TAF_CALL_CCWAI_SRV_CLASS_VIDEO               = 2,
    TAF_CALL_CCWAI_SRV_CLASS_SET_NW              = 3,

    TAF_CALL_CCWAI_SRV_CLASS_BUTT
};
typedef  VOS_UINT8  TAF_CALL_CCWAI_SRV_CLASS_ENUM_UINT8;

typedef struct
{
    TAF_CALL_CCWAI_MODE_ENUM_UINT8                          enMode;
    TAF_CALL_CCWAI_SRV_CLASS_ENUM_UINT8                     enSrvClass;
    VOS_UINT8                                               aucReserve[2];
}TAF_CALL_CCWAI_SET_REQ_STRU;


typedef union
{
    MN_CALL_ORIG_PARAM_STRU             stOrig;
    MN_CALL_END_PARAM_STRU              stEnd;
    MN_CALL_ANS_PARAM_STRU              stAns;
    TAF_CALL_DTMF_PARAM_STRU            stDtmf;
    MN_CALL_SUPS_PARAM_STRU             stCallMgmtCmd;
    MN_CALL_UUS1_PARAM_STRU             stUus1Info;   /* ����UUS1��Ϣ�ṹ */
    MN_CALL_SET_ALS_PARAM_STRU          stSetAls;     /* ����ALS NO�ṹ   */

    MN_CALL_APP_CUSTOM_ECC_NUM_REQ_STRU stEccNumReq;

    MN_CALL_SET_CSSN_REQ_STRU           stCssnReq;

    MN_CALL_MODIFY_REQ_STRU             stModifyReq;

    TAF_CALL_ECONF_DIAL_REQ_STRU        stEconfDial;

    TAF_CALL_CCWAI_SET_REQ_STRU         stCcwaiSet;

} MN_CALL_APP_REQ_PARM_UNION;

/* Ӧ��������Ϣ�Ľṹ */
typedef struct
{
    VOS_MSG_HEADER
    MN_CALL_APP_REQ_ENUM_UINT32         enReq;
    MN_CLIENT_ID_T                      clientId;
    MN_OPERATION_ID_T                   opId;
    MN_CALL_ID_T                        callId;
    MN_CALL_APP_REQ_PARM_UNION          unParm;
} MN_CALL_APP_REQ_MSG_STRU;


typedef struct
{
    MN_CALL_EVENT_ENUM_U32              enEvent;                                /* �¼��� */
    MN_CLIENT_ID_T                      usClientId;
    TAF_CALL_VOICE_DOMAIN_ENUM_UINT8    enVoiceDomain;                          /* VOICE DOMAIN */
    VOS_UINT8                           aucReserve[1];
    VOS_UINT8                           ucCallNum;
    VOS_UINT8                           aucCallId[MN_CALL_MAX_NUM];
    VOS_UINT8                           aucCurcRptCfg[MN_CALL_RPT_CFG_MAX_SIZE];        /* CURC���õ������ϱ���ʶ */
    VOS_UINT8                           aucUnsolicitedRptCfg[MN_CALL_RPT_CFG_MAX_SIZE]; /* �����������õ������ϱ���ʶ */
} MN_CALL_EVT_HOLD_STRU;


typedef struct
{
    MN_CALL_EVENT_ENUM_U32              enEvent;                                /* �¼��� */
    MN_CLIENT_ID_T                      usClientId;
    TAF_CALL_VOICE_DOMAIN_ENUM_UINT8    enVoiceDomain;                          /* VOICE DOMAIN */
    VOS_UINT8                           aucReserve[1];
    VOS_UINT8                           ucCallNum;
    VOS_UINT8                           aucCallId[MN_CALL_MAX_NUM];
    VOS_UINT8                           aucCurcRptCfg[MN_CALL_RPT_CFG_MAX_SIZE];        /* CURC���õ������ϱ���ʶ */
    VOS_UINT8                           aucUnsolicitedRptCfg[MN_CALL_RPT_CFG_MAX_SIZE]; /* �����������õ������ϱ���ʶ */
} MN_CALL_EVT_RETRIEVE_STRU;


typedef struct
{
    VOS_UINT32                          bitOpPI                 :1;
    VOS_UINT32                          bitOpNoCLICause         :1;
    VOS_UINT32                          bitOpRedirectNum        :1;
    VOS_UINT32                          bitOpRedirectSubaddr    :1;
    VOS_UINT32                          bitOpSpare              :28;
    MN_CALL_PRESENTATION_IND_ENUM_UINT8 enPI;
    MN_CALL_NO_CLI_CAUSE_ENUM_U8        enNoCLICause;
    VOS_UINT8                           aucRsv[2];
    MN_CALL_BCD_NUM_STRU                stRedirectNum;                          /* BCD number */
    MN_CALL_SUBADDR_STRU                stRedirectSubaddr;                      /* Subaddress */
}MN_CALL_REDIRECT_INFO;


typedef struct
{
    MN_CALL_EVENT_ENUM_U32              enEvent;                                /*�ظ����¼�����*/
    VOS_UINT32                          ulRet;
    AT_APPCTRL_STRU                     stAppCtrl;
    MN_CALL_REDIRECT_INFO               stRedirectInfo;
}MN_CALL_CLPR_GET_CNF_STRU;



typedef struct
{
    MN_CALL_EVENT_ENUM_U32              enEventId;          /* �¼�����   */

    AT_APPCTRL_STRU                     stAppCtrl;          /* Ӧ�ÿ���ͷ */
    TAF_CS_CAUSE_ENUM_UINT32            enCause;            /* ������     */
} MN_CALL_EVT_ERR_IND_STRU;


typedef struct
{
    MN_CALL_EVENT_ENUM_U32              enEvent;        /*�ظ����¼�����*/
    TAF_ERROR_CODE_ENUM_UINT32          ulRet;          /*�ظ����*/
    MN_CLIENT_ID_T                      ClientId;       /*�˿ں�*/
    MN_OPERATION_ID_T                   OpId;           /*����ID*/
    VOS_UINT8                           ucReserved;     /* �����ֶ� */
} MN_CALL_SET_CSSN_CNF_STRU;

typedef struct
{
    MN_CALL_EVENT_ENUM_U32              enEvent;                                /* �¼��� */
    MN_CLIENT_ID_T                      usClientId;

    VOS_UINT8                           ucIsLocalAlertingFlag;                  /* �Ƿ�Ϊ���ز��Ż�������ʶ1:���ز��Ż�������0:�������*/
    MN_CALL_CODEC_TYPE_ENUM_U8          enCodecType;                            /* codec type */

    TAF_CALL_VOICE_DOMAIN_ENUM_UINT8    enVoiceDomain;                          /* VOICE DOMAIN */
    VOS_UINT8                           aucRsv[3];
} MN_CALL_EVT_CHANNEL_INFO_STRU;





enum TAF_CALL_DTMF_STATE_ENUM
{
    TAF_CALL_DTMF_IDLE                  = 0,                                    /* ��ǰDTMF���� */
    TAF_CALL_DTMF_WAIT_START_CNF,                                               /* ������START�����ȴ��ظ� */
    TAF_CALL_DTMF_WAIT_STOP_CNF,                                                /* ������STOP�����ȴ��ظ� */
    TAF_CALL_DTMF_WAIT_AUTO_STOP_CNF,                                           /* ��ʱ����ʱ�Զ�����STOP�����ȴ��ظ� */
    TAF_CALL_DTMF_WAIT_ON_LENGTH_TIME_OUT,                                      /* ����DTMF��ʱ����ȴ���ʱ */
    TAF_CALL_DTMF_WAIT_OFF_LENGTH_TIME_OUT,                                     /* �յ�stop dtmf ack����ʱ����ʱ�ٴ�������start dtmf req */

    TAF_CALL_DTMF_STATE_BUTT
};
typedef VOS_UINT8 TAF_CALL_DTMF_STATE_ENUM_UINT8;


typedef struct
{
    MN_CLIENT_ID_T                      usClientId;
    MN_OPERATION_ID_T                   opId;
    VOS_UINT8                           aucReserved1[1];
    TAF_CS_CAUSE_ENUM_UINT32            enCause;                                /* ������ */

    MN_CALL_ID_T                        callId;
    TAF_CALL_DTMF_STATE_ENUM_UINT8      enDtmfState;
    VOS_UINT8                           ucDtmfCnt;
    VOS_UINT8                           aucReserved2[1];

} TAF_CALL_EVT_DTMF_CNF_STRU;


typedef struct
{
    MN_CLIENT_ID_T                      usClientId;
    MN_OPERATION_ID_T                   opId;
    VOS_UINT8                           ucReserved;
    VOS_UINT32                          ulResult;
} TAF_CALL_EVT_CCWAI_CNF_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT16                          usMsgName;                              /* ��Ϣ�� */
    VOS_UINT8                           aucReserve1[2];                         /* ���� */
    VOS_UINT16                          usClientId;                             /* Client ID */
    VOS_UINT8                           ucOpId;                                 /* Operation ID */
    VOS_UINT8                           aucReserve2[1];                         /* ���� */
}MN_CALL_QRY_ECALL_INFO_REQ_STRU;


typedef struct
{
    MN_CALL_ID_T                        ucCallId;                               /* ��ǰ��call ID */
    MN_CALL_TYPE_ENUM_U8                enEcallType;                            /* eCall�������� */
    MN_CALL_STATE_ENUM_U8               enCallState;                            /* ����״̬ */
    VOS_UINT8                           aucReserved[1];                         /* ���� */
    MN_CALL_CALLED_NUM_STRU             stDialNumber;                           /* ���к��� */
}MN_CALL_ECALL_INFO_PARA_STRU;


typedef struct
{
    VOS_UINT8                           ucNumOfEcall;                           /* ��ǰeCall���и��� */
    VOS_UINT8                           aucReserved[3];                         /* ���� */
    MN_CALL_ECALL_INFO_PARA_STRU        astEcallInfos[MN_CALL_MAX_NUM];         /* eCall��Ϣ���� */
}MN_CALL_ECALL_INFO_STRU;


typedef struct
{
    VOS_UINT16                          usClientId;
    VOS_UINT8                           ucOpId;
    VOS_UINT8                           aucReserved[1];
    MN_CALL_ECALL_INFO_STRU             stEcallInfo;
}MN_CALL_EVT_QRY_ECALL_INFO_CNF_STRU;


/* Added by l60609 for CDMA 1X Iteration 2, 2014-9-3, begin */

typedef struct
{
    VOS_UINT8                           ucDigitNum;
    /* ASCII �ַ����Ϸ����ַ���������'0' - '9', '*', '#', '+'��'+'ֻ�ܳ����ں������ǰ�棬
       �������󳤶Ȳ��ܳ���32��������"+"�� */
    VOS_UINT8                           aucDigit[TAF_CALL_MAX_FLASH_DIGIT_LEN + 1];
    VOS_UINT8                           aucReserved[2];
}TAF_CALL_FLASH_PARA_STRU;


typedef struct
{
    VOS_MSG_HEADER                                                  /*_H2ASN_Skip*/
    VOS_UINT16                          usMsgId;                    /*_H2ASN_Skip*//* ��Ϣ�� */
    VOS_UINT8                           aucReserved1[2];            /*_H2ASN_Skip*//* ���� */
    MN_CLIENT_ID_T                      usClientId;                 /*_H2ASN_Skip*/
    MN_OPERATION_ID_T                   ucOpId;                     /*_H2ASN_Skip*/
    VOS_UINT8                           aucReserved2[1];            /*_H2ASN_Skip*/
    TAF_CALL_FLASH_PARA_STRU            stFlashPara;
}TAF_CALL_APP_SEND_FLASH_REQ_STRU;


typedef struct
{
    MN_CLIENT_ID_T                      usClientId;
    MN_OPERATION_ID_T                   ucOpId;
    VOS_UINT8                           ucResult;                               /* VOS_OK:�ɹ�; VOS_ERR:ʧ�� */
}TAF_CALL_EVT_SEND_FLASH_RSLT_STRU;

/* Added by f279542 for CDMA 1X Iteration 4, 2014-11-10, begin */

typedef struct
{
    MN_CALL_ID_T                        ucCallId;                                   /* ����ID�����Я���ò����������ǵ�ǰ���еĺ���ID */
    VOS_UINT8                           aucReserved[3];
    VOS_UINT8                           ucDigitNum;
    VOS_UINT8                           aucDigit[TAF_CALL_MAX_BURST_DTMF_NUM];      /* DTMF Keys: ������Ӧ���ַ�  */
    VOS_UINT32                          ulOnLength;                                 /* DTMF������ʱ������ʱ��(��λms) */
    VOS_UINT32                          ulOffLength;                                /* DTMF�����͵���Сʱ����(��λms)  */
}TAF_CALL_BURST_DTMF_PARA_STRU;


typedef struct
{
    VOS_MSG_HEADER                                                                 /*_H2ASN_Skip*/
    VOS_UINT16                          usMsgId;                                   /*_H2ASN_Skip*/
    VOS_UINT8                           aucReserved1[2];                           /*_H2ASN_Skip*/
    MN_CLIENT_ID_T                      usClientId;                                /*_H2ASN_Skip*/
    MN_OPERATION_ID_T                   ucOpId;                                    /*_H2ASN_Skip*/
    VOS_UINT8                           aucReserved2[1];                           /*_H2ASN_Skip*/
    TAF_CALL_BURST_DTMF_PARA_STRU       stBurstDTMFPara;
} TAF_CALL_BURST_DTMF_REQ_MSG_STRU;
/* Added by f279542 for CDMA 1X Iteration 4, 2014-11-10, end */


typedef struct
{
    VOS_MSG_HEADER                                                                 /*_H2ASN_Skip*/
    VOS_UINT16                          usMsgId;                                   /*_H2ASN_Skip*/
    VOS_UINT8                           aucReserved1[2];                           /*_H2ASN_Skip*/
    MN_CLIENT_ID_T                      usClientId;                                /*_H2ASN_Skip*/
    MN_OPERATION_ID_T                   ucOpId;                                    /*_H2ASN_Skip*/
    VOS_UINT8                           ucCallId;
} TAF_CALL_SND_CCLPR_REQ_MSG_STRU;


typedef struct
{
    MN_CLIENT_ID_T                      usClientId;
    MN_OPERATION_ID_T                   ucOpId;
    VOS_UINT8                           ucCallId;
    TAF_CALL_NUMBER_TYPE_ENUM_UINT8     enNumType;
    TAF_CALL_NUMBER_PLAN_ENUM_UINT8     enNumPlan;
    VOS_UINT8                           ucDigitNum;
    VOS_UINT8                           aucRsved[1];
    VOS_UINT8                           aucDigit[TAF_CALL_MAX_CALLED_NUMBER_CHARI_OCTET_NUM];
}TAF_CALL_EVT_CALLED_NUM_INFO_IND_STRU;


typedef struct
{
    MN_CLIENT_ID_T                      usClientId;
    MN_OPERATION_ID_T                   ucOpId;
    VOS_UINT8                           ucCallId;
    TAF_CALL_NUMBER_TYPE_ENUM_UINT8     enNumType;
    TAF_CALL_NUMBER_PLAN_ENUM_UINT8     enNumPlan;
    VOS_UINT8                           ucPi;
    VOS_UINT8                           ucSi;
    VOS_UINT8                           ucDigitNum;
    VOS_UINT8                           aucRsved[3];
    VOS_UINT8                           aucDigit[TAF_CALL_MAX_CALLING_NUMBER_CHARI_OCTET_NUM];
}TAF_CALL_EVT_CALLING_NUM_INFO_IND_STRU;


typedef struct
{
    MN_CLIENT_ID_T                      usClientId;
    MN_OPERATION_ID_T                   ucOpId;
    VOS_UINT8                           ucCallId;
    VOS_UINT8                           ucDigitNum;
    VOS_UINT8                           aucRsved[3];
    VOS_UINT8                           aucDigit[TAF_CALL_MAX_DISPALY_CHARI_OCTET_NUM];
}TAF_CALL_EVT_DISPLAY_INFO_IND_STRU;


typedef struct
{
    VOS_UINT8                           ucDispalyTag;
    VOS_UINT8                           ucDispalyLen;
    VOS_UINT8                           ucDigitNum;
    VOS_UINT8                           aucRsved[1];
    VOS_UINT8                           aucDigit[TAF_CALL_MAX_EXTENDED_DISPALY_CHARI_OCTET_NUM];
}TAF_CALL_EVT_EXT_DISPLAY_INFO_DATA_STRU;


typedef struct
{
    MN_CLIENT_ID_T                                          usClientId;
    MN_OPERATION_ID_T                                       ucOpId;
    VOS_UINT8                                               ucCallId;
    VOS_UINT8                                               ucInfoRecsDataNum;
    VOS_UINT8                                               ucDisplayType;
    VOS_UINT8                                               ucExtDispInd;
    VOS_UINT8                                               ucReserved;
    TAF_CALL_EVT_EXT_DISPLAY_INFO_DATA_STRU                 aucInfoRecsData[TAF_CALL_MAX_EXT_DISPLAY_DATA_NUM];
}TAF_CALL_EVT_EXT_DISPLAY_INFO_IND_STRU;


typedef struct
{
    MN_CLIENT_ID_T                      usClientId;
    MN_OPERATION_ID_T                   ucOpId;
    VOS_UINT8                           ucCallId;
    TAF_CALL_NUMBER_TYPE_ENUM_UINT8     enNumType;
    TAF_CALL_NUMBER_PLAN_ENUM_UINT8     enNumPlan;
    VOS_UINT8                           ucPi;
    VOS_UINT8                           ucSi;
    VOS_UINT8                           ucDigitNum;
    VOS_UINT8                           aucRsved[3];
    VOS_UINT8                           aucDigit[TAF_CALL_MAX_CONNECTED_NUMBER_CHARI_OCTET_NUM];
}TAF_CALL_EVT_CONN_NUM_INFO_IND_STRU;


typedef struct
{
    MN_CLIENT_ID_T                      usClientId;
    MN_OPERATION_ID_T                   ucOpId;
    VOS_UINT8                           ucCallId;
    VOS_UINT32                          bitOpPi:1;
    VOS_UINT32                          bitOpSi:1;
    VOS_UINT32                          bitOpRedirReason:1;
    VOS_UINT32                          bitOpSpare:29;
    TAF_CALL_NUMBER_TYPE_ENUM_UINT8     enNumType;
    TAF_CALL_NUMBER_PLAN_ENUM_UINT8     enNumPlan;
    VOS_UINT8                           ucPi;
    VOS_UINT8                           ucSi;
    VOS_UINT8                           ucRedirReason;
    VOS_UINT8                           ucDigitNum;
    VOS_UINT8                           aucRsved[2];
    VOS_UINT8                           aucDigitNum[TAF_CALL_MAX_REDIRECTING_NUMBER_CHARI_OCTET_NUM];
}TAF_CALL_EVT_REDIR_NUM_INFO_IND_STRU;


typedef struct
{
    MN_CLIENT_ID_T                      usClientId;
    MN_OPERATION_ID_T                   ucOpId;
    VOS_UINT8                           ucCallId;
    VOS_UINT8                           ucSignalType;
    VOS_UINT8                           ucAlertPitch;
    VOS_UINT8                           ucSignal;
    VOS_UINT8                           aucRsved[1];
}TAF_CALL_EVT_SIGNAL_INFO_IND_STRU;


typedef struct
{
    MN_CLIENT_ID_T                      usClientId;
    MN_OPERATION_ID_T                   ucOpId;
    VOS_UINT8                           ucCallId;
    VOS_UINT8                           ucPolarityIncluded;
    VOS_UINT8                           ucToggleModePresent;
    VOS_UINT8                           ucToggleMode;
    VOS_UINT8                           ucReversePolarityPresent;
    VOS_UINT8                           ucReversePolarity;
    VOS_UINT8                           ucPowerDenialTime;
    VOS_UINT8                           aucReserved[2];
}TAF_CALL_EVT_LINE_CTRL_INFO_IND_STRU;


typedef struct
{
    MN_CLIENT_ID_T                      usClientId;
    MN_OPERATION_ID_T                   ucOpId;
    VOS_UINT8                           ucCallId;
    VOS_UINT8                           ucCallWaitingInd;
    VOS_UINT8                           aucRsved[3];
}TAF_CALL_EVT_CALL_WAITING_IND_STRU;

/* Added by l60609 for CDMA 1X Iteration 2, 2014-9-3, end */


typedef struct
{
    MN_CLIENT_ID_T                      usClientId;
    MN_OPERATION_ID_T                   ucOpId;
    VOS_UINT8                           ucCallId;
    TAF_CALL_NUMBER_TYPE_ENUM_UINT8     enNumType;
    TAF_CALL_NUMBER_PLAN_ENUM_UINT8     enNumPlan;
    VOS_UINT8                           ucPi;
    VOS_UINT8                           ucSi;
    VOS_UINT8                           ucDigitNum;
    VOS_UINT8                           ucSignalType;
    VOS_UINT8                           ucAlertPitch;
    VOS_UINT8                           ucSignal;
    VOS_UINT8                           ucSignalIsPresent;
    VOS_UINT8                           aucRsved[2];
    VOS_UINT8                           aucDigit[TAF_CALL_MAX_CALLING_NUMBER_CHARI_OCTET_NUM + 1];
}TAF_CALL_EVT_CCWAC_INFO_IND_STRU;


typedef struct
{
    MN_CALL_CALLED_NUM_STRU             stDialNumber;
    VOS_UINT32                          ulSo;
    MN_CALL_TYPE_ENUM_U8                enCallType;
    VOS_UINT8                           aucReserved[3];
}TAF_CALL_CUSTOM_DIAL_PARA_STRU;


typedef struct
{
    VOS_MSG_HEADER                                                  /*_H2ASN_Skip*/
    VOS_UINT16                          usMsgId;                    /*_H2ASN_Skip*//* ��Ϣ�� */
    VOS_UINT8                           aucReserved1[2];            /*_H2ASN_Skip*//* ���� */
    MN_CLIENT_ID_T                      usClientId;                 /*_H2ASN_Skip*/
    MN_OPERATION_ID_T                   ucOpId;                     /*_H2ASN_Skip*/
    VOS_UINT8                           aucReserved2[1];            /*_H2ASN_Skip*/
    TAF_CALL_CUSTOM_DIAL_PARA_STRU      stCustomDialPara;
}TAF_CALL_APP_SEND_CUSTOM_DIAL_REQ_STRU;



enum TAF_CALL_CONT_DTMF_SWITCH_ENUM
{
    TAF_CALL_CONT_DTMF_STOP         = 0,
    TAF_CALL_CONT_DTMF_START        = 1,
    TAF_CALL_CONT_DTMF_BUTT
};
typedef VOS_UINT8   TAF_CALL_CONT_DTMF_SWITCH_ENUM_U8;



typedef struct
{
    MN_CALL_ID_T                        ucCallId;                                   /* ����ID, ��ǰ���еĺ���ID */
    TAF_CALL_CONT_DTMF_SWITCH_ENUM_U8   enSwitch;                                   /* enSwitch: TAF_CALL_CONT_DTMF_START,TAF_CALL_CONT_DTMF_STOP  */
    VOS_UINT8                           ucDigit;                                    /* DTMF Keys: ������Ӧ���ַ���1-9��*��#  */
    VOS_UINT8                           ucReserved;
}TAF_CALL_CONT_DTMF_PARA_STRU;


typedef struct
{
    VOS_MSG_HEADER                                                                 /*_H2ASN_Skip*/
    VOS_UINT16                          usMsgId;                                   /*_H2ASN_Skip*/
    VOS_UINT8                           aucReserved1[2];                           /*_H2ASN_Skip*/
    MN_CLIENT_ID_T                      usClientId;                                /*_H2ASN_Skip*/
    MN_OPERATION_ID_T                   ucOpId;                                    /*_H2ASN_Skip*/
    VOS_UINT8                           ucReserved2;                               /*_H2ASN_Skip*/
    TAF_CALL_CONT_DTMF_PARA_STRU        stContDTMFPara;
} TAF_CALL_CONT_DTMF_REQ_MSG_STRU;


enum TAF_CALL_SEND_CONT_DTMF_CNF_RESULT_ENUM
{
    TAF_CALL_SEND_CONT_DTMF_CNF_RESULT_SUCCESS         = 0,
    TAF_CALL_SEND_CONT_DTMF_CNF_RESULT_ABNORMAL_STATE,
    TAF_CALL_SEND_CONT_DTMF_CNF_RESULT_BUFFER_FULL,
    TAF_CALL_SEND_CONT_DTMF_CNF_RESULT_ALLOC_MSG_FAIL,
    TAF_CALL_SEND_CONT_DTMF_CNF_RESULT_ALLOC_MEM_FAIL,

    TAF_CALL_SEND_CONT_DTMF_CNF_RESULT_BUTT
};
typedef VOS_UINT8   TAF_CALL_SEND_CONT_DTMF_CNF_RESULT_ENUM_U8;


typedef struct
{
    MN_CLIENT_ID_T                              usClientId;
    MN_OPERATION_ID_T                           ucOpId;
    MN_CALL_ID_T                                ucCallId;           /* ����ID, ��ǰ���еĺ���ID */
    TAF_CALL_SEND_CONT_DTMF_CNF_RESULT_ENUM_U8  enResult;           /* ����BURST DTMF CNF�Ľ�� */
    VOS_UINT8                                   aucReserved[3];
}TAF_CALL_EVT_SEND_CONT_DTMF_CNF_STRU;


typedef struct
{
    MN_CLIENT_ID_T                      usClientId;
    MN_OPERATION_ID_T                   ucOpId;
    VOS_UINT8                           ucResult;       /* ����ȡֵ:VOS_OK, VOS_ERR*/
}TAF_CALL_EVT_SEND_CONT_DTMF_RSLT_STRU;



typedef struct
{
    MN_CLIENT_ID_T                      usClientId;
    MN_OPERATION_ID_T                   ucOpId;
    MN_CALL_ID_T                        ucCallId;       /* ����ID, ��ǰ���еĺ���ID */
    TAF_CALL_CONT_DTMF_SWITCH_ENUM_U8   enSwitch;       /* ucSwitch: TAF_CALL_CONT_DTMF_STOP,TAF_CALL_CONT_DTMF_START  */
    VOS_UINT8                           ucDigit;        /* 1-9,0,*,# */
    VOS_UINT8                           aucReserved[2];
}TAF_CALL_EVT_RCV_CONT_DTMF_IND_STRU;


typedef struct
{
    MN_CLIENT_ID_T                      usClientId;
    MN_OPERATION_ID_T                   ucOpId;
    MN_CALL_ID_T                        ucCallId;                                   /* ����ID, ��ǰ���еĺ���ID */
    VOS_UINT8                           ucDigitNum;                                 /**<  Digit number */
    VOS_UINT8                           aucDigit[TAF_CALL_MAX_BURST_DTMF_NUM];      /**<  DTMF digits */
    VOS_UINT32                          ulOnLength;                                 /**<  refers to S.0005 3.7.3.3.2.9 */
    VOS_UINT32                          ulOffLength;                                /**<  refers to S.0005 3.7.3.3.2.9 */
}TAF_CALL_EVT_RCV_BURST_DTMF_IND_STRU;

typedef struct
{
    MN_CLIENT_ID_T                              usClientId;
    MN_OPERATION_ID_T                           ucOpId;
    MN_CALL_ID_T                                ucCallId;
    VOS_UINT32                                  ulRet;
    TAF_CALL_PRESENTATION_IND_ENUM_UINT8        enPI;
    VOS_UINT8                                   aucReserved[3];
}TAF_CALL_EVT_CCLPR_GET_CNF_STRU;


enum TAF_CALL_CHANNEL_TYPE_ENUM
{
    TAF_CALL_CHANNEL_TYPE_NONE   = 0x00,                 /* �޴�������Ϣ */
    TAF_CALL_CHANNEL_TYPE_NARROW = 0x01,                 /* ���������ã�խ������ */
    TAF_CALL_CHANEEL_TYPE_WIDE   = 0x02,                 /* ���������ã�������� */

    TAF_CALL_CHANNEL_TYPE_BUTT
};
typedef VOS_UINT8 TAF_CALL_CHANNEL_TYPE_ENUM_UINT8;


typedef struct
{
    AT_APPCTRL_STRU                     stAppCtrl;
    VOS_UINT32                          ulResult;                               /* ��ѯ��� */
    TAF_CALL_CHANNEL_TYPE_ENUM_UINT8    enChannelType;                          /* codec type */
    TAF_CALL_VOICE_DOMAIN_ENUM_UINT8    enVoiceDomain;                          /* VOICE DOMAIN */
    VOS_UINT8                           aucReserved[2];
}TAF_APP_CHANNEL_INFO_QRY_CNF_STRU;


typedef struct
{
    TAF_CALL_CHANNEL_TYPE_ENUM_UINT8    enChannelType;                       /* ��ǰ���������� */
    TAF_CALL_VOICE_DOMAIN_ENUM_UINT8    enVoiceDomain;                       /* VOICE DOMAIN */
    VOS_UINT8                           aucReserved[2];
}TAF_SPM_CHANNEL_INFO_STRU;

/*****************************************************************************
  4 �궨��
*****************************************************************************/


/*****************************************************************************
  5 ȫ�ֱ�������
*****************************************************************************/


/*****************************************************************************
  6 �ӿں�������
*****************************************************************************/


/* Orignate Call */
VOS_UINT32  MN_CALL_Orig(
    MN_CLIENT_ID_T                      clientId,
    MN_OPERATION_ID_T                   opId,
    MN_CALL_ID_T                        *pCallId,
    const MN_CALL_ORIG_PARAM_STRU       *pstOrigParam
);

/* End Call */
VOS_UINT32  MN_CALL_End(
    MN_CLIENT_ID_T                      clientId,
    MN_OPERATION_ID_T                   opId,
    MN_CALL_ID_T                        callId,
    const MN_CALL_END_PARAM_STRU        *pstEndParam
);

/* Send DTMF */
VOS_UINT32  TAF_CALL_SendDtmf(
    AT_MN_MSGTYPE_ENUM_UINT16           enMsgType,
    MN_CLIENT_ID_T                      clientId,
    MN_OPERATION_ID_T                   opId,
    const TAF_CALL_DTMF_PARAM_STRU     *pstDtmfParam
);

/* Call Management Command*/
VOS_UINT32  MN_CALL_Sups(
    MN_CLIENT_ID_T                      clientId,
    MN_OPERATION_ID_T                   opId,
    const MN_CALL_SUPS_PARAM_STRU       *pstCallSupsParam
);

/* Get All Call Info */
VOS_UINT32  MN_CALL_GetCallInfos(
    MN_CLIENT_ID_T                      clientId,
    MN_OPERATION_ID_T                   opId,
    MN_CALL_ID_T                        callId
);

VOS_UINT32 MN_CALL_SetAlsLineNo(
    TAF_UINT8                           ucIndex,
    MN_CALL_ALS_LINE_NO_ENUM_U8         enAlsLine
);

VOS_UINT32 MN_CALL_CheckUus1ParmValid(
    MN_CALL_SET_UUS1_TYPE_ENUM_U32      enSetType,
    MN_CALL_UUS1_INFO_STRU              *pstUus1Info
);

VOS_VOID  MN_CALL_SetUus1Info(
    MN_CLIENT_ID_T                      clientId,
    MN_OPERATION_ID_T                   opId,
    MN_CALL_UUS1_PARAM_STRU            *pstUus1Info
);

VOS_VOID  MN_CALL_QryUus1Info(
    MN_CLIENT_ID_T                      clientId,
    MN_OPERATION_ID_T                   opId
);
VOS_VOID  MN_CALL_SetAls(
    MN_CLIENT_ID_T                      clientId,
    MN_OPERATION_ID_T                   opId,
    MN_CALL_SET_ALS_PARAM_STRU         *pstAls
);
VOS_VOID  MN_CALL_QryAls(
    MN_CLIENT_ID_T                      clientId,
    MN_OPERATION_ID_T                   opId
);
VOS_UINT32  MN_CALL_SendAppRequest(
    MN_CALL_APP_REQ_ENUM_UINT32         enReq,
    MN_CLIENT_ID_T                      clientId,
    MN_OPERATION_ID_T                   opId,
    MN_CALL_ID_T                        callId,
    const MN_CALL_APP_REQ_PARM_UNION    *punParam
);

VOS_UINT32  MN_CALL_QryCdur(
    MN_CLIENT_ID_T                      clientId,
    MN_OPERATION_ID_T                   opId,
    MN_CALL_ID_T                        callId
);

VOS_VOID MN_CALL_RcvTafEccNumReq(
    MN_CALL_APP_CUSTOM_ECC_NUM_REQ_STRU*    pstCustomEccNumReq
);
VOS_VOID MN_CALL_ReportEccNumList(VOS_VOID);


VOS_VOID MN_CALL_RcvAtClprGetReq(MN_CALL_APP_REQ_MSG_STRU            *pstCallMsg);


/*****************************************************************************
 Prototype      : GMM_GetCurNetwork
 Description    : ��ȡGMM�ĵ�ǰ��������
                  HSS 4100 V200R001 ����
                  ��GMMģ��ʹ��
 Input          :
 Output         :
 Return Value   : GMM��������
                  GMM_RABM_CURNET_3G    //3G����
                  GMM_RABM_CURNET_2G    //2G����
 Calls          :
 Called By      :

 History        :
  1.Date        : 2005-12-08
    Author      : Roger Leo
    Modification: Created function
*****************************************************************************/
VOS_UINT8 GMM_GetCurNetwork(VOS_VOID);

/* Added by f279542 for CDMA 1X Iteration 4, 2014-11-12, begin */
extern VOS_UINT32  TAF_XCALL_SendBurstDtmf(
    MN_CLIENT_ID_T                      clientId,
    MN_OPERATION_ID_T                   opId,
    TAF_CALL_BURST_DTMF_PARA_STRU      *pstSndBurstDTMFPara
);

extern VOS_UINT32  TAF_XCALL_SendFlashReq(
    MN_CLIENT_ID_T                      clientId,
    MN_OPERATION_ID_T                   opId,
    TAF_CALL_FLASH_PARA_STRU           *pstFlashPara
);
/* Added by f279542 for CDMA 1X Iteration 4, 2014-11-12, end */

VOS_UINT32  TAF_XCALL_SendCustomDialReq(
    MN_CLIENT_ID_T                      clientId,
    MN_OPERATION_ID_T                   opId,
    TAF_CALL_CUSTOM_DIAL_PARA_STRU     *pstCustomDialPara
);

extern VOS_UINT32  TAF_XCALL_SendContinuousDtmf(
    MN_CLIENT_ID_T                      clientId,
    MN_OPERATION_ID_T                   opId,
    TAF_CALL_CONT_DTMF_PARA_STRU       *pstSndContDTMFPara
);

VOS_UINT32  TAF_XCALL_SendCclpr(
    MN_CLIENT_ID_T                      clientId,
    MN_OPERATION_ID_T                   opId,
    VOS_UINT8                           ucCallId
);

VOS_UINT32 TAF_XCALL_SendEncryptCall(
    VOS_UINT32                                              ulModuleId,
    MN_CLIENT_ID_T                                          usClientId,
    MN_OPERATION_ID_T                                       opId,
    VOS_UINT32                                              enEccVoiceType,
    TAF_ECC_CALL_BCD_NUM_STRU                              *pstDialNumber
);
VOS_UINT32 TAF_XCALL_SendEccCtrl(
    VOS_UINT32                                              ulModuleId,
    MN_CLIENT_ID_T                                          usClientId,
    MN_OPERATION_ID_T                                       opId,
    VOS_UINT32                                              enRemoteCtrlEvtType,
    VOS_UINT32                                              enResult
);
VOS_UINT32 TAF_XCALL_SetEccCap(
    VOS_UINT32                                              ulModuleId,
    MN_CLIENT_ID_T                                          usClientId,
    MN_OPERATION_ID_T                                       opId,
    VOS_UINT32                                              ulEccSrvCap,
    VOS_UINT32                                              ulEccSrvStatus
);
VOS_UINT32 TAF_XCALL_SetEccTestMode(
    VOS_UINT32                                              ulModuleId,
    MN_CLIENT_ID_T                                          usClientId,
    MN_OPERATION_ID_T                                       opId,
    VOS_UINT32                                              ulTestMode
);
VOS_UINT32 TAF_XCALL_SetEncryptCallKmc(
    VOS_UINT32                                              ulModuleId,
    MN_CLIENT_ID_T                                          usClientId,
    MN_OPERATION_ID_T                                       opId,
    MN_CALL_APP_EC_KMC_DATA_STRU                          *pstEccKmcData
);
VOS_UINT32 TAF_XCALL_QryEncryptCallCap(
    VOS_UINT32                                              ulModuleId,
    MN_CLIENT_ID_T                                          usClientId,
    MN_OPERATION_ID_T                                       opId
);
VOS_UINT32 TAF_XCALL_QryEncryptCallRandom(
    VOS_UINT32                                              ulModuleId,
    MN_CLIENT_ID_T                                          usClientId,
    MN_OPERATION_ID_T                                       opId
);
VOS_UINT32 TAF_XCALL_QryEncryptCallKmc(
    VOS_UINT32                                              ulModuleId,
    MN_CLIENT_ID_T                                          usClientId,
    MN_OPERATION_ID_T                                       opId
);
VOS_UINT32 TAF_XCALL_QryEncryptCallTestMode(
    VOS_UINT32                                              ulModuleId,
    MN_CLIENT_ID_T                                          usClientId,
    MN_OPERATION_ID_T                                       opId
);
VOS_UINT32 TAF_XCALL_SetPrivacyModePreferred(
    VOS_UINT32                                              ulModuleId,
    MN_CLIENT_ID_T                                          usClientId,
    MN_OPERATION_ID_T                                       opId,
    VOS_UINT32                                              ulPrivacyMode
);

VOS_UINT32 TAF_XCALL_QryPrivacyModePreferred(
    VOS_UINT32                                              ulModuleId,
    MN_CLIENT_ID_T                                          usClientId,
    MN_OPERATION_ID_T                                       opId
);


VOS_UINT32 TAF_CALL_QryCnap(
    VOS_UINT32                              ulModuleId,
    MN_CLIENT_ID_T                          usClientId,
    MN_OPERATION_ID_T                       opId
);

#if ((VOS_OS_VER == VOS_WIN32) || (VOS_OS_VER == VOS_NUCLEUS))
#pragma pack()
#else
#pragma pack(0)
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* MN_CALL_API_H */

