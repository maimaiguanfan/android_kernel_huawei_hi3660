/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2018. All rights reserved.
 *
 * Description: interfaces to indicate user types in kernel
 *      Author: shenchenkai <shenchenkai@huawei.com>
 */

#ifndef LOG_USERTYPE_H
#define LOG_USERTYPE_H

#define COMMERCIAL_USER            1
#define FANS_USER                  2
#define BETA_USER                  3
#define TEST_USER                  4
#define OVERSEA_USER               5
#define OVERSEA_COMMERCIAL_USER    6

unsigned int get_log_usertype(void);
#define get_logusertype_flag	get_log_usertype

#endif
