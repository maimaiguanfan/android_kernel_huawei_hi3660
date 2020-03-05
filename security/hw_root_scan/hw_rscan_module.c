/*
 * hw_rscan_module.c
 *
 * the hw_rscan_module.c for root scanner kernel space init and deinit
 *
 * likun <quentin.lee@huawei.com>
 * likan <likan82@huawei.com>
 *
 * Copyright (c) 2001-2021, Huawei Tech. Co., Ltd. All rights reserved.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <linux/types.h>
#include "./include/hw_rscan_utils.h"
#include "./include/hw_rscan_scanner.h"
#include "./include/hw_rscan_proc.h"
#include <chipset_common/security/hw_kernel_stp_interface.h>
#include <linux/workqueue.h>

static int __init rscan_module_init(void);
static void __exit rscan_module_exit(void);
static const char *TAG = "hw_rscan_module";
static void rscan_init_do_work(struct work_struct *data);
static DECLARE_WORK(rscan_init, rscan_init_do_work);

static void rscan_init_do_work(struct work_struct *data)
{
	int result = 0;

	RSLogDebug(TAG, "rscan work init.");

	do {
		/* init dynamic scanner */
		result = rscan_dynamic_init();
		if (result != 0) {
			RSLogError(TAG, "dynamic scanner init failed: %d",
						result);
			break;
		}

		/* init proc file */
		result = rscan_proc_init();
		if (result != 0) {
			RSLogError(TAG, "rscan_proc_init init failed.");
			break;
		}

		/* register to stp */
		(void)kernel_stp_scanner_register(stp_rscan_trigger);
	} while (0);

	if (0 != result) {
		/* The function __init should not references __exit*/
		/*rscan_module_exit();*/
		rscan_proc_deinit();
	}

	RSLogTrace(TAG, "+++root scan init end, result:%d", result);
}

static int __init rscan_module_init(void)
{
	if (0 == queue_work(system_wq, &rscan_init))
		RSLogError(TAG, "add rscan_init failed!");

	return 0;
}

static void __exit rscan_module_exit(void)
{
	rscan_proc_deinit();
}

late_initcall(rscan_module_init);   /* lint -save -e528 */
module_exit(rscan_module_exit);   /* lint -save -e528 */

MODULE_AUTHOR("likun <quentin.lee@huawei.com>");
MODULE_DESCRIPTION("Huawei root scanner");
