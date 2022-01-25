#ifndef __RDR_EXCEPTION_PUB_H__
#define __RDR_EXCEPTION_PUB_H__

#define MODULE_MAGIC                0xbaba0514
#define MODULE_VALID                 1
#define MODULE_EXCEPTION_REGISTER_MAXNUM   512
#define RDR_MODULE_NAME_LEN  16
#define RDR_EXCEPTIONDESC_MAXLEN 48

typedef struct excep_time_t {
    u64 tv_sec;
    u64 tv_usec;
}excep_time, rdr_time_st;

struct exc_description_s{
    u32 e_modid;                                                 /* �쳣id */
    u8 e_process_level;                                        /* �쳣������:PROCESS_PRI */
    u8 e_reboot_priority;                                      /* �쳣��������:REBOOT_PRI */
    u8 e_exce_type;                                             /* �쳣���� */ 
    u8 e_reentrant;                                              /* �쳣�Ƿ������ */
    u64 e_notify_core_mask;                                /* �쳣�������� */
    u8 e_desc[RDR_EXCEPTIONDESC_MAXLEN];        /* �쳣���� */ 
};

struct exc_info_s{
    excep_time e_clock;                                         /* ģ�鴥���쳣ʱ�� */
    u32 e_excepid;                                               /* ģ�鴥�����쳣id */
    u16 e_dump_status;                                        /* ģ�齫�쳣��Ϣ��Ԥ���ڴ�Ŀ���״̬ */
    u16 e_save_status;                                         /* �����쳣��Ϣ��Ԥ���ڴ浼���Ŀ���״̬ */ 
};

struct exc_module_info_s{
    u32 magic;                                                      /* ʹ�ú�MODULE_MAGIC */
    u16 e_mod_valid;                                            /* ģ��д��ע����쳣��������1 */
    u16 e_mod_num;                                            /* ģ��ע���쳣���� */
    u8 e_from_module[RDR_MODULE_NAME_LEN];   /* ģ���� */
    struct exc_info_s cur_info;                               /* ģ��dump��Ϣ����״̬ */
    u32 e_mini_offset;                                          /* ģ����С���쳣��Ϣƫ��ֵ������ģ��Ԥ���ڴ��׵�ַ����magic��ʼ */
    u32 e_mini_len;                                              /* ģ����С���쳣��Ϣ���� */
    u32 e_info_offset;                                          /* ģ��ȫ���쳣��Ϣƫ��ֵ������ģ��Ԥ���ڴ��׵�ַ����magic��ʼ */
    u32 e_info_len;                                              /* ģ��ȫ���쳣��Ϣ���� */
    struct exc_description_s e_description[1];         /* ģ���쳣ע����Ϣ */
};

enum MODULE_DUMP_STATUS {
    STATUS_INIT = 0,
    STATUS_DOING = 1,
    STATUS_DONE = 2,
};

#endif //__RDR_EXCEPTION_PUB_H__