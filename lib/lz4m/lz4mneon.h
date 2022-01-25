// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018 HUAWEI, Inc.
 */
#ifndef __ZRAM_ARM64_LZ4MNEON_H
#define __ZRAM_ARM64_LZ4MNEON_H

#include <linux/sched.h>
#include <linux/preempt.h>
#include <asm/neon.h>

struct lz4mneon_fpsimd_ctx {
	__uint128_t vregs[4];
};

#define __lz4mneon_vargs "%0"

static inline bool arm64_lz4m_neon_begin(struct lz4mneon_fpsimd_ctx *ctx)
{
	if (!in_interrupt()) {
		preempt_disable();

		/* Already saved by kernel_neon_begin() */
		if (!current->mm ||
				test_thread_flag(TIF_FOREIGN_FPSTATE)) {
			kernel_neon_begin_partial(32);
			preempt_enable();       /* preempt_count +1 */
			return false;
		}
	}

	/* Save NEON registers to lz4mneon_fpsimd_ctx */
	__asm__ __volatile__(
			"stp    q0, q1, [" __lz4mneon_vargs "]\n\r"
			"stp    q2, q3, [" __lz4mneon_vargs ", #32]"
			:: "r"(ctx->vregs)
			: "memory", "cc");
	return true;
}

static inline void __restore_lz4mneon_ctx(struct lz4mneon_fpsimd_ctx *ctx)
{
	__asm__ __volatile__(
			"ldp    q0, q1, [" __lz4mneon_vargs "]\n\r"
			"ldp    q2, q3, [" __lz4mneon_vargs ", #32]"
			:: "r"(ctx->vregs)
			: "memory", "cc");
}


static inline void arm64_lz4m_neon_end(bool saved,
		struct lz4mneon_fpsimd_ctx *ctx)
{
	if (in_interrupt()) {
		WARN_ON(!saved);
		__restore_lz4mneon_ctx(ctx);
		return;
	}

	if (saved)
		__restore_lz4mneon_ctx(ctx);
	preempt_enable();
}

#endif
