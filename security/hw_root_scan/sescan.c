/*
 * sescan.c
 *
 * the sescan.c for selinux status checking
 *
 * Yongzheng Wu <Wu.Yongzheng@huawei.com>
 * likun <quentin.lee@huawei.com>
 * likan <likan82@huawei.com>
 *
 * Copyright (c) 2001-2021, Huawei Tech. Co., Ltd. All rights reserved.
 */

#include "./include/sescan.h"

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 2, 0)
#include <linux/lsm_hooks.h>
#endif

static const char *TAG = "sescan";

int get_selinux_enforcing(void)
{
	return selinux_enforcing;
}

int sescan_hookhash(uint8_t *hash)
{
	int err;
	struct crypto_shash *tfm = crypto_alloc_shash("sha256", 0, 0);

	SHASH_DESC_ON_STACK(shash, tfm);

	if (IS_ERR(tfm)) {
		RSLogError(TAG, "crypto_alloc_hash(sha256) error %ld",
							PTR_ERR(tfm));
		return -ENOMEM;
	}

	shash->tfm = tfm;
	shash->flags = 0;

	err = crypto_shash_init(shash);
	if (err < 0) {
		RSLogError(TAG, "crypto_shash_init error: %d", err);
		crypto_free_shash(tfm);
		return err;
	}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 2, 0)
	struct security_operations **security_ops;

	security_ops = kallsyms_lookup_name("security_ops");
	if (!security_ops) {
		RSLogError(TAG, "get symbol security_ops addres failed");
		return -EFAULT;
	}

	crypto_shash_update(shash,(char *)*security_ops, sizeof(struct security_operations));
#else
// reference security/security.c: call_void_hook
#define DO_ONE_HEAD(FUNC) \
	do { \
		struct security_hook_list *P; \
		list_for_each_entry(P, &security_hook_heads.FUNC, list) { \
			crypto_shash_update(shash, (char *)&(P->hook.FUNC), sizeof(P->hook.FUNC)); \
		} \
	} while (0)
	// reference initialization in security_hook_heads in security/security.c
	DO_ONE_HEAD(binder_set_context_mgr);
	DO_ONE_HEAD(binder_transaction);
	DO_ONE_HEAD(binder_transfer_binder);
	DO_ONE_HEAD(binder_transfer_file);
	DO_ONE_HEAD(ptrace_access_check);
	DO_ONE_HEAD(ptrace_traceme);
	DO_ONE_HEAD(capget);
	DO_ONE_HEAD(capset);
	DO_ONE_HEAD(capable);
	DO_ONE_HEAD(quotactl);
	DO_ONE_HEAD(quota_on);
	DO_ONE_HEAD(syslog);
	DO_ONE_HEAD(settime);
	DO_ONE_HEAD(vm_enough_memory);
	DO_ONE_HEAD(bprm_set_creds);
	DO_ONE_HEAD(bprm_check_security);
	DO_ONE_HEAD(bprm_secureexec);
	DO_ONE_HEAD(bprm_committing_creds);
	DO_ONE_HEAD(bprm_committed_creds);
	DO_ONE_HEAD(sb_alloc_security);
	DO_ONE_HEAD(sb_free_security);
	DO_ONE_HEAD(sb_copy_data);
	DO_ONE_HEAD(sb_remount);
	DO_ONE_HEAD(sb_kern_mount);
	DO_ONE_HEAD(sb_show_options);
	DO_ONE_HEAD(sb_statfs);
	DO_ONE_HEAD(sb_mount);
	DO_ONE_HEAD(sb_umount);
	DO_ONE_HEAD(sb_pivotroot);
	DO_ONE_HEAD(sb_set_mnt_opts);
	DO_ONE_HEAD(sb_clone_mnt_opts);
	DO_ONE_HEAD(sb_parse_opts_str);
	DO_ONE_HEAD(dentry_init_security);
#ifdef CONFIG_SECURITY_PATH
	DO_ONE_HEAD(path_unlink);
	DO_ONE_HEAD(path_mkdir);
	DO_ONE_HEAD(path_rmdir);
	DO_ONE_HEAD(path_mknod);
	DO_ONE_HEAD(path_truncate);
	DO_ONE_HEAD(path_symlink);
	DO_ONE_HEAD(path_link);
	DO_ONE_HEAD(path_rename);
	DO_ONE_HEAD(path_chmod);
	DO_ONE_HEAD(path_chown);
	DO_ONE_HEAD(path_chroot);
#endif
	DO_ONE_HEAD(inode_alloc_security);
	DO_ONE_HEAD(inode_free_security);
	DO_ONE_HEAD(inode_init_security);
	DO_ONE_HEAD(inode_create);
	DO_ONE_HEAD(inode_link);
	DO_ONE_HEAD(inode_unlink);
	DO_ONE_HEAD(inode_symlink);
	DO_ONE_HEAD(inode_mkdir);
	DO_ONE_HEAD(inode_rmdir);
	DO_ONE_HEAD(inode_mknod);
	DO_ONE_HEAD(inode_rename);
	DO_ONE_HEAD(inode_readlink);
	DO_ONE_HEAD(inode_follow_link);
	DO_ONE_HEAD(inode_permission);
	DO_ONE_HEAD(inode_setattr);
	DO_ONE_HEAD(inode_getattr);
	DO_ONE_HEAD(inode_setxattr);
	DO_ONE_HEAD(inode_post_setxattr);
	DO_ONE_HEAD(inode_getxattr);
	DO_ONE_HEAD(inode_listxattr);
	DO_ONE_HEAD(inode_removexattr);
	DO_ONE_HEAD(inode_need_killpriv);
	DO_ONE_HEAD(inode_killpriv);
	DO_ONE_HEAD(inode_getsecurity);
	DO_ONE_HEAD(inode_setsecurity);
	DO_ONE_HEAD(inode_listsecurity);
	DO_ONE_HEAD(inode_getsecid);
	DO_ONE_HEAD(file_permission);
	DO_ONE_HEAD(file_alloc_security);
	DO_ONE_HEAD(file_free_security);
	DO_ONE_HEAD(file_ioctl);
	DO_ONE_HEAD(mmap_addr);
	DO_ONE_HEAD(mmap_file);
	DO_ONE_HEAD(file_mprotect);
	DO_ONE_HEAD(file_lock);
	DO_ONE_HEAD(file_fcntl);
	DO_ONE_HEAD(file_set_fowner);
	DO_ONE_HEAD(file_send_sigiotask);
	DO_ONE_HEAD(file_receive);
	DO_ONE_HEAD(file_open);
	DO_ONE_HEAD(task_create);
	DO_ONE_HEAD(task_free);
	DO_ONE_HEAD(cred_alloc_blank);
	DO_ONE_HEAD(cred_free);
	DO_ONE_HEAD(cred_prepare);
	DO_ONE_HEAD(cred_transfer);
	DO_ONE_HEAD(kernel_act_as);
	DO_ONE_HEAD(kernel_create_files_as);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 9, 0))
	DO_ONE_HEAD(kernel_fw_from_file);
	DO_ONE_HEAD(kernel_module_request);
	DO_ONE_HEAD(kernel_module_from_file);
#endif

	DO_ONE_HEAD(task_fix_setuid);
	DO_ONE_HEAD(task_setpgid);
	DO_ONE_HEAD(task_getpgid);
	DO_ONE_HEAD(task_getsid);
	DO_ONE_HEAD(task_getsecid);
	DO_ONE_HEAD(task_setnice);
	DO_ONE_HEAD(task_setioprio);
	DO_ONE_HEAD(task_getioprio);
	DO_ONE_HEAD(task_setrlimit);
	DO_ONE_HEAD(task_setscheduler);
	DO_ONE_HEAD(task_getscheduler);
	DO_ONE_HEAD(task_movememory);
	DO_ONE_HEAD(task_kill);
	DO_ONE_HEAD(task_wait);
	DO_ONE_HEAD(task_prctl);
	DO_ONE_HEAD(task_to_inode);
	DO_ONE_HEAD(ipc_permission);
	DO_ONE_HEAD(ipc_getsecid);
	DO_ONE_HEAD(msg_msg_alloc_security);
	DO_ONE_HEAD(msg_msg_free_security);
	DO_ONE_HEAD(msg_queue_alloc_security);
	DO_ONE_HEAD(msg_queue_free_security);
	DO_ONE_HEAD(msg_queue_associate);
	DO_ONE_HEAD(msg_queue_msgctl);
	DO_ONE_HEAD(msg_queue_msgsnd);
	DO_ONE_HEAD(msg_queue_msgrcv);
	DO_ONE_HEAD(shm_alloc_security);
	DO_ONE_HEAD(shm_free_security);
	DO_ONE_HEAD(shm_associate);
	DO_ONE_HEAD(shm_shmctl);
	DO_ONE_HEAD(shm_shmat);
	DO_ONE_HEAD(sem_alloc_security);
	DO_ONE_HEAD(sem_free_security);
	DO_ONE_HEAD(sem_associate);
	DO_ONE_HEAD(sem_semctl);
	DO_ONE_HEAD(sem_semop);
	DO_ONE_HEAD(netlink_send);
	DO_ONE_HEAD(d_instantiate);
	DO_ONE_HEAD(getprocattr);
	DO_ONE_HEAD(setprocattr);
	DO_ONE_HEAD(ismaclabel);
	DO_ONE_HEAD(secid_to_secctx);
	DO_ONE_HEAD(secctx_to_secid);
	DO_ONE_HEAD(release_secctx);
	DO_ONE_HEAD(inode_notifysecctx);
	DO_ONE_HEAD(inode_setsecctx);
	DO_ONE_HEAD(inode_getsecctx);
#ifdef CONFIG_SECURITY_NETWORK
	DO_ONE_HEAD(unix_stream_connect);
	DO_ONE_HEAD(unix_may_send);
	DO_ONE_HEAD(socket_create);
	DO_ONE_HEAD(socket_post_create);
	DO_ONE_HEAD(socket_bind);
	DO_ONE_HEAD(socket_connect);
	DO_ONE_HEAD(socket_listen);
	DO_ONE_HEAD(socket_accept);
	DO_ONE_HEAD(socket_sendmsg);
	DO_ONE_HEAD(socket_recvmsg);
	DO_ONE_HEAD(socket_getsockname);
	DO_ONE_HEAD(socket_getpeername);
	DO_ONE_HEAD(socket_getsockopt);
	DO_ONE_HEAD(socket_setsockopt);
	DO_ONE_HEAD(socket_shutdown);
	DO_ONE_HEAD(socket_sock_rcv_skb);
	DO_ONE_HEAD(socket_getpeersec_stream);
	DO_ONE_HEAD(socket_getpeersec_dgram);
	DO_ONE_HEAD(sk_alloc_security);
	DO_ONE_HEAD(sk_free_security);
	DO_ONE_HEAD(sk_clone_security);
	DO_ONE_HEAD(sk_getsecid);
	DO_ONE_HEAD(sock_graft);
	DO_ONE_HEAD(inet_conn_request);
	DO_ONE_HEAD(inet_csk_clone);
	DO_ONE_HEAD(inet_conn_established);
	DO_ONE_HEAD(secmark_relabel_packet);
	DO_ONE_HEAD(secmark_refcount_inc);
	DO_ONE_HEAD(secmark_refcount_dec);
	DO_ONE_HEAD(req_classify_flow);
	DO_ONE_HEAD(tun_dev_alloc_security);
	DO_ONE_HEAD(tun_dev_free_security);
	DO_ONE_HEAD(tun_dev_create);
	DO_ONE_HEAD(tun_dev_attach_queue);
	DO_ONE_HEAD(tun_dev_attach);
	DO_ONE_HEAD(tun_dev_open);
#endif  /* CONFIG_SECURITY_NETWORK */
#ifdef CONFIG_SECURITY_NETWORK_XFRM
	DO_ONE_HEAD(xfrm_policy_alloc_security);
	DO_ONE_HEAD(xfrm_policy_clone_security);
	DO_ONE_HEAD(xfrm_policy_free_security);
	DO_ONE_HEAD(xfrm_policy_delete_security);
	DO_ONE_HEAD(xfrm_state_alloc);
	DO_ONE_HEAD(xfrm_state_alloc_acquire);
	DO_ONE_HEAD(xfrm_state_free_security);
	DO_ONE_HEAD(xfrm_state_delete_security);
	DO_ONE_HEAD(xfrm_policy_lookup);
	DO_ONE_HEAD(xfrm_state_pol_flow_match);
	DO_ONE_HEAD(xfrm_decode_session);
#endif  /* CONFIG_SECURITY_NETWORK_XFRM */
#ifdef CONFIG_KEYS
	DO_ONE_HEAD(key_alloc);
	DO_ONE_HEAD(key_free);
	DO_ONE_HEAD(key_permission);
	DO_ONE_HEAD(key_getsecurity);
#endif  /* CONFIG_KEYS */
#ifdef CONFIG_AUDIT
	DO_ONE_HEAD(audit_rule_init);
	DO_ONE_HEAD(audit_rule_known);
	DO_ONE_HEAD(audit_rule_match);
	DO_ONE_HEAD(audit_rule_free);
#endif /* CONFIG_AUDIT */
#endif /* LINUX_VERSION_CODE */
	err = crypto_shash_final(shash, (u8 *)hash);
	RSLogDebug(TAG, "sescan result %d", err);

	crypto_free_shash(tfm);
	return err;
}

