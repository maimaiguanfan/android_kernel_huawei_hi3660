/*
 * DUBAI logging information API definition.
 *
 * Copyright (C) 2017 Huawei Device Co.,Ltd.
 */

#ifndef DUBAI_H
#define DUBAI_H

enum {
	WORK_ENTER = 0,
	WORK_EXIT,
};

extern int dubai_update_gpu_info(unsigned long freq, unsigned long busy_time,
	unsigned long total_time, unsigned long cycle_ms);
extern void dubai_update_wakeup_info(const char *tag, const char *fmt, ...);
extern void dubai_update_brightness(uint32_t brightness);
extern void dubai_log_kworker(unsigned long address, unsigned long long enter_time);
extern void dubai_log_uevent(const char *devpath, unsigned int action);
extern void dubai_log_binder_stats(int reply, uid_t c_uid, int c_pid, uid_t s_uid, int s_pid);
extern void dubai_log_packet_wakeup_stats(const char *tag, const char *key, int value);

#endif
