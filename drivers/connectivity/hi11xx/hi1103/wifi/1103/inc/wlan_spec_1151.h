

#ifndef __WLAN_SPEC_1151_H__
#define __WLAN_SPEC_1151_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  1 ����ͷ�ļ�����
*****************************************************************************/


/*****************************************************************************
  0.0 ���ƻ���������
*****************************************************************************/

/*TBD����ʼ������Dmac_main��*/

/*****************************************************************************
  0.1.4 �ȵ���������
*****************************************************************************/
/* ��ΪP2P GO �����������û�������1151 �в�ʹ�ã�������ΪUT ����ʹ�� */
#define WLAN_P2P_GO_ASSOC_USER_MAX_NUM_SPEC 0
/* оƬ�汾���� */
#define BOARD_VERSION                       0

/*****************************************************************************
  0.5.3 AMSDU����
*****************************************************************************/
#ifdef _PRE_WIFI_DMT
#define WLAN_AMSDU_MAX_NUM                  witp_dmt_get_amsdu_aggr_num()
#else
/* һ��amsdu������ӵ�е�msdu�������� */
#define WLAN_AMSDU_MAX_NUM                  25
#endif

/*****************************************************************************
  1.0 WLANоƬ��Ӧ��spec
*****************************************************************************/
/* ÿ��board֧��chip������������ƽ̨ */
/* ÿ��chip֧��device������������ƽ̨ */
/* ���֧�ֵ�MACӲ���豸��������ƽ̨ */
/* ����BOARD֧�ֵ�����device��Ŀ����ƽ̨ */

/*****************************************************************************
  1.3 oam��ص�spec
*****************************************************************************/
/*oam ����ƽ̨ */

/*****************************************************************************
  1.4 mem��Ӧ��spec
*****************************************************************************/
/*****************************************************************************
  1.4.1 �ڴ�ع��
*****************************************************************************/
/*�ڴ� spec ����ƽ̨ */

/*****************************************************************************
  1.4.2 �����������ڴ��������Ϣ
*****************************************************************************/
/*�ڴ� spec ����ƽ̨ */

/*****************************************************************************
  1.4.3 �������֡�ڴ��������Ϣ
*****************************************************************************/
/*�ڴ� spec ����ƽ̨ */


/*****************************************************************************
  1.4.4 ��������֡�ڴ��������Ϣ
*****************************************************************************/
/*�ڴ� spec ����ƽ̨ */

/*****************************************************************************
  1.4.5 �����ڴ��������Ϣ
*****************************************************************************/
/*�ڴ� spec ����ƽ̨ */

/*****************************************************************************
  1.4.6 �¼��ṹ���ڴ��
*****************************************************************************/
/*�ڴ� spec ����ƽ̨ */

/*****************************************************************************
  1.4.7 �û��ڴ��
*****************************************************************************/
/*****************************************************************************
  1.4.8 MIB�ڴ��  TBD :���ո��ӳصĿռ��С��������Ҫ���¿���
*****************************************************************************/
/*�ڴ� spec ����ƽ̨ */

/*****************************************************************************
  1.4.9 netbuf�ڴ��  TBD :���ո��ӳصĿռ��С��������Ҫ���¿���
*****************************************************************************/

/*�ڴ� spec ����ƽ̨ */


/*****************************************************************************
  1.4.9.1 sdt netbuf�ڴ��
*****************************************************************************/
/*�ڴ� spec ����ƽ̨ */


/*****************************************************************************
  1.5 frw��ص�spec
*****************************************************************************/
/*�¼����� spec ����ƽ̨ */


/*****************************************************************************
  2 �궨�壬�����DR����һ��
*****************************************************************************/
/*****************************************************************************
  2.1 ����Э��/���������Э������spec
*****************************************************************************/
/*****************************************************************************
  2.1.1 ɨ���STA ����
*****************************************************************************/
/* TBD һ�ο���ɨ������BSS���������������Ժϲ�*/
#define WLAN_SCAN_REQ_MAX_BSSID                 2
#define WLAN_SCAN_REQ_MAX_SSID                  8

/* TBD ɨ���ӳ�,us δʹ�ã���ɾ��*/
#define WLAN_PROBE_DELAY_TIME                   10

/* ɨ��ʱ����С���ŵ�פ��ʱ�䣬��λms��������������*/
#define WLAN_DEFAULT_SCAN_MIN_TIME              110
/* ɨ��ʱ�������ŵ�פ��ʱ�䣬��λms��������������*/
#define WLAN_DEFAULT_SCAN_MAX_TIME              500

/* һ��device����¼��ɨ�赽�����BSS����*/
#define WLAN_MAX_SCAN_BSS_NUM                   32
/* һ���ŵ��¼�¼ɨ�赽�����BSS���� */
#define WLAN_MAX_SCAN_BSS_PER_CH                8

#define WLAN_DEFAULT_FG_SCAN_COUNT_PER_CHANNEL         2       /* ǰ��ɨ��ÿ�ŵ�ɨ����� */
#define WLAN_DEFAULT_BG_SCAN_COUNT_PER_CHANNEL         1       /* ����ɨ��ÿ�ŵ�ɨ����� */
#define WLAN_DEFAULT_SEND_PROBE_REQ_COUNT_PER_CHANNEL  1       /* ÿ���ŵ�ɨ�跢��probe req֡�Ĵ��� */

#define WLAN_DEFAULT_MAX_TIME_PER_SCAN                4500     /* ɨ���Ĭ�ϵ����ִ��ʱ�䣬����С��wal���ɨ�賬ʱʱ��5s������ */

/* Beacon Interval���� */
#define WLAN_BEACON_INTVAL_MAX              3500            /* max beacon interval, ms */
#define WLAN_BEACON_INTVAL_MIN              20              /* modify from 40 to meet 51v2 product request--min beacon interval */
#define WLAN_BEACON_INTVAL_DEFAULT          100             /* min beacon interval */
/*AP IDLE״̬��beacon intervalֵ*/
#define WLAN_BEACON_INTVAL_IDLE             1000

/*TBD ɨ��ʱ��������ɨ�趨ʱʱ�䣬��λms��������������*/
#ifdef _PRE_WIFI_DMT
#define WLAN_DEFAULT_ACTIVE_SCAN_TIME           40
#define WLAN_DEFAULT_PASSIVE_SCAN_TIME          800
#else
#define WLAN_DEFAULT_ACTIVE_SCAN_TIME           20
#define WLAN_DEFAULT_PASSIVE_SCAN_TIME          60
#endif

#define WLAN_NEIGHBOR_SCAN_TIME                 (120)

#define WLAN_LONG_ACTIVE_SCAN_TIME              40             /* ָ��SSIDɨ���������3��ʱ,1��ɨ�賬ʱʱ��Ϊ40ms */

/*****************************************************************************
  2.1.3 STA��������
*****************************************************************************/
/* STA��ͬʱ���������AP����*/
#define WLAN_ASSOC_AP_MAX_NUM               2

/* TBD�������ӳ٣���λms��������Ҫ�޶�*/
#ifdef _PRE_WIFI_DMT
#define WLAN_JOIN_START_TIMEOUT                 10000
#define WLAN_AUTH_TIMEOUT                       500
#define WLAN_ASSOC_TIMEOUT                      500
#else
#define WLAN_JOIN_START_TIMEOUT                 10000
#define WLAN_AUTH_TIMEOUT                       300
#define WLAN_ASSOC_TIMEOUT                      300
#endif


#define WLAN_LINKLOSS_OFFSET_11H                5  /* ���ŵ�ʱ���ӳ� */

/*****************************************************************************
  2.1.4 �ȵ���������
*****************************************************************************/
/*
 * The 802.11 spec says at most 2007 stations may be
 * associated at once.  For most AP's this is way more
 * than is feasible so we use a default of 128.  This
 * number may be overridden by the driver and/or by
 * user configuration.
 */
#define WLAN_AID_MAX                        2007
#define WLAN_AID_DEFAULT                    128


/* �û�����ʱ����*/
/* ��Ծ��ʱ���������� */
#define WLAN_USER_ACTIVE_TRIGGER_TIME           1000
/* �ϻ���ʱ���������� */
#define WLAN_USER_AGING_TRIGGER_TIME            5000
/* ��λms */
#define WLAN_USER_ACTIVE_TO_INACTIVE_TIME       5000

#ifdef _PRE_WIFI_DMT
#define WLAN_AP_USER_AGING_TIME                  witp_dmt_get_user_aging_time()
#else
/* ��λms */
#define WLAN_AP_USER_AGING_TIME                 (300 * 1000)    /* AP �û��ϻ�ʱ�� 300S */
#define WLAN_P2PGO_USER_AGING_TIME              (30 * 1000)     /* 30S */
#endif

/* AP keepalive����,��λms */
/*sta keepalive ��ʱʵ��ʱ��ԭ����30*1000��Ϊ1*1000,ʵ�ֺ�Ļ�*/
#define WLAN_AP_KEEPALIVE_TRIGGER_TIME          (15 * 1000)       /* keepalive��ʱ���������� */
#define WLAN_AP_KEEPALIVE_INTERVAL              (WLAN_AP_KEEPALIVE_TRIGGER_TIME * 4)  /* ap����keepalive null֡��� */
#define WLAN_GO_KEEPALIVE_INTERVAL              (15*1000) /* P2P GO����keepalive null֡���  */

#define WLAN_STA_KEEPALIVE_INTERVAL (45*1000) /* STA����null֡���ڣ�E5��Ʒ���Ϊ45s */
#define WLAN_STA_KEEALIVE_CNT_TH (WLAN_STA_KEEPALIVE_INTERVAL/WLAN_AP_KEEPALIVE_TRIGGER_TIME)

#if WLAN_STA_KEEPALIVE_INTERVAL % WLAN_AP_KEEPALIVE_TRIGGER_TIME
#error "invalid interval setting"
#endif



#ifdef _PRE_WLAN_REALTIME_CALI
/* realtime calibration����, ��λms */
#define WLAN_REALTIME_CALI_INTERVAL_INIT        (26)      /* �����ŵ����ʼ500��У׼���� */
#define WLAN_REALTIME_CALI_INTERVAL             (1010)    /* ��������Ϊdbac���ڵ�����������Ҫ������֤2g��5gC0C1���õ���̬У׼�Ļ��� */
#endif

/*****************************************************************************
  2.1.6 ����ģʽ����
*****************************************************************************/
/*TBD RTS�������ޣ�ʵ�ʿ�ɾ��*/
#define WLAN_RTS_DEFAULT                    512
#define WLAN_RTS_MIN                        1
#define WLAN_RTS_MAX                        2346

/*****************************************************************************
  2.1.7 ��Ƭ����
*****************************************************************************/
/* Fragmentation limits */
/* default frag threshold */
#define WLAN_FRAG_THRESHOLD_DEFAULT         1544
/* min frag threshold */
#define WLAN_FRAG_THRESHOLD_MIN             200 /* Ϊ�˱�֤��Ƭ��С��16,����AP Ĭ��500  */
/* max frag threshold */
#define WLAN_FRAG_THRESHOLD_MAX             2346


/*****************************************************************************
  2.1.17 �������ʹ���
*****************************************************************************/
/* ������ز��� */

/* ��¼֧�ֵ����� */
#define WLAN_SUPP_RATES                         8
/* ���ڼ�¼ɨ�赽��ap֧�ֵ����������� */
#define WLAN_MAX_SUPP_RATES                     12

/* ÿ���û�֧�ֵ�������ʼ����� */
#define HAL_TX_RATE_MAX_NUM               4

/* HAL ������֧������ʹ��� */
#define HAL_TX_RATE_MAX_CNT                3

/*****************************************************************************
  2.2 ����Э��/����MAC ��Э������spec
*****************************************************************************/
/*****************************************************************************
  2.2.2 ������STA����
*****************************************************************************/
/* ������������ */
#define WLAN_MAX_RC_NUM                         20
/* ������λͼ���� */
#define WLAN_RC_BMAP_WORDS                      2

/*****************************************************************************
  2.2.4 WMM����
*****************************************************************************/
/*STA����WLAN_EDCA_XXX����ͬWLAN_QEDCA_XXX */
/* ����UT���̺�ɾ�� */
#define WLAN_EDCA_TABLE_CWMIN_MIN           0
#define WLAN_EDCA_TABLE_CWMIN_MAX           15
#define WLAN_EDCA_TABLE_CWMAX_MIN           0
#define WLAN_EDCA_TABLE_CWMAX_MAX           15
#define WLAN_EDCA_TABLE_AIFSN_MIN           0
#define WLAN_EDCA_TABLE_AIFSN_MAX           15
#define WLAN_EDCA_TABLE_TXOP_LIMIT_MIN      1
#define WLAN_EDCA_TABLE_TXOP_LIMIT_MAX      65535
#define WLAN_EDCA_TABLE_MSDU_LIFETIME_MAX   500

/* EDCA���� */
#define WLAN_QEDCA_TABLE_INDEX_MIN           1
#define WLAN_QEDCA_TABLE_INDEX_MAX           4
#define WLAN_QEDCA_TABLE_CWMIN_MIN           0
#define WLAN_QEDCA_TABLE_CWMIN_MAX           15
#define WLAN_QEDCA_TABLE_CWMAX_MIN           0
#define WLAN_QEDCA_TABLE_CWMAX_MAX           15
#define WLAN_QEDCA_TABLE_AIFSN_MIN           0
#define WLAN_QEDCA_TABLE_AIFSN_MAX           15
#define WLAN_QEDCA_TABLE_TXOP_LIMIT_MIN      1
#define WLAN_QEDCA_TABLE_TXOP_LIMIT_MAX      65535
#define WLAN_QEDCA_TABLE_MSDU_LIFETIME_MAX   500

/* TID������ͼƽ̨SPEC */

/* Ĭ�ϵ���������ҵ���TID */
#define WLAN_TID_FOR_DATA                   0
/* ���ն��еĸ��� ��HAL_RX_DSCR_QUEUE_ID_BUTT��� */
#define HAL_RX_QUEUE_NUM                3
/* ���Ͷ��еĸ��� */
#define HAL_TX_QUEUE_NUM                5
/* �洢Ӳ�������ϱ���������������Ŀ(ping pongʹ��) */
#define HAL_HW_RX_DSCR_LIST_NUM         1

/*****************************************************************************
  2.3 У׼����spec
*****************************************************************************/
/* ֧���ֶ����÷�Ƶϵ���ĸ��� */
#ifdef _PRE_WLAN_PHY_PLL_DIV
#define WITP_RF_SUPP_NUMS                  4
#endif

/*****************************************************************************
  2.4 ��ȫЭ������spec
*****************************************************************************/
/*****************************************************************************
  2.4.1 WPA����
*****************************************************************************/
/* 11i������WLAN_WPA_KEY_LEN �� WLAN_WPA_SEQ_LEN��Ϊƽ̨�ṹ���Ա������ƽ̨*/

/* ������صĺ궨�� */
/* Ӳ��MAC ���ȴ�32us�� ����ȴ�40us */
#define HAL_CE_LUT_UPDATE_TIMEOUT          4


/*****************************************************************************
  2.4.4 PMF ����
*****************************************************************************/
/* SA Query���̼��ʱ��,�ϻ�ʱ�������֮һ*/
/*#define WLAN_SA_QUERY_RETRY_TIME                (WLAN_AP_USER_AGING_TIME / 3)*/
#define WLAN_SA_QUERY_RETRY_TIME                201

/* SA Query���̳�ʱʱ��,С���ϻ�ʱ��*/
/*#define WLAN_SA_QUERY_MAXIMUM_TIME              (WLAN_SA_QUERY_RETRY_TIME * 3)*/
#define WLAN_SA_QUERY_MAXIMUM_TIME              1000

/*****************************************************************************
  2.4.6 �Զ��尲ȫ
*****************************************************************************/
#define WLAN_BLACKLIST_MAX      WLAN_ASSOC_USER_MAX_NUM    /* �����ڰ��������������û��� */

/*****************************************************************************
  2.5 ��������spec
*****************************************************************************/
/*****************************************************************************
  2.5.1 ��ȷ�Ϲ���
*****************************************************************************/
/* ֧�ֵĽ���ba���������� */
#define WLAN_MAX_RX_BA                      32

/* ֧�ֵķ���ba���������� */
#define WLAN_MAX_TX_BA                      32


#ifdef _PRE_WIFI_DMT
/* ��RSP��һ�£��������ĳ��������ʱʱ�� */
#define WLAN_ADDBA_TIMEOUT                      10000
#define WLAN_TX_PROT_TIMEOUT                    60000
#else
#define WLAN_ADDBA_TIMEOUT                      500
#define WLAN_TX_PROT_TIMEOUT                    6000
#endif

#ifdef _PRE_WLAN_FEATURE_PROXYSTA
#define HAL_PROXYSTA_MAX_BA_LUT_SIZE    16
#endif

/*****************************************************************************
  2.5.2 AMPDU����
*****************************************************************************/
#define WLAN_AMPDU_RX_BUFFER_SIZE       64  /* AMPDU���ն˽��ջ�������buffer size�Ĵ�С */
#define WLAN_AMPDU_RX_BA_LUT_WSIZE      64  /* AMPDU���ն�������дBA RX LUT���win size,
                                               Ҫ����ڵ���WLAN_AMPDU_RX_BUFFER_SIZE */
#define WLAN_AMPDU_TX_MAX_NUM           64  /* AMPDU���Ͷ����ۺ���MPDU���������Ͷ˵�buffer size */
#define WLAN_AMPDU_TX_MAX_BUF_SIZE      64  /* ���Ͷ˵�buffer size */

#define WLAN_AMPDU_TX_SCHD_STRATEGY     1   /* 51 ���ۺϸ���������󴰿� */

#define WLAN_AMPDU_MIN_START_SPACING    5   /* 1151�޶�ampdu��֡��С���ʱ��Ϊ5us */

#ifdef _PRE_WLAN_FEATURE_RX_AGGR_EXTEND
#define HAL_MAX_RX_BA_LUT_SIZE             40
#else
#define HAL_MAX_RX_BA_LUT_SIZE             32
#endif

#define HAL_MAX_TX_BA_LUT_SIZE             128
/*****************************************************************************
  2.5.3 AMSDU����
*****************************************************************************/
#define AMSDU_ENABLE_ALL_TID                0xFF
/* amsdu����msdu����󳤶� */
#define WLAN_MSDU_MAX_LEN                   1600
/* 1151 spec amsdu��󳤶� */
#define WLAN_AMSDU_FRAME_MAX_LEN            7935

/* >= WLAN_AMSDU_MAX_NUM/2  */
#define WLAN_DSCR_SUBTABEL_MAX_NUM          13

/*****************************************************************************
  2.5.6 Memory����
*****************************************************************************/
/*HAL ��ͷ���޶�ΪWLAN*/
/* ���������������ĺ궨�� */
/* ��ͨ���ȼ����������������������������� 32*2*3(amsduռ��buf����Ŀ) */
#if (_PRE_TARGET_PRODUCT_TYPE_ONT == _PRE_CONFIG_TARGET_PRODUCT)
#define HAL_NORMAL_RX_MAX_BUFFS           1024
#else
#define HAL_NORMAL_RX_MAX_BUFFS           512
#endif
/* �����ȼ�����������������������������:ȡ���ڲ����û���(64) */
#define HAL_HIGH_RX_MAX_BUFFS             64
#define HAL_RX_MAX_BUFFS                 (HAL_NORMAL_RX_MAX_BUFFS + HAL_HIGH_RX_MAX_BUFFS)
/*������������ping pong����*/
#define HAL_HW_MAX_RX_DSCR_LIST_IDX       1
/* ���ڴ洢��������ж������� */
#define HAL_HW_RX_ISR_INFO_MAX_NUMS       HAL_NORMAL_RX_MAX_BUFFS
#define HAL_DOWM_PART_RX_TRACK_MEM        200
#ifdef _PRE_DEBUG_MODE
/* ������������13�����ڴ�ʱ����������� */
#define HAL_DEBUG_RX_DSCR_LINE            13
#endif


#ifdef _PRE_WLAN_PHY_PERFORMANCE
/* PHY���ܲ���ʹ��֡���� */
    #define HAL_RX_FRAME_LEN              5100
    #define HAL_RX_FRAME_MAX_LEN          8000
#else
/* 80211֡��󳤶�:������Ϊ1600����20�ֽڵ���������ֹӲ������Խ�� */
    #define HAL_RX_FRAME_LEN              1580
    #define HAL_RX_FRAME_MAX_LEN          8000
#endif

#ifdef _PRE_WLAN_PRODUCT_1151V200
    #define HAL_AL_RX_FRAME_LEN               18000
#else
    #define HAL_AL_RX_FRAME_LEN               5100
#endif

/*****************************************************************************
  2.5.7 DEVICE �����ж�������
*****************************************************************************/
#define WLAN_RX_INTERRUPT_MAX_NUM_PER_DEVICE  8          /* һ��device���һ�δ���4��rx�ж� */

/*****************************************************************************
  2.6 �㷨����spec
*****************************************************************************/
/*****************************************************************************
  2.6.1 Autorate
*****************************************************************************/
/* Autorate 1151��֧��VO�ۺ� */
#define WLAN_AUTORATE_VO_AGGR_SUPPORT       1
/* Autorate ��С�ۺ�ʱ������ */
#define WLAN_AUTORATE_MIN_AGGR_TIME_IDX     0
/* ÿ�����ʵȼ���ƽ���ش����� */
#define ALG_AUTORATE_AVG_RATE_RETRY_NUM     3

/*****************************************************************************
  2.6.2 �㷨����ģʽ����ASIC��FPGA
*****************************************************************************/
#if (_PRE_WLAN_CHIP_ASIC != _PRE_WLAN_CHIP_VERSION)
#define WLAN_CCA_OPT_WORK_MODE       WLAN_CCA_OPT_DISABLE         //cca_opt
#define WLAN_EDCA_OPT_MODE_STA       WLAN_EDCA_OPT_STA_DISABLE    //edca_opt
#define WLAN_EDCA_OPT_MODE_AP        WLAN_EDCA_OPT_AP_EN_DISABLE
#define WLAN_TPC_WORK_MODE           WLAN_TPC_WORK_MODE_DISABLE   //TPC
#define WLAN_ANTI_INTF_WORK_MODE     WLAN_ANTI_INTF_EN_OFF        //weak intf opt

#else
#define WLAN_CCA_OPT_WORK_MODE       WLAN_CCA_OPT_ENABLE
#define WLAN_EDCA_OPT_MODE_STA       WLAN_EDCA_OPT_STA_DISABLE
#define WLAN_EDCA_OPT_MODE_AP        WLAN_EDCA_OPT_AP_EN_WITH_COCH
#define WLAN_TPC_WORK_MODE           WLAN_TPC_WORK_MODE_ENABLE
#define WLAN_ANTI_INTF_WORK_MODE     WLAN_ANTI_INTF_EN_PROBE

#endif

/*****************************************************************************
  2.6.3 edca
*****************************************************************************/
/* ecca������Ϣͳ�Ƶ�ģʽ */
#define WLAN_EDCA_STAT_MODE       0
/* ͬƵ������Ϣͳ�Ƶ�ģʽ */
#define WLAN_INTF_DET_COCH_MODE   0
/* ������̽�⹤��ģʽ */
#define WLAN_NEG_DET_WORK_MODE    0

/*****************************************************************************
  2.6.6 TXBF����
*****************************************************************************/
#define WLAN_MU_BFEE_ACTIVED      WLAN_MU_BFEE_DISABLE

/*****************************************************************************
  2.6.7 dbac
*****************************************************************************/
#define CFG_DBAC_TIMER_IDX                  HI5115_TIMER_INDEX_0

/*****************************************************************************
  2.6.8 schedule
*****************************************************************************/
#define ALG_SCH_MEANS_NUM   ALG_SCH_WMM_ENSURE
#define WLAN_TX_QUEUE_UAPSD_DEPTH    6

/*****************************************************************************
  2.6.15 TPC����
*****************************************************************************/
/* TPC��ز��� */
/* TPC����DB�� */
#define WLAN_TPC_STEP                       3
/* ����书�ʣ���λdBm */
#define WLAN_MAX_TXPOWER                    30



/*****************************************************************************
  2.7 ���ܵ͹�������spec
*****************************************************************************/
/*****************************************************************************
  2.7.1 оƬSleep״̬
*****************************************************************************/
/* default PCIE LO IDLETIME 1us */
#define WLAN_PCIE_L0_DEFAULT                8
/* default PCIE LO IDLETIME 1us */
#define WLAN_PCIE_L1_DEFAULT                32

/*****************************************************************************
  2.7.4 Э�����STA�๦��
*****************************************************************************/
/*PSM���Թ��*/
/* default DTIM period */
#define WLAN_DTIM_DEFAULT                   3

/* DTIM Period���� */
/* beacon interval�ı��� */
#define WLAN_DTIM_PERIOD_MAX                255
#define WLAN_DTIM_PERIOD_MIN                1

/*****************************************************************************
  2.7.5 Э�����AP�๦��
*****************************************************************************/



/*****************************************************************************
  2.8 �ܹ���̬����spec
*****************************************************************************/
/*****************************************************************************
  2.8.4  WiFiоƬ�����
*****************************************************************************/
/* оƬ���ռ�����Ŀ */
#define WLAN_MAX_NSS_NUM           (WLAN_DOUBLE_NSS)

/* ͨ���� */
#define WLAN_RF_CHANNEL_NUMS   2

/*****************************************************************************
  2.8.2 HAL Device0оƬ������
*****************************************************************************/
/* HAL DEV0֧�ֵĿռ����� */
#define WLAN_HAL0_NSS_NUM           WLAN_DOUBLE_NSS

/* HAL DEV0֧�ֵ������� */
#define WLAN_HAL0_BW_MAX_WIDTH      WLAN_BW_CAP_80M

/*HAL DEV0 ֧��SOUNDING���� */
#define WLAN_HAL0_11N_SOUNDING      OAL_TRUE

/*HAL DEV0 ֧��Green Field���� */
#define WLAN_HAL0_GREEN_FIELD       OAL_TRUE

/* HAL DEV0�Ƿ�֧��խ�� */
#define WLAN_HAL0_NB_IS_EN          OAL_FALSE

/* HAL DEV0�Ƿ�֧��1024QAM */
#define WLAN_HAL0_1024QAM_IS_EN     OAL_FALSE

/* HAL DEV0��SU_BFEE���� */
#define WLAN_HAL0_SU_BFEE_NUM       2

/* HAL DEV0�Ƿ�֧��11MC */
#define WLAN_HAL0_11MC_IS_EN        OAL_FALSE

/* HAL DEV0�Ƿ�֧��LDPC */
#define WLAN_HAL0_LDPC_IS_EN        OAL_TRUE

/* HAL DEV0��ͨ��ѡ�� */
#define WLAN_HAL0_PHY_CHAIN_SEL      WLAN_PHY_CHAIN_DOUBLE

/* HAL DEV0����Ҫ�õ����߷���11b��֡ʱ��TXͨ��ѡ�� */
#define WLAN_HAL0_SNGL_TX_CHAIN_SEL WLAN_TX_CHAIN_ZERO

/* HAL DEV0��RFͨ��ѡ�� */
#define WLAN_HAL0_RF_CHAIN_SEL      WLAN_RF_CHAIN_DOUBLE

/* HAL DEV0�Ƿ�support tx stbc */
#define WLAN_HAL0_TX_STBC_IS_EN     OAL_TRUE

/* HAL DEV0�Ƿ�support rx stbc */
#define WLAN_HAL0_RX_STBC_IS_EN     OAL_TRUE

/* HAL DEV0�Ƿ�support 2.4g dpd */
#define WLAN_HAL0_DPD_2G_IS_EN      OAL_FALSE

/* HAL DEV0�Ƿ�support 5g dpd */
#define WLAN_HAL0_DPD_5G_IS_EN      OAL_FALSE

/* HAL DEV0�Ƿ�support su txbfer */
#define WLAN_HAL0_SU_BFER_IS_EN     OAL_TRUE

/* HAL DEV0�Ƿ�support su txbfee */
#define WLAN_HAL0_SU_BFEE_IS_EN     OAL_TRUE

/* HAL DEV0�Ƿ�support mu txbfer */
#define WLAN_HAL0_MU_BFER_IS_EN     OAL_FALSE

/* HAL DEV0�Ƿ�support mu txbfee */
#define WLAN_HAL0_MU_BFEE_IS_EN     OAL_FALSE

/* HAL DEV0�Ƿ�support 11n txbf */
#define WLAN_HAL0_11N_TXBF_IS_EN   OAL_TRUE

/*HAL DEV0 �Ƿ�֧��11AX*/
#define WLAN_HAL0_11AX_IS_EN        OAL_FALSE

/*HAL DEV0 �Ƿ�֧��dpd*/
#define WLAN_HAL0_DPD_IS_EN         OAL_TRUE

/*HAL DEV0 �Ƿ�֧��radar detector*/
#define WLAN_HAL0_RADAR_DETECTOR_IS_EN      OAL_TRUE

/* HAL DEV0�Ƿ�֧��160M��80+80M��short GI */
#define WLAN_HAL0_VHT_SGI_SUPP_160_80P80    OAL_FALSE

/*HAL DEV0֧��TXOP PS*/
#define WLAN_HAL0_TXOPPS_IS_EN        OAL_FALSE
#define WLAN_HAL0_CONTROL_FRM_TX_DOUBLE_CHAIN_FLAG    OAL_FALSE


/*оƬ�汾�ѷ���ƽ̨�Ķ��ƻ�*/


/*****************************************************************************
  2.8.7  Proxy STA����
*****************************************************************************/
/* ÿ��DEVICE֧�ֵ����ҵ��VAP��Ŀ: ����һ��4��AP VAP����������1��AP VAP + 1��STA VAP */
#if (defined(_PRE_WLAN_FEATURE_PROXYSTA) || defined(_PRE_WLAN_PRODUCT_1151V200))
#define WLAN_STA0_HAL_VAP_ID                    4
#endif
#ifdef _PRE_WLAN_FEATURE_PROXYSTA
#define WLAN_MAX_PROXY_STA_NUM                  15
//#define WLAN_STA0_HAL_VAP_ID                    4
#ifdef _PRE_WLAN_PRODUCT_1151V200
#define WLAN_PROXY_STA_START_ID                 48
#define WLAN_PROXY_STA_END_ID                   63
#else
#define WLAN_PROXY_STA_START_ID                 16
#define WLAN_PROXY_STA_END_ID                   31
#endif
#define WLAN_PROXY_STA_MAX_REP                  2
#endif

#ifdef _PRE_WLAN_FEATURE_SINGLE_PROXYSTA
#define WLAN_SINGLE_PROXY_STA_NUM_PER_DEVICE             1   /* SINGLE PROXY STA�ĸ��� */
#endif


/*****************************************************************************
  2.8.8  ��AP����
*****************************************************************************/
#if 0
#define WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE     4
#endif
 /* AP VAP�Ĺ��STA VAP�Ĺ��STA P2P����Ĺ�����ƽ̨*/

/* PROXY STAģʽ��VAP���궨�����ƽ̨ */
//#define WLAN_VAP_MAX_NUM_PER_DEVICE          (WLAN_SERVICE_VAP_MAX_NUM_PER_DEVICE + 1)

#ifdef _PRE_WLAN_PRODUCT_1151V200
#define WLAN_HAL_OHTER_BSS_ID                   6     /* ����BSS��ID */
#else
#define WLAN_HAL_OHTER_BSS_ID                   5     /* ����BSS��ID */
#endif /* end _PRE_WLAN_CHIP_FPGA_V200 */
#define WLAN_HAL_OTHER_BSS_UCAST_ID             7     /* ��������bss�ĵ�������֡������֡��ά���� */
#ifdef _PRE_WLAN_PRODUCT_1151V200
#define WLAN_VAP_MAX_ID_PER_DEVICE_LIMIT        64    /* hal�㣬0-3 16��AP��4-5 2��STA 16-27 12��AP 28 1��STA 48-63 PROXY STA */
#else
#define WLAN_VAP_MAX_ID_PER_DEVICE_LIMIT        31    /* hal�㣬0-3 4��AP��4 1��STA 16-30 PROXY STA */
#endif /* end _PRE_WLAN_CHIP_FPGA_V200 */


/* ����BOARD֧�ֵ�����VAP��Ŀ�ѷ���ƽ̨ */

/* ����BOARD֧�ֵ����ҵ��VAP����Ŀ �ѷ���ƽ̨*/

#ifdef _PRE_WLAN_PRODUCT_1151V200
/* MAC�ϱ���tbtt�ж�������ֵ��16��ap��tbtt�ж�+2��sta��tbtt�ж�(������STA���漰TBTT�ж�) */
#define WLAN_MAC_REPORT_TBTT_IRQ_MAX           18
#else
/* MAC�ϱ���tbtt�ж�������ֵ��4��ap��tbtt�ж�+1��sta��tbtt�ж� */
#define WLAN_MAC_REPORT_TBTT_IRQ_MAX           5
#endif /* end _PRE_WLAN_CHIP_FPGA_V200 */

/*****************************************************************************
  2.8.11  ��������
*****************************************************************************/
/* ����Ĭ�Ͽ����رն���*/
/*TBD Feature��̬��ǰδʹ�ã�����������û��ʹ��*/
#define WLAN_FEATURE_AMPDU_IS_OPEN              OAL_TRUE
#define WLAN_FEATURE_AMSDU_IS_OPEN              OAL_TRUE
//#define WLAN_FEATURE_WME_IS_OPEN                OAL_TRUE
#define WLAN_FEATURE_DSSS_CCK_IS_OPEN           OAL_TRUE
#define WLAN_FEATURE_UAPSD_IS_OPEN              OAL_TRUE
//#define WLAN_FEATURE_PSM_IS_OPEN                OAL_TRUE
#define WLAN_FEATURE_WPA_IS_OPEN                OAL_TRUE
#define WLAN_FEATURE_TXBF_IS_OPEN               OAL_TRUE
//#define WLAN_FEATURE_MSDU_DEFRAG_IS_OPEN        OAL_TRUE

/*****************************************************************************
  2.8.20  DBAC
*****************************************************************************/
/*  ��ٶ��и�������������һ���ŵ�ʱ����ԭ�ŵ��Ϸŵ�Ӳ���������֡��������
��ǰֻ��2������: DBAC �� ����ɨ�� DBACռ��2�����У����0 1; ����ɨ��ռ��һ�������2 */
/*HAL ��ͷ���޶�ΪWLAN*/
#define HAL_TX_FAKE_QUEUE_NUM              3
#define HAL_TX_FAKE_QUEUE_BGSCAN_ID        2

#define HAL_FCS_PROT_MAX_FRAME_LEN         24

/*****************************************************************************
  2.9 WiFiӦ������spec
*****************************************************************************/
/*****************************************************************************
  2.9.4 P2P����
*****************************************************************************/
/* Hi1102 P2P������P2P vap�豸�Ĵ�С����(PER_DEVICE) */
/* P2Pֻ��1102�¶��壬�˴�������Ϊ��1151UT��׮ */
#ifdef _PRE_WLAN_FEATURE_P2P
#define WLAN_MAX_SERVICE_P2P_DEV_NUM          1
#define WLAN_MAX_SERVICE_P2P_GOCLIENT_NUM     1
#define WLAN_MAX_SERVICE_CFG_VAP_NUM          1
#endif
/*****************************************************************************
  2.10 MAC FRAME����
*****************************************************************************/
/*****************************************************************************
  2.10.1 ht cap info
*****************************************************************************/
#define HT_GREEN_FILED_DEFAULT_VALUE            0
#define HT_TX_STBC_DEFAULT_VALUE                1
#define HT_BFEE_NTX_SUPP_ANTA_NUM              (2)     /* 11n֧�ֵ�bfer����soundingʱ��������߸��� */
/*****************************************************************************
  2.10.2 vht cap info
*****************************************************************************/
#define VHT_TX_STBC_DEFAULT_VALUE               1
#define VHT_BFEE_NTX_SUPP_STS_CAP              (2)     /* Э���б�ʾ������Nsts���� */

/*****************************************************************************
  2.10.3 RSSI
*****************************************************************************/
#define WLAN_NEAR_DISTANCE_RSSI        (-35)             /*Ĭ�Ͻ������ź�����-35dBm*/
#define WLAN_CLOSE_DISTANCE_RSSI       (-25)             /*����ǰ�����ж�����-25dBm*/
#define WLAN_FAR_DISTANCE_RSSI         (-83)             /*Ĭ��Զ�����ź�����-83dBm*/
#define WLAN_NORMAL_DISTANCE_RSSI_UP   (-42)             /*�ź�ǿ��С��-42dBmʱ������Ϊ�ǳ�������*/
#define WLAN_NORMAL_DISTANCE_RSSI_DOWN (-76)             /*�ź�ǿ�ȴ���-76dBmʱ������Ϊ�Ƿǳ�Զ����*/
#define WLAN_NEAR_DISTANCE_IMPROVE_RSSI_UP      (-40)    /*improve 1*1������,Ҫ��������ж�������Ϊ-44dBm*/
#define WLAN_NEAR_DISTANCE_IMPROVE_RSSI_DOWN    (-48)    /*improve 1*1������,Ҫ��������ж�������Ϊ-50dBm*/
#define WLAN_FIX_MAX_POWER_RSSI        (-55)             /*�̶�������ź�����*/
#define WLAN_PHY_EXTLNA_CHGPTDBM_TH    (-40)             /* PHY����LNA��λ�л�����Ϊ-40dBm */
#define WLAN_PHY_EXTLNA_CHGPTDBM_TH_VAL(_en_band, _en_pm_flag) WLAN_PHY_EXTLNA_CHGPTDBM_TH

/*****************************************************************************
  2.10.4 TXBF cap
*****************************************************************************/
#define WLAN_TXBF_BFER_LEGACY_ENABLE            (1)      /* legacy txbf enable */
#define WLAN_TXBF_BFER_LOG_ENABLE               (1)      /* alg txbf log enable */

#define WLAN_TXBF_BFER_MATRIX_BUFFER_SIZE        (660)    /* ÿbuffer unit���Ϊ640byte = 10*2/2*512/8(160M),����ΪsnrֵԤ��10byte */
#define WLAN_TXBF_BFEE_MATRIX_BUFFER_SIZE        (660)

/*****************************************************************************

  2.11 ������ƫ��
*****************************************************************************/
#ifdef _PRE_WLAN_PRODUCT_1151V200
#define WLAN_RX_DSCR_SIZE                       (84)
#else
#define WLAN_RX_DSCR_SIZE                       (60)
#endif

#ifdef _PRE_WLAN_FEATURE_SMARTANT
#define DUAL_ANTENNA_AVAILABLE                      (OAL_SUCC)  //������OAL_SUCCһ��
#define DUAL_ANTENNA_ALG_CLOSE                      (DUAL_ANTENNA_AVAILABLE+1)
#define DUAL_ANTENNA_ALG_INTERRUPT                  (DUAL_ANTENNA_AVAILABLE+2)
#define DUAL_ANTENNA_SWITCH_FAIL                    (DUAL_ANTENNA_AVAILABLE+7)
#endif

/*****************************************************************************
  2.12 ץ������ѭ��buffer��غ�
*****************************************************************************/
#ifdef _PRE_WLAN_FEATURE_PACKET_CAPTURE
#define WLAN_PACKET_CAPTURE_CIRCLE_BUF_DEPTH        (104)
#define WLAN_HAL_EXT_AP11_VAP_ID                    (27)
#endif

/*****************************************************************************
  2.13 ����Ƶ���׼ֵ
*****************************************************************************/
#define WLAN_2G_CENTER_FREQ_BASE                (2407)
#define WLAN_5G_CENTER_FREQ_BASE                (5000)

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* #ifndef __WLAN_SPEC_1151_H__ */




