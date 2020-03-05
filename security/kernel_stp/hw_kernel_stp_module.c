/*
 * hw_kernel_stp_module.c
 *
 * the hw_kernel_stp_module.c for kernel stp module init and deinit
 *
 * sunhongqing <sunhongqing@huawei.com>
 *
 * Copyright (c) 2001-2021, Huawei Tech. Co., Ltd. All rights reserved.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include "hw_kernel_stp_utils.h"
#include "hw_kernel_stp_proc.h"
#include "hw_kernel_stp_scanner.h"
#include "hw_kernel_stp_uploader.h"

static const char *TAG = "kernel_stp_module";

struct kernel_stp_module_work {
	struct workqueue_struct *kernel_stp_wq;
	struct work_struct kernel_stp_work;
};

static struct kernel_stp_module_work kernel_stp_work_data;


static void kernel_stp_work_init(struct work_struct *data)
{
	int result = 0;

	KSTPLogTrace(TAG, "kernel stp work init.");

	do{
		/* init kernel stp proc */
		result = kernel_stp_proc_init();
		if (result != 0) {
			KSTPLogError(TAG, "kernel_stp_proc init failed.");
			break;
		}

		/* init kernel stp scanner */
		result = kernel_stp_scanner_init();
		if (result != 0) {
			KSTPLogError(TAG, "kernel_stp_scanner init failed.");
			break;
		}

		/* init kernel stp uploader */
		result = kernel_stp_uploader_init();
		if (result != 0) {
			KSTPLogError(TAG, "kernel_stp_uploader init failed.");
			break;
		}
	} while(0);

	if(result != 0) {

		if (kernel_stp_work_data.kernel_stp_wq != NULL) {
			destroy_workqueue(kernel_stp_work_data.kernel_stp_wq);
			kernel_stp_work_data.kernel_stp_wq = NULL;
		}

		kernel_stp_proc_exit();
		kernel_stp_scanner_exit();
		kernel_stp_uploader_exit();

		KSTPLogError(TAG, "kernel stp module init failed.");
		return;
	}

	KSTPLogTrace(TAG,"kernel stp module init success.");
}

static int __init kernel_stp_module_init(void)
{
	kernel_stp_work_data.kernel_stp_wq = create_singlethread_workqueue("HW_KERNEL_STP_MODULE");

	if (kernel_stp_work_data.kernel_stp_wq == NULL) {
		KSTPLogError(TAG, "kernel stp module wq error, no mem");
		return -ENOMEM;
	}

	INIT_WORK(&(kernel_stp_work_data.kernel_stp_work), kernel_stp_work_init);
	queue_work(kernel_stp_work_data.kernel_stp_wq, &(kernel_stp_work_data.kernel_stp_work));

	return 0;
}

static void __exit kernel_stp_module_exit(void)
{
	if (kernel_stp_work_data.kernel_stp_wq != NULL) {
		destroy_workqueue(kernel_stp_work_data.kernel_stp_wq);
		kernel_stp_work_data.kernel_stp_wq = NULL;
	}

	kernel_stp_proc_exit();
	kernel_stp_scanner_exit();
	kernel_stp_uploader_exit();

	return;
}

module_init(kernel_stp_module_init); 
module_exit(kernel_stp_module_exit); 

MODULE_AUTHOR("sunhongqing <sunhongqing@huawei.com>");
MODULE_DESCRIPTION("Huawei kernel stp");
