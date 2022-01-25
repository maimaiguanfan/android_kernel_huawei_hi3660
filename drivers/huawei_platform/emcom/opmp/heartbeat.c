/******************************************************************************
   OpMp heartbeat function
******************************************************************************/

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/tcp.h>
#include <linux/time.h>
#include <linux/timer.h>
#include <linux/types.h>
#include <net/mptcp_heartbeat.h>
#include <net/sock.h>
#include <huawei_platform/emcom/opmp_heartbeat.h>
#include <huawei_platform/log/hw_log.h>

#undef HWLOG_TAG
#define HWLOG_TAG opmp_heartbeat
HWLOG_REGIST();

#define LOGE(msg, ...) \
	do { \
		hwlog_err("[%s:%d %s]: "msg"\n", kbasename(__FILE__), \
			  __LINE__, __func__, ## __VA_ARGS__); \
	} while (0)

#define LOGI(msg, ...) \
	do { \
		hwlog_info("[%s:%d %s]: "msg"\n", kbasename(__FILE__), \
			   __LINE__, __func__, ## __VA_ARGS__); \
	} while (0)

#define LOGD(msg, ...) \
	do { \
		hwlog_debug("[%s:%d %s]: "msg"\n", kbasename(__FILE__), \
			    __LINE__, __func__, ## __VA_ARGS__); \
	} while (0)

#define INVALID_VALUE     (-1);
#define DEFAULT_PERIOD    300;       // in seconds
#define DEFAULT_RETRY_COUNT    3;

static struct sockaddr_in g_link_addrs[LINK_NUM];        // LINK_NUM definied in mptcp_heartbeat.h
static int32_t g_period = DEFAULT_PERIOD;
static int8_t g_retry_count = DEFAULT_RETRY_COUNT;
static bool g_mpheartbeat_enabled = false;
static struct timer_list g_timer;

static void timer_proc(unsigned long arg);
/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
void enable_mpheartbeat(struct sockaddr_in* link_addrs, int32_t period, int8_t retry_count)
{
	if (g_mpheartbeat_enabled)
	{
		LOGE("try to re-enable an enabled mpheartbeat, abort!");
		return;
	}
	if (NULL == link_addrs)
	{
		LOGE("null link_addrs in enable_mpheartbeat(), abort!");
		return;
	}
	/* first record these parameters */
	g_period = period;
	g_retry_count = retry_count;
	memcpy(g_link_addrs, link_addrs, sizeof(g_link_addrs));   // if it is necessary to record addrs and retry count?
	/* init mpheartbeat */
	heartbeat_init(link_addrs, retry_count);
	/* start the timer */
	init_timer(&g_timer);
	g_timer.function= timer_proc;
	add_timer(&g_timer);
	mod_timer(&g_timer, jiffies+(HZ*g_period));
	LOGI("mpheartbeat enabled");
}
EXPORT_SYMBOL(enable_mpheartbeat);

void disable_mpheartbeat(void)
{
	if (!g_mpheartbeat_enabled)
	{
		LOGE("try to disable an NOT enabled mpheartbeat, abort!");
		return;
	}
	/* stop the timer */
	del_timer(&g_timer);
	/* reset the global parameters */
	g_period = DEFAULT_PERIOD;
	g_retry_count = DEFAULT_RETRY_COUNT;
	memset(g_link_addrs, 0, sizeof(g_link_addrs));
	LOGI("mpheartbeat disabled");
}
EXPORT_SYMBOL(disable_mpheartbeat);

static bool check_heartbeat_needed(void)
{
	// read the node and determine whether heartbeat is needed
}

static void opmp_heartbeat_dection_callback(int type, int result)
{
	LOGD("the detection result for link %d is %d", type, result);
}

static void timer_proc(unsigned long arg)
{
	bool trigger = false;
	int detection_type = WIFI_DETECTION + LTE_DETECTION;  // both paths need detection

	mod_timer(&g_timer, jiffies+(HZ*g_period));  // is it safe here?
	trigger = check_heartbeat_needed();
	if (trigger)
	{
		LOGD("mpheartbeat is needed at the moment");
		heartbeat_trigger(detection_type, opmp_heartbeat_dection_callback);
	}
}