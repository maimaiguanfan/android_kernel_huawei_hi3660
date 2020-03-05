/*
 * hw_kernel_stp_utils.h
 *
 * the hw_kernel_stp_utils.h for kernel stp
 *
 * likun <quentin.lee@huawei.com>
 *
 * Copyright (c) 2001-2021, Huawei Tech. Co., Ltd. All rights reserved.
 */

#ifndef _HW_KERNEL_STP_UTILS_H_
#define _HW_KERNEL_STP_UTILS_H_

#include <asm/segment.h>
#include <linux/buffer_head.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/kobject.h>
#include <linux/ratelimit.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/vmalloc.h>

#define KSTP_ERRCODE -1

#define KSTP_ERROR "[Error]"
#define KSTPLogError(tag, fmt, args...) \
	pr_err_ratelimited("%s[%s][%s] " fmt "\n", KSTP_ERROR,\
						tag, __func__, ##args)

#define KSTP_WARNING "[Warning]"
#define KSTPLogWarning(tag, fmt, args...) \
	pr_warn_ratelimited("%s[%s][%s] " fmt "\n", KSTP_WARNING,\
						tag, __func__, ##args)

#define KSTP_TRACE "[Trace]"
#define KSTPLogTrace(tag, fmt, args...) \
	pr_info_ratelimited("%s[%s][%s] " fmt "\n", KSTP_TRACE,\
						tag, __func__, ##args)

#ifdef CONFIG_HW_KERNEL_STP_ENG_DEBUG

#define KSTP_DEBUG "[Debug]"
#define KSTPLogDebug(tag, fmt, args...) \
	printk_ratelimited(KERN_DEBUG "%s[%s][%s] " fmt "\n", KSTP_DEBUG,\
							tag, __func__, ##args)

#else
#define KSTPLogDebug(tag, fmt, args...) no_printk(fmt, ##args)	
#endif

#endif /* _HW_KERNEL_STP_UTILS_H_ */
