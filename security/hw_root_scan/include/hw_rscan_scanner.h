/*
 * hw_rscan_scanner.h
 *
 * the hw_rscan_scanner.h for kernel space root scan
 *
 * Yongzheng Wu <Wu.Yongzheng@huawei.com>
 * likun <quentin.lee@huawei.com>
 * likan <likan82@huawei.com>
 *
 * Copyright (c) 2001-2021, Huawei Tech. Co., Ltd. All rights reserved.
 */

#ifndef _HW_RSCAN_DYNAMIC_H_
#define _HW_RSCAN_DYNAMIC_H_

#include <chipset_common/security/root_scan.h>
#include <linux/fs.h>	/* for reading "/vendor/etc/rootscan.conf" */
#include <linux/kernel.h>
#include <linux/mutex.h>
#include <linux/power_supply.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/time.h>
#include <linux/types.h>
#include <linux/uaccess.h>	/* for reading "/vendor/etc/rootscan.conf" */
#include "kcode.h"
#include "rproc.h"
#include "sescan.h"
#include "setids.h"
#include "hw_rscan_interface.h"
#include "hw_rscan_utils.h"
#ifdef CONFIG_TEE_ANTIROOT_CLIENT
#include "rootagent.h"
#else
enum tee_rs_mask {
   ROOTSTATE_BIT   = 0,            /* 0    on */
   /* read from fastboot */
   OEMINFO_BIT,                    /* 1    on */
   FBLOCK_YELLOW_BIT,              /* 2    on */
   FBLOCK_RED_BIT,                 /* 3    on */
   FBLOCK_ORANGE_BIT,              /* 4    on */
                                   /* 5    off */
   /* dy scan result */
   KERNELCODEBIT   = 6,            /* 6    on */
   SYSTEMCALLBIT,                  /* 7    on */
   ROOTPROCBIT,                    /* 8    on */
   SESTATUSBIT,                    /* 9    on */
   SEHOOKBIT       = 10,           /* 10   on */
   SEPOLICYBIT,                    /* 11   off */
   PROCINTERBIT,                   /* 12   off */
   FRAMINTERBIT,                   /* 13   off */
   INAPPINTERBIT,                  /* 14   off */
   NOOPBIT        = 15,            /* 15   on */
   ITIMEOUTBIT,                    /* 16   on */
   EIMABIT,                        /*17    on */
   SETIDBIT,                       /* 18   on */
   CHECKFAILBIT,                   /* 19   on */
};

static inline uint32_t get_tee_status(void)
{
	return 0;
}
#endif

#define BATTERY_NAME "Battery"

enum ree_rs_mask {
	RS_KCODE = 0,
	RS_SYS_CALL,
	RS_SE_HOOKS,
	RS_SE_STATUS,
	RS_RRPOCS,
	RS_SETID,
};

#define check_status(status, type)    ((status >> type) & 0x1)

struct item_bits {
	int item_ree_bit;
	int item_tee_bit;
	unsigned int item_ree_mask;
};

#define              MAX_NUM_OF_ITEM          6
/*the order should be identical with item_info */
extern struct item_bits itembits[MAX_NUM_OF_ITEM];

int rscan_dynamic_init(void);

/*
 * rscan_dynamic - for kernel space root scan
 * Description: root scan by dynamic features, with result processed
 * @ops_mask, bit masks for caller to set different items
 *         for scanner. The lowest bit is for kernel code check, then the
 *         30th bit for system call verify, and so on.
 * @result, used for return results for each scanner item
 *          that correspond to ops_mask. In this function, result have been
 *          processed. For example, rproc had filtered whitelist.
 * @error_code, scan process result.
 * @return:
 *     Overall root status, return 0 if the device keep unrooted.
 *     Otherwise return an interger that each of its bit corresponds
 *     to its bitmasks
 */
int rscan_dynamic(uint op_mask, struct rscan_result_dynamic *result,
							int *error_code);

int stp_rscan_trigger(void);

#endif
