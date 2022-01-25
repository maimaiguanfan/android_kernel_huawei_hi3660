/*
* Copyright (C) Huawei Technologies Co., Ltd. 2012-2018. All rights reserved.
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

#ifndef __LNASNVINTERFACE_H__
#define __LNASNVINTERFACE_H__

/*****************************************************************************
  1 Include Headfile
*****************************************************************************/
#include  "AppNasComm.h"


/*****************************************************************************
  1.1 Cplusplus Announce
*****************************************************************************/
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  #pragma pack(*)    �����ֽڶ��뷽ʽ
*****************************************************************************/
#if (VOS_OS_VER != VOS_WIN32)
#pragma pack(4)
#else
#pragma pack(push, 4)
#endif

/*****************************************************************************
  2 Macro
*****************************************************************************/


/*****************************************************************************
  3 Massage Declare
*****************************************************************************/


/*****************************************************************************
  4 Enum
*****************************************************************************/

/*Added for DATA RETRY PHASEII 2016-05-23 start*/
#define LNAS_NV_DATA_RETRY_EMM_FAIL_CAUSE_EVENT_CNT     (9)
#define LNAS_NV_DATA_RETRY_EMM_FAIL_CAUSE_NUM           (6)

/*Added for DATA RETRY PHASEII 2016-05-23 end*/

/*Added for DATA RETRY PHASEII 2016-05-24 start*/
#define LNAS_NV_MAX_APN_CONTEXT_NUM               6
#define LNAS_NV_MAX_APN_LEN                       99

/*Added for DATA RETRY PHASEII 2016-05-24 end*/
/* mod for Attach/Tau Rej#17#19 Keep Conn, 2016-11-25, Begin */
#define LNAS_NV_ATTACH_REJ_NOT_REL_MAX_CAUSE_NUM  10
#define LNAS_NV_TAU_REJ_NOT_REL_MAX_CAUSE_NUM     10
/* mod for Attach/Tau Rej#17#19 Keep Conn, 2016-11-25, End */
/* Added for BOSTON_R13_CR_PHASEIII 2017-01-16 begin */
#define LNAS_NV_ACDC_APP_MAX_NUM                  4
#define LNAS_NV_ACDC_OSID_LEN                (16)
#define LNAS_NV_ACDC_MAX_APPID_LEN           (128)
/* Added for BOSTON_R13_CR_PHASEIII 2017-01-16 end */

/*Added for MT-DETACH issue 2017-04-13 start*/
#define LNAS_NV_MT_DETACH_OPT_OTHER_CAUSE_CNT (32)
/*Added for MT-DETACH issue 2017-04-13 end*/
/************************stNasFunFlag02 Begin***************************/

/*****************************************************************************
  5 STRUCT
*****************************************************************************/
/*****************************************************************************
�ṹ��    :RRC_PLMN_ID_STRU��
Э����  :
ASN.1���� :
�ṹ˵��  :
    MCC, Mobile country code (aucPlmnId[0], aucPlmnId[1] bits 1 to 4)
    MNC, Mobile network code (aucPlmnId[2], aucPlmnId[1] bits 5 to 8).

    The coding of this field is the responsibility of each administration but BCD
    coding shall be used. The MNC shall consist of 2 or 3 digits. For PCS 1900 for NA,
    Federal regulation mandates that a 3-digit MNC shall be used. However a network
    operator may decide to use only two digits in the MNC over the radio interface.
    In this case, bits 5 to 8 of octet 4 shall be coded as "1111". Mobile equipment
    shall accept MNC coded in such a way.

    ---------------------------------------------------------------------------
                 ||(BIT8)|(BIT7)|(BIT6)|(BIT5)|(BIT4)|(BIT3)|(BIT2)|(BIT1)
    ---------------------------------------------------------------------------
    aucPlmnId[0] ||    MCC digit 2            |           MCC digit 1
    ---------------------------------------------------------------------------
    aucPlmnId[1] ||    MNC digit 3            |           MCC digit 3
    ---------------------------------------------------------------------------
    aucPlmnId[2] ||    MNC digit 2            |           MNC digit 1
    ---------------------------------------------------------------------------

    AT���
    at+cops=1,2,"mcc digit 3, mcc digit 2, mcc digit 1, mnc digit 3, mnc digit 2, mnc digit 1",2 :

    e.g.
    at+cops=1,2,"789456",2 :
    --------------------------------------------------------------------------------
    (mcc digit 3)|(mcc digit 2)|(mcc digit 1)|(mnc digit 3)|(mnc digit 2)|(mnc digit 1)
    --------------------------------------------------------------------------------
       7         |     8       |      9      |     4       |      5      |     6
    --------------------------------------------------------------------------------

    ��aucPlmnId[3]�еĴ�Ÿ�ʽ:
    ---------------------------------------------------------------------------
                 ||(BIT8)|(BIT7)|(BIT6)|(BIT5)|(BIT4)|(BIT3)|(BIT2)|(BIT1)
    ---------------------------------------------------------------------------
    aucPlmnId[0] ||    MCC digit 2 = 8        |           MCC digit 1 = 9
    ---------------------------------------------------------------------------
    aucPlmnId[1] ||    MNC digit 3 = 4        |           MCC digit 3 = 7
    ---------------------------------------------------------------------------
    aucPlmnId[2] ||    MNC digit 2 = 6        |           MNC digit 1 = 5
    ---------------------------------------------------------------------------
*****************************************************************************/
typedef struct
{
    VOS_UINT8                           aucPlmnId[3];
    VOS_UINT8                           ucReserved;
}LRRC_LNAS_PLMN_ID_STRU;


typedef struct
{
    VOS_UINT32                          bitOpUeNetCap   :1;
    VOS_UINT32                          bitRsv          :31;

    NAS_MM_UE_NET_CAP_STRU              stUeNetCap;
}LNAS_LMM_NV_UE_NET_CAP_STRU;



typedef struct
{
    VOS_UINT32                          bitOpImsi     :1;
    VOS_UINT32                          bitOpRsv      :31;

    VOS_UINT8                           aucImsi[NAS_MM_MAX_UEID_BUF_SIZE];
    VOS_UINT8                           aucReserved1[2];
}LNAS_LMM_NV_IMSI_STRU;

/* LEQUIP_NVʹ�� */
typedef struct
{
    VOS_UINT32                          bitOpImei     :1;
    VOS_UINT32                          bitOpRsv      :31;

    VOS_UINT8                           aucImei[NAS_MM_MAX_UEID_BUF_SIZE];
    VOS_UINT8                           aucReserved1[2];
}LNAS_LMM_NV_IMEI_STRU;

typedef struct
{
    VOS_UINT32                          bitOpGuti     :1; /* 1: VALID; 0: INVALID*/
    VOS_UINT32                          bitOpTai      :1;
    VOS_UINT32                          bitOpUpState  :1;
    VOS_UINT32                          bitOpRsv      :29;

    NAS_MM_GUTI_STRU                    stGuti;
    NAS_MM_TA_STRU                      stLastRegTai;
    NAS_MM_UPDATE_STATE_ENUM_UINT32     enUpdateState;
}LNAS_LMM_NV_EPS_LOC_STRU;


typedef struct
{
    VOS_UINT32                          bitOpMsClassMark :1; /* 1: VALID; 0: INVALID*/
    VOS_UINT32                          bitOpRsv      :31;
    NAS_MM_MS_CLASSMARK_STRU            stMsClassMark;
}LNAS_LMM_NV_MS_CLASSMARK_STRU;


typedef struct
{
    VOS_UINT32                          bitOpAccClassMark :1; /* 1: VALID; 0: INVALID*/
    VOS_UINT32                          bitOpRsv      :31;
    VOS_UINT16                          usAccClassMark;
    VOS_UINT8                           aucReserved1[2];
}LNAS_LMM_NV_ACC_CLASSMARK_STRU;


typedef struct
{
    VOS_UINT32                          bitOpEpsSec   :1;       /* 1: VALID; 0: INVALID*/
    VOS_UINT32                          bitOpRsv      :31;

    VOS_UINT8                           ucKSIsgsn;
    VOS_UINT8                           ucKSIasme;
    VOS_UINT8                           ucSecuAlg;
    VOS_UINT8                           ucRsv;
    VOS_UINT8                           aucKasme[NAS_MM_AUTH_KEY_ASME_LEN];
    VOS_UINT32                          ulUlNasCount;
    VOS_UINT32                          ulDlNasCount;
}LNAS_LMM_NV_EPS_SEC_CONTEXT_STRU;
/* modified 2012-07-31 cs+ps1 begin */
typedef struct
{
    VOS_UINT32                          bitOpUeCenter       :1;       /* 1: VALID; 0: INVALID*/
    VOS_UINT32                          bitOpRsv            :31;

    LNAS_LMM_UE_CENTER_ENUM_UINT32      enUeCenter;
}LNAS_LMM_NV_UE_CENTER_STRU;
/* modified 2012-07-31 cs+ps1 end */


typedef struct
{
    VOS_UINT32                          bitOpVoicDomain     :1;       /* 1: VALID; 0: INVALID*/
    VOS_UINT32                          bitOpRsv            :31;
    NAS_LMM_VOICE_DOMAIN_ENUM_UINT32    enVoicDomain;
}LNAS_LMM_NV_VOICE_DOMAIN_STRU;

/*R10 modify begin for */
typedef struct
{
    VOS_UINT32                          bitOpReleaseName    :1;       /* 1: VALID; 0: INVALID*/
    VOS_UINT32                          bitOpRsv            :31;

    VOS_UINT32                          ulReleaseName;

}LNAS_LMM_NV_NAS_RELEASE_STRU;
/*R10 modify end for */

/* begin for r11 2014-09-18 */
typedef struct
{
    VOS_UINT32                          bitOpLocalIpCap     :1;       /* 1: VALID; 0: INVALID*/
    VOS_UINT32                          bitOpRsv            :31;

    VOS_UINT32                          ulLocalIpCap;

}LNAS_LMM_NV_NAS_LOCALIP_CAP_STRU;

/* end for r11 2014-09-18 */

/*self-adaption NW cause modify begin for */
typedef LNAS_LMM_CONFIG_NWCAUSE_STRU        LNAS_LMM_NV_CONFIG_NWCAUSE_STRU;

/*self-adaption NW cause modify end for */


/* mod for AT&T program 2015-01-15 DTS begin */
/* Modified for GU_BACK_OFF,2016-04-07,Begin */
/*****************************************************************************
�ṹ����    :LNAS_ESM_NV_BACKOFF_CONFIG_PARA_STRU
ʹ��˵��    :DAM�����NV���ݽṹ
*****************************************************************************/
typedef struct
{
    VOS_UINT32                           bitOpBackOffAlg   :1; /* Back-off�㷨����     */
    VOS_UINT32                           bitOpBackOffFx    :1; /* NV��Fxֵ�Ƿ���Ч     */
    VOS_UINT32                           bitOpShareEntityFlag :1;  /* �Ƿ���һ��ʵ�壻0 ��1 �� */
    VOS_UINT32                           bitOpRsv          :29;

    NAS_BACKOFF_RAT_SUPPORT_ENUM_UINT32  enBackOffRatSupport;  /* LTE��GU�㷨��Чö��  */
    NAS_CONFIG_BACKOFF_FX_PARA_STRU      stBackOffFx;          /* Fx����               */
    NAS_CONFIG_PDP_PERM_CAUSE_STRU       stPdpPermCause;       /* ���þܾ�ԭ��ֵ�б�   */
    NAS_CONFIG_PDP_TEMP_CAUSE_STRU       stPdpTempCause;       /* ��ʱ�ܾ�ԭ��ֵ�б�   */
}NAS_BACKOFF_NV_BACKOFF_CONFIG_PARA_STRU;
/* Modified for GU_BACK_OFF,2016-04-07,End */
/* mod for AT&T program 2015-01-15 DTS end */

/*Add for 4G No Account 2015-3-25 DTS start*/
/*****************************************************************************
�ṹ����    :LNAS_LMM_NV_LTE_NO_ACCOUNT_CONFIG_STRU
ʹ��˵��    :���LTEδ������������NV
*****************************************************************************/
typedef struct
{
    VOS_UINT8                           ucLteNoSubscribeVplmnSwitch;    /*4Gδ�������Կ���, VPLMN���أ�1: �򿪣� 0: �ر�*/
    VOS_UINT8                           ucLteNoSubscribeHplmnSwitch;    /*4Gδ�������Կ��أ�HPLMN����, 1: �򿪣� 0: �ر�*/
    VOS_UINT8                           ucDiscardExtendedEmmCauseFlag;  /*�Ƿ����������attach rej��tau rej�еĸ���ԭ��ֵ��
                                                                          0:������,1:���ԣ�����ܲ��Կ�����*/
    VOS_UINT8                           ucRsv2;                         /*����λ*/
    VOS_UINT32                          ulPublishmentTimerVplmnLen;     /*�ͷ���ʱ��ʱ��VPLMN*/
    VOS_UINT32                          ulPublishmentTimerHplmnLen;     /*�ͷ���ʱ��ʱ��HPLMN*/
}LNAS_LMM_NV_LTE_NO_SUBSCRIBE_CONFIG_STRU;
/*Add for 4G No Account 2015-3-25 DTS end*/

/* mod for Attach/Tau Rej#17#19 Keep Conn, 2016-11-25, Begin */
/*****************************************************************************
�ṹ����    :LNAS_LMM_NV_ATTACHTAU_REJ1719_NOT_REL_STRU
ʹ��˵��    :DOCOMO����ATTACH/TAU��17/19�ܾ����ͷ���·
*****************************************************************************/
typedef struct
{
    VOS_UINT8                           ucSwitch;    /*���Կ���,1: ��,�����ͷ���· 0: �ر�*/
    VOS_UINT8                           ucAttachCauseNum;
    VOS_UINT8                           aucAttachCause[LNAS_NV_ATTACH_REJ_NOT_REL_MAX_CAUSE_NUM];
    VOS_UINT8                           ucTauCauseNum;
    VOS_UINT8                           aucTauCause[LNAS_NV_TAU_REJ_NOT_REL_MAX_CAUSE_NUM];
    VOS_UINT8                           ucRsv;
}LNAS_LMM_NV_ATTACHTAU_REJ_NOT_REL_STRU;
/* mod for Attach/Tau Rej#17#19 Keep Conn, 2016-11-25, End */

/* Added for load balance TAU 2016-12-20 start*/
/*****************************************************************************
�ṹ����    :LNAS_LMM_NV_LOAD_BALANCE_TAU_CONTROL_CONFIG_STRU
ʹ��˵��    :����Ƶ���·��ͷ�Я��load balance����UEƵ����TAU���ڿ���
*****************************************************************************/
typedef struct
{
    VOS_UINT8                           ucSwitch;                   /*���Կ���,1: ��,���򿪵�ǰƵ����TAU�ĵ��ڿ��� 0: �ر�*/
    VOS_UINT8                           aucRsv[3];
    VOS_UINT32                          ulThrotCtlTimeLen;          /*���ڿ��Ƶ�ʱ������λ: ����*/
}LNAS_LMM_NV_LOAD_BALANCE_TAU_CONTROL_CONFIG_STRU;
/* Added for load balance TAU 2016-12-20 end*/

/* mod for AT&T program 2015-01-04 DTS begin */
/*****************************************************************************
�ṹ����    :LNAS_LMM_NV_COMM_CONFIG_PARA_STRU
ʹ��˵��    :LNAS����LMM��������Ĺ���NV���ݽṹ������LMM��صĹ�����������NV����
             ��NV�н������ӣ�����ά���Ϳ���
*****************************************************************************/
typedef struct
{
    /* Added for DATA RETRY PHASEIV, 2016-7-25, begin */
    VOS_UINT8                            ucMaxRej19AtmptCnt; /*  0: ��Э������ֱ������Ϊ5 ; ����ֵΪ����Դ���*/
    /* Added for DATA RETRY PHASEIV, 2016-7-25, end */
    VOS_UINT8                            ucRsv0[3];           /* ���������Ժ������������� */
    LNAS_LMM_NV_LTE_NO_SUBSCRIBE_CONFIG_STRU    stLteNoSubscribeConfig; /* 4Gδ��������ҪNV*/
    VOS_UINT32                           ulRsv1[13];          /* ���������Ժ������������� */
/* mod for AT&T program phaseIII 2015-03-15 DTS begin */
    NAS_TMO_IMSI_HPLMN_LIST              stTmoImsiHplmnList;  /* TMO����������Ч��IMSI PLMN�б� */
/* mod for AT&T program phaseIII 2015-03-15 DTS end */
    VOS_UINT8                            ucRsv2[28];          /* ���������Ժ������������� */
}LNAS_LMM_NV_COMM_CONFIG_PARA_STRU;

/* mod for AT&T program 2015-01-04 DTS end */

typedef struct
{
    VOS_UINT32                          bitOpAttachBearerReest   :1;   /* 1: VALID; 0: INVALID*/
    VOS_UINT32                          bitOpRsv                 :31;
    VOS_UINT32                          ulReestTimeLen;
}LNAS_ESM_NV_ATTACH_BEARER_REEST_STRU;
/* 2015-5-27 begin */
/*****************************************************************************
 �ṹ��    : LNAS_FUN_FLAG_NV_BIT_STRU_1
 Э����  :
 ASN.1���� :
 �ṹ˵��  : Э��ջ�ĺ�Э�鹦����صĿ��صĽṹ��
*****************************************************************************/
typedef struct
{
    VOS_UINT32  bitOpKeyInfoFlag                       :1;/* Lnas��ά�ɲ�ؼ���Ϣ�ϱ����ƿ���,Ĭ��ֵ:1 */
    VOS_UINT32  bitOpKeyEventFlag                      :1;/* Lnas��ά�ɲ�ؼ��¼��ϱ����ƿ���,Ĭ��ֵ:1 */
    VOS_UINT32  bitOpApiFlag                           :1;/* Lnas��ά�ɲ�API��Ϣ�ϱ����ƿ���,Ĭ��ֵ:1 */
    VOS_UINT32  bitOpSuccRatioFlag                     :1;/* Lnas��ά�ɲ�ɹ�����Ϣ�ϱ����ƿ���,Ĭ��ֵ:1 */
    VOS_UINT32  bitOpDelayFlag                         :1;/* Lnas��ά�ɲ�ʱ����Ϣ�ϱ����ƿ���,Ĭ��ֵ:1 */
    VOS_UINT32  bitOpEsrRej39OptimizeFlag              :1;/* Lnas Esr Rej #39�����Ż����ƿ���,Ĭ��ֵ:0 */
    VOS_UINT32  bitOpNasAustraliaFlag                  :1;/* Lnas �ĵ綨�����󿪹�,Ĭ��ֵ:0 */
    VOS_UINT32  bitOpImsiAttachWithInvalidTinFlag      :1;/* Lnas DSDS2.0���� */
    VOS_UINT32  bitOpDsdsOptimizeFlag                  :1;/* Lnas SRLTE���� */
    VOS_UINT32  bitOpNasSrlteFlag                      :1;/* srlte���ƿ��� */
    VOS_UINT32  bitOpNasT3402DefaultFlag               :1;/* T3402Ĭ�϶�ʱ�����ƿ��� */
    VOS_UINT32  bitOpThrotAlgSwitchFlag                :1;/* Lnas DATA RETRY���Ե����㷨����,Ĭ��ֵ:0 */
    VOS_UINT32  bitOpUiccResetClearFlag                :1;/* uicc resetʱ���GUTI,LVR TAI������EU2�Ŀ��� */
    VOS_UINT32  bitOpDataRetryCtrlFlag                 :1;/* Lnas data retry���Կ��� */
    VOS_UINT32  bitOpTauRej17OneMoreAttachOptimFlag    :1;/* LNAS TAU#17�Ż��������ƿ��أ�Ĭ��ֵ:0 */
    VOS_UINT32  bitOpIncreaseFreqFlag                  :1;/* LNAS EMM���̷����ʱ���Ƿ����Ƶ����(Ŀǰ���漰ATTACH����)��Ĭ��ֵ:0*/
    /* Mod for DSDS CSFB_FR_DELAY CHR, 2016-08-06, begin */
    VOS_UINT32  bitOpCsfbFrChr                         :1;/* LNAS CSFB FR DEALY CHR�ϱ����ƿ��أ�Ĭ��ֵ:0 */
    /* Mod for DSDS CSFB_FR_DELAY CHR, 2016-08-06, end */
    /* Added for Boston_R13_CR_PHASEI, 2016-10-18, begin */
    VOS_UINT32  bitOpServiceCounterFlag                :1;/* Lnas Service Counter����,Ĭ��ֵ:0 */
    /* Added for Boston_R13_CR_PHASEI, 2016-10-18, begin */
    /* Added for Boston_R13_CR_PHASEII 2016-12-06 begin */
    VOS_UINT32  bitOpAttachWithImsiFlag                 :1;/* Lnas Attach with IMSI���Կ���,Ĭ��ֵ:0 */
    /* Added for Boston_R13_CR_PHASEII 2016-12-06 end */
    /* Added for BOSTON_R13_CR_PHASEIII 2017-01-16 begin */
    VOS_UINT32  bitOpAcdcFlag                           :1;/* ACDC���Կ���,Ĭ��ֵ:0 */
    /* Added for BOSTON_R13_CR_PHASEIII 2017-01-16 end */
    /* Added for Boston_R13_CR_PHASEIII, 2017-01-12, begin */
    VOS_UINT32  bitOpAttachCause19WithoutEmmTimerFlag   :1;/*��APNע��ʱ��ֹͣT3411/T3402�����Կ��أ�Ĭ��ֵ�ر�:0*/
    /* Added for Boston_R13_CR_PHASEIII, 2017-01-12, end */
    VOS_UINT32  bitFlag22                   :1;
    VOS_UINT32  bitFlag23                   :1;
    VOS_UINT32  bitFlag24                   :1;
    VOS_UINT32  bitFlag25                   :1;
    VOS_UINT32  bitFlag26                   :1;
    VOS_UINT32  bitFlag27                   :1;
    VOS_UINT32  bitFlag28                   :1;
    VOS_UINT32  bitFlag29                   :1;
    VOS_UINT32  bitFlag30                   :1;
    VOS_UINT32  bitFlag31                   :1;
    VOS_UINT32  bitFlag32                   :1;
}LNAS_FUN_FLAG_NV_BIT_STRU_1;

/*****************************************************************************
 �ṹ��    : LNAS_FUN_FLAG_NV_BIT_STRU_2
 Э����  :
 ASN.1���� :
 �ṹ˵��  : Э��ջ�ĺ�Э�鹦����صĿ��صĽṹ��
*****************************************************************************/
typedef struct
{
    VOS_UINT32  bitFlag01                   :1;
    VOS_UINT32  bitFlag02                   :1;
    VOS_UINT32  bitFlag03                   :1;
    VOS_UINT32  bitFlag04                   :1;
    VOS_UINT32  bitFlag05                   :1;
    VOS_UINT32  bitFlag06                   :1;
    VOS_UINT32  bitFlag07                   :1;
    VOS_UINT32  bitFlag08                   :1;
    VOS_UINT32  bitFlag09                   :1;
    VOS_UINT32  bitFlag10                   :1;
    VOS_UINT32  bitFlag11                   :1;
    VOS_UINT32  bitFlag12                   :1;
    VOS_UINT32  bitFlag13                   :1;
    VOS_UINT32  bitFlag14                   :1;
    VOS_UINT32  bitFlag15                   :1;
    VOS_UINT32  bitFlag16                   :1;
    VOS_UINT32  bitFlag17                   :1;
    VOS_UINT32  bitFlag18                   :1;
    VOS_UINT32  bitFlag19                   :1;
    VOS_UINT32  bitFlag20                   :1;
    VOS_UINT32  bitFlag21                   :1;
    VOS_UINT32  bitFlag22                   :1;
    VOS_UINT32  bitFlag23                   :1;
    VOS_UINT32  bitFlag24                   :1;
    VOS_UINT32  bitFlag25                   :1;
    VOS_UINT32  bitFlag26                   :1;
    VOS_UINT32  bitFlag27                   :1;
    VOS_UINT32  bitFlag28                   :1;
    VOS_UINT32  bitFlag29                   :1;
    VOS_UINT32  bitFlag30                   :1;
    VOS_UINT32  bitFlag31                   :1;
    VOS_UINT32  bitFlag32                   :1;
}LNAS_FUN_FLAG_NV_BIT_STRU_2;

/*****************************************************************************
 �ṹ��    : LNAS_FUN_FLAG_NV_BIT_STRU_3
 Э����  :
 ASN.1���� :
 �ṹ˵��  : Э��ջ�ĺ�Э�鹦����صĿ��صĽṹ��
*****************************************************************************/
typedef struct
{
    VOS_UINT32  bitFlag01                   :1;
    VOS_UINT32  bitFlag02                   :1;
    VOS_UINT32  bitFlag03                   :1;
    VOS_UINT32  bitFlag04                   :1;
    VOS_UINT32  bitFlag05                   :1;
    VOS_UINT32  bitFlag06                   :1;
    VOS_UINT32  bitFlag07                   :1;
    VOS_UINT32  bitFlag08                   :1;
    VOS_UINT32  bitFlag09                   :1;
    VOS_UINT32  bitFlag10                   :1;
    VOS_UINT32  bitFlag11                   :1;
    VOS_UINT32  bitFlag12                   :1;
    VOS_UINT32  bitFlag13                   :1;
    VOS_UINT32  bitFlag14                   :1;
    VOS_UINT32  bitFlag15                   :1;
    VOS_UINT32  bitFlag16                   :1;
    VOS_UINT32  bitFlag17                   :1;
    VOS_UINT32  bitFlag18                   :1;
    VOS_UINT32  bitFlag19                   :1;
    VOS_UINT32  bitFlag20                   :1;
    VOS_UINT32  bitFlag21                   :1;
    VOS_UINT32  bitFlag22                   :1;
    VOS_UINT32  bitFlag23                   :1;
    VOS_UINT32  bitFlag24                   :1;
    VOS_UINT32  bitFlag25                   :1;
    VOS_UINT32  bitFlag26                   :1;
    VOS_UINT32  bitFlag27                   :1;
    VOS_UINT32  bitFlag28                   :1;
    VOS_UINT32  bitFlag29                   :1;
    VOS_UINT32  bitFlag30                   :1;
    VOS_UINT32  bitFlag31                   :1;
    VOS_UINT32  bitFlag32                   :1;
}LNAS_FUN_FLAG_NV_BIT_STRU_3;

/*****************************************************************************
 �ṹ��    : LNAS_FUN_FLAG_NV_BIT_STRU_4
 Э����  :
 ASN.1���� :
 �ṹ˵��  : Э��ջ�ĺ�Э�鹦����صĿ��صĽṹ��
*****************************************************************************/
typedef struct
{
    VOS_UINT32  bitFlag01                   :1;
    VOS_UINT32  bitFlag02                   :1;
    VOS_UINT32  bitFlag03                   :1;
    VOS_UINT32  bitFlag04                   :1;
    VOS_UINT32  bitFlag05                   :1;
    VOS_UINT32  bitFlag06                   :1;
    VOS_UINT32  bitFlag07                   :1;
    VOS_UINT32  bitFlag08                   :1;
    VOS_UINT32  bitFlag09                   :1;
    VOS_UINT32  bitFlag10                   :1;
    VOS_UINT32  bitFlag11                   :1;
    VOS_UINT32  bitFlag12                   :1;
    VOS_UINT32  bitFlag13                   :1;
    VOS_UINT32  bitFlag14                   :1;
    VOS_UINT32  bitFlag15                   :1;
    VOS_UINT32  bitFlag16                   :1;
    VOS_UINT32  bitFlag17                   :1;
    VOS_UINT32  bitFlag18                   :1;
    VOS_UINT32  bitFlag19                   :1;
    VOS_UINT32  bitFlag20                   :1;
    VOS_UINT32  bitFlag21                   :1;
    VOS_UINT32  bitFlag22                   :1;
    VOS_UINT32  bitFlag23                   :1;
    VOS_UINT32  bitFlag24                   :1;
    VOS_UINT32  bitFlag25                   :1;
    VOS_UINT32  bitFlag26                   :1;
    VOS_UINT32  bitFlag27                   :1;
    VOS_UINT32  bitFlag28                   :1;
    VOS_UINT32  bitFlag29                   :1;
    VOS_UINT32  bitFlag30                   :1;
    VOS_UINT32  bitFlag31                   :1;
    VOS_UINT32  bitFlag32                   :1;
}LNAS_FUN_FLAG_NV_BIT_STRU_4;

/*****************************************************************************
 �ṹ��    : LNAS_FUN_FLAG_NV_BIT_STRU_5
 Э����  :
 ASN.1���� :
 �ṹ˵��  : Э��ջ�ĺ�Э�鹦����صĿ��صĽṹ��
*****************************************************************************/
typedef struct
{
    VOS_UINT32  bitFlag01                   :1;
    VOS_UINT32  bitFlag02                   :1;
    VOS_UINT32  bitFlag03                   :1;
    VOS_UINT32  bitFlag04                   :1;
    VOS_UINT32  bitFlag05                   :1;
    VOS_UINT32  bitFlag06                   :1;
    VOS_UINT32  bitFlag07                   :1;
    VOS_UINT32  bitFlag08                   :1;
    VOS_UINT32  bitFlag09                   :1;
    VOS_UINT32  bitFlag10                   :1;
    VOS_UINT32  bitFlag11                   :1;
    VOS_UINT32  bitFlag12                   :1;
    VOS_UINT32  bitFlag13                   :1;
    VOS_UINT32  bitFlag14                   :1;
    VOS_UINT32  bitFlag15                   :1;
    VOS_UINT32  bitFlag16                   :1;
    VOS_UINT32  bitFlag17                   :1;
    VOS_UINT32  bitFlag18                   :1;
    VOS_UINT32  bitFlag19                   :1;
    VOS_UINT32  bitFlag20                   :1;
    VOS_UINT32  bitFlag21                   :1;
    VOS_UINT32  bitFlag22                   :1;
    VOS_UINT32  bitFlag23                   :1;
    VOS_UINT32  bitFlag24                   :1;
    VOS_UINT32  bitFlag25                   :1;
    VOS_UINT32  bitFlag26                   :1;
    VOS_UINT32  bitFlag27                   :1;
    VOS_UINT32  bitFlag28                   :1;
    VOS_UINT32  bitFlag29                   :1;
    VOS_UINT32  bitFlag30                   :1;
    VOS_UINT32  bitFlag31                   :1;
    VOS_UINT32  bitFlag32                   :1;
}LNAS_FUN_FLAG_NV_BIT_STRU_5;

/*****************************************************************************
 �ṹ��    : LNAS_FUN_FLAG_NV_BIT_STRU_6
 Э����  :
 ASN.1���� :
 �ṹ˵��  : Э��ջ�ĺ�Э�鹦����صĿ��صĽṹ��
*****************************************************************************/
typedef struct
{
    VOS_UINT32  bitFlag01                   :1;
    VOS_UINT32  bitFlag02                   :1;
    VOS_UINT32  bitFlag03                   :1;
    VOS_UINT32  bitFlag04                   :1;
    VOS_UINT32  bitFlag05                   :1;
    VOS_UINT32  bitFlag06                   :1;
    VOS_UINT32  bitFlag07                   :1;
    VOS_UINT32  bitFlag08                   :1;
    VOS_UINT32  bitFlag09                   :1;
    VOS_UINT32  bitFlag10                   :1;
    VOS_UINT32  bitFlag11                   :1;
    VOS_UINT32  bitFlag12                   :1;
    VOS_UINT32  bitFlag13                   :1;
    VOS_UINT32  bitFlag14                   :1;
    VOS_UINT32  bitFlag15                   :1;
    VOS_UINT32  bitFlag16                   :1;
    VOS_UINT32  bitFlag17                   :1;
    VOS_UINT32  bitFlag18                   :1;
    VOS_UINT32  bitFlag19                   :1;
    VOS_UINT32  bitFlag20                   :1;
    VOS_UINT32  bitFlag21                   :1;
    VOS_UINT32  bitFlag22                   :1;
    VOS_UINT32  bitFlag23                   :1;
    VOS_UINT32  bitFlag24                   :1;
    VOS_UINT32  bitFlag25                   :1;
    VOS_UINT32  bitFlag26                   :1;
    VOS_UINT32  bitFlag27                   :1;
    VOS_UINT32  bitFlag28                   :1;
    VOS_UINT32  bitFlag29                   :1;
    VOS_UINT32  bitFlag30                   :1;
    VOS_UINT32  bitFlag31                   :1;
    VOS_UINT32  bitFlag32                   :1;
}LNAS_FUN_FLAG_NV_BIT_STRU_6;

/*****************************************************************************
 �ṹ��    : LNAS_FUN_FLAG_NV_BIT_STRU_7
 Э����  :
 ASN.1���� :
 �ṹ˵��  : Э��ջ�ĺ�Э�鹦����صĿ��صĽṹ��
*****************************************************************************/
typedef struct
{
    VOS_UINT32  bitFlag01                   :1;
    VOS_UINT32  bitFlag02                   :1;
    VOS_UINT32  bitFlag03                   :1;
    VOS_UINT32  bitFlag04                   :1;
    VOS_UINT32  bitFlag05                   :1;
    VOS_UINT32  bitFlag06                   :1;
    VOS_UINT32  bitFlag07                   :1;
    VOS_UINT32  bitFlag08                   :1;
    VOS_UINT32  bitFlag09                   :1;
    VOS_UINT32  bitFlag10                   :1;
    VOS_UINT32  bitFlag11                   :1;
    VOS_UINT32  bitFlag12                   :1;
    VOS_UINT32  bitFlag13                   :1;
    VOS_UINT32  bitFlag14                   :1;
    VOS_UINT32  bitFlag15                   :1;
    VOS_UINT32  bitFlag16                   :1;
    VOS_UINT32  bitFlag17                   :1;
    VOS_UINT32  bitFlag18                   :1;
    VOS_UINT32  bitFlag19                   :1;
    VOS_UINT32  bitFlag20                   :1;
    VOS_UINT32  bitFlag21                   :1;
    VOS_UINT32  bitFlag22                   :1;
    VOS_UINT32  bitFlag23                   :1;
    VOS_UINT32  bitFlag24                   :1;
    VOS_UINT32  bitFlag25                   :1;
    VOS_UINT32  bitFlag26                   :1;
    VOS_UINT32  bitFlag27                   :1;
    VOS_UINT32  bitFlag28                   :1;
    VOS_UINT32  bitFlag29                   :1;
    VOS_UINT32  bitFlag30                   :1;
    VOS_UINT32  bitFlag31                   :1;
    VOS_UINT32  bitFlag32                   :1;
}LNAS_FUN_FLAG_NV_BIT_STRU_7;

/*****************************************************************************
 �ṹ��    : LNAS_FUN_FLAG_NV_BIT_STRU_8
 Э����  :
 ASN.1���� :
 �ṹ˵��  : Э��ջ�ĺ�Э�鹦����صĿ��صĽṹ��
*****************************************************************************/
typedef struct
{
    VOS_UINT32  bitFlag01                   :1;
    VOS_UINT32  bitFlag02                   :1;
    VOS_UINT32  bitFlag03                   :1;
    VOS_UINT32  bitFlag04                   :1;
    VOS_UINT32  bitFlag05                   :1;
    VOS_UINT32  bitFlag06                   :1;
    VOS_UINT32  bitFlag07                   :1;
    VOS_UINT32  bitFlag08                   :1;
    VOS_UINT32  bitFlag09                   :1;
    VOS_UINT32  bitFlag10                   :1;
    VOS_UINT32  bitFlag11                   :1;
    VOS_UINT32  bitFlag12                   :1;
    VOS_UINT32  bitFlag13                   :1;
    VOS_UINT32  bitFlag14                   :1;
    VOS_UINT32  bitFlag15                   :1;
    VOS_UINT32  bitFlag16                   :1;
    VOS_UINT32  bitFlag17                   :1;
    VOS_UINT32  bitFlag18                   :1;
    VOS_UINT32  bitFlag19                   :1;
    VOS_UINT32  bitFlag20                   :1;
    VOS_UINT32  bitFlag21                   :1;
    VOS_UINT32  bitFlag22                   :1;
    VOS_UINT32  bitFlag23                   :1;
    VOS_UINT32  bitFlag24                   :1;
    VOS_UINT32  bitFlag25                   :1;
    VOS_UINT32  bitFlag26                   :1;
    VOS_UINT32  bitFlag27                   :1;
    VOS_UINT32  bitFlag28                   :1;
    VOS_UINT32  bitFlag29                   :1;
    VOS_UINT32  bitFlag30                   :1;
    VOS_UINT32  bitFlag31                   :1;
    VOS_UINT32  bitFlag32                   :1;
}LNAS_FUN_FLAG_NV_BIT_STRU_8;

/*****************************************************************************
�ṹ����    :LNAS_LMM_NV_GRADUAL_FORBIDDEN_PARA_STRU
ʹ��˵��    :LNAS����Forbidden����NV�ṹ��
*****************************************************************************/
typedef struct
{
    VOS_UINT8                           ucGradualForbFlag;           /* ���Կ��ƿ���, 0: ��; 1: �� */
    VOS_UINT8                           ucRsv1;
    VOS_UINT8                           ucRsv2;
    VOS_UINT8                           ucRsv3;
    VOS_UINT16                          usGradualForbTimerFirstLen;  /* ��һ�α�#15��, �ͷ���ʱ��ʱ��, ��λ���� */
    VOS_UINT16                          usGradualForbTimerSecondLen; /* �ڶ��α�#15��, �ͷ���ʱ��ʱ��, ��λ���� */
    VOS_UINT32                          ulGradualForbAgingTimerLen;  /* �ϻ���ʱ��ʱ��, ��λ���� */
    VOS_UINT16                          usRsv1;
    VOS_UINT16                          usRsv2;
    VOS_UINT16                          usRsv3;
    VOS_UINT16                          usRsv4;
    VOS_UINT32                          ulRsv1;
    VOS_UINT32                          ulRsv2;
    VOS_UINT32                          ulRsv3;
    VOS_UINT32                          ulRsv4;
}LNAS_LMM_NV_GRADUAL_FORBIDDEN_PARA_STRU;

/*****************************************************************************
 �ṹ��    : LNAS_SWITCH_PARA_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : LNAS���ƿ��صĽṹ��(��ʹ��BITλ,������д��Ӧ˵��)
*****************************************************************************/
typedef struct
{
    /*bitλ������LNAS���ƿ���*/
    LNAS_FUN_FLAG_NV_BIT_STRU_1                 stNasFunFlag01;
    LNAS_FUN_FLAG_NV_BIT_STRU_2                 stNasFunFlag02;
    LNAS_FUN_FLAG_NV_BIT_STRU_3                 stNasFunFlag03;
    LNAS_FUN_FLAG_NV_BIT_STRU_4                 stNasFunFlag04;
    LNAS_FUN_FLAG_NV_BIT_STRU_5                 stNasFunFlag05;
    LNAS_FUN_FLAG_NV_BIT_STRU_6                 stNasFunFlag06;
    LNAS_FUN_FLAG_NV_BIT_STRU_7                 stNasFunFlag07;
    LNAS_FUN_FLAG_NV_BIT_STRU_8                 stNasFunFlag08;

    LNAS_LMM_NV_GRADUAL_FORBIDDEN_PARA_STRU stGradualForbPara;

    /* ���ƿ�����ʹ������Bitλ, ����Ϊ����λ */
    /*  tau #17 2016-05-20 begin */
    VOS_UINT8                               ucTauRej17MaxTimes;    /* TAU��17�ܾ��Ĵ����������������ת����10����attach��ȡֵ��Χ1-5 */
    /*  tau #17 2016-05-20 end */
    /* temp_forbidden_TA , 2016-06-17 begin */
    VOS_UINT8                               uc3402TempForbiddenTAFlag;
    /* 0: �Ż��ر�  */
    /* 1: HPLMN���� */
    /* 2: VPLMN���� */
    /* 3: �Ż�����  */
    /* temp_forbidden_TA , 2016-06-17 end */
    VOS_UINT8                               ucRsv3;
    VOS_UINT8                               ucRsv4;
    VOS_UINT8                               ucRsv5;
    VOS_UINT8                               ucRsv6;
    VOS_UINT8                               ucRsv7;
    VOS_UINT8                               ucRsv8;
    VOS_UINT8                               ucRsv9;
    VOS_UINT8                               ucRsv10;
    VOS_UINT8                               ucRsv11;
    VOS_UINT8                               ucRsv12;
    VOS_UINT8                               ucRsv13;
    VOS_UINT8                               ucRsv14;
    VOS_UINT8                               ucRsv15;
    VOS_UINT8                               ucRsv16;
    VOS_UINT8                               ucRsv17;
    VOS_UINT8                               ucRsv18;
    VOS_UINT8                               ucRsv19;
    VOS_UINT8                               ucRsv20;
    VOS_UINT8                               ucRsv21;
    VOS_UINT8                               ucRsv22;
    VOS_UINT8                               ucRsv23;
    VOS_UINT8                               ucRsv24;
    VOS_UINT8                               ucRsv25;
    VOS_UINT8                               ucRsv26;
    VOS_UINT8                               ucRsv27;
    VOS_UINT8                               ucRsv28;
    VOS_UINT8                               ucRsv29;
    VOS_UINT8                               ucRsv30;
    VOS_UINT8                               ucRsv31;
    VOS_UINT8                               ucRsv32;
    VOS_UINT8                               ucRsv33;
    VOS_UINT8                               ucRsv34;
    VOS_UINT8                               ucRsv35;
    VOS_UINT8                               ucRsv36;
    VOS_UINT8                               ucRsv37;
    VOS_UINT8                               ucRsv38;
    VOS_UINT8                               ucRsv39;
    VOS_UINT8                               ucRsv40;
    VOS_UINT8                               ucRsv41;
    VOS_UINT8                               ucRsv42;
    VOS_UINT8                               ucRsv43;
    VOS_UINT8                               ucRsv44;
    VOS_UINT8                               ucRsv45;
    VOS_UINT8                               ucRsv46;
    VOS_UINT8                               ucRsv47;
    VOS_UINT8                               ucRsv48;
    VOS_UINT8                               ucRsv49;
    VOS_UINT8                               ucRsv50;
    VOS_UINT8                               ucRsv51;
    VOS_UINT8                               ucRsv52;
    VOS_UINT8                               ucRsv53;
    VOS_UINT8                               ucRsv54;
    VOS_UINT8                               ucRsv55;
    VOS_UINT8                               ucRsv56;
    VOS_UINT8                               ucRsv57;
    VOS_UINT8                               ucRsv58;
    VOS_UINT8                               ucRsv59;
    VOS_UINT8                               ucRsv60;
    VOS_UINT8                               ucRsv61;
    VOS_UINT8                               ucRsv62;
    VOS_UINT8                               ucRsv63;
    VOS_UINT8                               ucRsv64;
    /* temp_forbidden_TA , 2016-06-17 begin */
    VOS_UINT16                              usTempForbTimerLen; /*��λΪ����*/
    /* temp_forbidden_TA , 2016-06-17 end */
    VOS_UINT16                              usRsv2;
    VOS_UINT16                              usRsv3;
    VOS_UINT16                              usRsv4;
    VOS_UINT16                              usRsv5;
    VOS_UINT16                              usRsv6;
    VOS_UINT16                              usRsv7;
    VOS_UINT16                              usRsv8;
    VOS_UINT16                              usRsv9;
    VOS_UINT16                              usRsv10;
    VOS_UINT16                              usRsv11;
    VOS_UINT16                              usRsv12;
    VOS_UINT16                              usRsv13;
    VOS_UINT16                              usRsv14;
    VOS_UINT16                              usRsv15;
    VOS_UINT16                              usRsv16;
    VOS_UINT16                              usRsv17;
    VOS_UINT16                              usRsv18;
    VOS_UINT16                              usRsv19;
    VOS_UINT16                              usRsv20;
    VOS_UINT16                              usRsv21;
    VOS_UINT16                              usRsv22;
    VOS_UINT16                              usRsv23;
    VOS_UINT16                              usRsv24;
    VOS_UINT16                              usRsv25;
    VOS_UINT16                              usRsv26;
    VOS_UINT16                              usRsv27;
    VOS_UINT16                              usRsv28;
    VOS_UINT16                              usRsv29;
    VOS_UINT16                              usRsv30;
    VOS_UINT16                              usRsv31;
    VOS_UINT16                              usRsv32;
    VOS_UINT16                              usRsv33;
    VOS_UINT16                              usRsv34;
    VOS_UINT16                              usRsv35;
    VOS_UINT16                              usRsv36;
    VOS_UINT16                              usRsv37;
    VOS_UINT16                              usRsv38;
    VOS_UINT16                              usRsv39;
    VOS_UINT16                              usRsv40;
    VOS_UINT16                              usRsv41;
    VOS_UINT16                              usRsv42;
    VOS_UINT16                              usRsv43;
    VOS_UINT16                              usRsv44;
    VOS_UINT16                              usRsv45;
    VOS_UINT16                              usRsv46;
    VOS_UINT16                              usRsv47;
    VOS_UINT16                              usRsv48;
    VOS_UINT16                              usRsv49;
    VOS_UINT16                              usRsv50;
    VOS_UINT16                              usRsv51;
    VOS_UINT16                              usRsv52;
    VOS_UINT16                              usRsv53;
    VOS_UINT16                              usRsv54;
    VOS_UINT16                              usRsv55;
    VOS_UINT16                              usRsv56;
    VOS_UINT16                              usRsv57;
    VOS_UINT16                              usRsv58;
    VOS_UINT16                              usRsv59;
    VOS_UINT16                              usRsv60;
    VOS_UINT16                              usRsv61;
    VOS_UINT16                              usRsv62;
    VOS_UINT16                              usRsv63;
    VOS_UINT16                              usRsv64;

    /*Add for attach Vote 2016-09-05 start*/
    VOS_UINT32                              ulCcpuIncreaseFreqValue; /*Ccpu��Ƶ��ֵ,��λKHZ*/
    VOS_UINT32                              ulDdrIncreaseFreqValue;  /*Ddr��Ƶ��ֵ,��λKHZ*/
    /*Add for attach Vote 2016-09-05 end*/

    VOS_UINT32                              ulRsv3;
    VOS_UINT32                              ulRsv4;
    VOS_UINT32                              ulRsv5;
    VOS_UINT32                              ulRsv6;
    VOS_UINT32                              ulRsv7;
    VOS_UINT32                              ulRsv8;
    VOS_UINT32                              ulRsv9;
    VOS_UINT32                              ulRsv10;
    VOS_UINT32                              ulRsv11;
    VOS_UINT32                              ulRsv12;
    VOS_UINT32                              ulRsv13;
    VOS_UINT32                              ulRsv14;
    VOS_UINT32                              ulRsv15;
    VOS_UINT32                              ulRsv16;
    VOS_UINT32                              ulRsv17;
    VOS_UINT32                              ulRsv18;
    VOS_UINT32                              ulRsv19;
    VOS_UINT32                              ulRsv20;
    VOS_UINT32                              ulRsv21;
    VOS_UINT32                              ulRsv22;
    VOS_UINT32                              ulRsv23;
    VOS_UINT32                              ulRsv24;
    VOS_UINT32                              ulRsv25;
    VOS_UINT32                              ulRsv26;
    VOS_UINT32                              ulRsv27;
    VOS_UINT32                              ulRsv28;
    VOS_UINT32                              ulRsv29;
    VOS_UINT32                              ulRsv30;
    VOS_UINT32                              ulRsv31;
    VOS_UINT32                              ulRsv32;
}LNAS_NV_SWITCH_PARA_STRU;
/* 2015-5-27 end */
typedef struct
{
    VOS_UINT32                          bitOpPeriodicRptTimes:1;                /* �����ϱ������������Կ��� */
    VOS_UINT32                          bitOp2:1;
    VOS_UINT32                          bitOp3:1;
    VOS_UINT32                          bitOp4:1;
    VOS_UINT32                          bitOp5:1;
    VOS_UINT32                          bitOp6:1;
    VOS_UINT32                          bitOp7:1;
    VOS_UINT32                          bitOp8:1;
    VOS_UINT32                          bitOp9:1;
    VOS_UINT32                          bitOp10:1;
    VOS_UINT32                          bitOp11:1;
    VOS_UINT32                          bitOp12:1;
    VOS_UINT32                          bitOp13:1;
    VOS_UINT32                          bitOp14:1;
    VOS_UINT32                          bitOp15:1;
    VOS_UINT32                          bitOp16:1;
    VOS_UINT32                          bitOp17:1;
    VOS_UINT32                          bitOp18:1;
    VOS_UINT32                          bitOp19:1;
    VOS_UINT32                          bitOp20:1;
    VOS_UINT32                          bitOp21:1;
    VOS_UINT32                          bitOp22:1;
    VOS_UINT32                          bitOp23:1;
    VOS_UINT32                          bitOp24:1;
    VOS_UINT32                          bitOp25:1;
    VOS_UINT32                          bitOp26:1;
    VOS_UINT32                          bitOp27:1;
    VOS_UINT32                          bitOp28:1;
    VOS_UINT32                          bitOp29:1;
    VOS_UINT32                          bitOp30:1;
    VOS_UINT32                          bitOp31:1;
    VOS_UINT32                          bitOp32:1;
}LNAS_LCS_NV_FEATURE_BIT_STRU;

/*Add for Lcs 2015-10-13 start*/
typedef struct
{
    LNAS_LCS_NV_FEATURE_BIT_STRU        stLcsFeatureBit;

    VOS_UINT8                           ucMaxRetryTimes;                        /* ������³��Դ��� */
    VOS_UINT8                           ucRsv1;
    VOS_UINT8                           ucRsv2;
    VOS_UINT8                           ucRsv3;
    VOS_UINT8                           ucRsv4;
    VOS_UINT8                           ucRsv5;
    VOS_UINT8                           ucRsv6;
    VOS_UINT8                           ucRsv7;
    VOS_UINT8                           ucRsv8;
    VOS_UINT8                           ucRsv9;
    VOS_UINT8                           ucRsv10;
    VOS_UINT8                           ucRsv11;
    VOS_UINT8                           ucRsv12;
    VOS_UINT8                           ucRsv13;
    VOS_UINT8                           ucRsv14;
    VOS_UINT8                           ucRsv15;
    VOS_UINT8                           ucRsv16;
    VOS_UINT8                           ucRsv17;
    VOS_UINT8                           ucRsv18;
    VOS_UINT8                           ucRsv19;
    VOS_UINT8                           ucRsv20;
    VOS_UINT8                           ucRsv21;
    VOS_UINT8                           ucRsv22;
    VOS_UINT8                           ucRsv23;
    VOS_UINT16                          usRetryTimerLen;                        /* ���³��Զ�ʱ��ʱ��, ��ʱ������½��� */
    VOS_UINT16                          usPeriodicRptTimes;                     /* MO-LRΪ�����ϱ�ʱ,���������ϱ����� */
    VOS_UINT16                          usRsv2;
    VOS_UINT16                          usRsv3;
    VOS_UINT16                          usRsv4;
    VOS_UINT16                          usRsv5;
    VOS_UINT16                          usRsv6;
    VOS_UINT16                          usRsv7;
    VOS_UINT16                          usRsv8;
    VOS_UINT16                          usRsv9;
    VOS_UINT16                          usRsv10;
    VOS_UINT16                          usRsv11;
    VOS_UINT16                          usRsv12;
    VOS_UINT16                          usRsv13;
    VOS_UINT16                          usRsv14;
    VOS_UINT16                          usRsv15;
    VOS_UINT32                          ulRsv0;
    VOS_UINT32                          ulRsv1;
    VOS_UINT32                          ulRsv2;
    VOS_UINT32                          ulRsv3;
    VOS_UINT32                          ulRsv4;
    VOS_UINT32                          ulRsv5;
    VOS_UINT32                          ulRsv6;
    VOS_UINT32                          ulRsv7;
    VOS_UINT32                          ulRsv8;
    VOS_UINT32                          ulRsv9;
    VOS_UINT32                          ulRsv10;
    VOS_UINT32                          ulRsv11;
    VOS_UINT32                          ulRsv12;
    VOS_UINT32                          ulRsv13;
    VOS_UINT32                          ulRsv14;
    VOS_UINT32                          ulRsv15;
}LNAS_LCS_NV_COMMON_CONFIG_STRU;
/*Add for Lcs 2015-10-13 end*/

/* Added for Boston_R13_CR_PHASEI 2016-10-17 begin */
/*****************************************************************************
�ṹ����    :LNAS_LMM_NV_T3402_INFO_STRU
ʹ��˵��    :LNAS����T3402��ʱ����NV���ݽṹ
*****************************************************************************/
typedef struct
{
    VOS_UINT8                           ucIsT3402DefaultValue;                  /* 3402��ʱ���Ƿ�ΪĬ��ʱ����0��1�� */
    VOS_UINT8                           ucRsv[3];                               /* ���� */
    VOS_UINT32                          ul3402Len;                              /* 3402��ʱ��ʱ������λ:�� */
    NAS_MM_PLMN_LIST_STRU               stT3402EPlmnList;                       /* 3402��ʱ��ʱ����Ч��EPLMNLIST */
}LNAS_LMM_NV_T3402_INFO_STRU;

/* Added for Boston_R13_CR_PHASEI 2016-10-17 end */

/* Added by for DATA RETRY PHASEI, 2016-03-21, Begin */
/*****************************************************************************
�ṹ����    :LNAS_LMM_NV_T3402_CTRL_STRU
ʹ��˵��    :LNAS����DATA RETRY����T3402��ʱ�����PLMN�б�ά����NV���ݽṹ
*****************************************************************************/
typedef struct
{
    VOS_UINT8                           ucT3402PlmnCtrlSwitch;                  /* T3402���PLMN��Ч���أ�0�رգ�1�� */
    VOS_UINT8                           ucT3402RmLenStore;                      /* T3402ʣ��ʱ���Ƿ���Ҫ���棬0��1�� */
    VOS_UINT8                           ucRsv1;                                 /* ���� */
    VOS_UINT8                           ucT3402PlmnNum;                         /* astT3402PlmnList����Ч��Ԫ���� */
    NAS_PLMN_T3402_STRU                 astT3402PlmnList[NAS_MM_MAX_T3402_PLMN_NUM];
    VOS_UINT32                          ulT3402DefaultValue;                    /* 3402��ʱ��Ĭ��ʱ������λ:�� */

}LNAS_LMM_NV_T3402_CTRL_STRU;
/* Added by for DATA RETRY PHASEI, 2016-03-21, End */

/*Added for DATA RETRY PHASEII 2016-05-23 start*/
typedef struct
{
    VOS_UINT32                                  ulRemainLogTime;    /*��ǰ�¼��ػ�ʱ��ʣ���ʱ��,��λΪ��*/
    VOS_UINT16                                  usEventCnt;         /*��ǰԭ��ֵ�¼��ļ���*/
    VOS_UINT8                                   ucRsv1;
    VOS_UINT8                                   ucRsv2;
    NAS_EMM_PLMN_ID_STRU                        stPlmnId;           /*��ǰPLMN ID*/
    NAS_EMM_TAC_STRU                            stTac;              /*��ǰTac*/
}LNAS_EMM_DATA_RETRY_FAIL_EVENT_STRU;

typedef struct
{
    VOS_UINT8                                   ucEmmcause;         /*ԭ��ֵID*/
    VOS_UINT8                                   ucRsv1;
    VOS_UINT8                                   ucRsv2;
    VOS_UINT8                                   ucRsv3;
    VOS_UINT32                                  ulEmmFailEventCnt;  /*��ʾ��ǰԭ��ֵ�б��д洢����Ч����*/
    LNAS_EMM_DATA_RETRY_FAIL_EVENT_STRU         astEmmFailEvent[LNAS_NV_DATA_RETRY_EMM_FAIL_CAUSE_EVENT_CNT];  /*����ԭ��ֵ�¼��б��list*/
}LNAS_EMM_DATA_RETRY_FAIL_EVENT_LIST_STRU;

typedef struct
{
    VOS_UINT32                                  aulMaxLogTime[LNAS_NV_DATA_RETRY_EMM_FAIL_CAUSE_NUM];       /*��ʾ�¼��洢�����ʱ��*/
    VOS_UINT8                                   aucMaxEventCounter[LNAS_NV_DATA_RETRY_EMM_FAIL_CAUSE_NUM];  /*��ʾ�¼���ulMaxLogTimeʱ���ڳ��Ե�������*/
    VOS_UINT8                                   ucRsv1;
    VOS_UINT8                                   ucRsv2;
	LNAS_EMM_DATA_RETRY_FAIL_EVENT_LIST_STRU    astEventCause[LNAS_NV_DATA_RETRY_EMM_FAIL_CAUSE_NUM];
}LNAS_EMM_DATA_RETRY_NV_PARA_CONFIG_STRU;


typedef struct
{
    VOS_UINT8                           ucApnLen;
    VOS_UINT8                           aucApnName[LNAS_NV_MAX_APN_LEN];
}LNAS_APN_INFO_STRU;

/* Added for DATA RETRY PHASEIII, 2016-6-23, begin */
/*****************************************************************************
�ṹ����    :LNAS_APN_PRIO_INFO_STRU
ʹ��˵��    :
*****************************************************************************/
typedef struct
{
    LNAS_APN_INFO_STRU                  stApnInfo;                              /* APN ��Ϣ */
    NAS_ESM_BEARER_PRIO_ENUM_UINT32     ulBearPrio;                             /* �������ȼ�*/
}LNAS_APN_PRIO_INFO_STRU;
/* Added for DATA RETRY PHASEIII, 2016-6-23, end */

typedef struct
{
    LNAS_APN_INFO_STRU                  stApnInfo;                          /* APN                */
    VOS_UINT32                          ulRemainTimerLen;                   /*��ǰAPN�ػ�ʱ����ڶ�ʱ����ʣ���ʱ��*/
}LNAS_ESM_APN_THROT_REMAIN_TIME_INFO_STRU;

typedef struct
{
    VOS_UINT32                                  ulWaitTime;            /* ��PDNȥ����֮����waittime֮����ܷ���PDN��������,��λ:s*/
    VOS_UINT32                                  ulPdnMaxConnTime;      /* �������ʱ�䣬��ʶÿһ��PDN��һ�η����������������ʱ�䣬��λ:s*/
    VOS_UINT32                                  ulPdnMaxConnCount;     /* ������Ӵ��������������ʱ�䣬������PDN��������Ӵ���*/
    VOS_UINT8                                   ucApnTotalNum;         /* ����ʹ�õ�APN���������� */
    VOS_UINT8                                   aucRsv[3];              /* ���� */

    /* PDN���ڶ�ʱ�������ڼ�ػ���ά��ʣ��ʱ���б������洢*/
    LNAS_ESM_APN_THROT_REMAIN_TIME_INFO_STRU    astApnThrotRemainTimeInfo[LNAS_NV_MAX_APN_CONTEXT_NUM];
}LNAS_ESM_DATA_RETRY_NV_PARA_CONFIG_STRU;

/*Added for DATA RETRY PHASEII 2016-05-23 end*/

/* Added for DSDS OPTIMIZE MT DETACH BY TAU, 2016-06-16, Begin */
/*****************************************************************************
�ṹ����    :LNAS_LMM_NV_MT_DETACH_TAU_CTRL_STRU
ʹ��˵��    :LNAS���ڲ�Ʒ�߶��Ƶ��������յ������DETACH����DSDS�����޷��յ�
             ����DETACH,����UE��ʱ�䱻�в�ͨ����Ķ���NV���ݽṹ.

             (1)��ucPlmnNum = 1,���б��и�PLMN��ΪȫF,��������������PLMN;
             (2)�������,����ֻ������פ���ڴ��б��е�PLMN��
*****************************************************************************/
typedef struct
{
    VOS_UINT8                           ucActiveFlag;                           /* ���Կ���, 1:���Կ�����0:�����Թر� */
    VOS_UINT8                           ucRsv0;                                 /* ���������Ժ������������� */
    VOS_UINT8                           ucRsv1;                                 /* ���������Ժ������������� */
    VOS_UINT8                           ucPlmnNum;                              /* �������õ�PLMN����  */
    NAS_MM_PLMN_ID_STRU                 astPlmnId[NAS_MT_DETACH_TAU_PLMN_MAX_NUM];/* �������õľ���PLMN�б� */
}LNAS_LMM_NV_MT_DETACH_TAU_CTRL_STRU;
/* Added for DSDS OPTIMIZE MT DETACH BY TAU, 2016-06-16, End */

/* Added for network not include eps_network_feature_support IE  ,2017-08-25,begin */
/*****************************************************************************
�ṹ����    :LNAS_LMM_NV_NETWORK_FEATURE_VOPS_OPTIMIZE_CTRL_STRU
ʹ��˵��    :��NV���ڽ��ɳ����Ӫ��zain����δ��ATTACH_ACP��Я��eps_network_feature_support
             ��ѡ��Ԫʱ,VOLTE�޷�����IMSע�������.
             ����δЯ��eps network feature support��Ԫʱ,��������Э��Ĵ����ǰ���Ԫ���
             ���Ϊ��֧��;��NV���ش�ʱ,Ϊ�˽��IMS�޷�ע������,��Ҫ�Ѵ���Ԫ��VOPS������
             ��Ϊ֧��.
             ��NV���ն˶����Ƿ�򿪡�
*****************************************************************************/
typedef struct
{
    VOS_UINT8                           ucNetVopsOptimizeCtrl;                  /* ���Կ���, 1:���Կ�����0:���Թر�. Ĭ�Ϲر� */
    VOS_UINT8                           ucRsv0;                                 /* ���������Ժ������������� */
    VOS_UINT8                           ucRsv1;                                 /* ���������Ժ������������� */
    VOS_UINT8                           ucRsv2;                                 /* ���������Ժ������������� */
}LNAS_LMM_NV_NETWORK_FEATURE_VOPS_OPTIMIZE_CTRL_STRU;
/* Added for network not include eps_network_feature_support IE  ,2017-08-25,end */

/* Added for DATA RETRY PHASEIII, 2016-6-23, begin */
typedef struct
{
    LNAS_APN_PRIO_INFO_STRU             stApnAndPrioInfo;                       /* APN and Prio INFO */
    VOS_UINT32                          ulT3396RemainLenForNonCustom;           /* ��λΪ�� */
    VOS_UINT8                           ucPlmnNum;                              /* PLMN���� */
    VOS_UINT8                           aucRsv[3];
    NAS_PLMN_T3396_STRU                 astT3396PlmnList[NAS_MM_MAX_T3396_PLMN_NUM];
}LNAS_ESM_T3396_APN_PLMN_INFO_STRU;
/*****************************************************************************
�ṹ����    :LNAS_ESM_NV_T3396_CTRL_CONFIG_STRU
ʹ��˵��    :LNAS����DATA RETRY����T3396��ʱ�����PLMN/APN�б�ά����NV���ݽṹ
*****************************************************************************/

typedef struct
{
    VOS_UINT8                           ucT3396CtrlSwitch;                      /* T3396���Կ��أ�0�رգ�1�� */
    VOS_UINT8                           ucIsPlmnCustom;                         /* PLMN���Ʊ�ʶ��0�Ƕ��ƣ�1���� */
    VOS_UINT8                           ucApnTotalNum;                          /* ����ʹ�õ�APN���������� */
    VOS_UINT8                           ucRsv;                                  /* ���� */

    LNAS_ESM_T3396_APN_PLMN_INFO_STRU    astT3396ApnPlmnInfo[LNAS_NV_MAX_APN_CONTEXT_NUM];

}LNAS_ESM_NV_T3396_CTRL_CONFIG_STRU;
/* Added for DATA RETRY PHASEIII, 2016-6-23, end */

/*Added for DATA RETRY PHASEIII 2016-06-21 start*/
/*****************************************************************************
�ṹ����    :LNAS_LMM_NV_EAB_CONFIG_STRU
ʹ��˵��    :LNAS����EAB���Ե�NV���ݽṹ
*****************************************************************************/
typedef struct
{
    VOS_UINT8                           ucUeCapacityLowPri;             /* UE�������Ƿ�֧����������ȼ��� 0��֧�֣�1֧�� */
    VOS_UINT8                           ucUeCapacityEab;                /* UE�������Ƿ�֧��EAB�� 0��֧�֣�1֧�� */
    VOS_UINT8                           ucEabFlag;                      /* EAB���Կ��أ�0��֧�֣�1֧�� */
    VOS_UINT8                           ucSigLowPriFlag;                /* NAS Signalling Priority ��־λ��0�ǵ����ȼ���1�����ȼ� */
    VOS_UINT8                           ucOverrideSigLowPriFlag;        /* NAS Signalling Priority ��д��־��0������д��1����д */
    VOS_UINT8                           ucOverrideEabFlag;              /* EAB������д��־��0������д��1����д */
    VOS_UINT8                           ucRsv1;
    VOS_UINT8                           ucRsv2;
}LNAS_LMM_NV_EAB_CONFIG_STRU;

/*Added for DATA RETRY PHASEIII 2016-06-21 end*/
/*****************************************************************************
�ṹ����    :LNAS_LMM_NV_COMPATIBLE_OPTIMIZE_FOR_PROTCL_AND_NET_STRU
ʹ��˵��    :��������������Ż����ƿ���
*****************************************************************************/
typedef struct
{
    VOS_UINT8                           ucSelectPlmnWhenAuthInTauCtrl;/*����������TAU�����з�����Ȩ����Ȩ����PLMN��ѡ�񿪹أ�1-�򿪣�0-�رգ� Ĭ�ϴ�*/
    VOS_UINT8                           ucRsv2;
    VOS_UINT8                           ucRsv3;
    VOS_UINT8                           ucRsv4;
    VOS_UINT8                           ucRsv5;
    VOS_UINT8                           ucRsv6;
    VOS_UINT8                           ucRsv7;
    VOS_UINT8                           ucRsv8;
    VOS_UINT8                           ucRsv9;
    VOS_UINT8                           ucRsv10;
    VOS_UINT8                           ucRsv11;
    VOS_UINT8                           ucRsv12;
    VOS_UINT8                           ucRsv13;
    VOS_UINT8                           ucRsv14;
    VOS_UINT8                           ucRsv15;
    VOS_UINT8                           ucRsv16;
    VOS_UINT8                           ucRsv17;
    VOS_UINT8                           ucRsv18;
    VOS_UINT8                           ucRsv19;
    VOS_UINT8                           ucRsv20;
}LNAS_LMM_NV_COMPATIBLE_OPTIMIZE_FOR_PROTCL_AND_NET_STRU;

/* Added  for DATA RETRY PHASEIV, 2016-07-25, begin */
/*****************************************************************************
�ṹ����    :LNAS_EMM_NV_T3346_CTRL_CONFIG_STRU
ʹ��˵��    :LNAS T3346��ʱ����NV���ݽṹ
*****************************************************************************/

typedef struct
{
    VOS_UINT8                           ucT3346CtrlSwitch;                      /* T3346���Կ��أ�0�رգ�1�� */
    VOS_UINT8                           ucNasSigPrio;                           /* NAS Signalling Priority ��0�ǵ����ȼ���1�����ȼ� */
    VOS_UINT8                           ucRsv1;                                 /* ���� */
    VOS_UINT8                           ucRsv2;                                 /* ���� */

    VOS_UINT32                          ulT3346RemainLen;                       /* ��λΪ�� */
    NAS_MM_PLMN_ID_STRU                 stPlmnId;                               /* PLMN */
    NAS_MM_PLMN_LIST_STRU               stEPlmnList;                            /* equivalent PLMN list */

}LNAS_EMM_NV_T3346_CTRL_CONFIG_STRU;
/* Added  for DATA RETRY PHASEIV, 2016-07-25, end */

/* Added for DATA RETRY PHASEIV, 2016-7-25, begin */
/*****************************************************************************
�ṹ����    :LNAS_ESM_NV_APN_SWITCH_CTRL_CONFIG_STRU
ʹ��˵��    :
*****************************************************************************/

typedef struct
{
    VOS_UINT8                           ucApnSwitchForVZW;          /*APN SWTICH ���ܿ���: 1 ������0 �ر�*/
    VOS_UINT8                           ucApnSwitchForNonVZW;      /*APN SWTICH ���ܿ���: 1 ������0 �ر�*/
    VOS_UINT8                           ucRsv1;
    VOS_UINT8                           ucRsv2;

}LNAS_ESM_NV_APN_SWITCH_CTRL_CONFIG_STRU;
/* Added for DATA RETRY PHASEIV, 2016-7-25, end */
/* Added  for Boston_R13_CR_PHASEIII, 2017-01-16, Begin */
/*****************************************************************************
�ṹ����    :LNAS_ESM_NV_BACKOFF_CTRL_CONFIG_STRU
ʹ��˵��    :LNAS����back-off��NV���ݽṹ
*****************************************************************************/

typedef struct
{
    VOS_UINT8                           ucBackOffCtrlSwitch;                    /* back-off���Կ��أ�0�رգ�1�� */
    VOS_UINT8                           aucRsv[3];                              /* ���� */

}LNAS_ESM_NV_BACKOFF_CTRL_CONFIG_STRU;
/* Added  for Boston_R13_CR_PHASEIII, 2017-01-16, End */

/* Added for ,2016-10-28,Begin */
/*****************************************************************************
�ṹ����    :LNAS_EMM_NV_HO_TAU_DELAY_CTRL_CONFIG_STRU
ʹ��˵��    :
*****************************************************************************/

typedef struct
{
    VOS_UINT8                           ucSwitchFlag;              /*SWTICH ���ܿ���: 1 ������0 �ر�,Ĭ�Ͽ���*/
    VOS_UINT8                           aucRsv[3];

    VOS_UINT32                          ulHoTauDelayTimeLen;      /*HO TAU DELAY ��ʱ��ʱ�� ms */
}LNAS_EMM_NV_HO_TAU_DELAY_CTRL_CONFIG_STRU;

/*****************************************************************************
�ṹ����    :LNAS_ESM_NV_NDIS_CONN_DELAY_CTRL_CONFIG_STRU
ʹ��˵��    :
*****************************************************************************/

typedef struct
{
    VOS_UINT8                           ucSwitchFlag;              /* SWTICH ���ܿ���: 1 ������0 �ر�,Ĭ�Ͽ���*/
    VOS_UINT8                           aucRsv[3];

    VOS_UINT32                          ulNdisConnDelayTimeLen;      /*NDIS CONN DELAY ��ʱ��ʱ�� ms */
}LNAS_ESM_NV_NDIS_CONN_DELAY_CTRL_CONFIG_STRU;

/* Added for ,2016-10-28,End */

/* add for separate special part from original NV  , 2016-11-07, begin */
/*****************************************************************************
 �ṹ��    : LNAS_AUSTRALIA_FLAG_CONFIG_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : �ĵ綨�ƵĿ��ƿ��ؽṹ��(��ʹ��BITλ,������д��Ӧ˵��)
*****************************************************************************/
typedef struct
{
    VOS_UINT32      ulAustraliaFlag;
    VOS_UINT32      ulRsv;
}LNAS_LMM_NV_AUSTRALIA_FLAG_CONFIG_STRU;
/* add for separate special part from original NV  , 2016-11-07, end */
/* Added for MTU_REQUIRE,2016-11-12,Begin */
/*****************************************************************************
�ṹ����    :LNAS_ESM_NV_NDIS_CONN_DELAY_CTRL_CONFIG_STRU
ʹ��˵��    :
*****************************************************************************/

typedef struct
{
    VOS_UINT8                           ucIpv4MtuForImsApnFlag;             /* ���IMS APN�Ƿ�����ipv4 mtu: 1 ����0 ������,Ĭ������*/
    VOS_UINT8                           ucIpv4MtuForOtherApnFlag;           /* ������� APN�Ƿ�����ipv4 mtu: 1 ����0 ������,Ĭ�ϲ�����*/
    VOS_UINT8                           aucRsv[2];
}LNAS_ESM_NV_IPV4_MTU_CTRL_CONFIG_STRU;
/* Added for MTU_REQUIRE,2016-11-12,End */
/* Added for BOSTON_R13_CR_PHASEII,2016-12-05,Begin */
/*****************************************************************************
�ṹ����    :LNAS_ESM_NV_NDIS_CONN_DELAY_CTRL_CONFIG_STRU
ʹ��˵��    :
*****************************************************************************/

typedef struct
{
    VOS_UINT8                           ucSwitchFlag;                       /* �����Ƿ���: 1 ������0������*/
    VOS_UINT8                           aucRsv[3];
    VOS_UINT8                           ucUsimInvalidForGprsMaxValue;       /* GRPSҵ����Ч�������ֵ */
    VOS_UINT8                           ucUsimInvalidForNonGprsMaxValue;    /* ��GRPSҵ����Ч�������ֵ */
    VOS_UINT8                           ucPlmnSpecificAttemptMaxValue;      /* �ض�PLMN���Լ������ֵ */
    VOS_UINT8                           ucPlmnSpecificPsAttemptMaxValue;    /* �ض�PLMN PS���Լ������ֵ */
}LNAS_EMM_NV_PLAIN_NAS_REJ_MSG_CTRL_CONFIG_STRU;
/* Added for BOSTON_R13_CR_PHASEII,2016-12-05,End */
/* Added for MO_DETACH_REL,2017-01-05,Begin */
/*****************************************************************************
�ṹ����    :LNAS_ESM_NV_NDIS_CONN_DELAY_CTRL_CONFIG_STRU
ʹ��˵��    :
*****************************************************************************/

typedef struct
{
    VOS_UINT8                           ucSwitchFlag;                       /* �����Ƿ���: 1 ������0������*/
    VOS_UINT8                           ucMaxDetachAttemptCnt;              /* detach����Դ��� */
    VOS_UINT8                           aucRsv[2];

}LNAS_EMM_NV_DETACH_ATTEMPT_CNT_CTRL_CONFIG_STRU;
/* Added for MO_DETACH_REL,2017-01-05,End */



/* Added by 2016-12-14 for KDDI,begin */
/*****************************************************************************
�ṹ����    :LNAS_ESM_NV_CHANGE_TO_IMSAPN_CONFIG_STRU
ʹ��˵��    :
*****************************************************************************/

typedef struct
{
    VOS_UINT8                           ucChangeToImsApnFlag;             /* ATTACH���ܺ��Ƿ�Ҫ�滻ΪIMS APN��VOS_TRUE:��Ҫ��VOS_fALSE_����Ҫ*/
    VOS_UINT8                           ucRsv1;
    VOS_UINT8                           ucRsv2;
    VOS_UINT8                           ucRsv3;
}LNAS_ESM_NV_CHANGE_TO_IMSAPN_CONFIG_STRU;
/* Added by 2016-12-14 for KDDI,end */
/* Added for MODETACH_ATTACH_COLLISION,2017-01-05,Begin */
/*****************************************************************************
�ṹ����    :LNAS_EMM_NV_STORE_MMC_DETACH_CTRL_CONFIG_STRU
ʹ��˵��    :
*****************************************************************************/

typedef struct
{
    VOS_UINT8                           ucSwitchFlag;                       /* �����Ƿ���: 1 ������0������*/
    VOS_UINT8                           aucRsv[3];

}LNAS_EMM_NV_STORE_MMC_DETACH_CTRL_CONFIG_STRU;


/* Added for MODETACH_ATTACH_COLLISION,2017-01-05,End */

/*Added for BOSTON_R13_CR_PHASEIII 2017-01-16 begin */
/*****************************************************************************
�ṹ����    :LNAS_LMM_NV_ACDC_APP_STRU
ʹ��˵��    :LNAS����ACDC����APP-ACDCcategory�����ݽṹ
*****************************************************************************/
typedef struct
{
    VOS_UINT8                                               aucOsId[LNAS_NV_ACDC_OSID_LEN];        /* OSID */
    VOS_UINT8                                               aucAppId[LNAS_NV_ACDC_MAX_APPID_LEN];      /* APPID */
    VOS_UINT8                                               ucAcdcCategory;     /* APPID */
    VOS_UINT8                                               ucRsv[3];           /* ���� */
}LNAS_LMM_NV_ACDC_APP_STRU;

/*Added for MT-DETACH issue 2017-04-13 start*/
/*****************************************************************************
�ṹ����    :NAS_EMM_MT_DETACH_OPTIMIZE_STRU
ʹ��˵��    :MT-DETACH�Ż��Ľṹ
*****************************************************************************/
typedef struct
{
    /*
    ��ʾMT-detach re-attach-not-required Я��ԭ��ֵ��Э����ȷ�涨�����µ�ԭ��ֵ:
    2��3��6��7��8��11��12��13��14��15��25 ��Щԭ��ֵЭ������ȷ�涨�˴���ԭ�򣬳���
    ֮�������ԭ��ֵ��ͳһ����otherԭ��ֵ�����ˣ������Ż����otherԭ��ֵ��
    */
    VOS_UINT8                           aucMtDetachOptForGivenCauseList[LNAS_NV_MT_DETACH_OPT_OTHER_CAUSE_CNT]; /*MT-DETACH�Ż���ԭ��ֵ�б�*/
    VOS_UINT16                          usRsv1;
    VOS_UINT16                          usRsv2;
    VOS_UINT16                          usRsv3;
    VOS_UINT16                          usRsv4;
    VOS_UINT32                          ulRsv1;
    VOS_UINT32                          u1Rsv2;
    VOS_UINT32                          ulRsv3;
}LNAS_LMM_NV_MT_DETACH_WITH_OPTIMZIE_CONFIG_STRU;
/*Added for MT-DETACH issue 2017-04-13 end*/

/*****************************************************************************
�ṹ����    :LNAS_ESM_NV_COMPATIBILITY_CONFIG_STRU
ʹ��˵��    :
*****************************************************************************/
typedef struct
{
    VOS_UINT8                           ucApnTotalLengthCheckCompaSwitch;     /* 0�رգ�1�� */
    VOS_UINT8                           ucDelPfLengthCheckCompaSwitch;
    VOS_UINT8                           ucRsv1;
    VOS_UINT8                           ucRsv2;
    VOS_UINT8                           ucRsv3;
    VOS_UINT8                           ucRsv4;
    VOS_UINT8                           ucRsv5;
    VOS_UINT8                           ucRsv6;
    VOS_UINT8                           ucRsv7;
    VOS_UINT8                           ucRsv8;
    VOS_UINT8                           ucRsv9;
    VOS_UINT8                           ucRsv10;
    VOS_UINT8                           ucRsv11;
    VOS_UINT8                           ucRsv12;
    VOS_UINT8                           ucRsv13;
    VOS_UINT8                           ucRsv14;
}LNAS_ESM_NV_COMPATIBILITY_CONFIG_STRU;


/*****************************************************************************
�ṹ����    :LNAS_LMM_NV_ACDC_APP_LIST_STRU
ʹ��˵��    :LNAS����ACDC����APP�б�����ݽṹ
*****************************************************************************/
typedef struct
{
    VOS_UINT32                          ulAppNum;                               /* APP�б���� */
    LNAS_LMM_NV_ACDC_APP_STRU           astAcdcAppList[LNAS_NV_ACDC_APP_MAX_NUM]; /* APP�б� */
}LNAS_EMM_NV_ACDC_CONFIG_STRU;
/*Added for BOSTON_R13_CR_PHASEIII 2017-01-16 end */

/* add for separate special part from original NV  , 2017-03-07, begin */
/*****************************************************************************
�ṹ��    : LNAS_LMM_NV_DSDS_OPTIMIZE_FLAG_CONFIG_STRU
Э����  :
ASN.1���� :
�ṹ˵��  : DSDS2.0�Ż��Ŀ��ƿ��ؽṹ��
*****************************************************************************/
typedef struct
{
    VOS_UINT8                           ucDsdsOptimizeFlag;                 /* �����Ƿ���: 1:������0:�ر� */
    VOS_UINT8                           ucRsv1;
    VOS_UINT8                           ucRsv2;
    VOS_UINT8                           ucRsv3;
}LNAS_LMM_NV_DSDS_OPTIMIZE_FLAG_CONFIG_STRU;

/*****************************************************************************
�ṹ��    : LNAS_LMM_NV_SRLTE_FLAG_CONFIG_STRU
Э����  :
ASN.1���� :
�ṹ˵��  : SRLTE�Ŀ��ƿ��ؽṹ��
*****************************************************************************/
typedef struct
{
    VOS_UINT8                           ucSrlteFlag;                       /* �����Ƿ���: 1:������0:�ر� */
    VOS_UINT8                           ucRsv1;
    VOS_UINT8                           ucRsv2;
    VOS_UINT8                           ucRsv3;
}LNAS_LMM_NV_SRLTE_FLAG_CONFIG_STRU;
/* add for separate special part from original NV  , 2017-03-07, end */

/*****************************************************************************
�ṹ����    :LNAS_ESM_NV_PROTOCOL_CONFIG_STRU
ʹ��˵��    :Э����ؿ��ƿ���
*****************************************************************************/
typedef struct
{
    VOS_UINT8                           ucRsv1;
    VOS_UINT8                           ucRsv2;
    VOS_UINT8                           ucRsv3;
    VOS_UINT8                           ucAllowUplinkPFmodifyToZeroFlag;/* ��������޸ĵ�������PFΪ0���Ƿ���Э��ظ�#44�ܾ���0:����Э��ظ�#44�ܾ���1:����#44�ܾ���Ĭ��Ϊ0*/
    VOS_UINT8                           ucIgnorePfPrecedenceCollisionFlag;/* ����PF���ȼ���ͻ���أ�0:�ر� 1:�򿪣�Ĭ��Ϊ0 */
    VOS_UINT8                           ucNotAllowNullApnAttachFlag;/* ����APN����ʱ���Ƿ��ֹʹ�ÿ�APN����ע�� 0:    �ر�1:    ��,Ĭ��Ϊ0     */
    VOS_UINT8                           ucRsv7;
    VOS_UINT8                           ucRsv8;
    VOS_UINT8                           ucRsv9;
    VOS_UINT8                           ucRsv10;
    VOS_UINT8                           ucRsv11;
    VOS_UINT8                           ucRsv12;
    VOS_UINT8                           ucRsv13;
    VOS_UINT8                           ucRsv14;
    VOS_UINT8                           ucRsv15;
    VOS_UINT8                           ucRsv16;
}LNAS_ESM_NV_PROTOCOL_CONFIG_STRU;

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

#endif /* end of MmcEmmInterface.h */




