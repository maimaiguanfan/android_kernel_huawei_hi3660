

#ifndef _HWAA_TEE_H
#define _HWAA_TEE_H
#include <linux/types.h>
#include <linux/wait.h>
#include <linux/workqueue.h>
#include "teek_client_api.h"


#define BICDROID_KERNEL_TA_UUID { 0xb0b71695, 0x2913, 0x4fc1, \
	{ 0x8e, 0x7f, 0x42, 0x7d, 0x92, 0x21, 0x22, 0x47 }}

typedef enum {
	HWAA_INIT_USER = 10,
	SET_QCODE_KEY,
	CLEAR_QCODE_KEY,
} hwaa_ta_cmd;

struct async_send_work_t {
	struct work_struct work;
	wait_queue_head_t notify_waitq;
	TEEC_Operation *op;
	TEEC_Session *sess;
	hwaa_ta_cmd cmd;
	TEEC_Result result;
	s32 response;
};

s32 hwaa_init_tee(void);

s32 send_qcode_request_towards_tee(u64 profile_id, const u8 *qcode,
	u32 qcode_length, hwaa_ta_cmd cmd, u8 ** phase1_key,
	u32 *phase1_key_size);

bool init_hwaa_work_queue(void);

void destory_hwaa_work_queue(void);
#endif
