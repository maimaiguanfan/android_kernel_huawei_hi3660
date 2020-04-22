/*
 * check_root proc and stp trace log save
 *
 * Copyright (c) 2001-2017, Huawei Tech. Co., Ltd., Zhebo Zhang <zhangzhebo at huawei.com>
 * Copyright (C) 2018 Huawei Tech. Co., Ltd., Ningyu Wang <wangningyu at huawei.com>
 *
 */

#ifndef _CHECK_ROOT_H_
#define _CHECK_ROOT_H_

#include <linux/types.h>

#define STP_NAME_USERCOPY      "usercopy"
#define STP_NAME_CFI           "cfi"

#define STP_ID_USERCOPY        0x00000180
#define STP_ID_CFI             0x00000181

#define CHECKROOT_SETUID_FLAG     (1 << 0)
#define CHECKROOT_SETGID_FLAG     (1 << 1)
#define CHECKROOT_SETRESUID_FLAG  (1 << 2)
#define CHECKROOT_SETRESGID_FLAG  (1 << 3)

struct checkroot_ref_cnt {
	int setuid;
	int setgid;
	int setresuid;
	int setresgid;
};

#ifdef CONFIG_HUAWEI_PROC_CHECK_ROOT
extern uint get_setids_state(void);
extern int checkroot_setuid(uid_t uid);
extern int checkroot_setgid(gid_t gid);
extern int checkroot_setresuid(uid_t uid);
extern int checkroot_setresgid(gid_t gid);
extern void stp_save_trace_log(const char *name);
#else
static inline uint get_setids_state(void) { return 0; }
static inline int checkroot_setuid(uid_t uid) { return 0; }
static inline int checkroot_setgid(gid_t gid) { return 0; }
static inline int checkroot_setresuid(uid_t uid) { return 0; }
static inline int checkroot_setresgid(gid_t gid) { return 0; }
static inline void stp_save_trace_log(const char *name) {}
#endif


#endif
