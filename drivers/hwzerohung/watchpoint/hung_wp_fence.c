/**********************************************************
 * Filename:    hung_wp_fence.c
 *
 * Discription: fence timeout watchpoint implementation file.
 *
 * When baseline update, that file that this interface located
 * deleted everytime, because the fence src changed obviously. so
 * reconstruct this watchpoint to new file.
 *
 * Copyright: (C) 2018 huawei.
 *
 * Author: huanghuijin
 *
**********************************************************/

#include <linux/kernel.h>
#include <linux/jiffies.h>
#include <linux/version.h>
#include "chipset_common/hwzrhung/zrhung.h"

#ifdef CONFIG_SW_SYNC
extern void sync_dump(void);
#endif

/* 4.14 kernel, there is no fence.c */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))

void fencewp_report(long timeout, bool dump)
{
    char fence_buf[128] = {0};

    if (dump == true) {
#ifdef CONFIG_SW_SYNC
        sync_dump();
#else
		;
#endif
    }


    snprintf(fence_buf, sizeof(fence_buf), "fence timeout after %dms\n", jiffies_to_msecs(timeout));
    zrhung_send_event(ZRHUNG_WP_FENCE, "K", fence_buf);
}
#endif
