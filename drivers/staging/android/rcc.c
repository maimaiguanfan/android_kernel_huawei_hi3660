

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/semaphore.h>
#include <linux/rwsem.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/sysfs.h>
#include <linux/kobject.h>
#include <linux/cpu.h>
#include <linux/jiffies.h>
#include <linux/mm.h>
#include <linux/swap.h>
#include <linux/vmstat.h>
#include <linux/kernel_stat.h>
#include <linux/tick.h>
#include <linux/version.h>
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 10, 0))
#include <linux/cputime.h>
#endif

#include "rcc.h"

/* Globals */

static struct rcc_module rcc_module;

static inline unsigned long elapsed_jiffies(unsigned long start)
{
	unsigned long end = jiffies;

	if (end >= start)
		return (unsigned long)(end - start);

	return (unsigned long)(end + (MAX_JIFFY_OFFSET - start) + 1);
}

static bool is_swap_full(int max_percent)
{
	struct sysinfo si;
	si_swapinfo(&si);
	if (si.totalswap == 0)
		return true;
	/* free/total > max_percent% */
	if ((si.totalswap - si.freeswap) * 100 >= si.totalswap * max_percent)
		return true;
	return false;
}

static bool is_memory_free_enough(int free_pages_min)
{
	unsigned long nr_free_pages;
#if(LINUX_VERSION_CODE >= KERNEL_VERSION(4,14,0))
	nr_free_pages = global_zone_page_state(NR_FREE_PAGES);
#else
	nr_free_pages = global_page_state(NR_FREE_PAGES);
#endif
    if (nr_free_pages > free_pages_min)
		return true;
	return false;
}

static bool is_anon_page_enough(int anon_pages_min)
{
	unsigned long nr_pages;

#if(LINUX_VERSION_CODE >= KERNEL_VERSION(4,9,0))
	nr_pages = global_node_page_state(NR_INACTIVE_ANON);
	nr_pages += global_node_page_state(NR_ACTIVE_ANON);
#else
	nr_pages = global_page_state(NR_INACTIVE_ANON);
	nr_pages += global_page_state(NR_ACTIVE_ANON);
#endif

	if (nr_pages > anon_pages_min)
		return true;

	return false;
}

static bool is_avail_above_target(int avail_target)
{
	long available;
	available = si_mem_available();
	if (available > avail_target)
		return true;

	return false;
}

static void rcc_set_full_clean(struct rcc_module *rcc, int set)
{
	rcc->full_clean_flag = !!set;
	pr_info("rcc: full_clean_flag = %d\n", rcc->full_clean_flag);
}

static inline bool is_display_off(struct rcc_module *rcc)
{
	return rcc->display_off;
}

static void rcc_update_display_stat(struct rcc_module *rcc, bool display_on)
{
	rcc->display_off = !display_on;
	pr_info("rcc: display_off = %d\n", rcc->display_off);
}

/**
 * purpose: check cpu is really in idle status
 * arguments:
 *    rcc: struct rcc_module. we use rcc_idle_* values here.
 * return:
 *    none.
 */
static inline bool is_cpu_idle(struct rcc_module *rcc)
{
	int threshold = rcc->idle_threshold;

	/* when display off, we try do more work. */
	if (rcc->display_off)
		threshold += 5; /* cpu threshold increase 5 percent */

	if (rcc->force_compress_flag)
		threshold = 100; /* cpu threshold set to 100 percent */

	if (rcc->cpu_load[0] <= threshold
	    && rcc->cpu_load[1] <= threshold
	    && rcc->cpu_load[2] <= threshold)
		return true;

	return false;
}

#ifdef arch_idle_time

static u64 get_idle_time(int cpu)
{
	u64 idle;

	idle = kcpustat_cpu(cpu).cpustat[CPUTIME_IDLE];
	if (cpu_online(cpu) && !nr_iowait_cpu(cpu))
		idle += arch_idle_time(cpu);
	return idle;
}

#else

static u64 get_idle_time(int cpu)
{
	u64 idle, idle_time = -1ULL;

	if (cpu_online(cpu))
		idle_time = get_cpu_idle_time_us(cpu, NULL);

	if (idle_time == -1ULL)
		/* !NO_HZ or cpu offline so we can rely on cpustat.idle */
		idle = kcpustat_cpu(cpu).cpustat[CPUTIME_IDLE];
	else
#if(LINUX_VERSION_CODE >= KERNEL_VERSION(4,14,0))
		idle = idle_time * NSEC_PER_USEC;
#else
		idle = usecs_to_cputime64(idle_time);
#endif
	return idle;
}

#endif

/**
 *  purpose: get average cpuload from last stat
 * return:
 *    cpu load in percent.
 */
static int _get_cpu_load(clock_t *last_cpu_stat)
{
	int i;
	clock_t delta[INDEX_CPU_MAX], tmp;

	u64 user, nice, system, idle;
	u64 total_time, busy_time;

	user = nice = system = idle = 0;

	for_each_possible_cpu(i) {
		user += kcpustat_cpu(i).cpustat[CPUTIME_USER];
		nice += kcpustat_cpu(i).cpustat[CPUTIME_NICE];
		system += kcpustat_cpu(i).cpustat[CPUTIME_SYSTEM];
		idle += get_idle_time(i);
	}

	total_time = user + nice + system + idle;
	busy_time = user + nice + system;

#if(LINUX_VERSION_CODE >= KERNEL_VERSION(4,14,0))
	tmp = nsec_to_clock_t(busy_time);
#else
	tmp = cputime64_to_clock_t(busy_time);
#endif
	delta[INDEX_CPU_BUSY] = tmp - last_cpu_stat[INDEX_CPU_BUSY];
	last_cpu_stat[INDEX_CPU_BUSY] = tmp;

#if(LINUX_VERSION_CODE >= KERNEL_VERSION(4,14,0))
	tmp = nsec_to_clock_t(total_time);
#else
	tmp = cputime64_to_clock_t(total_time);
#endif
	delta[INDEX_CPU_TOTAL] = tmp - last_cpu_stat[INDEX_CPU_TOTAL];
	last_cpu_stat[INDEX_CPU_TOTAL] = tmp;

	if (delta[INDEX_CPU_TOTAL] == 0)
		return -1;

	return 100 * delta[INDEX_CPU_BUSY] / delta[INDEX_CPU_TOTAL];
}

/**
 *  purpose: wrapper of _get_cpu_load()
 * return:
 *    cpu load in percent.
 */
static int get_cpu_load(struct rcc_module *rcc, bool need_delay)
{
	int cpu_load;
	/* delay some time for calc recent cpu load. */
	if (need_delay) {
		_get_cpu_load(rcc->last_cpu_stat);
		msleep(25);
	}
	cpu_load = _get_cpu_load(rcc->last_cpu_stat);
	if (cpu_load >= 0) {
		rcc->cpu_load[2] = rcc->cpu_load[1];
		rcc->cpu_load[1] = rcc->cpu_load[0];
		rcc->cpu_load[0] = cpu_load;
	}
	return cpu_load;
}

/**
 * purpose: swap out memory.
 * return count of reclaimed pages.
 */
static int rcc_swap_out(int nr_pages, int scan_mode)
{
	int unit_pages, total, real = 0;
	for (total = 0; total < nr_pages; total += 32) {
		unit_pages =
		    ((total + 32) > nr_pages) ? (nr_pages - total) : 32;
		real += try_to_free_pages_ex(unit_pages, scan_mode);
		cond_resched();
	}

#ifdef CONFIG_HUAWEI_RCC_DEBUG
	pr_info("scan mode: %d swap:  %d/%d pages\n", scan_mode, real,
		nr_pages);
#endif
	/* return real reclaimed page count */
	return real;
}

/* purpose: check is background thread running */
static inline bool rcc_is_enabled(struct rcc_module *rcc)
{
	return !!rcc->task;
}

/**
 * purpose: check system is ready to wakeup compress thread.
 *    now we check: enable & idle & memory_full & swap_full.
 * arguments:
 *    rcc: struct rcc_module.
 *    end: start condition checker or end condition checker.
 * return:
 *    thread exit code.
 */
static int get_system_stat(struct rcc_module *rcc, bool end)
{
	int ret = 0, value;
	if (!is_cpu_idle(rcc))
		ret |= WF_CPU_BUSY;

	value = rcc->free_pages_min + (end ? RCC_FREE_PAGE_MIN_EX : 0);
	if (is_memory_free_enough(value))
		ret |= WF_MEM_FREE_ENOUGH;

	if (!rcc->full_clean_flag)
		value = end ? rcc->anon_pages_min : rcc->anon_pages_max;
	else
		value = end ? RCC_ANON_PAGE_MIN_ON_BOOT : RCC_ANON_PAGE_MAX_ON_BOOT;

	if (!is_anon_page_enough(value))
		ret |= WF_NO_ANON_PAGE;

	value = rcc->swap_percent_low + (end ? RCC_SWAP_PERCENT_LOW_EX : 0);
	if (is_swap_full(value))
		ret |= WF_SWAP_FULL;

	value = rcc->avail_target_pages;
	if (is_avail_above_target(value))
		ret |= WF_AVAIL_ENOUGH;

	return ret ? ret : WS_NEED_WAKEUP;
}

/* purpose: wakeup background thread */
static int rcc_thread_wakeup(struct rcc_module *rcc)
{
	/* up semaphore of background thread. */
	up(&rcc->wait);
	return 0;
}

/**
 * purpose: waiting wakeup event in background thread.
 * arguments:
 *    rcc: struct rcc_module.
 *    timeout: timeout to wait. unit is ms. <0 for infinite wait.
 * return:
 *    -ETIME: timeout to wait.
 *         0: received events
 */
static int rcc_thread_wait(struct rcc_module *rcc, long timeout)
{
	/* down semaphore of background thread. */
	if (timeout > 0)
		return down_timeout(&rcc->wait, msecs_to_jiffies(timeout));
	down(&rcc->wait);
	return 0;
}

#define DO_SWAP_OUT(mode) do { \
		if (ret & WF_CPU_BUSY) { \
			nr_pages = 0; \
			busy_count++; \
		} else if (ret & WF_NO_ANON_PAGE) { \
			nr_pages = 0; \
			anon_count++; \
		} else { \
			one_loop_time = jiffies; \
			nr_pages = rcc_swap_out(RCC_NR_SWAP_UNIT_SIZE, mode); \
			one_loop_time = elapsed_jiffies(one_loop_time); \
			one_compress_time += one_loop_time; \
		} \
	} while (0)

#define _UPDATE_STATE() do { \
		nr_total_pages += nr_pages; \
		if (!rcc->force_compress_flag || rcc->full_clean_flag) { \
			msleep(RCC_IDLE_FAST); \
			get_cpu_load(rcc, false); \
		} \
		if (kthread_should_stop()) \
			goto out;  \
		ret = get_system_stat(rcc, true); \
	} while (0)

/**
 * purpose: main working thread for compress RAM
 * arguments:
 *    unused: unused.
 * return:
 *    thread exit code.
 */
static int rcc_thread(void *unused)
{
	int ret, nr_pages, nr_total_pages, busy_count = 0, anon_count = 0;
	unsigned long one_loop_time;
	unsigned long one_compress_time;
	struct task_struct *tsk = current;
	struct rcc_module *rcc = &rcc_module;
	long timeout;

	/* need swap out, PF_FREEZER_SKIP is protection from hung_task. */
	tsk->flags |= PF_MEMALLOC | PF_SWAPWRITE | PF_KSWAPD | PF_FREEZER_SKIP;

	while (true) {
		timeout = RCC_IDLE_SLOW;
		if (rcc->passive_mode)
			timeout = RCC_WAIT_INFINITE;
		/* wait wakeup_event or timeout. */
		ret = rcc_thread_wait(rcc, timeout);
		if (kthread_should_stop())
			goto out;
		/* force update cpu load stat. */
		get_cpu_load(rcc, true);

		nr_total_pages = 0;
		one_compress_time = 0;
		ret = get_system_stat(rcc, false);
		if (rcc->pause_flag) {
			pr_info("rcc pause\n");
			rcc_thread_wait(rcc, RCC_PAUSE_TIME);
			rcc->pause_flag = 0;
			pr_info("rcc continue\n");
		}
		pr_info("rcc try to run ,stat %d\n", ret);
		if (rcc->full_clean_flag) {
			ssleep(RCC_SLEEP_TIME);
			rcc->wakeup_count++;
			pr_info("rcc wakeup: full\n");

			/* full fill swap area. */
			busy_count = 0;
			anon_count = 0;
			do {
				DO_SWAP_OUT(RCC_MODE_ANON);
				_UPDATE_STATE();
			} while (!(ret & WF_SWAP_FULL)
				 && !(anon_count > RCC_MAX_WAIT_COUNT)
				 && nr_total_pages < rcc->full_clean_anon_pages);

			rcc_set_full_clean(rcc, 0);
			rcc->nr_full_clean_pages += nr_total_pages;
			rcc->total_spent_times += one_compress_time;
			pr_info("full cc: mem=%d MB, time=%d ms, out stat=%d, anon_count = %d\n",
				M(nr_total_pages), jiffies_to_msecs(one_compress_time),
				ret, anon_count);
		} else if (ret == WS_NEED_WAKEUP || rcc->force_compress_flag) {
			rcc->wakeup_count++;
			if (rcc->force_compress_flag)
				pr_info("rcc wakeup: force compress\n");
			else
				pr_info("rcc wakeup: normal\n");

			/* swap out pages. */
			busy_count = 0;
			do {
				DO_SWAP_OUT(RCC_MODE_ANON);
				_UPDATE_STATE();
				if (rcc->full_clean_flag || busy_count > 1000)
					break;
				if (rcc->force_compress_flag
					 && nr_total_pages > rcc->force_once_pages)
					break;
				if (rcc->pause_flag) {
					pr_info("rcc pause\n");
					rcc_thread_wait(rcc,RCC_PAUSE_TIME);
					pr_info("rcc continue\n");
					rcc->pause_flag = 0;
				}
			} while (!(ret & WF_MEM_FREE_ENOUGH)
				 && !(ret & WF_SWAP_FULL)
				 && !(ret & WF_NO_ANON_PAGE)
				 && !(ret & WF_AVAIL_ENOUGH));

			rcc->nr_normal_clean_pages += nr_total_pages;
			rcc->total_spent_times += one_compress_time;

			if (rcc->force_compress_flag) {
				pr_info("force cc: mem=%d MB, time=%d, out_stat=%d\n",
					M(nr_total_pages), jiffies_to_msecs(one_compress_time), ret);
				rcc->force_compress_flag = 0;
			} else {
				pr_info("normal cc: mem=%d MB, time=%d, out_stat=%d, busy=%d\n",
					M(nr_total_pages), jiffies_to_msecs(one_compress_time),
					ret, busy_count);
			}
		}
	}
out:
	tsk->flags &=
	    ~(PF_MEMALLOC | PF_SWAPWRITE | PF_KSWAPD | PF_FREEZER_SKIP);
	pr_info("rcc_thread: exit.\n");
	return 0;
}

/**
 * purpose: initialize this module
 * arguments:
 *    rcc: module handler need to be initialized.
 * return:
 *    none.
 */
static void rcc_setup(struct rcc_module *rcc)
{
	/* memset(rcc,0,sizeof(struct rcc_module)); */
	init_rwsem(&rcc->lock);
	sema_init(&rcc->wait, 0);

	rcc->passive_mode = 0;
	rcc->idle_threshold = RCC_IDLE_THRESHOLD;
	rcc->swap_percent_low = RCC_SWAP_PERCENT_LOW;
	rcc->free_pages_min = RCC_FREE_PAGE_MIN;
	rcc->full_clean_file_pages = RCC_FULL_CLEAN_FILE_PAGE;

	if (totalram_pages > 3 * TOTAL_RAM_PAGES_1G) /* >3g */
		rcc->anon_pages_min = RCC_ANON_PAGE_MIN + 2 * RCC_ANON_PAGE_RAM_GAP;
	else if (totalram_pages > 2 * TOTAL_RAM_PAGES_1G) /* 3g */
		rcc->anon_pages_min = RCC_ANON_PAGE_MIN + RCC_ANON_PAGE_RAM_GAP;
	else /*1g 2g*/
		rcc->anon_pages_min = RCC_ANON_PAGE_MIN;

	rcc->anon_pages_max = rcc->anon_pages_min + RCC_ANON_PAGE_START_GAP;
	rcc->full_clean_anon_pages = RCC_MAX_RECLAIM_ON_BOOT;
	rcc->avail_target_pages = TOTAL_RAM_PAGES_1G;

	rcc->cpu_load[0] = 100;	/* init cpu load as 100% */
	rcc->force_once_pages = RCC_ANON_PAGE_FORCE_ONCE;
}

/* purpose: start background thread */
static int rcc_thread_start(struct rcc_module *rcc)
{
	if (rcc->task)
		return -EFAIL;

	rcc_set_full_clean(rcc, 0);

	rcc->task = kthread_run(rcc_thread, NULL, "rcc");
	if (IS_ERR(rcc->task)) {
		pr_err("rcc: failed to start thread\n");
		return -EFAIL;
	}

	pr_info("rcc: thread started.\n");
	return 0;
}

/* purpose: stop background thread */
static void rcc_thread_stop(struct rcc_module *rcc)
{
	if (!rcc->task)
		return;
	rcc_thread_wakeup(rcc);	/* need wakeup thread first. */
	kthread_stop(rcc->task);
	rcc->task = NULL;
	pr_info("rcc: thread stopped.\n");
}

#define RCC_MODE_RO 0440
#define RCC_MODE_RW 0660

#define RCC_ATTR(_name, _mode, _show, _store) \
	struct kobj_attribute kobj_attr_##_name \
		= __ATTR(_name, _mode, _show, _store)

/* purpose: attr status: is background thread running */
static ssize_t enable_show(struct kobject *kobj, struct kobj_attribute *attr,
			   char *buf)
{
	struct rcc_module *rcc = &rcc_module;
	return scnprintf(buf, PAGE_SIZE, "%u\n", rcc_is_enabled(rcc));
}

/* purpose: attr set: background thread start/stop */
static ssize_t enable_store(struct kobject *kobj, struct kobj_attribute *attr,
			    const char *buf, size_t len)
{
	int ret;
	u16 enable;
	struct rcc_module *rcc = &rcc_module;
	ret = kstrtou16(buf, 10, &enable);
	if (ret)
		return ret;
	if (enable)
		rcc_thread_start(rcc);
	else
		rcc_thread_stop(rcc);
	return len;
}

static ssize_t event_show(struct kobject *kobj, struct kobj_attribute *attr,
			  char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "\n");
}

static ssize_t event_store(struct kobject *kobj, struct kobj_attribute *attr,
			   const char *buf, size_t len)
{
	struct rcc_module *rcc = &rcc_module;
	if (!strncmp(buf, "DISPLAY_OFF", strlen("DISPLAY_OFF"))) {
		rcc_update_display_stat(rcc, 0);
	} else if (!strncmp(buf, "DISPLAY_ON", strlen("DISPLAY_ON"))) {
		rcc_update_display_stat(rcc, 1);
	} else if (!strncmp(buf, "BOOT_COMPLETE", strlen("BOOT_COMPLETE"))) {
		rcc_set_full_clean(rcc, 1);
		rcc_thread_wakeup(rcc);
	} else if (!strncmp(buf, "PASSIVE_MODE", strlen("PASSIVE_MODE"))) {
		rcc->passive_mode = 1;
	} else if (!strncmp(buf, "POSITIVE_MODE", strlen("POSITIVE_MODE"))) {
		rcc->passive_mode = 0;
		rcc_thread_wakeup(rcc);
	} else if (!strncmp(buf, "NORMAL_COMPRESS", strlen("NORMAL_COMPRESS"))) {
		rcc_thread_wakeup(rcc);
	} else if (!strncmp(buf, "PAUSE_COMPRESS", strlen("PAUSE_COMPRESS"))) {
		rcc->pause_flag = 1;
	} else {
		pr_err("rcc: unknown event: [%s] size=%zu\n",
			buf, strlen(buf));
	}
	return len;
}

/* purpose: attr status:  */
static ssize_t idle_threshold_show(struct kobject *kobj,
				   struct kobj_attribute *attr, char *buf)
{
	struct rcc_module *rcc = &rcc_module;
	return scnprintf(buf, PAGE_SIZE, "%d%%\n", rcc->idle_threshold);
}

/* purpose: attr set:  */
static ssize_t idle_threshold_store(struct kobject *kobj,
				    struct kobj_attribute *attr,
				    const char *buf, size_t len)
{
	int ret;
	u16 value;
	struct rcc_module *rcc = &rcc_module;
	ret = kstrtou16(buf, 10, &value);
	if (ret)
		return ret;
	if (value > 100)
		return -EINVAL;
	rcc->idle_threshold = value;
	return len;
}

/* purpose: attr status:  */
static ssize_t swap_percent_low_show(struct kobject *kobj,
				    struct kobj_attribute *attr, char *buf)
{
	struct rcc_module *rcc = &rcc_module;
	return scnprintf(buf, PAGE_SIZE, "%d%%, extra: +%d\n",
		       rcc->swap_percent_low, RCC_SWAP_PERCENT_LOW_EX);
}

/* purpose: attr set:  */
static ssize_t swap_percent_low_store(struct kobject *kobj,
				     struct kobj_attribute *attr,
				     const char *buf, size_t len)
{
	int ret;
	u16 value;
	struct rcc_module *rcc = &rcc_module;

	ret = kstrtou16(buf, 10, &value);
	if (ret)
		return ret;
	if (value > 100)
		return -EINVAL;
	if (value > 100 - RCC_SWAP_PERCENT_LOW_EX)
		return value;
	rcc->swap_percent_low = value;
	return len;
}

/* purpose: attr status:  */
static ssize_t free_size_min_show(struct kobject *kobj,
				  struct kobj_attribute *attr, char *buf)
{
	struct rcc_module *rcc = &rcc_module;

	return scnprintf(buf, PAGE_SIZE, "%d MB , extra: +%d MB\n",
			M(rcc->free_pages_min),
			M(RCC_FREE_PAGE_MIN_EX));
}

/* purpose: attr set:  */
static ssize_t free_size_min_store(struct kobject *kobj,
			struct kobj_attribute *attr, const char *buf,
			size_t len)
{
	u64 size;
	struct rcc_module *rcc = &rcc_module;
	size = memparse(buf, NULL);

	if (!size)
		return -EINVAL;
	rcc->free_pages_min = (size >> PAGE_SHIFT);
	return len;
}

/* purpose: attr status:  */
static ssize_t full_clean_size_show(struct kobject *kobj,
			struct kobj_attribute *attr, char *buf)
{
	struct rcc_module *rcc = &rcc_module;
	int size = M(rcc->full_clean_file_pages);

	return scnprintf(buf, PAGE_SIZE, "%d MB\n", size);
}

/* purpose: attr set:  */
static ssize_t full_clean_size_store(struct kobject *kobj,
			struct kobj_attribute *attr,
			const char *buf, size_t len)
{
	u64 size;
	unsigned long nr_file_pages;
	struct rcc_module *rcc = &rcc_module;

	size = memparse(buf, NULL);
	if (!size)
		return -EINVAL;
#if(LINUX_VERSION_CODE >= KERNEL_VERSION(4,14,0))
	nr_file_pages = global_zone_page_state(NR_INACTIVE_FILE);
	nr_file_pages += global_zone_page_state(NR_ACTIVE_FILE);
#else
	nr_file_pages = global_page_state(NR_INACTIVE_FILE);
	nr_file_pages += global_page_state(NR_ACTIVE_FILE);
#endif
	size = size >> PAGE_SHIFT;

	/* size should not larger than file cache pages. */
	if (size > nr_file_pages)
		size = nr_file_pages;

	rcc->full_clean_file_pages = size;
	return len;
}

/* purpose: attr status:  */
static ssize_t rcc_stat_show(struct kobject *kobj,
			struct kobj_attribute *attr, char *buf)
{
	struct rcc_module *rcc = &rcc_module;

	return scnprintf(buf, PAGE_SIZE,
		"clean pages: full=%d MB, normal=%d MB\n"
		" boot anon : min=%d MB, max=%d MB\n"
		" wake count: %d, time=%d\n"
		"       mode: %s",
		M(rcc->nr_full_clean_pages), M(rcc->nr_normal_clean_pages),
		M(RCC_ANON_PAGE_MIN_ON_BOOT), M(RCC_ANON_PAGE_MAX_ON_BOOT),
		rcc->wakeup_count, jiffies_to_msecs(rcc->total_spent_times),
		rcc->passive_mode ? "passive" : "positive");
}

static ssize_t can_compress_show(struct kobject *kobj,
			struct kobj_attribute *attr, char *buf)
{
	struct rcc_module *rcc = &rcc_module;
	int value = rcc->anon_pages_max;

	if (!is_anon_page_enough(value) || !rcc->task || rcc->passive_mode)
		return scnprintf(buf, PAGE_SIZE, "0\n");

	value = rcc->swap_percent_low;
	if (is_swap_full(value))
		return scnprintf(buf, PAGE_SIZE, "0\n");

	value = rcc->avail_target_pages;
	if (is_avail_above_target(value))
		return scnprintf(buf, PAGE_SIZE, "0\n");

	return scnprintf(buf, PAGE_SIZE, "1\n");
}

/* purpose: show the size of max anon page to reclaim  */
static ssize_t max_anon_clean_size_show(struct kobject *kobj,
			struct kobj_attribute *attr, char *buf)
{
	struct rcc_module *rcc = &rcc_module;
	int size = M(rcc->full_clean_anon_pages);

	return scnprintf(buf, PAGE_SIZE, "%d MB\n", size);
}

/* purpose: set max anon page size to reclaim */
static ssize_t max_anon_clean_size_store(struct kobject *kobj,
			struct kobj_attribute *attr,
			const char *buf, size_t len)
{
	u64 size;
	struct rcc_module *rcc = &rcc_module;

	size = memparse(buf, NULL);
	if (!size)
		return -EINVAL;

	size = size >> PAGE_SHIFT;
	rcc->full_clean_anon_pages = size;
	return len;
}

/* purpose: target avail mem to compress  */
static ssize_t avail_target_show(struct kobject *kobj,
			struct kobj_attribute *attr, char *buf)
{
	struct rcc_module *rcc = &rcc_module;
	int size = M(rcc->avail_target_pages);

	return scnprintf(buf, PAGE_SIZE, "%d MB\n", size);
}

static ssize_t avail_target_store(struct kobject *kobj,
			struct kobj_attribute *attr,
			const char *buf, size_t len)
{
	u64 size;
	struct rcc_module *rcc = &rcc_module;
	/* read as mb */
	size = memparse(buf, NULL);
	if (!size)
		return -EINVAL;

	size = size << (20 - PAGE_SHIFT); /* mb to page */
	if (size > totalram_pages * 4)
		return -EINVAL;
	rcc->avail_target_pages = size;
	return len;
}

static ssize_t anon_target_show(struct kobject *kobj,
			struct kobj_attribute *attr, char *buf)
{
	struct rcc_module *rcc = &rcc_module;

	return scnprintf(buf, PAGE_SIZE, "%d MB ,extra:%d MB\n",
			M(rcc->anon_pages_min), M(rcc->anon_pages_max));
}

static ssize_t anon_target_store(struct kobject *kobj,
			struct kobj_attribute *attr,
			const char *buf, size_t len)
{
	u64 size;
	struct rcc_module *rcc = &rcc_module;
	/* read as mb */
	size = memparse(buf, NULL);
	if (!size)
		return -EINVAL;

	size = size << (20 - PAGE_SHIFT); /* mb to page */
	if (size > totalram_pages * 4)
		return -EINVAL;
	rcc->anon_pages_min = size;
	rcc->anon_pages_max = rcc->anon_pages_min + RCC_ANON_PAGE_START_GAP;
	return len;
}

static ssize_t force_once_show(struct kobject *kobj,
			struct kobj_attribute *attr, char *buf)
{
	struct rcc_module *rcc = &rcc_module;
	int size = M(rcc->force_once_pages);

	return scnprintf(buf, PAGE_SIZE, "%d MB\n", size);
}

/* purpose: wake compress ignoring cpu */
static ssize_t force_once_store(struct kobject *kobj,
			struct kobj_attribute *attr,
			const char *buf, size_t len)
{
	u64 size;
	struct rcc_module *rcc = &rcc_module;
	/* read as mb */
	size = memparse(buf, NULL);
	if (!size)
		return -EINVAL;

	if (rcc->full_clean_flag)
		return -EINVAL;

	size = size << (20 - PAGE_SHIFT); /* mb to page */
	if (size > totalram_pages * 4)
		return -EINVAL;
	rcc->force_once_pages = size;
	rcc->force_compress_flag = 1;
	rcc_thread_wakeup(rcc);
	return len;
}

static RCC_ATTR(enable, RCC_MODE_RW, enable_show, enable_store);
static RCC_ATTR(can_compress, RCC_MODE_RO, can_compress_show, NULL);
static RCC_ATTR(event, RCC_MODE_RW, event_show, event_store);
static RCC_ATTR(idle_threshold, RCC_MODE_RW, idle_threshold_show,
		idle_threshold_store);
static RCC_ATTR(swap_percent_low, RCC_MODE_RW, swap_percent_low_show,
		swap_percent_low_store);
static RCC_ATTR(free_size_min, RCC_MODE_RW, free_size_min_show,
		free_size_min_store);
static RCC_ATTR(full_clean_size, RCC_MODE_RW, full_clean_size_show,
		full_clean_size_store);
static RCC_ATTR(stat, RCC_MODE_RO, rcc_stat_show, NULL);
static RCC_ATTR(max_anon_clean_size, RCC_MODE_RW, max_anon_clean_size_show,
		max_anon_clean_size_store);
static RCC_ATTR(avail_target, RCC_MODE_RW, avail_target_show,
		avail_target_store);
static RCC_ATTR(anon_target, RCC_MODE_RW, anon_target_show,
		anon_target_store);
static RCC_ATTR(force_once, RCC_MODE_RW, force_once_show,
		force_once_store);

static struct attribute *rcc_attrs[] = {
	&kobj_attr_enable.attr,
	&kobj_attr_can_compress.attr,
	&kobj_attr_event.attr,
	&kobj_attr_idle_threshold.attr,
	&kobj_attr_swap_percent_low.attr,
	&kobj_attr_free_size_min.attr,
	&kobj_attr_full_clean_size.attr,
	&kobj_attr_stat.attr,
	&kobj_attr_max_anon_clean_size.attr,
	&kobj_attr_avail_target.attr,
	&kobj_attr_anon_target.attr,
	&kobj_attr_force_once.attr,
	NULL,
};

static struct attribute_group rcc_module_attr_group = {
	.attrs = rcc_attrs,
};

/**
 * purpose: create sysfs nodes for module
 * arguments:
 *    none
 * return:
 *    kobject : for future destroy.
 */
static struct kobject *sysfs_create(void)
{
	int err;
	struct kobject *kobj = NULL;
	kobj = kobject_create_and_add("rcc", kernel_kobj);

	if (!kobj) {
		pr_err("rcc: failed to create sysfs node.\n");
		return NULL;
	}
	err = sysfs_create_group(kobj, &rcc_module_attr_group);
	if (err) {
		pr_err("rcc: failed to create sysfs attrs.\n");
		kobject_put(kobj);
		return NULL;
	}
	return kobj;
}

/**
 * purpose: destroy sysfs nodes
 * arguments:
 *    kobj : kobject for release.
 * return:
 *    none
 */
static void sysfs_destroy(struct kobject *kobj)
{
	if (!kobj)
		return;
	kobject_put(kobj);
}

/* purpose: this module init */
static int __init rcc_init(void)
{
	/* int ret; */
	struct rcc_module *rcc = &rcc_module;

	pr_info("rcc init...\n");

	rcc_setup(rcc);

	rcc->kobj = sysfs_create();
	if (!rcc->kobj)
		goto failed_to_create_sysfs;

	pr_info("rcc inited successfully\n");
	return 0;

failed_to_create_sysfs:
	return -EFAIL;
}

/* purpose: this module de-init */
static void __exit rcc_exit(void)
{
	struct rcc_module *rcc = &rcc_module;

	sysfs_destroy(rcc->kobj);
	rcc->kobj = NULL;

	rcc_thread_stop(rcc);
}

module_init(rcc_init);
module_exit(rcc_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("davidly <davidly.lizhiwei@huawei.com>");
MODULE_DESCRIPTION("Dynamically compressed and clean RAM");
