#ifndef BATT_AUT_CHECKER_H
#define BATT_AUT_CHECKER_H

#include <linux/stddef.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/sysfs.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/workqueue.h>
#include <linux/list.h>
#include <linux/power/hisi/hisi_battery_data.h>
#include <linux/power/hisi/coul/hisi_coul_event.h>
#include <linux/notifier.h>
#include <huawei_platform/log/hw_log.h>
#include "batt_info_util.h"

enum RES_TYPE{
    RES_CT = 0,
    RES_SN,
};
enum NV_SN_VERSION{
    ILLEGAL_BIND_VERSION = 0,
    RAW_BIND_VERSION,
    PAIR_BIND_VERSION,
    LEFT_UNUSED_VERSION,
};
#define MAX_SN_LEN                      20
#define MAX_SN_BUFF_LENGTH              5

typedef struct {
    unsigned int version;
    char info[MAX_SN_BUFF_LENGTH][MAX_SN_LEN];
} binding_info;

enum BATT_MATCH_TYPE {
    BATTERY_UNREMATCHABLE = 0,
    BATTERY_REMATCHABLE = 1,
};

typedef enum {
    BATT_CHARGE_CYCLES = 0,
    BATT_MATCH_ABILITY,
} batt_safe_info_type;
enum IC_CR{
    IC_PASS = 0,
    IC_FAIL_UNMATCH,    //IC information is not right for product
    IC_FAIL_UNKOWN,     //IC unkown(communication error)
    IC_FAIL_MEM_STATUS, //IC memory check failed
};
enum KEY_CR{
    KEY_PASS = 0,
    KEY_FAIL_TIMEOUT,   //checking key timeout
    KEY_UNREADY,        //key is under checking
    KEY_FAIL_UNMATCH,   //key is unmatch
    KEY_FAIL_IC,        //
};
enum SN_CR{
    SN_PASS = 0,
    SN_OBD_REMATCH,     //old board rematch new battery
    SN_OBT_REMATCH,     //old battery rematch new board
    SN_NN_REMATCH,      //new board & new battery rematch
    SN_FAIL_TIMEOUT,    //get SN timeout
    SN_FAIL_NV,         //Read or Write NV fail
    SN_UNREADY,         //SN is under checking
    SN_FAIL_IC,         //SN get from IC timeout
    SN_OO_UNMATCH,      //old board & old battery unmatch
    SN_SNS_UNMATCH,     //sns of battery umatch
};
enum CHECK_MODE {
    FACTORY_CHECK_MODE = 0,
    COMMERCIAL_CHECK_MODE = 15,
};
typedef struct {
    batt_ic_type (*get_ic_type)(void);
    int (*get_batt_type)(struct platform_device *, const unsigned char **type, unsigned int *type_len);
    int (*get_batt_sn)(struct platform_device *, resource *res, const unsigned char **sn, unsigned int *sn_len_bits);
    int (*prepare)(struct platform_device *,enum RES_TYPE type, resource *res);
    int (*certification)(struct platform_device *, resource *res, enum KEY_CR *result);
    int (*set_batt_safe_info)(struct platform_device *, batt_safe_info_type type, void *value);
    int (*get_batt_safe_info)(struct platform_device *, batt_safe_info_type type, void *value);
}batt_ct_ops;


typedef struct {
    struct list_head node;
    struct platform_device *ic_dev;
    int (*ct_ops_register)(batt_ct_ops *);
    void (*ic_memory_release)(void);
} ct_ops_reg_list;

/* return macro */
#define BATTERY_DRIVER_FAIL                         -1
#define BATTERY_DRIVER_SUCCESS                      0

/* IC memory protection mode */
#define READ_PROTECTION                             0x80
#define WRITE_PROTECTION                            0x40
#define EPROM_EMULATION_MODE                        0x20
#define AUTHENTICATION_PROTECTION                   0x10
#define NO_PROTECTION                               0x00

/* sys node information show macro */
#define BATT_ID_PRINT_SIZE_PER_CHAR                 3

/* NVME macro */
#define MAX_SN_LEN_BITS                             32
#define NV_VERSION_INDEX                            0

/* Battery maxium current&voltage initialization value */
#define MAX_CHARGE_CURRENT                          10000
#define MAX_CHARGE_VOLTAGE                          10000

#define HEXBITS                                     4
#define BYTEBITS                                    8
#define BYTE_HIGH_N_BITS(n)                         ((0x00ff)>>(8-n)<<(8-n))

#define MAC_RESOURCE_TPYE0                          0
#define MAC_RESOURCE_TPYE1                          1

#define ERR_NO_STRING_SIZE                          128
#define DSM_BATTERY_DETECT_BUFFSIZE                 1024

#define DEV_GET_DRVDATA(data, dev, retval) \
do { \
    data = dev_get_drvdata(dev); \
    if(!data) { \
        hwlog_err("get drvdata failed in %s.\n", __func__); \
        return retval; \
    } \
} while(0)

typedef struct {
    enum IC_CR ic_status;
    enum KEY_CR key_status;
    enum SN_CR sn_status;
    enum CHECK_MODE check_mode;
} battery_check_result;

typedef struct __batt_checker_data batt_checker_data;
typedef int (*legal_sn_check_t)(batt_checker_data* drv_data);

struct batt_ct_wrapper_ops {
	batt_ic_type (*get_ic_type)(batt_checker_data *);
	int (*get_batt_type)(batt_checker_data *,
		const unsigned char **, unsigned int *);
	int (*get_batt_sn)(batt_checker_data *, resource *,
		const unsigned char **, unsigned int *);
	int (*prepare)(batt_checker_data *, enum RES_TYPE,
		resource *);
	int (*certification)(batt_checker_data *, resource *,
		enum KEY_CR *);
	int (*set_batt_safe_info)(batt_checker_data *,
		batt_safe_info_type, void *);
	int (*get_batt_safe_info)(batt_checker_data *,
		batt_safe_info_type, void *);
};

struct __batt_checker_data{
    unsigned char id_in_grp;
    unsigned char chks_in_grp;
    unsigned char id_of_grp;
    unsigned int sn_len;
    unsigned int free_cycles;
    enum BATT_MATCH_TYPE batt_rematch_onboot;
    enum BATT_MATCH_TYPE batt_rematch_current;
    const unsigned char* sn;
    struct platform_device* ic;
    struct work_struct check_key_work;
    struct work_struct check_sn_work;
    struct completion key_prepare_ready;
    struct completion sn_prepare_ready;
    struct notifier_block batt_cycles_listener;
    resource key_res;
    resource sn_res;
    batt_ic_type ic_type;
    batt_ct_ops ic_ops;
    struct batt_ct_wrapper_ops bco;
    battery_check_result result;
    batt_checkers_list_entry entry;
    legal_sn_check_t sn_checker;
    binding_info bind_sns;
};

void init_battery_check_result(battery_check_result* result);
void add_to_aut_ic_list(ct_ops_reg_list* entry);
const struct of_device_id* get_battery_checkers_match_table(void);
#endif
