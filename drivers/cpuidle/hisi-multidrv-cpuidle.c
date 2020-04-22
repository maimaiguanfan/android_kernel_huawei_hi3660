/*
 * ARM64 generic CPU idle driver.
 *
 * Copyright (C) 2014 ARM Ltd.
 * Author: Lorenzo Pieralisi <lorenzo.pieralisi@arm.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#define pr_fmt(fmt) "CPUidle arm64: " fmt

#include <linux/cpuidle.h>
#include <linux/cpumask.h>
#include <linux/cpu_pm.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/slab.h>
#include <linux/hisi/hisi_idle_sleep.h>
#include <linux/io.h>
#include <linux/version.h>
#include <linux/string.h>

#include <asm/cpuidle.h>
#include <asm/suspend.h>
#include <linux/sched.h>
#include <linux/cpu.h>
#include "dt_idle_states.h"
#ifdef CONFIG_HISI_CORESIGHT_TRACE
#include <linux/coresight.h>
#endif

static unsigned long cpu_on_hotplug = 0;
static struct cpumask idle_cpus_mask;
static spinlock_t idle_spin_lock;
bool hisi_cluster_cpu_all_pwrdn(void)
{
	struct cpuidle_driver *drv = cpuidle_get_driver();
	struct cpumask cluster_cpu_mask;
	int all_pwrdn;

	if (!drv)
		return false;
	cpumask_copy(&cluster_cpu_mask, drv->cpumask);

	spin_lock(&idle_spin_lock);
	all_pwrdn = cpumask_subset(&cluster_cpu_mask, &idle_cpus_mask);
	spin_unlock(&idle_spin_lock);

	return !!all_pwrdn;
}
EXPORT_SYMBOL(hisi_cluster_cpu_all_pwrdn);


bool hisi_fcm_cluster_pwrdn(void)
{
	int fcm_pwrdn = 0;

	spin_lock(&idle_spin_lock);
	fcm_pwrdn = cpumask_subset(cpu_online_mask, &idle_cpus_mask);
	spin_unlock(&idle_spin_lock);

	return !!fcm_pwrdn;
}
EXPORT_SYMBOL(hisi_fcm_cluster_pwrdn);


static struct cpumask pending_idle_cpumask;

static void kick_cpu_sync(int cpu)
{
	smp_send_reschedule(cpu);
}

static void __iomem *idle_flag_addr[NR_CPUS] = {0};
static u32 idle_flag_bit[NR_CPUS] = {0};

/*
 * get core-idle-flag
 *
 * failed: return 0
 * success: return core-idle-flag
 * */
u32 hisi_get_idle_cpumask(void)
{
	u32 i, tmp_flag, core_idle_flag = 0;

	for(i = 0; i < NR_CPUS; ++i) {
		tmp_flag = 0;

		if(!idle_flag_addr[i]) {
			pr_err("get pwron cpumask : flag addr%d error.\n", i);
			return 0;
		}

		tmp_flag = (unsigned int)readl(idle_flag_addr[i]) & BIT(idle_flag_bit[i]);
		if(tmp_flag)
			core_idle_flag |= BIT(i);
	}

	return core_idle_flag;
}

/*
 * hisi_enter_idle_state - Programs CPU to enter the specified state
 *
 * dev: cpuidle device
 * drv: cpuidle driver
 * idx: state index
 *
 * Called from the CPUidle framework to program the device to the
 * specified target state selected by the governor.
 */
static int hisi_enter_idle_state(struct cpuidle_device *dev,
				  struct cpuidle_driver *drv, int idx)
{
	int ret;
	int cpu;

	if ((dev == NULL) || (drv == NULL)) {
		idx = -1;
		return idx;
	}
	if (need_resched()) {
		return idx;
	}
	if (cpu_on_hotplug) {
		idx = 0;
	}

	if (!idx) {
		cpu_do_idle();
		return idx;
	}

	cpu = dev->cpu;
	/* we assume the deepest state is cluster_idle */
	if (idx == (drv->state_count - 1)) {
		spin_lock(&idle_spin_lock);
		cpumask_set_cpu(cpu, &idle_cpus_mask);
		smp_wmb();
		spin_unlock(&idle_spin_lock);
	}

	ret = cpu_pm_enter();
	if (!ret) {
#ifdef CONFIG_ARCH_HISI
		local_fiq_disable();
#endif
		/*
		 * Pass idle state index to cpu_suspend which in turn will
		 * call the CPU ops suspend protocol with idle index as a
		 * parameter.
		 */
		ret = arm_cpuidle_suspend(idx);
#ifdef CONFIG_HISI_CORESIGHT_TRACE
		/*Restore ETM registers */
		_etm4_cpuilde_restore();
#endif
#ifdef CONFIG_ARCH_HISI
		local_fiq_enable();
#endif
		cpu_pm_exit();
	}

	if (idx == (drv->state_count - 1)) {
		spin_lock(&idle_spin_lock);
		cpumask_clear_cpu(cpu, &idle_cpus_mask);
		smp_wmb();
		spin_unlock(&idle_spin_lock);
	}

	return ret ? -1 : idx;
}

static int hisi_enter_coupled_idle_state(struct cpuidle_device *dev,
				struct cpuidle_driver *drv, int idx)
{
	int ret;
	int cpu;
	int pending_cpu;

	if ((dev == NULL) || (drv == NULL)) {
		idx = -1;
		return idx;
	}
	if (need_resched()) {
		return idx;
	}

	if (cpu_on_hotplug) {
		idx = 0;
	}

	if (!idx) {
		cpu_do_idle();
		return idx;
	}


	cpu = dev->cpu;
	if (idx == (drv->state_count - 1)) {
		spin_lock(&idle_spin_lock);
		cpumask_set_cpu(cpu, &idle_cpus_mask);
		smp_wmb();

		if (cpumask_subset(drv->cpumask, &idle_cpus_mask)) {
			pending_cpu = cpumask_first(&pending_idle_cpumask);
			if (pending_cpu < nr_cpu_ids && pending_cpu != cpu) {
				cpumask_clear_cpu(pending_cpu, &pending_idle_cpumask);
				smp_wmb();
				kick_cpu_sync(pending_cpu);
			}
		} else {
			/* pending bit */
			cpumask_set_cpu(cpu, &pending_idle_cpumask);
			smp_wmb();
			spin_unlock(&idle_spin_lock);

			cpu_do_idle();

			spin_lock(&idle_spin_lock);
			cpumask_clear_cpu(cpu, &idle_cpus_mask);
			smp_wmb();
			spin_unlock(&idle_spin_lock);

			return idx;
		}
		spin_unlock(&idle_spin_lock);

	}

	ret = cpu_pm_enter();
	if (!ret) {
#ifdef CONFIG_ARCH_HISI
		local_fiq_disable();
#endif
		 /*
		* Pass idle state index to cpu_suspend which in turn will
		* call the CPU ops suspend protocol with idle index as a
		* parameter.
		*/
		ret = arm_cpuidle_suspend(idx);
#ifdef CONFIG_HISI_CORESIGHT_TRACE
		/*Restore ETM registers */
		_etm4_cpuilde_restore();
#endif
#ifdef CONFIG_ARCH_HISI
		local_fiq_enable();
#endif
		cpu_pm_exit();
	}

	if (idx == (drv->state_count - 1)) {
		spin_lock(&idle_spin_lock);
		cpumask_clear_cpu(cpu, &idle_cpus_mask);
		smp_wmb();
		spin_unlock(&idle_spin_lock);
	}

	return ret ? -1 : idx;
}

/*lint -e64 -e785 -e651*/


static const struct of_device_id arm64_idle_state_match[] __initconst = {
	{ .compatible = "arm,idle-state",
	  .data = hisi_enter_idle_state },
	{ .compatible = "arm,coupled-idle-state",
	  .data = hisi_enter_coupled_idle_state },
	{ },
};
/*lint +e64 +e785 +e651*/
static int __init hisi_idle_drv_cpumask_init(struct cpuidle_driver *drv, int cluster_id)
{
	struct cpumask *cpumask;
	int cpu;

	cpumask = kzalloc(cpumask_size(), GFP_KERNEL);
	if (!cpumask)
		return -ENOMEM;

	for_each_possible_cpu(cpu) {//lint !e713
		if (cpu_topology[cpu].cluster_id == cluster_id)
			cpumask_set_cpu((unsigned int)cpu, cpumask);
	}

	drv->cpumask = cpumask;

	return 0;
}

static void __init hisi_idle_drv_cpumask_uninit(struct cpuidle_driver *drv)
{
	kfree(drv->cpumask);
}

static int __init hisi_idle_drv_init(struct cpuidle_driver *drv)
{
	int cpu, ret;

	/*
	 * Initialize idle states data, starting at index 1.
	 * This driver is DT only, if no DT idle states are detected (ret == 0)
	 * let the driver initialization fail accordingly since there is no
	 * reason to initialize the idle driver if only wfi is supported.
	 */
	ret = dt_init_idle_driver(drv, arm64_idle_state_match, 1);
	if (ret <= 0) {
		if (ret)
			pr_err("failed to initialize idle states\n");
		return ret ? : -ENODEV;
	}

	/*
	 * Call arch CPU operations in order to initialize
	 * idle states suspend back-end specific data
	 */
	for_each_possible_cpu(cpu) {//lint !e713
		ret = arm_cpuidle_init((unsigned int)cpu);
		if (ret) {
			pr_err("CPU %d failed to init idle CPU ops\n", cpu);
			return ret;
		}
	}

	ret = cpuidle_register(drv, NULL);
	if (ret) {
		pr_err("failed to register cpuidle driver\n");
		return ret;
	}

	return 0;
}

static int __init hisi_multidrv_idle_init(struct cpuidle_driver *drv, int cluster_id)
{
	int ret;

	ret = hisi_idle_drv_cpumask_init(drv, cluster_id);
	if (ret) {
		pr_err("fail to init cluster%d idle driver! \n", cluster_id);
		return ret;
	}

	ret = hisi_idle_drv_init(drv);
	if (ret) {
		hisi_idle_drv_cpumask_uninit(drv);
		pr_err("fail to register cluster%d cpuidle drv.\n", cluster_id);
		return ret;
	}

	return 0;
}

static int __init get_idle_mask_init(void)
{
	int ret;
	struct device_node *np = NULL;
	u32 i, reg_base_addr, flag_data[2] = {0};
	char filename[64] = {0};
	void __iomem *base_addr;

	np = of_find_compatible_node(NULL, NULL, "hisilicon,cpu-idle-flag");//lint !e838
	if (!np) {
		pr_err("idle_mask_init : get base node error.\n");
		return -ENODEV;
	}

	ret = of_property_read_u32(np, "idle-reg-base", &reg_base_addr);
	if (ret) {
		pr_err("idle_mask_init : get node idle-reg-base error.\n");
		return ENODEV;
	}

	base_addr = ioremap(reg_base_addr, SZ_4K);
	if (!base_addr) {
		pr_err("idle_mask_init : ioremap base addr error.\n");
		return -ENOMEM;
	}

	for (i = 0; i < NR_CPUS; ++i) {
		memset(flag_data, 0, sizeof(flag_data)); /* unsafe_function_ignore: memset */
		memset(filename, 0, sizeof(filename)); /* unsafe_function_ignore: memset */

		snprintf(filename, sizeof(filename), "core-%u-flag", i); /* unsafe_function_ignore: snprintf */
		ret = of_property_read_u32_array(np, filename, &flag_data[0], 2UL);
		if (ret)
			return ENODEV;

		idle_flag_addr[i] = base_addr + flag_data[0];
		if (!idle_flag_addr[i]) {
			pr_err("pwron_mask_init : ioremap addr%d error.\n", i);
			return -ENOMEM;
		}

		idle_flag_bit[i] = flag_data[1];
	}

	return 0;
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
static int cpuidle_decoup_hotplug_notify(struct notifier_block *nb,
		unsigned long action, void *hcpu)
{
	long cpu = (long)(uintptr_t)hcpu;
	if (nb == NULL)
		return -1;
	if (action & CPU_TASKS_FROZEN)
		return NOTIFY_OK;
	switch (action & ~CPU_TASKS_FROZEN) {
	case CPU_UP_PREPARE:
		set_bit((int)cpu, &cpu_on_hotplug);
		kick_all_cpus_sync();
		break;
	case CPU_DOWN_PREPARE:
	case CPU_DOWN_FAILED:
	case CPU_UP_CANCELED:
	case CPU_ONLINE:
		clear_bit((int)cpu, &cpu_on_hotplug);
		break;
	case CPU_DEAD:
		break;
	default:
		break;
	}
	return NOTIFY_OK;
}
/*lint -e785*/
static struct notifier_block cpuidle_decoup_hotplug_notifier = {
	.notifier_call = cpuidle_decoup_hotplug_notify,
};
#else
static int __ref cpuidle_decoup_hotplug_online(unsigned int cpu)
{
	clear_bit((int)cpu, &cpu_on_hotplug);
	return 0;
}

static int __ref cpuidle_decoup_hotplug_offline(unsigned int cpu)
{
	clear_bit((int)cpu, &cpu_on_hotplug);
	return 0;
}

static int __ref cpuidle_decoup_hotplug_down_prepare(unsigned int cpu)
{
	clear_bit((int)cpu, &cpu_on_hotplug);
	return 0;
}

static int __ref cpuidle_decoup_hotplug_up_prepare(unsigned int cpu)
{
	set_bit((int)cpu, &cpu_on_hotplug);
	kick_all_cpus_sync();
	return 0;
}
#endif

#define HISI_CLUSTER_IDLE_DRV_NAME_LEN  32
static struct cpuidle_driver hisi_cluster_idle_driver[NR_CPUS];

static void __init hisi_cluster_wfi_state_init(struct cpuidle_state *pstate_wfi)
{
	/*
	 * State at index 0 is standby wfi and considered standard
	 * on all ARM platforms. If in some platforms simple wfi
	 * can't be used as "state 0", DT bindings must be implemented
	 * to work around this issue and allow installing a special
	 * handler for idle state index 0.
	 */
	pstate_wfi->enter            = hisi_enter_idle_state;
	pstate_wfi->exit_latency     = 1;
	pstate_wfi->target_residency = 1;
	pstate_wfi->power_usage      = (int)UINT_MAX;
	strncpy(pstate_wfi->name, "WFI", CPUIDLE_NAME_LEN);
	strncpy(pstate_wfi->desc, "ARM64 WFI", CPUIDLE_DESC_LEN);
}

static int __init hisi_cluster_idle_driver_init(void)
{
	int ret;
	int cpu = 0;
	int cluster_num = 0;
	char *drv_name;
	struct cpumask drv_init_cpumask;
	struct cpuidle_driver *drv;

	cpumask_clear(&drv_init_cpumask);
	memset(hisi_cluster_idle_driver, 0, sizeof(hisi_cluster_idle_driver));

	for_each_possible_cpu(cpu) {
		if(cpumask_test_cpu(cpu, &drv_init_cpumask))
			continue;

		drv_name = kzalloc(HISI_CLUSTER_IDLE_DRV_NAME_LEN, GFP_KERNEL);
		if (drv_name){
			snprintf(drv_name, HISI_CLUSTER_IDLE_DRV_NAME_LEN, "hisi_cluster%d_idle_driver", cluster_num);
		}

		drv = &hisi_cluster_idle_driver[cluster_num++];
		drv->owner = THIS_MODULE;
		drv->name  = drv_name;
		drv_name   = NULL;

		hisi_cluster_wfi_state_init(&drv->states[0]);

		ret = hisi_multidrv_idle_init(drv, topology_physical_package_id(cpu));
		if (ret) {
			pr_err("fail to register cluster cpuidle drv.ret:%d\n", ret);
			return ret;
		}

		cpumask_or(&drv_init_cpumask, &drv_init_cpumask, drv->cpumask);

		if(cpumask_weight(&drv_init_cpumask) >= NR_CPUS)
			break;
	}

	return 0;
}




/*lint +e785*/
/*
 * hisi_idle_init
 *
 * Registers the hisi multi cpuidle driver with the cpuidle
 * framework. It relies on core code to parse the idle states
 * and initialize them using driver data structures accordingly.
 */
static int __init hisi_idle_init(void)
{
	int ret;

	ret = get_idle_mask_init();
	if (ret) {
		pr_err("pwron mask init err%d\n", ret);
	}
	spin_lock_init(&idle_spin_lock);
	cpumask_clear(&idle_cpus_mask);
	cpumask_clear(&pending_idle_cpumask);

	ret = hisi_cluster_idle_driver_init();
	if (ret) {
		return ret;
	}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
	ret = register_cpu_notifier(&cpuidle_decoup_hotplug_notifier);
	if (ret) {
		pr_err("fail to register cpuidle_coupled_cpu_notifier.\n");
		return ret;
	}
#else
	cpuhp_setup_state_nocalls(CPUHP_BP_HISI_MULTIDRV_NOTIFY_PREPARE, "multidrv-cpuidle:notify",
				  cpuidle_decoup_hotplug_up_prepare, cpuidle_decoup_hotplug_down_prepare);
	cpuhp_setup_state_nocalls(CPUHP_AP_ONLINE_DYN, "multidrv-cpuidle:online/offline",
				  cpuidle_decoup_hotplug_online, cpuidle_decoup_hotplug_offline);
#endif

	return 0;
}
/*lint -e528 -esym(528,*)*/
device_initcall(hisi_idle_init);
/*lint -e528 +esym(528,*)*/
