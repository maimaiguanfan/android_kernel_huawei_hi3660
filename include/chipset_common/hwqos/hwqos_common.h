/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: Qos schedule declaration
 * Author: JiangXiaofeng jiangxiaofeng8@huawei.com
 * Create: 2019-03-01
 */

#ifndef HWQOS_COMMON_H
#define HWQOS_COMMON_H

#include "hwqos_sysctl.h"

void set_task_qos(struct task_struct *task, int qos);
int get_task_qos(struct task_struct *task);
void dynamic_qos_enqueue(struct task_struct *task,
	struct task_struct *from, unsigned int type);
void dynamic_qos_dequeue(struct task_struct *task, unsigned int type);
void iaware_proc_fork_inherit(struct task_struct *task,
	struct task_struct *from);
#endif
