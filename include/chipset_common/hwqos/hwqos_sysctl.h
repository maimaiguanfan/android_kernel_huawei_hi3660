/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: Qos schedule declaration
 * Author: WangHui jack.wanghui@huawei.com
 * Create: 2019-03-01
 */

#ifndef _HWQOS_SYSCTL_H_
#define _HWQOS_SYSCTL_H_
extern unsigned int g_sysctl_qos_sched;
#define QOS_SCHED_SET_ENABLE      (g_sysctl_qos_sched & 0x0001)
#define QOS_SCHED_ENQUEUE_ENABLE  (g_sysctl_qos_sched & 0x0002)
#define RTG_TRANS_ENABLE          (g_sysctl_qos_sched & 0x0004)
#define QOS_SCHED_LOCK_ENABLE     (g_sysctl_qos_sched & 0x0008)
#define BINDER_SCHED_ENABLE       (g_sysctl_qos_sched & 0x0010)
#define BINDER_SCHED_DEBUG_ENABLE (g_sysctl_qos_sched & 0x0020)
#endif
