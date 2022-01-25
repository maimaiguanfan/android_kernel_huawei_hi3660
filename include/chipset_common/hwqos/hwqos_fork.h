/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: Qos schedule declaration
 * Author: JiangXiaofeng jiangxiaofeng8@huawei.com
 * Create: 2019-03-01
 */

#ifndef HWQOS_FORK_H
#define HWQOS_FORK_H

#define QOS_BASE_VALUE 0x1

static inline void init_task_qos_info(struct task_struct *p)
{
	atomic_set(&(p->dynamic_qos), QOS_BASE_VALUE);
	atomic_set(&(p->trans_flags), 0);
	p->static_vip = 0;
}
#endif
