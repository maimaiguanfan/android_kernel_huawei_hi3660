/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: Qos schedule declaration
 * Author: JiangXiaofeng jiangxiaofeng8@huawei.com
 * Create: 2019-03-01
 */

#ifndef IAWARE_QOS_H
#define IAWARE_QOS_H

#include <linux/sched.h>

#define QOS_CTRL_MAGIC 'q'

enum {
	GET_QOS_STAT = 1,
	SET_QOS_STAT,
	GET_QOS_WHOLE,
	QOS_CTRL_MAX_NR,
};

#define QOS_CTRL_GET_QOS_STAT \
	_IOR(QOS_CTRL_MAGIC, GET_QOS_STAT, struct qos_stat)
#define QOS_CTRL_SET_QOS_STAT \
	_IOWR(QOS_CTRL_MAGIC, SET_QOS_STAT, struct qos_stat)
#define QOS_CTRL_GET_QOS_WHOLE \
	_IOR(QOS_CTRL_MAGIC, GET_QOS_WHOLE, struct qos_whole)

struct qos_stat {
	pid_t pid;
	int qos;
};

struct qos_whole {
	pid_t pid;
	int dynamic_qos;
	int trans_flags;
};

#endif
