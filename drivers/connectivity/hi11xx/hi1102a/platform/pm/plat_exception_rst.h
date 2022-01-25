

#ifndef __PLAT_EXCEPTION_RST_H__
#define __PLAT_EXCEPTION_RST_H__
/*****************************************************************************
  1 Include other Head file
*****************************************************************************/
#include "plat_type.h"
#include "oal_net.h"
/*****************************************************************************
  2 Define macro
*****************************************************************************/
#define WAIT_BFGX_READ_STACK_TIME  (10000)   /* �ȴ�bfgx��ջ������ɵ�ʱ�䣬5000����*/
#define BFGX_BEAT_TIME             (3)      /* bfgx������ʱʱ��Ϊ3����*/
#define WIFI_DUMP_BCPU_TIMEOUT     (1000)   /*wifi dump bcpu���ݵȴ�ʱ��*/

#define PLAT_EXCEPTION_RESET_IDLE  (0)      /* plat û���ڴ����쳣*/
#define PLAT_EXCEPTION_RESET_BUSY  (1)      /* plat ���ڴ����쳣*/

#define BFGX_NOT_RECV_BEAT_INFO    (0)      /* hostû���յ�������Ϣ*/
#define BFGX_RECV_BEAT_INFO        (1)      /* host�յ�������Ϣ*/

#define RF_TEMERATURE_NORMAL       (0)      /* rf�¶�����*/
#define RF_TEMERATURE_OVERHEAT     (1)      /* rf�¶ȹ���*/

#define EXCEPTION_SUCCESS          (0)
#define EXCEPTION_FAIL             (1)

#define BEAT_TIMER_DELETE          (0)
#define BEAT_TIMER_RESET           (1)

#define DFR_TEST_ENABLE          (0)
#define DFR_TEST_DISABLE         (1)

#if ((_PRE_PRODUCT_ID != _PRE_PRODUCT_ID_HI1103_HOST)&&(_PRE_PRODUCT_ID != _PRE_PRODUCT_ID_HI1102A_HOST))
#define BFGX_MEM_DUMP_BLOCK_COUNT  (3)       /* BFGX �ϱ����ڴ����� */
#else
#define BFGX_MEM_DUMP_BLOCK_COUNT  (2)       /* BFGX �ϱ����ڴ����� */
#endif
#define UART_READ_WIFI_MEM_TIMEOUT (10000)
#define UART_HALT_WCPU_TIMEOUT     (500)

#define BFGX_MAX_RESET_CMD_LEN     (9)
#define BT_RESET_CMD_LEN           (4)
#define FM_RESET_CMD_LEN           (1)
#define GNSS_RESET_CMD_LEN         (9)
#define IR_RESET_CMD_LEN           (4)
#define NFC_RESET_CMD_LEN          (4)

#define SDIO_STORE_BFGX_REGMEM     "readm_bfgx_sdio"
#define UART_STORE_BFGX_STACK      "readm_bfgx_uart"
#define UART_STORE_WIFI_MEM        "readm_wifi_uart"
#define SDIO_STORE_WIFI_MEM        "readm_wifi_sdio"

/*nfc buffer����*/
#define OML_STATUS_ADD_LENGTH       13
#define NFCLOGLEN                  (4 * sizeof(uint32) + OML_STATUS_ADD_LENGTH)
#define NFCLOGNUM                  (100)
#define OMLNFCDATABUFFLEN          (NFCLOGNUM * NFCLOGLEN)
#define NFC_SEND_LEN_LIMIT         (NFCLOGNUM*NFCLOGLEN)

#define MEMDUMP_ROTATE_QUEUE_MAX_LEN (10)
#define ARP_TIMEOUT_MAX_TIMES        (10)
/*****************************************************************************
  3 STRUCT DEFINE
*****************************************************************************/
enum DFR_RST_SYSTEM_TYPE_E
{
    DFR_SINGLE_SYS_RST    = 0,
    DFR_ALL_SYS_RST       = 1,
    DFR_SYSTEM_RST_TYPE_BOTT,
};

enum SUBSYSTEM_ENUM
{
	SUBSYS_WIFI  = 0,
	SUBSYS_BFGX  = 1,

	SUBSYS_BOTTOM,
};

enum WIFI_THREAD_ENUM
{
	THREAD_WIFI   = 0,

	WIFI_THREAD_BOTTOM,
};

enum BFGX_THREAD_ENUM
{
	THREAD_BT   = 0,
	THREAD_FM   = 1,
	THREAD_GNSS = 2,
	THREAD_IR   = 3,
	THREAD_NFC  = 4,

	BFGX_THREAD_BOTTOM,
};

enum EXCEPTION_TYPE_ENUM
{
    /*bfgx*/
    BFGX_BEATHEART_TIMEOUT  =0,/*������ʱ*/
    BFGX_LASTWORD_PANIC     =1, /*CPU��������������*/
    BFGX_TIMER_TIMEOUT      =2,/*��ʱ����ʱ*/
    BFGX_ARP_TIMEOUT        =3,/*arp verifier��ʱ���з�����*/
    BFGX_POWERON_FAIL       =4,
    BFGX_WAKEUP_FAIL        =5,

    /*wifi*/
    WIFI_WATCHDOG_TIMEOUT   =6,/*���Ź���ʱ*/
    WIFI_POWERON_FAIL       =7,
    WIFI_WAKEUP_FAIL        =8,
    WIFI_DEVICE_PANIC       =9,/*wcpu arm exception*/
    WIFI_TRANS_FAIL         =10,/*sdio read or write failed*/
    SDIO_DUMPBCPU_FAIL,

    EXCEPTION_TYPE_BOTTOM,
};

enum UART_WIFI_MEM_DUMP
{
	WIFI_PUB_REG   = 0,
	WIFI_PRIV_REG  = 1,
	WIFI_MEM       = 2,

	UART_WIFI_MEM_DUMP_BOTTOM,
};

enum DUMP_CMD_TYPE
{
    CMD_READM_WIFI_SDIO = 0,
    CMD_READM_WIFI_UART = 1,
    CMD_READM_BFGX_UART = 2,
    CMD_READM_BFGX_SDIO = 3,

    CMD_DUMP_BUFF,
};
enum excp_test_cfg_em
{
    BFGX_POWEON_FAULT = 0,
    BFGX_POWEOFF_FAULT = 1,
    WIFI_WKUP_FAULT   = 2,
    WIFI_POWER_ON_FAULT = 3,
    EXCP_TEST_CFG_BOTT,
};

struct st_uart_dump_wifi_mem_info
{
    uint8 *file_name;
    uint32 size;
};

struct st_uart_dump_wifi_info
{
    uint32 cmd;
    uint32 total_size;
    uint32 block_count;
    struct st_uart_dump_wifi_mem_info *block_info;
};

struct st_exception_mem_info
{
    uint8 *exception_mem_addr;
    uint32 total_size;
    uint32 recved_size;
    uint8  *file_name;
};

struct st_wifi_dump_mem_info
{
    unsigned long  mem_addr;
    uint32 size;
    uint8  *file_name;
};

struct st_wifi_dfr_callback
{
    void  (*wifi_recovery_complete)(void);
    void  (*notify_wifi_to_recovery)(void);
};

struct st_bfgx_reset_cmd
{
    uint32 len;
    uint8  cmd[BFGX_MAX_RESET_CMD_LEN];
};
struct excp_type_info_s{
    uint32 excp_cnt;
    uint32 fail_cnt;
    uint32 rst_type_cnt[DFR_SYSTEM_RST_TYPE_BOTT];
    ktime_t stime;
    unsigned long long maxtime;
};

struct st_exception_info
{
    uint32   exception_reset_enable;
    uint32   subsys_type;
    uint32   thread_type;
    uint32   excetion_type;

    atomic_t bfgx_beat_flag;
    atomic_t is_reseting_device;
    atomic_t is_memdump_runing;

    struct workqueue_struct *plat_exception_rst_workqueue;
    struct work_struct plat_exception_rst_work;
    oal_wakelock_stru  plat_exception_rst_wlock;
    struct work_struct uart_store_wifi_mem_to_file_work;
    struct timer_list bfgx_beat_timer;

    struct completion wait_read_bfgx_stack;
    struct completion wait_sdio_d2h_dump_ack;

    struct ps_plat_s *ps_plat_d;
    struct st_wifi_dfr_callback *wifi_callback;

    spinlock_t exception_spin_lock;

    struct excp_type_info_s etype_info[EXCEPTION_TYPE_BOTTOM];

    /*�±ߵı�������ʹ��*/
    uint32 debug_beat_flag;
    uint32 subsystem_rst_en;

    /*wifi��BCPU for sdio mem dump���ù��ܽ��ڵ���ʱʹ�ã�Ĭ�Ϲر�*/
    uint32 wifi_open_bcpu_enable;

    struct completion wait_uart_read_wifi_mem;
    struct completion wait_uart_halt_wcpu;

    /* wifi�쳣���� */
    struct work_struct          wifi_excp_worker;
    struct work_struct          wifi_excp_recovery_worker;
    struct workqueue_struct    *wifi_exception_workqueue;
    uint32                      wifi_excp_type;

};
struct sdio_dump_bcpu_buff
{
    uint8 *mem_addr;
    uint32 data_limit;
    uint32 data_len;
};
typedef struct excp_info_str_s {
    uint32 id;
    char*  name;
}excp_info_str_t;
/*****************************************************************************
  4 EXTERN VARIABLE
*****************************************************************************/

/*****************************************************************************
  5 ȫ�ֱ�������
*****************************************************************************/
extern struct sdio_dump_bcpu_buff st_bcpu_dump_buff;
extern oal_netbuf_stru*       st_bcpu_dump_netbuf;

/*****************************************************************************
  6 EXTERN FUNCTION
*****************************************************************************/
extern int32 mod_beat_timer(uint8 on);
extern int32 is_bfgx_exception(void);
extern int32 get_exception_info_reference(struct st_exception_info **exception_data);
extern int32 plat_exception_handler(uint32 subsys_type, uint32 thread_type, uint32 exception_type);
extern int32 plat_bfgx_exception_rst_register(struct ps_plat_s *data);
extern int32 plat_wifi_exception_rst_register(void *data);
extern int32 wifi_exception_mem_dump(struct st_wifi_dump_mem_info *pst_mem_dump_info, uint32 count, int32 excep_type);
extern int32 wifi_open_bcpu_set(uint8 enable);
extern int32 prepare_to_recv_bfgx_stack(uint32 len);
extern int32 bfgx_recv_dev_mem(uint8 *buf_ptr, uint16 count);
extern void store_wifi_mem_to_file(void);
extern int32 uart_recv_wifi_mem(uint8 *buf_ptr, uint16 count);
extern int32 uart_halt_wcpu(void);
extern int32 uart_read_wifi_mem(uint32 which_mem);
extern int32 debug_uart_read_wifi_mem(uint32 ul_lock);
extern int32 plat_exception_reset_init(void);
extern int32 plat_exception_reset_exit(void);
extern int32 wifi_exception_work_submit(uint32 wifi_excp_type);
extern int32 plat_power_fail_exception_info_set(uint32 subsys_type, uint32 thread_type, uint32 exception_type);
extern void plat_power_fail_process_done(void);
extern int32 bfgx_subsystem_reset(void);
extern int32 bfgx_system_reset(void);
extern int32 debug_sdio_read_bfgx_reg_and_mem(uint32 which_mem);
extern int32 exception_bcpu_dump_recv(uint8* str,oal_netbuf_stru* netbuf);
#endif

