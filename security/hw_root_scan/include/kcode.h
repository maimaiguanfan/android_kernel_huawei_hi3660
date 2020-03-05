/*
 * kcode.h
 *
 * the kcode.h for kernel code integrity checking
 *
 * Yongzheng Wu <Wu.Yongzheng@huawei.com>
 * likun <quentin.lee@huawei.com>
 * likan <likan82@huawei.com>
 *
 * Copyright (c) 2001-2021, Huawei Tech. Co., Ltd. All rights reserved.
 */

#ifndef _KCODE_H_
#define _KCODE_H_

#include <asm-generic/sections.h>
#include <asm/syscall.h>
#include <linux/crypto.h>
#include <linux/kernel.h>
#include <linux/scatterlist.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/version.h>
#include <crypto/hash.h>
#include <crypto/hash_info.h>
#include "./include/hw_rscan_utils.h"

int kcode_scan(uint8_t *hash);
size_t kcode_get_size(void);
void kcode_copy(char *buffer);
int kcode_syscall_scan(uint8_t *hash);

#endif
