

#ifndef __SYSNVID_H__
#define __SYSNVID_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/****************************************************************************
  1 ����ͷ�ļ�����
*****************************************************************************/
#include "NvIddef.h"


/*typedef VOS_UINT16  SYS_NV_ID_ENUM_U16;
 */

enum SYS_NV_ID_ENUM
{
/*    0  */      en_NV_Item_IMEI = GU_SYS_NV_ID_MIN,
/*    1  */      en_NV_Auth_Code_ID = GU_SYS_NV_ID_MIN + 1,
/*    2  */      en_NV_Auth_Num_ID,
/*    3  */      en_NV_Ttl_ID,
/*    4  */      en_NV_Resume_Flag,
/*    5  */      en_NV_Calibrated_Time,
/*    6  */      en_NV_Item_Serial_Num,
/*    7  */      en_NV_Item_LED_CONFIG_Data, /* used by mdrv == NV_ID_DRV_LED_CONTROL */
/*    8  */      en_NV_Item_Om_LogFile_Size,
/*    9  */      en_NV_Item_WatchDog_Flag,
/*   10  */      en_NV_Item_MonitorDSp_Flag,
/*   11  */      en_NV_Item_KickDog_Time = 11,
/*   12  */      en_NV_Item_ScheduleWatchDog_Time,
/*   13  */      en_NV_Item_USIM_TEMP_SENSOR_TABLE,
/*   14  */      en_NV_Item_USIM_TEMP_PROTECT,
/*   16  */      en_NV_Item_AGING_TEST_TABLE=16,
/*   18  */      en_NV_Item_TA_Stub=18,
/*   19  */      en_NV_Item_TA_RF_DELAY_VAL,
/*   21  */      en_NV_Item_USB_Enum_Status=21,
/*   22  */      en_NV_Item_LiveTimeControl,
/*   23  */      en_NV_Item_LiveTime,
/*   24  */      en_NV_Item_MONITORCPU_CFG,
/*   27  */      en_NV_Item_COMMDEGBUG_CFG = 27,
/*   28  */      en_NV_Item_Max_Gsm_Reg_Cnt,
/*   29  */      en_NV_Item_DFS_Config=29,
/*   31  */      en_NV_Item_PID_Enable_Type=31,
/*   32  */      en_NV_Item_AT_DISLOG_PWD,
/*   33  */      en_NV_Item_AT_SHELL_OPEN_FLAG, /* used by mdrv == NV_AT_SHELL_OPEN_FLAG */
/*   34  */      en_NV_Item_AT_E5_RIGHT_FLAG,
/*   35  */      en_NV_Item_BATTERY_TEMP_ADC,
/*   36  */      en_NV_Item_SW_VERSION_FLAG,
/*   37  */      en_NV_Item_ERRORLOG_ENABLEFLAG = 37,
/*   38  */      en_NV_Item_ERRORLOG_FLUSHBUFINTERVAL = 38,
/*   39  */      en_NV_Item_ERRORLOG_RECORDPERIOD = 39,
/*   40  */      en_NV_Item_USIM_TEMP_PROTECT_NEW,
/*   42  */      en_NV_Item_AT_DISSD_FLAG = 42,
/*   43  */      en_NV_Item_AT_DISSD_PWD = 43,
/*   45  */      en_NV_Item_Om_Port_Type=45,
/*   46  */      en_NV_Item_Om_Printf_Port,
/*   47  */      en_NV_Item_Om_PsLog_Port,
/*   48  */      en_NV_Item_AT_DISLOG_PWD_NEW,
/*   49  */      en_NV_Item_OLED_TEMP_ADC,
/*   51  */      en_NV_Item_WEBNAS_SD_WORKMODE=51,
/*   53  */      en_NV_Item_DEFAULT_LINK_OF_UART = 53,
/*   59  */      en_NV_Item_M2_ENABLE_FLAG = 59,
/*   60  */      en_NV_Item_SVLTE_FLAG = 60,
/*   63  */      en_NV_Item_NPNP_CONFIG = 63,
/*   64  */      en_NV_Item_VSIM_SUPPORT_FLAG = 64,
/*   65  */      en_NV_Item_VSIM_HVSDH_INFO = 65,
/*   66  */      en_NV_Item_VSIM_Fplmn_Info = 66,
/*   67  */      en_NV_Item_VSIM_Loci_Info = 67,
/*   68  */      en_NV_Item_VSIM_PsLoci_Info = 68,
/*   69  */      en_NV_Item_Card_Status_Cb_Type = 69,
/*   70  */      en_NV_Item_LC_Ctrl_PARA = 70,
/*   71  */      en_NV_Item_Usimm_T1_Ctrl_PARA = 71,
/*   90  */      en_NV_Item_BATTERY_ADC = GU_SYS_NV_ID_MIN + 90,
/*   114 */      en_NV_Item_Factory_Info = GU_SYS_NV_ID_MIN + 114,
/*   115 */      en_NV_Item_AT_FACTORY_INFO = GU_SYS_NV_ID_MIN + 115,
/*   116 */      en_NV_Item_AT_MANUFACTURE_DATE = GU_SYS_NV_ID_MIN + 116,
/*   117 */      en_NV_Item_LOG_FILE_SAVE = GU_SYS_NV_ID_MIN + 117,
/*   120 */      en_NV_Item_OAM_Config = GU_SYS_NV_ID_MIN + 120,
/*   121 */      en_NV_Item_System_APP_Config = GU_SYS_NV_ID_MIN + 121,
/*   122 */      en_NV_Item_ZSP_LDF_CONFIG = GU_SYS_NV_ID_MIN + 122,
/*   123 */      en_NV_Item_HIFI_LDF_CONFIG = GU_SYS_NV_ID_MIN + 123,
/*   124 */      en_NV_Item_TTF_MEM_SOLUTION_ID = GU_SYS_NV_ID_MIN + 124,       /*δʹ��*/
/*   125 */      en_NV_Item_TTF_MEM_SOLUTION_CFG = GU_SYS_NV_ID_MIN + 125,
/*   126 */      en_NV_Item_PHY_SW_CFG = GU_SYS_NV_ID_MIN + 126,
/*   127 */      en_NV_Item_NV_PRIORITY_THRESHOLD = GU_SYS_NV_ID_MIN + 127,    /*FlashLess Phase II*/
/*   128 */      ev_NV_Item_SCI_DSDA_CFG = 128, /* used by mdrv == NV_ID_DRV_SCI_DSDA_SELECT */
/*   129 */      en_NV_Item_Sim_ATR_Flag = GU_SYS_NV_ID_MIN + 129,
/*   130 */      en_NV_Item_TERMINAL_CAPABILITY = GU_SYS_NV_ID_MIN + 130,
/*   131 */      en_NV_Item_Write_Slice_Record_Flag = GU_SYS_NV_ID_MIN + 131,
                 en_NV_ID_SOCP_SDLOG_CFG = 132,
/*   133 */      en_NV_Item_CC_TimerLen  = 133,
/*   134 */      en_NV_Item_ErrLogCtrlInfo = 134,
/*   135 */      en_NV_Item_AlarmidRelationship = 135,
/*   136 */      en_NV_Item_FTMDetail = 136,
/*   137 */      en_NV_Item_RF_INTRUSION_CFG = GU_SYS_NV_ID_MIN + 137,
/*   138 */      en_NV_Item_GUMODE_CHAN_PARA = 138,
/*   139 */      en_NV_Item_RECORD_BASE_BORARD_ID = GU_SYS_NV_ID_MIN + 139,
/*   140 */      en_NV_Item_RECORD_CURRENT_BORARD_ID = GU_SYS_NV_ID_MIN + 140,
/*   141 */      en_NV_Item_UART_CFG = GU_SYS_NV_ID_MIN + 141,

/*   142 */      en_NV_Item_Usim_Support_Ims = GU_SYS_NV_ID_MIN + 142,
/*   143 */      en_NV_Item_FLASH_Log_Record_CFG = GU_SYS_NV_ID_MIN + 143,
/*   144 */      en_NV_Item_PORT_BUFF_CFG           = GU_SYS_NV_ID_MIN + 144,
/*   145 */      en_NV_Item_EVENT_RESEND_CFG = GU_SYS_NV_ID_MIN + 145,
/*   146 */      en_NV_Item_CBT_LOG_ENABLE = GU_SYS_NV_ID_MIN + 146,
/*   147 */      en_NV_Item_PS_TRANSFER_CFG = GU_SYS_NV_ID_MIN + 147,
/*   148 */      en_NV_Item_ProductTypeForLogDirectory = GU_SYS_NV_ID_MIN + 148,
/*   149 */      en_NV_Item_DRX_RESET_ENABLE_CFG = GU_SYS_NV_ID_MIN + 149,
/*   150 */      en_NV_Item_MTC_RF_LCD_CFG = GU_SYS_NV_ID_MIN + 150,
/*   151 */      en_NV_Item_MTC_RF_LCD_TIMER_INTERVAL_CFG = GU_SYS_NV_ID_MIN + 151,

/*   2000*/      en_NV_Item_1X_MRU_LIST = 2000,
/*   2001*/      en_NV_Item_PRL_LIST    = 2001,

/*   2002*/      en_NV_Item_1X_SMS_CFG = 2002,

/*   2003*/      en_NV_Item_CFreqLock_CFG = 2003,

/*   2005*/      en_NV_Item_1X_HOME_SID_NID_LIST       = 2005,
/*   2006*/      en_NV_Item_1X_OOC_TIMER_SCHEDULE_INFO = 2006,
/*   2007*/      en_NV_Item_1X_CHAN_REPEAT_SCAN_STRATEGY = 2007,

/*   2008*/      en_NV_Item_1X_EPDSZID_FEATURE_CFG       = 2008,

/*   2009*/      en_NV_Item_HRPD_MRU_LIST = 2009,
/*   2010*/      en_NV_Item_1X_LAST_SCI = 2010,

/*   2011*/      en_NV_Item_HRPD_LOC_INFO = 2011,
/*   2012*/      en_NV_Item_HRPD_STORAGE_BLOB = 2012,

/*  2013  */     en_NV_Item_CDATA_GENERIC_CONFIG  = 2013,

/* 2014 */       en_Nv_Item_Mmss_System_Acquire_Cfg = 2014,
/* 2015 */       en_Nv_Item_MmssLastLocationInfo = 2015,

/* 2016*/        en_NV_Item_MLPL_MSPL_FILE = 2016,
/*  2017  */     en_NV_Item_TEST_CONFIG  = 2017,

/*  2018  */     en_NV_Item_1X_MOB_TERM = 2018,
/*  2019  */     en_NV_Item_NO_CARD_MODE_CFG = 2019,

/*  2020  */     en_NV_Item_CNAS_HRPD_Session_Info_Ex = 2020,

/*  2021  */     en_NV_Item_CNAS_HRPD_Session_Ctrl_Cfg = 2021,

/* Added by k902809 for Iteration 15, 2015-5-21, begin */
/* 2022 */      en_NV_Item_EHRPD_Retry_Conn_Est_Config  = 2022,
/* 2023 */      en_NV_Item_EHRPD_Retry_Pdn_Setup_Config = 2023,
/* Added by k902809 for Iteration 15, 2015-5-21, end */

/*  2024  */     en_NV_Item_1X_ADD_AVOID_LIST_CFG  = 2024,
/*  2025 */     en_NV_Item_1X_NEG_PREF_SYS_CMP_CTRL  = 2025,

/*  2026 */      en_NV_Item_1X_CALL_CFG  = 2026,

/*  2027  */     en_NV_Item_CNAS_HRPD_Session_Retry_Cfg  = 2027,
/*  2028  */     en_NV_Item_EHRPD_Support_Flg = 2028,
/*  2029  */     en_Nv_Item_CNAS_HRPD_Session_Keep_Alive_Info = 2029,
/*  2030  */     en_Nv_Item_CNAS_HRPD_Session_Info = 2030,
/*  2032  */     en_Nv_Item_EMC_CALLBACK_CFG                = 2032,           /* ������CallBackģʽ���� */
/*  2033  */     en_Nv_Item_EMC_REDIAL_PERIOD               = 2033,           /* �������ز���ʱ��ʱ�� */
/*  2034  */     en_NV_Item_HOME_SID_NID_DEPEND_ON_PRL_CFG  = 2034,
/*  2035  */     en_NV_Item_OPER_LOCK_SYS_WHITE_LIST_INFO   = 2035,
/*  2036  */     en_NV_Item_CTCC_CUSTOMIZE_FREQ_INFO        = 2036,
/*  2037  */     en_NV_Item_CDMA_STANDARD_CHANNLES_INFO     = 2037,
/*  2038  */     en_Nv_Item_1X_DATA_SO_CFG = 2038,
/*  2039  */     en_NV_Item_CDMA_1X_CUSTOM_PREF_CHANNELS_INFO   = 2039,
/*  2040  */     en_Nv_Item_1X_VOICE_SO_CFG   = 2040,
/*  2042  */    en_NV_Item_START_AND_STOP_CONT_DTMF_INTERVAL    = 2042,     /* StopContDtmfReq��StartContDtmfReq���ͼ��ʱ�� */
/*  2043 */     en_NV_Item_CL_DELAY_REPORT_SERVICE_STATUS_CFG  = 2043,    /* CLģ����״̬��ʱ�ϱ���ʱ��ʱ�� */
/*  2044  */     en_Nv_Item_CNAS_HRPD_ACCESS_AUTH_INFO    = 2044,

/*  2045  */    en_NV_Item_1X_XSMS_KMC_ADDRESS             = 2045,     /* �绰���ܣ����ͺͽ��ܶ��ŵĵ�ַ */

/*  2046  */    en_NV_Item_VOICE_ENCRYPT_PUBLICKEY_AND_VERSION_INFO = 2046, /* ECC�Ĺ�Կ�Լ���Կ�汾�� */
/*  2047  */    en_NV_Item_VOICE_ENCRYPT_CAP_PARA_INFO              = 2047, /* ���������������ò����������������Լ���̬���ÿ��� */
/*  2048  */    en_NV_Item_VOICE_ENCRYPT_SECINFO_ERASE_SYSTIME_INFO = 2048, /* ��ȫ��Ϣ����ϵͳʱ�� */

/*  2049*/      en_NV_Item_OM_Port_Num = GU_SYS_NV_ID_MIN + 2049,

/*  2050  */    en_NV_Item_VOICE_ENCRYPT_PASSWD_RESET_SYSTIME_INFO  = 2050, /* ��������ϵͳʱ�� */
/*  2051  */    en_NV_Item_VOICE_ENCRYPT_TIMER_CFG_INFO             = 2051, /* ����������ʱ��Կ������ʱ�� */

/*  2052  */    en_NV_Item_EMC_REDIAL_SYS_ACQ_CFG    = 2052,

/*  2053  */    en_NV_Item_1X_PAGING_RSP_SO_CFG                 = 2053,

/*   2054*/      en_NV_Item_CDATA_DISCING_PARA_CFG = 2054,       /* CDMA������ȥ�����������漰���Ĳ��� */

/*  2055  */    en_NV_Item_1X_AVOID_SCHEDULE_INFO             = 2055,

/*   2056*/      en_NV_Item_HRPD_OOC_TIMER_SCHEDULE_INFO = 2056,

/*  2057  */    en_NV_Item_1X_SMS_MO_TL_ACK_TIME_OUT_CFG        = 2057,  /* ���ŷ���ʱ�ȴ�TL_ACK��ʱ�Ĵ������� */
/*  2059  */    en_NV_Item_1X_SERVICE_CL_SYSTEM_ACQUIRE_STRATEGY_CFG = 2059, /* 1x�з���ʱCLϵͳ����������� */

/*   2060*/      en_NV_Item_PPP_AUTH_INFO_FROM_CARD_FLAG = 2060,       /* 1X HRPD PPP�ļ�Ȩ�����������û�����card */


/* 2062 */       en_NV_Item_1X_POWER_OFF_CAMP_ON_CTRL = 2062,
/*  2063  */     en_NV_Item_CL_NO_SERVICE_DELAY_RPT_CFG  = 2063,       /* �ӳ��ϱ�HRPD��LTE�޷���ʱ��ʱ������ */

/* 2064 */       en_NV_Item_NDIS_FILTER_ENABLE_FLAG_CFG = 2064,
/* 2065  */     en_NV_Item_1X_OOS_SYS_ACQ_STRATEGY_CFG   = 2065,       /* 1x OOS����ϵͳ�����������NV */
/*  2066  */    en_NV_Item_CL_SYSTEM_ACQUIRE_DSDS_STRATEGY_CFG = 2066,  /* CLϵͳ����DSDS�������� */

/*  2067  */    en_NV_Item_CTCC_OOS_CFG = 2067,  /* �����л� OOS���ƶ�ʱ�� */

/*  2068  */    en_NV_Item_CDMA_ERR_LOG_ACTIVE_REPORT_CONTROL = 2068,         /* XģCHR�����ϱ����� */

/*  2069  */    en_NV_Item_1X_DO_SYS_ACQ_NO_RF_PROTECT_TIMER_CFG = 2069,  /* SRLTE��1x DO������������Դ ������ʱ�� */

/* 2070 */       en_NV_Item_HOME_MCC_INFO = 2070,

/*  2071  */    en_NV_Item_HRPD_AVOID_SCHEDULE_INFO = 2071,  /* ����avoidʱ�� */

/*  2073  */    en_NV_Item_GET_DNS_TROUGH_DHCP_CFG = 2073,              /* �����Ƿ�ͨ��DHCP��ȡDNS */
/*  2074  */    en_NV_Item_MRU0_SWITCH_ON_OOC_STRATEGY_CFG = 2074,  /* ������OOC������MRU0������� */
/*  2075  */    en_NV_Item_1X_PRL_ROAM_IND_STRATEGY_CFG = 2075,         /* ����1X PRL�������ȼ��ж��Ƿ���ROAM IND�ֶ� */

/*  2076  */    en_NV_Item_SYS_ACQ_BSR_CTRL_CFG = 2076,  /* BSR TIMER ���Ʋ��� */

/*  2077  */    en_NV_Item_1X_SYS_ACQ_SYNC_DELAY_INFO_WHEN_LTE_OR_DO_CONN = 2077,         /* hrpd ��������̬���� srlte��lte����̬������1x���磬����ͬ��֮��ĵȴ���ʱ��ʱ������ */

/*  2079  */    en_NV_Item_CDMA_1X_CUSTOMIZE_PREF_CHANNELS_INFO = 2079,         /* 1X�й����ų���Ƶ�����ݣ���ԭ����10������Ϊ20�� */

/*  2080  */    en_NV_Item_CDMA_HRPD_CUSTOMIZE_PREF_CHANNELS_INFO = 2080,         /* HRPD�й����ų���Ƶ�㣬����Ϊ20�� */

/*  2081  */    en_NV_Item_1X_NW_NORMAL_REL_REDIAL_CFG = 2081,            /* 1x nw normal release, ps redial ���Ʋ��� */

/*  2083  */    en_NV_Item_CCTC_ROAM_CTRL_CFG = 2083,                           /* �й����Ź������ο������� */

/*  2084  */    en_NV_Item_MODE_SELECTION_CFG = 2084,                              /* ģʽѡ����ؿ��Ʋ��� */
/*  2085  */    en_NV_Item_NO_CARD_SYS_ACQ_CFG = 2085,                             /* �޿��������Ʋ��� */

/* 2086 */      en_NV_Item_CBT_PRL_LIST                = 2086,

/* 2087 */     en_NV_Item_Xsd_Oos_Mode_Switch_Ctrl_Cfg = 2087,                     /* OOCģʽ�л��������� */

/* 2088 */      en_NV_Item_1X_CS_CALL_REDIR_CMPL_DELAY_TIMER_INFO = 2088,         /* ���ض����µ���������ʧ�ܣ�XCALL�յ�ϵͳ��Ϣ���ٴ��·�����������ӳٶ�ʱ������NV */

/* 2089 */      en_NV_Item_MODE_SELECTION_PUNISH_CTRL_INFO = 2089,                /* ģʽ�л��ͷ�������Ϣ */
/* 2090 */      en_NV_Item_MODE_SELECTION_RETRY_SYS_ACQ_STRATEGY = 2090,          /* ģʽѡ���������� */
/* 2091 */      en_NV_Item_UNKONWN_CDMA_DUAL_MODE_CARD_MODE_SWITCH_CFG = 2091,    /* δ֪CDMA˫ģ���Ƿ�����ģʽ�л� */

/* 2092 */      en_NV_Item_CTCC_ROAM_EMC_CALL_CFG = 2092,                         /* �й����Ž������п��� */


/* 2093 */      en_NV_Item_DO_BACK_TO_LTE_CFG_INFO = 2093,                       /* Release Do Tch for Lte History Search */

/* 2094 */      en_NV_Item_1X_REG_CFG_INFO = 2094,         /* ���������Ƿ�Ҫ��ǰ����ע�� */

/* 2095 */      en_NV_Item_LTE_SMS_CFG = 2095,

/* 2096 */      en_NV_Item_LTE_OOS_DELAY_ACTIVATE_1X_TIMER_INFO = 2096,             /* lte��������VoLTE�����ã���ֹ��������ȥ����1x����һ���ӳٶ�ʱ�� */

/* 2094 */      en_NV_Item_CTCC_DUAL_MODE_CARD_PLMN_INFO = 2097,                  /* ����˫ģ��IMSI��Ϣ */

/* 2098 */      en_NV_Item_CL_SYS_ACQ_TYPE_CTRL_CFG = 2098,

/* 2099 */      en_NV_Item_LTE_DO_CONN_INTERUPT_1X_SYS_ACQ_CFG = 2099,         /* ��������srlte�Ƿ�do����LTE�������Ӵ�ϵ�ǰ1xͬ������ */

/* 2100 */      en_NV_Item_CHINA_BOUNDARY_SID_AND_MCC_INFO = 2100,              /* ���������й��߽���������SID �� MCC ��Ϣ */

/* 2099 */      en_NV_Item_CL_VOLTE_CFG_INFO       = 2101,

/* 2102 */      en_NV_Item_CDMAMODEMSWITCH_NOT_RESET_CFG   = 2102,              /* ��������CDMAMODEMSWITCH�����������Ƿ������������������ú���RESET MODEM */

/* 2103 */      en_NV_Item_CHINA_HOME_SID_AND_MCC_INFO = 2103,                  /* ���������й����ڵ�SID �� MCC ��Ϣ�������й���½�Ͱ��� */

/* 2104 */      en_NV_Item_1X_MT_SMS_TCH_RELEASE_CFG       = 2104,              /* ��������ҵ���ŵ����ű��������粻�ͷ���·��UEû���յ������ͷ���·����ϢʱUE�����ͷ�ҵ����· */

/* 2105 */       en_NV_Item_1X_MT_Reest_Cfg                = 2105,              /* �������ñ��н���ʧ�����������Ϣ */

/*   2300*/      en_NV_Item_DSDS_Active_Modem_Mode = 2300,
/*   2302*/      en_NV_Item_HIGH_PRIO_RAT_HPLMN_TIMER_CFG = 2302,
/* 2303 */       en_NV_Item_ChangeNWCause_CFG = 2303,
/*   2304*/      en_NV_Item_ECID_TL2GSM_CFG = 2304,
/* 2305  */      en_NV_Item_CMCC_Cfg_Dplmn_Nplmn = 2305,
/* 2306  */      en_NV_Item_UNICOM_Cfg_Dplmn_Nplmn = 2306,
/* 2307  */      en_NV_Item_CT_Cfg_Dplmn_Nplmn = 2307,
/* 2308  */      en_NV_Item_Cfg_Dplmn_Nplmn_Flag = 2308,
/*   2309*/     en_NV_Item_REL_PS_SIGNAL_CON_CFG = 2309,
/* 2310 */      en_NV_Item_Wcdma_Voice_Prefer_Cfg = 2310,
/*  2311 */      en_NV_Item_PDN_TEARDOWN_POLICY = 2311,

/*   2312*/     en_NV_Item_User_Cfg_Ext_Ehplmn_Info = 2312,            /* �û����õ�EHplmn NVIM�� ID */

/*   2313*/     en_NV_Item_LTE_REJ_CAUSE_14_CFG = 2313,

/*   2314*/     en_NV_Item_TTY_Mode = 2314,
/*  2315 */     en_NV_Item_Csmo_Supported_Cfg_Info  = 2315,                     /* �����Ƿ�֧��CSMO */
/*   2317 */     en_NV_Item_CC_T303_Len_Cfg  = 2317,           /* T303��ʱ��ʱ������ΧΪ30~255����λΪ�� */
/*  2318 */     en_NV_Item_T3212_Timer_Cfg_Info     = 2318,                     /* ����T3212��ʱ��ʱ����Ϣ */
/*  2319 */     en_NV_Item_Roam_Display_Cfg = 2319,                /* ������ʾ���� */
/*   2320*/     en_NV_Item_Rat_Frequently_Switch_Chr_Rpt_Cfg = 2320,
/* 2324 */      en_NV_Item_Protect_Mt_Csfb_Paging_Procedure_Len = 2324,         /* csmt ����ʱ�� */

/*  2321 */     en_NV_Item_NVWR_SEC_CTRL            = 2321,                     /* NVWR���ȫ���� */
/* 2325 */      en_NV_Item_HIGH_PRIO_PLMN_REFRESH_TRIGGER_BG_SEARCH_CFG   = 2325,

/*   2322*/     en_NV_Item_DELAY_REPORT_SERVICE_STATUS_CFG = 2322,
                en_NV_Item_SMS_PS_CTRL              = 2326,
/* 2327  */      en_NV_Item_First_Preset_Dplmn_Nplmn_Cfg = 2327,
/* 2328  */      en_NV_Item_Second_Preset_Dplmn_Nplmn_Cfg = 2328,
/* 2329  */      en_NV_Item_Self_Learn_Dplmn_Nplmn_Cfg = 2329,


/*  2330 */     en_NV_Item_Non_Oos_Plmn_Search_Feature_Support_Cfg       = 2330,                     /* ������HISTORY������ʱ��ʱ������ */
/*  2331 */     en_NV_Item_Ccpu_Reset_Record        = 2331,                     /* C�˵�����λ��¼ */
/*  2332 */     en_NV_Item_Get_Geo_Cfg_Info         = 2332,                     /* ��ȡ�������������Ϣ */
/*  2333 */     en_NV_Item_Roam_Plmn_Selection_Sort_Cfg     = 2333,             /* ����ѡ�������������NV,�ڴ�NV��ʱ�򣬲ſ��ǽ��������簴DUO˳������ǰ�� */



/* 2334 */      en_NV_Item_DISABLE_LTE_START_T3402_ENABLE_LTE_CFG   = 2334,
/*   2335*/     en_NV_Item_LTE_OOS_2G_PREF_PLMN_SEL_CFG = 2335,

/*  2336 */     en_NV_Item_Ipv6_Address_Test_Mode_Cfg = 2336,                   /* IPV6��ַ����ģʽ���ã�0x55AA55AAΪ����ģʽ������ֵΪ����ģʽ */

/*  2337 */     en_NV_Item_ADS_IPF_MODE_CFG = 2337,                             /* IPF����ADS�������ݵ�ģʽ���� */

/*  2338 */     en_NV_Item_Dynload_CTRL     = 2338,

/* 2339 */      en_NV_Item_Nw_Search_Chr_Record_Cfg = 2339,                     /* ����CHR��¼���� */
/*  2341 */     en_NV_Item_PS_REG_FAIL_MAX_TIMES_TRIG_LAU_ONCE_CFG              = 2341,
/*  2342 */     en_NV_Item_KEEP_SRCH_HPLMN_EVEN_REJ_BY_CAUSE_13_CFG             = 2342,
/*  2343 */     en_NV_Item_EPS_REJ_BY_CAUSE_14_IN_VPLMN_Allow_PS_REG_CFG        = 2343,
/*  2344 */     en_NV_Item_CARRY_EPLMN_WHEN_SRCH_RPLMN_CFG                      = 2344,
/*  2345 */     en_NV_Item_LAU_REJ_NORETRY_WHEN_CM_SRV_EXIST                    = 2345,
/*  2346 */     en_NV_Item_LAU_REJ_TRIG_PLMN_SEARCH                             = 2346,
/*  2347 */     en_NV_Item_CALL_REDIAL_CM_SRV_REJ_CAUSE_CFG                     = 2347,
/*  2348 */     en_NV_Item_SMS_RETRY_CM_SRV_REJ_CAUSE_CFG                       = 2348,
/*  2349 */     en_NV_Item_SS_RETRY_CM_SRV_REJ_CAUSE_CFG                        = 2349,
/*  2350 */     en_NV_Item_REDIAL_IMS_TO_CS_DOMAIN_CFG                          = 2350,

/*  2340 */     en_NV_Item_Ccwa_Ctrl_Mode                   = 2340,

/*  2351 */     en_NV_Item_Plmn_Search_Phase_One_Total_Timer_CFG       = 2351,  /* ��һ�׶�������ʱ������ */
/* 2352 */      en_NV_Item_Privacy_Log_Filter_Cfg = 2352,
/* 2353 */      en_NV_Item_Low_Prio_Anycell_Search_Lte_Cfg   = 2353,
/*  2354 */     en_NV_Item_ADS_WAKE_LOCK_CFG = 2354,                            /* ADS WAKELOCK ���� */
/*  2355 */     en_NV_Item_CS_REG_FAIL_FORB_LA_TIME_CFG                = 2355,  /* csע��ʧ�ܽ���ǰLA����FORB LAʱ������ */

/*  2356 */     en_NV_Item_Add_EHPLMN_WHEN_SRCH_HPLMN_CFG = 2356,               /* ����HPLMNʱЯ��EHPLMN ���� */

/*  2357 */     en_NV_Item_BUFFER_SERVICE_REQ_PROTECT_TIMER_CFG = 2357,         /* ����cc ss sms ��������ʱ�ı�����ʱ������ */

/*  2358 */     en_NV_Item_REFRESH_RPLMN_WHEN_EPLMN_INVALID_CFG                 = 2358,

/*  2359  */    en_NV_Item_PROGRESS_INDICATOR_START_T310_INFO   = 2359,         /* ��progress indicator Ϊ#1��#2, #64ʱ���Ƿ�����T310 */

/*  2360 */     en_NV_Item_Dplmn_Nplmn_Cfg                  = 2360,

/*  2361 */     en_NV_Item_Print_Modem_Log_Type     = 2361,                     /* �����Ƿ����modem log������ */

/*  2362 */     en_NV_Item_Oos_Plmn_Search_Strategy_Cfg     = 2362,             /* ������������ʱ��ʱ������ */

/*  2363 */     en_NV_Item_NONNORMAL_REG_STATUS_MERGE_CFG     = 2363,  /* �������������Ƿ��ϱ�ע��״̬�ı������ */

/*  2365 */     en_NV_Item_Dynload_Exception_CTRL     = 2365,

/* 2366 */      en_NV_Item_CSG_CTRL_CFG            = 2366,     /* CSG�����������NV�� */

/* 2367 */      en_NV_Item_KEEP_CS_FORB_INFO_WHEN_PS_REG_SUCC_CFG = 2367,              /* PSע��ɹ�ʱ�Ƿ���FORB��Ϣ������ */

/*  2369 */     en_NV_Item_ADS_MEM_POOL_CFG = 2369,

/* 2370 */      en_NV_Item_CLEAR_CKSN_CFG = 2370,                               /* ��Ҫ���CKSN������ */

/* 2371 */      en_NV_Item_XCPOSRRPT_CFG = 2371,                               /* �Ƿ��ϱ����GPS����ĸ�����λ��Ϣ */

/* 2372 */      en_NV_Item_OOS_BG_NETWORK_SEARCH_CUSTOM     = 2372,             /* OOS BG�������� */

/* 2373 */      en_NV_Item_CALL_REDIAL_DISC_CAUSE_CFG = 2373,
/* 2374 */      en_NV_Item_REFRESH_USIM_DELAY_RESTART_CFG      = 2374,          /* �յ�refresh������������绰�����ţ�����ҵ��ʱ���Ƿ�����modem */

/* 2375 */      en_NV_EMC_CS_TO_IMS_REDIAL_CFG = 2375,                          /* CS������ʧ�ܵ�IMS�ز���NV */

/* 2376 */      en_NV_Item_OTA_SECURITY_SMS_CFG  = 2376,         /* �Ƿ���Ҫ��OTA��������ȫ��� */

/* 2377 */      en_NV_Item_User_Cfg_Forb_Plmn_Info          = 2377,             /* �û����õ�Forb Plmn��Ϣ */

/* 2378 */      en_NV_Item_Cause18_Enable_Lte_Support_Flg          = 2378,             /* LTE�ϱ�Cause18�ܾ����Ƿ�����Enable LTE */

/* 2379 */      en_NV_Item_RPM_Config   = 2379,                                 /* RPM�������� */

/* 2380 */      en_NV_Item_EXTEND_T3240_LEN_CFG     = 2380,                     /* T3240ʱ������ */

/* 2381 */      en_NV_Item_Cs_Loci                          = 2381,             /* ��¼CS�������Ϣ��tmsi, LAI,RAU ״̬�ȴ� */

/* 2382 */      en_NV_Item_Custom_Supplement_Oplmn_Info     = 2382,             /* �û����õ�oPlmn��Ϣ */
/*  2383 */     en_NV_Item_AI_MODEM_CFG     = 2383,                            /* Sensor Hub������� */

/* 2384 */      en_NV_Item_Trig_Resel_Disconnect_Cause_Cfg = 2384,  /* ��¼����AS������ѡС����disconnetԭ��ֵ */

/* 2385 */      en_NV_Item_Lte_Lost_Chr_Cfg = 2385,                           /* ����4G CHR��������Ϣ */

/* 2386 */      en_NV_Item_ATT_FRAT_CFG_Info                = 2386,             /* �û����õ�ATT:FRAT��Ϣ */

/* 2387  */    en_NV_Item_USER_SYS_CFG_RAT_INFO = 2387,                       /* �û�SYS CFG RAT���� */

/* 2388 */      en_NV_Item_Edo_Rpm_Timer_Info   = 2388,                         /* ��¼RPM�����Ϣ */

/* ��Ϊԭ������RP CAUSE������Ҫ�ط��ģ�ֻ��38��ԭ��ֵ�������ط������Զ��岻�ط�ԭ��ֵ */
/* 2389 */      en_NV_Item_SMS_NO_RETRY_RP_CAUSE_CFG   = 2389,                  /* ���Ų��ط�RPԭ��ֵ���� */

/* 2390 */     en_NV_Item_Oos_Mode_Switch_Ctrl_Cfg = 2390,                       /* OOCģʽ�л��������� */

/* 2391 */      en_NV_Item_User_Reboot_Support_Flg          = 2391,             /* �û��Ƿ���Է���������λ */
/* 2392 */      en_NV_Item_Nonregular_Servic_Custom       = 2392,             /* �ǳ���ҵ������Ϣ */

/* 2393 */      en_NV_Item_Hold_Retrieve_Rej_Optimize      = 2393,              /* ��Hold/Retrive rej�Ż����� */

/* 2394 */      en_NV_Item_Dsds_Skip_Band_Plmn_Search_Cfg = 2394,

/* 2395 */      en_NV_Item_PAGING_RESPONSE_RETRY_CFG       = 2395,              /* ����Ѱ����Ӧ�ط� */

/* 2396 */      en_NV_Item_Gps_Cust_CFG  = 2396,                                /* ��ѯGPSоƬ���� */

/* 2397 */      en_NV_Item_Ss_Wait_User_Rsp_Len = 2397,                         /* SSҵ��ȴ��û���Ӧ��ʱ�� */

/* 2398 */      en_NV_Item_Cc_Status_Enquiry_Cfg = 2398,                        /* Status Enquiry��������Ϣ */

/*  2399 */     en_NV_Item_Open_Icc_Cfg           = 2399,                       /* ICCͨ�������� */
/*  2400 */     en_NV_Item_Sar_Sensor_hub_CFG     = 2400,                       /* SAR sensor hub������ */

/* 2401 */      en_NV_Item_MTC_Dynamic_FM_Intrusion_Ctrl_CFG = 2401,            /* ��̬��Ƶ���ſ��Ƶ�������Ϣ */

/* 2402 */      en_NV_Item_Custom_Voice_Domain_Sel_Cfg = 2402,

/* 2403 */      en_NV_Item_SMS_FAIL_LINK_CTRL_CFG            = 2403,             /* ��AP���ö�������ʱ�������һ�����ŷ���ʧ�ܣ��Ƿ�رն����������� */

/* 2404 */      en_NV_Item_Extended_Forbidden_Plmn_List_Cfg = 2404,             /* ������չ��forbidden plmn�б�������Ϣ */


/* 2405 */      en_NV_Item_Lgu_Support_Config   = 2405,                         /* LGU+�Ƿ�֧������ */

/* 2406 */      en_NV_Item_Power_On_Read_Usim_Optimize_Cfg = 2406,              /* ���������Ż�������Ϣ */

/*  2408 */     en_NV_Item_Border_Plmn_Search_Cfg           = 2408,
/*  2409 */     en_NV_Item_Ap_Preset_Border_Info            = 2409,

/* 2410 */      en_NV_Item_REJ_MAX_TIMES_DISABLE_LTE_CFG = 2410,           /* attach tau ���ܴ�������disable LTE ������Ϣ */
/* 2411 */      en_NV_Item_HIGH_SPEED_MODE_RPT_CFG = 2411,
/* 2413 */      en_NV_Item_PDP_REDIAL_FOR_NO_CAUSE_CFG = 2413,                  /* PDP˫ջ���������ԭ��ֵ�ز����ܿ������� */

/* 2414 */      en_NV_Item_Srvcc_Sn_Modulo_Cfg = 2414,                          /* SRVCC��G��SNģ���� */

/* 2415 */      en_NV_Item_READ_USIM_FILE_TIMER_LEN_CFG = 2415,            /* ��USIM���ļ���ʱ��ʱ�� */

/* 2416 */      en_NV_Item_CHECK_USIM_CSIM_SMS_STATUS_CFG = 2416,

/* 2417 */      en_NV_Item_Power_On_Quick_Display_Normal_Service_Optimize_Cfg = 2417,          /* �Ƿ񿪻���ǰ�ϱ�ע�������Ϣ */

/* 2418 */     en_NV_Item_CPOS_PROTOCOL_VERSION_CFG = 2418,

/* 2419 */      en_NV_Item_Read_Backoff_File_Config   = 2419,                   /* �Ƿ������ȡBACKOFF�㷨��ز����ļ����� */

/* 2420 */      en_NV_Item_Oos_Chr_Cfg = 2420,             /* ���͹��ĵĶ���CHR���� */

/* 2421 */      en_NV_Item_Rnic_Net_If_Cfg = 2421,                              /* RNIC�������ݵ�Linux����Э��ջ�Ľӿ�ģʽ���� */

/* 2422 */      en_NV_Item_Smsp_Preseted              = 2422,                   /* NV���ö������� */

/* 2423 */      en_NV_Item_T3396_Running_Disable_Lte_Cfg = 2423,                /* LMM->MMC attach ind���ԭ��ֵΪT3396�����У���Ҫdisable lte������ */

/* 2424 */      en_NV_Item_Telcel_Pdp_Act_Limit_Cfg         = 2424,             /* PDP������������ */

/*  2425 */      en_NV_Item_Deact_Emc_Pdn_Policy        = 2425,                 /* ��������PDN�ص�GU�����ӶϿ����� */

/* 2426 */      en_NV_Item_ImsCall_To_CsRedial_Cfg     = 2426,

/* 2427 */      en_NV_Item_Match_Register_Apn_Cfg         = 2427,               /* �Ƿ�򿪱Ƚ��û������APN��ע��LTEʱ�����APN */

/* 2428 */      en_NV_Item_Nas_Log_Print_Cfg  = 2428,

/* 2429 */      en_NV_Item_Limited_Service_Status_Report_cfg = 2429,

/* 2430 */      en_NV_Item_Tmsi_Or_P_Tmsi_Realloc_Plmn_Valid_Cfg  = 2430,             /* TMSI��P_TMSI�ط����PLMNΪ0�Ƿ���Ч���� */

/* 2432 */      en_NV_Item_EMC_Bar_Trigger_Plmn_Search_Cfg = 2432,              /* EMC��barʱ�Ƿ���Ҫ�������� */

/* 2431 */      en_NV_Item_CC_T310_Cfg                  = 2431,                 /* T310���� */

/*  2433 */     en_NV_Item_Emc_Under_Net_Pin_Cfg            = 2433,             /* NAS_NVIM_EMC_UNDER_NET_PIN_CFG_STRU  �Ƿ��VDF��������״̬�·������������ */

/* 2434 */      en_NV_Item_Time_Info_Report_Cfg         = 2434,                 /* ʱ��û�б仯ʱ�Ƿ��ϱ�^TIME����ʱ����Ϣ */

/* 2435 */       en_NV_Item_Dsds_Delay_Time          = 2435,

/* 2437 */      en_NV_Item_AT_CLIENT_CFG             = 2437,                    /* ˫��˫ͨ����ATͨ�������ĸ�Modem��Ϣ */

/* 2439 */      en_NV_Item_Emc_Cate_Support_ECALL_CFG             = 2439,       /* ������SETUP��Ϣ�У��Ƿ�֧��ECALL���� */

/* 2441 */      en_NV_Item_DSDS_Ctrl_Cfg             = 2441,                /* DSDS ������Ϣ */


/* 2436 */      en_NV_Item_CC_MO_Conllision_Cfg         = 2436,                 /* �����г�ͻ�Ż����ƿ��� */

/* 2444 */      en_NV_Item_Dcm_Custom_Disable_Lte_Cfg       = 2444,                 /* DCM disable Lte �������� */
/* 2445 */      en_NV_Item_Smc_Fail_Csfb_Mt_Force_Auth_Cfg  = 2445,                 /* SMC FAIL��csfb����ǿ�Ƽ�Ȩ �������� */

/* 2449 */       en_NV_Item_MT_MM_S12_RCV_SYSINFO_CFG             = 2449,
/*  2462 */     en_NV_Item_HIGH_PRIO_PS_CFG                 = 2462,     /* NV_NAS_HIGH_PRIO_PS_CFG_STRU */
/* 2467 */       en_NV_Item_Alerting_Srvcc_Open_Codec_Cfg         = 2467,       /* ֮ǰ�Ǳ������壬Alerting Srvcc��ҲҪ��Codec */
/* 2468 */       en_NV_Item_Srvcc_No_Call_Num_T3240_Cfg           = 2468,       /* SRVCC��G��IMSAͬ���ĺ�����Ϣʱ�յģ��Ƿ������ͷ����ӣ����޸�T3240��ʱ��ʱ�� */
/*  2475 */     en_NV_Item_SS_IMS2CS_REDIAL_CFG                   = 2475,     /* TAF_NV_SS_IMS2CS_REDIAL_CFG_STRU ����USSD�����ز�ԭ��ֵ */
/*  2482 */     en_NV_Item_REDIAL_WIFI_TO_CS_DOMAIN_CFG      = 2482,     /* TAF_NV_SWITCH_WIFI_TO_CS_REDIAL_CONFIG_STRU */
/*  2491 */     en_NV_Item_Classmark_SuppCodec_CapRpt_Ctrl        = 2491,     /* NAS_NVIM_SRCH_STGY_AFTER_HISTORY_REJ_STRU */

/*  2509 */     en_NV_Item_Manual_Mode_Reg_Hplmn_Cfg              = 2509,     /* NAS_NVIM_MANUAL_MODE_REG_HPLMN_CFG_STRU */

/* 2514 */     en_NV_Item_Ss_Conn_State_Rcv_Rel_Ind_Set_Cause_Cfg = 2514,     /* NAS_NVIM_SS_CONN_STATE_RCV_REL_IND_SET_CAUSE_STRU */

/* 2524 */     en_NV_Item_Taf_Spm_Emc_Custom_Cfg = 2524,                      /* TAF_NVIM_SPM_EMC_CUSTOM_CFG_STRU */

/*  2548 */     en_NV_Item_SMS_MO_CUSTOMIZE_INFO = 2548,                      /* TAF_NV_SMS_MO_CUSTOMIZE_INFO_STRU */

/*   3000*/      en_NV_Item_ErrLog_CsHo_Len = 3000,
 /* 3001 */      en_NV_Item_SBM_CUSTOM_DUAL_IMSI_NEW = 3001,

/* 3002 */      en_NV_Item_GAS_UTRAN_TDD_DEFAULT_Q_RXLMIN = 3002,

/* Added by yangsicong for L2G REDIR C1 CUSTUME, 2015-1-26, begin */
/* 3003 */      en_NV_Item_GAS_C1_Calc_Opt_White_Plmn_List = 3003,
/* Added by yangsicong for L2G REDIR C1 CUSTUME, 2015-1-26, end */

/* 3004 */      en_NV_Item_ErrLog_Stay_Self_Rat_Timer_Threshold      = 3004,

/* 3005 */  en_NV_Item_GSM_RAPID_HO_CUSTOMIZE_CFG     = 3005,
/* 3006 */      en_NV_Item_Was_DMCR_CFG      = 3006,
/* 3007 */      en_NV_Item_Was_T320          = 3007,

                en_NV_Item_Csfb_Mcc_Band_Info = 3008,

/* 3009 */     en_NV_Item_GAS_GSM_BAND_CUSTOMIZE_CFG    = 3009,
/* 3010 */     en_NV_Item_Was_Ac_CHECK_Ctrl_Info         = 3010,
/* 3011 */      en_NV_Item_SBM_CUSTOM_DCDPA_CTRL = 3011,

/* 3012 */      en_NV_Item_Was_Csfb_Search_Ctr_Info          = 3012,

/* 3013 */     en_NV_Item_GAS_GSM_CELL_HISTORY_BCCH_SI_CFG    = 3013,

/* 3014 */      en_NV_Item_GSM_SEARCH_CUSTOMIZE_CFG     = 3014,

/* 3015 */      en_NV_Item_GAS_FAST_AREA_LOST_CFG = 3015,

/* 3016 */      en_NV_Item_Inter_Rat_Resel_H_PRIO_Customization      = 3016,
/* 3017 */     en_NV_Item_Was_Eutra_Cell_Resel_Ctr_Info = 3017,

/* 3018 */      en_NV_Item_GAS_AUTO_FR_CFG      = 3018,

/* 3019 */      en_NV_Item_Was_Report_Connected_Location_Info    = 3019,
/* 3024 */      en_NV_Item_Was_Capbility_FddList_Ctr_Info    = 3024,
/* 3020 */      en_NV_Item_Was_Radom_Acc_Ctr_Info            = 3020,

/* 3021 */      en_NV_Item_GAS_INTER_RAT_RESEL_CFG     = 3021,

/* 3022 */      en_NV_Item_Was_Noise_Optimize_Ctr_Info       = 3022,
/* 3023 */      en_NV_Item_WCDMA_HISTORY_PLMN_FREQ_LIST      = 3023,
/* 3025 */     en_NV_Item_Was_Cell_Indi_Offset_Ctrl_Info = 3025,

/* 3026 */      en_NV_Item_GAS_PSEUD_BTS_IDENT_CUST_CFG      = 3026,

/* 3027 */      en_NV_Item_GSM_PARALLEL_SEARCH_CUSTOMIZE_CFG    = 3027,

/* 3028 */     en_NV_Item_Was_Cu_Cell_Search_Ctrl_Info       = 3028,
/* 3029 */     en_NV_Item_Was_Operator_Freq_List_Info   = 3029,  /* ��Ӫ�̶���Ƶ�� */
/* 3030 */     en_NV_Item_Was_Broken_Cell_Info   = 3030,         /* BrokenCellList */

/* 3031 */      en_NV_Item_GSM_NETWORK_SEARCH_CUSTOMIZE_CFG             = 3031,
/* 3032 */      en_NV_Item_GSM_OPERATOR_CUSTOMIZE_FREQ_LIST_PART1       = 3032,
/* 3033 */      en_NV_Item_GSM_OPERATOR_CUSTOMIZE_FREQ_LIST_PART2       = 3033,
/* 3034 */      en_NV_Item_GSM_PREFER_PLMN_CUSTOMIZE_CFG                = 3034,

/* 3035 */      en_NV_Item_GSM_ENABLE_HISTORY_ARFCN_WITH_SPEC_ARFCN_LST = 3035,

                 en_NV_Item_GAS_RR_CONNECT_FAIL_PUNISH_CFG              = 3036,
/* 3037 */en_NV_Item_WAS_NETWORK_SEARCH_CUSTOMIZE_CFG                   = 3037,

/* 3038 */      en_NV_Item_GSM_PING_PONG_HO_CUSTOMIZE_CFG               = 3038,
               en_NV_Item_Was_Gsm_Meas_Offset_Ctrl_Info                 = 3039,
/* 3040 */      en_NV_Item_GAS_SAME_LAI_PREFER_CFG                      = 3040,

/* 3041 */     en_NV_Item_WAS_T314_CFG                                  = 3041,

/* 3042 */     en_NV_Item_GSM_LTE_MEASURE_CFG                           = 3042,

/* 3043 */      en_NV_Item_GSM_HO_CUSTOMIZE_CFG                         = 3043,

/* 3044 */     en_NV_Item_GSM_SEC_RXQUAL_SUB_ALPHA_FILTERING_CFG        = 3044,

               en_NV_Item_Plmn_List_Search_Threshold                    = 3045,

/* 3046 */     en_NV_Item_GSM_PMR_CFG                                   = 3046,

/* 3047 */     en_NV_Item_GSM_GCBS_MESSAGE_CUSTOMIZE_CFG                = 3047,

               en_NV_Item_WAS_Rlc_Capbility                             = 3048,

/* 3049 */     en_NV_Item_GAS_GSM_ACTIVE_CELL_RESELECT_CFG              = 3049,

/* 3050 */     en_NV_Item_GAS_GSM_PASSIVE_RESELECT_OPTIMIZE_CFG         = 3050,

/* 3051 */     en_NV_Item_WAS_CAMP_HANDLE_CUSTOMIZE_CFG                 = 3051,

			   en_NV_Item_WAS_History_Rssi_Search_Info					= 3052,
/* 3053 */     en_NV_Item_GAS_GSM_SACCH_BA_INHERIT_OPTIMIZE_CFG         = 3053,

/* 3054 */     en_NV_Item_GAS_GSM_PAGE_RCV_CFG                          = 3054,

/* 3055 */     en_NV_Item_WAS_OUT_SERVICE_RESEL_THRES                   = 3055,
/* 3057 */    en_NV_Item_Was_Erach_Broken_Cell_Info                     = 3057,         /* Erach BrokenCellList */
/* 3058 */    en_NV_Item_Was_Erach_Sys_ChgInd_Ctrl                      = 3058,         /* Erach Sys change */
/* 3059 */     en_NV_Item_WAS_Errlog_Active_Rpt_Time_Interval_Ctrl      = 3059,

/* 3060 */     en_NV_Item_WAS_NW_REL_TIMER_CTRL                         = 3060,
/* 3061 */     en_NV_Item_WAS_MFBI_CTRL                                 = 3061,
/* 3062 */     en_NV_Item_WAS_SValue_CUSTOMIZE_CFG                      = 3062,

/* 3063 */     en_NV_Item_GAS_Revision_Level_Customization              = 3063,

               en_NV_Item_WAS_GEO_SEARCH_FOR_SIB1                       = 3064,
/* 3065 */     en_NV_Item_Was_Multi_Modem_History_Freq_Share_Cfg        = 3065,
/* 3066 */     en_NV_Item_Was_Net_Srch_Rmv_Inter_Rat_Freq_Band_Ctrl     = 3066,
/* 3067 */     en_NV_Item_GAS_Net_Srch_Rmv_Inter_Rat_Freq_Cfg           = 3067,

/* 3068 */     en_NV_Item_GUAS_CELLULAR_PREFER_PARA_CFG               = 3068,

/* 3069 */     en_NV_Item_WAS_W2L_Resel_Punish_CUSTOMIZE_CFG            = 3069,
/* 3070 */     en_NV_Item_Was_Recal_Active_Time_Ctrl     = 3070,
/* 3071 */     en_NV_Item_Was_Print_Level_Ctrl                          = 3071,

/* 3072 */     en_NV_Item_WAS_Ho2Eutran_Cfg                  = 3072,
/* 3073 */     en_NV_Item_WAS_EutranMeas_Cfg                 = 3073,
/* 3074 */     en_NV_Item_WAS_FDPCH_EFDPCH_Cfg               = 3074,
/* 3075 */     en_NV_Item_WAS_CPC_Cfg                        = 3075,
/* 3076 */     en_NV_Item_WAS_EFACH_Cfg                      = 3076,
/* 3077 */     en_NV_Item_GUAS_AcBar_Cfg                     = 3077,

/* 3079 */     en_NV_Item_WAS_EDRX_Cfg                                  = 3079,

/* 3080 */     en_NV_Item_Was_Treselection_Customize_Cfg                = 3080,

/* 3078 */     en_NV_Item_Was_SIB_PRECISE_RCV_CFG                       = 3078,
/* 3082 */     en_NV_Item_GAS_MNTN_CHR_DIRECT_RPT_CFG                   = 3082,
/* 3083 */      en_NV_Item_Was_Fake_DMCR_CFG                 = 3083,
/* 3084 */     en_NV_Item_GAS_Csfb_Lai_Penalty_Cfg           = 3084,
/* 3085 */     en_NV_Item_Was_Sys_Info_Chg_Fail_Optimize_Cfg = 3085,
/* 3086 */     en_NV_Item_GUAS_Dsds_Need_Apply_Search_Ctrl               = 3086,
/* 3087 */     en_NV_Item_WAS_OOS_Cell_Search_Optimize_Cfg               = 3087,
/* 3088 */     en_NV_Item_Was_Errlog_Threshold_Cfg = 3088,
/* 3089 */     en_NV_Item_Was_Initial_Sir_Target_Cfg         = 3089,
/* 3090 */     en_NV_Item_Was_NotAllowed_InterRat_Resel_Cfg  = 3090,
/* 3091 */     en_NV_Item_GAS_Network_Not_Alloc_Utran_Priority_Cfg       = 3091,
/* 3094 */     en_NV_Item_Was_Meas_Evt_Eval_Customize_Cfg                = 3094,
/* 3095 */     en_NV_Item_GUAS_Csfb_Redirect_Optimize_Cfg                = 3095,
/* 3096 */     en_NV_Item_Was_Direct_Chr_Evt_Cfg             = 3096,
/* 3098 */     en_NV_Item_Was_Emergency_Camp_Cfg             = 3098,

/* 3104 */     en_NV_Item_Gsm_Irat_Active_Resel_Custom_Cfg   = 3104,

/* 3107 */     en_NV_Item_Was_Inter_Freq_Handover_Cust_Cfg   = 3107,

/* 3123  */    en_NV_Item_WAS_Bar_Cell_For_Fach_Count_In_Sib_Is_Zero_Cfg = 3123,

/* 3149  */    en_NV_Item_WAS_Primary_Plmn_Compare_Cfg       = 3149,

/* 3500 */     en_NV_Item_CAS_1X_RC_PREF                     = 3500,
/* 3501 */     en_NV_Item_CAS_1X_NVIM_TERMINAL_INFO          = 3501,
/* 3502 */     en_NV_Item_CAS_1X_NVIM_CELL_THRESHOLD         = 3502,
/* 3503 */     en_NV_Item_CAS_1X_NVIM_ENCRYPT_CAPA           = 3503,
/* 3504 */     en_NV_Item_CAS_1X_NVIM_CHANNEL_CFG_CAPA_INFO  = 3504,
/* 3505 */     en_NV_Item_CAS_1X_NVIM_CAPA_INFO              = 3505,
/* 3506 */     en_NV_Item_CAS_1X_NVIM_RATE_FEATURE_CAPA_INFO = 3506,
/* 3507 */     en_NV_Item_CAS_1X_NVIM_MO_CAPA                = 3507,
/* 3508 */     en_NV_Item_CAS_1X_NVIM_IMSI_TYPE_INFO         = 3508,
/* 3509 */     en_NV_Item_CAS_1X_NVIM_PROTOCOL_OPTIMIZE      = 3509,
/* 3510 */     en_NV_Item_CPROC_1X_NVIM_FILTER_COEF          = 3510,
/* 3511 */     en_NV_Item_CPROC_1X_NVIM_OPTIMIZE_SWITCH      = 3511,
/* 3512 */     en_NV_Item_CPROC_1X_NVIM_SM_MPS_THRESHOLD     = 3512,
/* 3513 */     en_NV_Item_CPROC_1X_NVIM_TAS_PARA_INFO        = 3513,
/* 3514 */     en_NV_Item_CAS_1X_NVIM_TIMER_LENGTH           = 3514,
/* 3515 */     en_NV_Item_CAS_1X_NVIM_TCH_MEAS_FILTER_INFO   = 3515,

/* 3516 */     en_NV_Item_CAS_1X_NVIM_PM_LOG_THRESHOLD_INFO  = 3516,

/* 3517 */     en_NV_Item_CAS_1X_NVIM_TCH_MEAS_REPORT_OPTIMIZE          = 3517,
/* 3518 */     en_NV_Item_CAS_1X_NVIM_TCH_RELEASE_OPTIMIZE   = 3518,


/* 3519 */     en_NV_Item_CAS_1X_NVIM_VOWIFI_THRESH          = 3519,

/*        NV ID for CPROC_1X,from 3570 to 3599, begin         */

/* 3570 */     en_NV_Item_CPROC_1X_NVIM_DM_THRESHOLD                    = 3570,
/* 3571 */     en_NV_Item_CPROC_1X_NVIM_DM_TESTMODE_THRESHOLD           = 3571,

/* 3572 */     en_NV_Item_CPROC_1X_NVIM_STATISTICS_THRESHOLD            = 3572,

/*        NV ID for CPROC_1X,from 3570 to 3599, end           */


/* 3600 */     en_NV_Item_CAS_HRPD_NVIM_AT_REV                          = 3600,
/* 3601 */     en_NV_Item_CDMA_SUPPORT_BANDCLASS_MASK                   = 3601,
/* 3602 */     en_NV_Item_CAS_HRPD_NVIM_CELL_THRESHOLD                  = 3602,
/* 3603 */     en_NV_Item_CAS_HRPD_NVIM_SUSPEND_TIMER                   = 3603,
/* 3604 */     en_NV_Item_CAS_HRPD_NVIM_CONNCLOSE_TO_1X_SWITCH          = 3604,
/* 3605 */     en_NV_Item_CAS_HRPD_NVIM_C2L_PARA                        = 3605,
/* 3606 */     en_NV_Item_CAS_HRPD_NVIM_IDLE_HO_PARA                    = 3606,
/* 3607 */     en_NV_Item_CAS_HRPD_NVIM_CFG_PARA                        = 3607,
/* 3608 */     en_NV_Item_CAS_HRPD_NVIM_TIMER                           = 3608,

/* 3609 */     en_NV_Item_CAS_HRPD_NVIM_SESSION_CFG_DATA_PART1          = 3609,
/* 3610 */     en_NV_Item_CAS_HRPD_NVIM_SESSION_CFG_DATA_PART2          = 3610,

/* 3611 */     en_NV_Item_CAS_HRPD_NVIM_SWITCH_PARA                     = 3611,

/* 3612 */     en_NV_Item_CAS_HRPD_NVIM_STRENGTH_FILTER_RATIO           = 3612,

/* 3613 */     en_NV_Item_CAS_HRPD_NVIM_INTERFREQ_MEAS_THRESHOLD        = 3613,

/* 3614 */     en_NV_Item_CAS_HRPD_NVIM_EUTRA_CELL_RESEL_CTR_INFO       = 3614,

/* 3615 */     en_NV_Item_CAS_NVIM_CHR_REPORT_CTR_INFO                  = 3615,

/* 3616 */     en_NV_Item_CAS_HRPD_NVIM_PM_LOG_THRESHOLD_INFO           = 3616,

/* 3671 */     en_NV_Item_CPROC_HRPD_NVIM_PILOT_SEARCH_CTRL             = 3671,

/* 3672 */     en_NV_Item_CPROC_RM_NVIM_1X_OCCUPY_INFO                  = 3672,

/* 3673 */     en_NV_ITEM_CPROC_HRPD_NVIM_OPTIMIZE_SWITCH               = 3673,

/* 3674 */     en_NV_Item_CPROC_HRPD_NVIM_TCH_HO_CTRL                   = 3674,

/* 3675 */     en_NV_Item_CPROC_RM_NVIM_1X_PAGING_FAIL_MAX_CNT          = 3675,

/* 3676 */     en_NV_Item_CPROC_HRPD_NVIM_MEASUREMENT_CTRL              = 3676,

/* 3677 */     en_NV_Item_CPROC_HRPD_NVIM_STATISTICS_THRESHOLD          = 3677,

/* 3678 */     en_NV_Item_CPROC_HRPD_NVIM_MEASUREMENT_FILTER_FACTOR     = 3678,


/*   4000 */     en_NV_Item_DDR_ADJUST_CFG  = 4000,
/*   4001 */     en_NV_Item_Usim_App_Priority_Cfg = 4001,
/* 4002 */       en_NV_Item_PESN   = 4002,
/* 4003 */       en_NV_Item_MEID   = 4003,
/* 4004 */       en_NV_Item_SLEEP_BBE16_CTRL    = 4004,
/* Added by zhuli 2015-10-16 begin */
/* 4005 */       en_NV_Item_Cdma_Test_Card_Cfg                          = 4005,
/* 4006 */       en_NV_Item_STK_Support_Feature_Cfg                     = 4006,
/* Added by zhuli 2015-10-16 begin */

/* 4007 */       en_NV_Item_SC_PERS_Support_Platform_Cfg                = 4007,
/* 4008 */       en_NV_Item_NV_SC_PERS_CTRL_CFG                         = 4008,
/* 4009 */       en_NV_Item_Xpds_Feature_Ctrl_Cfg                       = 4009,
/* 4010 */       en_NV_Item_Isim_Enable_Cfg                             = 4010,

/* 4011 */       en_NV_Item_Usim_Uicc_App_Priority_Cfg                  = 4011,

/* 4012 */       en_NV_Item_Usimm_Ack_Protect_Ctrl                      = 4012,

/* 4013 */       en_NV_Item_Usimm_Check_KeyFile_Ctrl                    = 4013,

/* 4014 */       en_NV_Item_Disable_Real_Isim_Ctrl                      = 4014,

/* 4015 */       en_NV_Item_IMEI_Bind_Slot_Ctrl                         = 4015,

/* 4045 */       en_NV_Item_USIM_CLOSE_CHANN_SND_STATUS_Cust            = 4045,

/* 4046 */       en_NV_Item_Provide_Command_Ctrl                        = 4046,

/* 4072 */       en_NV_Item_Usim_Status_ProtectReset_CheckSW            = 4072,

/* 4074 */       en_NV_Item_Usim_Plmn_Digit_Ctrl                        = 4074,

/* 4500 */       en_NV_Item_TTF_PPPC_CONFIG_OPTIONS                       = 4500,

/* 4501 */       en_NV_Item_CTTF_MAC_PROBE_INIT_POWER_CTRL                = 4501,

/* 4502 */       en_NV_Item_TTF_TPE_CTRL                                  = 4502,

/* 4503 */       en_NV_Item_HRPD_RfAllocSwitchMask                        = 4503,

/* 4504 */       en_NV_Item_CTTF_FID_CORE_BIND_CONFIG                     = 4504,

/* 4800 */       en_NV_Item_WTTF_SRB_NOT_SEND_THRESHOLD                 = 4800,

/* 4801 */       en_NV_Item_NODE_RESET_CTRL                             = 4801,

/* 4802 */       en_NV_Item_TTF_CICOM_IP_NUMBER                         = 4802,

/* 4803 */       en_NV_Item_PPP_CONFIG                                  = 4803,

/* 4804 */       en_NV_Item_TTF_SEQ_OUT_OF_ORDER_COMPATIBLE             = 4804,

/* 4805 */       en_NV_Item_TTF_CORE_BIND_CONFIG                        = 4805,

/* 4807 */       en_NV_Item_WTTF_CSPS_RLC_PS_NOT_RPT_DATALINK_LOSS      = 4807,

/* 4808 */       en_NV_Item_RLC_Flow_Control                            = 4808,

/*   4900  */    en_NV_Item_Bastet_CONFIG = 4900,
/*   8250*/      en_NV_Item_GCF_TYPE_CONTENT_ID = 8250,
/*   8517*/      en_NV_Item_ENHANCE_SIMCARD_LOCK_STATUS = 8517,
/*   8518*/      en_NV_Item_GENHANCE_SIMCARD_REMAIN_TIMES = 8518,
/*   8524*/      en_NV_Item_TOTOLPC_PARA_CTRL = 8524,
/*   8525*/      en_NV_Item_Wcdma_OLPC_MapWeight_Para = 8525,
/*   9037*/      en_NV_Item_Flow_Ctrl_Config         = 9037,
/*   9040*/      en_NV_Item_WIFI_INFO                = 9040,
/*   9043 */     en_NV_Item_MED_CODEC_TYPE           = 9043,

/*  9046 */      en_NV_Item_Ho_Wait_Sysinfo_Timer_Config    = 9046,

/* Added by f62575 for C50_IPC Project, 2012/02/23, begin */
/*   9049 */     en_NV_Item_FDN_Info                 = 9049,
/* 9050  */      en_NV_Item_CSFB_PPAC_Para           = 9050,                  /* WAS����PPAC NV�� */
/* Added by f62575 for C50_IPC Project, 2012/02/23, end   */
/*   9051*/      en_NV_BREATH_TIME = 9051,
/* ����LTE�������ζ���NV */
/* 9052 */       en_NV_Item_Lte_Internation_Roam_Config     = 9052,

/* ���Ӳ��ű���11,12,13,15����NV */
/* 9053 */       en_NV_Item_Dail_Reject_Config              = 9053,

/* ���ӹرն��Ŷ���NV */
/* 9054 */       en_NV_Item_Close_SMS_Capability_Config     = 9054,

/*  9065 */      en_NV_Item_XO_DEFINE               = 9065,

/*  9041 */      en_NV_Item_SubPlatFormInfo      = 9041,
/*  9070 */      en_NV_Item_Eqver                        = 9070,
/*  9071 */      en_NV_Item_Csver                        = 9071,

/* 9066 */       en_NV_Item_Fastdorm_Enable_Flag         = 9066,
/*   9067 */     en_NV_Item_Report_Cell_Sign              = 9067,/* WAS����NV��ϱ�С���ź�ǿ�� */
                 en_NV_Item_ETWS_Service_Cfg_Para = 9068,                              /* ETWS ��������� */
/*   9069*/      en_NV_Item_WAS_Customized_Para        = 9069,       /* WAS����NV��洢����NV���� */
/* 9072 */       en_NV_Item_FC_ACPU_DRV_ASSEMBLE_PARA  = 9072, /* ��̬�����������������λ��Ϣ */

/* Added by L60609 for V7R1C50 AT&T&DCM, 2012-6-11, begin */
/* 9080 */       en_NV_Item_Scan_Ctrl_Para             = 9080,
/* 9081 */       en_NV_Item_Att_Ens_Ctrl_Para          = 9081,
/* 9082 */       en_NV_Item_Pdp_Act_Limit_Para         = 9082,
/* Added by L60609 for V7R1C50 AT&T&DCM, 2012-6-11, end */
/* 9083 */       en_NV_Item_REPORT_ECC_NUM_SUPPORT_FLAG = 9083,
/* 9084 */       en_NV_Item_CUSTOM_ECC_NUM_LIST         = 9084,
/* 9085 */       en_NV_Item_ACTING_PLMN_SUPPORT_FLAG    = 9085,
/* 9086 */       en_NV_Item_HPLMN_SEARCH_REGARDLESS_MCC_SUPPORT = 9086,
/* 9087 */       en_NV_Item_SINGLE_DOMAIN_FAIL_ACTION_LIST      = 9087,
/* 9088 */       en_NV_Item_CS_FAIL_NETWORK_FAILURE_PLMN_SEARCH_FLAG = 9088,
/* 9089 */       en_NV_Item_REPORT_PLMN_SUPPORT_FLAG                 = 9089,

                 /* Added by h59254 for V7R1C50 AT&T, 2012-6-29, begin */
/* 9090 */       en_NV_Item_Usim_Support_Feature_Config              = 9090,
                 /* Added by h59254 for V7R1C50 AT&T, 2012-6-29, begin */

/* 9091 */       en_NV_Item_UTRAN_TDD_FREQ_BAND		   = 9091, /* ָʾTDS Band��Ϣ */

/* 9092 */       en_NV_Item_Utran_Mode                 = 9092,                  /* ָʾ��ǰUTRANģʽΪWCDMA����TD-SCDMA */
/* 9094  */      en_NV_Item_CALL_CallNotSupportedCause = 9094,

/* Added by L60609 for MUX��2012-08-08,  Begin */
/* 9100 */       en_NV_Item_Mux_Support_Flg            = 9100,
/* Added by L60609 for MUX��2012-08-08,  End */

/* 9102 */       en_NV_Item_User_Cfg_Ehplmn_Info       = 9102,                  /* �û����õ�EHplmn NVIM ID */
/* 9103 */       en_NV_Item_Utran_Mode_Auto_Switch     = 9103,                  /* �ڵ�ǰ��GUTL�汾��֧�����ó�GUL�汾,GTL�汾���Լ�GUTL�汾����NVIM ID */
/* 9104 */       en_NV_Item_Disabled_Rat_Plmn_Info     = 9104,                  /* ��ֹ�����뼼����PLMN��Ϣ */

/* Modify by z60575 for multi_ssid, 2012-9-25 begin */
/* 9110 */       en_NV_Item_MULTI_WIFI_KEY             = 9110,       /* ֧�ֶ���SSID��չ����WIKEY NV�� */
/* 9111 */       en_NV_Item_MULTI_WIFI_STATUS_SSID     = 9111,       /* ֧�ֶ���SSID��չ����SSID NV�� */
/* Modify by z60575 for multi_ssid, 2012-9-25 end */

/* 9112 */      en_NV_Item_UMTS_CODEC_TYPE              = 9112,

/* 9113 */       en_NV_Item_AT_ABORT_CMD_PARA          = 9113,
/* 9116 */       en_NV_Item_CCallState_Rpt_Status           = 9116,             /* ^CCALLSTATE���������ϱ�״̬ */
/* 9118 */       en_NV_Item_SMS_MT_CUSTOMIZE_INFO       = 9118,

 /* 9119  */     en_NV_Item_CS_Call_Redial_CFG = 9119,

 /* 9120  */     en_NV_Item_QSearch_Customization = 9120,

/* 9121 */       en_NV_Item_Gsm_Poor_RxQual_ThresHold = 9121,

/* 9122 */       en_NV_Item_Hplmn_Register_Ctrl_Flg = 9122,
/*9123*/        en_NV_Item_CELL_SIGN_REPORT_CFG        = 9123,
/* 9124 */       en_NV_Item_LOW_POWER_Cell_Resel_OffSet = 9124,
/* 9125 */       en_NV_Item_MUX_REPORT_CFG = 9125,
/* 9126 */       en_NV_Item_Usim_OpFile_Init_Cfg = 9126,
/* 9127 */       en_NV_Item_Sim_OpFile_Init_Cfg = 9127,
/* 9128 */       en_NV_Item_CIMI_PORT_CFG = 9128,
/* Added by h59254 2013/01/04, begin */
/* 9129 */       en_NV_Item_Test_Card_Cfg = 9129,
/* Added by h59254 2013/01/04, end */

/* Added by l60609 for V9R1 IPv6&TAF/SM Project, 2013-4-24, begin */
/* 9130 */       en_NV_Item_IPV6_BACKPROC_EXT_CAUSE = 9130,
/* Added by l60609 for V9R1 IPv6&TAF/SM Project, 2013-4-24, end */

                 en_NV_Item_SMS_TIMER_LENGTH = 9131,
                 en_NV_Item_SS_CUSTOMIZE_PARA = 9132,
/* 9133 */       en_NV_Item_UCS2_Customization = 9133,

/* 9134 */       en_NV_Item_Csfb_Customization = 9134,

/* 9200 */       en_NV_Item_WTTF_MACDL_WATERMARK_LEVEL_Config = 9200, /* BBP�����ж�ˮ�ߵȼ�������Ϣ */
/* 9201 */       en_NV_Item_WTTF_MACDL_BBPMST_TB_HEAD_Config  = 9201, /* BBPMSTͷ��TB����Ϣ */

/* Added by l60609 for DSDA Phase II, 2012-12-13, Begin */
/* 9202 */       en_NV_Item_AT_CLIENT_CONFIG    = 9202,                           /* ˫��˫ͨ����ATͨ�������ĸ�Modem��Ϣ */
/* Added by l60609 for DSDA Phase II, 2012-12-13, End */

/* 9203 */       en_NV_Item_Platform_RAT_CAP        = 9203,
/* 9205 */       en_NV_Item_BBP_DUMP_ENABLE             = 9205,  /* BBP ���ɿ��� */
/* 9206 */       en_NV_Item_SUPPORT_RATMODE_MASK    = 9206, /* ��֪�����ÿ��MODEM֧�ֵ�ģʽ */

/* 9207 */       en_NV_Item_Clvl_Volume         = 9207,

/* 9206 */       en_NV_Item_BODY_SAR_PARA               = 9208,

/* 9209 */       en_NV_Item_Ext_Tebs_Flag              = 9209, /* TTF BO��չ���� */
/* 9210 */       en_NV_Item_TCP_ACK_Delete_Flg         = 9210, /* TTF �ɵ�TCP ACKɾ������ */

/* 9211 */       en_NV_Item_H3g_Customization          = 9211,

/* 9212 */       en_NV_Item_TEMP_PROTECT_CONFIG = 9212,

/*9213*/         en_NV_Item_THERMAL_TSENSOR_CONFIG          = 9213, /* �±�tsensor����NV */
/*9214*/         en_NV_Item_THERMAL_BAT_CONFIG              = 9214, /* �±���ؿ���NV */
/*9215*/         en_NV_Item_THERMAL_HKADC_CONFIG            = 9215, /* �±�HKADCͨ������NV */
/*9216*/         en_NV_Item_HKADC_PHYTOLOGICAL_CONFIG       = 9216, /* �±������߼�ת������NV */
/*9217*/         en_NV_Item_NV_TCXO_CFG                     = 9217, /* TCXO���ò��� */

/*9218*/         en_NV_Item_SHARE_PDP_INFO                  = 9218, /* Share PDP���Կ���NV */
/*9219*/         en_NV_Item_DEACT_USIM_WHEN_POWEROFF        = 9219, /* AT+CFUN=0������ػ���ȥ����SIM�����ܿ���NV */
                en_NV_Item_Enable_Lte_Timer_Len = 9220,

/* Added by f62575 for VSIM FEATURE, 2013-8-29, begin */
                en_Item_NAS_VSIM_CTRL_FEATURE               = 9224,
/* Added by f62575 for VSIM FEATURE, 2013-8-29, end */

/*   9225  */   en_NV_Item_CSFB_RAU_FOLLOW_ON_FLAG = 9225,

/* 9227 */      en_NV_Item_ChangeRegRejectCause_Flg           = 9227,

/* 9228  */      en_NV_Item_ACC_BAR_PLMN_SEARCH_FLG = 9228,           /*���Ʒ�HPLMN��RPLMN�����ֹʱ�Ƿ���Ҫ���� */
/* 9229  */      en_NV_Item_USER_CFG_OPLMN_LIST = 9229,               /*�����û�Ԥ��OPLMN�б���SIM�����⣬���ȼ�����SIM�� */

/* 9230  */      en_NV_Item_Rat_Forbidden_List_Config = 9230,

/* 9231 */       en_NV_Item_SMC_Ctrl_Flg                    = 9231,             /* UTRAN SMC����NV */

/* 9232  */      en_NV_Item_HKADC_CHANNEL_CONFIG = 9232, /* �±�ͨ������ */

/* 9233 */      en_NV_Item_Dfs_Dsflow_Rate_Config     = 9233,

/* 9234 */       en_NV_Item_DSDA_PLMN_SEARCH_ENHANCED_CFG = 9234,

/* 9235 */       en_NV_Item_VOICE_TEST_FLAG       = 9235,


/* 9236  */      en_NV_Item_USER_CFG_OPLMN_EXTEND_LIST = 9236,  /*�����û�Ԥ��OPLMN�б�(֧��256��OPLMN)����SIM�����⣬���ȼ�����SIM�� */

/* 9237  */      en_NV_Item_SMS_DOMAIN                         = 9237,

               en_NV_Item_3G_TO_2G_Config      = 9238,
/* 9239 */       en_NV_Item_UART_SWITCH_CFG = 9239,
                en_NV_Item_WAIT_IMS_VOICE_AVAIL_Timer_Len      = 9240,

                 en_NV_Item_REDIAL_IMS_TO_CS_DOMAIN          = 9241,
                 en_NV_Item_IMS_ROAMING_SUPPORT_FLG          = 9242,

                en_NV_Item_IMS_VOICE_MOBILE_MANAGEMENT       = 9243,
/* 9244 */       en_NV_MODEM_RF_SHARE_CFG          = 9244,
/* 9246  */      en_NV_Item_CS_ONLY_DATA_SERVICE_SUPPORT_FLG = 9246,            /* �û�����NV: PSע�ᱻ��ֹ����£��Ƿ���������ҵ�񴥷�ע��ı�־ */
/* 9247  */      en_NV_Item_Ignore_Auth_Rej_CFG = 9247,
/* 9248 */       en_NV_Item_GSM_C1_CUSTOMIZE = 9248,
/* 9249 */       en_NV_Item_HIGH_PRIO_RAT_HPLMN_TIMER_INFO = 9249,
/*9250*/         en_NV_Item_LTE_DISABLED_USE_LTE_INFO_FLAG = 9250,            /* L������֧��ʱgmm��rauʱ�Ƿ���Ҫ��l��ȡ��ȫ�����Ļ�gutiӳ����Ϣ��VOS_TRUE:L disable��rau��Ҫ��l��ȡ��Ϣ��VOS_FALSE:L disable��rau����Ҫ��l��ȡ��Ϣ */
/* 9251 */      en_NV_Item_SBM_CUSTOM_DUAL_IMSI        = 9251,       /* WAS����NV��洢����˫imsi�Ŀ����Ƿ�� */
/* 9252 */      en_NV_Item_Roam_Search_Rplmn_Config    = 9252,
/* 9254 */       en_NV_Item_IMS_USSD_SUPPORT_FLG            = 9254,
/* 9256  */      en_Nv_Item_RCM_PARA_CONFIG                  = 9256,

/* 9257 */       en_NV_Item_GSM_DSDS_PS_CONFIG               = 9257,
/* 9258  */      en_NV_Item_PHY_MODE_DSDS_FLAG               = 9258,
/* 9259 */       en_NV_Item_ECALL_CFG_INFO                  = 9259,
/* 9262 */       en_NV_Item_GSM_SSC_CONFIG_PARA              = 9262,      /* GSM SSCģ��NV��������  */

/* 9263 */       en_NV_Item_Multi_Dfs_Dsflow_Rate_CFG    = 9263,
/* 9266 */       en_NV_Item_Ultra_Flash_Csfb_Support_Flg    = 9266,
/* 9268 */       en_NV_Item_ADS_DYNAMIC_THRESHOLD_CFG        = 9268,

/* 9269 */       en_NV_Item_DMS_DEBUG_CFG = 9269,

/* 9270 */       en_NV_Item_3GPP2_Uplmn_Not_Pref_Flg    = 9270,

/* 9272 */       en_NV_Item_ROAMING_REJECT_NORETRY_CFG  = 9272,   /* ����NV: Other Plmn���յ�reject 17ʱ������ */

/* 9274  */      en_NV_Item_GSM_PCH_DUMMY_DETECT_FLAG    = 9274,

/* ATA�Ƿ��첽��ʽ�ϱ�OK NV���ƣ��첽��ʽ��������connectδ��
   ������connect ack���ϱ�ok�����첽��ʽ�ǵ��յ�����connect ack���ϱ�ok*/
/* 9276 */     en_NV_Item_Ata_Report_Ok_Async_Cfg = 9276,

/* 9278  */     en_NV_Item_W_HARQ_OUT_CONFIG = 9278,

/* 9279 */       en_NV_Item_W_Drx_Ctrl                  = 9279,         /* WCDMA�Ƿ�֧��DRX�Ĳ������� */
/* 9280 */       en_NV_Item_GSM_Drx_Ctrl                = 9280,         /* GSM�Ƿ�֧��DRX�Ĳ������� */

/* 9281 */       en_NV_Item_DSDS_END_SESSION_DELAY      = 9281,

/* 9282 */     en_NV_Item_Syscfg_Trigger_Plmn_Search_Cfg = 9282,

/* 9283 */       en_NV_Item_ESN_MEID = 9283,

/* 9284 */       en_Nv_Item_RCM_TAS_TABLE_CONFIG       = 9284,

/* 9285 */       en_Nv_Item_TAS_FUNC_CONFIG         = 9285,

/* 9286  */     en_NV_Item_Was_Freqbands_Priority  = 9286,

/* 9287 */       en_Nv_Item_TAS_CDMA_CONFIG         = 9287,

/* 9288 */       en_NV_Item_RCM_TEST_CTRL           = 9288,

/* 9289  */     en_NV_Item_GSM_HARQ_OUT_CONFIG      = 9289,

/* 9290 */       en_NV_MODEM_RF_SHARE_EX_CFG        = 9290,

/* 16042  */     en_NV_Item_W_T313_BACK_CTRL  = 16042,

/* 9284 */      en_Nv_Item_RCM_TAS_TABLE_CONFIG_TRI_MODEM   = 16043,

/* 16044  */     en_NV_Item_W_SYNC_MODULE_OPTIMIZE_PARA = 16044,
/* 16046 */     en_Nv_Item_GSM_PSRCH_PCELL_NUM   = 16046,

/* 16044  */     en_NV_Item_W_FET_CTRL = 16047,

/* 16077  */     en_NV_Item_W_SOFT_DEM_SYNC_CTRL = 16077,

/* 16088  */     en_NV_Item_W_MP_USEANT2_THRESHOLD = 16088,

/* 16091  */     en_NV_Item_W_FDPCH_SYNC_THRESHOLD = 16091,

/* 16095  */     en_NV_Item_W_DPA_MPO_ELUSION_THRESHOLD = 16095,
                 en_NV_Item_W_AFC_CALC_THRESHOLD   = 16096,

/* 16115  */     en_NV_Item_W_BCCH_AFC_ADJUST_CTRL  = 16115,

/* 16044  */     en_NV_Item_W_HSPA_CQI_REPLACE_TABLE_INFO = 16117,

/* 16121  */     en_NV_Item_W_MULTI_CELL_SEARCH_CTRL = 16121,

/* 16140 */       en_NV_Item_W_SLAVE_CS_PERIOD_CTRL  = 16140,


/* 20765 */      en_NV_Item_CDMA_PA_TEMP_DET_CHANNEL_BC0 = 20765,

/* 50009 */      en_NV_Item_TRAFFIC_CLASS_Type = GU_CUSTOM_EXTEND_NV_ID_MIN + 9,
/* 50012 */      en_NV_Item_WIFI_KEY = GU_CUSTOM_EXTEND_NV_ID_MIN + 12,
/* 50014 */      en_NV_Item_WIFI_MAC_ADDR = 50014,
/* 50016 */      en_NV_Item_BATTERY_TEMP_PROTECT = 50016,
/* 50018 */      en_NV_Item_SW_VER = 50018,
/* 50023 */      en_NV_Item_PRI_VERSION = 50023,
/* 50024  */     en_NV_Item_HUAWEI_NW_OPL_NAME_CUSTOMIZED = 50024,
/* 50025  */     en_NV_Item_PRIVATE_CMD_STATUS_RPT = 50025,
/* 50029  */     en_NV_Item_HUAWEI_IRAN_OPEN_PAGE_I = 50029,
/* 50031  */     en_NV_Item_MEAN_THROUGHPUT = 50031,
/* 50032  */     en_NV_Item_HUAWEI_PCCW_HS_HSPA_BLUE = 50032,
/* 50036  */     en_NV_Item_HUAWEI_CARDLOCK_PERM_EXT      = 50036,
/* 50037  */     en_NV_Item_HUAWEI_CARDLOCK_OPERATOR_EXT  = 50037,
/* 50041  */     en_NV_Item_NV_HUAWEI_DOUBLE_IMSI_CFG_I   = 50041,
/* 50048 */      en_NV_Item_PRODUCT_ID = 50048,
/* 50050  */     en_NV_Item_APN_Customize = 50050,
/* 50051  */     en_NV_Item_VIDEO_CALL = 50051,
/* 50052  */     en_NV_Item_CUST_USSD_MODE = 50052,
/* 50054  */     en_NV_Item_Forbidden_Band = 50054,
/* 50055  */     en_NV_Item_Enhanced_Hplmn_Srch_Flg = 50055,
/* 50056  */     en_NV_Item_SMS_CLASS0_TAILOR = 50056,
/* 50060  */     en_NV_Item_2G_DISABLE_SPARE_BIT3 = 50060,
/* 50061  */     en_NV_Item_PPP_DIAL_ERR_CODE = 50061,
/* 50063  */     en_NV_Item_Special_Roam_Flag = 50063,
/* 50064  */     en_NV_Item_MultiSimCallConf = 50064,
/* 50071  */     en_NV_Item_Cust_PID_Type = 50071,
/* 50091  */     en_NV_Item_Huawei_Dynamic_PID_Type = 50091,
/* 50201  */     en_NV_Item_SEC_BOOT_FLAG = 50201,
/* 50549 */      en_NV_Item_WCDMA_JAM_DETECT_CFG     = 50549,
/* 52000 */      en_NV_Item_WIFI_STATUS_SSID = 52000, /* used by mdrv == NV_ID_DRV_WIFI_STATUS_SSID */
/* 52001 */      en_NV_Item_WEB_ADMIN_PASSWORD = 52001,
/* 52002 */      en_NV_Item_AP_RPT_SRV_URL = 52002,
/* 52003 */      en_NV_Item_AP_XML_INFO_TYPE = 52003,
/* 52004 */      en_NV_Item_AP_XML_RPT_FLAG = 52004,
/* 52005 */      en_NV_Item_BATT_LOW_TEMP_PROTECT = 52005,
/* 52006 */      en_NV_Item_ISDB_DEFAULT_KEY = 52006,

/* 52008 */      en_NV_Item_REFRESH_TERMINAL_RESPONSE_CP_OR_AP = 52008,

                 en_NV_Item_SYS_Butt
};


#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* __SYSNVID_H__ */


