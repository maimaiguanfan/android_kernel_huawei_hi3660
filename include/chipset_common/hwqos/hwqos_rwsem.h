/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: Qos schedule declaration
 * Author: ZhangDaPeng zhangdapeng1@huawei.com
 * Create: 2019-03-01
 */

#ifndef _HWQOS_RWSEM_H_
#define _HWQOS_RWSEM_H_
#include "hwqos_common.h"

void rwsem_dynamic_qos_enqueue(struct task_struct *owner,
	struct task_struct *waiter_task);
void rwsem_dynamic_qos_dequeue(struct task_struct *task);
#endif
