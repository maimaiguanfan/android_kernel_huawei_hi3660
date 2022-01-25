/*
 * DUBAI common header file.
 *
 * Copyright (C) 2017 Huawei Device Co.,Ltd.
 */

#ifndef DUBAI_COMMON_H
#define DUBAI_COMMON_H

#include <linux/sched.h>
#include <linux/uaccess.h>

#define DUBAI_LOG_TAG	"DUBAI"

#define DUBAI_DEBUG		0
#define DUBAI_INFO		1

#define DUBAI_LOGD(fmt, ...) \
	do { \
		if (DUBAI_DEBUG) \
			pr_debug("["DUBAI_LOG_TAG"] %s: "fmt"\n", \
					__func__, ##__VA_ARGS__); \
	} while (0)

#define DUBAI_LOGI(fmt, ...) \
	do { \
		if (DUBAI_INFO) \
			pr_info("["DUBAI_LOG_TAG"] %s: "fmt"\n", \
					__func__, ##__VA_ARGS__); \
	} while (0)

#define DUBAI_LOGE(fmt, ...) \
	pr_err("["DUBAI_LOG_TAG"] %s: "fmt"\n", __func__, ##__VA_ARGS__)


#define PREFIX_LEN				(32)
#define NAME_LEN				(PREFIX_LEN + TASK_COMM_LEN)
#define BINDER_STATS_LIST_LEN	(10)

enum {
	BUFFERED_LOG_MAGIC_PROC_CPUTIME = 0,
	BUFFERED_LOG_MAGIC_KWORKER,
	BUFFERED_LOG_MAGIC_UEVENT,
	BUFFERED_LOG_MAGIC_BINDER_STATS,
	BUFFERED_LOG_MAGIC_WS_LASTING_NAME,
};

struct process_name {
	pid_t pid;
	char comm[TASK_COMM_LEN];
	char name[NAME_LEN];
} __packed;

struct dev_transmit_t {
	int length;
	char data[0];
} __packed;

struct buffered_log_entry {
	int length;
	int magic;
	unsigned char data[0];
} __packed;

static inline int get_enable_value(void __user *argp, bool *enable) {
	uint8_t value;

	if (copy_from_user(&value, argp, sizeof(uint8_t)))
		return -EFAULT;

	if (value != 1 && value != 0) {
		DUBAI_LOGE("Invalid enable value: %u", value);
		return -EFAULT;
	}
	*enable = (value == 1);

	return 0;
}

static inline int get_timestamp_value(void __user *argp, long long *timestamp) {
	if (copy_from_user(timestamp, argp, sizeof(long long)))
		return -EFAULT;

	return 0;
}

void dubai_proc_cputime_init(void);
void dubai_proc_cputime_exit(void);
int dubai_proc_cputime_enable(void __user *argp);
int dubai_get_task_cpupower_enable(void __user *argp);
int dubai_get_proc_cputime(void __user *argp);
int dubai_get_proc_name(void __user *argp);
int dubai_get_process_name(struct process_name *process);
int dubai_set_proc_decompose(void __user *argp);

void dubai_gpu_init(void);
void dubai_gpu_exit(void);
int dubai_set_gpu_enable(void __user *argp);
int dubai_get_gpu_info(void __user *argp);

extern int (*send_buffered_log)(const struct buffered_log_entry *entry);
struct buffered_log_entry *create_buffered_log_entry(long long size, int magic);
void free_buffered_log_entry(struct buffered_log_entry *entry);
void buffered_log_release(void);

void dubai_stats_init(void);
void dubai_stats_exit(void);
int dubai_log_stats_enable(void __user *argp);
int dubai_get_kworker_info(void __user *argp);
int dubai_get_uevent_info(void __user *argp);
int dubai_set_brightness_enable(void __user *argp);
int dubai_get_brightness_info(void __user *argp);
int dubai_get_binder_stats(void __user *argp);
int dubai_set_binder_list(void __user *argp);
int dubai_binder_stats_enable(void __user *argp);
int dubai_get_aod_duration(void __user *argp);
int dubai_get_battery_rm(void __user *argp);
int dubai_get_ws_lasting_name(void __user *argp);

#endif
