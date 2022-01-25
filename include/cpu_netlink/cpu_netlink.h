#include <linux/netlink.h>
#include <net/sock.h>
#include <linux/version.h>
#include <linux/module.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,14,0)
#include <linux/sched/task.h>
#endif

enum sock_no {
    CPU_HIGH_LOAD = 1,
    PROC_FORK = 2,
    PROC_COMM = 3
};

int send_to_user(int sock_no, size_t len, int *data);
void iaware_proc_fork_connector(struct task_struct *task);
void iaware_proc_comm_connector(struct task_struct *task, const char* comm);
