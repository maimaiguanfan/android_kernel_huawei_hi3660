/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2018. All rights reserved.
 * Description: the hw_kernel_stp_uploader.h for kernel data uploading through uevent
 * Author: sunhongqing <sunhongqing@huawei.com>
 * Create: 2018-03-31
 */

#ifndef _HW_KERNEL_STP_UPLOADER_H_
#define _HW_KERNEL_STP_UPLOADER_H_

#include <linux/kobject.h>
#include <linux/slab.h>
#include <chipset_common/security/hw_kernel_stp_interface.h>
#include "hw_kernel_stp_utils.h"

#define KERNEL_STP_UEVENT_LEN  2
#define STP_INFO_MAXLEN   1024

int kernel_stp_uploader_init(void);
void kernel_stp_uploader_exit(void);
#endif /* _HW_KERNEL_STP_UPLOADER_H_*/

