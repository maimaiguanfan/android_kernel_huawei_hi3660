
#ifndef _IO_TRACE_H
#define _IO_TRACE_H

#include<linux/tracepoint.h>

/* Event 914009000
	the block info */
/* sd card exist */
#define V914009000_SD_CARD_EXIST_SMALLINT "SD_CARD_EXIST"
/* data part free space */
#define V914009000_DATA_PART_FREE_SPACE_INT "DATA_PART_FREE_SPACE"
/* total read data in blk */
#define V914009000_HOST_TOTAL_READ_INT "HOST_TOTAL_READ"
/* total write data in blk */
#define V914009000_HOST_TOTAL_WRITE_INT "HOST_TOTAL_WRITE"
/* 4k read delay in five */
#define V914009000_READ_DELAY_4K_FIVE_INT "READ_DELAY_4K_FIVE"
/* 4k write delay in five */
#define V914009000_WRITE_DELAY_4K_FIVE_INT "WRITE_DELAY_4K_FIVE"
/* 4k read delay in four */
#define V914009000_READ_DELAY_4K_FOUR_INT "READ_DELAY_4K_FOUR"
/* 4k write delay in four */
#define V914009000_WRITE_DELAY_4K_FOUR_INT "WRITE_DELAY_4K_FOUR"
/* 4k read delay in three */
#define V914009000_READ_DELAY_4K_THREE_INT "READ_DELAY_4K_THREE"
/* 4k write delay in three */
#define V914009000_WRITE_DELAY_4K_THREE_INT "WRITE_DELAY_4K_THREE"
/* 4k read delay in two */
#define V914009000_READ_DELAY_4K_TWO_INT "READ_DELAY_4K_TWO"
/* 4k write delay in two */
#define V914009000_WRITE_DELAY_4K_TWO_INT "WRITE_DELAY_4K_TWO"
/* 4k read delay in one */
#define V914009000_READ_DELAY_4K_ONE_INT "READ_DELAY_4K_ONE"
/* 4k write delay in one */
#define V914009000_WRITE_DELAY_4K_ONE_INT "WRITE_DELAY_4K_ONE"
/* 4k read delay in five */
#define V914009000_READ_DELAY_512K_FIVE_INT "READ_DELAY_512K_FIVE"
/* 512k write delay in five */
#define V914009000_WRITE_DELAY_512K_FIVE_INT "WRITE_DELAY_512K_FIVE"
/* 512k read delay in four */
#define V914009000_READ_DELAY_512K_FOUR_INT "READ_DELAY_512K_FOUR"
/* 512k write delay in four */
#define V914009000_WRITE_DELAY_512K_FOUR_INT "WRITE_DELAY_512K_FOUR"
/* 512k read delay in three */
#define V914009000_READ_DELAY_512K_THREE_INT "READ_DELAY_512K_THREE"
/* 512k write delay in three */
#define V914009000_WRITE_DELAY_512K_THREE_INT "WRITE_DELAY_512K_THREE"
/* 512k read delay in two */
#define V914009000_READ_DELAY_512K_TWO_INT "READ_DELAY_512K_TWO"
/* 512k write delay in two */
#define V914009000_WRITE_DELAY_512K_TWO_INT "WRITE_DELAY_512K_TWO"
/* 512k read delay in one */
#define V914009000_READ_DELAY_512K_ONE_INT "READ_DELAY_512K_ONE"
/* 512k write delay in one */
#define V914009000_WRITE_DELAY_512K_ONE_INT "WRITE_DELAY_512K_ONE"
/* 4k average read delay */
#define V914009000_READ_DELAY_4K_AVERAGE_INT "READ_DELAY_4K_AVERAGE"
/* 4k average write delay */
#define V914009000_WRITE_DELAY_4K_AVERAGE_INT "WRITE_DELAY_4K_AVERAGE"
/* 512k  average read delay */
#define V914009000_READ_DELAY_512K_AVERAGE_INT "READ_DELAY_512K_AVERAGE"
/* 512k average write delay */
#define V914009000_WRITE_DELAY_512K_AVERAGE_INT "WRITE_DELAY_512K_AVERAGE"
/* min read delay */
#define V914009000_READ_DELAY_MIN_INT "READ_DELAY_MIN"
/* min write delay */
#define V914009000_WRITE_DELAY_MIN_INT "WRITE_DELAY_MIN"
/* max read delay */
#define V914009000_READ_DELAY_MAX_INT "READ_DELAY_MAX"
/* max read delay */
#define V914009000_WRITE_DELAY_MAX_INT "WRITE_DELAY_MAX"
/* running read io number */
#define V914009000_READ_RUN_IO_NUM_INT "READ_RUN_IO_NUM"
/* running write io number */
#define V914009000_WRITE_RUN_IO_NUM_INT "WRITE_RUN_IO_NUM"
/* running read io sectors */
#define V914009000_READ_RUN_IO_SECTOR_INT "READ_RUN_IO_SECTOR"
/* running write io sectors */
#define V914009000_WRITE_RUN_IO_SECTOR_INT "WRITE_RUN_IO_SECTOR"
/* running read io ticks */
#define V914009000_READ_RUN_IO_TICKS_INT "READ_RUN_IO_TICKS"
/* running write io ticks */
#define V914009000_WRITE_RUN_IO_TICKS_INT "WRITE_RUN_IO_TICKS"
/* running read io flight */
#define V914009000_READ_RUN_IO_FLIGHT_INT "READ_RUN_IO_FLIGHT"
/* running write io flight */
#define V914009000_WRITE_RUN_IO_FLIGHT_INT "WRITE_RUN_IO_FLIGHT"
/* read iops */
#define V914009000_READ_IOPS_INT "READ_IOPS"
/* write iops */
#define V914009000_WRITE_IOPS_INT "WRITE_IOPS"
/* read bandith */
#define V914009000_READ_BANDITH_INT "READ_BANDITH"
/* write bandith */
#define V914009000_WRITE_BANDITH_INT "WRITE_BANDITH"

DECLARE_TRACE(generic_perform_write_enter,
        TP_PROTO(struct file *file, size_t count, loff_t pos),
        TP_ARGS(file, count, pos));

DECLARE_TRACE(generic_perform_write_end,
        TP_PROTO(struct file *file, size_t count),
        TP_ARGS(file, count));

DECLARE_TRACE(generic_file_read_begin,
        TP_PROTO(struct file *filp, size_t count),
        TP_ARGS(filp, count));

DECLARE_TRACE(generic_file_read_end,
        TP_PROTO(struct file *filp, size_t count),
        TP_ARGS(filp, count));

/*
DECLARE_TRACE(ext4_da_write_begin_enter,
        TP_PROTO(struct inode *inode, loff_t pos, unsigned int len,
              unsigned int flags),
        TP_ARGS(inode, pos, len, flags)
    );
    */
DECLARE_TRACE(ext4_da_write_begin_end,
        TP_PROTO(struct inode *inode, loff_t pos, unsigned int len,
              unsigned int flags),
        TP_ARGS(inode, pos, len, flags)
    );

DECLARE_TRACE(ext4_sync_write_wait_end,
        TP_PROTO(struct file *file, int datasync),
        TP_ARGS(file, datasync)
    );

/*DECLARE_TRACE(ext4_sync_file_begin,
        TP_PROTO(struct file *file, int datasync),
        TP_ARGS(file, datasync)
    );

*/
DECLARE_TRACE(ext4_sync_file_end,
        TP_PROTO(struct file *file, int ret),
        TP_ARGS(file, ret)
    );

DECLARE_TRACE(f2fs_detected_quasi,
        TP_PROTO(struct bio *bio),
        TP_ARGS(bio)
    );

DECLARE_TRACE(block_write_begin_enter,
        TP_PROTO(struct inode *inode, struct page *page, loff_t pos,
            unsigned int len),
        TP_ARGS(inode, page, pos, len)
    );

DECLARE_TRACE(block_write_begin_end,
        TP_PROTO(struct inode *inode, struct page *page, int err),
        TP_ARGS(inode, page, err)
    );

DECLARE_TRACE(mpage_da_map_and_submit,
        TP_PROTO(struct inode *inode, unsigned long long fblk, unsigned int len),
        TP_ARGS(inode, fblk, len)
    );

/*M*/
DECLARE_TRACE(block_crypt_dec_pending,
        TP_PROTO(struct bio *bio),
        TP_ARGS(bio)
    );

DECLARE_TRACE(block_kcryptd_crypt,
        TP_PROTO(struct bio *bio),
        TP_ARGS(bio)
    );

DECLARE_TRACE(block_dm_request,
        TP_PROTO(struct request_queue *q, struct bio *bio),
        TP_ARGS(q, bio)
    );

DECLARE_TRACE(block_crypt_map,
        TP_PROTO(struct bio *bio, sector_t sector),
        TP_ARGS(bio, sector)
    );

DECLARE_TRACE(block_throttle_weight,
        TP_PROTO(struct bio *bio, unsigned int weight, unsigned int nr_queued),
        TP_ARGS(bio, weight, nr_queued)
    );

DECLARE_TRACE(block_throttle_dispatch,
        TP_PROTO(struct bio *bio, unsigned int weight),
        TP_ARGS(bio, weight)
    );

DECLARE_TRACE(block_throttle_iocost,
        TP_PROTO(uint64_t bps, unsigned int iops, uint64_t bytes_disp, unsigned int io_disp),
        TP_ARGS(bps, iops, bytes_disp, io_disp)
    );

DECLARE_TRACE(block_throttle_limit_start,
        TP_PROTO(struct bio *bio, int max_inflights, atomic_t inflights),
        TP_ARGS(bio, max_inflights, inflights)
    );

DECLARE_TRACE(block_throttle_limit_end,
        TP_PROTO(struct bio *bio),
        TP_ARGS(bio)
    );

DECLARE_TRACE(block_throttle_bio_in,
        TP_PROTO(struct bio *bio),
        TP_ARGS(bio)
    );

DECLARE_TRACE(block_throttle_bio_out,
        TP_PROTO(struct bio *bio, long delay),
        TP_ARGS(bio, delay)
    );

DECLARE_TRACE(block_bio_wbt_done,
        TP_PROTO(struct bio *bio),
        TP_ARGS(bio)
    );

#endif
