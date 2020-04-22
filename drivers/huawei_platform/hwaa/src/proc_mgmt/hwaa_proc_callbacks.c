/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 * Description: This file contains the function required for proc callback
 * Author: xuejikan 370596
 * Create: 2018-12-10
 */
#include "inc/proc_mgmt/hwaa_proc_callbacks.h"
#include "inc/proc_mgmt/hwaa_proc_mgmt.h"

static struct hwaa_proc_callbacks_t g_proc_callbacks = {
	.pre_execve = hwaa_proc_mgmt_pre_execve,
	.post_execve = hwaa_proc_mgmt_post_execve,
	.on_task_forked = hwaa_proc_mgmt_on_task_forked,
	.on_task_exit = hwaa_proc_mgmt_on_task_exit,
	.on_caps_setuid = hwaa_proc_mgmt_on_caps_setuid,
};

void hwaa_register_proc_callbacks_proxy()
{
	hwaa_register_proc_callbacks(&g_proc_callbacks);
}

void hwaa_unregister_proc_callbacks_proxy(void)
{
	hwaa_unregister_proc_callbacks();
}