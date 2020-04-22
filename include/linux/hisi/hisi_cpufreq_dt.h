#ifndef _HISI_CPUFREQ_DT_H
#define _HISI_CPUFREQ_DT_H

#include <linux/version.h>

#ifdef CONFIG_HISI_CPUFREQ_DT
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0)
int hisi_cpufreq_set_supported_hw(struct cpufreq_policy *policy);
void hisi_cpufreq_put_supported_hw(struct cpufreq_policy *policy);
#else
struct opp_table *hisi_cpufreq_set_supported_hw(struct device *cpu_dev);
void hisi_cpufreq_put_supported_hw(struct opp_table *opp_table);
#endif
void hisi_cpufreq_get_suspend_freq(struct cpufreq_policy *policy);
int hisi_cpufreq_init(void);

#ifdef CONFIG_HISI_HW_VOTE_CPU_FREQ
#include <linux/hisi/hisi_hw_vote.h>

#ifdef CONFIG_HISI_L2_DYNAMIC_RETENTION
void l2_dynamic_retention_ctrl(struct cpufreq_policy *policy, unsigned int freq);
#endif

struct hvdev *hisi_cpufreq_hv_init(struct device *cpu_dev);
void hisi_cpufreq_hv_exit(struct hvdev *cpu_hvdev, unsigned int cpu);
int hisi_cpufreq_set(struct hvdev *cpu_hvdev, unsigned int freq);
unsigned int hisi_cpufreq_get(unsigned int cpu);
void hisi_cpufreq_policy_cur_init(struct hvdev *cpu_hvdev,
				  struct cpufreq_policy *policy);
#endif

#endif

#endif /* _HISI_CPUFREQ_DT_H */
