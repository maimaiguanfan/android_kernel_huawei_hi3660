#include <huawei_platform/power/power_genl.h>

#ifdef  HWLOG_TAG
#undef  HWLOG_TAG
#endif
#define HWLOG_TAG power_genl
HWLOG_REGIST();

static LIST_HEAD(power_genl_node_head);

static int probe_status = POWER_GENL_PROBE_UNREADY;

static struct genl_family power_genl = {
    .hdrsize = POWER_USER_HDR_LEN,
    .name = POWER_GENL_NAME,
    .maxattr = POWER_GENL_MAX_ATTR_INDEX,
    .parallel_ops = 1,
    .n_ops = 0,
    };

typedef struct {
    unsigned int port_id;
    unsigned int probed;
    const struct device_attribute dev_attr;
}power_genl_target_t;

static ssize_t powerct_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    power_genl_target_t *target = container_of(attr, power_genl_target_t, dev_attr);
    return snprintf(buf, PAGE_SIZE, "%u", target->port_id);
}
static ssize_t powerct_store(struct device *dev, struct device_attribute *attr,
                                   const char *buf, size_t count)
{
    power_genl_target_t *target = container_of(attr, power_genl_target_t, dev_attr);
    power_genl_easy_node_t *temp;

    if(count != sizeof(unsigned) ) {
        hwlog_err("Illegal write length(%zu) found in %s.", count, __func__);
        return -EINVAL;
    }
    memcpy(&target->port_id, buf, count);
    list_for_each_entry(temp, &power_genl_node_head, node) {
        if(temp->srv_on_cb) {
            if(temp->srv_on_cb()) {
                hwlog_err("%s srv_on_cb failed.\n", temp->name);
            }
        }
    }

    return count;
}

static power_genl_target_t nl_target[__TARGET_PORT_MAX] = {
    [POWERCT_PORT] = {
                       .port_id = 0,
                       .probed = 0,
                       .dev_attr = __ATTR_RW(powerct),
                     },
};

int power_genl_send_attrs(power_mesg_node_t *genl_node, unsigned char cmd, unsigned char version,
                          resource *attrs, unsigned char attr_num)
{
    power_genl_easy_node_t *temp;
    power_genl_error_t ret_val;
    struct sk_buff *skb;
    void *msg_head;
    int i;
    unsigned int len;
    struct nlmsghdr *nlh;
    struct genlmsghdr *hdr;

    if(!attrs) {
        hwlog_err("Resoure for cmd(%d) given to %s is NULL.\n", cmd, __func__);
        return POWER_GENL_ENULL;
    }

    list_for_each_entry(temp, &power_genl_node_head, node) {
        if(temp == genl_node) {
            break;
        }
    }
    if(&temp->node == &power_genl_node_head) {
        hwlog_err("this power_genl_node(%s) unregistered.\n", temp->name);
        return POWER_GENL_EUNREGED;
    }

    if( temp->target > TARGET_PORT_MAX || nl_target[temp->target].port_id == 0 ) {
        hwlog_err("target port id had not set.\n");
        return POWER_GENL_EPORTID;
    }

    len = 0;
    for(i = 0; i < attr_num; i++) {
        len = attrs[i].len;
    }
    if(len > NLMSG_GOODSIZE - POWER_GENL_MEM_MARGIN) {
        return POWER_GENL_EMESGLEN;
    }
    /* allocate some memory, since the size is not yet known use NLMSG_GOODSIZE*/
    skb = genlmsg_new(NLMSG_GOODSIZE, GFP_KERNEL);
    if(!skb) {
        hwlog_err("new general message failed in %s.\n", __func__);
        return POWER_GENL_EALLOC;
    }
    /* genlmsg_put is not used because diffent version of genl_family may used at same time */
    nlh = nlmsg_put(skb, POWER_GENL_PORTID, POWER_GENL_SEQ, power_genl.id, GENL_HDRLEN +
                    power_genl.hdrsize, POWER_GENL_FLAG);
    if (nlh == NULL) {
        hwlog_err("Create message for genlmsg failed in %s.\n", __func__);
        ret_val = POWER_GENL_EPUTMESG;
        goto nosend;
    }
    /* fill genl head */
    hdr = nlmsg_data(nlh);
    hdr->cmd = cmd;
    hdr->version = version;
    hdr->reserved = 0;
    /* get message head */
    msg_head = (char *) hdr + GENL_HDRLEN;
    /* fill attributes */
    for(i = 0; i < attr_num; i++) {
        /* add a BATT_RAW_MESSAGE attribute (actual value to be sent) */
        if (nla_put(skb, attrs[i].type, attrs[i].len, attrs[i].data)) {
            hwlog_err("Add attribute to genlmsg failed in %s.\n", __func__);
            ret_val = POWER_GENL_EADDATTR;
            goto nosend;
        }
    }
    /* finalize the message */
    genlmsg_end(skb, msg_head);

    /* send the message back */
    if( temp->target > TARGET_PORT_MAX ||
        genlmsg_unicast(&init_net, skb, nl_target[temp->target].port_id)) {
        hwlog_err("Unicast genlmsg failed in %s.\n", __func__);
        return POWER_GENL_EUNICAST;
    }
    hwlog_info("%s cmd %d data was sent in %s.\n", temp->name, cmd, __func__);

    return POWER_GENL_SUCCESS;

nosend:
    kfree_skb(skb);
    return ret_val;
}


int power_genl_easy_send(power_mesg_node_t *genl_node, unsigned char cmd,
                         unsigned char version, void *data, unsigned int len)
{
    resource attr;
    attr.data = data;
    attr.len = len;
    attr.type = POWER_GENL_RAW_DATA_ATTR;
    return power_genl_send_attrs(genl_node, cmd, version, &attr, 1);
}

int check_port_id(power_target_t type, unsigned int pid)
{
    if(type > TARGET_PORT_MAX) {
        hwlog_err("invalid power_target_t number.\n");
        return -1;
    }
    return !(nl_target[type].port_id == pid);
}

static int easy_node_mesg_cb(struct sk_buff *skb_in, struct genl_info *info)
{
    int i;
    int len;
    struct nlattr * raw_data_attr;
    void *data;
    power_genl_easy_node_t *temp;

    if(!info) {
        hwlog_err("info is null.\n");
        return -1;
    }

    if(!info->attrs) {
        hwlog_err("info attrs is null.\n");
        return -1;
    }

    raw_data_attr = info->attrs[POWER_GENL_RAW_DATA_ATTR];
    if(!raw_data_attr) {
        hwlog_err("raw_data_attr is null.\n");
        return -1;
    }

    len = nla_len(raw_data_attr);
    data = nla_data(raw_data_attr);

    list_for_each_entry(temp, &power_genl_node_head, node) {
        for(i = 0; i < temp->n_ops; i++) {
            if(!temp->ops) {
                hwlog_err("ops is null.\n");
                break;
            }
            if(info->genlhdr->cmd == temp->ops[i].cmd) {
                if(check_port_id(POWERCT_PORT, info->snd_portid)) {
                    hwlog_err("%s cmd %d filted by checking port id failed.\n",
                              temp->name, info->genlhdr->cmd);
                    return POWER_GENL_EPORTID;
                }
                if (temp->ops[i].doit) {
                    hwlog_info("%s cmd %d called by %s.\n", temp->name, info->genlhdr->cmd, __func__);
                    temp->ops[i].doit(info->genlhdr->version, data, len);
                } else {
                    hwlog_info("%s cmd %d doit is null.\n", temp->name, info->genlhdr->cmd);
                }
                break;
            }
        }
    }

    return 0;
}

int power_genl_node_register(power_genl_easy_node_t *genl_node)
{
    power_genl_easy_node_t *temp;

    if(probe_status == POWER_GENL_PROBE_START) {
        return POWER_GENL_ELATE;
    }
    if(strnlen(genl_node->name,POWER_NODE_NAME_MAX_LEN) >= POWER_NODE_NAME_MAX_LEN) {
        hwlog_err("%s unsupports name which is not a C string.\n", __func__);
    }

    list_for_each_entry(temp, &power_genl_node_head, node) {
        if(!strncmp(genl_node->name, temp->name, POWER_NODE_NAME_MAX_LEN)) {
            hwlog_err("name(%s) had been registered.\n", genl_node->name);
            return POWER_GENL_EREGED;
        }
        if(genl_node->target > TARGET_PORT_MAX) {
            hwlog_err("%s want illegal targer port(%d).\n", genl_node->name, genl_node->target);
            return POWER_GENL_ETARGET;
        }
    }
    INIT_LIST_HEAD(&genl_node->node);
    list_add(&genl_node->node, &power_genl_node_head);
    return POWER_GENL_SUCCESS;
}
int power_genl_easy_node_register(power_genl_easy_node_t *genl_node)
{
    if(genl_node->cbs || genl_node->n_cbs) {
        hwlog_err("genl_node cbs illegal found in %s.\n", __func__);
        return POWER_GENL_EUNCHG;
    }
    if(!genl_node->ops || genl_node->n_ops == 0) {
        hwlog_err("genl_node ops illegal found in %s.\n", __func__);
        return POWER_GENL_EUNCHG;
    }

    return power_genl_node_register(genl_node);
}
int power_genl_normal_node_register(power_genl_easy_node_t *genl_node)
{
    if(genl_node->ops || genl_node->n_ops) {
        hwlog_err("genl_node ops illegal found in %s.\n", __func__);
        return POWER_GENL_EUNCHG;
    }
    if(!genl_node->cbs || genl_node->n_cbs == 0) {
        hwlog_err("genl_node cbs illegal found in %s.\n", __func__);
        return POWER_GENL_EUNCHG;
    }

    return power_genl_node_register(genl_node);
}
int power_genl_init(void)
{
    struct genl_ops *ops, *ops_o, *ops_temp;
    power_genl_easy_node_t *temp;
    int i;
    unsigned int total_ops;

    /* power_genl_register is not allowed from now on */
    probe_status = POWER_GENL_PROBE_START;

    total_ops = 0;
    list_for_each_entry(temp, &power_genl_node_head, node) {
        total_ops += temp->n_ops + temp->n_cbs;
    }
    if(total_ops > 255 || total_ops <= 0) {
        hwlog_err("illegal ops num(%d).\n", total_ops);
        return POWER_GENL_EPROBE;
    }
    power_genl.n_ops = total_ops;
    ops = kzalloc(power_genl.n_ops * sizeof(struct genl_ops), GFP_KERNEL);
    if(!ops) {
        hwlog_err("malloc for genl_ops points failed.\n");
        return POWER_GENL_EPROBE;
    }
    ops_o = ops;
    list_for_each_entry(temp, &power_genl_node_head, node) {
        for(i = 0; i < temp->n_ops; i++) {
            for(ops_temp = ops_o; ops_temp < ops; ops_temp++) {
                if(temp->ops[i].cmd == ops_temp->cmd) {
                    hwlog_err("cmd(%d) of ops in %s had been regist.\n", ops->cmd, temp->name);
                    kfree(ops_o);
                    return POWER_GENL_EPROBE;
                }
            }
            ops->cmd = temp->ops[i].cmd;
            ops->doit = easy_node_mesg_cb;
            ops++;
        }
        for(i = 0; i < temp->n_cbs; i++) {
            for(ops_temp = ops_o; ops_temp < ops; ops_temp++) {
                if(temp->cbs[i].cmd == ops_temp->cmd) {
                    hwlog_err("cmd(%d) of cbs in %s had been regist.\n", ops->cmd, temp->name);
                    kfree(ops_o);
                    return POWER_GENL_EPROBE;
                }
            }
            ops->cmd = temp->cbs[i].cmd;
            ops->doit = temp->cbs[i].doit;
            ops++;
        }
    }
    power_genl.ops = ops_o;
    if(genl_register_family(&power_genl)) {
        hwlog_err("power_genl register failed.\n");
        kfree(ops_o);
        return POWER_GENL_EPROBE;
    }

    list_for_each_entry(temp, &power_genl_node_head, node) {
        if( temp->target > TARGET_PORT_MAX || nl_target[temp->target].probed ) {
            continue;
        }
        if( create_attr_for_power_mesg(&nl_target[temp->target].dev_attr) ) {
            hwlog_err("attribute %s created failed.\n", nl_target[temp->target].dev_attr.attr.name);
        }
        nl_target[temp->target].probed = 1;
    }

    hwlog_info("power_genl driver probe success.\n");
    return POWER_GENL_SUCCESS;
}
