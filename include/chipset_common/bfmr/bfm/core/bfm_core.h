/**
    @copyright: Huawei Technologies Co., Ltd. 2016-xxxx. All rights reserved.

    @file: bfm_core.h

    @brief: define the core's external public enum/macros/interface for BFM (Boot Fail Monitor)

    @version: 2.0

    @author: QiDechun ID: 216641

    @date: 2016-08-17

    @history:
*/

#ifndef BFM_CORE_H
#define BFM_CORE_H


/*----includes-----------------------------------------------------------------------*/

#include <linux/ioctl.h>
#include <linux/types.h>
#include <chipset_common/bfmr/public/bfmr_public.h>
#include <chipset_common/bfmr/common/bfmr_common.h>
#include <chipset_common/bfmr/bfr/core/bfr_core.h>
#include <chipset_common/bfmr/bfm/core/bfm_timer.h>


/*----c++ support-------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif


/*----export prototypes---------------------------------------------------------------*/

struct bfmr_dev_path
{
    char dev_name[BFMR_SIZE_64];
    char dev_path[BFMR_SIZE_1K];
};

struct bfmr_boot_fail_info
{
    int boot_fail_no;
    int suggested_recovery_method;
    bfmr_bootfail_addl_info_t addl_info;
};

struct bfmr_dbrd_ioctl_block {
	unsigned int kbytes;
	int number; // compatible with brd's brd_number
};

#define BFMR_SIZE_DISK_NAME 32
#define BFMR_SIZE_BOOTDEVICE_PROD_INFO 64

/* ioctl data structure */
struct bfmr_storage_rochk_iocb {
	unsigned int data;
};

struct bfmr_bootdisk_wp_status_iocb {
	char disk_name[BFMR_SIZE_DISK_NAME];
	unsigned int use_name;
	unsigned int write_prot;
};

struct bfmr_storage_rofa_info_iocb {
	unsigned int mode;
	unsigned int round;
	unsigned int method;
};

struct bfmr_disk_info {
	char name[BFMR_SIZE_DISK_NAME];
	int major;
	int minor;
	unsigned long long capacity;
	unsigned int write_prot;
};

struct bfmr_bootdevice_disk_info_iocb {
	unsigned int in_cnt;
	unsigned int out_cnt;
	struct bfmr_disk_info info_arr[0];
} __packed;

struct bfmr_bootdevice_prod_info_iocb {
	char prod_info[BFMR_SIZE_BOOTDEVICE_PROD_INFO];
};

typedef struct
{
    bfr_recovery_method_e recovery_method;
    bfmr_detail_boot_stage_e boot_stage;
    bfmr_selfheal_code_e selfheal_code;
} bfmr_recovery_method_maptable_t;


/*----export macroes-----------------------------------------------------------------*/

#define BFMR_IOCTL_BASE 'B'
#define BFMR_GET_TIMER_STATUS _IOR(BFMR_IOCTL_BASE, 1, int)
#define BFMR_ENABLE_TIMER _IOW(BFMR_IOCTL_BASE, 2, int)
#define BFMR_DISABLE_TIMER _IOW(BFMR_IOCTL_BASE, 3, int)
#define BFMR_GET_TIMER_TIMEOUT_VALUE _IOR(BFMR_IOCTL_BASE, 4, int)
#define BFMR_SET_TIMER_TIMEOUT_VALUE _IOW(BFMR_IOCTL_BASE, 5, int)
#define BFMR_GET_BOOT_STAGE  _IOR(BFMR_IOCTL_BASE, 6, int)
#define BFMR_SET_BOOT_STAGE  _IOW(BFMR_IOCTL_BASE, 7, int)
#define BFMR_PROCESS_BOOT_FAIL _IOW(BFMR_IOCTL_BASE, 8, struct bfmr_boot_fail_info)
#define BFMR_GET_DEV_PATH _IOR(BFMR_IOCTL_BASE, 9, struct bfmr_dev_path)
#define BFMR_ENABLE_CTRL _IOW(BFMR_IOCTL_BASE, 10, int)
#define BFMR_ACTION_TIMER_CTL _IOW(BFMR_IOCTL_BASE, 11, struct action_ioctl_data)
#define BFMR_IOC_CREATE_DYNAMIC_RAMDISK _IOWR(BFMR_IOCTL_BASE, 12, struct bfmr_dbrd_ioctl_block)
#define BFMR_IOC_DELETE_DYNAMIC_RAMDISK _IOW(BFMR_IOCTL_BASE, 13, struct bfmr_dbrd_ioctl_block)
#define BFMR_IOC_CHECK_BOOTDISK_WP _IOWR(BFMR_IOCTL_BASE, 14, struct bfmr_bootdisk_wp_status_iocb)
#define BFMR_IOC_ENABLE_MONITOR   _IOW(BFMR_IOCTL_BASE, 15, struct bfmr_storage_rochk_iocb)
#define BFMR_IOC_DO_STORAGE_WRTRY _IOWR(BFMR_IOCTL_BASE, 16, struct bfmr_storage_rochk_iocb)
#define BFMR_IOC_GET_STORAGE_ROFA_INFO _IOWR(BFMR_IOCTL_BASE, 17, struct bfmr_storage_rofa_info_iocb)
#define BFMR_IOC_GET_BOOTDEVICE_DISK_COUNT _IOWR(BFMR_IOCTL_BASE, 18, struct bfmr_storage_rochk_iocb)
#define BFMR_IOC_GET_BOOTDEVICE_DISK_INFO _IOWR(BFMR_IOCTL_BASE, 19, struct bfmr_bootdevice_disk_info_iocb)
#define BFMR_IOC_GET_BOOTDEVICE_PROD_INFO _IOWR(BFMR_IOCTL_BASE, 20, struct bfmr_bootdevice_prod_info_iocb)


/*----global variables----------------------------------------------------------------*/


/*----export function prototypes--------------------------------------------------------*/

/**
    @function: int boot_fail_err(bfmr_bootfail_errno_e bootfail_errno,
        bfr_suggested_recovery_method_e suggested_recovery_method,
        bfmr_bootfail_addl_info_t *paddl_info)
    @brief: save the log and do proper recovery actions when meet with error during system booting process.

    @param: bootfail_errno [in], boot fail error no.
    @param: suggested_recovery_method [in], suggested recovery method, if you don't know, please transfer NO_SUGGESTION for it
    @param: paddl_info [in], saving additional info such as log path and so on.

    @return: 0 - succeeded; -1 - failed.

    @note:
*/
int boot_fail_err(bfmr_bootfail_errno_e bootfail_errno,
    bfr_suggested_recovery_method_e suggested_recovery_method,
    bfmr_bootfail_addl_info_t *paddl_info);

/**
    @function: int bfmr_get_boot_stage(bfmr_detail_boot_stage_e *pboot_stage)
    @brief: get current boot stage during boot process.

    @param: pboot_stage [out], buffer storing the boot stage.

    @return: 0 - succeeded; -1 - failed.

    @note:
*/
int bfmr_get_boot_stage(bfmr_detail_boot_stage_e *pboot_stage);

/**
    @function: int bfmr_set_boot_stage(bfmr_detail_boot_stage_e boot_stage)
    @brief: get current boot stage during boot process.

    @param: boot_stage [in], boot stage to be set.

    @return: 0 - succeeded; -1 - failed.

    @note:
*/
int bfmr_set_boot_stage(bfmr_detail_boot_stage_e boot_stage);

/**
    @function: int bfmr_get_timer_state(int *state)
    @brief: get state of the timer.

    @param: state [out], the state of the boot timer.

    @return: 0 - success, -1 - failed.

    @note:
        1. this fuction only need be initialized in kernel.
        2. if *state == 0, the boot timer is disabled, if *state == 1, the boot timer is enbaled.
*/
int bfmr_get_timer_state(int *state);

/**
    @function: int bfm_enable_timer(void)
    @brief: enbale timer.

    @param: none.

    @return: 0 - succeeded; -1 - failed.

    @note:
*/
int bfmr_enable_timer(void);

/**
    @function: int bfm_disable_timer(void)
    @brief: disable timer.

    @param: none.

    @return: 0 - succeeded; -1 - failed.

    @note:
*/
int bfmr_disable_timer(void);

/**
    @function: int bfm_set_timer_timeout_value(unsigned int timeout)
    @brief: set timeout value of the kernel timer. Note: the timer which control the boot procedure is in the kernel.

    @param: timeout [in] timeout value (unit: msec).

    @return: 0 - succeeded; -1 - failed.

    @note:
*/
int bfmr_set_timer_timeout_value(unsigned int timeout);

/**
    @function: int bfm_get_timer_timeout_value(unsigned int *timeout)
    @brief: get timeout value of the kernel timer. Note: the timer which control the boot procedure is in the kernel.

    @param: timeout [in] buffer will store the timeout value (unit: msec).

    @return: 0 - succeeded; -1 - failed.

    @note:
*/
int bfmr_get_timer_timeout_value(unsigned int *timeout);

/**
    @function: int bfm_init(void)
    @brief: init bfm module in kernel.

    @param: none.

    @return: 0 - succeeded; -1 - failed.

    @note: it need be initialized in bootloader and kernel.
*/
int bfm_init(void);

int bfm_get_log_count(char *bfmr_log_root_path);
void bfm_delete_dir(char *log_path);

//send sgnal to init, it will show init task trace.
void bfm_send_signal_to_init(void);


#ifdef __cplusplus
}
#endif

#endif

