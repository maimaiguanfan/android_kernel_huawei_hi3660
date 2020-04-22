/*
 *
 * (C) COPYRIGHT 2014-2015, 2017 ARM Limited. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you can access it online at
 * http://www.gnu.org/licenses/gpl-2.0.html.
 *
 * SPDX-License-Identifier: GPL-2.0
 *
 */

#ifndef _KBASE_MEM_POOL_DEBUGFS_H
#define _KBASE_MEM_POOL_DEBUGFS_H

#include <mali_kbase.h>

/**
 * kbase_mem_pool_debugfs_init - add debugfs knobs for @pool
 * @parent:  Parent debugfs dentry
 * @pool:    Memory pool of small pages to control
 * @lp_pool: Memory pool of large pages to control
 *
 * Adds four debugfs files under @parent:
 * - mem_pool_size: get/set the current size of @pool
 * - mem_pool_max_size: get/set the max size of @pool
 * - lp_mem_pool_size: get/set the current size of @lp_pool
 * - lp_mem_pool_max_size: get/set the max size of @lp_pool
 */
void kbase_mem_pool_debugfs_init(struct dentry *parent,
		struct kbase_mem_pool *pool,
		struct kbase_mem_pool *lp_pool);


/**
 * kbase_lb_mem_pool_debugfs_init - add debugfs knobs for lb_pools
 * @kbdev:    The kbase_device to operate.
 * @parent:   Parent debugfs dentry
 * @lb_pools: The memory pools to init debugfs.
 *
 * Adds several debugfs nodes under kctx->kctx_dentry. For each memory
 * pool, there are four debugfs nodes under policy_id_<nr> directory.
 * - mem_pool_size: get/set the current size of pool
 * - mem_pool_max_size: get/set the max size of pool
 * - lp_mem_pool_size: get/set the current size of lp_pool
 * - lp_mem_pool_max_size: get/set the max size of lp_pool
 */
#ifdef CONFIG_MALI_LAST_BUFFER
int kbase_lb_mem_pool_debugfs_init(struct kbase_device *kbdev, struct dentry * parent,
                                   struct kbase_hisi_lb_pools *lb_pools);
int kbase_lb_ctx_pool_debugfs_init(struct kbase_context *kctx);
int kbase_lb_dev_pool_debugfs_init(struct kbase_device *kbdev);
#endif

#endif  /*_KBASE_MEM_POOL_DEBUGFS_H*/

