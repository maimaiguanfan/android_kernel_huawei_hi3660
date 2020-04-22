#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/hugetlb.h>
#include <linux/mman.h>
#include <linux/mmzone.h>
#include <linux/proc_fs.h>
#include <linux/quicklist.h>
#include <linux/seq_file.h>
#include <linux/swap.h>
#include <linux/vmstat.h>
#include <linux/atomic.h>
#include <linux/vmalloc.h>
#include <linux/bootdevice.h>

#ifdef CONFIG_CMA
#include <linux/cma.h>
#endif
#include <asm/page.h>
#include <asm/pgtable.h>
#include "internal.h"

#ifdef CONFIG_ION
#include <linux/hisi/hisi_ion.h>
#endif

#ifdef CONFIG_HUAWEI_UNMOVABLE_ISOLATE
#include <linux/unmovable_isolate.h>
#endif

void __attribute__((weak)) arch_report_meminfo(struct seq_file *m)
{
}

#define UI_IO_BLOCK_DOT (26)

u64 ui_thread_io_block_counts[UI_IO_BLOCK_DOT] = {0};


#define hwioinfo_stat_add(var, amt)    do { var += (amt); } while (0)
#define hwioinfo_stat_get(var)        (var)
#define hwioinfo_stat_inc(var)         do { var++; } while (0)



void sched_account_ui_thread_io_block_counts(int msecs) {
    u64 ui_io_block_idx;

    ui_io_block_idx = msecs/200;
    if (ui_io_block_idx >= UI_IO_BLOCK_DOT) {
        ui_io_block_idx = UI_IO_BLOCK_DOT - 1;
    }

    hwioinfo_stat_inc(ui_thread_io_block_counts[ui_io_block_idx]);
}

static void show_val(struct seq_file *m, const char *s, unsigned long num)
{
    char v[32];
    static const char blanks[7] = {' ', ' ', ' ', ' ',' ', ' ', ' '};
    int len;

    len = num_to_str(v, sizeof(v), num);

    seq_write(m, s, 16);

    if (len > 0) {
        if (len < 8)
            seq_write(m, blanks, 8 - len);

        seq_write(m, v, len);
    }
    seq_write(m, "\n", 1);
}

static int hwioinfo_proc_show(struct seq_file *m, void *v)
{
    int i;
    char buf[128];
    struct vm_event_state vm_stat_val;

    show_val(m, "StorageManfid:	    ", get_bootdevice_manfid());

    for (i = 0; i < UI_IO_BLOCK_DOT; i++) {
        sprintf(buf, "UiIoDelaySeg%u:  ", (i+1));
        show_val(m, buf, hwioinfo_stat_get(ui_thread_io_block_counts[i]));
    }
    /*read and clear*/
    memset(ui_thread_io_block_counts, 0, sizeof(ui_thread_io_block_counts));

    all_vm_events((long*)&vm_stat_val);
    show_val(m, "vmstat_pswpin:  ", vm_stat_val.event[PSWPIN]);
    show_val(m, "vmstat_pswpout: ", vm_stat_val.event[PSWPOUT]);

    return 0;
}

static int hwioinfo_proc_open(struct inode *inode, struct file *file)
{
    return single_open(file, hwioinfo_proc_show, NULL);
}

static const struct file_operations hwioinfo_proc_fops = {
    .open       = hwioinfo_proc_open,
    .read       = seq_read,
    .llseek     = seq_lseek,
    .release    = single_release,
};

static int __init proc_hwioinfo_init(void)
{
    proc_create("hwioinfo", 0440, NULL, &hwioinfo_proc_fops);
    return 0;
}
fs_initcall(proc_hwioinfo_init);
