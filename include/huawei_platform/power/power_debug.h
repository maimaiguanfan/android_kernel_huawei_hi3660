/*
 * power_debug.h
 *
 * debug for power module
 *
 * Copyright (c) 2012-2018 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef _POWER_DEBUG_H_
#define _POWER_DEBUG_H_

#define POWER_DBG_NODE_NAME_LEN      (16)

/* power debugfs interface template file operations */
typedef ssize_t (*power_dgb_show)(void *, char *, ssize_t);
typedef ssize_t (*power_dgb_store)(void *, const char *, ssize_t);

struct power_dbg_attr {
	char name[POWER_DBG_NODE_NAME_LEN];
	void *dev_data;
	power_dgb_show show;
	power_dgb_store store;
	struct list_head list;
};

#ifdef CONFIG_HUAWEI_POWER_DEBUG
void power_dbg_ops_register(char *name, void *dev_data,
	power_dgb_show show, power_dgb_store store);

#else

static inline void power_dbg_ops_register(char *name, void *dev_data,
	power_dgb_show show, power_dgb_store store)
{
}
#endif /* CONFIG_HUAWEI_POWER_DEBUG */

#endif /* _POWER_DEBUG_H_ */
