#ifndef BATT_INFO_PUB_H
#define BATT_INFO_PUB_H

#define BATTERY_TYPE_BUFF_SIZE  6
#ifdef CONFIG_HUAWEI_BATTERY_INFORMATION
int get_battery_type(unsigned char name[BATTERY_TYPE_BUFF_SIZE]);
#else 
int get_battery_type(unsigned char name[BATTERY_TYPE_BUFF_SIZE]) {return -1;}
#endif

#endif