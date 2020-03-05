/*
 * hw_kernel_stp_uploader.h
 *
 * the hw_kernel_stp_uploader.h for kernel data uploading through uevent.
 *
 * sunhongqing <sunhongqing@huawei.com>
 *
 * Copyright (c) 2001-2021, Huawei Tech. Co., Ltd. All rights reserved.
 */

#ifndef _HW_KERNEL_STP_UPLOADER_H_
#define _HW_KERNEL_STP_UPLOADER_H_

#include <linux/kobject.h>
#include <linux/slab.h>
#include "hw_kernel_stp_utils.h"
#include <chipset_common/security/hw_kernel_stp_interface.h>

#define KERNEL_STP_UEVENT_LEN  2
#define STP_INFO_MAXLEN   1024 

int kernel_stp_uploader_init(void);
void kernel_stp_uploader_exit(void);
#endif /* _HW_KERNEL_STP_UPLOADER_H_*/

