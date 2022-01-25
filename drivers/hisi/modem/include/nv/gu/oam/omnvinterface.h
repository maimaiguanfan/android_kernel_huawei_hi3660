


/*****************************************************************************
  1 ����ͷ�ļ�����
*****************************************************************************/
#include "vos.h"

#ifndef __OM_NV_INTERFACE_H__
#define __OM_NV_INTERFACE_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#if (VOS_OS_VER != VOS_WIN32)
#pragma pack(4)
#else
#pragma pack(push, 4)
#endif
/*****************************************************************************
  2 �궨��
*****************************************************************************/
/* ���Կ�������Ϣ */
#define USIMM_TEST_CARD_CFG_NUM                     (8)
#define USIMM_TEST_CARD_PLMN_LEN                    (3)
#define USIMM_CDMATEST_CARD_PLMN_LEN                (2)

/*�ϻ�������ĸ���*/
#define OM_AGING_TEST_NUM                           (20)

#define SYS_ZSP_LDF_NUM                             (64)

#define SYS_HIFI_LDF_NUM                            (64)


#define STK_TERMINAL_ND                             (0x1)
#define STK_TERMINAL_NK                             (0x2)
#define STK_TERMINAL_NA                             (0x4)
#define STK_TERMINAL_NS                             (0x8)
#define STK_TERMINAL_NL                             (0x10)

#define STK_SIM_PROFILE_DEFAULT                     {0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0x0F, 0x00, 0xDF, 0x7F, 0x03,\
                                                     0x00, 0x1F, 0xE2, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00}

#define STK_USIM_PROFILE_DEFAULT                    {0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0x00, 0xDF, 0xFF, 0x03,\
                                                     0x00, 0x1F, 0xE2, 0x00, 0x00, 0x00, 0xC3, 0xCB, 0x00, 0x00,\
                                                     0x00, 0x01, 0x00, 0x00, 0x91, 0x00, 0x00, 0x00, 0x00, 0x08}

#define VSIM_DH_PUBIIC_KEY                          (128)
#define VSIM_DH_PRIVATE_KEY                         (128)
#define VSIM_DH_AGREE_KEY                           (256)
#define VSIM_FILE_WRITE_COUNT                       (1)
#define VSIM_FILE_READ_COUNT                        (1)

#define VSIM_KEYLEN_MAX                             (128)                   /* ��Կ���Ȳ�����128�ֽ� */

#define VSIM_EF_LOCI_LEN                            (11)
#define VSIM_EF_PS_LOCI_LEN                         (14)
#define VSIM_EF_FPLMN_LEN                           (60)                    /* FPLMN����20����ʵ�� */

#define ESN_NV_DATA_LEN                             (8)
#define MEID_NV_DATA_LEN                            (8)

#define PESN_NV_DATA_LEN                            (4)
#define MEID_NV_DATA_LEN_NEW                        (7)

#define USIMM_PLMN_DATA_MAX_LEN                     (32)

/*****************************************************************************
  3 ö�ٶ���
*****************************************************************************/

enum SC_APSEC_DEFAULT_KEY_VER_ENUM
{
    SC_APSEC_DEFAULT_KEY_VER_TEST       =   0,              /* ���԰汾 */
    SC_APSEC_DEFAULT_KEY_VER_DCM        =   1,              /* Docomo�汾 */
    SC_APSEC_DEFAULT_KEY_VER_BUTT,
};
typedef VOS_UINT8 SC_APSEC_DEFAULT_KEY_VER_ENUM_UINT8;

enum OM_OUTPUT_PORT
{
    OM_OUTPUT_SHELL = 0,        /*�����ṩ��SHELLͨ��*/
    OM_OUTPUT_FS,               /*д�ļ�ϵͳ��ʽ*/
    OM_OUTPUT_SDT,              /*�����SDT��*/
    OM_OUTPUT_BUTT
};
typedef VOS_UINT32      OM_OUTPUT_PORT_ENUM_UINT32;

/*****************************************************************************
 ö����    : AT_PHY_PORT_ENUM
 ö��˵��  : ����˿ں�ö��ֵ
*****************************************************************************/
enum AT_PHY_PORT_ENUM
{
    AT_UART_PORT = 0,
    AT_PCUI_PORT,
    AT_CTRL_PORT,
    AT_HSUART_PORT,
    AT_PCUI2_PORT,
    AT_PORT_BUTT
};
typedef VOS_UINT32  AT_PHY_PORT_ENUM_UINT32;

enum
{
    CPM_IND_PORT = AT_PORT_BUTT,    /* OM�����ϱ��˿� */
    CPM_CFG_PORT,                   /* OM���ö˿� */
    CPM_SD_PORT,
    CPM_WIFI_OM_IND_PORT,           /* WIFI��OM�����ϱ��˿� */
    CPM_WIFI_OM_CFG_PORT,           /* WIFI��OM�����·��˿� */
    CPM_WIFI_AT_PORT,               /* WIFI��AT�˿� */
    CPM_HSIC_IND_PORT,
    CPM_HSIC_CFG_PORT,
    CPM_VCOM_IND_PORT,              /* VCOM��OM�����ϱ��ӿ� */
    CPM_VCOM_CFG_PORT,              /* VCOM��OM���ýӿ� */
    CPM_FS_PORT,
    CPM_PORT_BUTT
};
typedef VOS_UINT32  CPM_PHY_PORT_ENUM_UINT32;

enum
{
    CPM_OM_PORT_TYPE_USB,
    CPM_OM_PORT_TYPE_VCOM,
    CPM_OM_PORT_TYPE_WIFI,
    CPM_OM_PORT_TYPE_SD,
    CPM_OM_PORT_TYPE_FS,
    CPM_OM_PORT_TYPE_HSIC,
    CBP_OM_PORT_TYPE_BUTT
};
typedef VOS_UINT32 CPM_OM_PORT_ENUM_UINT32;




enum VSIM_ALGORITHM_TYPE_ENUM
{
    VSIM_ALGORITHM_NULL = 0,
    VSIM_ALGORITHM_AES  = 1,
    VSIM_ALGORITHM_3DES = 2,
    VSIM_ALGORITHM_BUTT
};
typedef VOS_UINT32 VSIM_ALGORITHM_TYPE_ENUM_UINT32;


enum VSIM_ACTIVE_TYPE_ENUM
{
    VSIM_VIRT_CARD_DEACTIVE = 0,
    VSIM_VIRT_CARD_ACTIVE   = 1,
    VSIM_ACTIVE_TYPE_BUTT
};
typedef VOS_UINT8 VSIM_ACTIVE_TYPE_ENUM_UINT8;


enum USIMM_CARD_STATUS_REG_TYPE_ENUM
{
    USIMM_CARD_STATUS_REG_TYPE_E5       = 0,    /* ע��E5���͵Ŀ�״̬����ص����� */
    USIMM_CARD_STATUS_REG_TYPE_M2M      = 1,    /* ע��M2M���͵Ŀ�״̬����ص����� */
    USIMM_CARD_STATUS_REG_TYPE_BUTT
};

typedef VOS_UINT16 USIMM_CARD_STATUS_REG_TYPE_ENUM_UINT16;


enum EVENT_RESEND_FLAG_ENUM
{
    EVENT_RESEND_OFF        = 0,
    EVENT_RESEND_ON         = 1,
    EVENT_RESEND_FLAG_BUTT
};
typedef VOS_UINT8 EVENT_RESEND_FLAG_ENUM_UINT8;


enum ESN_MEID_FLAG_ENUM
{
    ESN_MEID_DISABLE        = 0,
    ESN_ME_ENABLE_FLAG      = 1,
    MEID_ME_ENABLE_FLAG     = 2,
    ESN_MEID_ENABLE_BOTH    = 3,
    ESN_MEID_ENABLE_FLAG_BUTT
};
typedef VOS_UINT8 ESN_MEID_FLAG_ENUM_UINT8;

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
    VOS_UINT8                           aucPassword[16];
}OM_AUTH_PASSWORD_STRU;


typedef struct
{
    VOS_UINT16                          usVerifyCnt;
}OM_VERIFYCNT_STRU;



typedef struct
{
    VOS_UINT32                        ulTtl;
}OM_TTL_STRU;


typedef struct
{
    VOS_UINT16                          usYear;
    VOS_UINT16                          usMonth;
    VOS_UINT16                          usDayOfWeek;
    VOS_UINT16                          usDay;
    VOS_UINT16                          usHour;
    VOS_UINT16                          usMinute;
    VOS_UINT16                          usSecond;
    VOS_UINT16                          usMilliSeconds;
}OM_CALIBRATED_TIME_STRU;


typedef struct
{
    VOS_UINT8                           aucSerialNumber[20];
}OM_SERIAL_NUM_STRU;


typedef struct
{
    VOS_UINT32                          ulLogFileMaxSize;
}OM_LOG_FILE_MAX_SIZE_STRU;



typedef struct
{
    VOS_UINT32                          ulWatchDogFlag; /*[0,1]*/
}OM_WATCHDOG_FLAG_STRU;


typedef struct
{
    VOS_UINT32                          ulKickDogTime;
}OM_KICKDOG_TIME_STRU;


typedef struct
{
    VOS_UINT32                          ulScheduleWatchDogTime;
}OM_SCHEDULE_WATCH_TIME_STRU;


typedef struct
{
    VOS_UINT8                           ucTempOverCount;
    VOS_UINT8                           ucTempResumeCount;
    VOS_UINT8                           ucTempCloseCount;
    VOS_UINT8                           ucTempReserve;
    VOS_UINT32                          ulWaitTimer;
}OM_MONITORPARA_CTRL_STRU;


typedef struct
{
    VOS_UINT32                          ulProtectEnable;
    VOS_INT32                           lPmicCloseAdcThreshold;
    VOS_INT32                           lPmicAlarmAdcThreshold;
    VOS_INT32                           lPmicResumeAdcThreshold;
    OM_MONITORPARA_CTRL_STRU            stMonitorParaCtrl;
}OM_USIM_TEMP_PROTECT_STRU;



typedef struct
{
    MODEM_ID_ENUM_UINT16                enModemID;
    VOS_UINT16                          usNetworkMode;  /*����ģʽ*/
    VOS_UINT16                          usBand;
    VOS_UINT16                          usSlotCnt;      /*ʱ϶��*/
    VOS_UINT16                          usChannel;
    VOS_UINT16                          usPower;        /*PA����*/
    VOS_UINT16                          usModType;      /*�������ģʽ*/
}OM_RF_CONFIG_STRU;



typedef struct
{
    VOS_UINT32                          ulIsEnable;     /*�Ƿ������ϻ����ԣ�VOS_YES/VOS_NO*/
    VOS_UINT32                          ulTimerLen;     /*��ʱ�����ȣ���λΪ����*/
    VOS_UINT32                          ulNumber;       /*ָʾ���������ĸ���*/
    OM_RF_CONFIG_STRU                   astOmRfConfig[OM_AGING_TEST_NUM];
}OM_AGING_TEST_NV_STRU;



typedef struct
{
    VOS_UINT32                          ulEnable;
    VOS_UINT32                          ulCycle;
} OM_LIVE_TIME_CONTROL_STRU;



typedef struct
{
    VOS_UINT32                          ulLiveTime;
}OM_LIVE_TIME_STRU;


typedef struct
{
    VOS_UINT16                          usDownLoadQos;
    VOS_UINT16                          usUpLoadQos;
}OM_SPY_QOSCNF_STRU;


typedef struct
{
    VOS_UINT8                           ucSPYCPUEnable;
    VOS_UINT8                           ucCPUUpLimit;
    VOS_UINT8                           ucCPUDownLimit;
    VOS_UINT8                           ucSleepRunBase;
    VOS_UINT8                           ucSleepWFIBase;
    VOS_UINT8                           ucUpLimitTimeMax;
    VOS_UINT8                           ucDownLimitTimeMax;
    VOS_UINT8                           ucRepirValue;
    OM_SPY_QOSCNF_STRU                  astSpyQosCnf[2];
}OM_MONITOR_CPU_STRU;


typedef struct
{
    VOS_UINT32                          ulRegCnt;
}OM_MAX_REG_CNT_GSM_STRU;


typedef struct
{
    VOS_UINT16                          usUpLimite;
    VOS_UINT16                          usDownLimite;
}OM_DFS_CONFIG_THRESHOLD_STRU;


typedef struct
{
    VOS_UINT32                          ulEnableFlag;
    VOS_UINT32                          ulTimerLen;
    VOS_UINT16                          usUpTime;
    VOS_UINT16                          usDownTime;
    VOS_UINT32                          ulProfileNum;
    OM_DFS_CONFIG_THRESHOLD_STRU        astThreshold[20];
}OM_DFS_CONFIG_STRU;


typedef struct
{
    VOS_UINT32                          ulNvSwVerFlag; /*[0,1]*/
}OM_SW_VER_FLAG_STRU;


typedef struct
{
    VOS_UINT32                          ulErrorLogEnable; /*[0,1]*/
}OM_ERRORLOG_ENABLE_FLAG_STRU;


typedef struct
{
    VOS_UINT32                          ulErrorFlushInter;
}OM_ERRORLOG_FLUSHBUFINTER_STRU;


typedef struct
{
    VOS_UINT32                          ulErrorRecord; /*[0,103680]*/
}OM_ERRORLOG_RECORDPERIOD_STRU;



typedef struct
{
    VOS_UINT8                           ucTempOverCount;     /*������ģʽ���¶����������澯���޻�͹������޵ļ��޴���*/
    VOS_UINT8                           ucTempRsumeCount;    /*�ڵ͹���ģʽ��澯ģʽ���¶��������ڸ澯�ŵļ��޴���*/
    VOS_UINT8                           ucTempCloseCount;    /*�ڽ���ģʽ���¶����������澯���޵ļ��޴���*/
    VOS_UINT8                           ucTempPowerOffCount; /*�����µ����޵ļ��޴���*/
    VOS_UINT32                          ulWaitTimer;         /*�ȴ���ʱ��ʱ��*/
}TEMP_PARA_CTRL_STRU;


typedef struct
{
    VOS_UINT32                          ulIsEnable;           /*�Ƿ���Ҫ������������, 0Ϊ�رգ�1Ϊ����*/
    VOS_UINT32                          ulLocation;           /*ʹ��������¶���Ϊ�±������룬�ò���ֱ����Ϊ��ν������ĺ���,Ŀǰ��ȡֵ��Χ��0��6 */
    VOS_INT32                           lPowerOffThreshold;   /*�����µ�ģʽ������*/
    VOS_INT32                           lCloseAdcThreshold;   /*����͹���ģʽ���¶�����*/
    VOS_INT32                           lAlarmAdcThreshold;   /*��Ҫ���и����������¶�����*/
    VOS_INT32                           lResumeAdcThreshold;  /*�ָ�������ģʽ���¶�����*/
    VOS_INT32                           lSIMPowerOffThreshold;  /*E5��̬��SIM���±�������*/
    TEMP_PARA_CTRL_STRU                 stTempCtrlPara;
}SPY_TEMP_PROTECT_NV_STRU;


enum
{
    CPM_CBT_PORT_USB,
    CPM_CBT_PORT_VCOM,
    CPM_CBT_PORT_SOCKET,
    CBP_CBT_PORT_BUTT
};
typedef VOS_UINT32 CPM_CBT_PORT_ENUM_UINT32;


typedef struct
{
    CPM_OM_PORT_ENUM_UINT32             enPortNum;         /* ����ͨ������ */
    CPM_CBT_PORT_ENUM_UINT32            enCbtPortNum;      /* У׼ͨ������ */
}OM_CHANNLE_PORT_CFG_STRU;


typedef struct
{
    OM_OUTPUT_PORT_ENUM_UINT32          enPortType;     /*�˿�����*/
    VOS_UINT32                          ulMaxFileSize;  /*�ļ���¼������С*/
}OM_PORT_CFG_STRU;


typedef struct
{
    VOS_UINT32                          ulM2Enable;
}OM_M2_ENABLE_STRU;


typedef struct
{
    VOS_UINT32                          ulResult;       /* У׼��� */
    VOS_UINT32                          ulLogFileSize;  /* ����LOG�ļ���С */
}LOG_FILE_SAVE_CONFIG_STRU;


typedef struct
{
    VOS_UINT32                                  ulIsEnable;
    VOS_UINT32                                  ulLen;
    VOS_UINT8                                   aucData[10];
    VOS_UINT16                                  usRsv;
}USIMM_TERMINAL_CAPABILITY_STRU;


typedef struct
{
    VOS_UINT8                           aucInfo[128];
}OM_MANUFACTUREINFO_STRU;



typedef struct
{
    VOS_UINT8                           aucMacAddr[32];
}OM_MAC_ADDR_STRU;


typedef struct
{
    VOS_UINT8                           aucMeanThroughPut[2];
}OM_MEAN_THROUGHPUT_STRU;



typedef struct
{
    SC_APSEC_DEFAULT_KEY_VER_ENUM_UINT8 aenISDBKey[2];
}OM_ISDB_DEFAULT_KEY_STRU;



typedef struct
{
    VOS_UINT32                        ulSimAtrFlag;
}OM_SIM_ATR_FLAG_STRU;


typedef struct
{
    VOS_UINT8                           ucOamConfig;
    VOS_UINT8                           aucRev[15];
}OM_NV_CONFIG_STRU;


typedef struct
{
    VOS_UINT32                          ulDataAddr;
    VOS_UINT32                          ulDataLen;
}LDF_DUMP_DATA_INFO_STRU;


typedef struct
{
    VOS_UINT32                          ulDataNum;
    LDF_DUMP_DATA_INFO_STRU             astLDFData[SYS_ZSP_LDF_NUM];
}ZSP_LDF_NV_CONFIG_STRU;


typedef struct
{
    VOS_UINT32                          ulDataNum;
    LDF_DUMP_DATA_INFO_STRU             astLDFData[SYS_HIFI_LDF_NUM];
}HIFI_LDF_NV_CONFIG_STRU;


typedef struct
{
    VOS_UINT16                          ausNvMiddle[6];
    VOS_UINT16                          ausRev[2];
}OM_PRI_THRESHOLD_STRU;


typedef struct
{
    VOS_UINT32                          ulAtt_flg               : 1;
    VOS_UINT32                          ulNFCFlg                : 1;
    VOS_UINT32                          ulAidLenCheckFlg        : 1;
    VOS_UINT32                          ulAuthCmdCheckFlg       : 1;
    VOS_UINT32                          ulCglaInsCheckFlg       : 1;
    VOS_UINT32                          ulCglaSelectCheckFlg    : 1;
    VOS_UINT32                          ulTmo_flg               : 1;
    VOS_UINT32                          ulImsiPolling           : 1;
    VOS_UINT32                          ulCglaGetRsp            : 1;
    VOS_UINT32                          ulAIDFCPSave            : 1;
    VOS_UINT32                          ulCUIMCheck             : 1;
    VOS_UINT32                          ulTimerOutInd           : 1;
    VOS_UINT32                          ulPKCS15                : 1;
    VOS_UINT32                          ulP2ActiveAID           : 1;
    VOS_UINT32                          ulWarningDataInd        : 1;

    VOS_UINT32                          bMeidPesnAPWriteEnable  : 1;  /* Meid Pesn�Ƿ���APд�룬Ĭ����modem����ʼ��д�� */
    VOS_UINT32                          ulADFActiveAll          : 1;

    /* ��CSIM APP�������Ҷ�ӦAID����,����ʼ����ICC��ʼ����������߶������Ƿ�����hisiĬ�Ϲر� */
    VOS_UINT32                          ulCsimAppAidCheck       : 1;

    /* ��ܳ�����ĳһ���ο�����CSIM��ʼ������д���ļ��󣬷���STATSU�������ͣ����60������ */
    VOS_UINT32                          ulCsimReselectFile      : 1;

    VOS_UINT32                          ulImsAccessCustomFlg    : 1;

    VOS_UINT32                          ulDocomoReadOPFileFlg   : 1;

    /* ���CGLA�Ƿ���&0x6F ������ */
    VOS_UINT32                          ulCglaGetRspCustomFlg   : 1;

    VOS_UINT32                          ulRsv                   : 10;
}USIMM_FEATURE_CFG_BIT_STRU;


typedef struct
{
    union
    {
        VOS_UINT32                      aulValue[1];
        USIMM_FEATURE_CFG_BIT_STRU      stFeatureCfg;
    }unCfg;
}USIMM_FEATURE_CFG_STRU;


typedef struct
{
    VOS_UINT32                          ulCfg;
}USIMM_FEATURE_CFG_NV_STRU;


typedef struct
{
    VOS_UINT32                          ulLociEventCmp          : 1;

    VOS_UINT32                          ulRsv                   :31;
}STK_FEATURE_CFG_BIT_STRU;


typedef struct
{
    union
    {
        VOS_UINT32                    aulValue[1];
        STK_FEATURE_CFG_BIT_STRU      stFeatureCfg;
    }unCfg;
}STK_FEATURE_CFG_STRU;


typedef struct
{
    VOS_UINT32                          ulCfg;
}STK_FEATURE_CFG_NV_STRU;


typedef struct
{
    VOS_UINT32                          ulUsimNeedFcp;
}OM_USIM_NEED_FCP_STRU;



typedef struct
{
    VOS_UINT16                          usOptionFileNum;
    VOS_UINT16                          ausOptionFileList[16];
    VOS_UINT16                          usReserved;
}OM_USIM_OP_FILE_CFG_STRU;


typedef struct
{
    VOS_UINT8                           aucPlmn[USIMM_TEST_CARD_PLMN_LEN];
    VOS_UINT8                           ucRsv;
}USIMM_TEST_CARD_PLMN_ST;


typedef struct
{
    VOS_UINT8                           aucPlmn[USIMM_CDMATEST_CARD_PLMN_LEN];
    VOS_UINT8                           ucRsv1;
    VOS_UINT8                           ucRsv2;
}USIMM_CDMATEST_CARD_PLMN_ST;


typedef struct
{
    VOS_UINT32                          ulPlmnNum;
    USIMM_TEST_CARD_PLMN_ST             astPlmnList[USIMM_TEST_CARD_CFG_NUM];
}USIMM_TEST_CARD_CFG_ST;


typedef struct
{
    VOS_UINT32                          ulPlmnNum;
    USIMM_CDMATEST_CARD_PLMN_ST         astPlmnList[USIMM_TEST_CARD_CFG_NUM];
}USIMM_CDMATEST_CARD_CFG_ST;


typedef struct
{
    VOS_UINT32                        ulEnable;
}OM_BBP_DUMP_ENABLE_STRU;



typedef struct
{
    VOS_UINT8                           aucContent[250];
    VOS_UINT16                          usReserved;
}OM_FILE_EF6F62_STRU;


typedef struct
{
    VOS_UINT8                           aucContent[12];
}OM_FILE_EF6F7B_STRU;


typedef struct
{
    VOS_UINT8                           aucContent[170];
    VOS_UINT16                          usReserved;
}OM_FILE_EF6F60_STRU;

typedef struct
{
    VOS_UINT8                           aucContent[250];
    VOS_UINT16                          usReserved;
}OM_FILE_EF6F61_STRU;


typedef struct
{
    VOS_UINT8                           aucContent[2];
    VOS_UINT16                          usReserved;
}OM_FILE_EF6F31_STRU;


typedef struct
{
    VOS_UINT8                           aucContent[4];
}OM_FILE_EF6FAD_STRU;


typedef struct
{
    VOS_UINT8                           aucContent[8];
}OM_FILE_EF6F38_STRU;


typedef struct
{
    VOS_UINT8                           aucContent[12];
}OM_FILE_EF6F7E_STRU;


typedef struct
{
    VOS_UINT8                           aucContent[14];
    VOS_UINT16                          usReserved;
}OM_FILE_EF6F73_STRU;


typedef struct
{
    VOS_UINT8                           aucContent[12];
}OM_FILE_EF6F53_STRU;


typedef struct
{
    VOS_UINT8                           aucContent[10];
    VOS_UINT16                          usReserved;
}OM_FILE_EF6F07_STRU;


typedef struct
{
    VOS_UINT8                           aucContent[34];
    VOS_UINT16                          usReserved;
}OM_FILE_EF6F08_STRU;


typedef struct
{
    VOS_UINT8                           aucContent[34];
    VOS_UINT16                          usReserved;
}OM_FILE_EF6F09_STRU;


typedef struct
{
    VOS_UINT8                           aucContent[2];
    VOS_UINT16                          usReserved;
}OM_FILE_EF6F20_STRU;


typedef struct
{
    VOS_UINT8                           aucContent[8];
}OM_FILE_EF6F52_STRU;


typedef struct
{
    VOS_UINT8                           aucContent[10];
    VOS_UINT16                          usReserved;
}OM_FILE_EF4F20_STRU;


typedef struct
{
    VOS_UINT8                           aucContent[10];
    VOS_UINT16                          usReserved;
}OM_FILE_EF4F52_STRU;


typedef struct
{
    VOS_UINT8                           aucContent[14];
    VOS_UINT16                          usReserved;
}OM_FILE_EF6FB7_STRU;


typedef struct
{
    VOS_UINT8                           aucContent[2];
    VOS_UINT16                          usReserved;
}OM_FILE_EF6F78_STRU;


typedef struct
{
    VOS_UINT8                           aucContent[6];
    VOS_UINT16                          usReserved;
}OM_FILE_EF6F5B_STRU;


typedef struct
{
    VOS_UINT8                           aucContent[4];
}OM_FILE_EF6F5C_STRU;


typedef struct
{
    VOS_UINT8                           aucContent[50];
    VOS_UINT16                          usReserved;
}OM_FILE_EF6FC4_STRU;


typedef struct
{
    VOS_UINT8                           aucContent[50];
    VOS_UINT16                          usReserved;
}OM_FILE_EF6F74_STRU;


typedef VOS_UINT32 SI_PB_STORATE_TYPE;
typedef struct
{
    SI_PB_STORATE_TYPE                  enCurMaterial;
    VOS_UINT8                           aucPasswords[8];
    VOS_UINT16                          usUsed;
    VOS_UINT16                          usTotal;
    VOS_UINT8                           ucAnrMaxNum;        /*Balong֧�ֵ�ANR�����ֵ*/
    VOS_UINT8                           ucEmailFlag;        /*Balong֧��Email���*/
    VOS_UINT8                           ucSPBFlag;         /*���ϵ绰��֧�ֱ�ǣ�Ϊ1ʱ��ʾ֧��*/
    VOS_UINT8                           ucRsv;
}SI_PB_CTRL_INFO_ST;


typedef struct
{
    VOS_UINT16                          usFlag;
    VOS_UINT16                          usTimerValue;
}USIMM_CMD_DEBUG_MODE_ST;


typedef struct
{
    VOS_UINT8                           ucProfileLen;
    VOS_UINT8                           aucProfile[47];
}USIMM_USIM_PROFILE_STRU;


typedef struct
{
    VOS_UINT8                           ucProfileLen;
    VOS_UINT8                           aucProfile[31];
}USIMM_SIM_PROFILE_STRU;

typedef struct
{
    VOS_UINT8                           ucFuncEnable;
    VOS_UINT8                           ucTimer;
    VOS_UINT8                           ucTerminalType;
    VOS_UINT8                           ucRsv;
    USIMM_USIM_PROFILE_STRU             stUsimProfile;
    USIMM_SIM_PROFILE_STRU              stSimProfile;
}USIMM_STK_CFG_STRU;


typedef struct
{
    VOS_UINT32                           ulUsimSupImsEnable;
}USIMM_USIMSUPIMS_STRU;


typedef struct
{
    VOS_UINT16                          usStrLen;
    VOS_UINT8                           aucMatchStr[22];
}SI_STK_MATCH_STRING_STRU;


typedef struct
{
    VOS_UINT16                          usDualIMSIEnable;      /* ��һ��ѡ���ITEM ID */
    VOS_UINT16                          usMatchStrCnt;
    SI_STK_MATCH_STRING_STRU            astMatchStr[3];
}SI_STK_IMSICHG_MATCH_STRU;


typedef struct
{
    VOS_UINT32                          ulStkSmsindCtrl;
}SI_STK_SMSIN_CTRL_STRU;


typedef struct
{
    VOS_UINT32                          ulVivo;
}OM_VIVO_STK_CODEC_STRU;


typedef struct
{
    VOS_UINT8                           ucCfg;
    VOS_UINT8                           ucEnable;
}NV_HUAWEI_DOUBLE_IMSI_CFG_I_STRU;


typedef struct
{
    VOS_UINT32                          ulRecordFlag;
}NV_ID_WRITE_SLICE_RECORD_FLAG_STRU;


typedef struct
{
    VOS_UINT32 ulSocpDelayWriteFlg;/* SOCP�ӳ�д�빦��������� */
    VOS_UINT32 ulIndSocpLevel;     /* INDͨ��Ŀ��BUFFER����ˮ�� */
    VOS_UINT32 ulCfgSocpLevel;     /* CFGͨ��Ŀ��BUFFER����ˮ�� */
    VOS_UINT32 ulTimeOutValue;     /* SOCPĿ��BUFFER�ĳ�ʱʱ�� */
}NV_SOCP_SDLOG_CFG_STRU;


typedef struct
{
    VOS_UINT8                           ucAlmStatus; /* �澯״̬,Ĭ��0:close;1:open  */
    VOS_UINT8                           ucAlmLevel;  /* ����&�澯����
                                                                Warning��0x04������ʾ��
                                                                Minor��0x03�����Ҫ
                                                                Major��0x02������Ҫ
                                                                Critical��0x01���������Ĭ�ϣ�
                                                              ˵����ֵΪ0x03�� 0x03/0x02/0x01���ϱ� */
    VOS_UINT8                          aucReportBitMap[2]; /* A0000000 00000DCB
                                                              A���������ϱ�����
                                                              B���������������Կ���
                                                              C�����������Թ��������ϱ�����
                                                              D����RATƵ���л������ϱ�����
                                                           */
}NV_ID_ERR_LOG_CTRL_INFO_STRU;


typedef struct
{
    VOS_UINT32                          ulAlarmid;        /* �澯��ʶ */
    VOS_UINT32                          ulAlarmidDetail;  /* �澯����ԣ�32bit��ÿ��bit����һ������ԣ�0����͸�������޹� */
}OM_ALARM_ID_DETAIL_STRU;


typedef struct
{
    OM_ALARM_ID_DETAIL_STRU          astOmAlarmidRelationship[40]; /* Ԥ��40�� */
}NV_ALARM_ID_RELATIONSHIP_STRU;


typedef struct
{
    VOS_UINT32                          ulFTMDetail; /* ����ģʽ����ԣ�32bit��ÿ��bit����һ������ԣ�0����͸�������޹� */
}NV_ID_FTM_DETAIL_STRU;


typedef struct
{
    VOS_UINT32                          ulKeyLen;
    VOS_UINT8                           aucKey[VSIM_KEYLEN_MAX];
}VSIM_KEYDATA_STRU;


typedef struct
{
    VSIM_ACTIVE_TYPE_ENUM_UINT8         enVsimState;
    VOS_UINT8                           aucRfu[3];
}VSIM_CARD_STATE_NV_STRU;


typedef struct
{
    VOS_UINT8                           aucEfloci[VSIM_EF_LOCI_LEN];
    VOS_UINT8                           ucRsv;
}VSIM_CARD_LOCI_FILE_NV_STRU;


typedef struct
{
    VOS_UINT8                           aucPsEfloci[VSIM_EF_PS_LOCI_LEN];
    VOS_UINT16                          usReserved;
}VSIM_CARD_PSLOCI_FILE_NV_STRU;


typedef struct
{
    VOS_UINT8                           aucFplmn[VSIM_EF_FPLMN_LEN];
}VSIM_CARD_FPLMN_FILE_NV_STRU;


typedef struct
{
    VSIM_ALGORITHM_TYPE_ENUM_UINT32     enAlgorithm;
    VOS_UINT32                          ulDHLen;         /* DH�㷨�������ӵĳ��� */
    VSIM_KEYDATA_STRU                   stCPrivateKey;   /* �����˽Կ */
    VSIM_KEYDATA_STRU                   stCPublicKey;    /* ����⹫Կ */
    VSIM_KEYDATA_STRU                   stSPublicKey;    /* ��������Կ */
}NVIM_VSIM_HVSDH_NV_STRU;


typedef struct
{
    VOS_UINT8                           ucABBSwitch;    /*��Ӧģʽʹ�õ�ABB����ͨ����0ΪABB CH0��1ΪABB CH1��2Ϊ��ǰģʽʹ��������ͨ��*/
    VOS_UINT8                           ucRFSwitch;     /*��Ӧģʽʹ�õ�RF����ͨ����0ΪRFʹ��mipi0����PA-Star�ϵ磻1ΪRFʹ��mipi1����PA-Star�ϵ磬����ʹ��Smart-Star��BUCK6���ϵ磬2Ϊ��ǰģʽʹ������RFIC*/
    VOS_UINT8                           ucTCXOSwitch;   /*��Ӧģʽʹ�õ�TCXO��0ΪTCXO0��1ΪTCXO */
    VOS_UINT8                           ucRsv1;
}NV_MODE_BASIC_PARA_STRU;



typedef struct
{
    NV_MODE_BASIC_PARA_STRU             astModeBasicParam[2];    /*��ͬ��ʽ��ͨ������*/
}NV_GUMODE_CHAN_PARA_STRU;



typedef struct
{
    VOS_UINT8                           aucXMLBaseBoardId[24]; /* ��¼base board xml ���� */
}NV_RECORD_BASE_BOARD_XML_STRU;



typedef struct
{
    VOS_UINT8                           aucXMLCurrentBoardId[24]; /* ��¼Currnet board xml ���� */
}NV_RECORD_CURRENT_BOARD_XML_STRU;


typedef struct
{
    VOS_UINT32                          ulGULogFileSize;
    VOS_UINT32                          ulTLLogFileSize;
}NV_FLASH_LOG_RECORD_STRU;


typedef struct
{
    USIMM_CARD_STATUS_REG_TYPE_ENUM_UINT16                  enType;
    VOS_UINT16                                              usReserved;
}NVIM_USIM_CARD_STATUS_CB_TYPE_STRU;


typedef struct
{
    EVENT_RESEND_FLAG_ENUM_UINT8            enResendFlag;   /*1������0�ر�*/
    VOS_UINT8                               ucRetryTime;    /*���Դ������������Ϊ0Ϊ���ط�*/
    VOS_UINT8                               ucTimerLen;     /*���ʱ�䣬��λΪ��, �������Ϊ0Ϊ���ط�*/
    VOS_UINT8                               ucRsv;
}NV_EVENT_RESEND_CFG_STRU;


enum USIMM_DL_T_MODE_ENUM
{
    USIMM_DL_T_MODE_T0        = 0,    /* ֧��T=0ģʽ */
    USIMM_DL_T_MODE_T1        = 1,    /* ֧��T=1ģʽ */
    USIMM_DL_T_MODE_BUTT
};
typedef VOS_UINT32      USIMM_DL_T_MODE_ENUM_UINT32;


enum USIMM_T1_ERR_DETECT_MODE_ENUM
{
    USIMM_T1_ERR_DETECT_MODE_ISO_IEC_7816_3  = 0,           /* NVĬ��ֵ����ѭISO_IEC 7816-3 2006�淶��EDC����ATR�е�ָʾ��ȷ��(LRC��CRC) */
    USIMM_T1_ERR_DETECT_MODE_TS102221        = 1,           /* ��ѭTS_102221v110000p�淶��EDCֻʹ��LRC */

    USIMM_T1_ERR_DETECT_MODE_BUTT
};
typedef VOS_UINT32 USIMM_T1_ERR_DETECT_MODE_ENUM_UINT32;


enum USIMM_DL_T1_GCF_FLAG_ENUM
{
    USIMM_DL_T1_GCF_DISABLE       = 0,    /* ��ǰ����GCF���� */
    USIMM_DL_T1_GCF_ENABLE        = 1,    /* ��ǰGCF����ʹ�� */
    USIMM_DL_T1_GCF_BUTT
};
typedef VOS_UINT32      USIMM_DL_T1_GCF_FLAG_ENUM_UINT32;



typedef struct
{
    USIMM_DL_T_MODE_ENUM_UINT32                             enTMode;
    USIMM_T1_ERR_DETECT_MODE_ENUM_UINT32                    enEDM;
    USIMM_DL_T1_GCF_FLAG_ENUM_UINT32                        enGcfFlag;
    VOS_UINT32                                              ulDefaultIFSD;
    VOS_UINT32                                              aulRsv[2];
}NV_USIMM_T1_CTRL_PARA_STRU;


typedef struct
{
    VOS_BOOL                           bCBTLogEnable;
}NV_OM_CBT_LOG_ENABLE_STRU;


typedef struct
{
    VOS_BOOL                          bResetEnable;
}NV_SLEEP_DRX_RESET_ENABLE_STRU;


typedef struct
{
    ESN_MEID_FLAG_ENUM_UINT8        enEsnMeIDFlag;
    VOS_UINT8                       ucRSv;
    VOS_UINT8                       aucEsn[ESN_NV_DATA_LEN];  /*Len+Data*/
    VOS_UINT8                       aucMeID[MEID_NV_DATA_LEN]; /*Len+Data*/
    VOS_UINT8                       aucRfu[6];
}NV_ESN_MEID_STRU;


typedef struct
{
    VOS_BOOL                            bEnable;       /*ʹ�ܱ�־λ*/
    VOS_UINT32                          ulRsv;
}NV_DDR_ADJUST_ENABLE_STRU;


typedef struct
{
    VOS_UINT8                       aucPEsn[PESN_NV_DATA_LEN];      /*Data*/
}NV_PESN_STRU;


typedef struct
{
    VOS_UINT8                       aucMeID[MEID_NV_DATA_LEN_NEW];  /*Data*/
    VOS_UINT8                       aucRsv[1];
}NV_MEID_STRU;


enum USIMM_NV_CARDAPP_ENUM
{
    USIMM_NV_GUTL_APP          = 0x00000000,
    USIMM_NV_IMS_APP           = 0x00000001,
    USIMM_NV_CDMA_APP          = 0x00000002,
    USIMM_NV_CARDAPP_BUTT      = 0x00000003
};
typedef VOS_UINT32 USIMM_NV_CARDAPP_ENUM_UINT32;


typedef struct
{
    VOS_UINT8                           ucAppNum;
    VOS_UINT8                           aucRsv[3];
    USIMM_NV_CARDAPP_ENUM_UINT32        aenAppList[5];
}USIMM_APP_PRIORITY_CONFIG_STRU;


typedef struct
{
    VOS_UINT32                          ulSleepBBE16Switch;
}NV_SLEEP_BBE16_CTRL_STRU;


enum SC_PERS_PLATFORM_ENUM
{
    SC_PERS_PLATFORM_G,   /* �ǵ���ģʽ�������� */

    SC_PERS_PLATFORM_C,   /* ����ģʽ�������� */

    SC_PERS_PLATFORM_BUT,
};
typedef VOS_UINT32 SC_PERS_PLATFORM_ENUM_UINT32;


typedef struct
{
    SC_PERS_PLATFORM_ENUM_UINT32        enScPersModem;
}SC_PERS_SUPPORT_PLATFORM_STRU;


typedef struct
{
    VOS_UINT16                          usImei0ReplaseImei2;
    VOS_UINT16                          usRsv;
}NV_SC_PERS_CTRL_STRU;


typedef struct
{
    VOS_UINT8                   ucSiWaitGpsPseudoCnfTimeLen;   /* NI���̵ȴ�GPSα����Ϣ�ظ�ʱ��:��λ�� */
    VOS_UINT8                   ucNiWaitGpsPseudoCnfTimeLen;   /* ��NI���̵ȴ�GPSα����Ϣ�ظ�ʱ��:��λ�� */
    VOS_UINT8                   ucWaitCasRefLocIndTimeLen;     /* �ȴ�CAS REF LOC IND��Ϣ��ʱʱ��:��λ�� */
    VOS_UINT8                   ucMsaMaxPilotCnt;              /* MSA�������Pilot��Ŀ */

    VOS_UINT8                   ucRsv[8];
}NV_XPDS_FEATURE_CONTROL_STRU;


typedef struct
{
    VOS_UINT8                           ucIsimEnableFlag;
    VOS_UINT8                           aucRsv[3];
}NV_ISIM_ENABLE_STRU;


typedef struct
{
    VOS_UINT8                           ucEnable;               /* 1Ϊ������0Ϊ�ر� */
    VOS_UINT8                           ucRsv;
    VOS_UINT16                          usProtectTime;          /* ����ʱ������λΪ�� */
}USIMM_ACK_PROTECT_NV_STRU;


typedef struct
{
    VOS_UINT8                           ucEnable;               /* 1Ϊ������0Ϊ�ر� Ĭ�Ͽ��� */
    VOS_UINT8                           ucInitCheckTimeLen;     /* ��ʼ���ʱ������λΪ�룬Ĭ��Ϊ5s */
    VOS_UINT8                           ucCheckTimeLen;         /* ��μ��󣬶�ʱ����������λΪ�룬Ĭ��Ϊ60s */
    VOS_UINT8                           ucRcv;
}USIMM_CHECK_KEY_FILE_NV_STRU;


typedef struct
{
    VOS_UINT8                           ucDisableRealIsim;               /* 1Ϊ��ֹ��ʵISIM��0Ϊ����ֹ��ʵISIM */
    VOS_UINT8                           aucRsv[3];
}USIMM_DISABLE_REAL_ISIM_CTRL_NV_STRU;


typedef struct
{
    VOS_UINT16                          usImeiBindSlotFlag;
    VOS_UINT16                          usRsv;
}NV_SC_PERS_IMEI_BIND_SLOT_STRU;


typedef struct
{
    VOS_UINT32                          ulEnable;
}USIM_CLOSE_CHANNEL_SND_STATUS_STRU;


typedef struct
{
    VOS_UINT32                          ulFilterProvideCommandFlag;
}USIMM_PROVIDE_COMMAND_CTRL_NV_STRU;


typedef struct
{
    VOS_UINT32                          ulCheckSwFlg;
}USIMM_STATUS_PROTECTRESET_CHECKSW_STRU;


typedef struct
{
    VOS_UINT32                          ulEnable; /* 0:disable, 1:enable */
    VOS_UINT32                          ulThreeBytesMncNum;
    VOS_UINT32                          aulThreeBytesMncPlmnData[USIMM_PLMN_DATA_MAX_LEN];
    VOS_UINT32                          ulTwoBytesMncNum;
    VOS_UINT32                          aulTwoBytesMncPlmnData[USIMM_PLMN_DATA_MAX_LEN];
}USIMM_PLMN_DIGIT_NV_CTRL_STRU;

/*****************************************************************************
  8 UNION����
*****************************************************************************/


/*****************************************************************************
  9 OTHERS����
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
