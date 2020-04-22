#include <huawei_platform/power/power_mesg.h>

#ifdef  HWLOG_TAG
#undef  HWLOG_TAG
#endif
#define HWLOG_TAG power_mesg
HWLOG_REGIST();

/* power mesg */
static struct device *power_mesg = NULL;

int power_easy_send(power_mesg_node_t *node, unsigned char cmd,
                    unsigned char version, void *data, unsigned len)
{
    return power_genl_easy_send(node, cmd, version, data, len);
}

int power_easy_node_register(power_genl_easy_node_t *node)
{
    return power_genl_easy_node_register(node);
}

static struct device * get_power_mesg_device(void)
{
    struct class *hw_power;
    if(!power_mesg) {
        hw_power = hw_power_get_class();
        if(hw_power) {
            power_mesg = device_create(hw_power, NULL, 0, NULL, "power_mesg");
        }
    }
    return power_mesg;
}

int create_attr_for_power_mesg(const struct device_attribute *attr)
{
    struct device * dev;
    dev = get_power_mesg_device();
    if(dev) {
        return device_create_file(dev, attr);
    }
    hwlog_err("get power mesg device failed.\n");
    return -1;
}

int __init power_msg_init(void)
{
    hwlog_info("power mesg driver init...\n");
    if(power_genl_init()) {
       hwlog_info("power genl probe failed, no attrs created.\n");
    }
    return 0;
}

void __exit power_msg_exit(void)
{
    hwlog_info("power generic netlink driver exit...\n");
}

late_initcall(power_msg_init);
module_exit(power_msg_exit);

MODULE_LICENSE("GPL");
