/*
 * Huawei Touchpanel driver
 *
 * Copyright (C) 2013 Huawei Device Co.Ltd
 * License terms: GNU General Public License (GPL) version 2
 *
 */

#ifndef __HUAWEI_TS_KIT_MISC_H_
#define __HUAWEI_TS_KIT_MISC_H_

#include <linux/ioctl.h>
#include <linux/miscdevice.h>

#define DEVICE_AFT_GET_INFO  "ts_aft_get_info"
#define DEVICE_AFT_SET_INFO  "ts_aft_set_info"
/* commands */
#define  INPUT_AFT_GET_IO_TYPE  (0xBA)
#define  INPUT_AFT_SET_IO_TYPE  (0xBB)

struct double_click_data {
	unsigned int key_code;
	unsigned int x_position;
	unsigned int y_position;
};

#define INPUT_AFT_IOCTL_CMD_GET_TS_FINGERS_INFO \
    _IOWR(INPUT_AFT_GET_IO_TYPE, 0x01, \
struct ts_fingers)

#define INPUT_AFT_IOCTL_CMD_GET_ALGO_PARAM_INFO \
    _IOWR(INPUT_AFT_GET_IO_TYPE, 0x02, \
struct ts_aft_algo_param)

#define INPUT_AFT_IOCTL_CMD_GET_DIFF_DATA_INFO \
    _IOWR(INPUT_AFT_GET_IO_TYPE, 0x03, char )



#define INPUT_AFT_IOCTL_CMD_SET_COORDINATES \
    _IOWR(INPUT_AFT_SET_IO_TYPE, 0x01, \
    struct ts_fingers)

#define INPUT_AFT_IOCTL_CMD_SET_SENSIBILITY_CFG \
	_IOWR(INPUT_AFT_SET_IO_TYPE, 0x02,  int)

#define INPUT_IOCTL_CMD_SET_FLIP_KEY \
	_IOWR(INPUT_AFT_SET_IO_TYPE, 0x03, unsigned int)

#define INPUT_IOCTL_CMD_SET_DOUBLE_CLICK \
	_IOWR(INPUT_AFT_SET_IO_TYPE, 0x04, struct double_click_data)

extern int copy_fingers_to_aft_daemon(struct ts_kit_platform_data *pdata, struct ts_fingers *fingers);
extern int ts_kit_misc_init(struct ts_kit_platform_data *pdata);
extern void ts_kit_misc_destory(void);
#endif
