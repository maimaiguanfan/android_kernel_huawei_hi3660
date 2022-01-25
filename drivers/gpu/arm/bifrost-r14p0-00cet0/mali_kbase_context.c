/*
 *
 * (C) COPYRIGHT 2010-2018 ARM Limited. All rights reserved.
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



/*
 * Base kernel context APIs
 */

#include <mali_kbase.h>
#include <mali_midg_regmap.h>
#include <mali_kbase_mem_linux.h>
#include <mali_kbase_dma_fence.h>
#include <mali_kbase_ctx_sched.h>
#ifdef CONFIG_MALI_LAST_BUFFER
#include <platform/hisilicon/last_buffer/mali_kbase_hisi_lb_callback.h>
#endif

#if BASE_DEBUG_FENCE_TIMEOUT
#ifdef CONFIG_HW_ZEROHUNG
extern void fencewp_report(long timeout, bool dump);
#endif /* CONFIG_HW_ZEROHUNG */
enum hrtimer_restart kbase_wait_fence_timeout_callback(struct hrtimer *timer)
{
	pr_err("[MALI_MIDGARD] kbase_wait_fence_timeout, wait fence time out\n");

#ifdef CONFIG_HW_ZEROHUNG
	fencewp_report(FENCE_TIMEOUT_TIME, false);
#endif /* CONFIG_HW_ZEROHUNG */

	return HRTIMER_NORESTART;
}
#endif

/**
 * kbase_create_context() - Create a kernel base context.
 * @kbdev: Kbase device
 * @is_compat: Force creation of a 32-bit context
 *
 * Allocate and init a kernel base context.
 *
 * Return: new kbase context
 */
struct kbase_context *
kbase_create_context(struct kbase_device *kbdev, bool is_compat)
{
	struct kbase_context *kctx;
	int err;
	struct page *p;

	KBASE_DEBUG_ASSERT(kbdev != NULL);

	/* zero-inited as lot of code assume it's zero'ed out on create */
	kctx = vzalloc(sizeof(*kctx));

	if (!kctx)
		goto out;

	/* creating a context is considered a disjoint event */
	kbase_disjoint_event(kbdev);

	kctx->kbdev = kbdev;
	kctx->as_nr = KBASEP_AS_NR_INVALID;
	atomic_set(&kctx->refcount, 0);
	if (is_compat)
		kbase_ctx_flag_set(kctx, KCTX_COMPAT);
#if defined(CONFIG_64BIT)
	else
		kbase_ctx_flag_set(kctx, KCTX_FORCE_SAME_VA);
#endif /* !defined(CONFIG_64BIT) */

#ifdef CONFIG_MALI_TRACE_TIMELINE
	kctx->timeline.owner_tgid = task_tgid_nr(current);
#endif
	atomic_set(&kctx->setup_complete, 0);
	atomic_set(&kctx->setup_in_progress, 0);
	spin_lock_init(&kctx->mm_update_lock);
	kctx->process_mm = NULL;
	atomic_set(&kctx->nonmapped_pages, 0);
	kctx->slots_pullable = 0;
	kctx->tgid = current->tgid;
	kctx->pid = current->pid;

	err = kbase_mem_pool_init(&kctx->mem_pool,
				  kbdev->mem_pool_max_size_default,
				  KBASE_MEM_POOL_4KB_PAGE_TABLE_ORDER,
				  0,
				  kctx->kbdev,
				  &kbdev->mem_pool);
	if (err)
		goto free_kctx;

	err = kbase_mem_pool_init(&kctx->lp_mem_pool,
				  (kbdev->mem_pool_max_size_default >> 9),
				  KBASE_MEM_POOL_2MB_PAGE_TABLE_ORDER,
				  0,
				  kctx->kbdev,
				  &kbdev->lp_mem_pool);
	if (err)
		goto free_mem_pool;

#ifdef CONFIG_MALI_LAST_BUFFER
	lb_pools_callbacks *pools_cbs = kbase_hisi_get_lb_pools_cbs(kbdev);
	KBASE_DEBUG_ASSERT(pools_cbs);
	if (pools_cbs->init_ctx_pools(kctx)) {
		/* We have terminated the lb_pools if init failed, so free the normal pools.
		 */
		goto free_both_pools;
	}
#endif

	err = kbase_mem_evictable_init(kctx);
	if (err)
		goto free_both_pools;

	atomic_set(&kctx->used_pages, 0);

	err = kbase_jd_init(kctx);
	if (err)
		goto deinit_evictable;

	err = kbasep_js_kctx_init(kctx);
	if (err)
		goto free_jd;	/* safe to call kbasep_js_kctx_term  in this case */

	err = kbase_event_init(kctx);
	if (err)
		goto free_jd;


	atomic_set(&kctx->drain_pending, 0);

	mutex_init(&kctx->reg_lock);

	spin_lock_init(&kctx->mem_partials_lock);
	INIT_LIST_HEAD(&kctx->mem_partials);

	INIT_LIST_HEAD(&kctx->waiting_soft_jobs);
	spin_lock_init(&kctx->waiting_soft_jobs_lock);
	err = kbase_dma_fence_init(kctx);
	if (err)
		goto free_event;

	err = kbase_mmu_init(kbdev, &kctx->mmu, kctx);
	if (err)
		goto term_dma_fence;

	p = kbase_mem_alloc_page(&kctx->mem_pool);
	if (!p)
		goto no_sink_page;
	kctx->aliasing_sink_page = as_tagged(page_to_phys(p));

	init_waitqueue_head(&kctx->event_queue);

	kctx->cookies = KBASE_COOKIE_MASK;


	/* Make sure page 0 is not used... */
	err = kbase_region_tracker_init(kctx);
	if (err)
		goto no_region_tracker;

	err = kbase_sticky_resource_init(kctx);
	if (err)
		goto no_sticky;

	err = kbase_jit_init(kctx);
	if (err)
		goto no_jit;
#ifdef CONFIG_GPU_TRACEPOINTS
	atomic_set(&kctx->jctx.work_id, 0);
#endif
#ifdef CONFIG_MALI_TRACE_TIMELINE
	atomic_set(&kctx->timeline.jd_atoms_in_flight, 0);
#endif

	kctx->id = atomic_add_return(1, &(kbdev->ctx_num)) - 1;

#if BASE_DEBUG_FENCE_TIMEOUT
	kctx->timer_started = 0;
	hrtimer_init(&kctx->fence_wait_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	kctx->fence_wait_timer.function = kbase_wait_fence_timeout_callback;
#endif
	mutex_init(&kctx->vinstr_cli_lock);

	kbase_timer_setup(&kctx->soft_job_timeout,
			  kbasep_soft_job_timeout_worker);
	spin_lock(&kbdev->ctx_list_head_lock);
	list_add_tail(&kctx->ctx_list_entry, &kbdev->ctx_list_head);
	spin_unlock(&kbdev->ctx_list_head_lock);

	dev_err(kbdev->dev, "kctx %pK create \n", (void *)kctx);
	return kctx;

no_jit:
	kbase_gpu_vm_lock(kctx);
	kbase_sticky_resource_term(kctx);
	kbase_gpu_vm_unlock(kctx);
no_sticky:
	kbase_region_tracker_term(kctx);
no_region_tracker:
	kbase_mem_pool_free(&kctx->mem_pool, p, false);
no_sink_page:
	kbase_mmu_term(kbdev, &kctx->mmu);
term_dma_fence:
	kbase_dma_fence_term(kctx);
free_event:
	kbase_event_cleanup(kctx);
free_jd:
	/* Safe to call this one even when didn't initialize (assuming kctx was sufficiently zeroed) */
	kbasep_js_kctx_term(kctx);
	kbase_jd_exit(kctx);
deinit_evictable:
	kbase_mem_evictable_deinit(kctx);
free_both_pools:
	kbase_mem_pool_term(&kctx->lp_mem_pool);
free_mem_pool:
	kbase_mem_pool_term(&kctx->mem_pool);
free_kctx:
	vfree(kctx);
out:
	return NULL;
}
KBASE_EXPORT_SYMBOL(kbase_create_context);

static void kbase_reg_pending_dtor(struct kbase_device *kbdev,
		struct kbase_va_region *reg)
{
	dev_dbg(kbdev->dev, "Freeing pending unmapped region\n");
	kbase_mem_phy_alloc_put(reg->cpu_alloc);
	kbase_mem_phy_alloc_put(reg->gpu_alloc);
	kfree(reg);
}

/**
 * kbase_destroy_context - Destroy a kernel base context.
 * @kctx: Context to destroy
 *
 * Calls kbase_destroy_os_context() to free OS specific structures.
 * Will release all outstanding regions.
 */
void kbase_destroy_context(struct kbase_context *kctx)
{
	struct kbase_device *kbdev;
	int pages;
	unsigned long pending_regions_to_clean;
	unsigned long flags;
	struct page *p;

	KBASE_DEBUG_ASSERT(NULL != kctx);

	kbdev = kctx->kbdev;
	KBASE_DEBUG_ASSERT(NULL != kbdev);

	spin_lock(&kbdev->ctx_list_head_lock);
	list_del(&kctx->ctx_list_entry);
	spin_unlock(&kbdev->ctx_list_head_lock);

	dev_err(kbdev->dev, "kctx %pK being destroyed\n", (void *)kctx);
	KBASE_TRACE_ADD(kbdev, CORE_CTX_DESTROY, kctx, NULL, 0u, 0u);

	/* Ensure the core is powered up for the destroy process */
	/* A suspend won't happen here, because we're in a syscall from a userspace
	 * thread. */
	kbase_pm_context_active(kbdev);

	kbase_mem_pool_mark_dying(&kctx->mem_pool);

#ifdef CONFIG_MALI_LAST_BUFFER
	lb_pools_callbacks *pools_cbs = kbase_hisi_get_lb_pools_cbs(kbdev);
	KBASE_DEBUG_ASSERT(pools_cbs);
	pools_cbs->dying_ctx_pools(kctx);
#endif

	kbase_jd_zap_context(kctx);

	/* We have already waited for the jobs to complete (and hereafter there
	* can be no more submissions for the context). However the wait could
	* have timedout and there could still be work items in flight that
	* would do the completion processing of jobs.
	* kbase_jd_exit() will destroy the 'job_done_wq'. And destroying the wq
	* will cause it do drain and implicitly wait for those work items to
	* complete.
	*/
	kbase_jd_exit(kctx);

#ifdef CONFIG_HISI_DEBUG_FS
	/* Removing the rest of the debugfs entries here as we want to keep the
	 * atom debugfs interface alive until all atoms have completed. This
	 * is useful for debugging hung contexts. */
	debugfs_remove_recursive(kctx->kctx_dentry);
#endif

	kbase_event_cleanup(kctx);


	/*
	 * JIT must be terminated before the code below as it must be called
	 * without the region lock being held.
	 * The code above ensures no new JIT allocations can be made by
	 * by the time we get to this point of context tear down.
	 */
	kbase_jit_term(kctx);

	kbase_gpu_vm_lock(kctx);

	kbase_sticky_resource_term(kctx);

	/* drop the aliasing sink page now that it can't be mapped anymore */
	p = as_page(kctx->aliasing_sink_page);
	kbase_mem_pool_free(&kctx->mem_pool, p, false);

	/* free pending region setups */
	pending_regions_to_clean = (~kctx->cookies) & KBASE_COOKIE_MASK;
	while (pending_regions_to_clean) {
		unsigned int cookie = __ffs(pending_regions_to_clean);

		BUG_ON(!kctx->pending_regions[cookie]);

		kbase_reg_pending_dtor(kbdev, kctx->pending_regions[cookie]);

		kctx->pending_regions[cookie] = NULL;
		pending_regions_to_clean &= ~(1UL << cookie);
	}

	kbase_region_tracker_term(kctx);
	kbase_gpu_vm_unlock(kctx);


	/* Safe to call this one even when didn't initialize (assuming kctx was sufficiently zeroed) */
	kbasep_js_kctx_term(kctx);

	kbase_dma_fence_term(kctx);

	mutex_lock(&kbdev->mmu_hw_mutex);
	spin_lock_irqsave(&kctx->kbdev->hwaccess_lock, flags);
	kbase_ctx_sched_remove_ctx(kctx);
	spin_unlock_irqrestore(&kctx->kbdev->hwaccess_lock, flags);
	mutex_unlock(&kbdev->mmu_hw_mutex);

	kbase_mmu_term(kbdev, &kctx->mmu);

	pages = atomic_read(&kctx->used_pages);
	if (pages != 0)
		dev_warn(kbdev->dev, "%s: %d pages in use!\n", __func__, pages);

	kbase_mem_evictable_deinit(kctx);
	kbase_mem_pool_term(&kctx->mem_pool);
	kbase_mem_pool_term(&kctx->lp_mem_pool);

#ifdef CONFIG_MALI_LAST_BUFFER
	pools_cbs->term_ctx_pools(kctx);
#endif

	WARN_ON(atomic_read(&kctx->nonmapped_pages) != 0);

	vfree(kctx);
	kctx = NULL;

	kbase_pm_context_idle(kbdev);
}
KBASE_EXPORT_SYMBOL(kbase_destroy_context);

/**
 * kbase_context_set_create_flags - Set creation flags on a context
 * @kctx: Kbase context
 * @flags: Flags to set
 *
 * Return: 0 on success
 */
int kbase_context_set_create_flags(struct kbase_context *kctx, u32 flags)
{
	int err = 0;
	struct kbasep_js_kctx_info *js_kctx_info;
	unsigned long irq_flags;

	KBASE_DEBUG_ASSERT(NULL != kctx);

	js_kctx_info = &kctx->jctx.sched_info;

	/* Validate flags */
	if (flags != (flags & BASE_CONTEXT_CREATE_KERNEL_FLAGS)) {
		err = -EINVAL;
		goto out;
	}

	mutex_lock(&js_kctx_info->ctx.jsctx_mutex);
	spin_lock_irqsave(&kctx->kbdev->hwaccess_lock, irq_flags);

	/* Translate the flags */
	if ((flags & BASE_CONTEXT_SYSTEM_MONITOR_SUBMIT_DISABLED) == 0)
		kbase_ctx_flag_clear(kctx, KCTX_SUBMIT_DISABLED);

	/* Latch the initial attributes into the Job Scheduler */
	kbasep_js_ctx_attr_set_initial_attrs(kctx->kbdev, kctx);

	spin_unlock_irqrestore(&kctx->kbdev->hwaccess_lock, irq_flags);
	mutex_unlock(&js_kctx_info->ctx.jsctx_mutex);
 out:
	return err;
}
KBASE_EXPORT_SYMBOL(kbase_context_set_create_flags);
