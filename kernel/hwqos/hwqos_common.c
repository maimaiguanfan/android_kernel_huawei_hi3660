/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: Qos schedule implementation
 * Author: JiangXiaofeng jiangxiaofeng8@huawei.com
 * Create: 2019-03-01
 */

#include <linux/sched.h>
#include <trace/events/sched.h>
#include <../kernel/sched/sched.h>
#include <linux/kernel.h>
#ifdef CONFIG_HW_RTG_SCHED
#include "hwrtg/trans_rtg.h"
#endif

#define BASE_FLAG 0x00000001

#define get_trans_type(flags, type) \
	(((int)flags) & (BASE_FLAG << type))
#define set_trans_type(flags, type) \
	(((int)flags) | (BASE_FLAG << type))
#define remove_trans_type(flags, type) \
	(((int)flags) & (~(BASE_FLAG << type)))

atomic_t g_init_qos = ATOMIC_INIT(0);
// binary:0001, enable qos set by default
unsigned int g_sysctl_qos_sched = 1;

static inline struct transact_qos *get_transact_qos(unsigned int type,
	struct task_struct *task)
{
	return &(task->trans_qos[type]);
}

void set_task_qos(struct task_struct *task, int qos)
{
	if (unlikely(!QOS_SCHED_SET_ENABLE))
		return;
	if (!task)
		return;
	get_task_struct(task);
	atomic_set(&task->dynamic_qos, qos); /*lint !e446 !e734*/
	trace_sched_qos(task, NULL, OPERATION_QOS_SET);
	put_task_struct(task);
}

int get_trans_qos_by_type(struct task_struct *task, unsigned int type)
{
	struct transact_qos *tq;
	int qos;
	int flags = atomic_read(&task->trans_flags);

	if (!get_trans_type(flags, type))
		return VALUE_QOS_LOW;
	tq = get_transact_qos(type, task);
	if (!tq->qos)
		return VALUE_QOS_LOW;
	qos = atomic_read(tq->qos); /*lint !e529*/
	if ((qos < VALUE_QOS_LOW) || (qos > VALUE_QOS_CRITICAL))
		return VALUE_QOS_LOW;
	return qos;
}

int get_task_qos(struct task_struct *task)
{
	unsigned int i;
	int qos_trans = 0;
	int qos;

	if (unlikely(!QOS_SCHED_SET_ENABLE))
		return VALUE_QOS_NORMAL;
	if (!task)
		return VALUE_QOS_NORMAL;
	get_task_struct(task);
	qos = atomic_read(&task->dynamic_qos);
	if (qos == VALUE_QOS_CRITICAL) {
		put_task_struct(task);
		return qos;
	}
	for (i = DYNAMIC_QOS_BINDER; i < DYNAMIC_QOS_TYPE_MAX; i++) {
		qos_trans = get_trans_qos_by_type(task, i);
		qos = (qos > qos_trans) ? qos : qos_trans;
		if (qos == VALUE_QOS_CRITICAL)
			break;
	}
	put_task_struct(task);
	return qos;
}

void dynamic_qos_enqueue(struct task_struct *task,
	struct task_struct *from, unsigned int type)
{
	unsigned int i = 0;
	atomic_t *trans_qos = NULL;

	if (unlikely(!QOS_SCHED_ENQUEUE_ENABLE))
		return;
	if ((!task) || (!from))
		return;
	if (get_trans_qos_by_type(task, type) == VALUE_QOS_CRITICAL)
		return;
	if (atomic_read(&from->dynamic_qos) == VALUE_QOS_CRITICAL) {
		trans_qos = &from->dynamic_qos;
	} else {
		for (i = DYNAMIC_QOS_BINDER; i < DYNAMIC_QOS_TYPE_MAX; i++) {
			if (get_trans_qos_by_type(from, i) ==
				VALUE_QOS_CRITICAL) {
				trans_qos = get_transact_qos(i, from)->qos;
				break;
			}
		}
	}
	if (trans_qos) {
		int flags = 0;
		struct transact_qos *tq = get_transact_qos(type, task);

		tq->qos = trans_qos;
		tq->trans_from = from->pid;
		tq->trans_type = type;
#ifdef CONFIG_HW_RTG_SCHED
		if (RTG_TRANS_ENABLE && (type == DYNAMIC_QOS_BINDER))
			add_trans_thread(task, from);
#endif
		flags = set_trans_type(atomic_read(&task->trans_flags), type);
		atomic_set(&task->trans_flags, flags); /*lint !e446 !e734*/
		trace_sched_qos(task, tq, OPERATION_QOS_ENQUEUE);
	}
}

/*lint -save -e502 -e446 -e734*/
void dynamic_qos_dequeue(struct task_struct *task, unsigned int type)
{
	int flags;
	struct transact_qos *tq;

	if (unlikely(!QOS_SCHED_ENQUEUE_ENABLE))
		return;
	if (!task)
		return;
	flags = atomic_read(&task->trans_flags);
	if (!get_trans_type(flags, type))
		return;
	tq = get_transact_qos(type, task);
	tq->qos = &g_init_qos;
	flags = remove_trans_type(atomic_read(&task->trans_flags), type);
	atomic_set(&task->trans_flags, flags);
#ifdef CONFIG_HW_RTG_SCHED
	if (RTG_TRANS_ENABLE && (type == DYNAMIC_QOS_BINDER))
		remove_trans_thread(task);
#endif
	trace_sched_qos(task, tq, OPERATION_QOS_DEQUEUE);
}
/*lint -restore*/

void iaware_proc_fork_inherit(struct task_struct *task,
	struct task_struct *from)
{
	int qos;

	if (unlikely(!QOS_SCHED_SET_ENABLE))
		return;
	if (!from)
		return;
	qos = atomic_read(&from->dynamic_qos);
	if (!task)
		return;
	get_task_struct(task);
	if (qos == VALUE_QOS_CRITICAL)
		qos = VALUE_QOS_HIGH;
	atomic_set(&task->dynamic_qos, qos); /*lint !e446 !e734*/
	trace_sched_qos(task, NULL, OPERATION_QOS_SET);
	put_task_struct(task);
}
