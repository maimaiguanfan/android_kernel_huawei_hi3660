#ifndef POWER_GENL_H
#define POWER_GENL_H

#include <linux/list.h>
#include <net/genetlink.h>
#include <net/netlink.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <huawei_platform/log/hw_log.h>
#include <linux/spinlock_types.h>
#include <linux/spinlock.h>
#include <linux/string.h>
#include <huawei_platform/power/power_mesg_type.h>

/* generic netlink macro */
#define POWER_GENL_SEQ                      0
#define POWER_GENL_PORTID                   0
#define POWER_GENL_FLAG                     0
#define POWER_USER_HDR_LEN                  0
#define POWER_GENL_MEM_MARGIN               200
#define POWER_GENL_NAME                     "POWER_GENL"

typedef power_mesg_node_t power_genl_easy_node_t;

int power_genl_send_attrs(power_mesg_node_t *genl_node, unsigned char cmd, unsigned char version,
                          resource *attrs, unsigned char attr_num);
int power_genl_easy_send(power_genl_easy_node_t *genl_node, unsigned char cmd,
        unsigned char version, void *data, unsigned len);
int power_genl_easy_node_register(power_genl_easy_node_t *genl_node);
int power_genl_normal_node_register(power_genl_easy_node_t *genl_node);
int power_genl_node_register(power_genl_easy_node_t *genl_node);
int power_genl_init(void);

typedef unsigned int power_genl_port_t;

typedef enum {
    POWER_GENL_SUCCESS = 0,
    POWER_GENL_EUNCHG,
    POWER_GENL_ETARGET,
    POWER_GENL_ENULL,
    POWER_GENL_ENEEDLESS,
    POWER_GENL_EUNREGED,
    POWER_GENL_EMESGLEN,
    POWER_GENL_EALLOC,
    POWER_GENL_EPUTMESG,
    POWER_GENL_EUNICAST,
    POWER_GENL_EADDATTR,
    POWER_GENL_EPORTID,
    POWER_GENL_EPROBE,
    POWER_GENL_ELATE,
    POWER_GENL_EREGED,
    POWER_GENL_ECMD,
}power_genl_error_t;

#define POWER_GENL_PROBE_UNREADY    0
#define POWER_GENL_PROBE_START      1

#endif