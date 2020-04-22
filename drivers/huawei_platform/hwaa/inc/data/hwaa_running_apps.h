
#ifndef _HWAA_RUNNING_APPS_H
#define _HWAA_RUNNING_APPS_H
#include <linux/types.h>

struct running_app_pid_t {
	pid_t pid;
	struct list_head list;
};

struct running_app_t {
	uid_t uid;
	struct list_head pid_list;
	struct hlist_node hash_list;
};

bool hwaa_running_apps_exists(uid_t uid);

s32 hwaa_running_apps_insert(uid_t uid, pid_t pid);

s32 hwaa_running_apps_delete(uid_t uid);

void hwaa_running_apps_delete_all(void);

s32 hwaa_running_apps_add_pid(uid_t uid, pid_t pid);

s32 hwaa_running_apps_remove_pid(uid_t uid, pid_t pid);
#endif
