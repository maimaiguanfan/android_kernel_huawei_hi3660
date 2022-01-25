/*
 * Copyright (C) 2011 Sascha Hauer, Pengutronix <s.hauer@pengutronix.de>
 * Copyright (C) 2011 Richard Zhao, Linaro <richard.zhao@linaro.org>
 * Copyright (C) 2011-2012 Mike Turquette, Linaro Ltd <mturquette@linaro.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Simple multiplexer clock implementation
 */

#include <linux/clk-provider.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/err.h>
#ifdef CONFIG_HISI_CLK_DEBUG
#include "hisi-clk-debug.h"
#endif

/*
 * DOC: basic adjustable multiplexer clock that cannot gate
 *
 * Traits of this clock:
 * prepare - clk_prepare only ensures that parents are prepared
 * enable - clk_enable only ensures that parents are enabled
 * rate - rate is only affected by parent switching.  No clk_set_rate support
 * parent - parent is adjustable through clk_set_parent
 */

static u8 clk_mux_get_parent(struct clk_hw *hw)
{
	struct clk_mux *mux = to_clk_mux(hw);
	u32 num_parents = clk_hw_get_num_parents(hw);
	u32 val;

	/*
	 * FIXME need a mux-specific flag to determine if val is bitwise or numeric
	 * e.g. sys_clkin_ck's clksel field is 3 bits wide, but ranges from 0x1
	 * to 0x7 (index starts at one)
	 * OTOH, pmd_trace_clk_mux_ck uses a separate bit for each clock, so
	 * val = 0x4 really means "bit 2, index starts at bit 0"
	 */
	val = clk_readl(mux->reg) >> mux->shift;
	val &= mux->mask;

	if (mux->table) {
		u32 i;

		for (i = 0; i < num_parents; i++)
			if (mux->table[i] == val)
				return i;
		return -EINVAL;/*lint !e570 */
	}

	if (val && (mux->flags & CLK_MUX_INDEX_BIT))
		val = ffs(val) - 1;

	if (val && (mux->flags & CLK_MUX_INDEX_ONE))
		val--;

	if (val >= num_parents)
		return -EINVAL;/*lint !e570 */

	return val;
}

static int clk_mux_set_parent(struct clk_hw *hw, u8 index)
{
	struct clk_mux *mux = to_clk_mux(hw);
	u32 val;
	unsigned long flags = 0;

	if (mux->table) {
		index = mux->table[index];
	} else {
		if (mux->flags & CLK_MUX_INDEX_BIT)
			index = 1 << index;

		if (mux->flags & CLK_MUX_INDEX_ONE)
			index++;
	}

	if (mux->lock)
		spin_lock_irqsave(mux->lock, flags);
	else
		__acquire(mux->lock);

	if (mux->flags & CLK_MUX_HIWORD_MASK) {
		val = mux->mask << (mux->shift + 16);
	} else {
		val = clk_readl(mux->reg);
		val &= ~(mux->mask << mux->shift);
	}
	val |= index << mux->shift;
	clk_writel(val, mux->reg);

	if (mux->lock)
		spin_unlock_irqrestore(mux->lock, flags);
	else
		__release(mux->lock);

	return 0;
}

#ifdef CONFIG_HISI_CLK_DEBUG
static int hisi_selreg_check(struct clk_hw *hw)
{
	struct clk_mux *mux = to_clk_mux(hw);
	struct clk *clk = hw->clk;
	u32 val = 0;

	val = readl(mux->reg) >> mux->shift;
	val &= mux->mask;
	if (val && (mux->flags & CLK_MUX_INDEX_BIT))
		val = ffs(val) - 1;

	if (val && (mux->flags & CLK_MUX_INDEX_ONE))
		val--;

	if (NULL == clk_get_parent(clk))
		return 3;

	if (clk_get_parent_by_index(clk, val) == clk_get_parent(clk))
		return 1;
	else
		return 0;
}

static int hi3xxx_dumpmux(struct clk_hw *hw, char* buf, struct seq_file *s)
{
	struct clk_mux *mux = to_clk_mux(hw);
	long unsigned int clk_base_addr = 0;
	unsigned int clk_bit = 0;
	u32 val = 0;

	if (mux->reg && buf && !s) {
		val = readl(mux->reg) ;
		snprintf(buf, DUMP_CLKBUFF_MAX_SIZE, "[%s] : regAddress = 0x%pK, regval = 0x%x\n",  \
			__clk_get_name(hw->clk), mux->reg, val);
	}
	if(mux->reg && !buf && s) {
		clk_base_addr = (uintptr_t)mux->reg & CLK_ADDR_HIGH_MASK;
		clk_bit = (uintptr_t)mux->reg & CLK_ADDR_LOW_MASK;
		seq_printf(s, "    %-15s    %-15s    0x%03X    bit-%u:%u", hs_base_addr_transfer(clk_base_addr),  \
			"mux", clk_bit, mux->shift, (mux->shift + fls((mux->mask + 1)) - 2));
	}
	return 0;

}
#endif

const struct clk_ops clk_mux_ops = {
	.get_parent = clk_mux_get_parent,
	.set_parent = clk_mux_set_parent,
	.determine_rate = __clk_mux_determine_rate,
#ifdef CONFIG_HISI_CLK_DEBUG
	.check_selreg = hisi_selreg_check,
	.dump_reg = hi3xxx_dumpmux,
#endif
};
EXPORT_SYMBOL_GPL(clk_mux_ops);

const struct clk_ops clk_mux_ro_ops = {
	.get_parent = clk_mux_get_parent,
};
EXPORT_SYMBOL_GPL(clk_mux_ro_ops);

struct clk_hw *clk_hw_register_mux_table(struct device *dev, const char *name,
		const char * const *parent_names, u8 num_parents,
		unsigned long flags,
		void __iomem *reg, u8 shift, u32 mask,
		u8 clk_mux_flags, u32 *table, spinlock_t *lock)
{
	struct clk_mux *mux;
	struct clk_hw *hw;
	struct clk_init_data init;
	u8 width = 0;
	int ret;

	if (clk_mux_flags & CLK_MUX_HIWORD_MASK) {
		width = fls(mask) - ffs(mask) + 1;
		if (width + shift > 16) {
			pr_err("mux value exceeds LOWORD field\n");
			return ERR_PTR(-EINVAL);
		}
	}

	/* allocate the mux */
	mux = kzalloc(sizeof(struct clk_mux), GFP_KERNEL);
	if (!mux) {
		pr_err("%s: could not allocate mux clk\n", __func__);
		return ERR_PTR(-ENOMEM);
	}

	init.name = name;
	if (clk_mux_flags & CLK_MUX_READ_ONLY)
		init.ops = &clk_mux_ro_ops;
	else
		init.ops = &clk_mux_ops;
	init.flags = flags | CLK_IS_BASIC;
	init.parent_names = parent_names;
	init.num_parents = num_parents;

	/* struct clk_mux assignments */
	mux->reg = reg;
	mux->shift = shift;
	mux->mask = mask;
	mux->flags = clk_mux_flags;
	mux->lock = lock;
	mux->table = table;
	mux->hw.init = &init;

	hw = &mux->hw;
	ret = clk_hw_register(dev, hw);
	if (ret) {
		kfree(mux);
		hw = ERR_PTR(ret);
	}

	return hw;/*lint !e593 */
}
EXPORT_SYMBOL_GPL(clk_hw_register_mux_table);

struct clk *clk_register_mux_table(struct device *dev, const char *name,
		const char * const *parent_names, u8 num_parents,
		unsigned long flags,
		void __iomem *reg, u8 shift, u32 mask,
		u8 clk_mux_flags, u32 *table, spinlock_t *lock)
{
	struct clk_hw *hw;

	hw = clk_hw_register_mux_table(dev, name, parent_names, num_parents,
				       flags, reg, shift, mask, clk_mux_flags,
				       table, lock);
	if (IS_ERR(hw))
		return ERR_CAST(hw);
	return hw->clk;
}
EXPORT_SYMBOL_GPL(clk_register_mux_table);

struct clk *clk_register_mux(struct device *dev, const char *name,
		const char * const *parent_names, u8 num_parents,
		unsigned long flags,
		void __iomem *reg, u8 shift, u8 width,
		u8 clk_mux_flags, spinlock_t *lock)
{
	u32 mask = BIT(width) - 1;

	return clk_register_mux_table(dev, name, parent_names, num_parents,
				      flags, reg, shift, mask, clk_mux_flags,
				      NULL, lock);
}
EXPORT_SYMBOL_GPL(clk_register_mux);

struct clk_hw *clk_hw_register_mux(struct device *dev, const char *name,
		const char * const *parent_names, u8 num_parents,
		unsigned long flags,
		void __iomem *reg, u8 shift, u8 width,
		u8 clk_mux_flags, spinlock_t *lock)
{
	u32 mask = BIT(width) - 1;

	return clk_hw_register_mux_table(dev, name, parent_names, num_parents,
				      flags, reg, shift, mask, clk_mux_flags,
				      NULL, lock);
}
EXPORT_SYMBOL_GPL(clk_hw_register_mux);

void clk_unregister_mux(struct clk *clk)
{
	struct clk_mux *mux;
	struct clk_hw *hw;

	hw = __clk_get_hw(clk);
	if (!hw)
		return;

	mux = to_clk_mux(hw);

	clk_unregister(clk);
	kfree(mux);
}
EXPORT_SYMBOL_GPL(clk_unregister_mux);

void clk_hw_unregister_mux(struct clk_hw *hw)
{
	struct clk_mux *mux;

	mux = to_clk_mux(hw);

	clk_hw_unregister(hw);
	kfree(mux);
}
EXPORT_SYMBOL_GPL(clk_hw_unregister_mux);
