#ifndef _HKIP_ATKINFO_H_
#define _HKIP_ATKINFO_H_

#define HHEE_EVENT_MAGIC			(0x6851895ba852fb79)

#define MAX_UPLOAD_INFO_LEN 64

struct hkip_atkinfo {
    struct hhee_event_header *header;
    struct hhee_event_footer *footer;
    struct workqueue_struct *wq_atkinfo;
    struct delayed_work atkinfo_work;
    struct mutex atkinfo_mtx;
    struct timer_list timer;
	/*make sure hkip don't upload attack logs no more 3 times each 24hours */
    unsigned int cycle_time;
};

/* Event types */
enum hhee_event_type {
        HHEE_EV_UNSET /* Unused event buffer */,
        HHEE_EV_BOOT /* Hypervisor boot message */,

        /* System register violation */
        HHEE_EV_SR_START = 0x100-1,
        /* Attempt to disable stage 1 address translation */
        HHEE_EV_MMU_DISABLE = 0x100,
        /* Attempt to replace stage 1 top level address translation page table */
        HHEE_EV_MMU_REPLACE,
        /* Reserved for: Attempt to disable WXN */
        HHEE_EV_WXN_DISABLE,
        HHEE_EV_SR_END,

        /* Memory write violation */
        /* Attempt to overwrite kernel code */
        HHEE_EV_MW_START = 0x201-1,
        HHEE_EV_OS_TEXT_OVERWRITE = 0x201,
        HHEE_EV_MW_END
};

/* Event format */
struct hhee_event {
	uint16_t type; /* Event type */
	uint16_t seq_no;
	uint32_t flags; /* Reserved for flags, write zero, read ignore */

	uint64_t link; /* Exception return address (ELR_EL2) */
	uint64_t virt_addr; /* Virtual address (FAR_EL2) */
	uint64_t rsvd1; /* Reserved for IPA, undefined */
	uint32_t syndrome; /* Exception syndrome (ESR_EL2) */
	uint32_t ctx_id; /* Context ID (CONTEXTIDR_EL1) */
	uint64_t tid_system; /* Kernel mode thread ID (TPIDR_EL1) */
	uint64_t tid_shared; /* Thread ID (TPIDRRO_EL0) */
	uint64_t tid_user; /* User space thread ID (TPIDR_EL0) */

	uint32_t task_id __aligned(128);

	char description[256] __aligned(256); /* Human-readable description */
};

struct hhee_event_header {
	uint64_t magic __aligned(PAGE_SIZE); /* HHEE_EVENT_HEAD magic constant */
	uint64_t write_offset; /* Total number of messages ever written */
	uint64_t buffer_size; /* Size in bytes of entire buffer area(including header and footer) */
	uint64_t buffer_capacity; /* Capacity in messages of the circual buffer */
    uint64_t buffer_offset; /* Offset in bytes from &magic to &events[0] */
    uint64_t footer_offset; /* Offset in bytes of base address of struct hhee_event_footer */
	struct hhee_event events[] __aligned(sizeof (struct hhee_event));
};

struct hhee_event_footer {
	uint64_t read_offset __aligned(PAGE_SIZE);
};

#ifdef CONFIG_HKIP_ATKINFO_DEBUGFS
extern int __init atkinfo_create_debugfs(struct hkip_atkinfo *atkinfo);
#else
static inline int atkinfo_create_debugfs(struct hkip_atkinfo *atkinfo)
{
	return 0;
}
#endif
#endif
