#ifndef POWER_MESG_TYPE
#define POWER_MESG_TYPE

#include <linux/list.h>
#include <net/genetlink.h>
#include <huawei_platform/power/power_mesg_srv.h>

#define POWER_NODE_NAME_MAX_LEN         16

typedef int (*srv_on_cb_t)(void);

typedef enum {
    POWERCT_PORT = 0,
    __TARGET_PORT_MAX,
}power_target_t;
#define TARGET_PORT_MAX                     (__TARGET_PORT_MAX-1)

typedef struct {
    unsigned char cmd;
    int (*doit)(unsigned char version, void *data, int length);
} easy_cbs_t;

typedef struct {
    unsigned char cmd;
    int (*doit)(struct sk_buff *skb_in, struct genl_info *info);
} call_backs_t;

typedef struct {
    struct list_head node;
    const power_target_t target;
    const char name[POWER_NODE_NAME_MAX_LEN];
    const easy_cbs_t *ops;
    const unsigned char n_ops;  /* n_ops means there is n ops valid */
    const call_backs_t *cbs;
    const unsigned char n_cbs;  /* n_cbs means there is n cbs valid */
    srv_on_cb_t srv_on_cb;
}power_mesg_node_t;

int create_attr_for_power_mesg(const struct device_attribute *attr);

#endif
