/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2016-2018. All rights reserved.
 * Description: the sescan.h for selinux status checking
 * Author: Yongzheng Wu <Wu.Yongzheng@huawei.com>
 *         likun <quentin.lee@huawei.com>
 *         likan <likan82@huawei.com>
 * Create: 2016-06-18
 */

#ifndef _SESCAN_H_
#define _SESCAN_H_

#include <asm-generic/sections.h>
#include <linux/crypto.h>
#include <linux/kallsyms.h>
#include <linux/kernel.h>
#include <linux/scatterlist.h>
#include <linux/security.h>
#include <linux/string.h>
#include <linux/version.h>
#include <crypto/hash.h>
#include <crypto/hash_info.h>
#include "./include/hw_rscan_utils.h"

/* selinux_enforcing is kernel variable */
#ifdef CONFIG_SECURITY_SELINUX_DEVELOP
extern int selinux_enforcing;
#else
#define selinux_enforcing 1
#endif

int get_selinux_enforcing(void);
int sescan_hookhash(uint8_t *hash);

#endif
