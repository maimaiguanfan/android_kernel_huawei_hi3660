#ifndef BATT_INFO_UTIL_H
#define BATT_INFO_UTIL_H

#include <huawei_platform/power/power_mesg_srv.h>
typedef struct {
    struct list_head node;
    struct platform_device *pdev;
} batt_checkers_list_entry;

void add_to_batt_checkers_lists(batt_checkers_list_entry *entry);
int send_batt_info_mesg(nl_dev_info *info, void *data, unsigned int len);

#endif