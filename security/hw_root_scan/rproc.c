/*
 * rproc.c
 *
 * the rproc.c for root processes list checking
 *
 * Yongzheng Wu <Wu.Yongzheng@huawei.com>
 * likun <quentin.lee@huawei.com>
 * likan <likan82@huawei.com>
 *
 * Copyright (c) 2001-2021, Huawei Tech. Co., Ltd. All rights reserved.
 */

#include <linux/dcache.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/oom.h> /* for find_lock_task_mm */
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/sort.h>
#include <linux/string.h>
#include "./include/hw_rscan_utils.h"
#include <linux/version.h>

#define MAX_PROC_NUM 128 /* 128 root processes should be enough for anybody */

static const char *TAG = "rproc";

/*
 * return number of tokens
 * Note, paths will be modified, just like strtok
 */
static int paths_explode(char **const tokens, char *paths)
{
	int ntoken = 0;
	int i = 0;

	tokens[ntoken++] = paths;
	while ((paths + i) && *(paths + i)) {
		if (paths[i] == ':') {
			paths[i] = '\0';
			if (ntoken >= MAX_PROC_NUM) {
				RSLogError(TAG, "paths_explode overflow");
				break;
			}

			if ((paths + i + 1) && *(paths + i + 1)) {
				tokens[ntoken++] = paths + i + 1;
			} else {
				RSLogError(TAG, "white list formate abnormal");
				break;
			}
		}
		i++;
	}

	return ntoken;
}

static void paths_implode(char *paths,  char **const tokens, int ntoken)
{
	int ti;

	if (ntoken == 0) {
		paths[0] = '\0';
		return;
	}

	for (ti = 0; ti < ntoken; ti++)
		paths += sprintf(paths, ti == 0 ? "%s" : ":%s", tokens[ti]);
}

static int tokens_cmp(const void *p1, const void *p2)
{
	char *const *i1 = p1;
	char *const *i2 = p2;

	return strcmp(*i1, *i2);
}

static char *g_whitelist_proc[MAX_PROC_NUM] = { NULL };
static int g_whitelist_count;
static char *g_whitelist_proc_buf;

bool is_dup_token(char **const tokens, int index)
{
	/* we should delete empty token */
	if (tokens[index][0]  == '\0')
		return true;

	/* the current token is same with the last, so it's dup token */
	if ((index > 0) && (strcmp(tokens[index - 1], tokens[index]) == 0))
		return true;

	return false;
}

/*
 * "::sbin/c:sbin/a:sbin/b::sbin/a"  ->   [sbin/a][sbin/b][sbin/c]
 */
static bool format_rprocs(char **const tokens, int *ntoken, char *whitelist)
{
	int cur = 0;
	int token_count = 0;

	if (NULL == tokens || NULL == whitelist || NULL == ntoken)
		return false;

	token_count = paths_explode(tokens, whitelist);

	sort(tokens, (ulong)token_count, sizeof(char *), tokens_cmp, NULL);

	/* delete dup */
	for (cur = 0; cur < token_count; cur++) {
		if (is_dup_token(tokens, cur)) {
			int next = cur + 1;

			if (next < token_count) {
				memmove(tokens + cur, tokens + next,
				(ulong)((token_count - next) * sizeof(char *)));
			}
			cur--;
			token_count--;
		}
	}
	(*ntoken) = token_count;

	return true;
}

bool init_rprocs_whitelist(const char *whitelist)
{
	if (NULL == g_whitelist_proc_buf) {

		/* never freed */
		g_whitelist_proc_buf = (char *)__get_free_page(GFP_KERNEL);

		if (NULL != g_whitelist_proc_buf)
			strncpy(g_whitelist_proc_buf, whitelist, PAGE_SIZE);
		else
			return false;
	}
	return format_rprocs(g_whitelist_proc, &g_whitelist_count,
							g_whitelist_proc_buf);
}

bool find_proc_in_init_list(const char *proc_name)
{
	/* use dichotomization */
	int begin = 0;
	int end = g_whitelist_count - 1;

	if (proc_name == NULL)
		return false;

	while (begin <= end) {
		int pos = (begin + end) / 2;
		int ret = strcmp(proc_name, g_whitelist_proc[pos]);

		if (ret == 0) {
			return true;
		} else if (ret < 0) {
			/* proc_name < current */
			end = pos - 1;
		} else {
			/* proc_name > current */
			begin = pos + 1;
		}
	}
	return false;
}

/*
 * "::sbin/c:sbin/a:sbin/b::sbin/a"  ->   "sbin/a:sbin/b:sbin/c"
 */
void _rprocs_strip_whitelist(char **const tokens, char **final_tokens,
					char *rprocs, ssize_t rprocs_len)
{
	int ntoken = 0;
	int final_ntoken = 0;
	int i = 0;
	char *tmp = NULL;

	if (NULL == tokens || NULL == final_tokens
			|| NULL == rprocs || rprocs_len > MAX_RPROC_SIZE) {
		RSLogError(TAG, "strip white list failed, input value invalid");
		return;
	}

	tmp = vmalloc((ulong)rprocs_len);
	if (NULL == tmp) {
		RSLogError(TAG, "rprocs_strip_whitelist failed, out of memory");
		return;
	}
	memcpy(tmp, rprocs, rprocs_len);

	if (!format_rprocs(tokens, &ntoken, tmp))
		RSLogError(TAG, "strip white list failed, format_rprocs error");

	/* strip whitelist */
	for (i = 0; i < ntoken; i++) {
		if (!find_proc_in_init_list(tokens[i]))
			final_tokens[final_ntoken++] = tokens[i];
	}

	/* reconstruct */
	paths_implode(rprocs, final_tokens, final_ntoken);
	vfree(tmp);
	tmp = NULL;
}

/*
 * remove tokens that are in rprocs_whitelist, also sort and remove duplicates
 * Example:
 * input: "a:d:a:c:d:b"
 * rprocs_whitelist: "a:c"
 * output: "b:d"
 */
void rprocs_strip_whitelist(char *rprocs, ssize_t rprocs_len)
{
	char **tokens = NULL;
	char **final_tokens = NULL;

	tokens = vmalloc((ulong)MAX_PROC_NUM * sizeof(char *));
	if (NULL == tokens) {
		RSLogError(TAG, "no enough memory for allocation");
		return;
	}

	final_tokens = vmalloc((ulong)MAX_PROC_NUM * sizeof(char *));
	if (NULL == final_tokens) {
		RSLogError(TAG, "no enough memory for allocation");
		vfree(tokens);
		tokens = NULL;
		return;
	}

	_rprocs_strip_whitelist(tokens, final_tokens, rprocs, rprocs_len);

	vfree(tokens);
	tokens = NULL;

	vfree(final_tokens);
	final_tokens = NULL;
}

/* For pre-4.1 kernel, calling get_mm_exe_file causes kernel crash, because
   we are holding task_lock and get_mm_exe_file calls down_read() which calls
   might_sleep(). We reimplement get_mm_exe_file and use down_read_trylock()
   instead of down_read().
   Since kernel 4.1, get_mm_exe_file() start to use RCU instead of semaphore.
   We are safe to call it.
   The exact commit is 90f31d0ea88880f780574f3d0bb1a227c4c66ca3.
*/
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0)
#define my_get_mm_exe_file(mm) get_mm_exe_file(mm)
#else
static struct file *my_get_mm_exe_file(struct mm_struct *mm)
{
	struct file *exe_file;

	if (!down_read_trylock(&mm->mmap_sem))
		return NULL;
	exe_file = mm->exe_file;
	if (exe_file)
		get_file(exe_file);
	up_read(&mm->mmap_sem);
	return exe_file;
}
#endif

/*
 * get the path of the process' execute file.
 * refer to fs/proc/base.c proc_exe_link() and do_proc_readlink()
 * return length of the path string.
 * return -1 if outlen is not enough
 * return 0 on other failure
 */
static int get_task_exe(struct mm_struct *mm, pid_t pid, char *out,
					int outlen, char *dbuf, int dbuflen)
{
	struct file *exe_file;
	char *pathname = NULL;
	char *blank_spaces = NULL;
	size_t len = 0;

	if (NULL == mm || NULL == out || NULL == dbuf) {
		RSLogError(TAG, "input parameter invalid");
		return -EINVAL;
	}
	if (NULL == &mm->mmap_sem)
		return -EINVAL;
	exe_file = my_get_mm_exe_file(mm);

	if (!exe_file) {
		RSLogError(TAG, "get mm exe file %d failed", pid);
		return 0;
	}
	pathname = d_path(&exe_file->f_path, dbuf, dbuflen);
	fput(exe_file);
	if (pathname == NULL || IS_ERR(pathname)) {
		RSLogError(TAG, "get task exe d_path %d failed", pid);
		return 0;
	}

	blank_spaces = strchr(pathname, ' ');
	if (blank_spaces != NULL) { /* handle "/path/file (deleted)" */
		blank_spaces[0] = '\0';
	}

	len = strlen(pathname);
	if (len >= SIZE_MAX) {
		RSLogError(TAG, "value of len larger than SIZE_MAX");
		return 0;
	}

	if (outlen < len) {
		RSLogWarning(TAG, "get task exe path too long \"%s\" > %d",
							pathname, outlen);
		return -1;
	}

	memcpy(out, pathname, len);

	return len;
}

int get_root_procs(char *out, size_t outlen)
{
	char *tmp = NULL;
	struct task_struct *task;
	int pos = 0;

	if (NULL == out || outlen <= 0) {
		RSLogError(TAG, "input parameter invalid");
		return -EINVAL;
	}

	tmp = (char *)__get_free_page(GFP_TEMPORARY);
	if (tmp == NULL) {
		RSLogError(TAG, "get free page failed");
		return -EINVAL;
	}

	read_lock(&tasklist_lock);
	for_each_process(task) { /*lint -save -e550 */
		int len;
		struct task_struct *t;

		/*
		 * last char must be '\0',
		 * so the invalid space for the process name must -1
		 */
		if (pos >= outlen - 1)
			break;

		if ((task->flags & PF_KTHREAD)
				|| task->mm == NULL
				|| from_kuid_munged(&init_user_ns,
							task->cred->euid) != 0)
			continue;

		/* follow mm/oom_kill.c:dump_tasks */
		t = find_lock_task_mm(task);
		if (!t || !t->mm)
			continue;

		len = get_task_exe(t->mm, t->pid, out + pos,
					outlen - pos - 1, tmp, PAGE_SIZE);
		task_unlock(t);

		if (len < 0)
			break;

		if (len == 0)
			continue;

		pos += len;

		/* add split char */
		out[pos++] = ':';
	}
	read_unlock(&tasklist_lock);
	free_page((unsigned long)tmp);

	/* replace last split char to '\0' */
	if (pos > 0) {
		pos--;
		out[pos] = '\0';
	}

	return pos;
}
