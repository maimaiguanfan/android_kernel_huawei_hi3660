#ifndef _HISI_CPUFREQ_REQ_H
#define _HISI_CPUFREQ_REQ_H

#include <linux/notifier.h>

struct cpufreq_req {
	struct notifier_block nb;
	int cpu;
	unsigned int freq;
};

#ifdef CONFIG_HISI_CPUFREQ_DT
extern int hisi_cpufreq_init_req(struct cpufreq_req *req, int cpu);
extern void hisi_cpufreq_update_req(struct cpufreq_req *req,
				    unsigned int freq);
extern void hisi_cpufreq_exit_req(struct cpufreq_req *req);
#else
static inline int hisi_cpufreq_init_req(struct cpufreq_req *req, int cpu)
{
	return 0;
}

static inline void hisi_cpufreq_update_req(struct cpufreq_req *req,
					   unsigned int freq)
{
}

static inline void hisi_cpufreq_exit_req(struct cpufreq_req *req)
{
}
#endif

#endif /* _HISI_CPUFREQ_REQ_H */
