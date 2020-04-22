#define pr_fmt(fmt) "hisi_lowmem: " fmt

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/swap.h>
#include <linux/version.h>

#include "lowmem_killer.h"
#define CREATE_TRACE_POINTS
#include "lowmem_trace.h"

#define INVALID_NUM -1
#define CMA_TUNE 1

static int nzones = INVALID_NUM;

static inline void hisi_lowmem_init(void)
{
	int n = 0;
	struct zone *zone;

	for_each_populated_zone(zone)
		n++;

	nzones = n;
}

int hisi_lowmem_tune(int *other_free, int *other_file,
		     struct shrink_control *sc)
{
	if (nzones == INVALID_NUM)
		hisi_lowmem_init();

	if (!(sc->gfp_mask & ___GFP_CMA)) {
		int nr_free_cma;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
		nr_free_cma = (int)global_page_state(NR_FREE_CMA_PAGES);
#else
		nr_free_cma = (int)global_zone_page_state(NR_FREE_CMA_PAGES);
#endif
		trace_lowmem_tune(nzones, sc->gfp_mask, *other_free,
				  *other_file, -nr_free_cma, 0);
		*other_free -= nr_free_cma;

		return CMA_TUNE;
	}

	return 0;
}
