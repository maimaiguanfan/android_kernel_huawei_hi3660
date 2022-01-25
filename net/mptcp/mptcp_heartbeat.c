

#include <net/mptcp.h>
#include <net/sock.h>
#include <linux/tcp.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/semaphore.h>
#include <linux/mutex.h>
#include <linux/ip.h>
#include <linux/skbuff.h>
#include <linux/inetdevice.h>
#include <linux/types.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/syscalls.h>
#include <net/mptcp_heartbeat.h>
#include <linux/kernel.h>

/* marco and constant definitions */
#define HB_LOGE(fmt, args...)					\
	do {								\
		pr_err("mpheartbeat : " fmt, ##args);	\
	} while (0)

#define HB_LOGD(fmt, args...)					\
	do {								\
		pr_err("mpheartbeat : " fmt, ##args);	\
	} while (0)

#define CHECK_MSG_TYPE(x)         ((x > MIN_MSG) && (x < MAX_MSG))
#define CHECK_DETECTION_TYPE(x)                 (((x) & LINK_MASK) > 0)
#define CHECK_DETECTION_RESULT(x)          ((x ==LINK_OK) || (x==LINK_BROKEN))
#define CONVERT_BITSHIFT_TO_MSG(bitshift)       (bitshift)      // note that this is due to the tricky design
#define LINK_MASK                                   ((1<<LINK_NUM) - 1)

#define HEARTBEAT_REQUEST                   (11)
#define HEARTBEAT_RESPONSE                  (12)

#define HEARTBEAT_DEFAULT_RETRY_COUT               (3)
#define HEARTBEAT_DEFAULT_RETRY_INTERVAL (1)
#define HEARTBEAT_DEFAULT_TIMEOUT                (5)    // in second

#define INVALID_VALUE                            (-1)

/* structure definitions */
typedef struct __reception_thread_parameters
{
	struct socket *sock;
	char *buffer;
	int interval;
	int retry_count;
} reception_thread_parameters;

typedef enum __heartbeat_queue_msg_type
{
	MIN_MSG = -1,
	WIFI_HEARTBEAT_TRIGGERED,              // it is a little tricky here, remember to set the values in accordance with detection_type
	LTE_HEARTBEAT_TRIGGERED,
	STOP,                                 // put STOP above MAX_MSG
	MAX_MSG,
} heartbeat_queue_msg_type;

typedef struct __heartbeat_msg
{
	heartbeat_queue_msg_type msg_type;
	void* para;
	struct __heartbeat_msg* next;
} heartbeat_msg;

/* global variables */
static int g_heartbeat_retry_count = HEARTBEAT_DEFAULT_RETRY_COUT;
static struct sockaddr_in g_link_addrs[LINK_NUM] = {0};
static bool g_heartbeat_initialized  = false;
static volatile int g_detection_result = INVALID_VALUE;

static spinlock_t g_heartbeat_parameter_lock;      // lock g_heartbeat_interval, g_heartbeat_retry_count, g_link_addrs
static heartbeat_msg* g_msg_queue = NULL;
static struct semaphore g_msg_queue_sema;
static spinlock_t g_msg_queue_lock;
static struct semaphore g_detection_sema;
static spinlock_t g_detection_lock;

static struct task_struct* g_heartbeat_msg_task = NULL;
static struct task_struct* g_reception_task= NULL;

static const char* IF_NAMES[LINK_NUM] = IF_NAME_LIST;

/* functions */
static int heartbeat_msg_loop(void* para);
static void handle_msg(heartbeat_msg* msg);
static int detect(int type);
static void report_detection_result(int type, int result, HEARTBEAT_DETECTION_CALLBACK cb);
static void insert_msg(int msg_type, void* para);
static void clear_msg_queue(void);
static int prepare_socket(struct socket* sock, char* ifname);
static int bind_to_device(struct net *net, struct socket* sock, char* ifname);
static int reception_thread(void* data);

/*
 *	initialization function, called by emcom kernel module
 *	input param : link_addrs the addresses of all links, size LINK_NUM
 *	            retry_count,
 */
void heartbeat_init(struct sockaddr_in* link_addrs, int retry_count)
{
	if (retry_count <= 0 || NULL == link_addrs)
	{
		HB_LOGE("illegal parameters, init failed!");
		return;
	}
	if (g_heartbeat_initialized)
	{
		HB_LOGE("try to reinitialize heartbeat, abort");
		return;
	}

	/* init locks */
	spin_lock_init(&g_heartbeat_parameter_lock);     // this macro essentially sets lock to UNLOCK status
	sema_init(&g_msg_queue_sema, 0);
	spin_lock_init(&g_msg_queue_lock);
	sema_init(&g_detection_sema, 0);
	spin_lock_init(&g_detection_lock);

	/* record the parameters */
	spin_lock_bh(&g_heartbeat_parameter_lock);
	g_heartbeat_retry_count = retry_count;
	memcpy(g_link_addrs, link_addrs, sizeof(g_link_addrs));
	spin_unlock_bh(&g_heartbeat_parameter_lock);

	/* start the heartbeat thread */
	if (NULL == g_heartbeat_msg_task)    // theoretically it should be NULL
	{
		g_heartbeat_msg_task = kthread_run(heartbeat_msg_loop, NULL, "heartbeat_process");
	}
	g_heartbeat_initialized = true;

	HB_LOGD("heartbeat initialized!");
}
EXPORT_SYMBOL(heartbeat_init);

/*
 *	de-initialization function, called by emcom kernel module
 *    intuitively this is needed
 */
 void heartbeat_deinit(void)
{
	if (!g_heartbeat_initialized)
	{
		HB_LOGE("try to deinit an un-initialized heartbeat,abort");
		return;
	}
	spin_lock_bh(&g_heartbeat_parameter_lock);
	g_heartbeat_retry_count = HEARTBEAT_DEFAULT_RETRY_COUT;
	memset(g_link_addrs, 0, sizeof(g_link_addrs));
	spin_unlock_bh(&g_heartbeat_parameter_lock);

	// do NOT set the detection result here, otherwise dead lock will occur
	// the msg thread CANNOT be stopped by kthread_stop() since it would block this thread, so send a stop msg to the queue
	insert_msg(STOP, NULL);    // let the thread stop itself, it might be necessary to put STOP msg in the head
	g_heartbeat_msg_task = NULL;
	g_heartbeat_initialized = false;
}
EXPORT_SYMBOL(heartbeat_deinit);

/*
 *	trigger one heartbeat detection, called by emcom kernel module
 *	input param : type, can be a combination of multiple links (each bit indicates a link)
 */
void heartbeat_trigger(int type, HEARTBEAT_DETECTION_CALLBACK cb_fun)
{
	int bit_shift = 0;
	void* para = (void*) cb_fun;
	if (!CHECK_DETECTION_TYPE(type) || NULL == cb_fun)
	{
		HB_LOGE("illegal detection type or null callback function in heartbeat_trigger()");
		return;
	}
	if (!g_heartbeat_initialized)
	{
		HB_LOGE("try to trigger an un-initialized heartbeat, abort");
		return;
	}

	// no need to record this type, since it won't be queried and the detection process won't be interrupted
	/* this loop design applies even if LINK_NUM increases */
	for (bit_shift = 0; bit_shift < LINK_NUM; bit_shift++)
	{
		if (((type >> bit_shift) & 1) == 1)
		{
			insert_msg(CONVERT_BITSHIFT_TO_MSG(bit_shift), para);
		}
	}

	HB_LOGD("heartbeat triggered with type %d", type);
}
EXPORT_SYMBOL(heartbeat_trigger);

/*
 *	heartbeat thread entry function
 */
static int heartbeat_msg_loop(void* para)
{
	heartbeat_msg* current_msg;
	HB_LOGD("entered heartbeat_msg_loop");

	/* message loop */
	while(true)               // should NOT be stopped by kthread_should_stop()
	{
		//if all messages are handled, sleep and wait for new message
		HB_LOGD("try to lock in heartbeat_msg_loop");
		spin_lock_bh(&g_msg_queue_lock);
		while(NULL == g_msg_queue)
		{
			spin_unlock_bh(&g_msg_queue_lock);
			down(&g_msg_queue_sema);		// TODO: if a timeout is needed?
			HB_LOGD("wake up in heartbeat_msg_loop");
			spin_lock_bh(&g_msg_queue_lock);
		}
		// at this point, the lock is obtained
		if (STOP == g_msg_queue->msg_type)
		{
			HB_LOGD("heartbeat_msg_loop handles a STOP msg");
			spin_unlock_bh(&g_msg_queue_lock);    // note this, do not cause dead lock
			break;
		}
		current_msg = g_msg_queue;
		g_msg_queue = g_msg_queue->next;
		HB_LOGD("try to unlock in heartbeat_msg_loop");
		spin_unlock_bh(&g_msg_queue_lock);
		handle_msg(current_msg);
		kfree(current_msg);		// free this message, otherwise memery leak may occur
	}

	clear_msg_queue();
	HB_LOGD("heartbeat_msg_loop exits gracefully");
	return 0;
}

/*
 *	handle messages in msg queue
 */
static void handle_msg(heartbeat_msg* msg)
{
	heartbeat_queue_msg_type type = MIN_MSG;
	void* para = NULL;
	int status = INVALID_VALUE;
	if (NULL == msg)
	{
		HB_LOGE("null msg in handle_msg");
		return;
	}
	type = msg->msg_type;
	para = msg->para;
	HB_LOGD("about to handle a msg %d", msg);
	switch(type)
	{
		case WIFI_HEARTBEAT_TRIGGERED:
			status = detect(WIFI_DETECTION);
			report_detection_result(WIFI_DETECTION, status, (HEARTBEAT_DETECTION_CALLBACK)para);
			break;
		case LTE_HEARTBEAT_TRIGGERED:
			status = detect(LTE_DETECTION);
			report_detection_result(LTE_DETECTION, status, (HEARTBEAT_DETECTION_CALLBACK)para);
			break;
		default:
			HB_LOGE("impossible branch in handle_msg");
	}
}

/*
 *	report the detection result to the callback function, no matter the link is broken or not
 */
static void report_detection_result(int type, int result, HEARTBEAT_DETECTION_CALLBACK cb_fun)
{
/*
 *	there were 3 alternative mechanisms of detection report:
 *	1. by msg to daemon
 *	2. add listeners. The problem is, multiple modules may trigger heartbeat thus not all listeners need to know every detecion
 *	3. impose a callback in trigger() function. This looks good, however as delay may occur, not sure if an extra "serial number" is needed
 *	mechanism 3 is eventually applied
 */
	if (!CHECK_DETECTION_TYPE(type) || !CHECK_DETECTION_RESULT(result) || NULL == cb_fun)
	{
		HB_LOGE("illegal parameter in report_detection_result");
		return;
	}

	(*cb_fun)(type, result);
	HB_LOGD("reported a heartbeat detection, type %d, result %d, callback %d", type, result, cb_fun);
}

/*
 *	insert message to the tail of the msg queue
 */
static void insert_msg(int msg_type, void* para)
{
	heartbeat_msg* msg = NULL;
	heartbeat_msg* search = NULL;
	if (!CHECK_MSG_TYPE(msg_type))
	{
		HB_LOGE("illegal msg type %d", msg_type);
		return;
	}

	msg = (heartbeat_msg*)kmalloc(sizeof(heartbeat_msg), GFP_KERNEL);      // remeber to free this pointer somewhere else
	msg->msg_type = msg_type;
	msg->para = para;
	msg->next = NULL;
	spin_lock_bh(&g_msg_queue_lock);
	if (NULL == g_msg_queue)
	{
		g_msg_queue = msg;
		spin_unlock_bh(&g_msg_queue_lock);
		up(&g_msg_queue_sema);                            // remeber to trigger the sema
		HB_LOGD("inserted a msg %d to the queue", msg_type);
		return;
	}
	// search for the tail and insert
	search = g_msg_queue;
	while(NULL != search->next)
	{
		search = search->next;
	}
	search->next = msg;
	spin_unlock_bh(&g_msg_queue_lock);
	up(&g_msg_queue_sema);                            // remeber to trigger the sema
	HB_LOGD("inserted a msg %d to the queue", msg_type);
}

/*
 *	clear the msg queue, should only be called when the msg loop is exited
 */
static void clear_msg_queue(void)
{
	heartbeat_msg* search = NULL;
	spin_lock_bh(&g_msg_queue_lock);
	search = g_msg_queue;
	while(NULL != g_msg_queue)
	{
		search = g_msg_queue;
		g_msg_queue = g_msg_queue->next;
		kfree(search);
	}
	spin_unlock_bh(&g_msg_queue_lock);
	HB_LOGD("cleared the msg queue");
}

/*
 *	detect the corresponding link, one at a time
 *    currently type is either WIFI_DETECTION (value 1) or LTE_DETECTION (value 2)
 */
static int detect(int type)
{
	int status = INVALID_VALUE;
	int index = 0;
	struct sockaddr_in dest_addr = {0};
	struct socket* sock;
	reception_thread_parameters* thread_info;
	char* buffer = "test";            // TODO:
	int buf_len = 5;
	char* ifname = NULL;
	int retry_count = 0;
	struct sockaddr_in link_addrs[LINK_NUM] = {0};

	// it is neccessary to check the type first
	if (type < 0 || type > LINK_MASK || (type % 2 != 0 && type != 1) )
	{
		HB_LOGE("illgal type in detect(), abort");
		return INVALID_VALUE;
	}
	/* read the global parameters */
	spin_lock_bh(&g_heartbeat_parameter_lock);
	if (g_heartbeat_retry_count < 1 || NULL == g_link_addrs)
	{
		HB_LOGE("illegal heartbeat parameter in detect(), CRITICAL!");
		spin_unlock_bh(&g_heartbeat_parameter_lock);
		return INVALID_VALUE;
	}
	retry_count = g_heartbeat_retry_count;
	memcpy(&link_addrs, g_link_addrs, sizeof(struct sockaddr_in)*LINK_NUM);
	spin_unlock_bh(&g_heartbeat_parameter_lock);

	/* fill the dest_addr according to the type */
	for (index = 0; index < LINK_NUM; index++)
	{
		if (((type >> index) & 1) == 1)
		{
			memcpy(&dest_addr, &link_addrs[index], sizeof(dest_addr));
			ifname = IF_NAMES[index];    // TODO: CHECK THIS
			break;
		}
	}

	/* prepare socket */
	sock = (struct socket *)kmalloc(sizeof(struct socket),GFP_KERNEL);
	status = prepare_socket(sock, ifname);
	if (0 != status)
	{
		HB_LOGE("failed to prepare socket");
		return INVALID_VALUE;
	}
	/* start a reception thread */
	thread_info = kmalloc(sizeof(reception_thread_parameters), GFP_KERNEL);    // remember to free this pointer  in the thread
	if (!thread_info)
	{
		HB_LOGE("kmalloc threadinfo err in detect(), CRITICAL");
		kernel_sock_shutdown(sock, SHUT_RDWR);
		release_sock(sock->sk);
		g_reception_task = NULL;
		return INVALID_VALUE;
	}
	thread_info->sock = sock;
	thread_info->buffer = buffer;
	thread_info->interval = HEARTBEAT_DEFAULT_TIMEOUT;
	thread_info->retry_count = 0;                                              // in the first, 1 heartbeat packet will be sent
	g_reception_task = kthread_run(reception_thread, thread_info, "reception_thread");
	if (NULL == g_reception_task)
	{
		HB_LOGE("failed to start reception task");
		kernel_sock_shutdown(sock, SHUT_RDWR);
		release_sock(sock);
		kfree(thread_info);
		return INVALID_VALUE;
	}
	/* start to send 1 heartbeat packet */
	sys_sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
	HB_LOGD("sent a heartbeat packet to destination");
	//wait for the response from the reception thread
	spin_lock_bh(&g_detection_lock);
	while(INVALID_VALUE == g_detection_result) // any "g_detection_result = INVALID_VALUE" operation must be carefully considered
	{
		spin_unlock_bh(&g_detection_lock);
		down(&g_detection_sema);
		spin_lock_bh(&g_detection_lock);
	}
	status = g_detection_result;
	spin_unlock_bh(&g_detection_lock);
	if (LINK_OK == status)
	{
		HB_LOGD("heartbeat triggered and LINK_OK detected, link type %d", type);
		kernel_sock_shutdown(sock, SHUT_RDWR);
		release_sock(sock->sk);
		return status;
	}

	HB_LOGD("result of first detection is LINK_BROKEN");
	status = INVALID_VALUE;
	/* if the first heartbeat failed, do retry count times more */
	// start another reception task thread
	thread_info = kmalloc(sizeof(reception_thread_parameters), GFP_KERNEL); // it should be safe because the reception thread triggers the detection sema only when it exits
	if (!thread_info)
	{
		HB_LOGE("kmalloc threadinfo err in detect(), CRITICAL!\n");
		kernel_sock_shutdown(sock, SHUT_RDWR);
		release_sock(sock->sk);
		g_reception_task = NULL;
		return INVALID_VALUE;
	}
	thread_info->sock = sock;
	thread_info->buffer = buffer;
	thread_info->interval = HEARTBEAT_DEFAULT_TIMEOUT;
	thread_info->retry_count = retry_count;            // in this part, multiple heartbeat packets will be sent
	g_reception_task = kthread_run(reception_thread, thread_info, "reception_thread");
	if (NULL == g_reception_task)
	{
		HB_LOGE("failed to start reception task");
		kernel_sock_shutdown(sock, SHUT_RDWR);
		release_sock(sock->sk);
		kfree(thread_info);
		return INVALID_VALUE;
	}
	//continuously send multiple heartbeat packets, however, abort once a reply is received by the reception thread
	index = 0;
	while (index < retry_count)
	{
		// wait HEARTBEAT_DEFAULT_RETRY_INTERVAL second
		(void) down_timeout(&g_detection_sema, HEARTBEAT_DEFAULT_RETRY_INTERVAL*HZ);
		spin_lock_bh(&g_detection_lock);
		status = g_detection_result;        // use this variable instead of error code of  down_timeout
		spin_unlock_bh(&g_detection_lock);
		if (LINK_OK == status)
		{
			HB_LOGD("heartbeat triggered and LINK_OK detected, link type %d", type);
			kernel_sock_shutdown(sock, SHUT_RDWR);
			release_sock(sock->sk);
			return status;
		}
		// if no response, send one heartbeat packet
		sys_sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
	}
	// even if no reponse in the previous stage, we still wait for the response from reception thread
	// which will wait at most HEARTBEAT_DEFAULT_TIMEOUT seconds
	spin_lock_bh(&g_detection_lock);
	while(INVALID_VALUE == g_detection_result)
	{
		spin_unlock_bh(&g_detection_lock);
		down(&g_detection_sema);
		spin_lock_bh(&g_detection_lock);
	}
	status = g_detection_result;
	spin_unlock_bh(&g_detection_lock);

	HB_LOGD("the heartbeat detection result in final stage is %d", status);
	kernel_sock_shutdown(sock, SHUT_RDWR);
	release_sock(sock->sk);
	return status;
}

/*
 *	udp heartbeat socket preparation
 *    sock is the output
 */
static int prepare_socket(struct socket* sock, char* ifname)
{
	int err = INVALID_VALUE;
	struct net *net;

	if (NULL == sock || NULL == ifname)
	{
		HB_LOGE("null parameters in prepare(), critical!");
		return INVALID_VALUE;
	}

	/* NOTE THAT ANY ERROR FOR ANY LINK WILL CAUSE RETURN */
	/* create socket */
	net = sock_net(sock->sk);
	err = sock_create_kern(net, PF_INET, SOCK_DGRAM, IPPROTO_UDP, &sock);
	if (err < 0)
	{
		HB_LOGE("UDP create sock err, err=%d\n", err);
		goto create_error;
	}
	sock->sk->sk_reuse = 1;

	/* bind to device */
	err = bind_to_device(net, sock, ifname);
	if (err < 0)
	{
		HB_LOGE("Bind to %s err, err=%d\n", ifname, err);
		goto bind_error;
	}
	return 0;

bind_error:
	release_sock(sock->sk);
create_error:
	return INVALID_VALUE;
}

static int bind_to_device(struct net* net, struct socket* sock, char* ifname)
{
	struct net_device *dev;
	__be32 addr;
	struct sockaddr_in sin;
	int err;

	if (NULL == net || NULL == sock || NULL == ifname)
	{
		HB_LOGE("null pointer in bind_to_device");
		return;
	}

	dev = __dev_get_by_name(net, ifname);

	if (!dev)
	{
		HB_LOGE("No such device named %s\n", ifname);
		return -ENODEV;
	}
	addr = inet_select_addr(dev, 0, RT_SCOPE_UNIVERSE);
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = addr;
	sin.sin_port = 0;
	err = sock->ops->bind(sock, (struct sockaddr*)&sin, sizeof(sin));
	if (err < 0)
	{
		HB_LOGE("sock bind err, err=%d\n", err);
		return err;
	}
	return 0;
}

static int reception_thread(void* data)
{
	struct socket* sock = NULL;
	char* buffer = NULL;
	int timeout = 0;
	int retry_count = 0;
	int total_wait_time = 0;
	reception_thread_parameters* params = data;
	int recv_len = 0;
	struct sockaddr_in src_addr;
	int src_add_len = 0;
	if (NULL == params)
	{
		HB_LOGE("null para in reception_thread(), CRITICAL");
		return INVALID_VALUE;
	}
	sock = params->sock;
	buffer = params->buffer;
	timeout = params->interval;
	retry_count = params->retry_count;
	kfree(params);            // free this pointer

	if (NULL == sock || NULL == buffer || timeout <= 0 || retry_count < 0)
	{
		HB_LOGE("illegal parameters in reception_thread(), CRITICAL");
		return INVALID_VALUE;
	}
	// calculate total waiting time
	total_wait_time = (retry_count * HEARTBEAT_DEFAULT_RETRY_INTERVAL) + HEARTBEAT_DEFAULT_TIMEOUT;
	/* wait for reply */
	recv_len = sys_recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr*)&src_addr, &src_add_len);
	if (recv_len > 0)
	{
		HB_LOGD("received a reply in reception_thread()");
		// TODO: a validation should be applied
		spin_lock_bh(&g_detection_lock);
		g_detection_result = LINK_OK;
		spin_unlock_bh(&g_detection_lock);
	}
	else
	{
		HB_LOGE("reception_thread() timeout! no reply received");
		spin_lock_bh(&g_detection_lock);
		g_detection_result = LINK_BROKEN;
		spin_unlock_bh(&g_detection_lock);
	}
	// remember to trigger the sema
	up(&g_detection_sema);
	return 0;
}
