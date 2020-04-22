/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2018. All rights reserved.
 * Description: the hw_kernel_stp_scanner.h for kernel stp trigger scan
 * Author: sunhongqing <sunhongqing@huawei.com>
 * Create: 2018-03-31
 */

#ifndef _HW_KERNEL_STP_SCANNER_H_
#define _HW_KERNEL_STP_SCANNER_H_

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/types.h>
#include <linux/mutex.h>
#include "hw_kernel_stp_utils.h"
#include <chipset_common/security/hw_kernel_stp_interface.h>

#define KSTP_SCAN_LIST_INIT 1
#define KSTP_SCAN_LIST_UNINIT 0

struct kernel_stp_scanner_node {
	struct list_head stp_scanner_list; /* kernel stp scanner list */
	stp_cb callbackfunc; /* detection modules callbackfunc */
};

struct kernel_stp_scan_work {
	stp_cb callbackfunc; /* detection modules callbackfunc */
	struct work_struct work;
};

int kernel_stp_scanner_init(void);

void kernel_stp_scanner(void);

void kernel_stp_scanner_exit(void);

#endif /* _HW_KERNEL_STP_SCANNER_H_ */

