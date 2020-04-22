

#include "ril_sim_netlink.h"
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/mutex.h>
#include <linux/semaphore.h>
#include <linux/time.h>
#include <linux/spinlock.h>
#include <linux/skbuff.h>
#include <linux/types.h>
#include <linux/kthread.h>
#include <linux/socket.h>
#include <linux/netlink.h>
#include <uapi/linux/netlink.h>
#include <net/sock.h>
#include <net/netlink.h>
#include <net/ip.h>
#include <net/tcp.h>
#include <net/route.h>
#include <net/inet_hashtables.h>
#include <net/net_namespace.h>
#include <huawei_platform/log/hw_log.h>

#undef HWLOG_TAG
#define HWLOG_TAG           ril_sim_netlink

HWLOG_REGIST();
MODULE_LICENSE("GPL");

DEFINE_MUTEX(sim_receive_sem);
DEFINE_MUTEX(sim_send_sem);

#define KNL_SUCCESS         0
#define KNL_FAILED          -1

static u32 req_pid = 0;
static u32 rcv_msg_type = 0;
static int req_slot_id = -1;

static struct sock *sim_nlfd = NULL;    /* netlink socket fd */
static struct task_struct *sim_netlink_task = NULL;
static struct semaphore sim_netlink_sync_sema;
static knl_sim_pin_info sim_pin_info[MAX_SIM_CNT];

static int is_slot_valid(int slot)
{
	if (slot < 0 || slot >= MAX_SIM_CNT) {
		hwlog_err("%s: Invalid slot: %d\n", __func__, slot);
		return 0;
	}
	return 1;
}

static int process_sim_update_msg(struct nlmsghdr *nlh)
{
	int slot_id;
	knl_sim_pin_info *pin_info = (knl_sim_pin_info *)nlmsg_data(nlh);

	if (!pin_info) {
		hwlog_err("%s: pin_info is NULL\n", __func__);
		return KNL_FAILED;
	}

	slot_id = pin_info->sim_slot_id;
	if (!is_slot_valid(slot_id)) {
		hwlog_err("%s: with invalid slot_id: %d\n", __func__, slot_id);
		return KNL_FAILED;
	}

	memcpy(&sim_pin_info[slot_id], pin_info, sizeof(*pin_info));
	hwlog_info("%s: sim%d success\n", __func__, slot_id);
	return KNL_SUCCESS;
}

static int process_sim_get_pin_msg(struct nlmsghdr *nlh)
{
	int slot_id;
	knl_ril_sim_card_info *p_card_info = (knl_ril_sim_card_info *)nlmsg_data(nlh);

	if (!p_card_info) {
		hwlog_err("%s: p_card_info is NULL\n", __func__);
		return KNL_FAILED;
	}

	slot_id = p_card_info->sim_slot_id;
	if (!is_slot_valid(slot_id)) {
		hwlog_err("%s: SIM_GET_PIN_INFO_REQ with invalid slot_id: %d\n",
			__func__, slot_id);
		return KNL_FAILED;
	}

	rcv_msg_type = NETLINK_KNL_SIM_GET_PIN_INFO_REQ;
	req_pid = nlh->nlmsg_pid;
	req_slot_id = slot_id;

	up(&sim_netlink_sync_sema);
	hwlog_info("%s: sim%d success\n", __func__, slot_id);
	return KNL_SUCCESS;
}

static int process_sim_msg(struct nlmsghdr *nlh)
{
	int slot_id;

	if (nlh->nlmsg_type == NETLINK_KNL_SIM_UPDATE_IND_MSG) {
		return process_sim_update_msg(nlh);
	} else if (nlh->nlmsg_type == NETLINK_KNL_SIM_GET_PIN_INFO_REQ) {
		return process_sim_get_pin_msg(nlh);
	} else if (nlh->nlmsg_type == NETLINK_KNL_SIM_QUERY_VARIFIED_REQ) {
		rcv_msg_type = NETLINK_KNL_SIM_QUERY_VARIFIED_REQ;
		req_pid = nlh->nlmsg_pid;
		up(&sim_netlink_sync_sema);
		hwlog_info("%s: SIM_QUERY_VARIFIED_REQ success\n", __func__);
	} else if (nlh->nlmsg_type == NETLINK_KNL_SIM_CLEAR_ALL_VARIFIED_FLG) {
		for (slot_id = 0; slot_id < MAX_SIM_CNT; ++slot_id)
			sim_pin_info[slot_id].is_verified = 0; // 0 means not verified
		hwlog_info("%s: SIM_CLEAR_ALL_VARIFIED_FLG success\n", __func__);
	} else {
		hwlog_err("%s: Invalid message from ril, nlmsg_type: %d\n",
			__func__, nlh->nlmsg_type);
	}

	return KNL_SUCCESS;
}

/*
 * netlink socket's callback function, it will be called by system when
 * user space send a message to kernel.
 */
static void kernel_ril_sim_receive(struct sk_buff *__skb)
{
	int ret;
	struct nlmsghdr *nlh;
	struct sk_buff *skb;

	hwlog_info("%s: enter\n", __func__);
	if (!__skb) {
		hwlog_err("%s: Invalid parameter: NULL pointer(__skb)\n", __func__);
		return;
	}

	skb = skb_get(__skb);
	if (!skb) {
		hwlog_err("%s: skb_get return NULL\n", __func__);
		return;
	}

	mutex_lock(&sim_receive_sem);
	if (skb->len >= NLMSG_HDRLEN) {
		nlh = nlmsg_hdr(skb);
		if (!nlh || nlh->nlmsg_pid <= 0) {
			hwlog_err("%s: nlmsg_hdr is NULL or invalid nlmsg_pid\n", __func__);
			goto end;
		}

		if (nlh->nlmsg_len >= sizeof(*nlh) && skb->len >= nlh->nlmsg_len) {
			hwlog_info("%s: receive message form user space, nlmsg_type: %d, nlmsg_pid: %d\n",
				__func__, nlh->nlmsg_type, nlh->nlmsg_pid);
			ret = process_sim_msg(nlh);
			hwlog_info("%s: process_sim_msg ret: %d\n", __func__, ret);
		}
	} else {
		hwlog_err("%s: Invalid len message, skb->len: %d\n", __func__, skb->len);
	}
end:
	hwlog_info("%s: exit, skb->len: %d\n", __func__, skb->len);
	kfree_skb(skb);
	mutex_unlock(&sim_receive_sem);
}

static int send_get_pin_info_cnf(u32 pid, int slot_id)
{
	int ret;
	int size;
	struct sk_buff *skb;
	struct nlmsghdr *nlh;

	mutex_lock(&sim_send_sem);

	if (!pid || !sim_nlfd) {
		hwlog_err("%s: Invalid pid or sim_nlfd, pid: %d\n", __func__, pid);
		ret = KNL_FAILED;
		goto end;
	}

	if (!is_slot_valid(slot_id)) {
		hwlog_err("%s: Invalid slot_id: %d\n", __func__, slot_id);
		ret = KNL_FAILED;
		goto end;
	}

	size = sizeof(knl_sim_pin_info);
	/* socket buffer size: socket head size + netlink size + user msg size */
	skb = alloc_skb(size, GFP_ATOMIC);
	if (!skb) {
		hwlog_err("%s: alloc skb fail\n", __func__);
		ret = KNL_FAILED;
		goto end;
	}

	/* fill part of netlink: skb, pid, seq, type, len, flags */
	nlh = nlmsg_put(skb, pid, 0, NETLINK_KNL_SIM_GET_PIN_INFO_CNF, size, 0);
	if (!nlh) {
		hwlog_err("%s: nlmsg_put return fail\n", __func__);
		kfree_skb(skb);
		skb = NULL;
		ret = KNL_FAILED;
		goto end;
	}

	/* from kernel */
	NETLINK_CB(skb).portid = 0;
	NETLINK_CB(skb).dst_group = 0;

	/* fill user data */
	memcpy(nlmsg_data(nlh), &sim_pin_info[slot_id], size);

	/* skb will be freed in netlink_unicast */
	ret = netlink_unicast(sim_nlfd, skb, pid, MSG_DONTWAIT);
	hwlog_info("%s: send to ril, pid: %d, ret: %d\n", __func__, pid, ret);
end:
	mutex_unlock(&sim_send_sem);
	return ret;
}

static int send_query_verified_cnf(u32 pid)
{
	int ret;
	int size;
	int slot_id;
	int is_verified = 1; // 1 means verified
	struct sk_buff *skb;
	struct nlmsghdr *nlh;

	mutex_lock(&sim_send_sem);

	if (!pid || !sim_nlfd) {
		hwlog_err("%s: Invalid pid or sim_nlfd, pid: %d\n", __func__, pid);
		ret = KNL_FAILED;
		goto end;
	}

	size = sizeof(int);
	/* socket buffer size: socket head size + netlink size + user msg size */
	skb = alloc_skb(size, GFP_ATOMIC);
	if (!skb) {
		hwlog_err("%s: alloc skb fail\n", __func__);
		ret = KNL_FAILED;
		goto end;
	}

	/* fill part of netlink: skb, pid, seq, type, len, flags */
	nlh = nlmsg_put(skb, pid, 0, NETLINK_KNL_SIM_QUERY_VARIFIED_CNF, size, 0);
	if (!nlh) {
		hwlog_err("%s: nlmsg_put return fail\n", __func__);
		kfree_skb(skb);
		skb = NULL;
		ret = KNL_FAILED;
		goto end;
	}

	/* from kernel */
	NETLINK_CB(skb).portid = 0;
	NETLINK_CB(skb).dst_group = 0;

	/* fill user data */
	for (slot_id = 0; slot_id < MAX_SIM_CNT; ++slot_id) {
		/* PIN valid, but not verified, give back 0 */
		if (sim_pin_info[slot_id].is_valid_flg &&
			!(sim_pin_info[slot_id].is_verified)) {
			is_verified = 0; // 0 means not verified
			break;
		}
	}
	memcpy(nlmsg_data(nlh), &is_verified, size);

	/* skb will be freed in netlink_unicast */
	ret = netlink_unicast(sim_nlfd, skb, pid, MSG_DONTWAIT);
	hwlog_info("%s: send to ril, pid: %d, is_verified: %d, ret: %d\n",
		__func__, pid, is_verified, ret);
end:
	mutex_unlock(&sim_send_sem);
	return ret;
}

static int sim_netlink_thread(void *data)
{
	int ret;

	hwlog_info("%s: enter\n", __func__);
	while (1) {
		if (kthread_should_stop())
			break;

		/* netlink thread will block at this semaphone when no message received,
		only receive message from user space and then up the sema, this thread
		will go to next sentence. */
		down(&sim_netlink_sync_sema);
		hwlog_info("%s: rcv_msg_type: %d\n", __func__, rcv_msg_type);

		if (rcv_msg_type == NETLINK_KNL_SIM_GET_PIN_INFO_REQ) {
			ret = send_get_pin_info_cnf(req_pid, req_slot_id);
			hwlog_info("%s: send_get_pin_info_cnf slot%d ret: %d\n",
				__func__, req_slot_id, ret);
		} else if (rcv_msg_type == NETLINK_KNL_SIM_QUERY_VARIFIED_REQ) {
			ret = send_query_verified_cnf(req_pid);
			hwlog_info("%s: send_query_verified_cnf ret: %d\n", __func__, ret);
		}
	}

	hwlog_info("%s: end\n", __func__);
	return KNL_SUCCESS;
}

static void sim_netlink_init(void)
{
	struct netlink_kernel_cfg ril_nl_cfg = {
		.input = kernel_ril_sim_receive,
	};

	hwlog_info("%s: enter\n", __func__);

	sim_nlfd = netlink_kernel_create(&init_net, NETLINK_RIL_EVENT_SIM, &ril_nl_cfg);
	if (!sim_nlfd) {
		hwlog_err("%s: netlink_kernel_create failed\n", __func__);
		return;
	} else {
		hwlog_info("%s: netlink_kernel_create success\n", __func__);
	}

	sema_init(&sim_netlink_sync_sema, 0);
	memset(sim_pin_info, 0, sizeof(sim_pin_info));
	sim_netlink_task = kthread_run(sim_netlink_thread, NULL, "sim_netlink_thread");

	hwlog_info("%s: end\n", __func__);
}

static void sim_netlink_deinit(void)
{
	if (sim_nlfd && sim_nlfd->sk_socket) {
		sock_release(sim_nlfd->sk_socket);
		sim_nlfd = NULL;
	}

	if (sim_netlink_task) {
		kthread_stop(sim_netlink_task);
		sim_netlink_task = NULL;
	}
}

static int __init sim_netlink_module_init(void)
{
	sim_netlink_init();
	return 0;
}

static void __exit sim_netlink_module_exit(void)
{
	sim_netlink_deinit();
}

module_init(sim_netlink_module_init);
module_exit(sim_netlink_module_exit);

