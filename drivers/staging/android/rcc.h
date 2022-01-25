

#ifndef _RCC_MODULE_H_
#define _RCC_MODULE_H_

#include <linux/mutex.h>

enum {
	INDEX_CPU_BUSY = 0,
	INDEX_CPU_TOTAL,
	INDEX_CPU_MAX
};

struct rcc_module {
	/* protect configs from other threads,notifications. */
	struct rw_semaphore	lock;
	/* wakeup/wait for working thread. */
	struct semaphore		wait;
	/* sysfs node */
	struct kobject		*kobj;
	/* kthread handler, this also is enable status flag. */
	struct task_struct	*task;
	/* only action on received event, remove periodically action. */
	int                  passive_mode;
	/* display is off. */
	bool					display_off;
	/* flag to indicate that we will do a full clean. */
	int					full_clean_flag;
	/* flag to indicate that we will wake up ignoring cpu load */
	int					force_compress_flag;
	/* flag to indicate that we should pause compress now */
	int					pause_flag;
	/* old cpu stat, for calculate cpu idle. */
	clock_t				last_cpu_stat[INDEX_CPU_MAX];
	/*  last time to get cpu stat. */
	unsigned int			last_cpu_msec;
	/* cpu load in percent. */
	int					cpu_load[3];
	/* cpu load threshld in percent for compress action. */
	int					idle_threshold;
	/* max usage of swap area, in percent. */
	int					swap_percent_low;
	/* min free pages to keep in our system. */
	int					free_pages_min;
	/* max file pages to be cleaned in full clean process. */
	int					full_clean_file_pages;
	/* min anon pages kept uncompress. */
	int					anon_pages_min;
	/* max anon pages kept uncompress. */
	int					anon_pages_max;
	/* max anon pages to be cleaned in full clean process. */
	int					full_clean_anon_pages;
	/* counter for recording wakeup times. */
	int					wakeup_count;
	/* counter for total full clean pages. */
	unsigned int		nr_full_clean_pages;
	/* counter for total normal clean pages. */
	unsigned int		nr_normal_clean_pages;
	/* jiffies counter for all swap time. */
	unsigned int		total_spent_times;
	/* target avail mem to clean */
	unsigned int		avail_target_pages;
	/* force once mem to clean */
	unsigned int		force_once_pages;
};

/* common fail, no special reason. */
#define EFAIL					1

/* page to KB,MB */
#define K(x) ((x) << (PAGE_SHIFT - 10))
#define M(x) ((x) >> (20 - PAGE_SHIFT))

/* threshold time for idle stat judgement. in ms. */
#define RCC_IDLE_THRESHOLD		70

/* swap full percent for stop compress thread. in percent. */
#define RCC_SWAP_PERCENT_LOW		85
/* stop free process until +RCC_SWAP_PERCENT_LOW_EX */
#define RCC_SWAP_PERCENT_LOW_EX	5

/* min free memory size to be kept in out system. in page count. */
#define RCC_FREE_PAGE_MIN		((320 * 1024 * 1024) >> (PAGE_SHIFT))
/* stop free process until RCC_FREE_PAGE_MIN+RCC_FREE_PAGE_MIN_EX */
#define RCC_FREE_PAGE_MIN_EX		((48 * 1024 * 1024) >> (PAGE_SHIFT))

/* keep uncompressed anon pages at least RCC_ANON_PAGE_MIN_ON_BOOT at full compress */
#define RCC_ANON_PAGE_MIN_ON_BOOT		((160*1024*1024)>>(PAGE_SHIFT))
/* when anon pages larger than RCC_ANON_PAGE_MAX_ON_BOOT, start compress. */
#define RCC_ANON_PAGE_MAX_ON_BOOT		((320*1024*1024)>>(PAGE_SHIFT))

/* keep uncompressed anon pages at least RCC_ANON_PAGE_MIN. */
#define RCC_ANON_PAGE_MIN		((200 * 1024 * 1024) >> (PAGE_SHIFT))
/* when anon pages larger than RCC_ANON_PAGE_MIN + GAP, start compress. */
#define RCC_ANON_PAGE_START_GAP		((60 * 1024 * 1024) >> (PAGE_SHIFT))
/* different RCC_ANON_PAGE_MIN for different ram */
#define RCC_ANON_PAGE_RAM_GAP		((250 * 1024 * 1024) >> (PAGE_SHIFT))
/* force reclaim pages one */
#define RCC_ANON_PAGE_FORCE_ONCE		((100 * 1024 * 1024) >> (PAGE_SHIFT))

#define TOTAL_RAM_PAGES_1G	(1 << 18)

/* full clean file pages. */
#define RCC_FULL_CLEAN_FILE_PAGE	((16 * 1024 * 1024) >> (PAGE_SHIFT))

/* swap page size once */
#define RCC_NR_SWAP_UNIT_SIZE		((1 * 1024 * 1024) >> (PAGE_SHIFT))

/* max reclaim page size on boot complete */
#define RCC_MAX_RECLAIM_ON_BOOT	((320 * 1024 * 1024) >> (PAGE_SHIFT))

/* if anon page is less RCC_ANON_PAGE_MIN or cpu busy, count++,
 * and then count more than RCC_MAX_WAIT_COUNT, exit the full fill swap
 */
#define RCC_MAX_WAIT_COUNT    20000

/* threshold time for idle stat judgement. in ms. */
#define RCC_IDLE_SLOW			1000
#define RCC_IDLE_FAST			20

#define WS_NEED_WAKEUP			0
#define WF_NOT_ENABLED			(1 << 0)
#define WF_CPU_BUSY				(1 << 1)
#define WF_MEM_FREE_ENOUGH		(1 << 2)
#define WF_SWAP_FULL			(1 << 3)
#define WF_NO_ANON_PAGE			(1 << 4)
#define WF_AVAIL_ENOUGH			(1 << 5)

#define RCC_WAIT_INFINITE		-1

#define RCC_SLEEP_TIME          200
#define RCC_PAUSE_TIME          3000
#endif
