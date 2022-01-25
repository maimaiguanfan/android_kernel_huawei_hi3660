/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: This file contains the function required for operations
 *              about list
 * Create: 2020-10-10
 */

#include "inc/base/hwaa_list.h"
#include <linux/slab.h>
#include "inc/base/hwaa_define.h"

static struct list_entry_t *get_list_node(u64 list_key, struct list_head *head)
{
	struct list_entry_t *cursor = NULL;

	list_for_each_entry(cursor, head, next)
		if (cursor->list_key == list_key)
			return cursor;
	return NULL;
}

static s32 do_insert_into_list(struct list_entry_t *new_entry,
	struct list_head *head, u8 *data,
	u32 data_size, u64 list_key)
{
	struct list_entry_t *cursor = NULL;

	new_entry->data = data;
	new_entry->data_size = data_size;
	new_entry->list_key = list_key;

	cursor = get_list_node(list_key, head);
	if (cursor != NULL)
		return ERR_MSG_LIST_NODE_ALREADY_IN;
	list_add(&new_entry->next, head);
	return ERR_MSG_SUCCESS;
}

s64 insert_into_list(struct list_head *head, u8 *data, u32 data_size,
	u64 list_key)
{
	s64 status_code;
	struct list_entry_t *new_entry = NULL;

	if (!head || !data)
		return ERR_MSG_NULL_PTR;
	new_entry = kzalloc(sizeof(struct list_entry_t),
		GFP_KERNEL);

	if (!new_entry)
		return ERR_MSG_OUT_OF_MEMORY;

	status_code = do_insert_into_list(new_entry, head, data,
		data_size, list_key);
	if (status_code != ERR_MSG_SUCCESS)
		kzfree(new_entry);

	return status_code;
}

s64 retrieve_from_list(u64 list_key, struct list_head *head, u8 **data,
	u32 *data_size)
{
	s64 status_code;
	struct list_entry_t *cursor = NULL;

	if (!head || !data || !data_size)
		return ERR_MSG_NULL_PTR;
	if (list_empty(head))
		return ERR_MSG_LIST_EMPTY;
	status_code = ERR_MSG_LIST_NODE_NOT_EXIST;

	cursor = get_list_node(list_key, head);
	if (cursor != NULL) {
		if (cursor->data == NULL)
			return status_code;
		*data = cursor->data;
		*data_size = cursor->data_size;
		status_code = ERR_MSG_SUCCESS;
	}
	return status_code;
}

s64 delete_from_list(u64 list_key, struct list_head *head, u8 **data)
{
	struct list_entry_t *cursor = NULL;
	struct list_entry_t *tmp = NULL;

	if (!head || !data)
		return ERR_MSG_NULL_PTR;
	if (!list_empty(head))
		list_for_each_entry_safe(cursor, tmp, head, next)
			if (cursor->list_key == list_key) {
				*data = cursor->data;
				list_del(&cursor->next);
				kzfree(cursor);
				return ERR_MSG_SUCCESS;
			}
	return ERR_MSG_LIST_NODE_NOT_EXIST;
}

void purge_list(struct list_head *head)
{
	struct list_entry_t *cursor = NULL;
	struct list_entry_t *tmp = NULL;

	if (!head)
		return;
	if (!list_empty(head))
		list_for_each_entry_safe(cursor, tmp, head, next) {
			kzfree(cursor->data);
			list_del(&cursor->next);
			kzfree(cursor);
		}
}
