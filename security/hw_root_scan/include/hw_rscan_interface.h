/*
 * hw_rscan_interface.h
 *
 * the hw_rscan_interface.h for TEE root scan using.
 *
 * likun <quentin.lee@huawei.com>
 *
 * Copyright (c) 2001-2021, Huawei Tech. Co., Ltd. All rights reserved.
 */

#ifndef _HW_RSCAN_INTERFACE_H_
#define _HW_RSCAN_INTERFACE_H_
#include <chipset_common/security/root_scan.h>
#ifndef HASH_DIGEST_SIZE
#define HASH_DIGEST_SIZE 32  /* here use sha256 */
#endif

#define MAX_RPROC_SIZE 10240 /* 10k */


/* check all items */
#define RSOPID_ALL (0xFFFFFFFF)

enum skip_value {
	NOT_SKIP = 0,
	SKIP = 1,
};

struct rscan_skip_flags {
	int skip_kcode;
	int skip_syscall;
	int skip_se_hooks;
	int skip_se_status;
	int skip_rprocs;
	int skip_setid;
};

struct rscan_result_dynamic {
	uint8_t kcode[HASH_DIGEST_SIZE];	/* kernel code hash */
	uint8_t syscalls[HASH_DIGEST_SIZE];	/* system call table hash */
	uint8_t sehooks[HASH_DIGEST_SIZE]; /* struct security_operations hash */
	int seenforcing;	/* SE-Linux state */
	char rprocs[MAX_RPROC_SIZE];	/* root processes */
	int setid;                      /* get setids */
};

struct rscan_status {
	uint32_t cpuload;	/* CPU usage: 0~100 */
	uint32_t battery;	/* Remaining battery: 0~100 */
	uint32_t charging;	/* Is it charging? 0~1 */

	/* Current time in UNIX Epoch time. same as the time() systemcall */
	uint32_t time;

	/*
	 * Timezone in minutes.
	 * see tz_minuteswest in gettimeofday().
	 * e.g. Beijing +8 is -480
	 */
	uint32_t timezone;
};

enum HW_RSCAN_ERR_CODE {
	RSCAN_SUCC = 0x0,	/* succeed */
	RSCAN_ERR_SECURE_MODE = 0xFFFF0001, /* env not ready for secure mode */
	RSCAN_ERR_NULL_POINTER = 0xFFFF0002,	/* null pointer error */
	RSCAN_ERR_KOBJ_ADD = 0xFFFF0003,	/* creat kobject failed */
	RSCAN_ERR_KSET_ADD = 0xFFFF0004,	/* creat kset failed */
	RSCAN_ERR_SCANNER_INIT = 0xFFFF0005,	/* scanner init failed */
	RSCAN_ERR_KOBJECT_EVENT = 0xFFFF0006,	/* err in kobject_uevent */

	RSCAN_ERR_GENARIC = 0xFFFFF000,	/* genaric error */
};

/*
 * get_ro_secure - for TEE and kernel space to
 *  aquire the current status of complie
 * Description: get ro.secure property, 0 stand for running in engneering mode
 * other value stand for normal mode
 * @return: running statu, return 0 with engneering mode.
 * Otherwise return none zero value
 */
int get_ro_secure(void);

/*
 * rscan_dynamic_raw - for TEE calling
 * Description: root scan by dynamic features, with raw result
 * @ops_mask, bit masks for caller to set different items
 *         for scanner. The lowest bit is for kernel code check, then the
 *         30th bit for system call verify, and so on.
 * @result, used for return results for each scanner item
 *          that correspond to ops_mask. In this function, result keep its
 *          original statu. For example, rproc contains whitelist.
 */
int rscan_dynamic_raw(uint op_mask, struct rscan_result_dynamic *result);

/*
 * rscan_dynamic_raw_and_upload - for TEE calling
 * Description: root scan by dynamic features, with raw result, upload to the
 *              stp and return the status
 * @ops_mask, bit masks for caller to set different items
 *         for scanner. The lowest bit is for kernel code check, then the
 *         30th bit for system call verify, and so on.
 * @result, used for return results for each scanner item
 *          that correspond to ops_mask. In this function, result keep its
 *          original statu. For example, rproc contains whitelist.
 */
int rscan_dynamic_raw_and_upload(uint op_mask, struct rscan_result_dynamic *result);

/*
 * rscan_get_status - get the device current status, such as battery, charging
 * Description: get the current status of the device
 * @status, buffer for getting device status in
 * @return: Result of loading.
 *     0, get device correctly.
 *     -1, parameter invalid.
 *     1,  get current battery status failed.
 *         in this case, battery and charging would be 0.
 */
int rscan_get_status(struct rscan_status *status);

/*
 * load_rproc_whitelist - for both TEE and kernel to load rproc whitelist
 * Description: read running root procs from kernel
 * @whitelist, buffer for loading whitelist in
 * @len, length of buffer whitelist. This parameter must be precise,
 *          otherwise this function will be failed.
 * @return:
 *     Result of loading.
 *     0, load whitelist correctly.
 *     -1, generic error
 */
int load_rproc_whitelist(char *whitelist, size_t len);

#endif
