#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/mutex.h>
#include <linux/time.h>
#include <net/sock.h>
#include <net/tcp.h>
#include <net/ip.h>
#include <net/netlink.h>
#include <net/inet_connection_sock.h>
#include <net/tcp_states.h>
#include <linux/skbuff.h>
#include <linux/types.h>
#include <linux/netlink.h>
#include <uapi/linux/netlink.h>
#include <linux/kthread.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netdevice.h>
#include <linux/inetdevice.h>
#include <linux/spinlock.h>
#include <linux/string.h>
#include <linux/in.h>
#include <net/pkt_sched.h>
#include <net/sch_generic.h>
#include "../emcom_netlink.h"
#include "../emcom_utils.h"
#include <huawei_platform/emcom/network_evaluation.h>
#ifdef CONFIG_MPTCP
#include <net/mptcp.h>
#endif

#ifdef CONFIG_HUAWEI_BASTET
#include <huawei_platform/net/bastet/bastet_utils.h>
#endif
#include <huawei_platform/emcom/emcom_xengine.h>
#include <linux/version.h>
#include <asm/uaccess.h>

#ifndef CONFIG_MPTCP
/* These states need RST on ABORT according to RFC793 */
static inline bool tcp_need_reset(int state)
{
	return (1 << state) &
		(TCPF_ESTABLISHED | TCPF_CLOSE_WAIT | TCPF_FIN_WAIT1 |
		TCPF_FIN_WAIT2 | TCPF_SYN_RECV);
}
#endif

#undef HWLOG_TAG
#define HWLOG_TAG emcom_xengine
HWLOG_REGIST();
MODULE_LICENSE("GPL");


#define     EMCOM_MAX_ACC_APP  (5)
#define     EMCOM_UID_ACC_AGE_MAX  (1000)

#define     EMCOM_SPEED_CTRL_BASE_WIN_SIZE   (10000)


static spinlock_t g_mpflow_lock;
struct emcom_xengine_mpflow_info g_mpflow_uids[EMCOM_MPFLOW_MAX_APP];
static uint8_t g_mpflow_index;
static bool g_mpflow_tm_running = false;
static struct timer_list g_mpflow_tm;
static bool g_mpflow_nf_hook = false;

#ifdef CONFIG_HUAWEI_BASTET_COMM
	extern int bastet_comm_keypsInfo_write(uint32_t ulState);
#endif

struct Emcom_Xengine_acc_app_info     g_CurrentUids[EMCOM_MAX_ACC_APP];
struct Emcom_Xengine_speed_ctrl_info  g_SpeedCtrlInfo;

struct sk_buff_head g_UdpSkbList;
struct timer_list   g_UdpSkb_timer;
uid_t  g_UdpRetranUid;
bool   g_Emcom_udptimerOn = false;
uid_t g_FastSynUid;
#define FAST_SYN_COUNT (5)
#define EMCOM_UDPRETRAN_NODELAY
#define UDPTIMER_DELAY  (4)
#define EMCOM_MAX_UDP_SKB  (20)
#define MIN_JIFFIE         1
struct Emcom_Xengine_netem_skb_cb {
	psched_time_t    time_to_send;
	ktime_t          tstamp_save;
};

struct mutex g_Mpip_mutex;
struct  Emcom_Xengine_mpip_config g_MpipUids[EMCOM_MAX_MPIP_APP];/* The uid of bind to Mpip Application */
bool    g_MpipStart               = false;/* The uid of bind to Mpip Application */
char    g_Ifacename[IFNAMSIZ]     = {0};/* The uid of bind to Mpip Application */
static uint8_t g_SocketIndex      = 0;


LIST_HEAD(emcom_xengine_mpflow_list);

void Emcom_Xengine_Mpip_Init(void);
static void emcom_xengine_mpflow_fi_init(struct emcom_xengine_mpflow_info *mpflow_uid);
static void emcom_xengine_mpflow_register_nf_hook(void);
static void emcom_xengine_mpflow_unregister_nf_hook(void);
static void emcom_xengine_mpflow_download_flow_del(struct emcom_xengine_mpflow_iface *iface,
	struct emcom_xengine_mpflow_node *flow);
static bool emcom_xengine_mpflow_ptn_init(struct emcom_xengine_mpflow_ptn ptn[], uint8_t *num, const char *hex);
static void emcom_xengine_mpflow_ptn_deinit(struct emcom_xengine_mpflow_ptn ptn[], uint8_t num);
static bool emcom_xengine_mpflow_bm_build(const uint8_t *ptn, uint32_t ptnlen,
	uint8_t **skip, uint8_t **shift);
static void emcom_xengine_mpflow_apppriv_deinit(struct emcom_xengine_mpflow_info *uid);

/******************************************************************************
   6 º¯ÊýÊµÏÖ
******************************************************************************/
static inline bool invalid_uid(uid_t uid)
{
	/* if uid less than 10000, it is not an Android apk */
	return (uid < UID_APP);
}

static inline bool invalid_SpeedCtrlSize(uint32_t grade)
{
	/* the speed control grade bigger than 10000 */
	return (grade < EMCOM_SPEED_CTRL_BASE_WIN_SIZE);
}



static inline struct Emcom_Xengine_netem_skb_cb *Emcom_Xengine_netem_skb_cb(struct sk_buff *skb)
{
	/* we assume we can use skb next/prev/tstamp as storage for rb_node */
	qdisc_cb_private_validate(skb, sizeof(struct Emcom_Xengine_netem_skb_cb));
	return (struct Emcom_Xengine_netem_skb_cb *)qdisc_skb_cb(skb)->data;
}

#ifndef EMCOM_UDPRETRAN_NODELAY

static void Emcom_Xengine_setUdpTimerCb(struct sk_buff *skb)
{
	struct Emcom_Xengine_netem_skb_cb *cb;
	unsigned long now;
	now = jiffies;
	cb = Emcom_Xengine_netem_skb_cb(skb);
	/* translate to jiffies */
	cb->time_to_send = now + UDPTIMER_DELAY*HZ/MSEC_PER_SEC;
}
#endif


int Emcom_Xengine_udpretran_clear(void)
{
	g_UdpRetranUid = UID_INVALID_APP;
	skb_queue_purge(&g_UdpSkbList);
	if(g_Emcom_udptimerOn)
	{
		del_timer(&g_UdpSkb_timer);
		g_Emcom_udptimerOn = false;
	}
	return 0;
}


static void Emcom_Xengine_UdpTimer_handler(unsigned long pac)
{
	struct sk_buff *skb;
	unsigned long now;
	struct Emcom_Xengine_netem_skb_cb *cb;
	int jiffie_n;

	/* anyway, send out the first skb */
    if(!skb_queue_empty(&g_UdpSkbList))
	{
		skb = skb_dequeue(&g_UdpSkbList);
		if(skb)
		{
			dev_queue_xmit(skb);
			EMCOM_LOGD("Emcom_Xengine_UdpTimer_handler send skb\n");
		}
	}

	skb = skb_peek(&g_UdpSkbList);
	if(!skb)
	{
		goto timer_off;
		return;
	}
	cb = Emcom_Xengine_netem_skb_cb(skb);
	now = jiffies;
	/* if remaining time is little than 1 jiffie, send out */
	while(cb->time_to_send <= now + MIN_JIFFIE)
	{
		EMCOM_LOGD("Emcom_Xengine_UdpTimer_handler send another skb\n");
		skb = skb_dequeue(&g_UdpSkbList);
		if(skb)
		{
			dev_queue_xmit(skb);
		}
		skb = skb_peek(&g_UdpSkbList);
		if(!skb)
		{
			goto timer_off;
			return;
		}
		cb = Emcom_Xengine_netem_skb_cb(skb);
		now = jiffies;
	}
	/* set timer based on next skb cb */
	now = jiffies;
	jiffie_n = cb->time_to_send - now;

	if(jiffie_n < MIN_JIFFIE)
	{
		jiffie_n = MIN_JIFFIE;
	}
	EMCOM_LOGD("Emcom_Xengine_UdpTimer_handler modify timer hz %d\n", jiffie_n);
	mod_timer(&g_UdpSkb_timer, jiffies + jiffie_n);
	g_Emcom_udptimerOn = true;
	return;

timer_off:
	g_Emcom_udptimerOn = false;
}


void Emcom_Xengine_Init(void)
{
	uint8_t  index;
	for( index = 0; index < EMCOM_MAX_ACC_APP; index ++)
	{
		g_CurrentUids[index].lUid = UID_INVALID_APP;
		g_CurrentUids[index].ulAge = 0;
	}
	g_SpeedCtrlInfo.lUid = UID_INVALID_APP;
	g_SpeedCtrlInfo.ulSize = 0;
	spin_lock_init(&g_SpeedCtrlInfo.stLocker);
	g_UdpRetranUid = UID_INVALID_APP;
	g_Emcom_udptimerOn = false;
	skb_queue_head_init(&g_UdpSkbList);
	init_timer(&g_UdpSkb_timer);
	g_UdpSkb_timer.function = Emcom_Xengine_UdpTimer_handler;
	mutex_init(&g_Mpip_mutex);
	Emcom_Xengine_Mpip_Init();
	emcom_xengine_mpflow_init();
	g_FastSynUid = UID_INVALID_APP;
}


void Emcom_Xengine_Mpip_Init(void)
{
	uint8_t  uIndex;
	mutex_lock(&g_Mpip_mutex);
	for( uIndex = 0; uIndex < EMCOM_MAX_MPIP_APP; uIndex ++)
	{
		g_MpipUids[uIndex].lUid = UID_INVALID_APP;
		g_MpipUids[uIndex].ulType = EMCOM_XENGINE_MPIP_TYPE_BIND_NEW;
	}
	mutex_unlock(&g_Mpip_mutex);
}


bool Emcom_Xengine_IsAccUid(uid_t lUid)
{
	uint8_t  index;
	for( index = 0; index < EMCOM_MAX_ACC_APP; index ++)
	{
		if( lUid == g_CurrentUids[index].lUid )
		{
			return true;
		}
	}

	return false;
}



bool Emcom_Xengine_Hook_Ul_Stub(struct sock *pstSock)
{
	uid_t lSockUid = 0;
	bool  bFound   = false;

	if(( NULL == pstSock ) )
	{
		EMCOM_LOGD("Emcom_Xengine_Hook_Ul_Stub param invalid\n");
		return false;
	}

	/**
	 * if uid equals current acc uid, accelerate it,else stop it
	 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 10)
	lSockUid = sock_i_uid(pstSock).val;
#else
	lSockUid = sock_i_uid(pstSock);
#endif

	if( invalid_uid ( lSockUid ))
	{
		return false;
	}

	bFound = Emcom_Xengine_IsAccUid ( lSockUid );

	return bFound;
}


int Emcom_Xengine_clear(void)
{
	uint8_t  index;
	for( index = 0; index < EMCOM_MAX_ACC_APP; index ++)
	{
		g_CurrentUids[index].lUid = UID_INVALID_APP;
		g_CurrentUids[index].ulAge = 0;
	}
	mutex_lock(&g_Mpip_mutex);
	for( index = 0; index < EMCOM_MAX_MPIP_APP; index ++)
	{
		g_MpipUids[index].lUid = UID_INVALID_APP;
		g_MpipUids[index].ulType = EMCOM_XENGINE_MPIP_TYPE_BIND_NEW;
	}
	memset(g_Ifacename, 0, sizeof(char)*IFNAMSIZ);
	g_MpipStart = false;
	mutex_unlock(&g_Mpip_mutex);
	emcom_xengine_mpflow_clear();
	Emcom_Xengine_udpretran_clear();
	EMCOM_XENGINE_SetSpeedCtrl(g_SpeedCtrlInfo, UID_INVALID_APP, 0);
	g_FastSynUid = UID_INVALID_APP;
	return 0;
}



int Emcom_Xengine_StartAccUid(uint8_t *pdata, uint16_t len)
{
	uid_t              uid;
	uint8_t            index;
	uint8_t            ucIdleIndex;
	uint8_t            ucOldIndex;
	uint8_t            ucOldAge;
	bool               bFound;
	/*input param check*/
	if( NULL == pdata )
	{
		EMCOM_LOGE("Emcom_Xengine_StartAccUid:data is null");
		return -EINVAL;
	}

	/*check len is invalid*/
	if(len != sizeof(uid_t))
	{
		EMCOM_LOGI("Emcom_Xengine_StartAccUid: len:%d is illegal", len);
		return -EINVAL;
	}

	uid =*(uid_t *)pdata;

	/*check uid*/
	if (invalid_uid(uid))
		return -EINVAL;

	EMCOM_LOGD("Emcom_Xengine_StartAccUid: uid:%d ready to added", uid);
	ucIdleIndex = EMCOM_MAX_ACC_APP;
	ucOldIndex  = EMCOM_MAX_ACC_APP;
	ucOldAge    = 0;
	bFound  = false;

	/*check whether has the same uid, and  record the first idle position and the oldest position*/
	for( index = 0; index < EMCOM_MAX_ACC_APP; index ++)
	{
		if( UID_INVALID_APP == g_CurrentUids[index].lUid )
		{
			if( EMCOM_MAX_ACC_APP == ucIdleIndex )
			{
				ucIdleIndex  = index;
			}
		}
		else if( uid == g_CurrentUids[index].lUid )
		{
			g_CurrentUids[index].ulAge = 0;
			bFound = true;
		}
		else
		{
			g_CurrentUids[index].ulAge ++;
			if( g_CurrentUids[index].ulAge > ucOldAge )
			{
				ucOldAge    = g_CurrentUids[index].ulAge;
				ucOldIndex  = index ;
			}

		}
	}

	/*remove the too old acc uid*/
	if(ucOldAge  > EMCOM_UID_ACC_AGE_MAX )
	{
		EMCOM_LOGD("Emcom_Xengine_StartAccUid: uid:%d added too long, remove it", g_CurrentUids[ucOldIndex].lUid );
		g_CurrentUids[ucOldIndex].ulAge = 0;
		g_CurrentUids[ucOldIndex].lUid  = UID_INVALID_APP;
	}

	EMCOM_LOGD("Emcom_Xengine_StartAccUid: ucIdleIndex=%d,ucOldIndex=%d,ucOldAge=%d",ucIdleIndex, ucOldIndex,ucOldAge);

	/*if has already added, return*/
	if(bFound)
	{
		EMCOM_LOGD("Emcom_Xengine_StartAccUid: uid:%d already added", uid);
		return 0;
	}

	/*if it is new uid, and has idle position , add it*/
	if( ucIdleIndex < EMCOM_MAX_ACC_APP )
	{
		EMCOM_LOGD("Emcom_Xengine_StartAccUid: uid:%d added", uid);
		g_CurrentUids[ucIdleIndex].ulAge = 0;
		g_CurrentUids[ucIdleIndex].lUid = uid;
		return 0;
	}


	/*if it is new uid, and acc list if full , replace the oldest*/
	if( ucOldIndex < EMCOM_MAX_ACC_APP )
	{
		EMCOM_LOGD("Emcom_Xengine_StartAccUid: uid:%d replace the oldest uid:%d", uid,g_CurrentUids[ucOldIndex].lUid);
		g_CurrentUids[ucOldIndex].ulAge = 0;
		g_CurrentUids[ucOldIndex].lUid = uid;
		return 0;
	}

	return 0;
}




int Emcom_Xengine_StopAccUid(uint8_t *pdata, uint16_t len)
{
	uid_t              uid;
	uint8_t            index;

	/*input param check*/
	if( NULL == pdata )
	{
		EMCOM_LOGE("Emcom_Xengine_StopAccUid:data is null");
		return -EINVAL;
	}

	/*check len is invalid*/
	if(len != sizeof(uid_t))
	{
		EMCOM_LOGI("Emcom_Xengine_StopAccUid: len: %d is illegal", len);
		return -EINVAL;
	}

	uid =*(uid_t *)pdata;

	/*check uid*/
	if (invalid_uid(uid))
		return -EINVAL;

	/*remove specify uid*/
	for( index = 0; index < EMCOM_MAX_ACC_APP; index ++)
	{
		if( uid == g_CurrentUids[index].lUid )
		{
			g_CurrentUids[index].ulAge = 0;
			g_CurrentUids[index].lUid  = UID_INVALID_APP;
			EMCOM_LOGD("Emcom_Xengine_StopAccUid:lUid:%d",uid);
			break;
		}
	}

	return 0;
}


int Emcom_Xengine_SetSpeedCtrlInfo(uint8_t *pdata, uint16_t len)
{
	struct Emcom_Xengine_speed_ctrl_data* pSpeedCtrlInfo;
	uid_t              lUid;
	uint32_t           ulSize;

	/*input param check*/
	if( NULL == pdata )
	{
		EMCOM_LOGE("Emcom_Xengine_SetSpeedCtrlInfo:data is null");
		return -EINVAL;
	}

	/*check len is invalid*/
	if(len != sizeof(struct Emcom_Xengine_speed_ctrl_data))
	{
		EMCOM_LOGI("Emcom_Xengine_SetSpeedCtrlInfo: len:%d is illegal", len);
		return -EINVAL;
	}

	pSpeedCtrlInfo = (struct Emcom_Xengine_speed_ctrl_data *)pdata;
	lUid = pSpeedCtrlInfo->lUid;
	ulSize = pSpeedCtrlInfo->ulSize;

	/* if uid and size is zero, clear the speed control info */
	if(!lUid && !ulSize)
	{
		EMCOM_LOGD("Emcom_Xengine_SetSpeedCtrlInfo: clear speed ctrl state");
		EMCOM_XENGINE_SetSpeedCtrl(g_SpeedCtrlInfo, UID_INVALID_APP, 0);
		return 0;
	}

	/*check uid*/
	if (invalid_uid(lUid))
	{
		EMCOM_LOGI("Emcom_Xengine_SetSpeedCtrlInfo: uid:%d is illegal", lUid);
		return -EINVAL;
	}

	/*check size*/
	if (invalid_SpeedCtrlSize(ulSize))
	{
		EMCOM_LOGI("Emcom_Xengine_SetSpeedCtrlInfo: size:%d is illegal", ulSize);
		return -EINVAL;
	}

	EMCOM_LOGD("Emcom_Xengine_SetSpeedCtrlInfo: uid:%d size:%d", lUid, ulSize);
	EMCOM_XENGINE_SetSpeedCtrl(g_SpeedCtrlInfo, lUid, ulSize);
	return 0;
}



void Emcom_Xengine_SpeedCtrl_WinSize(struct sock *pstSock, uint32_t *pstSize)
{
	uid_t lSockUid = 0;
	uid_t lUid = 0;
	uint32_t ulSize = 0;

	if( NULL == pstSock )
	{
		EMCOM_LOGD("Emcom_Xengine_Hook_Ul_Stub param invalid\n");
		return;
	}

	if( NULL == pstSize )
	{
		EMCOM_LOGD(" Emcom_Xengine_SpeedCtrl_WinSize window size invalid\n");
		return;
	}

	EMCOM_XENGINE_GetSpeedCtrlUid(g_SpeedCtrlInfo, lUid);
	if( invalid_uid ( lUid ))
	{
		return;
	}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 10)
	lSockUid = sock_i_uid(pstSock).val;
#else
	lSockUid = sock_i_uid(pstSock);
#endif

	if( invalid_uid ( lSockUid ))
	{
		return;
	}

	EMCOM_XENGINE_GetSpeedCtrlInfo(g_SpeedCtrlInfo, lUid, ulSize);
	/* check uid */
	if( lSockUid == lUid)
	{
		return;
	}

	if (ulSize)
	{
		*pstSize = g_SpeedCtrlInfo.ulSize < *pstSize ? g_SpeedCtrlInfo.ulSize : *pstSize;
	}

}




int Emcom_Xengine_Config_MPIP(uint8_t *pdata, uint16_t len)
{
	uint8_t            uIndex;
	uint8_t            *ptemp;
	uint8_t            ulength;
	/*The empty updated list means clear the Mpip App Uid list*/
	EMCOM_LOGD("The Mpip list will be update to empty.");

	/*Clear the Mpip App Uid list*/
	mutex_lock(&g_Mpip_mutex);
	for( uIndex = 0; uIndex < EMCOM_MAX_MPIP_APP; uIndex ++)
	{
		g_MpipUids[uIndex].lUid = UID_INVALID_APP;
		g_MpipUids[uIndex].ulType = EMCOM_XENGINE_MPIP_TYPE_BIND_NEW;
	}
	mutex_unlock(&g_Mpip_mutex);
	if((NULL == pdata) || (0 == len))
	{
		/*pdata == NULL or len == 0 is ok, just return*/
		return 0;
	}
	ptemp = pdata;
	ulength = len/sizeof(struct Emcom_Xengine_mpip_config);
	if(EMCOM_MAX_MPIP_APP < ulength )
	{
		EMCOM_LOGE("The length of received MPIP APP uid list is error.");
		return -EINVAL;
	}
	mutex_lock(&g_Mpip_mutex);
	for(uIndex = 0; uIndex < ulength; uIndex++)
	{
		g_MpipUids[uIndex].lUid = *(uid_t *)ptemp;
		g_MpipUids[uIndex].ulType = *(uint32_t*)(ptemp + sizeof(uid_t));
		EMCOM_LOGD("The Mpip config [%d] is: lUid %d and type %d.",uIndex, g_MpipUids[uIndex].lUid, g_MpipUids[uIndex].ulType);
		ptemp += sizeof(struct Emcom_Xengine_mpip_config);
	}
	mutex_unlock(&g_Mpip_mutex);

	return 0;
}


int Emcom_Xengine_Clear_Mpip_Config(uint8_t *pdata, uint16_t len)
{
	uint8_t            uIndex;

	/*The empty updated list means clear the Mpip App Uid list*/
	EMCOM_LOGD("The Mpip list will be update to empty.");

	/*Clear the Mpip App Uid list*/
	mutex_lock(&g_Mpip_mutex);
	for( uIndex = 0; uIndex < EMCOM_MAX_MPIP_APP; uIndex ++)
	{
		g_MpipUids[uIndex].lUid = UID_INVALID_APP;
		g_MpipUids[uIndex].ulType = EMCOM_XENGINE_MPIP_TYPE_BIND_NEW;
	}
	mutex_unlock(&g_Mpip_mutex);

	return 0;
}






int Emcom_Xengine_StartMPIP(char *pdata, uint16_t len)
{
	/*input param check*/
	if( (NULL == pdata) || (0 == len) || (IFNAMSIZ < len) )
	{
	    EMCOM_LOGE("MPIP interface name or length %d is error", len);
		return -EINVAL;
	}
	mutex_lock(&g_Mpip_mutex);
	memcpy (g_Ifacename, pdata, len);
	g_MpipStart = true;
	mutex_unlock(&g_Mpip_mutex);
	EMCOM_LOGD("Mpip is :%d to start.", g_MpipStart);
	return 0;
}




int Emcom_Xengine_StopMPIP(uint8_t *pdata, uint16_t len)
{
	mutex_lock(&g_Mpip_mutex);
	g_MpipStart = false;
	mutex_unlock(&g_Mpip_mutex);
	EMCOM_LOGD("MPIP function is :%d, ready to stop", g_MpipStart);

	return 0;
}


int Emcom_Xengine_IsMpipBindUid(uid_t lUid)
{
	int ret = -1;
	uint8_t  uIndex;
	mutex_lock(&g_Mpip_mutex);
	for( uIndex = 0; uIndex < EMCOM_MAX_MPIP_APP; uIndex ++)
	{
		if( lUid == g_MpipUids[uIndex].lUid )
		{
			mutex_unlock(&g_Mpip_mutex);
			ret = uIndex;
			return ret;
		}
	}
	mutex_unlock(&g_Mpip_mutex);

	return ret;
}


void Emcom_Xengine_Mpip_Bind2Device(struct sock *pstSock)
{
	int iFound             = -1;
	uint8_t  uIndex        = 0;
	uid_t lSockUid         = 0;
	struct net *net        = NULL;
	struct net_device *dev = NULL;

	if(NULL == pstSock)
	{
		EMCOM_LOGE(" param invalid.\n");
		return;
	}

	if(!g_MpipStart)
	{
		return;
	}
	/**
	 * if uid equals current bind uid, bind 2 device
	 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 10)
	lSockUid = sock_i_uid(pstSock).val;
#else
	lSockUid = sock_i_uid(pstSock);
#endif

	if( invalid_uid ( lSockUid ))
	{
		return;
	}

	net = sock_net(pstSock);
	iFound = Emcom_Xengine_IsMpipBindUid( lSockUid );
	if(iFound != -1)
	{
		rcu_read_lock();
		dev = dev_get_by_name_rcu(net, g_Ifacename);
		if(dev)
		{
            uIndex = dev->ifindex;
		}
		rcu_read_unlock();
		if ((!dev) || (!test_bit(__LINK_STATE_START, &dev->state)))
		{
			mutex_lock(&g_Mpip_mutex);
			g_MpipStart = false;
			mutex_unlock(&g_Mpip_mutex);
			emcom_send_msg2daemon(NETLINK_EMCOM_KD_XENIGE_DEV_FAIL, NULL, 0);
			EMCOM_LOGE(" get dev fail or dev is not up.\n");
			return;
		}

		if(g_MpipUids[iFound].ulType == EMCOM_XENGINE_MPIP_TYPE_BIND_RANDOM)
		{
			if(g_SocketIndex % 2 == 0)
			{
				lock_sock(pstSock);
				pstSock->sk_bound_dev_if = uIndex;
				sk_dst_reset(pstSock);
				release_sock(pstSock);
			}
			g_SocketIndex++;
			g_SocketIndex = g_SocketIndex % 2;
		}
		else
		{
			lock_sock(pstSock);
			pstSock->sk_bound_dev_if = uIndex;
			sk_dst_reset(pstSock);
			release_sock(pstSock);
		}
	}
}



int Emcom_Xengine_RrcKeep( void )
{
#ifdef CONFIG_HUAWEI_BASTET
	post_indicate_packet(BST_IND_RRC_KEEP,NULL,0);
#endif
	return 0;
}




int Emcom_Send_KeyPsInfo(uint8_t *pdata, uint16_t len)
{
	uint32_t            ulState;

	/*input param check*/
	if( NULL == pdata )
	{
		EMCOM_LOGE("Emcom_Send_KeyPsInfo:data is null");
		return -EINVAL;
	}

	/*check len is invalid*/
	if( len < sizeof( uint32_t ) )
	{
		EMCOM_LOGE("Emcom_Send_KeyPsInfo: len: %d is illegal", len);
		return -EINVAL;
	}

	ulState =*(uint32_t *)pdata;

	if( true != Emcom_Is_Modem_Support() )
	{
		EMCOM_LOGI( "Emcom_Send_KeyPsInfo: modem not support" );
		return -EINVAL;
	}

#ifdef CONFIG_HUAWEI_BASTET_COMM
	bastet_comm_keypsInfo_write( ulState );
#endif
	return 0;
}



static inline bool Emcom_Xengine_isWlan(struct sk_buff *skb)
{
	const char *delim = "wlan0";
	int len = strlen(delim);
	if(!skb->dev)
	{
		return false;
	}

	if (strncmp(skb->dev->name, delim, len))
	{
		return false;
	}

	return true;
}



void Emcom_Xengine_UdpEnqueue(struct sk_buff *skb)
{
	struct sock *sk;
	struct sk_buff *skb2;
	uid_t lSockUid = UID_INVALID_APP;
	/* invalid g_UdpRetranUid means UDP retran is closed */

	if(invalid_uid(g_UdpRetranUid))
	{
		return;
	}

	if((!skb))
	{
		EMCOM_LOGE("Emcom_Xengine_UdpEnqueue skb null");
		return;
	}
	if(g_UdpSkbList.qlen >= EMCOM_MAX_UDP_SKB)
	{
		EMCOM_LOGE("Emcom_Xengine_UdpEnqueue max skb");
		return;
	}

	sk = skb_to_full_sk(skb);
	if (unlikely(!sk))
	{
		EMCOM_LOGE("Emcom_Xengine_UdpEnqueue sk null");
		return;
	}

	if (unlikely(!sk->sk_socket))
	{
		EMCOM_LOGE("Emcom_Xengine_UdpEnqueue sk_socket null");
		return;
	}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 10)
	lSockUid = sock_i_uid(sk).val;
#else
	lSockUid = sock_i_uid(sk);
#endif
	if(lSockUid == g_UdpRetranUid)
	{
		if(!Emcom_Xengine_isWlan(skb))
		{
			EMCOM_LOGD("Emcom_Xengine_UdpEnqueue not wlan");
			Emcom_Xengine_udpretran_clear();
			return;
		}
		if(sk->sk_socket->type == SOCK_DGRAM)
		{
			skb2 = skb_copy(skb, GFP_ATOMIC);
			if(unlikely(!skb2))
			{
				EMCOM_LOGE("Emcom_Xengine_UdpEnqueue skb2 null");
				return;
			}
#ifdef EMCOM_UDPRETRAN_NODELAY
			dev_queue_xmit(skb2);
			return;
#else
			skb_queue_tail(&g_UdpSkbList,skb2);
			Emcom_Xengine_setUdpTimerCb(skb2);
			if(!g_Emcom_udptimerOn)
			{
				skb2 = skb_peek(&g_UdpSkbList);
				if(!skb2)
				{
					EMCOM_LOGE("Emcom_Xengine_UdpEnqueue peek skb2 null");
					return;
				}
				g_Emcom_udptimerOn = true;
				g_UdpSkb_timer.expires = jiffies + UDPTIMER_DELAY*HZ/MSEC_PER_SEC;
				EMCOM_LOGD("Emcom_Xengine_UdpEnqueue: jiffie %d",UDPTIMER_DELAY*HZ/MSEC_PER_SEC);
				add_timer(&g_UdpSkb_timer);
			}
#endif
		}
	}
}



int Emcom_Xengine_StartUdpReTran(uint8_t *pdata, uint16_t len)
{
	uid_t              uid;

	/*input param check*/
	if( NULL == pdata )
	{
		EMCOM_LOGE("Emcom_Xengine_StartUdpReTran:data is null");
		return -EINVAL;
	}

	/*check len is invalid*/
	if(len != sizeof(uid_t))
	{
		EMCOM_LOGI("Emcom_Xengine_StartUdpReTran: len: %d is illegal", len);
		return -EINVAL;
	}

	uid =*(uid_t *)pdata;
	/*check uid*/
	if (invalid_uid(uid))
	{
		EMCOM_LOGE("Emcom_Xengine_StartUdpReTran: uid is invalid %d", uid);
		return -EINVAL;
	}
	EMCOM_LOGI("Emcom_Xengine_StartUdpReTran: uid: %d ", uid);
	g_UdpRetranUid = uid;
	return 0;
}


int Emcom_Xengine_StopUdpReTran(uint8_t *pdata, uint16_t len)
{
	Emcom_Xengine_udpretran_clear();
	return 0;
}



void Emcom_Xengine_FastSyn(struct sock *pstSock)
{
	uid_t lSockUid = 0;
	struct inet_connection_sock *icsk;

	if( NULL == pstSock )
	{
		EMCOM_LOGD(" Emcom_Xengine_FastSyn param invalid\n");
		return;
	}
	if( pstSock->sk_state != TCP_SYN_SENT )
	{
		return;
	}

	if( invalid_uid ( g_FastSynUid ))
	{
		return;
	}


#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 10)
	lSockUid = sock_i_uid(pstSock).val;
#else
	lSockUid = sock_i_uid(pstSock);
#endif

	if( lSockUid != g_FastSynUid )
	{
		return;
	}

	icsk = inet_csk(pstSock);
	if( icsk->icsk_retransmits <= FAST_SYN_COUNT )
	{
		icsk->icsk_rto = TCP_TIMEOUT_INIT;
	}
}

int Emcom_Xengine_StartFastSyn(uint8_t *pdata, uint16_t len)
{
	uid_t              uid;

	/*input param check*/
	if( NULL == pdata )
	{
		EMCOM_LOGE(" Emcom_Xengine_StartFastSyn:data is null");
		return -EINVAL;
	}

	/*check len is invalid*/
	if(len != sizeof(uid_t))
	{
		EMCOM_LOGI(" Emcom_Xengine_StartFastSyn: len: %d is illegal", len);
		return -EINVAL;
	}

	uid =*(uid_t *)pdata;
	/*check uid*/
	if (invalid_uid(uid))
	{
		EMCOM_LOGE(" Emcom_Xengine_StartFastSyn: uid is invalid %d", uid);
		return -EINVAL;
	}
	EMCOM_LOGI(" Emcom_Xengine_StartFastSyn: uid: %d ", uid);
	g_FastSynUid = uid;
	return 0;
}

int Emcom_Xengine_StopFastSyn(uint8_t *pdata, uint16_t len)
{
	g_FastSynUid = UID_INVALID_APP;
	return 0;
}

bool emcom_xengine_check_ip_addrss(struct sockaddr *addr)
{
	struct sockaddr_in *usin = (struct sockaddr_in *)addr;

	if (usin->sin_family == AF_INET) {
		return !ipv4_is_loopback(usin->sin_addr.s_addr) && !ipv4_is_multicast(usin->sin_addr.s_addr) &&
				!ipv4_is_zeronet(usin->sin_addr.s_addr) && !ipv4_is_lbcast(usin->sin_addr.s_addr);
	} else if (usin->sin_family == AF_INET6) {
		struct sockaddr_in6 *usin6 = (struct sockaddr_in6 *)addr;
		return !ipv6_addr_loopback(&usin6->sin6_addr) && !ipv6_addr_is_multicast(&usin6->sin6_addr);
	}

	return true;
}

bool emcom_xengine_check_ip_is_private(struct sockaddr *addr)
{
	struct sockaddr_in *usin = (struct sockaddr_in *)addr;
	if (usin->sin_family == AF_INET) {
		return (ipv4_is_linklocal_169(usin->sin_addr.s_addr) ||
			ipv4_is_private_10(usin->sin_addr.s_addr) ||
			ipv4_is_private_172(usin->sin_addr.s_addr) ||
			ipv4_is_private_192(usin->sin_addr.s_addr));
	} else if (usin->sin_family == AF_INET6) {
		struct sockaddr_in6 *usin6 = (struct sockaddr_in6 *)addr;
		int addr_type = ipv6_addr_type(&usin6->sin6_addr);
		if (addr_type & IPV6_ADDR_MAPPED) {
			__be32 s_addr = usin6->sin6_addr.s6_addr32[3];
			return (ipv4_is_linklocal_169(s_addr) ||
				ipv4_is_private_10(s_addr) ||
				ipv4_is_private_172(s_addr) ||
				ipv4_is_private_192(s_addr));
		}
	}

	return false;
}

void emcom_xengine_mpflow_init(void)
{
	uint8_t uindex;

	EMCOM_LOGD("mpflow init.");
	spin_lock_init(&g_mpflow_lock);
	spin_lock_bh(&g_mpflow_lock);
	for (uindex = 0; uindex < EMCOM_MPFLOW_MAX_APP; uindex++) {
		g_mpflow_uids[uindex].uid = UID_INVALID_APP;
		g_mpflow_uids[uindex].bindmode = EMCOM_XENGINE_MPFLOW_BINDMODE_NONE;
		g_mpflow_uids[uindex].enableflag = 0;
		g_mpflow_uids[uindex].protocol = 0;
		memset(&g_mpflow_uids[uindex].dport_range,
		       0,
		       sizeof(g_mpflow_uids[uindex].dport_range));
	}
	spin_unlock_bh(&g_mpflow_lock);
	g_mpflow_index = 0;
}

void emcom_xengine_mpflow_clear(void)
{
	uint8_t  index;
	struct emcom_xengine_mpflow_node *node = NULL;
	struct emcom_xengine_mpflow_node *tmp = NULL;

	spin_lock_bh(&g_mpflow_lock);
	for (index = 0; index < EMCOM_MPFLOW_MAX_APP; index++) {
		if (g_mpflow_uids[index].uid != UID_INVALID_APP &&
		    (g_mpflow_uids[index].algorithm_type == EMCOM_MPFLOW_ENABLETYPE_NET_DISK ||
		     g_mpflow_uids[index].algorithm_type == EMCOM_MPFLOW_ENABLETYPE_MARKET)) {
			list_for_each_entry_safe(node, tmp, &g_mpflow_uids[index].wifi.flows, list)
				emcom_xengine_mpflow_download_flow_del(&g_mpflow_uids[index].wifi, node);

			list_for_each_entry_safe(node, tmp, &g_mpflow_uids[index].lte.flows, list)
				emcom_xengine_mpflow_download_flow_del(&g_mpflow_uids[index].lte, node);

			emcom_xengine_mpflow_ptn_deinit(g_mpflow_uids[index].ptn_80, g_mpflow_uids[index].ptn_80_num);
			emcom_xengine_mpflow_ptn_deinit(g_mpflow_uids[index].ptn_443, g_mpflow_uids[index].ptn_443_num);
			emcom_xengine_mpflow_apppriv_deinit(&g_mpflow_uids[index]);
		}

		g_mpflow_uids[index].uid = UID_INVALID_APP;
		g_mpflow_uids[index].bindmode = EMCOM_XENGINE_MPFLOW_BINDMODE_NONE;
		g_mpflow_uids[index].enableflag = 0;
		g_mpflow_uids[index].protocol = 0;
		memset(&g_mpflow_uids[index].dport_range,
		       0,
		       sizeof(g_mpflow_uids[index].dport_range));
	}
	spin_unlock_bh(&g_mpflow_lock);

	emcom_xengine_mpflow_unregister_nf_hook();
	g_mpflow_index = 0;
}

int8_t emcom_xengine_mpflow_finduid(uid_t uid)
{
	int8_t index;
	for (index = 0; index < EMCOM_MPFLOW_MAX_APP; index++) {
		if (g_mpflow_uids[index].uid == uid)
			return index;
	}

	return INDEX_INVALID;
}

static bool emcom_xengine_mpflow_uid_empty(void)
{
	int8_t index;
	for (index = 0; index < EMCOM_MPFLOW_MAX_APP; index++) {
		if (g_mpflow_uids[index].uid != UID_INVALID_APP)
			return false;
	}

	return true;
}

int8_t emcom_xengine_mpflow_getfreeindex(void)
{
	int8_t index;

	for (index = 0; index < EMCOM_MPFLOW_MAX_APP; index++) {
		if (g_mpflow_uids[index].uid == UID_INVALID_APP)
			return index;
	}
	return INDEX_INVALID;
}

static bool emcom_xengine_mpflow_fi_start(bool is_new_uid_enable, uint8_t index, bool *ret,
						struct emcom_xengine_mpflow_parse_start_info *mpflowstartinfo)
{
	int i;
	struct emcom_xengine_mpflow_app_priv *app_priv = NULL;
	if (g_mpflow_uids[index].algorithm_type == EMCOM_MPFLOW_ENABLETYPE_WEIBO) {
		if (is_new_uid_enable)
			emcom_xengine_mpflow_fi_init(&g_mpflow_uids[index]);
		*ret = true;
		return true;
	}
	if (is_new_uid_enable) {
		emcom_xengine_mpflow_fi_init(&g_mpflow_uids[index]);
		app_priv = kzalloc(sizeof(struct emcom_xengine_mpflow_app_priv), GFP_ATOMIC);
		if (!app_priv)
			return false;

		if (g_mpflow_uids[index].algorithm_type == EMCOM_MPFLOW_ENABLETYPE_NET_DISK) {
			if (g_mpflow_uids[index].reserve_field & EMCOM_MPFLOW_ENABLEFLAG_LTE_FIRST) {
				app_priv->lte_thresh = EMCOM_MPFLOW_LTE_FIRST_LTE_THREH;
				app_priv->lte_thresh_max = EMCOM_MPFLOW_LTE_FIRST_LTE_THREH_MAX;
				app_priv->lte_thresh_min = EMCOM_MPFLOW_LTE_FIRST_LTE_THREH_MIN;
				app_priv->lte_first = 1;
			} else {
				app_priv->lte_thresh = EMCOM_MPFLOW_WIFI_FIRST_LTE_THREH;
				app_priv->lte_thresh_max = EMCOM_MPFLOW_WIFI_FIRST_LTE_THREH_MAX;
				app_priv->lte_thresh_min = EMCOM_MPFLOW_WIFI_FIRST_LTE_THREH_MIN;
				app_priv->lte_first = 0;
			}
		}

		for (i = 0; i < EMCOM_MPFLOW_HASH_SIZE; i++)
			INIT_HLIST_HEAD(&app_priv->hashtable[i]);

		g_mpflow_uids[index].app_priv = app_priv;
		if (emcom_xengine_mpflow_ptn_init(g_mpflow_uids[index].ptn_80, &(g_mpflow_uids[index].ptn_80_num),
			mpflowstartinfo->ptn_80))
			*ret = true;
		if (emcom_xengine_mpflow_ptn_init(g_mpflow_uids[index].ptn_443, &(g_mpflow_uids[index].ptn_443_num),
			mpflowstartinfo->ptn_443))
			*ret = true;
	} else {
		emcom_xengine_mpflow_ptn_deinit(g_mpflow_uids[index].ptn_80, g_mpflow_uids[index].ptn_80_num);
		emcom_xengine_mpflow_ptn_deinit(g_mpflow_uids[index].ptn_443, g_mpflow_uids[index].ptn_443_num);
		if (emcom_xengine_mpflow_ptn_init(g_mpflow_uids[index].ptn_80, &(g_mpflow_uids[index].ptn_80_num),
			mpflowstartinfo->ptn_80))
			*ret = true;
		if (emcom_xengine_mpflow_ptn_init(g_mpflow_uids[index].ptn_443, &(g_mpflow_uids[index].ptn_443_num),
			mpflowstartinfo->ptn_443))
			*ret = true;
	}
	return true;
}
void emcom_xengine_mpflow_start(const char *pdata, uint16_t len)
{
	struct emcom_xengine_mpflow_parse_start_info *mpflowstartinfo = NULL;
	int8_t index;
	int8_t newindex = INDEX_INVALID;
	bool ret = false;
	bool fi_start_ret = false;
	bool is_new_uid_enable = false;

	/* input param check */
	if ((!pdata) || (len != sizeof(struct emcom_xengine_mpflow_parse_start_info))) {
		EMCOM_LOGE("mpflow start data or length %d is error", len);
		return;
	}

	mpflowstartinfo = (struct emcom_xengine_mpflow_parse_start_info *)pdata;

	EMCOM_LOGD("mpflow start uid: %u, enableflag: %d, "
		   "protocol: %d, bindmode: %d",
		   mpflowstartinfo->uid, mpflowstartinfo->enableflag,
		   mpflowstartinfo->protocol, mpflowstartinfo->bindmode);

	spin_lock_bh(&g_mpflow_lock);
	index = emcom_xengine_mpflow_finduid(mpflowstartinfo->uid);
	if (index == INDEX_INVALID) {
		EMCOM_LOGD("mpflow add new mpinfo uid: %d.", mpflowstartinfo->uid);
		newindex = emcom_xengine_mpflow_getfreeindex();
		if (newindex == INDEX_INVALID) {
			EMCOM_LOGE("mpflow start get free index exceed. uid: %d",
				   mpflowstartinfo->uid);
			spin_unlock_bh(&g_mpflow_lock);
			return;
		}
		index = newindex;
		is_new_uid_enable = true;
	}

	if (is_new_uid_enable)
		emcom_xengine_mpflow_clear_blocked(mpflowstartinfo->uid);

	/* Fill mpflow info.*/
	g_mpflow_uids[index].uid = mpflowstartinfo->uid;
	g_mpflow_uids[index].enableflag = mpflowstartinfo->enableflag;
	g_mpflow_uids[index].protocol = mpflowstartinfo->protocol;
	g_mpflow_uids[index].bindmode = mpflowstartinfo->bindmode;
	g_mpflow_uids[index].algorithm_type = mpflowstartinfo->algorithm_type;
	g_mpflow_uids[index].reserve_field = mpflowstartinfo->reserve_field;

	if (mpflowstartinfo->enableflag & EMCOM_MPFLOW_ENABLEFLAG_DPORT) {
		memcpy(g_mpflow_uids[index].dport_range,
		       mpflowstartinfo->dport_range,
		       sizeof(mpflowstartinfo->dport_range));
	}
	if (g_mpflow_uids[index].algorithm_type != EMCOM_MPFLOW_ENABLETYPE_NET_DISK &&
		g_mpflow_uids[index].algorithm_type != EMCOM_MPFLOW_ENABLETYPE_MARKET &&
		g_mpflow_uids[index].algorithm_type != EMCOM_MPFLOW_ENABLETYPE_WEIBO)
		goto RETURN;
	fi_start_ret = emcom_xengine_mpflow_fi_start(is_new_uid_enable, index, &ret, mpflowstartinfo);
	if (!fi_start_ret) {
		spin_unlock_bh(&g_mpflow_lock);
		return;
	}

RETURN:
	spin_unlock_bh(&g_mpflow_lock);

	if (ret)
		emcom_xengine_mpflow_register_nf_hook();
	else
		emcom_xengine_mpflow_unregister_nf_hook();
	emcom_xengine_mpflow_show();
}

void emcom_xengine_mpflow_stop(const char *pdata, uint16_t len)
{
	struct emcom_xengine_mpflow_parse_stop_info *mpflowstopinfo = NULL;
	int8_t index;
	int32_t stop_reason;
	struct emcom_xengine_mpflow_node *node = NULL;
	struct emcom_xengine_mpflow_node *tmp = NULL;
	bool mpflow_uid_empty = false;

	/* input param check */
	if ((!pdata) || (len != sizeof(struct emcom_xengine_mpflow_parse_stop_info))) {
		EMCOM_LOGE("mpflow stop data or length %d is error", len);
		return;
	}

	mpflowstopinfo = (struct emcom_xengine_mpflow_parse_stop_info*)pdata;
	stop_reason = mpflowstopinfo->stop_reason;
	EMCOM_LOGD("mpflow stop uid: %u, stop reason: %u", mpflowstopinfo->uid,stop_reason);
	spin_lock_bh(&g_mpflow_lock);
	index = emcom_xengine_mpflow_finduid(mpflowstopinfo->uid);
	if (index != INDEX_INVALID) {
		if (stop_reason == EMCOM_MPFLOW_STOP_REASON_NETWORK_ROAMING ||
		    stop_reason == EMCOM_MPFLOW_STOP_REASON_APPDIED) {
			EMCOM_LOGD("mpflow stop clear info uid: %u, index: %d ", mpflowstopinfo->uid, index);

			if (g_mpflow_uids[index].algorithm_type == EMCOM_MPFLOW_ENABLETYPE_NET_DISK ||
			    g_mpflow_uids[index].algorithm_type == EMCOM_MPFLOW_ENABLETYPE_MARKET) {
				list_for_each_entry_safe(node, tmp, &g_mpflow_uids[index].wifi.flows, list)
					emcom_xengine_mpflow_download_flow_del(&g_mpflow_uids[index].wifi, node);

				list_for_each_entry_safe(node, tmp, &g_mpflow_uids[index].lte.flows, list)
					emcom_xengine_mpflow_download_flow_del(&g_mpflow_uids[index].lte, node);

				emcom_xengine_mpflow_ptn_deinit(g_mpflow_uids[index].ptn_80, g_mpflow_uids[index].ptn_80_num);
				emcom_xengine_mpflow_ptn_deinit(g_mpflow_uids[index].ptn_443, g_mpflow_uids[index].ptn_443_num);
				emcom_xengine_mpflow_apppriv_deinit(&g_mpflow_uids[index]);
			}

			g_mpflow_uids[index].uid = UID_INVALID_APP;
			g_mpflow_uids[index].enableflag = 0;
			g_mpflow_uids[index].protocol = 0;
			g_mpflow_uids[index].bindmode = EMCOM_XENGINE_MPFLOW_BINDMODE_NONE;
			memset(&g_mpflow_uids[index].dport_range,
			       0,
			       sizeof(g_mpflow_uids[index].dport_range));
		}
		else
			g_mpflow_uids[index].bindmode = EMCOM_XENGINE_MPFLOW_BINDMODE_NONE;
	}

	mpflow_uid_empty = emcom_xengine_mpflow_uid_empty();
	spin_unlock_bh(&g_mpflow_lock);

	if (mpflow_uid_empty)
		emcom_xengine_mpflow_unregister_nf_hook();
	emcom_xengine_mpflow_show();
	emcom_xengine_mpflow_delete(mpflowstopinfo->uid);
}

bool emcom_xengine_mpflow_checkvalid(struct sock *sk, struct sockaddr *uaddr, int8_t index, uint16_t *dport)
{
	struct sockaddr_in *usin = (struct sockaddr_in *)uaddr;
	struct sockaddr_in6 *usin6 = NULL;
	bool isvalidaddr = false;
	bool bfinddport = false;

	if ((!sk) || (!uaddr))
		return false;

	isvalidaddr = emcom_xengine_check_ip_addrss(uaddr) && (!emcom_xengine_check_ip_is_private(uaddr));
	if (isvalidaddr == false) {
		EMCOM_LOGD("mpflow check valid addr is not valid. uid: %u",
			   g_mpflow_uids[index].uid);
		return false;
	}

	EMCOM_LOGD("mpflow check valid uid: %u link famliy: %d, link proto: %d,"
		   "mpflow protocol: %d, bindmode: %u, ",
		   g_mpflow_uids[index].uid, sk->sk_family, sk->sk_protocol,
		   g_mpflow_uids[index].protocol,
		   g_mpflow_uids[index].bindmode);

	if (!(((sk->sk_protocol == IPPROTO_TCP) &&
	    (EMCOM_MPFLOW_PROTOCOL_TCP & g_mpflow_uids[index].protocol))
	    || ((sk->sk_protocol == IPPROTO_UDP) &&
		(EMCOM_MPFLOW_PROTOCOL_UDP & g_mpflow_uids[index].protocol)))) {
		EMCOM_LOGD("mpflow check valid protocol not correct uid: %u, sk: %pK",
			   g_mpflow_uids[index].uid, sk);
		return false;
	}

	if (g_mpflow_uids[index].enableflag & EMCOM_MPFLOW_ENABLEFLAG_DPORT) {
		if (usin->sin_family == AF_INET) {
			*dport = ntohs(usin->sin_port);
		} else if (usin->sin_family == AF_INET6) {
			usin6 = (struct sockaddr_in6 *)uaddr;
			*dport = (uint16_t)ntohs(usin6->sin6_port);
		} else {
			EMCOM_LOGD("mpflow check valid not support family uid: %u,"
				   " sin_family: %d",
				   g_mpflow_uids[index].uid, usin->sin_family);
			return false;
		}
		bfinddport = emcom_xengine_mpflow_finddport(&g_mpflow_uids[index], *dport);
		if (bfinddport == false) {
			EMCOM_LOGD("mpflow check valid can not find uid: %u, dport: %d",
				   g_mpflow_uids[index].uid, *dport);
			return false;
		}
	}

	return true;
}

bool emcom_xengine_mpflow_getinetaddr(struct net_device *dev)
{
	struct in_device *in_dev = NULL;
	struct in_ifaddr *ifa = NULL;

	if (!dev) {
		EMCOM_LOGD("mpflow get inet addr dev is null");
		return false;
	}

	if (!(in_dev = __in_dev_get_rcu(dev))) {
		EMCOM_LOGD("mpflow get inet addr in_dev is null dev: %s", dev->name);
		return false;
	}

	ifa = in_dev->ifa_list;
	if (ifa != NULL)
		return true;

	return false;
}

static inline uint8_t emcom_xengine_mpflow_ip_hash(__be32 addr)
{
	uint32_t h;

	h = addr << 8;
	h ^= addr >> 4;
	h ^= addr >> 12;
	h ^= addr >> 16;

	return (uint8_t)(h & (EMCOM_MPFLOW_HASH_SIZE - 1));
}

static struct emcom_xengine_mpflow_ip *emcom_xengine_mpflow_hash(__be32 addr,
	struct hlist_head *hashtable, uint32_t algorithm_type)
{
	struct emcom_xengine_mpflow_ip *ip = NULL;
	struct hlist_node *tmp = NULL;
	uint8_t hash = emcom_xengine_mpflow_ip_hash(addr);
	unsigned long aging;

	if (algorithm_type == EMCOM_MPFLOW_ENABLETYPE_NET_DISK)
		aging = EMCOM_MPFLOW_NETDISK_DOWNLOAD_THREH;
	else
		aging = EMCOM_MPFLOW_IP_AGING_THREH;

	hlist_for_each_entry_safe(ip, tmp, &hashtable[hash], node) {
		if (ip->addr == addr)
			return ip;

		/* free too old entrys */
		if ((jiffies - ip->jiffies[(ip->tot_cnt - 1) % EMCOM_MPFLOW_FLOW_JIFFIES_REC]) >
			aging) {
			hlist_del(&ip->node);
			kfree(ip);
		}
	}

	ip = kzalloc(sizeof(struct emcom_xengine_mpflow_ip), GFP_ATOMIC);
	if (!ip)
		return NULL;

	ip->addr = addr;
	hlist_add_head(&ip->node, &hashtable[hash]);
	return ip;
}

static void emcom_xengine_mpflow_hash_clear(struct emcom_xengine_mpflow_app_priv *priv)
{
	int i;
	struct emcom_xengine_mpflow_ip *ip = NULL;
	struct hlist_node *tmp = NULL;

	for (i = 0; i < EMCOM_MPFLOW_HASH_SIZE; i++) {
		hlist_for_each_entry_safe(ip, tmp, &priv->hashtable[i], node) {
			hlist_del(&ip->node);
			kfree(ip);
		}
	}
}

static bool emcom_xengine_mpflow_ip_chk_bind_lte(
	struct emcom_xengine_mpflow_app_priv *priv,
	struct emcom_xengine_mpflow_ip *ip,
	uint32_t type)
{
	if (type == EMCOM_MPFLOW_ENABLETYPE_NET_DISK) {
		bool lte_first = priv->lte_first ? true : false;
		uint8_t index = ip->tot_cnt % EMCOM_MPFLOW_FLOW_JIFFIES_REC;
		unsigned long now = jiffies;

		if (ip->tot_cnt < EMCOM_MPFLOW_FLOW_JIFFIES_REC)
			return lte_first;

		if (time_before(now, ip->jiffies[index] + EMCOM_MPFLOW_NETDISK_DOWNLOAD_THREH)) {
			/* if (lte_cnt/tot_cnt < lte_thresh/10), then we need bind on lte */
			if (ip->lte_cnt * EMCOM_MPFLOW_FI_NETDISK_FLOW_NUM < priv->lte_thresh * ip->tot_cnt)
				return true;
			else
				return false;
		}

		return lte_first;
	} else {
		if (ip->tot_cnt % EMCOM_MPFLOW_DEV_NUM)
			return true;
		else
			return false;
	}
}

static bool emcom_xengine_mpflow_get_addr_port(struct sockaddr *addr, __be32 *s_addr, uint16_t *port)
{
	if (addr->sa_family == AF_INET) {
		struct sockaddr_in *usin = (struct sockaddr_in *)addr;
		*s_addr = usin->sin_addr.s_addr;
		*port = ntohs(usin->sin_port);
		return true;
	}
#if IS_ENABLED(CONFIG_IPV6)
	else if (addr->sa_family == AF_INET6) {
		struct sockaddr_in6 *usin6 = (struct sockaddr_in6 *)addr;

		if (!ipv6_addr_v4mapped(&usin6->sin6_addr))
			return false;
		*s_addr = usin6->sin6_addr.s6_addr32[3];
		*port = ntohs(usin6->sin6_port);
		return true;
	}
#endif
	else
		return false;

}

static int emcom_xengine_mpflow_ip_bind(struct sockaddr *addr,
					struct emcom_xengine_mpflow_info *uid)
{
	__be32 daddr;
	uint16_t dport;
	struct emcom_xengine_mpflow_ip *ip = NULL;
	int bind_dev;
	struct emcom_xengine_mpflow_app_priv *priv = uid->app_priv;
	uint32_t type = uid->algorithm_type;

	if (!emcom_xengine_mpflow_get_addr_port(addr, &daddr, &dport))
		return EMCOM_MPFLOW_BIND_WIFI;

	if (dport == EMCOM_MPFLOW_FI_PORT_443 && !uid->ptn_443_num)
		return EMCOM_MPFLOW_BIND_WIFI;

	if (dport == EMCOM_MPFLOW_FI_PORT_80 && !uid->ptn_80_num)
		return EMCOM_MPFLOW_BIND_WIFI;

	ip = emcom_xengine_mpflow_hash(daddr, priv->hashtable, type);
	if (!ip)
		return EMCOM_MPFLOW_BIND_WIFI;

	if (emcom_xengine_mpflow_ip_chk_bind_lte(priv, ip, type)) {
		ip->lte_cnt++;
		bind_dev = EMCOM_MPFLOW_BIND_LTE;
	} else
		bind_dev = EMCOM_MPFLOW_BIND_WIFI;
	ip->jiffies[ip->tot_cnt % EMCOM_MPFLOW_FLOW_JIFFIES_REC] = jiffies;
	ip->tot_cnt++;
	return bind_dev;
}

static inline void emcom_xengine_mpflow_netdisk_lte_thresh(struct emcom_xengine_mpflow_app_priv *priv, int add)
{
	if (add > 0 && priv->lte_thresh < priv->lte_thresh_max)
		priv->lte_thresh++;
	else if (add < 0 && priv->lte_thresh > priv->lte_thresh_min)
		priv->lte_thresh--;
}

static void emcom_xengine_mpflow_download_finish(struct emcom_xengine_mpflow_info *uid)
{
	struct emcom_xengine_mpflow_app_priv *priv = uid->app_priv;
	int add = 0;

	if (!priv)
		return;

	if (uid->algorithm_type == EMCOM_MPFLOW_ENABLETYPE_NET_DISK) {
		EMCOM_LOGD("lte %u %u, wifi %u %u",
			uid->lte.max_rate_received_flow,
			uid->lte.bytes_received,
			uid->wifi.max_rate_received_flow,
			uid->wifi.bytes_received);

		if ((uid->lte.bytes_received + uid->wifi.bytes_received) < EMCOM_MPFLOW_LTE_THREH_ADJUST_BYTES)
			return;

		if (uid->lte.max_rate_received_flow && uid->wifi.max_rate_received_flow &&
			uid->lte.max_rate_received_flow < uid->wifi.max_rate_received_flow) {
			/* avg_speed_per_flow_LTE< avg_speed_per_flow_WIFI, lte_thresh-- */
			EMCOM_LOGD("case 1: lte_thresh--");
			add--;
		} else if (uid->lte.max_rate_received_flow > EMCOM_MPFLOW_NETDISK_RATE_GOOD &&
			(5 * uid->wifi.max_rate_received_flow < 4 * uid->lte.max_rate_received_flow)) {
			/* avg_speed_per_flow_LTE>1M && avg_speed_per_flow_WIFI < 0.8* avg_speed_per_flow_LTE, lte_thresh++ */
			EMCOM_LOGD("case 2: lte_thresh++");
			add++;
		} else if (!priv->lte_thresh && uid->wifi.max_rate_received_flow < EMCOM_MPFLOW_NETDISK_RATE_BAD) {
			EMCOM_LOGD("case 3: lte_thresh++");
			add++;
		} else if (priv->lte_thresh == EMCOM_MPFLOW_LTE_FIRST_LTE_THREH_MAX &&
			uid->lte.max_rate_received_flow < EMCOM_MPFLOW_NETDISK_RATE_BAD) {
			EMCOM_LOGD("case 4: lte_thresh--");
			add--;
		}

		emcom_xengine_mpflow_netdisk_lte_thresh(priv, add);
	}

	emcom_xengine_mpflow_hash_clear(priv);
}

int emcom_xengine_mpflow_getmode_rand(int8_t index, uid_t uid, struct sockaddr *uaddr)
{
	bool isWifiBlock = false;
	bool isLteBlock = false;
	int bindDevice = EMCOM_MPFLOW_BIND_NONE;

	if (g_mpflow_uids[index].rst_bind_mode == EMCOM_XENGINE_MPFLOW_BINDMODE_WIFI)
		bindDevice = EMCOM_MPFLOW_BIND_WIFI;
	else if (g_mpflow_uids[index].rst_bind_mode == EMCOM_XENGINE_MPFLOW_BINDMODE_LTE)
		bindDevice = EMCOM_MPFLOW_BIND_LTE;
	else if (g_mpflow_uids[index].app_priv)
		bindDevice = emcom_xengine_mpflow_ip_bind(uaddr, &g_mpflow_uids[index]);
	else {
		g_mpflow_index++;
		if (emcom_xengine_mpflow_connum(uid, EMCOM_WLAN_IFNAME) == 0)
			bindDevice = EMCOM_MPFLOW_BIND_WIFI;
		else if (g_mpflow_index % EMCOM_MPFLOW_DEV_NUM == 0)
			bindDevice = EMCOM_MPFLOW_BIND_WIFI;
		else
			bindDevice = EMCOM_MPFLOW_BIND_LTE;
	}

	isWifiBlock = emcom_xengine_mpflow_blocked(uid, EMCOM_WLAN_IFNAME);
	isLteBlock = emcom_xengine_mpflow_blocked(uid, EMCOM_LTE_IFNAME);
	if ((isWifiBlock == true) && (bindDevice == EMCOM_MPFLOW_BIND_WIFI))
		bindDevice = EMCOM_MPFLOW_BIND_LTE;
	else if ((isLteBlock == true) && (bindDevice == EMCOM_MPFLOW_BIND_LTE))
		bindDevice = EMCOM_MPFLOW_BIND_WIFI;

	return bindDevice;
}

int emcom_xengine_mpflow_getmode_spec(int8_t index, uid_t uid)
{
	bool isWifiBlock = false;
	bool isLteBlock = false;
	int bindDevice = EMCOM_MPFLOW_BIND_NONE;

	isWifiBlock = emcom_xengine_mpflow_blocked(uid, EMCOM_WLAN_IFNAME);
	isLteBlock = emcom_xengine_mpflow_blocked(uid, EMCOM_LTE_IFNAME);

	if ((isWifiBlock && (g_mpflow_uids[index].bindmode == EMCOM_XENGINE_MPFLOW_BINDMODE_WIFI))
	|| (isLteBlock && (g_mpflow_uids[index].bindmode == EMCOM_XENGINE_MPFLOW_BINDMODE_LTE))) {
		EMCOM_LOGD("mpflow bind blocked uid: %u, bindmode: %d, blocked:%d, %d, connnum:%d",
				uid, g_mpflow_uids[index].bindmode, isWifiBlock, isLteBlock,
				emcom_xengine_mpflow_connum(uid, EMCOM_WLAN_IFNAME));
		return bindDevice;
	}

	if (g_mpflow_uids[index].bindmode == EMCOM_XENGINE_MPFLOW_BINDMODE_WIFI)
		bindDevice = EMCOM_MPFLOW_BIND_WIFI;

	if (g_mpflow_uids[index].bindmode == EMCOM_XENGINE_MPFLOW_BINDMODE_LTE)
		bindDevice = EMCOM_MPFLOW_BIND_LTE;

	return bindDevice;
}


int emcom_xengine_mpflow_getmode(int8_t index, uid_t uid, struct sockaddr *uaddr)
{
	int bindDevice = EMCOM_MPFLOW_BIND_NONE;

	if (g_mpflow_uids[index].rst_bind_mode != EMCOM_XENGINE_MPFLOW_BINDMODE_NONE) {
		if (time_after(jiffies, g_mpflow_uids[index].rst_jiffies + EMCOM_MPFLOW_RST_TIME_THREH))
			g_mpflow_uids[index].rst_bind_mode = EMCOM_XENGINE_MPFLOW_BINDMODE_NONE;
	}

	if (g_mpflow_uids[index].bindmode == EMCOM_XENGINE_MPFLOW_BINDMODE_RANDOM)
		bindDevice = emcom_xengine_mpflow_getmode_rand(index, uid, uaddr);
	else
		bindDevice = emcom_xengine_mpflow_getmode_spec(index, uid);

	return bindDevice;
}

void emcom_xengine_mpflow_bind2device(struct sock *sk, struct sockaddr *uaddr)
{
	uid_t uid;
	int8_t index;
	struct net *net = NULL;
	struct net_device *dev = NULL;
	int ifindex;
	char ifname[IFNAMSIZ] = {0};
	bool valid = false;
	int bindDevice = 0;
	uint16_t dport;

	if ((!sk) || (!uaddr))
		return;

	uid = sock_i_uid(sk).val;
	if (invalid_uid(uid))
		return;

	spin_lock_bh(&g_mpflow_lock);
	index = emcom_xengine_mpflow_finduid(uid);
	if (index == INDEX_INVALID) {
		spin_unlock_bh(&g_mpflow_lock);
		return;
	}

	valid = emcom_xengine_mpflow_checkvalid(sk, uaddr, index, &dport);
	if (valid == false) {
		EMCOM_LOGD("mpflow bind2device check valid fail uid: %u", uid);
		spin_unlock_bh(&g_mpflow_lock);
		return;
	}

	bindDevice = emcom_xengine_mpflow_getmode(index, uid, uaddr);
	if (bindDevice == EMCOM_MPFLOW_BIND_WIFI)
		memcpy(ifname, EMCOM_WLAN_IFNAME, (strlen(EMCOM_WLAN_IFNAME) + 1));
	else if (bindDevice == EMCOM_MPFLOW_BIND_LTE)
		memcpy(ifname, EMCOM_LTE_IFNAME, (strlen(EMCOM_LTE_IFNAME) + 1));
	else if (bindDevice == EMCOM_MPFLOW_BIND_NONE) {
		spin_unlock_bh(&g_mpflow_lock);
		return;
	}

	sk->is_mp_flow = 1;
	spin_unlock_bh(&g_mpflow_lock);

	net = sock_net(sk);
	rcu_read_lock();
	dev = dev_get_by_name_rcu(net, ifname);
	if ((!dev) || (emcom_xengine_mpflow_getinetaddr(dev) == false)) {
		rcu_read_unlock();
		EMCOM_LOGD("mpflow bind2device dev not ready uid: %u, sk: %pK, dev: %pK, name: %s",
				   uid, sk, dev, (dev == NULL ? "null" : dev->name));
		return;
	}
	ifindex = dev->ifindex;
	rcu_read_unlock();
	sk->sk_bound_dev_if = ifindex;
	EMCOM_LOGD("mpflow bind2device success uid: %u, sk: %pK, "
			   "ifname: %s, ifindex: %d",
			   uid, sk, ifname, ifindex);
}


bool emcom_xengine_mpflow_finddport(struct emcom_xengine_mpflow_info *mpflowinfo, uint16_t dport)
{
	int i;

	if (!mpflowinfo) {
		EMCOM_LOGE("mpflow finddport mpflow info is NULL.");
		return false;
	}

	EMCOM_LOGD("mpflow finddport dport: %d", dport);

	for (i = 0; i < EMCOM_MPFLOW_PORT_RANGE_NUM_MAX; i++) {
		if (mpflowinfo->dport_range[i].start_port <= dport &&
			mpflowinfo->dport_range[i].end_port >= dport)
			return true;
	}

	return false;
}

struct emcom_xengine_mpflow_stat *emcom_xengine_mpflow_get(int uid, char *name, int ifindex)
{
	struct emcom_xengine_mpflow_stat *node = NULL;

	list_for_each_entry(node, &emcom_xengine_mpflow_list, list) {
		if ((node->uid == uid) && (!strncmp(node->name, name, strlen(name))))
			return node;
	}

	node = (struct emcom_xengine_mpflow_stat *)kzalloc(sizeof(struct emcom_xengine_mpflow_stat), GFP_ATOMIC);
	if (!node) {
		EMCOM_LOGD("MpFlow %d kzalloc Emcom_Xengine_mpflow_stat node fail\n", __LINE__);
		return NULL;
	}

	node->uid = uid;
	node->ifindex = ifindex;
	strncpy(node->name, name, IFNAMSIZ);
	node->mpflow_fallback = EMCOM_MPFLOW_FALLBACK_CLR;
	node->mpflow_fail_nopayload = 0;
	node->mpflow_fail_syn_rst = 0;
	node->mpflow_fail_syn_timeout = 0;
	node->mpflow_estlink = 0;
	node->start_jiffies = 0;
	memset(node->retrans_count, 0, sizeof(node->retrans_count));

	list_add(&(node->list), &emcom_xengine_mpflow_list);

	return node;
}


void emcom_xengine_mpflow_delete(int uid)
{
	struct emcom_xengine_mpflow_stat *node = NULL;
	struct emcom_xengine_mpflow_stat *nodesafe = NULL;

	list_for_each_entry_safe(node, nodesafe, &emcom_xengine_mpflow_list, list) {
		if (node->uid == uid) {
			list_del(&node->list);
			kfree(node);
		}
	}
}

void emcom_xengine_mpflow_clear_blocked(int uid)
{
	struct emcom_xengine_mpflow_stat *node = NULL;

	list_for_each_entry(node, &emcom_xengine_mpflow_list, list) {
		if (node->uid == uid) {
			node->mpflow_fallback = EMCOM_MPFLOW_FALLBACK_CLR;
			node->mpflow_fail_nopayload = 0;
			node->mpflow_fail_syn_rst = 0;
			node->mpflow_fail_syn_timeout = 0;
			node->start_jiffies = 0;
			memset(node->retrans_count, 0, sizeof(node->retrans_count));
		}
	}
}

bool emcom_xengine_mpflow_blocked(int uid, char *name)
{
	struct emcom_xengine_mpflow_stat *node = NULL;

	list_for_each_entry(node, &emcom_xengine_mpflow_list, list) {
		if ((node->uid == uid) && (!strncmp(node->name, name, strlen(name))))
			return (node->mpflow_fallback == EMCOM_MPFLOW_FALLBACK_SET);
	}

	return false;
}

int16_t emcom_xengine_mpflow_connum(int uid, char *name)
{
	struct emcom_xengine_mpflow_stat *node = NULL;

	list_for_each_entry(node, &emcom_xengine_mpflow_list, list) {
		if ((node->uid == uid) && (!strncmp(node->name, name, strlen(name))))
			return node->mpflow_estlink;
	}
	return 0;
}


void emcom_xengine_mpflow_report(void *data, int len)
{
	emcom_send_msg2daemon(NETLINK_EMCOM_KD_MPFLOW_FALLBACK, data, len);
}

void emcom_xengine_mpflow_show(void)
{
	struct emcom_xengine_mpflow_stat *node = NULL;

	list_for_each_entry(node, &emcom_xengine_mpflow_list, list) {
		EMCOM_LOGE("MpFlow showinfo uid:%d inf:%d(%s) fail(%d) estlink:%d "
				   "nodata,rst,tout(%d,%d,%d)\n",
				   node->uid, node->ifindex, node->name,
				   node->mpflow_fallback, node->mpflow_estlink,
				   node->mpflow_fail_nopayload, node->mpflow_fail_syn_rst,
				   node->mpflow_fail_syn_timeout);
	}
	return;
}

bool emcom_xengine_mpflow_errlink(int reason, struct emcom_xengine_mpflow_stat *node)
{
	if (reason == EMCOM_MPFLOW_FALLBACK_NOPAYLOAD)
		node->mpflow_fail_nopayload++;
	else if (reason == EMCOM_MPFLOW_FALLBACK_SYN_RST)
		node->mpflow_fail_syn_rst++;
	else if (reason == EMCOM_MPFLOW_FALLBACK_SYN_TOUT)
		node->mpflow_fail_syn_timeout++;
	else
		return false;

	if ((node->mpflow_fail_nopayload >= EMCOM_MPFLOW_FALLBACK_NOPAYLOAD_THRH)
		|| (node->mpflow_fail_syn_rst >= EMCOM_MPFLOW_FALLBACK_SYN_RST_THRH)
		|| (node->mpflow_fail_syn_timeout >= EMCOM_MPFLOW_FALLBACK_SYN_TOUT_THRH))
		return true;

	return false;
}

bool emcom_xengine_mpflow_retrans(int reason, struct emcom_xengine_mpflow_stat *node)
{
	int i;

	if (reason != EMCOM_MPFLOW_FALLBACK_RETRANS)
		return false;

	for (i = 1; (i <= node->retrans_count[0]) && (i <= EMCOM_MPFLOW_FALLBACK_RETRANS_TIME); i++) {
		if time_before_eq(jiffies, node->start_jiffies + i * HZ) {
			node->retrans_count[i]++;
			break;
		}
	}

	/* Time range matched */
	if (i <= node->retrans_count[0]) {
		/* expand time range */
		if (node->retrans_count[i] == EMCOM_MPFLOW_FALLBACK_RETRANS_THRH) {
			node->retrans_count[0]++;
			EMCOM_LOGD("MpFlow fallback retrans uid:%d inf:%d(%s) count:%d, jiffies:%lu\n",
				node->uid, node->ifindex, node->name, node->retrans_count[0], node->start_jiffies);
		}
		/* retransmission fallback */
		if (node->retrans_count[0] > EMCOM_MPFLOW_FALLBACK_RETRANS_TIME) {
			memset(node->retrans_count, 0, sizeof(node->retrans_count));
			return true;
		}
	} else {
		memset(node->retrans_count, 0, sizeof(node->retrans_count));
		node->retrans_count[0] = 1;
		node->retrans_count[1] = 1;
		node->start_jiffies = jiffies;
	}

	return false;
}

int8_t emcom_xengine_mpflow_checkstatus(struct sock *sk, int reason, int state, struct emcom_xengine_mpflow_stat *node)
{
	struct tcp_sock *tp = tcp_sk(sk);
	struct inet_sock *inet = inet_sk(sk);
	int8_t result = EMCOM_MPFLOW_FALLBACK_NONE;
	int oldstate = sk->sk_state;

	if (reason == EMCOM_MPFLOW_FALLBACK_NOPAYLOAD) {
#ifdef CONFIG_MPTCP
		if (mptcp_meta_sk(sk) == sk)
			return result;
#endif
		/* EST->DOWN */
		if ((oldstate == TCP_ESTABLISHED) && (state != TCP_ESTABLISHED)) {
			result = ((tp->bytes_received <= 1) && ((tp->bytes_acked > 1) ||
				(tp->snd_nxt - tp->snd_una > EMCOM_MPFLOW_SND_BYTE_THRESHOLD))) ?
				EMCOM_MPFLOW_FALLBACK_SET : EMCOM_MPFLOW_FALLBACK_CLR;
			if (node->mpflow_estlink > 0)
				node->mpflow_estlink--;
		/* DOWN->EST */
		} else if ((oldstate != TCP_ESTABLISHED) && (state == TCP_ESTABLISHED)) {
			result = EMCOM_MPFLOW_FALLBACK_SYNCLR;
			node->mpflow_estlink++;
		}
	} else if (reason == EMCOM_MPFLOW_FALLBACK_SYN_TOUT)
		result = ((1 << oldstate) & (TCPF_SYN_SENT | TCPF_SYN_RECV)) ? EMCOM_MPFLOW_FALLBACK_SET : EMCOM_MPFLOW_FALLBACK_NONE;
	else
		result = EMCOM_MPFLOW_FALLBACK_SET;

	if (result != EMCOM_MPFLOW_FALLBACK_NONE)
		EMCOM_LOGD("MpFlow checkinfo uid:%d sk:%pK inf:%d[%s] R:%d P[%d->%d] ret:%d,snt,fly,ack,rcv:%u,%u,%llu,%llu\n",
				   node->uid, sk, node->ifindex, node->name, reason,
				   ntohs(inet->inet_sport), ntohs(inet->inet_dport),
				   result, tp->segs_out, tp->snd_nxt - tp->snd_una, tp->bytes_acked, tp->bytes_received);

	return result;
}

void emcom_xengine_mpflow_fallback(struct sock *sk, int reason, int state)
{
	struct emcom_xengine_mpflow_stat *node = NULL;
	struct emcom_xengine_mpflow_fallback fallback;
	struct net *net = sock_net(sk);
	struct net_device *dev = NULL;
	char *name = NULL;
	int8_t result = INDEX_INVALID;
	uid_t uid;
	int8_t index;

	uid = sock_i_uid(sk).val;
	if (invalid_uid(uid))
		return;

	index = emcom_xengine_mpflow_finduid(uid);
	if (index == INDEX_INVALID)
		return;

	rcu_read_lock();
	dev = dev_get_by_index_rcu(net, sk->sk_bound_dev_if);
	rcu_read_unlock();
	if (!dev)
		name = "none";
	else
		name = dev->name;

	node = emcom_xengine_mpflow_get(uid, name, sk->sk_bound_dev_if);
	if (!node)
		return;

	result = emcom_xengine_mpflow_checkstatus(sk, reason, state, node);
	if (result == EMCOM_MPFLOW_FALLBACK_SET) {
		if (emcom_xengine_mpflow_errlink(reason, node) || emcom_xengine_mpflow_retrans(reason, node)) {

			node->mpflow_fallback = EMCOM_MPFLOW_FALLBACK_SET;
			EMCOM_LOGE("MpFlow fallback uid:%d inf:%d(%s) estlink:%d nodata,rst,tout:%d,%d,%d\n",
					   node->uid, node->ifindex, node->name, node->mpflow_estlink,
					   node->mpflow_fail_nopayload, node->mpflow_fail_syn_rst, node->mpflow_fail_syn_timeout);

			/* report connection unreachabled */
			if (!strncmp(EMCOM_WLAN_IFNAME, name, strlen(name))) {
				fallback.uid = node->uid;
				fallback.reason = EMCOM_MPFLOW_FALLBACK_WLAN_OFFSET + reason;
				emcom_xengine_mpflow_report(&fallback, sizeof(fallback));
				EMCOM_LOGE("MpFlow fallback report uid:%d reason: %d\n", fallback.uid, fallback.reason);
			} else if (!strncmp(EMCOM_LTE_IFNAME, name, strlen(name)) && (reason != EMCOM_MPFLOW_FALLBACK_RETRANS)) {
				fallback.uid = node->uid;
				fallback.reason = EMCOM_MPFLOW_FALLBACK_LTE_OFFSET + reason;
				emcom_xengine_mpflow_report(&fallback, sizeof(fallback));
				EMCOM_LOGE("MpFlow fallback report uid:%d reason: %d\n", fallback.uid, fallback.reason);
			}
		}
	} else if (result == EMCOM_MPFLOW_FALLBACK_CLR) {
		node->mpflow_fallback = EMCOM_MPFLOW_FALLBACK_CLR;
		node->mpflow_fail_nopayload = 0;
		node->mpflow_fail_syn_rst = 0;
		node->mpflow_fail_syn_timeout = 0;
	} else if (result == EMCOM_MPFLOW_FALLBACK_SYNCLR) {
		node->mpflow_fail_syn_rst = 0;
		node->mpflow_fail_syn_timeout = 0;
	}

	return;
}

char *strtok(char *string_org, const char *demial)
{
	static unsigned char *last;
	unsigned char *str;
	const unsigned char *ctrl = (const unsigned char *)demial;
	unsigned char map[32];
	int count;

	for (count = 0; count < 32; count++) {
		map[count] = 0;
	}
	do {
		map[*ctrl >> 3] |= (1 << (*ctrl & 7));
	} while (*ctrl++);
	if (string_org != NULL) {
		str = (unsigned char *)string_org;
	} else {
		str = last;
	}
	while ((map[*str >> 3] & (1 << (*str & 7))) && *str) {
		str++;
	}
	string_org = (char *)str;
	for (; *str; str++) {
		if (map[*str >> 3] & (1 << (*str & 7))) {
			*str++ = '\0';
			break;
		}
	}
	last = str;
	if (string_org == (char *)str) {
		return NULL;
	} else {
		return string_org;
	}
}

static int emcom_xengine_mpflow_split(char *src, const char *separator, char **dest, int *num)
{
	char *p = NULL;
	int count = 0;

	if (src == NULL || strlen(src) == 0 || separator == NULL || strlen(separator) == 0) {
		return *num;
	}
	p = strtok(src, separator);
	while (p != NULL) {
		*dest++ = p;
		++count;
		p = strtok(NULL, separator);
	}
	*num = count;
	return *num;
}

static bool emcom_xengine_mpflow_ptn_init(struct emcom_xengine_mpflow_ptn ptn[], uint8_t *num, const char *hex)
{
	bool ret = false;
	char *revbuf[EMCOM_MPFLOW_FI_PTN_MAX_NUM] = {0};
	int n = 0;
	int i;

	EMCOM_LOGD("hex %s\n", hex);
	if (emcom_xengine_mpflow_split((char *)hex, EMCOM_MPFLOW_FI_PTN_SEPERATE_CHAR, revbuf, &n) == 0) {
		EMCOM_LOGE("hex split fail\n");
		return false;
	}

	for (i = 0; i < n; i++) {
		ptn[i].skip = NULL;
		ptn[i].shift = NULL;
		memset(ptn[i].ptn, 0, EMCOM_MPFLOW_FI_PTN_MAX_SIZE);
		ptn[i].ptnlen = strnlen(revbuf[i], EMCOM_MPFLOW_FI_PTN_MAX_SIZE) >> 1;
		if (hex2bin(ptn[i].ptn, revbuf[i], ptn[i].ptnlen)) {
			ptn[i].is_init = false;
			EMCOM_LOGE("hex2bin fail\n");
			return false;
		}
		ret = emcom_xengine_mpflow_bm_build((const uint8_t *)ptn[i].ptn, ptn[i].ptnlen,
											&(ptn[i].skip), &(ptn[i].shift));
		if (!ret) {
			EMCOM_LOGE("emcom_xengine_mpflow_bm_build failed!\n");
			ptn[i].is_init = false;
			return false;
		}
		ptn[i].is_init = true;
		EMCOM_LOGD("ptn %s init succ!\n", ptn[i].ptn);
	}
	*num = n;
	return true;
}

static void emcom_xengine_mpflow_reset(struct tcp_sock *tp)
{
	struct sock *sk = (struct sock *)tp;

	EMCOM_LOGD("reset sk %pK sport is %u", tp, sk->sk_num);
	if (sk->sk_state == TCP_ESTABLISHED) {
		local_bh_disable();
		bh_lock_sock(sk);

		if (!sock_flag(sk, SOCK_DEAD)) {
			sk->sk_err = ECONNABORTED;
			/* This barrier is coupled with smp_rmb() in tcp_poll() */
			smp_wmb();
			sk->sk_error_report(sk);
			if (tcp_need_reset(sk->sk_state))
				tcp_send_active_reset(sk, sk->sk_allocation);
			tcp_done(sk);
		}

		bh_unlock_sock(sk);
		local_bh_enable();
	}
}

static void emcom_xengine_mpflow_download_flow_del(struct emcom_xengine_mpflow_iface *iface,
	struct emcom_xengine_mpflow_node *flow)
{
	struct sock *sk = (struct sock *)flow->tp;

	EMCOM_LOGD("remove sk port %u", sk->sk_num);
	sock_put(sk);
	list_del(&flow->list);
	kfree(flow);
	iface->flow_cnt--;
}

static void emcom_xengine_mpflow_update(struct emcom_xengine_mpflow_iface *flows)
{
	struct emcom_xengine_mpflow_node *node = NULL;
	struct emcom_xengine_mpflow_node *tmp = NULL;
	struct sock *sk = NULL;
	u32 rcv_bytes;
	u32 srtt_ms_sum = 0;
	u32 max_rate_received = 0;
	u64 bytes_received;
	u16 is_slow = 1;
	u16 flow_valid_cnt = 0;
	u16 flow_cnt = 0;
	uint32_t rate_received_flow = 0;
	int i;
	for (i = EMCOM_MPFLOW_FI_STAT_SECONDS-1; i > 0; i--)
		flows->rate_received[i] = flows->rate_received[i-1];
	flows->rate_received[0] = 0;
	list_for_each_entry_safe(node, tmp, &flows->flows, list) {
		sk = (struct sock *)node->tp;

		bytes_received = node->tp->bytes_received;
		rcv_bytes = (u32)(bytes_received - node->last_bytes_received);
		node->last_bytes_received = bytes_received;
		/* now update interval is 1s, rcv_bytes is the rate */
		for (i = EMCOM_MPFLOW_FI_STAT_SECONDS-1; i > 0; i--)
			node->rate_received[i] = node->rate_received[i-1];
		node->rate_received[0] = rcv_bytes;
		/* srtt_us is smoothed round trip time << 3 in usecs */
		srtt_ms_sum += (node->tp->srtt_us >> 3) / EMCOM_MPFLOW_MICROSECOND_OF_MILLISECOND;
		flow_cnt++;
		for (i = EMCOM_MPFLOW_FI_STAT_SECONDS-1; i >= 0; i--) {
			if (node->rate_received[i] > EMCOM_MPFLOW_FLOW_SLOW_THREH) {
				is_slow = 0;
				break;
			}
		}
		if (bytes_received > EMCOM_MPFLOW_RATE_VALID_THREH)
			flow_valid_cnt++;
		max_rate_received += node->rate_received[0];
		flows->bytes_received += rcv_bytes;
		flows->rate_received[0] += rcv_bytes;
		if (sk->sk_state != TCP_ESTABLISHED)
			emcom_xengine_mpflow_download_flow_del(flows, node);
	}
	if (flow_cnt > 0)
		flows->mean_srtt_ms = srtt_ms_sum / flow_cnt;
	flows->is_slow = is_slow;
	flows->flow_valid_cnt = flow_valid_cnt;
	if (flow_valid_cnt) {
		rate_received_flow = max_rate_received / flow_valid_cnt;
		if (flows->max_rate_received_flow < rate_received_flow)
			flows->max_rate_received_flow = rate_received_flow;
	}

	if (flows->max_rate_received < max_rate_received)
		flows->max_rate_received = max_rate_received;
}

static void emcom_xengine_mpflow_set_bind(struct emcom_xengine_mpflow_info *mpflow_uid, int bind_mode)
{
	switch (bind_mode) {
	case EMCOM_XENGINE_MPFLOW_BINDMODE_NORST:
		return;
	case EMCOM_XENGINE_MPFLOW_BINDMODE_RST2FAST:
		mpflow_uid->rst_bind_mode = (mpflow_uid->wifi.max_rate_received >= mpflow_uid->lte.max_rate_received) ?
			EMCOM_XENGINE_MPFLOW_BINDMODE_WIFI : EMCOM_XENGINE_MPFLOW_BINDMODE_LTE;
		break;
	case EMCOM_XENGINE_MPFLOW_BINDMODE_RST2WIFI:
		mpflow_uid->rst_bind_mode = EMCOM_XENGINE_MPFLOW_BINDMODE_WIFI;
		break;
	case EMCOM_XENGINE_MPFLOW_BINDMODE_RST2LTE:
		mpflow_uid->rst_bind_mode = EMCOM_XENGINE_MPFLOW_BINDMODE_LTE;
		break;
	default:
		return;
	}

	mpflow_uid->rst_jiffies = jiffies;
	EMCOM_LOGD("uid %u rst_bind_mode %u rst_jiffies %lu", mpflow_uid->uid,
			   mpflow_uid->rst_bind_mode, mpflow_uid->rst_jiffies);
}

static bool emcom_xengine_mpflow_unbalance_netdisk(struct emcom_xengine_mpflow_iface *iface1,
					struct emcom_xengine_mpflow_iface *iface2, uint8_t index)
{
	if (iface1->max_rate_received && iface1->max_rate_received < (iface2->max_rate_received >> 1) && iface2->is_slow &&
		iface1->max_rate_received_flow < EMCOM_MPFLOW_NETDISK_RATE_BAD &&
		((iface1->is_wifi && g_mpflow_uids[index].bindmode == EMCOM_XENGINE_MPFLOW_BINDMODE_LTE) ||
		(!iface1->is_wifi && g_mpflow_uids[index].bindmode == EMCOM_XENGINE_MPFLOW_BINDMODE_WIFI) ||
		g_mpflow_uids[index].bindmode == EMCOM_XENGINE_MPFLOW_BINDMODE_RANDOM))
		return true;
	else
		return false;
}

static bool emcom_xengine_mpflow_unbalance_market(struct emcom_xengine_mpflow_iface *iface1,
					struct emcom_xengine_mpflow_iface *iface2, uint8_t index)
{
	if (iface1->max_rate_received &&
		((iface1->is_wifi && iface2->is_slow && iface1->max_rate_received < (iface2->max_rate_received >> 1) &&
		(g_mpflow_uids[index].bindmode == EMCOM_XENGINE_MPFLOW_BINDMODE_LTE ||
		g_mpflow_uids[index].bindmode == EMCOM_XENGINE_MPFLOW_BINDMODE_RANDOM)) ||
		(!iface1->is_wifi && iface1->max_rate_received < iface2->max_rate_received &&
		(g_mpflow_uids[index].bindmode == EMCOM_XENGINE_MPFLOW_BINDMODE_WIFI ||
		g_mpflow_uids[index].bindmode == EMCOM_XENGINE_MPFLOW_BINDMODE_RANDOM))))
		return true;
	else
		return false;
}

static bool emcom_xengine_mpflow_single_path(struct emcom_xengine_mpflow_iface *iface1,
					struct emcom_xengine_mpflow_iface *iface2, uint8_t index)
{
	if (!g_mpflow_uids[index].rst_to_another && g_mpflow_uids[index].bindmode == EMCOM_XENGINE_MPFLOW_BINDMODE_RANDOM &&
		iface1->flow_valid_cnt > 1 && !iface2->max_rate_received_flow)
		return true;
	else
		return false;
}

static int emcom_xengine_mpflow_chk_rst_market(struct emcom_xengine_mpflow_iface *iface1,
					struct emcom_xengine_mpflow_iface *iface2, uint8_t index)
{
	struct emcom_xengine_mpflow_node *node = NULL;
	struct emcom_xengine_mpflow_node *tmp = NULL;
	int need_set_bind = EMCOM_XENGINE_MPFLOW_BINDMODE_NORST;
	list_for_each_entry_safe(node, tmp, &iface1->flows, list) {
		if (time_before(jiffies, node->start_jiffies + EMCOM_MPFLOW_FLOW_TIME_THREH))
			continue;
		/* all downloading flows are on same iface */
		if (node->last_bytes_received > EMCOM_MPFLOW_RST_RCV_BYTES_THREH &&
			emcom_xengine_mpflow_single_path(iface1,iface2,index)) {
			emcom_xengine_mpflow_reset(node->tp);
			need_set_bind = iface1->is_wifi ? EMCOM_XENGINE_MPFLOW_BINDMODE_RST2LTE :
				EMCOM_XENGINE_MPFLOW_BINDMODE_RST2WIFI;
			g_mpflow_uids[index].rst_to_another = 1;
			break;
		}
		if ((iface1->bytes_received + iface2->bytes_received) <= EMCOM_MPFLOW_RST_IFACE_GOOD)
			break;
		/* iface1 is slower than half of iface2 */
		if (iface1->max_rate_received && iface1->max_rate_received < (iface2->max_rate_received >> 1))
			need_set_bind = EMCOM_XENGINE_MPFLOW_BINDMODE_RST2FAST;
		/* both wifi and lte download over */
		if (iface1->is_slow && (iface2->is_slow || !iface2->bytes_received)) {
			emcom_xengine_mpflow_reset(node->tp);
			need_set_bind = EMCOM_XENGINE_MPFLOW_BINDMODE_RST2FAST;
		}
		/* wifi is slower than half of lte or lte is slower than wifi */
		if (node->last_bytes_received > EMCOM_MPFLOW_RST_RCV_BYTES_THREH &&
			emcom_xengine_mpflow_unbalance_market(iface1,iface2,index)) {
			emcom_xengine_mpflow_reset(node->tp);
			need_set_bind = EMCOM_XENGINE_MPFLOW_BINDMODE_RST2FAST;
		}
	}
	return need_set_bind;
}

static int emcom_xengine_mpflow_chk_rst_netdisk(struct emcom_xengine_mpflow_iface *iface1,
					struct emcom_xengine_mpflow_iface *iface2, uint8_t index)
{
	struct emcom_xengine_mpflow_node *node = NULL;
	struct emcom_xengine_mpflow_node *tmp = NULL;
	int need_set_bind = EMCOM_XENGINE_MPFLOW_BINDMODE_NORST;
	list_for_each_entry_safe(node, tmp, &iface1->flows, list) {
		if (time_before(jiffies, node->start_jiffies + EMCOM_MPFLOW_FLOW_TIME_THREH) ||
			(iface1->bytes_received + iface2->bytes_received) <= EMCOM_MPFLOW_RST_IFACE_GOOD)
			continue;
		/* iface1 is slower than half of iface2 */
		if (iface1->max_rate_received &&
			iface1->max_rate_received < (iface2->max_rate_received >> 1))
			need_set_bind = EMCOM_XENGINE_MPFLOW_BINDMODE_RST2FAST;
		/* both wifi and lte download over */
		if (iface1->is_slow && (iface2->is_slow || !iface2->bytes_received)) {
			emcom_xengine_mpflow_reset(node->tp);
			need_set_bind = EMCOM_XENGINE_MPFLOW_BINDMODE_RST2FAST;
		}
		if (node->last_bytes_received > EMCOM_MPFLOW_RST_RCV_BYTES_THREH &&
			emcom_xengine_mpflow_unbalance_netdisk(iface1,iface2,index)) {
			emcom_xengine_mpflow_reset(node->tp);
		}
	}
	return need_set_bind;
}

static int emcom_xengine_mpflow_chk_rst_weibo(struct emcom_xengine_mpflow_iface *iface1,
					struct emcom_xengine_mpflow_iface *iface2)
{
	int need_set_bind = EMCOM_XENGINE_MPFLOW_BINDMODE_NORST;
	uint32_t  wifi_download_time = 0;
	uint32_t lte_download_time = 0;
	if (!iface1->max_rate_received_flow || !iface2->max_rate_received_flow ||
		time_before(jiffies, iface1->start_jiffies + EMCOM_MPFLOW_FLOW_TIME_THREH) ||
		time_before(jiffies, iface2->start_jiffies + EMCOM_MPFLOW_FLOW_TIME_THREH))
		return need_set_bind;
	wifi_download_time = EMCOM_MPFLOW_MICROSECOND_OF_MILLISECOND * EMCOM_MPFLOW_WEIBO_SIZE / iface1->max_rate_received;
	lte_download_time =  EMCOM_MPFLOW_MICROSECOND_OF_MILLISECOND * EMCOM_MPFLOW_WEIBO_SIZE / iface2->max_rate_received;
	if (iface1->mean_srtt_ms + wifi_download_time < iface2->mean_srtt_ms + lte_download_time /EMCOM_MPFLOW_DEV_NUM)
		need_set_bind = EMCOM_XENGINE_MPFLOW_BINDMODE_RST2WIFI;
	else if (iface2->mean_srtt_ms + lte_download_time < iface1->mean_srtt_ms + wifi_download_time /EMCOM_MPFLOW_DEV_NUM)
		need_set_bind = EMCOM_XENGINE_MPFLOW_BINDMODE_RST2LTE;
	return need_set_bind;
}

static void emcom_xengine_mpflow_timer(unsigned long arg)
{
	uint8_t index;
	bool need_reset_timer = false;
	int bind_mode_wifi = EMCOM_XENGINE_MPFLOW_BINDMODE_NORST;
	int bind_mode_lte = EMCOM_XENGINE_MPFLOW_BINDMODE_NORST;
	int bind_mode;
	struct emcom_xengine_mpflow_info *mpflow_uid = NULL;

	spin_lock(&g_mpflow_lock);
	for (index = 0; index < EMCOM_MPFLOW_MAX_APP; index++) {
		mpflow_uid = &g_mpflow_uids[index];
		if (mpflow_uid->uid == UID_INVALID_APP || !mpflow_uid->is_downloading)
			continue;
		emcom_xengine_mpflow_update(&mpflow_uid->wifi);
		emcom_xengine_mpflow_update(&mpflow_uid->lte);
		if (g_mpflow_uids[index].algorithm_type == EMCOM_MPFLOW_ENABLETYPE_NET_DISK) {
			bind_mode_wifi = emcom_xengine_mpflow_chk_rst_netdisk(&mpflow_uid->wifi, &mpflow_uid->lte, index);
			bind_mode_lte = emcom_xengine_mpflow_chk_rst_netdisk(&mpflow_uid->lte, &mpflow_uid->wifi, index);
		}else if (g_mpflow_uids[index].algorithm_type == EMCOM_MPFLOW_ENABLETYPE_MARKET) {
			bind_mode_wifi = emcom_xengine_mpflow_chk_rst_market(&mpflow_uid->wifi, &mpflow_uid->lte, index);
			bind_mode_lte = emcom_xengine_mpflow_chk_rst_market(&mpflow_uid->lte, &mpflow_uid->wifi, index);
		}else{
			bind_mode_wifi = emcom_xengine_mpflow_chk_rst_weibo(&mpflow_uid->wifi, &mpflow_uid->lte);
		}
		EMCOM_LOGD("uid %u rst_mode are %u and %u",mpflow_uid->uid,bind_mode_wifi,bind_mode_lte);
		bind_mode = (bind_mode_wifi == EMCOM_XENGINE_MPFLOW_BINDMODE_NORST) ?
			bind_mode_lte : bind_mode_wifi;
		emcom_xengine_mpflow_set_bind(mpflow_uid, bind_mode);
		if (mpflow_uid->wifi.flow_cnt || mpflow_uid->lte.flow_cnt)
			need_reset_timer = true;
		else {
			EMCOM_LOGD("uid %u download is stop", mpflow_uid->uid);
			emcom_xengine_mpflow_download_finish(mpflow_uid);
			emcom_xengine_mpflow_fi_init(mpflow_uid);
		}
	}

	if (need_reset_timer)
		mod_timer(&g_mpflow_tm, jiffies + HZ);
	else {
		EMCOM_LOGD("stop mpflow timer");
		g_mpflow_tm_running = false;
	}
	spin_unlock(&g_mpflow_lock);
}

static void emcom_xengine_mpflow_download_flow_add(struct sock *sk)
{
	struct tcp_sock *tp = tcp_sk(sk);
	uid_t uid = sk->sk_uid.val;
	int index;
	struct dst_entry *dst = NULL;
	struct emcom_xengine_mpflow_iface *iface = NULL;
	struct emcom_xengine_mpflow_node *node = NULL;
	uint16_t port = sk->sk_num;
	int i;

	spin_lock_bh(&g_mpflow_lock);
	index = emcom_xengine_mpflow_finduid(uid);
	if (index != INDEX_INVALID) {
		dst = sk_dst_get(sk);
		if (dst) {
			if (strncmp(EMCOM_WLAN_IFNAME, dst->dev->name, IFNAMSIZ) == 0) {
				iface = &g_mpflow_uids[index].wifi;
			} else if (strncmp(EMCOM_LTE_IFNAME, dst->dev->name, IFNAMSIZ) == 0) {
				iface = &g_mpflow_uids[index].lte;
			} else {
				EMCOM_LOGD("sk port %u iface is %s", port, dst->dev->name);
				dst_release(dst);
				spin_unlock_bh(&g_mpflow_lock);
				return;
			}
			dst_release(dst);
		} else {
			EMCOM_LOGD("sk port %u dst is not found", port);
			spin_unlock_bh(&g_mpflow_lock);
			return;
		}

		sk->is_download_flow = 1;

		node = kmalloc(sizeof(struct emcom_xengine_mpflow_node), GFP_ATOMIC);
		if (node) {
			node->last_bytes_received = tp->bytes_received;
			for (i = EMCOM_MPFLOW_FI_STAT_SECONDS-1; i >= 0; i--)
				node->rate_received[i] = 0;
			node->tp = tp;
			node->start_jiffies = jiffies;
			sock_hold(sk);
			list_add(&node->list, &iface->flows);
			iface->flow_cnt++;
			if (!g_mpflow_uids[index].is_downloading) {
				EMCOM_LOGD("uid %u is_downloading", g_mpflow_uids[index].uid);
				g_mpflow_uids[index].is_downloading = 1;
				iface->start_jiffies = jiffies;
			}

			EMCOM_LOGD("sk %pK is a download flow sport %u ", tp, port);
			if (!g_mpflow_tm_running) {
				init_timer(&g_mpflow_tm);
				g_mpflow_tm.function = emcom_xengine_mpflow_timer;
				g_mpflow_tm.data     = (unsigned long)NULL;
				g_mpflow_tm_running = true;
				EMCOM_LOGD("start mpflow timer");
				mod_timer(&g_mpflow_tm, jiffies + HZ);
			}
		}
	} else
		EMCOM_LOGD("emcom_xengine_mpflow_finduid fail");

	spin_unlock_bh(&g_mpflow_lock);
}

static uint8_t *emcom_xengine_mpflow_make_skip(const uint8_t *ptrn, uint8_t plen)
{
	int i;
	uint8_t *skip = NULL;

	skip = (uint8_t *)kmalloc(EMCOM_MPFLOW_FI_ASCII_CODE_SIZE *
		 sizeof(uint8_t), GFP_ATOMIC);
	if (!skip)
		return NULL;

	for (i = 0; i < EMCOM_MPFLOW_FI_ASCII_CODE_SIZE; i++) {
		*(skip + i) = (plen >= EMCOM_MPFLOW_FI_ASCII_CODE_MARK) ?
			EMCOM_MPFLOW_FI_ASCII_CODE_MARK : (plen + 1);
	}
	while (plen != 0)
		*(skip + (uint8_t)*ptrn++) = plen--;
	return skip;
}

static uint8_t *emcom_xengine_mpflow_make_shift(const uint8_t *ptrn, uint8_t plen)
{
	uint8_t *sptr = NULL;
	const uint8_t *pptr = NULL;
	uint8_t c;
	uint8_t *shift = NULL;
	const uint8_t *p1 = NULL;
	const uint8_t *p2 = NULL;
	const uint8_t *p3 = NULL;

	shift = (uint8_t *)kmalloc(plen * sizeof(uint8_t), GFP_ATOMIC);
	if (!shift)
		return NULL;
	sptr = shift + plen - 1;
	pptr = ptrn + plen - 1;
	c = *(ptrn + plen - 1);
	*sptr = 1;

	while (sptr-- != shift) {
		p1 = ptrn + plen - 2;
		do {
			while ((p1 >= ptrn) && (*p1-- != c));
			p2 = ptrn + plen - 2;
			p3 = p1;
			while ((p3 >= ptrn) && (*p3-- == *p2--) &&
				 (p2 >= pptr));
		} while ((p3 >= ptrn) && (p2 >= pptr));
		*sptr = shift + plen - sptr + p2 - p3;
		pptr--;
	}
	return shift;
}

static void emcom_xengine_mpflow_bm_free(uint8_t **skip, uint8_t **shift)
{
	if (!skip) {
		kfree(*skip);
		*skip = NULL;
	}
	if (!shift) {
		kfree(*shift);
		*shift = NULL;
	}
}

static bool emcom_xengine_mpflow_bm_build(const uint8_t *ptn, uint32_t ptnlen,
			uint8_t **skip, uint8_t **shift)
{
	if (ptn != NULL && ptnlen > 0 && skip != NULL && shift != NULL) {
		*skip = emcom_xengine_mpflow_make_skip(ptn, ptnlen);
		if (!(*skip))
			return false;
		*shift = emcom_xengine_mpflow_make_shift(ptn, ptnlen);
		if (!(*shift)) {
			kfree(*skip);
			*skip = NULL;
			return false;
		}
		return true;
	}
	return false;
}

static bool emcom_xengine_mpflow_bm_search(const uint8_t *buf, uint32_t buflen,
			const struct emcom_xengine_mpflow_ptn *sptn, uint32_t *offset)
{
	uint32_t ptnlen = sptn->ptnlen;
	const uint8_t *ptn = sptn->ptn;
	const uint8_t *skip = sptn->skip;
	const uint8_t *shift = sptn->shift;
	uint32_t pindex;
	uint8_t skip_stride;
	uint8_t shift_stride;
	uint32_t bindex = ptnlen;

	if (buf != NULL && ptn != NULL && skip != NULL &&
		shift != NULL && ptnlen > 0 && buflen > ptnlen) {
		while (bindex <= buflen) {
			pindex = ptnlen;
			while (ptn[--pindex] == buf[--bindex]) {
				if (pindex == 0) {
					if (offset != NULL)
						*offset = bindex;
					return true;
				}
			}
			skip_stride = skip[buf[bindex]];
			shift_stride = shift[pindex];
			bindex += ((skip_stride > shift_stride) ?
				skip_stride : shift_stride);
		}
	}
	return false;
}

static void emcom_xengine_mpflow_ptn_deinit(struct emcom_xengine_mpflow_ptn ptn[], uint8_t num)
{
	uint8_t i;

	for (i = 0; i < num; i++) {
		if (ptn[i].is_init) {
			emcom_xengine_mpflow_bm_free(&(ptn[i].skip), &(ptn[i].shift));
			ptn[i].is_init = false;
		}
	}
}

static void emcom_xengine_mpflow_apppriv_deinit(struct emcom_xengine_mpflow_info *uid)
{
	if (!uid->app_priv)
		return;

	emcom_xengine_mpflow_hash_clear(uid->app_priv);
	kfree(uid->app_priv);
	uid->app_priv = NULL;
}

static void emcom_xengine_mpflow_fi_init(struct emcom_xengine_mpflow_info *mpflow_uid)
{
	memset(&mpflow_uid->wifi, 0, sizeof(struct emcom_xengine_mpflow_iface));
	mpflow_uid->wifi.is_wifi = 1;
	memset(&mpflow_uid->lte, 0, sizeof(struct emcom_xengine_mpflow_iface));
	mpflow_uid->lte.is_wifi = 0;
	INIT_LIST_HEAD(&mpflow_uid->wifi.flows);
	INIT_LIST_HEAD(&mpflow_uid->lte.flows);
	mpflow_uid->rst_bind_mode = EMCOM_XENGINE_MPFLOW_BINDMODE_NONE;
	mpflow_uid->rst_jiffies = 0;
	mpflow_uid->is_downloading = 0;
	mpflow_uid->rst_to_another = 0;
}

static bool emcom_xengine_mpflow_chk_download_flow(struct sk_buff *skb)
{
	int i;
	int index;
	uint16_t port;
	struct tcphdr *tcph = tcp_hdr(skb);
	uint16_t buflen = skb_headlen(skb);
	uint32_t offset = 0;

	port = ntohs(tcph->dest);
	/* download flow must be http(80) or https(443) */
	if (port != EMCOM_MPFLOW_FI_PORT_80 && port != EMCOM_MPFLOW_FI_PORT_443)
		return false;

	spin_lock_bh(&g_mpflow_lock);
	index = emcom_xengine_mpflow_finduid(skb->sk->sk_uid.val);
	if (index != INDEX_INVALID) {
		if (g_mpflow_uids[index].algorithm_type == EMCOM_MPFLOW_ENABLETYPE_WEIBO) {
			spin_unlock_bh(&g_mpflow_lock);
			return true;
		} else if (port == EMCOM_MPFLOW_FI_PORT_80 && g_mpflow_uids[index].ptn_80_num != 0) {
			for (i = 0; i < g_mpflow_uids[index].ptn_80_num; i++) {
				if (emcom_xengine_mpflow_bm_search((const uint8_t *)skb->data, buflen,
					(const struct emcom_xengine_mpflow_ptn *)&(g_mpflow_uids[index].ptn_80[i]),
					&offset)) {
					EMCOM_LOGD("received a port 80 packet match ptn: %s\n",
							   g_mpflow_uids[index].ptn_80[i].ptn);
					spin_unlock_bh(&g_mpflow_lock);
					return true;
				}
			}
		} else if (port == EMCOM_MPFLOW_FI_PORT_443 && g_mpflow_uids[index].ptn_443_num != 0) {
			for (i = 0; i < g_mpflow_uids[index].ptn_443_num; i++) {
				if (emcom_xengine_mpflow_bm_search((const uint8_t *)skb->data, buflen,
					(const struct emcom_xengine_mpflow_ptn *)&(g_mpflow_uids[index].ptn_443[i]),
					&offset)) {
					EMCOM_LOGD("received a port 443 packet match ptn: %s\n",
							   g_mpflow_uids[index].ptn_443[i].ptn);
					spin_unlock_bh(&g_mpflow_lock);
					return true;
				}
			}
		}
	}
	spin_unlock_bh(&g_mpflow_lock);
	return false;
}

static unsigned int emcom_xengine_mpflow_hook_out(void *priv, struct sk_buff *skb,
			const struct nf_hook_state *state)
{
	struct sock *sk = skb->sk;
	struct iphdr *iph = ip_hdr(skb);
	struct tcphdr *tcph = tcp_hdr(skb);

	if (!tcph)
		return NF_ACCEPT;

	if (!sk || !iph || !tcph)
		return NF_ACCEPT;

	if (sk->sk_protocol != IPPROTO_TCP || sk->sk_state != TCP_ESTABLISHED)
		return NF_ACCEPT;

	if (!sk->is_mp_flow || sk->is_download_flow || sk->snd_pkt_cnt > 0)
		return NF_ACCEPT;

	if (skb_headlen(skb) <= (tcp_hdrlen(skb) + ip_hdrlen(skb)))
		return NF_ACCEPT;

	sk->snd_pkt_cnt++;
	if (emcom_xengine_mpflow_chk_download_flow(skb))
		emcom_xengine_mpflow_download_flow_add(sk);
	return NF_ACCEPT;
}

static const struct nf_hook_ops emcom_xengine_mpflow_nfhooks[] = {
	{
		.hook        = emcom_xengine_mpflow_hook_out,
		.pf          = PF_INET,
		.hooknum     = NF_INET_LOCAL_OUT,
		.priority    = NF_IP_PRI_FILTER + 1,
	},
	{
		.hook        = emcom_xengine_mpflow_hook_out,
		.pf          = PF_INET6,
		.hooknum     = NF_INET_LOCAL_OUT,
		.priority    = NF_IP_PRI_FILTER + 1,
	},
};

static void emcom_xengine_mpflow_register_nf_hook(void)
{
	int ret = 0;

	if (g_mpflow_nf_hook)
		return;

	ret = nf_register_net_hooks(&init_net, emcom_xengine_mpflow_nfhooks, ARRAY_SIZE(emcom_xengine_mpflow_nfhooks));
	if (!ret)
		g_mpflow_nf_hook = true;

	EMCOM_LOGD("start emcom_xengine_mpflow_nfhooks\n");
}

static void emcom_xengine_mpflow_unregister_nf_hook(void)
{
	if (!g_mpflow_nf_hook)
		return;

	nf_unregister_net_hooks(&init_net, emcom_xengine_mpflow_nfhooks, ARRAY_SIZE(emcom_xengine_mpflow_nfhooks));
	g_mpflow_nf_hook = false;
	EMCOM_LOGD("stop emcom_xengine_mpflow_nfhooks\n");
}

#ifdef CONFIG_MPTCP
void Emcom_Xengine_MptcpSocketClosed(void *data, int len)
{
	emcom_send_msg2daemon(NETLINK_EMCOM_KD_MPTCP_SOCKET_CLOSED, data, len);
}
EXPORT_SYMBOL(Emcom_Xengine_MptcpSocketClosed);

void Emcom_Xengine_MptcpSocketSwitch(void *data, int len)
{
	emcom_send_msg2daemon(NETLINK_EMCOM_KD_MPTCP_SOCKET_SWITCH, data, len);
}
EXPORT_SYMBOL(Emcom_Xengine_MptcpSocketSwitch);

void Emcom_Xengine_MptcpProxyFallback(void *data, int len)
{
	emcom_send_msg2daemon(NETLINK_EMCOM_KD_MPTCP_PROXY_FALLBACK, data, len);
}
EXPORT_SYMBOL(Emcom_Xengine_MptcpProxyFallback);

void Emcom_Xengine_MptcpFallback(void *data, int len)
{
	emcom_send_msg2daemon(NETLINK_EMCOM_KD_MPTCP_FALLBACK, data, len);
}
EXPORT_SYMBOL(Emcom_Xengine_MptcpFallback);
#endif


void Emcom_Xengine_EvtProc(int32_t event, uint8_t *pdata, uint16_t len)
{
	switch(event)
	{
		case NETLINK_EMCOM_DK_START_ACC:
			EMCOM_LOGD("emcom netlink receive acc start\n");
			Emcom_Xengine_StartAccUid(pdata,len);
			break;
		case NETLINK_EMCOM_DK_STOP_ACC:
			EMCOM_LOGD("emcom netlink receive acc stop\n");
			Emcom_Xengine_StopAccUid(pdata,len);
			break;
		case NETLINK_EMCOM_DK_CLEAR:
			EMCOM_LOGD("emcom netlink receive clear info\n");
			Emcom_Xengine_clear();
			break;
		case NETLINK_EMCOM_DK_RRC_KEEP:
			EMCOM_LOGD("emcom netlink receive rrc keep\n");
			Emcom_Xengine_RrcKeep();
			break;
		case NETLINK_EMCOM_DK_KEY_PSINFO:
			EMCOM_LOGD("emcom netlink receive psinfo\n");
			Emcom_Send_KeyPsInfo(pdata,len);
			break;
		case NETLINK_EMCOM_DK_SPEED_CTRL:
			EMCOM_LOGD("emcom netlink receive speed control uid\n");
			Emcom_Xengine_SetSpeedCtrlInfo(pdata,len);
			break;
		case NETLINK_EMCOM_DK_START_UDP_RETRAN:
			EMCOM_LOGD("emcom netlink receive wifi udp start\n");
			Emcom_Xengine_StartUdpReTran(pdata,len);
			break;
		case NETLINK_EMCOM_DK_STOP_UDP_RETRAN:
			EMCOM_LOGD("emcom netlink receive wifi udp stop\n");
			Emcom_Xengine_StopUdpReTran(pdata,len);
			break;
		case NETLINK_EMCOM_DK_CONFIG_MPIP:
			EMCOM_LOGD("emcom netlink receive btm config start\n");
			Emcom_Xengine_Config_MPIP(pdata,len);
			break;
		case NETLINK_EMCOM_DK_CLEAR_MPIP:
			EMCOM_LOGD("emcom netlink receive clear mpip config\n");
			Emcom_Xengine_Clear_Mpip_Config(pdata,len);
			break;
		case NETLINK_EMCOM_DK_START_MPIP:
			EMCOM_LOGD("emcom netlink receive btm start\n");
			Emcom_Xengine_StartMPIP(pdata,len);
			break;
		case NETLINK_EMCOM_DK_STOP_MPIP:
			EMCOM_LOGD("emcom netlink receive btm stop\n");
			Emcom_Xengine_StopMPIP(pdata,len);
			break;
		case NETLINK_EMCOM_DK_START_FAST_SYN:
			EMCOM_LOGD("emcom netlink receive fast syn start\n");
			Emcom_Xengine_StartFastSyn(pdata, len);
			break;
		case NETLINK_EMCOM_DK_STOP_FAST_SYN:
			EMCOM_LOGD("emcom netlink receive fast syn stop\n");
			Emcom_Xengine_StopFastSyn(pdata, len);
			break;
		case NETLINK_EMCOM_DK_START_MPFLOW:
			EMCOM_LOGD(" emcom netlink start mpflow control algorithm");
			emcom_xengine_mpflow_start(pdata, len);
			break;
		case NETLINK_EMCOM_DK_STOP_MPFLOW:
			EMCOM_LOGD(" emcom netlink stop mpflow control algorithm");
			emcom_xengine_mpflow_stop(pdata, len);
			break;
		default:
			EMCOM_LOGI("emcom Xengine unsupport packet, the type is %d.\n", event);
			break;
	}
}


int Emcom_Xengine_SetProxyUid(struct sock *sk, char __user *optval, int optlen)
{
    uid_t uid = 0;
    int ret = 0;

    ret = -EINVAL;
    if (optlen != sizeof(uid_t))
        goto out;

    ret = -EFAULT;
    if (copy_from_user(&uid, optval, optlen))
        goto out;

    lock_sock(sk);
    sk->sk_uid.val = uid;
    release_sock(sk);
    EMCOM_LOGD("hicom set proxy uid, uid: %u", sk->sk_uid.val);

    ret = 0;

out:

    return ret;
}

int Emcom_Xengine_SetSockFlag(struct sock *sk, char __user *optval, int optlen)
{
    int ret = 0;
    int hicom_flag = 0;

    ret = -EINVAL;
    if (optlen != sizeof(uid_t))
        goto out;

    ret = -EFAULT;
    if (copy_from_user(&hicom_flag, optval, optlen))
        goto out;

    lock_sock(sk);
    sk->hicom_flag = hicom_flag;
    release_sock(sk);

    EMCOM_LOGD(" hicom set proxy flag, uid: %u, flag: %x", sk->sk_uid.val, sk->hicom_flag);

    ret = 0;

out:

    return ret;
}

void Emcom_Xengine_NotifySockError(struct sock *sk)
{
    if (sk->hicom_flag == HICOM_SOCK_FLAG_FINTORST) {
        EMCOM_LOGD(" hicom change fin to rst, uid: %u, flag: %x", sk->sk_uid.val, sk->hicom_flag);
        sk->sk_err = ECONNRESET;
        sk->sk_error_report(sk);
    }

    return;
}

