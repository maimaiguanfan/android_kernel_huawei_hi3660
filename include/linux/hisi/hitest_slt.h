#ifndef __HITEST_SLT_H__
#define __HITEST_SLT_H__

#ifdef CONFIG_HISILICON_PLATFORM_HITEST
extern int is_running_kernel_slt(void);
#else
static inline int is_running_kernel_slt(void) {return 0;}
#endif
#endif /* __HITEST_SLT_H__ */
