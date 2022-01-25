/*
 * upload_double_free.c
 *
 * the upload_double_free.c provide interface to
 *
 * upload stack trace when double free happen
 *
 * Copyright (c) 2012-2019, Huawei Tech. Co., Ltd. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/printk.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <linux/security.h>
#include <linux/slub_def.h>
#include <chipset_common/security/hw_kernel_stp_interface.h>
#include <linux/module.h>
#define MAX_MESSAGE_LENGTH 512
#define DOUBLE_FREE_STACK_DEPTH 10

struct double_free_upload_work {
	struct work_struct worker;
	char data[MAX_MESSAGE_LENGTH];
};
/*
 * when found double free
 * slub caceh maybe in wrong status
 * thus function 'upload_double_free_log' should not use kalloc
 * so we use static g_work_node otherwise using kalloc
 */
static struct double_free_upload_work g_work_node;
static void double_free_work_handle(struct work_struct *work)
{
	struct stp_item item;
	struct double_free_upload_work *work_node = container_of(
		work, struct double_free_upload_work, worker);

	item.id = item_info[DOUBLE_FREE].id;
	item.status = STP_RISK;
	item.credible = STP_CREDIBLE;
	item.version = 0;

	(void)strncpy(item.name,
		item_info[DOUBLE_FREE].name, STP_ITEM_NAME_LEN - 1);
	if (kernel_stp_upload(item, work_node->data))
		pr_err("stp double free upload fail\n");
}

void upload_double_free_log(struct kmem_cache *s, char *add_info)
{
	int ret;
	unsigned long entries[DOUBLE_FREE_STACK_DEPTH] = {0};
	struct stack_trace trace;

	if ((s == NULL) || (add_info == NULL))
		return;
	/* If context is in irq ,we can NOT do log upload etc.*/
	if (in_irq() || in_softirq() || irqs_disabled())
		return;
	(void)memset(&g_work_node,
		0, sizeof(struct double_free_upload_work));
	trace.nr_entries = 0;
	trace.max_entries = DOUBLE_FREE_STACK_DEPTH;
	trace.entries = &entries[0];
	trace.skip = 0;

	save_stack_trace(&trace);

	ret = snprintf(g_work_node.data, MAX_MESSAGE_LENGTH,
		"add_info:%s, kmem_cache_name:%s, pid:%d, pcomm:%.20s, tgid:%d, tgcomm:%.20s, stack:\n",
		add_info, s->name, current->pid,
		current->comm, current->tgid,
		current->group_leader->comm);
	if (ret >= MAX_MESSAGE_LENGTH) {
		g_work_node.data[MAX_MESSAGE_LENGTH - 1] = 0;
		goto out;
	}
	ret += snprint_stack_trace(
		g_work_node.data + ret, MAX_MESSAGE_LENGTH - ret, &trace, 0);
out:
	INIT_WORK(&g_work_node.worker, double_free_work_handle);
	queue_work(system_long_wq, &g_work_node.worker);
}

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("upload stack trace when double free happen");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
