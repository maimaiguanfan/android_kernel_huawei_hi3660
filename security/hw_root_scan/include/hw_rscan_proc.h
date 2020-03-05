/*
 * hw_rscan_proc.h
 *
 * the hw_rscan_proc.h for proc file create and destroy
 *
 * likun <quentin.lee@huawei.com>
 *
 * Copyright (c) 2001-2021, Huawei Tech. Co., Ltd. All rights reserved.
 */

#ifndef _HW_RSCAN_PROC_H_
#define _HW_RSCAN_PROC_H_

#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/vmalloc.h>
#include "./include/hw_rscan_scanner.h"

int rscan_proc_init(void);
void rscan_proc_deinit(void);

#endif

