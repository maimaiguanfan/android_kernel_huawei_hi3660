

#ifndef __MTACOMMINTERFACE_H__
#define __MTACOMMINTERFACE_H__

/*****************************************************************************
  1 ����ͷ�ļ�����
*****************************************************************************/
#include  "vos.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(4)

/*****************************************************************************
  2 �궨��
*****************************************************************************/
/*network monitor GSM���� �ϱ������� */
#define NETMON_MAX_GSM_NCELL_NUM                (6)

/*network monitor UTRAN ���� �ϱ������� */
#define NETMON_MAX_UTRAN_NCELL_NUM              (16)

/*network monitor LTE ���� �ϱ������� */
#define NETMON_MAX_LTE_NCELL_NUM                (16)

/*****************************************************************************
  3 ö�ٶ���
*****************************************************************************/

enum MTA_RRC_GSM_BAND_ENUM
{
    MTA_RRC_GSM_BAND_850  = 0x00,
    MTA_RRC_GSM_BAND_900,
    MTA_RRC_GSM_BAND_1800,
    MTA_RRC_GSM_BAND_1900,

    MTA_RRC_GSM_BAND_BUTT
};
typedef VOS_UINT16 MTA_RRC_GSM_BAND_ENUM_UINT16;


enum MTA_NETMON_CELL_TYPE_ENUM
{
    MTA_NETMON_SCELL_TYPE               = 0,
    MTA_NETMON_NCELL_TYPE               = 1,
    MTA_NETMON_CELL_TYPE_BUTT
};

typedef VOS_UINT32 MTA_NETMON_CELL_TYPE_ENUM_UINT32;


enum MTA_NETMON_UTRAN_TYPE_ENUM
{
    MTA_NETMON_UTRAN_FDD_TYPE           = 0,
    MTA_NETMON_UTRAN_TDD_TYPE           = 1,
    MTA_NETMON_UTRAN_TYPE_BUTT
};
typedef VOS_UINT32 MTA_NETMON_UTRAN_TYPE_ENUM_UINT32;


enum MTA_NETMON_RESULT_ENUM
{
    MTA_NETMON_RESULT_NO_ERR            = 0,
    MTA_NETMON_RESULT_ERR               = 1,
    MTA_NETMON_RESULT_BUTT
};
typedef VOS_UINT32 MTA_NETMON_RESULT_ENUM_UINT32;

/*****************************************************************************
 ö����    : MTA_NETMON_CELL_QRY_MODULE_ENUM
 Э����  :
 ASN.1���� :
 ö��˵��  : ��ǰ�������ѯС����ģ��ö��ֵ
*****************************************************************************/
enum MTA_NETMON_CELL_QRY_MODULE_ENUM
{
    MTA_NETMON_CELL_QRY_MODULE_AT    = 0x00,    /* ATģ���������ѯС����Ϣ��AT^MONSC/AT^MONNC���� */
    MTA_NETMON_CELL_QRY_MODULE_MTC   = 0x01,    /* MTCģ���������ѯС����Ϣ */
    MTA_NETMON_CELL_QRY_MODULE_BUTT
};
typedef VOS_UINT32 MTA_NETMON_CELL_QRY_MODULE_ENUM_UINT32;

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

/* ========�����ǽ������MTA֮�����Ϣ�ṹ��======== */

/**************************network monitor��ѯ������Ϣ�ṹ�岿��**********************************************************/

/* MTA������������Ϣ��ͨ�ã�������2G/3G/����С��/���� */

typedef struct
{
    VOS_MSG_HEADER                                     /* ��Ϣͷ*/              /*_H2ASN_Skip*/
    VOS_UINT32                          ulMsgName;     /*��Ϣ����*/             /*_H2ASN_Skip*/
    MTA_NETMON_CELL_TYPE_ENUM_UINT32    enCelltype;    /*0:��ѯ����С����1:��ѯ����*/
    VOS_UINT32                          ulQryModule;   /* ��ǰ��ѯ��ģ����Ϣ */
}MTA_RRC_NETMON_CELL_QRY_REQ_STRU;

/*GSM �����ṹ*/

typedef struct
{
    VOS_UINT32                          bitOpBsic     : 1;
    VOS_UINT32                          bitOpCellID   : 1;
    VOS_UINT32                          bitOpLAC      : 1;
    VOS_UINT32                          bitOpSpare    : 29;
    VOS_UINT32                          ulCellID;       /*С��ID*/
    VOS_UINT16                          usLAC;          /*λ������*/
    VOS_UINT16                          usAfrcn;        /*Ƶ��*/
    VOS_INT16                           sRSSI;          /* Ƶ���RSSI */
    MTA_RRC_GSM_BAND_ENUM_UINT16        enBand;         /* band 0-3 */
    VOS_UINT8                           ucBsic;         /*С����վ��*/
    VOS_UINT8                           aucReserved[3];
}MTA_NETMON_GSM_NCELL_INFO_STRU;


/*LTE �����ṹ,��ʱ��������ݽṹ��������Ҫ���е���*/

typedef struct
{
    VOS_UINT32                          ulPID;            /*����С��ID*/
    VOS_UINT16                          usArfcn;          /*Ƶ��*/
    VOS_INT16                           sRSRP;            /* RSRP�ο��źŽ��չ���*/
    VOS_INT16                           sRSRQ;            /* RSRQ�ο��źŽ�������*/
    VOS_INT16                           sRSSI;            /* Receiving signal strength in dbm */
}MTA_NETMON_LTE_NCELL_INFO_STRU;


/*WCDMA �����ṹ*/

typedef struct
{
    VOS_UINT16                          usArfcn;          /*Ƶ��*/
    VOS_UINT16                          usPSC;            /*������*/
    VOS_INT16                           sECN0;            /*ECN0*/
    VOS_INT16                           sRSCP;            /*RSCP*/
}MTA_NETMON_UTRAN_NCELL_INFO_FDD_STRU;


/*TD_SCDMA �����ṹ,��ʱ����ṹ��������Ҫ��������*/

typedef struct
{
    VOS_UINT16                          usArfcn;          /*Ƶ��*/
    VOS_UINT16                          usSC;             /*����*/
    VOS_UINT16                          usSyncID;         /*���е�Ƶ��*/
    VOS_INT16                           sRSCP;            /*RSCP*/
}MTA_NETMON_UTRAN_NCELL_INFO_TDD_STRU;


/*�������ݽṹ*/

typedef struct
{
    VOS_UINT8                                ucGsmNCellCnt;      /*GSM ��������*/
    VOS_UINT8                                ucUtranNCellCnt;    /*WCDMA ��������*/
    VOS_UINT8                                ucLteNCellCnt;      /*LTE ��������*/
    VOS_UINT8                                ucReserved;
    MTA_NETMON_GSM_NCELL_INFO_STRU           astGsmNCellInfo[NETMON_MAX_GSM_NCELL_NUM];          /*GSM �������ݽṹ*/
    MTA_NETMON_UTRAN_TYPE_ENUM_UINT32        enCellMeasTypeChoice;     /* NETMONƵ����Ϣ����:FDD,TDD */
    union
    {
        MTA_NETMON_UTRAN_NCELL_INFO_FDD_STRU astFddNCellInfo[NETMON_MAX_UTRAN_NCELL_NUM];         /* FDD�������ݽṹ */
        MTA_NETMON_UTRAN_NCELL_INFO_TDD_STRU astTddNCellInfo[NETMON_MAX_UTRAN_NCELL_NUM];         /*  TDD�������ݽṹ */
    }u;
    MTA_NETMON_LTE_NCELL_INFO_STRU           astLteNCellInfo[NETMON_MAX_LTE_NCELL_NUM];      /*LTE �������ݽṹ*/
}RRC_MTA_NETMON_NCELL_INFO_STRU;


/*********************network monitor��ѯGSM С����Ϣ����************************************************/

typedef struct
{
    VOS_MSG_HEADER                                     /* ��Ϣͷ */             /*_H2ASN_Skip*/
    VOS_UINT32                          ulMsgName;     /* ��Ϣ���� */           /*_H2ASN_Skip*/
}MTA_GRR_NETMON_TA_QRY_REQ_STRU;


typedef struct
{
    VOS_MSG_HEADER                                          /* ��Ϣͷ */        /*_H2ASN_Skip*/
    VOS_UINT32                          ulMsgName;          /* ��Ϣ���� */      /*_H2ASN_Skip*/
    MTA_NETMON_RESULT_ENUM_UINT32       enResult;           /* ���ؽ�� */
    VOS_UINT16                          usTa;               /* ���ص�TAֵ */
    VOS_UINT8                           aucReserved[2];     /* ���ֽڶ���ı���λ */
}GRR_MTA_NETMON_TA_QRY_CNF_STRU;


typedef struct
{
    VOS_UINT32                          ulMcc;              /* �ƶ������� */
    VOS_UINT32                          ulMnc;              /* �ƶ������� */
    VOS_UINT32                          ulCellID;           /* С��ID */
    VOS_UINT16                          usLac;              /* λ������ */
    VOS_UINT16                          usArfcn;            /* ����Ƶ��� */
    VOS_INT16                           sRssi;              /* Receiving signal strength in dbm */
    MTA_RRC_GSM_BAND_ENUM_UINT16        enBand;             /* GSMƵ��(0-3) */
    VOS_UINT8                           ucBsic;             /* С����վ�� */
    VOS_UINT8                           ucRxQuality;        /* IDLE̬�»���PS����̬����Ч,ר��̬������ŵ�����ֵ����Χ[0,7] ,��Чֵ99*/
    VOS_UINT8                           aucReserved[2];     /* ���ֽڶ���ı���λ */
}GRR_MTA_NETMON_SCELL_INFO_STRU;


typedef struct
{
    VOS_MSG_HEADER                                                              /*_H2ASN_Skip*/
    VOS_UINT32                          ulMsgName;      /* ��Ϣ���� */          /*_H2ASN_Skip*/
    MTA_NETMON_RESULT_ENUM_UINT32       enResult;
    MTA_NETMON_CELL_TYPE_ENUM_UINT32    enCelltype;     /* 0:��ѯ����С����1:��ѯ���� */
    VOS_UINT32                          ulQryModule;    /* ��ǰ��ѯ��ģ����Ϣ */
    union
    {
        GRR_MTA_NETMON_SCELL_INFO_STRU  stSCellinfo;    /* GSM�µķ���С����Ϣ */
        RRC_MTA_NETMON_NCELL_INFO_STRU  stNCellinfo;    /* GSM�µ�������Ϣ */
    }u;
}GRR_MTA_NETMON_CELL_QRY_CNF_STRU;

/******************************network monitor��ѯUTRAN С����Ϣ����****************************************************/


/*FDD ����С����Ϣ�ṹ*/

typedef struct
{
    VOS_UINT32                          bitOpDRX      : 1;
    VOS_UINT32                          bitOpURA      : 1;
    VOS_UINT32                          bitOpSpare    : 30;
    VOS_UINT32                          ulDrx;          /* Discontinuous reception cycle length */
    VOS_INT16                           sECN0;          /*ECN0*/
    VOS_INT16                           sRSSI;          /* Receiving signal strength in dbm */
    VOS_INT16                           sRSCP;          /*Received Signal Code Power in dBm�������ź��빦��*/
    VOS_UINT16                          usPSC;          /*������*/
    VOS_UINT16                          usURA;          /* UTRAN Registration Area Identity */
    VOS_UINT8                           aucReserved[2];

} MTA_NETMON_UTRAN_SCELL_INFO_FDD_STRU;


/*TDD ����С����Ϣ�ṹԤ������ȷ�Ϻ����*/

typedef struct
{
    VOS_UINT32                          ulDrx;              /* Discontinuous reception cycle length */
    VOS_UINT16                          usSC;               /*����*/
    VOS_UINT16                          usSyncID;           /*���е�Ƶ��*/
    VOS_UINT16                          usRac;              /*RAC*/
    VOS_INT16                           sRSCP;              /*RSCP*/
} MTA_NETMON_UTRAN_SCELL_INFO_TDD_STRU;



 /*UTRAN ����С���ṹ*/
 
typedef struct
{
    VOS_UINT32                                  bitOpCellID  : 1;
    VOS_UINT32                                  bitOpLAC     : 1;
    VOS_UINT32                                  bitOpSpare   : 30;
    VOS_UINT32                                  ulMcc;                  /*�ƶ�������*/
    VOS_UINT32                                  ulMnc;                  /*�ƶ�������*/
    VOS_UINT32                                  ulCellID;               /*С��ID*/
    VOS_UINT16                                  usArfcn;                /*Ƶ��*/
    VOS_UINT16                                  usLAC;                  /*λ������*/
    MTA_NETMON_UTRAN_TYPE_ENUM_UINT32           enCellMeasTypeChoice;   /* NETMONƵ����Ϣ����:FDD,TDD */
    union
    {
        MTA_NETMON_UTRAN_SCELL_INFO_FDD_STRU    stCellMeasResultsFDD;    /* FDD */
        MTA_NETMON_UTRAN_SCELL_INFO_TDD_STRU    stCellMeasResultsTDD;    /*  TDD */
    }u;
}RRC_MTA_NETMON_UTRAN_SCELL_INFO_STRU;



typedef struct
{
    VOS_MSG_HEADER                                              /* ��Ϣͷ*/     /*_H2ASN_Skip*/
    VOS_UINT32                                ulMsgName;        /*��Ϣ����*/    /*_H2ASN_Skip*/
    MTA_NETMON_RESULT_ENUM_UINT32             enResult;
    MTA_NETMON_CELL_TYPE_ENUM_UINT32          enCelltype;       /*0:��ѯ����С����1:��ѯ����*/
    VOS_UINT32                                ulQryModule;      /* ��ǰ��ѯ��ģ����Ϣ */
    union
    {
        RRC_MTA_NETMON_UTRAN_SCELL_INFO_STRU  stSCellinfo;      /* UTRAN�µķ���С����Ϣ */
        RRC_MTA_NETMON_NCELL_INFO_STRU        stNCellinfo;      /* UTRAN�µ�������Ϣ */
    }u;
}RRC_MTA_NETMON_CELL_INFO_QRY_CNF_STRU;

/*LTE ����С���ṹ*/

typedef struct
{
    VOS_UINT32                                  ulMcc;                  /*�ƶ�������*/
    VOS_UINT32                                  ulMnc;                  /*�ƶ�������*/
    VOS_UINT32                                  ulCellID;               /*С��ID*/
    VOS_UINT32                                  ulPID;                  /*����С��ID*/
    VOS_UINT16                                  usArfcn;                /*Ƶ��*/
    VOS_UINT16                                  usTAC;
    VOS_INT16                                   sRSRP;
    VOS_INT16                                   sRSRQ;
    VOS_INT16                                   sRSSI;
    VOS_UINT8                                   aucReserved[2];
} MTA_NETMON_EUTRAN_SCELL_INFO_STRU;


typedef struct
{
    VOS_MSG_HEADER                                  /* ��Ϣͷ */                /*_H2ASN_Skip*/
    VOS_UINT32                          ulMsgName;   /* ��Ϣ���� */             /*_H2ASN_Skip*/
    MTA_NETMON_CELL_TYPE_ENUM_UINT32    enCelltype;  /* 0����ѯ����С����1����ѯ���� */
    VOS_UINT32                          ulQryModule; /* ��ǰ��ѯ��ģ����Ϣ */
}MTA_LRRC_NETMON_CELL_QRY_REQ_STRU;


typedef struct
{
    VOS_MSG_HEADER                                              /* ��Ϣͷ*/     /*_H2ASN_Skip*/
    VOS_UINT32                                ulMsgName;        /*��Ϣ����*/    /*_H2ASN_Skip*/
    MTA_NETMON_RESULT_ENUM_UINT32             enResult;
    MTA_NETMON_CELL_TYPE_ENUM_UINT32          enCelltype;       /*0:��ѯ����С����1:��ѯ����*/
    VOS_UINT32                                ulQryModule;      /* ��ǰ��ѯ��ģ����Ϣ */
    union
    {
        MTA_NETMON_EUTRAN_SCELL_INFO_STRU     stSCellinfo;      /* LTE�µķ���С����Ϣ */
        RRC_MTA_NETMON_NCELL_INFO_STRU        stNCellinfo;      /* LTE�µ�������Ϣ */
    }u;
}LRRC_MTA_NETMON_CELL_INFO_QRY_CNF_STRU;


/*****************************************************************************
  10 ��������
*****************************************************************************/

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

#endif /* end of MtaCommInterface.h */
