/*
 * Huawei Touchscreen Driver
 *
 * Copyright (C) 2017 Huawei Device Co.Ltd
 * License terms: GNU General Public License (GPL) version 2
 *
 */

#ifndef __HUAWEI_THP_ATTR_H_
#define __HUAWEI_THP_ATTR_H_

int is_tp_detected(void);
int thp_set_prox_switch_status(bool enable);
bool thp_get_prox_switch_status(void);

#endif
