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
#ifndef __NASOMINTERFACE_H__
#define __NASOMINTERFACE_H__

#ifdef __cplusplus
#if __cplusplus
    extern "C" {
#endif
#endif

/*****************************************************************************
  1 ����ͷ�ļ�����
*****************************************************************************/

#pragma pack(4)

/*****************************************************************************
  2 �궨��
*****************************************************************************/


#define LEN_LAI                                             5
#define LEN_RAI                                             6

#define NAS_OM_EVENT_NO_PARA                                0
#define NAS_OM_EVENT_SERVICE_REJ_LEN                        2
#define NAS_OM_EVENT_ATTACH_REQ_LEN                         1
#define NAS_OM_EVENT_ATTACH_FAIL_LEN                        2
#define NAS_OM_EVENT_PDP_ACTIVE_REQ_LEN                     1
#define NAS_OM_EVENT_PDP_ACTIVE_FAIL_LEN                    1
#define NAS_OM_EVENT_PDP_DEACTIVE_REQ_LEN                   1
#define NAS_OM_EVENT_PDP_CONTEXT_MODIFY_REQ_LEN             1
#define NAS_OM_EVENT_PDP_CONTEXT_MODIFY_FAIL_LEN            1
#define NAS_OM_EVENT_RAU_REQ_LEN                            1
#define NAS_OM_EVENT_RAU_FAIL_LEN                           2
#define NAS_OM_EVENT_AUTH_AND_CIPHER_FAIL_LEN               1
#define NAS_OM_EVENT_LAU_REQ_LEN                            1
#define NAS_OM_EVENT_LAU_FAIL_LEN                           2
#define NAS_OM_EVENT_AUTH_FAIL_LEN                          1
#define NAS_OM_EVENT_SMS_MO_REPORT_LEN                      2
#define NAS_OM_EVENT_SMS_RECEIVE_LEN                        1
#define NAS_OM_EVENT_SMS_MT_NOTIFY_LEN                      1
#define NAS_OM_EVENT_HPLMN_TIMER_START_LEN                  4
#define NAS_OM_EVENT_STK_ACCESS_TECHNOLOGY_CHANGE_LEN       1
#define NAS_OM_EVENT_STK_NETWORK_SEARCH_MODE_CHANGE_LEN     1

/*OTA���: ��Ҫ��OMȷ�������ֵ*/
#define OM_NAS_OTA_REQ    0xC101
#define NAS_OM_OTA_CNF    0xC102
#define NAS_OM_OTA_IND    0xC103

#define VOS_MSG_HEADER  VOS_UINT32 ulSenderCpuId;  \
                        VOS_UINT32 ulSenderPid;    \
                        VOS_UINT32 ulReceiverCpuId;\
                        VOS_UINT32 ulReceiverPid;  \
                        VOS_UINT32 ulLength;

/* sizeof(VOS_MSG_HEADER) + sizeof(usTransPrimId) + sizeof(usRsv1)
 + sizeof(ucFuncType)     + sizeof(ucReserve)    + sizeof(usLength)*/
#define NAS_OM_TRANS_MSG_OM_HEADER_LEN    28

#define ID_NAS_OM_TRANS_MSG               0x5001                /* NAS��OM ��͸����Ϣԭ�� */



#define SM_OM_PDP_ACTIVATED               0
#define SM_OM_PDP_NOT_ACTIVATED           1

/* ˫��˫����Ŀ�пտ���Ϣ��trans��Ϣ����modem0��modem1 */
#define NAS_OM_FUNCTION_TYPE_MODEM_MASK   (0xc0)
#define NAS_OM_FUNCTION_TYPE_VALUE_MASK   (0x3f)
#define NAS_MAX_TIMER_EVENT               (100)

/*****************************************************************************
  3 ö�ٶ���
*****************************************************************************/

typedef enum
{
    NAS_OM_EVENT_DATA_SERVICE_REQ           = 0,
    NAS_OM_EVENT_DATA_SERVICE_ACC           = 1,
    NAS_OM_EVENT_DATA_SERVICE_REJ           = 2,             /*_H2ASN_MsgChoice NAS_OM_MM_CAUSE_ENUM*/
    NAS_OM_EVENT_LOCATION_UPDATE_REQ        = 3,             /*_H2ASN_MsgChoice NAS_OM_MM_LAU_TYPE_ENUM*/
    NAS_OM_EVENT_LOCATION_UPDATE_SUCCESS    = 4,
    NAS_OM_EVENT_LOCATION_UPDATE_FAILURE    = 5,             /*_H2ASN_MsgChoice NAS_OM_MM_CAUSE_ENUM*/
    NAS_OM_EVENT_ATTACH_REQ                 = 6,             /*_H2ASN_MsgChoice NAS_OM_GMM_ATTACH_TYPE_ENUM*/
    NAS_OM_EVENT_ATTACH_SUCC                = 7,
    NAS_OM_EVENT_ATTACH_FAIL                = 8,             /*_H2ASN_MsgChoice NAS_OM_MM_CAUSE_ENUM*/
    NAS_OM_EVENT_PDP_ACTIVE_REQ             = 9,             /*_H2ASN_MsgChoice NAS_OM_SM_ACTIVE_REQUEST_TYPE*/
    NAS_OM_EVENT_PDP_ACTIVE_SUCC            = 10,
    NAS_OM_EVENT_PDP_ACTIVE_FAIL            = 11,            /*_H2ASN_MsgChoice NAS_OM_SM_CAUSE_ENUM*/
    NAS_OM_EVENT_PDP_DEACTIVE_REQ           = 12,            /*_H2ASN_MsgChoice NAS_OM_SM_DEACTIVE_REQUEST_TYPE*/
    NAS_OM_EVENT_PDP_CONTEXT_MODIFY_REQ     = 13,            /*_H2ASN_MsgChoice NAS_OM_SM_MODIFY_REQUEST_TYPE*/
    NAS_OM_EVENT_PDP_CONTEXT_MODIFY_SUCC    = 14,
    NAS_OM_EVENT_PDP_CONTEXT_MODIFY_FAIL    = 15,            /*_H2ASN_MsgChoice NAS_OM_SM_CAUSE_ENUM*/
    NAS_OM_EVENT_RAU_REQ                    = 16,            /*_H2ASN_MsgChoice NAS_OM_GMM_RAU_TYPE_ENUM*/
    NAS_OM_EVENT_RAU_SUCC                   = 17,
    NAS_OM_EVENT_RAU_FAIL                   = 18,            /*_H2ASN_MsgChoice NAS_OM_MM_CAUSE_ENUM*/
    NAS_OM_EVENT_DETACH_FINISH              = 19,
    NAS_OM_EVENT_AUTH_REQ                   = 20,
    NAS_OM_EVENT_AUTH_SUCCESS               = 21,
    NAS_OM_EVENT_AUTH_FAILURE               = 22,            /*_H2ASN_MsgChoice NAS_OM_MM_CAUSE_ENUM*/
    NAS_OM_EVENT_AUTH_REJECT                = 23,
    NAS_OM_EVENT_AUTH_AND_CIPHER_REQ        = 24,
    NAS_OM_EVENT_AUTH_AND_CIPHER_SUCC       = 25,
    NAS_OM_EVENT_AUTH_AND_CIPHER_FAIL       = 26,            /*_H2ASN_MsgChoice NAS_OM_MM_CAUSE_ENUM*/
    NAS_OM_EVENT_AUTH_AND_CIPHER_REJ        = 27,
    NAS_OM_EVENT_PLMN_SELECTION_START       = 28,
    NAS_OM_EVENT_PLMN_SELECTION_SUCCESS     = 29,
    NAS_OM_EVENT_PLMN_SELECTION_FAILURE     = 30,
    NAS_OM_EVENT_COVERAGE_LOST              = 31,
    NAS_OM_EVENT_SMS_RECEIVE                = 32,
    NAS_OM_EVENT_SMS_SENT                   = 33,
    /*����Ϣ����ȷ��    */
    NAS_OM_EVENT_SMS_MO_SUCC                = 34,            /*_H2ASN_MsgChoice NAS_OM_SMS_MO_REPORT_STRU*/
    NAS_OM_EVENT_SMS_MO_FAIL                = 35,            /*_H2ASN_MsgChoice NAS_OM_SMS_MO_REPORT_STRU*/
    /*����Ϣ״̬����    */
    NAS_OM_EVENT_SMS_MT_NOTIFY              = 36,
    /*����Ϣ�洢״̬����*/
    NAS_OM_EVENT_SMS_STATICS                = 37,            /*_H2ASN_MsgChoice NAS_OM_SMS_STATICS_STRU*/
    NAS_OM_EVENT_READY_TIMER_START          = 38,
    NAS_OM_EVENT_READY_TIMER_STOP           = 39,
    NAS_OM_EVENT_IMSI_DETACH_IND            = 40,
    NAS_OM_EVENT_CS_SERVICE                 = 41,
    NAS_OM_EVENT_PS_SERVICE                 = 42,
    NAS_OM_EVENT_CS_PS_SERVICE              = 43,
    NAS_OM_EVENT_LIMITED_SERVICE            = 44,
    NAS_OM_EVENT_EPLMN_LIST                 = 45,
    NAS_OM_EVENT_FPLMN_LIST                 = 46,
    NAS_OM_EVENT_CC_MO_ATTEMPT              = 47,
    NAS_OM_EVENT_CC_MO_ALERTING             = 48,
    NAS_OM_EVENT_CC_MO_ANSWERED             = 49,
    NAS_OM_EVENT_CC_MO_DISCONNECT           = 50,
    NAS_OM_EVENT_CC_MT_ATTEMPT              = 51,
    NAS_OM_EVENT_CC_MT_ALERTING             = 52,
    NAS_OM_EVENT_CC_MT_ANSWERED             = 53,
    NAS_OM_EVENT_CC_MT_DISCONNECT           = 54,
    NAS_OM_EVENT_HPLMN_TIMER_START          = 55,
    NAS_OM_EVENT_HPLMN_TIMER_STOP           = 56,
    NAS_OM_EVENT_HPLMN_TIMER_EXPIRE         = 57,
    NAS_OM_EVENT_DTE_UP_DTR                 = 58,
    NAS_OM_EVENT_DTE_DOWN_DTR               = 59,
    NAS_OM_EVENT_DCE_UP_DSR                 = 60,
    NAS_OM_EVENT_DCE_DOWN_DSR               = 61,
    NAS_OM_EVENT_DCE_UP_DCD                 = 62,
    NAS_OM_EVENT_DCE_DOWN_DCD               = 63,
    NAS_OM_EVENT_DCE_UP_CTS                 = 64,
    NAS_OM_EVENT_DCE_DOWN_CTS               = 65,
    NAS_OM_EVENT_DCE_UP_RI                  = 66,
    NAS_OM_EVENT_DCE_DOWN_RI                = 67,
    NAS_OM_EVENT_DCE_CREATE_PPP             = 68,
    NAS_OM_EVENT_DTE_RELEASE_PPP            = 69,
    NAS_OM_EVENT_DTE_DOWN_DTR_RELEASE_PPP_IP_TYPE   = 70,
    NAS_OM_EVENT_DTE_DOWN_DTR_RELEASE_PPP_PPP_TYPE  = 71,

    NAS_OM_EVENT_LOCATION_STATUS_EVENT      = 72,
    NAS_OM_EVENT_STK_ACCESS_TECHNOLOGY_CHANGE_EVENT = 73,
    NAS_OM_EVENT_STK_NETWORK_SEARCH_MODE_CHANGE_EVENT   = 74,

    NAS_OM_EVENT_TIMER_OPERATION_START      = 75,
    NAS_OM_EVENT_TIMER_OPERATION_STOP       = 76,
    NAS_OM_EVENT_TIMER_OPERATION_EXPIRED    = 77,

    /* Added by z60575 for PA_STAR, 2013-7-19 begin */
    NAS_OM_EVENT_PA_STAR_ABNORMAL           = 78,
    /* Added by z60575 for PA_STAR, 2013-7-19 end */

    NAS_OM_EVENT_SMS_MT_FAIL                = 79,

    NAS_OM_EVENT_1X_SYS_ACQUIRE_SUCCESS     = 80,
    NAS_OM_EVENT_1X_SYS_LOST                = 81,
    NAS_OM_EVENT_HRPD_SYS_ACQUIRE_SUCCESS   = 82,
    NAS_OM_EVENT_HRPD_SYS_LOST              = 83,
    NAS_OM_EVENT_HRPD_SESSION_OPEN          = 84,
    NAS_OM_EVENT_1X_POWER_SAVE              = 85,
    NAS_OM_EVENT_HRPD_POWER_SAVE            = 86,
    NAS_OM_EVENT_HRPD_SESSION_CLOSE         = 87,
    NAS_OM_EVENT_HRPD_SESSION_SUPPORT_EHRPD = 88,
    NAS_OM_EVENT_HRPD_SESSION_NOT_SUPPORT_EHRPD = 89,
    NAS_OM_EVENT_BASTET_SMS_BARRED          = 90,
    NAS_OM_EVENT_RADIO_RESOURCE_CHECK_EXCEPTION          = 91,

    NAS_OM_EVENT_ID_BUTT
}NAS_OM_EVENT_ID_ENUM;

typedef VOS_UINT16 NAS_OM_EVENT_ID_ENUM_UINT16;

typedef enum
{
    NAS_OM_MM_CAUSE_NULL                                  = 0,
    NAS_OM_MM_CAUSE_IMSI_UNKNOWN_IN_HLR                   = 2,
    NAS_OM_MM_CAUSE_ILLEGAL_MS                            = 3,
    NAS_OM_MM_CAUSE_IMSI_UNKNOWN_IN_VLR                   = 4,
    NAS_OM_MM_CAUSE_IMSI_NOT_ACCEPTED                     = 5,
    NAS_OM_MM_CAUSE_ILLEGAL_ME                            = 6,
    NAS_OM_MM_CAUSE_GPRS_SERV_NOT_ALLOW                   = 7,
    NAS_OM_MM_CAUSE_GPRS_SERV_AND_NON_GPRS_SERV_NOT_ALLOW = 8,
    NAS_OM_MM_CAUSE_MS_ID_NOT_DERIVED                     = 9,
    NAS_OM_MM_CAUSE_IMPLICIT_DETACHED                     = 10,
    NAS_OM_MM_CAUSE_PLMN_NOT_ALLOW                        = 11,
    NAS_OM_MM_CAUSE_LA_NOT_ALLOW                          = 12,
    NAS_OM_MM_CAUSE_ROAM_NOT_ALLOW                        = 13,
    NAS_OM_MM_CAUSE_GPRS_SERV_NOT_ALLOW_IN_PLMN           = 14,
    NAS_OM_MM_CAUSE_NO_SUITABL_CELL                       = 15,
    NAS_OM_MM_CAUSE_MSC_UNREACHABLE                       = 16,
    NAS_OM_MM_CAUSE_NETWORK_FAILURE                       = 17,
    NAS_OM_MM_CAUSE_MAC_FAILURE                           = 20,
    NAS_OM_MM_CAUSE_SYNCH_FAILURE                         = 21,
    NAS_OM_MM_CAUSE_PROCEDURE_CONGESTION                  = 22,
    NAS_OM_MM_CAUSE_GSM_AUT_UNACCEPTABLE                  = 23,
    NAS_OM_MM_CAUSE_AUT_NETWORK_REJECT                    = 24,
    NAS_OM_MM_CAUSE_AUT_UMTS_OTHER_FAILURE                = 25,
    NAS_OM_MM_CAUSE_AUT_GSM_OTHER_FAILURE                 = 26,
    NAS_OM_MM_CAUSE_SERV_OPT_NOT_SUPPORT                  = 32,
    NAS_OM_MM_CAUSE_REQ_SERV_OPT_NOT_SUBSCRIBE            = 33,
    NAS_OM_MM_CAUSE_SERV_OPT_OUT_OF_ORDER                 = 34,
    NAS_OM_MM_CAUSE_CALL_CANNOT_IDENTIFY                  = 38,
    NAS_OM_MM_CAUSE_NO_PDP_CONTEXT_ACT                    = 40,
    NAS_OM_MM_CAUSE_RETRY_UPON_ENTRY_CELL                 = 60,
    NAS_OM_MM_CAUSE_RETRY_UPON_ENTRY_CELL_MIN             = 48,
    NAS_OM_MM_CAUSE_RETRY_UPON_ENTRY_CELL_MAX             = 63,
    NAS_OM_MM_CAUSE_SEMANTICALLY_INCORRECT_MSG            = 95,
    NAS_OM_MM_CAUSE_INVALID_MANDATORY_INF                 = 96,
    NAS_OM_MM_CAUSE_MSG_NONEXIST_NOTIMPLEMENTE            = 97,
    NAS_OM_MM_CAUSE_MSG_TYPE_NOT_COMPATIBLE               = 98,
    NAS_OM_MM_CAUSE_IE_NONEXIST_NOTIMPLEMENTED            = 99,
    NAS_OM_MM_CAUSE_CONDITIONAL_IE_ERROR                  = 100,
    NAS_OM_MM_CAUSE_MSG_NOT_COMPATIBLE                    = 101,
    NAS_OM_MM_CAUSE_PROTOCOL_ERROR                        = 111,
    NAS_OM_MM_CAUSE_TIMER_TIMEOUT                         = 251,
    NAS_OM_MM_CAUSE_RR_CONN_EST_FAIL                      = 252,
    NAS_OM_MM_CAUSE_RR_CONN_FAIL                          = 253,
    NAS_OM_MM_CAUSE_ACCESS_BARRED                         = 254,
    NAS_OM_MM_CAUSE_UNSUITABLE_MODE                       = 255,
    NAS_OM_MM_CAUSE_BUTT
}NAS_OM_MM_CAUSE_ENUM;

typedef enum
{
    NAS_OM_SM_REJ_CAUSE_INSUFF_RESOURCE                   = 26,
    NAS_OM_SM_REJ_CAUSE_MISS_OR_UNKNOWN_APN               = 27,
    NAS_OM_SM_REJ_CAUSE_UNKNOWN_PDP_ADDR_OR_TYPE          = 28,
    NAS_OM_SM_REJ_CAUSE_USER_AUTHEN_FAIL                  = 29,
    NAS_OM_SM_REJ_CAUSE_ACT_REJ_BY_GGSN                   = 30,
    NAS_OM_SM_REJ_CAUSE_ACT_REJ_UNSPEC                    = 31,
    NAS_OM_SM_REJ_CAUSE_SERV_OPT_NOT_SUPP                 = 32,
    NAS_OM_SM_REJ_CAUSE_REQ_SERV_OPT_NOT_SUBSCRIB         = 33,
    NAS_OM_SM_REJ_CAUSE_SERV_OPT_TEMP_OUT_ORDER           = 34,
    NAS_OM_SM_REJ_CAUSE_NSAPI_USED                        = 35,
    NAS_OM_SM_REJ_CAUSE_SEMANTIC_ERR_IN_TFT               = 41,
    NAS_OM_SM_REJ_CAUSE_SYNTACTIC_ERR_IN_TFT              = 42,
    NAS_OM_SM_REJ_CAUSE_UNKNOWN_PDP_CONTEXT               = 43,
    NAS_OM_SM_REJ_CAUSE_SEMANTIC_ERR_IN_PF                = 44,
    NAS_OM_SM_REJ_CAUSE_SYNTACTIC_ERR_IN_PF               = 45,
    NAS_OM_SM_REJ_CAUSE_PDP_CONT_WITHOUT_TFT_ACT          = 46,
    NAS_OM_SM_REJ_CAUSE_INVALID_TI                        = 81,
    NAS_OM_SM_REJ_CAUSE_INVALID_MANDATORY_IE              = 96,
    NAS_OM_SM_REJ_CAUSE_MSGTYPE_NOT_EXIT                  = 97,
    NAS_OM_SM_REJ_CAUSE_MSGTYPE_NOT_COMP_WITH_STATE       = 98,
    NAS_OM_SM_REJ_CAUSE_STATE_ERR                         = 101,
    NAS_OM_SM_REJ_CAUSE_PROTOCOL_ERR                      = 111,
    NAS_OM_SM_REJ_CAUSE_TIMER_TIMEOUT                     = 201,
    NAS_OM_SM_REJ_CAUSE_GPRS_ATTACH_FAIL                  = 202,
    NAS_OM_SM_REJ_CAUSE_OTHER_PDP_CONT_EXIST              = 203,
    NAS_OM_SM_REJ_CAUSE_NSAPI_IN_USE                      = 204,
    NAS_OM_SM_REJ_CAUSE_COLLISION_WITH_NW                 = 205,
    NAS_OM_SM_REJ_CAUSE_GMM_INIT                          = 206,
    NAS_OM_SM_REJ_CAUSE_BACKOFF_ALG_NOT_ALLOWED           = 207,
    NAS_OM_SM_REJ_CAUSE_BUTT
}NAS_OM_SM_CAUSE_ENUM;

enum NAS_OM_SM_ACTIVE_REQUEST_TYPE
{
    ACTIVE_REQUEST_INIT_TYPE_MO                           = 0x00,
    ACTIVE_REQUEST_INIT_TYPE_MT                           = 0x01,
    ACTIVE_REQUEST_INIT_TYPE_BUTT
};
typedef VOS_UINT8 NAS_SM_ACTIVE_REQUEST_INIT_TYPE_UINT8;

enum NAS_OM_SM_DEACTIVE_REQUEST_TYPE
{
    DEACTIVE_REQUEST_INIT_TYPE_MO                         = 0x00,
    DEACTIVE_REQUEST_INIT_TYPE_MT                         = 0x01,
    DEACTIVE_REQUEST_INIT_TYPE_BUTT
};
typedef VOS_UINT8 NAS_SM_DEACTIVE_REQUEST_INIT_TYPE_UINT8;

enum NAS_OM_SM_MODIFY_REQUEST_TYPE
{
    MODIFY_REQUEST_INIT_TYPE_MO                           = 0x00,
    MODIFY_REQUEST_INIT_TYPE_MT                           = 0x01,
    MODIFY_REQUEST_INIT_TYPE_BUTT
};
typedef VOS_UINT8 NAS_SM_MODIFY_REQUEST_INIT_TYPE_UINT8;

enum NAS_OM_GMM_ATTACH_TYPE_ENUM
{
    NAS_OM_GMM_GPRS_ATTACH                                = 1,
    NAS_OM_GMM_GPRS_ATTACH_WHILE_IMSI_ATTACH              = 2,
    NAS_OM_GMM_COMBINED_GPRS_IMSI_ATTACH                  = 3,
    NAS_OM_GMM_ATTACH_TYPE_BUTT
};
typedef VOS_UINT8 NAS_GMM_ATTACH_TYPE_ENUM_UINT8;

enum NAS_OM_GMM_RAU_TYPE_ENUM
{
    NAS_OM_GMM_RA_UPDATING                                = 0,
    NAS_OM_GMM_COMBINED_RALA_UPDATING                     = 1,
    NAS_OM_GMM_COMBINED_RALAU_WITH_IMSI_ATTACH            = 2,
    NAS_OM_GMM_PERIODC_UPDATING                           = 3,
    NAS_OM_GMM_UPDATING_TYPE_INVALID
};
typedef VOS_UINT8 NAS_GMM_RAU_TYPE_ENUM_UINT8;

enum NAS_OM_SMS_SAVE_AREA_ENUM
{
    NAS_SMS_SAVE_AREA_SM,
    NAS_SMS_SAVE_AREA_ME,
    NAS_SMS_SAVE_AREA_BUTT
};
typedef VOS_UINT8 NAS_OM_SMS_SAVE_AREA_ENUM_UINT8;

enum NAS_OM_MM_LAU_TYPE_ENUM
{
    NAS_OM_MM_LUT_NORMAL_LU                               = 0,
    NAS_OM_MM_LUT_PERIODIC_UPDATING                       = 1,
    NAS_OM_MM_LUT_IMSI_ATTACH                             = 2,
    NAS_OM_MM_LUT_TYPE_BUTT
};
typedef VOS_UINT8 NAS_OM_MM_LAU_TYPE_ENUM_UINT8;














enum NAS_OM_MMC_STATE_ENUM
{
    SWITCH_ON_END                                 = 0,                          /* �ϵ������ʱ���ڴ�״̬�µȴ�ATTACHҪ��   */
    TRYING_PLMN                                   = 1,                          /* ����ע��״̬                             */
    WAIT_FOR_PLMN_LIST                            = 2,                          /* �ȴ�����PLMN list���                    */
    WAIT_FOR_AS_RESULT                            = 3,                          /* �ȴ�ָ������PLMN��suitable cell���      */
    ON_PLMN                                       = 4,                          /* ָʾPS���CS��������һ���ɹ�ע��         */
    LIMIT_SERVICE                                 = 5,                          /* ��ʱMSֻ���ṩ���޷���                   */
    WAIT_FOR_PLMNS                                = 6,                          /* ��������ʧ������״̬                   */
    NOT_ON_PLMN                                   = 7,                          /* �������ֶ�ģʽ���ȴ��û�ѡ��PLMN         */
    NULL_STATE                                    = 8,                          /* MMC�ػ�ʱ�����״̬                      */
    SWITCH_ON_INIT                                = 9,                          /* �ڴ�״̬�³�ʼ��MM���AS��               */
    TC_STATE                                      = 10,                         /* TC����ʱǨ���״̬                       */
    SUSPEND                                       = 11,
    MMC_STATE_BUTT
};
typedef VOS_UINT8 NAS_OM_MMC_STATE_ENUM_UINT8;



/*OTA���*/
enum OM_NAS_OTA_SWITCH_ENUM
{
    NAS_OTA_SWITCH_OFF   = 0,     /* ֹͣ�ϱ��տ���Ϣ */
    NAS_OTA_SWITCH_ON    = 1      /* ��ʼ�ϱ��տ���Ϣ */
};
typedef VOS_UINT32   NAS_OTA_SWITCH_ENUM_UINT32;

enum NAS_OM_OTA_CNF_RESULT_ENUM
{
    NAS_OTA_DIRECTION_UP     = 0,     /* �տ����÷���: ����(MS-->Network) */
    NAS_OTA_DIRECTION_DOWN   = 1      /* �տ����÷���: ����(Network-->MS) */
};

enum NAS_OTA_CNF_ERRCODE_ENUM
{
    NAS_OTA_CNF_ERRCODE_NO    = 0,     /* �տ����óɹ� */
    NAS_OTA_CNF_ERRCODE_YES   = 1      /* �տ�����ʧ�� */
};
typedef VOS_UINT32   NAS_OTA_CNF_ERRCODE_ENUM_UINT32;

typedef VOS_UINT8   NSM_OTA_DIRECTION_ENUM_UINT8;

typedef enum
{
    /*GMM Event*/
    NAS_OTA_MSG_DATA_SERVICE_REQ                =  0x0000,
    NAS_OTA_MSG_DATA_SERVICE_ACC,
    NAS_OTA_MSG_DATA_SERVICE_REJ,
    NAS_OTA_MSG_ATTACH_REQ,
    NAS_OTA_MSG_ATTACH_ACCEPT,
    NAS_OTA_MSG_ATTACH_REJECT,
    NAS_OTA_MSG_ATTACH_COMPLETE,
    NAS_OTA_MSG_RAU_REQ,
    NAS_OTA_MSG_RAU_ACCEPT,
    NAS_OTA_MSG_RAU_REJECT,
    NAS_OTA_MSG_RAU_COMPLETE,
    NAS_OTA_MSG_DETACH_REQ,
    NAS_OTA_MSG_DETACH_REQ_BY_NET,
    NAS_OTA_MSG_DETACH_ACCEPT,
    NAS_OTA_MSG_DETACH_ACCEPT_BY_MS,
    NAS_OTA_MSG_AUTH_AND_CIPHER_REQ,
    NAS_OTA_MSG_AUTH_AND_CIPHER_RESPONSE,
    NAS_OTA_MSG_AUTH_AND_CIPHER_FAIL,
    NAS_OTA_MSG_AUTH_AND_CIPHER_REJ,
    NAS_OTA_MSG_READY_TIMER_START,
    NAS_OTA_MSG_READY_TIMER_STOP,
    NAS_OTA_MSG_P_TMSI_REALLOCATION,
    NAS_OTA_MSG_P_TMSI_REALLOCATION_COMPLETE,
    NAS_OTA_MSG_IDENTITY_REQUEST,
    NAS_OTA_MSG_IDENTITY_RESPONSE,
    NAS_OTA_MSG_GMM_STATUS,
    NAS_OTA_MSG_GMM_STATUS_BY_MS,
    NAS_OTA_MSG_GMM_INFORMATION,

    /*MM Event*/
    NAS_OTA_MSG_IMSI_DETACH_INDICATION          = 0x0100,
    NAS_OTA_MSG_LOCATION_UPDATE_REQUEST,
    NAS_OTA_MSG_LOCATION_UPDATE_ACCEPT,
    NAS_OTA_MSG_LOCATION_UPDATE_REJECT,
    NAS_OTA_MSG_AUTH_REQ,
    NAS_OTA_MSG_AUTH_RSP,
    NAS_OTA_MSG_AUTH_FAILURE,
    NAS_OTA_MSG_AUTH_REJECT,
    NAS_OTA_MSG_ID_REQ,
    NAS_OTA_MSG_ID_RES,
    NAS_OTA_MSG_TMSI_REALLOC_CMD,
    NAS_OTA_MSG_TMSI_REALLOC_CPL,
    NAS_OTA_MSG_CM_SERV_ACCEPT,
    NAS_OTA_MSG_CM_SERV_REJECT,
    NAS_OTA_MSG_CM_SERV_ABORT,
    NAS_OTA_MSG_CM_SERV_REQUEST,
    NAS_OTA_MSG_PAGING_RSP,
    NAS_OTA_MSG_ABORT,
    NAS_OTA_MSG_MM_STA_N2M,
    NAS_OTA_MSG_MM_INFO,
    NAS_OTA_MSG_MM_RR_CONNECTION,
    NAS_OTA_MSG_REEST_REQ,

    NAS_OTA_MSG_MM_STA_M2N,

    /*SM Event*/
    NAS_OTA_MSG_ACT_PDP_CONTEXT_REQ                  = 0x0200,
    NAS_OTA_MSG_ACT_PDP_CONTEXT_ACC,
    NAS_OTA_MSG_ACT_PDP_CONTEXT_REJ,
    NAS_OTA_MSG_REQ_PDP_CONTEXT_ACT,
    NAS_OTA_MSG_REQ_PDP_CONTEXT_REJ,
    NAS_OTA_MSG_DEACT_PDP_CONTEXT_REQ,
    NAS_OTA_MSG_DEACT_PDP_CONTEXT_NET_REQ,
    NAS_OTA_MSG_DEACT_PDP_CONTEXT_ACC,
    NAS_OTA_MSG_DEACT_PDP_CONTEXT_MS_ACC,
    NAS_OTA_MSG_MOD_PDP_CONTEXT_REQ_N2M,
    NAS_OTA_MSG_MOD_PDP_CONTEXT_ACC_M2N,
    NAS_OTA_MSG_MOD_PDP_CONTEXT_REQ_M2N,
    NAS_OTA_MSG_MOD_PDP_CONTEXT_ACC_N2M,
    NAS_OTA_MSG_MOD_PDP_CONTEXT_REJ,
    NAS_OTA_MSG_ACT_SEC_PDP_CONTEXT_REQ,
    NAS_OTA_MSG_ACT_SEC_PDP_CONTEXT_ACC,
    NAS_OTA_MSG_ACT_SEC_PDP_CONTEXT_REJ,
    NAS_OTA_MSG_SM_STATUS,
    NAS_OTA_MSG_SM_MS_STATUS,

    /*SMS Event*/
    NAS_OTA_MSG_SMS_DATA_TYPE_CP_DATA_UP         = 0x0300,
    NAS_OTA_MSG_SMS_DATA_TYPE_CP_ACK_UP,
    NAS_OTA_MSG_SMS_DATA_TYPE_CP_ERROR_UP,
    NAS_OTA_MSG_SMS_DATA_TYPE_CP_DATA_DOWN,
    NAS_OTA_MSG_SMS_DATA_TYPE_CP_ACK_DOWN,
    NAS_OTA_MSG_SMS_DATA_TYPE_CP_ERROR_DOWN,

    /*CC Event*/
    NAS_OTA_MSG_CC_SETUP_DOWN                     = 0x0400,
    NAS_OTA_MSG_CC_SETUP_UP                       = 0x0401,
    NAS_OTA_MSG_CC_EMERGENCY_UP                   = 0x0402,
    NAS_OTA_MSG_CC_RELEASE_DOWN                   = 0x0403,
    NAS_OTA_MSG_CC_RELEASE_UP                     = 0x0404,
    NAS_OTA_MSG_CC_RELEASE_COMPLETE_DOWN          = 0x0405,
    NAS_OTA_MSG_CC_RELEASE_COMPLETE_UP            = 0x0406,
    NAS_OTA_MSG_CC_CALL_PROCEEDING                = 0x0407,
    NAS_OTA_MSG_CC_ALERTING_DOWN                  = 0x0408,
    NAS_OTA_MSG_CC_ALERTING_UP                    = 0x0409,
    NAS_OTA_MSG_CC_CONNECT_DOWN                   = 0x040A,
    NAS_OTA_MSG_CC_CONNECT_UP                     = 0x040B,
    NAS_OTA_MSG_CC_CONNECT_ACK_UP                 = 0x040C,
    NAS_OTA_MSG_CC_CONNECT_ACK_DOWN               = 0x040D,
    NAS_OTA_MSG_CC_DISCONNECT_DOWN                = 0x040E,
    NAS_OTA_MSG_CC_DISCONNECT_UP                  = 0x040F,
    NAS_OTA_MSG_CC_NOTIFY                         = 0x0410,
    NAS_OTA_MSG_CC_HOLD_ACK                       = 0x0411,
    NAS_OTA_MSG_CC_HOLD_REJ                       = 0x0412,
    NAS_OTA_MSG_CC_RETREIVE_ACK                   = 0x0413,
    NAS_OTA_MSG_CC_RETREIVE_REJ                   = 0x0414,
    NAS_OTA_MSG_CC_MODIFY_DOWN                    = 0x0415,
    NAS_OTA_MSG_CC_MODIFY_UP                      = 0x0416,
    NAS_OTA_MSG_CC_MODIFY_COMPLETE_DOWN           = 0x0417,
    NAS_OTA_MSG_CC_MODIFY_COMPLETE_UP             = 0x0418,
    NAS_OTA_MSG_CC_MODIFY_REJ_DOWN                = 0x0419,
    NAS_OTA_MSG_CC_MODIFY_REJ_UP                  = 0x041A,
    NAS_OTA_MSG_CC_PROGRESS                       = 0x041B,
    NAS_OTA_MSG_CC_FACILITY_DOWN                  = 0x041C,
    NAS_OTA_MSG_CC_FACILITY_UP                    = 0x041D,
    NAS_OTA_MSG_CC_START_DTMF_ACK                 = 0x041E,
    NAS_OTA_MSG_CC_START_DTMF_REJ                 = 0x041F,
    NAS_OTA_MSG_CC_STOP_DTMF_ACK                  = 0x0420,
    NAS_OTA_MSG_CC_STATUS_ENQUIRY                 = 0x0421,
    NAS_OTA_MSG_CC_STATUS_DOWN                    = 0x0422,
    NAS_OTA_MSG_CC_STATUS_UP                      = 0x0423,
    NAS_OTA_MSG_CC_START_DTMF                     = 0x0424,
    NAS_OTA_MSG_CC_STOP_DTMF                      = 0x0425,
    NAS_OTA_MSG_CC_CALL_CONFIRM                   = 0x0426,
    NAS_OTA_MSG_CC_HOLD                           = 0x0427,
    NAS_OTA_MSG_CC_RETREIVE                       = 0x0428,
    NAS_OTA_MSG_CC_CCBS_ESTABLISHMENT             = 0x0429,
    NAS_OTA_MSG_CC_CCBS_EST_CNF                   = 0x042A,
    NAS_OTA_MSG_CC_CCBS_RECALL                    = 0x042B,

    /* SS Event */
    NAS_OTA_MSG_SS_REGISTER_DOWN                  = 0x0500,
    NAS_OTA_MSG_SS_REGISTER_UP                    = 0x0501,
    NAS_OTA_MSG_SS_FACILITY_DOWN                  = 0x0502,
    NAS_OTA_MSG_SS_FACILITY_UP                    = 0x0503,
    NAS_OTA_MSG_SS_RELCMPL_DOWN                   = 0x0504,
    NAS_OTA_MSG_SS_RELCMPL_UP                     = 0x0505,

    NAS_OTA_MSG_HLU_LOC_REQUEST                   = 0x0600,
    NAS_OTA_MSG_HLU_LOC_NOTIFICATION              = 0x0601,

    NAS_OTA_MSG_HLU_LOC_ASSIGNMENT                = 0x0603,
    NAS_OTA_MSG_HLU_LOC_COMPLETE_UP               = 0x0604,
    NAS_OTA_MSG_HLU_LOC_COMPLETE_DOWN             = 0x0605,
    NAS_OTA_MSG_HLU_BLOB_REQUEST                  = 0x0606,
    NAS_OTA_MSG_HLU_BLOB_NOTIFICATION             = 0x0607,

    NAS_OTA_MSG_HLU_BLOB_ASSIGNMENT               = 0x0609,
    NAS_OTA_MSG_HLU_BLOB_COMPLETE_UP              = 0x060A,
    NAS_OTA_MSG_HLU_BLOB_COMPLETE_DOWN            = 0x060B,

    NAS_OTA_MSG_HSM_AMP_MSG_TYPE_UATI_REQ         = 0x060C,
    NAS_OTA_MSG_HSM_AMP_MSG_TYPE_UATI_ASSIGN      = 0x060D,
    NAS_OTA_MSG_HSM_AMP_MSG_TYPE_UATI_COMPLETE    = 0x060E,
    NAS_OTA_MSG_HSM_AMP_MSG_TYPE_HARDWAREID_REQ   = 0x060F,
    NAS_OTA_MSG_HSM_AMP_MSG_TYPE_HARDWAREID_RESP  = 0x0610,

    NAS_OTA_MSG_HSM_SMP_MESSAGE_TYPE_SESSION_CLOSE_UP          = 0x0611,
    NAS_OTA_MSG_HSM_SMP_MESSAGE_TYPE_SESSION_CLOSE_DOWN        = 0x0612,
    NAS_OTA_MSG_HSM_SMP_MESSAGE_TYPE_KEEP_ALIVE_REQ_UP         = 0x0613,
    NAS_OTA_MSG_HSM_SMP_MESSAGE_TYPE_KEEP_ALIVE_REQ_DOWN       = 0x0614,
    NAS_OTA_MSG_HSM_SMP_MESSAGE_TYPE_KEEP_ALIVE_RESP_UP        = 0x0615,
    NAS_OTA_MSG_HSM_SMP_MESSAGE_TYPE_KEEP_ALIVE_RESP_DOWN      = 0x0616,

    NAS_OTA_MSG_ID_BUTT
}NAS_OTA_MSG_ID_ENUM;

typedef VOS_UINT16 NAS_OTA_MSG_ID_ENUM_UINT16;


/*****************************************************************************
 ö����    : NAS_OM_REGISTER_STATE_ENUM
 �ṹ˵��  : NAS�ṩ��OM�������ĵ�ǰע��״̬ö��
*****************************************************************************/
enum NAS_OM_REGISTER_STATE_ENUM
{
    NAS_OM_REGISTER_STATE_POWEROFF          = 0,
    NAS_OM_REGISTER_STATE_NOT_REGISTER      = 1,
    NAS_OM_REGISTER_STATE_REGISTER_2G       = 2,
    NAS_OM_REGISTER_STATE_REGISTER_3G       = 3,
    NAS_OM_REGISTER_STATE_REGISTER_4G       = 4,
    NAS_OM_REGISTER_STATE_BUTT
};
typedef VOS_UINT8 NAS_OM_REGISTER_STATE_ENUM_UINT8;

/*****************************************************************************
 ö����    : NAS_OM_SERVICE_TYPE_ENUM
 �ṹ˵��  : NAS�ṩ��OM�������ǰ���е�ҵ������ö��
*****************************************************************************/
enum NAS_OM_SERVICE_TYPE_ENUM
{
    NAS_OM_SERVICE_TYPE_NO_SERVICE          = 0,
    NAS_OM_SERVICE_TYPE_GSM                 = 1,
    NAS_OM_SERVICE_TYPE_WCDMA               = 2,
    NAS_OM_SERVICE_TYPE_HSPA                = 3,
    NAS_OM_SERVICE_TYPE_LTE                 = 4,
    NAS_OM_SERVICE_TYPE_BUTT
};
typedef VOS_UINT8 NAS_OM_SERVICE_TYPE_ENUM_UINT8;

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
    VOS_UINT8                           ucSmsMr;
    VOS_UINT8                           aucReserve[3];
    VOS_UINT32                          ulCause;
}NAS_OM_SMS_MO_REPORT_STRU;

typedef struct
{
    NAS_OM_SMS_SAVE_AREA_ENUM_UINT8 ucMemType;
    VOS_UINT8 ucMemCap;
    VOS_UINT8 ucMemUsed;
}NAS_OM_SMS_STATICS_STRU;

typedef struct
{
    NAS_OM_EVENT_ID_ENUM_UINT16         usEventId;          /*_H2ASN_MsgChoice_Export NAS_OM_EVENT_ID_ENUM_UINT16*/
    VOS_UINT16                          usReserve;
    VOS_UINT32                          ulModuleId;         /*����ģ��PID �����߸���PID����������Ϣ*/
    VOS_UINT8                           aucData[4];

    /***************************************************************************
        _H2ASN_MsgChoice_When_Comment          NAS_OM_EVENT_ID_ENUM_UINT16
    ****************************************************************************/

}NAS_OM_EVENT_IND_STRUCT;















#define NAS_OM_DATA_PTR_LEN          4                                          /* NAS_OM֮�䴫�����ݵ�ָ�볤��*/


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgId;                                /* ԭ��ID                                   */
    VOS_UINT16                          usOriginalId;
    VOS_UINT16                          usTerminalId;
    VOS_UINT32                          ulTimeStamp;
    VOS_UINT32                          ulSN;
    VOS_UINT8                           aucData[NAS_OM_DATA_PTR_LEN];           /* �������Ϣ����                         */
}ID_NAS_OM_INQUIRE_STRU;


typedef struct
{
    VOS_UINT32                          ulItems;
    VOS_UINT32                          aulTimerMsg[NAS_MAX_TIMER_EVENT];
}NAS_TIMER_EVENT_INFO_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgId;                                /* ԭ��ID                                   */

    VOS_UINT16                          usOriginalId;
    VOS_UINT16                          usTerminalId;
    VOS_UINT32                          ulTimeStamp;
    VOS_UINT32                          ulSN;

    VOS_UINT32                          ulCommand;
    NAS_TIMER_EVENT_INFO_STRU           stTimerMsg;
}NAS_OM_CONFIG_TIMER_REPORT_REQ_STRU;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgId;                                /* ԭ��ID                                   */

    VOS_UINT16                          usOriginalId;
    VOS_UINT16                          usTerminalId;
    VOS_UINT32                          ulTimeStamp;
    VOS_UINT32                          ulSN;
}NAS_OM_CONFIG_TIMER_REPORT_CNF_STRU;

#define NAS_OM_DATA_OFFSET              12                                      /* usLength��һ�ֽڵ�aucData��ƫ��*/

typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgId;                                /* ԭ��ID                                   */
    VOS_UINT16                          usOriginalId;
    VOS_UINT16                          usTerminalId;
    VOS_UINT32                          ulTimeStamp;
    VOS_UINT32                          ulSN;                             /* ����ID������֧�ֶ๤�ߣ��ɹ��߶��壬UE��ԭֵ���ؼ��� */
    VOS_UINT8                           aucData[NAS_OM_DATA_PTR_LEN];                             /* �������Ϣ����                         */
}ID_NAS_OM_CNF_STRU;


/*OTA���*/
typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgId;

    VOS_UINT16                          usOriginalId;
    VOS_UINT16                          usTerminalId;
    VOS_UINT32                          ulTimeStamp;
    VOS_UINT32                          ulSN;

    VOS_UINT32                          ulOnOff;                                /* 0 - ֹͣ�ϱ� 1 - ��ʼ�ϱ� */
}OM_NAS_OTA_REQ_STRUCT;


typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgId;

    VOS_UINT16                          usOriginalId;
    VOS_UINT16                          usTerminalId;
    VOS_UINT32                          ulTimeStamp;
    VOS_UINT32                          ulSN;

    VOS_UINT32                          ulErrCode;                              /* �������Ϣ���� */
}NAS_OM_OTA_CNF_STRUCT;

typedef struct
{
    VOS_UINT32    ulModule;
    VOS_UINT32    ulPid;
    VOS_UINT32    ulMsgId;
    VOS_UINT32    ulDirection;
    VOS_UINT32    ulLength;
    VOS_UINT8     aucData[NAS_OM_DATA_PTR_LEN];
}NAS_OM_OTA_IND_STRUCT;

typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT16                          usPrimId;                               /* ԭ��ID                                   */
    VOS_UINT16                          usToolsId;                              /* ����ID������֧�ֶ๤�ߣ��ɹ��߶��壬UE��ԭֵ���ؼ��� */
    VOS_UINT8                           aucData[NAS_OM_DATA_PTR_LEN];                             /* �������Ϣ����                         */
}ID_NAS_OM_CONFIG_TIMER_REPORT_STRU;

/*****************************************************************************
  8 UNION����
*****************************************************************************/

/*****************************************************************************
  9 OTHERS����
*****************************************************************************/

/*****************************************************************************
  10 ��������
*****************************************************************************/
extern VOS_VOID NAS_EventReport(
    VOS_UINT32                          ulPid,
    NAS_OM_EVENT_ID_ENUM                enEventId,
    VOS_VOID                            *pPara,
    VOS_UINT32                          ulLen
);

extern VOS_VOID NAS_SendAirMsgToOM(
    VOS_UINT32                          ulPid,
    VOS_UINT16                          usMsgID,
    VOS_UINT8                           ucUpDown,
    VOS_UINT32                          ulMsgLen,
    VOS_UINT8                           *pucMsg
);

extern VOS_VOID NAS_RcvOmOtaReq( OM_NAS_OTA_REQ_STRUCT *pstOtaReq );

extern NAS_OM_REGISTER_STATE_ENUM_UINT8 NAS_GetRegisterState(VOS_VOID);

extern NAS_OM_SERVICE_TYPE_ENUM_UINT8   NAS_GetServiceType(VOS_VOID);


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

#endif /* end of NasOm.h */

