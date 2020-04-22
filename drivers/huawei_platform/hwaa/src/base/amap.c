/*
 * Copyright (C) BicDroid Inc. 2018-2019. All Rights Reserved.
 * Description: Declaration of hashmap data structure and
 *              insert/retrieve/delete/purge functions
 * Author: Xiang Yu, <yuxiang@bicdroid.com>
 *         Lorant Polya, <lpolya@bicdroid.com>
 * Create: 2018-03
 */
#include "inc/base/amap.h"
#include <linux/slab.h>
#include "inc/base/macros.h"

static s32 do_insert_into_amap(struct map_entry_t *new_entry,
	struct map_entry_t **map, u32 map_len, u8 *data,
	u32 data_size, u64 map_key)
{
	s32 map_index = (s32)(map_key % map_len);
	struct map_entry_t *cursor;

	new_entry->data = data;
	new_entry->data_size = data_size;
	new_entry->map_key = map_key;
	new_entry->next = NULL;

	if (map[map_index] == NULL) {
		map[map_index] = new_entry;
	} else {
		cursor = map[map_index];

		while ((cursor->map_key != map_key) &&
			(cursor->next != NULL))
			cursor = cursor->next;

		if (cursor->map_key == map_key)
			return ERR_MSG_APP_MAP_ENTRY_EXIST;
		cursor->next = new_entry;
	}
	return ERR_MSG_SUCCESS;
}

s64 insert_into_amap(struct map_entry_t **map, u32 map_len, u8 *data,
	u32 data_size, u64 map_key)
{
	s64 status_code;
	struct map_entry_t *new_entry;
	if (!map || !data)
		return ERR_MSG_NULL_PTR;
	new_entry = kzalloc(sizeof(struct map_entry_t),
		GFP_KERNEL);

	if (!new_entry)
		return ERR_MSG_OUT_OF_MEMORY;

	status_code = do_insert_into_amap(new_entry, map, map_len, data,
		data_size, map_key);
	if (status_code != ERR_MSG_SUCCESS)
		kzfree(new_entry);

	return status_code;
}

s64 retrieve_from_map(u64 map_key, struct map_entry_t **map, u32 map_len,
	u8 **data, u32 *data_size)
{
	s64 status_code;
	s32 map_index = (s32)(map_key % map_len);
	struct map_entry_t *cursor;

	if (!map || !data || !data_size)
		return ERR_MSG_NULL_PTR;
	cursor = map[map_index];
	status_code = ERR_MSG_APP_MAP_ENTRY_NOTEXIST;

	while (cursor) {
		if (cursor->map_key == map_key) {
			if (data) {
				*data = cursor->data;
				*data_size = cursor->data_size;
				status_code = ERR_MSG_SUCCESS;
			}
			break;
		}
		cursor = cursor->next;
	}

	return status_code;
}

static s64 do_delete_from_map(struct map_entry_t *cursor,
	struct map_entry_t *prev, int map_index,
	struct map_entry_t **map, u32 map_len, u8 **data)
{
	// pass map_len is just to obey the coding rules, no actual use
	// to be standard
	if (map_index >= map_len)
		return ERR_MSG_APP_MAP_ENTRY_NOTEXIST;
	if (cursor) {
		if (cursor == prev)
			map[map_index] = cursor->next;
		else
			prev->next = cursor->next;

		if (data)
			*data = cursor->data;
		else
			kzfree(cursor->data);

		kzfree(cursor);
		return ERR_MSG_SUCCESS;
	}
	return ERR_MSG_APP_MAP_ENTRY_NOTEXIST;
}

s64 delete_from_map(u64 map_key, struct map_entry_t **map, u32 map_len,
	u8 **data)
{
	struct map_entry_t *cursor;
	struct map_entry_t *prev;
	s32 map_index = (s32)(map_key % map_len);

	if (!map || !data)
		return ERR_MSG_NULL_PTR;
	cursor = map[map_index];
	prev = cursor;

	while (cursor) {
		if (cursor->map_key == map_key)
			break;

		prev = cursor;
		cursor = cursor->next;
	}

	return do_delete_from_map(cursor, prev, map_index,
		map, map_len, data);
}

void purge_map(struct map_entry_t **map, u32 map_len)
{
	s32 i;
	struct map_entry_t *map_head;
	struct map_entry_t *map_next;

	if (!map)
		return;
	for (i = 0; i < map_len; ++i) {
		map_head = map[i];
		if (map_head) {
			while (map_head->next) {
				map_next = map_head->next;
				kzfree(map_next->data);
				map_next->data = NULL;
				map_head->next = map_next->next;
				kzfree(map_next);
			}
			kzfree(map_head->data);
			map_head->data = NULL;
			kzfree(map_head);
		}
	}
}
