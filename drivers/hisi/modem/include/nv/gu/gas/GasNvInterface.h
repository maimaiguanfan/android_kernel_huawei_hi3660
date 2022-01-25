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

#ifndef __GASNVINTERFACE_H__
#define __GASNVINTERFACE_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 Include Headfile
*****************************************************************************/
#if (VOS_OS_VER != VOS_WIN32)
#pragma pack(2)
#else
#pragma pack(push, 2)
#endif

#include "vos.h"

/*****************************************************************************
  2 Macro
*****************************************************************************/
#define NVIM_ULTRA_CLASSMARK_LEN                            (24)
#define NVIM_BAND_PWR_LEN                                   (8)
#define NVIM_CLASSMARK1_LEN                                 (2)
#define NVIM_CLASSMARK2_LEN                                 (4)
#define NVIM_CLASSMARK3_LEN                                 (16)
#define NVIM_CLASSMARK3_R8_LEN                              (36)
#define NVIM_GCF_ITEM_LEN                                   (80)
#define NVIM_GSM_BA_MAX_SIZE                                (33)
#define NVIM_EGPRS_RA_CAPABILITY_DATA_LEN                   (53)
#define NVIM_PREFER_GSM_PLMN_LIST_LEN                       (976)
#define NVIM_GSM_DEC_FAIL_ARFCN_LIST_LEN                    (120)
#define NVIM_EUTRA_MAX_SUPPORT_BANDS_NUM                    (64)
#define NVIM_TDS_MAX_SUPPORT_BANDS_NUM                      (8)
#define NVIM_EUTRA_CAPA_COMM_INFO_SIZE                      (260)
#define NVIM_CBS_MID_LIST_LEN                               (2004)
#define NVIM_CBS_MID_RANGE_LIST_LEN                         (2004)
/* Added by yangsicong for L2G REDIR C1 CUSTUME, 2015-1-26, begin */
#define NVIM_GAS_C1_CALC_OPT_PLMN_WHITE_LIST_CNT_MAX        (20)
/* Added by yangsicong for L2G REDIR C1 CUSTUME, 2015-1-26, end */

#define NVIM_GAS_GSM_BAND_CUSTOMIZE_LIST_MAX_CNT            (80)

#define NVIM_GSM_OPERATE_CUSTOMIZE_FREQ_PLMN_MAX_CNT        (10)
#define NVIM_GSM_OPERATE_CUSTOMIZE_FREQ_RANGE_MAX_CNT       (8)
#define NVIM_GSM_OPERATE_CUSTOMIZE_DESCRETE_FREQ_MAX_CNT    (16)

#if defined( __PS_WIN32_RECUR__ ) || defined (DMT)
#define MAX_CHR_ALARM_ID_NUM (20)
#endif

/*****************************************************************************
  3 Massage Declare
*****************************************************************************/


/*****************************************************************************
  4 Enum
*****************************************************************************/
enum NVIM_TDS_FREQ_BAND_LIST_ENUM
{
    ID_NVIM_TDS_FREQ_BAND_A         = 0x01,         /* Ƶ�㷶Χ: 9504~9596  10054~10121 */
    ID_NVIM_TDS_FREQ_BAND_B         = 0x02,         /* Ƶ�㷶Χ: 9254~9546  9654~9946 */
    ID_NVIM_TDS_FREQ_BAND_C         = 0x04,         /* Ƶ�㷶Χ: 9554~9646 */
    ID_NVIM_TDS_FREQ_BAND_D         = 0x08,         /* Ƶ�㷶Χ: 12854~13096 */
    ID_NVIM_TDS_FREQ_BAND_E         = 0x10,         /* Ƶ�㷶Χ: 11504~11996 */
    ID_NVIM_TDS_FREQ_BAND_F         = 0x20,         /* Ƶ�㷶Χ: 9404~9596 */
    ID_NVIM_TDS_FREQ_BAND_BUTT
};
typedef VOS_UINT8  NVIM_TDS_FREQ_BAND_LIST_ENUM_UINT8;


enum NVIM_BAND_IND_ENUM
{
    NVIM_BAND_IND_2                 = 0x2,          /* BAND2 */
    NVIM_BAND_IND_3                 = 0x3,          /* BAND3 */
    NVIM_BAND_IND_5                 = 0x5,          /* BAND5 */
    NVIM_BAND_IND_8                 = 0x8,          /* BAND8 */
    NVIM_BAND_IND_BUTT
};
typedef VOS_UINT8 NVIM_BAND_IND_ENUM_UINT8;


/*****************************************************************************
  5 STRUCT
*****************************************************************************/
/*****************************************************************************
*                                                                            *
*                           ����������Ϣ�ṹ                                 *
*                                                                            *
******************************************************************************/

/*****************************************************************************
 �ṹ��    : NVIM_ULTRA_CLASSMARK_STRU
 �ṹ˵��  : en_NV_Item_Ultra_Classmark �ṹ
 DESCRIPTION: ������
*****************************************************************************/
typedef struct
{
    VOS_UINT8                           aucUltraClassmark[NVIM_ULTRA_CLASSMARK_LEN];
}NVIM_ULTRA_CLASSMARK_STRU;

/*****************************************************************************
 �ṹ��    : NVIM_GAS_MULTIRATE_FLAG_STRU
 �ṹ˵��  : en_NV_Item_Gas_MultiRateFlag �ṹ
 DESCRIPTION: ������
*****************************************************************************/
typedef struct
{
    VOS_UINT16                          usMultiRateFlag;
    VOS_UINT8                           aucReserve[2];
}NVIM_GAS_MULTIRATE_FLAG_STRU;

/*****************************************************************************
 �ṹ��    : NVIM_BAND_PWR_STRU
 �ṹ˵��  : en_NV_Item_Band_Pwr �ṹ
 DESCRIPTION: MS���������ʵȼ�
*****************************************************************************/
typedef struct
{
    VOS_UINT8                           aucBandPwr[NVIM_BAND_PWR_LEN];          /* �洢MS���������ʵȼ� */
}NVIM_BAND_PWR_STRU;

/*****************************************************************************
 �ṹ��    : NVIM_VGCS_FLAG_STRU
 �ṹ˵��  : en_NV_Item_Vgcs_Flag �ṹ
 DESCRIPTION: MS�Ƿ�֧��������Ѱ��ҵ��
*****************************************************************************/
typedef struct
{
    VOS_UINT16                          usVgcsFlag;                             /* 0x0: ��֧��������Ѱ��ҵ��
                                                                                   0x1: ֧��������Ѱ��ҵ�� */
    VOS_UINT8                           aucReserve[2];
}NVIM_VGCS_FLAG_STRU;

/*****************************************************************************
 �ṹ��    : NVIM_EGPRS_MULTI_SLOT_CLASS_STRU
 �ṹ˵��  : en_NV_Item_Egprs_Multi_Slot_Class �ṹ
 DESCRIPTION:  ��ʶMS��EGPRS��ʱ϶�����ȼ�
*****************************************************************************/
typedef struct
{
    VOS_UINT16                          usEgprsMultiSlotClass;                  /* Range: [0,12]
                                                                                   0x0:MS��ʱ϶�����ȼ�Ϊ0xC
                                                                                   0x1~0xC:ָʾMS��ʱ϶�����ȼ� */
    VOS_UINT8                           aucReserve[2];
}NVIM_EGPRS_MULTI_SLOT_CLASS_STRU;

/*****************************************************************************
 �ṹ��    : NVIM_GSM_CLASSMARK1_STRU
 �ṹ˵��  : en_NV_Item_Gsm_Classmark1 �ṹ
 DESCRIPTION:  GSM Classmark1�����������ڲ�ʹ��
*****************************************************************************/
typedef struct
{
    VOS_UINT8                           aucGsmClassmark1[NVIM_CLASSMARK1_LEN];  /* CLASSMARK1 ���� */
    VOS_UINT8                           aucReserve[2];
}NVIM_GSM_CLASSMARK1_STRU;

/*****************************************************************************
 �ṹ��    : NVIM_GSM_CLASSMARK2_STRU
 �ṹ˵��  : en_NV_Item_Gsm_Classmark2 �ṹ
 DESCRIPTION:  GSM Classmark2�����������ڲ�ʹ��
*****************************************************************************/
typedef struct
{
    VOS_UINT8                           aucGsmClassmark2[NVIM_CLASSMARK2_LEN];  /* CLASSMARK2 ���� */
}NVIM_GSM_CLASSMARK2_STRU;

/*****************************************************************************
 �ṹ��    : NVIM_GSM_CLASSMARK3_STRU
 �ṹ˵��  : en_NV_Item_Gsm_Classmark3 �ṹ
 DESCRIPTION:  GSM Classmark3�����������ڲ�ʹ��
*****************************************************************************/
typedef struct
{
    VOS_UINT8                           aucGsmClassmark3[NVIM_CLASSMARK3_LEN];  /* CLASSMARK3 ���� */
}NVIM_GSM_CLASSMARK3_STRU;

/*****************************************************************************
 �ṹ��    : NVIM_GSM_IND_FREQ_STRU
 �ṹ˵��  : en_NV_Item_Gsm_Ind_Freq �ṹ
 DESCRIPTION:  ����GSMƵ��ʱ���洢�û�ָ����Ƶ�㡣
               ��������ʱ���������û�ָ����Ƶ�㣬��Ҫ�Ƿ��������Ա����ʹ��
*****************************************************************************/
typedef struct
{
    VOS_UINT16                          usGsmIndFreq;                           /* �û�����Ƶ�㣬Ƶ��Ϊ0x0000~0xFFFE֮һ��
                                                                                   �û�û������Ƶ�㣬Ƶ��Ϊ0xFFFF */
    VOS_UINT8                           aucReserve[2];
}NVIM_GSM_IND_FREQ_STRU;

/*****************************************************************************
 �ṹ��    : NVIM_GCF_ITEM_STRU
 �ṹ˵��  : en_NV_Item_GCF_Item �ṹ
 DESCRIPTION:  GCF����׮���ڲ�ʹ�á��ѷ�����
*****************************************************************************/
typedef struct
{
    VOS_UINT8                           aucGcfItem[NVIM_GCF_ITEM_LEN];
}NVIM_GCF_ITEM_STRU;

/*****************************************************************************
 �ṹ��    : NVIM_G2W_RSSI_RSCP_OFFSET_STRU
 �ṹ˵��  : en_NV_Item_G2W_RSSI_RSCP_OFFSET �ṹ
 DESCRIPTION:  �ѷ���
*****************************************************************************/
typedef struct
{
    VOS_UINT16                          usG2WRssiRscpOffset;
    VOS_UINT8                           aucReserve[2];
}NVIM_G2W_RSSI_RSCP_OFFSET_STRU;

/*****************************************************************************
 �ṹ��    : NVIM_GSM_BA_COUNT_STRU
 �ṹ˵��  : en_NV_Item_GSM_Ba_Count �ṹ
 DESCRIPTION:  ����BA�����С���ĸ�����ÿ��פ����MS���Ὣ��ǰ����С���������ĸ������µ���NV���С�
*****************************************************************************/
typedef struct
{
    VOS_UINT16                          usGsmBaCount;                           /* 0x0000:�����BA�����С��������Ϊ0��
                                                                                   0x0001~0x0021:GSM BA LIST�б����BA�ͷ���С�������� */
    VOS_UINT8                           aucReserve[2];
}NVIM_GSM_BA_COUNT_STRU;

/*****************************************************************************
 �ṹ��    : NVIM_GSM_BA_LIST_STRU
 �ṹ˵��  : en_NV_Item_GSM_Ba_List �ṹ
 DESCRIPTION:  �������С��������PLMN�Լ�����С����������Ƶ����Ϣ��
               ÿ��פ���󣬶��Ὣ��ǰ�ķ���С��������PLMN�͵�ǰ����С����������Ƶ����µ���NV���С�
*****************************************************************************/
typedef struct
{
    VOS_UINT32                          ulMcc;                                  /* ����С�����ƶ������� */
    VOS_UINT32                          ulMnc;                                  /* ����С�����ƶ����� */
    VOS_UINT16                          usArfcn[NVIM_GSM_BA_MAX_SIZE];          /* ����С������������Ƶ��ţ�ausArfcn[0]�������С����Ƶ��
                                                                                   ausArfcn[1]~ausArfcn[33]����������Ƶ�� */
    VOS_UINT8                           aucReserve[2];
}NVIM_GSM_BA_LIST_STRU;

/*****************************************************************************
 �ṹ��    : NVIM_EGPRS_FLAG_STRU
 �ṹ˵��  : en_NV_Item_Egprs_Flag �ṹ
 DESCRIPTION:  MS�Ƿ�֧��EGPRS����NV�������ÿ��Modem��������
*****************************************************************************/
typedef struct
{
    VOS_UINT16                          usEgprsFlag;                            /* 0x0000:��֧��EGPRS
                                                                                   0x0001:֧��EGPRS */
    VOS_UINT8                           aucReserve[2];
}NVIM_EGPRS_FLAG_STRU;

/*****************************************************************************
 �ṹ��    : NVIM_EGPRS_RA_CAPABILITY_STRU
 �ṹ˵��  : en_NV_Item_EgprsRaCapability �ṹ
 DESCRIPTION:  EGPRS��RA�������ѷ���
*****************************************************************************/
typedef struct
{
    VOS_UINT8                           ucLength;                               /* �ֽڳ��� */
    VOS_UINT8                           aucEgprsRaCapabilityData[NVIM_EGPRS_RA_CAPABILITY_DATA_LEN];    /* �������� */
    VOS_UINT8                           aucReserve[2];
}NVIM_EGPRS_RA_CAPABILITY_STRU;

/*****************************************************************************
 �ṹ��    : NVIM_PREFER_GSM_PLMN_COUNT_STRU
 �ṹ˵��  : en_NV_Item_Prefer_GSM_PLMN_Count �ṹ
 DESCRIPTION:  �洢GSM��ѡPLMN�ĸ���
*****************************************************************************/
typedef struct
{
    VOS_UINT16                          usPreferGsmPlmnCount;                   /* ��ѡPLMN�ĸ��� */
    VOS_UINT8                           aucReserve[2];
}NVIM_PREFER_GSM_PLMN_COUNT_STRU;

/*****************************************************************************
 �ṹ��    : NVIM_PREFER_GSM_PLMN_LIST_STRU
 �ṹ˵��  : en_NV_Item_Prefer_GSM_PLMN_List �ṹ
 DESCRIPTION:  �洢GSM��ѡС������Ϣ������MCC��MNC�����PLMN�µĽ���ɹ�Ƶ������;���Ƶ��š�
               �����Ա���15��PLMN��ÿ��PLMN�������Ա���60��Ƶ�㡣
*****************************************************************************/
typedef struct
{
    VOS_UINT16                          ausPreferGsmPlmnListData[NVIM_PREFER_GSM_PLMN_LIST_LEN];    /* GSM��ѡС�����б����� */
}NVIM_PREFER_GSM_PLMN_LIST_STRU;

/*****************************************************************************
 �ṹ��    : NVIM_GSM_DEC_FAIL_ARFCN_COUNT_STRU
 �ṹ˵��  : en_NV_Item_GSM_DEC_FAIL_ARFCN_Count �ṹ
 DESCRIPTION:  �洢GSM����ʧ�ܵ�Ƶ��������ѷ���
*****************************************************************************/
typedef struct
{
    VOS_UINT16                          usGsmDecFailArfcnCount;                 /* ����ʧ�ܵ�Ƶ����� */
    VOS_UINT8                           aucReserve[2];
}NVIM_GSM_DEC_FAIL_ARFCN_COUNT_STRU;

/*****************************************************************************
 �ṹ��    : NVIM_GSM_DEC_FAIL_ARFCN_LIST_STRU
 �ṹ˵��  : en_NV_Item_GSM_DEC_FAIL_ARFCN_List �ṹ
 DESCRIPTION:  �洢GSM����ʧ�ܵ�Ƶ���б��ѷ���
*****************************************************************************/
typedef struct
{
    VOS_UINT16                          usGsmDecFailArfcnList[NVIM_GSM_DEC_FAIL_ARFCN_LIST_LEN];    /* ����ʧ�ܵ��б����� */
}NVIM_GSM_DEC_FAIL_ARFCN_LIST_STRU;

/*****************************************************************************
 �ṹ��    : NVIM_PREFER_GSM_PLMN_SWITCH_STRU
 �ṹ˵��  : en_NV_Item_Prefer_GSM_PLMN_Switch �ṹ
 DESCRIPTION:  ��ѡС���Ŀ��ƿ��ء���ѡС����ʱ��GSM_PLMN_Count��GSM_PLMN_List����NV�����á�
               ��NV�������á�
*****************************************************************************/
typedef struct
{
    VOS_UINT16                          usPreferGsmPlmnSwitch;                  /* 0x0000:���عرգ�0x0001�����ش� */
    VOS_UINT8                           aucReserve[2];
}NVIM_PREFER_GSM_PLMN_SWITCH_STRU;


typedef struct
{
    VOS_UINT8                           ucAgingEnable;                          /* ��ѡС���ϻ���NVʹ�� */
    VOS_UINT8                           ucRsrv1;                                /* ����λ */
    VOS_UINT8                           ucRsrv2;                                /* ����λ */
    VOS_UINT8                           ucRsrv3;                                /* ����λ */
    VOS_INT16                           sDecFailedRssiThreshold;                /* ����ѡС���б���ɾ������ʧ�ܵ�RSSI���� */
    VOS_UINT16                          usRsrv1;                                /* ����λ */
    VOS_UINT16                          usRsrv2;                                /* ����λ */
    VOS_UINT16                          usRsrv3;                                /* ����λ */
    VOS_UINT32                          ulRsrv1;                                /* ����λ */
    VOS_UINT32                          ulRsrv2;                                /* ����λ */
}NVIM_PREFER_GSM_PLMN_CUSTOMIZE_CFG_STRU;

/*****************************************************************************
 �ṹ��    : NVIM_GERAN_FEATURE_PACKAGE1_STRU
 �ṹ˵��  : en_NV_Item_Geran_Feature_Package1 �ṹ
 DESCRIPTION: ����MS�Ƿ�֧��Geran Feature Package1
*****************************************************************************/
typedef struct
{
    VOS_UINT16                          usGeranFeaturePackage1;                 /* Range: [0,1],0x0:��֧��,0x1:֧�� */
    VOS_UINT8                           aucReserve[2];
}NVIM_GERAN_FEATURE_PACKAGE1_STRU;

/*****************************************************************************
 �ṹ��    : NVIM_GSM_A5_STRU
 �ṹ˵��  : en_NV_Item_Gsm_A5 �ṹ
 DESCRIPTION: ���浥����֧�ֵ�A5�����㷨��Ŀǰ֧��A5/1��A5/2��A5/3��A5/4��
              Bitλ������������Ϊbit0��bit1...bit7��ÿ��bit����ĺ�������:
              bit0=0:A5/1��֧�֣�bit0=1:A5/1֧�֣�
              bit1=0:A5/2��֧�֣�bit1=1:A5/2֧�֣�
              bit2=0:A5/3��֧�֣�bit2=1:A5/3֧�֣�
              bit3=0:A5/4��֧�֣�bit3=1:A5/4֧�֣�
              ����bitλ�ݲ�ʹ��
*****************************************************************************/
typedef struct
{
    VOS_UINT16                          usGsmA5;                                /* Range: [0,15]
                                                                                   0x0:��֧��A5�㷨;
                                                                                   0x1:֧��A5/1;
                                                                                   0x2:֧��A5/2;
                                                                                   0x3:֧��A5/1��A5/2 
                                                                                   0x4:֧��A5/3;
                                                                                   0x5:֧��A5/3��A5/1;
                                                                                   0x6:֧��A5/3��A5/2;
                                                                                   0x7:֧��A5/3��A5/2��A5/3 */
    VOS_UINT8                           aucReserve[2];
}NVIM_GSM_A5_STRU;

/*****************************************************************************
 �ṹ��    : NVIM_LOW_COST_EDGE_FLAG_STRU
 �ṹ˵��  : en_NV_Item_LowCostEdge_Flag �ṹ
 DESCRIPTION: ��ʾMS�Ƿ�֧��lowCostEdge���ԡ����MS֧�ָ����ԣ������в�֧��8PSK���뷽ʽ��
*****************************************************************************/
typedef struct
{
    VOS_UINT16                          usLowCostEdgeFlag;                      /* Range: [0,1],0x0:��֧��,0x1:֧�� */
    VOS_UINT8                           aucReserve[2];
}NVIM_LOW_COST_EDGE_FLAG_STRU;

/*****************************************************************************
 �ṹ��    : NVIM_GPRS_ACTIVE_TIMER_LENGTH_STRU
 �ṹ˵��  : en_NV_Item_GPRS_ActiveTimerLength �ṹ
 DESCRIPTION: GPRS Active Timer�ĳ��ȡ�
*****************************************************************************/
typedef struct
{
    VOS_UINT32                          ulGprsActiveTimerLength;                /* 0x0000:100ms;
                                                                                   0x0001:100ms;
                                                                                   0x0002:200ms;�������� */
}NVIM_GPRS_ACTIVE_TIMER_LENGTH_STRU;

/*****************************************************************************
 �ṹ��    : NVIM_GPRS_MULTI_SLOT_CLASS_STRU
 �ṹ˵��  : en_Nv_Item_Gprs_Multi_Slot_Class �ṹ
 DESCRIPTION: ��ʶMS��GPRS��ʱ϶�����ȼ�
*****************************************************************************/
typedef struct
{
    VOS_UINT16                          usGprsMultiSlotClass;                   /* Range: [0,12]
                                                                                   0x0:MS��ʱ϶�����ȼ�Ϊ0xC
                                                                                   0x1~0xC:ָʾMS��ʱ϶�����ȼ� */
    VOS_UINT8                           aucReserve[2];
}NVIM_GPRS_MULTI_SLOT_CLASS_STRU;

/*****************************************************************************
 �ṹ��    : NVIM_GSM_PLMN_SEARCH_ARFCN_MAX_NUM_STRU
 �ṹ˵��  : en_NV_Item_GSM_PLMN_SEARCH_ARFCN_MAX_NUM �ṹ
 DESCRIPTION: GSM����ʱ�������������Ƶ��������Ƽ�ʹ��0x003C�����ϵ���ֵ
*****************************************************************************/
typedef struct
{
    VOS_UINT16                          usGsmPlmnSearchArfcmMaxNum;             /* Range: [0x1,0x8C],����ʱ���������Ƶ�����������ʹ��Ĭ��ֵ���ϵ�ֵ��ȷ������������ */
    VOS_UINT8                           aucReserve[2];
}NVIM_GSM_PLMN_SEARCH_ARFCN_MAX_NUM_STRU;

/*****************************************************************************
 �ṹ��    : NVIM_GCBS_CONF_STRU
 �ṹ˵��  : en_Nv_Item_GCBS_Conf �ṹ
 DESCRIPTION: ����GCBS�������ԣ�����ECBCH�ŵ�ʹ�ܿ��ء�GCBS Drx����ʹ�ܿ��غ�GCBS���ʱ��ʱ��
*****************************************************************************/
typedef struct
{
    VOS_UINT16                                      usECBCHSwitch;              /* �Ƿ�����ECBCH�ŵ�,0x0:������,0x1:���� */
    VOS_UINT16                                      usDrxSwitch;                /* �Ƿ�����GCBS DRX����,0x0:������,0x1:���� */
    VOS_UINT32                                      ulGCBSActiveTimerLength;    /* GCBS���ʱ��ʱ������λms */
}NVIM_GCBS_CONF_STRU;

/*****************************************************************************
 �ṹ��    : NVIM_REPEATED_ACCH_CAPABILITY_STRU
 �ṹ˵��  : en_NV_Item_Repeated_Acch_Capability �ṹ
 DESCRIPTION: �Ƿ�֧��Repeated FACCH/SACCH���ܡ�������R6��ʹ��
*****************************************************************************/
typedef struct
{
    VOS_UINT16                                      usRepeatedAcchCapability;   /* 0x0:��֧�֣�BSSֻ��������repeated FACCH�鷢��;
                                                                                   0x1:֧�֣�BSS����������repeated FACCH�������͡� */
    VOS_UINT8                                       aucReserve[2];
}NVIM_REPEATED_ACCH_CAPABILITY_STRU;


/*****************************************************************************
 �ṹ��    : NVIM_ES_IND_STRU
 �ṹ˵��  : en_NV_Item_ES_IND �ṹ
 DESCRIPTION: �Ƿ�֧��Controlled Early Classmark Sending
*****************************************************************************/
typedef struct
{
    VOS_UINT16                                      usEsInd;                    /* Range: [0,1],0x0:��֧��,0x1:֧�� */
    VOS_UINT8                                       aucReserve[2];
}NVIM_ES_IND_STRU;

/*****************************************************************************
 �ṹ��    : NVIM_GPRS_EXTENDED_DYNAMIC_ALLOCATION_CAPABILITY_STRU
 �ṹ˵��  : en_NV_Item_GPRS_Extended_Dynamic_Allocation_Capability �ṹ
 DESCRIPTION: �Ƿ�֧��GPRS��չ��̬��������
*****************************************************************************/
typedef struct
{
    VOS_UINT16                                      usGprsExtDynAllocCap;       /* Range: [0,1],0x0:��֧��,0x1:֧�� */
    VOS_UINT8                                       aucReserve[2];
}NVIM_GPRS_EXTENDED_DYNAMIC_ALLOCATION_CAPABILITY_STRU;

/*****************************************************************************
 �ṹ��    : NVIM_EGPRS_EXTENDED_DYNAMIC_ALLOCATION_CAPABILITY_STRU
 �ṹ˵��  : en_NV_Item_EGPRS_Extended_Dynamic_Allocation_Capability �ṹ
 DESCRIPTION: �Ƿ�֧��EGPRS��չ��̬��������
*****************************************************************************/
typedef struct
{
    VOS_UINT16                                      usEGprsExtDynAllocCap;      /* Range: [0,1],0x0:��֧��,0x1:֧�� */
    VOS_UINT8                                       aucReserve[2];
}NVIM_EGPRS_EXTENDED_DYNAMIC_ALLOCATION_CAPABILITY_STRU;

/*****************************************************************************
 �ṹ��    : NVIM_REVISION_LEVEL_INDICATOR_STRU
 �ṹ˵��  : en_NV_Item_Revision_Level_Indicator �ṹ
 DESCRIPTION: ME�����汾
*****************************************************************************/
typedef struct
{
    VOS_UINT16                                      usRevLvlInd;                /* Range: [0,1],0x0:R99֮ǰ�汾,0x1:RR��֮��汾 */
    VOS_UINT8                                       aucReserve[2];
}NVIM_REVISION_LEVEL_INDICATOR_STRU;

/*****************************************************************************
 �ṹ��    : NVIM_GAS_REVISION_LEVEL_CUSTOMIZE_STRU
 �ṹ˵��  : en_NV_Item_GAS_Revision_Level_Customization �ṹ
 DESCRIPTION: ��ϵͳ��ȡclassmark2�е�revision level IE��������
*****************************************************************************/
typedef struct
{
    VOS_UINT8                            ucInterRatCustomizeFlag;       /* ��ϵͳ��ȡclassmark2�е�Revison level IE�������ƣ�
                                                                           0:��ϵͳ��ȡ���������ƣ�
                                                                           1:��ʾ������Ч����ϵͳ��ȡ����classmark2��Revison level�̶����R99���Ժ�汾
                                                                        */
    
    VOS_UINT8                            ucRsv1;                        /* ����λ1 */
    
    VOS_UINT8                            ucRsv2;                        /* ����λ2 */
    
    VOS_UINT8                            ucRsv3;                        /* ����λ3 */

}NVIM_GAS_REVISION_LEVEL_CUSTOMIZE_STRU;

/*****************************************************************************
 �ṹ��    : NVIM_DOWNLINK_ADVANCED_RECEIVER_PERFORMANCE_STRU
 �ṹ˵��  : en_NV_Item_Downlink_Advanced_Receiver_Performance �ṹ
 DESCRIPTION: �Ƿ�֧�����и߼���������
*****************************************************************************/
typedef struct
{
    VOS_UINT16                                      usDlAdvRcvPer;              /* Range: [0,1],0x0:��֧��,0x1:֧�� */
    VOS_UINT8                                       aucReserve[2];
}NVIM_DOWNLINK_ADVANCED_RECEIVER_PERFORMANCE_STRU;

/*****************************************************************************
 �ṹ��    : NVIM_EXT_RLC_MAC_CTRL_MSG_SEGMENT_CAPABILITY_STRU
 �ṹ˵��  : en_NV_Item_Ext_RLC_MAC_Ctrl_Msg_Segment_Capability �ṹ
 DESCRIPTION: MS�Ƿ�֧����չRLC/MAC������Ϣ�ֶ�
*****************************************************************************/
typedef struct
{
    VOS_UINT16                                      usExtRlcMacCtrlMsgSegCap;   /* Range: [0,1],0x0:��֧��,0x1:֧�� */
    VOS_UINT8                                       aucReserve[2];
}NVIM_EXT_RLC_MAC_CTRL_MSG_SEGMENT_CAPABILITY_STRU;

/*****************************************************************************
 �ṹ��    : NVIM_PS_HANDOVER_CAPABILITY_STRU
 �ṹ˵��  : en_NV_Item_PS_Handover_Capability �ṹ
 DESCRIPTION: MS�Ƿ�֧��PS���л�
*****************************************************************************/
typedef struct
{
    VOS_UINT16                                      usPsHandoverCapability;     /* Range: [0,1],0x0:��֧��,0x1:֧�� */
    VOS_UINT8                                       aucReserve[2];
}NVIM_PS_HANDOVER_CAPABILITY_STRU;

/*****************************************************************************
 �ṹ��    : NVIM_GAS_WEAK_SIGNAL_THREHOLD_STRU
 �ṹ˵��  : en_NV_Item_GAS_Errorlog_Energy_Threshold �ṹ
 DESCRIPTION: ����GSM ErrorLog�������źŵ�RSSI����ֵ 
*****************************************************************************/
typedef struct
{
    VOS_INT16                                       sWeakSignalThreshold;       /* GSM ErrorLog�������źŵ�RSSI����ֵ��Range:[-120,20] */
    VOS_UINT8                                       aucReserve[2];
}NVIM_GAS_WEAK_SIGNAL_THREHOLD_STRU;

/*****************************************************************************
 �ṹ��    : NVIM_GSM_MULTIRATE_CAP_STRU
 �ṹ˵��  : en_NV_Item_GSM_Multirate_Capability �ṹ
 DESCRIPTION: UE֧�ֵ�GSM����������
*****************************************************************************/
typedef struct
{
    VOS_UINT8                                       ucGsmMultirateCap;          /* GSM����������.0x0:��֧��ȫ����,0x1:֧��DUAL RATE,0x2:��֧��SDCCH */
    VOS_UINT8                                       aucRsv[3];
}NVIM_GSM_MULTIRATE_CAP_STRU;

/*****************************************************************************
 �ṹ��    : NVIM_GSM_CLASSMARK3_R8_STRU
 �ṹ˵��  : en_NV_Item_Gsm_Classmark3_R8 �ṹ
 DESCRIPTION: GSM Classmark3�����������ڲ�ʹ��
*****************************************************************************/
typedef struct
{
    VOS_UINT8                                       aucGsmClassMark3Data[NVIM_CLASSMARK3_R8_LEN];   /* CLASSMARK3 ���� */
}NVIM_GSM_CLASSMARK3_R8_STRU;

/*****************************************************************************
 �ṹ��    : NVIM_EUTRA_MEAS_AND_REPORTING_SUPPORT_FLG_STRU
 �ṹ˵��  : en_NV_Item_EUTRA_MEAS_AND_REPORTING_SUPPORT_FLG �ṹ
 DESCRIPTION: MS�Ƿ�֧����ר��̬�½���EUTRAN�������������ϱ�
*****************************************************************************/
typedef struct
{
    VOS_UINT16                                      usLteMeasSupportedFlg;      /* Range: [0,1],0x0:��֧��,0x1:֧�� */
    VOS_UINT8                                       aucReserve[2];
}NVIM_EUTRA_MEAS_AND_REPORTING_SUPPORT_FLG_STRU;

/*****************************************************************************
 �ṹ��    : NVIM_PRI_BASED_RESEL_SUPPORT_FLG_STRU
 �ṹ˵��  : en_NV_Item_PRI_BASED_RESEL_SUPPORT_FLG �ṹ
 DESCRIPTION: MS�Ƿ�֧�ֻ������ȼ�����ѡ
*****************************************************************************/
typedef struct
{
    VOS_UINT16                                      usPriBasedReselSupportFlg;  /* Range: [0,1],0x0:��֧��,0x1:֧�� */
    VOS_UINT8                                       aucReserve[2];
}NVIM_PRI_BASED_RESEL_SUPPORT_FLG_STRU;

/*****************************************************************************
 �ṹ��    : NVIM_GERAN_TO_EUTRA_SUPPORT_IN_TRANSFER_MODE_STRU
 �ṹ˵��  : en_NV_Item_GERAN_TO_EUTRA_SUPPORT_IN_TRANSFER_MODE �ṹ
 DESCRIPTION: MS������̬֧�ֵ�LTE��ص���ϵͳ����������
*****************************************************************************/
typedef struct
{
    VOS_UINT16                                      usLteSupportInTransferMode; /* Range: [0,3]
                                                                                   0x0:��֧��GSM����̬�µ�E-UTRAN����������������ѡ;
                                                                                   0x1:֧��GSM����̬�µ�E-UTRAN����������������ѡ,��֧��GSM����̬��EUTRAN������������CCN/CCO��EUTRAN
                                                                                   0x2:֧��GSM����̬�µ�E-UTRAN����������������ѡ,֧��GSM����̬��EUTRAN������������CCN/CCO��EUTRAN
                                                                                   0x3:����1��2���⣬��֧��PS HANDOVER��EUTRAN,��ʱƽ̨��֧�ָ����� */
    VOS_UINT8                                       aucReserve[2];
}NVIM_GERAN_TO_EUTRA_SUPPORT_IN_TRANSFER_MODE_STRU;

/*****************************************************************************
 �ṹ��    : NVIM_UE_EUTRA_FREQ_BAND_INFO_STRU
 �ṹ˵��  : ��������һ�� LTE Ƶ��
*****************************************************************************/
typedef struct
{
    VOS_UINT8                           ucBandNo;
    VOS_UINT8                           ucDuplexModeFlg;
    VOS_UINT8                           aucReserve[2];
}NVIM_UE_EUTRA_FREQ_BAND_INFO_STRU;

/*****************************************************************************
 �ṹ��    : NVIM_UE_EUTRA_SUPPORT_FREQ_BAND_LIST_STRU
 �ṹ˵��  : en_NV_Item_EUTRA_CAPA_COMM_INFO �ṹ
*****************************************************************************/
typedef struct
{
    VOS_UINT16                              usBandCnt;
    VOS_UINT8                               aucReserved1[2];
    NVIM_UE_EUTRA_FREQ_BAND_INFO_STRU       astCandBands[NVIM_EUTRA_MAX_SUPPORT_BANDS_NUM];
}NVIM_UE_EUTRA_SUPPORT_FREQ_BAND_LIST_STRU;

/*****************************************************************************
 �ṹ��    : NVIM_GAS_HIGH_MULTISLOT_CLASS_STRU
 �ṹ˵��  : en_NV_Item_GAS_High_Multislot_Class �ṹ
 DESCRIPTION: MS�Ƿ�֧��Multi class33
*****************************************************************************/
typedef struct
{
    VOS_UINT16                              usHighMultislotClassFlg;            /* Range: [0,1],0x0:��Ч,0x1:��Ч */
    VOS_UINT16                              usHighMultislotClass;               /* �ȼ����ͣ���ϸ����NV˵���� */
}NVIM_GAS_HIGH_MULTISLOT_CLASS_STRU;

/*****************************************************************************
 �ṹ��    : NVIM_GPRS_NON_DRX_TIMER_LENGTH_STRU
 �ṹ˵��  : en_NV_Item_GPRS_Non_Drx_Timer_Length �ṹ
 DESCRIPTION: �ն˵�NON-DRXʱ������������Ҫ��RAU��Attach��Ϣ�д�������
*****************************************************************************/
typedef struct
{
    VOS_UINT16                              usNonDrxTimerLen;                   /* Range: [0,7]
                                                                                   0:NON-DRXʱ��Ϊ0��
                                                                                   1:NON-DRXʱ��Ϊ1��
                                                                                   2:NON-DRXʱ��Ϊ2��
                                                                                   3:NON-DRXʱ��Ϊ4��
                                                                                   4:NON-DRXʱ��Ϊ8��
                                                                                   5:NON-DRXʱ��Ϊ16��
                                                                                   6:NON-DRXʱ��Ϊ32��
                                                                                   7:NON-DRXʱ��Ϊ64��
                                                                                   */
    VOS_UINT8                               aucReserve[2];
}NVIM_GPRS_NON_DRX_TIMER_LENGTH_STRU;

/*****************************************************************************
 �ṹ��    : NVIM_UE_TDS_SUPPORT_FREQ_BAND_LIST_STRU
 �ṹ˵��  : en_NV_Item_UTRAN_TDD_FREQ_BAND �ṹ
 DESCRIPTION: TDS֧�ֵ�Ƶ�θ�����Ƶ�κ�
*****************************************************************************/
typedef struct
{
    VOS_UINT8                               ucBandCnt;                          /* ֧�ֵ�TDSƵ�θ���,Ŀǰ���֧��3��Ƶ��(A/E/F),Range: [0,6] */
    VOS_UINT8                               aucReserved[3];
    VOS_UINT8                               aucBandNo[NVIM_TDS_MAX_SUPPORT_BANDS_NUM];  /* aucBandNo[x]��ʾ֧�ֵ�TDSƵ�κ� */
}NVIM_UE_TDS_SUPPORT_FREQ_BAND_LIST_STRU;

/*****************************************************************************
 �ṹ��    : NVIM_QSEARCH_CUSTOMIZATION_STRU
 �ṹ˵��  : en_NV_Item_QSearch_Customization �ṹ
 DESCRIPTION: ��ϵͳ�������Ʋ���QSearchȡֵ��Ķ���ģʽ
*****************************************************************************/
typedef struct
{
    VOS_UINT16                              usQSearchCustomMode;                /* 0x0:3GPPЭ��ģʽ,0x1:���ƶ�����ģʽ */
    VOS_UINT8                               aucReserve[2];
}NVIM_QSEARCH_CUSTOMIZATION_STRU;

/*****************************************************************************
 �ṹ��    : NVIM_GSM_POOR_RXQUAL_THRESHOLD_STRU
 �ṹ˵��  : en_NV_Item_Gsm_Poor_RxQual_ThresHold �ṹ
 DESCRIPTION: �ѷ���
*****************************************************************************/
typedef struct
{
    VOS_UINT16                              usRrPoorRxQualThresHold;
    VOS_UINT8                               aucReserve[2];
}NVIM_GSM_POOR_RXQUAL_THRESHOLD_STRU;

/*****************************************************************************
 �ṹ��    : NVIM_CSFB_CUSTOMIZATION_STRU
 �ṹ˵��  : en_NV_Item_Csfb_Customization �ṹ
 DESCRIPTION: L2G CSFB����ʱ���Ż����ܵĶ���ģʽ
*****************************************************************************/
typedef struct
{
    VOS_UINT16                              usCsfbCustomization;                /* 0x0:���Ʋ���Ч,0x1:������Ч */
    VOS_UINT8                               aucReserve[2];
}NVIM_CSFB_CUSTOMIZATION_STRU;

/*****************************************************************************
 �ṹ��    : NVIM_CBS_MID_LIST_STRU
 �ṹ˵��  : en_NV_Item_CBS_MID_List �ṹ
 DESCRIPTION: �ѷ���
*****************************************************************************/
typedef struct
{
    VOS_UINT8                               aucCbsMidList[NVIM_CBS_MID_LIST_LEN];
}NVIM_CBS_MID_LIST_STRU;

/*****************************************************************************
 �ṹ��    : NVIM_CBS_MID_RANGE_LIST_STRU
 �ṹ˵��  : en_NV_Item_CBS_MID_Range_List �ṹ
 DESCRIPTION: �ѷ���
*****************************************************************************/
typedef struct
{
    VOS_UINT8                               aucCbsMidRangeList[NVIM_CBS_MID_RANGE_LIST_LEN];
}NVIM_CBS_MID_RANGE_LIST_STRU;


typedef struct
{
   VOS_UINT8                            ucFrLteMeasFlag;                        /* �Ƿ���GSM�����ض���LTE��LTEƵ�������0: �ر�, 1: ������Ĭ��ֵΪ1 */
   VOS_UINT8                            ucMaxSavedMeasTimes;                    /* ��󱣴�Ĳ���������Ĭ��ֵΪ8����Χ:[0,8] */

   VOS_UINT8                            ucFrInvalidMeasFlag;                    /* �Ƿ����ò�������ж�GSM�����ض���LTE�Ƿ���Ч��0: �ر�, 1: ������Ĭ��ֵΪ1 */
   VOS_UINT8                            ucMaxInvalidMeasTimes;                  /* �����Ч���������������ж�Ƶ���Ƿ���Ч����������󱣴�Ĳ���������Ĭ��ֵΪ8����Χ:[0,8] */
}NVIM_GSM_AUTO_FR_LTE_MEAS_CONFIG_STRU;


typedef struct
{
   VOS_UINT8                            ucC1CustomizeSwitchFlag;          /* �Ƿ���·��C1���ƣ�0: �ر�, 1: ������Ĭ��ֵΪ0 */
   VOS_UINT8                            ucRxlevAccessMin;                 /* ����·��C1���ƺ󣬶��Ƶ���С�����ƽ�ȼ���Ĭ�ϵȼ�Ϊ8����Χ:[0,63] */
   VOS_UINT8                            ucC1ValueThreshold;               /* C1��ֵ */
   VOS_UINT8                            aucRsv[1];
}NVIM_GSM_C1_CUSTOMIZE_CFG_STRU;

/*****************************************************************************
 �ṹ��    : NVIM_GSM_NFREQ_THRESHOLD_STRU
 �ṹ˵��  : en_NV_Item_Gsm_NFreq_Threshold �ṹ
 DESCRIPTION: ��Ƶ���Ź�������
*****************************************************************************/
typedef struct
{
    VOS_UINT8                            ucFirstNFreqThreshold;                 /* ��Ƶ���Ź������� */
    VOS_UINT8                            aucRsv[3];
}NVIM_GSM_NFREQ_THRESHOLD_STRU;

/*****************************************************************************
 �ṹ��    : NVIM_GAS_W_NON_NCELL_MEAS_CTRL_STRU
 �ṹ˵��  : en_NV_Item_Gas_W_Non_NCell_Meas_Ctrl �ṹ
 DESCRIPTION: ��NV��������GSM�£�WCDMA�����������Ĳ���
*****************************************************************************/
typedef struct
{
    VOS_UINT8                            ucIdleNonNCellMeasEnable;              /* ����̬���Ƿ�����������������0: �ر�, 1: ���� */
    VOS_UINT8                            ucTransferNonNCellMeasEnable;          /* ����̬���Ƿ�����������������0: �ر�, 1: ���� */
    VOS_INT16                            sRscpThreshold;                        /* ���������������� RSCP ���ޣ�Range:[-119,0) */
    VOS_INT16                            sEcn0Threshold;                        /* ���������������� ECN0 ���ޣ�Range:[-20,0) */
    VOS_INT16                            sRssiThreshold;                        /* ���������������� RSSI ���ޣ�Range:[-101,0) */
}NVIM_GAS_W_NON_NCELL_MEAS_CTRL_STRU;
/* Added by l67237 for Gas Dsds Feature, 2014-3-4, begin */
/*****************************************************************************
 �ṹ��    : NV_GSM_RF_UNAVAILABLE_CFG_STRU
 �ṹ˵��  : en_NV_Item_GSM_RF_UNAVAILABLE_CFG �ṹ
 DESCRIPTION: �Ƿ����������Ŀ��غ�����������ʱ��
*****************************************************************************/
typedef struct
{
    VOS_UINT8                           ucStartSearchFlag;                      /* �Ƿ���������:,1:��; 0:�� */
    VOS_UINT8                           ucSearchTimerLen;                       /* �೤ʱ��������������λ:s */
    VOS_UINT8                           ucEnableScellSbFbFlag;                  /* �Ƿ���Ч����С��ͬ����1:�ǣ�0:�� */
    VOS_UINT8                           ucRsv;
}NV_GSM_RF_UNAVAILABLE_CFG_STRU;
/* Added by l67237 for Gas Dsds Feature, 2014-3-4, end */

/*****************************************************************************
 �ṹ��    : NVIM_GSM_CELL_INFO_RPT_CFG_STRU
 �ṹ˵��  : en_NV_Item_Gsm_Cell_Info_Rpt_Cfg �ṹ
 DESCRIPTION: GAS��MTC�ϱ���ǰС����Ϣ��������Ϣ
*****************************************************************************/
typedef struct
{
    VOS_UINT8                            ucCellInfoRptFlg;                      /* �Ƿ������ϱ�: 0:������; 1:���� */
    VOS_UINT8                            aucRsv[1];                             /* ����λ */
    VOS_UINT16                           usRptInterval;                         /* �ϱ����ڳ���, ��λ: ms */
}NVIM_GSM_CELL_INFO_RPT_CFG_STRU;

/*****************************************************************************
 �ṹ��    : NVIM_GAS_INDIVIDUAL_CUSTOMIZE_CFG_STRU
 �ṹ˵��  : en_NV_Item_Gas_Individual_Customize_Cfg �ṹ
 DESCRIPTION: �������÷Ǳ�����ⶨ����ص�����
*****************************************************************************/
typedef struct
{
    VOS_UINT8                           ucAccFailNoPunishCfg;                   /* ���ֶ��ѱ�����,�滻ʹ��NV3036 */
    VOS_INT8                            cFreqRxlevThreshold;                    /* ��ЧƵ����������,��Ҫ������������
                                                                                   ������ѡ Ŀ��С��ѡ��ʱҲ���õ� */

    VOS_UINT8                           ucAutoFrNonChanRelCase;                 /* û���յ� Channel Release ���쳣�������Ƿ����� FR:
                                                                                   bit0: 1: �����Ҷ�ʱ���� FR;
                                                                                         0: �����Ҷ�ʱ������FR
                                                                                   bit1: 1: ������·ʧ������FR;
                                                                                         0: ������·ʧ�ܲ�����FR
                                                                                   bit2: 1: ������·ʧЧ��������ǰ��Ҫ�ȳ��Ժ����ؽ�;
                                                                                         0: ������·ʧЧ��������ǰ����Ҫ�ȳ��Ժ����ؽ� */

    VOS_UINT8                           ucScellPrioFastEnableFlg;               /* ��SI2Qδ����ǰ���Ƿ������Ч����С�����ȼ���0:����Ч,1:��Ч */

    VOS_UINT8                           aucRsv[36];
}NVIM_GAS_INDIVIDUAL_CUSTOMIZE_CFG_STRU;


typedef struct
{
   VOS_UINT16                           usCsHoTimeAlarmThreshold;              /* CS�л�ʱ�䳤�����ޣ��������л�ʱ�䣬GAS�����ϱ��澯��Ϣ */

   VOS_UINT16                           usRptPseudBtsMinTimeInterval;          /* ����α��վCHR�ϱ������ʱ��������λ:���ӣ�Ĭ��ֵΪ30���� */   

   VOS_INT16                            sRptRaFailRssiThreshold;               /* �������ʧ�ܵ��������ޣ�������������GAS�����ϱ��������ʧ���¼�, Ĭ��ֵΪ-85dBm */

   VOS_UINT16                           usRptRaNoRspMinTimeInterval;           /* ���������������ӦCHR�ϱ������ʱ��������λ:���ӣ�Ĭ��ֵΪ30���� */   
   
   VOS_INT16                            sRaNoRspRssiThreshold;                 /* һ����Ч�������������Ӧ���������ޣ�������������GAS��¼һ����Ч�������������Ӧ,Ĭ��ֵΪ-85dBm */
   
   VOS_UINT8                            usRaNoRspAlarmTimesThreshold;          /* ���������������Ӧ�������ޣ������ô�����GAS�����ϱ��澯��Ϣ��Ĭ��ֵ��10�� */   

   VOS_UINT8                            ucRptAbnormalAirMsgMinTimeInterval;    /* ���οտ��쳣CHR�ϱ������ʱ��������λ:���ӣ�Ĭ��ֵΪ30���� */      

   VOS_UINT32                           ulRptRaFailMinTimeInterval;            /* �����������ʧ�������ϱ������ʱ��������λ:���ӣ�Ĭ��ֵΪ30���� */

   VOS_UINT32                           ulRsv7;                                /* Ԥ��λ7��Ϊ�Ժ�KWKCԤ�� */
}NVIM_GAS_MNTN_CONFIG_STRU;


typedef struct
{
   VOS_UINT8                            ucNotSuppBssPagingCoorFlag;             /* ��֧��Ѱ��Э���¼��ϱ����� */
   
   VOS_UINT8                            ucNotSuppBssPagingCoorMaxNum;           /* ��֧��Ѱ��Э���¼��ϱ�С���� */

   VOS_UINT8                            ucRsrcCheckExcpFlag;                    /* ��Դ�˲��쳣�¼��ϱ����� */

   VOS_UINT8                            ucRsrcCheckExcpMaxNum;                  /* ��Դ�˲��쳣�¼��ϱ���¼���� */

   VOS_UINT8                            ucCustomC1CellFlag;                     /* C1�Ż�С���¼��ϱ����� */

   VOS_UINT8                            ucCustomC1CellMaxNum;                   /* C1�Ż�С���¼��ϱ����� */

   VOS_UINT8                            ucRsv5;                                 /* Ԥ��λ��Ϊ�Ժ�KWKCԤ�� */

   VOS_UINT8                            ucRsv6;                                 /* Ԥ��λ��Ϊ�Ժ�KWKCԤ�� */

   VOS_UINT16                           usRsv1;                                 /* Ԥ��λ��Ϊ�Ժ�KWKCԤ�� */   

   VOS_UINT16                           usRsv2;                                 /* Ԥ��λ��Ϊ�Ժ�KWKCԤ�� */

   VOS_UINT16                           usRsv3;                                 /* Ԥ��λ��Ϊ�Ժ�KWKCԤ�� */
   
   VOS_UINT16                           usRsv4;                                 /* Ԥ��λ��Ϊ�Ժ�KWKCԤ�� */

   VOS_UINT32                           ulNotSuppBssPagingCoorInterval;         /* ��֧��Ѱ��Э���¼��ϱ�ʱ������Ĭ�� 30min, ��λ: 10ms */  

   VOS_UINT32                           ulRsrcCheckExcpInterval;                /* ��Դ�˲��쳣�¼��ϱ�ʱ������Ĭ�� 60min, ��λ: min */

   VOS_UINT32                           ulCustomC1CellInterval;                 /* C1�Ż�С���¼��ϱ�ʱ������Ĭ�� 30min, ��λ: min */

   VOS_UINT32                           ulRsv4;                                 /* Ԥ��λ��Ϊ�Ժ�KWKCԤ�� */
}NVIM_GAS_MNTN_CHR_DIRECT_RPT_CONFIG_STRU;



typedef struct
{
   VOS_UINT8                            ucTdsDefaultQRxlMin;            /* TDS Ĭ����ѡ����, ��λ: -1dB */

   VOS_UINT8                            aucRsv[3];                      /* ����λ */
}NVIM_GAS_UTRAN_TDD_DEFAULT_Q_RXLMIN;

/* Added by yangsicong for L2G REDIR C1 CUSTUME, 2015-1-26, begin */
typedef struct
{
    VOS_UINT32                          ulMcc;
    VOS_UINT32                          ulMnc;
}NVIM_GAS_PLMN_ID_STRU;


typedef struct
{
   VOS_UINT16                           usPlmnCnt;                          /* ��������PLMN���� */
   VOS_UINT16                           usRxlevAccessMin;                   /* ��������С�����Ƶ���С�����ƽ�ȼ�, Ĭ�ϵȼ�Ϊ8����Χ:[0,63]
                                                                               0ֵ��ʾ�������Ч, ��������ʹ�� NV9248 �� ucRxlevAccessMin ���ֵ */

   NVIM_GAS_PLMN_ID_STRU                astPlmn[NVIM_GAS_C1_CALC_OPT_PLMN_WHITE_LIST_CNT_MAX];       /* �������е�PLMN ID���������20�� */
}NVIM_GAS_C1_CALC_OPT_PLMN_WHITE_LIST_STRU;
/* Added by yangsicong for L2G REDIR C1 CUSTUME, 2015-1-26, end */

/*****************************************************************************
 �ṹ��    : NVIM_GSM_RAPID_HO_CUSTOMIZE_CFG_STRU
 �ṹ˵��  : en_NV_Item_GSM_RAPID_HO_CUSTOMIZE_CFG �ṹ,GSM�����л����������������
*****************************************************************************/
typedef struct
{
    VOS_UINT8                           ucSwitchFlag;                   /* ���Կ��� */
    VOS_UINT8                           ucBadQualityThreshold;          /* �ź������ж�����.������.��λ: �ٷ�֮һ */
    VOS_UINT16                          usAlpha;                        /* Alpha����. ��λ: 0.001 */
    VOS_UINT8                           ucBadQualityCntThreshold;       /* �ź�������ͳ������ */
    VOS_UINT8                           ucNCellRptAddValue;             /* �����ϱ�����ֵ */
    VOS_UINT8                           aucRsv[2];                      /* ����λ */
}NVIM_GSM_RAPID_HO_CUSTOMIZE_CFG_STRU;


typedef struct
{
    NVIM_GAS_PLMN_ID_STRU               stPlmn;                                 /* ����Ƶ�ε�PLMN */
    VOS_UINT32                          ulBand;                                 /* Ƶ�� */
}NVIM_GSM_BAND_CUSTOMIZE_STRU;


typedef struct
{
   VOS_UINT16                           usItemCnt;                  /* �����������Range:[0-80] */
   VOS_UINT16                           usRestoreEnableMask;        /* ���ñ������ĳ���Э�鷶Χ��Ƶ����Ч����,
                                                                       bit0:L2G�ض������̣�0x0:����Ч,0x1:��Ч 
                                                                       bit1:GOOS������, 0x0:����Ч,0x1:��Ч */
   VOS_UINT32                           ulCustomizeBandEnableMask;  /* ���� Band ��Ч����
                                                                       bit0:L2G�ض������̣�0x0:����Ч,0x1:��Ч 
                                                                       bit1:GOOS������, 0x0:����Ч,0x1:��Ч */
   NVIM_GSM_BAND_CUSTOMIZE_STRU         astCustomizeBand[NVIM_GAS_GSM_BAND_CUSTOMIZE_LIST_MAX_CNT];     /* PLMN�б� */
}NVIM_GAS_GSM_BAND_CUSTOMIZE_CFG_STRU;

/*****************************************************************************
 �ṹ��    : NVIM_GSM_SEARCH_CUSTOMIZE_CFG_STRU
 �ṹ˵��  : en_NV_Item_GSM_SEARCH_CUSTOMIZE_CFG �ṹ��GSM���������������
 DESCRIPTION: GSM���������������
*****************************************************************************/
typedef struct
{
    VOS_UINT8                           ucRsv1;                                 /* ������������ָ���������Ƿ�ʹ����ʷƵ�� */
    VOS_UINT8                           ucRsv2;                                 /* ������GOos�������Ƿ�ʹ����ʷƵ�� */

    VOS_UINT8                           ucGeoHighUseStoreFlag;                  /* ��������ȡ������Ϣ�����Ƿ�ʹ����ʷƵ�� */
    VOS_UINT8                           ucGeoHighRemoveStoreFlag;               /* ��������ȡ������Ϣ�����Ƿ��ڸ�����ɨƵ�׶�ɾ����ʷƵ�� */
    VOS_UINT8                           ucGeoNormalUseStoreFlag;                /* �Ǹ�������ȡ������Ϣ�����Ƿ�ʹ����ʷƵ�� */

    VOS_UINT8                           ucRmvDecFailSamePlmnFlag;               /* Ƶ�����ʧ�ܺ��Ƿ�ɾ��������ͬPLMN����ѡС�� */

    VOS_UINT8                           ucUpdateRssiBySi;                       /* ����ϵͳ��Ϣ�����ź�ǿ�� */

    VOS_UINT8                           ucHistorySrchOperatorCustomizeCellCnt;  /* HISTORY������Ӫ�̶���Ƶ��ĸ�����Χ */

}NVIM_GSM_SEARCH_CUSTOMIZE_CFG_STRU;


typedef struct
{
    VOS_UINT8                           ucEnableFlg;                    /* �Ƿ���Ч,0x0:����Ч,0x1:��Ч */
    VOS_UINT8                           ucRefreshTimeLen;               /* ʹ����ʷϵͳ��Ϣפ������ǿ��ˢ��ϵͳ��Ϣ.
                                                                           ��λ:��. 0��ʾ��ǿ��ˢ�� */

    VOS_UINT16                          usValidTimeLen;                 /* ��Чʱ��, ��λ:���� */
    VOS_UINT32                          ulUsableMask;                   /* ����ָ������ʹ����ʷϵͳ��Ϣפ�������� */
    VOS_UINT8                           ucNcellEnableFlg;               /* ����ϵͳ��Ϣ�Ƿ���Ч */
    VOS_UINT8                           ucNcellValidTimeLen;            /* ��Чʱ��, ��λ:�� */
    VOS_UINT8                           ucRsv3;                         /* ����λ */
    VOS_UINT8                           ucRsv4;                         /* ����λ */
}NVIM_GAS_GSM_CELL_HISTORY_BCCH_SI_CFG_STRU;


typedef struct
{
    VOS_UINT8                            ucHPrioCustomizeFlag;   /* H_PRIOʹ�ó�����������, 0��ʾʹ��H_PRIO������������ϵͳС�������ȼ���
                                                                    1��ʾʹ��H_PRIO������ǰ����������ϵͳС�������ȼ��ȷ���С�����ȼ��� */
    
    VOS_UINT8                            ucRsv1;                 /* ����λ1 */
    
    VOS_UINT8                            ucRsv2;                 /* ����λ2 */
    
    VOS_UINT8                            ucRsv3;                 /* ����λ3 */
}NVIM_GAS_INTER_RAT_RESEL_H_PRIO_CUSTOMIZE_STRU;


typedef struct
{
   VOS_UINT8                            ucRrRaFailEnableFlg;        /* ��Ч��־,0:����Ч;1:��Ч */

   VOS_UINT8                            ucRsv1;                     /* ����λ */
   VOS_UINT8                            ucRsv2;                     /* ����λ */
   VOS_UINT8                            ucRsv3;                     /* ����λ */
}NVIM_GAS_FAST_AREA_LOST_CFG_STRU;


typedef struct
{
    VOS_UINT8                            ucAutoFrInvalidMask;   /* ��Ч��־����bitֵ 0 ��ʾ���Է���, 1 ��ʾ��Ч�����Է��� */
                                                                /* bit0: CSFB ����, NAS��δ����ҵ���������������Ƿ���Է��� */
                                                                /* bit1: CSFB ����, NAS��δ����ҵ���������������Ƿ���Է��� */
                                                                /* bit2~bit8: ����λ */

    VOS_UINT8                            ucAutoFrNonCsfbFlg;       /* ��CSFB������, ҵ�����ʱ�Ƿ��������� */
    VOS_UINT8                            ucRsv2;                /* ����λ */
    VOS_UINT8                            ucRsv3;                /* ����λ */
}NVIM_GAS_AUTO_FR_CFG_STRU;


typedef struct
{
    VOS_UINT8                           ucPrioReselDisableMask; /* ���û������ȼ���������ѡ׼��ĳ���.���¸� bit ֵ1��ʾ����.0��ʾ������ */
                                                                /* ע: ����Ϊ0ʱ�Ƿ����û������ȼ����㷨����Э��� NV 9003��Լ�� */
                                                                /* bit0: LTE ģʽ��֧��, ��û��3G���ȼ������ĳ��� */
                                                                /* bit1: LTE ģʽ�� Disable, ��û��3G���ȼ������ĳ��� */
                                                                /* bit2~bit8: ����λ */

    VOS_UINT8                           ucWaitAllNCellDecodedByThreshGsmLow;    /* �����ȼ��о��У��ȴ���������������ɲŽ��вþ� */
    VOS_UINT8                           ucRsv2;                 /* ����λ */
    VOS_UINT8                           ucRsv3;                 /* ����λ */
    VOS_UINT16                          usRsv4;                 /* ����λ */
    VOS_UINT16                          usRsv5;                 /* ����λ */
}NVIM_GAS_INTER_RAT_RESEL_CFG_STRU;


typedef struct
{
    VOS_UINT16      usEnableBitmap;             /* ʶ���㷨�Ƿ�ʹ�� ��0=�ǣ�1=�� */
                                                    /* bit0: X2G REDIR�����Ƿ�ʹ��ʶ���㷨; */
                                                    /* bit1: X2G reselect�����Ƿ�ʹ��ʶ���㷨; */
                                                    /* bit2: X2G CCO�����Ƿ�ʹ��ʶ���㷨; */
                                                    /* bit3: GSM  SPEC��/GOOS�������Ƿ�ʹ��ʶ���㷨; */
                                                    /* bit4: GSM HISTORY��/GSM FAST�������Ƿ�ʹ��ʶ���㷨; */
                                                    /* bit5~bit15: Ԥ�� */
    VOS_UINT16      ucReserve1;                 /* �����ֶ� */
    VOS_UINT16      usFeatureSetEnableBitmap;   /* ʹ�ܵ�α��վ������BITMAP: bit (n)=1��ʾ��n������������Ч��; */
    VOS_UINT16      usReserve2;                 /* �����ֶ� */
    VOS_UINT16      usPseudBTSFeatureSet0;      /* ������0,ÿ��bit����һ������,��bitΪ1ʱ��ʾ���������������������:
                                                    bit0���� RX_ACCESS_MIN=0; 
                                                    bit1 ���� CCCH-CONF='001'ʱ,BS-AG-BLKS-RES=2; 
                                                    bit2 ���� ��֧��GPRS; 
                                                    bit3 ����CRO���� 60;
                                                    bit4 ����MCCΪ460;
                                                    bit5~bit15Ԥ�� */
    VOS_UINT16      usPseudBTSFeatureSet1;      /* �����ֶ�*/
    VOS_UINT16      usPseudBTSFeatureSet2;      /* �����ֶ�*/
    VOS_UINT16      usPseudBTSFeatureSet3;      /* �����ֶ�*/
    VOS_UINT16      usPseudBTSFeatureSet4;      /* �����ֶ�*/
    VOS_UINT16      usPseudBTSFeatureSet5;      /* �����ֶ�*/
    VOS_UINT16      usPseudBTSFeatureSet6;      /* �����ֶ�*/
    VOS_UINT16      usPseudBTSFeatureSet7;      /* �����ֶ�*/
} NVIM_GAS_PSEUD_BTS_IDENT_CUSTOMIZE_CFG_STRU;


typedef struct
{
    VOS_UINT32              ulHistoryFreqEnableBitmap;                          /* ��ʷƵ����������ʹ��bitλ */
    VOS_UINT32              ulOperateFreqEnableBitmap;                          /* ��Ӫ�̶���Ƶ����������ʹ��bitλ */
    VOS_UINT32              ulFullListEnableBitmap;                             /* ȫƵ����������ʹ��bitλ */
    VOS_UINT32              ulHistoryFilterSrchedFreqEnableBitmap;              /* ��ʷƵ���������̹���������Ƶ�㹦��ʹ�� bitλ */
    VOS_UINT32              ulOperateFilterSrchedFreqEnableBitmap;              /* ��Ӫ�̶���Ƶ���������̹���������Ƶ�㹦��ʹ�� bitλ */
    VOS_UINT32              ulFullListFilterSrchedFreqEnableBitmap;             /* ȫƵ��Ƶ���������̹���������Ƶ�㹦��ʹ�� bitλ */
    VOS_UINT32              ulIgnoreLowPrioJudgeEnableBitmap;                   /* �����ֵ����ȼ�����ʹ��bitλ */
    VOS_UINT32              ulCloudFreqEnableBitmap;                            /* �ƶ˶���Ƶ����������ʹ��bitλ */
    VOS_UINT32              ulHistoryFreqBandSrchEnableBitmap;                  /* ��ʷƵ������Ƶ����������ʹ��λ */
    VOS_UINT32              ulPresetFreqBandSrchEnableBitmap;                   /* Ԥ��Ƶ������Ƶ����������ʹ��λ */
    VOS_UINT32              ulCloudBandEnableBitmap;                            /* �ƶ˶���Ƶ����������ʹ��bitλ */
    VOS_UINT32              ulCloudFreqFilterSrchedFreqEnableBitmap;            /* �ƶ�Ƶ���������̹���������Ƶ�㹦��ʹ�� bitλ */
    VOS_UINT32              ulRescueCellByPresetBandEnableBitmap;               /* Ԥ��Ƶ����ȵĹ��ܿ���ʹ��bitλ */
    VOS_UINT32              ulCssShareFreqSrchEnableBitmap;                     /* CSS��Modem������ʷƵ����������ʹ��bitλ */
    VOS_UINT32              ulReserved3;                                        /* ����λ */
    VOS_UINT32              ulReserved4;                                        /* ����λ */
    VOS_UINT32              ulReserved5;                                        /* ����λ */
}NVIM_GSM_NETWORK_SEARCH_CUSTOMIZE_CFG_STRU;


typedef struct
{
    VOS_UINT16                          usFreqBegin;                            /* ��ʼƵ�� */
    VOS_UINT16                          usFreqEnd;                              /* ��ֹƵ�� */
    NVIM_BAND_IND_ENUM_UINT8            enBand;                                 /* BANDָʾ */
    VOS_UINT8                           ucRsv1;                                 /* ����λ */
    VOS_UINT8                           ucRsv2;                                 /* ����λ */
    VOS_UINT8                           ucRsv3;                                 /* ����λ */
}NVIM_GSM_OPERATE_CUSTOMIZE_FREQ_RANGE_STRU;


typedef struct
{
    VOS_UINT16                          usFreq;                                 /* Ƶ��� */
    NVIM_BAND_IND_ENUM_UINT8            enBand;                                 /* BANDָʾ */
    VOS_UINT8                           ucRsv1;                                 /* ����λ */
}NVIM_GSM_OPERATE_CUSTOMIZE_DISCRETE_STRU;


typedef struct
{
    NVIM_GAS_PLMN_ID_STRU                       stPlmn;                         /* PLMN�ṹ */
    VOS_UINT8                                   ucRangeCnt;                     /* Ƶ�㷶Χ�ĸ�����������:NVIM_GSM_OPERATE_CUSTOMIZE_FREQ_RANGE_MAX_CNT */
    VOS_UINT8                                   ucDiscreteCnt;                  /* ��ɢƵ��ĸ�����������:NVIM_GSM_OPERATE_CUSTOMIZE_DESCRETE_FREQ_MAX_CNT */
    VOS_UINT8                                   ucRsv1;                         /* ����λ */
    VOS_UINT8                                   ucRsv2;                         /* ����λ */
    NVIM_GSM_OPERATE_CUSTOMIZE_FREQ_RANGE_STRU  astGsmOperateCustomFreqRange[NVIM_GSM_OPERATE_CUSTOMIZE_FREQ_RANGE_MAX_CNT];
                                                                                /* Ƶ�㷶Χ�ṹ�б� */
    NVIM_GSM_OPERATE_CUSTOMIZE_DISCRETE_STRU    astGsmOperateCustomDiscrete[NVIM_GSM_OPERATE_CUSTOMIZE_DESCRETE_FREQ_MAX_CNT];
                                                                                /* ��ɢƵ��ṹ�б� */
    VOS_UINT8                                   ucRsv3;                         /* ����λ */
    VOS_UINT8                                   ucRsv4;                         /* ����λ */
    VOS_UINT16                                  usRsv1;                         /* ����λ */
    VOS_UINT32                                  ulRsv1;                         /* ����λ */
}NVIM_GSM_OPERATOR_CUSTOMIZE_PLMN_ITEM_STRU;


typedef struct
{
    VOS_UINT8                                   ucPlmnCnt;                      /* ��Ӫ�̶��Ƶ�PLMN�б������������:NVIM_GSM_OPERATE_CUSTOMIZE_FREQ_PLMN_MAX_CNT */
    VOS_UINT8                                   ucRsv1;                         /* ����λ */
    VOS_UINT8                                   ucRsv2;                         /* ����λ */
    VOS_UINT8                                   ucRsv3;                         /* ����λ */
    NVIM_GSM_OPERATOR_CUSTOMIZE_PLMN_ITEM_STRU  astGsmOperatorCustomPlmnItem[NVIM_GSM_OPERATE_CUSTOMIZE_FREQ_PLMN_MAX_CNT];   
                                                                                /* ���Ƶ�PLMN�б� */
}NVIM_GSM_OPERATOR_CUSTOMIZE_FREQ_CFG_STRU;


typedef struct
{
    VOS_UINT32                          ulWaitFirstUsableMask;          /* ����ָ���ȴ���һС�������� */
    VOS_UINT16                          usWaitFirstTimerLen;            /* �ȴ���һС����ʱ�����ȣ���λ:ms */

    VOS_INT16                           sSameLaiThreshold;              /* ����ѡ��ͬһ��LAIƵ���RSSI���� */
    VOS_UINT32                          ulSameLaiUsableMask;            /* ����ָ������ѡ��ͬһ��LAI������ */
    VOS_UINT16                          usSameLaiTimerLen;              /* �ȴ�ͬһ��LAIС����ʱ�����ȣ���λ:ms */
    VOS_UINT8                           ucStoreSameLaiFlag;             /* ��ʷƵ��׶��Ƿ�����ѡ��ͬһ��LAIƵ�� */
    VOS_UINT8                           ucFullSameLaiFlag;              /* ȫƵ�ν׶��Ƿ�����ѡ��ͬһ��LAIƵ�� */

    VOS_UINT32                          ulWithSiUsableMask;             /* ����ָ������ѡ���ϵͳ��Ϣ������ */
    VOS_UINT8                           ucStoreWithSiFlag;              /* ��ʷƵ��׶��Ƿ�����ѡ���ϵͳ��ϢƵ�� */
    VOS_UINT8                           ucFullWithSiFlag;               /* ȫƵ�ν׶��Ƿ�����ѡ���ϵͳ��ϢƵ�� */
    VOS_INT16                           sWithSiThreshold;               /* ����ѡ���ϵͳ��ϢƵ���RSSI���� */

    VOS_UINT8                           ucRmvUtranImpactFlg;            /* ɨƵ������Ƿ�ɾ��Utran����Ƶ�� */
    VOS_UINT8                           ucRmvPseudoNCell;               /* ɨƵ������Ƿ�ɾ����Ƶ����Ƶ�� */
    VOS_UINT8                           ucAutoAnycellCamp;              /* ָ����SUITABLEפ��ʧ�ܺ��Ƿ���������ANYCELLפ�� */
    VOS_UINT8                           ucMaxArfcnNum;                  /* �������������Ƶ����� */

    VOS_UINT8                           ucFirstTcIgnoreSi2Q;            /* ��һ��TC����ǰ�Ƿ����SI2Quater */
    VOS_UINT8                           ucRmvNoNeedCellFlg;             /* �׶������������Ƿ�ɾ���Ѿ�ȷ�����õ�С�� */
    VOS_UINT8                           ucRmvCellBetterThreshold;       /* �׶�����������ɾ������С��Ҫ���źű�õ����� */
    VOS_UINT8                           ucAnyCellSrchBaFlag;            /* Any Cell�����Ƿ���Ҫ����BA */

    VOS_UINT16                          usRsv1;                         /* ����λ */
    VOS_UINT16                          usRsv2;                         /* ����λ */
    VOS_UINT16                          usRsv3;                         /* ����λ */
    VOS_UINT16                          usRsv4;                         /* ����λ */

    VOS_INT16                           sRmvCellLastRxlevThreshold;     /* �׶�����������ɾ������С��Ҫ��Ƶ���б����źŵ����� */
    VOS_INT16                           sRsv2;                          /* ����λ */
    VOS_INT16                           sRsv3;                          /* ����λ */
    VOS_INT16                           sRsv4;                          /* ����λ */

    VOS_UINT32                          ulRsv1;                         /* ����λ */
    VOS_UINT32                          ulRsv2;                         /* ����λ */
    VOS_UINT32                          ulRsv3;                         /* ����λ */
    VOS_UINT32                          ulRsv4;                         /* ����λ */

}NVIM_GSM_PARALLEL_SEARCH_CUSTOMIZE_CFG_STRU;


typedef struct
{
    VOS_UINT8                           ucCsfbEnableFlg;                        /* L2G�ض���ʽ��CSFB�������Ƿ������ù���,0x0:����Ч,0x1:��Ч */
    VOS_UINT8                           ucRedirEnableFlg;                       /* L2G�ض���ʽ�ķ�CSFB�������Ƿ������ù���,0x0:����Ч,0x1:��Ч */
    VOS_INT8                            cSpecArfcnPreferThreshold;              /* ����ָ��Ƶ����������ޡ� */
    VOS_UINT8                           ucHistoryArfcnRssiReduceValue;          /* ��ʷƵ��RSSI��ȥucHistoryArfcnRssiReduceValue��������� */
    VOS_UINT8                           ucRsv1;
    VOS_UINT8                           ucRsv2;
    VOS_UINT8                           ucRsv3;
    VOS_UINT8                           ucRsv4;
}NVIM_GSM_ENABLE_HISTORY_ARFCN_WITH_SPEC_ARFCN_LST_STRU;


typedef struct
{
    /* ===============�ͷ�����������ص�NV������===================== */
    VOS_UINT8           ucCsAccFailPunishSwitchFlag;        /* �Ƿ���CS�������ʧ�ܳͷ����ƣ�0: �ر�, 1: ������Ĭ��ֵΪ1 */

    VOS_UINT8           ucPsAccFailPunishSwitchFlag;        /* �Ƿ���PS�������ʧ�ܳͷ����ƣ�0: �ر�, 1: ������Ĭ��ֵΪ0 */

    VOS_UINT8           ucRrConnFailOptimizeSwtichFlag;     /* �Ƿ�������ʧ���Ż����ƿ��أ�
                                                               0: �ر�, 1: ������Ĭ��ֵΪ1
                                                               �򿪸�NV���
                                                               1.
                                                               �������ʧ��һ�κ��С��ΪFORBIND���ȼ�С����
                                                               �������ʧ�ܳ���һ�����������ͷ����ƣ��ͷ������ڽ���Ϊ��С����BAR���ͷ�ʱ�䳬ʱ����Ϊ�ǵ����ȼ�С����

                                                               2.
                                                               N200ʧ�ܴ����ﵽһ������(NV������)��ִ�б�����ѡ��
                                                               N200ʧ�ܴﵽһ������(NV������), �����ͷ����ƣ��ͷ������ڽ���Ϊ��С����BAR���ͷ�ʱ�䳬ʱ����Ϊ�ǵ����ȼ�С����

                                                               3.
                                                               �����������ɹ��������С������Ϊ�������ʧ�ܼ����б����б���ɾ����С����
                                                               ���²�������ɹ��������С������ΪN200ʧ�ܼ����б����б���ɾ����С����                                                               

                                                               �رո�NV���FORBIND���ȼ�û�е����ͷ����ԣ�Ҳû��N200�ͷ�����
                                                               �������ʧ��һ�κ��С��ΪFORBIND���ȼ�С��������3�β���פ����
                                                               */

    /* ===============�������ʧ����ص�NV������===================== */
    VOS_UINT8           ucStartPunishRaFailTimesThreshold;     /* �ڸ�GSMС�����������ʧ�ܴ�������, �ﵽ�ô����Ը�С�������ͷ����� */

    VOS_UINT8           ucRssiOffset;                         /* ��ʷ�ź�ǿ�ȵ������ޣ�С���ź�ǿ���������޴ﵽ�����޽���С����Bar�б���ɾ�� */


    /* ===============N200��ص�NV������============================== */
    VOS_UINT8           ucN200FailPunishSwitchFlag;             /* �Ƿ���N200ʧ�ܳͷ����ƣ�0: �ر�, 1: ������Ĭ��ֵΪ1 */

    VOS_UINT8           ucStartReselN200FailTimesThreshold;     /* �ڸ�GSMС����N200ʧ�ܴ�������, �ﵽ�ô���������������ѡ */

    VOS_UINT8           ucStartPunishN200FailTimesThreshold;    /* �ڸ�GSMС����N200ʧ�ܴ�������, �ﵽ�ô����Ը�С�������ͷ����� */
    
    VOS_INT16           sN200FailRssiThreshold;                 /* �ڸ�GSMС����N200ʧ��RSSI���ޣ���λ:dBm  */

    /* ===============�ͷ�ʱ����ص�NV������============================== */
    VOS_UINT16          usInitialForbidTimeLen;                /* ����ʧ�ܵ�һ�εĳͷ�ʱ�䣬��λ:�� */

    VOS_UINT16          usMaxForbidTimeLen;                    /* ����ʧ�ܵ�С�������ͷ�ʱ������λ:�� */

    VOS_INT16           sRssiHighThresh;                      /* ��λ:dBm, ����ͷ��б��С������ź�ǿ�ȳ�����ֵ����С���ӳͷ��б����Ƴ����������ź�ǿ�ȱ仯 */

    VOS_UINT32          ulForbidCellMaxReServeTimeLen;        /* ForbidС���ڳͷ��б��е������ʱ������λ:�� */

    VOS_UINT32          ulRssiThresholdSet;                    /* �������ź�ǿ�ȵ������޼��ϣ������ź�ǿ�ȵĵ�λ:dBm��
                                                                  �ӵ͵��ߣ���һ���ֽڱ�ʾ���ź�ǿ�ȵ�����,Ĭ��ֵΪ-85dBm�������ź���ָ�ź�ǿ�ȴ���-85dBm��������-85dBm;
                                                                  �ڶ����ֽڱ�ʾ�е��ź�ǿ�ȵ�����,Ĭ��ֵΪ-95dBm, �е��ź���ָ�ź�ǿ�Ƚ���-85dBm��-95dBm, ����-85dBm��-95dBm;*/

    VOS_UINT32          ulRssiOffsetSet;                       /* �������ź�ǿ���������ʧ�ܼ�¼�ϻ���ƫ��ֵ��
                                                                  �ӵ͵��ߣ���һ���ֽڱ�ʾ���ź�ǿ���������ʧ�ܼ�¼�ϻ���ƫ��ֵ,Ĭ��ֵΪ20;
                                                                  �ڶ����ֽڱ�ʾ�е��ź�ǿ���������ʧ�ܼ�¼�ϻ���ƫ��ֵ,Ĭ��ֵΪ8;
                                                                  �������ֽڱ�ʾ���ź�ǿ���������ʧ�ܼ�¼�ϻ���ƫ��ֵ,Ĭ��ֵΪ16;
                                                                  */
    VOS_UINT32          ulRsv6;                              /* ����λ */
    VOS_UINT32          ulRsv7;                              /* ����λ */

}NVIM_GAS_RR_CONNECT_FAIL_PUNISH_CFG_STRU;


/*****************************************************************************
 �ṹ��    : NVIM_GSM_PING_PONG_HO_CUSTOMIZE_CFG_STRU
 �ṹ˵��  : en_NV_Item_GSM_PING_PONG_HO_CUSTOMIZE_CFG �ṹ,GSM ƹ���л����������������
 DESCRIPTION: GSM ƹ���л����������������
*****************************************************************************/
typedef struct
{
    VOS_UINT8                           ucSwitchFlag;                   /* ���Կ��� */
    VOS_UINT8                           ucHoTimeThreshold;              /* ͳ�Ʒ�Χ�ڵ�С��ƽ��פ��ʱ������, �����Ƽ�30s */
    VOS_UINT8                           ucBadCellTimeScale;             /* ��С��ƽ��ʱ����������, ��λ: 0.1, �����Ƽ� 20 */
    VOS_UINT8                           ucGoodQualityThreshold;         /* �ź��������ж�����.������. ��λ: ǧ��֮һ, �����Ƽ�ֵΪ5 */
    VOS_UINT8                           ucGoodQualityCntThreshold;      /* �ź������õ�ͳ�ƴ�������, �����Ƽ�ֵ 3 */
    VOS_UINT8                           ucNCellRptReduceValue;          /* �Ż���Чʱ,�����ϱ�������, ��λ dB, �����Ƽ�ֵΪ20 */
    VOS_UINT8                           ucWatchHoTimes;                 /* �л�����, �����Ƽ�ֵΪ 4 */
    VOS_UINT8                           aucRsv1;                        /* ����λ */
    VOS_UINT16                          ausRsv1;
    VOS_UINT16                          ausRsv2;
    VOS_UINT8                           aucRsv3;
    VOS_UINT8                           aucRsv4;
    VOS_UINT8                           aucRsv5;
    VOS_UINT8                           aucRsv6;
}NVIM_GSM_PING_PONG_HO_CUSTOMIZE_CFG_STRU;


typedef struct
{
    VOS_UINT8                           ucDistLteAcsTypeEnable;         /* ʹ������LTE��ʽ���в����Ŀ��� */
    VOS_UINT8                           aucRsv1;                        /* ����λ */
    VOS_UINT8                           aucRsv2;
    VOS_UINT8                           aucRsv3;
    VOS_UINT16                          ausRsv1;
    VOS_UINT16                          ausRsv2;
}NVIM_GSM_LTE_MEASURE_CFG_STRU;

/*****************************************************************************
 �ṹ��    : NVIM_GSM_SEC_RXQUAL_SUB_ALPHA_FILTERING_CFG_STRU
 �ṹ˵��  : en_NV_Item_GSM_SEC_RXQUAL_SUB_ALPHA_FILTERING_CFG �ṹ,��GSM SUB���������ʽ���ALPHA�˲�, ���жϷ���С���Ƿ���Լ���פ�����������
 DESCRIPTION: ��GSM SUB���������ʽ���ALPHA�˲�, ���жϷ���С���Ƿ���Լ���פ�����������
*****************************************************************************/
typedef struct
{
    VOS_UINT16                          usSecRrPoorRxQualSubThresHold;  /* �ź������������ж�����, ����:ǧ��֮һ, ��ЧֵΪ0~1000, �Ƽ�ֵ: 80, ��������Ϊ8% */
    VOS_UINT16                          usSecRxQualSubAlpha;            /* Alpha����, ����:ǧ��֮һ, ��ЧֵΪ0~1000, �Ƽ�ֵ: 250, ��AlphaΪ0.25 */
    VOS_UINT16                          usSecConSubBerCntThresHold;     /* ����usSecConSubBerCntThresHold�������������ֵ, �ٿ���Poor����, �Ƽ�ֵ: 2, ���������� */
    VOS_UINT8                           ucRsv1;                         /* ����λ */
    VOS_UINT8                           ucRsv2;
    VOS_UINT16                          usRsv1;
    VOS_UINT16                          usRsv2;
}NVIM_GSM_SEC_RXQUAL_SUB_ALPHA_FILTERING_CFG_STRU;

#if defined( __PS_WIN32_RECUR__ ) || defined (DMT)

typedef struct  
{
    VOS_UINT32  ulPermitedChrAlarmIdCount;

    VOS_UINT16  aucAlarmIds[MAX_CHR_ALARM_ID_NUM];
    
}NVIM_GAS_CHR_PC_CFG_STRU;
#endif

/*****************************************************************************
 �ṹ��    : NVIM_Item_GAS_SAME_LAI_PREFER_CFG_STRU
 �ṹ˵��  : en_NV_Item_GAS_SAME_LAI_PREFER_CFG �ṹ, ��������������ѡ����ͬ LAI С��
             ���������.
 DESCRIPTION: ��������������ѡ����ͬ LAI С�����������.
*****************************************************************************/
typedef struct
{
    VOS_UINT8                           ucMtSwitchFlag;                 /* ���Կ��� */
    VOS_UINT8                           ucMoSwitchFlag;                 /* ���Կ��� */

    VOS_INT8                            cFirstLevelAddValue;           /* ��һ�����ȼ�����ͬ LAI С������ʱ����������ֵ */
    VOS_INT8                            cFirstLevelThreshold;          /* ��һ�����ȼ����� */

    VOS_INT8                            cSecLevelAddValue;             /* �ڶ������ȼ�����ͬ LAI С������ʱ����������ֵ */
    VOS_INT8                            cSecLevelThreshold;            /* �ڶ������ȼ����� */
    VOS_UINT16                          usValidTimeLen;                 /* ��Чʱ��, ��λ:���� */
    VOS_UINT8                           aucRsv1;                        /* ����λ */
    VOS_UINT8                           aucRsv2;                        /* ����λ */
    VOS_UINT8                           aucRsv3;                        /* ����λ */
    VOS_UINT8                           aucRsv4;                        /* ����λ */
}NVIM_Item_GAS_SAME_LAI_PREFER_CFG_STRU;

/*****************************************************************************
 �ṹ��    : NVIM_GSM_HO_CUSTOMIZE_CFG_STRU
 �ṹ˵��  : en_NV_Item_GSM_HO_CUSTOMIZE_CFG �ṹ,GSM �л����������������
 DESCRIPTION: GSM �л����������������
*****************************************************************************/
typedef struct
{
    VOS_UINT8                           ucChanModeRptIndication;        /* �����ŵ��л��л�����,�и� MM �ϱ� GAS_RR_CHAN_IND ��ʱ�� */
    VOS_UINT8                           aucRsv1;
    VOS_UINT16                          ausRsv1;
    VOS_UINT8                           aucRsv2;
    VOS_UINT8                           aucRsv3;
    VOS_UINT8                           aucRsv4;
    VOS_UINT8                           aucRsv5;
}NVIM_GSM_HO_CUSTOMIZE_CFG_STRU;

/*****************************************************************************
 �ṹ��    : NVIM_GSM_PMR_CFG_STRU
 �ṹ˵��  : en_NV_Item_GSM_PMR_CFG �ṹ,GSM PMR��ض���
 DESCRIPTION: GSM PMR��ض���
*****************************************************************************/
typedef struct
{
    VOS_UINT8                           ucNcReportPeriodIMin;           /* ����̬ PMR ������Сֵ */
    VOS_UINT8                           aucRsv1;
    VOS_UINT16                          ausRsv1;
    VOS_UINT8                           aucRsv2;
    VOS_UINT8                           aucRsv3;
    VOS_UINT8                           aucRsv4;
    VOS_UINT8                           aucRsv5;
}NVIM_GSM_PMR_CFG_STRU;

/*****************************************************************************
 �ṹ��    : NVIM_GSM_GCBS_MESSAGE_CUSTOMIZE_CFG_STRU
 �ṹ˵��  : en_NV_Item_GCbs_Message_Customize_CFG �ṹ, GCBS Message��ض���
 DESCRIPTION: GCBS Message��ض���
*****************************************************************************/
typedef struct
{
    VOS_UINT8                           ucIsFixedPageDataLengthFlag;    /* ���Կ���,���ڿ����Ƿ�̶�88�ֽ�Page�ϱ�TAF.���ΪVOS_TRUE,���ǰ��̶�88�ֽ��ϱ�;ΪVOS_FALSE,�����(��ЧBLOCK��x22�ֽ�)�ϱ�;Ĭ��ΪVOS_TRUE */
    VOS_UINT8                           ucIsOptionalMessageReadFlag;    /* ���Կ���,���ڿ����Ƿ��Optional reading CB Message.���ΪVOS_TRUE,���;ΪVOS_FALSE,�򲻶�;Ĭ��ΪVOS_TRUE */
    VOS_UINT8                           ucIsAdvisedMessageReadFlag;     /* ���Կ���,���ڿ����Ƿ��Reading advised CB Message.���ΪVOS_TRUE,���;ΪVOS_FALSE,�򲻶�;Ĭ��ΪVOS_TRUE */
    VOS_UINT8                           ucRsv1;                         /* ����λ */
    VOS_UINT8                           ucRsv2;
    VOS_UINT8                           ucRsv3;
    VOS_UINT16                          usRsv1;
    VOS_UINT16                          usRsv2;
    VOS_UINT16                          usRsv3;
}NVIM_GSM_GCBS_MESSAGE_CUSTOMIZE_CFG_STRU;

/*****************************************************************************
 �ṹ��    : NVIM_GAS_GSM_ACTIVE_CELL_RESELECT_CFG_STRU
 �ṹ˵��  : en_NV_Item_GAS_GSM_CELL_RESELECT_CFG �ṹ, GAS ��G2G��ѡ�����������
 DESCRIPTION: GAS ��G2G��ѡ�����������
*****************************************************************************/
typedef struct
{
    VOS_UINT8                           ucC2ReselPartialReadFlg;        /* ������ѡ�Ƿ���Ҫ���ֶ�
                                                                           1: ��Ҫ;
                                                                           0: ����Ҫ */
    VOS_UINT8                           ucRsv1;
    VOS_UINT16                          usHistorySi3ValidTimeLen;       /* פ���׶���ʷSI3��Чʱ��. ��λ: s */
    VOS_UINT8                           ucRsv2;
    VOS_UINT8                           ucRsv3;
    VOS_UINT8                           ucRsv4;
    VOS_UINT8                           ucRsv5;

    VOS_UINT8                           ucC2ReselPunishSwitchFlg;       /* ������ѡ�����ͷ������Ƿ���
                                                                           1: ����;
                                                                           0: �ر� */
    VOS_UINT8                           ucRsv6;
    VOS_INT16                           sScellRssiThreshold;            /* ����С������������ֵ����λ: dBm */
    VOS_UINT16                          usScellPchBerThreshold;         /* ����С��PCH������Alpha�˲���ֵ����λ: %��ǧ��֮һ���� */
    VOS_UINT16                          usScellPchBerAlpha;             /* ����С��PCH������Alpha�˲���Alpha���ӣ���λ: %��ǧ��֮һ���� */
    VOS_UINT16                          usForbiddenCellPunishTimeLen;   /* Forbidden С����ѡ�ͷ�ʱ����������ʱ������С���������λ: s */
    VOS_UINT16                          usForbiddenCellPunishStep;      /* ������С��Ϊ��С��ʱ�����������гͷ�����ֵ����λ: dBm */
    VOS_UINT8                           ucScellRaSuccStep;              /* ����С��RA�ɹ�һ�β���ֵ */
    VOS_UINT8                           ucScellRaFailStep;              /* ����С��RAʧ��һ�β���ֵ */
    VOS_UINT8                           ucRsv9;
    VOS_UINT8                           ucRsv10;  
    VOS_UINT16                          usRsv1;
    VOS_UINT16                          usRsv2;
}NVIM_GAS_GSM_ACTIVE_CELL_RESELECT_CFG_STRU;

/*****************************************************************************
 �ṹ��    : NVIM_GAS_GSM_PASSIVE_RESELECT_OPTIMIZE_CFG_STRU
 �ṹ˵��  : en_NV_Item_GAS_GSM_PASSIVE_RESELECT_OPTIMIZE_CFG �ṹ������ָ�ɱ����Ż���T3166/T3168��ʱ�Ż�
 DESCRIPTION: ����ָ�ɱ����Ż���T3166/T3168��ʱ�Ż�����
*****************************************************************************/
typedef struct
{
    VOS_UINT8                           ucImmAssRejOptSwitchFlg;            /* ���Կ��� */
    VOS_UINT8                           ucImmAssRejCntThreshold;            /* �������ָ�ɱ��ܴ��� */
    VOS_UINT8                           ucImmAssRejWaitIndThreshold;        /* �������ָ�ɱ���ʱ������λ: s */

    VOS_UINT8                           ucCcAbnormalRelCnt;                 /* ����ʧ�ܳ���������ʱ���𱻶���ѡ, 0xFF ��ʾ�رն�Ӧ���� */

    VOS_UINT8                           ucT3166T3168ExpiredOptSwitchFlg;    /* ���Կ��� */
    VOS_UINT8                           ucT3166T3168ExpiredCntThreshold;    /* T3166/T3168��ʱ�������ʱ���� */
    VOS_UINT16                          usT3166T3168ExpiredPunishTimeLen;   /* T3166/T3168��ʱ�ͷ���ʱ��ʱ������λ: ms */

    VOS_UINT8                           ucRsv1;                             /* ����λ */
    VOS_UINT8                           ucRsv2;
    VOS_UINT8                           ucRsv3;
    VOS_UINT8                           ucRsv4;
    VOS_UINT8                           ucRsv5;
    VOS_UINT8                           ucRsv6;
    VOS_UINT8                           ucRsv7;
    VOS_UINT8                           ucRsv8;

    VOS_UINT16                          usRsv1;
    VOS_UINT16                          usRsv2;
}NVIM_GAS_GSM_PASSIVE_RESELECT_OPTIMIZE_CFG_STRU;

/*****************************************************************************
 �ṹ��    : NVIM_GAS_GSM_SACCH_BA_INHERIT_OPTIMIZE_CFG_STRU
 �ṹ˵��  : en_NV_Item_GAS_GSM_SACCH_BA_INHERIT_OPTIMIZE_CFG �ṹ, SACCH Ba �̳��Ż����ýṹ
 DESCRIPTION: SACCH Ba �̳��Ż����ýṹ
*****************************************************************************/
typedef struct
{
    VOS_UINT8                           ucInheritBcchFlg;
    VOS_UINT8                           ucInheritLastCellFlg;

    VOS_UINT8                           ucSi5terNotExistJugeTimes;

    VOS_UINT8                           ucRsv1;                             /* ����λ */
    VOS_UINT8                           ucRsv2;
    VOS_UINT8                           ucRsv3;
    VOS_UINT8                           ucRsv4;
    VOS_UINT8                           ucRsv5;
}NVIM_GAS_GSM_SACCH_BA_INHERIT_OPTIMIZE_CFG_STRU;

/*****************************************************************************
 �ṹ��    : NVIM_GAS_GSM_PAGE_RCV_CFG_STRU
 �ṹ˵��  : en_NV_Item_GAS_GSM_PAGE_RCV_CFG �ṹ, Ѱ��������ز�������
 DESCRIPTION: Ѱ��������ز�������
*****************************************************************************/
typedef struct
{
    VOS_UINT8                           ucRcvPchInPsFlg;            /* �����ܿ���. 0:�ر�; 1:�� */
    VOS_UINT8                           ucRcvPchInPsSpecialFlg;     /* ���������µĿ���. 0:�ر�; 1:��
                                                                       bit0: ����ģʽI��С����. �Ƿ������
                                                                       bit1: ֧�� BSS ��С����. �Ƿ������
                                                                       bit2: RAU ������. �Ƿ������
                                                                       bit3: Attach ������. �Ƿ������
                                                                       bit4: PDP �������. �Ƿ������ */

    VOS_UINT8                           ucRsv1;                             /* ����λ */
    VOS_UINT8                           ucRsv2;
    VOS_UINT8                           ucRsv3;
    VOS_UINT8                           ucRsv4;
    VOS_UINT8                           ucRsv5;
    VOS_UINT8                           ucRsv6;
}NVIM_GAS_GSM_PAGE_RCV_CFG_STRU;

/*****************************************************************************
 �ṹ��    : NVIM_GAS_NET_SRCH_RMV_INTER_RAT_FREQ_CFG_STRU
 �ṹ˵��  : en_NV_Item_GAS_Net_Srch_Rmv_Inter_Rat_Freq_Cfg �ṹ, ��������ϵͳ�ص�Ƶ�ʲ������� 
 DESCRIPTION: ��������ϵͳ�ص�Ƶ�ʲ������� 
*****************************************************************************/
typedef struct
{
    VOS_UINT8                           ucPreferBandSrchRmvLteFreqSwitch;       /* PREFER Band�������̿۳�LTEƵ�����Χ�ڵ�GģƵ�����Կ��أ�0: �ر�, 1: ������Ĭ��ֵΪ1 */

    VOS_UINT8                           ucPreferBandSrchRmvWcdmaFreqSwitch;     /* PREFER Band�������̿۳�WCDMAƵ�����Χ�ڵ�GģƵ�����Կ��أ�0: �ر�, 1: ������Ĭ��ֵΪ1 */

    VOS_UINT8                           ucPreferBandSrchRmvCdmaFreqSwitch;      /* PREFER Band�������̿۳�CDMAƵ��Ĵ���Χ�ڵ�GģƵ�����Կ��أ�0: �ر�, 1: ������Ĭ��ֵΪ1 */

    VOS_UINT8                           ucRsv1;                                 /* ����λ */

    VOS_UINT8                           ucRsv2;

    VOS_UINT8                           ucRsv3;

    VOS_UINT16                          usRsv1;

    VOS_UINT16                          usRsv2;

    VOS_UINT16                          usRsv3;

    VOS_UINT32                          ulRsv1;

    VOS_UINT32                          ulRsv2;
}NVIM_GAS_NET_SRCH_RMV_INTER_RAT_FREQ_CFG_STRU;


typedef struct
{
    VOS_INT16                   sHighThresh;            /* �ϱ����������ź����ޣ���λdbm */
    VOS_INT16                   sLowThresh;             /* �ϱ����������ź����ޣ���λdbm */
    VOS_UINT16                  usOffset;               /* ���Ͳ�������λdbm */
    VOS_UINT16                  usRsv;                  /* ����λ */
    VOS_UINT32                  ulTEvaluation;          /* ����ʱ������λms */
    VOS_UINT32                  ulRsv;                  /* ����λ */
}NVIM_GUAS_CELLULAR_PREFER_REPORT_PARA_CFG_STRU;


typedef struct
{
    NVIM_GUAS_CELLULAR_PREFER_REPORT_PARA_CFG_STRU     stGsmCellPreferCfg;      /* gģ������� */
    NVIM_GUAS_CELLULAR_PREFER_REPORT_PARA_CFG_STRU     stWcdmaCellPreferCfg;    /* wģ������� */
}NVIM_GUAS_CELLULAR_PREFER_PARA_CFG_STRU;


typedef struct
{
    VOS_UINT8                       ucEnableFlag;                               /* ����ʹ�ܿ��� */
    VOS_UINT8                       ucReserved1;                                /* ����λ */
    VOS_UINT8                       ucReserved2;                                /* ����λ */
    VOS_UINT8                       ucReserved3;                                /* ����λ */
    VOS_INT16                       sGsmRxlevThresh;                            /* GSM scell Rxlev Great���ޣ���λdb */
    VOS_INT16                       sUtranFddRscpThreshOnRanking;               /* ��GSM Great�źų�����RANKING��ѡ�����У�WCDMA RSCP�������ޣ���λdb */
    VOS_INT16                       sUtranFddEcNoThreshOnRanking;               /* ��GSM Great�źų�����RANKING��ѡ�����У�WCDMA EcNo�������ޣ���λdb */
    VOS_INT16                       sUtranFddRscpThreshOnPrio;                  /* ��GSM Great�źų�����Prio��ѡ�����У�WCDMA RSCP�������ޣ���λdb */
    VOS_INT16                       sUtranFddEcNoThreshOnPrio;                  /* ��GSM Great�źų�����Prio��ѡ�����У�WCDMA EcNo�������ޣ���λdb */
    VOS_UINT16                      usReserved1;                                /* ����λ */
    VOS_UINT16                      usReserved2;                                /* ����λ */
    VOS_UINT16                      usReserved3;                                /* ����λ */
    VOS_UINT16                      usReserved4;                                /* ����λ */
    VOS_UINT16                      usReserved5;                                /* ����λ */
    VOS_UINT16                      usReserved6;                                /* ����λ */
    VOS_UINT16                      usReserved7;                                /* ����λ */
}NVIM_GSM_IRAT_ACTIVE_RESEL_CUSTOM_CFG_STRU;




typedef struct
{
    VOS_UINT8                   ucCsfbMoPenaltyFlg;             /* �Ƿ��CSFB MO�������гͷ�,0: ���ͷ�, 1: �ͷ���Ĭ��ֵΪ0 */                        
    VOS_UINT8                   ucCsfbMtPenaltyFlg;             /* �Ƿ��CSFB MT�������гͷ�,0: ���ͷ�, 1: �ͷ���Ĭ��ֵΪ0 */
    VOS_UINT16                  usCsfbPenaltyTime;              /* �ͷ�ʱ��,��λ:���ӣ�Ĭ��ֵΪ2160���� */
    VOS_UINT8                   ucCsfbFailCountForPenalty;      /* ���гͷ���ҪCSFBʧ�ܵĴ���:Ĭ��3�� */     
    VOS_UINT8                   ucCsfbDifLaiFailCount;          /* CSFB����ͬLAI�Ż���ҪCSFB��LAIʧ�ܵĴ���:Ĭ��3�� */      
    VOS_INT16                   sCsfbSameLaiThreshold;          /* CSFB����ѡ��ͬһ��LAIƵ���RSSI���ޣ�Ĭ��ֵΪ-90dbm */
    VOS_UINT16                  usCsfbSameLaiTimerLen;          /* CSFB�ȴ�ͬһ��LAIС����ʱ�����ȣ���λ:ms��Ĭ��ֵΪ5000ms */    
    VOS_UINT16                  usRsv1;                         /* ����λ */
    VOS_UINT16                  usRsv2;                         /* ����λ */
    VOS_UINT16                  usRsv3;                         /* ����λ */
}NVIM_GAS_CSFB_FAIL_LAI_PENALTY_CFG_STRU;


typedef struct
{
    VOS_UINT8                   ucAllocUtranPriFlg;             /* �����ܿ���. 0:�ر�; 1:�� */    
    VOS_UINT8                   ucDefaultThreshUtran;           /* �������û��������ѡ��صĲ�������NV���� DEFAULT_THRESH_UTRAN Ĭ��ֵ */   
    VOS_UINT8                   ucDefaultUtranQRxlMin;          /* �������û��������ѡ��صĲ�������NV���� DEFAULT_UTRAN_QRXLEVMIN Ĭ��ֵ */   
    VOS_UINT8                   ucSetUtranPriMinValueFlg;       /* �����Ż���Ĭ��3G���ȼ��̶�Ϊ������ȼ������趨Ϊ1���ȼ���0: ���̶�Ϊ1���ȼ�; 1: �̶�Ϊ1���ȼ� */
    VOS_UINT8                   ucRsv1;                         /* ����λ */
    VOS_UINT8                   ucRsv2;                         /* ����λ */
    VOS_UINT16                  usRsv3;                         /* ����λ */
    VOS_UINT16                  usRsv4;                         /* ����λ */
    VOS_UINT16                  usRsv5;                         /* ����λ */
    VOS_UINT32                  ulRsv6;                         /* ����λ */
}NVIM_GAS_NETWORK_NOT_ALLOC_UTRAN_PRIORITY_CFG_STRU;

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
