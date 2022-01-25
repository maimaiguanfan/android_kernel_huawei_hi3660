/*
 * audio codec rdr.
 *
 * Copyright (c) 2015 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/kernel.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/uaccess.h>
#include <linux/kthread.h>
#include <linux/thread_info.h>
#include <linux/slab.h>
#include <linux/wakelock.h>
#include <linux/vmalloc.h>

#include <linux/hisi/util.h>
#include <linux/hisi/rdr_hisi_ap_hook.h>

#include <linux/hisi/hi64xx_hifi_misc.h>
#include <linux/hisi/hi64xx/hi64xx_dsp_regs.h>

#include "rdr_print.h"
#include "rdr_hisi_audio_adapter.h"
#include "rdr_hisi_audio_codec.h"

/*lint -e750 -e730*/

struct rdr_codec_des_s {
	u32 modid;
	char *pathname;
	pfn_cb_dump_done dumpdone_cb;

	struct semaphore handler_sem;
	struct semaphore dump_sem;
	struct task_struct *kdump_task;
	struct task_struct *khandler_task;
	struct wake_lock rdr_wl;
};
static struct rdr_codec_des_s codec_des;

#define MODID_AP_S_PANIC_AUDIO_CODEC 0x80000029 /* must be same with rdr_hisi_platform.h */
#define CODEC_ERR_RECORD_FILE "/data/hisi_logs/codecerr_times.log"
#define CODEC_ERR_REBOOT_THRESHOLD 3

static unsigned char rdr_audio_get_codec_err_times(void)
{
	struct file *fp;
	ssize_t red_len;
	unsigned char times  = 0;
	mm_segment_t fs = 0;

	fp = filp_open(CODEC_ERR_RECORD_FILE, O_RDONLY, 0660);
	if (IS_ERR(fp)) {
		BB_PRINT_PN("open %s fail\n", CODEC_ERR_RECORD_FILE);
		return 0;
	}

	fs = get_fs();
	set_fs(KERNEL_DS);/*lint !e501*/

	vfs_llseek(fp, 0L, SEEK_SET);
	red_len = vfs_read(fp, &times, sizeof(times), &fp->f_pos);
	if (red_len == 0) {
		times = 0;
	}

	set_fs(fs);
	filp_close(fp, NULL);

	return times;
}

static void rdr_audio_set_codec_err_times(unsigned char times)
{
	struct file *fp;
	ssize_t write_len;
	char buf  = 0;
	mm_segment_t fs = 0;

	fp = filp_open(CODEC_ERR_RECORD_FILE, O_CREAT | O_RDWR, 0660);
	if (IS_ERR(fp)) {
		BB_PRINT_ERR("open %s fail\n", CODEC_ERR_RECORD_FILE);
		return;
	}

	fs = get_fs();
	set_fs(KERNEL_DS);/*lint !e501*/

	vfs_llseek(fp, 0L, SEEK_SET);
	write_len = vfs_write(fp, &times, sizeof(times), &(fp->f_pos));
	if (write_len == sizeof(buf)) {
		vfs_fsync(fp, 0);
	}

	set_fs(fs);
	filp_close(fp, NULL);

	return;
}

void rdr_audio_clear_reboot_times()
{
	if (rdr_audio_get_codec_err_times() > 0) {
		BB_PRINT_PN("have rebooted before, clear count\n");
		rdr_audio_set_codec_err_times(0);
	}
}

void rdr_audio_codec_err_process()
{
	unsigned char err_times;

	err_times = rdr_audio_get_codec_err_times();
	err_times++;
	if (err_times <= CODEC_ERR_REBOOT_THRESHOLD) {
		BB_PRINT_ERR("reboot for codec error %u times, reboot now\n", err_times);
		rdr_audio_set_codec_err_times(err_times);
		rdr_syserr_process_for_ap((u32)MODID_AP_S_PANIC_AUDIO_CODEC, 0UL, 0UL);
	} else {
		BB_PRINT_ERR("reboot for codec error %u times, more than %u times, do not reboot again\n",
			err_times, CODEC_ERR_REBOOT_THRESHOLD);
	}
}

/*lint -e838*/
static int dump_codec(const char *filepath)
{
	WARN_ON(NULL == filepath);
	hi64xx_hifi_dump_with_path(filepath);

	return 0;
}
/*lint +e838*/
void rdr_codec_hifi_watchdog_process(void)
{
	wake_lock(&codec_des.rdr_wl);
	up(&codec_des.handler_sem);
}/*lint !e454*/
/*lint -e715*/
static int irq_handler_thread(void *arg)
{
	unsigned int addr;
	BB_PRINT_START();

	while (!kthread_should_stop()) {
		if (down_interruptible(&codec_des.handler_sem)) {
			BB_PRINT_ERR("down codec_des.handler_sem fail\n");
			continue;
		}

		addr = hi64xx_misc_get_ocram_dump_addr();
		if(addr){
			addr += 0x14;
			BB_PRINT_PN("%s():[codechifi timestamp: %u]codechifi watchdog coming\n", __func__, hi64xx_hifi_read_reg(addr));
		}else
			BB_PRINT_PN("%s():codechifi watchdog coming\n", __func__);

		BB_PRINT_PN("enter rdr process for codechifi watchdog\n");
		rdr_system_error((unsigned int)RDR_AUDIO_CODEC_WD_TIMEOUT_MODID, 0, 0);
		BB_PRINT_PN("exit rdr process for codechifi watchdog\n");
	}

	BB_PRINT_END();

	return 0;
}

static int dump_thread(void *arg)
{
	BB_PRINT_START();

	while (!kthread_should_stop()) {
		if (down_interruptible(&codec_des.dump_sem)) {
			BB_PRINT_ERR("down codec_des.dump_sem fail\n");
			continue;
		}

		BB_PRINT_DBG("begin to dump codec hifi log\n");
		dump_codec(codec_des.pathname);
		BB_PRINT_DBG("end dump codec hifi log\n");

		if (codec_des.dumpdone_cb) {
			BB_PRINT_DBG
			    ("begin dump codec hifi done callback, modid: 0x%x\n",
			     codec_des.modid);
			codec_des.dumpdone_cb(codec_des.modid, (unsigned long long)RDR_HIFI);
			BB_PRINT_DBG("end dump codec hifi done callback\n");
		}
	}

	BB_PRINT_END();

	return 0;
}

void rdr_audio_codec_dump(u32 modid, char *pathname, pfn_cb_dump_done pfb)
{
	WARN_ON(NULL == pathname);

	BB_PRINT_START();

	codec_des.modid = modid;
	codec_des.dumpdone_cb = pfb;
	codec_des.pathname = pathname;

	up(&codec_des.dump_sem);

	BB_PRINT_END();

	return;
}

void rdr_audio_codec_reset(u32 modid, u32 etype, u64 coreid)
{
	BB_PRINT_START();
/* todo :hi6402_watchdog_send_event undefined in dallas */
	/*       ....send watchdog event.....   */
	hi64xx_watchdog_send_event();

	wake_unlock(&codec_des.rdr_wl);/*lint !e455*/

	BB_PRINT_END();

	return;
}
/*lint +e715*/
int rdr_audio_codec_init(void)
{
	BB_PRINT_START();

	codec_des.modid = ~0;
	codec_des.pathname = NULL;
	codec_des.dumpdone_cb = NULL;

	wake_lock_init(&codec_des.rdr_wl, WAKE_LOCK_SUSPEND, "rdr_codechifi");
	sema_init(&codec_des.dump_sem, 0);
	sema_init(&codec_des.handler_sem, 0);
	codec_des.kdump_task = NULL;
	codec_des.khandler_task = NULL;

	codec_des.kdump_task =
	    kthread_run(dump_thread, NULL, "rdr_codec_hifi_dump_thread");
	if (!codec_des.kdump_task) {
		BB_PRINT_ERR("create rdr codec dump thead fail\n");
		goto error;
	}

	codec_des.khandler_task =
	    kthread_run(irq_handler_thread, NULL, "rdr_codec_hifi_irq_handler_thread");
	if (!codec_des.khandler_task) {
		BB_PRINT_ERR("create rdr codec irq handler thead fail\n");
		goto error;
	}

	BB_PRINT_END();

	return 0;

error:
	if (codec_des.kdump_task != NULL) {
		kthread_stop(codec_des.kdump_task);
		up(&codec_des.dump_sem);
		codec_des.kdump_task = NULL;
	}

	if (codec_des.khandler_task != NULL) {
		kthread_stop(codec_des.khandler_task);
		up(&codec_des.handler_sem);
		codec_des.khandler_task = NULL;
	}

	wake_lock_destroy(&codec_des.rdr_wl);

	BB_PRINT_END();

	return -1;
}

void rdr_audio_codec_exit(void)
{
	BB_PRINT_START();

	if (codec_des.kdump_task != NULL) {
		kthread_stop(codec_des.kdump_task);
		up(&codec_des.dump_sem);
		codec_des.kdump_task = NULL;
	}

	if (codec_des.khandler_task != NULL) {
		kthread_stop(codec_des.khandler_task);
		up(&codec_des.handler_sem);
		codec_des.khandler_task = NULL;
	}

	wake_lock_destroy(&codec_des.rdr_wl);

	BB_PRINT_END();

	return;
}
