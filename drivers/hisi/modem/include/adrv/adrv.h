

#ifndef HISI_AP_DRV_H
#define HISI_AP_DRV_H
#include <linux/module.h>
/*************************************************************************
*
*   ����/����/��λ/У��
*
*************************************************************************/


#define PARTITION_MODEM_IMAGE_NAME       "modem"
#define PARTITION_MODEM_DSP_NAME         "modem_dsp"
#define PARTITION_MODEM_NVDLOAD_NAME     "modemnvm_update"
#define PARTITION_MODEM_NVDLOAD_CUST_NAME "modemnvm_cust"
#define PARTITION_MODEM_NVDEFAULT_NAME   "modemnvm_factory"
#define PARTITION_MODEM_NVBACKUP_NAME    "modemnvm_backup"
#define PARTITION_MODEM_NVSYS_NAME       "modemnvm_system"
#define PARTITION_MODEM_NVIMG_NAME       "modemnvm_img"
#define PARTITION_MODEM_LOG_NAME         "modem_om"
#define PARTITION_MODEM_DTB_NAME	 "modem_dtb"
#define PARTITION_PTN_VRL_P_NAME 	 "vrl"
#define PARTITION_PTN_VRL_M_NAME	 "vrl_backup"

#define BSP_RESET_NOTIFY_REPLY_OK 		   0
#define BSP_RESET_NOTIFY_SEND_FAILED      -1
#define BSP_RESET_NOTIFY_TIMEOUT          -2

typedef enum
{
	BSP_START_MODE_CHARGING = 0,
	BSP_START_MODE_NORMAL,
	BSP_START_MODE_UPDATE,
	BSP_START_MODE_BUILT
} BSP_START_MODE_E;

typedef enum {
	BSP_CCORE = 0,
	BSP_HIFI,
	BSP_BBE,
	BSP_CDSP,
	BSP_CCORE_TAS,
	BSP_CCORE_WAS,
	BSP_CCORE_CAS,
	BSP_BUTT
} BSP_CORE_TYPE_E;

/*****************************************************************************
* �� �� ��  : flash_find_ptn
*
* ��������  : ���ݷ��������ط�����Ϣ
*
* �������  : str ������
*
* �������  : ��
*
* �� �� ֵ  : 0 �ɹ� -1 ʧ��
*
* ����˵��  : kernel
*
*****************************************************************************/
int flash_find_ptn(const char* str, char* pblkname);

/*****************************************************************************
* �� �� ��  : bsp_need_loadmodem
*
* ��������  : ��ȡ�Ƿ���Ҫ����modem
*
* �������  : ��
*
* �������  : ��
*
* �� �� ֵ  : 1 ��Ҫ����
			  0 ����Ҫ����
*
* ����˵��  : kernel
*
*****************************************************************************/
int bsp_need_loadmodem(void);

/*****************************************************************************
* �� �� ��  : bsp_get_bootmode
*
* ��������  : ��ȡ����ʱmodem��״̬����������/�ػ����/����
*
* �������  : ��
*
* �������  : ��
*
* �� �� ֵ  : ��BSP_START_MODE_E
*
* ����˵��  : kernel
*
*****************************************************************************/
int bsp_get_bootmode(void);

/*****************************************************************************
* �� �� ��  : bsp_get_bootmode
*
* ��������  : Modem��HIFI ��λ����ӿ�
*
* �������  : ecoretype ��BSP_CORE_TYPE_E
*
* �������  : ��
*
* �� �� ֵ  : 0  ����ɹ�
			  <0 ����ʧ��
*
* ����˵��  : kernel
*
*****************************************************************************/
int bsp_reset_request (BSP_CORE_TYPE_E ecoretype);


/*****************************************************************************
* �� �� ��  : bsp_reset_loadimage
*
* ��������  : Modem��HIFI �������
*
* �������  : ecoretype ��BSP_CORE_TYPE_E
*
* �������  : ��
*
* �� �� ֵ  : 0  ����ɹ�
			  <0 ����ʧ��
*
* ����˵��  : kernel
*
*****************************************************************************/
int bsp_reset_loadimage (BSP_CORE_TYPE_E ecoretype);

/*****************************************************************************
* �� �� ��  : bsp_image_seccheck
*
* ��������  : Modem��HIFI ����ȫУ�� У��ͨ����ֱ�Ӵ�����λ
*
* �������  : ecoretype ��BSP_CORE_TYPE_E
*
* �������  : ��
*
* �� �� ֵ  : 0  ����ɹ�
			  <0 ����ʧ��
*
* ����˵��  : kernel
*
*****************************************************************************/
int bsp_image_seccheck (BSP_CORE_TYPE_E ecoretype);

/*
 * Function name:bsp_reset_core_notify.
 * Description:notify the remote processor MODEM is going to reset.
 * Parameters:
 *      @ ecoretype: the core to be notified.
 *      @ cmdtype: to send to remote processor.
 *      @ timeout_ms: max time to wait, ms.
 *      @ retval: the ack's value get from the remote processor.
 *Called in:
 *      @ modem is going to reset. <reset_balong.c>
 *Return value:
 *      @ BSP_RESET_NOTIFY_REPLY_OK-->the remote processor give response in time.
 *      @ BSP_RESET_NOTIFY_SEND_FAILED-->the parameter is wrong or other rproc_sync_send's self error.
 *      @ BSP_RESET_NOTIFY_TIMEOUT-->after wait timeout_ms's time, the remote processor give no response.
*/
int bsp_reset_core_notify(BSP_CORE_TYPE_E ecoretype, unsigned int cmdtype, unsigned int timeout_ms, unsigned int *retval);



/*************************************************************************
*
*   RDR/Coresight
*
*************************************************************************/

#define STR_EXCEPTIONDESC_MAXLEN	48

/*��AP Լ����������¸�ʽ������������AP���*/
#ifdef CONFIG_HISI_MNTN_ESUBTYPE
typedef enum
{
    CP_S_MODEMDMSS     = 0x70,
    CP_S_MODEMNOC      = 0x71,
    CP_S_MODEMAP       = 0x72,
    CP_S_EXCEPTION     = 0x73,
    CP_S_RESETFAIL     = 0x74,
    CP_S_NORMALRESET   = 0x75,
    CP_S_RILD_EXCEPTION= 0x76,
    CP_S_3RD_EXCEPTION = 0x77,
    CP_S_DRV_EXC       = 0x78,
    CP_S_PAM_EXC       = 0x79,
    CP_S_GUAS_EXC      = 0x7a,
    CP_S_CTTF_EXC      = 0x7b,
    CP_S_CAS_CPROC_EXC = 0x7c,
    CP_S_GUDSP_EXC     = 0x7d,
    CP_S_TLPS_EXC      = 0x7e,
    CP_S_TLDSP_EXC     = 0x7f,
    CP_S_CPHY_EXC      = 0x80,
    CP_S_GUCNAS_EXC    = 0x81,
} EXCH_SOURCE;
#else
typedef enum
{
    CP_S_MODEMDMSS = 0x50,
    CP_S_MODEMNOC = 0x51,
    CP_S_MODEMAP = 0x52,
    CP_S_EXCEPTION = 0x53,
    CP_S_RESETFAIL = 0x54,
    CP_S_NORMALRESET = 0x55,
    CP_S_RILD_EXCEPTION = 0x56,
    CP_S_3RD_EXCEPTION = 0x57,
} EXCH_SOURCE;
#endif


struct list_head_rdr {
	struct list_head_rdr *next, *prev;
};
typedef void (*rdr_e_callback)(unsigned int, void*);

/*
 *   struct list_head_rdr   e_list;
 *   u32 modid,			    exception id;
 *		if modid equal 0, will auto generation modid, and return it.
 *   u32 modid_end,		    can register modid region. [modid~modid_end];
		need modid_end >= modid,
 *		if modid_end equal 0, will be register modid only,
		but modid & modid_end cant equal 0 at the same time.
 *   u32 process_priority,	exception process priority
 *   u32 reboot_priority,	exception reboot priority
 *   u64 save_log_mask,		need save log mask
 *   u64 notify_core_mask,	need notify other core mask
 *   u64 reset_core_mask,	need reset other core mask
 *   u64 from_core,		    the core of happen exception
 *   u32 reentrant,		    whether to allow exception reentrant
 *   u32 exce_type,		    the type of exception
 *   char* from_module,		    the module of happen excption
 *   char* desc,		        the desc of happen excption
 *   rdr_e_callback callback,	will be called when excption has processed.
 *   u32 reserve_u32;		reserve u32
 *   void* reserve_p		    reserve void *
 */
struct rdr_exception_info_s {
	struct list_head_rdr e_list;	/*list_head_rdr instead of list_head  to solve the confliction between <linux/types.h>*/
	unsigned int	e_modid;
	unsigned int	e_modid_end;
	unsigned int	e_process_priority;
	unsigned int	e_reboot_priority;
	unsigned long long	e_notify_core_mask;
	unsigned long long	e_reset_core_mask;
	unsigned long long	e_from_core;
	unsigned int	e_reentrant;
	unsigned int    e_exce_type;
#ifdef CONFIG_HISI_MNTN_ESUBTYPE
	unsigned int    e_exce_subtype;
#endif
	unsigned int    e_upload_flag;
	unsigned char	e_from_module[MODULE_NAME_LEN];
	unsigned char	e_desc[STR_EXCEPTIONDESC_MAXLEN];
	unsigned int	e_reserve_u32;
	void*	e_reserve_p;
	rdr_e_callback e_callback;
};

enum UPLOAD_FLAG {
    RDR_UPLOAD_YES = 0xff00fa00,
    RDR_UPLOAD_NO
};
enum REENTRANT {
    RDR_REENTRANT_ALLOW = 0xff00da00,
    RDR_REENTRANT_DISALLOW
};

enum REBOOT_PRI {
    RDR_REBOOT_NOW      = 0x01,
    RDR_REBOOT_WAIT,
    RDR_REBOOT_NO,
    RDR_REBOOT_MAX
};
enum CORE_LIST {
    RDR_AP      = 0x1,
    RDR_CP      = 0x2,
    RDR_HIFI    = 0x8,
    RDR_LPM3    = 0x10,
    RDR_MODEMAP = 0x200,
};
enum PROCESS_PRI {
    RDR_ERR      = 0x01,
    RDR_WARN,
    RDR_OTHER,
    RDR_PPRI_MAX
};


typedef void (*pfn_cb_dump_done)(unsigned int modid, unsigned long long coreid);
typedef void (*pfn_dump)(unsigned int modid, unsigned int etype, unsigned long long coreid, char* logpath, pfn_cb_dump_done fndone);
typedef void (*pfn_reset)(unsigned int modid, unsigned int etype, unsigned long long coreid);

struct rdr_module_ops_pub {
    pfn_dump    ops_dump;
    pfn_reset   ops_reset;
};

struct rdr_register_module_result {
    unsigned long long   log_addr;
    unsigned int         log_len;
    unsigned long long   nve;
};


/*****************************************************************************
* �� �� ��  : rdr_register_exception
*
* ��������  : RDR�쳣ע��ӿ�
*
* �������  : rdr_exception_info_s
*
* �������  : ��
*
* �� �� ֵ  : 0  ����ʧ��
			  >0 e_modid_end
*
* ����˵��  : kernel
*
*****************************************************************************/
unsigned int rdr_register_exception (struct rdr_exception_info_s* e);

/*****************************************************************************
* �� �� ��  : rdr_register_module_ops
*
* ��������  : ע���쳣������
*
* �������  : coreid  �쳣��ID
			  ops     �쳣��������reset����
*             retinfo ע�᷵����Ϣ
* �������  : ��
*
* �� �� ֵ  : 0  ����ɹ�
			  <0 ����ʧ��
*
* ����˵��  : kernel
*
*****************************************************************************/
int rdr_register_module_ops(unsigned long long coreid, struct rdr_module_ops_pub* ops, struct rdr_register_module_result* retinfo);


/*****************************************************************************
* �� �� ��  : rdr_system_error
*
* ��������  : ����쳣��¼�ӿڣ�����쳣�ļ���¼֮��������λ����
*
* �������  :
*
* �������  : ��
*
* �� �� ֵ  : ��
*
* ����˵��  : kernel
*
*****************************************************************************/
void rdr_system_error(unsigned int modid, unsigned int arg1, unsigned int arg2);

/*****************************************************************************
* �� �� ��  : rdr_syserr_process_for_ap
*
* ��������  : ����ע�ᵽrdr�ϵ��쳣����ʱ����Ҫ���ô˽ӿڴ�������log��reset���̡��˽ӿڹ�APʹ�á�
*
* �������  : ��
*
* �������  : ��
*
* �� �� ֵ  : ��
*
* ����˵��  : kernel
*
*****************************************************************************/
void rdr_syserr_process_for_ap(unsigned int modid, unsigned long long arg1, unsigned long long arg2);

/*
void top_tmc_enable(void);

void top_tmc_disable(void);

unsigned int rdr_register_soc_ops (RDR_U64 coreid, struct rdr_soc_ops_pub* ops,
        struct rdr_register_soc_result* retinfo);
unsigned long long rdr_unregister_soc_ops_info (RDR_U64 coreid);
*/


/*************************************************************************
*
*   eFuse
*
*************************************************************************/

/*****************************************************************************
* �� �� ��  : get_efuse_dieid_value
*
* ��������  : ����die id ��ֵ
*
* �������  : u32Length ���ֽ�Ϊ��λ������Ҫ����С��4���ֽ�
			  timeout   ��ʱʱ��
*
* �������  : pu8Buffer �洢��ȡefuse��ֵ
*
* �� �� ֵ  : 0  ����ɹ�
			  <0 ����ʧ��
*
* ����˵��  : kernel
*
*****************************************************************************/
int get_efuse_dieid_value(unsigned char *pu8Buffer, unsigned int u32Length, unsigned int timeout);

/*****************************************************************************
* �� �� ��  : get_efuse_chipid_value
*
* ��������  : ����chip id ��ֵ
*
* �������  : u32Length ���ֽ�Ϊ��λ������Ҫ����С��4���ֽ�
			  timeout   ��ʱʱ��
*
* �������  : pu8Buffer �洢��ȡefuse��ֵ
*
* �� �� ֵ  : 0  ����ɹ�
			  <0 ����ʧ��
*
* ����˵��  : kernel
*
*****************************************************************************/
int get_efuse_chipid_value(unsigned char *pu8Buffer, unsigned int u32Length, unsigned int timeout);

/*****************************************************************************
* �� �� ��  : get_efuse_kce_value
*
* ��������  : ����kce��ֵ
*
* �������  : u32Length ���ֽ�Ϊ��λ������Ҫ����С��4���ֽ�
			  timeout   ��ʱʱ��
*
* �������  : pu8Buffer �洢��ȡefuse��ֵ
*
* �� �� ֵ  : 0  ����ɹ�
			  <0 ����ʧ��
*
* ����˵��  : kernel
*
*****************************************************************************/
int get_efuse_kce_value(unsigned char *pu8Buffer, unsigned int u32Length, unsigned int timeout);

/*****************************************************************************
* �� �� ��  : set_efuse_kce_value
*
* ��������  : ����kce��ֵ
*
* �������  : pu8Buffer �洢��ȡefuse��ֵ
			  u32Length ���ֽ�Ϊ��λ������Ҫ����С��4���ֽ�
			  timeout   ��ʱʱ��
*
* �������  : ��
*
* �� �� ֵ  : 0  ����ɹ�
			  <0 ����ʧ��
*
* ����˵��  : kernel
*
*****************************************************************************/
int set_efuse_kce_value(unsigned char *pu8Buffer, unsigned int u32Length, unsigned int timeout);



/*************************************************************************
*
*   USB
*
*************************************************************************/
typedef void (*USB_ENABLE_CB_T)(void);
typedef void (*USB_DISABLE_CB_T)(void);

/*****************************************************************************
* �� �� ��  : bsp_acm_open
*
* ��������  : ���豸
*
* �������  : dev_id Ҫ�򿪵��豸id
*
* �������  : ��
*
* �� �� ֵ  : ��0 ���ص�handle
			  NULL ����
*
* ����˵��  : kernel
*
*****************************************************************************/
void *bsp_acm_open(unsigned int dev_id);

/*****************************************************************************
* �� �� ��  : bsp_acm_close
*
* ��������  : �ر��豸
*
* �������  : Ҫ�رյ��豸handle
*
* �������  : ��
*
* �� �� ֵ  : 0  ����ɹ�
			  <0 ����ʧ��
*
* ����˵��  : kernel
*
*****************************************************************************/
int bsp_acm_close(void *handle);

/*****************************************************************************
* �� �� ��  : bsp_acm_write
*
* ��������  : ����д
*
* �������  : handle �豸handle
			  buf  ��д�������
			  size ��д������ݴ�С
*
* �������  : ��
*
* �� �� ֵ  : >0  ����ɹ������ݴ�С
			  <=0 ����ʧ��
*
* ����˵��  : kernel
*
*****************************************************************************/
int bsp_acm_write(void *handle, void *buf, unsigned int size);

/*****************************************************************************
* �� �� ��  : bsp_acm_read
*
* ��������  : ����д
*
* �������  : handle �豸handle
			  size ��д������ݴ�С
*
* �������  : buf  �����ص�����
*
* �� �� ֵ  : >0  ����ɹ������ݴ�С
			  <=0 ����ʧ��
*
* ����˵��  : kernel
*
*****************************************************************************/
int bsp_acm_read(void *handle, void *buf, unsigned int size);

/*****************************************************************************
* �� �� ��  : bsp_acm_ioctl
*
* ��������  : �����������������
*
* �������  : handle �豸handle
			  cmd ������
			  para �������
*
* �������  : ��
*
* �� �� ֵ  : 0  ����ɹ�
			  <0 ����ʧ��
*
* ����˵��  : kernel
*
*****************************************************************************/
int bsp_acm_ioctl(void *handle, unsigned int cmd, void *para);

/*****************************************************************************
* �� �� ��  : BSP_USB_RegUdiEnableCB
*
* ��������  : ע��usbö����ɻص�
*
* �������  : pFunc �ص�ָ��
*
* �������  : ��
*
* �� �� ֵ  : 0  ����ɹ�
			  <0 ����ʧ��
*
* ����˵��  : kernel
*
*****************************************************************************/
unsigned int BSP_USB_RegUdiEnableCB(USB_ENABLE_CB_T pFunc);

/*****************************************************************************
* �� �� ��  : BSP_USB_RegUdiDisableCB
*
* ��������  : ע��usb�γ���ɻص�
*
* �������  : pFunc �ص�ָ��
*
* �������  : ��
*
* �� �� ֵ  : 0  ����ɹ�
			  <0 ����ʧ��
*
* ����˵��  : kernel
*
*****************************************************************************/
unsigned int BSP_USB_RegUdiDisableCB(USB_DISABLE_CB_T pFunc);

/*************************************************************************
*
*   PMU
*
*************************************************************************/


typedef  void   (*PMU_OCP_CALLBACK)( const char * name);
/*****************************************************************************
* �� �� ��  : BSP_USB_RegUdiDisableCB
*
* ��������  : ע��usb�γ���ɻص�
*
* �������  : name OCP�����ĵ�Դ���ƣ���pmu_mntn DTS����
			  pFunc
*
* �������  : ��
*
* �� �� ֵ  : 0  ����ɹ�
			  <0 ����ʧ��
*
* ����˵��  : kernel
*
*****************************************************************************/
int pmu_ocp_event_reg (const char * name, PMU_OCP_CALLBACK pFunc);

enum EDITION_KIND{
    EDITION_USER            = 1,
    EDITION_INTERNAL_BETA   = 2,
    EDITION_OVERSEA_BETA    = 3,
    EDITION_MAX
};
/*
 * func name: bbox_check_edition
 * func args:
 *   void
 * return:
 *   unsigned int:	return edition information
 *				0x01		User
 *				0x02		Internal BETA
 *				0x03		Oversea BETA
 */
unsigned int bbox_check_edition(void);

/* hisi_pmic_special_ocp_register��Ҫ����ӿڹܿ�  */

int atfd_hisi_service_access_register_smc(unsigned long long main_fun_id, 
                                          unsigned long long buff_addr_phy, 
                                          unsigned long long data_len, 
                                          unsigned long long sub_fun_id);

#endif /* HISI_AP_DRV_H */

