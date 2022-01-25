
#include "inc/tee/hwaa_tee.h"
#include <linux/spinlock.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <securec.h>
#include "inc/base/hwaa_define.h"
#include "inc/base/hwaa_utils.h"
EXPORT_SYMBOL(saved_command_line);

static struct workqueue_struct *g_hwaa_workqueue = NULL;
static u32 g_uid;
static const u8 *g_hwaa_package_name = "/dev/bdkernel_ca";
static TEEC_Context g_hwaa_context = {0};
static TEEC_Session g_hwaa_session = {0};

static DEFINE_MUTEX(g_tee_inited_mutex);
static struct mutex g_tee_inited_mutex;
static bool g_is_tee_inited;
static const u32 g_paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INOUT,
						 TEEC_VALUE_INOUT,
						 TEEC_NONE,
						 TEEC_NONE);

bool init_hwaa_work_queue(void)
{
	if (!g_hwaa_workqueue)
		g_hwaa_workqueue = create_singlethread_workqueue(
			"TEE_SEESSION_WORK_QUEUE");
	return (g_hwaa_workqueue) ? true : false;
}

void destory_hwaa_work_queue(void)
{
	if (g_hwaa_workqueue)
		destroy_workqueue(g_hwaa_workqueue);
}

static s32 teec_err(TEEC_Result res)
{
	switch (res) {
	case TEEC_SUCCESS:
		return ERR_MSG_SUCCESS;
	case TEEC_ERROR_OUT_OF_MEMORY:
		return ERR_MSG_OUT_OF_MEMORY;
	case TEEC_ERROR_ITEM_NOT_FOUND:
		return ERR_MSG_KERNEL_PHASE1_KEY_NULL;
	case TEEC_ERROR_MAC_INVALID:
		return ERR_MSG_KERNEL_PHASE1_KEY_NOTMATCH;
	default:
		return ERR_MSG_GENERATE_FAIL;
	}
}

void hwaa_open_session(struct work_struct *work)
{
	u32 err_origin = 0;
	TEEC_UUID uuid = HWAA_TA_UUID;
	TEEC_Operation op;
	TEEC_Result res;

	struct async_send_work_t *teec_init_work;
	teec_init_work = container_of(work, struct async_send_work_t, work);
	res = TEEK_InitializeContext(NULL, &g_hwaa_context);
	if (res != TEEC_SUCCESS) {
		hwaa_pr_err("TEEK_InitializeContext returns %x", res);
		goto cleanup;
	}
	if (memset_s(&op, sizeof(op), 0, sizeof(op)) != EOK)
		goto cleanup;
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_NONE, TEEC_NONE,
		TEEC_MEMREF_TEMP_INPUT, TEEC_MEMREF_TEMP_INPUT);
	// prepare operation parameters
	op.started = 1;
	// params[2] is TEEC_MEMREF_TEMP_INPUT
	op.params[2].tmpref.buffer = (void *)&g_uid;
	op.params[2].tmpref.size = sizeof(g_uid);
	// params[3] is TEEC_MEMREF_TEMP_INPUT
	op.params[3].tmpref.buffer = (void *)g_hwaa_package_name;
	op.params[3].tmpref.size = strlen(g_hwaa_package_name) + 1;
	// Open a session to the TA
	res = TEEK_OpenSession(&g_hwaa_context, &g_hwaa_session, &uuid,
		TEEC_LOGIN_IDENTIFY, NULL, &op, &err_origin);
	hwaa_pr_info("TEEK_OpenSession returned %d and err_origin is %d",
		res, err_origin);

cleanup:
	teec_init_work->response = 1;
	teec_init_work->result = res;
	wake_up(&teec_init_work->notify_waitq);
}

s32 work_open_session(void)
{
	s32 ret = 0;
	struct async_send_work_t *teec_init_work =
		kzalloc(sizeof(struct async_send_work_t), GFP_KERNEL);
	if (!teec_init_work) {
		hwaa_pr_err("kmalloc teec_init_wrok failed");
		goto cleanup;
	}
	init_waitqueue_head(&teec_init_work->notify_waitq);
	INIT_WORK(&teec_init_work->work, hwaa_open_session);
	if (g_hwaa_workqueue)
		ret = queue_work(g_hwaa_workqueue, &teec_init_work->work);
	if (ret == 0) {
		hwaa_pr_err("teec_init add work queue failed!");
		goto cleanup;
	}
	wait_event(teec_init_work->notify_waitq, teec_init_work->response);
	if (teec_init_work->result != TEEC_SUCCESS) {
		hwaa_pr_err("work_open_session returned 0x%x",
			    teec_init_work->result);
		ret = 0;
	}

cleanup:
	if (teec_init_work) {
		kfree(teec_init_work);
		teec_init_work = NULL;
	}

	if (ret == 0)
		return ERR_MSG_GENERATE_FAIL;
	return ERR_MSG_SUCCESS;
}

s32 hwaa_init_tee(void)
{
	s32 ret = ERR_MSG_SUCCESS;

	mutex_lock(&g_tee_inited_mutex);
	if (!g_is_tee_inited) {
		ret = work_open_session();
		if (ret != ERR_MSG_SUCCESS)
			hwaa_pr_err(" work_open_session failed");
		else
			g_is_tee_inited = true;
	}
	mutex_unlock(&g_tee_inited_mutex);
	return ret;
}

void hwaa_invoke_ta(struct work_struct *work)
{
	TEEC_Result tee_res = TEEC_ERROR_GENERIC;
	u32 origin = 0;
	hwaa_ta_cmd cmd;
	struct async_send_work_t *teec_init_work =
		container_of(work, struct async_send_work_t, work);
	TEEC_Operation *op = teec_init_work->op;
	TEEC_Session *sess = teec_init_work->sess;
	cmd = teec_init_work->cmd;
	if (!sess) {
		hwaa_pr_err("open session is null");
		goto cleanup;
	} else if (!op) {
		hwaa_pr_err("operation is null");
		goto cleanup;
	}
	tee_res = TEEK_InvokeCommand(sess, cmd, op, &origin);
	if (tee_res != TEEC_SUCCESS) {
		hwaa_pr_err("InvokeCommand failed res=0x%x orig=0x%x cmd=%d",
			tee_res, origin, cmd);
		goto cleanup;
	}
	hwaa_pr_info("TEEK_InvokeCommand called successfully");

cleanup:
	teec_init_work->response = 1;
	teec_init_work->result = tee_res;
	wake_up(&teec_init_work->notify_waitq);
}

static s32 teec_work_init(u8 *shmem, u32 shmem_size, hwaa_ta_cmd cmd,
	struct async_send_work_t *teec_init_work, TEEC_Operation *op)
{
	s32 ret;

	// prepare operation parameters
	op->started = 1;
	op->paramTypes = g_paramTypes;
	// params[1] is TEEC_VALUE_INOUT
	op->params[1].value.a = shmem_size;
	// params[0] is TEEC_MEMREF_TEMP_INOUT
	op->params[0].tmpref.size = shmem_size;
	op->params[0].tmpref.buffer = shmem;
	op->session = &g_hwaa_session;
	if (!g_hwaa_workqueue) {
		hwaa_pr_err("workqueue is null,need to create first");
		return ERR_MSG_GENERATE_FAIL;
	}
	teec_init_work->sess = &g_hwaa_session;
	teec_init_work->op = op;
	teec_init_work->cmd = cmd;
	init_waitqueue_head(&(teec_init_work->notify_waitq));
	INIT_WORK(&(teec_init_work->work), hwaa_invoke_ta);
	ret = queue_work(g_hwaa_workqueue, &(teec_init_work->work));
	if (ret == 0) {
		hwaa_pr_err("teec_work_init add work queue failed!");
		return ERR_MSG_GENERATE_FAIL;
	}
	wait_event(teec_init_work->notify_waitq, teec_init_work->response);

	return ERR_MSG_SUCCESS;
}

/*
 * This function copy into the buffer shmem and no risk
 * @return true for success
 */
static bool build_shmem(u8 *shmem, u32 shmem_size, u64 profile_id,
	const u8 *cred, u32 cred_length)
{
	u32 size = shmem_size;
	if (memcpy_s(shmem, size, &profile_id, sizeof(profile_id)) != EOK)
		return false;
	shmem += sizeof(profile_id);
	size -= sizeof(profile_id);
	if (memcpy_s(shmem, size, &cred_length, sizeof(cred_length))
		!= EOK)
		return false;
	shmem += sizeof(cred_length);
	size -= sizeof(cred_length);
	if (memcpy_s(shmem, size, cred, cred_length) != EOK)
	       return false;
	return true;
}

static bool init_send_operation_and_para(TEEC_Operation *op,
	struct async_send_work_t *teec_init_work)
{
	if (memset_s(op, sizeof(TEEC_Operation), 0,
		sizeof(TEEC_Operation)) != EOK) {
		hwaa_pr_err("memset_s failed, should never be here!");
		return false;
	}
	if (memset_s(teec_init_work, sizeof(struct async_send_work_t), 0,
		sizeof(struct async_send_work_t)) != EOK) {
		hwaa_pr_err("memset_s failed, should never be here!");
		return false;
	}

	return true;
}

static bool check_para(const u8 *cred, u32 cred_length, u8 **phase1_key,
	u32 *phase1_key_size)
{
	if (!cred || !phase1_key || !phase1_key_size)
		return false;
	if (cred_length != PHASE_1_KEY_LENGTH)
		return false;

	return true;
}

s32 send_credential_request(u64 profile_id, const u8 *cred,
	u32 cred_length, hwaa_ta_cmd cmd, u8 **phase1_key,
	u32 *phase1_key_size)
{
	TEEC_Operation op;
	TEEC_Result tee_res;
	struct async_send_work_t teec_init_work;
	u32 shmem_size;
	u8 *shmem = NULL;
	s32 ret;

	if (!init_send_operation_and_para(&op, &teec_init_work))
		return ERR_MSG_GENERATE_FAIL;
	if (!check_para(cred, cred_length, phase1_key, phase1_key_size)) {
		hwaa_pr_err("bad value");
		return ERR_MSG_BAD_PARAM;
	}
	shmem_size = sizeof(profile_id) + sizeof(cred_length) + cred_length;
	shmem = kzalloc(shmem_size, GFP_KERNEL);
	if (!shmem)
		return ERR_MSG_OUT_OF_MEMORY;
	if (!build_shmem(shmem, shmem_size, profile_id, cred, cred_length)) {
		tee_res = TEEC_ERROR_GENERIC;
		goto done;
	}
	ret = teec_work_init(shmem, shmem_size, cmd, &teec_init_work, &op);
	if (ret != ERR_MSG_SUCCESS) {
		tee_res = TEEC_ERROR_GENERIC;
		goto done;
	}
	tee_res = teec_init_work.result;
	if (tee_res != TEEC_SUCCESS) {
		hwaa_pr_err("hwaa_invoke_ta failed,ret 0x%x", tee_res);
		goto done;
	}
	if (op.params[0].tmpref.size != PHASE_1_KEY_LENGTH) {
		hwaa_pr_err("phase1 key size %ld is wrong",
			    op.params[0].tmpref.size);
		tee_res = TEEC_ERROR_GENERIC;
		goto done;
	}
	*phase1_key = kzalloc(PHASE_1_KEY_LENGTH, GFP_KERNEL);
	if (*phase1_key == NULL) {
		tee_res = TEEC_ERROR_OUT_OF_MEMORY;
		goto done;
	}
	if (memcpy_s(*phase1_key, PHASE_1_KEY_LENGTH,
		op.params[0].tmpref.buffer, op.params[0].tmpref.size)) {
		tee_res = TEEC_ERROR_GENERIC;
		goto done;
	}
	*phase1_key_size = PHASE_1_KEY_LENGTH;
done:
	kzfree(shmem);
	shmem = NULL;

	return teec_err(tee_res);
}
