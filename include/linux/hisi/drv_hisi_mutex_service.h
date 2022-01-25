/* Copyright (c) 2008-2011, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 *  drv_hisi_mutex_service.h
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 */

#ifndef __DRV_HISI_MUTEX_SERVICE_H__
#define __DRV_HISI_MUTEX_SERVICE_H__

typedef int (*pPREEMPTFUNC )(int param);

#define START_OK 0
#define START_TIMEOUT 1
#define START_ERROR 2
#define STATUS_INVALID 3

#define STOP_OK 0
#define STOP_ERROR -1

#define NOTIFY_PREEMPT_STRING "NOTIFY_PREEMPT"
#define ABEND "ABEND"

#define STATUS_ALIVE 0
#define T2  3 //����T2ʱ��Ϊ3��

/*ҵ������*/
#define MUTEX_SERVICE_WIFI_NAME "WIFI_DISPLAY"
#define MUTEX_SERVICE_MIRROR_NAME "MIRROR"
#define MUTEX_SERVICE_HMP_NAME "HMP"
#define MUTEX_SERVICE_HDMI_NAME "HDMI"

#define MUTEX_SERVICE_GROUP_ID 0 /*�л����ϵ��ҵ������ID*/

/*ҵ��ID*/
enum hisi_mutex_service_id {
    MUTEX_SERVICE_WIFIDIS_ID = 0,
    MUTEX_SERVICE_MIRROR_ID = 1,
    MUTEX_SERVICE_HMP_ID = 2,
    MUTEX_SERVICE_HDMI_ID = 3,
};

/*ҵ�����ȼ�*/
enum hisi_mutex_service_priority {
    MUTEX_SERVICE_PRIORITY_0 = 0,
    MUTEX_SERVICE_PRIORITY_1 = 1,
    MUTEX_SERVICE_PRIORITY_2 = 2,
    MUTEX_SERVICE_PRIORITY_3 = 3,
    MUTEX_SERVICE_PRIORITY_4 = 4,
    MUTEX_SERVICE_PRIORITY_5 = 5,
    MUTEX_SERVICE_PRIORITY_6 = 6,
    MUTEX_SERVICE_PRIORITY_7 = 7,
    MUTEX_SERVICE_PRIORITY_8 = 8,
    MUTEX_SERVICE_PRIORITY_9 = 9,
};


extern int hisi_mutex_mng_service_start(int  hisi_id, int (*pPREEMPTFUNC)(int param));

extern int hisi_mutex_mng_service_stop(int  hisi_id);

extern int hisi_mutex_mng_notify_alive(int  hisi_id);
#endif