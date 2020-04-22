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

#include <linux/debugfs.h>
#include <linux/seq_file.h>

#include <mali_kbase_mem_pool_debugfs.h>

#ifdef CONFIG_HISI_DEBUG_FS

static int kbase_mem_pool_debugfs_size_get(void *data, u64 *val)
{
	struct kbase_mem_pool *pool = (struct kbase_mem_pool *)data;

	*val = kbase_mem_pool_size(pool);

	return 0;
}

static int kbase_mem_pool_debugfs_size_set(void *data, u64 val)
{
	struct kbase_mem_pool *pool = (struct kbase_mem_pool *)data;

	kbase_mem_pool_trim(pool, val);

	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(kbase_mem_pool_debugfs_size_fops,
		kbase_mem_pool_debugfs_size_get,
		kbase_mem_pool_debugfs_size_set,
		"%llu\n");

static int kbase_mem_pool_debugfs_max_size_get(void *data, u64 *val)
{
	struct kbase_mem_pool *pool = (struct kbase_mem_pool *)data;

	*val = kbase_mem_pool_max_size(pool);

	return 0;
}

static int kbase_mem_pool_debugfs_max_size_set(void *data, u64 val)
{
	struct kbase_mem_pool *pool = (struct kbase_mem_pool *)data;

	kbase_mem_pool_set_max_size(pool, val);

	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(kbase_mem_pool_debugfs_max_size_fops,
		kbase_mem_pool_debugfs_max_size_get,
		kbase_mem_pool_debugfs_max_size_set,
		"%llu\n");

void kbase_mem_pool_debugfs_init(struct dentry *parent,
		struct kbase_mem_pool *pool,
		struct kbase_mem_pool *lp_pool)
{
	debugfs_create_file("mem_pool_size", S_IRUGO | S_IWUSR, parent,
			pool, &kbase_mem_pool_debugfs_size_fops);

	debugfs_create_file("mem_pool_max_size", S_IRUGO | S_IWUSR, parent,
			pool, &kbase_mem_pool_debugfs_max_size_fops);

	debugfs_create_file("lp_mem_pool_size", S_IRUGO | S_IWUSR, parent,
			lp_pool, &kbase_mem_pool_debugfs_size_fops);

	debugfs_create_file("lp_mem_pool_max_size", S_IRUGO | S_IWUSR, parent,
			lp_pool, &kbase_mem_pool_debugfs_max_size_fops);
}

#ifdef CONFIG_MALI_LAST_BUFFER
int kbase_lb_mem_pool_debugfs_init(struct kbase_device *kbdev, struct dentry * parent, struct kbase_hisi_lb_pools *lb_pools)
{
	int err;

	struct dentry *debugfs_lb_directory = debugfs_create_dir("lb_mem_pools",
			parent);
	if (!debugfs_lb_directory) {
		dev_err(kbdev->dev, "Couldn't create mali debugfs lb_mem_pools directory\n");
		return -ENOMEM;
	}

	KBASE_DEBUG_ASSERT(lb_pools);
	lb_pools_callbacks *pools_cbs = kbase_hisi_get_lb_pools_cbs(kbdev);
	KBASE_DEBUG_ASSERT(pools_cbs);

	// loop all the lb pools. use policy id to identify.
	struct kbase_mem_pool *walker;
	struct list_head *pool_list = &lb_pools->mem_pools;
	struct list_head *lp_pool_list = &lb_pools->lp_mem_pools;
	list_for_each_entry(walker, pool_list, pool_entry) {
		char pool_name[64];
		snprintf(pool_name, 64, "policy_id_%d", walker->lb_policy_id);

		struct dentry * debugfs_dev_lb_sub_dir = debugfs_create_dir(pool_name,
			debugfs_lb_directory);
		if (!debugfs_dev_lb_sub_dir) {
			dev_err(kbdev->dev, "Couldn't create mali debugfs dev lb_mem_pools sub directory\n");
			err = -ENOMEM;
			goto out;
		}

		// get corresponding large page pool.
		struct kbase_mem_pool * lp_pool = pools_cbs->find_pool(walker->lb_policy_id, lp_pool_list);
		if (!lp_pool) {
			dev_err(kbdev->dev, "Can not find the specific memory pool of policy id(%d).\n", walker->lb_policy_id);
			err = -EINVAL;
			goto out;
		}

		// create debugfs node for 4KB/2MB page pool.
		kbase_mem_pool_debugfs_init(debugfs_dev_lb_sub_dir, walker, lp_pool);
	}

	return 0;
out:
	debugfs_remove_recursive(debugfs_lb_directory);
	return err;
}

int kbase_lb_ctx_pool_debugfs_init(struct kbase_context *kctx)
{
	struct kbase_device *kbdev = kctx->kbdev;
	struct kbase_hisi_lb_pools *lb_pools = &kctx->hisi_ctx_data.lb_pools;
	return kbase_lb_mem_pool_debugfs_init(kbdev, kctx->kctx_dentry, lb_pools);
}

int kbase_lb_dev_pool_debugfs_init(struct kbase_device *kbdev)
{
	struct dentry *parent = kbdev->mali_debugfs_directory;
	struct kbase_hisi_lb_pools *lb_pools = &kbdev->hisi_dev_data.lb_pools;
	return kbase_lb_mem_pool_debugfs_init(kbdev, parent, lb_pools);
}

#endif

#endif /* CONFIG_HISI_DEBUG_FS */
