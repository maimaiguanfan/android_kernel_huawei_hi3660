/*
 * DUBAI process cputime.
 *
 * Copyright (C) 2017 Huawei Device Co.,Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include <linux/hashtable.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/net.h>
#include <linux/profile.h>
#include <linux/slab.h>
#include <linux/version.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
#include <linux/sched.h>
#include <linux/sched/cputime.h>
#include <linux/sched/signal.h>
#endif

#include <chipset_common/dubai/dubai_common.h>

#define PROC_HASH_BITS			(10)
#define MAX_CMDLINE_LEN			(128)
#define BASE_COUNT				(500)
#define KERNEL_TGID				(0)
#define KERNEL_NAME				"kernel"
#define SYSTEM_SERVER			"system_server"
#define DUBAID_NAME				"dubaid"
#define CMP_TASK_COMM(p, q) 	strncmp(p, q, TASK_COMM_LEN - 1)
#define COPY_TASK_COMM(p, q)	strncpy(p, q, TASK_COMM_LEN - 1)/* unsafe_function_ignore: strncpy */
#define COPY_PROC_NAME(p, q)	strncpy(p, q, NAME_LEN - 1)/* unsafe_function_ignore: strncpy */
#define DECOMPOSE_COUNT_MAX		(10)
#define DECOMPOSE_RETYR_MAX		(30)
#define LOG_ENTRY_SIZE(count) \
	sizeof(struct dubai_cputime_transmit) \
		+ (long long)(count) * sizeof(struct dubai_cputime)

enum {
    CMDLINE_NEED_TO_GET = 0,
    CMDLINE_PROCESS,
    CMDLINE_THREAD,
};

enum {
	PROCESS_STATE_DEAD = 0,
	PROCESS_STATE_ACTIVE,
	PROCESS_STATE_INVALID,
};

enum {
	TASK_STATE_RUNNING = 0,
	TASK_STATE_SLEEPING,
	TASK_STATE_DISK_SLEEP,
	TASK_STATE_STOPPED,
	TASK_STATE_TRACING_STOP,
	TASK_STATE_DEAD,
	TASK_STATE_ZOMBIE,
	TASK_STATE_PARKED,
	TASK_STATE_IDLE,
};

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
// keep sync with fs/proc/array.c
static const int task_state_array[] = {
	TASK_STATE_RUNNING,
	TASK_STATE_SLEEPING,
	TASK_STATE_DISK_SLEEP,
	TASK_STATE_STOPPED,
	TASK_STATE_TRACING_STOP,
	TASK_STATE_DEAD,
	TASK_STATE_ZOMBIE,
	TASK_STATE_PARKED,
	TASK_STATE_IDLE,
};
#else
static const int task_state_array[] = {
	TASK_STATE_RUNNING,
	TASK_STATE_SLEEPING,
	TASK_STATE_DISK_SLEEP,
	TASK_STATE_STOPPED,
	TASK_STATE_TRACING_STOP,
	TASK_STATE_DEAD,
	TASK_STATE_ZOMBIE,
};
#endif

struct dubai_cputime {
	uid_t uid;
	pid_t pid;
	unsigned long long time;
	unsigned long long power;
	unsigned char cmdline;
	char name[NAME_LEN];
} __packed;

struct dubai_thread_entry {
	pid_t pid;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
	u64 utime;
	u64 stime;
#else
	cputime_t utime;
	cputime_t stime;
#endif
#ifdef CONFIG_HUAWEI_DUBAI_TASK_CPU_POWER
	unsigned long long power;
#endif
	bool alive;
	char name[NAME_LEN];
	struct list_head node;
};

struct dubai_proc_entry {
	pid_t tgid;
	uid_t uid;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
	u64 utime;
	u64 stime;
	u64 active_utime;
	u64 active_stime;
#else
	cputime_t utime;
	cputime_t stime;
	cputime_t active_utime;
	cputime_t active_stime;
#endif
#ifdef CONFIG_HUAWEI_DUBAI_TASK_CPU_POWER
	unsigned long long active_power;
	unsigned long long power;
#endif
	bool alive;
	bool cmdline;
	char name[NAME_LEN];
	struct list_head threads;
	struct hlist_node hash;
};

struct dubai_cputime_transmit {
	long long timestamp;
	int type;
	int count;
	unsigned char value[0];
} __packed;

struct dubai_decompose_info {
	uid_t uid;
	char comm[TASK_COMM_LEN];
	char prefix[PREFIX_LEN];
} __packed;

struct dubai_proc_decompose {
	pid_t tgid;
	struct dubai_decompose_info decompose;
	struct list_head node;
};

static DECLARE_HASHTABLE(proc_hash_table, PROC_HASH_BITS);
static DEFINE_MUTEX(dubai_proc_lock);
static atomic_t proc_cputime_enable;
static atomic_t dead_count;
static atomic_t active_count;

static void dubai_decompose_work_fn(struct work_struct *work);
static DECLARE_DELAYED_WORK(dubai_decompose_work, dubai_decompose_work_fn);
static LIST_HEAD(dubai_proc_decompose_list);
static const int decompose_check_delay = HZ;
static atomic_t decompose_count_target;
static atomic_t decompose_count;
static atomic_t decompose_check_retry;

#ifdef CONFIG_HUAWEI_DUBAI_TASK_CPU_POWER
static const atomic_t task_power_enable = ATOMIC_INIT(1);
#else
static const atomic_t task_power_enable = ATOMIC_INIT(0);
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
static inline unsigned long long dubai_cputime_to_usecs(u64 time)
{
	return ((unsigned long long)ktime_to_ms(time) * USEC_PER_MSEC);
}
#else
static inline unsigned long long dubai_cputime_to_usecs(cputime_t time)
{
	return ((unsigned long long)
		jiffies_to_msecs(cputime_to_jiffies(time)) * USEC_PER_MSEC);
}
#endif

// keep sync with fs/proc/array.c
static inline int dubai_get_task_state(const struct task_struct *task)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
	BUILD_BUG_ON(1 + ilog2(TASK_REPORT_MAX) != ARRAY_SIZE(task_state_array));
	unsigned int state = __get_task_state(task);
#else
	unsigned int state = (task->state | task->exit_state) & TASK_REPORT;

	/*
	 * Parked tasks do not run; they sit in __kthread_parkme().
	 * Without this check, we would report them as running, which is
	 * clearly wrong, so we report them as sleeping instead.
	 */
	if (task->state == TASK_PARKED)
		state = TASK_INTERRUPTIBLE;

	BUILD_BUG_ON(1 + ilog2(TASK_REPORT)
			!= ARRAY_SIZE(task_state_array) - 1);
#endif

	return task_state_array[fls(state)];
}

static bool dubai_task_alive(const struct task_struct *task)
{
#ifdef CONFIG_HUAWEI_DUBAI_TASK_CPU_POWER
	/*
	 * if this task is exiting, we have already accounted for the
	 * time and power.
	 */
	if (unlikely(task == NULL) || (task->cpu_power == ULLONG_MAX))
		return false;
#else
	int state;

	if (unlikely(task == NULL)
		|| (task->flags & PF_EXITING)
		|| (task->flags & PF_EXITPIDONE)
		|| (task->flags & PF_SIGNALED))
		return false;

	state = dubai_get_task_state(task);
	if (state == TASK_STATE_DEAD || state == TASK_STATE_ZOMBIE)
		return false;
#endif

	return true;
}

static void dubai_copy_name(char *to, const char *from)
{
	char *p;
	int len = NAME_LEN - 1;

	if (strlen(from) <= len) {
		strncpy(to, from, len);/* unsafe_function_ignore: strncpy */
		return;
	}

	p = strrchr(from, '/');
	if (p != NULL && (*(p + 1) != '\0'))
		strncpy(to, p + 1, len);/* unsafe_function_ignore: strncpy */
	else
		strncpy(to, from, len);/* unsafe_function_ignore: strncpy */
}

static bool dubai_check_proc_compose_count(void)
{
	int count = atomic_read(&decompose_count);

	return (count > 0 && count == atomic_read(&decompose_count_target));
}

static void dubai_schedule_decompose_work(void)
{
	if (atomic_read(&decompose_check_retry) < DECOMPOSE_RETYR_MAX) {
		atomic_inc(&decompose_check_retry);
		schedule_delayed_work(&dubai_decompose_work, decompose_check_delay);
	}
}

static struct dubai_proc_decompose *dubai_find_decompose_entry(pid_t tgid)
{
	struct dubai_proc_decompose *entry;

	list_for_each_entry(entry, &dubai_proc_decompose_list, node) {
		if (entry->tgid == tgid)
			return entry;
	}

	return NULL;
}

static void dubai_remove_proc_decompose(pid_t tgid)
{
	struct dubai_proc_decompose *entry;

	entry = dubai_find_decompose_entry(tgid);
	if (entry != NULL) {
		DUBAI_LOGI("Remove decompose proc: %d, %s", tgid, entry->decompose.prefix);
		entry->tgid = -1;
		atomic_dec(&decompose_count);
		dubai_schedule_decompose_work();
	}
}

static pid_t dubai_find_tgid(uid_t t_uid, const char *t_comm)
{
	pid_t tgid = -1;
	uid_t uid;
	struct task_struct *task;

	rcu_read_lock();
	for_each_process(task) {
		if (task->flags & PF_KTHREAD || !dubai_task_alive(task))
			continue;

		uid = from_kuid_munged(current_user_ns(), task_uid(task));
		if (uid == t_uid && !CMP_TASK_COMM(task->comm, t_comm)) {
			DUBAI_LOGI("Get [%s] tgid: %d", t_comm, task->tgid);
			tgid = task->tgid;
			break;
		}
	}
	rcu_read_unlock();

	return tgid;
}

static void dubai_check_proc_decompose(void)
{
	pid_t tgid;
	struct dubai_proc_decompose *entry;

	if (dubai_check_proc_compose_count())
		return;

	list_for_each_entry(entry, &dubai_proc_decompose_list, node) {
		if (entry->tgid >= 0)
			continue;

		if (!CMP_TASK_COMM(entry->decompose.comm, KERNEL_NAME)) {
			tgid = KERNEL_TGID;
		} else {
			tgid = dubai_find_tgid(entry->decompose.uid, entry->decompose.comm);
		}
		if (tgid >= 0) {
			entry->tgid = tgid;
			atomic_inc(&decompose_count);
			DUBAI_LOGI("Add decompose proc: %d, %s", tgid, entry->decompose.prefix);
		}
	}
}

static void dubai_decompose_work_fn(struct work_struct *work)
{
	mutex_lock(&dubai_proc_lock);
	dubai_check_proc_decompose();

	if (!dubai_check_proc_compose_count()) {
		dubai_schedule_decompose_work();
	} else {
		atomic_set(&decompose_check_retry, 0);
		DUBAI_LOGI("Check process to decompose successfully, count: %d", atomic_read(&decompose_count));
	}
	mutex_unlock(&dubai_proc_lock);
}

static void dubai_clear_proc_decompose(void)
{
	struct dubai_proc_decompose *entry, *tmp;

	list_for_each_entry_safe(entry, tmp, &dubai_proc_decompose_list, node) {
		list_del_init(&entry->node);
		kfree(entry);
	}
	atomic_set(&decompose_count_target, 0);
	atomic_set(&decompose_count, 0);
	atomic_set(&decompose_check_retry, 0);
}

static int dubai_set_proc_decompose_list(int count, const struct dubai_decompose_info *data)
{
	int ret = 0, i;
	struct dubai_proc_decompose *entry;

	mutex_lock(&dubai_proc_lock);
	dubai_clear_proc_decompose();

	for (i = 0; i < count; i++) {
		entry = kzalloc(sizeof(struct dubai_proc_decompose), GFP_ATOMIC);
		if (entry == NULL) {
			ret = -ENOMEM;
			goto exit;
		}
		memcpy(&entry->decompose, data, sizeof(struct dubai_decompose_info));
		entry->decompose.comm[TASK_COMM_LEN - 1] = '\0';
		entry->decompose.prefix[PREFIX_LEN - 1] = '\0';
		entry->tgid = -1;
		list_add_tail(&entry->node, &dubai_proc_decompose_list);
		atomic_inc(&decompose_count_target);
		data++;
	}

exit:
	if (atomic_read(&decompose_count_target) > 0)
		dubai_schedule_decompose_work();
	mutex_unlock(&dubai_proc_lock);

	return 0;
}

int dubai_set_proc_decompose(void __user *argp)
{
	int ret = 0, size, count, length, remain;
	struct dev_transmit_t *transmit = NULL;

	if (!atomic_read(&proc_cputime_enable)
		|| strstr(current->comm, DUBAID_NAME) == NULL)
		return -EPERM;

	if (copy_from_user(&length, argp, sizeof(int)))
		return -EFAULT;

	count = length / (int)(sizeof(struct dubai_decompose_info));
	remain = length % (int)(sizeof(struct dubai_decompose_info));
	if ((count <= 0) || (count > DECOMPOSE_COUNT_MAX) || (remain != 0)) {
		DUBAI_LOGE("Invalid length, length: %d, count: %d, remain: %d", length, count, remain);
		return -EINVAL;
	}

	size = length + sizeof(struct dev_transmit_t);
	transmit = kzalloc(size, GFP_KERNEL);
	if (transmit == NULL)
		return -ENOMEM;

	if (copy_from_user(transmit, argp, size)) {
		ret = -EFAULT;
		goto exit;
	}

	ret = dubai_set_proc_decompose_list(count, (const struct dubai_decompose_info *)transmit->data);
	if (ret < 0)
		DUBAI_LOGE("Failed to set process decompose list");

exit:
	kfree(transmit);

	return ret;
}

/*
 * Create log entry to store process cputime structures
 */
static struct buffered_log_entry *dubai_create_log_entry(long long timestamp, int count, int type)
{
	long long size = 0;
	struct buffered_log_entry *entry = NULL;
	struct dubai_cputime_transmit *transmit = NULL;

	if ((count < 0) || (type >= PROCESS_STATE_INVALID)) {
		DUBAI_LOGE("Invalid parameter, count=%d, type=%d", count, type);
		return NULL;
	}

	/*
	 * allocate more space(BASE_COUNT)
	 * size = dubai_cputime_transmit size + dubai_cputime size * count
	 */
	count += BASE_COUNT;
	size = LOG_ENTRY_SIZE(count);
	entry = create_buffered_log_entry(size, BUFFERED_LOG_MAGIC_PROC_CPUTIME);
	if (entry == NULL) {
		DUBAI_LOGE("Failed to create buffered log entry");
		return NULL;
	}

	transmit = (struct dubai_cputime_transmit *)entry->data;
	transmit->timestamp = timestamp;
	transmit->type = type;
	transmit->count = count;

	return entry;
}

static void dubai_proc_entry_copy(unsigned char *value, const struct dubai_proc_entry *entry)
{
	struct dubai_cputime stat;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
	u64 total_time;
#else
	cputime_t total_time;
#endif

	total_time = entry->active_utime + entry->active_stime;
	total_time += entry->utime + entry->stime;

	memset(&stat, 0, sizeof(struct dubai_cputime));/* unsafe_function_ignore: memset */
	stat.uid = entry->uid;
	stat.pid = entry->tgid;
	stat.time = dubai_cputime_to_usecs(total_time);
#ifdef CONFIG_HUAWEI_DUBAI_TASK_CPU_POWER
	stat.power = entry->active_power + entry->power;
#endif
	stat.cmdline = entry->cmdline? CMDLINE_PROCESS : CMDLINE_NEED_TO_GET;
	dubai_copy_name(stat.name, entry->name);

	memcpy(value, &stat, sizeof(struct dubai_cputime));/* unsafe_function_ignore: memcpy */
}

static void dubai_thread_entry_copy(unsigned char *value, uid_t uid, const struct dubai_thread_entry *thread)
{
	struct dubai_cputime stat;

	memset(&stat, 0, sizeof(struct dubai_cputime));/* unsafe_function_ignore: memset */
	stat.uid = uid;
	stat.pid = thread->pid;
	stat.time = dubai_cputime_to_usecs(thread->utime + thread->stime);
#ifdef CONFIG_HUAWEI_DUBAI_TASK_CPU_POWER
	stat.power = thread->power;
#endif
	stat.cmdline = CMDLINE_THREAD;
	dubai_copy_name(stat.name, thread->name);

	memcpy(value, &stat, sizeof(struct dubai_cputime));/* unsafe_function_ignore: memcpy */
}

static struct dubai_thread_entry *dubai_find_or_register_thread(pid_t pid,
		const char *comm, struct dubai_proc_entry *entry)
{
	struct dubai_thread_entry *thread;
	struct dubai_proc_decompose *d_proc;

	d_proc = dubai_find_decompose_entry(entry->tgid);
	if (d_proc == NULL && list_empty(&entry->threads))
		return NULL;

	list_for_each_entry(thread, &entry->threads, node) {
		if (thread->pid == pid)
			return thread;
	}

	thread = kzalloc(sizeof(struct dubai_thread_entry), GFP_ATOMIC);
	if (thread == NULL) {
		DUBAI_LOGE("Failed to allocate memory");
		return NULL;
	}
	thread->pid = pid;
	thread->alive = true;
	if (d_proc != NULL)
		snprintf(thread->name, NAME_LEN - 1, "%s_%s", d_proc->decompose.prefix, comm);/* unsafe_function_ignore: snprintf */
	else
		snprintf(thread->name, NAME_LEN - 1, "%s_%s", entry->name, comm);/* unsafe_function_ignore: snprintf */

	list_add_tail(&thread->node, &entry->threads);
	atomic_inc(&active_count);

	return thread;
}

static struct dubai_proc_entry *dubai_find_proc_entry(pid_t pid, pid_t tgid, const char *comm,
		struct dubai_thread_entry **thread)
{
	struct dubai_proc_entry *entry;

	hash_for_each_possible(proc_hash_table, entry, hash, tgid) {
		if (entry->tgid == tgid) {
			*thread = dubai_find_or_register_thread(pid, comm, entry);
			return entry;
		}
	}

	return NULL;
}

static struct dubai_proc_entry *dubai_find_or_register_proc(const struct task_struct *task,
		struct dubai_thread_entry **thread)
{
	pid_t tgid = task->tgid;
	pid_t pid = task->pid;
	char comm[TASK_COMM_LEN] = {0};
	struct dubai_proc_entry *entry = NULL;
	struct task_struct *leader = task->group_leader;

	/* combine kernel thread to same pid_entry which pid is 0 */
	if (task->flags & PF_KTHREAD) {
		tgid = KERNEL_TGID;
		COPY_TASK_COMM(comm, KERNEL_NAME);
	} else {
		if (leader != NULL)
			COPY_TASK_COMM(comm, leader->comm);
		else
			COPY_TASK_COMM(comm, task->comm);
	}

	entry = dubai_find_proc_entry(pid, tgid, task->comm, thread);
	if (entry != NULL) {
		if (likely(!entry->cmdline))
			COPY_PROC_NAME(entry->name, comm);

		return entry;
	}

	entry = kzalloc(sizeof(struct dubai_proc_entry), GFP_ATOMIC);
	if (entry == NULL) {
		DUBAI_LOGE("Failed to allocate memory");
		return NULL;
	}
	entry->tgid = tgid;
	entry->uid = from_kuid_munged(current_user_ns(), task_uid(task));
	entry->alive = true;
	entry->cmdline = (tgid == KERNEL_TGID);
	COPY_PROC_NAME(entry->name, comm);
	INIT_LIST_HEAD(&entry->threads);
	*thread = dubai_find_or_register_thread(pid, task->comm, entry);

	hash_add(proc_hash_table, &entry->hash, tgid);
	atomic_inc(&active_count);

	return entry;
}

int dubai_update_proc_cputime(void)
{
	struct dubai_proc_entry *entry;
	struct dubai_thread_entry *thread;
	struct task_struct *task, *temp;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
	u64 utime;
	u64 stime;
#else
	cputime_t utime;
	cputime_t stime;
#endif

	rcu_read_lock();
	/* update active time from alive task */
	do_each_thread(temp, task) {
		/*
		 * check whether task is alive or not
		 * if not, do not record this task's cpu time
		 */
		if (!dubai_task_alive(task))
			continue;

		thread = NULL;
		entry = dubai_find_or_register_proc(task, &thread);
		if (entry == NULL) {
			rcu_read_unlock();
			DUBAI_LOGE("Failed to find the entry for process %d", task->tgid);
			return -ENOMEM;
		}
		task_cputime_adjusted(task, &utime, &stime);
		entry->active_utime += utime;
		entry->active_stime += stime;
#ifdef CONFIG_HUAWEI_DUBAI_TASK_CPU_POWER
		entry->active_power += task->cpu_power;
#endif
		if (thread != NULL && thread->alive) {
			thread->utime = utime;
			thread->stime = stime;
#ifdef CONFIG_HUAWEI_DUBAI_TASK_CPU_POWER
			thread->power = task->cpu_power;
#endif
		}
	} while_each_thread(temp, task);
	rcu_read_unlock();

	return 0;
}

/*
 * initialize hash list
 * report dead process and delete hash node
 */
static void dubai_clear_dead_entry(struct dubai_cputime_transmit *transmit)
{
	int max;
	bool decompose;
	unsigned char *value;
	unsigned long bkt;
	struct dubai_proc_entry *entry;
	struct hlist_node *tmp;
	struct dubai_thread_entry *thread, *temp;

	value = transmit->value;
	max = transmit->count;
	transmit->count = 0;

	hash_for_each_safe(proc_hash_table, bkt, tmp, entry, hash) {
		decompose = false;
		entry->active_stime = 0;
		entry->active_utime = 0;
#ifdef CONFIG_HUAWEI_DUBAI_TASK_CPU_POWER
		entry->active_power = 0;
#endif

		if (!list_empty(&entry->threads)) {
			decompose= true;
			list_for_each_entry_safe(thread, temp, &entry->threads, node) {
				if (thread->alive)
					continue;

				if (transmit->count < max
					&& (thread->utime + thread->stime) > 0) {
					dubai_thread_entry_copy(value, entry->uid, thread);
					value += sizeof(struct dubai_cputime);
					transmit->count++;
				}
				list_del_init(&thread->node);
				kfree(thread);
			}
		}

		if (entry->alive)
			continue;

		if (!decompose
			&& transmit->count < max
			&& (entry->utime + entry->stime) > 0) {
			dubai_proc_entry_copy(value, entry);
			value += sizeof(struct dubai_cputime);
			transmit->count++;
		}
		if (list_empty(&entry->threads)) {
			hash_del(&entry->hash);
			kfree(entry);
		}
	}
}

static int dubai_clear_and_update(long long timestamp)
{
	int ret = 0, count = 0;
	struct dubai_cputime_transmit *transmit;
	struct buffered_log_entry *entry;

	count = atomic_read(&dead_count);
	entry = dubai_create_log_entry(timestamp, count, PROCESS_STATE_DEAD);
	if (entry == NULL) {
		DUBAI_LOGE("Failed to create log entry for dead processes");
		return -ENOMEM;
	}
	transmit = (struct dubai_cputime_transmit *)(entry->data);

	mutex_lock(&dubai_proc_lock);

	dubai_clear_dead_entry(transmit);
	atomic_set(&dead_count, 0);
	dubai_check_proc_decompose();

	ret = dubai_update_proc_cputime();
	if (ret < 0) {
		DUBAI_LOGE("Failed to update process cpu time");
		mutex_unlock(&dubai_proc_lock);
		goto exit;
	}
	mutex_unlock(&dubai_proc_lock);

	if (transmit->count > 0) {
		entry->length = LOG_ENTRY_SIZE(transmit->count);
		ret = send_buffered_log(entry);
		if (ret < 0)
			DUBAI_LOGE("Failed to send dead process log entry");
	}

exit:
	free_buffered_log_entry(entry);

	return ret;
}

static int dubai_send_active_process(long long timestamp)
{
	int ret = 0, max = 0;
	unsigned char *value;
	unsigned long bkt;
	struct dubai_proc_entry *entry;
	struct dubai_cputime_transmit *transmit;
	struct buffered_log_entry *log_entry;
	struct dubai_thread_entry *thread;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
	u64 active_time;
#else
	cputime_t active_time;
#endif

	max = atomic_read(&active_count);
	log_entry = dubai_create_log_entry(timestamp, max, PROCESS_STATE_ACTIVE);
	if (log_entry == NULL) {
		DUBAI_LOGE("Failed to create log entry for active processes");
		return -ENOMEM;
	}
	transmit = (struct dubai_cputime_transmit *)(log_entry->data);
	value = transmit->value;
	max = transmit->count;
	transmit->count = 0;

	mutex_lock(&dubai_proc_lock);
	hash_for_each(proc_hash_table, bkt, entry, hash) {
		active_time = entry->active_utime + entry->active_stime;
		if (active_time == 0 || !entry->alive)
			continue;

		if (transmit->count >= max)
			break;

		if (list_empty(&entry->threads)) {
			dubai_proc_entry_copy(value, entry);
			value += sizeof(struct dubai_cputime);
			transmit->count++;
			continue;
		}

		list_for_each_entry(thread, &entry->threads, node) {
			if ((thread->utime + thread->stime) == 0 || !thread->alive)
				continue;

			if (transmit->count >= max)
				break;

			dubai_thread_entry_copy(value, entry->uid, thread);
			value += sizeof(struct dubai_cputime);
			transmit->count++;
		}
	}
	mutex_unlock(&dubai_proc_lock);

	if (transmit->count > 0) {
		log_entry->length = LOG_ENTRY_SIZE(transmit->count);
		ret = send_buffered_log(log_entry);
		if (ret < 0)
			DUBAI_LOGE("Failed to send active process log entry");
	}
	free_buffered_log_entry(log_entry);

	return ret;
}

/*
 * if there are dead processes in the list,
 * we should clear these dead processes
 * in case of pid reused
 */
int dubai_get_proc_cputime(void __user *argp)
{
	int ret = 0;
	long long timestamp;

	if (!atomic_read(&proc_cputime_enable))
		return -EPERM;

	ret = get_timestamp_value(argp, &timestamp);
	if (ret < 0) {
		DUBAI_LOGE("Failed to get timestamp");
		goto exit;
	}

	ret = dubai_clear_and_update(timestamp);
	if (ret < 0) {
		DUBAI_LOGE("Failed to clear dead process and update list");
		goto exit;
	}

	ret = dubai_send_active_process(timestamp);
	if (ret < 0) {
		DUBAI_LOGE("Failed to send active process cpu time");
	}

exit:
	return ret;
}

int dubai_get_process_name(struct process_name *process)
{
	struct task_struct *task, *leader;
	char cmdline[MAX_CMDLINE_LEN] = {0};
	int ret = 0, compare = 0;

	if (process == NULL || process->pid <= 0) {
		DUBAI_LOGE("Invalid parameter");
		return -EINVAL;
	}

	rcu_read_lock();
	task = find_task_by_vpid(process->pid);
	if (task)
		get_task_struct(task);
	rcu_read_unlock();
	if (!task)
		return -EFAULT;

	leader = task->group_leader;
	compare = CMP_TASK_COMM(process->comm, task->comm);
	if (compare != 0 && leader != NULL)
		compare = CMP_TASK_COMM(process->comm, leader->comm);

	ret = get_cmdline(task, cmdline, MAX_CMDLINE_LEN - 1);
	cmdline[MAX_CMDLINE_LEN - 1] = '\0';
	if (ret > 0 && (compare == 0 || strstr(cmdline, process->comm) != NULL))
		dubai_copy_name(process->name, cmdline);

	put_task_struct(task);

	return ret;
}

int dubai_get_proc_name(void __user *argp)
{
	int ret = 0, size;
	struct process_name *process = NULL;
	struct dev_transmit_t *transmit = NULL;

	size = sizeof(struct dev_transmit_t)
		+ sizeof(struct process_name);
	transmit = kzalloc(size, GFP_KERNEL);
	if (transmit == NULL) {
		DUBAI_LOGE("Failed to allocate memory for process name");
		return -ENOMEM;
	}
	if (copy_from_user(transmit, argp, size)) {
		ret = -EFAULT;
		goto exit;
	}

	process = (struct process_name *)transmit->data;
	process->comm[TASK_COMM_LEN - 1] = '\0';
	process->name[NAME_LEN - 1] = '\0';
	dubai_get_process_name(process);
	if (copy_to_user(argp, transmit, size))
		ret = -EFAULT;

exit:
	kfree(transmit);

	return ret;
}

static bool is_same_process(const struct dubai_proc_entry *entry, const struct task_struct *task)
{
	uid_t uid;

	uid = from_kuid_munged(current_user_ns(), task_uid(task));

	// task with same pid has already died before OR it's not in same uid
	if (entry->tgid == task->pid || uid != entry->uid)
		return false;

	if (dubai_task_alive(task->group_leader)
		&& (strstr(entry->name, task->comm) == NULL)
		&& (strstr(entry->name, task->group_leader->comm) == NULL))
		return false;

	return true;
}

static int dubai_process_notifier(struct notifier_block *self, unsigned long cmd, void *v)
{
	int ret;
	bool got_cmdline = false;
	struct task_struct *task = v;
	pid_t tgid, pid;
	struct task_struct *leader = NULL;
	struct dubai_proc_entry *entry = NULL;
	struct dubai_thread_entry *thread = NULL;
	char cmdline[MAX_CMDLINE_LEN] = {0};
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
	u64 utime, stime;
#else
	cputime_t utime, stime;
#endif

	if (task == NULL || !atomic_read(&proc_cputime_enable))
		return NOTIFY_OK;

	tgid = task->tgid;
	pid = task->pid;
	leader = task->group_leader;

	/* kernel thread, we assume tgid is 0 */
	if (task->flags & PF_KTHREAD)
		tgid = KERNEL_TGID;

	if (tgid == task->pid) {
		ret = get_cmdline(task, cmdline, MAX_CMDLINE_LEN - 1);
		cmdline[MAX_CMDLINE_LEN - 1] = '\0';
		if (ret > 0 && strlen(cmdline) > 0)
			got_cmdline = true;
	}

	mutex_lock(&dubai_proc_lock);

	if (task->tgid == task->pid || dubai_task_alive(leader)) {
		entry = dubai_find_or_register_proc(task, &thread);
	} else {
		entry = dubai_find_proc_entry(pid, tgid, task->comm, &thread);
	}

	if (entry == NULL)
		goto exit;

	if (got_cmdline) {
		dubai_copy_name(entry->name, cmdline);
		entry->cmdline = true;
	}

	task_cputime_adjusted(task, &utime, &stime);
	if (!entry->alive && !is_same_process(entry, task))
		goto update_thread;

	entry->utime += utime;
	entry->stime += stime;
#ifdef CONFIG_HUAWEI_DUBAI_TASK_CPU_POWER
	if (task->cpu_power != ULLONG_MAX) {
		entry->power += task->cpu_power;
	}
#endif

	/* process has died */
	if (entry->tgid == task->pid && entry->alive) {
		entry->alive = false;
		atomic_dec(&active_count);
		atomic_inc(&dead_count);
		if (!list_empty(&entry->threads))
			dubai_remove_proc_decompose(entry->tgid);
	}

update_thread:
	if (thread != NULL && thread->alive) {
		thread->utime = utime;
		thread->stime = stime;
#ifdef CONFIG_HUAWEI_DUBAI_TASK_CPU_POWER
		if (task->cpu_power != ULLONG_MAX) {
			thread->power = task->cpu_power;
		}
#endif
		thread->alive = false;
		atomic_dec(&active_count);
		atomic_inc(&dead_count);
	}

exit:
	mutex_unlock(&dubai_proc_lock);

	return NOTIFY_OK;
}

static struct notifier_block process_notifier_block = {
	.notifier_call	= dubai_process_notifier,
	.priority = INT_MAX,
};

static void dubai_proc_cputime_reset(void)
{
	unsigned long bkt;
	struct dubai_proc_entry *entry;
	struct hlist_node *tmp;
	struct dubai_thread_entry *thread, *temp;

	mutex_lock(&dubai_proc_lock);

	hash_for_each_safe(proc_hash_table, bkt, tmp, entry, hash) {
		list_for_each_entry_safe(thread, temp, &entry->threads, node) {
			list_del_init(&thread->node);
			kfree(thread);
		}
		hash_del(&entry->hash);
		kfree(entry);
	}
	dubai_clear_proc_decompose();
	atomic_set(&dead_count, 0);
	atomic_set(&active_count, 0);
	atomic_set(&proc_cputime_enable, 0);
	hash_init(proc_hash_table);

	mutex_unlock(&dubai_proc_lock);
}

int dubai_proc_cputime_enable(void __user *argp)
{
	int ret;
	bool enable;

	ret = get_enable_value(argp, &enable);
	if (ret == 0) {
		dubai_proc_cputime_reset();
		atomic_set(&proc_cputime_enable, enable ? 1 : 0);
		DUBAI_LOGI("Dubai cpu process stats enable: %d", enable ? 1 : 0);
	}

	return ret;
}

int dubai_get_task_cpupower_enable(void __user *argp) {
	bool enable;

	enable = !!atomic_read(&task_power_enable);
	if (copy_to_user(argp, &enable, sizeof(bool)))
		return -EFAULT;

	return 0;
}

void dubai_proc_cputime_init(void)
{
	dubai_proc_cputime_reset();
	profile_event_register(PROFILE_TASK_EXIT, &process_notifier_block);
}

void dubai_proc_cputime_exit(void)
{
	profile_event_unregister(PROFILE_TASK_EXIT, &process_notifier_block);
	dubai_proc_cputime_reset();
}
