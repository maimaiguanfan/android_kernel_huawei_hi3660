#ifndef __HICAM_BUF_H_INCLUDED__
#define __HICAM_BUF_H_INCLUDED__
#include <linux/types.h>

struct device;
struct sg_table;

#define HICAM_BUF_SYNC_READ    (1u << 0)
#define HICAM_BUF_SYNC_WRITE   (1u << 1)

enum hicam_buf_cfg_type {
    HICAM_BUF_MAP_IOMMU,
    HICAM_BUF_UNMAP_IOMMU,
    HICAM_BUF_SYNC,
    HICAM_BUF_LOCAL_SYNC,
    HICAM_BUF_GET_PHYS,
};
// kernel coding style prefers __xx as types shared with userspace
struct iommu_format {
    __u32 prot;
    __u64 iova;
    __u64 size;
};

struct phys_format {
    __u64 phys;
};

struct sync_format {
    // sync direction read/write from CPU view point
    __u32 direction;
};

struct local_sync_format {
    // sync direction read/write from CPU view point
    __u32 direction;
    // local sync needs to set apva/offset/length
    __u64 apva;
    __u64 offset;
    __u64 length;
};

struct hicam_buf_cfg {
    __s32 fd;
    enum hicam_buf_cfg_type type;
    union {
        struct iommu_format iommu_format;
        struct phys_format phys_format;
        struct sync_format sync_format;
        struct local_sync_format local_sync_format;
    };
};

#define HICAM_BUF_BASE 'H'
#define HICAM_BUF_IOC_CFG           _IOWR(HICAM_BUF_BASE, 0, struct hicam_buf_cfg)

int hicam_buf_map_iommu(int fd, struct iommu_format *fmt);
void hicam_buf_unmap_iommu(int fd, struct iommu_format *fmt);
int hicam_buf_get_phys(int fd, struct phys_format *fmt);
phys_addr_t hicam_buf_get_pgd_base(void);

// keep in mind, get sgtable will hold things of sg_table,
// please release after use.
struct sg_table* hicam_buf_get_sgtable(int fd);
void hicam_buf_put_sgtable(struct sg_table *sgt);

#endif /* ifndef __HICAM_BUF_H_INCLUDED__ */
