#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/perf_event.h>
#include <linux/perf/arm_pmu.h>
#include <asm/mmu.h>

struct wa2_pmu_monitor {
	struct perf_event *pevent;
	bool monitor_started;
};

DEFINE_PER_CPU(struct wa2_pmu_monitor, wa2_monitor);
DEFINE_PER_CPU_READ_MOSTLY(u64, pmu_counter);
DEFINE_PER_CPU_READ_MOSTLY(u64, v2_apply);

#define ITER_NUM	1000
unsigned long test_bp_hardening(void)
{
	ktime_t before, after;
	unsigned long consumed_ns = 0;
	int i, cpu = smp_processor_id();

	before = ktime_get();
	/* PC has already been checked in entry.S */
	for (i = 0; i < ITER_NUM; i++)
		arm64_apply_bp_hardening();
	after = ktime_get();

	consumed_ns = ktime_to_ns(ktime_sub(after, before));
	consumed_ns /= ITER_NUM;

	pr_err("cpu=%d consumed_ns=%lu\n", cpu, consumed_ns);

	return consumed_ns;
}

bool need_to_apply(int cpu)
{
	struct wa2_pmu_monitor* monitor = &per_cpu(wa2_monitor, cpu);
	u64 need_apply = 0;

	if (!monitor->monitor_started || monitor->pevent == NULL)
		return true;

	need_apply = per_cpu(v2_apply, cpu);
	if (need_apply) {
		per_cpu(v2_apply, cpu) = 0;
		return true;
	} else {
		return false;
	}
}

static struct perf_event_attr *alloc_attr(void)
{
	struct perf_event_attr *attr;

	attr = kzalloc(sizeof(struct perf_event_attr), GFP_KERNEL);
	if (!attr)
		return attr;

	attr->type = PERF_TYPE_RAW;
	attr->size = sizeof(struct perf_event_attr);
	attr->pinned = 1;
	attr->exclude_idle = 1;

	return attr;
}

/* ignore debug code */
int get_wa2_monitor_status(void)
{
	struct wa2_pmu_monitor *monitor;
	unsigned long stat = 0;
	int cpu;
	u64 counter, need_apply;

	for_each_possible_cpu(cpu) {
		need_apply = per_cpu(v2_apply, cpu);
		monitor = &per_cpu(wa2_monitor, cpu);
		if (monitor->monitor_started) {
			stat |= (u32)BIT(cpu);
			counter = per_cpu(pmu_counter, cpu);
			pr_err("cpu=%d wa2_monitor_enable, counter=%llu, need-apply=%llu\n", cpu, counter, need_apply);
		} else {
			pr_err("cpu=%d,wa2_monitor_disable, need_apply=%llu\n", cpu, need_apply);
		}
		per_cpu(v2_apply, cpu) = 0;
	}

	return stat;
}

extern void hisi_get_slow_cpus(struct cpumask * cpumask);
extern u32 armv8pmu_get_counter(struct perf_event *event);
static int __init wa2_monitor_init(void)
{
	int cpu;
	struct wa2_pmu_monitor *monitor;
	struct perf_event *pevent;
	struct perf_event_attr *attr;
	struct cpumask slow_cpus;

	attr = alloc_attr();
	if (!attr)
		return -ENOMEM;

	attr->config = 0x11c;

	hisi_get_slow_cpus(&slow_cpus);

	for_each_possible_cpu(cpu) {
		monitor = &per_cpu(wa2_monitor, cpu);

		/* littlecores doesn't need to enable this PMU event */
		if (cpumask_test_cpu(cpu, &slow_cpus)) {
			monitor->monitor_started = false;
			monitor->pevent = NULL;

			per_cpu(pmu_counter, cpu) = 0;
			continue;
		}

		pevent = perf_event_create_kernel_counter(attr, cpu, NULL,
							  NULL, NULL);
		if (IS_ERR(pevent)) {
			monitor->monitor_started = false;
			monitor->pevent = NULL;
			per_cpu(pmu_counter, cpu) = 0;
			pr_err("fail to create wa2 pmu counter for cpu%d\n", cpu);
		} else {
			perf_event_enable(pevent);
			per_cpu(pmu_counter, cpu) = armv8pmu_get_counter(pevent);

			monitor->pevent = pevent;
			monitor->monitor_started = true;
			pr_info("success to create wa2 pmu counter for cpu%d\n", cpu);
		}
	}

	kfree(attr);

	return 0;
}

static void __exit wa2_monitor_exit(void)
{
	int cpu;
	struct wa2_pmu_monitor *monitor;

	for_each_possible_cpu(cpu) {
		monitor = &per_cpu(wa2_monitor, cpu);
		if (!monitor->monitor_started)
			continue;

		monitor->monitor_started = false;
		perf_event_release_kernel(monitor->pevent);
		monitor->pevent = NULL;
	}
}

late_initcall(wa2_monitor_init);
module_exit(wa2_monitor_exit);
