/*
 * upload_double_free.c
 *
 * the upload_double_free.c provide interface to upload stack trace when double free happen
 *
 * chenli <chenli45@huawei.com>
 *
 * Copyright (c) 2018-2022, Huawei Tech. Co., Ltd. All rights reserved.
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
#define MAX_MESSAGE_LENGTH 512
#define DOUBLE_FREE_STACK_DEPTH 10

struct double_free_upload_work
{
	struct work_struct worker;
	char data[MAX_MESSAGE_LENGTH];
};

static void double_free_work_handle(struct work_struct *work)
{
	struct double_free_upload_work *work_node = container_of(work, struct double_free_upload_work, worker);

	struct stp_item item;
	item.id = item_info[DOUBLE_FREE].id;
	item.status = STP_RISK;
	item.credible = STP_CREDIBLE;
	item.version = 0;
	(void)strncpy(item.name, item_info[DOUBLE_FREE].name, STP_ITEM_NAME_LEN-1);
	if (kernel_stp_upload(item, work_node->data))
	{
		pr_err("stp double free upload fail\n");
	}
	kfree(work_node);
}


void upload_double_free_log(struct kmem_cache *s, char *add_info)
{
	if (s == NULL || add_info == NULL)
	{
		return;
	}
	struct double_free_upload_work *work_node = kzalloc(sizeof(struct double_free_upload_work), GFP_ATOMIC);
	if (work_node == NULL)
	{
		return;
	}

	unsigned int len;
	unsigned long entries[DOUBLE_FREE_STACK_DEPTH] = {0};
	struct stack_trace trace;
	trace.nr_entries = 0;
	trace.max_entries = DOUBLE_FREE_STACK_DEPTH;
	trace.entries = &entries[0];
	trace.skip = 0;

	save_stack_trace(&trace);

	int ret = snprintf(work_node->data, MAX_MESSAGE_LENGTH, "add_info:%s, kmem_cache_name:%s, pid:%d, pcomm:%.20s, tgid:%d, tgcomm:%.20s, stack:\n",add_info, s->name, current->pid, current->comm, current->tgid, current->group_leader->comm);

	if (ret >= MAX_MESSAGE_LENGTH)
	{
		work_node->data[MAX_MESSAGE_LENGTH-1] = 0;
		goto out;
	}

	ret += snprint_stack_trace(work_node->data + ret, MAX_MESSAGE_LENGTH - ret, &trace, 0);

out:
	INIT_WORK(&work_node->worker, double_free_work_handle);
	queue_work(system_long_wq, &work_node->worker);
}
