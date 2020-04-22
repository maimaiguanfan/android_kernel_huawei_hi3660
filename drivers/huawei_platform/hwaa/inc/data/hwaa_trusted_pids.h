
#ifndef _HWAA_TRUSTED_PIDS_H
#define _HWAA_TRUSTED_PIDS_H
#include <linux/types.h>

typedef enum {
	HWAA_TRUSTED_APP,
	HWAA_TRUSTED_ZYGOTE
} hwaa_trusted_pid_t;

struct app_pid_t {
	pid_t pid;
	struct hlist_node hash_list;
};

struct zygote_pid_t {
	pid_t pid;
	struct list_head list;
};

void hwaa_trusted_pids_init(void);

void hwaa_trusted_pids_deinit(void);

s32 hwaa_trusted_pids_insert(hwaa_trusted_pid_t trusted_pid_type, pid_t pid);

bool hwaa_trusted_pids_exists(hwaa_trusted_pid_t trusted_pid_type, pid_t pid);

void hwaa_trusted_pids_delete(hwaa_trusted_pid_t trusted_pid_type, pid_t pid);

void hwaa_trusted_pids_delete_all(hwaa_trusted_pid_t trusted_pid_type);
#endif
