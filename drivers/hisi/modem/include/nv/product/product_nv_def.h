/******************************************************************************

    Copyright(C)2008,Hisilicon Co. LTD.

 ******************************************************************************
  File Name       :   product_nv_def.h
  Description     :   ����NV �ṹ�嶨��
  History         :
******************************************************************************/

#ifndef __PRODUCT_NV_DEF_H__
#define __PRODUCT_NV_DEF_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define MAX_SINGLE_GUC_BAND_CHECK_NUM       28
#define MAX_SINGLE_TL_BAND_CHECK_NUM        12
#define MAX_NV_GUC_CHECK_ITEM_NUM           32
#define MAX_NV_TL_CHECK_ITEM_NUM            64

#ifndef __u8_defined
#define __u8_defined
typedef signed char          s8;
typedef unsigned char        u8;
typedef signed short         s16;
typedef unsigned short       u16;
typedef signed int           s32;
typedef unsigned int         u32;
typedef signed long long     s64;
typedef unsigned long long   u64;
#endif

/*��ο��ṹ������*/
typedef struct
{
    int reserved;  /*note */
}PRODUCT_MODULE_STRU;

typedef struct
{
    u32 uwGucCrcResult;                     		/* GUc����NV��У���� */
    u32 uwTlCrcResult;                      		/* TL����NV��У���� */
    u32 uwGucM2CrcResult;                   		/* GUC����NV��У���� */
}NV_CRC_CHECK_RESULT_STRU;

typedef struct
{
    u16 uhwTransmitMode;                       		/* ָʾ����ķ���ģʽ */
    u16 uhwBand;                              		/* ָʾ�����BAND�� */
    u16 uhwEnable;                              	/* ָʾ�Ƿ���Ч */
    u16 uhwValidCount;                              	/* ָʾ��Ч���ݵĸ��� */
    u16 auhwNeedCheckID[MAX_SINGLE_GUC_BAND_CHECK_NUM]; /* ������Ҫ����NV_IDֵ */
}SINGLE_GUC_BAND_NV_ID_STRU;

typedef struct
{
    u16 uhwTransmitMode;                       		/* ָʾ����ķ���ģʽ */
    u16 uhwBand;                              		/* ָʾ�����BAND�� */
    u16 uhwEnable;                              	/* ָʾ�Ƿ���Ч */
    u16 uhwValidCount;                              	/* ָʾ��Ч���ݵĸ��� */
    u16 auhwNeedCheckID[MAX_SINGLE_TL_BAND_CHECK_NUM]; 	/* ������Ҫ����NV_IDֵ */
}SINGLE_TL_BAND_NV_ID_STRU;

typedef struct
{
    SINGLE_GUC_BAND_NV_ID_STRU astNVIDCheckItem[MAX_NV_GUC_CHECK_ITEM_NUM];  	/* ������Ҫ����NV_IDֵ */
}NV_GUC_CHECK_ITEM_STRU;

typedef struct
{
    SINGLE_TL_BAND_NV_ID_STRU astNVIDCheckItem[MAX_NV_TL_CHECK_ITEM_NUM];   	/* ������Ҫ����NV_IDֵ */
}NV_TL_CHECK_ITEM_STRU;

/*****************************************************************************
 �ṹ��    : CHG_BATTERY_LOW_TEMP_PROTECT_NV
 �ṹ˵��  : CHG_BATTERY_LOW_TEMP_PROTECT_NV�ṹ ID=52005
*****************************************************************************/
typedef struct
{
    u32  ulIsEnable;
    s32  lCloseAdcThreshold;
    u32  ulTempLowCount;
}CHG_BATTERY_LOW_TEMP_PROTECT_NV;


typedef struct
{
    u32                          ulIsEnable;
    s32                           lCloseADCHold;
    u32                          ulTempOverMax;
}OM_BATTREY_TEMP_CFG_STRU;

/*****************************************************************************
 �ṹ��    : nvi_cust_pid_type
 �ṹ˵��  : nvi_cust_pid_type�ṹ
*****************************************************************************/
typedef struct
{
    u32  nv_status;
    u16  cust_first_pid;
    u16  cust_rewind_pid;
}nvi_cust_pid_type;

/*****************************************************************************
 �ṹ��    : NV_WEBNAS_SD_WORKMODE_STRU
 �ṹ˵��  : NV_WEBNAS_SD_WORKMODE�ṹ
*****************************************************************************/
typedef struct
{
    u32 ulSDWorkMode;
}NV_WEBNAS_SD_WORKMODE_STRU;

/*****************************************************************************
 �ṹ��    : NV_OLED_TEMP_ADC_STRU
 �ṹ˵��  : NV_OLED_TEMP_ADC�ṹ
*****************************************************************************/
typedef struct
{
    s16       sTemp;
    s16       sADC;
}NV_OLED_TEMP_ADC_STRU;

/*****************************************************************************
 �ṹ��    : NV_OLED_TEMP_ADC_STRU_ARRAY
 �ṹ˵��  : NV_OLED_TEMP_ADC_STRU_ARRAY�ṹ  ID=49
*****************************************************************************/
typedef struct
{
    NV_OLED_TEMP_ADC_STRU          stNV_OLED_TEMP_ADC[28];
}NV_OLED_TEMP_ADC_STRU_ARRAY;

/*****************************************************************************
 �ṹ��    : TEMP_ADC_STRU
 �ṹ˵��  : TEMP_ADC�ṹ
*****************************************************************************/
typedef struct
{
    s16    sTemp;
    u16    usADC;
}TEMP_ADC_STRU;

/*****************************************************************************
 �ṹ��    : TEMP_ADC_STRU_ARRAY
 �ṹ˵��  : TEMP_ADC_STRU_ARRAY�ṹ
*****************************************************************************/
typedef struct
{
    TEMP_ADC_STRU    stTempAdc[28];
}TEMP_ADC_STRU_ARRAY;

/*****************************************************************************
 �ṹ��    : USB_SN_NV_INFO_STRU
 �ṹ˵��  : USB_SN_NV_INFO�ṹ
*****************************************************************************/
typedef struct
{
    u32    usbSnNvStatus;
    u32    usbSnNvSucFlag;
    u32    usbSnReserved1;
    /*u32    usbSnReserved2;*/
}USB_SN_NV_INFO_STRU;

/*****************************************************************************
 �ṹ��    : LIVE_TIME_STRU
 �ṹ˵��  : LIVE_TIME�ṹ
*****************************************************************************/
typedef struct
{
    u32    ulLiveTime;
}LIVE_TIME_STRU;

/*****************************************************************************
 �ṹ��    : LIVE_TIME_CONTROL_STRU
 �ṹ˵��  : LIVE_TIME_CONTROL�ṹ
*****************************************************************************/
typedef struct
{
    u32    ulEnable;
    u32    ulCycle;
} LIVE_TIME_CONTROL_STRU;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif


