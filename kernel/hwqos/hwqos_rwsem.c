/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: Qos schedule implementation
 * Author: ZhangDaPeng zhangdapeng1@huawei.com
 * Create: 2019-03-01
 */

#include <linux/sched.h>

#include <chipset_common/hwqos/hwqos_common.h>

#define RWSEM_READER_OWNED ((struct task_struct *)1UL)

static inline bool rwsem_owner_is_writer(struct task_struct *owner)
{
	return owner && (owner != RWSEM_READER_OWNED);
}

void rwsem_dynamic_qos_enqueue(struct task_struct *owner,
	struct task_struct *waiter_task)
{
	if (rwsem_owner_is_writer(owner))
		dynamic_qos_enqueue(owner, waiter_task, DYNAMIC_QOS_RWSEM);
}

void rwsem_dynamic_qos_dequeue(struct task_struct *task)
{
	dynamic_qos_dequeue(task, DYNAMIC_QOS_RWSEM);
}
