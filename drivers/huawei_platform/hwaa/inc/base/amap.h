/*
 * Copyright (C) BicDroid Inc. 2018-2019. All Rights Reserved.
 * Description: Declaration of hashmap data structure and
 *              insert/retrieve/delete/purge functions
 * Author: Xiang Yu, <yuxiang@bicdroid.com>
 *         Lorant Polya, <lpolya@bicdroid.com>
 * Create: 2018-03
 */
#ifndef _HWAA_AMAP_H
#define _HWAA_AMAP_H

#include <linux/types.h>
/* number of entry lists in hashmap */
#define NUMMAPENTRY 1024

/* hashmap entry definition */
struct map_entry_t {
	void *data;
	u32 data_size;
	u64 map_key;
	struct map_entry_t *next;
};

/*
 * This function insert a key/value pair to a hashmap
 * @param stpp_map    [in] hashmap
 * @param vp_data     [in] value to be inserted
 * @param ll_map_key  [in] key to be inserted
 * @return 0 if successful
 */
s64 insert_into_amap(struct map_entry_t **map, u32 map_len, u8 *data,
	u32 data_size, u64 map_key);

/*
 * This function retrieves a value by a key from a hashmap
 * @param stpp_map    [in] hashmap
 * @param vpp_data    [inout] pointer to the destination that holds the value
 *                            to be retrieved
 * @param ll_map_key  [in] key to be retrieved
 * @return 0 if successful
 */
s64 retrieve_from_map(u64 map_key, struct map_entry_t **map, u32 map_len,
	u8 **data, u32 *data_size);

/*
 * This function removes a value by a key from a hashmap
 * @param stpp_map    [in] hashmap
 * @param vpp_data    [inout] pointer to the destination that holds the value
 *                            to be deleted
 * @param ll_map_key  [in] key to be deleted
 * @return 0 if successful
 */
s64 delete_from_map(u64 map_key, struct map_entry_t **map, u32 map_len,
	u8 **data);

/*
 * This function purges the whole hashmap
 * @param stpp_map      [in] hashmap
 */
void purge_map(struct map_entry_t **map, u32 map_len);
#endif
