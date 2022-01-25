#ifndef __MNTN_PUBLIC_INTERFACE_H__
#define __MNTN_PUBLIC_INTERFACE_H__ 

#include <linux/mntn/rdr_types.h>
#define PMU_RESET_REG_OFFSET (PMIC_HRST_REG0_ADDR(0)<<2)
#define RST_FLAG_MASK (0xFF)
#define PMU_RESET_VALUE_USED 0xFFFFFF00
#define PMU_RESET_RECORD_DDR_AREA_SIZE 0x100
#define RECORD_PC_STR_MAX_LENGTH 0x48
typedef struct
{
 char exception_info[RECORD_PC_STR_MAX_LENGTH];
 unsigned long exception_info_len;
}
AP_RECORD_PC;
#define ETR_MAGIC_START "ETRTRACE"
#define ETR_MAGIC_SIZE ((unsigned int)sizeof(ETR_MAGIC_START))
typedef struct
{
    char magic[ETR_MAGIC_SIZE];
    u64 paddr;
    u32 size;
    u32 rd_offset;
}
AP_RECORD_ETR;
/*  ά�⿪�� */
enum himntnEnum
{
    HIMNTN_NVE_VALID = 0,
    HIMNTN_GOBAL_RESETLOG,
    HIMNTN_KERNEL_DUMP_ENABLE,
    HIMNTN_OOM_CALL_PANIC,
    HIMNTN_DIE_CALL_PANIC,
    HIMNTN_WDT_MIN,
    HIMNTN_RESERVED_WDT1 = HIMNTN_WDT_MIN,
    HIMNTN_WDT_MAX = HIMNTN_RESERVED_WDT1,
    HIMNTN_FTRACE,
    HIMNTN_RESERVED2,
    HIMNTN_BOTTOM
};

/* �쳣���� */
typedef enum
{
    REBOOT_REASON_LABEL0 = 0x0,             /* label0: bios�׶�ģʽ���� */
    AP_S_COLDBOOT = REBOOT_REASON_LABEL0,                   /* ����������ػ����һ�ο�����������һ�ο��� */
    BOOTLOADER = 0x01,                                                      /* ִ��adb reboot bootloader��fastboot reboot-bootloader����ʱ�ĸ�λ���� */
    RECOVERY = 0x02,                                                          /* ִ��adb reboot recovery����ȳ���ʱ�ĸ�λ���� */
    RESETFACTORY = 0x03,                                                  /* ִ��adb reboot resetfactory����ȳ���ʱ�ĸ�λ���ͣ��������ָ��������� */
    RESETUSER = 0x04,                                                        /* �û����ظ��������� */
    ERECOVERY = 0x08,                                                        /* ִ��adb reboot erecovery����ȳ���ʱ�ĸ�λ���� */
    USBUPDATE = 0x09,                                                        /* usb���� */
    HUNGDETECT = 0x0f,                                                      /* ����ϲ���������120����ǿ����������������ϵͳ */
    COLDBOOT = 0x10,                                                         /* �ȸ�λ����ִ��adb reboot����fastboot reboot����ʱ�ĸ�λ���� */
    AP_S_FASTBOOTFLASH = 0x13,                                       /* fastboot flash�����û������ֱ��fastboot oem bootʱ�ĸ�λ���� */
    REBOOT_REASON_LABEL1 = 0x14,            /* label1:Ӳ��ԭ��λ */
    AP_S_ABNORMAL = REBOOT_REASON_LABEL1,                   /* ����ͨ��ά���ܸ�λ��ϵͳ���������쳣 */
    AP_S_TSENSOR0 = 0x15,                                                 /* soc�±���λ */
    AP_S_TSENSOR1 = 0x16,                                                 /* soc�±���λ */
    AP_S_AWDT = 0x17,                                                        /* A�˹���λ */
    LPM3_S_GLOBALWDT = 0x18,                                          /* ȫ��Ӳ����λ */
    LPM3_S_LPMCURST = 0x1a,                                             /* lpmcuӲ����λ */
    AP_S_DDR_UCE_WD = 0x21,                                            /* ddrӲ����λ */
    AP_S_DDR_FATAL_INTER = 0X22,                                     /* ddr fatal�ж��ϱ��쳣 */
    REBOOT_REASON_LABEL2 = 0x24,            /* label2:ap���ԭ��λ */
    AP_S_PANIC = REBOOT_REASON_LABEL2,                    /* A��panic������ʷǷ���ַ */
    AP_S_NOC = 0x25,                                                    /* ͨ�����߷��������쳣 */
    AP_S_DDRC_SEC = 0x27,                                           /* ddr���ʱ��� */
    AP_S_F2FS = 0x28,                                                   /* F2FS�쳣 */
    AP_S_MAILBOX = 0x2b,                                             /* mailbox �쳣 */
    REBOOT_REASON_LABEL3 = 0x2c,            /* label3:����ģ�鸴λ */
    STARTUP_S_EXCEPTION = 0x2c,                               /* ����ģ�������쳣 */
    HEARTBEAT_S_EXCEPTION = 0x2d,                           /* ����ģ�������쳣 */
    LPM3_S_EXCEPTION = 0x32,                                     /* LPM3��ϵͳ��⵽�ĸ����쳣 */
    TS_S_EXCEPTION = 0x33,                                         /* TS��ϵͳ��⵽�ĸ����쳣 */
    AICPU_S_EXCEPTION = 0x34,                                   /* AICPU��ϵͳ��⵽�ĸ����쳣 */
    TEE_S_EXCEPTION = 0x38,                                       /* teeos�쳣 */
    REBOOT_REASON_LABEL4 = 0x40,            /* label4: */
    BR_REBOOT_CPU_BUCK = 0x55,                                      /* ��оƬ����buck�쳣��������λ */
    REBOOT_REASON_LABEL5 = 0x65,            /* label5:��Դ�쳣 */
    AP_S_PMU = 0x65,                                                         /* PMU���ɵ�Դ������⵽�쳣������������ */
    AP_S_SMPL = 0x66,                                                        /* �����쳣 */
    AP_S_SCHARGER = 0x67,                                                /* pmu��ǰ�����⣬����charger�͵�� */
    REBOOT_REASON_LABEL6 = 0x6A,            /* label6:xloader�쳣 */
    XLOADER_S_DDRINIT_FAIL = REBOOT_REASON_LABEL6,   /* xloader����ddr��ʼ��ʧ�� */
    XLOADER_S_LOAD_FAIL = 0x6C,                                     /* xloader������ؾ���ʧ�� */
    XLOADER_S_VERIFY_FAIL = 0x6D,                                  /* xloader����У�龵��ʧ�� */
    XLOADER_S_WATCHDOG = 0x6E,                                    /* xloader���湷��λ */
    REBOOT_REASON_LABEL7 = 0x74,            /* label7:fastboot�쳣 */
    FASTBOOT_S_PANIC = 0x75,                                         /* fastboot����panic */
    FASTBOOT_S_WATCHDOG = 0x76,                                 /* fastboot���湷��λ */
    FASTBOOT_S_OCV_VOL_ERR = 0x78,                             /* Fastboot�׶ι���Ƿѹ�쳣���������� */
    FASTBOOT_S_LOAD_DTIMG_ERR = 0x7B,                       /* fastboot����DT����ʧ�ܣ��������� */ 
    FASTBOOT_S_LOAD_OTHER_IMGS_ERR = 0x7C,             /* fastboot������������ʧ�ܣ��������� */
    FASTBOOT_S_KERNEL_IMG_ERR = 0x7D,                       /* fastboot����kernel����ʧ�ܣ��������� */
    FASTBOOT_S_LOADLPMCU_FAIL = 0x7E,                        /* fastboot����LPM3����ʧ�ܣ��������� */
    FASTBOOT_S_IMG_VERIFY_FAIL = 0x7F,                        /* fastboot����У�龵��ʧ�� */
    REBOOT_REASON_LABEL8 = 0x89,            /* label8: */
    REBOOT_REASON_LABEL9 = 0x90,            /* label9: */
    BFM_S_BOOT_NATIVE_BOOT_FAIL = REBOOT_REASON_LABEL9,         /* native boot fail */
    BFM_S_BOOT_TIMEOUT,                                                                 /* Boot timeout */
    BFM_S_BOOT_FRAMEWORK_BOOT_FAIL,                                 /* Framework boot fail */
    BFM_S_BOOT_NATIVE_DATA_FAIL,                                          /* Native data error */
    REBOOT_REASON_LABEL10 = 0xB0,           /* label10: */
} EXCH_SOURCE;

#define RDR_EXCEPTION_REASON_INVALID 0xFFFFFFFF

/* �쳣id�Σ���ģ����� */
enum MODID_LIST {
    HISI_BB_MOD_MODEM_DRV_START = 0x00000000,
    HISI_BB_MOD_MODEM_DRV_END = 0x0fffffff,
    HISI_BB_MOD_TS_START = 0x10000000,
    HISI_BB_MOD_TS_END = 0x1fffffff,
    HISI_BB_MOD_AICPU_START = 0x20000000,
    HISI_BB_MOD_AICPU_END = 0x2fffffff,
    HISI_BB_MOD_RESERVED3_START = 0x30000000,
    HISI_BB_MOD_RESERVED3_END = 0x3fffffff,
    HISI_BB_MOD_RESERVED4_START = 0x40000000,
    HISI_BB_MOD_RESERVED4_END = 0x4fffffff,
    HISI_BB_MOD_RESERVED5_START = 0x50000000,
    HISI_BB_MOD_RESERVED5_END = 0x5fffffff,
    HISI_BB_MOD_RESERVED6_START = 0x60000000,
    HISI_BB_MOD_RESERVED6_END = 0x6fffffff,
    HISI_BB_MOD_RESERVED7_START = 0x70000000,
    HISI_BB_MOD_RESERVED7_END = 0x7fffffff,
    HISI_BB_MOD_AP_START = 0x80000000,
    HISI_BB_MOD_AP_END = 0x81ffffff,
    HISI_BB_MOD_DRIVER_START = 0x82000000,
    HISI_BB_MOD_DRIVER_END = 0x82ffffff,
    HISI_BB_MOD_TEE_START = 0x83000000,
    HISI_BB_MOD_TEE_END = 0x83ffffff,
    HISI_BB_MOD_BIOS_START = 0x84000000,
    HISI_BB_MOD_BIOS_END = 0x84ffffff,
    HISI_BB_MOD_LPM_START = 0x85000000,
    HISI_BB_MOD_LPM_END = 0x85ffffff,
    HISI_BB_MOD_RESERVED10_START = 0x86000000,
    HISI_BB_MOD_RESERVED10_END = 0x86ffffff,
    HISI_BB_MOD_RESERVED11_START = 0x87000000,
    HISI_BB_MOD_RESERVED11_END = 0x87ffffff,
    HISI_BB_MOD_RESERVED_START = 0x88000000,
    HISI_BB_MOD_RESERVED_END = 0x9fffffff,
    HISI_BB_MOD_RESERVED12_START = 0xa0000000,
    HISI_BB_MOD_RESERVED12_END = 0xa0ffffff,
    HISI_BB_MOD_RESERVED13_START = 0xb0000000,
    HISI_BB_MOD_RESERVED13_END = 0xb0ffffff,
    HISI_BB_MOD_RANDOM_ALLOCATED_START = 0xc0000000,
    HISI_BB_MOD_RANDOM_ALLOCATED_END = 0xf0ffffff
};

/* ģ��bitλ�б� */
typedef enum CORE_LIST {
    RDR_AP        = 0x1,
    RDR_DRIVER = 0x2,
    RDR_TEEOS     = 0x4,
    RDR_RESERVED1 = 0x8,
    RDR_RESERVED2 = 0x10,
    RDR_TS        = 0x20,
    RDR_AICPU     = 0x40,
    RDR_RESERVED3 = 0x80,
    RDR_RESERVED4 = 0x100,
    RDR_RESERVED5 = 0x200,
    RDR_BIOS = 0x400,
    RDR_LPM3      = 0x800,
    RDR_CORE_MAX = 12,
} rdr_coreid; 

#endif
