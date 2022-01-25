/*
 * reclaim_acct.h
 *
 * Memory reclaim delay accounting
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _RECLAIM_ACCT_H
#define _RECLAIM_ACCT_H

#include <linux/sched.h>

/* RA is the abbreviation of reclaim accouting */
enum reclaimacct_stubs {
	RA_DIRECTRECLAIM = 0,
	RA_DRAINALLPAGES,
	RA_SHRINKFILE,
	RA_SHRINKANON,
	RA_SHRINKSLAB,
};
#define NR_RA_STUBS (RA_SHRINKSLAB + 1)

#define DIRECT_RECLAIM_STR "direct_reclaim"
#define DRAIN_ALL_PAGES_STR "drain_all_pages"
#define SHRINK_FILE_LIST_STR "shrink_file_list"
#define SHRINK_ANON_LIST_STR "shrink_anon_list"
#define SHRINK_SLAB_STR "shrink_slab"

#define DELAY_LV0 5000000 /* 5ms */
#define DELAY_LV1 10000000 /* 10ms */
#define DELAY_LV2 50000000 /* 50ms */
#define DELAY_LV3 100000000 /* 100ms */
#define DELAY_LV4 2000000000 /* 2000ms */
#define DELAY_LV5 50000000000 /* 50000ms */
#define NR_DELAY_LV 6

void reclaimacct_tsk_init(struct task_struct *tsk);
void reclaimacct_init(void);
void reclaimacct_directreclaim_start(void);
void reclaimacct_directreclaim_end(void);
void reclaimacct_directcompact_start(void);
void reclaimacct_directcompact_end(void);
void reclaimacct_drainallpages_start(void);
void reclaimacct_drainallpages_end(void);
void reclaimacct_shrinklist_start(int file);
void reclaimacct_shrinklist_end(int file);
void reclaimacct_shrinkslab_start(void);
void reclaimacct_shrinkslab_end(const void *shrinker);

enum ra_show_type {
	RA_DELAY,
	RA_COUNT,
	RA_DELAY_MAX,
	RA_DELAY_MAX_T,
};

/*
 * When type is RA_DELAY or RA_COUNT, the caller should make sure
 * 0 <= level < NR_DELAY_LV and 0 <= stub < NR_RA_STUBS.
 */
u64 reclaimacct_get_data(enum ra_show_type type, int level, int stub);

#endif /* _RECLAIM_ACCT_H */
