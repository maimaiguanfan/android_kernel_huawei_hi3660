/*
 * reclaim_acct.c
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

#define pr_fmt(fmt) "reclaimacct: " fmt

#include <chipset_common/reclaim_acct/reclaim_acct.h>

#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/moduleparam.h>
#include <linux/ratelimit.h>
#include <linux/slab.h>
#include <linux/types.h>

#include <log/log_usertype.h>

/*
 * Memory pool of reclaim_acct. Maximum of NR_POOLMEMBER structs can be
 * used at the same time. Once memory pool is used up, the record should
 * be simply abandoned without any runtime error.
 */
struct reclaim_acct {
	u64 start[NR_RA_STUBS];
	u64 delay[NR_RA_STUBS];
	u64 count[NR_RA_STUBS];
};

/* Define a mem pool of NR_POOLMEMBER pointers */
#define NR_POOLMEMBER 32
static struct reclaim_acct *g_mempool[NR_POOLMEMBER];
static int g_mempool_index = NR_POOLMEMBER - 1;
static DEFINE_SPINLOCK(g_mempool_lock);

/* Store reclaim accounting data */
static struct reclaimacct_show {
	u64 delay[NR_DELAY_LV][NR_RA_STUBS];
	u64 count[NR_DELAY_LV][NR_RA_STUBS];
	u64 delay_max;
	u64 delay_max_t;
} *g_reclaimacct_show;
static DEFINE_SPINLOCK(g_reclaimacct_show_lock);

/* Once initialized, the variable should never be changed */
static bool g_reclaimacct_is_off = true;
static int g_reclaimacct_disable = 1;

static void reclaimacct_clear(u64 *start, u64 *delay, u64 *count)
{
	*start = 0;
	*delay = 0;
	*count = 0;
}

/* reclaimacct_alloc MUST be used with reclaimacct_free */
static struct reclaim_acct *reclaimacct_alloc(void)
{
	struct reclaim_acct *elem = NULL;

	spin_lock(&g_mempool_lock);
	if (g_mempool_index >= 0 &&
	    g_mempool_index < NR_POOLMEMBER) {
		elem = g_mempool[g_mempool_index];
		g_mempool_index--;
	} else if (g_mempool_index == -1) {
		pr_warn_ratelimited("mempool is used up\n");
	} else {
		WARN_ONCE(1, "index %d out of range\n", g_mempool_index);
	}
	spin_unlock(&g_mempool_lock);

	return elem;
}

/* reclaimacct_free MUST be used with reclaimacct_alloc */
static void reclaimacct_free(struct reclaim_acct *elem)
{
	spin_lock(&g_mempool_lock);
	if (g_mempool_index >= -1 &&
	    g_mempool_index < NR_POOLMEMBER - 1) {
		g_mempool_index++;
		g_mempool[g_mempool_index] = elem;
	} else {
		WARN_ONCE(1, "index %d out of range\n", g_mempool_index);
	}
	spin_unlock(&g_mempool_lock);
}

/* The caller should make sure start, total, count and func are not NULL */
static void reclaimacct_end(const u64 *start, u64 *delay, u64 *count,
			    const char *func_name, const void *shrinker)
{
	u64 now, ns;

	now = ktime_get_ns();
	if (now < *start)
		return;

	ns = now - *start;
	if (ns < DELAY_LV5) {
		*delay += ns;
		(*count)++;
	}
	if (ns > DELAY_LV4 && ns < DELAY_LV5) {
		if (shrinker)
			pr_warn_ratelimited("shrinker=%pF\n", shrinker);

		if (func_name)
			pr_warn_ratelimited("%s timeout:%lu\n", func_name, ns);
	}
}

void reclaimacct_tsk_init(struct task_struct *tsk)
{
	if (tsk)
		tsk->reclaim_acct = NULL;
}

/* Reinitialize in case parent's non-null pointer was duped */
void reclaimacct_init(void)
{
	reclaimacct_tsk_init(&init_task);
}

void reclaimacct_directreclaim_start(void)
{
	/*
	 * Recursion call of direct reclaim is avoided by PF_MEMALLOC.
	 * So reclaimacct_alloc will not be called recursively.
	 * Check it just for safe, return if NOT NULL.
	 */
	if (!g_reclaimacct_disable && !g_reclaimacct_is_off &&
	    !current->reclaim_acct)
		current->reclaim_acct = reclaimacct_alloc();

	if (!current->reclaim_acct)
		return;

	current->reclaim_acct->start[RA_DIRECTRECLAIM] = ktime_get_ns();
}

static void __reclaimacct_collect_data(int level)
{
	int i;

	if (!current->reclaim_acct || !g_reclaimacct_show)
		return;

	spin_lock(&g_reclaimacct_show_lock);
	for (i = 0; i < NR_RA_STUBS; i++) {
		g_reclaimacct_show->delay[level][i] +=
			current->reclaim_acct->delay[i];
		g_reclaimacct_show->count[level][i] +=
			current->reclaim_acct->count[i];
	}

	if (current->reclaim_acct->delay[RA_DIRECTRECLAIM] >
	    g_reclaimacct_show->delay_max) {
		g_reclaimacct_show->delay_max =
			current->reclaim_acct->delay[RA_DIRECTRECLAIM];
		g_reclaimacct_show->delay_max_t = hisi_getcurtime();
	}
	spin_unlock(&g_reclaimacct_show_lock);
}

static void reclaimacct_collect_data(void)
{
	int i;
	const u64 delay[NR_DELAY_LV] = {
		DELAY_LV0, DELAY_LV1, DELAY_LV2, DELAY_LV3, DELAY_LV4, DELAY_LV5
	};

	for (i = 0; i < NR_DELAY_LV; i++) {
		if (current->reclaim_acct->delay[RA_DIRECTRECLAIM] < delay[i]) {
			__reclaimacct_collect_data(i);
			break;
		}
	}
}

void reclaimacct_directreclaim_end(void)
{
	int i;

	if (!current->reclaim_acct)
		return;

	reclaimacct_end(&(current->reclaim_acct->start[RA_DIRECTRECLAIM]),
			&(current->reclaim_acct->delay[RA_DIRECTRECLAIM]),
			&(current->reclaim_acct->count[RA_DIRECTRECLAIM]),
			NULL, NULL);

	if (current->reclaim_acct->delay[RA_DIRECTRECLAIM] > DELAY_LV4)
		pr_warn_ratelimited(
			"Summary %s=%lu %lu %s=%lu %lu %s=%lu %lu %s=%lu %lu %s=%lu %lu\n",
			DIRECT_RECLAIM_STR,
			current->reclaim_acct->delay[RA_DIRECTRECLAIM],
			current->reclaim_acct->count[RA_DIRECTRECLAIM],
			DRAIN_ALL_PAGES_STR,
			current->reclaim_acct->delay[RA_DRAINALLPAGES],
			current->reclaim_acct->count[RA_DRAINALLPAGES],
			SHRINK_FILE_LIST_STR,
			current->reclaim_acct->delay[RA_SHRINKFILE],
			current->reclaim_acct->count[RA_SHRINKFILE],
			SHRINK_ANON_LIST_STR,
			current->reclaim_acct->delay[RA_SHRINKANON],
			current->reclaim_acct->count[RA_SHRINKANON],
			SHRINK_SLAB_STR,
			current->reclaim_acct->delay[RA_SHRINKSLAB],
			current->reclaim_acct->count[RA_SHRINKSLAB]);

	reclaimacct_collect_data();

	for (i = 0; i < NR_RA_STUBS; i++)
		reclaimacct_clear(&(current->reclaim_acct->start[i]),
				  &(current->reclaim_acct->delay[i]),
				  &(current->reclaim_acct->count[i]));

	reclaimacct_free(current->reclaim_acct);
	current->reclaim_acct = NULL;
}

void reclaimacct_drainallpages_start(void)
{
	if (!current->reclaim_acct)
		return;

	current->reclaim_acct->start[RA_DRAINALLPAGES] = ktime_get_ns();
}

void reclaimacct_drainallpages_end(void)
{
	if (!current->reclaim_acct)
		return;

	reclaimacct_end(&(current->reclaim_acct->start[RA_DRAINALLPAGES]),
			&(current->reclaim_acct->delay[RA_DRAINALLPAGES]),
			&(current->reclaim_acct->count[RA_DRAINALLPAGES]),
			__func__, NULL);
}

void reclaimacct_shrinklist_start(int file)
{
	if (!current->reclaim_acct)
		return;

	if (file)
		current->reclaim_acct->start[RA_SHRINKFILE] = ktime_get_ns();
	else
		current->reclaim_acct->start[RA_SHRINKANON] = ktime_get_ns();
}

void reclaimacct_shrinklist_end(int file)
{
	if (!current->reclaim_acct)
		return;

	if (file) {
		reclaimacct_end(&(current->reclaim_acct->start[RA_SHRINKFILE]),
				&(current->reclaim_acct->delay[RA_SHRINKFILE]),
				&(current->reclaim_acct->count[RA_SHRINKFILE]),
				__func__, NULL);
	} else {
		reclaimacct_end(&(current->reclaim_acct->start[RA_SHRINKANON]),
				&(current->reclaim_acct->delay[RA_SHRINKANON]),
				&(current->reclaim_acct->count[RA_SHRINKANON]),
				__func__, NULL);
	}
}

void reclaimacct_shrinkslab_start(void)
{
	if (!current->reclaim_acct)
		return;

	current->reclaim_acct->start[RA_SHRINKSLAB] = ktime_get_ns();
}

void reclaimacct_shrinkslab_end(const void *shrinker)
{
	if (!current->reclaim_acct)
		return;

	reclaimacct_end(&(current->reclaim_acct->start[RA_SHRINKSLAB]),
			&(current->reclaim_acct->delay[RA_SHRINKSLAB]),
			&(current->reclaim_acct->count[RA_SHRINKSLAB]),
			__func__, shrinker);
}

u64 reclaimacct_get_data(enum ra_show_type type, int level, int stub)
{
	u64 ret;

	if (g_reclaimacct_is_off || !g_reclaimacct_show)
		return 0;

	spin_lock(&g_reclaimacct_show_lock);
	switch (type) {
	case RA_DELAY:
		ret = g_reclaimacct_show->delay[level][stub];
		break;
	case RA_COUNT:
		ret = g_reclaimacct_show->count[level][stub];
		break;
	case RA_DELAY_MAX:
		ret = g_reclaimacct_show->delay_max;
		break;
	case RA_DELAY_MAX_T:
		ret = g_reclaimacct_show->delay_max_t;
		break;
	default: /* impossible */
		ret = 0;
		break;
	}
	spin_unlock(&g_reclaimacct_show_lock);
	return ret;
}

/* Reclaim accounting module initialize */
static int reclaimacct_init_handle(void *p)
{
	int i;
	unsigned int beta_flag;
	int alloc_cnt;

	/* Try 60 times, wait 120s at most */
	for (i = 0; i < 60; i++) {
		beta_flag = get_logusertype_flag();
		/* Non-zero value means it is initialized */
		if (beta_flag != 0)
			break;
		/* Sleep 2 seconds */
		msleep(2000);
	}

	/* Only enabled in beta version */
	if (beta_flag != BETA_USER) {
		pr_err("non-beta user\n");
		goto reclaimacct_disabled;
	}

	/* Init only in non-beta version to save memory */
	g_reclaimacct_show = kzalloc(sizeof(struct reclaimacct_show),
				     GFP_KERNEL);
	if (!g_reclaimacct_show)
		goto alloc_show_failed;

	alloc_cnt = 0; /* For safe */
	for (i = 0; i < NR_POOLMEMBER; i++) {
		g_mempool[i] = kzalloc(sizeof(struct reclaim_acct),
				       GFP_KERNEL);
		if (!g_mempool[i]) {
			alloc_cnt = i;
			goto alloc_acct_failed;
		}
	}

	g_reclaimacct_is_off = false;
	pr_info("enabled\n");
	return 0;

alloc_acct_failed:
	for (i = 0; i < alloc_cnt; i++) {
		kfree(g_mempool[i]);
		g_mempool[i] = NULL;
	}
	kfree(g_reclaimacct_show);
	g_reclaimacct_show = NULL;
alloc_show_failed:
reclaimacct_disabled:
	g_reclaimacct_is_off = true;
	pr_err("disabled\n");
	return 0;
}

static int __init reclaimacct_module_init(void)
{
	struct task_struct *task = NULL;

	task = kthread_run(reclaimacct_init_handle, NULL, "reclaimacct_init");
	if (IS_ERR(task))
		pr_err("run reclaimacct_init failed\n");
	else
		pr_info("run reclaimacct_init successfully\n");
	return 0;
}

late_initcall(reclaimacct_module_init);

module_param_named(reclaimacct_disable, g_reclaimacct_disable, int, 0644);
