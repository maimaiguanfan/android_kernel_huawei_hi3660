/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: This file contains the function required for operations
 *              about list
 * Create: 2020-10-10
 */

#ifndef _HWAA_HWAA_LIST_H
#define _HWAA_HWAA_LIST_H

#include <linux/types.h>
#include <linux/list.h>

struct list_entry_t {
	void *data;
	u32 data_size;
	u64 list_key;
	struct list_head next;
};

s64 insert_into_list(struct list_head *head, u8 *data, u32 data_size,
	u64 list_key);

s64 retrieve_from_list(u64 list_key, struct list_head *head, u8 **data,
	u32 *data_size);

s64 delete_from_list(u64 list_key, struct list_head *head, u8 **data);

/* This function free and delete all list node */
void purge_list(struct list_head *head);
#endif
