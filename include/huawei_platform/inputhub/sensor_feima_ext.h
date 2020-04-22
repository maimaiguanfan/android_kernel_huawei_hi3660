/*
 *  Sensor Hub driver
 *
 * Copyright (C) 2018 Huawei, Inc.
 * Author: lishuai <lishuai88@huawei.com>
 *
 */

#ifndef __SENSOR_FEIMA_EXT_H_
#define __SENSOR_FEIMA_EXT_H_

void save_light_to_sensorhub(uint32_t mipi_level, uint32_t bl_level);
void send_dc_status_to_sensorhub(uint32_t dc_status);

#endif
