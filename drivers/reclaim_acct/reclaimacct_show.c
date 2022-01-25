/*
 * reclaimacct_show.c
 *
 * Show memory reclaim delay accounting data
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

#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#include <chipset_common/reclaim_acct/reclaim_acct.h>

static int reclaimacct_proc_show(struct seq_file *m, void *v)
{
	const char *stub_name[NR_RA_STUBS] = {
		DIRECT_RECLAIM_STR,
		DRAIN_ALL_PAGES_STR,
		SHRINK_FILE_LIST_STR,
		SHRINK_ANON_LIST_STR,
		SHRINK_SLAB_STR
	};
	int i, j;
	u64 delay;
	u64 count;
	u64 delay_max;
	u64 delay_max_t;
	const u64 ns_to_ms = 1000000;

	seq_puts(m, "watch_point(unit:ms/-)\t\t0-5ms\t\t5-10ms\t\t");
	seq_puts(m, "10-50ms\t\t50-100ms\t100-2000ms\t2000-50000ms\n");
	for (i = 0; i < NR_RA_STUBS; i++) {
		seq_printf(m, "%s_delay\t\t", stub_name[i]);
		for (j = 0; j < NR_DELAY_LV; j++) {
			delay = reclaimacct_get_data(RA_DELAY, j, i) / ns_to_ms;
			seq_printf(m, "%-15lu ", delay);
		}
		seq_puts(m, "\n");

		seq_printf(m, "%s_count\t\t", stub_name[i]);
		for (j = 0; j < NR_DELAY_LV; j++) {
			count = reclaimacct_get_data(RA_COUNT, j, i);
			seq_printf(m, "%-15lu ", count);
		}
		seq_puts(m, "\n");
	}
	delay_max = reclaimacct_get_data(RA_DELAY_MAX, 0, 0);
	delay_max_t = reclaimacct_get_data(RA_DELAY_MAX_T, 0, 0);
	seq_printf(m, "Max delay:%lu Happened:%lu\n", delay_max, delay_max_t);
	return 0;
}

static int reclaimacct_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, reclaimacct_proc_show, NULL);
}

static const struct file_operations reclaimacct_proc_fops = {
	.open = reclaimacct_proc_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static int __init proc_reclaimacct_init(void)
{
	proc_create("reclaimacct", 0440, NULL, &reclaimacct_proc_fops);
	return 0;
}
fs_initcall(proc_reclaimacct_init);

