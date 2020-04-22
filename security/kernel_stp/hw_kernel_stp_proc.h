/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2018. All rights reserved.
 * Description: the hw_kernel_stp_proc.h for proc file create and destroy
 * Author: sunhongqing <sunhongqing@huawei.com>
 * Create: 2018-03-31
 */

#ifndef _HW_KERNEL_STP_PROC_H_
#define _HW_KERNEL_STP_PROC_H_

#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/vmalloc.h>
#include <linux/uidgid.h>
#include "hw_kernel_stp_utils.h"
#include "hw_kernel_stp_scanner.h"

#define KERNEL_STP_TRIGGER_MARK 0xffffffff
#define KERNEL_STP_PROC_MAX_LEN 16
#define KERNEL_STP_PROC_HEX_BASE 16

int kernel_stp_proc_init(void);
void kernel_stp_proc_exit(void);

#endif

