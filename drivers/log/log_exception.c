/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2018. All rights reserved.
 *
 * Description: drivers to write messages to exception node
 *      Author: wangtanyun <wangtanyun@huawei.com>
 */

#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/uio.h>

#include <log/hw_log.h>
#include <log/log_exception.h>

#define HWLOG_TAG	log_exception
HWLOG_REGIST();

static int CHECK_CODE = 0x7BCDABCD;

/*
 * log_to_exception - write command to /dev/log/exception
 * @tag: tag of the command
 * @msg: command string to write
 * Returns number of bytes written or error code
 */
int log_to_exception(char *tag, char *msg)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
	struct iov_iter iter;
#else
	mm_segment_t oldfs;
#endif
	struct file *filp;
	unsigned char prio_err = 6;	//ANDROID_ERROR
	int ret = 0;
	struct iovec vec[5];
	unsigned long vcount = 0;

	if (tag == NULL || msg == NULL) {
		hwlog_err("%s: invalid arguments\n", __func__);
		return -EINVAL;
	}

	hwlog_info("%s: exception tag '%s' msg '%s'", __func__, tag, msg);

	filp = filp_open(LOG_EXCEPTION_FS, O_RDWR, 0);
	if (filp == NULL || IS_ERR(filp)) {
		hwlog_err("%s: access '%s' failed.\n",
				__func__, LOG_EXCEPTION_FS);
		return -ENODEV;
	}

	vcount = 0;
	vec[vcount].iov_base  = &CHECK_CODE;
	vec[vcount++].iov_len = sizeof(CHECK_CODE);
	vec[vcount].iov_base  = &prio_err;
	vec[vcount++].iov_len = 1;
	vec[vcount].iov_base  = tag;
	vec[vcount++].iov_len = strlen(tag) + 1;
	vec[vcount].iov_base  = msg;
	vec[vcount++].iov_len = strlen(msg) + 1;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
	iov_iter_init(&iter, WRITE, vec, vcount, iov_length(vec, vcount));
	ret = vfs_iter_write(filp, &iter, &filp->f_pos, 0);
#else
	oldfs = get_fs();
	set_fs(KERNEL_DS);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 0))
	ret = vfs_writev(filp, vec, vcount, &filp->f_pos, 0);
#else
	ret = vfs_writev(filp, vec, vcount, &filp->f_pos);
#endif
	set_fs(oldfs);
#endif
	if (ret < 0) {
		hwlog_err("%s: write '%s' failed: %d\n",
				__func__, LOG_EXCEPTION_FS, ret);
		filp_close(filp, NULL);
		return -EIO;
	}
	filp_close(filp, NULL);
	return ret;
}
EXPORT_SYMBOL(log_to_exception);

int logbuf_to_exception(char category, int level, char log_type,
			char sn, void *msg, int msglen)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
	struct iov_iter iter;
#else
	mm_segment_t oldfs;
#endif
	struct file *filp = NULL;
	int ret = 0;
	struct idapheader idaphdr;
	struct iovec vec[5];
	unsigned long vcount = 0;

	if (msg == NULL || msglen < 0) {
		hwlog_err("%s: invalid arguments\n", __func__);
		return -EINVAL;
	}

	hwlog_info("%s: exception msg '%s'", __func__, (char *)msg);

	filp = filp_open(LOG_EXCEPTION_FS, O_RDWR, 0);

	if (filp == NULL || IS_ERR(filp)) {
		hwlog_err("%s: access '%s' failed.\n",
				__func__, LOG_EXCEPTION_FS);
		return -ENODEV;
	}

	idaphdr.level    = level;
	idaphdr.category = category;
	idaphdr.log_type = log_type;
	idaphdr.sn       = sn;

	vcount = 0;
	vec[vcount].iov_base  = &CHECK_CODE;
	vec[vcount++].iov_len = sizeof(CHECK_CODE);
	vec[vcount].iov_base  = (void *)&idaphdr;
	vec[vcount++].iov_len = sizeof(idaphdr);
	vec[vcount].iov_base  = msg;
	vec[vcount++].iov_len = msglen;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
	iov_iter_init(&iter, WRITE, vec, vcount, iov_length(vec, vcount));
	ret = vfs_iter_write(filp, &iter, &filp->f_pos, 0);
#else
	oldfs = get_fs();
	set_fs(KERNEL_DS);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 0))
	ret = vfs_writev(filp, vec, vcount, &filp->f_pos, 0);
#else
	ret = vfs_writev(filp, vec, vcount, &filp->f_pos);
#endif
	set_fs(oldfs);
#endif
	if (ret < 0) {
		hwlog_err("%s: write '%s' failed: %d\n",
				__func__, LOG_EXCEPTION_FS, ret);
		filp_close(filp, NULL);
		return -EIO;
	}

	filp_close(filp, NULL);
	return ret;
}
EXPORT_SYMBOL(logbuf_to_exception);

