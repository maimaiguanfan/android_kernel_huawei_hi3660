#undef TRACE_SYSTEM
#define TRACE_SYSTEM hisi_cpufreq_req

#if !defined(_TRACE_HISI_CPUFREQ_REQ_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_HISI_CPUFREQ_REQ_H

#include <linux/tracepoint.h>

TRACE_EVENT(cpufreq_req_notify,/* [false alarm]:原生宏定义 */
	TP_PROTO(int cpu, void *req, unsigned int freq,
		 unsigned int min, unsigned int max),
	TP_ARGS(cpu, req, freq, min, max),

	TP_STRUCT__entry(
		__field(int,		cpu	)
		__field(void *,		req	)
		__field(unsigned int,	freq	)
		__field(unsigned int,	min	)
		__field(unsigned int,	max	)
	),

	TP_fast_assign(
		__entry->cpu	= cpu;
		__entry->req	= req;
		__entry->freq	= freq;
		__entry->min	= min;
		__entry->max	= max;
	),

	TP_printk("cpu=%d req=%pK freq=%u min=%u max=%u",
		  __entry->cpu, __entry->req, __entry->freq,
		  __entry->min, __entry->max)
);

TRACE_EVENT(cpufreq_req_update,/* [false alarm]:原生宏定义 */
	TP_PROTO(int cpu, void *req, unsigned int freq),
	TP_ARGS(cpu, req, freq),

	TP_STRUCT__entry(
		__field(int,		cpu	)
		__field(void *,		req	)
		__field(unsigned int,	freq	)
	),

	TP_fast_assign(
		__entry->cpu	= cpu;
		__entry->req	= req;
		__entry->freq	= freq;
	),

	TP_printk("cpu=%d req=%pK freq=%u",
		  __entry->cpu, __entry->req, __entry->freq)
);

#endif /* _TRACE_HISI_CPUFREQ_REQ_H */

/* This part must be outside protection */
#include <trace/define_trace.h>
