#ifndef BATT_INFO_H
#define BATT_INFO_H

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
#include <linux/of_platform.h>
#include <linux/power/hisi/coul/hisi_coul_drv.h>
#include <linux/workqueue.h>
#include <linux/list.h>
#include <linux/mtd/hisi_nve_interface.h>
#include <dsm/dsm_pub.h>
#include <huawei_platform/power/power_genl.h>
#include <huawei_platform/power/power_mesg.h>
#include <huawei_platform/power/power_dsm.h>
#include <huawei_platform/power/batt_info_pub.h>
#include "batt_aut_checker.h"
#include "batt_info.h"
#include "batt_info_util.h"

enum BATT_INFO_TYPE{
    DMD_INVALID = 0,
    DMD_ROM_ID_ERROR,
    DMD_IC_STATE_ERROR,
    DMD_IC_KEY_ERROR,
    DMD_OO_UNMATCH,
    DMD_OBD_UNMATCH,
    DMD_OBT_UNMATCH,
    DMD_NV_ERROR,
    DMD_SERVICE_ERROR,
    DMD_UNMATCH_BATTS,
};

enum {
    IC_DMD_GROUP = 0,
    KEY_DMD_GROUP,
    SN_DMD_GROUP,
    BATT_INFO_DMD_GROUPS,
};

typedef struct __batt_info_data batt_info_data;
typedef int (*final_sn_checker_t )(batt_info_data* drv_data);

struct dmd_record {
    struct list_head node;
    int dmd_type;
    int dmd_no;
    int content_len;
    char *content;
};

#define DMD_CONTENT_BASE_LEN 24
#define DMD_REPORT_CONTENT_LEN 256
struct dmd_record_list {
    struct list_head dmd_head;

    struct mutex lock;
    struct delayed_work dmd_record_report;
};

#define DMD_INFO_MESG_SIZE  128
struct __batt_info_data{
    char sn_buff[MAX_SN_LEN];
    int dmd_retry;
    int dmd_no;
    unsigned int sn_version;
    unsigned int sn_len;
    unsigned int total_checkers;
    const unsigned char* sn;
    struct delayed_work dmd_report_dw;
    struct work_struct check_work;
    final_sn_checker_t sn_checker;
    battery_check_result result;
};

#endif