/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: Qos schedule implementation
 * Author: JiangXiaofeng jiangxiaofeng8@huawei.com
 * Create: 2019-03-01
 */

#include <chipset_common/hwqos/iaware_qos.h>

#include <linux/module.h>
#include <linux/device.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#ifdef CONFIG_COMPAT
#include <linux/compat.h>
#endif
#include <chipset_common/hwqos/hwqos_common.h>

static long qos_ctrl_get_qos_stat(unsigned long arg)
{
	struct task_struct *tsk = NULL;
	struct qos_stat qs;
	void __user *uarg = (void __user *)arg;
	long ret = 0;

	if (!uarg)
		return -EINVAL;

	if (copy_from_user(&qs, uarg, sizeof(qs))) {
		pr_err("QOS_CTRL_GET_QOS_STAT: failed to copy from user\n");
		return -EFAULT;
	}
	// get qos
	rcu_read_lock();
	if (qs.pid) {
		tsk = find_task_by_vpid(qs.pid);
		if (!tsk)
			ret = -EFAULT;
		else
			qs.qos = get_task_qos(tsk);
	}
	rcu_read_unlock();

	if (copy_to_user(uarg, &qs, sizeof(qs))) {
		pr_err("QOS_CTRL_GET_QOS_STAT: failed to copy to user\n");
		return -EFAULT;
	}

	return ret;
}

static long qos_ctrl_set_qos_stat(unsigned long arg)
{
	struct task_struct *tsk = NULL;
	struct qos_stat qs;
	void __user *uarg = (void __user *)arg;
	long ret = 0;

	if (!uarg)
		return -EINVAL;

	if (copy_from_user(&qs, uarg, sizeof(qs))) {
		pr_err("QOS_CTRL_SET_QOS_STAT: failed to copy from user\n");
		return -EFAULT;
	}

	if ((!qs.pid) || (qs.qos < VALUE_QOS_LOW)
		|| (qs.qos > VALUE_QOS_CRITICAL)) {
		pr_err("QOS_CTRL_SET_QOS_STAT: bad parameter\n");
		return -EINVAL;
	}
	// set qos
	rcu_read_lock();
	if (qs.pid) {
		tsk = find_task_by_vpid(qs.pid);
		if (!tsk)
			ret = -EFAULT;
		else
			set_task_qos(tsk, qs.qos);
	}
	rcu_read_unlock();

	return ret;
}

static long qos_ctrl_get_qos_whole(unsigned long arg)
{
	struct task_struct *tsk = NULL;
	struct qos_whole qs_whole;
	void __user *uarg = (void __user *)arg;
	long ret = 0;

	if (!uarg)
		return -EINVAL;

	if (copy_from_user(&qs_whole, uarg, sizeof(qs_whole))) {
		pr_err("QOS_CTRL_GET_QOS_WHOLE: failed to copy from user\n");
		return -EFAULT;
	}
	// get qos whole
	rcu_read_lock();
	if (qs_whole.pid) {
		tsk = find_task_by_vpid(qs_whole.pid);
		if (!tsk) {
			ret = -EFAULT;
		} else {
			qs_whole.dynamic_qos =
				atomic_read(&tsk->dynamic_qos);
			qs_whole.trans_flags =
				atomic_read(&tsk->trans_flags);
		}
	}
	rcu_read_unlock();

	if (copy_to_user(uarg, &qs_whole, sizeof(qs_whole))) {
		pr_err("QOS_CTRL_GET_QOS_WHOLE: failed to copy to user\n");
		return -EFAULT;
	}

	return ret;
}

static long qos_ctrl_ioctl(struct file *file,
	unsigned int cmd, unsigned long arg)
{
	long ret = 0;

	if (unlikely(!QOS_SCHED_SET_ENABLE))
		return ret;

	if (_IOC_TYPE(cmd) != QOS_CTRL_MAGIC) {
		pr_err("qos_ctrl: invalid magic number. type = %d\n",
			_IOC_TYPE(cmd));
		return -EINVAL;
	}

	if (_IOC_NR(cmd) > QOS_CTRL_MAX_NR) {
		pr_err("qos_ctrl: invalid qos cmd. cmd = %d\n", _IOC_NR(cmd));
		return -EINVAL;
	}

	switch (cmd) {
	case QOS_CTRL_GET_QOS_STAT:
		ret = qos_ctrl_get_qos_stat(arg);
		break;
	case QOS_CTRL_SET_QOS_STAT:
		ret = qos_ctrl_set_qos_stat(arg);
		break;
	case QOS_CTRL_GET_QOS_WHOLE:
		ret = qos_ctrl_get_qos_whole(arg);
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

#ifdef CONFIG_COMPAT
static long qos_ctrl_compat_ioctl(struct file *file,
	unsigned int cmd, unsigned long arg)
{
	/*lint -e712*/
	return qos_ctrl_ioctl(file, cmd, (unsigned long)(compat_ptr(arg)));
	/*lint +e712*/
}
#endif

static int qos_ctrl_open(struct inode *inode, struct file *file)
{
	return 0;
}

static int qos_ctrl_release(struct inode *inode, struct file *file)
{
	return 0;
}

static const struct file_operations qos_ctrl_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = qos_ctrl_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = qos_ctrl_compat_ioctl,
#endif
	.open = qos_ctrl_open,
	.release = qos_ctrl_release,
};

static struct miscdevice qos_ctrl_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "iaware_qos_ctrl",
	.fops = &qos_ctrl_fops,
};

static int __init qos_ctrl_dev_init(void)
{
	int err;

	err = misc_register(&qos_ctrl_device);
	if (err)
		return err;
	return 0;
}

static void __exit qos_ctrl_dev_exit(void)
{
	misc_deregister(&qos_ctrl_device);
}

module_init(qos_ctrl_dev_init);
module_exit(qos_ctrl_dev_exit);
