/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
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

#ifndef __TAFNVINTERFACE_H__
#define __TAFNVINTERFACE_H__

#include "vos.h"
#include "product_config.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  1 Include Headfile
*****************************************************************************/
#if (VOS_OS_VER != VOS_WIN32)
#pragma pack(4)
#else
#pragma pack(push, 4)
#endif

/*****************************************************************************
  2 Macro
*****************************************************************************/

#define AT_NVIM_SETZ_LEN                (16)
#define AT_NOTSUPPORT_STR_LEN           (16)

/* ���ȼ���������� */
#define ADS_UL_QUEUE_SCHEDULER_PRI_MAX  (9)

/*WIFI ��� NV����*/
#define AT_WIFI_BASIC_NV_LEN            (116)
#define AT_WIFI_SEC_NV_LEN              (205)

/*WIFI SSID KEY��󳤶�*/
#define AT_WIFI_SSID_LEN_MAX            (33)
#define AT_WIFI_KEY_LEN_MAX             (27)

/* Add by z60575 for multi_ssid, 2012-9-5 begin */
/* ��Ȩģʽ�ַ������� */
#define AT_WIFI_WLAUTHMODE_LEN          (16)

/* ����ģʽ�ַ������� */
#define AT_WIFI_ENCRYPTIONMODES_LEN     (5)

/* WPA�������ַ������� */
#define AT_WIFI_WLWPAPSK_LEN            (65)

/* ���֧��4��SSID */
#define AT_WIFI_MAX_SSID_NUM            (4)

#define AT_WIFI_KEY_NUM                 (AT_WIFI_MAX_SSID_NUM)

#define TAF_CBA_NVIM_MAX_ETWS_DUP_DETECT_SPEC_MCC_NUM            (5)                 /* ��Чʱ���������������ָ��MCC���� */
#define TAF_CBA_NV_MAX_USER_SPEC_ETWS_MSGID_RANGE_NUM            (2)

/* 9130��չIPv6���˴�����չԭ��ֵ������ */
#define TAF_NV_IPV6_FALLBACK_EXT_CAUSE_MAX_NUM      (20)

#define AT_AP_NVIM_XML_RPT_SRV_URL_LEN              (127)
/* Added by l60609 for XML, 2011-08-11 Begin */
#define AT_AP_XML_RPT_SRV_URL_LEN                   (127)
#define AT_AP_XML_RPT_SRV_URL_STR_LEN               (AT_AP_XML_RPT_SRV_URL_LEN + 1)
#define AT_AP_XML_RPT_INFO_TYPE_LEN                 (127)

/*WEB UI ������󳤶�*/
#define AT_WEBUI_PWD_MAX                            (16)
#define AT_WEBUI_PWD_MAX_SET                        (0)
#define AT_WEBUI_PWD_VERIFY                         (1)

#define AT_AP_NVIM_XML_RPT_INFO_TYPE_LEN            (127)
#define AT_AP_NVIM_XML_RPT_INFO_TYPE_STR_LEN        (AT_AP_NVIM_XML_RPT_INFO_TYPE_LEN + 1)
/* PRODUCT NAME*/
#define AT_PRODUCT_NAME_MAX_NUM                     (29)
#define AT_PRODUCT_NAME_LENGHT                      (AT_PRODUCT_NAME_MAX_NUM + 1)

#define TAF_NVIM_DFS_MAX_PROFILE_NUM                (8)

#define TAF_NVIM_DIFF_DFS_NUM                       (8)

#define TAF_NVIM_MAX_APN_LEN                        (99)
#define TAF_NVIM_MAX_APN_STR_LEN                    (TAF_NVIM_MAX_APN_LEN + 1)

#define AT_MAX_ABORT_CMD_STR_LEN                    (16)
#define AT_MAX_ABORT_RSP_STR_LEN                    (16)

#define AT_NVIM_BODYSARGSM_MAX_PARA_GROUP_NUM       (8)

#define AT_NVIM_RIGHT_PWD_LEN                       (16)

#define TAF_PH_NVIM_MAX_GUL_RAT_NUM                 (3)                 /*AT^syscfgex��acqorder����Ľ��뼼������ */

#define TAF_NVIM_ITEM_IMEI_SIZE                     (16)

#define AT_DISSD_PWD_LEN                            (16)

#define AT_OPWORD_PWD_LEN                           (16)

#define AT_FACINFO_INFO1_LENGTH                     (128)
#define AT_FACINFO_INFO2_LENGTH                     (128)
#define AT_FACINFO_STRING_LENGTH        \
((AT_FACINFO_INFO1_LENGTH + 1) + (AT_FACINFO_INFO2_LENGTH + 1))

#define AT_FACINFO_INFO1_STR_LENGTH                 (AT_FACINFO_INFO1_LENGTH + 1)
#define AT_FACINFO_INFO2_STR_LENGTH                 (AT_FACINFO_INFO2_LENGTH + 1)

#define AT_MDATE_STRING_LENGTH                      (20)

#define MMA_FORB_BAND_NV_MAX_SIZE                   (10)     /* FobBand��NV���� */

#define MMA_OPERTOR_NAME_MAX_SIZE                   (256)

#define TAF_PH_WCDMA_CLASSMAEK1_LEN                         (2)
#define TAF_PH_WCDMA_CLASSMAEK2_LEN                         (4)
#define TAF_PH_WCDMA_CLASSMAEK3_LEN                         (16)

#define TAF_MAX_MFR_ID_LEN                                  (31)
#define TAF_MAX_MFR_ID_STR_LEN                              (TAF_MAX_MFR_ID_LEN + 1)

#define NAS_MMA_NVIM_OPERATOR_NAME_LEN                      (360)

#define TAF_NVIM_ME_PERSONALISATION_PWD_LEN_MAX             (8)

#define TAF_NVIM_MAX_IMSI_LEN                               (15)
#define TAF_NVIM_MAX_IMSI_STR_LEN                           (TAF_NVIM_MAX_IMSI_LEN + 1)

#define TAF_NVIM_MSG_ACTIVE_MESSAGE_MAX_URL_LEN             (160)

/*^AUTHDATA�������û������������󳤶�*/
#define TAF_NVIM_MAX_NDIS_USERNAME_LEN_OF_AUTHDATA          (128)                            /* USERNAME Ϊ127 */
#define TAF_NVIM_MAX_NDIS_PASSWORD_LEN_OF_AUTHDATA          (128)                            /* PASSWORD Ϊ127 */

/*^AUTHDATA������<PLMN>��������󳤶�*/
#define TAF_NVIM_MAX_NDIS_PLMN_LEN                          (7)

#define TAF_NVIM_PDP_PARA_LEN                               (1952)

#define TAF_PH_PRODUCT_NAME_LEN                             (15)
#define TAF_PH_PRODUCT_NAME_STR_LEN                         (TAF_PH_PRODUCT_NAME_LEN + 1)

/* ����ҵ������ṹ��NV���д洢��λ�� */
#define MN_MSG_SRV_PARAM_LEN                                (8)                 /* ����ҵ������ṹ��NV���д洢�ĳ��� */
#define MN_MSG_SRV_RCV_SM_ACT_OFFSET                        (0)                 /* ���Ž����ϱ���ʽ��NV���д洢��ƫ�� */
#define MN_MSG_SRV_RCV_SM_MEM_STORE_OFFSET                  (1)                 /* ���Ž��մ洢������NV���д洢��ƫ�� */
#define MN_MSG_SRV_RCV_STARPT_ACT_OFFSET                    (2)                 /* ����״̬��������ϱ���ʽc */
#define MN_MSG_SRV_RCV_STARPT_MEM_STORE_OFFSET              (3)                 /* ����״̬������մ洢������NV���д洢��ƫ�� */
#define MN_MSG_SRV_CBM_MEM_STORE_OFFSET                     (4)                 /* �㲥���Ž��մ洢������NV���д洢��ƫ�� */
#define MN_MSG_SRV_APP_MEM_STATUS_OFFSET                    (5)                 /* APP���Ž��մ洢���ʿ��ñ�־��NV���д洢��ƫ�ƣ���Ӧ�ֽ�ȡֵ����0:�洢���洢���ʲ����� 1:�洢���д洢���ʿ��� */
#define MN_MSG_SRV_SM_MEM_ENABLE_OFFSET                     (6)                 /* ���Ž��մ洢������NV���д洢��־��NV���д洢��ƫ�ƣ���Ӧ�ֽ�ȡֵ����0:��ʹ�� 1:ʹ�� */
#define MN_MSG_SRV_MO_DOMAIN_PROTOCOL_OFFSET                (7)                 /* ���ŷ�����Э��Ҫ��ʵ�ֵ�ƫ�ƣ���Ӧ�ֽ�ȡֵ����0:��ʹ�� 1:ʹ�� */

#define MN_MSG_MAX_EF_LEN                                   (255)
#define MN_MSG_EFSMSS_PARA_LEN                              (256)
#define MN_MSG_EFSMSP_PARA_LEN                              (256)

#define TAF_PH_SIMLOCK_PLMN_STR_LEN                         (8)                 /* Plmn �Ŷγ��� */
#define TAF_MAX_SIM_LOCK_RANGE_NUM                          (20)

/* WINS������NV��Ľṹ�� */
#define WINS_CONFIG_DISABLE                                 (0)                 /* WINS��ʹ�� */
#define WINS_CONFIG_ENABLE                                  (1)                 /* WINSʹ�� */

#define TAF_CBA_NVIM_MAX_CBMID_RANGE_NUM                    (100)               /* �û������������ϢID��Χ�������ڽ���ģʽ�»�Ҫ�ܵ�CBMIR�ļ���С���� */
#define TAF_CBA_NVIM_MAX_LABEL_NUM                          (16)                /* �û����������������Ϣ�ĳ��ȣ���λBYTE */

#define TAF_SVN_DATA_LENGTH                                 (2)                 /* SVN��Ч���ݳ��� */

/*  ��ǰ֧�ֵ�UMTS��codec���ͽ�����3��,��Э���л�����չ����˴˴�Ԥ����չ���ֶ� */
#define MN_CALL_MAX_UMTS_CODEC_TYPE_NUM                     (7)

#define MN_CALL_NVIM_BC_MAX_SPH_VER_NUM                     (6)
#define MN_CALL_NVIM_MAX_CUSTOM_ECC_NUM                     (20)                /* �û����ƵĽ���������������� */

#define MN_CALL_NVIM_MAX_BCD_NUM_LEN                        (20)

#define TAF_AT_NVIM_CLIENT_CONFIG_LEN                       (64)

#define TAF_AT_NVIM_CLIENT_CFG_LEN                          (96)

#define TAF_NVIM_CBA_MAX_LABEL_NUM                          (16)

#define MTA_BODY_SAR_WBAND_MAX_NUM                          (5)
#define MTA_BODY_SAR_GBAND_MAX_NUM                          (4)

#define MTC_RF_LCD_MIPICLK_MAX_NUM                          (8)                 /* MIPICLK������ */
#define MTC_RF_LCD_MIPICLK_FREQ_MAX_NUM                     (8)                 /* ÿ��MIPICLKӰ������Ƶ�ʸ��� */

#define TAF_NV_BLACK_LIST_MAX_NUM                           (51)


#define TAF_NVIM_MAX_OPER_NAME_SERVICE_PRIO_NUM              (4)
#define TAF_NVIM_STORED_OPER_NAME_NUM                        (3)

#define TAF_NVIM_IMS2CS_CALL_REDIAL_CAUSE_MAX_NUM     (128)       /* IMS --> CS ��Ҫ�ز���ԭ��ֵ������Ϊ128 */
#define TAF_NVIM_IMSA2CS_CALL_REDIAL_CAUSE_MAX_NUM    (64)        /* IMSA --> CS ��Ҫ�ز���ԭ��ֵ������Ϊ128 */


#define TAF_NVIM_CALL_REDIAL_CAUSE_MAX_NUM            (32)
#define TAF_NVIM_SS_RETRY_CAUSE_MAX_NUM               (32)
#define TAF_NVIM_SMS_RETRY_CAUSE_MAX_NUM              (32)
#define TAF_NVIM_SMS_NO_RETRY_RP_CAUSE_MAX_NUM        (32)

#define TAF_NVIM_CALL_RETRY_DISC_CAUSE_MAX_NUM               (64)

#define TAF_NVIM_CALL_TRIG_RESEL_DISC_CAUSE_MAX_NUM      (64)

#define TAF_NVIM_KMC_MSG_MAX_KMC_PUB_KEY_LEN          (48)

#define TAF_NVIM_CTCC_OOS_TIMER_MAX_PHASE              (10)

#define TAF_SIM_FORMAT_PLMN_LEN                        (3)                      /* Sim����ʽ��Plmn���� */
#define TAF_NVIM_OTA_SECURITY_MSG_IMSI_PLMN_MAX_NUM    (6)

#define TAF_NVIM_MAX_1X_NW_NORMAL_REL_REDIAL_PHASE_NUM (4)

#define TAF_NVIM_ICC_ID_MAX                             (11)
#define TAF_NVIM_RPM_RSV_PARA_NUM                       (26)

#define TAF_NVIM_MAX_USER_SYS_CFG_RAT_NUM               (5)

#define MTC_INTRUSION_DEVICE_GROUP_MAX_NUM              (8)
#define MTC_INTRUSION_DEVICE_FREQ_CASE_MAX_NUM          (4)
#define MTC_INTRUSION_DEVICE_FREQ_LIST_MAX_NUM          (12)

#define MTC_MODEM_SCELL_MAX_WEIGHT                      (100)               /* Modem����С�������Ȩ��ֵ */
#define MTC_MODEM_NCELL_MAX_WEIGHT                      (10)                /* Modem���������Ȩ��ֵ */
#define MTC_INTRUSION_FREQ_MAX_WEIGHT                   (10)                /* ��������Ƶ�ʵ����Ȩ��ֵ */

#define TAF_NVIM_MAX_CTCC_USIM_PLMN_NUM                 (10)

#define TAF_NVIM_MAX_CTCC_CSIM_PLMN_NUM                 (10)

#define MN_MSG_MIN_SRV_PARM_LEN                         (28)                  /*EFSMSP�ļ����ٵĳ���*/

#define TAF_NVIM_CHINA_HOME_NETWORK_NUM_MAX                 (5)

typedef VOS_UINT8  MN_CALL_STATE_ENUM_UINT8;

typedef VOS_UINT32  MMA_QUICK_START_STA_UINT32;

#define TAF_NV_SSA_IMS2CS_REDIAL_CAUSE_MAX_NUM                  (172)

/*****************************************************************************
  3 Massage Declare
*****************************************************************************/


/*****************************************************************************
  4 Enum
*****************************************************************************/
/* ME Storage Function On or Off*/
enum MN_MSG_ME_STORAGE_STATUS_ENUM
{
    MN_MSG_ME_STORAGE_DISABLE           = 0x00,
    MN_MSG_ME_STORAGE_ENABLE            = 0x01,
    MN_MSG_ME_STORAGE_BUTT
};
typedef VOS_UINT8 MN_MSG_ME_STORAGE_STATUS_ENUM_UINT8;

enum AT_UART_LINK_TYPE_ENUM
{
    AT_UART_LINK_TYPE_OM        = 1,          /* OMģʽ */
    AT_UART_LINK_TYPE_AT        = 2,          /* ATģʽ */
    AT_UART_LINK_TYPE_BUTT
};
typedef VOS_UINT16 AT_UART_LINK_TYPE_ENUM_UINT16;

enum MMA_CUSTOM_CARDLOCK_OPERATOR_ENUM
{
    MMA_CUSTOM_CARDLOCK_NO_AVAILABLE   = 0,                                     /* 0��Ĭ�ϲ��򿪣�ʹ�ÿ�����д���SIMLOCK */
    MMA_CUSTOM_CARDLOCK_EGYPT_VDF,                                              /* 1���򿪰���VDF���Ƶ�37��SIMLOCK */
    MMA_CUSTOM_CARDLOCK_NORWAY_NETCOM,                                          /* 2����Ų��Netcomm��SIMLOCK */
    MMA_CUSTOM_CARDLOCK_MEXICO_TELCEL,                                          /* 3����ī����TELCEL�����������Ŷ�334020 */
    MMA_CUSTOM_CARDLOCK_DOMINICA_TELCEL,                                        /* 4���򿪶������TELCEL�����������Ŷ�37002��33870��42502 */
    MMA_CUSTOM_CARDLOCK_BUTT
};



enum MTA_WCDMA_BAND_ENUM
{
    MTA_WCDMA_I_2100                    = 0x0001,
    MTA_WCDMA_II_1900,
    MTA_WCDMA_III_1800,
    MTA_WCDMA_IV_1700,
    MTA_WCDMA_V_850,
    MTA_WCDMA_VI_800,
    MTA_WCDMA_VII_2600,
    MTA_WCDMA_VIII_900,
    MTA_WCDMA_IX_J1700,
    /* Ƶ���ݲ�֧��
    MTA_WCDMA_X,
    */
    MTA_WCDMA_XI_1500                   = 0x000B,
    /* ����Ƶ���ݲ�֧��
    MTA_WCDMA_XII,
    MTA_WCDMA_XIII,
    MTA_WCDMA_XIV,
    MTA_WCDMA_XV,
    MTA_WCDMA_XVI,
    MTA_WCDMA_XVII,
    MTA_WCDMA_XVIII,
    */
    MTA_WCDMA_XIX_850                   = 0x0013,

    MTA_WCDMA_BAND_BUTT
};
typedef VOS_UINT16 MTA_WCDMA_BAND_ENUM_UINT16;


enum TAF_NVIM_LC_WORK_CFG_ENUM
{
    TAF_NVIM_LC_INDEPENT_WORK = 0,
    TAF_NVIM_LC_INTER_WORK    = 1,
    TAF_NVIM_LC_WORK_CFG_BUTT
};
typedef VOS_UINT8 TAF_NVIM_LC_WORK_CFG_ENUM_UINT8;


enum TAF_NVIM_LC_RAT_COMBINED_ENUM
{
    TAF_NVIM_LC_RAT_COMBINED_GUL  = 0x55,
    TAF_NVIM_LC_RAT_COMBINED_CL   = 0xAA,
    TAF_NVIM_LC_RAT_COMBINED_BUTT
};
typedef VOS_UINT8 TAF_NVIM_LC_RAT_COMBINED_ENUM_UINT8;


enum MTC_PS_TRANSFER_ENUM
{
    MTC_PS_TRANSFER_NONE                = 0x00,                                 /* ��PS��Ǩ�Ʋ��� */
    MTC_PS_TRANSFER_LOST_AREA           = 0x01,                                 /* ������ѡ���� */
    MTC_PS_TRANSFER_OFF_AREA            = 0x02,                                 /* ������ѡ���� */

    MTC_PS_TRANSFER_SOLUTION_BUTT
};
typedef VOS_UINT8 MTC_PS_TRANSFER_ENUM_UINT8;


enum TAF_NV_CLIENT_CFG_ENUM
{
    TAF_NV_CLIENT_CFG_PCUI              = 0,
    TAF_NV_CLIENT_CFG_CTRL              = 1,
    TAF_NV_CLIENT_CFG_MODEM             = 2,
    TAF_NV_CLIENT_CFG_NDIS              = 3,
    TAF_NV_CLIENT_CFG_UART              = 4,
    TAF_NV_CLIENT_CFG_SOCK              = 5,
    TAF_NV_CLIENT_CFG_APPSOCK           = 6,
    TAF_NV_CLIENT_CFG_HSIC1             = 7,
    TAF_NV_CLIENT_CFG_HSIC2             = 8,
    TAF_NV_CLIENT_CFG_HSIC3             = 9,
    TAF_NV_CLIENT_CFG_HSIC4             = 10,
    TAF_NV_CLIENT_CFG_MUX1              = 11,
    TAF_NV_CLIENT_CFG_MUX2              = 12,
    TAF_NV_CLIENT_CFG_MUX3              = 13,
    TAF_NV_CLIENT_CFG_MUX4              = 14,
    TAF_NV_CLIENT_CFG_MUX5              = 15,
    TAF_NV_CLIENT_CFG_MUX6              = 16,
    TAF_NV_CLIENT_CFG_MUX7              = 17,
    TAF_NV_CLIENT_CFG_MUX8              = 18,
    TAF_NV_CLIENT_CFG_APP               = 19,
    TAF_NV_CLIENT_CFG_APP1              = 20,
    TAF_NV_CLIENT_CFG_APP2              = 21,
    TAF_NV_CLIENT_CFG_APP3              = 22,
    TAF_NV_CLIENT_CFG_APP4              = 23,
    TAF_NV_CLIENT_CFG_APP5              = 24,
    TAF_NV_CLIENT_CFG_APP6              = 25,
    TAF_NV_CLIENT_CFG_APP7              = 26,
    TAF_NV_CLIENT_CFG_APP8              = 27,
    TAF_NV_CLIENT_CFG_APP9              = 28,
    TAF_NV_CLIENT_CFG_APP10             = 29,
    TAF_NV_CLIENT_CFG_APP11             = 30,
    TAF_NV_CLIENT_CFG_APP12             = 31,
    TAF_NV_CLIENT_CFG_APP13             = 32,
    TAF_NV_CLIENT_CFG_APP14             = 33,
    TAF_NV_CLIENT_CFG_APP15             = 34,
    TAF_NV_CLIENT_CFG_APP16             = 35,
    TAF_NV_CLIENT_CFG_APP17             = 36,
    TAF_NV_CLIENT_CFG_APP18             = 37,
    TAF_NV_CLIENT_CFG_APP19             = 38,
    TAF_NV_CLIENT_CFG_APP20             = 39,
    TAF_NV_CLIENT_CFG_APP21             = 40,
    TAF_NV_CLIENT_CFG_APP22             = 41,
    TAF_NV_CLIENT_CFG_APP23             = 42,
    TAF_NV_CLIENT_CFG_APP24             = 43,
    TAF_NV_CLIENT_CFG_APP25             = 44,
    TAF_NV_CLIENT_CFG_APP26             = 45,
    TAF_NV_CLIENT_CFG_HSIC_MODEM        = 46,
    TAF_NV_CLIENT_CFG_HSUART            = 47,
    TAF_NV_CLIENT_CFG_PCUI2             = 48,

    TAF_NV_CLIENT_CFG_APP27             = 49,
    TAF_NV_CLIENT_CFG_APP28             = 50,
    TAF_NV_CLIENT_CFG_APP29             = 51,
    TAF_NV_CLIENT_CFG_APP30             = 52,
    TAF_NV_CLIENT_CFG_APP31             = 53,
    TAF_NV_CLIENT_CFG_APP32             = 54,
    TAF_NV_CLIENT_CFG_APP33             = 55,
    TAF_NV_CLIENT_CFG_APP34             = 56,
    TAF_NV_CLIENT_CFG_APP35             = 57,
    TAF_NV_CLIENT_CFG_APP36             = 58,
    TAF_NV_CLIENT_CFG_APP37             = 59,
    TAF_NV_CLIENT_CFG_APP38             = 60,
    TAF_NV_CLIENT_CFG_APP39             = 61,
    TAF_NV_CLIENT_CFG_APP40             = 62,
    TAF_NV_CLIENT_CFG_APP41             = 63,
    TAF_NV_CLIENT_CFG_APP42             = 64,
    TAF_NV_CLIENT_CFG_APP43             = 65,
    TAF_NV_CLIENT_CFG_APP44             = 66,
    TAF_NV_CLIENT_CFG_APP45             = 67,
    TAF_NV_CLIENT_CFG_APP46             = 68,
    TAF_NV_CLIENT_CFG_APP47             = 69,
    TAF_NV_CLIENT_CFG_APP48             = 70,
    TAF_NV_CLIENT_CFG_APP49             = 71,
    TAF_NV_CLIENT_CFG_APP50             = 72,
    TAF_NV_CLIENT_CFG_APP51             = 73,
    TAF_NV_CLIENT_CFG_APP52             = 74,
    TAF_NV_CLIENT_CFG_MAX               = TAF_AT_NVIM_CLIENT_CFG_LEN
};
typedef VOS_UINT16 TAF_NV_CLIENT_CFG_ENUM_UINT16;

enum TAF_FLASH_DIRECTORY_TYPE_ENUM
{

    /* V3R3�汾��E5��STICK */
    TAF_FLASH_DIRECTORY_TYPE_V3R3E5_V3R3STICK               = 0x00,

    /* V7R2�汾��V3R3�汾��M2M��Ʒ��̬ */
    TAF_FLASH_DIRECTORY_TYPE_V7R2_V3R3M2M                   = 0x01,

    /* V9R1�ֻ� */
    TAF_FLASH_DIRECTORY_TYPE_V9R1PHONE                      = 0x02,

    /* FLASH�ļ��洢��һ·������ǰ��������K3V3V8R1�汾 */
    TAF_FLASH_DIRECTORY_TYPE_K3V3V8R1                       = 0x03,

    TAF_FLASH_DIRECTORY_TYPE_BUTT
};
typedef VOS_UINT8 TAF_FLASH_DIRECTORY_TYPE_ENUM_UINT16;


enum TAF_NVIM_RAT_MODE_ENUM
{
    TAF_NVIM_RAT_MODE_GSM               = 0x01,
    TAF_NVIM_RAT_MODE_WCDMA,
    TAF_NVIM_RAT_MODE_LTE,
    TAF_NVIM_RAT_MODE_CDMA1X,
    TAF_NVIM_RAT_MODE_TDSCDMA,
    TAF_NVIM_RAT_MODE_WIMAX,
    TAF_NVIM_RAT_MODE_EVDO,

    TAF_NVIM_RAT_MODE_BUTT
};
typedef VOS_UINT8 TAF_NVIM_RAT_MODE_ENUM_UINT8;


enum TAF_NVIM_GSM_BAND_ENUM
{
    TAF_NVIM_GSM_BAND_850               = 0,
    TAF_NVIM_GSM_BAND_900,
    TAF_NVIM_GSM_BAND_1800,
    TAF_NVIM_GSM_BAND_1900,

    TAF_NVIM_GSM_BAND_BUTT
};
typedef VOS_UINT16 TAF_NVIM_GSM_BAND_ENUM_UINT16;


enum TAF_NV_ACTIVE_MODEM_MODE_ENUM
{
    TAF_NV_ACTIVE_SINGLE_MODEM              = 0x00,
    TAF_NV_ACTIVE_MULTI_MODEM               = 0x01,

    TAF_NV_ACTIVE_MODEM_MODE_BUTT
};
typedef VOS_UINT8 TAF_NV_ACTIVE_MODEM_MODE_ENUM_UINT8;

/*****************************************************************************
 ö����    : CNAS_XSMS_DATA_REQ_SEND_CHAN_ENUM
 ö��˵��  : ���ŷ��͵��ŵ�ָʾ
*****************************************************************************/
enum TAF_XSMS_DATA_REQ_SEND_CHAN_ENUM
{
    TAF_XSMS_DATA_REQ_SEND_CHAN_TRAFFIC_CHAN   = 0x0,
    TAF_XSMS_DATA_REQ_SEND_CHAN_ACCESS_CHAN    = 0x1,
    TAF_XSMS_DATA_REQ_SEND_CHAN_BOTH           = 0x2,
    TAF_XSMS_DATA_REQ_SEND_CHAN_BUTT
};

typedef VOS_UINT8 TAF_XSMS_DATA_REQ_SEND_CHAN_ENUM_UINT8;

/** ****************************************************************************
 * Name        : TAF_MMA_CFREQ_LOCK_MODE_TYPE_ENUM
 *
 * Description :
 *******************************************************************************/
enum TAF_MMA_CFREQ_LOCK_MODE_TYPE_ENUM
{
    TAF_MMA_CFREQ_LOCK_MODE_OFF         = 0x00,     /* ��ֹ��Ƶ���� */
    TAF_MMA_CFREQ_LOCK_MODE_ON          = 0x01,     /* ������Ƶ���� */
    TAF_MMA_CFREQ_LOCK_MODE_BUTT        = 0x02
};
typedef VOS_UINT8 TAF_MMA_CFREQ_LOCK_MODE_TYPE_ENUM_UINT8;


enum TAF_VC_TTYMODE_ENUM
{
    TAF_VC_TTY_OFF                      = 0,
    TAF_VC_TTY_FULL                     = 1,
    TAF_VC_TTY_VCO                      = 2,
    TAF_VC_TTY_HCO                      = 3,
    TAF_VC_TTYMODE_BUTT
};
typedef VOS_UINT8 TAF_VC_TTYMODE_ENUM_UINT8;


enum TAF_CALL_CCWA_CTRL_MODE_ENUM
{
    TAF_CALL_CCWA_CTRL_BY_NW          = 0,
    TAF_CALL_CCWA_CTRL_BY_UE            = 1,

    TAF_CALL_CCWA_CTRL_MODE_BUTT
};
typedef VOS_UINT8   TAF_CALL_CCWA_CTRL_MODE_ENUM_U8;


enum TAF_CALL_CCWAI_MODE_ENUM
{
    TAF_CALL_CCWAI_MODE_DISABLE               = 0,
    TAF_CALL_CCWAI_MODE_ENABLE                = 1,

    TAF_CALL_CCWAI_MODE_BUTT
};
typedef  VOS_UINT8  TAF_CALL_CCWAI_MODE_ENUM_UINT8;


enum TAF_XSMS_TL_ACK_NEED_LEVEL_ENUM
{
    TAF_XSMS_TL_ACK_NEED_LEVEL_ALWAYS_NEEDED                    = 0,
    TAF_XSMS_TL_ACK_NEED_LEVEL_NOT_NEEDED_WHEN_RCV_SMS          = 1,  /* �����Ͷ��Ź����У��յ����ŵ�ʱ�򣬿��Բ��ж�TL_ACK */
    TAF_XSMS_TL_ACK_NEED_LEVEL_ALWAYS_NOT_NEEDED                = 2,  /* �����Ͷ��Ź����У�����Ҫ�ж�TL_ACK */

    TAF_XSMS_TL_ACK_NEED_LEVEL_BUTT
};
typedef VOS_UINT8 TAF_XSMS_TL_ACK_NEED_LEVEL_ENUM_UINT8;



enum TAF_NV_GPS_CHIP_TYPE_ENUM
{
    TAF_NV_GPS_CHIP_BROADCOM          = 0,
    TAF_NV_GPS_CHIP_HISI1102          = 1,

    TAF_NV_GPS_CHIP_BUTT
};
typedef VOS_UINT8 TAF_NV_GPS_CHIP_TYPE_ENUM_UINT8;

enum TAF_SPM_NVIM_MULTIMODE_EMC_CS_PRFER_TYPE_ENUM
{
    TAF_SPM_NVIM_MULTIMODE_EMC_CS_NOT_PREFER                = 0,
    TAF_SPM_NVIM_MULTIMODE_EMC_CS_3GPP_PRFER                = 1,
    TAF_SPM_NVIM_MULTIMODE_EMC_CS_3GPP2_PRFER               = 2,

    TAF_SPM_NVIM_MULTIMODE_EMC_CS_PRFER_BUTT
};
typedef VOS_UINT8 TAF_SPM_NVIM_MULTIMODE_EMC_CS_PRFER_TYPE_ENUM_UINT8;


enum TAF_LSMS_RESEND_FLAG_ENUM
{
    TAF_LSMS_RESEND_FLAG_DISABLE          = 0,  /* ��ֹ�ط� */
    TAF_LSMS_RESEND_FLAG_ENABLE           = 1,  /* ʹ���ط� */

    TAF_LSMS_RESEND_FLAG_BUTT
};
typedef VOS_UINT8 TAF_LSMS_RESEND_FLAG_ENUM_UINT8;


enum TAF_IMS2CS_REDIAL_AFTER_ALERTING_DOMAIN_ENUM
{
    TAF_IMS2CS_REDIAL_AFTER_ALERTING_NONE                   = 0,                /* ��ʾ��֧���������ز� */
    TAF_IMS2CS_REDIAL_AFTER_ALERTING_WIFI                   = 1,                /* VOWIFI */
    TAF_IMS2CS_REDIAL_AFTER_ALERTING_LTE                    = 2,                /* IMS */
    TAF_IMS2CS_REDIAL_AFTER_ALERTING_WIFI_LTE               = 3,                /* IMS & WIFI */

    TAF_IMSCALL_2_CS_REDIAL_AFTER_ALERTING_BUTT
};
typedef VOS_UINT8 TAF_IMS2CS_REDIAL_AFTER_ALERTING_DOMAIN_ENUM_UINT8;

/*****************************************************************************
  5 STRUCT
*****************************************************************************/

typedef struct
{
    VOS_INT8    acTz[AT_NVIM_SETZ_LEN];
}TAF_AT_TZ_STRU;


typedef struct
{
    VOS_INT8    acErrorText[AT_NOTSUPPORT_STR_LEN];
}TAF_AT_NOT_SUPPORT_CMD_ERROR_TEXT_STRU;


typedef struct
{
    /* ��NV���״̬ 0:δʹ�ܣ���ʾ���������ȼ����ȼ����ȴ��� 1:ʹ�ܣ��������ȼ��㷨 */
    VOS_UINT32                              ulStatus;

    /* ���ж������ȼ��ļ�Ȩ�� */
    VOS_UINT16                              ausPriWeightedNum[ADS_UL_QUEUE_SCHEDULER_PRI_MAX];
    VOS_UINT8                               aucRsv[2];
}ADS_UL_QUEUE_SCHEDULER_PRI_NV_STRU;



typedef struct
{

    VOS_UINT8    aucWifiSsid[AT_WIFI_MAX_SSID_NUM][AT_WIFI_SSID_LEN_MAX];
    VOS_UINT8    aucReserved[84];
}TAF_AT_MULTI_WIFI_SSID_STRU;


typedef struct
{
    VOS_UINT8    aucWifiAuthmode[AT_WIFI_WLAUTHMODE_LEN];
    VOS_UINT8    aucWifiBasicencryptionmodes[AT_WIFI_ENCRYPTIONMODES_LEN];
    VOS_UINT8    aucWifiWpaencryptionmodes[AT_WIFI_ENCRYPTIONMODES_LEN];
    VOS_UINT8    aucWifiWepKey1[AT_WIFI_MAX_SSID_NUM][AT_WIFI_KEY_LEN_MAX];
    VOS_UINT8    aucWifiWepKey2[AT_WIFI_MAX_SSID_NUM][AT_WIFI_KEY_LEN_MAX];
    VOS_UINT8    aucWifiWepKey3[AT_WIFI_MAX_SSID_NUM][AT_WIFI_KEY_LEN_MAX];
    VOS_UINT8    aucWifiWepKey4[AT_WIFI_MAX_SSID_NUM][AT_WIFI_KEY_LEN_MAX];
    VOS_UINT8    ucWifiWepKeyIndex[AT_WIFI_MAX_SSID_NUM];
    VOS_UINT8    aucWifiWpapsk[AT_WIFI_MAX_SSID_NUM][AT_WIFI_WLWPAPSK_LEN];
    VOS_UINT8    ucWifiWpsenbl;
    VOS_UINT8    ucWifiWpscfg;
}TAF_AT_MULTI_WIFI_SEC_STRU;


typedef struct
{
    VOS_UINT8 aucApRptSrvUrl[AT_AP_XML_RPT_SRV_URL_STR_LEN];
}TAF_AT_NVIM_AP_RPT_SRV_URL_STRU;


typedef struct
{
    VOS_UINT8 aucWebPwd[AT_WEBUI_PWD_MAX];
}TAF_AT_NVIM_WEB_ADMIN_PASSWORD_STRU;


typedef struct
{
    VOS_UINT8 aucApXmlInfoType[AT_AP_NVIM_XML_RPT_INFO_TYPE_STR_LEN];
}TAF_AT_NVIM_AP_XML_INFO_TYPE_STRU;


/*lint -e958 -e959 �޸���:l60609;ԭ��:64bit*/
typedef struct
{
    VOS_UINT32                          ulNvStatus;
    VOS_UINT8                           aucProductId[AT_PRODUCT_NAME_LENGHT];   /* product id */
    VOS_UINT8                           ucReserve1;
    VOS_UINT8                           ucReserve2;
}TAF_AT_PRODUCT_ID_STRU;
/*lint +e958 +e959 �޸���:l60609;ԭ��:64bit*/

/* ����TIM���Ŵ�����NV��ṹ�� */

typedef struct
{
    VOS_UINT8                           ucStatus;                               /* 1: NV��Ч��־λ��0����Ч */
    VOS_UINT8                           ucErrCodeRpt;                           /*�������ϱ���־1: 0�����ϱ�,  1���ϱ�*/
}NAS_NV_PPP_DIAL_ERR_CODE_STRU;


typedef struct
{
    VOS_UINT8                           ucAbortEnableFlg;                           /* AT��Ͽ��ر�־ */
    VOS_UINT8                           ucReserve1;
    VOS_UINT8                           ucReserve2;
    VOS_UINT8                           ucReserve3;
    VOS_UINT8                           aucAbortAtCmdStr[AT_MAX_ABORT_CMD_STR_LEN]; /* ���AT������ */
    VOS_UINT8                           aucAbortAtRspStr[AT_MAX_ABORT_RSP_STR_LEN]; /* �������ķ��ؽ�� */
}AT_NVIM_ABORT_CMD_PARA_STRU;


typedef struct
{
    VOS_UINT8                   ucParaNum;                                      /* ��������������� */
    VOS_UINT8                   ucRsv[3];                                       /* ����λ */
    VOS_INT16                   asPower[AT_NVIM_BODYSARGSM_MAX_PARA_GROUP_NUM];      /* GƵ�ι�������ֵ */
    VOS_UINT32                  aulBand[AT_NVIM_BODYSARGSM_MAX_PARA_GROUP_NUM];      /* GƵ��λ�� */
}AT_BODYSARGSM_SET_PARA_STRU;


typedef struct
{
    VOS_UINT32                          enRightOpenFlg;
    VOS_INT8                            acPassword[AT_NVIM_RIGHT_PWD_LEN];
}TAF_AT_NVIM_RIGHT_OPEN_FLAG_STRU;


typedef struct
{
    VOS_UINT32 ulDSTotalSendFluxLow;
    VOS_UINT32 ulDSTotalSendFluxHigh;
    VOS_UINT32 ulDSTotalReceiveFluxLow;
    VOS_UINT32 ulDSTotalReceiveFluxHigh;
    TAF_AT_NVIM_RIGHT_OPEN_FLAG_STRU stRightPwd;
}NAS_NV_RABM_TOTAL_RX_BYTES_STRU;


typedef struct
{
    VOS_UINT8                          aucImei[TAF_NVIM_ITEM_IMEI_SIZE];
}IMEI_STRU;


typedef struct
{
    VOS_INT8                           acATE5DissdPwd[AT_DISSD_PWD_LEN];
}TAF_AT_NVIM_DISSD_PWD_STRU;


typedef struct
{
    VOS_INT8                           acATOpwordPwd[AT_OPWORD_PWD_LEN];
}TAF_AT_NVIM_DISLOG_PWD_NEW_STRU;


typedef struct
{
    VOS_UINT16                          usEqver;
    VOS_UINT8                           ucReserve1;
    VOS_UINT8                           ucReserve2;
}TAF_AT_EQ_VER_STRU;


typedef struct
{
    VOS_UINT16                          usCsver;
    VOS_UINT8                           ucReserve1;
    VOS_UINT8                           ucReserve2;
}TAF_NVIM_CS_VER_STRU;


typedef struct
{
    VOS_UINT8   ucEnableFlag;
    VOS_UINT8   ucReserve;
}NAS_RABM_NVIM_FASTDORM_ENABLE_FLG_STRU;

/*****************************************************************************
 �ṹ����   : AT_IPV6_CAPABILITY_STRU
 Э����   :
 ASN.1 ���� :
 �ṹ˵��   : IPV6����NV����ƽṹ��
*****************************************************************************/
typedef struct
{
    VOS_UINT8                           ucStatus;           /* NV��Ч��־, 1: ��Ч��0����Ч */
    VOS_UINT8                           ucIpv6Capablity;    /* IPV6���� */
    VOS_UINT8                           aucReversed[2];     /* ���ֽڶ��� */

} AT_NV_IPV6_CAPABILITY_STRU;


typedef struct
{

    VOS_UINT8   ucGsmConnectRate;
    VOS_UINT8   ucGprsConnectRate;
    VOS_UINT8   ucEdgeConnectRate;
    VOS_UINT8   ucWcdmaConnectRate;
    VOS_UINT8   ucDpaConnectRate;
    VOS_UINT8   ucReserve1;
    VOS_UINT8   ucReserve2;
    VOS_UINT8   ucReserve3;
}AT_NVIM_DIAL_CONNECT_DISPLAY_RATE_STRU;

/*****************************************************************************
 �ṹ��    : AT_TRAFFIC_CLASS_CUSTOMIZE_STRU
 �ṹ˵��  : ���ڶ���PDP����������QoS�� Traffic Class��ֵ�Ľṹ��
*****************************************************************************/

typedef struct
{
    VOS_UINT8                          ucStatus;                         /* 1: NV��Ч��־λ��0����Ч */
    VOS_UINT8                          ucTrafficClass;                   /* Traffic Class��ֵ */
    VOS_UINT8                          ucReserve1;
    VOS_UINT8                          ucReserve2;
}AT_TRAFFIC_CLASS_CUSTOMIZE_STRU;


typedef struct
{
    VOS_UINT8                           ucStatus;                               /* NV�Ƿ񼤻��־,  */
    VOS_UINT8                           ucSsCmdCustomize;
    VOS_UINT8                           aucReserved1[2];
} AT_SS_CUSTOMIZE_PARA_STRU;


typedef struct
{
    VOS_UINT32 ulCimiPortCfg;
}TAF_AT_NVIM_CIMI_PORT_CFG_STRU;


typedef struct
{
    VOS_UINT32 ulMuxReportCfg;
}TAF_AT_NVIM_MUX_REPORT_CFG_STRU;


typedef struct
{
    VOS_UINT32 ulTotalMsg;
}NAS_MN_NVIM_TOTAL_MSG_STRU;


typedef struct
{
    VOS_UINT8   ucApXmlRptFlg;
    VOS_UINT8   ucReserve[3];
}TAF_AT_NVIM_AP_XML_RPT_FLG_STRU;

/* Added by l60609 for XML, 2011-08-11 End */


typedef struct
{
    VOS_UINT16                          usModemId;                              /* �ö˿������ĸ�modem */
    VOS_UINT8                           ucReportFlg;                            /* �ö˿��Ƿ����������ϱ���VOS_TRUEΪ����VOS_FALSEΪ������Ĭ������ */
    VOS_UINT8                           aucRsv[1];
}AT_NVIM_CLIENT_CONFIGURATION_STRU;


typedef struct
{
    VOS_UINT8                           aucForband[MMA_FORB_BAND_NV_MAX_SIZE];
    VOS_UINT8                           ucReserve1;
    VOS_UINT8                           ucReserve2;
}NAS_MMA_NVIM_FORBAND_STRU;



typedef struct
{
    VOS_UINT8 aucRcvData[MMA_OPERTOR_NAME_MAX_SIZE];
}NAS_MMA_NVIM_OPERATOR_NAME_STRU;


typedef struct
{
    MMA_QUICK_START_STA_UINT32          ulQuickStartSta;
}NAS_NVIM_FOLLOWON_OPENSPEED_FLAG_STRU;


typedef struct
{
    VOS_UINT8                           aucClassmark1[TAF_PH_WCDMA_CLASSMAEK1_LEN];
}NAS_MMA_NVIM_CLASSMARK1_STRU;


typedef struct
{
    VOS_UINT8 aucClassmark2[TAF_PH_WCDMA_CLASSMAEK2_LEN];
}NAS_MMA_NVIM_CLASSMARK2_STRU;


typedef struct
{
    VOS_UINT8 aucClassmark3[TAF_PH_WCDMA_CLASSMAEK3_LEN];
}NAS_MMA_NVIM_CLASSMARK3_STRU;


typedef struct
{
    VOS_UINT8   aucSmsServicePara[MN_MSG_SRV_PARAM_LEN];
}TAF_NVIM_SMS_SERVICE_PARA_STRU;


typedef struct
{
    VOS_UINT8   aucSmsEfsmssPara[MN_MSG_EFSMSS_PARA_LEN];
}TAF_MMA_NVIM_SMS_EFSMSS_PARA_STRU;


typedef struct
{
    VOS_UINT8   aucSmsEfsmspPara[MN_MSG_EFSMSP_PARA_LEN];
}TAF_MMA_NVIM_SMS_EFSMSP_PARA_STRU;


typedef struct
{
    VOS_UINT8   ucActFlg;                                                       /* �Ƿ�ʹ��NV������Ϣ��ʶ: 0��ʾ��ʹ�ã�1��ʾʹ�� */
    VOS_UINT8   ucReserved1;
    VOS_UINT8   ucReserved2;
    VOS_UINT8   ucReserved3;
    VOS_UINT8   aucSmscAddr[MN_MSG_MIN_SRV_PARM_LEN];                           /* NV���õĶ������ĵ�ַ */
}MN_MSG_NVIM_EFSMSP_PRESETED_STRU;


typedef struct
{
    VOS_UINT8  aucPwd[TAF_NVIM_ME_PERSONALISATION_PWD_LEN_MAX];
}TAF_MMA_SIM_PERSONAL_PWD_STRU;


typedef struct
{
    VOS_UINT8  aucImsiStr[TAF_NVIM_MAX_IMSI_STR_LEN];
}NAS_MMA_SIM_PERSONAL_IMST_STRU;


typedef struct
{
    VOS_UINT8                           aucDisplaySpnFlag[2];
}NAS_MMA_NVIM_DISPLAY_SPN_FLAG_STRU;


typedef struct
{
    VOS_UINT8   ucVaild;
    VOS_UINT8   ucReserved1;
    VOS_UINT8   ucReserved2;
    VOS_UINT8   ucReserved3;
}TAF_AT_NVIM_RXDIV_CONFIG_STRU;


/*lint -e958 -e959 �޸���:l60609;ԭ��:64bit*/
typedef struct
{
    VOS_INT8    cStatus;
    VOS_UINT8   ucEncodeType;
    VOS_UINT8   ucReserved1;
    VOS_UINT8   ucReserved2;
    VOS_UINT32  ulLength;
    VOS_UINT8   ucData[TAF_NVIM_MSG_ACTIVE_MESSAGE_MAX_URL_LEN];
}TAF_AT_NVIM_SMS_ACTIVE_MESSAGE_STRU;
/*lint +e958 +e959 �޸���:l60609;ԭ��:64bit*/

/*8301-8312���ýṹ��*/

typedef struct
{
    VOS_UINT8                           ucAuthType;
    VOS_UINT8                           aucPlmn[TAF_NVIM_MAX_NDIS_PLMN_LEN];
    VOS_UINT8                           aucPassword[TAF_NVIM_MAX_NDIS_PASSWORD_LEN_OF_AUTHDATA];
    VOS_UINT8                           aucUsername[TAF_NVIM_MAX_NDIS_USERNAME_LEN_OF_AUTHDATA];
}TAF_NVIM_NDIS_AUTHDATA_STRU;


typedef struct
{
    VOS_UINT8                           ucUsed;                                             /*0��δʹ�ã�1��ʹ��*/
    VOS_UINT8                           aucRsv[3];
    TAF_NVIM_NDIS_AUTHDATA_STRU         stAuthData;                                         /*��Ȩ����*/
}TAF_NVIM_NDIS_AUTHDATA_TABLE_STRU;

/*8451-8462���ýṹ�� en_NV_Item_Taf_PdpPara_0*/

typedef struct
{
    VOS_UINT8          aucPdpPara[TAF_NVIM_PDP_PARA_LEN];
}TAF_NVIM_PDP_PARA_STRU;

/* en_NV_Item_ProductName 8205 */

typedef struct
{
    VOS_UINT8          aucProductName[TAF_PH_PRODUCT_NAME_STR_LEN];
}TAF_PH_PRODUCT_NAME_STRU;

/*en_NV_Item_Imei_Svn 8337*/

typedef struct
{
    VOS_UINT8                           ucActiveFlag;
    VOS_UINT8                           aucSvn[TAF_SVN_DATA_LENGTH];
    VOS_UINT8                           aucReserve[1];
}TAF_SVN_DATA_STRU;

/*en_NV_Item_SMS_MO_RETRY_PERIOD 8293*/

typedef struct
{
    VOS_UINT8                           ucActFlg;                                           /* NVIM�и����Ƿ񼤻� */
    VOS_UINT8                           ucReserved[3];
    VOS_UINT32                          ulRetryPeriod;                                      /*�����ط�������*/
}MN_MSG_NVIM_RETRY_PERIOD_STRU;

/*en_NV_Item_SMS_MO_RETRY_INTERVAL 8294*/

typedef struct
{
    VOS_UINT8                           ucActFlg;                               /* NVIM�и����Ƿ񼤻� */
    VOS_UINT8                           ucReserved[3];
    VOS_UINT32                          ulRetryInterval;                        /*�����ط���ʱ����*/
}MN_MSG_NVIM_RETRY_INTERVAL_STRU;


typedef struct
{
    VOS_UINT8                           ucUsimActFlg;                           /* �Ƿ�֧�ֲ�ѯUSIM SMS״̬���� */
    VOS_UINT8                           ucCsimActFlg;                           /* �Ƿ�֧�ֲ�ѯCSIM SMS״̬���� */
    VOS_UINT16                          usTimerLength;                          /* �ȴ�USIM����CSIM�ظ������б�Ķ�ʱ��ʱ��,��λ���� */
}MN_MSG_NVIM_CHECK_USIM_CSIM_SMS_STATUS_CFG_STRU;
typedef struct
{
    VOS_UINT8                           ucSmsRetryCmSrvRejCauseNum;
    VOS_UINT8                           aucReserved[3];
    VOS_UINT8                           aucSmsRetryCmSrvRejCause[TAF_NVIM_SMS_RETRY_CAUSE_MAX_NUM];
}TAF_MSG_NVIM_RETRY_CM_SRV_REJ_CFG_STRU;


typedef struct
{
    VOS_UINT8                           ucSmsNoRetryRpCauseNum;
    VOS_UINT8                           ucSmsNeedToJudgeRpCauseFlg;
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucReserved2;
    VOS_UINT8                           aucSmsNoRetryRpCause[TAF_NVIM_SMS_NO_RETRY_RP_CAUSE_MAX_NUM];
}TAF_MSG_NVIM_NO_RETRY_RP_CAUSE_CFG_STRU;

/*en_NV_Item_SMS_SEND_DOMAIN 8295*/
/* NVIM���ж��ŷ�����Ľṹ */

typedef struct
{
    VOS_UINT8                           ucActFlg;
    VOS_UINT8                           ucSendDomain;
    VOS_UINT8                           ucReserve1;
    VOS_UINT8                           ucReserve2;
}AT_NVIM_SEND_DOMAIN_STRU;

/*en_NV_Item_WINS_Config 8297*/

typedef struct
{
    VOS_UINT8                           ucStatus;        /* 1: NV��Ч��־λ��0����Ч */
    VOS_UINT8                           ucWins;          /* WINSʹ�ܱ��: 0��Disable,  1��Enable */
    VOS_UINT8                           ucReserve1;
    VOS_UINT8                           ucReserve2;
}WINS_CONFIG_STRU;

/*en_NV_Item_CustomizeSimLockPlmnInfo 8267*/

typedef struct
{
    VOS_UINT8                           ucMncNum;
    VOS_UINT8                           aucRangeBegin[TAF_PH_SIMLOCK_PLMN_STR_LEN];
    VOS_UINT8                           aucRangeEnd[TAF_PH_SIMLOCK_PLMN_STR_LEN];
}TAF_CUSTOM_SIM_LOCK_PLMN_RANGE_STRU;


typedef struct
{
    VOS_UINT32                          ulStatus;/*�Ƿ񼤻0�����1���� */
    TAF_CUSTOM_SIM_LOCK_PLMN_RANGE_STRU astSimLockPlmnRange[TAF_MAX_SIM_LOCK_RANGE_NUM];
}TAF_CUSTOM_SIM_LOCK_PLMN_INFO_STRU;

/* en_NV_Item_CardlockStatus 8268 */

typedef struct
{
    VOS_UINT32                          ulStatus;            /*�Ƿ񼤻0�����1���� */
    VOS_UINT32                          ulCardlockStatus;    /**/
    VOS_UINT32                          ulRemainUnlockTimes; /*����ʣ�����*/
}TAF_NVIM_CUSTOM_CARDLOCK_STATUS_STRU;

/*en_NV_Item_CustomizeSimLockMaxTimes 8269*/

typedef struct
{
    VOS_UINT32                          ulStatus;            /*�Ƿ񼤻0�����1���� */
    VOS_UINT32                          ulLockMaxTimes;
}TAF_CUSTOM_SIM_LOCK_MAX_TIMES_STRU;

/*en_NV_Item_CCA_TelePara 8230*/

/*Ϊ����V1R1NV��ƥ��ǰ15���ֽڱ���*/
typedef struct
{
    VOS_UINT8               aucRsv[15];              /*NV��Ľṹ�У�4�ֽڶ��뷽ʽ���пն�Ҫ�ֶ�����*/
    VOS_UINT8               ucS0TimerLen;
} TAF_CCA_TELE_PARA_STRU;

/*en_NV_Item_PS_TelePara 8231*/

typedef struct
{
    VOS_UINT8         AnsMode;
    VOS_UINT8         AnsType;
    VOS_UINT16        ClientId;
}TAF_APS_NVIM_PS_ANS_MODE_STRU;

/*en_NV_Item_User_Set_Freqbands 8265*/
/*��ȡbandֵ����MS��RACIEZ�л��*/
/*
  80��CM_BAND_PREF_GSM_DCS_1800��              GSM DCS systems
  100��CM_BAND_PREF_GSM_EGSM_900��             Extended GSM 900
  200��CM_BAND_PREF_GSM_PGSM_900��             Primary GSM 900
  100000��CM_BAND_PREF_GSM_RGSM_900��          GSM Railway GSM 900
  200000��CM_BAND_PREF_GSM_PCS_1900��          GSM PCS
  400000��CM_BAND_PREF_WCDMA_I_IMT_2000��      WCDMA IMT 2000
  3FFFFFFF��CM_BAND_PREF_ANY��                 �κ�Ƶ��
  40000000��CM_BAND_PREF_NO_CHANGE��           Ƶ�����仯
*/

typedef struct
{
    VOS_UINT32 ulBandLow; /*��32λ*/
    VOS_UINT32 ulBandHigh;/*��32λ*/
} TAF_MMA_NVIM_USER_SET_PREF_BAND64;


typedef struct
{
    VOS_UINT16                          usReportRegActFlg;
    VOS_UINT8                           ucReserve1;
    VOS_UINT8                           ucReserve2;
}TAF_AT_NVIM_REPORT_REG_ACT_FLG_STRU;


typedef struct
{
    VOS_UINT16                          usMePersonalActFlag;
    VOS_UINT8                           ucReserve1;
    VOS_UINT8                           ucReserve2;
}NAS_MMA_ME_PERSONAL_ACT_FLAG_STRU;


typedef struct
{
    VOS_UINT8                           ucCbStatus;         /* CBSҵ���Ƿ����ñ�־*/

    VOS_UINT8                           ucDupDetectCfg;     /* �ظ��������� */

    VOS_UINT8                           ucRptAppFullPageFlg; /* VOS_TRUE: �ϱ���ҳ��88���ֽ�; VOS_FALSE: �ϱ�ʵ����Ч�ֽ� */

    VOS_UINT8                           ucRsv;              /* NV����صĽṹ�壬��4�ֽڷ�ʽ�£����ֶ�����ն� */
}TAF_CBA_NVIM_CBS_SERVICE_PARM_STRU;


typedef struct
{
    VOS_UINT32                          ulCommDebugFlag;
}TAF_AT_NVIM_COMMDEGBUG_CFG_STRU;


typedef struct
{
    AT_UART_LINK_TYPE_ENUM_UINT16       enUartLinkType;
    VOS_UINT8                           ucReserve1;
    VOS_UINT8                           ucReserve2;
}TAF_AT_NVIM_DEFAULT_LINK_OF_UART_STRU;


typedef struct
{
    VOS_UINT8 aucMfrId[TAF_MAX_MFR_ID_STR_LEN];
}TAF_PH_FMR_ID_STRU;


typedef struct
{
    VOS_UINT8 aucOperatorName[NAS_MMA_NVIM_OPERATOR_NAME_LEN];
}NAS_MMA_OPERATOR_NAME_STRU;


typedef struct
{
    VOS_UINT8                           aucFactInfo1[AT_FACINFO_INFO1_STR_LENGTH];
    VOS_UINT8                           aucFactInfo2[AT_FACINFO_INFO2_STR_LENGTH];
    VOS_UINT8                           ucReserve1;
    VOS_UINT8                           ucReserve2;
}TAF_AT_NVIM_FACTORY_INFO_STRU;


typedef struct
{
    VOS_UINT8                           aucMDate[AT_MDATE_STRING_LENGTH];
}TAF_AT_NVIM_MANUFACTURE_DATE_STRU;


typedef struct
{
    VOS_UINT16                          usPcVoiceSupportFlag; /*Range:[0,1]*/
    VOS_UINT8                           ucReserve1;
    VOS_UINT8                           ucReserve2;
}APP_VC_NVIM_PC_VOICE_SUPPORT_FLAG_STRU;




typedef struct
{
    MN_MSG_ME_STORAGE_STATUS_ENUM_UINT8 enMeStorageStatus;                      /* ME�洢���Ź����Ƿ����ñ�־*/
    VOS_UINT8                           aucReserve[1];                          /* NV����صĽṹ�壬��4�ֽڷ�ʽ�£����ֶ�����ն� */
    VOS_UINT16                          usMeStorageNum;                         /* ME�洢�������� */
}MN_MSG_ME_STORAGE_PARM_STRU;

typedef struct
{
    VOS_UINT32      ulDSLastLinkTime;                       /*DS���һ������ʱ��*/
    VOS_UINT32      ulDSTotalSendFluxLow;                   /*DS�ۼƷ����������ĸ��ֽ�*/
    VOS_UINT32      ulDSTotalSendFluxHigh;                  /*DS�ۼƷ����������ĸ��ֽ�*/
    VOS_UINT32      ulDSTotalLinkTime;                      /*DS�ۼ�����ʱ��*/
    VOS_UINT32      ulDSTotalReceiveFluxLow;                /*DS�ۼƽ����������ĸ��ֽ�*/
    VOS_UINT32      ulDSTotalReceiveFluxHigh;               /*DS�ۼƽ����������ĸ��ֽ�*/

} TAF_APS_DSFLOW_NV_STRU;


typedef struct
{
    VOS_UINT8                           ucActFlg;/* MN_MSG_NVIM_ITEM_ACTIVE */
    VOS_UINT8                           aucReserved[3];
}MN_MSG_DISCARD_DELIVER_MSG_STRU;

typedef struct
{
    VOS_UINT8                           ucActFlg;/* MN_MSG_NVIM_ITEM_ACTIVE */
    VOS_UINT8                           aucReserved[3];
}MN_MSG_REPLACE_DELIVER_MSG_STRU;

/* Added by z40661 for AMR-WB , 2012-02-09 , end */

typedef struct
{
    VOS_UINT8       ucDsFlowStatsRptCtrl;                   /* ��ʶ�Ƿ���������ϱ� */
    VOS_UINT8       ucDsFlowStatsSave2NvCtrl;               /* ��ʶ�Ƿ���Ҫ������ʷ������Ϣ��NV�� */
    VOS_UINT8       ucDsFlowSavePeriod;                     /* ����дNV������ */
    VOS_UINT8       aucReserve[1];                          /* ����λ*/
} TAF_APS_DSFLOW_STATS_CTRL_NV_STRU;


typedef struct
{
    VOS_UINT8                           ucRatOrderNum;                          /* syscfgex�����õ�acqoder�е�ָʾ���� */
    VOS_UINT8                           aenRatOrder[TAF_PH_NVIM_MAX_GUL_RAT_NUM];    /* at^syscfgex�����õ�acqoder��˳�� */
}TAF_PH_NVIM_RAT_ORDER_STRU;


typedef struct
{
    VOS_UINT8                           ucActFlg;
    VOS_UINT8                           aucReserved[3];
}MN_MSG_NVIM_MO_SMS_CTRL_STRU;



typedef struct
{
    VOS_UINT8                           ucNvimActiveFlg;                        /* en_NV_Item_PDP_Actving_Limit NV���Ƿ񼤻VOS_TRUE:���VOS_FALSE:δ���� */
    VOS_UINT8                           ucReserved1;                            /* ���� */
    VOS_UINT8                           ucReserved2;                            /* ���� */
    VOS_UINT8                           ucReserved3;                            /* ���� */
}TAF_APS_NVIM_PDP_ACT_LIMIT_STRU;

typedef struct
{
    VOS_UINT8                           ucCategory;                             /* �������������� */
    VOS_UINT8                           ucValidSimPresent;                      /* �������������п�ʱ��Ч�����޿�ʱ��Ч��1�п�ʱ��Ч��0�޿�ʱ��Ч */
    VOS_UINT8                           ucAbnormalServiceFlg;                   /* �����������п�ʱ�Ƿ���ڷ���������ʱ��Ч��0:�κη���ʱ����Ч��1:������������ʱ��Ч */
    VOS_UINT8                           ucEccNumLen;
    VOS_UINT8                           aucEccNum[MN_CALL_NVIM_MAX_BCD_NUM_LEN];
    VOS_UINT32                          ulMcc;                                  /* MCC,3 bytes */
} MN_CALL_NVIM_CUSTOM_ECC_NUM_STRU;



typedef struct
{
    VOS_UINT8                           ucEccNumCount;
    VOS_UINT8                           aucReserve[3];
    MN_CALL_NVIM_CUSTOM_ECC_NUM_STRU    astCustomEccNumList[MN_CALL_NVIM_MAX_CUSTOM_ECC_NUM];
} MN_CALL_NVIM_CUSTOM_ECC_NUM_LIST_STRU;


typedef struct
{
    VOS_UINT8                           ucBufferCcProtectTimerLen;              /* 0: ��ѡ��ʱ�����л��棻����0: ����CC����ı�����ʱ��ʱ������λ �� */
    VOS_UINT8                           ucBufferSmsProtectTimerLen;             /* 0: ��ѡ��ʱ�����л��棻����0: ����SMS����ı�����ʱ��ʱ������λ �� */
    VOS_UINT8                           ucBufferSsProtectTimerLen;              /* 0: ��ѡ��ʱ�����л��棻����0: ����SS����ı�����ʱ��ʱ������λ �� */
    VOS_UINT8                           ucReserve1;
}TAF_NVIM_BUFFER_SERVICE_REQ_PROTECT_TIMER_CFG_STRU;




typedef struct
{
    VOS_UINT8                           ucSimCallCtrlSupportFlg;                /* SIM��Call Controlҵ��*/
    VOS_UINT8                           ucReserved1;                            /* ����*/
    VOS_UINT8                           ucReserved2;                            /* ����*/
    VOS_UINT8                           ucReserved3;                            /* ����*/
}MN_CALL_NVIM_SIM_CALL_CONTROL_FLG_STRU;

typedef struct
{
    VOS_UINT8                           ucCallDeflectionSupportFlg;             /* ֧�ֺ���ƫתҵ��*/
    VOS_UINT8                           ucReserved1;                            /* ����*/
    VOS_UINT8                           ucReserved2;                            /* ����*/
    VOS_UINT8                           ucReserved3;                            /* ����*/
}MN_CALL_NVIM_CALL_DEFLECTION_SUPPORT_FLG_STRU;

typedef struct
{
    VOS_UINT8                           ucAlsSupportFlg;                        /* ֧����·�л�ҵ��*/
    VOS_UINT8                           ucReserved1;                            /* ����*/
    VOS_UINT8                           ucReserved2;                            /* ����*/
    VOS_UINT8                           ucReserved3;                            /* ����*/
}MN_CALL_NVIM_ALS_SUPPORT_FLG_STRU;

typedef struct
{
    VOS_UINT8                           ucGetCsmpParaFromUsimSupportFlg;        /*��(U)SIM���ж�ȡCSMP����*/

    VOS_UINT8                           ucGetScAddrIgnoreScIndication; /* 0x6f42�ļ���sc indicationָʾ�����ڶ������ĺ���ʱ������������ĺ���Ϸ��Ƿ��ȡ��
                                                                       VOS_FALSE:sc indicationָʾ�������ĺ��벻�����򲻶�ȡ�������ĺ��룻
                                                                       VOS_TRUE: sc indicationָʾ�������ĺ��벻���ڶ�ȡ�������ĺ��� */
}MN_MSG_GET_CSMP_PARA_FROM_USIM_SUPPORT_FLG_STRU;

typedef struct
{
    VOS_UINT8                           ucSmsPpDownlodSupportFlg;               /*�����Ƿ�֧�ֶ��ŵ�PP-DOWNLOAD����*/
    VOS_UINT8                           ucReserved1;                            /* ����*/
    VOS_UINT8                           ucReserved2;                            /* ����*/
    VOS_UINT8                           ucReserved3;                            /* ����*/
}MN_MSG_SMS_PP_DOWNLOAD_SUPPORT_FLG_STRU;

typedef struct
{
    VOS_UINT8                           ucSmsNvSmsRexitSupportFlg;              /*����������ΪPS ONLYʱ���Ƿ�֧��CS����źͺ���ҵ��(�������г���) */
    VOS_UINT8                           ucReserved1;                            /* ����*/
    VOS_UINT8                           ucReserved2;                            /* ����*/
    VOS_UINT8                           ucReserved3;                            /* ����*/
}MN_MSG_SMS_NVIM_SMSREXIST_SUPPORT_FLG_STRU;


typedef struct
{
    VOS_UINT8                           ucSmsStatusInEfsmsSupportFlg;           /* ����NVIM���Ƿ��ܱ������״̬����*/
    VOS_UINT8                           ucReserved1;                            /* ����*/
    VOS_UINT8                           ucReserved2;                            /* ����*/
    VOS_UINT8                           ucReserved3;                            /* ����*/
}MN_MSG_SMS_STATUS_IN_EFSMS_SUPPORT_FLG_STRU;


/* Added by z40661 for AMR-WB , 2012-02-09 , begin */



typedef struct
{
    VOS_UINT8                           ucCodecTypeNum;
    VOS_UINT8                           aucCodecType[MN_CALL_NVIM_BC_MAX_SPH_VER_NUM];
    VOS_UINT8                           ucReserve;
}MN_CALL_NIMV_ITEM_CODEC_TYPE_STRU;

/* Added by z40661 for AMR-WB , 2012-02-09 , end */

	/* Added by f62575 for C50_IPC Project, 2012/02/23, begin */

typedef struct
{
    VOS_UINT32       ulMeStatus;
}TAF_FDN_NVIM_CONFIG_STRU;
/* Added by f62575 for C50_IPC Project, 2012/02/23, end   */


typedef struct
{
    VOS_UINT8                           ucVoiceCallNotSupportedCause;
    VOS_UINT8                           ucVideoCallNotSupportedCause;
    VOS_UINT8                           aucReserved1[2];
}TAF_CALL_NVIM_CALL_NOT_SUPPORTED_CAUSE_STRU;



typedef struct
{
    VOS_UINT8                           ucNvimActiveFlg;                        /* en_NV_Item_Network_Selection_Menu_Ctrl_Para NV���Ƿ񼤻VOS_TRUE:���VOS_FALSE:δ���� */
    VOS_UINT8                           ucReserved;                             /* ���� */
}TAF_MMA_NVIM_REPORT_PLMN_SUPPORT_FLG_STRU;

	
typedef struct
{
    VOS_UINT8                           ucNotDisplayLocalNetworkNameFlg;
    VOS_UINT8                           ucReserved;
}TAF_MMA_NOT_DISPLAY_LOCAL_NETWORKNAME_NVIM_STRU;


typedef struct
{
    VOS_UINT8                           ucActFlag;
    VOS_UINT8                           enMtCustomize;
    VOS_UINT8                           aucMtCustomize[2];                      /* MT������������. [0]: download mt�������Ƿ���ʱ����stk���� */
}MN_MSG_MT_CUSTOMIZE_INFO_STRU;


typedef struct
{
    VOS_UINT8                           ucNotRptTpFcsFlag;
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucReserved2;
    VOS_UINT8                           ucReserved3;
}TAF_NV_SMS_MO_CUSTOMIZE_INFO_STRU;



typedef struct {
    VOS_UINT8                           aucSimPlmn[TAF_SIM_FORMAT_PLMN_LEN];
    VOS_UINT8                           aucReserve[1];
}TAF_SIM_FORMAT_PLMN_ID;


typedef struct
{
    VOS_UINT32                          ulPlmnNum;
    TAF_SIM_FORMAT_PLMN_ID              astSimPlmnId[TAF_NVIM_OTA_SECURITY_MSG_IMSI_PLMN_MAX_NUM];
} TAF_MN_OTA_SECURITY_SMS_IMSI_PLMN_LIST;


typedef struct
{
    VOS_UINT8                                               ucOtaSmsActiveFlg;
    VOS_UINT8                                               ucReserve1;
    VOS_UINT8                                               ucReserve2;
    VOS_UINT8                                               ucReserve3;
    TAF_MN_OTA_SECURITY_SMS_IMSI_PLMN_LIST                  stImsiPlmnList;  /* IMSI��MCC-MNC���б�  */
}TAF_MN_NVIM_OTA_SECURITY_SMS_CFG_STRU;


typedef struct
{
    TAF_NV_GPS_CHIP_TYPE_ENUM_UINT8     enGpsChipType;
    VOS_UINT8                           ucReserve1;
    VOS_UINT8                           ucReserve2;
    VOS_UINT8                           ucReserve3;
}TAF_NVIM_GPS_CUST_CFG_STRU;


typedef struct
{
    VOS_UINT8  ucCnt;
    VOS_UINT8  aucUmtsCodec[MN_CALL_MAX_UMTS_CODEC_TYPE_NUM];
} MN_CALL_UMTS_CODEC_TYPE_STRU;

typedef struct
{
    VOS_UINT8                           ucStatus;/*�Ƿ񼤻0�����1���� */
    VOS_UINT8                           ucCardLockPerm;
    VOS_UINT8                           ucReserve1;
    VOS_UINT8                           ucReserve2;
}MMA_CUSTOM_CARDLOCK_PERM_STRU;


typedef struct
{
    VOS_UINT16                          usMsgIdFrom;                            /*Cell broadcast message id value range from  */
    VOS_UINT16                          usMsgIdTo;                              /*Cell broadcast message id value range to    */
}TAF_CBA_NVIM_ETWS_MSGID_RANGE_STRU;

typedef struct
{
    VOS_UINT8                                               ucEnhDupDetcFlg;    /* DoCoMo��ǿ���ظ���ⶨ�����ԣ���ETWS����֪ͨ����ǿ���ظ���������Ƿ񼤻�, VOS_TRUE:����,VOS_FALSE:δ����.
                                                                                        1. ����֪ͨ,������ͨCBS��Ϣ, �ظ����ʱ��Ҫ�ж�PLMN
                                                                                        2. ����֪ͨ,������ͨCBS��Ϣ, �ظ�����ʱ������ݵ�ǰPLMN��MCC��������
                                                                                        3. ETWS��Ϣ����ͨCBS��Ϣ����ͬ�Ĺ������ظ����˺�ʱЧ��� */
    VOS_UINT8                                               ucRsv;

    VOS_UINT16                                              usNormalTimeLen;   /* ��λ:����, ��������δ����ʱʹ�ô���Чʱ�� */

    VOS_UINT16                                              usSpecMccTimeLen;  /* ��λ:����, ��ǿ���ظ���ⶨ�����Լ���ʱ��/��֪ͨ/CBS��MCC��ָ��Mcc����ʹ�ô���Чʱ�� */
    VOS_UINT16                                              usOtherMccTimeLen; /* ��λ:����, ��ǿ���ظ���ⶨ�����Լ���ʱ��/��֪ͨ/CBS��MCC����ָ��Mcc����ʹ�ô���Чʱ�� */
    VOS_UINT32                                              aulSpecMcc[TAF_CBA_NVIM_MAX_ETWS_DUP_DETECT_SPEC_MCC_NUM];  /* ��ǿ���ظ���ⶨ��������ص�MCC */
}TAF_CBA_NVIM_ETWS_DUP_DETC_CFG_STRU;


typedef struct
{
    VOS_UINT8                           ucReportEccNumFlg;                      /* 1:֧����APP�ϱ�����������, 0:��֧����APP�ϱ�����������*/
    VOS_UINT8                           ucReserved1;                            /* ����*/
    VOS_UINT8                           ucReserved2;                            /* ����*/
    VOS_UINT8                           ucReserved3;                            /* ����*/
}MN_CALL_NVIM_REPORT_ECC_NUM_SUPPORT_FLG_STRU;



typedef struct
{
    VOS_UINT8                           ucStatus;/*�Ƿ񼤻0�����1���� */
    MN_CALL_STATE_ENUM_UINT8            enCardLockOperator;
    VOS_UINT8                           ucReserved1;                            /* ����*/
    VOS_UINT8                           ucReserved2;                            /* ����*/
}MMA_CUSTOM_CARDLOCK_OPERATOR_STRU;


typedef struct
{
    VOS_UINT8                           ucStatus;                               /* 1: NV��Ч��־λ��0����Ч */
    VOS_UINT8                           ucVpCfgState;                           /* ��ǰNV����ֵ */
}MN_CALL_NV_ITEM_VIDEO_CALL_STRU;
/* Added by f62575 for AT Project, 2011-10-27, begin */


typedef struct
{
    VOS_UINT8                           ucActFlg;                               /* NVIM�и����Ƿ񼤻� */
    VOS_UINT8                           enClass0Tailor;
}MN_MSG_NVIM_CLASS0_TAILOR_STRU;
/* Added by f62575 for AT Project, 2011-10-27, end */
/* �ն˿���������: 2G���������ֹSpare_bit3�Ȳ��� */
typedef struct
{
    VOS_UINT8                           ucStatus;       /* 1: NV��Ч��־λ��0����Ч */
    VOS_UINT8                           ucDisable2GBit3;
    VOS_UINT8                           ucReserved1;                            /* ����*/
    VOS_UINT8                           ucReserved2;                            /* ����*/
}APS_2G_DISABLE_BIT3_CUSTOMIZE_STRU;


typedef struct
{
    VOS_UINT16                          usMultiSimCallConf;                             /*oΪ��������ʱ�ϱ���1��������ʱ���ϱ�*/
    VOS_UINT8                           ucReserved1;                            /* ����*/
    VOS_UINT8                           ucReserved2;                            /* ����*/
}MN_CALL_NV_ITEM_MULTISIM_CALLCON_STRU;



typedef struct
{
    VOS_UINT8                           ucStatus;        /* 1: NV��Ч��־λ��0����Ч */
    VOS_UINT8                           ucUssdTransMode;
    VOS_UINT8                           ucReserve1;
    VOS_UINT8                           ucReserve2;
}TAF_USSD_NVIM_TRANS_MODE_STRU;


typedef struct
{
    VOS_UINT8                           ucCallRedialCmSrvRejCauseNum;                                   /* �ز�ԭ��ֵ���� */
    VOS_UINT8                           aucReserved[3];
    VOS_UINT8                           aucCallRedialCmSrvRejCause[TAF_NVIM_CALL_REDIAL_CAUSE_MAX_NUM]; /* �ز�ԭ��ֵ */
}TAF_CALL_REDIAL_CM_SRV_REJ_CFG_STRU;


typedef struct
{
    VOS_UINT8                           ucCallRedialForDiscSupportFlg;                            /* ����disconnect ԭ��ֵ�����Ի��ƵĿ���VOS_TRUE:�򿪣�VOS_FALSE:�ر�*/
    VOS_UINT8                           ucCallRedialDiscCauseNum;                                   /* �ز�ԭ��ֵ���� */
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucReserved2;
    VOS_UINT8                           aucCallRedialDiscCause[TAF_NVIM_CALL_RETRY_DISC_CAUSE_MAX_NUM]; /* �ز�ԭ��ֵ */
}TAF_CALL_REDIAL_DISC_CFG_STRU;



typedef struct
{
    VOS_UINT8                           ucCallTrigReselForDiscSupportFlg;                            /* ����������ѡС��disconnectԭ��ֵ���ƵĿ���VOS_TRUE:�򿪣�VOS_FALSE:�ر�*/
    VOS_UINT8                           ucCallTrigReselDiscCauseNum;                                 /* ����������ѡС��ԭ��ֵ���� */
    VOS_UINT8                           ucDisconnectNum;                                             /* �쳣�ҶϵĴ������ﵽ�ô����ʹ���AS������ѡС��,Ŀǰδʹ�ã���Ϊ��չ�� */
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           aucCallTrigReselDiscCause[TAF_NVIM_CALL_TRIG_RESEL_DISC_CAUSE_MAX_NUM]; /* �ز�ԭ��ֵ */
}TAF_CALL_TRIG_RESEL_DISC_CAUSE_CFG_STRU;


typedef struct
{
    VOS_UINT8                           ucCallRedialSupportFlg;     /* ֧�ֺ����ؽ����ܱ�־��VOS_TRUE:֧�֣�VOS_FALSE:��֧��*/
    VOS_UINT8                           aucReserved1[3];
    VOS_UINT32                          ulCallRedialPeriod;         /* �����ؽ����Դ�ʱ�������ؽ����ʱ������λ���� */
    VOS_UINT32                          ulCallRedialInterval;       /* �����ؽ����Դ�ʱ�������ز������λ���� */
}MN_CALL_REDIAL_CFG_STRU;


typedef struct
{
    VOS_UINT8                           ucEcallT9Len;                /* T9ʱ����ʱ��ʱ������λΪСʱ��1~12Сʱ */
    VOS_UINT8                           ucEcallRedialSupportFlg;   /* ֧��eCall�ؽ����ܱ�־��VOS_TRUE:֧�֣�VOS_FALSE:��֧��*/
    VOS_UINT8                           aucReserved1[2];
    VOS_UINT16                          usEcallRedialPeriod;         /* eCall�ؽ����Դ�ʱ��eCall�ؽ����ʱ������λ���� */
    VOS_UINT16                          usEcallRedialInterval;       /* eCall�ؽ����Դ�ʱ��eCall�ز������λ���� */
}TAF_CALL_NVIM_ECALL_CFG_STRU;



typedef struct
{
    VOS_INT16                           sHandSetVolValue;
    VOS_INT16                           sHandsFreeVolValue;
    VOS_INT16                           sCarFreeVolValue;
    VOS_INT16                           sEarphoneVolValue;
    VOS_INT16                           sBlueToothVolValue;
    VOS_INT16                           sPcVoiceVolValue;
    VOS_INT16                           sHeadPhoneVolValue;
    VOS_INT16                           sSuperFreeVolValue;
    VOS_INT16                           sSmartTalkVolValue;
    VOS_INT16                           sPreVolume;
    VOS_UINT16                          usCurrDevMode;
    VOS_UINT16                          usPreDevMode;
    VOS_UINT16                          usSetVoiceFlg;                          /*����DEV��Vol�Ĺ�����flag��ȷ����modemֻ��һ�������ù�����*/
    VOS_INT16                           sRsv[3];
} APP_VC_NV_CLVL_VOLUME_STRU;

/* �ն˿���������: ����Ĭ��APN���� */
typedef struct
{
    VOS_UINT8   ucStatus;       /* 1: NV��Ч��־λ��0����Ч */
    VOS_UINT8   aucApn[TAF_NVIM_MAX_APN_STR_LEN];       /* APN�Ӹ�����ĵ�һ���ֽڿ�ʼд�룬������'\0'��Ϊ��β */
    VOS_UINT8   aucRsv[3];
}APS_APN_CUSTOMIZE_STRU;


typedef struct
{
    VOS_UINT8                                               ucEtwsEnableFlg;    /* ETWS���Կ���, VOS_TRUE:����, VOS_FALSE:δ���� */
    VOS_UINT8                                               ucRsv;
    VOS_UINT16                                              usTempEnableCbsTimeLen; /* ��λ:����, CBS����δ����ʱ,�յ���֪ͨ����ʱʹ��CBS�Խ��մ�֪ͨ��ʱ�� */

    TAF_CBA_NVIM_ETWS_DUP_DETC_CFG_STRU                     stDupDetcCfg;       /* TAF_CBA_NV_MAX_USER_SPEC_ETWS_MSGID_RANGE_NUM Duplication Detection Time ������ */
    TAF_CBA_NVIM_ETWS_MSGID_RANGE_STRU                      astSpecEtwsMsgIdList[TAF_CBA_NV_MAX_USER_SPEC_ETWS_MSGID_RANGE_NUM];   /* �û����Ƶ�ETWS��ϢID��Χ */
}TAF_CBA_NVIM_ETWS_CFG_STRU;


typedef struct
{
    VOS_UINT8                           enAlsLine;
    VOS_UINT8                           aucMmaImsi[9];
}MN_CALL_ALS_LINE_INFO_STRU;


typedef struct
{
    VOS_UINT8                          aucAtClientConfig[TAF_AT_NVIM_CLIENT_CONFIG_LEN];
}TAF_AT_NVIM_AT_CLIENT_CONFIG_STRU;


typedef struct
{
    VOS_UINT8                          aucAtClientConfig[TAF_AT_NVIM_CLIENT_CFG_LEN+32];     /*������nvԤ����32���ֽ�*/
}TAF_AT_NVIM_AT_CLIENT_CFG_STRU;


typedef struct
{
    VOS_UINT8                           ucECallNotifySupport;                   /* �����������¶ȱ�������NV�� */
    VOS_UINT8                           ucSpyStatusIndSupport;                  /* �±�״̬�ϱ�ʹ��NV�� */
    VOS_UINT8                           aucReserved[2];                          /* ������ */
}TAF_TEMP_PROTECT_CONFIG_STRU;


typedef struct
{
    VOS_UINT8                           ucNetSelMenuFlg;                        /* ����ѡ��˵����ƣ�VOS_TRUE:���VOS_FALSE:δ���� */
    VOS_UINT8                           ucRatBalancingFlg;                      /* ���뼼��ƽ����ƣ�VOS_TRUE:���VOS_FALSE:δ���� */
    VOS_UINT8                           aucReserved[2];                         /* ���� */
}NVIM_ATT_ENS_CTRL_STRU;


typedef struct
{
    VOS_UINT32                          aulUserSetLtebands[2];
}TAF_NVIM_USER_SET_LTEBANDS_STRU;


typedef struct
{
    VOS_UINT32                          ulDSTotalSendFluxLow;
    VOS_UINT32                          ulDSTotalSendFluxHigh;
    VOS_UINT32                          ulDSTotalReceiveFluxLow;
    VOS_UINT32                          ulDSTotalReceiveFluxHig;
}TAF_NVIM_RABM_TOTAL_RX_BYTES_STRU;


typedef struct
{
    VOS_UINT8                           aucRoamWhiteListFlag[2];
    VOS_UINT8                           ucReserve1;
    VOS_UINT8                           ucReserve2;
}TAF_NVIM_E5_ROAM_WHITE_LIST_SUPPORT_FLG_STRU;


typedef struct
{
    VOS_UINT32                         ulNdisDialUpAdd;
}TAF_NVIM_NDIS_DIALUP_ADDRESS_STRU;

/*nv 9130, Ϊipv6��Ŀ����nvԤ��*/

typedef struct
{
    VOS_UINT32                          ulActiveFlag;
    VOS_UINT8                           aucSmCause[TAF_NV_IPV6_FALLBACK_EXT_CAUSE_MAX_NUM];

} TAF_NV_IPV6_FALLBACK_EXT_CAUSE_STRU;


typedef struct
{
    VOS_UINT32                          ulIpv6RouterMtu;
}TAF_NDIS_NV_IPV6_ROUTER_MTU_STRU;


typedef struct
{
    VOS_INT32                           lValue;
    VOS_INT8                            acPassword[16];
}TAF_AT_NV_DISLOG_PWD_STRU;


typedef struct
{
    VOS_UINT32                          ulE5RightFlag;
}TAF_AT_NV_E5_RIGHT_FLAG_STRU;


typedef struct
{
    VOS_UINT32                          ulDissdFlag;
}TAF_AT_NV_DISSD_FLAG_STRU;


typedef struct
{
    VOS_UINT32                          ulOmPortNum;
}TAF_AT_NV_OM_PORT_NUM_STRU;


typedef struct
{
    VOS_UINT8                           ucLength;
    VOS_UINT8                           aucData[53];
    VOS_UINT8                           ucReserve1;
    VOS_UINT8                           ucReserve2;
}TAF_AT_NV_RA_CAPABILITY_STRU;


typedef struct
{
    VOS_UINT8                           aucUmtsAuth[2];
    VOS_UINT8                           ucReserve1;
    VOS_UINT8                           ucReserve2;
}TAF_AT_NV_UMTS_AUTH_STRU;


typedef struct
{
    VOS_UINT8                           aucGmmInfo[2];
    VOS_UINT8                           ucReserve1;
    VOS_UINT8                           ucReserve2;
}TAF_AT_NV_GMM_INFO_STRU;


typedef struct
{
    VOS_UINT8                           aucMmInfo[2];
    VOS_UINT8                           ucReserve1;
    VOS_UINT8                           ucReserve2;
}TAF_AT_NV_MM_INFO_STRU;


typedef struct
{
    VOS_UINT8                           aucSmsText[16];
}TAF_AT_NV_SMS_TEXT_STRU;


typedef struct
{
    VOS_UINT32                          ulStatus;
    VOS_UINT32                          ulGprsRecentActTime;
}TAF_AT_NV_CUSTOM_GPRS_RECENT_ACT_TIMER_STRU;


typedef struct
{
    VOS_UINT16                          usPsDelayFlag;
    VOS_UINT8                           ucReserve1;
    VOS_UINT8                           ucReserve2;
}TAF_AT_NV_PS_DELAY_FLAG_STRU;


typedef struct
{
    VOS_UINT8                           aucEhplmnSupportFlag[2];
    VOS_UINT8                           ucReserve1;
    VOS_UINT8                           ucReserve2;
}TAF_AT_NV_EHPLMN_SUPPORT_FLAG_STRU;


typedef struct
{
    VOS_UINT8                          aucBgFsFbsRatio[4];
}TAF_AT_NV_BG_FS_FBS_RATIO_STRU;


typedef struct
{
    VOS_UINT16                          usEplmnUseRatFlag;
    VOS_UINT8                           ucReserve1;
    VOS_UINT8                           ucReserve2;
}TAF_AT_NV_EPLMN_USE_RAT_FLAG_STRU;

typedef struct
{
    VOS_UINT8                                               ucLabel[TAF_NVIM_CBA_MAX_LABEL_NUM]; /* С���㲥��Ϣid��Χ��ǩ */
    VOS_UINT16                                              usMsgIdFrom;                    /* С���㲥��ϢID�Ŀ�ʼ���  */
    VOS_UINT16                                              usMsgIdTo;                      /* С���㲥��ϢID�Ľ������ */
    VOS_UINT32                                              ulRcvMode;                      /* ÿ��CBMI RANGE �Ľ���ģʽ, Ŀǰ��֧�� ACCEPT��ģʽ */
}TAF_CBA_NVIM_CBMI_RANGE_STRU;

typedef struct
{
    VOS_UINT32                          ulCbmirNum;                             /* С���㲥��Ϣ��ID���� */
    TAF_CBA_NVIM_CBMI_RANGE_STRU        astCbmiRangeInfo[TAF_CBA_NVIM_MAX_CBMID_RANGE_NUM]; /* С���㲥��Ϣ�ķ�Χ��Ϣ */
}TAF_CBA_NVIM_CBMI_RANGE_LIST_STRU;



typedef struct
{
    VOS_UINT16                          usPlatform;
    VOS_UINT8                           ucReserve1;
    VOS_UINT8                           ucReserve2;
}NAS_NVIM_PLATFORM_STRU;


typedef struct
{
    VOS_UINT8                           ucMuxSupportFlg;
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucReserved2;
    VOS_UINT8                           ucReserved3;
}TAF_AT_NVIM_MUX_SUPPORT_FLG_STRU;

/*����USSDת���ַ���*/
typedef struct
{
    VOS_UINT8                           ucStatus;            /*�Ƿ񼤻0�����1���� */
    VOS_UINT8                           ucAlphaTransMode;    /* �ַ���ת��*/
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucReserved2;
}SSA_NV_ITEM_ALPHA_to_ASCII_STRU;


typedef struct
{
    VOS_UINT8                           ucCbStatus;         /* CBSҵ���Ƿ����ñ�־*/

    VOS_UINT8                           ucDupDetectCfg;     /* �ظ��������� */


    VOS_UINT8                           ucRptAppFullPageFlg; /* VOS_TRUE: �ϱ���ҳ��88���ֽ�; VOS_FALSE: �ϱ�ʵ����Ч�ֽ� */

    VOS_UINT8                           ucRsv;              /* NV����صĽṹ�壬��4�ֽڷ�ʽ�£����ֶ�����ն� */
}TAF_CBA_CBS_SERVICE_PARM_STRU;


typedef struct
{
    MTA_WCDMA_BAND_ENUM_UINT16          enBand;                     /* WCDMAƵ�� */
    VOS_INT16                           sPower;                     /* ��������ֵ */
}MTA_BODY_SAR_W_PARA_STRU;


typedef struct
{
    VOS_INT16                           sGPRSPower;                 /* GPRS��������ֵ */
    VOS_INT16                           sEDGEPower;                 /* EDGE��������ֵ */
}MTA_BODY_SAR_G_PARA_STRU;


typedef struct
{
    VOS_UINT32                          ulGBandMask;                                /* GSM Band��Maskλ */
    VOS_UINT16                          usWBandNum;                                 /* WCDMA��Band���� */
    VOS_UINT16                          ausReserved1[1];                            /* ����λ */
    MTA_BODY_SAR_G_PARA_STRU            astGBandPara[MTA_BODY_SAR_GBAND_MAX_NUM];   /* GSM��������ֵ */
    MTA_BODY_SAR_W_PARA_STRU            astWBandPara[MTA_BODY_SAR_WBAND_MAX_NUM];   /* WCDMA��������ֵ */
}MTA_BODY_SAR_PARA_STRU;

typedef struct
{
    VOS_UINT32  ulNvStatus;
    VOS_UINT8   aucFirstPortStyle[17];   /* �豸�л�ǰ�˿���̬ */
    VOS_UINT8   aucRewindPortStyle[17];  /* �豸�л���˿���̬ */
    VOS_UINT8   aucReserved[22];         /* ���� */
}AT_DYNAMIC_PID_TYPE_STRU;

/* Added by L47619 for V3R3 Share-PDP Project, 2013-6-3, begin */

typedef struct
{
    VOS_UINT8                           ucEnableFlag;       /* �Ƿ�ʹ��Share PDP����, 0 - ��ֹ;  1 - ʹ�� */
    VOS_UINT8                           ucReserved;         /* ���� */
    VOS_UINT16                          usAgingTimeLen;     /* �ϻ�ʱ�䣬��λΪ�� */

} TAF_NVIM_SHARE_PDP_INFO_STRU;


typedef struct
{
    VOS_UINT8                           ucEnableFlag;  /* AT+CFUN=0��ػ���ȥ����(U)SIM������ʹ�ܱ�ʶ, 0 - ��ֹ, 1 - ʹ��*/
    VOS_UINT8                           aucReserved[3];
} TAF_NVIM_DEACT_SIM_WHEN_POWEROFF_STRU;
/* Added by L47619 for V3R3 Share-PDP Project, 2013-6-3, end */

/* ���Ƶ�MSCC module */


typedef struct
{
    VOS_UINT32                          bitCardNum      : 3;
    VOS_UINT32                          bitReserved0    : 5;
    VOS_UINT32                          bitCard0        : 3;
    VOS_UINT32                          bitCard1        : 3;
    VOS_UINT32                          bitCard2        : 3;
    VOS_UINT32                          bitReserved1    : 15;
} TAF_NV_SCI_CFG_STRU;


typedef struct
{
    VOS_UINT8                           ucNvimValid;
    VOS_UINT8                           ucSmsClosePathFlg;
} TAF_NVIM_SMS_CLOSE_PATH_CFG_STRU;



typedef struct
{
    /* λ���� :0�����1:����
    bit0�����ƶ�˫����B39/B3���ų�ͻ����
    bit1��Notch Bypass���Կ���
    bit2: NarrowBand Dcs ����(V9 not support)
    bit3: B39/B3 ���ų�ͻ����2(G��Ƶ����ģʽ)
    bit4: RES ʹ�ܿ��أ�1��ʾ֧��RSE���ԣ�0��ʾ��֧��RSE����
    */
    VOS_UINT8                          ucSolutionMask;
    VOS_UINT8                          aucAdditonCfg[3];
} MTC_NV_RF_INTRUSION_CFG_STRU;

/* Added by f62575 for VSIM FEATURE, 2013-8-29, begin */

typedef struct
{
    VOS_UINT8                           ucVsimCtrlFlg;
    VOS_UINT8                           aucReserved1;
    VOS_UINT8                           aucReserved2;
    VOS_UINT8                           aucReserved3;
} TAF_NVIM_VSIM_CFG_STRU;

/* Added by f62575 for VSIM FEATURE, 2013-8-29, end */

typedef struct
{
    VOS_UINT8                           ucIsSsRetrySupportFlg;                  /* �Ƿ�֧��ss�ط����ܣ�VOS_TRUE:֧�֣�VOS_FALSE:��֧��*/
    VOS_UINT8                           aucReserved1[3];
    VOS_UINT32                          ulSsRetryPeriod;                        /* ss�ط����Դ�ʱ��ss�ط����ʱ������λ��,Ĭ��30s */
    VOS_UINT32                          ulSsRetryInterval;                      /* ss�ط����Դ�ʱ��ss�ط������λ�룬Ĭ��5s */
}TAF_SSA_NVIM_RETRY_CFG_STRU;


typedef struct
{
    VOS_UINT8                           ucSsRetryCmSrvRejCauseNum;                                      /* ss�ط�ԭ��ֵ����*/
    VOS_UINT8                           aucReserved1[3];
    VOS_UINT8                           aucSsRetryCmSrvRejCause[TAF_NVIM_SS_RETRY_CAUSE_MAX_NUM];       /* ss�ط�ԭ�� */
}TAF_SSA_NVIM_RETRY_CM_SRV_REJ_CFG_STRU;



typedef struct
{
    VOS_UINT16                          usWaitUserRspLen;                       /* �ȴ��û���Ӧ��ʱ�� */
    VOS_UINT8                           aucReserved1;
    VOS_UINT8                           aucReserved2;
}TAF_NVIM_SS_WAIT_USER_RSP_LEN_STRU;


typedef struct
{
    VOS_UINT32                          ulSmsRiOnInterval;      /* ����RI�ߵ�ƽ����ʱ��(ms) */
    VOS_UINT32                          ulSmsRiOffInterval;     /* ����RI�͵�ƽ����ʱ��(ms) */

    VOS_UINT32                          ulVoiceRiOnInterval;    /* ����RI�ߵ�ƽ����ʱ��(ms) */
    VOS_UINT32                          ulVoiceRiOffInterval;   /* ����RI�͵�ƽ����ʱ��(ms) */
    VOS_UINT8                           ucVoiceRiCycleTimes;    /* ����RI�������ڴ���     */
    VOS_UINT8                           aucReserved[3];

} TAF_NV_UART_RI_STRU;


typedef struct
{
    VOS_UINT8                           ucFormat;               /* UART ����λֹͣλλ�� */
    VOS_UINT8                           ucParity;               /* UARTУ�鷽ʽ */
    VOS_UINT8                           aucReserved[2];

} TAF_NV_UART_FRAME_STRU;


typedef struct
{
    VOS_UINT32                          ulBaudRate;             /* UART������ */
    TAF_NV_UART_FRAME_STRU              stFrame;                /* UART֡��ʽ */
    TAF_NV_UART_RI_STRU                 stRiConfig;             /* UART Ring������ */

} TAF_NV_UART_CFG_STRU;


typedef struct
{
    VOS_UINT32                          ulDlRate;
    VOS_UINT32                          ulUlRate;
    VOS_UINT32                          ulDdrBand;
} TAF_NV_DFS_RATE_BAND_STRU;


typedef struct
{
    VOS_UINT32                          ulProfileNum;
    TAF_NV_DFS_RATE_BAND_STRU           astProfile[TAF_NVIM_DFS_MAX_PROFILE_NUM];
} TAF_NV_DFS_DSFLOW_RATE_CONFIG_STRU;

typedef struct
{
    TAF_NV_DFS_DSFLOW_RATE_CONFIG_STRU          astDfsConfig[TAF_NVIM_DIFF_DFS_NUM];
} TAF_NV_MULTI_DFS_DSFLOW_RATE_CONFIG_STRU;


typedef struct
{
    VOS_UINT32                          ulFlagValue;
    VOS_UINT32                          ulReserved;
} TAF_NV_VOICE_TEST_FLAG_STRU;


typedef struct
{
    VOS_UINT8                          ucSmsDomain;
    VOS_UINT8                          aucReserved[3];
} TAF_NVIM_SMS_DOMAIN_STRU;



typedef struct
{
    VOS_UINT8                           ucEnableFlg;
    VOS_UINT8                           aucReserved[3];
} TAF_NV_PORT_BUFF_CFG_STRU;



typedef struct
{
    VOS_UINT8                                     ucLCEnableFlg;
    TAF_NVIM_LC_RAT_COMBINED_ENUM_UINT8           enRatCombined;
    TAF_NVIM_LC_WORK_CFG_ENUM_UINT8               enLCWorkCfg;
    VOS_UINT8                                     aucReserved[1];
}TAF_NV_LC_CTRL_PARA_STRU;


typedef struct
{
    VOS_UINT8                 ucImsRedialCauseNum;                                                  /* IMS call�ز�ԭ��ֵ���� */
    VOS_UINT8                 ucImsaRedialCauseNum;                                                 /* IMSA call�ز�ԭ��ֵ���� */
    VOS_UINT8                 aucReserve[2];
    VOS_UINT16                ausImsRedialCause[TAF_NVIM_IMS2CS_CALL_REDIAL_CAUSE_MAX_NUM];
    VOS_UINT16                ausImsaRedialCause[TAF_NVIM_IMSA2CS_CALL_REDIAL_CAUSE_MAX_NUM];
}TAF_NV_SWITCH_IMS_TO_CS_REDIAL_CAUSE_CONFIG_STRU;


typedef struct
{
    VOS_UINT8                                               ucCallRedial;                           /* call�ز�flag */
    VOS_UINT8                                               ucSmsRedial;                            /* SMS �ز�flag */
    VOS_UINT8                                               ucSsRedial;                             /* SS  �ز�flag */
    /* �����������������֮ǰ�ı���λ�����飬���ܸı�NV�Ľṹ */
    TAF_IMS2CS_REDIAL_AFTER_ALERTING_DOMAIN_ENUM_UINT8      aenImsRedialAfterAlertingDomain[1];     /* IMS�绰��alertingʧ�ܺ�֧���ز����� */
    TAF_NV_SWITCH_IMS_TO_CS_REDIAL_CAUSE_CONFIG_STRU        stCallRedialCauseCfg;
}TAF_NV_SWITCH_IMS_TO_CS_REDIAL_CONFIG_STRU;


typedef struct
{
    VOS_UINT8                                               ucLteWifiCauseDifferFlag; /* VOLTE��VOWIFIԭ��ֵ���ֱ�־ */
    VOS_UINT8                                               aucReserve[3];
    TAF_NV_SWITCH_IMS_TO_CS_REDIAL_CAUSE_CONFIG_STRU        stCallRedialCauseCfg;
}TAF_NV_SWITCH_WIFI_TO_CS_REDIAL_CONFIG_STRU;


typedef struct
{
    VOS_UINT8                                              ucCallRedial;
    VOS_UINT8                                              ucSmsRedial;
    VOS_UINT8                                              ucSsRedial;
    VOS_UINT8                                              aucReserve[1];
}TAF_NV_SWITCH_CS_TO_IMS_REDIAL_CONFIG_STRU;


typedef struct
{
    VOS_UINT8                           ucCallRedial;
    VOS_UINT8                           ucSmsRedial;
    VOS_UINT8                           ucSsRedial;
    VOS_UINT8                           aucReserve[1];
}TAF_NV_SWITCH_DOMAIN_REDIAL_CONFIG_STRU;


typedef struct
{
    VOS_UINT8                           ucImsRoamingFlg;
    VOS_UINT8                           aucReserve[3];
}TAF_NV_IMS_ROAMING_SUPPORT_STRU;


typedef struct
{
    VOS_UINT8                           ucUssdOnImsSupportFlag;
    VOS_UINT8                           aucReserve[3];
}TAF_NV_IMS_USSD_SUPPORT_STRU;


typedef struct
{
    MTC_PS_TRANSFER_ENUM_UINT8         enSolutionCfg;
    VOS_UINT8                          aucAdditonCfg[3];
} MTC_NV_PS_TRANSFER_CFG_STRU;


typedef struct
{
    TAF_FLASH_DIRECTORY_TYPE_ENUM_UINT16                     enFlashDirectoryType;
    VOS_UINT8                                                aucReserved1[2];
}TAF_NV_FLASH_DIRECTORY_TYPE_STRU;


typedef struct
{
    VOS_UINT32                          ulTimerInterval;                        /* ʱ����ֵ,��λ�� */
}MTC_RF_LCD_TIMER_INTERVAL_STRU;


typedef struct
{
    VOS_UINT32                          ulMipiClk;                              /* MipiClkֵ */
    VOS_UINT32                          aulFreq[MTC_RF_LCD_MIPICLK_FREQ_MAX_NUM];/* ulMipiClkֵӰ���Ƶ�ʱ� */
}MTC_NVIM_RF_LCD_MIPICLK_FREQ_STRU;


typedef struct
{
    VOS_UINT16                          usEnableBitMap;                         /* ʹ�ܿ��أ�ÿ��bit����ͬ���㷨����0����ʹ�� */
    VOS_UINT16                          usFreqWidth;                            /* ���� ��λ100KHZ */
    MTC_NVIM_RF_LCD_MIPICLK_FREQ_STRU   astRfMipiClkFreqList[MTC_RF_LCD_MIPICLK_MAX_NUM]; /* ��λ100KHZ */
} MTC_NVIM_RF_LCD_CFG_STRU;


typedef struct
{
    VOS_UINT32                          ulWaterLevel1;                          /* ˮ�߽��1 */
    VOS_UINT32                          ulWaterLevel2;                          /* ˮ�߽��2 */
    VOS_UINT32                          ulWaterLevel3;                          /* ˮ�߽��3 */
    VOS_UINT32                          ulWaterLevel4;                          /* ˮ�߽��4,Ԥ�� */
} ADS_UL_WATER_MARK_LEVEL_STRU;


typedef struct
{
    VOS_UINT32                          ulThreshold1;                           /* �ް�����1 */
    VOS_UINT32                          ulThreshold2;                           /* �ް�����2 */
    VOS_UINT32                          ulThreshold3;                           /* �ް�����3 */
    VOS_UINT32                          ulThreshold4;                           /* �ް�����4 */
} ADS_UL_THRESHOLD_LEVEL_STRU;


typedef struct
{
    VOS_UINT32                          ulActiveFlag;                           /* ʹ�ܱ�ʶ: 0��ʾȥʹ��,1��ʾʹ�� */
    VOS_UINT32                          ulProtectTmrExpCnt;                     /* ������ʱ����ʱ����ʱ�� */
    ADS_UL_WATER_MARK_LEVEL_STRU        stWaterMarkLevel;
    ADS_UL_THRESHOLD_LEVEL_STRU         stThresholdLevel;
    VOS_UINT32                          aulReserved[6];
} ADS_NV_DYNAMIC_THRESHOLD_STRU;


typedef struct
{
    /* VOS_TRUE:���ata�첽�ϱ�ok������connect���ϱ�ok����������connect ack��
       VOS_FALSE:δ����,ataͬ���ϱ�������connect����յ�����connect ack�����ϱ�ok */
    VOS_UINT8                           ucAtaReportOkAsyncFlag;
    VOS_UINT8                           aucReserved1[3];
}TAF_CALL_NVIM_ATA_REPORT_OK_ASYNC_CFG_STRU;




typedef struct
{
    VOS_UINT8                           ucMode;                                 /* JAM���õ�ģʽ��0:�ر�, 1:�� */
    VOS_UINT8                           ucMethod;                               /* JAM���ʹ�õķ�����1:����1��2:����2,Ŀǰֻ֧��2 */
    VOS_UINT8                           ucFreqNum;                              /* �����Ҫ�ﵽ��Ƶ�������ȡֵ��Χ:[0,255] */
    VOS_UINT8                           ucThreshold;                            /* �����Ҫ�ﵽ��Ƶ�����ֵ��ȡֵ��Χ:[0,70] */
    VOS_UINT8                           ucJamDetectingTmrLen;                   /* ǿ�ź�Ƶ������������޺󣬼��ͬ������Ķ�ʱ��ʱ��(s) */
    VOS_UINT8                           ucJamDetectedTmrLen;                    /* ��⵽���ź󣬵ȴ�����������ʱ��(s) */
    VOS_UINT8                           ucFastReportFlag;                       /* �����������Ƿ���Ҫ�����ϱ� */
    VOS_UINT8                           aucRsv[1];
}NV_NAS_JAM_DETECT_CFG_STRU;

typedef struct
{
    VOS_UINT8                           ucMode;                                 /* JAM���õ�ģʽ��0:�ر�, 1:�� */
    VOS_UINT8                           ucMethod;                               /* JAM���ʹ�õķ�����1:����1��2:����2,Ŀǰֻ֧��2 */
    VOS_UINT8                           ucJamBandWidth;                         /* �������źŴ���Ĭ��30M */
    VOS_UINT8                           ucFastReportFlag;                       /* �����������Ƿ���Ҫ�����ϱ� */
    VOS_UINT8                           ucRssiSrhThreshold;                     /* ����RSSIʱ�����Ҫ�ﵽ��Ƶ�����ֵ��ȡֵ��Χ:[0,70]��ʵ���õ�ʱ���70ʹ�� */
    VOS_UINT8                           ucRssiSrhFreqPercent;                   /* ����RSSIʱ�����Ҫ�ﵽ��Ƶ�����ռƵ������(BAND�͸����ź�ȡ����)�İٷֱȣ�ȡֵ��Χ:[0,100] */
    VOS_UINT16                          usPschSrhThreshold;                     /* ����PSCHʱ�����ҪС�ڵ��ڵ�Ƶ�����ֵ��ȡֵ��Χ:[0,65535] */
    VOS_UINT8                           ucPschSrhFreqPercent;                   /* ����PSCHʱ�����Ҫ�ﵽ��Ƶ�����ռƵ������(BAND�͸����ź�ȡ����)�İٷֱȣ�ȡֵ��Χ:[0,100] */
    VOS_UINT8                           ucJamDetectingTmrLen;                   /* ǿ�ź�Ƶ������������޺󣬼��ͬ������Ķ�ʱ��ʱ��(s) */
    VOS_UINT8                           ucJamDetectedTmrLen;                    /* ��⵽���ź󣬵ȴ�����������ʱ��(s) */
    VOS_UINT8                           aucRsv[1];
}NV_NAS_WCDMA_JAM_DETECT_CFG_STRU;

typedef struct
{
    VOS_UINT32                          ulDebugLevel;                           /* VCOM DEBUG����:ERR,NORMAL,INFO,DEBUG */
    VOS_UINT32                          ulAppVcomPortIdMask;                    /* VCOM�˿�ID���� */
    VOS_UINT32                          ulReserved[4];                          /* Ԥ�� */
} TAF_NV_PORT_DEBUG_CFG_STRU;


typedef struct
{
    VOS_UINT8                           ucEnableFlg;                            /* 1:��Ƶ���ܴ򿪣�0:���ܹر� */
    TAF_NVIM_RAT_MODE_ENUM_UINT8        enRatMode;
    VOS_UINT16                          usLockedFreq;
    TAF_NVIM_GSM_BAND_ENUM_UINT16       enBand;
    VOS_UINT8                           aucReserved[2];
} TAF_NVIM_FREQ_LOCK_CFG_STRU;


typedef struct
{
    TAF_NV_ACTIVE_MODEM_MODE_ENUM_UINT8 enActiveModem;
    VOS_UINT8                           aucReserve[3];
}TAF_NV_DSDS_ACTIVE_MODEM_MODE_STRU;


typedef struct
{
    VOS_UINT8                           ucEnableFlg;                            /* 1:���ܴ򿪣�0:���ܹر� */
    VOS_UINT8                           ucReserved;
    VOS_UINT16                          usMcc;                                  /* �����룬��ʽʾ��: 0x0460(�й���½) */
} TAF_NVIM_ECID_TL2GSM_CFG_STRU;


typedef struct
{
    VOS_UINT8                           ucOperNameServicePrioNum;

    /* 1����֧��PNN��cosp��^eonsucs2��ѯʱ���PNN�ļ�����ƥ�����Ӫ������;
       2����֧��CPHS,cops��^eonsucs2��ѯʱ���CPHS�ļ�����ƥ�����Ӫ������;
       3����֧��MM INFO,cops��^eonsucs2��ѯʱ��mm/gmm/emm information�в���ƥ�����Ӫ������;
       4����֧��SPN,cops��^eonsucs2��ѯʱ���SPN�ļ�����ƥ�����Ӫ������*/
    VOS_UINT8                           aucOperNameSerivcePrio[TAF_NVIM_MAX_OPER_NAME_SERVICE_PRIO_NUM];
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucReserved2;
    VOS_UINT8                           ucReserved3;
    VOS_UINT8                           ucPlmnCompareSupportWildCardFlag; /* plmn�Ƚ�ʱ�Ƿ�֧��ͨ��� */
    VOS_UINT8                           ucWildCard;                       /* ͨ���,ȡֵa-f,�ɴ���0-9�������� */
    VOS_UINT8                           ucReserved4;
    VOS_UINT8                           ucReserved5;                      /* �����ڲ���������^usimstub��^refreshstub�����Ƿ����*/
}TAF_NVIM_ENHANCED_OPER_NAME_SERVICE_CFG_STRU;


typedef struct
{
    VOS_UINT8                           ucRefreshAllFileRestartFlag;  /* 0: mma�յ�pih��refreshָʾ�����ļ����ʱ������modem; 1:mma�յ�pih��refreshָʾ�����ļ����ʱ�޽����绰ҵ��ʱ����modem*/
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucReserved2;
    VOS_UINT8                           ucReserved3;
}TAF_NVIM_REFRESH_ALL_FILE_RESTART_CFG_STRU;



typedef struct
{
    VOS_UINT8                           ucNormalCallDelayRestartFlg;            /* �յ�refresh������������绰ʱ: 0: ��������modem; 1:�ӳ�����modem */
    VOS_UINT8                           ucSmsDelayRestartFlg;                   /* �յ�refresh�������smsʱ: 0: ��������modem; 1:�ӳ�����modem */
    VOS_UINT8                           ucSsDelayRestartFlg;                    /* �յ�refresh�������ssʱ: 0: ��������modem; 1:�ӳ�����modem */
    VOS_UINT8                           ucNoServiceDelayRestartFlg;             /* �յ�refresh���û��ҵ��ʱ: 0: ��������modem; 1:�ӳ�����modem */
    VOS_UINT8                           ucReserved2;
    VOS_UINT8                           ucReserved3;
    VOS_UINT8                           ucReserved4;
    VOS_UINT8                           ucReserved5;
}TAF_NVIM_REFRESH_USIM_DELAY_RESTART_CFG_STRU;



typedef struct
{
    VOS_UINT8                           ucAllowDefPdnTeardownFlg;
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucReserved2;
    VOS_UINT8                           ucReserved3;

} TAF_NV_PDN_TEARDOWN_POLICY_STRU;

typedef struct
{
    TAF_XSMS_DATA_REQ_SEND_CHAN_ENUM_UINT8                  enXsmsChannelOption;    /* �����ŵ���ר���ŵ� */
    VOS_UINT8                                               ucResendMax;            /* ���������������5�� */
    VOS_UINT8                                               ucResendInterval;       /* ���·������ݵ������ĵȴ�ʱ�䣬1�� */
    VOS_UINT8                                               ucReconnectInterval;    /* ���½������ӵĵȴ�ʱ�䣬5�� */
}TAF_NVIM_1X_XSMS_CFG_STRU;


typedef struct
{
    VOS_UINT8                           ucIsCfgEnableFlg;                /* ʹ��λ��TRUEΪ������FALSEΪ�ر� */
    VOS_UINT8                           ucTimerLength;                   /* ����ʱ������λ��  */
    VOS_UINT8                           aucReserved[2];
}TAF_NVIM_1X_MT_SMS_TCH_RELEASE_CFG;


typedef struct
{
    TAF_MMA_CFREQ_LOCK_MODE_TYPE_ENUM_UINT8                 enFreqLockMode;
    VOS_UINT8                                               aucReserve[3];
    VOS_UINT16                                              usSid;
    VOS_UINT16                                              usNid;
    VOS_UINT16                                              usCdmaBandClass;
    VOS_UINT16                                              usCdmaFreq;
    VOS_UINT16                                              usCdmaPn;
    VOS_UINT16                                              usEvdoBandClass;
    VOS_UINT16                                              usEvdoFreq;
    VOS_UINT16                                              usEvdoPn;
}TAF_NVIM_CFREQ_LOCK_CFG_STRU;


typedef struct
{
    VOS_UINT8                           ucTTYMode;/*TTYģʽֵ��0-3��default:0*/
    VOS_UINT8                           ucReserved0;
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucReserved2;

} TAF_NV_TTY_CFG_STRU;


typedef struct
{
    VOS_UINT32                          ulStatisticTime;                        /* ͳ��ʱ�䣬��λΪ�� */
    VOS_UINT32                          ulSwitchNum;                            /* gutlƵ���л��Ĵ��� */
} TAF_NV_RAT_FREQUENTLY_SWITCH_CHR_RPT_CFG_STRU;


typedef struct
{
    /* ��NV �������ú��еȴ��Ŀ���ģʽ��ulCcwaCtrlModeΪ0������еȴ���3gpp������ƣ�
       ulCcwaCtrlModeΪ1������еȴ���UE���ƣ�����VOLTE�����硣
      ����VoLTE�������ϣ�AP����CCWA֧��ʱ��IMS��û�к����罻����VoLTE�ĵ绰��CCWA��UE���ƣ���
       ֧��VOLTE���նˣ�����VOLTE������û�д򿪣������·�CCWAI�����ú��еȴ���
    */
    TAF_CALL_CCWA_CTRL_MODE_ENUM_U8     enCcwaCtrlMode;
    TAF_CALL_CCWAI_MODE_ENUM_UINT8      enCcwaiMode;
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucReserved2;
} TAF_CALL_NVIM_CCWA_CTRL_MODE_STRU;


typedef struct
{
    VOS_UINT8                           ucEmcCs2ImsRedialSupportFlg;
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucReserved2;
    VOS_UINT8                           ucReserved3;
} TAF_EMC_CS_TO_IMS_REDIAL_CFG_STRU;



typedef struct
{
    VOS_UINT8                           ucCcpuResetFlag;                        /* 0:����C�˵�����λ�������ϵ� ;
                                                                                   1:��C�˵�����λ*/
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucReserved2;
    VOS_UINT8                           ucReserved3;
}TAF_NVIM_CCPU_RESET_RECORD_STRU;



typedef struct
{
    VOS_UINT16                          usSo;
    VOS_UINT8                           aucReserved[2];
}TAF_NVIM_1X_DATA_SO_CFG;




typedef struct
{
    VOS_UINT8                           ucPktCdataInactivityTmrLen;     /* CDMA���ݷ����������ݴ���ʱ��, ��λ�� */
    VOS_UINT8                           ucSwitchOnWaitCLTmerLen;        /* �����ȴ�CL������ʱ��ʱ������λ�� */
    VOS_UINT8                           uc1xBackOffToDoTmrLen;          /* 1X����ҵ������̬��ʱ������������inactive timer��ʱ����ʱ����
                                                                           1X backof to DO.��λ�룬0 ��ʾ�����Թر� */
    VOS_UINT8                           ucLessPktCdataInactivityTmrLen; /* CDMA���ݷ�������С�����ݴ���ʱ��, ��λ��, Ĭ��ֵ1s */
}TAF_NVIM_CDATA_GENERIC_CONFIG_STRU;



typedef struct
{
    VOS_UINT32                          ulEnable;           /* ʹ�ܱ�ʶ */
    VOS_UINT32                          ulTxWakeTimeout;    /* ���з��ͻ��ѳ�ʱ���� */
    VOS_UINT32                          ulRxWakeTimeout;    /* ���н��ջ��ѳ�ʱ���� */
    VOS_UINT32                          ulReserved;

} TAF_NV_ADS_WAKE_LOCK_CFG_STRU;


typedef struct
{
    VOS_UINT8                           ucIpfMode;                              /* IPF����ADS�������ݵ�ģʽ, 0: �ж�������(Ĭ��)��1���߳������� */
    VOS_UINT8                           ucReserved0;
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucReserved2;
} TAF_NV_ADS_IPF_MODE_CFG_STRU;


typedef struct
{
    VOS_UINT8                           ucNetInterfaceMode;                     /* RNIC�����������ݵ�Linux����Э��ջ�Ľӿ�ģʽ, 0: Net_rx(Ĭ��)��1��NAPI�ӿ� */
    VOS_UINT8                           ucNapiPollWeight;                       /* RNIC����NAPI��ʽһ��poll��������� */
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucReserved2;
} TAF_NV_RNIC_NET_IF_CFG_STRU;


typedef struct
{
    VOS_UINT16                          usBlkNum;
    VOS_UINT16                          usReserved0;
    VOS_UINT16                          usReserved1;
    VOS_UINT16                          usReserved2;

} TAF_NV_ADS_MEM_CFG_STRU;


typedef struct
{
    VOS_UINT32                          ulEnable;
    TAF_NV_ADS_MEM_CFG_STRU             astMemCfg[2];

} TAF_NV_ADS_MEM_POOL_CFG_STRU;



typedef struct
{
    VOS_UINT32                                              ulEmcCallRedialPeriod;  /* �������ز���ʱ��ʱ�� */
} TAF_XCALL_NVIM_1X_EMC_CALL_REDIAL_PERIOD_STRU;


typedef struct
{
    VOS_UINT16                          usSo;
    VOS_UINT8                           aucReserved[2];
}TAF_NVIM_1X_VOICE_SO_CFG;




typedef struct
{
    VOS_UINT8                           ucSecType;
    VOS_UINT8                           ucBlackListNum;
    VOS_UINT16                          ausBlackList[TAF_NV_BLACK_LIST_MAX_NUM];
} TAF_NV_NVWR_SEC_CTRL_STRU;


typedef struct
{
    VOS_UINT32                           ulIpv6AddrTestModeCfg;                 /* 0x55aa55aa�ǲ���ģʽ������ֵΪ����ģʽ */
}TAF_NVIM_IPV6_ADDR_TEST_MODE_CFG_STRU;


typedef struct
{
    VOS_UINT8                           ucEnable;                               /* 0:NV δ����; 1:NV ���� */
    VOS_UINT8                           aucReserved[3];
    VOS_UINT32                          ulRptSrvStaDelayTimerValue;             /*��ʱ�ϱ���ʱ��ʱ��,��λ:�� */
}TAF_MMA_NVIM_DELAY_REPORT_SERVICE_STATUS_STRU;


typedef struct
{
    VOS_UINT8   ucCLDelayTimerLen;                                              /* DO+LTE�ӳ��ϱ��޷���ʱ��ʱ��: 0:���ӳ��ϱ�����������ʱ���ӳٺ��ϱ� */
    VOS_UINT8   ucDoDelayTimerLen;                                              /* DO�ӳ��ϱ��޷���ʱ��ʱ��: 0:���ӳ��ϱ�����������ʱ���ӳٺ��ϱ� */
    VOS_UINT8   ucReserved1;
    VOS_UINT8   ucReserved2;
}TAF_MMA_NVIM_DELAY_RPT_CLNOSERVICE_STRU;


typedef struct
{
    VOS_UINT32                          ulStartAndStopContDtmfIntervalLen;  /* ��ʱ��ʱ������λ:ms  */
}TAF_XCALL_NVIM_START_AND_STOP_CONT_DTMF_INTERVAL_STRU;


typedef struct
{
    VOS_UINT8                           ucIsAllowCallInForeign;
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucReserved2;
    VOS_UINT8                           ucReserved3;
}TAF_XCALL_NVIM_CTCC_ROAM_EMC_CALL_CFG_STRU;



typedef struct
{
    VOS_UINT8                           ucPrintModemLogType;                    /* 0:���modem log��1:�����modem log��default:0 */
    VOS_UINT8                           ucReserved0;
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucReserved2;
} TAF_NV_PRINT_MODEM_LOG_TYPE_STRU;


typedef struct
{
    VOS_UINT8                           ucCLDelayRptEnable;
    VOS_UINT8                           aucReserved1[3];
    VOS_UINT32                          ulCLRptSrvStaDelayTimerLen;
    VOS_UINT8                           aucReserved2[12];
}TAF_MMA_NVIM_CL_DELAY_REPORT_SERVICE_STATUS_STRU;


/*****************************************************************************
 �ṹ��    : CNAS_MMA_NVIM_CDMA_SUPPORT_BANDCLASS_MASK_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : �ն�֧�ֵ�CDMAƵ����������  3601
*****************************************************************************/
typedef struct
{
    VOS_UINT32                          ulSupportBandclassMask;               /* �ն�֧�ֵ�Ƶ���������룬ÿbit��ʾ�Ƿ�֧�ֶ�Ӧ��Ƶ������������0x00000001��ʾֻ֧��Ƶ��0 */
}CNAS_MMA_NVIM_CDMA_SUPPORT_BANDCLASS_MASK_STRU;



typedef struct
{
    VOS_UINT8                           ucMergeFlg;                             /* �������������Ƿ�ϲ�ע��״̬�ϱ� */
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucReserved2;
    VOS_UINT8                           ucReserved3;
}TAF_NVIM_NONNORMAL_REG_STATUS_MERGE_CFG_STRU;


typedef struct
{
    VOS_UINT8                           ucEccSrvCap;        /* ECC��������: TRUE -- ֧��ECC����FALSE -- ��֧��ECC���� */
    VOS_UINT8                           ucEccSrvStatus;     /* ECC����״̬: TRUE -- ECC����򿪣�FALSE -- ECC����ر� */
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucReserved2;
} TAF_NVIM_ENCVOICE_ECC_SRV_CAP_INFO_STRU;


typedef struct
{
    VOS_UINT8                           ucVerNum;
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucReserved2;
    VOS_UINT8                           ucReserved3;
    VOS_UINT8                           aucPubKey[TAF_NVIM_KMC_MSG_MAX_KMC_PUB_KEY_LEN];
} TAF_NVIM_ENCVOICE_ECC_PUB_KEY_INFO_STRU;


typedef struct
{
    VOS_UINT16                          usYear;     /* Year [1980..2100) */
    VOS_UINT16                          usMonth;    /* Month of year [1..12] */
    VOS_UINT16                          usDay;      /* Day of month [1..31] */
    VOS_UINT16                          usHour;     /* Hour of day [0..23] */
    VOS_UINT16                          usMinute;   /* Minute of hour [0..59] */
    VOS_UINT16                          usSecond;   /* Second of minute [0..59] */
} TAF_NVIM_ENCVOICE_SECINFO_ERASE_SYSTIME_INFO_STRU;


typedef struct
{
    VOS_UINT16                          usYear;     /* Year [1980..2100) */
    VOS_UINT16                          usMonth;    /* Month of year [1..12] */
    VOS_UINT16                          usDay;      /* Day of month [1..31] */
    VOS_UINT16                          usHour;     /* Hour of day [0..23] */
    VOS_UINT16                          usMinute;   /* Minute of hour [0..59] */
    VOS_UINT16                          usSecond;   /* Second of minute [0..59] */
} TAF_NVIM_ENCVOICE_PASSWD_RESET_SYSTIME_INFO_STRU;


typedef struct
{
    VOS_UINT8                           ucDelayEncVoiceReqTimerLen;
    VOS_UINT8                           ucReserved1;
    VOS_UINT16                          usRemoteCtrlAnswerTimerLen;
} TAF_NVIM_ENCVOICE_TIMER_CFG_INFO_STRU;


typedef struct
{
    VOS_UINT32                          ulCallBackEnableFlg;
    VOS_UINT32                          ulCallBackModeTimerLen;
} TAF_NVIM_1X_CALLBACK_CFG_STRU;


typedef struct
{
    VOS_UINT8                           ucEnableFlag;                           /* VOS_TRUE:����Filter���˹���VOS_FALSE:�����ù��˹��� */
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucReserved2;
    VOS_UINT8                           ucReserved3;
}TAF_NVIM_NDIS_FILTER_ENABLE_FLAG_CFG_STRU;


typedef struct
{
    VOS_UINT8                           ucRedialInterTimerLen;   /* 1���ز��׶��У�ÿ���ز������ʱ��ʱ�� */
    VOS_UINT8                           ucRedialNum;             /* 1���ز��׶��У����Գ��Ե��ز����� */
    VOS_UINT8                           ucRsv1;
    VOS_UINT8                           ucRsv2;
} TAF_NVIM_1X_NW_NORMAL_REL_REDIAL_STRATEGY_STRU;


typedef struct
{
    VOS_UINT8                                               ucPhaseNum;    /* �ܽ׶θ���: Ĭ��4���׶Σ�4s��10s��15s��20s */
    VOS_UINT8                                               ucReserved1;
    VOS_UINT8                                               ucReserved2;
    VOS_UINT8                                               ucReserved3;
    TAF_NVIM_1X_NW_NORMAL_REL_REDIAL_STRATEGY_STRU          astRedialInfo[TAF_NVIM_MAX_1X_NW_NORMAL_REL_REDIAL_PHASE_NUM];
} TAF_NVIM_1X_NW_NORMAL_REL_REDIAL_STRATEGY_CFG_STRU;


typedef struct
{
    VOS_UINT8                           ucHrpdEnable;                    /* SRLTE: HRPD������ʹ���� */
    VOS_UINT8                           ucEhrpdEnable;                   /* SRLTE: EHRPD������ʹ���� */
    VOS_UINT8                           ucDoBackToLteTmrLen;             /* DO����ҵ��̬��ʱ������������inactive timer��ʱ����ʱ����
                                                                                   ��λΪ�룬Ĭ��55�� */
    VOS_UINT8                           ucMinPktCdataInactivityTmrLen;   /* CDMA���ݷ�������С�����ݴ���ʱ��, ��λ��, Ĭ��ֵ1s */

} TAF_NVIM_DO_BACK_TO_LTE_CFG_STRU;


typedef struct
{
    VOS_UINT8                                               ucIsCfgEnable;  /* VOS_TRUE: ʹ�� VOS_FALSE:��ʹ�� */
    TAF_XSMS_TL_ACK_NEED_LEVEL_ENUM_UINT8                   enTlAckNeedLevel;
    VOS_UINT8                                               ucMaxFlacQryTime; /* ������Դ��� Ĭ��ֵ : 2*/
    VOS_UINT8                                               ucFlacQryInterVal;/* ���Լ�� ��λ:�ٺ��� Ĭ���� 6 ��600ms */
}TAF_XSMS_MO_TL_ACK_TIME_OUT_CFG_STRU;


typedef struct
{
    VOS_UINT16                          usNumOfDigits;
    VOS_UINT8                           aucDigits[38]; /* DTMF���� */
}TAF_XSMS_KMC_DEF_ADDRESS_STRU;


typedef struct
{
    VOS_UINT16                          usTimes;
    VOS_UINT16                          usTimerLen;
}TAF_NVIM_CTCC_OOS_TIMER_CFG_STRU;


typedef struct
{
    TAF_NVIM_CTCC_OOS_TIMER_CFG_STRU    astClOosTimerCfg[TAF_NVIM_CTCC_OOS_TIMER_MAX_PHASE];
    TAF_NVIM_CTCC_OOS_TIMER_CFG_STRU    astGulOosTimerCfg[TAF_NVIM_CTCC_OOS_TIMER_MAX_PHASE];
}TAF_NVIM_CTCC_OOS_CONFIG_STRU;


typedef struct
{
    VOS_UINT8                           ucNeedGetDnsByDhcp;                     /* 1��Ҫ��0����Ҫ��Ĭ��Ϊ0 */

    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucReserved2;
    VOS_UINT8                           ucReserved3;

}TAF_NVIM_GET_DNS_THTOUGH_DHCP_CFG_STRU;



typedef struct
{
    VOS_UINT8                           ucEnable;                       /* 0:NV δ����; 1:NV ���� */
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucReserved2;
    VOS_UINT8                           ucReserved3;
    VOS_UINT32                          ulGutlTriggerLteLostTimerValue; /* GUTLģʽ�£�����4G�¼�������ʱ��ʱ������λ���� */
    VOS_UINT32                          ulClTriggerLteLostTimerValue;   /* CLģʽ�£�����4G�¼�������ʱ��ʱ������λ���� */
    VOS_UINT32                          ulRptLteLostTimerValue;         /* ����4Gʱ���ϱ���ʱ��ʱ��,��λ:�롣�״��ϱ�ʱ����һ�β���4G�¼��¼��������ϱ���������Ҫ�������ʱ������ϱ� */
    VOS_UINT32                          ulReserved1;
    VOS_UINT32                          ulReserved2;
    VOS_UINT32                          ulReserved3;
}TAF_NVIM_LTE_LOST_EVENT_STRU;



typedef struct
{
    VOS_UINT8                           ucXcposrRptNvCfg;                       /* �Ƿ��ϱ����GPS����ĸ�����λ��Ϣ */
    VOS_UINT8                           ucCposrDefault;
    VOS_UINT8                           ucXcposrDefault;
    VOS_UINT8                           ucXcposrRptDefault;
}TAF_NVIM_XCPOSRRPT_CFG_STRU;

typedef struct
{
    VOS_UINT8                           uc1xCsCallWaitRedirCmplTimerLen;         /* ��λ:�� �ض����º��н���ʧ�ܺ󣬾����´ν��������ʱ�� */
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucReserved2;
    VOS_UINT8                           ucReserved3;
}TAF_NVIM_1X_CS_CALL_REDIR_CMPL_DELAY_TIMER_INFO_STRU;


typedef struct
{
    VOS_UINT8                           ucLen;
    VOS_UINT8                           aucIccId[TAF_NVIM_ICC_ID_MAX];
}TAF_NVIM_ICC_ID_STRU;


typedef struct
{
    VOS_UINT8                           ucRpmCap;
    VOS_UINT8                           ucRpmEnableFlag;
    VOS_UINT8                           ucRpmVersion;
    VOS_UINT8                           ucReserved0;
    TAF_NVIM_ICC_ID_STRU                stUiccInfo;
    VOS_UINT8                           ucRpmParaN1;
    VOS_UINT8                           ucRpmParaT1;
    VOS_UINT8                           ucRpmParaF1;
    VOS_UINT8                           ucRpmParaF2;
    VOS_UINT8                           ucRpmParaF3;
    VOS_UINT8                           ucRpmParaF4;
    VOS_UINT8                           aucReserved1[TAF_NVIM_RPM_RSV_PARA_NUM];
}TAF_NVIM_RPM_CFG_STRU;


typedef struct
{
    VOS_UINT8                           ucTimersStatus;                         /* RPM��ʱ��N1��LR1��LR2����״̬ */
    VOS_UINT8                           ucApResetCounter;
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucReserved2;
    VOS_UINT32                          ulN1TimerStartRtc;
    VOS_UINT32                          ulLR1TimerStartRtc;
    VOS_UINT32                          ulLR2TimerStartRtc;
    VOS_UINT32                          ulLR3TimerStartRtc;
}TAF_NVIM_RPM_TIMER_INFO_STRU;


typedef struct
{
    VOS_UINT8                           bitOpRpmEnabledFlagFile     :1;         /* RPM��׮�ļ� EnabledFlag��־ */
    VOS_UINT8                           bitOpRpmParaFile            :1;         /* RPM��׮�ļ� Para��־ */
    VOS_UINT8                           bitOpRpmOmcLrFile           :1;         /* RPM��׮�ļ� OmcLr��־ */
    VOS_UINT8                           bitOpRpmOmcFile             :1;         /* RPM��׮�ļ� Omc��־ */
    VOS_UINT8                           bitSpare                    :4;

    VOS_UINT8                           ucStubEnable;                           /* RPM׮���ܿ��� */
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucImsiStubFlag;                         /* SIM��IMSI��׮��־ */
    VOS_UINT32                          ulRpmMcc;                               /* RPM׮MCC */
    VOS_UINT32                          ulRpmMnc;                               /* RPM׮MNC */

    /* RPM׮�ļ� Enabled Flag */
    VOS_UINT8                           ucRpmEnabledFlagFile;

    /* RPM׮�ļ� Parameters */
    VOS_UINT8                           ucRpmParaFileN1;
    VOS_UINT8                           ucRpmParaFileT1;
    VOS_UINT8                           ucRpmParaFileF1;
    VOS_UINT8                           ucRpmParaFileF2;
    VOS_UINT8                           ucRpmParaFileF3;
    VOS_UINT8                           ucRpmParaFileF4;

    /* RPM׮�ļ� Operational Management Counters Leak Rate */
    VOS_UINT8                           ucRpmOmcLrFileLr1;
    VOS_UINT8                           ucRpmOmcLrFileLr2;
    VOS_UINT8                           ucRpmOmcLrFileLr3;

    /* RPM׮�ļ� Operational Management Counters */
    VOS_UINT8                           ucRpmOmcFileCbr1;
    VOS_UINT8                           ucRpmOmcFileCr1;
    VOS_UINT8                           ucRpmOmcFileCPdp1;
    VOS_UINT8                           ucRpmOmcFileCPdp2;
    VOS_UINT8                           ucRpmOmcFileCPdp3;
    VOS_UINT8                           ucRpmOmcFileCPdp4;

}TAF_NVIM_RPM_STUB_STRU;



typedef struct
{
    VOS_UINT8                           ucUserRebootConfig;
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucReserved2;
    VOS_UINT8                           ucReserved3;
}TAF_NVIM_USER_REBOOT_SUPPORT_STRU;


typedef struct
{
    VOS_UINT8                           ucModemCtrlFlag;
    VOS_UINT8                           aucReserved[3];
}TAF_NVIM_CCTC_ROAM_CTRL_CFG_STRU;



typedef struct
{
    VOS_UINT8                           ucRatOrderNum;                                      /* syscfgex�����õ�acqoder�е�ָʾ���� */
    VOS_UINT8                           aenRatOrder[TAF_NVIM_MAX_USER_SYS_CFG_RAT_NUM];     /* syscfgex�����õ�acqoder���� */
    VOS_UINT8                           aucReserved[2];
}TAF_NVIM_MULTIMODE_RAT_CFG_STRU;


typedef struct
{
    VOS_UINT8                           ucAlloweModeSwitchFlg;
    VOS_UINT8                           aucReserved[3];
}TAF_NVIM_UNKONWN_DUAL_MODE_CARD_MODE_SWITCH_CFG_STRU;



typedef struct
{
    VOS_UINT32                                              ulEmcRetryPeriod;           /* ���������Զ�ʱ��ʱ�� */
    TAF_SPM_NVIM_MULTIMODE_EMC_CS_PRFER_TYPE_ENUM_UINT8     enMultiModeEmcCsPreferType; /* ȫģ����������������ѡΪcs preferʱ��3gpp��3gpp2 cs�����ȼ� */
    VOS_UINT8                                               ucCycleRetryFlag;           /* ѭ�����Ա�־ */
    VOS_UINT8                                               ucLteLimitServiceSelIms;    /* ������������SPMģ���������ѡPS����LTE���Ʒ�������IMS�� */
    VOS_UINT8                                               ucCycRetryIntervalPeriod;   /* �������л����ز����ʱ�� */
    VOS_UINT8                                               ucEnableFlg;                /* SPMģ��������ѡ��UE����������������Ƿ�ʹ�� */
    VOS_UINT8                                               ucSpmVoiceDomain;           /* SPMģ�����ͨ����ѡ��0:cs only 1:ps only 2:cs prefer 3:ps prefer */
    VOS_UINT8                                               ucSpmEmsDomain;             /* SPMģ���������ѡ��0:cs forced 1:cs prefer 2:ps prefer */
    VOS_UINT8                                               ucEmcCellularSrvAcqTiLen;   /* ������vowifiע����ʱcellularҵ�񲶻�ʱ��ʱ�� ��λ:s */
}TAF_NVIM_SPM_VOICE_DOMAIN_SEL_CFG_STRU;


typedef struct
{
    VOS_UINT8                           ucInternationalRoamEmcNotSelIms;
    VOS_UINT8                           usReserved1;
    VOS_UINT8                           usReserved2;
    VOS_UINT8                           usReserved3;
    VOS_UINT8                           usReserved4;
    VOS_UINT8                           usReserved5;
    VOS_UINT8                           usReserved6;
    VOS_UINT8                           usReserved7;
}TAF_NVIM_SPM_EMC_CUSTOM_CFG_STRU;



typedef struct
{
    VOS_UINT8                                               ucLteSmsEnable;         /* LTE����3GPP2�������Կ��� VOS_TURE:ʹ��,VOS_FALSE:��ֹ */
    TAF_LSMS_RESEND_FLAG_ENUM_UINT8                         enResendFlag;           /* �ط�ʹ�ܱ�־ */
    VOS_UINT8                                               ucResendMax;            /* ������������Ĭ�����1�� */
    VOS_UINT8                                               ucResendInterval;       /* ���·��͵ȴ�ʱ�䣬��λ: �룬Ĭ��30�� */
}TAF_NVIM_LTE_SMS_CFG_STRU;


typedef struct
{
    VOS_UINT16                          usGsmFreqWidth;                         /* GSM����  ��λ100KHZ */
    VOS_UINT16                          usWcdmaFreqWidth;                       /* WCDMA������λ100KHZ */
    VOS_UINT16                          usTdsFreqWidth;                         /* TDS����  ��λ100KHZ */
    VOS_UINT16                          usLteFreqWidth;                         /* LTE����  ��λ100KHZ */
    VOS_UINT16                          usCdmaFreqWidth;                        /* CDMA���� ��λ100KHZ */
    VOS_UINT16                          usReserved1;
} MTC_NVIM_FREQ_WIDTH_STRU;


typedef struct
{
    VOS_UINT16                          usIntrusionFreq;                        /* ����Ƶ��, ��λ100KHZ */
    VOS_UINT8                           ucIntrusionFreqWidth;                   /* ����Ƶ�ʴ���,��λ100KHZ */
    VOS_UINT8                           ucIntrusionFreqWeight;                  /* ����Ƶ��Ȩ��,��Χ0^10 */
}MTC_NVIM_INTRUSION_FREQ_LIST_STRU;


typedef struct
{
    VOS_UINT8                           ucIntrusionFreqNum;                                           /* ����Ƶ����Ŀ */
    VOS_UINT8                           aucReserved[3];
    MTC_NVIM_INTRUSION_FREQ_LIST_STRU   astIntrusionFreqList[MTC_INTRUSION_DEVICE_FREQ_LIST_MAX_NUM]; /* ����Ƶ�������б� */
}MTC_NVIM_DEVICE_FREQ_CASE_STRU;


typedef struct
{
    VOS_UINT8                           ucActiveFreqCaseNum;                                       /* ���������������Ƶ������Ŀ */
    VOS_UINT8                           ucDeviceID;                                                /* ����ID */
    VOS_UINT8                           aucReserved[2];
    MTC_NVIM_DEVICE_FREQ_CASE_STRU      astDeviceFreqCase[MTC_INTRUSION_DEVICE_FREQ_CASE_MAX_NUM]; /* ����Ƶ����������Ϣ */
}MTC_NVIM_DEVICE_FREQ_CFG_STRU;


typedef struct
{
    VOS_UINT8                           ucScellWeight;                           /* ����С��Ȩ�ط�ֵ,��Χ0^100 */
    VOS_UINT8                           ucNcellWeight;                           /* ����Ȩ�ط�ֵ,��Χ0~10 */
    VOS_UINT8                           aucReserved[6];
}MTC_NVIM_MODEM_FREQ_WEIGHT_STRU;


typedef struct
{
    VOS_UINT8                           ucActiveDeviceNum;                                    /* �������������Ŀ����Чֵ1~8,��Ϊ0��ʾ�����Բ�����������������漰�ϱ� */
    VOS_UINT8                           aucReserved[3];
    MTC_NVIM_FREQ_WIDTH_STRU            stFreqWidth;                                          /* GUTLC���뼼���Ĵ�����Ϣ */
    MTC_NVIM_DEVICE_FREQ_CFG_STRU       astDeviceFreqCfg[MTC_INTRUSION_DEVICE_GROUP_MAX_NUM]; /* 8�������ĸ���Ƶ��������Ϣ */
    MTC_NVIM_MODEM_FREQ_WEIGHT_STRU     stModemFreqWeightCfg;                                 /* ModemƵ�ʵļƷ�Ȩ�ع��� */
} MTC_NVIM_DYNAMIC_FM_INTRUSION_CTRL_CFG_STRU;


typedef struct
{
    VOS_UINT8                           ucEnable;                               /* ������ԭ��ֵ�ز����ܿ���, 1: �򿪣�0���ر� */
    VOS_UINT8                           aucReversed[3];                         /* ���ֽڶ��� */
} TAF_NV_PDP_REDIAL_FOR_NO_CAUSE_CFG_STRU;



typedef struct
{
    VOS_UINT8                           ucLguSupportConfig;                     /* 1:LGU���ƿ�����0:LGU���Ʋ�������Ĭ��Ϊ0 */
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucReserved2;
    VOS_UINT8                           ucReserved3;
    VOS_UINT8                           ucReserved4;
    VOS_UINT8                           ucReserved5;
    VOS_UINT8                           ucReserved6;
    VOS_UINT8                           ucReserved7;
    VOS_UINT8                           ucReserved8;
    VOS_UINT8                           ucReserved9;
    VOS_UINT8                           ucReserved10;
    VOS_UINT8                           ucReserved11;
}TAF_NVIM_LGU_SUPPORT_CFG_STRU;



typedef struct
{
    VOS_UINT8                                               ucCsimPlmnNum;
    VOS_UINT8                                               ucUsimPlmnNum;
    VOS_UINT8                                               aucReserved[2];
    VOS_UINT32                                              aulCsimPlmn[TAF_NVIM_MAX_CTCC_CSIM_PLMN_NUM];
    VOS_UINT32                                              aulUsimPlmn[TAF_NVIM_MAX_CTCC_USIM_PLMN_NUM];
}TAF_MMA_NVIM_CTCC_DUAL_MODE_IMSI_PLMN_INFO_STRU;


typedef struct
{
    /* 0: R12; 1: R13 */
    VOS_UINT32                          bitOpQosRespTime            : 2;
    VOS_UINT32                          bitOpLocInfoTypeRespTime    : 2;
    VOS_UINT32                          bitOpGnssTodS               : 2;
    VOS_UINT32                          bitOpReserved0              : 2;
    VOS_UINT32                          bitOpReserved1              : 2;
    VOS_UINT32                          bitOpReserved2              : 2;
    VOS_UINT32                          bitOpReserved3              : 2;
    VOS_UINT32                          bitOpReserved4              : 2;
    VOS_UINT32                          bitOpReserved5              : 2;
    VOS_UINT32                          bitOpReserved6              : 2;
    VOS_UINT32                          bitOpReserved7              : 2;
    VOS_UINT32                          bitOpReserved8              : 2;
    VOS_UINT32                          bitOpReserved9              : 2;
    VOS_UINT32                          bitOpReserved10             : 2;
    VOS_UINT32                          bitOpReserved11             : 2;
    VOS_UINT32                          bitOpReserved12             : 2;
    VOS_UINT32                          bitOpReserved13             : 2;
    VOS_UINT32                          bitOpReserved14             : 2;
    VOS_UINT32                          bitOpReserved15             : 2;
    VOS_UINT32                          bitOpReserved16             : 2;
    VOS_UINT32                          bitOpReserved17             : 2;
    VOS_UINT32                          bitOpReserved18             : 2;
    VOS_UINT32                          bitOpReserved19             : 2;
    VOS_UINT32                          bitOpReserved20             : 2;
    VOS_UINT32                          bitOpReserved21             : 2;
    VOS_UINT32                          bitOpReserved22             : 2;
    VOS_UINT32                          bitOpReserved23             : 2;
    VOS_UINT32                          bitOpReserved24             : 2;
    VOS_UINT32                          bitOpReserved25             : 2;
    VOS_UINT32                          bitOpReserved26             : 2;
    VOS_UINT32                          bitOpReserved27             : 2;
    VOS_UINT32                          bitOpReserved28             : 2;
}TAF_MTA_NVIM_CPOS_PROTOCOL_VERSION_CFG_STRU;


typedef struct
{
    VOS_UINT8                           ucReadBackoffFileConfig;                /* 1:������ȡBACKOFF�㷨��ز����ļ���0:��������Ĭ��Ϊ0����NV��2379������رյ�����²���Ч */
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucReserved2;
    VOS_UINT8                           ucReserved3;
}TAF_NVIM_READ_BACKOFF_FILE_CFG_STRU;


typedef struct
{
    VOS_UINT8                           ucPowerSaveEnableFlag;             /* 0:NV δ����; 1:NV ���� */
    VOS_UINT8                           ucScreenOnEnableFlag;              /* 0:����ʱֱ���ϱ�;1:����ʱҲҪ���ʿ��� */
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucReserved2;
    VOS_UINT32                          ulGutlOosRptTimerInterval;         /* GUTLģʽ�£������ã���λ���� */
    VOS_UINT32                          ulGutlOosCsUserSenseTimerInterval; /* ��¼CS�ĵ����೤ʱ������ǳ�ʱ�䶪������λ���� */
    VOS_UINT32                          ulGutlOosPsUserSenseTimerInterval; /* ��¼PS�ĵ����೤ʱ������ǳ�ʱ�䶪������λ���� */
    VOS_UINT32                          ul1xOosRptTimerInterval;           /* CLģʽ�£�1x�����ϱ�ʱ��������λ:��*/
    VOS_UINT32                          ulDoLteOosRptTimerInterval;        /* CLģʽ�£�DO_LTE�����ϱ�ʱ��������λ:��*/
    VOS_UINT32                          ulReserved3;                       /* ����λ */
    VOS_UINT32                          ulReserved4;                       /* ����λ */
}TAF_NVIM_OOS_CHR_POWER_SAVE_CFG_STRU;


typedef struct
{
    VOS_UINT8                           ucEnableFlg;                            /* CDMAMODEMSWITCH����������NV����, 1: �򿪣�0���ر� */
    VOS_UINT8                           aucReversed[7];                         /* ���ֽڶ��� */
} TAF_NVIM_CDMAMODEMSWITCH_NOT_RESET_CFG_STRU;


typedef struct
{
    VOS_UINT16                         usSidRangeStart;                                      /*SID ��Χ��ʼֵ */
    VOS_UINT16                         usSidRangeEnd;                                        /*SID ��Χ����ֵ */
    VOS_UINT32                         ulMcc;                                                /*mobile country code */
}TAF_NVIM_SID_RANGE_AND_MCC_INFO_STRU;


typedef struct
{
    VOS_UINT8                                               ucActiveFlg;
    VOS_UINT8                                               ucHomeNetworkNum;               /* �й�home������Ŀ */
    VOS_UINT8                                               ucRsved1;
    VOS_UINT8                                               ucRsved2;
    TAF_NVIM_SID_RANGE_AND_MCC_INFO_STRU                    astSidRangeMccInfo[TAF_NVIM_CHINA_HOME_NETWORK_NUM_MAX];
}TAF_NVIM_CHINA_HOME_SID_RANGE_AND_MCC_INFO_STRU;


typedef struct
{
    VOS_UINT8                           ucHangupImsCallInCsRedialCfg;          /* �Ƿ�Ҷ�ims�������ͨ����־ */

    VOS_UINT8                           ucRedialImsToCsHifiStartedSupportFlg;  /* HIFI�Ѿ��������Ƿ���Ҫ����CS��ʶ */
    VOS_UINT8                           ucImsEmcFirstTryFlgVolteOff;
    VOS_UINT8                           ucReserv1;
    VOS_UINT8                           ucReserv2;
    VOS_UINT8                           ucReserv3;

    VOS_UINT8                           ucReserv4;
    VOS_UINT8                           ucReserv5;
}TAF_NVIM_SPM_IMSCALL_TO_CSREDIAL_CUSTOM_CFG_STRU;


typedef struct
{
    VOS_UINT8                           ucMatchRegApnConfig;                    /* 1:�����Ƚ��û������APN��ע��LTEʱ�����APN��0:������ */
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucReserved2;
    VOS_UINT8                           ucReserved3;
}TAF_NVIM_MATCH_REGISTER_APN_CFG_STRU;

typedef struct
{
    VOS_UINT8                           ucLimitedServiceReportFlg;                  /* �Ƿ����ر����Ʒ���״̬�ϱ���NV */
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucReserved2;
    VOS_UINT8                           ucReserved3;
}TAF_NVIM_LIMITED_SERVICE_STATUS_REPORT_CFG_STRU;


typedef struct
{
    VOS_UINT8                           ucModemCtrlFlg;                  /* DSDS��Modem���ƿ��� */
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucReserved2;
    VOS_UINT8                           ucReserved3;
}TAF_NVIM_DSDS_CTRL_CFG_STRU;



typedef struct
{
    VOS_UINT8                           ucAlertingSrvccOpenCodecFlag;           /* ֮ǰ�Ǳ������壬Alerting Srvcc��ҲҪ��Codec */
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucReserved2;
    VOS_UINT8                           ucReserved3;
}TAF_NVIM_ALERTING_SRVCC_OPEN_CODEC_CFG_STRU;


typedef struct
{
    VOS_UINT8                           ucAllowDeactEmcPdnFlg;
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucReserved2;
    VOS_UINT8                           ucReserved3;
} TAF_NVIM_DEACT_EMC_PDN_POLICY_STRU;

/*****************************************************************************
 �ṹ��    : nv_wifi_info
 �ṹ˵��  : nv_wifi_info�ṹ
*****************************************************************************/
typedef struct
{
    VOS_UINT32 ulOpSupport;
    VOS_UINT8  usbHighChannel;
    VOS_UINT8  usbLowChannel;
    VOS_UINT8  aucRsv1[2];
    VOS_UINT16 ausbPower[2];
    VOS_UINT8  usgHighsChannel;
    VOS_UINT8  usgLowChannel;
    VOS_UINT8  aucRsv2[2];
    VOS_UINT16 ausgPower[2];
    VOS_UINT8  usnHighsChannel;
    VOS_UINT8  usnLowChannel;
    VOS_UINT8  ausnRsv3[2];
    VOS_UINT16 ausnPower[2];
}nv_wifi_info;

/*****************************************************************************
 �ṹ��    : USB_ENUM_STATUS_STRU
 �ṹ˵��  : USB_ENUM_STATUS�ṹ
*****************************************************************************/
typedef struct
{
    VOS_UINT32    status;
    VOS_UINT32    value;
    VOS_UINT32    reserve1;
    VOS_UINT32    reserve2;
}USB_ENUM_STATUS_STRU;

/*****************************************************************************
 �ṹ��    : VBAT_CALIBART_TYPE
 �ṹ˵��  : ���У׼�������ݽṹ
*****************************************************************************/
typedef struct
{
    VOS_UINT16 min_value;
    VOS_UINT16 max_value;
}VBAT_CALIBART_TYPE;


typedef struct
{
    VOS_UINT8                           ucIms2CsRedialCauseNum;
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucReserved2;
    VOS_UINT8                           ucReserved3;
    VOS_UINT16                          ausIms2CsRedialCause[TAF_NV_SSA_IMS2CS_REDIAL_CAUSE_MAX_NUM];
}TAF_NV_SS_IMS2CS_REDIAL_CFG_STRU;

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of NasNvInterface.h */
