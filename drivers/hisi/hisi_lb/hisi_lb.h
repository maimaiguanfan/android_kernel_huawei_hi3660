#ifndef __HISI_LB_H
#define __HISI_LB_H
#include <asm/pgtable-prot.h>
#include <linux/mm_types.h>
struct scatterlist;

/* it need same with dts */
enum lb_pid {
	PID_BY_PASS = 0,

	PID_OPENCL  = 1,
	PID_GPUFBO  = 2,
	PID_GPUTXT  = 3,
	PID_IDISPLAY= 4,
	PID_JPEG    = 5,
	PID_VIDEO   = 6,
	PID_DMAP    = 7,
	PID_TINY    = 8,
	PID_AUDIO   = 9,

	PID_MAX,
};

#ifdef CONFIG_HISI_LB
extern void __lb_assert_page(struct page *pg);
extern void __lb_assert_phys(phys_addr_t phys);
extern void __lb_assert_pte(pte_t pte);
extern void *lb_page_to_virt(struct page *page);
extern unsigned int lb_page_to_gid(struct page *page);
extern int lb_request_quota(unsigned int pid);
extern int lb_release_quota(unsigned int policy_id);
extern struct page *lb_alloc_pages(unsigned int pid, gfp_t gfp_mask, unsigned int order);
extern int lb_free_pages(unsigned int pid, struct page *pages, unsigned int order);
extern unsigned long lb_pte_attr(phys_addr_t phy_addr);
extern int lb_prot_build(struct page* pages, pgprot_t *pageprot);
extern int lb_pid_prot_build(unsigned int pid, pgprot_t *pageprot);
extern int lb_sg_attach(unsigned int pid, struct scatterlist *sgl, unsigned int nents);
extern int lb_sg_detach(unsigned int pid, struct scatterlist *sgl, unsigned int nents);
extern u64 lb_pid_to_gidphys(u32 pid);
#else
static inline void __lb_assert_page(struct page *pg){}
static inline void __lb_assert_phys(phys_addr_t phys){}
static inline void __lb_assert_pte(pte_t pte){}
static inline void *lb_page_to_virt(struct page *page){return 0;}
static inline unsigned int lb_page_to_gid(struct page *page){return 0;}
static inline int lb_request_quota(unsigned int pid){return 0;}
static inline int lb_release_quota(unsigned int policy_id){return 0;}
static inline struct page *lb_alloc_pages(unsigned int pid, gfp_t gfp_mask, unsigned int order){return 0;}
static inline int lb_free_pages(unsigned int pid, struct page *pages, unsigned int order){return 0;}
static inline unsigned long lb_pte_attr(phys_addr_t phy_addr){return 0;}
static inline int lb_prot_build(struct page* pages, pgprot_t *pageprot){return 0;}
static inline int lb_pid_prot_build(struct page* pages, pgprot_t *pageprot){return 0;}
static inline int lb_sg_attach(unsigned int pid, struct scatterlist *sgl, unsigned int nents){return 0;}
static inline int lb_sg_detach(unsigned int pid, struct scatterlist *sgl, unsigned int nents){return 0;}
static inline u64 lb_pid_to_gidphys(u32 pid){return 0;}
#endif
#endif
