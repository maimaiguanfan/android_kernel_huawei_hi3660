/*
 * hw_rscan_utils.c
 *
 * the hw_rscan_utils.c - get current run mode, eng or user
 *
 * likun <quentin.lee@huawei.com>
 * likan <likan82@huawei.com>
 *
 * Copyright (c) 2001-2021, Huawei Tech. Co., Ltd. All rights reserved.
 */

#include "./include/hw_rscan_utils.h"

int get_ro_secure(void)
{
#ifdef CONFIG_HW_ROOT_SCAN_ENG_DEBUG
	return RO_SECURE;
#else
	return RO_NORMAL;
#endif
}
