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

#ifndef __NASNVINTERFACE_H__
#define __NASNVINTERFACE_H__

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

#include "PsTypeDef.h"

/*****************************************************************************
  2 Macro
*****************************************************************************/
#define NAS_MMC_NV_ITEM_ACTIVE          (1)                                     /* NV��� */
#define NAS_MMC_NV_ITEM_DEACTIVE        (0)                                     /* NV��δ���� */

/* GPRS GEA �㷨֧�ֺ궨�� */
#define NAS_MMC_GPRS_GEA1_SUPPORT       (0x01)                                  /* ֧��GPRS GEA1�㷨 */
#define NAS_MMC_GPRS_GEA1_VALUE         (0x80)                                  /* ֧��GEA1ʱ����������ֵ */
#define NAS_MMC_GPRS_GEA2_VALUE         (0x40)                                  /* ֧��GEA2ʱ����������ֵ */
#define NAS_MMC_GPRS_GEA3_VALUE         (0x20)                                  /* ֧��GEA3ʱ����������ֵ */
#define NAS_MMC_GPRS_GEA4_VALUE         (0x10)                                  /* ֧��GEA4ʱ����������ֵ */
#define NAS_MMC_GPRS_GEA5_VALUE         (0x08)                                  /* ֧��GEA5ʱ����������ֵ */
#define NAS_MMC_GPRS_GEA6_VALUE         (0x04)                                  /* ֧��GEA6ʱ����������ֵ */
#define NAS_MMC_GPRS_GEA7_VALUE         (0x02)                                  /* ֧��GEA7ʱ����������ֵ */

#define NAS_MMC_GPRS_GEA2_SUPPORT       (0x02)                                  /* ֧��GPRS GEA2�㷨 */
#define NAS_MMC_GPRS_GEA3_SUPPORT       (0x04)                                  /* ֧��GPRS GEA3�㷨 */
#define NAS_MMC_GPRS_GEA4_SUPPORT       (0x08)                                  /* ֧��GPRS GEA4�㷨 */
#define NAS_MMC_GPRS_GEA5_SUPPORT       (0x10)                                  /* ֧��GPRS GEA5�㷨 */
#define NAS_MMC_GPRS_GEA6_SUPPORT       (0x20)                                  /* ֧��GPRS GEA6�㷨 */
#define NAS_MMC_GPRS_GEA7_SUPPORT       (0x40)                                  /* ֧��GPRS GEA7�㷨 */
#define NAS_MMC_NVIM_MAX_EPLMN_NUM      (16)                                    /* en_NV_Item_EquivalentPlmn NV�е�Чplmn���� */
#define NAS_MMC_NVIM_MAX_MCC_SIZE       (3)                                     /* plmn��Mcc��󳤶� */
#define NAS_MMC_NVIM_MAX_MNC_SIZE       (3)                                     /* plmn��Mnc��󳤶� */
#define NAS_MMC_LOW_BYTE_MASK           (0x0f)

#define NAS_MMC_A5_1_SUPPORT            (0x0001)                                  /* ֧��A5-1�㷨 */
#define NAS_MMC_A5_2_SUPPORT            (0x0002)                                  /* ֧��A5-2�㷨 */
#define NAS_MMC_A5_3_SUPPORT            (0x0004)                                  /* ֧��A5-3�㷨 */
#define NAS_MMC_A5_4_SUPPORT            (0x0008)                                  /* ֧��A5-4�㷨 */
#define NAS_MMC_A5_5_SUPPORT            (0x0010)                                  /* ֧��A5-5�㷨 */
#define NAS_MMC_A5_6_SUPPORT            (0x0020)                                  /* ֧��A5-6�㷨 */
#define NAS_MMC_A5_7_SUPPORT            (0x0040)                                  /* ֧��A5-7�㷨 */

#define NAS_MMC_NVIM_MAX_USER_CFG_IMSI_PLMN_NUM                  (6)                 /* �û����õ�����֧�ֵ�USIM/SIM���ĸ��� */
#define NAS_MMC_NVIM_MAX_USER_CFG_EHPLMN_NUM                     (6)                 /* �û����õ�EHplmn�ĸ��� */
#define NAS_MMC_MAX_BLACK_LOCK_PLMN_WITH_RAT_NUM            (8)                 /* ��ֹ���뼼����PLMN ID�������� */
#define NAS_MMC_NVIM_MAX_USER_CFG_EXT_EHPLMN_NUM              (8)                /* ��չ��NV���EHplmn���������*/
#define NAS_MMC_NVIM_MAX_USER_CFG_FORB_PLMN_GROUP_NUM         (8)                /* �û����õ�forb plmn��������� */
#define NAS_NVIM_MAX_REJ_INFO_NOT_RPT_GROUP_NUM               (8)             /* rejInfo���ϱ������� */

#define NAS_MMC_NVIM_MAX_USER_OPLMN_VERSION_LEN               (8)               /* �û����õ�OPLMN�汾����󳤶� */
#define NAS_MMC_NVIM_MAX_USER_OPLMN_IMSI_NUM                  (6)               /* �û����õ�OPLMN����֧�ֵ�USIM/SIM���ĸ��� */
#define NAS_MMC_NVIM_MAX_USER_CFG_OPLMN_NUM                   (256)             /* �û����õ�OPLMN�������� */
#define NAS_MMC_NVIM_OPLMN_WITH_RAT_UNIT_LEN                  (5)               /* �û����õĴ����뼼��OPLMN������Ԫ���ȣ���6F61�ļ��Ļ������ȵ�ԪΪ5 */
#define NAS_MMC_NVIM_MAX_USER_CFG_OPLMN_DATA_LEN              (500)             /* �û�����OPLMN������ֽ���,����ǰֻ֧��500*/
#define NAS_MMC_NVIM_MAX_USER_CFG_OPLMN_DATA_EXTEND_LEN       (1280)            /* ��չ����û�����OPLMN������ֽ���*/


/* ����:��128��256; ����Ԥ��������Ϣ,��6��7*/
#define NAS_MMC_NVIM_MAX_CFG_DPLMN_DATA_LEN             (7*256)            /* ��չ����û�����DPLMN������ֽ��� */
#define NAS_MMC_NVIM_MAX_CFG_NPLMN_DATA_LEN             (7*256)            /* ��չ����û�����NPLMN������ֽ��� */

#define NAS_MMC_NVIM_MAX_CFG_DPLMN_DATA_EXTEND_LEN       (6*128)            /* ��չ����û�����DPLMN������ֽ���*/
#define NAS_MMC_NVIM_MAX_CFG_NPLMN_DATA_EXTEND_LEN       (6*128)            /* ��չ����û�����NPLMN������ֽ���*/
#define NAS_MMC_NVIM_MAX_CFG_HPLMN_NUM                   (3*8)
#define NAS_MCC_NVIM_VERSION_LEN                         (9)                /* �汾�ţ��̶�Ϊxx.xx.xxx */

#define NAS_MMC_NVIM_MAX_CFG_DPLMN_NUM                   (256)            /* ��չ����û�����DPLMN���� */
#define NAS_MMC_NVIM_MAX_CFG_NPLMN_NUM                   (256)            /* ��չ����û�����NPLMN���� */
/* �߾���Ϣ����,ÿ7���ֽڴ���һ���߾���Ϣ����1-3���ֽ�Ϊsim����ʽplmn id��
   ��4-5�ֽ�Ϊ֧�ֵĽ��뼼��(0x8000Ϊ֧��w��0x4000Ϊ֧��lte��0x0080Ϊ֧��gsm)����6-7�ֽ�ΪLAC��TAC��Ϣ
   ���뼼��ΪWGʱ��6-7�ֽڱ�ʾLAC;���뼼��ΪLTEʱ��6-7�ֽڱ�ʾTAC */
#define NAS_MMC_EVERY_BORDER_INFO_LEN                    (7)              /* en_NV_Item_Ap_Preset_Border_Info nv��ÿ���߾���Ϣռ���ֽ��� */
#define NAS_MMC_NVIM_MAX_CFG_BORDER_DATA_LEN             (7*256)          /* APԤ�ñ߾���Ϣ��������ݳ��ȣ�ÿ���߾���Ϣռ��7�ֽڣ����Ԥ��256���߾���Ϣ */
#define NAS_NVIM_MAX_OPER_SHORT_NAME_LEN                36
#define NAS_NVIM_MAX_OPER_LONG_NAME_LEN                 40

#define NAS_NVIM_ITEM_MAX_IMSI_LEN          (9)                     /* ����IMSI�ĳ��� */
#define NAS_MML_NVIM_PS_LOCI_SIM_FILE_LEN                    (14)                    /* USIM��PSLOCI�ļ��ĳ��� */
#define NAS_NVIM_PS_LOCI_SIM_FILE_LEN                    (14)                    /* USIM��PSLOCI�ļ��ĳ��� */

#define NAS_UTRANCTRL_MAX_NVIM_CFG_TD_MCC_LIST_NUM          (6)                 /* ģʽ�Զ��л������Ŀ����ù�������� */
#define NAS_UTRANCTRL_MAX_NVIM_CFG_IMSI_PLMN_LIST_NUM       (6)                 /* ģʽ�Զ��л������Ŀ�����USIM/IMSI������Ÿ��� */

#define NAS_MML_NVIM_MAX_DISABLED_RAT_PLMN_NUM              (8)                 /* ��ֹ���뼼����PLMN ID�������� */

#define NAS_MML_NVIM_MAX_SKIP_BAND_TYPE_SEARCH_MCC_NUM      (8)

#define NAS_MML_MAX_EXTENDED_FORB_PLMN_NUM                  (32)

#define NAS_MML_NVIM_FRAT_MAX_IMSI_NUM                       (6)                 /* FRAT�������֧��IMSI���� */
#define NAS_MML_NVIM_FRAT_MAX_PLMN_ID_NUM                    (40)                /* FRAT�������֧��PLMN���� */

#define NAS_SIM_FORMAT_PLMN_LEN                     (3)                     /* Sim����ʽ��Plmn���� */

#define NAS_MML_NVIM_MAX_BLACK_LOCK_PLMN_NUM                 (16)                    /* ����������֧�ֵ�PLMN ID�������� */

#define NAS_MML_NVIM_MAX_WHITE_LOCK_PLMN_NUM                 (16)                    /* ����������֧�ֵ�PLMN ID�������� */

#define NAS_MML_BG_SEARCH_REGARDLESS_MCC_NUMBER         (10)                     /* BG�Ѳ����ǹ�����Ĺ����������� */
#define NAS_MML_SINGLE_DOMAIN_FAIL_ACTION_MAX_LIST      (5)                     /* ����ԭ��ֵ����б� */

#define NV_ITEM_NET_CAPABILITY_MAX_SIZE                     (10)

#define NAS_MMC_NVIM_SUPPORTED_3GPP_RELEASE_SIZE            (2)                 /* ��ǰ֧�ֵ�Э��汾 */

#define NAS_MMC_NVIM_MAX_IMSI_LEN                           (9)                     /* ����IMSI�ĳ��� */

#define NVIM_MAX_EPLMN_NUM                                  (16)
#define NVIM_MAX_MCC_SIZE                                   (3)
#define NVIM_MAX_MNC_SIZE                                   (3)

#define NVIM_MAX_FDD_FREQ_BANDS_NUM                         (12)

/* Add by z60575 for multi_ssid, 2012-9-5 end */
/*+CGMI - ��ȡ����������*/

#define NAS_NVIM_MAX_IMSI_FORBIDDEN_LIST_NUM         (16)
#define NAS_NVIM_MAX_RAT_FORBIDDEN_LIST_NUM          (8)    /* Ԥ����չ */
#define NAS_NVIM_MAX_SUPPORTED_FORBIDDEN_RAT_NUM     (2)

#define NAS_NVIM_FORBIDDEN_RAT_NUM_0                 (0)
#define NAS_NVIM_FORBIDDEN_RAT_NUM_1                 (1)
#define NAS_NVIM_FORBIDDEN_RAT_NUM_2                 (2)


#define NAS_NVIM_MAX_REJECT_NO_RETRY_CAUSE_NUM              (8)

#define NAS_NVIM_MAX_LAU_REJ_TRIG_PLMN_SEARCH_CAUSE_NUM     (12)

/* ��NVIDö�ٵ�ת����(PS_NV_ID_ENUM, SYS_NV_ID_ENUM, RF_NV_ID_ENUM) */
typedef VOS_UINT16  NV_ID_ENUM_U16;
#define NV_ITEM_IMEI_SIZE                      16
#define NV_ITEM_MMA_OPERATORNAME_SIZE          84

#define NV_ITEM_OPER_NAME_LONG          (40)
#define NV_ITEM_OPER_NAME_SHORT         (36)
#define NV_ITEM_PLMN_ID_LEN             (8)

#define NV_ITEM_AT_PARA_SIZE                   100
#define NV_ITEM_HPLMN_FIRST_SEARCH_SIZE        1  /* ��һ��HPLMN������ʱ���� */
#define NVIM_ITEM_MAX_IMSI_LEN          (9)                     /* ����IMSI�ĳ��� */

#define CNAS_NVIM_MAX_1X_MRU_SYS_NUM                            (12)

#define NAS_MMC_NVIM_MAX_CAUSE_NUM      (10)     /* NV����ԭ��ֵ������ */

#define CNAS_NVIM_PRL_SIZE                                      (4096) /* PRL NV size: 4K byte */

#define CNAS_NVIM_CBT_PRL_SIZE                                  (48) /* PRL NV size: 45 bytes */

#define CNAS_NVIM_MAX_1X_BANDCLASS_NUM                          (32)

#define CNAS_NVIM_MAX_1X_HOME_SID_NID_NUM                       (20)
#define CNAS_NVIM_MAX_OOC_SCHEDULE_PHASE_NUM                    (8)

#define CNAS_NVIM_MAX_1X_OOC_SCHEDULE_PHASE_NUM                    (8)

#define CNAS_NVIM_MAX_HRPD_MRU_SYS_NUM                           (12)
#define CNAS_NVIM_HRPD_SUBNET_LEN                                (16)

#define CNAS_NVIM_MAX_STORAGE_BLOB_LEN                      ( 255 )

#define NAS_NVIM_BYTES_IN_SUBNET                            (16)
#define NAS_NVIM_MAX_RAT_NUM                                (7)                 /* Ԥ����1X��HRPD */

#define NAS_MSCC_NVIM_MLPL_SIZE                                      (1024) /* PRL NV size: 1K byte */
#define NAS_MSCC_NVIM_MSPL_SIZE                                      (1024) /* PRL NV size: 1K byte */

#define NAS_NV_TRI_MODE_CHAN_PARA_PROFILE_NUM      ( 8 )                       /*  ����ȫ��ͨ���ص�ͨ�����ó�����Ŀ */

#define CNAS_NVIM_ICCID_OCTET_LEN                            (10)
#define CNAS_NVIM_MEID_OCTET_NUM                             (7)
#define CNAS_NVIM_UATI_OCTET_LENGTH                          (16)

#define CNAS_NVIM_MAX_WHITE_LOCK_SYS_NUM                     (20)

#define CNAS_NVIM_MAX_HRPD_CUSTOMIZE_FREQ_NUM                (10)

#define CNAS_NVIM_MAX_CDMA_1X_CUSTOM_PREF_CHANNELS_NUM              (10)
#define CNAS_NVIM_MAX_CDMA_1X_CUSTOMIZE_PREF_CHANNELS_NUM           (20)

/* �����ȼ�PLMN refresh ����������Ĭ���ӳ�ʱ��: ��λ �� */
#define NV_ITEM_HIGH_PRIO_PLMN_REFRESH_TRIGGER_BG_SEARCH_DEFAULT_DELAY_LEN    (5)

#define NAS_NVIM_MAX_PLMN_CSG_ID_NUM             (35)
#define NAS_NVIM_MAX_CSG_REJ_CAUSE_NUM           (10)

#define NAS_NVIM_LTE_OOS_2G_PREF_PLMN_SEL_MAX_IMSI_LIST_NUM      (16)    /* SIM���б� (LTE OOS������2G����3G) */

#define NAS_MML_NVIM_MAX_REG_FAIL_CAUSE_NUM        (16)        /*֧�ֵĽ�ֹLA�б��С*/
#define CNAS_NVIM_MAX_AUTHDATA_USERNAME_LEN                      (253)   /* HRPD ppp AN��Ȩ�û�����û�����
                                                                            �ο�C.S0023-D section 3.4.53�Լ�
                                                                            C.S0016-D,section 3.5.8.13,
                                                                            �ó��������Ϊ255-2(NAI������
                                                                            ����Ȩ�㷨�����ֽ�ռ�õ��ֽ�) */

#define CNAS_NVIM_1X_MAX_MRU_SYS_NUM                   (12)

#define CNAS_NVIM_1X_AVOID_MAX_PHASE_NUM               (8)
#define CNAS_NVIM_1X_AVOID_REASON_MAX                  (20)

#define CNAS_NVIM_MAX_HRPD_OOC_SCHEDULE_PHASE_NUM                    (8)

#define CNAS_NVIM_HRPD_AVOID_MAX_PHASE_NUM               (8)
#define CNAS_NVIM_HRPD_AVOID_REASON_MAX                  (16)

#define CNAS_NVIM_HOME_MCC_MAX_NUM                      (5)

#define NAS_NVIM_MAX_BSR_PHASE_NUM                      (2)

#define NAS_MMC_NVIM_MAX_EXTENDED_FORB_PLMN_NUM         (32)
#define NAS_NVIM_MAX_LIMIT_PDP_ACT_PLMN_NUM             (8)
#define NAS_NVIM_MAX_LIMIT_PDP_ACT_CAUSE_NUM            (8)

#define NAS_MAX_TMSI_LEN                                (4)                     /* ����TMSI�ĳ��� */
#define NAS_SIM_MAX_LAI_LEN                             (6)                     /* SIM���б����LAI��󳤶� */
#define NAS_MMC_NVIM_MAX_CUSTOM_SUPPLEMENT_OPLMN_NUM    (16)

#define CNAS_NVIM_MAX_CDMA_HRPD_CUSTOMIZE_PREF_CHANNELS_NUM           (20)

#define NAS_NVIM_MODE_SELECTION_RETRY_TIMER_PHASE_NUM_MAX             (10)

#define NAS_NVIM_MODE_SELECTION_PUNISH_TIMER_PHASE_NUM_MAX            (10)

#define NAS_NVIM_MODE_SELECTION_RETRY_SYS_ACQ_PHASE_NUM_MAX           (4)

#define NAS_NVIM_CUSTOMIZE_MAX_CL_ACQ_SCENE_NUM                       (40)

#define NAS_NVIM_CHINA_BOUNDARY_NETWORK_NUM_MAX             (30)

#define NAS_NVIM_CHINA_HOME_NETWORK_NUM_MAX                 (5)

#define NAS_NVIM_1X_MT_EST_CNF_REEST_CAUSE_MAX_NUM          (10)
#define NAS_NVIM_1X_MT_TERMINATE_IND_REEST_CAUSE_MAX_NUM    (9)
/*****************************************************************************
  3 Massage Declare
*****************************************************************************/


/*****************************************************************************
  4 Enum
*****************************************************************************/

enum NAS_MMC_NVIM_SINGLE_DOMAIN_REG_FAIL_ACTION_ENUM
{
    NAS_MMC_NVIM_SINGLE_DOMAIN_REG_FAIL_ACTION_PLMN_SELECTION                    = 0,            /* �������� */
    NAS_MMC_NVIM_SINGLE_DOMAIN_REG_FAIL_ACTION_NORMAL_CAMP_ON                    = 1,            /* ����פ�� */
    NAS_MMC_NVIM_SINGLE_DOMAIN_REG_FAIL_ACTION_OPTIONAL_PLMN_SELECTION           = 2,            /* ������ѡ���� */
    NAS_MMC_NVIM_SINGLE_DOMAIN_REG_FAIL_ACTION_LIMITED_CAMP_ON                   = 3,            /* ����פ�� */

    NAS_MMC_NVIM_SINGLE_DOMAIN_ROAMING_REG_FAIL_ACTION_PLMN_SELECTION            = 4,            /* ������������ע�ᷢ����������HOME�����ϲ���Ч */

    NAS_MMC_NVIM_SINGLE_DOMAIN_REG_FAIL_ACTION_BUTT
};
typedef VOS_UINT8 NAS_MMC_NVIM_SINGLE_DOMAIN_REG_FAIL_ACTION_ENUM_UINT8;


enum NAS_MMC_NVIM_REG_FAIL_CAUSE_ENUM
{
    NAS_MMC_NVIM_REG_FAIL_CAUSE_GPRS_SERV_NOT_ALLOW_IN_PLMN = 14,
    NAS_MMC_NVIM_REG_FAIL_CAUSE_TIMER_TIMEOUT               = 301,                                 /* �ȴ���������ʱ����ʱ */
    NAS_MMC_NVIM_REG_FAIL_CAUSE_BUTT
};
typedef VOS_UINT16 NAS_MMC_NVIM_REG_FAIL_CAUSE_ENUM_UINT16;


enum NAS_MMC_NVIM_REG_DOMAIN_ENUM
{
    NAS_MMC_NVIM_REG_DOMAIN_CS = 1,
    NAS_MMC_NVIM_REG_DOMAIN_PS = 2,                                 /* �ȴ���������ʱ����ʱ */
    NAS_MMC_NVIM_REG_DOMAIN_BUTT
};
typedef VOS_UINT8 NAS_MMC_NVIM_REG_DOMAIN_ENUM_UINT8;



enum NAS_MMC_UCS2_ENUM
{
    NAS_MMC_UCS2_HAS_PREFER                                = 0,
    NAS_MMC_UCS2_NO_PREFER                                 = 1,

    NAS_MMC_UCS2_BUTT
};

typedef VOS_UINT16 NAS_MMC_UCS2_ENUM_UINT16;


enum NV_MS_MODE_ENUM
{
    NV_MS_MODE_CS_ONLY,                                                 /* ��֧��CS�� */
    NV_MS_MODE_PS_ONLY,                                                 /* ��֧��PS�� */
    NV_MS_MODE_CS_PS,                                                   /* CS��PS��֧�� */

    NV_MS_MODE_ANY,                                                     /* ANY,�൱�ڽ�֧��CS�� */

    NV_MS_MODE_BUTT
};
typedef VOS_UINT8 NV_MS_MODE_ENUM_UINT8;


enum NAS_NV_LTE_CS_SERVICE_CFG_ENUM
{
    NAS_NV_LTE_SUPPORT_CSFB_AND_SMS_OVER_SGS = 1,           /* ֧��cs fallback��sms over sgs*/
    NAS_NV_LTE_SUPPORT_SMS_OVER_SGS_ONLY,                   /* ֧��sms over sgs only*/
    NAS_NV_LTE_SUPPORT_1XCSFB,                              /* ֧��1XCSFB */
    NAS_NV_LTE_SUPPORT_BUTT
};
typedef VOS_UINT8 NAS_NV_LTE_CS_SERVICE_CFG_ENUM_UINT8;



enum NAS_NVIM_CHANGE_REG_REJ_CAUSE_TYPE_ENUM
{
    NAS_NVIM_CHANGE_REG_REJ_CAUSE_TYPE_INACTIVE     = 0,        /* ���ܲ���Ч */
    NAS_NVIM_CHANGE_REG_REJ_CAUSE_TYPE_CS_PS,                   /* �޸�CS+PS�ľܾ�ԭ��ֵ */
    NAS_NVIM_CHANGE_REG_REJ_CAUSE_TYPE_CS_ONLY,                 /* ���޸�CS��ľܾ�ԭ��ֵ */
    NAS_NVIM_CHANGE_REG_REJ_CAUSE_TYPE_PS_ONLY,                 /* ���޸�PS��ľܾ�ԭ��ֵ */
    NAS_NVIM_CHANGE_REG_REJ_CAUSE_TYPE_BUTT
};
typedef VOS_UINT8 NAS_NVIM_CHANGE_REG_REJ_CAUSE_TYPE_ENUM_UINT8;


enum NAS_NVIM_CHAN_REPEAT_SCAN
{
    NAS_NVIM_CHAN_SCAN_NORMAL,
    NAS_NVIM_CHAN_REPEAT_SCAN_REACQ_0_1_2_3_4S,
    NAS_NVIM_CHAN_REPEAT_SCAN_PING_5,
    NAS_NVIM_CHAN_REPEAT_SCAN_2_7,

    NAS_NVIM_CHAN_REPEAT_SCAN_BUTT
};
typedef VOS_UINT8 NAS_NVIM_CHAN_REPEAT_SCAN_ENUM_UINT8;


enum NAS_NVIM_EPDSZID_SUPPORT_TYPE_ENUM
{
    NAS_NVIM_EPDSZID_SUPPORT_TYPE_PDSZID,
    NAS_NVIM_EPDSZID_SUPPORT_TYPE_PDSZID_SID,
    NAS_NVIM_EPDSZID_SUPPORT_TYPE_PDSZID_SID_NID,

    NAS_NVIM_EPDSZID_SUPPORT_TYPE_BUTT
};
typedef VOS_UINT8 NAS_NVIM_EPDSZID_SUPPORT_TYPE_ENUM_UINT8;


enum NAS_NVIM_LC_RAT_COMBINED_ENUM
{
    NAS_NVIM_LC_RAT_COMBINED_GUL,
    NAS_NVIM_LC_RAT_COMBINED_CL,

    NAS_NVIM_LC_RAT_COMBINED_BUTT
};
typedef VOS_UINT8 NAS_NVIM_LC_RAT_COMBINED_ENUM_UINT8;


enum CNAS_NVIM_1X_NEG_PREF_SYS_CMP_TYPE_ENUM
{
    CNAS_NVIM_1X_NEG_PREF_SYS_CMP_BAND_CHAN_AMBIGUOUS_MATCH,                /* Band Channel ģ��ƥ�� */
    CNAS_NVIM_1X_NEG_PREF_SYS_CMP_BAND_CHAN_ACCURATE_MATCH,                 /* Band Channel ��ȷƥ��*/
    CNAS_NVIM_1X_NEG_PREF_SYS_CMP_BUTT
};
typedef VOS_UINT8 CNAS_NVIM_1X_NEG_PREF_SYS_CMP_TYPE_ENUM_UINT8;


enum NAS_NVIM_CL_SYS_ACQ_DSDS_STRATEGY_SCENE_ENUM
{
    NAS_NVIM_CL_SYS_ACQ_DSDS_STRATEGY_SCENE_SWITCH_ON                 = 0,        /* ���� */

    NAS_NVIM_CL_SYS_ACQ_DSDS_STRATEGY_SCENE_SWITCH_ON_AND_SYSCFG_SET  = 0x1,      /* ������syscfg���ó��� */

    NAS_NVIM_CL_SYS_ACQ_DSDS_STRATEGY_SCENE_ANY                       = 0xFE,     /* CLϵͳ��������ⳡ�� */

    NAS_NVIM_CL_SYS_ACQ_DSDS_STRATEGY_SCENE_ENUM_BUTT
};
typedef VOS_UINT8 NAS_NVIM_CL_SYS_ACQ_DSDS_STRATEGY_SCENE_ENUM_UINT8;



enum NAS_MSCC_NVIM_SYS_ACQ_SCENE_ENUM
{
    NAS_MSCC_NVIM_SYS_ACQ_SCENE_SWITCH_ON                                        = 0,       /* ���� */

    NAS_MSCC_NVIM_SYS_ACQ_SCENE_HRPD_LOST,                                                  /* hrpd ���� */

    NAS_MSCC_NVIM_SYS_ACQ_SCENE_HRPD_LOST_NO_RF,                                            /* hrpd no rf���� */

    NAS_MSCC_NVIM_SYS_ACQ_SCENE_SLEEP_TIMER_EXPIRED,                                        /* sleep ��ʱ����ʱ */

    NAS_MSCC_NVIM_SYS_ACQ_SCENE_SYS_CFG_SET,                                                /* system configure���ô������� */

    NAS_MSCC_NVIM_SYS_ACQ_SCENE_LTE_RF_AVAILABLE,                                           /* LTE RF��Դ���� */

    NAS_MSCC_NVIM_SYS_ACQ_SCENE_HRPD_RF_AVAILABLE,                                          /* HRPD RF��Դ���� */

    NAS_MSCC_NVIM_SYS_ACQ_SCENE_MO_TRIGGER,                                                 /* ���д��� */

    NAS_MSCC_NVIM_SYS_ACQ_SCENE_AVAILABLE_TIMER_EXPIRED_1XSRVEXIST_HISTORY,                 /* 1x�з���ʱavailable��ʱ����ʱ��ʷ�� */
    NAS_MSCC_NVIM_SYS_ACQ_SCENE_AVAILABLE_TIMER_EXPIRED_1XSRVEXIST_PREFBAND,                /* 1x�з���ʱavailable��ʱ����ʱpref band�� */
    NAS_MSCC_NVIM_SYS_ACQ_SCENE_AVAILABLE_TIMER_EXPIRED_1XSRVEXIST_FULLBAND,                /* 1x�з���ʱavailable��ʱ����ʱfull band�� */

    NAS_MSCC_NVIM_SYS_ACQ_BSR,                                                              /* �����ѳ��� */

    NAS_MSCC_NVIM_SYS_ACQ_SCENE_BUTT
};
typedef VOS_UINT32 NAS_MSCC_NVIM_SYS_ACQ_SCENE_ENUM_UINT32;
/*****************************************************************************
  5 STRUCT
*****************************************************************************/
/*****************************************************************************
*                                                                            *
*                           ����������Ϣ�ṹ                                 *
*                                                                            *
******************************************************************************/

typedef struct
{
    VOS_UINT16                          usManualSearchHplmnFlg; /*Range:[0,1]*/
}NAS_MMC_NVIM_MANUAL_SEARCH_HPLMN_FLG_STRU;


typedef struct
{
    VOS_UINT16                          usAutoSearchHplmnFlg; /*Range:[0,3]*/
}NAS_MMC_NVIM_AUTO_SEARCH_HPLMN_FLG_STRU;


typedef struct
{
    VOS_UINT8                           ucActiveFlag;       /* ucActiveFlag �Ƿ񼤻VOS_TRUE:���VOS_FALSE:δ���� */
    VOS_UINT8                           ucReserve1;
    VOS_UINT8                           ucReserve2;
    VOS_UINT8                           ucReserve3;
}NAS_NVIM_ADD_EHPLMN_WHEN_SRCH_HPLMN_CFG_STRU;


typedef struct
{
    VOS_UINT16                          usEHPlmnSupportFlg; /*Range:[0,1]*/
    VOS_UINT8                           ucReserve1;
    VOS_UINT8                           ucReserve2;
}NAS_MMC_NVIM_EHPLMN_SUPPORT_FLG_STRU;


typedef struct
{
    VOS_UINT16                          usStkSteeringOfRoamingSupportFlg; /*Range:[0,1]*/
    VOS_UINT8                           ucReserve1;
    VOS_UINT8                           ucReserve2;
}NAS_MMC_NVIM_STK_STEERING_OF_ROAMING_SUPPORT_FLG_STRU;


typedef struct
{
    VOS_UINT8                           ucNvimActiveFlg;                        /* en_NV_Item_Scan_Ctrl_Para NV���Ƿ񼤻VOS_TRUE:���VOS_FALSE:δ���� */
    VOS_UINT8                           ucReserved1;                            /* ���� */
    VOS_UINT8                           ucReserved2;                            /* ���� */
    VOS_UINT8                           ucReserved3;                            /* ���� */
}NVIM_SCAN_CTRL_STRU;


typedef struct
{
    VOS_UINT32                          ulMcc;                                  /* MCC,3 bytes */
    VOS_UINT32                          ulMnc;                                  /* MNC,2 or 3 bytes */
}NAS_NVIM_PLMN_ID_STRU;


typedef struct
{
    NAS_NVIM_PLMN_ID_STRU               stOperatorPlmnId;
    VOS_UINT8                           aucOperatorNameShort[NAS_NVIM_MAX_OPER_SHORT_NAME_LEN];/* ��ǰפ��������Ӫ�̵Ķ����� */
    VOS_UINT8                           aucOperatorNameLong[NAS_NVIM_MAX_OPER_LONG_NAME_LEN];  /* ��ǰפ��������Ӫ�̵ĳ����� */
}NAS_MMC_NVIM_OPERATOR_NAME_INFO_STRU;


typedef struct
{
    VOS_UINT8                           ucNotReadFileEnableFlg;                  /* ���������Ż�ʹ�ܱ�־��Ĭ�Ͽ�������ʾ��ʱ�򣬿�״̬ǰ���ޱ仯������Ҫ���¶�ȡ���ļ� */
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucReserved2;
    VOS_UINT8                           ucReserved3;
}NAS_NVIM_POWER_ON_READ_USIM_OPTIMIZE_INFO_STRU;



typedef struct
{
    VOS_UINT32                          ulBlackPlmnLockNum;                     /* ֧�ֺ������ĸ���,����Ϊ0ʱ��ʾ��֧�ֺ����� */
    NAS_NVIM_PLMN_ID_STRU               astBlackPlmnId[NAS_MML_NVIM_MAX_BLACK_LOCK_PLMN_NUM];
}NAS_MMC_NVIM_OPER_LOCK_BLACKPLMN_STRU;

typedef struct
{
    VOS_UINT8                           ucEnableFlg;                            /* �������Ƿ�� */
    VOS_UINT8                           ucHighPrioRatType;                              /* ����֧�ֵĽ��뼼�� */
    VOS_UINT8                           aucReserve[2];
    NAS_NVIM_PLMN_ID_STRU               stHighPrioPlmnId;                       /* ����֧�ֵĸ����ȼ���PLMN ID ,��ʹפ����HPLMN��,��PLMNID�����ȼ�Ҳ�ϸ�*/
    NAS_NVIM_PLMN_ID_STRU               stSimHPlmnId;                           /* SIM����HPLMN ID */
}NAS_MMC_NVIM_AIS_ROAMING_CFG_STRU;


typedef struct
{
    VOS_UINT8                           ucAutoReselActiveFlg;                   /* �Ƿ�����LTE�������α��:VOS_TRUE ��ʾ����LTE�������� VOS_FALSE ��ʾ��ֹLTE�������� */
    VOS_UINT8                           ucReserve;
}NAS_MMC_NVIM_USER_AUTO_RESEL_CFG_STRU;


typedef struct
{
    VOS_UINT8                            ucStatus;        /* NV��Ч��־, 1: ��Ч��0����Ч */
	VOS_UINT8                            ucReserved;      /* ���ֽڶ��� */
    VOS_UINT16                           usPrioHplmnAct;      /*���Ƶ����Ƚ��뼼��*/
}NAS_MMC_NVIM_PRIO_HPLMNACT_CFG_STRU;


typedef struct
{
    VOS_UINT32                          ulFirstSearchTimeLen;                   /* available timer��ʱ����һ�ε�ʱ�� */
    VOS_UINT32                          ulFirstSearchTimeCount;                 /* available timer��ʱ����һ�εĴ��� */
    VOS_UINT32                          ulDeepSearchTimeLen;                    /* available timer��ʱ����˯��ʱ�� */
    VOS_UINT32                          ulDeepSearchTimeCount;
}NAS_MMC_NVIM_AVAIL_TIMER_CFG_STRU;



typedef struct
{
    VOS_UINT32                          ulNvActiveFlg;                          /* ���Ƶ�ǰNV�Ƿ�ʹ�� */
    VOS_UINT32                          ulT3212StartSceneCtrlBitMask;           /* BIT0~BIT31,���ڿ�����·ʧ�ܺ�����T3212ʱ���Ƿ�ʹ�õ�ǰNV���õ�ʱ�� */
    VOS_UINT32                          ulT3212Phase1TimeLen;                   /* t3212��ʱ����1�׶ε�ʱ��,��λΪs */
    VOS_UINT32                          ulT3212Phase1Count;                     /* t3212��ʱ����1�׶εĴ��� */
    VOS_UINT32                          ulT3212Phase2TimeLen;                   /* t3212��ʱ����2�׶ε�ʱ��,��λΪs  */
    VOS_UINT32                          ulT3212Phase2Count;                     /* t3212��ʱ����2�׶εĴ��� */
}NAS_MMC_NVIM_T3212_TIMER_CFG_STRU;


typedef struct
{
    VOS_UINT32                          ulWhitePlmnLockNum;                     /* ֧�ְ������ĸ���,����Ϊ0ʱ��ʾ��֧�ְ����� */
    NAS_NVIM_PLMN_ID_STRU               astWhitePlmnId[NAS_MML_NVIM_MAX_WHITE_LOCK_PLMN_NUM];
}NAS_MMC_NVIM_OPER_LOCK_WHITEPLMN_STRU;


typedef struct
{
    VOS_UINT16                          usSupportFlg; /*Range:[0,1]*/
    VOS_UINT8                           ucReserve1;
    VOS_UINT8                           ucReserve2;
}NAS_MMC_NVIM_CPHS_SUPPORT_FLG_STRU;

/* Added by l60609 for B060 Project, 2012-2-20, Begin   */

typedef struct
{
    VOS_UINT8                           ucStatus;                               /*�Ƿ񼤻0�����1���� */
    VOS_UINT8                           ucActFlg;
    VOS_UINT8                           ucReserve1;
    VOS_UINT8                           ucReserve2;
}NAS_PREVENT_TEST_IMSI_REG_STRU;
/* Added by l60609 for B060 Project, 2012-2-20, End   */
/*****************************************************************************
*                                                                            *
*                           ����������Ϣ�ṹ                                 *
*                                                                            *
******************************************************************************/


typedef struct
{
    VOS_UINT8   ucHplmnSearchPowerOn;
    VOS_UINT8   ucReserve1;
    VOS_UINT8   ucReserve2;
    VOS_UINT8   ucReserve3;
}NAS_MMC_NVIM_HPLMN_SEARCH_POWERON_STRU;



typedef struct
{
    VOS_UINT8                           ucTinType;                              /* TIN���� */
    VOS_UINT8                           aucReserve[2];
    VOS_UINT8                           aucImsi[NAS_NVIM_ITEM_MAX_IMSI_LEN];        /* �ϴα����IMSI������ */
}NAS_NVIM_TIN_INFO_STRU;


typedef struct
{
    VOS_UINT32 ulAutoStart;
}NAS_MMA_NVIM_AUTO_START_STRU;



typedef struct
{
    VOS_UINT8                           ucSingleDomainFailPlmnSrchFlag;         /* DT�������󣬵���ע�ᱻ�ܺ���Ҫ�������� */
    VOS_UINT8                           ucReserved;                             /* ����*/
}NAS_MMC_NVIM_SINGLE_DOMAIN_FAIL_CNT_STRU;


typedef struct
{
    VOS_UINT8                           aucPsLocInfo[NAS_NVIM_PS_LOCI_SIM_FILE_LEN];
}NAS_NVIM_PS_LOCI_SIM_FILES_STRU;


typedef struct
{
    VOS_UINT8                           aucTmsi[NAS_MAX_TMSI_LEN];
    VOS_UINT8                           aucLastLai[NAS_SIM_MAX_LAI_LEN];
    VOS_UINT8                           ucLauStaus;
    VOS_UINT8                           ucReserved;
}NAS_NVIM_CS_LOCI_SIM_FILES_STRU;


typedef struct
{
    VOS_UINT8                          ucStatus;                                /* NV�Ƿ񼤻��־, 0: �����1: ���� */
    VOS_UINT8                          ucGeaSupportCtrl;                        /* �ն�����:GPRS GEA�㷨֧�ֿ��� */
}NAS_MMC_NVIM_GPRS_GEA_ALG_CTRL_STRU;


typedef struct
{
    VOS_UINT8                           ucNvimActiveFlg;    /* en_NV_Item_Lte_Cs_Service_Config NV���Ƿ񼤻VOS_TRUE:���VOS_FALSE:δ���� */
    VOS_UINT8                           ucLteCsServiceCfg;  /* LTE֧�ֵ� cs��ҵ������*/
}NAS_NVIM_LTE_CS_SERVICE_CFG_STRU;


typedef struct
{
    VOS_UINT8                           ucNvimActiveFlg;
    VOS_UINT8                           ucWaitSysinfoTimeLen;
}NAS_MMC_NVIM_HO_WAIT_SYSINFO_TIMER_CFG_STRU;




typedef struct
{
    VOS_UINT8                           ucLteRoamAllowedFlg;
    VOS_UINT8                           aucReserve[1];
    VOS_UINT8                           aucRoamEnabledMccList[20];/* �������εĹ������б� */
}NAS_MMC_NVIM_LTE_INTERNATIONAL_ROAM_CFG_STRU;


typedef struct
{
    VOS_UINT8                           ucRoamRplmnflg;
    VOS_UINT8                           aucReserve[3];
    VOS_UINT32                         aucRoamEnabledMccList[5];/* �������εĹ������б� */
}NAS_MMC_NVIM_ROAM_SEARCH_RPLMN_CFG_STRU;


typedef struct
{
    VOS_UINT8                           ucActFlg;
    VOS_UINT8                           aucReserved[3];
}NAS_NVIM_CLOSE_SMS_CAPABILITY_CFG_STRU;


typedef struct
{
    VOS_UINT8                           ucWcdmaPriorityGsmFlg;                  /* H3G��������W����������G*/

    VOS_UINT8                           ucSortAvailalePlmnListRatPrioFlg;    /* �Ƿ�syscfg���ý��뼼�����ȼ�����ߵ����������б��ʶ��1:��; 0:���Ǹ��������簴���������������粻����*/
    VOS_UINT8                           ucReserve1;
    VOS_UINT8                           ucReserve2;
}NAS_MMC_NVIM_WCDMA_PRIORITY_GSM_FLG_STRU;


typedef struct
{
    VOS_UINT8                           ucPsOnlyCsServiceSupportFlg;            /* ����������ΪPS ONLYʱ���Ƿ�֧��CS����źͺ���ҵ��(�������г���)*/
    VOS_UINT8                           ucReserved1;                            /* ����*/
    VOS_UINT8                           ucReserved2;                            /* ����*/
    VOS_UINT8                           ucReserved3;                            /* ����*/
}NAS_NVIM_PS_ONLY_CS_SERVICE_SUPPORT_FLG_STRU;


typedef struct
{
    VOS_UINT8                           ucCcbsSupportFlg;                       /* CCBS(��æ�������)ҵ��*/
    VOS_UINT8                           ucReserved1;                            /* ����*/
    VOS_UINT8                           ucReserved2;                            /* ����*/
    VOS_UINT8                           ucReserved3;                            /* ����*/
}NAS_NVIM_CCBS_SUPPORT_FLG_STRU;


typedef struct
{
    VOS_UINT8                           ucNvimActiveFlg;
    VOS_UINT8                           ucCustomMccNum;
    VOS_UINT8                           aucReserve[2];
    VOS_UINT32                          aulCustommMccList[10];                   /* �������εĹ������б� */
}NAS_MMC_NVIM_HPLMN_SEARCH_REGARDLESS_MCC_SUPPORT_STRU;



typedef struct
{
    VOS_UINT8                           ucNvimActiveFlg;
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucReserved2;
    VOS_UINT8                           ucReserved3;
}NVIM_ACTING_HPLMN_SUPPORT_FLAG_STRU;


typedef struct
{
    VOS_UINT8                           ucNvimActiveFlg;
    VOS_UINT8                           ucReserved1;                            /* ����*/
    VOS_UINT8                           ucReserved2;                            /* ����*/
    VOS_UINT8                           ucReserved3;                            /* ����*/
}NAS_MMC_NVIM_REG_FAIL_NETWORK_FAILURE_CUSTOM_FLG_STRU;




typedef struct
{
    NAS_MMC_NVIM_REG_FAIL_CAUSE_ENUM_UINT16                 enRegCause;
    NAS_MMC_NVIM_REG_DOMAIN_ENUM_UINT8                      enDomain;
    NAS_MMC_NVIM_SINGLE_DOMAIN_REG_FAIL_ACTION_ENUM_UINT8   enAction;
}NAS_MMC_NVIM_SINGLE_DOMAIN_REG_FAIL_ACTION_STRU;



typedef struct
{
    VOS_UINT8                                               ucActiveFlag;
    VOS_UINT8                                               ucCount;
    VOS_UINT8                                               auReserv[2];
    NAS_MMC_NVIM_SINGLE_DOMAIN_REG_FAIL_ACTION_STRU         astSingleDomainFailActionList[NAS_MML_SINGLE_DOMAIN_FAIL_ACTION_MAX_LIST];
}NAS_MMC_NVIM_SINGLE_DOMAIN_FAIL_ACTION_LIST_STRU;


typedef struct {
    VOS_UINT8                           aucSimPlmn[NAS_SIM_FORMAT_PLMN_LEN];
    VOS_UINT8                           aucReserve[1];
}NAS_SIM_FORMAT_PLMN_ID;


typedef struct
{
    VOS_UINT8                           ucUtranMode;                            /* ��ǰ֧�ֵ�UTRANģʽ */
    VOS_UINT8                           ucReserve;
}NAS_UTRANCTRL_NVIM_UTRAN_MODE_STRU;


typedef struct
{
    VOS_UINT8                           ucUtranSwitchMode;                      /* 0��UTRANģʽΪFDD 1��UTRANģʽΪTDD 2:AUTO SWITCH */
    VOS_UINT8                           ucTdMccListNum;                         /* ֧��TD����Ĺ��ҺŸ��� */
    VOS_UINT8                           ucImsiPlmnListNum;                      /* ����IMS PLMN����UTRANģʽ�Ƿ�AUTO SWITCHʱ����ǰIMSI��PLMN�ڴ��б��еĶ�֧���Զ��л�������̶�ΪWģ */
    VOS_UINT8                           aucReserve[1];

    VOS_UINT32                          aulTdMccList[NAS_UTRANCTRL_MAX_NVIM_CFG_TD_MCC_LIST_NUM];
    NAS_SIM_FORMAT_PLMN_ID              astImsiPlmnList[NAS_UTRANCTRL_MAX_NVIM_CFG_IMSI_PLMN_LIST_NUM];
}NAS_UTRANCTRL_NVIM_UTRAN_MODE_AUTO_SWITCH_STRU;





typedef struct
{
    VOS_UINT8                           ucIsUtranTddCsSmcNeeded;                /* ������3G TDDģʽ���Ƿ���Ҫ����CS SMC��֤���:0-����Ҫ��1-��Ҫ */
    VOS_UINT8                           ucIsUtranTddPsSmcNeeded;                /* ������3G TDDģʽ���Ƿ���Ҫ����PS SMC��֤���:0-����Ҫ��1-��Ҫ */
    VOS_UINT8                           aucReserved[2];                         /* ���� */
}NAS_UTRANCTRL_NVIM_SMC_CTRL_FLAG_STRU;



typedef struct
{
    VOS_UINT8   ucAccBarPlmnSearchFlg;
    VOS_UINT8   aucReserve[3];
}NAS_MMC_NVIM_ACC_BAR_PLMN_SEARCH_FLG_STRU;



typedef struct
{
    VOS_UINT8                           ucActiveFlg;                                          /* ������ʹ�ܱ�־ */
    VOS_UINT8                           ucImsiCheckFlg;                                       /* �Ƿ���IMSI�б�İ�������0:����Ҫ 1:��Ҫ */
    VOS_UINT8                           aucVersion[NAS_MMC_NVIM_MAX_USER_OPLMN_VERSION_LEN];  /* �������õİ汾�� */
    VOS_UINT8                           ucImsiPlmnListNum;                                    /* ���Ƶ�IMSI�б���� */
    VOS_UINT8                           ucOplmnListNum;                                       /* �������õ�Oplmn�ĸ��� */
    NAS_SIM_FORMAT_PLMN_ID              astImsiPlmnList[NAS_MMC_NVIM_MAX_USER_OPLMN_IMSI_NUM];
    VOS_UINT8                           aucOPlmnList[NAS_MMC_NVIM_MAX_USER_CFG_OPLMN_DATA_LEN];/* OPLMN��PDU���ݣ���EFOplmn�ļ�һ�� */
}NAS_MMC_NVIM_USER_CFG_OPLMN_INFO_STRU;

/*****************************************************************************
 �ṹ��    : NAS_MMC_NVIM_USER_CFG_OPLMN_EXTEND_STRU
 �ṹ˵��  : NAS_MMC_NVIM_USER_CFG_OPLMN_EXTEND_STRU NVIM���е��û����õ�OPLMN
 1.��    ��   : 2013��11��26��
   �޸�����   : ��NV֧�ֵ����OPLMN������չ��256��
*****************************************************************************/
typedef struct
{
    VOS_UINT8                           ucActiveFlg;                                          /* ������ʹ�ܱ�־ */
    VOS_UINT8                           ucImsiCheckFlg;                                       /* �Ƿ���IMSI�б�İ�������0:����Ҫ 1:��Ҫ */
    VOS_UINT8                           aucVersion[NAS_MMC_NVIM_MAX_USER_OPLMN_VERSION_LEN];  /* �������õİ汾�� */
    VOS_UINT16                          usOplmnListNum;                                       /* �������õ�Oplmn�ĸ��� */
    VOS_UINT8                           ucImsiPlmnListNum;                                    /* ���Ƶ�IMSI�б���� */
    VOS_UINT8                           aucReserve[3];
    NAS_SIM_FORMAT_PLMN_ID              astImsiPlmnList[NAS_MMC_NVIM_MAX_USER_OPLMN_IMSI_NUM];
    VOS_UINT8                           aucOPlmnList[NAS_MMC_NVIM_MAX_USER_CFG_OPLMN_DATA_EXTEND_LEN];/* OPLMN��PDU���ݣ���EFOplmn�ļ�һ�� */
}NAS_MMC_NVIM_USER_CFG_OPLMN_EXTEND_STRU;



typedef struct
{
    VOS_UINT8                           ucEnableFlg;                                                    /* NVʹ�ܱ�־ */
    VOS_UINT8                           ucExtendedForbPlmnNum;                                          /* ��չ��Forb Plmn���� */
    VOS_UINT8                           ucReserve1;
    VOS_UINT8                           ucReserve2;
    NAS_NVIM_PLMN_ID_STRU               astForbPlmnIdList[NAS_MMC_NVIM_MAX_EXTENDED_FORB_PLMN_NUM];     /* ��չ��Forb Plmn�б� */
}NAS_MMC_NVIM_EXTENDED_FORBIDDEN_PLMN_LIST_CFG_STRU;


typedef struct
{
    VOS_UINT16                         usDplmnListNum;                                       /* �������õ�Dplmn�ĸ��� */
    VOS_UINT16                         usNplmnListNum;                                       /* �������õ�Nplmn�ĸ��� */

    /* DPLMN����,ÿ7���ֽڴ���һ��dplmn��Ϣ����1-3���ֽ�Ϊsim����ʽplmn id��
       ��4-5�ֽ�Ϊ֧�ֵĽ��뼼��(0x8000Ϊ֧��w��0x4000Ϊ֧��lte��0x0080Ϊ֧��gsm)��
       ��6�ֽ�Ϊ����Ϣ:1(cs��ע��ɹ�)��2(ps��ע��ɹ�)��3(cs ps��ע��ɹ�)
       ��7ֱ��ΪԤ�ñ�ʾ��Ϣ: 1(Ԥ��Dplmn), 0(��ѧϰ����DPLMN) */
    VOS_UINT8                          aucDPlmnList[NAS_MMC_NVIM_MAX_CFG_DPLMN_DATA_LEN];

    /* NPLMN����,ÿ7���ֽڴ���һ��nplmn��Ϣ����1-3���ֽ�Ϊsim����ʽplmn id��
       ��4-5�ֽ�Ϊ֧�ֵĽ��뼼��(0x8000Ϊ֧��w��0x4000Ϊ֧��lte��0x0080Ϊ֧��gsm)��
       ��6�ֽ�Ϊ����Ϣ:1(cs��ע��ɹ�)��2(ps��ע��ɹ�)��3(cs ps��ע��ɹ�)
       ��7ֱ��ΪԤ�ñ�ʾ��Ϣ: 1(Ԥ��nplmn), 0(��ѧϰ����nplmn) */
    VOS_UINT8                          aucNPlmnList[NAS_MMC_NVIM_MAX_CFG_NPLMN_DATA_LEN];/* NPLMN����*/
}NAS_MMC_NVIM_CFG_DPLMN_NPLMN_INFO_STRU;


typedef struct
{
    VOS_UINT16                         usDplmnListNum;                                       /* �������õ�Dplmn�ĸ��� */
    VOS_UINT16                         usNplmnListNum;                                       /* �������õ�Nplmn�ĸ��� */

    /* DPLMN����,ÿ6���ֽڴ���һ��dplmn��Ϣ����1-3���ֽ�Ϊsim����ʽplmn id��
       ��4-5�ֽ�Ϊ֧�ֵĽ��뼼��(0x8000Ϊ֧��w��0x4000Ϊ֧��lte��0x0080Ϊ֧��gsm)����6�ֽ�Ϊ����Ϣ:1(cs��ע��ɹ�)��2(ps��ע��ɹ�)��3(cs ps��ע��ɹ�)*/
    VOS_UINT8                          aucDPlmnList[NAS_MMC_NVIM_MAX_CFG_DPLMN_DATA_EXTEND_LEN];

    /* NPLMN����,ÿ6���ֽڴ���һ��nplmn��Ϣ����1-3���ֽ�Ϊsim����ʽplmn id��
       ��4-5�ֽ�Ϊ֧�ֵĽ��뼼��(0x8000Ϊ֧��w��0x4000Ϊ֧��lte��0x0080Ϊ֧��gsm)����6�ֽ�Ϊ����Ϣ:1(cs��ע��ɹ�)��2(ps��ע��ɹ�)��3(cs ps��ע��ɹ�)*/
    VOS_UINT8                          aucNPlmnList[NAS_MMC_NVIM_MAX_CFG_NPLMN_DATA_EXTEND_LEN];/* NPLMN����*/
}NAS_MMC_NVIM_CFG_DPLMN_NPLMN_INFO_OLD_STRU;


typedef struct
{
    VOS_UINT16                         usCfgDplmnNplmnFlag;
    VOS_UINT8                          ucCMCCHplmnNum;
    VOS_UINT8                          aucCMCCHplmnList[NAS_MMC_NVIM_MAX_CFG_HPLMN_NUM];
    VOS_UINT8                          ucUNICOMHplmnNum;
    VOS_UINT8                          aucUNICOMHplmnList[NAS_MMC_NVIM_MAX_CFG_HPLMN_NUM];
    VOS_UINT8                          ucCTHplmnNum;
    VOS_UINT8                          aucCTHplmnList[NAS_MMC_NVIM_MAX_CFG_HPLMN_NUM];
    VOS_UINT8                          aucReserve[3];
}NAS_MMC_NVIM_CFG_DPLMN_NPLMN_FLAG_STRU;


typedef struct
{
    VOS_UINT8                   ucRestrainHighPrioRatHPlmnSrch;         /* Home��������Ƚ��뼼���������Ƿ������ƣ�1:���ƣ�0:������ Dallas���� */
    VOS_UINT8                   ucRestrainAnyCellSrch;                  /* AnyCell�����Ƿ������ƣ�1:���ƣ�0:������ */
    VOS_UINT16                  usHighPrioRatHPlmnSrchTimerRetryLen;    /* HighPrioRatHPlmnSrch�����ƺ����Զ�ʱ��ʱ������λ:��  Dallas����*/

    VOS_UINT8                   ucRsv1;                                /* ����λ1 */
    VOS_UINT8                   ucRsv2;                                /* ����λ2 */
    VOS_UINT8                   ucRsv3;                                /* ����λ3 */
    VOS_UINT8                   ucRsv4;                                /* ����λ4 */
}NV_NAS_HIGH_PRIO_PS_CFG_STRU;


typedef struct
{
    VOS_UINT8                           ucEHplmnNum;
    VOS_UINT8                           aucEHplmnList[NAS_MMC_NVIM_MAX_CFG_HPLMN_NUM];
    VOS_UINT8                           aucVersionId[NAS_MCC_NVIM_VERSION_LEN];
    VOS_UINT8                           aucReserved[2];
}NAS_MMC_NVIM_DPLMN_NPLMN_CFG_STRU;


typedef struct
{
    VOS_UINT8                           aucVersionId[NAS_MCC_NVIM_VERSION_LEN];
    VOS_UINT8                           ucReserved;                                                 /* ����λ */
    VOS_UINT16                          usBorderNum;                                                /* �߾���Ϣ�ĸ��� */
    /* �߾���Ϣ����,ÿ7���ֽڴ���һ���߾���Ϣ����1-3���ֽ�Ϊsim����ʽplmn id��
       ��4-5�ֽ�Ϊ֧�ֵĽ��뼼��(0x8000Ϊ֧��w��0x4000Ϊ֧��lte��0x0080Ϊ֧��gsm)����6-7�ֽ�ΪLAC��TAC��Ϣ
       ���뼼��ΪWGʱ��6-7�ֽڱ�ʾLAC;���뼼��ΪLTEʱ��6-7�ֽڱ�ʾTAC */
    VOS_UINT8                           aucBorderList[NAS_MMC_NVIM_MAX_CFG_BORDER_DATA_LEN];        /* �߾���Ϣ�б� */
}NAS_NVIM_BORDER_INFO_STRU;



typedef struct
{
    VOS_UINT8                           ucImsiPlmnListNum;                      /* ���Ƶ�IMSI�б���� */
    VOS_UINT8                           ucEhplmnListNum;                        /* �û����õ�EHplmn�ĸ��� */
    VOS_UINT8                           aucReserve[2];
    NAS_SIM_FORMAT_PLMN_ID              astImsiPlmnList[NAS_MMC_NVIM_MAX_USER_CFG_IMSI_PLMN_NUM];
    NAS_SIM_FORMAT_PLMN_ID              astEhPlmnList[NAS_MMC_NVIM_MAX_USER_CFG_EHPLMN_NUM];
}NAS_MMC_NVIM_USER_CFG_EHPLMN_INFO_STRU;


typedef struct
{
    VOS_UINT32                                ulNvimEhplmnNum;
    NAS_MMC_NVIM_USER_CFG_EHPLMN_INFO_STRU    astNvimEhplmnInfo[NAS_MMC_NVIM_MAX_USER_CFG_EXT_EHPLMN_NUM];
}NAS_MMC_NVIM_USER_CFG_EXT_EHPLMN_INFO_STRU;



typedef struct
{
    VOS_UINT8                           ucImsiPlmnListNum;                      /* ÿһ���ﶨ�Ƶ�IMSI�б���� */
    VOS_UINT8                           ucForbPlmnListNum;                      /* ÿһ�����û����õ�Forb Plmn�ĸ��� */
    VOS_UINT8                           aucReserve[2];
    NAS_SIM_FORMAT_PLMN_ID              astImsiPlmnList[NAS_MMC_NVIM_MAX_USER_CFG_IMSI_PLMN_NUM];
    NAS_SIM_FORMAT_PLMN_ID              astForbPlmnList[NAS_MMC_MAX_BLACK_LOCK_PLMN_WITH_RAT_NUM];
}NAS_MMC_NVIM_FORB_PLMN_INFO_STRU;



typedef struct
{
    VOS_UINT32                          ulGroupNum;
    NAS_MMC_NVIM_FORB_PLMN_INFO_STRU    astForbPlmnInfo[NAS_MMC_NVIM_MAX_USER_CFG_FORB_PLMN_GROUP_NUM];
}NAS_MMC_NVIM_USER_CFG_FORB_PLMN_INFO_STRU;


typedef struct
{
    VOS_UINT8                          ucCause18EnableLteSupportFlg; /*�Ƿ�֧��Cause18 Enable Lte*/
    VOS_UINT8                          aucReserved0;
    VOS_UINT8                          aucReserved1;
    VOS_UINT8                          aucReserved2;
}NAS_MMC_NVIM_CAUSE18_ENABLE_LTE_SUPPORT_FLG_STRU;


typedef struct
{
    VOS_UINT32                          ulMcc;                                  /* MCC,3 bytes */
    VOS_UINT32                          ulMnc;                                  /* MNC,2 or 3 bytes */
    VOS_UINT8                           enRat;
    VOS_UINT8                           aucReserve[3];
}NAS_MMC_NVIM_PLMN_WITH_RAT_STRU;



typedef struct
{
    VOS_UINT32                          ulDisabledRatPlmnNum;                   /* ֧�ֽ�ֹ���뼼����PLMN����,����Ϊ0��ʾ��֧�ָ����� */

    NAS_MMC_NVIM_PLMN_WITH_RAT_STRU     astDisabledRatPlmnId[NAS_MML_NVIM_MAX_DISABLED_RAT_PLMN_NUM];/* ��ֹ���뼼����PLMN��RAT��Ϣ */

}NAS_MMC_NVIM_DISABLED_RAT_PLMN_INFO_STRU;


enum NAS_MMC_NVIM_RAT_FORBIDDEN_LIST_SWITCH_FLAG_ENUM
{
    NAS_MMC_NVIM_RAT_FORBIDDEN_LIST_SWITCH_INACTIVE                   = 0,           /* ����δ���� */
    NAS_MMC_NVIM_RAT_FORBIDDEN_LIST_SWITCH_BLACK                      = 1,           /* �������������� */
    NAS_MMC_NVIM_RAT_FORBIDDEN_LIST_SWITCH_WHITE                      = 2,           /* �������������� */
    NAS_MMC_NVIM_RAT_FORBIDDEN_LIST_SWITCH_BUTT
};
typedef VOS_UINT8 NAS_MMC_NVIM_RAT_FORBIDDEN_LIST_SWITCH_FLAG_ENUM_UINT8;


enum NAS_MMC_NVIM_PLATFORM_SUPPORT_RAT_ENUM
{
    NAS_MMC_NVIM_PLATFORM_SUPPORT_RAT_GERAN                   = 0,           /* GERAN */
    NAS_MMC_NVIM_PLATFORM_SUPPORT_RAT_UTRAN                   = 1,           /* UTRAN����WCDMA/TDS-CDMA */
    NAS_MMC_NVIM_PLATFORM_SUPPORT_RAT_EUTRAN                  = 2,           /* E-UTRAN */
    NAS_MMC_NVIM_PLATFORM_SUPPORT_RAT_BUTT
};
typedef VOS_UINT8 NAS_MMC_NVIM_PLATFORM_SUPPORT_RAT_ENUM_UINT8;


enum NAS_MSCC_NVIM_SYS_PRI_CLASS_ENUM
{
    NAS_MSCC_NVIM_SYS_PRI_CLASS_HOME             = 0,    /* home or ehome plmn */
    NAS_MSCC_NVIM_SYS_PRI_CLASS_PREF             = 1,    /* UPLMN or OPLMN */
    NAS_MSCC_NVIM_SYS_PRI_CLASS_ANY              = 2,    /* Acceptable PLMN */
    NAS_MSCC_NVIM_SYS_PRI_CLASS_BUTT
};
typedef VOS_UINT8 NAS_MSCC_NVIM_SYS_PRI_CLASS_ENUM_UINT8;


typedef struct
{
    NAS_MMC_NVIM_RAT_FORBIDDEN_LIST_SWITCH_FLAG_ENUM_UINT8  enSwitchFlag;                                         /*�����Ƿ���Ч�����ܵ�����  */
    VOS_UINT8                                               ucImsiListNum;                                        /*������Ч��SIM����Ŀ(������/������)  */
    VOS_UINT8                                               ucForbidRatNum;                                       /*��ֹRAT����Ŀ  */
    VOS_UINT8                                               aucReserve[1];
    NAS_SIM_FORMAT_PLMN_ID                                  astImsiList[NAS_NVIM_MAX_IMSI_FORBIDDEN_LIST_NUM];        /* SIM���б� (������/������) */
    NAS_MMC_NVIM_PLATFORM_SUPPORT_RAT_ENUM_UINT8            aenForbidRatList[NAS_NVIM_MAX_RAT_FORBIDDEN_LIST_NUM];    /*��ֹ�Ľ��뼼��  */
}NAS_MMC_NVIM_RAT_FORBIDDEN_LIST_STRU;


typedef struct
{
    VOS_UINT8                           ucCsfbEmgCallLaiChgLauFirstFlg;
    VOS_UINT8                           aucRserved[1];
}NAS_MMC_NVIM_CSFB_EMG_CALL_LAI_CHG_LAU_FIRST_CFG_STRU;

typedef struct
{
    VOS_UINT8                           ucPlmnExactlyCompareFlag;
    VOS_UINT8                           aucRsv[3];                         /* ����*/
}NAS_MMC_NVIM_PLMN_EXACTLY_COMPARE_FLAG_STRU;


typedef struct
{
    VOS_UINT8                           ucHplmnRegisterCtrlFlg;                 /* HPLMNע����Ʊ�� */
    VOS_UINT8                           aucRsv[3];                              /* ���� */
}NAS_MMC_NVIM_HPLMN_REGISTER_CTRL_FLAG_STRU;

typedef struct
{
    VOS_UINT8                                               ucSignThreshold;    /* �źű仯����,��RSSI�仯������ֵ��
                                                                                  �������Ҫ�����ϱ��ź�������ȡֵ0��ʾ����㰴Ĭ��ֵ���� */
    VOS_UINT8                                               ucMinRptTimerInterval;     /* ����ϱ���ʱ��   */
    VOS_UINT8                                               ucRserved1;
    VOS_UINT8                                               ucRserved2;
} NAS_NVIM_CELL_SIGN_REPORT_CFG_STRU;


typedef struct
{
    VOS_UINT8                           ucH3gCtrlFlg;                           /* H3G���Ʊ�� */
    VOS_UINT8                           aucRsv[3];                              /* ���� */
}NAS_MMC_NVIM_H3G_CTRL_FLAG_STRU;


typedef struct
{
    VOS_UINT16                          usUcs2Customization;
    VOS_UINT8                           ucRserved1;
    VOS_UINT8                           ucRserved2;
}NAS_MMC_NVIM_UCS2_CUSTOMIZATION_STRU;


typedef struct
{
    VOS_UINT16                          usTc1mLength;
    VOS_UINT16                          usTr1mLength;
    VOS_UINT16                          usTr2mLength;
    VOS_UINT16                          usTramLength;
}SMS_NVIM_TIMER_LENGTH_STRU;



typedef struct
{
    VOS_UINT8                           ucStatus;                               /* NV�Ƿ񼤻��־, 0: �����1: ���� */
    VOS_UINT8                           ucStatusRptGeneralControl;        /* ˽�������Ƿ�����״̬�ϱ� 0:���ϱ���1:�ϱ� */
    VOS_UINT8                           ucReserve1;
    VOS_UINT8                           ucReserve2;
}NVIM_PRIVATE_CMD_STATUS_RPT_STRU;


typedef struct
{
    VOS_UINT8                           ucStatus;                               /* NV�Ƿ񼤻��־, 0: �����1: ����  */
    VOS_UINT8                           ucSpecialRoamFlg;                         /* Vplmn��Hplmn��ͬ������ʱ,�Ƿ�����ص�Hplmn,1:����0:������ */
    VOS_UINT8                           ucReserve1;
    VOS_UINT8                           ucReserve2;
}NAS_MMC_NVIM_SPECIAL_ROAM_STRU;


typedef struct
{
    VOS_UINT16                          usEnhancedHplmnSrchFlg;
    VOS_UINT8                           ucReserve1;
    VOS_UINT8                           ucReserve2;
}NAS_MMC_NVIM_ENHANCED_HPLMN_SRCH_FLG_STRU;


typedef struct
{
    VOS_UINT8   ucT305Len;
    VOS_UINT8   ucT308Len;
    VOS_UINT8   aucReserve[2];
}NAS_CC_NVIM_TIMER_LEN_STRU;


typedef struct
{
    VOS_UINT8                           ucT303ActiveFlag;                       /* �Ƿ���T303��ʱ����0:�رգ�1:������*/
    VOS_UINT8                           ucT303Len;                              /* T303��ʱ��ʱ�� */
    VOS_UINT8                           ucReserve1;
    VOS_UINT8                           ucReserve2;
}NAS_CC_NVIM_T303_LEN_CFG_STRU;


typedef struct
{
    VOS_UINT8   ucCmSrvExistTrigPlmnSearch;                                                     /* ҵ�����ʱ�Ƿ񴥷����� */
    VOS_UINT8   ucCmSrvTrigPlmnSearchCauseNum;                                                  /* ����ҵ�����ʱ���������ı���ԭ��ֵ���� */
    VOS_UINT8   ucReserve1;
    VOS_UINT8   ucReserve2;
    VOS_UINT8   aucCmSrvTrigPlmnSearchCause[NAS_NVIM_MAX_LAU_REJ_TRIG_PLMN_SEARCH_CAUSE_NUM];   /* ����ҵ�����ʱ���������ı���ԭ��ֵ */
    VOS_UINT8   aucReserve[NAS_NVIM_MAX_LAU_REJ_TRIG_PLMN_SEARCH_CAUSE_NUM];                    /* Ԥ����ע�ᱻ�ܴ�������ʹ�� */
}NAS_NVIM_LAU_REJ_TRIG_PLMN_SEARCH_CFG_STRU;


typedef struct
{
    VOS_UINT8   ucLauRejCauseNum;
    VOS_UINT8   ucLauRejTimes;
    VOS_UINT8   ucReserve1;
    VOS_UINT8   ucReserve2;
    VOS_UINT8   aucLauRejCause[NAS_NVIM_MAX_REJECT_NO_RETRY_CAUSE_NUM];
}NAS_NVIM_LAU_REJ_NORETRY_WHEN_CM_SRV_EXIST_CFG_STRU;


typedef struct
{
    NAS_NVIM_CHANGE_REG_REJ_CAUSE_TYPE_ENUM_UINT8           enChangeRegRejCauCfg;
    VOS_UINT8   ucPreferredRegRejCau_HPLMN_EHPLMN;             /* HPLMN/EHPLMNʱʹ�õľܾ�ԭ��ֵ */
    VOS_UINT8   ucPreferredRegRejCau_NOT_HPLMN_EHPLMN;         /* ��HPLMN/EHPLMNʱʹ�õľܾ�ԭ��ֵ */
    VOS_UINT8   aucReserve[1];
}NAS_NVIM_CHANGE_REG_REJECT_CAUSE_FLG_STRU;


typedef struct
{
    VOS_UINT8   ucNoRetryRejectCauseNum;
    VOS_UINT8   aucNoRetryRejectCause[NAS_NVIM_MAX_REJECT_NO_RETRY_CAUSE_NUM];
    VOS_UINT8   aucReserve[3];
}NAS_NVIM_ROAMINGREJECT_NORETYR_CFG_STRU;


typedef struct
{
   VOS_UINT8                           ucIgnoreAuthRejFlg;
   VOS_UINT8                           ucMaxAuthRejNo;
   VOS_UINT8                           aucReserved[2];
}NAS_MMC_NVIM_IGNORE_AUTH_REJ_CFG_STRU;


typedef struct
{
    VOS_UINT8 ucStatus;
    VOS_UINT8 ucReserved1;
    VOS_UINT8 aucE5GwMacAddr[18];
}NAS_NV_GWMAC_ADDR_STRU;


typedef struct
{
    VOS_UINT8 aucE5_RoamingWhiteList_Support_Flg[2];
}NAS_NVIM_E5_ROAMING_WHITE_LIST_SUPPORT_FLG_STRU;


typedef struct
{
    VOS_UINT32 ulNDIS_DIALUP_ADDRESS;
}NAS_NVIM_NDIS_DIALUP_ADDRESS_STRU;


typedef struct
{
    VOS_UINT8 ucBreOnTime; /*Range:[0,7]*/
    VOS_UINT8 ucBreOffTime; /*Range:[0,7]*/
    VOS_UINT8 ucBreRiseTime; /*Range:[0,5]*/
    VOS_UINT8 ucBreFallTime; /*Range:[0,5]*/
}NAS_NVIM_NV_BREATH_LED_STR_STRU;


typedef struct
{
    VOS_UINT8                           ucIsManualModeRegHplmnFlg;              /* �ֶ�����ģʽ���û�ָ�������HPLMN���ѵ�HPLMNʱ�Ƿ�����ע��Ŀ��� */
    VOS_UINT8                           ucReserved1;                            /* ����λ */
    VOS_UINT8                           ucReserved2;                            /* ����λ */
    VOS_UINT8                           ucReserved3;                            /* ����λ */
}NAS_NVIM_MANUAL_MODE_REG_HPLMN_CFG_STRU;


typedef struct
{
VOS_UINT8  aucwlAuthMode[16];
VOS_UINT8  aucBasicEncryptionModes[5];
VOS_UINT8  aucWPAEncryptionModes[5];
VOS_UINT8  aucwlKeys1[27];
VOS_UINT8  aucwlKeys2[27];
VOS_UINT8  aucwlKeys3[27];
VOS_UINT8  aucwlKeys4[27];
VOS_UINT32 ulwlKeyIndex;
VOS_UINT8  aucwlWpaPsk[65];
VOS_UINT8  ucwlWpsEnbl;
VOS_UINT8  ucwlWpsCfg;
VOS_UINT8  ucReserved;
}NAS_NVIM_NV_WIFI_KEY_STRU;


typedef struct
{
VOS_UINT8 aucPRIVersion[32];
VOS_UINT8 aucReserve[32];
}NAS_NVIM_NV_PRI_VERSION_STRU;


typedef struct
{
    VOS_UINT16                          usSysAppConfigType;
    VOS_UINT8                           ucReserve1;
    VOS_UINT8                           ucReserve2;
}NAS_NVIM_SYSTEM_APP_CONFIG_STRU;


typedef struct
{
    VOS_UINT8                           aucNetworkCapability[NV_ITEM_NET_CAPABILITY_MAX_SIZE];
}NAS_MMC_NVIM_NETWORK_CAPABILITY_STRU;


typedef struct
{
    VOS_UINT16                          usAutoattachFlag;
    VOS_UINT8                           ucReserve1;
    VOS_UINT8                           ucReserve2;
}NAS_NVIM_AUTOATTACH_STRU;


typedef struct
{
    VOS_UINT16                          usSelPlmnMode;
    VOS_UINT8                           ucReserve1;
    VOS_UINT8                           ucReserve2;
}NAS_NVIM_SELPLMN_MODE_STRU;


typedef struct
{
    VOS_UINT8                           aucAccessMode[2];
    VOS_UINT8                           ucReserve1;
    VOS_UINT8                           ucReserve2;
}NAS_MMA_NVIM_ACCESS_MODE_STRU;


typedef struct
{
    VOS_UINT8                           ucMsClass;
    VOS_UINT8                           ucReserved;
}NAS_NVIM_MS_CLASS_STRU;


typedef struct
{
    VOS_UINT16                          usRfAutoTestFlg;
}NAS_MMA_NVIM_RF_AUTO_TEST_FLAG_STRU;


typedef struct
{
    VOS_UINT8   ucHplmnTimerLen;
    VOS_UINT8   ucReserved1;
    VOS_UINT8   ucReserved2;
    VOS_UINT8   ucReserved3;
}NAS_NVIM_HPLMN_FIRST_TIMER_STRU;


typedef struct
{
    VOS_UINT8   aucSupported3GppRelease[NAS_MMC_NVIM_SUPPORTED_3GPP_RELEASE_SIZE];
}NAS_MMC_NVIM_SUPPORT_3GPP_RELEASE_STRU;


typedef struct
{
    VOS_UINT8   ucValid;
    VOS_UINT8   aucImsi[NAS_MMC_NVIM_MAX_IMSI_LEN];
    VOS_UINT8   ucReserved1;
    VOS_UINT8   ucReserved2;
}NAS_MMC_NVIM_LAST_IMSI_STRU;


typedef struct
{
    VOS_UINT8   aucRoamingBroker[2];
}NAS_MMA_NVIM_ROAMING_BROKER_STRU;


typedef struct
{
    VOS_UINT16  usUseSingleRplmnFlag;
    VOS_UINT8   ucReserved1;
    VOS_UINT8   ucReserved2;
}NAS_MMC_NVIM_USE_SINGLE_RPLMN_STRU;

/* en_NV_Item_EquivalentPlmn 8215 */

typedef struct
{
    VOS_UINT8    ucMcc[NVIM_MAX_MCC_SIZE];
    VOS_UINT8    ucMnc[NVIM_MAX_MNC_SIZE];
}NVIM_PLMN_VALUE_STRU;


typedef struct
{
    VOS_UINT8             ucCount;
    NVIM_PLMN_VALUE_STRU  struPlmnList[NVIM_MAX_EPLMN_NUM];
    VOS_UINT8             aucReserve[3];   /*NV����صĽṹ�壬��4�ֽڷ�ʽ�£����ֶ�����ն�*/
}NVIM_EQUIVALENT_PLMN_LIST_STRU;

/*en_NV_Item_Support_Freqbands 8229*/
/*
NVIM_UE_SUPPORT_FREQ_BAND_STRU�ṹ˵��:
usWcdmaBand��usGsmBand��Bitλ��ʾ�û����õ�Ƶ�Σ�bit1����Ƶ��I,bit2����Ƶ��II,
��������,����λΪ1,��ʾ֧�ָ�Ƶ��.�±��Ǳ���λ��Ƶ�ζ�Ӧ��ϵ:
-------------------------------------------------------------------------------
        bit8       bit7      bit6     bit5    bit4     bit3      bit2     bit1
-------------------------------------------------------------------------------
WCDMA   900(VIII)  2600(VII) 800(VI)  850(V)  1700(IV) 1800(III) 1900(II) 2100(I) oct1
        spare      spare     spare    spare   spare    spare     spare   J1700(IX)oct2
-------------------------------------------------------------------------------
GSMƵ�� 1900(VIII) 1800(VII) E900(VI) R900(V) P900(IV) 850(III)  480(II)  450(I)  oct3
        spare      spare     spare    spare   spare    spare     spare    700(IX) oct4
-------------------------------------------------------------------------------
aucUeSupportWcdmaBand��aucUeSupportGsmBand�������ʾUE֧�ֵ�Ƶ��,���Դ洢˳���
�Ⱥ��ʾƵ������˳��,��0xff��ʾ��Ч.

����:
oct1-oct4�ֱ��ǣ�0x03,0x00,0x7B,0x00
   ������û�����Ƶ��Ϊ��W��WCDMA-I-2100, WCDMA-II-1900
                         G��850(III),P900(IV),R900(V),E900(VI),1800(VII)
oct5-oct16�ֱ���:2,5,1,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff
   �����UE֧��WƵ��I,II,V,����˳����:II,V,I.
oct17-oct28�ֱ���:4,5,8,7,6,3,0xff,0xff,0xff,0xff,0xff,0xff
   �����UE֧��GƵ��III,IV,V,VI,VII,VIII,����˳����:IV,V,VIII,VII,VI,III.
*/

typedef struct
{
    VOS_UINT32                  ulWcdmaBand;
    VOS_UINT32                  ulGsmBand;
    VOS_UINT8                   aucReserved1[12];
    VOS_UINT8                   aucReserved2[12];
    VOS_UINT8                   aucReserved[24];        /* Ϊ��֤nv����һ�±��� */
}NVIM_UE_SUPPORT_FREQ_BAND_STRU;

/*en_NV_Item_Roam_Capa 8266*/

typedef struct
{
    VOS_UINT8                               ucRoamFeatureFlg;                   /*��¼���������Ƿ񼤻�,VOS_FALSE:������,VOS_TRUE:����*/
    VOS_UINT8                               ucRoamCapability;                   /*��¼�û����õ���������*/
    VOS_UINT8                               ucReserve1;
    VOS_UINT8                               ucReserve2;
}NAS_NVIM_ROAM_CFG_INFO_STRU;

/*en_NV_Item_CustomizeService 8271*/

typedef struct
{
    VOS_UINT32                          ulStatus;           /*�Ƿ񼤻0�����1���� */
    VOS_UINT32                          ulCustomizeService; /*�ն�˵������1��byte��Ϊ��û�пն��������4byte����3byte����*/
}NAS_NVIM_CUSTOMIZE_SERVICE_STRU;

/*en_NV_Item_RPlmnWithRat 8275*/

typedef struct
{
    NAS_NVIM_PLMN_ID_STRU               stGRplmn;                               /* G RPLMN��Ϣ */
    NAS_NVIM_PLMN_ID_STRU               stWRplmn;                               /* W RPLMN��Ϣ*/
    VOS_UINT8                           ucLastRplmnRat;                         /* �ϴιػ�ʱפ������Ľ��뼼��0:GSM;1:WCDMA;0xFF:��Чֵ */
    VOS_UINT8                           ucLastRplmnRatEnableFlg;                /* 0:NV δ����; 1:NV���� */
    VOS_UINT8                           aucReserved[2];                          /* ���� */
}NAS_NVIM_RPLMN_WITH_RAT_STRU;


typedef struct
{
    VOS_UINT8                            ucStatus;                              /* 0:NV δ����; 1:NV ���� */
    VOS_UINT8                            ucReserve;                             /* ����δ�� */
    VOS_UINT16                           usSrchHplmnTimerValue;                 /* VPLMN�·��״���HPLMNʱ��,��λ:���� */

}NAS_MMC_NVIM_SEARCH_HPLMN_TIMER_STRU;


typedef struct
{
    VOS_UINT8                           ucTriggerBGSearchFlag;  /* 0: mmc�յ������ȼ�PLMN����ʱ������BG SEARCH; 1:mmc�յ������ȼ�PLMN����ʱ����BG SEARCH */
    VOS_UINT8                           ucReserved;
    VOS_UINT16                          usSearchDelayLen;             /* BG SEARCH Delay ʱ��, ��λ: ��  */
}NAS_MMC_NVIM_HIGH_PRIO_PLMN_REFRESH_TRIGGER_BG_SEARCH_STRU;



/*en_NV_Item_EFust_Service_Cfg 8285*/

typedef struct
{
    VOS_UINT8                           ucStatus;                               /* NV���Ƿ񼤻�0��δ���1������ */
    VOS_UINT8                           ucForbidReg2GNetWork;                   /* 0��EFust GSM�����־������ȥʹ�ܡ��������������EFust 27��38������GSM���롣
                                                                                   �������ö�������UE����SIM���е�EFust 27��38λ��ȷ���Ƿ�����GSM���룬
                                                                                   ���EFUST�ļ���ֹ����λ��ͬʱ����Ϊ0�������ֹGSM����ע�ᣬ����GSM���Խ��롣
                                                                                   ע�⣺SIM���в�����EFust 27��38λ����˸ù��ܽ����USIM����Ч��1��EFust GSM�����־������ʹ�� */
    VOS_UINT8                           ucForbidSndMsg;                         /* 0��EFust���ŷ��ͱ�־������ȥʹ�ܡ��������������EFust 10��12������SMS���͡�
                                                                                   1��EFust���ŷ��ͱ�־������ʹ�ܡ�NV���ö�������UE����SIM����EFUST��10��12λ��
                                                                                   ȷ���Ƿ��ֹ���ŷ���,�����ֹ����λ��ͬʱ����Ϊ0�������ֹ����SMS�����򣬶��ſ��Է��͡�*/
    VOS_UINT8                           ucReserved[13];
}NAS_MMC_NVIM_EFUST_SERVICE_CFG_STRU;

/*en_NV_Item_UE_MSCR_VERSION 8289*/

typedef struct
{
    VOS_UINT8                           ucStatus;                               /* NV�Ƿ񼤻��־, 0: �����1: ���� */
    VOS_UINT8                           ucUeMscrVersion;                        /* �ն��ϱ���SGSN�汾 */
    VOS_UINT8                           ucReserve1;
    VOS_UINT8                           ucReserve2;
}NAS_MMC_NVIM_MSCR_VERSION_STRU;

/*en_NV_Item_UE_SGSNR_VERSION 8290*/

typedef struct
{
    VOS_UINT8                           ucStatus;                               /* NV�Ƿ񼤻��־, 0: �����1: ���� */
    VOS_UINT8                           ucUeSgsnrVersion;                       /* �ն��ϱ���SGSN�汾 */
    VOS_UINT8                           ucReserve1;
    VOS_UINT8                           ucReserve2;
}NAS_MMC_NVIM_SGSNR_VERSION_STRU;

/*en_NV_Item_SteeringofRoaming_SUPPORT_CTRL 8292*/

typedef struct
{
    VOS_UINT8                           ucStatus;                               /* NV�Ƿ񼤻��־, 0: �����1: ���� */
    VOS_UINT8                           ucCsRejSearchSupportFlg;                /* vodafone��������������,CS��ʧ���Ĵκ��·���������,VOS_FALSE:��֧��,VOS_TRUE:֧�� */
}NAS_MMC_NVIM_CS_REJ_SEARCH_SUPPORT_STRU;

/*en_NV_Item_Max_Forb_Roam_La 8320*/

typedef struct
{
    VOS_UINT8                           ucMaxForbRoamLaFlg;                     /* NV�����õ�����ֹLA�����Ƿ���Ч: VOS_TRUE:valid;VOS_FALSE:INVALID */
    VOS_UINT8                           ucMaxForbRoamLaNum;                     /* NV�����õ�����ֹLA���� */
    VOS_UINT8                           ucReserve1;
    VOS_UINT8                           ucReserve2;
}NAS_MMC_NVIM_MAX_FORB_ROAM_LA_STRU;

/*en_NV_Item_Default_Max_Hplmn_Srch_Peri 8321*/

typedef struct
{
    VOS_UINT8                           ucDefaultMaxHplmnPeriFlg;               /* Ĭ������HPLMN���������Ƿ���Ч��־,VOS_TRUE:valid;VOS_FALSE:INVALID */
    VOS_UINT8                           ucDefaultMaxHplmnTim;                   /* �û����Զ���Ĭ�ϵ�����HPLMN�������� */
    VOS_UINT8                           ucReserve1;
    VOS_UINT8                           ucReserve2;
}NAS_MMC_NVIM_DEFAULT_MAX_HPLMN_PERIOD_STRU;

/*en_NV_Item_USSD_Apha_To_Ascii 8327*/
/*����USSDת���ַ���*/

typedef struct
{
    VOS_UINT8                       ucStatus;            /*�Ƿ񼤻0�����1���� */
    VOS_UINT8                       ucAlphaTransMode;    /* �ַ���ת��*/
}NAS_SSA_NVIM_ALPHA_to_ASCII_STRU;

/*en_NV_Item_Register_Fail_Cnt 8338*/

typedef struct
{
    VOS_UINT8                           ucNvActiveFlag;                         /* ��ǰNV���Ƿ񼤻� */
    VOS_UINT8                           ucRegFailCnt;                           /* NV�����õ�ע��ʧ�ܵĴ�����Ĭ��ֵΪ2����ע��ʧ�����κ��������� */
    VOS_UINT8                           ucReserve1;
    VOS_UINT8                           ucReserve2;
}NAS_MMC_NVIM_REG_FAIL_CNT_STRU;

/*en_NV_Item_CREG_CGREG_CI_Four_Byte_Rpt 8345*/
/* VDF����: CREG/CGREG����<CI>���Ƿ���4�ֽ��ϱ���NV����ƽṹ�� */

typedef struct
{
    VOS_UINT8                           ucStatus;                        /* 1: NV��Ч��־λ��0����Ч */
    VOS_UINT8                           ucCiBytesRpt;                    /* <CI>���ϱ��ֽ�����־��0��2�ֽ��ϱ�, 1��4�ֽ��ϱ� */
    VOS_UINT8                           ucReserve1;
    VOS_UINT8                           ucReserve2;
}NAS_NV_CREG_CGREG_CI_FOUR_BYTE_RPT_STRU;


typedef struct
{
    VOS_UINT8                           ucEnableFlg;                            /* ���ű����Ƿ�ʹ�� VOS-TRUE:���ű���֧�� VOS_FALSE:���ű��ܲ�֧�� */
    VOS_UINT8                           ucReserve1;
    VOS_UINT8                           ucReserve2;
    VOS_UINT8                           ucReserve3;
}NAS_MMC_NVIM_DAIL_REJECT_CFG_STRU;

/* en_NV_Item_NDIS_DHCP_DEF_LEASE_TIME 8344 */
typedef struct
{
    VOS_UINT32                          ulDhcpLeaseHour;    /*Range:[0x1,0x2250]*/
}NDIS_NV_DHCP_LEASE_HOUR_STRU;

typedef struct
{
    VOS_UINT32                          ulIpv6Mtu;          /*Range:[1280,65535]*/
}NDIS_NV_IPV6_MTU_STRU;


typedef struct
{
    VOS_UINT8                           ucStatus;
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucReserved2;
    VOS_UINT8                           ucReserved3;
}NAS_NVIM_CCALLSTATE_RPT_STATUS_STRU;


typedef struct
{
   VOS_UINT8                            ucPlmnExactlyCompareFlag;               /* PLMN�Ƚ��Ƿ���о�ȷ�Ƚϵķ�ʽ�ı�� */
   VOS_UINT8                            aucRsv[3];                              /* ����λ */
}NVIM_PLMN_EXACTLY_COMPARE_FLAG_STRU;


enum PLATFORM_RAT_TYPE_ENUM
{
    PLATFORM_RAT_GSM,                                                       /*GSM���뼼�� */
    PLATFORM_RAT_WCDMA,                                                     /* WCDMA���뼼�� */
    PLATFORM_RAT_LTE,                                                       /* LTE���뼼�� */
    PLATFORM_RAT_TDS,                                                       /* TDS���뼼�� */
    PLATFORM_RAT_1X,                                                        /* CDMA-1X���뼼�� */
    PLATFORM_RAT_HRPD,                                                      /* CDMA-EV_DO���뼼�� */

    PLATFORM_RAT_BUTT
};
typedef VOS_UINT16 PLATFORM_RAT_TYPE_ENUM_UINT16;

#define PLATFORM_MAX_RAT_NUM            (7)                                    /* ���뼼�����ֵ */


typedef struct
{
    VOS_UINT16                           usRatNum;                              /* ���뼼������Ŀ*/
    PLATFORM_RAT_TYPE_ENUM_UINT16        aenRatList[PLATFORM_MAX_RAT_NUM];  /* ���뼼�� */
}PLATAFORM_RAT_CAPABILITY_STRU;

/*en_NV_Item_Rplmn 8216*/

typedef struct
{
    VOS_UINT8                           aucRplmnInfo[56];
}NAS_NVIM_RPLMN_INFO_STRU;


typedef struct
{
    VOS_UINT8                           ucSvlteSupportFlag;                     /* SVLTE�����Ƿ�֧��:0-��֧�֣�1-֧�� */
    VOS_UINT8                           aucReserved[3];                         /* ���� */
}SVLTE_SUPPORT_FLAG_STRU;



typedef struct
{
    VOS_UINT8                           ucNvimActiveFlag;                       /* 0: nv��δ���1:nv��� */
    VOS_UINT8                           ucCsPsMode1EnableLteTimerLen;           /* 1)cs ps mode1 L����ע��eps only�ɹ�cs����#16/#17/#22������������disable lteʱ����enable lte��ʱ��ʱ��
                                                                                   2)cs ps mode1 L����ע��cs eps��ʧ��ԭ��ֵother cause ������������disable lte����enable lte��ʱ��ʱ��,��λ:���ӣ� nv���ʱ���Ϊ0Ĭ��54���� */
    VOS_UINT8                           ucCsfbEmgCallEnableLteTimerLen;         /* L�½��������޷�����csfb��gu��ͨ��������gu����disable lte����enable lte��ʱ��ʱ������λ:���ӣ� nv���ʱ���Ϊ0Ĭ��5���� */

    /* ����ԭ��ֵΪ#16��#17��#18������ע��ɹ���Additional Update Result IE��Ϣ��
    ָʾSMS Only��CSFB Not Preferred��Ҫdisable lteʱ�����Э��汾���ڵ���R11��
    ���ñ��ܾ���PLMN��RAT��¼�ڽ�ֹ���뼼�������б��еĳͷ�ʱ�䣬���Ϊ0xFF��ʾ���óͷ�
    ��λ:���� */
    VOS_UINT8                           ucLteVoiceNotAvailPlmnForbiddenPeriod;
}NAS_MMC_NVIM_ENABLE_LTE_TIMER_LEN_STRU;



typedef struct
{
    VOS_UINT8                           ucActiveFlag;                           /* 0: nv��δ���1:nv��� */
    VOS_UINT8                           ucEnableLteTimerLen;                    /* EnableLte timer len, ��λ:���ӣ� nv���ʱ���Ϊ0��ʹ�� */
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucReserved2;
}NAS_MMC_NVIM_DCM_CUSTOM_DISABLE_LTE_CFG_STRU;


typedef struct
{
    VOS_UINT8                           ucCsfbMtForceAuthFlag;                  /* 0: nv��δ���1:nv��� */
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucReserved2;
    VOS_UINT8                           ucReserved3;
}NAS_MMC_NVIM_SMC_FAIL_CSFB_MT_FORCE_AUTH_CFG_STRU;



typedef struct
{
    VOS_UINT8                           ucT3402Flag;                           /* 0: ��ʹ��LMM_MMC_T3402_LEN_NOTIFY��Ϣ�еĳ���; 1:ʹ��LMM_MMC_T3402_LEN_NOTIFY��Ϣ�еĳ��� */
    VOS_UINT8                           ucHighPrioRatTimerNotEnableLteFlag;    /* 1:�����ȼ�RAT HPLMN TIMER ��ʱ������ENABLE lte��0: �����ȼ�RAT HPLMN TIMER ��ʱ����ENABLE lte */
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucReserved2;
}NAS_MMC_NVIM_DISABLE_LTE_START_T3402_ENABLE_LTE_CFG_STRU;




typedef struct
{
    VOS_UINT8                           ucRejMaxTimesDisableLte;          /* 0: ���ܴ�������ʱ������r12�汾disable lte; 1: ���ܴ�������ʱ����r12�汾disable lte */
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucReserved2;
    VOS_UINT8                           ucReserved3;
}NAS_MMC_NVIM_REJ_MAX_TIMES_DISABLE_LTE_CFG_STRU;



typedef struct
{
    /* ����24007 11.2.3.2.3.1.1
       after successful completion of SRVCC handover (see 3GPP TS 23.216 [27]),
       the mobile station shall perform modulo 4 arithmetic operations on V(SD).
       The mobile station shall keep using modulo 4 until the release of the RR
       connection established at SRVCC handover.

       During SRVCC handover the MSCR bit is not provided to the mobile station,
       and therefore the mobile station assumes to access to a Release 99 or
       later core network.
    */
    VOS_UINT8                           ucSrvccSnModuloCfg;                     /* 0: ����SN����SRVCCǰg_stMmNsd.ucNsdMod����; 1: ����Э�飬����SNͳһ����R99ģ4���� */
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucReserved2;
    VOS_UINT8                           ucReserved3;
}NAS_NVIM_SRVCC_SN_MODULO_CFG_STRU;


typedef struct
{
    VOS_UINT8                                               ucIsrSupport;       /* ISR ???? */
    VOS_UINT8                                               ucReserve1;
} NAS_NVIM_ISR_CFG_STRU;



typedef struct
{
    VOS_UINT8                                               ucEmcBarTriggerPlmnSearch;       /* VOS_FALSE: ������;  VOS_TRUE: ���� */
    VOS_UINT8                                               ucReserve1;
    VOS_UINT8                                               ucReserve2;
    VOS_UINT8                                               ucReserve3;
} NAS_NVIM_EMC_BAR_TRIGGER_PLMN_SEARCH_STRU;



typedef struct
{
    VOS_UINT8                           ucIsRauNeedFollowOnCsfbMtFlg;           /* Csfb mt�����У�RAU�Ƿ���Ҫ��follow on���:0-����Ҫ��1-��Ҫ */
    VOS_UINT8                           ucIsRauNeedFollowOnCsfbMoFlg;           /* Csfb mo�����У�RAU�Ƿ���Ҫ��follow on���:0-����Ҫ��1-��Ҫ */
    VOS_UINT8                           aucReserved[2];                         /* ���� */
}NAS_MMC_CSFB_RAU_FOLLOW_ON_FLAG_STRU;


typedef struct
{
   VOS_UINT16                           usSolutionMask;/*����ͨ������Modem����Ϣ��������ǿ�͵��������ԣ�Bitλ���Ƹ����������ԣ�bitn=0����n�����Թرգ�bitn=1����n�����Կ�����
                                                         Ŀǰֻ��bit0��bit1 ��Ч��
                                                         BIT0������˫Modem��ͨ����һModem��PLMN��Ϣ����FDD�����Ƿ������Ĳ����Ƿ�������
                                                         BIT1��ͨ��Modem1��GSM�ϱ���L��TDS������Ϣ, Modem0��֧��GSM������£�T/L�������ܹ����ݴ��ݵ�����Ƶ�����������TDS/LTE��
                                                               ���Modem1���ݹ�����������Ϣ�����ڵ�����£�Ҳ��ͨ����ʷƵ��֧��NCELL������������������Ƶ����ucSolution2NcellSearchTimer������
                                                         BIT2~BIT15:Ԥ��*/
   VOS_UINT8                            ucSolution2NcellQuickSearchTimer;       /*����Ƶ������������Ե�һ�׶�ʱ��������λ�룩��*/

   VOS_UINT8                            ucSolution2NcellQuickSearchTimer2;      /* ����Ƶ������������ԵĶ��׶�ʱ��������λ�룩��*/
   VOS_UINT8                            aucAdditonCfg[4];
}NV_DSDA_PLMN_SEARCH_ENHANCED_CFG_STRU;



typedef struct
{
    VOS_UINT8   ucRelFlg;
    VOS_UINT8   ucReserve;
}NV_NAS_GMM_REL_CONN_AFTER_PDP_DEACT_STRU;


typedef struct
{
    VOS_UINT32                          ulWband;                                /* ֧�ֵ�WCDMA����ͨ· */
    VOS_UINT32                          ulWbandExt;                             /* ֧�ֵ�WCDMA����ͨ·��չ�ֶ� */
    VOS_UINT32                          ulGband;                                /* ֧�ֵ�GSM����ͨ· */
}NAS_NVIM_WG_RF_MAIN_BAND_STRU;



typedef struct
{
    VOS_UINT8                           ucImsVoiceInterSysLauEnable;           /* ISR�������ϵͳ��L�任��GU��LAIδ�ı䣬�Ƿ���Ҫǿ��LAU */
    VOS_UINT8                           ucImsVoiceMMEnable;         /* IMS�ƶ��Թ��� NV */
    VOS_UINT8                           aucReserved[2];             /* ���� */
}NAS_MMC_IMS_VOICE_MOBILE_MANAGEMENT;


typedef struct
{
    VOS_UINT8                           ucLDisabledRauUseLInfoFlag;             /* l disabled��rau�Ƿ���Ҫ��l��ȡ��ȫ�����Ļ�gutiӳ����Ϣ��vos_true:��Ҫ��ȡ��vos_false:�����ȡ*/
    VOS_UINT8                           ucReserved[3];
}NAS_MMC_LTE_DISABLED_USE_LTE_INFO_FLAG_STRU;



typedef struct
{
    VOS_UINT8                           ucActiveFlg;                            /* �Ƿ񼤻�� */
    VOS_UINT8                           ucCsOnlyDataServiceSupportFlg;          /* PSע�ᱻ��ֹ����£��Ƿ���������ҵ�񴥷�ע��ı�־ */
}NAS_MML_CS_ONLY_DATA_SERVICE_SUPPORT_FLG_STRU;

typedef struct
{
    VOS_UINT8                           ucActiveFLg;                             /* �ö�ʱ���Ƿ�ʹ�� */                       /* TD��ʼ�����ѵĴ��� */
    VOS_UINT8                           aucRsv[3];
    VOS_UINT32                          ulFirstSearchTimeLen;                   /* high prio rat timer��ʱ����һ�ε�ʱ�� ��λ:�� */
    VOS_UINT32                          ulFirstSearchTimeCount;                 /* high prio rat timer��ʱ����һ��ʱ���������������� */
    VOS_UINT32                          ulNonFirstSearchTimeLen;                /* high prio rat timer��ʱ�����״ε�ʱ�� ��λ:�� */
    VOS_UINT32                          ulRetrySearchTimeLen;                   /* high prio rat �ѱ���ֹ���������������Ե�ʱ�� ��λ:��*/
}NAS_MMC_NVIM_HIGH_PRIO_RAT_HPLMN_TIMER_INFO_STRU;


typedef struct
{
    VOS_UINT8                           ucActiveFLg;                            /* �ö�ʱ���Ƿ�ʹ�� */
    VOS_UINT8                           ucTdThreshold;                          /* TD��ʼ�����ѵĴ��� */
    VOS_UINT8                           aucRsv[2];
    VOS_UINT32                          ulFirstSearchTimeLen;                   /* high prio rat timer��ʱ����һ�ε�ʱ�� ��λ:�� */
    VOS_UINT32                          ulFirstSearchTimeCount;                 /* high prio rat timer��ʱ����һ��ʱ���������������� */
    VOS_UINT32                          ulNonFirstSearchTimeLen;                /* high prio rat timer��ʱ�����״ε�ʱ�� ��λ:�� */
    VOS_UINT32                          ulRetrySearchTimeLen;                   /* high prio rat �ѱ���ֹ���������������Ե�ʱ�� ��λ:��*/
}NAS_MMC_NVIM_HIGH_PRIO_RAT_HPLMN_TIMER_CFG_STRU;



typedef struct
{
    VOS_UINT8                           ucUltraFlashCsfbSupportFLg;                 /* �Ƿ�֧��ultra flash csfb */
    VOS_UINT8                           aucRsv[3];
}NAS_MMC_NVIM_ULTRA_FLASH_CSFB_SUPPORT_FLG_STRU;


typedef struct
{
    VOS_UINT8                           uc3GPP2UplmnNotPrefFlg;                    /* �Ƿ���3GPP2 pref plmn */
    VOS_UINT8                           aucRsv[3];
}NAS_MMC_NVIM_3GPP2_UPLMN_NOT_PREF_STRU;


typedef struct
{
    VOS_UINT8                           ucHighPrioRatPlmnSrchFlg;                  /* �Ƿ��������ȼ����뼼������ */
    VOS_UINT8                           aucReserved1[3];
}NAS_MMC_NVIM_SYSCFG_TRIGGER_PLMN_SEARCH_CFG_STRU;


typedef struct
{
    VOS_UINT32                           ulCsRegEndSessionDelayTime;              /* Lau�ӳ�ʱ������λ:���� */
    VOS_UINT32                           ulPsRegEndSessionDelayTime;              /* Rau�ӳ�ʱ������λ:���� */
    VOS_UINT32                           ulCsfbCallEndSessionDelayTime;               /* CSFB�ӳ�ʱ������λ:���� */
    VOS_UINT32                           ulReserve2;
} NAS_MMC_NVIM_DSDS_END_SESSION_DELAY_STRU;



typedef struct
{
    VOS_UINT32                           ulCsRegEndSessionDelayTime;            /* Lau�ӳ�ʱ������λ:���� */
    VOS_UINT32                           ulPsRegEndSessionDelayTime;            /* Rau�ӳ�ʱ������λ:���� */
    VOS_UINT32                           ulCsfbCallEndSessionDelayTime;         /* CSFB�ӳ�ʱ������λ:���� */
    VOS_UINT32                           ulCsCallEndSessionDelayTime;           /* CS CALL�ӳ�ʱ������λ:���� */
    VOS_UINT32                           ulCsSmsEndSessionDelayTime;            /* CS SMS�ӳ�ʱ������λ:���� */
    VOS_UINT32                           ulCsSsEndSessionDelayTime;             /* CS SS�ӳ�ʱ������λ:���� */
    VOS_UINT32                           ulCsRegEnableRfOccupyDelayTime;        /* LAU�ӳ��ͷ���ռ����ʱ������λ:���� */
    VOS_UINT32                           ulReserve1;
    VOS_UINT32                           ulReserve2;
    VOS_UINT32                           ulReserve3;
    VOS_UINT32                           ulReserve4;
} NAS_MMC_NVIM_DSDS_DELAY_TIME_STRU;


typedef struct
{
    VOS_UINT16                                              usSid;
    VOS_UINT16                                              usNid;
    VOS_UINT16                                              usBandClass;
    VOS_UINT16                                              usChannel;
}CNAS_NVIM_1X_SYSTEM_STRU;


typedef struct
{
    VOS_UINT8                           ucSysNum;
    VOS_UINT8                           aucReserve[3];
    CNAS_NVIM_1X_SYSTEM_STRU            astSystem[CNAS_NVIM_MAX_1X_MRU_SYS_NUM];
}CNAS_NVIM_1X_MRU_LIST_STRU;



typedef struct
{
    VOS_UINT8                           ucReadNvPrlDirectly;
    VOS_UINT8                           ucReadDefaultPrl;        /* ��ȡDefault Prl */
    VOS_UINT8                           ucIsMod1xAvailTimerLen;
    VOS_UINT8                           ucNvPrlCombinedFlag;     /* NV PRL������־ */
    VOS_UINT8                           aucReserve[12];

}CNAS_NVIM_TEST_CONFIG_STRU;


typedef struct
{
    VOS_UINT32                          ulCallBackEnableFlg;
    VOS_UINT32                          ulCallBackModeTimerLen;
} CNAS_NVIM_1X_CALLBACK_CFG_STRU;


typedef struct
{
    VOS_UINT8                           ucIsAllowCallInForeign;
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucReserved2;
    VOS_UINT8                           ucReserved3;
}NAS_MSCC_NVIM_CTCC_ROAM_EMC_CALL_CFG_STRU;


typedef struct
{
    VOS_UINT16                          usRejCauseBegin;                        /* RejCause begin */
    VOS_UINT16                          usRejCauseEnd;                          /* RejCause end */
    VOS_UINT32                          ulRptInterval;                          /* ���ϱ�ʱ���� */
}NAS_NVIM_REJ_INFO_GROUP_CFG_STRU;


typedef struct
{
    VOS_UINT8                           ucEnableFlg;                                                          /* RejInfo ���ϱ�ʹ�� */
    VOS_UINT8                           ucAvailNum;                                                           /* ��Ч�������� */
    VOS_UINT8                           ucReserved1;                                                          /* ����λ1 */
    VOS_UINT8                           ucReserved2;                                                          /* ����λ2 */
    NAS_NVIM_REJ_INFO_GROUP_CFG_STRU    astRejInfoGroupCfg[NAS_NVIM_MAX_REJ_INFO_NOT_RPT_GROUP_NUM];          /* 8�����ã�CauseBegin CauseEnd ���ϱ�ʱ���� */
}NAS_NVIM_REJ_INFO_NOT_RPT_CFG_STRU;


typedef struct
{
    VOS_UINT8                           ucIsNegSysAdd;
    VOS_UINT8                           aucReserve[15];
}CNAS_NVIM_1X_ADD_AVOID_LIST_CFG_STRU;


typedef struct
{
    CNAS_NVIM_1X_NEG_PREF_SYS_CMP_TYPE_ENUM_UINT8           enNegPrefSysCmpType;
    VOS_UINT8                                               aucReserve[15];
}CNAS_NVIM_1X_NEG_PREF_SYS_CMP_CTRL_STRU;


typedef struct
{
    VOS_UINT8                                               ucIsL3ErrReOrigCount;
    VOS_UINT8                                               ucPrivacyMode;      /* privacy mode flag: 0 - disable 1 - enable */

    VOS_UINT8                                               aucReserve[14];
}CNAS_NVIM_1X_CALL_NVIM_CFG_STRU;


/*****************************************************************************
 �ṹ��    : CNAS_NVIM_1X_SUPPORT_BANDCLASS_MASK_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : �ն�֧�ֵ�Ƶ����������  3601
*****************************************************************************/
typedef struct
{
    VOS_UINT32                          ulSupportBandclassMask;               /* �ն�֧�ֵ�Ƶ���������룬ÿbit��ʾ�Ƿ�֧�ֶ�Ӧ��Ƶ������������0x00000001��ʾֻ֧��Ƶ��0 */
}CNAS_NVIM_1X_SUPPORT_BANDCLASS_MASK_STRU;

/*****************************************************************************
 �ṹ��    : CNAS_NVIM_HRPD_SUPPORT_BANDCLASS_MASK_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : �ն�֧�ֵ�Ƶ����������  3601
*****************************************************************************/
typedef struct
{
    VOS_UINT32                          ulSupportBandclassMask;               /* �ն�֧�ֵ�Ƶ���������룬ÿbit��ʾ�Ƿ�֧�ֶ�Ӧ��Ƶ������������0x00000001��ʾֻ֧��Ƶ��0 */
}CNAS_NVIM_HRPD_SUPPORT_BANDCLASS_MASK_STRU;


typedef struct
{
    VOS_UINT16                          ausAvoidTimerLen[CNAS_NVIM_HRPD_AVOID_MAX_PHASE_NUM];
}CNAS_NVIM_HRPD_AVOID_PHASE_STRU;


typedef struct
{
    CNAS_NVIM_HRPD_AVOID_PHASE_STRU     astAvoidPhaseNum[CNAS_NVIM_HRPD_AVOID_REASON_MAX];
}CNAS_NVIM_HRPD_AVOID_SCHEDULE_INFO_STRU;

/*****************************************************************************
�ṹ����    :NAS_MMC_NVIM_ADAPTION_CAUSE_STRU
ʹ��˵��    :�û���������ԭ������ݽṹ
*****************************************************************************/
typedef struct
{
    VOS_UINT8                           ucCnCause;     /* ����ԭ��ֵ */
    VOS_UINT8                           ucHplmnCause;  /* �û�����ƥ��HPLMN��ԭ��ֵ */
    VOS_UINT8                           ucVplmnCause;  /* �û�����ƥ��VPLMN��ԭ��ֵ */
    VOS_UINT8                           aucReserved[1];
}NAS_MMC_NVIM_ADAPTION_CAUSE_STRU;

/*****************************************************************************
�ṹ����    :NAS_MMC_NVIM_CHANGE_NW_CAUSE_CFG_STRU
ʹ��˵��    :en_NV_Item_ChangeNWCause_CFG NV��ṹ
*****************************************************************************/
typedef struct
{
    /* CS��ע������(LU)�ܾ�ԭ��ֵ�滻��Ϣ */
    VOS_UINT8                           ucCsRegCauseNum;
    VOS_UINT8                           aucReserved1[3];
    NAS_MMC_NVIM_ADAPTION_CAUSE_STRU    astCsRegAdaptCause[NAS_MMC_NVIM_MAX_CAUSE_NUM];

    /* PS��ע������(ATTACH/RAU)�ܾ�ԭ��ֵ�滻��Ϣ */
    VOS_UINT8                           ucPsRegCauseNum;
    VOS_UINT8                           aucReserved2[3];
    NAS_MMC_NVIM_ADAPTION_CAUSE_STRU    astPsRegAdaptCause[NAS_MMC_NVIM_MAX_CAUSE_NUM];

    /* ����GPRS Detach ���ܾ̾�ԭ��ֵ�滻��Ϣ */
    VOS_UINT8                           ucDetachCauseNum;
    VOS_UINT8                           aucReserved3[3];
    NAS_MMC_NVIM_ADAPTION_CAUSE_STRU    astDetachAdaptCause[NAS_MMC_NVIM_MAX_CAUSE_NUM];

    /* GMM service request���ܾ̾�ԭ��ֵ�滻��Ϣ */
    VOS_UINT8                           ucPsSerRejCauseNum;
    VOS_UINT8                           aucReserved4[3];
    NAS_MMC_NVIM_ADAPTION_CAUSE_STRU    astPsSerRejAdaptCause[NAS_MMC_NVIM_MAX_CAUSE_NUM];

    /* MM ABORT���ܾ̾�ԭ��ֵ�滻��Ϣ */
    VOS_UINT8                           ucMmAbortCauseNum;
    VOS_UINT8                           aucReserved5[3];
    NAS_MMC_NVIM_ADAPTION_CAUSE_STRU    astMmAbortAdaptCause[NAS_MMC_NVIM_MAX_CAUSE_NUM];

    /* CM Service���ܾ̾�ԭ��ֵ�滻��Ϣ */
    VOS_UINT8                           ucCmSerRejCauseNum;
    VOS_UINT8                           aucReserved6[3];
    NAS_MMC_NVIM_ADAPTION_CAUSE_STRU    astCmSerRejAdaptCause[NAS_MMC_NVIM_MAX_CAUSE_NUM];

    VOS_UINT8                           ucHplmnPsRejCauseChangTo17MaxNum; /* HPLMN PS/EPS��ܾ�ԭ��ֵ�޸�Ϊ#17�������� */
    VOS_UINT8                           ucHplmnCsRejCauseChangTo17MaxNum; /* HPLMN CS��ܾ�ԭ��ֵ�޸�Ϊ#17�������� */
    VOS_UINT8                           ucVplmnPsRejCauseChangTo17MaxNum; /* VPLMN PS/EPS��ܾ�ԭ��ֵ�޸�Ϊ#17�������� */
    VOS_UINT8                           ucVplmnCsRejCauseChangTo17MaxNum; /* VPLMN CS��ܾ�ԭ��ֵ�޸�Ϊ#17�������� */
}NAS_MMC_NVIM_CHANGE_NW_CAUSE_CFG_STRU;


typedef struct
{
    VOS_UINT8                           ucRelPsSignalConFlg;/* �Ƿ�������������������� */

    VOS_UINT8                           ucPdpExistNotStartT3340Flag; /* rau��attach���󲻴�follow on������ظ�attach accept��rau acceptҲ����follow on������pdp�����ĳ����Ƿ���Ҫ����T3340,0:��Ҫ����T3340; 1:����Ҫ���� */
    VOS_UINT8                           aucReserved[2];

    VOS_UINT32                          ulT3340Len;         /* ���õ�GMM T3340��ʱ��,��λ:�� */
}NAS_MMC_NVIM_REL_PS_SIGNAL_CON_CFG_STRU;


typedef struct
{
    VOS_UINT8   ucEnableFlag;/*ʹ�ܿ��� */
    VOS_UINT8   aucReserve[3];
    VOS_UINT32  ulMcc;    /*���ڲ���ʹ��ָ��MCC*/
    VOS_UINT32  ulMnc;    /*���ڲ���ʹ��ָ��MNC*/
}NAS_RABM_NVIM_WCDMA_VOICE_PREFER_STRU;

typedef struct
{
    VOS_UINT16                              usEnable;                          /* ȫ��ͨ���Կ��� */
    VOS_UINT16                              usReserved;
}NAS_NV_TRI_MODE_ENABLE_STRU;


typedef struct
{
    VOS_UINT32                              ulProfileId;                        /* ����ʹ�ó���������ǰ�����������µ磨ABB��TCXO��RF���Լ�RFͨ���Ŀ��ơ�
                                                                                   ��AT�����·����á�Ĭ��ֵΪ0��ȡֵ��Χ0-7�� */
    VOS_UINT32                              ulReserved[3];                     /* ������������չʹ�� */
}NAS_NV_TRI_MODE_FEM_PROFILE_ID_STRU;




typedef struct
{
    VOS_UINT16                              usABBSwitch;                       /* ����ABB PLL���صĿ��ơ�0:ABB CH0 1:ABB CH1 2:ABB CH0&CH1���� */
    VOS_UINT16                              usRFSwitch;                        /* ����RFIC��Դ���صĿ��ơ�0:RFICʹ��MIPI0���ƹ��緽ʽ 1��RFICʹ��MIPI1���ƹ��緽ʽ 2��ͬʱ����·��Դ��*/
    VOS_UINT16                              usTCXOSwitch;                      /* 0:TCXO0 1:TCXO1 */
    VOS_UINT16                              usReserved;                        /* ������������չʹ�� */
}NAS_NV_MODE_BASIC_PARA_STRU;

/*****************************************************************************
 �ṹ��    : NV_TRI_MODE_CHAN_PARA_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : ȫ��ͨͨ���������ò���  (���ܸı�)
*****************************************************************************/
typedef struct
{
    NAS_NV_MODE_BASIC_PARA_STRU            stModeBasicPara[2];                  /* �±�[0]:��ʾGSMģʽ�µ�ǰ�����������µ���ơ�
                                                                                   �±�[1]:��ʾWCDMAģʽ�µ�ǰ�����������µ���ơ�
                                                                                    ע������ʱ����ʹ��WCDMAģʽ�����á�*/
    VOS_UINT32                              ulRfSwitch;                         /* ���ڿ��ƹ��ּ��Ŀ��� */
    VOS_UINT32                              ulGsmRficSel;                       /* ����ģʽ�µ�ǰʹ�õ�ͨ����0��RF0,1��RF1�� */
    VOS_UINT32                              ulGpioCtrl;                         /* gpio */
    VOS_UINT32                              aulReserved[14];                    /* ������������չʹ�� */
}NAS_NV_TRI_MODE_CHAN_PARA_STRU;

/*****************************************************************************
 �ṹ��    : NAS_NV_TRI_MODE_FEM_CHAN_PROFILE_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : 8�ֳ�����ͨ������
*****************************************************************************/
typedef struct
{
    NAS_NV_TRI_MODE_CHAN_PARA_STRU          stPara[NAS_NV_TRI_MODE_CHAN_PARA_PROFILE_NUM];  /* ���֧��8������������ */
}NAS_NV_TRI_MODE_FEM_CHAN_PROFILE_STRU;






typedef struct
{
    VOS_UINT8                           ucCsmoSupportedFlg;
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucReserved2;
    VOS_UINT8                           ucReserved3;
}NAS_MMC_NVIM_CSMO_SUPPORTED_CFG_STRU;



typedef struct
{
    VOS_UINT8                           ucHplmnInEplmnDisplayHomeFlg;
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucReserved2;
    VOS_UINT8                           ucReserved3;
}NAS_MMC_NVIM_ROAM_DISPLAY_CFG_STRU;


typedef struct
{
    VOS_UINT16                          usMtCsfbPagingProcedureLen;
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucReserved2;
}NAS_MMC_NVIM_PROTECT_MT_CSFB_PAGING_PROCEDURE_LEN_STRU;


typedef struct
{
    VOS_UINT8                           ucRoamPlmnSelectionSortFlg;
    VOS_UINT8                           ucSrchUOplmnPriorToDplmnFlg;
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucReserved2;
}NAS_MMC_NVIM_ROAM_PLMN_SELECTION_SORT_CFG_STRU;



typedef struct
{
    VOS_UINT16                          usTotalTimerLen;                        /* ���׶�������ʱ��,��λ:s */
    VOS_UINT16                          usSleepTimerLen;                        /* ���������˯��ʱ��,��λ:s */
    VOS_UINT16                          usReserve1;
    VOS_UINT16                          usReserve2;
    VOS_UINT8                           ucHistoryNum;                           /* �ڼ��ε���ʷ�� ���� PrefBand/FullBand�� */
    VOS_UINT8                           ucPrefBandNum;                          /* �ڼ��ε�PrefBand�� ���� FullBand�� */
    VOS_UINT8                           ucFullBandNum;                          /* �ڼ���FullBand����, �˽׶ν���, ������һ�׶� */
    VOS_UINT8                           ucReserve1;
    VOS_UINT8                           ucReserve2;
    VOS_UINT8                           ucReserve3;
    VOS_UINT8                           ucReserve4;
    VOS_UINT8                           ucReserve5;
}NAS_MMC_NVIM_OOS_PLMN_SEARCH_PATTERN_CFG_STRU;


typedef struct
{
    NAS_MMC_NVIM_OOS_PLMN_SEARCH_PATTERN_CFG_STRU           stPhaseOnePatternCfg;    /* ��һ�׶ε���������������˯��ʱ�� */
    NAS_MMC_NVIM_OOS_PLMN_SEARCH_PATTERN_CFG_STRU           stPhaseTwoPatternCfg;    /* �ڶ��׶ε���������������˯��ʱ�� */
    NAS_MMC_NVIM_OOS_PLMN_SEARCH_PATTERN_CFG_STRU           stPhaseThreePatternCfg;  /* �����������׶ε���������������˯��ʱ�� */
    NAS_MMC_NVIM_OOS_PLMN_SEARCH_PATTERN_CFG_STRU           stPhaseFourPatternCfg;   /* ���������Ľ׶ε���������������˯��ʱ�� */
}NAS_MMC_NVIM_OOS_PLMN_SEARCH_STRATEGY_CFG_STRU;


typedef struct
{
    VOS_UINT8                           ucIsSupportSkipBandTypeSearch;                              /* �Ƿ�֧������band���͵����� */
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucReserved2;
    VOS_UINT8                           ucMccNum;                                                   /* ֧������band���������Ĺ�������� */
    VOS_UINT32                          aulMccList[NAS_MML_NVIM_MAX_SKIP_BAND_TYPE_SEARCH_MCC_NUM]; /* ֧������band���������Ĺ����� */
}NAS_MMC_NVIM_SKIP_BAND_TYPE_PLMN_SEARCH_CFG_STRU;


typedef struct
{
    VOS_UINT8           ucPrefbandCfg;
    VOS_UINT8           ucSearchTypeAfterHistoryInAreaLostScene;
    VOS_UINT8           ucSearchTypeAfterGetGeoFail;
    VOS_UINT8           ucReserved1;

    VOS_UINT32          ulHistoryCfg;
    VOS_UINT32          ulReserved2;
}NAS_MMC_NVIM_NON_OOS_PLMN_SEARCH_FEATURE_SUPPORT_CFG_STRU;



typedef struct
{
    VOS_UINT32                          ulDelayNetworkSearchTimerLenAfterEmc;   /* ������֮������ʱ������λ:s��ʱ��Ϊ0��ʾҪ���� */
    VOS_UINT8                           ucBgHistorySupportFlg;                  /* BG��ʽhistory���Ƿ�֧�� */
    VOS_UINT8                           ucPrefBandListSupportFlg;               /* �б�ʽpref band���Ƿ�֧�� */
    VOS_UINT8                           ucFullBandListSupportFlg;               /* �б�ʽfull band���Ƿ�֧�� */
    VOS_UINT8                           ucRsv1;
    VOS_UINT8                           ucRsv2;
    VOS_UINT8                           ucRsv3;
    VOS_UINT8                           ucRsv4;
    VOS_UINT8                           ucRsv5;
}NAS_MMC_NVIM_OOS_BG_NETWORK_SEARCH_CUSTOM_STRU;


typedef struct
{
    VOS_UINT8                           ucExtendT3240LenFlg;                    /* �Ƿ���T3240ʱ���Ķ��� 0:δ���� 1:���� */
    VOS_UINT8                           ucReserve;                              /* ����λ */
    VOS_UINT16                          usCustomizedT3240Len;                   /* ���Ƶ�T3240��ʱ��ʱ������λΪ���룬���֧��65535���� */
}NAS_NVIM_EXTEND_T3240_LEN_CFG_STRU;


typedef struct
{
    VOS_UINT8                           ucRatOrderNum;                          /* ��ȡ����λ����Ϣ�Ľ��뼼������ */
    VOS_UINT8                           aucRatOrder[NAS_NVIM_MAX_RAT_NUM];      /* ��ȡ����λ����Ϣ�Ľ��뼼�����ȼ� */
}NAS_NVIM_GET_GEO_PRIO_RAT_LIST_STRU;


typedef struct
{
    VOS_UINT8                                               ucGetGeoFlag;   /* ʹ�ñ���λ��Ǹ��������Ƿ���GET GEO; ���λ��ʾ�ϵ��Ƿ�GET GEO; �ε�λ��ʾ����ģʽ�Ƿ�GET GEO; ��3��λ��ʾOOC�Ƿ�GET GEO
                                                                                        ����: 0x07, ��ʾ�������г�����GET GEO */
    VOS_UINT8                                               ucGetGeoTimerlen;               /* ��ȡ�����붨ʱ��ʱ������λ���� */
    VOS_UINT8                                               ucScanTypeOfFlightModeGetGeo;   /* ����ģʽGET GEOʱFFTɨƵ����*/
    VOS_UINT8                                               ucScanTypeOfOocGetGeo;          /* OOC GET GEOʱFFTɨƵ����*/
    VOS_UINT32                                              ulGeoEffectiveTimeLen;          /* ���������Ч��ʱ������λ�Ƿ��� */
    NAS_NVIM_GET_GEO_PRIO_RAT_LIST_STRU                     stGetGeoPrioRatList;            /* ��ȡ����λ����Ϣ�Ľ��뼼�����ȼ��б���Ϣ */
}NAS_NVIM_GET_GEO_CFG_INFO_STRU;


typedef struct
{
    VOS_UINT8                           ucLowPrioAnycellSearchLteFlg;
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucReserved2;
    VOS_UINT8                           ucReserved3;
}NAS_NVIM_LOW_PRIO_ANYCELL_SEARCH_LTE_FLG_STRU;


typedef struct
{
    VOS_UINT8                           ucDeleteRplmnFlg;
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucReserved2;
    VOS_UINT8                           ucReserved3;
}NAS_NVIM_REFRESH_RPLMN_WHEN_EPLMN_INVALID_CFG_STRU;


typedef struct
{
    VOS_UINT8                                               ucActiveFlag;       /* NV���Ƿ񼤻� */
    VOS_UINT8                                               ucReserved1;
    VOS_UINT8                                               ucReserved2;
    VOS_UINT8                                               ucReserved3;
    VOS_UINT32                                              ulRecordNum;        /* ��¼�Ĵ��� */
}NAS_NVIM_NW_SEARCH_CHR_RECORD_CFG_STRU;


typedef struct
{
    VOS_UINT8                           ucPrlData[CNAS_NVIM_PRL_SIZE];
}CNAS_NVIM_1X_EVDO_PRL_LIST_STRU;


typedef struct
{
    VOS_UINT8                           ucPrlData[CNAS_NVIM_CBT_PRL_SIZE];
}CNAS_NVIM_CBT_PRL_LIST_STRU;


typedef struct
{
    VOS_UINT16                          usSid;
    VOS_UINT16                          usNid;
    VOS_UINT16                          usBand;
    VOS_UINT16                          usReserved;
}CNAS_NVIM_1X_HOME_SID_NID_STRU;


typedef struct
{
    VOS_UINT8                           ucSysNum;
    VOS_UINT8                           aucReserve[3];
    CNAS_NVIM_1X_HOME_SID_NID_STRU      astHomeSidNid[CNAS_NVIM_MAX_1X_HOME_SID_NID_NUM];
}CNAS_NVIM_1X_HOME_SID_NID_LIST_STRU;


typedef struct
{
    VOS_UINT16                          usTimes;
    VOS_UINT16                          usTimerLen;
}CNAS_NVIM_OOC_TIMER_INFO_STRU;


typedef struct
{
    VOS_UINT8                           ucMru0SearchTimerLen;
    VOS_UINT8                           ucPhaseNum;
    VOS_UINT8                           uc1xOocDoTchPhase1TimerLen;              /* Do TCH��ǰ4�γ��� Ooc Timer ���ʱ�� */
    VOS_UINT8                           uc1xOocDoTchPhase2TimerLen;              /* Do TCH��4�����ϳ��� Ooc Timer ���ʱ�� */
    CNAS_NVIM_OOC_TIMER_INFO_STRU       astOocTimerInfo[CNAS_NVIM_MAX_OOC_SCHEDULE_PHASE_NUM];
}CNAS_NVIM_OOC_TIMER_SCHEDULE_INFO_STRU;

typedef struct
{
    VOS_UINT8                                               ucInsertOrigChanFlg;        /* ��ͬ������ǰ�Ƿ������ǰפ��Ƶ�� */
    NAS_NVIM_CHAN_REPEAT_SCAN_ENUM_UINT8                    enChanRepeatScanStrategy;   /*Ƶ���ظ��������� */
    VOS_UINT8                                               aucReserved[2];
}CNAS_NVIM_OOC_REPEAT_SCAN_STRATEGY_INFO_STRU;


typedef struct
{
    NAS_NVIM_EPDSZID_SUPPORT_TYPE_ENUM_UINT8                ucEpdszidType;  /* EPDSZID֧������ */
    VOS_UINT8                                               ucHatLen;       /* HATʱ������λs */
    VOS_UINT8                                               ucHtLen;        /* HTʱ������λs */
    VOS_UINT8                                               aucReserved[5];
}CNAS_NVIM_1X_EPDSZID_FEATURE_CFG_STRU;


typedef struct
{
    VOS_UINT8                          aucSubnet[CNAS_NVIM_HRPD_SUBNET_LEN];
    VOS_UINT16                         usBandClass;
    VOS_UINT16                         usChannel;
}CNAS_NVIM_HRPD_SYSTEM_STRU;


typedef struct
{
    VOS_UINT8                           ucSysNum;
    VOS_UINT8                           aucRsv[3];
    CNAS_NVIM_HRPD_SYSTEM_STRU          astSystem[CNAS_NVIM_MAX_HRPD_MRU_SYS_NUM];
}CNAS_NVIM_HRPD_MRU_LIST_STRU;


typedef struct
{
    VOS_UINT32                          ulSicValue;
}CNAS_NVIM_1X_LAST_SCI_STRU;


typedef struct
{
    VOS_UINT8                           ucNvimActiveFlag;                       /* 0: nv��δ���1:nv��� */
    VOS_UINT8                           ucWaitImsVoiceAvailTimerLen;            /* �ȴ�IMS VOICE�Ŀ���ָʾ�Ķ�ʱ��ʱ��,��λΪ�뼶,��Ҫת��Ϊ���� */
    VOS_UINT8                           ucWaitImsWithWifiVoiceAvailTimerLen;    /* ֧��IMS WIFIʱ,�ȴ�IMS VOICE�Ŀ���ָʾ�Ķ�ʱ��ʱ��,��λΪ�뼶,��Ҫת��Ϊ���� */
    VOS_UINT8                           ucReserved;
}NAS_NVIM_WAIT_IMS_VOICE_AVAIL_TIMER_LEN_STRU;

typedef struct
{
    VOS_UINT32                          ulIsValid;
    VOS_UINT8                           ucLocType;
    VOS_UINT8                           ucLocLen;
    VOS_UINT16                          usSID;
    VOS_UINT16                          usNID;
    VOS_UINT8                           ucPacketZoneID;
    VOS_UINT8                           ucReserve;
}CNAS_NVIM_HRPD_LOC_INFO_STRU;


typedef struct
{
    VOS_UINT32                          ulIsValid;
    VOS_UINT16                          usStrgBLOBType;
    VOS_UINT8                           ucStrgBLOBLen;
    VOS_UINT8                           aucStorageBLOB[CNAS_NVIM_MAX_STORAGE_BLOB_LEN];
    VOS_UINT8                           aucReserve[2];
}CNAS_NVIM_HRPD_STORAGE_BLOB_STRU;


typedef struct
{
    VOS_UINT8                           ucLteRejCause14Flg;               /* �Ƿ���LTE #14ԭ��ܾ��Ż���0: δ������1:���� */
    VOS_UINT8                           aucReserved[1];
    VOS_UINT16                          usLteRejCause14EnableLteTimerLen; /* LTE #14ԭ��ܾ�ʱ��ͨ��������gu����disable lte����enable lte��ʱ��ʱ������λ:���� */
}NAS_MMC_NVIM_LTE_REJ_CAUSE_14_CFG_STRU;


typedef struct
{
    VOS_UINT8                           ucIsT3396RunningDisableLteFlg;            /* LMM->MMC ATTACH IND���T3396��ʱ�������Ƿ���Ҫdisable lte�����ã�0: δ������1:���� */
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucReserved2;
    VOS_UINT8                           ucReserved3;
}NAS_MMC_NVIM_T3396_RUNNING_DISABLE_LTE_CFG_STRU;


typedef struct
{
    VOS_UINT32                          ulActiveFlag;
    VOS_UINT32                          ulExpireTimerLen;
    VOS_UINT16                          usMaxNoOfRetry;
    VOS_UINT8                           ucMaxConnFail;
    VOS_UINT8                           aucRsv[1];
}CNAS_EHSM_RETRY_CONN_EST_NVIM_INFO_STRU;


typedef struct
{
    VOS_UINT32                          ulActiveFlag;
    VOS_UINT32                          ulExpireTimerLen;
    VOS_UINT16                          usMaxNoOfRetry;
    VOS_UINT8                           aucRsv[2];
}CNAS_NVIM_EHRPD_PDN_SETUP_RETRY_STRU;


enum NAS_SMS_PS_CONCATENATE_ENUM
{
    NAS_SMS_PS_CONCATENATE_DISABLE      = 0,
    NAS_SMS_PS_CONCATENATE_ENABLE,

    NAS_SMS_PS_CONCATENATE_BUTT
};
typedef VOS_UINT8 NAS_SMS_PS_CONCATENATE_ENUM_UINT8;


typedef struct
{
    NAS_SMS_PS_CONCATENATE_ENUM_UINT8   enSmsConcatenateFlag;
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucReserved2;
    VOS_UINT8                           ucReserved3;
} NAS_NV_SMS_PS_CTRL_STRU;

typedef struct
{
    VOS_UINT8                           ucFilterEnableFlg;
    VOS_UINT8                           aucReserved[3];
} NAS_NV_PRIVACY_FILTER_CFG_STRU;



typedef struct
{
    VOS_UINT8                           ucCLOosTimerMaxExpiredTimes;    /* �͹���ģʽ��Available Timer��ʱ��ʱ����ʱ���� */

    VOS_UINT8                           ucCLSysAcqWaitHsdCnfTimerLen;   /* MSCC CL�����ȴ�HSD���������ʱ��ʱ�� */

    VOS_UINT16                          usReserved1;/* ԭʼϵͳ����MSPL����ʱ,bsr��ʱ��ʱ�� */

    VOS_UINT32                          ulFirstSearchAvailTimerLen; /* Contains the Available timer length to be used , when the
                                                                    number of successive triggers is less than ulFirstSearchAvailTimerCount */

    VOS_UINT32                          ulFirstSearchAvailTimerCount; /* For the number successive triggers of avaiable timer less
                                                                      than or equal to ulFirstSearchAvailTimerCount , MSCC uses a timer
                                                                      length value of ulFirstSearchAvailTimerLen */

    VOS_UINT32                          ulDeepSearchAvailTimerLen;  /* For the number successive triggers of avaiable timer greater
                                                                    than ulFirstSearchAvailTimerCount , MSCC uses a timer
                                                                    length value of ulDeepSearchAvailTimerLen */

    VOS_UINT32                          ulScanTimerLen; /* Contains the scan timer length */
    VOS_UINT8                           ucBsrCtrlDoEnterIdleRstLen;     /* BSR 10�볢�Զ�ʱ������ʱ��DO��CONN̬��ΪIDLE̬������������bsr��
                                                                                ���������������Զ�ʱ������ʱ��ʱ������ʱ����ʱ���ٷ���bsr����ֹ�ײ���Ƶ�л������¶��ݽ���idle̬�������  */
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucReserved2;
    VOS_UINT8                           ucReserved3;
    VOS_UINT32                          ulSleepTimerLen; /* Contains the sleep timer length */
}NAS_NVIM_MSCC_SYS_ACQ_TIMER_CFG_STRU;



typedef struct
{
    VOS_UINT32                                              ulMcc;
    VOS_UINT32                                              ulMnc;
    VOS_UINT16                                              usSid;
    VOS_UINT16                                              usNid;

    NAS_MSCC_NVIM_SYS_PRI_CLASS_ENUM_UINT8                  en1xPrioClass;
    NAS_MSCC_NVIM_SYS_PRI_CLASS_ENUM_UINT8                  enAIPrioClass;
    VOS_UINT8                                               aucRsv[2];  /* remain four bytes in future */
}NAS_NVIM_1X_LOC_INFO_STRU;


typedef struct
{
    VOS_UINT32                                              ulMcc;                                  /* MCC,3 bytes */
    VOS_UINT32                                              ulMnc;                                  /* MNC,2 or 3 bytes */
    NAS_MSCC_NVIM_SYS_PRI_CLASS_ENUM_UINT8                  enPrioClass;
    VOS_UINT8                                               aucRsv[3];  /* remain four bytes in future */
}NAS_NVIM_3GPP_LOC_INFO_STRU;


typedef struct
{
    VOS_UINT8                           ucIsLocInfoUsedInSwitchOn;
    NAS_NVIM_LC_RAT_COMBINED_ENUM_UINT8 enSysAcqMode;
    VOS_UINT8                           ucIs1xLocInfoValid;
    VOS_UINT8                           ucIsLteLocInfoValid;
    NAS_NVIM_1X_LOC_INFO_STRU           st1xLocInfo;
    NAS_NVIM_3GPP_LOC_INFO_STRU         st3gppLocInfo;
}NAS_NVIM_MMSS_LAST_LOCATION_INFO_STRU;


typedef struct
{

    VOS_UINT8                                               ucReAcqLteOnHrpdSyncIndFlag; /* The NVIM Flag controls if
                                                                                                       MSCC must search  for LTE Service
                                                                                                       when HSD sends Sync Ind, if LTE
                                                                                                       is preferred */

    VOS_UINT8                                               ucIs1xLocInfoPrefThanLte;  /* The NVIM Flag controls if CDMA 1x
                                                                                                     Loc info is more preferred than LTe
                                                                                                     Loc Info */
    VOS_UINT8                                               aucReserved[2];
    NAS_NVIM_MSCC_SYS_ACQ_TIMER_CFG_STRU                    stMmssSysAcqTimerCfg;  /* Contains the Timer Configuration
                                                                                                for MMSS System Acquire */
}NAS_NVIM_MMSS_SYSTEM_ACQUIRE_CFG_STRU;



typedef struct
{
    VOS_UINT8                           ucMlplMsplActiveFlag;
    VOS_UINT8                           aucRsv[3];
    VOS_UINT16                          usMlplBufSize;
    VOS_UINT16                          usMsplBufSize;
    VOS_UINT8                           aucMlplBuf[NAS_MSCC_NVIM_MLPL_SIZE];
    VOS_UINT8                           aucMsplBuf[NAS_MSCC_NVIM_MSPL_SIZE];
}NAS_MSCC_NVIM_MLPL_MSPL_STRU;


typedef struct
{
    VOS_UINT8                           ucMobTermForNid;
    VOS_UINT8                           ucMobTermForSid;
    VOS_UINT8                           ucMobTermHome;
    VOS_UINT8                           ucRsv;
}CNAS_NVIM_1X_MOB_TERM_STRU;


typedef struct
{
    VOS_UINT8                           ucActiveFlag;           /* NV item is active not not */
    VOS_UINT8                           ucRsv1;
    VOS_UINT16                          usActTimerLen;          /* Session activate timer length, unit is second */
    VOS_UINT8                           ucMaxActCountConnFail;  /* Max session activate count of reason conntion fail */
    VOS_UINT8                           ucMaxActCountOtherFail; /* Max session activate count of reason other fail */
    VOS_UINT8                           ucRsv2;
    VOS_UINT8                           ucRsv3;
}CNAS_HSM_NVIM_SESSION_RETRY_CFG_STRU;


typedef struct
{
    VOS_UINT8                           ucStartUatiReqAfterSectorIdChgFlg;   /* Ϊ��ͨ��RF���Ժ�CCF2.2.2.8����������NV����
                                                                                          NV��(Ĭ��):���ձ��ֻҪsector ID�����仯������UATI��������
                                                                                          NV�ر�:      �ϸ���Э��C.S0024 7.3.7.1.6.1������ֻ����HO����
                                                                                                       Conn Close��sector ID�����仯������UATI�������� */

    VOS_UINT8                           ucWaitUatiAssignTimerLenInAmpSetup; /* ��λ:��(s),Ĭ��5s.��ĳЩ�������豸�̵������ϣ�Ϊ�˽���UE�Ľ���ʱ����
                                                                                        ����UE��UATI assign��ʱ������ʱ�����Ϊ5s���Ϊ120s(Э�鶨ʱ��ʱ��) */
    VOS_UINT8                           ucWaitUatiAssignTimerLenInAmpOpen; /* ��λ:��(s),Ĭ��120s(Э�鶨ʱ��ʱ��)����ʱ�����5s���120s��*/

    VOS_UINT8                           ucUatiReqRetryTimesWhenUatiAssignTimerExpireInAmpOpen;/* ����NV�������open̬ʱ��UATI req���ͳɹ���
                                                                                                             ������UATI assign��ʱ������Դ���,Ĭ�ϲ����ԣ��������4�� */
    VOS_UINT8                           ucClearKATimerInConnOpenFlg;                          /* ����NV����ƣ��Ƿ������ӽ����ɹ������keep alive����
                                                                                                             NV��(Ĭ��): ���ӽ����ɹ���ֹͣkeep alive��ʱ��������keep alive��صļ���
                                                                                                                           ���ӶϿ��󣬿���keep alive��ʱ������������keep alive���̡�
                                                                                                             NV�ر�:       ���ӽ����ɹ��󣬲����keep alive���̣�����Э�鴦�� */
    VOS_UINT8                           ucRecoverEhrpdAvailFlg;                              /* ����NV����ƣ����UEƽ̨����֧��ehrpd,�Ƿ��ڿ��ػ���ָ�ehrpd����
                                                                                                             NV��:        ���ػ���ָ�ehrpd����������ػ�ǰ��hrpd�ĻỰ���򿪻�����Ҫ����Э��
                                                                                                             NV�ر�(Ĭ��):  ���ػ��󲻻ָ�ehrpd����������ػ�ǰ��hrpd�ĻỰ���򿪻���ִ��recover�Ự���� */

    VOS_UINT8                           ucRecoverEhrpdCapAfterSessionCloseFlg;              /* ����NV����ƣ���������session close���Ƿ���Ҫ���¿���eHRPD������ǣ�Ĭ�Ϲر�
                                                                                                             NV��:       ������������session close��UE�ָ�eHRPD�������(��NVĿǰֻ�ڵ���׼�����ʱ��)
                                                                                                             NV�ر�(Ĭ��):  ������������session close��UE���ָ�eHRPD�������*/
    VOS_UINT8                           ucCloseEhrpdCapAfterSyscfgNotSupportLteFlg;         /* ����NV����ƣ���Ʒ�߷Ǳ궨�ơ��ر�4G���غ󣬴���һ��session��������ָ�����ʼ���Ƿ�ر�eHRPD������ǣ�Ĭ�Ϲر�
                                                                                                             NV��:       4G���عرգ��ر�UE eHRPD����(��NVĿǰֻ��Բ�Ʒ�ߵ����������ԣ���������ģʽ������������ʱ��)
                                                                                                             NV�ر�(Ĭ��): ���UEƽ̨����֧��eHRPD,4G���ؼ�ʹ�رգ�UE��֧��eHRPD����*/

    VOS_UINT32                          ulUatiSigProtectTimeLen;                             /*  UATI���¹����е������ʱ��(�յ�CTTF����Uati req��SNP_DATA_CNF�󣬵�UATI assignment�ı���ʱ��)��
                                                                                                             Ĭ��1200ms,��λms */

    VOS_UINT8                           aucRsv1[20];
}CNAS_HSM_NVIM_SESSION_CTRL_CFG_STRU;


typedef struct
{
    VOS_UINT8                           ucIsKeepAliveInfoValid;    /* If TRUE, then the Keep alive paramters are valid */
    VOS_UINT8                           ucRsv1;                    /* for padding */
    VOS_UINT16                          usTsmpClose;               /* stores the TsmpClose value of the last session. Unit is minutes */
    VOS_UINT32                          ulTsmpCloseRemainTime;     /* Stores the time remaining for Tsmpclose minutes to
                                                                               expire. Unit is seconds */
    VOS_UINT32                          aulLastPowerOffSysTime[2]; /* Stores the CDMA system time at last Power Off.
                                                                              Unit is Milliseconds. */
}CNAS_HSM_NVIM_SESSION_KEEP_ALIVE_INFO_STRU;


typedef struct
{
    VOS_UINT8                           ucEhrpdSupportFlg;       /* EHRPD is support or not */
    VOS_UINT8                           ucRsv1;
    VOS_UINT8                           ucRsv2;
    VOS_UINT8                           ucRsv3;
}NAS_NVIM_EHRPD_SUPPORT_FLG_STRU;



typedef struct
{
    VOS_UINT8                           ucActiveFlag;                           /* NV���Ƿ񼤻� */
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucReserved2;
    VOS_UINT8                           ucReserved3;
    VOS_UINT32                          ulTotalTimeLen;                         /* ��һ�׶�������ʱ��,��λ:s */
}NAS_NVIM_PLMN_SEARCH_PHASE_ONE_TOTAL_TIMER_CFG_STRU;



typedef struct
{
    VOS_UINT16                          usRegFailCauseNum;                      /* ֧�ֵ�ע��ʧ��ԭ��ֵ����,����Ϊ0��ʾ��֧�ָ����� */
    VOS_UINT16                          usForbLaTimeLen;                        /* ��ֹLAʱ��,��λ:s */
    VOS_UINT16                          usPunishTimeLen;                        /* ��ƹ�һ�������Ҫ�ͷ���ʱ�� */
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucReserved2;
    VOS_UINT16                          ausRegFailCauseList[NAS_MML_NVIM_MAX_REG_FAIL_CAUSE_NUM];
}NAS_MMC_NVIM_CUSTOMIZED_FORB_LA_CFG_STRU;


typedef struct
{
    VOS_UINT16                          usCsgAutonomousSrchFirstSrchTimeLen;                   /* CSG����������ʱ����һ�ε�ʱ�� */
    VOS_UINT8                           ucCsgAutonomousSrchFirstSrchTimeCount;                 /* CSG����������ʱ����һ�εĴ��� */
    VOS_UINT8                           ucReserved1;
    VOS_UINT16                          usCsgAutonomousSrchSecondSrchTimeLen;                  /* CSG����������ʱ���ڶ��ε�ʱ�� */
    VOS_UINT8                           ucCsgAutonomousSrchSecondSrchTimeCount;                /* CSG����������ʱ���ڶ��εĴ��� */
    VOS_UINT8                           ucReserved2;
} NAS_NVIM_CSG_AUTONOMOUS_SEARCH_CFG_STRU;


typedef struct
{
    NVIM_PLMN_VALUE_STRU                stPlmnId;
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucReserved2;
    VOS_UINT32                          ulCsgId;
}NAS_NVIM_PLMN_WITH_CSG_ID_STRU;



typedef struct
{
    VOS_UINT8                           ucPlmnWithCsgIdNum;
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucReserved2;
    VOS_UINT8                           ucReserved3;
    NAS_NVIM_PLMN_WITH_CSG_ID_STRU      astPlmnWithCsgIdList[NAS_NVIM_MAX_PLMN_CSG_ID_NUM];
}NAS_NVIM_ALLOWED_CSG_LIST_STRU;

typedef struct
{
    VOS_UINT8                           ucPlmnWithCsgIdNum;
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucReserved2;
    VOS_UINT8                           ucReserved3;
    NAS_NVIM_PLMN_WITH_CSG_ID_STRU      astPlmnWithCsgIdList[NAS_NVIM_MAX_PLMN_CSG_ID_NUM];
}NAS_NVIM_UE_BASED_OPERATOR_CSG_LIST_STRU;

typedef struct
{
    VOS_UINT8                           ucNum;
    VOS_UINT8                           ucRat;                                  /* ��¼duplicate rplmn�Ľ��뼼��*/
    VOS_UINT8                           aucReserve[2];                          /*NV����صĽṹ�壬��4�ֽڷ�ʽ�£����ֶ�����ն�*/
    NVIM_PLMN_VALUE_STRU                astRplmnAndEplmnList[NVIM_MAX_EPLMN_NUM];
}NAS_NVIM_CSG_DUPLICATED_RPLMN_AND_EPLMN_LIST_STRU;


typedef struct
{
    /* CSG��ֹ����ͷ�ʱ��,��λ�� */
    VOS_UINT16                          usCsgPlmnForbiddenPeriod;

    /* ����CSG���类����Щԭ��ֵʱ��Ҫ������������ֹcsg�����б� */
    VOS_UINT8                           ucCauseNum;
    VOS_UINT8                           ucReserved;
    VOS_UINT16                          ausCause[NAS_NVIM_MAX_CSG_REJ_CAUSE_NUM];
}NAS_NVIM_CSG_FORBIDDEN_PLMN_CFG_STRU;

typedef struct
{
    VOS_UINT8                                               ucBitOp1:1;
    VOS_UINT8                                               ucBitOp2:1;
    VOS_UINT8                                               ucBitOp3:1;
    VOS_UINT8                                               ucBitOp4:1;
    VOS_UINT8                                               ucBitOp5:1;
    VOS_UINT8                                               ucBitOp6:1;
    VOS_UINT8                                               ucBitOp7:1;
    VOS_UINT8                                               ucBitOp8:1;
    VOS_UINT8                                               ucReserved0;
    VOS_UINT8                                               ucIsSupportCsgFlag;             /* VOS_TRUE:UE֧��CSG����; VOS_FALSE:UE��֧��CSG���� */
    VOS_UINT8                                               ucCsgListOnlyReportOperatorCsgListFlag; /* VOS_TRUE:csg�б��������ֻ�ϱ���operator csg list������; VOS_FALSE:CSG�б������������CSG ID���ϱ� */
    NAS_NVIM_CSG_FORBIDDEN_PLMN_CFG_STRU                    stCsgForbiddenPlmnCfg;       /* CSG��ֹ����������� */
    NAS_NVIM_ALLOWED_CSG_LIST_STRU                          stAllowedCsgList;               /* ��¼Allowed CSG List��Ϣ */
    VOS_UINT32                                              ulCsgPeriodicSearchPeriod;      /* CSG����������ʱ��,��λ���� */
    NAS_NVIM_CSG_DUPLICATED_RPLMN_AND_EPLMN_LIST_STRU       stDuplicatedRplmnAndEplmnList;  /* �ػ�ʱ���֮ǰ���й�CSGָ���������ҹػ�ʱפ��CSG���磬���¼CSGָ������ǰ��RPLMN��EPLMN*/
    NAS_NVIM_CSG_AUTONOMOUS_SEARCH_CFG_STRU                 stCsgAutonomousSrchCfg;         /* CSG��������������� */
	NAS_NVIM_UE_BASED_OPERATOR_CSG_LIST_STRU                stUeBasedOperatorCsgList;       /* ��¼Based CSG List��Ϣ */
    VOS_UINT8                                               ucRplmnCellType;                /* �ϴ�פ��С�����ͣ������ж��Ƿ����duplicated RPLMN */
    VOS_UINT8                                               ucReserved1;
    VOS_UINT8                                               ucReserved2;
    VOS_UINT8                                               ucReserved3;
    VOS_UINT32                                              ulReserved1;
} NAS_NVIM_CSG_CTRL_CFG_STRU;


typedef struct
{
    VOS_UINT8                           ucKeepCsForbInfoFlg;
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucReserved2;
    VOS_UINT8                           ucReserved3;
}NAS_NVIM_KEEP_CS_FORB_INFO_WHEN_PS_REG_SUCC_STRU;


typedef struct
{
    VOS_UINT8                           ucNwIgnoreAuthFailFlg;                  /* ������Լ�Ȩʧ�ܽ��û����Ӧ���ͷ���·ʱ�Ƿ����CKSN������ */
    VOS_UINT8                           ucUsimGsmAuthActiveFlg;                            /* USIM����GSM����2G cs/ps ��Ȩ����3G���Ƿ���Ҫ���CS/ps CKSNֵ */
    VOS_UINT8                           ucUsimDoneGsmCsAuthFlg;
    VOS_UINT8                           ucUsimDoneGsmPsAuthFlg;
}NAS_NVIM_CLEAR_CKSN_STRU;


typedef struct
{
    VOS_UINT8                           ucNetSelMenuFlg;                        /* ����ѡ��˵����ƣ�VOS_TRUE:���VOS_FALSE:δ���� */
    VOS_UINT8                           ucRatBalancingFlg;                      /* ���뼼��ƽ����ƣ�VOS_TRUE:���VOS_FALSE:δ���� */
    VOS_UINT8                           aucReserved[2];                         /* ���� */
}NAS_NVIM_ATT_ENS_CTRL_STRU;



typedef struct
{
    VOS_UINT8                           ucEnableFlg;                            /* FRAT�����Ƿ���Ч */
    VOS_UINT8                           ucIgnitionState;                        /* FRAT������Чʱ����ǰIgnition״̬ */
    VOS_UINT8                           ucImsiPlmnNum;
    VOS_UINT8                           ucFratPlmnNum;

    VOS_UINT32                          ulFratSlowTimerValue;                   /* FRAT������Чʱ��Slow Timerʱ������λ �룬Ĭ��ֵ180���� */
    VOS_UINT32                          ulFratFastTimerValue;                   /* FRAT������Чʱ��Fast Timerʱ������λ �룬Ĭ��ֵ3���� */
    NAS_SIM_FORMAT_PLMN_ID              astImsiPlmnIdList[NAS_MML_NVIM_FRAT_MAX_IMSI_NUM];          /* FRAT������Чʱ��֧�ֵ�IMSI��PLMN���б� */
    NAS_SIM_FORMAT_PLMN_ID              astFratPlmnIdList[NAS_MML_NVIM_FRAT_MAX_PLMN_ID_NUM];       /* FRAT������Чʱ��פ��PLMN���б����ٲ�������20��PLMN�б� */
}NAS_NVIM_ATT_FRAT_CFG_STRU;


typedef struct
{
    VOS_UINT8                           ucPagingResponseRetrySupportFlg;        /* ��ʶλ�����ڱ�ʾ�Ƿ�ʹ��Ѱ����Ӧ�ظ����ܡ�1:ʹ�� 0:��ʹ�� */
    VOS_UINT8                           ucPagingResponseRetryPeriod;            /* Ѱ����Ӧ�ظ����ڳ���,��λ����*/
    VOS_UINT8                           ucPagingResoneseRetry2GInterval;        /* 2G��Ѱ����Ӧ�ظ��ļ��ʱ��,��λ���� */
    VOS_UINT8                           ucPagingResoneseRetry3GInterval;        /* 3G��Ѱ����Ӧ�ظ��ļ��ʱ��,��λ���� */

}NAS_MMC_PAGING_RESPONSE_RETRY_CFG_STRU;

typedef struct
{
    VOS_UINT8                           ucTiNonregularCustom;
    VOS_UINT8                           ucReserve1;
    VOS_UINT8                           ucReserve2;
    VOS_UINT8                           ucReserve3;
    VOS_UINT8                           ucReserve4;
    VOS_UINT8                           ucReserve5;
    VOS_UINT8                           ucReserve6;
    VOS_UINT8                           ucReserve7;
}NAS_NVIM_NONREGULAR_SERVICE_CUSTOM_STRU;

typedef struct
{
    VOS_UINT8                           ucHoldRetrieveRejOptimize;
    VOS_UINT8                           ucReserve1;
    VOS_UINT8                           ucReserve2;
    VOS_UINT8                           ucReserve3;
}NAS_NVIM_HOLD_RETRIEVE_REJ_OPTIMIZE_STRU;


typedef struct
{
    VOS_UINT8                           ucEnableFlag;                           /* VOS_TRUE:ʹ��,VOS_FALSE:ȥʹ�� */
    VOS_UINT8                           aucReserved1;
    VOS_UINT8                           aucReserved2;
    VOS_UINT8                           aucReserved3;
}NAS_NVIM_CC_STATUS_ENQUIRY_CFG_STRU;


enum CNAS_NVIM_HRPD_SESSION_STATUS_ENUM
{
    CNAS_NVIM_HRPD_SESSION_STATUS_CLOSE,
    CNAS_NVIM_HRPD_SESSION_STATUS_OPEN,
    CNAS_NVIM_HRPD_SESSION_STATUS_BUTT
};
typedef VOS_UINT8 CNAS_NVIM_HRPD_SESSION_STATUS_ENUM_UINT8;


enum CNAS_NVIM_HRPD_SESSION_TYPE_ENUM
{
    CNAS_NVIM_HRPD_SESSION_TYPE_HRPD,
    CNAS_NVIM_HRPD_SESSION_TYPE_EHRPD,
    CNAS_NVIM_HRPD_SESSION_TYPE_BUTT
};
typedef VOS_UINT8 CNAS_NVIM_HRPD_SESSION_TYPE_ENUM_UINT8;


enum CNAS_NVIM_HARDWARE_ID_TYPE_ENUM
{
    CNAS_NVIM_HARDWARE_ID_TYPE_MEID                    = 0x0000FFFF,
    CNAS_NVIM_HARDWARE_ID_TYPE_ESN                     = 0x00010000,
    CNAS_NVIM_HARDWARE_ID_TYPE_NULL                    = 0x00FFFFFF,
    CNAS_NVIM_HARDWARE_ID_TYPE_BUTT
};
typedef VOS_UINT32 CNAS_NVIM_HARDWARE_ID_TYPE_ENUM_UINT32;


enum CNAS_NVIM_HARDWARE_ID_SRC_TYPE_ENUM
{
    CNAS_NVIM_HARDWARE_ID_SRC_TYPE_NVIM,
    CNAS_NVIM_HARDWARE_ID_SRC_TYPE_RAND,
    CNAS_NVIM_HARDWARE_ID_SRC_TYPE_UIM,
    CNAS_NVIM_HARDWARE_ID_SRC_TYPE_BUTT
};
typedef VOS_UINT32 CNAS_NVIM_HARDWARE_ID_SRC_TYPE_ENUM_UINT32;



typedef struct
{
    VOS_UINT8                           aucCurUATI[CNAS_NVIM_UATI_OCTET_LENGTH];
    VOS_UINT8                           ucUATIColorCode;
    VOS_UINT8                           ucUATISubnetMask;
    VOS_UINT8                           ucUATIAssignMsgSeq;
    VOS_UINT8                           ucRsv2;
}CNAS_NVIM_HRPD_UATI_INFO_STRU;


typedef struct
{
    CNAS_NVIM_HARDWARE_ID_TYPE_ENUM_UINT32                  enHwidType;
    CNAS_NVIM_HARDWARE_ID_SRC_TYPE_ENUM_UINT32              enHwidSrcType;
    VOS_UINT32                                              ulEsn;                                /* 32-bit */
    VOS_UINT8                                               aucMeId[CNAS_NVIM_MEID_OCTET_NUM];    /* 56-bit */
    VOS_UINT8                                               ucRsv1;
}CNAS_NVIM_HARDWARE_ID_INFO_STRU;


typedef struct
{
    VOS_INT32                           lLongitude;
    VOS_INT32                           lLatitude;
}CNAS_NVIM_LOC_INFO_STRU;


typedef struct
{
    CNAS_NVIM_HRPD_SESSION_STATUS_ENUM_UINT8                enSessionStatus;
    CNAS_NVIM_HRPD_SESSION_TYPE_ENUM_UINT8                  enSessionType;
    VOS_UINT8                                               aucIccid[CNAS_NVIM_ICCID_OCTET_LEN];
    CNAS_NVIM_HRPD_UATI_INFO_STRU                           stUatiInfo;
    CNAS_NVIM_LOC_INFO_STRU                                 stLocInfo;
    CNAS_NVIM_HARDWARE_ID_INFO_STRU                         stHwid;
}CNAS_NVIM_HRPD_SESSION_INFO_STRU;


typedef struct
{
    PS_BOOL_ENUM_UINT8                  enSuppOnlyDo0;                          /* �Ƿ�ֻ֧��DO0�汾 */
    PS_BOOL_ENUM_UINT8                  enSuppDoaWithMfpa;                      /* �Ƿ�֧��DOA�汾��Ӧ������ֻ֧��MFPA */
    PS_BOOL_ENUM_UINT8                  enSuppDoaWithEmfpa;                     /* �Ƿ�֧��DOA�汾��Ӧ������֧��MFPA��EMPA */
    PS_BOOL_ENUM_UINT8                  enSuppDoaEhrpd;                         /* �Ƿ�֧��eHRPD */
}CNAS_NVIM_HRPD_UE_REV_INFO_STRU;


typedef struct
{
    VOS_UINT32                          ulSessionSeed;
    CNAS_NVIM_HRPD_UE_REV_INFO_STRU     stUERevInfo;
    VOS_UINT8                           ucLteRegSuccFlg;
    VOS_UINT8                           aucReserve1[3];
    VOS_UINT32                          ulPseudorandomNumber;  /* ��������sessionseed��α����� */
    VOS_UINT8                           aucReserve2[4];
}CNAS_NVIM_HRPD_SESSION_INFO_EX_STRU;

/*****************************************************************************
�ṹ��    : NAS_NVIM_TIME_INFO_REPORT_OPTIMIZE_CFG_STRU
�ṹ˵��  : en_NV_Item_Time_Info_Report_Cfg(2434) ʱ���Ż��ϱ����Կ���
*****************************************************************************/
typedef struct
{
    VOS_UINT8                           ucRptOptType;                           /* �ϱ����Կ��� 0:�����ƣ�1:^TIMEʱ��������ʱ�仯�ϱ��� */
                                                                                /*  2:������ͬ0��3:��1�Ļ��������������ϱ����ʱ����� */
    VOS_UINT8                           ucInterval;                             /* RptOptTypeΪ3ʱ�������ϱ����ʱ�䣬��λmin��ȡֵ0-255 */
    VOS_UINT8                           ucReserved2;
    VOS_UINT8                           ucReserved3;
}NAS_NVIM_TIME_INFO_REPORT_OPTIMIZE_CFG_STRU;


typedef struct
{
    VOS_UINT8                           ucAccessAuthAvailFlag;
    VOS_UINT8                           ucAccessAuthUserNameLen;
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           aucAccessAuthUserName[CNAS_NVIM_MAX_AUTHDATA_USERNAME_LEN];
}CNAS_NVIM_HRPD_ACCESS_AUTH_INFO_STRU;


typedef struct
{
    VOS_UINT8                           ucUseImsiFlg;                           /* VDF��������״̬�·��������Я��IMSI���� */
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucReserved2;
    VOS_UINT8                           ucReserved3;
}NAS_NVIM_EMC_UNDER_NET_PIN_CFG_STRU;


typedef struct
{
    VOS_UINT8                 ucPsRegFailMaxTimesTrigLauOnceFlg;
    VOS_UINT8                 ucReserved1;
    VOS_UINT8                 ucReserved2;
    VOS_UINT8                 ucReserved3;
}NAS_NVIM_PS_REG_FAIL_MAX_TIMES_TRIG_LAU_ONCE_CFG_STRU;


typedef struct
{
    VOS_UINT8                 ucKeepSrchHplmnEvenRejByCause13Flg;
    VOS_UINT8                 ucReserved1;
    VOS_UINT8                 ucReserved2;
    VOS_UINT8                 ucReserved3;
}NAS_NVIM_KEEP_SRCH_HPLMN_EVEN_REJ_BY_CAUSE_13_CFG_STRU;


typedef struct
{
    VOS_UINT8                 ucEpsRejByCause14InVplmnAllowPsRegFlg;
    VOS_UINT8                 ucReserved1;
    VOS_UINT8                 ucReserved2;
    VOS_UINT8                 ucReserved3;
}NAS_NVIM_EPS_REJ_BY_CAUSE_14_IN_VPLMN_ALLOW_PS_REG_CFG_STRU;


enum NAS_MMC_NVIM_CARRY_EPLMN_SWITCH_FLAG_ENUM
{
    NAS_MMC_NVIM_CARRY_EPLMN_SWITCH_OFF                     = 0,           /* �رմ��Ż� */
    NAS_MMC_NVIM_CARRY_EPLMN_SWITCH_ON_FOR_ROAM             = 1,           /* �Ż��򿪣����ǽ�������ʱ��Ч */
    NAS_MMC_NVIM_CARRY_EPLMN_SWITCH_ON_FOR_ALL              = 2,           /* �Ż��򿪣�����������ζ���Ч */
    NAS_MMC_NVIM_CARRY_EPLMN_SWITCH_BUTT
};
typedef VOS_UINT8 NAS_MMC_NVIM_CARRY_EPLMN_SWITCH_FLAG_ENUM_UINT8;


typedef struct
{
    NAS_MMC_NVIM_CARRY_EPLMN_SWITCH_FLAG_ENUM_UINT8         enSwitchFlag;
    VOS_UINT8                                               ucCarryEplmnSceneSwitchOn;
    VOS_UINT8                                               ucCarryEplmnSceneAreaLost;
    VOS_UINT8                                               ucCarryEplmnSceneAvailableTimerExpired;
    VOS_UINT8                                               ucCarryEplmnSceneSysCfgSet;
    VOS_UINT8                                               ucCarryEplmnSceneDisableLte;
    VOS_UINT8                                               ucCarryEplmnSceneEnableLte;
    VOS_UINT8                                               ucCarryEplmnSceneCSFBServiceRej;
    VOS_UINT8                                               ucReserved1;
    VOS_UINT8                                               ucReserved2;
    VOS_UINT8                                               ucReserved3;
    VOS_UINT8                                               ucReserved4;
}NAS_NVIM_CARRY_EPLMN_WHEN_SRCH_RPLMN_CFG_STRU;


typedef struct
{
    VOS_UINT8                           ucActiveFlg;                                                /* �����Ƿ�� */
    VOS_UINT8                           ucRetryTimerLenOnLte;                                       /* LTE��PDP retryʱ������λ��s */
    VOS_UINT8                           ucRetryTimerLenOnGu;                                        /* GU��PDP retryʱ������λ��s */
    VOS_UINT8                           ucLimitTimerLenAfterMaxCnt;                                 /* PDP����ܴﵽ������������PDP�����ʱ������λ��min */
    VOS_UINT8                           ucMaxRetryCnt;                                              /* ���retry���� */
    VOS_UINT8                           ucReserve;
    VOS_UINT8                           ucPlmnNum;
    VOS_UINT8                           ucCauseNum;
    NAS_SIM_FORMAT_PLMN_ID              astPlmnIdList[NAS_NVIM_MAX_LIMIT_PDP_ACT_PLMN_NUM];         /* ��Ӫ���б� */
    VOS_UINT8                           aucCauseList[NAS_NVIM_MAX_LIMIT_PDP_ACT_CAUSE_NUM];         /* ԭ��ֵ�б� */
}NAS_NVIM_TELCEL_PDP_ACT_LIMIT_CFG_STRU;

typedef struct
{
    VOS_UINT8                 ucSwitchOnBorderPlmnSearchFlg;                 /* �����߾������Ż����� VOS_TRUE:���� VOS_FALSE:�ر� */
    VOS_UINT8                 ucBorderBgSearchFlg;                              /* �߾������������Ż����� VOS_TRUE:���� VOS_FALSE:�ر� */
    VOS_UINT8                 ucReserved1;
    VOS_UINT8                 ucReserved2;
}NAS_NVIM_BORDER_PLMN_SEARCH_CFG_STRU;


typedef struct
{
    VOS_UINT8                           ucHomeSidNidDependOnPrlFlg;
    VOS_UINT8                           aucReserved[3];
}CNAS_NVIM_HOME_SID_NID_DEPEND_ON_PRL_CFG_STRU;


typedef struct
{
    VOS_UINT16                          usStartSid;
    VOS_UINT16                          usEndSid;
    VOS_UINT32                          ulMcc;
}CNAS_NVIM_SYS_INFO_STRU;


typedef struct
{
    VOS_UINT8                           ucEnable;                          /* �������Ƿ�ʹ�� */
    VOS_UINT8                           ucReserved;
    VOS_UINT16                          usWhiteSysNum;                     /* ֧�ְ������ĸ���,����Ϊ0ʱ��ʾ��֧�ְ����� */
    CNAS_NVIM_SYS_INFO_STRU             astSysInfo[CNAS_NVIM_MAX_WHITE_LOCK_SYS_NUM];
}CNAS_NVIM_OPER_LOCK_SYS_WHITE_LIST_STRU;


typedef struct
{
    VOS_UINT16                          usChannel;
    VOS_UINT8                           aucReserved[2];
}CNAS_NVIM_FREQENCY_CHANNEL_STRU;


typedef struct
{
    VOS_UINT8                           ucEnableFlg;
    VOS_UINT8                           ucReserved;
    VOS_UINT16                          usFreqNum;
    CNAS_NVIM_FREQENCY_CHANNEL_STRU     astFreqList[CNAS_NVIM_MAX_HRPD_CUSTOMIZE_FREQ_NUM];
}CNAS_NVIM_CTCC_CUSTOMIZE_FREQ_LIST_STRU;


typedef struct
{
    VOS_UINT16                          usPrimaryA;
    VOS_UINT16                          usPrimaryB;
    VOS_UINT16                          usSecondaryA;
    VOS_UINT16                          usSecondaryB;
}CNAS_NVIM_CDMA_STANDARD_CHANNELS_STRU;


typedef struct
{
    VOS_UINT32                                              ulEnableFlag;
}CNAS_NVIM_NO_CARD_MODE_CFG_STRU;


typedef struct
{
    VOS_UINT8                                               aucRedialTimes[CNAS_NVIM_1X_MAX_MRU_SYS_NUM];  /* ��������ʧ��ʱ��ǰפ����Ƶ����mru list���ز���������������������*/
}CNAS_NVIM_1X_EMC_REDIAL_SYS_ACQ_CFG_STRU;


typedef struct
{
    VOS_UINT8                           aucAvoidTimerLen[CNAS_NVIM_1X_AVOID_MAX_PHASE_NUM];
}CNAS_NVIM_1X_AVOID_PHASE_NUM;


typedef struct
{
    CNAS_NVIM_1X_AVOID_PHASE_NUM        astAvoidPhaseNum[CNAS_NVIM_1X_AVOID_REASON_MAX];
}CNAS_NVIM_1X_AVOID_SCHEDULE_INFO_STRU;


typedef struct
{
    VOS_UINT8                                               ucPowerOffCampOnCtrlFlg;
    VOS_UINT8                                               aucReserved[3];
}CNAS_NVIM_1X_POWER_OFF_CAMP_ON_CTRL_STRU;


typedef struct
{
    VOS_UINT8                                               ucIsConsiderRoamIndInPRLFlg;        /* �Ƿ���PRL������ϵͳ�ж��п���ROAM IND���Եı�־ */
    VOS_UINT8                                               ucReserved1;
    VOS_UINT8                                               ucReserved2;
    VOS_UINT8                                               ucReserved3;
} CNAS_NVIM_1X_PRL_ROAM_IND_STRATEGY_CFG_STRU;


typedef struct
{
    VOS_UINT32                                              ul1xSysAcqNoRfProtectTimerLen;
    VOS_UINT32                                              ul1xRedirNoRfProtectTimerLen;
    VOS_UINT32                                              ulHrpdSysAcqNoRfProtectTimerLen;
    VOS_UINT8                                               aucReserved[4];
}CNAS_NVIM_1X_DO_SYS_ACQ_NO_RF_PROTECT_TIMER_CFG_STRU;


typedef struct
{
    VOS_UINT8                                               ucLteDoConnInterrupt1xSysAcqFlg;
    VOS_UINT8                                               ucReserved1;
    VOS_UINT8                                               ucReserved2;
    VOS_UINT8                                               ucReserved3;
}CNAS_NVIM_LTE_DO_CONN_INTERUPT_1X_SYS_ACQ_CFG_STRU;


typedef struct
{
    VOS_UINT32                                              ul1xSysAcqDelayTimerLen;
    VOS_UINT8                                               uc1xSysAcqSyncDelayEnable;
    VOS_UINT8                                               auc1xSysAcqSyncDelayFreqNum[3];             //  ��ʹ�õ�һ���ֽڱ�ʾ�ӳ�ͬ��ÿ��sync��Ƶ�����
}CNAS_NVIM_LTE_OR_DO_CONN_1X_SYS_ACQ_SYNC_DELAY_INFO_STRU;


typedef struct
{
    VOS_UINT8                          ucImsiListNum;                                                  /*������Ч��SIM����Ŀ(LTE OOS������2G����3G)  */
    VOS_UINT8                          ucReserved1;
    VOS_UINT8                          ucReserved2;
    VOS_UINT8                          ucReserved3;
    NAS_SIM_FORMAT_PLMN_ID             astImsiList[NAS_NVIM_LTE_OOS_2G_PREF_PLMN_SEL_MAX_IMSI_LIST_NUM];/* SIM���б� (LTE OOS������2G����3G) */
}NAS_MMC_NVIM_LTE_OOS_2G_PREF_PLMN_SEL_CFG_STRU;


typedef struct
{
    VOS_UINT16                          usChannel;
    VOS_UINT16                          usBandClass;
} CNAS_NVIM_CUSTOM_FREQUENCY_CHANNEL_STRU;


typedef struct
{
    VOS_UINT8                               ucEnableFlg;
    VOS_UINT8                               ucReserved;
    VOS_UINT16                              usFreqNum;
    CNAS_NVIM_CUSTOM_FREQUENCY_CHANNEL_STRU astFreqList[CNAS_NVIM_MAX_CDMA_1X_CUSTOM_PREF_CHANNELS_NUM];
} CNAS_NVIM_CDMA_1X_CUSTOM_PREF_CHANNELS_STRU;


typedef struct
{
    VOS_UINT8                               ucEnableFlg;
    VOS_UINT8                               ucReserved;
    VOS_UINT16                              usFreqNum;
    CNAS_NVIM_CUSTOM_FREQUENCY_CHANNEL_STRU astFreqList[CNAS_NVIM_MAX_CDMA_1X_CUSTOMIZE_PREF_CHANNELS_NUM];
} CNAS_NVIM_CDMA_1X_CUSTOMIZE_PREF_CHANNELS_STRU;


typedef struct
{
    VOS_UINT8                           ucEnableDynloadTW;
    VOS_UINT8                           ucReseverd1;
    VOS_UINT8                           ucReseverd2;
    VOS_UINT8                           ucReseverd3;
}NAS_NVIM_DYNLOAD_CTRL_STRU;


typedef struct
{
    VOS_UINT8                           ucTWMaxAttemptCount;
    VOS_UINT8                           ucReseverd1;
    VOS_UINT8                           ucReseverd2;
    VOS_UINT8                           ucReseverd3;
}NAS_NVIM_DYNLOAD_EXCEPTION_CTRL_STRU;


typedef struct
{
    VOS_UINT8                           ucNoDataSrvRspSo33;                     /* û������ҵ��ʱ��Paging rsp�Ļظ���0 - Paging Rsp��SO��0�� 1 - Paging Rsp��SO��33 */
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucReserved2;
    VOS_UINT8                           ucReserved3;
}CNAS_NVIM_1X_PAGING_RSP_SO_CFG_STRU;




typedef struct
{
    VOS_UINT8                           ucIsStartT310AccordWith3GPP;            /* ��proceeding����progress�У�Я��progress indicatorֵΪ#1��#2��#64ʱ��
                                                                                   �Ƿ�����T310, 0 - ����Э����;  1 -  ����Э���� */
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucReserved2;
    VOS_UINT8                           ucReserved3;
}NAS_NVIM_PROGRESS_INDICATOR_START_T310_INFO_STRU;


typedef struct
{
    VOS_UINT8                           ucPppDeactTimerLen;                     /* ��λ(S), PPPȥ��������ʱ�� */
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucReserved2;
    VOS_UINT8                           ucReserved3;
}NAS_NVIM_CDATA_DISCING_PARA_INFO_STRU;

typedef struct
{
    VOS_UINT32                          ulIsPppAuthGetFromCard;
}NAS_NVIM_PPP_AUTH_INFO_FROM_CARD_STRU;


typedef struct
{
    VOS_UINT8                           ucEnableFlag;                           /* VOS_TRUE:Auto attach����ʹ�ܣ�VOS_FALSE:Auto attach���ܹر� */
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucReserved2;
    VOS_UINT8                           ucReserved3;
}NAS_NVIM_EHRPD_AUTO_ATTACH_CTRL_CFG_STRU;


typedef struct
{
    VOS_UINT8                           ucEccSrvCap;        /* ECC��������: TRUE -- ֧��ECC����FALSE -- ��֧��ECC���� */
    VOS_UINT8                           ucEccSrvStatus;     /* ECC����״̬: TRUE -- ECC����򿪣�FALSE -- ECC����ر� */
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucReserved2;
} CNAS_NVIM_1X_CALL_ENCVOICE_ECC_SRV_CAP_INFO_STRU;



typedef struct
{
    VOS_UINT16                          usTotalTimeLen;         /* �ý׶���������ʱ��,��λΪ�� */
    VOS_UINT16                          usSleepTimeLen;         /* �ý׶�available��ʱ��������ʱ��,��λΪ�� */
    VOS_UINT16                          usReserve1;
    VOS_UINT16                          usReserve2;
    VOS_UINT8                           ucLteHistorySrchNum;       /* �ý׶�LTE��ʷƵ�������ܴ��� */
    VOS_UINT8                           ucLteFullBandSrchNum;      /* �ý׶�LTEȫƵ�������ܴ��� */
    VOS_UINT8                           ucLtePrefBandSrchNum;      /* �ý׶�LTE pref band���ܴ��� */
    VOS_UINT8                           ucReserve2;
    VOS_UINT8                           ucReserve3;
    VOS_UINT8                           ucReserve4;
    VOS_UINT8                           ucReserve5;
    VOS_UINT8                           ucReserve6;
} NAS_NVIM_1X_SERVICE_CL_SYSTEM_ACQUIRE_PATTERN_CFG_STRU;


typedef struct
{
    VOS_UINT8                           uc1xBsrLteActiveFlg;                /* �����л��л���1X��BSR LTE�ļ�����--CCF����Ҫ�� */
    VOS_UINT8                           uc1xBsrLteTimerLen;                 /* �����л��л���1X��BSR LTE�Ķ�ʱ��ʱ��-��λ�� */
    VOS_UINT8                           ucSrlte1xBsrLteEnableFlg;           /* SRLTE��1x����ҵ��̬BSR LTEʹ�ܱ�־ */
    VOS_UINT8                           ucReserve4;
    VOS_UINT8                           ucReserve5;
    VOS_UINT8                           ucReserve6;
    VOS_UINT8                           ucReserve7;
    VOS_UINT8                           ucReserve8;
} NAS_NVIM_1X_SERVICE_CL_SYSTEM_ACQUIRE_CTRL_STRU;



typedef struct
{
    NAS_NVIM_1X_SERVICE_CL_SYSTEM_ACQUIRE_CTRL_STRU         stCtrlInfo;
    NAS_NVIM_1X_SERVICE_CL_SYSTEM_ACQUIRE_PATTERN_CFG_STRU  stPhaseOnePatternCfg;   /* �׽׶�������Ϣ */
    NAS_NVIM_1X_SERVICE_CL_SYSTEM_ACQUIRE_PATTERN_CFG_STRU  stPhaseTwoPatternCfg;   /* ���׶�������Ϣ */
    NAS_NVIM_1X_SERVICE_CL_SYSTEM_ACQUIRE_PATTERN_CFG_STRU  stPhaseThreePatternCfg; /* ���׶�������Ϣ:Ŀǰδʹ��,Ԥ�� */
} NAS_NVIM_1X_SERVICE_CL_SYSTEM_ACQUIRE_STRATEGY_CFG_STRU;

typedef struct
{
    VOS_UINT8                           ucLteOos1xActDelayTimerLen;   /* VOLTE,lte oos��1x�������ӳٶ�ʱ��,��λ(s) */

    VOS_UINT8                           ucReserve1;
    VOS_UINT8                           ucReserve2;
    VOS_UINT8                           ucReserve3;
    VOS_UINT8                           ucReserve4;
    VOS_UINT8                           ucReserve5;
    VOS_UINT8                           ucReserve6;
    VOS_UINT8                           ucReserve7;
} NAS_NVIM_LTE_OOS_DELAY_ACTIVATE_1X_TIMER_INFO_STRU;



typedef struct
{
    VOS_UINT8                           ucClImsSupportFlag;             /* CLģʽ�£�IMS����֧�ֿ��� */

    VOS_UINT8                           ucClEnhanceVolteFlag;           /* cdma normal volte flag, effective only when ucClImsSupportFlag is true
                                                                                  0: normal volte , ����volte��srlte�л�ͨ��ap�·�LTEIMSSWITHC������;
                                                                                  1: enhance volte,  ����volte��srlte�Զ��л�  */

    VOS_UINT8                           ucReserve1;
    VOS_UINT8                           ucReserve2;
    VOS_UINT32                          ulDisableLteTimerLen;           /* �ͷ���ʱ��ʱ�� ��λ:�� */
    VOS_UINT8                           ucPingPongCtrlTimerLen;           /* ƹ���л��޶�ʱ�� ��λ:���� */
    VOS_UINT8                           ucMaxPingPongNum;                 /* �޶�ʱ���ڵ�����л����� */
    VOS_UINT8                           ucReserve3;
    VOS_UINT8                           ucReserve4;
} NAS_NVIM_CL_VOLTE_CFG_INFO_STRU;


typedef struct
{
    VOS_UINT8                           ucMru0SearchTimerLen;
    VOS_UINT8                           ucPhaseNum;
    VOS_UINT8                           ucHrpdMru0TimerMaxExpiredTimes; /* �͹���ģʽ�£�HRPD MRU0������ʱ����ʱ�������ﵽ����ֵ��ŷ������� */
    VOS_UINT8                           ucReserved;
    CNAS_NVIM_OOC_TIMER_INFO_STRU       astOocTimerInfo[CNAS_NVIM_MAX_HRPD_OOC_SCHEDULE_PHASE_NUM];
}CNAS_NVIM_HRPD_OOC_TIMER_SCHEDULE_INFO_STRU;


typedef struct
{
    VOS_UINT8                                               ucReAcqLteWithNoRfEnable;           /* lte no rfʱ���²���lte�Ƿ�ʹ�� */
    NAS_NVIM_CL_SYS_ACQ_DSDS_STRATEGY_SCENE_ENUM_UINT8      enReAcqLteWithNoRfScene;            /* lte no rfʱ���²���lte���������� */
    VOS_UINT8                                               ucReAcqLteWithNoRfDelayTime;        /* lte no rfʱ���²���lte���ӳ�ʱ��-��λ�� */
    VOS_UINT8                                               ucRsv1;
    VOS_UINT16                                              usRsv1;
    VOS_UINT16                                              usRsv2;
    VOS_UINT8                                               ucRsv2;
    VOS_UINT8                                               ucRsv3;
    VOS_UINT8                                               ucRsv4;
    VOS_UINT8                                               ucRsv5;
    VOS_UINT8                                               ucRsv6;
    VOS_UINT8                                               ucRsv7;
    VOS_UINT8                                               ucRsv8;
    VOS_UINT8                                               ucRsv9;
}NAS_NVIM_CL_SYSTEM_ACQUIRE_DSDS_STRATEGY_CFG_STRU;



typedef struct
{
    VOS_UINT8                                               ucPhaseNum;                 /* �ܽ׶θ��� */
    NAS_NVIM_CHAN_REPEAT_SCAN_ENUM_UINT8                    enChanRepeatScanStrategy;   /* Ƶ���ظ��������� */
    VOS_UINT16                                              usReserved;
    VOS_UINT8                                               uc1xOocDoTchPhase1TimerLen; /* Do TCH��ǰ4�γ��� Ooc Timer ���ʱ��,��λ�� */
    VOS_UINT8                                               uc1xOocDoTchPhase2TimerLen; /* Do TCH��4�����ϳ��� Ooc Timer ���ʱ��,��λ�� */
    VOS_UINT16                                              usRsv1;
    VOS_UINT8                                               uc1xMru0TimerMaxExpiredTimes;   /* �͹���ģʽ�£�MRU0��ʱ����ʱ���������ﵽ����ֵ����MRU0���� */
    VOS_UINT8                                               ucRsv2;
    VOS_UINT8                                               ucRsv3;
    VOS_UINT8                                               ucRsv4;
}CNAS_NVIM_1X_OOS_SYS_ACQ_STRATEGY_CTRL_STRU;


typedef struct
{
    VOS_UINT16                          usTotalTimeLen;         /* 1xһ���׶�������ʱ��,��λΪ�� */
    VOS_UINT16                          usSleepTimeLen;         /* 1x����һ�ּ��ʱ��,��λΪ�� */
    VOS_UINT8                           ucSrchNum;              /* 1xһ���׶��������� */
    VOS_UINT8                           ucRsv1;
    VOS_UINT8                           ucRsv2;
    VOS_UINT8                           ucRsv3;
    VOS_UINT16                          usMru0SearchTimerLen;       /* ����MRU0ʱavailable��ʱ����ʱ��,ÿ���׶ε�ʱ��,��λΪ�� */

    VOS_UINT16                          usModem0MinSleepTimerLen;     /* 1x�������ϣ���ǰ�׶����SleepTimeLen,���usSleepTimeLenС�ڸ�ֵ��ȡucMinSleepTimerLen,���ڶԲ�ͬ��Ʒ�Ķ������� */

    VOS_UINT8                           ucModem1MinSleepTimerLen; /* 1x�ڸ����ϣ���ǰ�׶����SleepTimeLen,���usSleepTimeLenС�ڸ�ֵ��ȡucMinSleepTimerLen,���ڶԲ�ͬ��Ʒ�Ķ������� */
    VOS_UINT8                           ucModem2MinSleepTimerLen;  /* 1x�����ͨ�ϣ���ǰ�׶����SleepTimeLen,���usSleepTimeLenС�ڸ�ֵ��ȡucMinSleepTimerLen,���ڶԲ�ͬ��Ʒ�Ķ������� */
    VOS_UINT8                           ucRsv6;
    VOS_UINT8                           ucRsv7;
}CNAS_NVIM_1X_OOS_SYS_ACQ_STRATEGY_PATTERN_STRU;


typedef struct
{
    CNAS_NVIM_1X_OOS_SYS_ACQ_STRATEGY_CTRL_STRU             stCtrlInfo;
    CNAS_NVIM_1X_OOS_SYS_ACQ_STRATEGY_PATTERN_STRU          astSysAcqPhaseInfo[CNAS_NVIM_MAX_1X_OOC_SCHEDULE_PHASE_NUM];
}CNAS_NVIM_1X_OOS_SYS_ACQ_STRATEGY_CFG_STRU;


typedef struct
{
    VOS_UINT8                           uc1xOosReportEnable;
    VOS_UINT8                           uc1xMtServiceExceptionReportEnable;
    VOS_UINT8                           ucHrpdUatiFailReportEnable;
    VOS_UINT8                           ucHrpdSessionFailReportEnable;
    VOS_UINT8                           ucHrpdSessionExceptionDeactReportEnable;
    VOS_UINT8                           uctHrpdOrLteOosReportEnable;
    VOS_UINT8                           uctXregResltReportEnable;
    VOS_UINT8                           ucXsmsReportEnable;
    VOS_UINT8                           ucVolteIms1xSwitchReportEnable;
    VOS_UINT8                           ucReserve1;
    VOS_UINT8                           ucReserve2;
    VOS_UINT8                           ucReserve3;
    VOS_UINT8                           ucReserve4;
    VOS_UINT8                           ucReserve5;
    VOS_UINT8                           ucReserve6;

    VOS_UINT8                           ucReserve7;
    VOS_UINT8                           ucReserve8;
    VOS_UINT8                           ucReserve9;
    VOS_UINT8                           ucReserve10;
    VOS_UINT8                           ucReserve11;
    VOS_UINT8                           ucReserve12;
    VOS_UINT8                           ucReserve13;
    VOS_UINT8                           ucReserve14;
    VOS_UINT8                           ucReserve15;
}NAS_NVIM_CDMA_ERR_LOG_ACTIVE_REPORT_CONTRL_STRU;


typedef struct
{
    VOS_UINT32                          ulMccNum;                               /* �������MCC�ĸ��� */
    VOS_UINT32                          aulMcc[CNAS_NVIM_HOME_MCC_MAX_NUM];     /* �������MCC��ֵ */
} CNAS_NVIM_HOME_MCC_INFO_STRU;

typedef struct
{
    VOS_UINT8                           ucEnable;
    VOS_UINT8                           ucReserved;
    VOS_UINT16                          usMru0Times;
}CNAS_NVIM_MRU0_SWITCH_ON_OOC_STRATEGY_CFG_STRU;


typedef struct
{
    VOS_UINT8                           ucMaxHistorySrchTimes;
    VOS_UINT8                           ucMaxPrefBandSrchTimes;
    VOS_UINT8                           ucBsrTimerMaxExpiredTimes; /* �͹���ģʽ�£�BSR��ʱ����ʱ�������ﵽ����ֵ�������� */
    VOS_UINT8                           ucReserve1;
    VOS_UINT16                          usBsrTimerLenWithNoMatchedMsplRec;  /* MSPL����û��ƥ��ļ�¼��BSRʱ�� */
    VOS_UINT16                          usBsrTimerLen;                      /* MSPL������ƥ��ļ�¼��BSRʱ�� */
}NAS_NVIM_SYS_ACQ_BSR_TIMER_INFO_STRU;


typedef struct
{
    VOS_UINT8                                               ucBsrTimerActivateFlag;
    VOS_UINT8                                               ucBsrPhaseNum;
    VOS_UINT8                                               ucHrpdConnBsrActiveFlg; /* HRPD�´�������ʱBSR�Ƿ�򿪱�ǡ�VOS_TRUE:��;VOS_FALSE:�ر� */
    VOS_UINT8                                               ucEhrpdConnBsrActiveFlg;/* EHRPD�´�������ʱBSR�Ƿ�򿪱�ǡ�VOS_TRUE:��;VOS_FALSE:�ر� */
    NAS_NVIM_SYS_ACQ_BSR_TIMER_INFO_STRU                    astBsrTimerInfo[NAS_NVIM_MAX_BSR_PHASE_NUM];
}NAS_NVIM_SYS_ACQ_BSR_CTRL_STRU;

typedef struct
{
    VOS_UINT8                           ucImsiPlmnListNum;              /* ���Ƶ�IMSI�б���� */
    VOS_UINT8                           ucOPlmnListNum;                 /* �û����õ�Oplmn�ĸ��� */
    VOS_UINT8                           ucRsv1;
    VOS_UINT8                           ucRsv2;
    NAS_SIM_FORMAT_PLMN_ID              astImsiPlmnList[NAS_MMC_NVIM_MAX_USER_CFG_IMSI_PLMN_NUM];
    NAS_SIM_FORMAT_PLMN_ID              astOPlmnList[NAS_MMC_NVIM_MAX_CUSTOM_SUPPLEMENT_OPLMN_NUM];
}NAS_MMC_NVIM_CUSTOM_SUPPLEMENT_OPLMN_INFO_STRU;


typedef struct
{
    VOS_UINT8                           ucOosSrchSleepCnt;
    VOS_UINT8                           ucHighPrioSrchSleepCnt;
    VOS_UINT8                           ucReserve1;
    VOS_UINT8                           ucReserve2;
    VOS_UINT8                           ucReserve3;
    VOS_UINT8                           ucReserve4;
    VOS_UINT8                           ucReserve5;
    VOS_UINT8                           ucReserve6;
}AI_MODEM_NAS_PLMN_SRCH_CFG_STRU;



typedef struct
{
    VOS_INT16                           sThresholdRsrq;                         /* ����С����RSRQ���ޣ�AI MODE�����Ż�ʱ����С��RSRQ����ڸ����ޡ�ȡֵ��Χ��[-15, -1],��λ1dB */
    VOS_INT16                           sThresholdRsrp;                         /* ����С����RSRP���ޣ�AI MODE�����Ż�ʱ����С��RSRP����ڸ����ޡ�ȡֵ��Χ��[-115 , -1],��λ1dB */
    VOS_UINT16                          usStillTimeThres;
    VOS_UINT8                           ucReserve1;
    VOS_UINT8                           ucReserve2;
}AI_MODEM_LTE_MEASURE_CFG_STRU;



typedef struct
{
    VOS_INT16                           sThresholdRssi;                         /* GSM��RSSI�ķ�Χ[-115��0] */
    VOS_UINT16                          usMeasOptimizeTimerLen;                 /* GSM�����Խ����ļ�鶨ʱ��ʱ������λ:s */
    VOS_UINT16                          usStillTimeThres;
    VOS_UINT8                           ucReserve1;
    VOS_UINT8                           ucReserve2;
}AI_MODEM_GSM_MEASURE_CFG_STRU;



typedef struct
{
    VOS_INT16                           sThresholdRscp;                         /* WCDMA��RSCP�ķ�Χ[-120��0] */
    VOS_INT16                           sThresholdEcn0;                         /* WCDMA��ECN0��Χ[-24,0] */
    VOS_UINT16                          usStillTimeThres;
    VOS_UINT8                           ucReserve1;
    VOS_UINT8                           ucReserve2;
}AI_MODEM_WCDMA_MEASURE_CFG_STRU;




typedef struct
{
    VOS_UINT8                           ucTimeStart;
    VOS_UINT8                           ucTimeEnd;
    VOS_UINT8                           ucReserve1;
    VOS_UINT8                           ucReserve2;
}AI_MODEM_TIME_CFG_STRU;




typedef struct
{
    VOS_UINT8                           ucActFlg;                               /*�Ƿ񼤻0�����1���� */
    VOS_UINT8                           ucApplyArea;                            /*��������,Ĭ��0��ʾ�������� */
    VOS_UINT8                           ucSensorIccFlg;                         /*SensorHub״̬ͨ��iccͨ�����±�־,0��ʹ��ICC��1ʹ��ICC*/
    VOS_UINT8                           ucReserve2;

    AI_MODEM_TIME_CFG_STRU              stTimeCfg;
    AI_MODEM_NAS_PLMN_SRCH_CFG_STRU     stNasSrchCfg;
    AI_MODEM_LTE_MEASURE_CFG_STRU       stLteMeasureCfg;
    AI_MODEM_GSM_MEASURE_CFG_STRU       stGasMeasureCfg;
    AI_MODEM_WCDMA_MEASURE_CFG_STRU     stWcdmaMeasureCfg;


    VOS_UINT8                           ucReserve3;
    VOS_UINT8                           ucReserve4;

    VOS_UINT8                           ucReserve5;
    VOS_UINT8                           ucReserve6;
    VOS_UINT8                           ucReserve7;
    VOS_UINT8                           ucReserve8;

    VOS_UINT8                           ucReserve9;
    VOS_UINT8                           ucReserve10;
    VOS_UINT8                           ucReserve11;
    VOS_UINT8                           ucReserve12;

    VOS_UINT8                           ucReserve13;
    VOS_UINT8                           ucReserve14;
}AI_MODEM_CFG_NVIM_STRU;




typedef struct
{
    VOS_UINT8                           ucSensorIccFlg;            /* ICCͨ���Ƿ񼤻0�����1���� */
    VOS_UINT8                           ucReserve1;          /* ����λ */
    VOS_UINT8                           ucReserve2;          /* ����λ */
    VOS_UINT8                           ucReserve3;          /* ����λ */

}OPEN_ICC_CFG_STRU;



typedef struct
{
    VOS_UINT8                           ucSarSensorFlg;            /* sar sensor hub�Ƿ񼤻0�����1���� */
    VOS_UINT8                           ucReserve1;          /* ����λ */
    VOS_UINT8                           ucReserve2;          /* ����λ */
    VOS_UINT8                           ucReserve3;          /* ����λ */

}SAR_SENSOR_HUB_CFG_STRU;


typedef struct
{
    VOS_UINT8                               ucEnableFlg;
    VOS_UINT8                               ucReserved;
    VOS_UINT16                              usFreqNum;
    CNAS_NVIM_FREQENCY_CHANNEL_STRU         astFreqList[CNAS_NVIM_MAX_CDMA_HRPD_CUSTOMIZE_PREF_CHANNELS_NUM];
} CNAS_NVIM_CDMA_HRPD_CUSTOMIZE_PREF_CHANNELS_STRU;


typedef struct
{
    VOS_UINT16                                              usImsiFileTimerLen;        /* IMSI��ȡ�ظ���ʱ��ʱ�� ����λ�� */
    VOS_UINT16                                              usIccidFileTimerLen;       /* ICCID��ȡ�ظ���ʱ��ʱ������λ�� */
    VOS_UINT16                                              usServiceFileTimerLen;     /* �����ļ���ȡ�ظ���ʱ��ʱ������λ�� */
    VOS_UINT16                                              usReserved1;
    VOS_UINT16                                              usReserved2;
    VOS_UINT16                                              usReserved3;
    VOS_UINT16                                              usReserved4;
    VOS_UINT16                                              usReserved5;
    VOS_UINT16                                              usReserved6;
    VOS_UINT16                                              usReserved7;
}NAS_NVIM_READ_USIM_FILE_TIMER_CTRL_STRU;



typedef struct
{
    VOS_UINT16                          usTimes;
    VOS_UINT16                          usTimerLen;
    VOS_UINT8                           ucRsv1;
    VOS_UINT8                           ucRsv2;
    VOS_UINT8                           ucRsv3;
    VOS_UINT8                           ucRsv4;
    VOS_UINT8                           ucRsv5;
    VOS_UINT8                           ucRsv6;
    VOS_UINT8                           ucRsv7;
    VOS_UINT8                           ucRsv8;
}NAS_NVIM_MODE_SELECTION_RETRY_TIMER_INFO_STRU;


typedef struct
{
    VOS_UINT8                                               ucRetryPhaseNum;
    VOS_UINT8                                               aucRsved[3];
    NAS_NVIM_MODE_SELECTION_RETRY_TIMER_INFO_STRU           astRetryTimerInfo[NAS_NVIM_MODE_SELECTION_RETRY_TIMER_PHASE_NUM_MAX];
}NAS_NVIM_MODE_SELECTION_RETRY_TIMER_CTRL_STRU;


typedef struct
{
    VOS_UINT8                                               ucNoCardPowerSaveFlg;         /* �޿�������Ƿ�PowerSave����ģʽ */
    VOS_UINT8                                               ucClAcqLteFlg;                /* CLģʽ���޿�������Ƿ���LTE */
    VOS_UINT8                                               ucReserve1;
    VOS_UINT8                                               ucReserve2;
}NAS_NVIM_NO_CARD_SYS_ACQ_CFG_STRU;



typedef struct
{
    VOS_UINT8                                               ucActiveFlg;

    /* ���ѵĿ��ƣ�Ĭ������VOS_TRUE */
    VOS_UINT8                                               ucIsAllowFirstSrch;

    /* ���ѵĿ��ƣ�Ĭ�ϲ�����VOS_FALSE */
    VOS_UINT8                                               ucIsAllowLastSrch;

    /* CLģʽֻ�б߽���⣬�Ƿ���GUL�Ŀ��ƣ�Ĭ����GUL��VOS_TRUE*/
    VOS_UINT8                                               ucIsOnlyBoundarySwitchMode;

    /* Retry timer len��en_NV_Item_MODE_SELECTION_RETRY_SYS_ACQ_STRATEGY�е�sleep timer��ȡ */
    NAS_NVIM_MODE_SELECTION_RETRY_TIMER_CTRL_STRU           stRsv;

    /* ģʽ�л����Ƿ���Ҫ����֪ͨ����IMSI */
    VOS_UINT8                                               ucIsNeedFastSwitchImsi;
    VOS_UINT8                                               ucRsv2;
    VOS_UINT8                                               ucRsv3;
    VOS_UINT8                                               ucRsv4;
}NAS_NVIM_MODE_SELECTION_CFG_STRU;



typedef struct
{
    VOS_UINT32                          ulCount;            /* ƹ�Ҵ��� */
    VOS_UINT32                          ulTimerLen;         /* ����ƹ�Ҵ�����Χ�ڵĳͷ�ʱ�� */
}NAS_NVIM_MODE_SELECTION_PUNISH_TIMER_INFO_STRU;


typedef struct
{
    VOS_UINT8                                               ucActiveFlg;
    VOS_UINT8                                               ucRsv1;
    VOS_UINT8                                               ucRsv2;
    VOS_UINT8                                               ucPunishPhaseNum;
    NAS_NVIM_MODE_SELECTION_PUNISH_TIMER_INFO_STRU          astPunishTimerInfo[NAS_NVIM_MODE_SELECTION_PUNISH_TIMER_PHASE_NUM_MAX];
}NAS_NVIM_MODE_SELECTION_PUNISH_CTRL_INFO_STRU;


typedef struct
{
    VOS_UINT16                          usSleepTimerLen;
    VOS_UINT16                          usRsved1;
    VOS_UINT8                           ucHistoryNum;                           /* �ڼ��ε���ʷ�� ���� PrefBand/FullBand�� */
    VOS_UINT8                           ucPrefBandNum;                          /* �ڼ��ε�PrefBand�� ���� FullBand�� */
    VOS_UINT8                           ucFullBandNum;                          /* �ڼ���FullBand����, �˽׶ν���, ������һ�׶� */
    VOS_UINT8                           ucRsved1;                               /* Ԥ�� */
    VOS_UINT8                           ucRsved2;
    VOS_UINT8                           ucRsved3;
    VOS_UINT8                           ucRsved4;
    VOS_UINT8                           ucRsved5;
}NAS_NVIM_MODE_SELECTION_RETRY_SYS_ACQ_PHASE_STRU;


typedef struct
{
    VOS_UINT8                                               ucActiveFlg;
    VOS_UINT8                                               ucPhaseNum;
    VOS_UINT8                                               ucRsved1;
    VOS_UINT8                                               ucRsved2;
    NAS_NVIM_MODE_SELECTION_RETRY_SYS_ACQ_PHASE_STRU        astSysAcqPhase[NAS_NVIM_MODE_SELECTION_RETRY_SYS_ACQ_PHASE_NUM_MAX];
}NAS_NVIM_MODE_SELECTION_RETRY_SYS_ACQ_STRATEGY_STRU;


typedef struct
{
    VOS_UINT8                           ucHistoryCfgSwitch;                                               /* History�������ܿؿ��ء�0:History�������Ͳ������á�  1:History�������Ϳ�����*/
    VOS_UINT8                           ucPrefBandCfgSwitch;                                              /* PrefBand�������ܿؿ��ء�0:PrefBand�������Ͳ������á�1:PrefBand�������Ϳ�����*/
    VOS_UINT8                           ucRsv1;                                                           /* RFU */
    VOS_UINT8                           ucRsv2;                                                           /* RFU */
    VOS_UINT8                           aucHistoryActiveFlg[NAS_NVIM_CUSTOMIZE_MAX_CL_ACQ_SCENE_NUM];      /* ucHistoryCfgSwitchΪ0ʱ�����������塣
                                                                                                                        �����±������History�����͵�����������ÿ������Ԫ��:
                                                                                                                        0��ʾ�ó���������History������;
                                                                                                                        1��ʾ�ó�������History�����͡�
                                                                                                                        �����±��Ӧ�ľ��������������ṹ��NAS_MSCC_NVIM_SYS_ACQ_SCENE_ENUM_UINT32 */
    VOS_UINT8                           aucPrefBandActiveFlg[NAS_NVIM_CUSTOMIZE_MAX_CL_ACQ_SCENE_NUM];    /* ucPrefBandCfgSwitchΪ0ʱ�����������塣
                                                                                                                          �����±������PrefBand�����͵�����������ÿ������Ԫ��:
                                                                                                                          0��ʾ�ó���������PrefBand������;
                                                                                                                          1��ʾ�ó�������PrefBand�����͡�
                                                                                                                          �����±��Ӧ�ľ��������������ṹ��NAS_MSCC_NVIM_SYS_ACQ_SCENE_ENUM_UINT32 */
    VOS_UINT8                           ucRsv3;                                                           /* RFU */
    VOS_UINT8                           ucRsv4;                                                           /* RFU */
    VOS_UINT8                           ucRsv5;                                                           /* RFU */
    VOS_UINT8                           ucRsv6;                                                           /* RFU */
}NAS_NVIM_CL_SYS_ACQ_TYPE_CTRL_CFG_STRU;


typedef struct
{
    VOS_UINT16                                              usOosModeSwitchSrchTimerLen;     /* MMC OOSģʽ�л�����ʱ�� */
    VOS_UINT16                                              usPowerOnModeSwitchSrchTimerLen; /* MMC ���������£�ģʽ�л�����ʱ�� */
    VOS_UINT8                                               ucOosModeSwitchSrchTimes;        /* MMC OOS�£�ģʽ�л��������� */
    VOS_UINT8                                               ucPowerOnModeSwitchSrchTimes;    /* MMC ���������£�ģʽ�л��������� */
    VOS_UINT8                                               aucReserved[2];
}NAS_MMC_NVIM_OOS_MODE_SWITCH_SRCH_CTRL_CFG_STRU;


typedef struct
{
    VOS_UINT16                                              usOosModeSwitchSrchTimerLen;     /* MMC OOSģʽ�л�����ʱ�� */
    VOS_UINT16                                              usPowerOnModeSwitchSrchTimerLen; /* MMC ���������£�ģʽ�л�����ʱ�� */
    VOS_UINT8                                               ucOosModeSwitchSrchTimes;        /* MMC OOS�£�ģʽ�л��������� */
    VOS_UINT8                                               ucPowerOnModeSwitchSrchTimes;    /* MMC ���������£�ģʽ�л��������� */
    VOS_UINT8                                               aucReserved[2];
}CNAS_XSD_NVIM_OOS_MODE_SWITCH_SRCH_CTRL_CFG_STRU;


typedef struct
{
    VOS_UINT8                                               ucIsPowerupRegAdv;              /* �Ƿ񽫿���ע����ǰ,VOS_TRUEΪ��ǰ,VOS_FALSEΪ����ǰ */
    VOS_UINT8                                               aucRsv[3];
}CNAS_NVIM_1X_REG_CFG_INFO_STRU;


typedef struct
{
    VOS_UINT8                               ucEnableFlg;
    VOS_UINT8                               ucReserved1;
    VOS_UINT8                               ucReserved2;
    VOS_UINT8                               ucReserved3;
} NAS_NVIM_SMS_FAIL_LINK_CTRL_CFG_STRU;


typedef struct
{
    VOS_UINT8                               ucEnableFlg;
    VOS_UINT8                               ucReserved1;
    VOS_UINT8                               ucReserved2;
    VOS_UINT8                               ucReserved3;
} NAS_NVIM_HIGH_SPEED_MODE_RPT_CFG_STRU;


typedef struct
{
    VOS_UINT16                         usSidRangeStart;                                      /*SID ��Χ��ʼֵ */
    VOS_UINT16                         usSidRangeEnd;                                        /*SID ��Χ����ֵ */
    VOS_UINT32                         ulMcc;                                                /*mobile country code */
}NAS_NVIM_SID_AND_MCC_INFO_STRU;


typedef struct
{
    VOS_UINT32                          ulNum;                                                /* �й��߽����������Ŀ */
    NAS_NVIM_SID_AND_MCC_INFO_STRU      astSidMccInfo[NAS_NVIM_CHINA_BOUNDARY_NETWORK_NUM_MAX];
}NAS_NVIM_CHINA_BOUNDARY_SID_AND_MCC_INFO_STRU;



typedef struct
{
    VOS_UINT8                                               ucActiveFlg;
    VOS_UINT8                                               ucHomeNetworkNum;               /* �й�home������Ŀ */
    VOS_UINT8                                               ucRsved1;
    VOS_UINT8                                               ucRsved2;
    NAS_NVIM_SID_AND_MCC_INFO_STRU                          astSidRangeMccInfo[NAS_NVIM_CHINA_HOME_NETWORK_NUM_MAX];
}NAS_NVIM_CHINA_HOME_SID_AND_MCC_INFO_STRU;


typedef struct
{
    VOS_UINT8                                               ucReestEnableFlg;                                                               /* �Ƿ�֧�ֱ����ؽ��� */
    VOS_UINT8                                               ucProtectTimerLen;                                                              /* ���Ա�����ʱ��ʱ�� */
    VOS_UINT8                                               ucIntervalTimerLen;                                                             /* ���Լ����ʱ��ʱ�� */
    VOS_UINT8                                               ucMtEstCnfReestCauseNum;                                                        /* est_cnf�����Ե�ԭ��ֵ���� */
    VOS_UINT8                                               aucMtEstCnfReestCause[NAS_NVIM_1X_MT_EST_CNF_REEST_CAUSE_MAX_NUM];              /* est_cnf����ԭ��ֵ */
    VOS_UINT8                                               ucMtTerminateIndReestCauseNum;                                                  /* terminate_ind�����Ե�ԭ��ֵ���� */
    VOS_UINT8                                               aucMtTerminateIndReestCause[NAS_NVIM_1X_MT_TERMINATE_IND_REEST_CAUSE_MAX_NUM];  /* terminate_ind����ԭ��ֵ */
}NAS_NVIM_1X_MT_NVIM_REEST_CFG_STRU;


typedef struct
{
    VOS_UINT8                               ucQuickDisplayEnableFlg;
    VOS_UINT8                               ucReserved1;
    VOS_UINT8                               ucReserved2;
    VOS_UINT8                               ucReserved3;
} NAS_NVIM_POWER_ON_QUICK_DISPLAY_NORMAL_SERVICE_OPTIMIZE_INFO_STRU;


typedef struct
{
    VOS_UINT8                           ucZeroPlmnInvalidFlag;                         /* TMSI��P_TMSI�ط����PLMNΪ0�Ƿ���Ч�ı�־ */
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucReserved2;
    VOS_UINT8                           ucReserved3;
}NAS_NVIM_TMSI_OR_P_TMSI_REALLOC_PLMN_VALID_CFG_STRU;


typedef struct
{
    VOS_UINT8                           ucLogPrintMaxCnt;                       /* ���LOG��ӡ���� */
    VOS_UINT8                           ucReserve1;
    VOS_UINT8                           ucReserve2;
    VOS_UINT8                           ucReserve3;
    VOS_UINT8                           ucReserve4;
    VOS_UINT8                           ucReserve5;
    VOS_UINT8                           ucReserve6;
    VOS_UINT8                           ucReserve7;
} NAS_NVIM_LOG_PRINT_CFG_STRU;


typedef struct
{
    VOS_UINT16                          usGsmA5;
    VOS_UINT8                           aucReserve[2];
}NAS_NVIM_GSM_A5_STRU;

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
    VOS_UINT32                          ul1xRegFailRptTimerInterval;       /* CLģʽ�£�1xע��ʧ��CHR�ϱ�ʱ����,��λ:�� */
    VOS_UINT32                          ulReserved4;                       /* ����λ */
}NAS_NVIM_OOS_CHR_POWER_SAVE_CFG_STRU;


typedef struct
{
    VOS_UINT8                               ucNvActiveFlg;                             /* NV�Ƿ񼤻� */
    VOS_UINT8                               ucT310CHREnableFlg;                        /* ����310��ʱ��CHR�ϱ� */
    VOS_UINT8                               ucT310Len;
    VOS_UINT8                               ucReserved1;
} NAS_CC_NVIM_T310_CFG_STRU;


typedef struct
{
    VOS_UINT8                           ucEmcCateSupportEcallFlag;
    VOS_UINT8                           ucBit8Is1OtherBitOKFlag;
    VOS_UINT8                           ucReserved2;
    VOS_UINT8                           ucReserved3;
}NAS_NVIM_EMC_CATE_SUPPORT_ECALL_CFG_STRU;


typedef struct
{
    VOS_UINT8                               ucNvActiveFlg;                             /* NV�Ƿ񼤻� */
    VOS_UINT8                               ucProtectTimerLen;                         /* ������ʱ��ʱ�� */
    VOS_UINT8                               ucReserved1;
    VOS_UINT8                               ucReserved2;
} NAS_NVIM_MO_COLLISION_STRU;


typedef struct
{
    VOS_UINT8                           ucSrvccNoCallNumT3240Flag;              /* 0��Ĭ��ֵ������ԭ���߼��������ͷ����ӣ����޸�T3240��ʱ��ʱ��
                                                                                   1���������ͷ����ӣ��޸�T3240��ʱ��ʱ��ΪulSrvccNoCallNumT3240Len ms */
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucReserved2;
    VOS_UINT8                           ucReserved3;
    VOS_UINT32                          ulSrvccNoCallNumT3240Len;               /* �޸�T3240��ʱ��ʱ������λms��Ĭ��ֵ��1000 ms */
}NAS_NVIM_SRVCC_NO_CALL_NUM_T3240_CFG_STRU;


typedef struct
{
    VOS_UINT8                           ucClamk2RptFlag;                        /* GU��RAU ATTACH�Ƿ��classmark 2 */
    VOS_UINT8                           ucClamk3RptFlag;                        /* GU��RAU ATTACH�Ƿ��classmark 3 */
    VOS_UINT8                           ucSuppCodecRptFlag;                     /* GU��RAU ATTACH�Ƿ��supported codecs */
    VOS_UINT8                           ucReserved;                             /* ����λ */
}NAS_NVIM_CLASSMARK_SUPPCODEC_CAPRPT_CTRL_STRU;


typedef struct
{
    VOS_UINT8                           ucSsConnStateRcvRelIndSetCauseFlg;      /* SS����״̬�յ�REL IND�Ƿ�Ҫ����ԭ��ֵ��־:
                                                                                   0--������ԭ��ֵ��
                                                                                   1--����ԭ��ֵ */
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucReserved2;
    VOS_UINT8                           ucReserved3;

}NAS_NVIM_SS_CONN_STATE_RCV_REL_IND_SET_CAUSE_STRU;



typedef struct
{
    VOS_UINT8                           ucMtS12DiscardSysInfoFlg;                  /* �յ�paging����est req������յ�ϵͳ��Ϣ���Ƿ���������� */
    VOS_UINT8                           ucChrEnableFlg;                            /* ���ֳ����µ�����CHR�ϱ��Ƿ�ʹ�� */
    VOS_UINT8                           ucReserved1;
    VOS_UINT8                           ucReserved2;
}NAS_NVIM_MT_MM_S12_RCV_SYSINFO_CFG_STRU;




/*****************************************************************************
  6 UNION
*****************************************************************************/


/*****************************************************************************
  7 Extern Global Variable
*****************************************************************************/


/*****************************************************************************
  8 Fuction Extern
*****************************************************************************/


/*****************************************************************************
  9 OTHERS
*****************************************************************************/










#if (VOS_OS_VER != VOS_WIN32)
#pragma pack()
#else
#pragma pack(pop)
#endif





#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of NasNvInterface.h */
