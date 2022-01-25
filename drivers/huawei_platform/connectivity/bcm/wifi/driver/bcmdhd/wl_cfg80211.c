/*
 * Linux cfg80211 driver
 *
 * Copyright (C) 1999-2014, Broadcom Corporation
 *
 *      Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed to you
 * under the terms of the GNU General Public License version 2 (the "GPL"),
 * available at http://www.broadcom.com/licenses/GPLv2.php, with the
 * following added to such license:
 *
 *      As a special exception, the copyright holders of this software give you
 * permission to link this software with independent modules, and to copy and
 * distribute the resulting executable under terms of your choice, provided that
 * you also meet, for each linked independent module, the terms and conditions of
 * the license of that module.  An independent module is a module which is not
 * derived from this software.  The special exception does not apply to any
 * modifications of the software.
 *
 *      Notwithstanding the above, under no circumstances may you combine this
 * software in any way with any other Broadcom software provided under a license
 * other than the GPL, without Broadcom's express prior written consent.
 *
 * $Id: wl_cfg80211.c 477711 2014-05-14 08:45:17Z $
 */
/* */
#include <typedefs.h>
#include <linuxver.h>
#include <osl.h>
#include <linux/kernel.h>
#ifdef WL_TEM_CTRL
#include <linux/timer.h>
#include <linux/rtc.h>
#endif

#include <bcmutils.h>
#include <bcmwifi_channels.h>
#include <bcmendian.h>
#include <proto/ethernet.h>
#include <proto/802.11.h>
#include <linux/if_arp.h>
#include <asm/uaccess.h>
#ifndef  BRCM_RSDB
#include <dngl_stats.h>
#include <dhd.h>
#include <dhd_linux.h>
#include <dhdioctl.h>
#include <wlioctl.h>
#include <dhd_cfg80211.h>
#ifdef PNO_SUPPORT
#include <dhd_pno.h>
#endif /* PNO_SUPPORT */
#endif
#include <proto/ethernet.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/netdevice.h>
#include <linux/sched.h>
#include <linux/etherdevice.h>
#include <linux/wireless.h>
#include <linux/ieee80211.h>
#include <linux/wait.h>
#include <net/cfg80211.h>
#include <net/rtnetlink.h>

#if defined WL_TEM_CTRL || defined WL_TIM_EVENT
#include <linux/proc_fs.h>
#endif

#include <wlioctl.h>
#include <wldev_common.h>
#include <wl_cfg80211.h>
#include <wl_cfgp2p.h>
#include <wl_android.h>
#include <dhd_dbg.h>
#include <chipset_common/hwnet/hw_event.h>
#include "hw_wifi.h"
#ifdef  BRCM_RSDB
#include <dngl_stats.h>
#include <dhd.h>
#include <dhd_linux.h>
#include <dhdioctl.h>
#include <wlioctl.h>
#include <dhd_cfg80211.h>
#include <dhd_bus.h>
#ifdef PNO_SUPPORT
#include <dhd_pno.h>
#endif /* PNO_SUPPORT */
#if defined(WL_VENDOR_EXT_SUPPORT)
#include <wl_cfgvendor.h>
#endif /* defined(WL_VENDOR_EXT_SUPPORT) */

#ifdef WL_NAN
#include <wl_cfgnan.h>
#endif /* WL_NAN */
#endif /* BRCM_RSDB */

#ifdef PROP_TXSTATUS
#include <dhd_wlfc.h>
#endif
#ifndef  BRCM_RSDB
#if (LINUX_VERSION_CODE > KERNEL_VERSION(3, 13, 0)) || defined(WL_VENDOR_EXT_SUPPORT)
#include <wl_cfgvendor.h>
#endif /* (LINUX_VERSION_CODE > KERNEL_VERSION(3, 13, 0)) || defined(WL_VENDOR_EXT_SUPPORT) */
#else
#ifdef BCMPCIE
#include <dhd_flowring.h>
#endif
#endif  /* BRCM_RSDB */
#include <dhd_debug.h>
#ifdef WL11U
#if !defined(WL_ENABLE_P2P_IF) && !defined(WL_CFG80211_P2P_DEV_IF)
#error You should enable 'WL_ENABLE_P2P_IF' or 'WL_CFG80211_P2P_DEV_IF' \
	according to Kernel version and is supported only in Android-JB
#endif /* !WL_ENABLE_P2P_IF && !WL_CFG80211_P2P_DEV_IF */
#endif /* WL11U */

#ifdef BCMWAPI_WPI
/* these items should evetually go into wireless.h of the linux system headfile dir */
#ifndef IW_ENCODE_ALG_SM4
#define IW_ENCODE_ALG_SM4 0x20
#endif

#ifndef IW_AUTH_WAPI_ENABLED
#define IW_AUTH_WAPI_ENABLED 0x20
#endif

#ifndef IW_AUTH_WAPI_VERSION_1
#define IW_AUTH_WAPI_VERSION_1  0x00000008
#endif

#ifndef IW_AUTH_CIPHER_SMS4
#define IW_AUTH_CIPHER_SMS4     0x00000020
#endif

#ifndef IW_AUTH_KEY_MGMT_WAPI_PSK
#define IW_AUTH_KEY_MGMT_WAPI_PSK 4
#endif

#ifndef IW_AUTH_KEY_MGMT_WAPI_CERT
#define IW_AUTH_KEY_MGMT_WAPI_CERT 8
#endif
#endif /* BCMWAPI_WPI */

#ifdef BCMWAPI_WPI
#define IW_WSEC_ENABLED(wsec)   ((wsec) & (WEP_ENABLED | TKIP_ENABLED | AES_ENABLED | SMS4_ENABLED))
#else /* BCMWAPI_WPI */
#define IW_WSEC_ENABLED(wsec)   ((wsec) & (WEP_ENABLED | TKIP_ENABLED | AES_ENABLED))
#endif /* BCMWAPI_WPI */

#if defined(WLFBT)
#ifndef WLAN_AKM_SUITE_FT_PSK
#define WLAN_AKM_SUITE_FT_PSK           0x000FAC04
#endif

#ifndef WLAN_AKM_SUITE_FT_8021X
#define WLAN_AKM_SUITE_FT_8021X		0x000FAC03
#endif

#ifndef WLAN_CIPHER_SUITE_PMK
#define WLAN_CIPHER_SUITE_PMK		0x00904C00
#endif
#endif

static struct device *cfg80211_parent_dev = NULL;
/* g_bcm_cfg should be static. Do not change */
static struct bcm_cfg80211 *g_bcm_cfg = NULL;
#ifdef CUSTOMER_HW4_DEBUG
u32 wl_dbg_level = WL_DBG_ERR | WL_DBG_P2P_ACTION | WL_DBG_INFO;
#else
u32 wl_dbg_level = WL_DBG_ERR;
#endif /* CUSTOMER_HW4_DEBUG */
#ifdef HW_SET_PM
extern uint32 dhd_power_lock_status;
#endif

#ifdef HW_SCAN_TRACE
u32 g_scan_trace = 1;
#endif

s32 g_pre_chain = 0;
#ifdef HW_P2PGO_2G_SISO
s32 g_p2pgo_siso = 0;
#endif
/* cfg80211 suspend flag */
#ifdef HW_WIFI_SUSPEND_ISSUE
volatile bool hw_cfg80211_suspend = FALSE;
#endif

#if defined(DHD_DEBUG) && defined(BCM_PATCH_DEBUG_BEACON_LOSS)
volatile bool fw_log_open_by_beacon_loss = FALSE;
#endif

#ifdef SCAN_TIMEOUT_FIX
#define MAX_SCANTIMEOUT_COUNT 2
static int	scan_timeout_counter = 0;
#endif
#define MAX_WAIT_TIME 1500
#ifdef HW_WIFI_FIRST_SCAN_OPTIMIZE
extern bool g_dhd_init_flag;
#endif
#ifndef  BRCM_RSDB
#define CHAN_INFO_LEN 128
#define IBSS_IF_NAME "ibss%d"
#else
#ifdef WLAIBSS_MCHAN
#define IBSS_IF_NAME "ibss%d"
#endif /* WLAIBSS_MCHAN */
#endif
#ifdef VSDB
/* sleep time to keep STA's connecting or connection for continuous af tx or finding a peer */
#define DEFAULT_SLEEP_TIME_VSDB		120
#define OFF_CHAN_TIME_THRESHOLD_MS	200
#define AF_RETRY_DELAY_TIME			40

/* if sta is connected or connecting, sleep for a while before retry af tx or finding a peer */
#define WL_AF_TX_KEEP_PRI_CONNECTION_VSDB(cfg)	\
	do {	\
		if (wl_get_drv_status(cfg, CONNECTED, bcmcfg_to_prmry_ndev(cfg)) ||	\
			wl_get_drv_status(cfg, CONNECTING, bcmcfg_to_prmry_ndev(cfg))) {	\
			OSL_SLEEP(DEFAULT_SLEEP_TIME_VSDB);			\
		}	\
	} while (0)
#else /* VSDB */
/* if not VSDB, do nothing */
#define WL_AF_TX_KEEP_PRI_CONNECTION_VSDB(cfg)
#endif /* VSDB */

#ifdef WL_CFG80211_SYNC_GON
#define WL_DRV_STATUS_SENDING_AF_FRM_EXT(cfg) \
	(wl_get_drv_status_all(cfg, SENDING_ACT_FRM) || \
		wl_get_drv_status_all(cfg, WAITING_NEXT_ACT_FRM_LISTEN))
#else
#define WL_DRV_STATUS_SENDING_AF_FRM_EXT(cfg) wl_get_drv_status_all(cfg, SENDING_ACT_FRM)
#endif /* WL_CFG80211_SYNC_GON */
#ifndef  BRCM_RSDB
#define WL_IS_P2P_DEV_EVENT(e) ((e->emsg.ifidx == 0) && \
		(e->emsg.bsscfgidx == P2PAPI_BSSCFG_DEVICE))
#endif
#define DNGL_FUNC(func, parameters) func parameters
#define COEX_DHCP

#define WLAN_EID_SSID	0
#define CH_MIN_5G_CHANNEL 34
#define CH_MIN_2G_CHANNEL 1
#ifdef  BRCM_RSDB
#define ACTIVE_SCAN 1
#define PASSIVE_SCAN 0
#ifdef WLAIBSS
enum abiss_event_type {
	AIBSS_EVENT_TXFAIL
};
#endif

#if defined(STRICT_GCC_WARNINGS) && defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == \
	4 && __GNUC_MINOR__ >= 6))
#define BCM_SET_LIST_FIRST_ENTRY(entry, ptr, type, member) \
_Pragma("GCC diagnostic push") \
_Pragma("GCC diagnostic ignored \"-Wcast-qual\"") \
(entry) = list_first_entry((ptr), type, member); \
_Pragma("GCC diagnostic pop") \

#define BCM_SET_CONTAINER_OF(entry, ptr, type, member) \
_Pragma("GCC diagnostic push") \
_Pragma("GCC diagnostic ignored \"-Wcast-qual\"") \
entry = container_of((ptr), type, member); \
_Pragma("GCC diagnostic pop") \

#else
#define BCM_SET_LIST_FIRST_ENTRY(entry, ptr, type, member) \
(entry) = list_first_entry((ptr), type, member); \

#define BCM_SET_CONTAINER_OF(entry, ptr, type, member) \
entry = container_of((ptr), type, member); \

#endif /* STRICT_GCC_WARNINGS */

enum rmc_event_type {
	RMC_EVENT_NONE,
	RMC_EVENT_LEADER_CHECK_FAIL
};
#endif
/* This is to override regulatory domains defined in cfg80211 module (reg.c)
 * By default world regulatory domain defined in reg.c puts the flags NL80211_RRF_PASSIVE_SCAN
 * and NL80211_RRF_NO_IBSS for 5GHz channels (for 36..48 and 149..165).
 * With respect to these flags, wpa_supplicant doesn't start p2p operations on 5GHz channels.
 * All the chnages in world regulatory domain are to be done here.
 */
#ifdef  BRCM_RSDB
#if defined(STRICT_GCC_WARNINGS) && defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == \
	4 && __GNUC_MINOR__ >= 6))
_Pragma("GCC diagnostic push")
_Pragma("GCC diagnostic ignored \"-Wmissing-field-initializers\"")
#endif
#endif
static const struct ieee80211_regdomain brcm_regdom = {
	.n_reg_rules = 4,
	.alpha2 =  "99",
	.reg_rules = {
		/* IEEE 802.11b/g, channels 1..11 */
		REG_RULE(2412-10, 2472+10, 40, 6, 20, 0),
		/* If any */
		/* IEEE 802.11 channel 14 - Only JP enables
		 * this and for 802.11b only
		 */
		REG_RULE(2484-10, 2484+10, 20, 6, 20, 0),
		/* IEEE 802.11a, channel 36..64 */
		REG_RULE(5150-10, 5350+10, 40, 6, 20, 0),
		/* IEEE 802.11a, channel 100..165 */
		REG_RULE(5470-10, 5850+10, 40, 6, 20, 0), }
};
#ifdef  BRCM_RSDB
#if defined(STRICT_GCC_WARNINGS) && defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == \
	4 && __GNUC_MINOR__ >= 6))
_Pragma("GCC diagnostic pop")
#endif
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 0, 0)) && \
	(defined(WL_IFACE_COMB_NUM_CHANNELS) || defined(WL_CFG80211_P2P_DEV_IF))
/*
 * Possible interface combinations supported by driver
 *
 * ADHOC Mode     - #ADHOC <= 1 on channels = 1
 * SoftAP Mode    - #AP <= 1 on channels = 1
 * STA + P2P Mode - #STA <= 2, #{P2P-GO, P2P-client} <= 1, #P2P-device <= 1
 *                  on channels = 2
 */
static const struct ieee80211_iface_limit common_if_limits[] = {
	{
#ifndef  BRCM_RSDB
	.max = 1,
#else
	.max = 2,
#endif
	.types = BIT(NL80211_IFTYPE_AP),
	},
	{
	/*
	 * During P2P-GO removal, P2P-GO is first changed to STA and later only
	 * removed. So setting maximum possible number of STA interfaces according
	 * to kernel version.
	 *
	 * less than linux-3.8 - max:3 (wlan0 + p2p0 + group removal of p2p-p2p0-x)
	 * linux-3.8 and above - max:2 (wlan0 + group removal of p2p-wlan0-x)
	 */
#ifdef WL_ENABLE_P2P_IF
	.max = 3,
#else
	.max = 2,
#endif /* WL_ENABLE_P2P_IF */
	.types = BIT(NL80211_IFTYPE_STATION),
	},
	{
	.max = 2,
	.types = BIT(NL80211_IFTYPE_P2P_GO) | BIT(NL80211_IFTYPE_P2P_CLIENT),
	},
#if defined(WL_CFG80211_P2P_DEV_IF)
	{
	.max = 1,
	.types = BIT(NL80211_IFTYPE_P2P_DEVICE),
	},
#endif /* WL_CFG80211_P2P_DEV_IF */
	{
	.max = 1,
	.types = BIT(NL80211_IFTYPE_ADHOC),
	},
};
#ifdef BCM4330_CHIP
#define NUM_DIFF_CHANNELS 1
#else
#define NUM_DIFF_CHANNELS 2
#endif
static const struct ieee80211_iface_combination
common_iface_combinations[] = {
	{
	.num_different_channels = NUM_DIFF_CHANNELS,
	/*
	 * max_interfaces = 4
	 * The max no of interfaces will be used in dual p2p case.
	 * {STA, P2P Device, P2P Group 1, P2P Group 2}. Though we
	 * will not be using the STA functionality in this case, it
	 * will remain registered as it is the primary interface.
	 */
	.max_interfaces = 4,
	.limits = common_if_limits,
	.n_limits = ARRAY_SIZE(common_if_limits),
	},
};
#endif /* LINUX_VER >= 3.0 && (WL_IFACE_COMB_NUM_CHANNELS || WL_CFG80211_P2P_DEV_IF) */

/* Data Element Definitions */
#define WPS_ID_CONFIG_METHODS     0x1008
#define WPS_ID_REQ_TYPE           0x103A
#define WPS_ID_DEVICE_NAME        0x1011
#define WPS_ID_VERSION            0x104A
#define WPS_ID_DEVICE_PWD_ID      0x1012
#define WPS_ID_REQ_DEV_TYPE       0x106A
#define WPS_ID_SELECTED_REGISTRAR_CONFIG_METHODS 0x1053
#define WPS_ID_PRIM_DEV_TYPE      0x1054

/* Device Password ID */
#define DEV_PW_DEFAULT 0x0000
#define DEV_PW_USER_SPECIFIED 0x0001,
#define DEV_PW_MACHINE_SPECIFIED 0x0002
#define DEV_PW_REKEY 0x0003
#define DEV_PW_PUSHBUTTON 0x0004
#define DEV_PW_REGISTRAR_SPECIFIED 0x0005

/* Config Methods */
#define WPS_CONFIG_USBA 0x0001
#define WPS_CONFIG_ETHERNET 0x0002
#define WPS_CONFIG_LABEL 0x0004
#define WPS_CONFIG_DISPLAY 0x0008
#define WPS_CONFIG_EXT_NFC_TOKEN 0x0010
#define WPS_CONFIG_INT_NFC_TOKEN 0x0020
#define WPS_CONFIG_NFC_INTERFACE 0x0040
#define WPS_CONFIG_PUSHBUTTON 0x0080
#define WPS_CONFIG_KEYPAD 0x0100
#define WPS_CONFIG_VIRT_PUSHBUTTON 0x0280
#define WPS_CONFIG_PHY_PUSHBUTTON 0x0480
#define WPS_CONFIG_VIRT_DISPLAY 0x2008
#define WPS_CONFIG_PHY_DISPLAY 0x4008

#define PM_BLOCK 1
#define PM_ENABLE 0
#ifndef  BRCM_RSDB
#ifdef MFP
#define WL_AKM_SUITE_MFP_1X  0x000FAC05
#define WL_AKM_SUITE_MFP_PSK 0x000FAC06
#endif /* MFP */
#else
#ifdef BCMCCX
#ifndef WLAN_AKM_SUITE_CCKM
#define WLAN_AKM_SUITE_CCKM 0x00409600
#endif
#define DOT11_LEAP_AUTH	0x80 /* LEAP auth frame paylod constants */
#endif /* BCMCCX */

#define WL_AKM_SUITE_SHA256_1X  0x000FAC05
#define WL_AKM_SUITE_SHA256_PSK 0x000FAC06
#endif


#ifndef IBSS_COALESCE_ALLOWED
#define IBSS_COALESCE_ALLOWED 0
#endif

#ifndef IBSS_INITIAL_SCAN_ALLOWED
#define IBSS_INITIAL_SCAN_ALLOWED 0
#endif

#define CUSTOM_RETRY_MASK 0xff000000 /* Mask for retry counter of custom dwell time */
#ifdef  BRCM_RSDB
#define LONG_LISTEN_TIME 2000

#define MAX_SCAN_ABORT_WAIT_CNT 20
#define WAIT_SCAN_ABORT_OSL_SLEEP_TIME 10

#define IDSUP_4WAY_HANDSHAKE_TIMEOUT	10000
enum idsup_event_type {
	IDSUP_EVENT_SUCCESS = 0,
	IDSUP_EVENT_4WAY_HANDSHAKE_TIMEOUT
};
#endif
/*
 * cfg80211_ops api/callback list
 */
static s32 wl_frame_get_mgmt(u16 fc, const struct ether_addr *da,
	const struct ether_addr *sa, const struct ether_addr *bssid,
	u8 **pheader, u32 *body_len, u8 *pbody);
static s32 __wl_cfg80211_scan(struct wiphy *wiphy, struct net_device *ndev,
	struct cfg80211_scan_request *request,
	struct cfg80211_ssid *this_ssid);
#if defined(WL_CFG80211_P2P_DEV_IF)
static s32
wl_cfg80211_scan(struct wiphy *wiphy, struct cfg80211_scan_request *request);
#else
static s32
wl_cfg80211_scan(struct wiphy *wiphy, struct net_device *ndev,
	struct cfg80211_scan_request *request);
#endif /* WL_CFG80211_P2P_DEV_IF */
static s32 wl_cfg80211_set_wiphy_params(struct wiphy *wiphy, u32 changed);
#if defined(WLAIBSS_MCHAN) || !defined(BRCM_RSDB)
static bcm_struct_cfgdev* bcm_cfg80211_add_ibss_if(struct wiphy *wiphy, char *name);
static s32 bcm_cfg80211_del_ibss_if(struct wiphy *wiphy, bcm_struct_cfgdev *cfgdev);
#endif /* WLAIBSS_MCHAN or BRCM_RSDB */
static s32 wl_cfg80211_join_ibss(struct wiphy *wiphy, struct net_device *dev,
	struct cfg80211_ibss_params *params);
static s32 wl_cfg80211_leave_ibss(struct wiphy *wiphy,
	struct net_device *dev);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0))
static s32 wl_cfg80211_get_station(struct wiphy *wiphy,
	struct net_device *dev, const u8 *mac,
	struct station_info *sinfo);
#else
static s32 wl_cfg80211_get_station(struct wiphy *wiphy,
	struct net_device *dev, u8 *mac,
	struct station_info *sinfo);
#endif
static s32 wl_cfg80211_set_power_mgmt(struct wiphy *wiphy,
	struct net_device *dev, bool enabled,
	s32 timeout);
static int wl_cfg80211_connect(struct wiphy *wiphy, struct net_device *dev,
	struct cfg80211_connect_params *sme);
static s32 wl_cfg80211_disconnect(struct wiphy *wiphy, struct net_device *dev,
	u16 reason_code);
#if defined(WL_CFG80211_P2P_DEV_IF)
static s32
wl_cfg80211_set_tx_power(struct wiphy *wiphy, struct wireless_dev *wdev,
	enum nl80211_tx_power_setting type, s32 mbm);
#else
static s32
wl_cfg80211_set_tx_power(struct wiphy *wiphy,
	enum nl80211_tx_power_setting type, s32 dbm);
#endif /* WL_CFG80211_P2P_DEV_IF */
#if defined(WL_CFG80211_P2P_DEV_IF)
static s32 wl_cfg80211_get_tx_power(struct wiphy *wiphy,
	struct wireless_dev *wdev, s32 *dbm);
#else
static s32 wl_cfg80211_get_tx_power(struct wiphy *wiphy, s32 *dbm);
#endif /* WL_CFG80211_P2P_DEV_IF */
static s32 wl_cfg80211_config_default_key(struct wiphy *wiphy,
	struct net_device *dev,
	u8 key_idx, bool unicast, bool multicast);
static s32 wl_cfg80211_add_key(struct wiphy *wiphy, struct net_device *dev,
	u8 key_idx, bool pairwise, const u8 *mac_addr,
	struct key_params *params);
static s32 wl_cfg80211_del_key(struct wiphy *wiphy, struct net_device *dev,
	u8 key_idx, bool pairwise, const u8 *mac_addr);
static s32 wl_cfg80211_get_key(struct wiphy *wiphy, struct net_device *dev,
	u8 key_idx, bool pairwise, const u8 *mac_addr,
	void *cookie, void (*callback) (void *cookie,
	struct key_params *params));
static s32 wl_cfg80211_config_default_mgmt_key(struct wiphy *wiphy,
	struct net_device *dev,	u8 key_idx);
static s32 wl_cfg80211_resume(struct wiphy *wiphy);
#if defined(WL_SUPPORT_BACKPORTED_KPATCHES) || (LINUX_VERSION_CODE >= KERNEL_VERSION(3, \
	2, 0))
static s32 wl_cfg80211_mgmt_tx_cancel_wait(struct wiphy *wiphy,
	bcm_struct_cfgdev *cfgdev, u64 cookie);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 19, 0))
static s32 wl_cfg80211_del_station(
		struct wiphy *wiphy, struct net_device *ndev,
		struct station_del_parameters *params);
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0))
static s32 wl_cfg80211_del_station(struct wiphy *wiphy,
	struct net_device *ndev, const u8* mac_addr);
#else
static s32 wl_cfg80211_del_station(struct wiphy *wiphy,
	struct net_device *ndev, u8* mac_addr);
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0))
static s32 wl_cfg80211_change_station(struct wiphy *wiphy,
	struct net_device *dev, const u8 *mac, struct station_parameters *params);
#else
static s32 wl_cfg80211_change_station(struct wiphy *wiphy,
	struct net_device *dev, u8 *mac, struct station_parameters *params);
#endif
#endif /* WL_SUPPORT_BACKPORTED_KPATCHES || KERNEL_VER >= KERNEL_VERSION(3, 2, 0)) */
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 39)) || defined(WL_COMPAT_WIRELESS)
static s32 wl_cfg80211_suspend(struct wiphy *wiphy, struct cfg80211_wowlan *wow);
#else
static s32 wl_cfg80211_suspend(struct wiphy *wiphy);
#endif /* KERNEL_VERSION(2, 6, 39) || WL_COMPAT_WIRELES */
static s32 wl_cfg80211_set_pmksa(struct wiphy *wiphy, struct net_device *dev,
	struct cfg80211_pmksa *pmksa);
static s32 wl_cfg80211_del_pmksa(struct wiphy *wiphy, struct net_device *dev,
	struct cfg80211_pmksa *pmksa);
static s32 wl_cfg80211_flush_pmksa(struct wiphy *wiphy,
	struct net_device *dev);
static void wl_cfg80211_scan_abort(struct bcm_cfg80211 *cfg);
#ifdef  BRCM_RSDB
static void wl_cfg80211_cancel_scan(struct bcm_cfg80211 *cfg);
#endif
static s32 wl_notify_escan_complete(struct bcm_cfg80211 *cfg,
	struct net_device *ndev, bool aborted, bool fw_abort);
#ifdef BRCM_RSDB
#if (LINUX_VERSION_CODE > KERNEL_VERSION(3, 2, 0)) || defined(WL_COMPAT_WIRELESS)
#if (defined(CONFIG_ARCH_MSM) && defined(TDLS_MGMT_VERSION2)) || (LINUX_VERSION_CODE < \
	KERNEL_VERSION(3, 16, 0) && LINUX_VERSION_CODE >= KERNEL_VERSION(3, 15, 0))
static s32 wl_cfg80211_tdls_mgmt(struct wiphy *wiphy, struct net_device *dev,
	u8 *peer, u8 action_code, u8 dialog_token, u16 status_code,
	u32 peer_capability, const u8 *data, size_t len);
#elif ((LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0)) && \
		(LINUX_VERSION_CODE < KERNEL_VERSION(3, 18, 0)))
static s32 wl_cfg80211_tdls_mgmt(struct wiphy *wiphy, struct net_device *dev,
	const u8 *peer, u8 action_code, u8 dialog_token, u16 status_code,
	u32 peer_capability, const u8 *data, size_t len);
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 18, 0))
static s32 wl_cfg80211_tdls_mgmt(struct wiphy *wiphy, struct net_device *dev,
	const u8 *peer, u8 action_code, u8 dialog_token, u16 status_code,
	u32 peer_capability, bool initiator, const u8 *data, size_t len);
#else
static s32 wl_cfg80211_tdls_mgmt(struct wiphy *wiphy, struct net_device *dev,
	u8 *peer, u8 action_code, u8 dialog_token, u16 status_code, const u8 *data,
	size_t len);
#endif /* CONFIG_ARCH_MSM && TDLS_MGMT_VERSION2 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0))
static s32 wl_cfg80211_tdls_oper(struct wiphy *wiphy, struct net_device *dev,
	const u8 *peer, enum nl80211_tdls_operation oper);
#else
static s32 wl_cfg80211_tdls_oper(struct wiphy *wiphy, struct net_device *dev,
	u8 *peer, enum nl80211_tdls_operation oper);
#endif
#endif /* LINUX_VERSION > KERNEL_VERSION(3,2,0) || WL_COMPAT_WIRELESS */
#else  /* BRCM_RSDB */
#if (LINUX_VERSION_CODE > KERNEL_VERSION(3, 2, 0))
static s32 wl_cfg80211_tdls_oper(struct wiphy *wiphy, struct net_device *dev,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0))
	const
#endif
	u8 *peer, enum nl80211_tdls_operation oper);
#endif
#endif /* BRCM_RSDB */
#ifdef WL_SCHED_SCAN
static int wl_cfg80211_sched_scan_stop(struct wiphy *wiphy, struct net_device *dev);
#endif

#if defined(WL_VIRTUAL_APSTA) || defined(DUAL_STA_STATIC_IF)
bcm_struct_cfgdev*
wl_cfg80211_create_iface(struct wiphy *wiphy, enum nl80211_iftype
		 iface_type, u8 *mac_addr, const char *name);
s32
wl_cfg80211_del_iface(struct wiphy *wiphy, bcm_struct_cfgdev *cfgdev);
#endif /* defined(WL_VIRTUAL_APSTA) || defined(DUAL_STA_STATIC_IF) */
#ifdef  BRCM_RSDB
s32 wl_cfg80211_interface_ops(struct bcm_cfg80211 *cfg,
	struct net_device *ndev, s32 bsscfg_idx,
	enum nl80211_iftype iface_type, s32 del, u8 *addr);
s32 wl_cfg80211_add_del_bss(struct bcm_cfg80211 *cfg,
	struct net_device *ndev, s32 bsscfg_idx,
	enum nl80211_iftype iface_type, s32 del, u8 *addr);
chanspec_t wl_chspec_driver_to_host(chanspec_t chanspec);
chanspec_t wl_chspec_host_to_driver(chanspec_t chanspec);
#ifdef WL11ULB
static s32 wl_cfg80211_get_ulb_bw(struct wireless_dev *wdev);
static chanspec_t wl_cfg80211_ulb_get_min_bw_chspec(struct wireless_dev *wdev, s32 bssidx);
static s32 wl_cfg80211_ulbbw_to_ulbchspec(u32 ulb_bw);
#else
static inline chanspec_t wl_cfg80211_ulb_get_min_bw_chspec(
		struct wireless_dev *wdev, s32 bssidx)
{
	return WL_CHANSPEC_BW_20;
}
#endif /* WL11ULB */
#endif /* BRCM_RSDB */
/*
 * event & event Q handlers for cfg80211 interfaces
 */
static s32 wl_create_event_handler(struct bcm_cfg80211 *cfg);
static void wl_destroy_event_handler(struct bcm_cfg80211 *cfg);
static s32 wl_event_handler(void *data);
static void wl_init_eq(struct bcm_cfg80211 *cfg);
static void wl_flush_eq(struct bcm_cfg80211 *cfg);
static unsigned long wl_lock_eq(struct bcm_cfg80211 *cfg);
static void wl_unlock_eq(struct bcm_cfg80211 *cfg, unsigned long flags);
static void wl_init_eq_lock(struct bcm_cfg80211 *cfg);
static void wl_init_event_handler(struct bcm_cfg80211 *cfg);
static struct wl_event_q *wl_deq_event(struct bcm_cfg80211 *cfg);
static s32 wl_enq_event(struct bcm_cfg80211 *cfg, struct net_device *ndev, u32 type,
	const wl_event_msg_t *msg, void *data);
static void wl_put_event(struct wl_event_q *e);
static void wl_wakeup_event(struct bcm_cfg80211 *cfg);
static s32 wl_notify_connect_status_ap(struct bcm_cfg80211 *cfg, struct net_device *ndev,
	const wl_event_msg_t *e, void *data);
static s32 wl_notify_connect_status(struct bcm_cfg80211 *cfg,
	bcm_struct_cfgdev *cfgdev, const wl_event_msg_t *e, void *data);
static s32 wl_notify_roaming_status(struct bcm_cfg80211 *cfg,
	bcm_struct_cfgdev *cfgdev, const wl_event_msg_t *e, void *data);
static s32 wl_notify_scan_status(struct bcm_cfg80211 *cfg, bcm_struct_cfgdev *cfgdev,
	const wl_event_msg_t *e, void *data);
static s32 wl_bss_connect_done(struct bcm_cfg80211 *cfg, struct net_device *ndev,
	const wl_event_msg_t *e, void *data, bool completed);
static s32 wl_bss_roaming_done(struct bcm_cfg80211 *cfg, struct net_device *ndev,
	const wl_event_msg_t *e, void *data);
static s32 wl_notify_mic_status(struct bcm_cfg80211 *cfg, bcm_struct_cfgdev *cfgdev,
	const wl_event_msg_t *e, void *data);
#ifdef BT_WIFI_HANDOVER
static s32 wl_notify_bt_wifi_handover_req(struct bcm_cfg80211 *cfg,
	bcm_struct_cfgdev *cfgdev, const wl_event_msg_t *e, void *data);
#endif /* BT_WIFI_HANDOVER */
#ifdef WL_SCHED_SCAN
static s32
wl_notify_sched_scan_results(struct bcm_cfg80211 *cfg, struct net_device *ndev,
	const wl_event_msg_t *e, void *data);
#endif /* WL_SCHED_SCAN */
#ifdef PNO_SUPPORT
static s32 wl_notify_pfn_status(struct bcm_cfg80211 *cfg, bcm_struct_cfgdev *cfgdev,
	const wl_event_msg_t *e, void *data);
#endif /* PNO_SUPPORT */
#ifdef GSCAN_SUPPORT
static s32 wl_notify_gscan_event(struct bcm_cfg80211 *wl, bcm_struct_cfgdev *cfgdev,
	const wl_event_msg_t *e, void *data);
#ifndef  BRCM_RSDB
static s32 wl_handle_roam_exp_event(struct bcm_cfg80211 *wl, bcm_struct_cfgdev *cfgdev,
	const wl_event_msg_t *e, void *data);
#endif
#endif /* GSCAN_SUPPORT */
#ifndef  BRCM_RSDB
static s32 wl_handle_rssi_monitor_event(struct bcm_cfg80211 *wl, bcm_struct_cfgdev *cfgdev,
	const wl_event_msg_t *e, void *data);
#endif
static s32 wl_notifier_change_state(struct bcm_cfg80211 *cfg, struct net_info *_net_info,
	enum wl_status state, bool set);
#ifdef  BRCM_RSDB
#ifdef DHD_LOSSLESS_ROAMING
static s32 wl_notify_roam_prep_status(struct bcm_cfg80211 *cfg,
	bcm_struct_cfgdev *cfgdev, const wl_event_msg_t *e, void *data);
static void wl_del_roam_timeout(struct bcm_cfg80211 *cfg);
#endif /* DHD_LOSSLESS_ROAMING */
#ifdef CUSTOM_EVENT_PM_WAKE
static s32 wl_check_pmstatus(struct bcm_cfg80211 *cfg, bcm_struct_cfgdev *cfgdev,
	const wl_event_msg_t *e, void *data);
#endif /* CUSTOM_EVENT_PM_WAKE */
#endif
#ifdef WLTDLS
static s32 wl_tdls_event_handler(struct bcm_cfg80211 *cfg, bcm_struct_cfgdev *cfgdev,
	const wl_event_msg_t *e, void *data);
#endif /* WLTDLS */

#ifdef WRONG_ACTION_PATCH
static s32 wl_prune_event_handler(struct bcm_cfg80211 *wl, bcm_struct_cfgdev *cfgdev,
	const wl_event_msg_t *e, void *data);

static s32 wl_check_assoc_status(struct bcm_cfg80211 *wl, struct net_device *ndev,
	const wl_event_msg_t *e);	

static s32 wl_clear_wrong_action_handle(struct bcm_cfg80211 *wl, struct net_device *ndev, wl_extjoin_params_t *ext_join_params);
#endif

/*
 * register/deregister parent device
 */
static void wl_cfg80211_clear_parent_dev(void);

/*
 * ioctl utilites
 */

/*
 * cfg80211 set_wiphy_params utilities
 */
static s32 wl_set_frag(struct net_device *dev, u32 frag_threshold);
static s32 wl_set_rts(struct net_device *dev, u32 frag_threshold);
static s32 wl_set_retry(struct net_device *dev, u32 retry, bool l);

/*
 * cfg profile utilities
 */
static s32 wl_update_prof(struct bcm_cfg80211 *cfg, struct net_device *ndev,
#ifndef  BRCM_RSDB
	const wl_event_msg_t *e, void *data, s32 item);
#else
	const wl_event_msg_t *e, const void *data, s32 item);
#endif
static void *wl_read_prof(struct bcm_cfg80211 *cfg, struct net_device *ndev, s32 item);
static void wl_init_prof(struct bcm_cfg80211 *cfg, struct net_device *ndev);

/*
 * cfg80211 connect utilites
 */
static s32 wl_set_wpa_version(struct net_device *dev,
	struct cfg80211_connect_params *sme);
static s32 wl_set_auth_type(struct net_device *dev,
	struct cfg80211_connect_params *sme);
static s32 wl_set_set_cipher(struct net_device *dev,
	struct cfg80211_connect_params *sme);
static s32 wl_set_key_mgmt(struct net_device *dev,
	struct cfg80211_connect_params *sme);
static s32 wl_set_set_sharedkey(struct net_device *dev,
	struct cfg80211_connect_params *sme);
#ifdef BCMWAPI_WPI
static s32 wl_set_set_wapi_ie(struct net_device *dev,
        struct cfg80211_connect_params *sme);
#endif
static s32 wl_get_assoc_ies(struct bcm_cfg80211 *cfg, struct net_device *ndev);
#ifndef  BRCM_RSDB
static void wl_ch_to_chanspec(int ch,
	struct wl_join_params *join_params, size_t *join_params_size);
#else
static s32 wl_ch_to_chanspec(struct net_device *dev, int ch,
	struct wl_join_params *join_params, size_t *join_params_size);
void wl_cfg80211_clear_security(struct bcm_cfg80211 *cfg);
#endif

/*
 * information element utilities
 */
static void wl_rst_ie(struct bcm_cfg80211 *cfg);
static __used s32 wl_add_ie(struct bcm_cfg80211 *cfg, u8 t, u8 l, u8 *v);
#ifndef  BRCM_RSDB
static void wl_update_hidden_ap_ie(struct wl_bss_info *bi, u8 *ie_stream, u32 *ie_size, bool roam);
#else
static void wl_update_hidden_ap_ie(struct wl_bss_info *bi, const u8 *ie_stream, u32 *ie_size,
	bool roam);
#endif
static s32 wl_mrg_ie(struct bcm_cfg80211 *cfg, u8 *ie_stream, u16 ie_size);
static s32 wl_cp_ie(struct bcm_cfg80211 *cfg, u8 *dst, u16 dst_size);
static u32 wl_get_ielen(struct bcm_cfg80211 *cfg);
#ifdef MFP
static int wl_cfg80211_get_rsn_capa(bcm_tlv_t *wpa2ie, u8* capa);
#endif

#ifdef WL11U
bcm_tlv_t *
wl_cfg80211_find_interworking_ie(u8 *parse, u32 len);
static s32
wl_cfg80211_add_iw_ie(struct bcm_cfg80211 *cfg, struct net_device *ndev, s32 bssidx, s32 pktflag,
            uint8 ie_id, uint8 *data, uint8 data_len);
#endif /* WL11U */

static s32 wl_setup_wiphy(struct wireless_dev *wdev, struct device *dev, void *data);
static void wl_free_wdev(struct bcm_cfg80211 *cfg);
#ifndef  BRCM_RSDB
#ifdef CONFIG_CFG80211_INTERNAL_REGDB
static void
wl_cfg80211_reg_notifier(struct wiphy *wiphy, struct regulatory_request *request);
#endif /* CONFIG_CFG80211_INTERNAL_REGDB */
#endif /* BRCM_RSDB */
static s32 wl_inform_bss(struct bcm_cfg80211 *cfg);
static s32 wl_inform_single_bss(struct bcm_cfg80211 *cfg, struct wl_bss_info *bi, bool roam);
static s32 wl_update_bss_info(struct bcm_cfg80211 *cfg, struct net_device *ndev, bool roam);
static chanspec_t wl_cfg80211_get_shared_freq(struct wiphy *wiphy);
s32 wl_cfg80211_channel_to_freq(u32 channel);

#if defined(DHCP_SCAN_SUPPRESS)
static void wl_cfg80211_work_handler(struct work_struct *work);
static void wl_cfg80211_scan_supp_timerfunc(ulong data);
#endif /* DHCP_SCAN_SUPPRESS */

static void wl_cfg80211_work_handler(struct work_struct *work);
static s32 wl_add_keyext(struct wiphy *wiphy, struct net_device *dev,
	u8 key_idx, const u8 *mac_addr,
	struct key_params *params);
/*
 * key indianess swap utilities
 */
static void swap_key_from_BE(struct wl_wsec_key *key);
static void swap_key_to_BE(struct wl_wsec_key *key);

/*
 * bcm_cfg80211 memory init/deinit utilities
 */
static s32 wl_init_priv_mem(struct bcm_cfg80211 *cfg);
static void wl_deinit_priv_mem(struct bcm_cfg80211 *cfg);

static void wl_delay(u32 ms);

#ifdef CONFIG_HW_VOWIFI
static s32 wl_notify_vowifi_event(struct bcm_cfg80211 *cfg, bcm_struct_cfgdev *cfgdev,const wl_event_msg_t *e, void *data);
#endif

#ifdef HW_DFX_TXFAIL_EVENT
static s32 wl_notify_txfail_event(struct bcm_cfg80211 *cfg, bcm_struct_cfgdev *cfgdev,const wl_event_msg_t *e, void *data);
#endif

#ifdef CONFIG_HW_ABS
static s32 wl_notify_ant_event(struct bcm_cfg80211 *cfg, bcm_struct_cfgdev *cfgdev,const wl_event_msg_t *e, void *data);
#endif

/*
 * ibss mode utilities
 */
static bool wl_is_ibssmode(struct bcm_cfg80211 *cfg, struct net_device *ndev);
static __used bool wl_is_ibssstarter(struct bcm_cfg80211 *cfg);

/*
 * link up/down , default configuration utilities
 */
static s32 __wl_cfg80211_up(struct bcm_cfg80211 *cfg);
static s32 __wl_cfg80211_down(struct bcm_cfg80211 *cfg);
static bool wl_is_linkdown(struct bcm_cfg80211 *cfg, const wl_event_msg_t *e);
static bool wl_is_linkup(struct bcm_cfg80211 *cfg, const wl_event_msg_t *e,
	struct net_device *ndev);
static bool wl_is_nonetwork(struct bcm_cfg80211 *cfg, const wl_event_msg_t *e);
static void wl_link_up(struct bcm_cfg80211 *cfg);
static void wl_link_down(struct bcm_cfg80211 *cfg);
static s32 wl_config_ifmode(struct bcm_cfg80211 *cfg, struct net_device *ndev, s32 iftype);
static void wl_init_conf(struct wl_conf *conf);
static s32 wl_cfg80211_handle_ifdel(struct bcm_cfg80211 *cfg, wl_if_event_info *if_event_info,
	struct net_device* ndev);

int wl_cfg80211_get_ioctl_version(void);

/*
 * find most significant bit set
 */
static __used u32 wl_find_msb(u16 bit16);

/*
 * rfkill support
 */
static int wl_setup_rfkill(struct bcm_cfg80211 *cfg, bool setup);
static int wl_rfkill_set(void *data, bool blocked);
#ifdef DEBUGFS_CFG80211
static s32 wl_setup_debugfs(struct bcm_cfg80211 *cfg,struct net_device *ndev);
static s32 wl_free_debugfs(struct bcm_cfg80211 *cfg);
#endif

static wl_scan_params_t *wl_cfg80211_scan_alloc_params(int channel,
	int nprobes, int *out_params_size);
static bool check_dev_role_integrity(struct bcm_cfg80211 *cfg, u32 dev_role);

#ifdef WL_CFG80211_ACL
/* ACL */
static int wl_cfg80211_set_mac_acl(struct wiphy *wiphy, struct net_device *cfgdev,
	const struct cfg80211_acl_data *acl);
#endif /* WL_CFG80211_ACL */

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,5,0)) || defined(BCM_PATCH_ABORT_SCAN)
static void wl_cfg80211_abort_scan(struct wiphy *wiphy, struct wireless_dev *wdev);
#endif /*  if(LINUX_VERSION_CODE >= KERNEL_VERSION(4,5,0)) || defined(BCM_PATCH_ABORT_SCAN) */
#ifndef  BRCM_RSDB
static void wl_send_event(struct net_device *dev, uint32 event_type, uint32 status, uint32 reason);
#endif
/*
 * Some external functions, TODO: move them to dhd_linux.h
 */
int dhd_add_monitor(char *name, struct net_device **new_ndev);
int dhd_del_monitor(struct net_device *ndev);
int dhd_monitor_init(void *dhd_pub);
int dhd_monitor_uninit(void);
int dhd_start_xmit(struct sk_buff *skb, struct net_device *net);

#ifdef ROAM_CHANNEL_CACHE
#ifdef  BRCM_RSDB
int init_roam_cache(struct bcm_cfg80211 *cfg, int ioctl_ver);
void reset_roam_cache(struct bcm_cfg80211 *cfg);
void add_roam_cache(wl_bss_info_t *bi);
int  get_roam_channel_list(int target_chan,
	chanspec_t *channels, const wlc_ssid_t *ssid, int ioctl_ver);
void print_roam_cache(struct bcm_cfg80211 *cfg);
#else
void init_roam(int ioctl_ver);
void reset_roam_cache(void);
void add_roam_cache(wl_bss_info_t *bi);
int  get_roam_channel_list(int target_chan,
	chanspec_t *channels, const wlc_ssid_t *ssid, int ioctl_ver);
void print_roam_cache(void);
#endif /* BRCM_RSDB */
void set_roam_band(int band);
void update_roam_cache(struct bcm_cfg80211 *cfg, int ioctl_ver);
#endif /* ROAM_CHANNEL_CACHE */
#ifndef  BRCM_RSDB
#define MAX_ROAM_CACHE_NUM 100
#else
#ifdef DHD_IFDEBUG
void wl_dump_ifinfo(struct bcm_cfg80211 *cfg);
#endif
#endif /* BRCM_RSDB */
#ifdef  BRCM_RSDB
#ifdef P2P_LISTEN_OFFLOADING
s32 wl_cfg80211_p2plo_deinit(struct bcm_cfg80211 *cfg);
#endif /* P2P_LISTEN_OFFLOADING */
#endif /* BRCM_RSDB */
static int wl_cfg80211_delayed_roam(struct bcm_cfg80211 *cfg, struct net_device *ndev,
	const struct ether_addr *bssid);

static int bw2cap[] = { 0, 0, WLC_BW_CAP_20MHZ, WLC_BW_CAP_40MHZ, WLC_BW_CAP_80MHZ,
	WLC_BW_CAP_160MHZ, WLC_BW_CAP_160MHZ };
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 2, 0))
#define CFG80211_DISCONNECTED(dev, reason, ie, len, loc_gen, gfp) \
	cfg80211_disconnected(dev, reason, ie, len, gfp);
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 2, 0))
#define CFG80211_DISCONNECTED(dev, reason, ie, len, loc_gen, gfp) \
	cfg80211_disconnected(dev, reason, ie, len, loc_gen, gfp);
#endif
#ifndef BRCM_RSDB
#define RETURN_EIO_IF_NOT_UP(wlpriv)						\
do {									\
	struct net_device *checkSysUpNDev = bcmcfg_to_prmry_ndev(wlpriv);       	\
	if (unlikely(!wl_get_drv_status(wlpriv, READY, checkSysUpNDev))) {	\
		WL_INFORM(("device is not ready\n"));			\
		return -EIO;						\
	}								\
} while (0)
#endif /* BRCM_RSDB */
#ifdef RSSI_OFFSET
static s32 wl_rssi_offset(s32 rssi)
{
	rssi += RSSI_OFFSET;
	if (rssi > 0)
		rssi = 0;
	return rssi;
}
#else
#define wl_rssi_offset(x)	x
#endif


#define RSSI_TREMBLE_LIMIT 2
int wl_rssi_average(s32 rssi)
{
	int delt_val = 0;
	uint delt_flag = 0;
	static int old_rssi = 0;
	int real_rssi = 0;
	real_rssi = rssi;
	if(old_rssi){
		delt_flag = (real_rssi > old_rssi)? 1:0;
		delt_val = (delt_flag)? (real_rssi - old_rssi):(old_rssi - real_rssi);
		if((delt_val >= RSSI_TREMBLE_LIMIT) && (delt_val < 8 * RSSI_TREMBLE_LIMIT)) {
			real_rssi = delt_flag ?old_rssi + (delt_val/2) + 1: old_rssi - (delt_val/2) - 1;
		}
	}
	old_rssi = real_rssi;
	return real_rssi;
}


#define IS_WPA_AKM(akm) ((akm) == RSN_AKM_NONE || 			\
				 (akm) == RSN_AKM_UNSPECIFIED || 	\
				 (akm) == RSN_AKM_PSK)
#ifdef HW_READ_FW_LOG
extern void dhd_read_console_ex(dhd_pub_t *dhd);
#endif
extern int dhd_wait_pend8021x(struct net_device *dev);
#ifdef PROP_TXSTATUS_VSDB
extern int disable_proptx;
#endif /* PROP_TXSTATUS_VSDB */
#ifdef  BRCM_RSDB
#if defined(FORCE_DISABLE_SINGLECORE_SCAN)
extern void dhd_force_disable_singlcore_scan(dhd_pub_t *dhd);
#endif /* FORCE_DISABLE_SINGLECORE_SCAN */

extern int passive_channel_skip;

static s32
wl_ap_start_ind(struct bcm_cfg80211 *cfg, bcm_struct_cfgdev *cfgdev,
	const wl_event_msg_t *e, void *data);
static s32
wl_csa_complete_ind(struct bcm_cfg80211 *cfg, bcm_struct_cfgdev *cfgdev,
	const wl_event_msg_t *e, void *data);
#if ((LINUX_VERSION_CODE >= KERNEL_VERSION (3, 5, 0)) && (LINUX_VERSION_CODE <= KERNEL_VERSION (3, 7, \
	0)))
struct chan_info {
	int freq;
	int chan_type;
};
#endif
#endif /*BRCM_RSDB */

#if (WL_DBG_LEVEL > 0)
#define WL_DBG_ESTR_MAX	50
static s8 wl_dbg_estr[][WL_DBG_ESTR_MAX] = {
	"SET_SSID", "JOIN", "START", "AUTH", "AUTH_IND",
	"DEAUTH", "DEAUTH_IND", "ASSOC", "ASSOC_IND", "REASSOC",
	"REASSOC_IND", "DISASSOC", "DISASSOC_IND", "QUIET_START", "QUIET_END",
	"BEACON_RX", "LINK", "MIC_ERROR", "NDIS_LINK", "ROAM",
	"TXFAIL", "PMKID_CACHE", "RETROGRADE_TSF", "PRUNE", "AUTOAUTH",
	"EAPOL_MSG", "SCAN_COMPLETE", "ADDTS_IND", "DELTS_IND", "BCNSENT_IND",
	"BCNRX_MSG", "BCNLOST_MSG", "ROAM_PREP", "PFN_NET_FOUND",
	"PFN_NET_LOST",
	"RESET_COMPLETE", "JOIN_START", "ROAM_START", "ASSOC_START",
	"IBSS_ASSOC",
#ifdef  BRCM_RSDB	
	"RADIO", "PSM_WATCHDOG",
	"WLC_E_XXX_ASSOC_START", "WLC_E_XXX_ASSOC_ABORT",
#else
	"RADIO", "PSM_WATCHDOG", "WLC_E_CCX_ASSOC_START", "WLC_E_CCX_ASSOC_ABORT",
#endif
	"PROBREQ_MSG",
	"SCAN_CONFIRM_IND", "PSK_SUP", "COUNTRY_CODE_CHANGED",
	"EXCEEDED_MEDIUM_TIME", "ICV_ERROR",
	"UNICAST_DECODE_ERROR", "MULTICAST_DECODE_ERROR", "TRACE",
	"WLC_E_BTA_HCI_EVENT", "IF", "WLC_E_P2P_DISC_LISTEN_COMPLETE",
	"RSSI", "PFN_SCAN_COMPLETE", "WLC_E_EXTLOG_MSG",
	"ACTION_FRAME", "ACTION_FRAME_COMPLETE", "WLC_E_PRE_ASSOC_IND",
	"WLC_E_PRE_REASSOC_IND", "WLC_E_CHANNEL_ADOPTED", "WLC_E_AP_STARTED",
	"WLC_E_DFS_AP_STOP", "WLC_E_DFS_AP_RESUME", "WLC_E_WAI_STA_EVENT",
	"WLC_E_WAI_MSG", "WLC_E_ESCAN_RESULT", "WLC_E_ACTION_FRAME_OFF_CHAN_COMPLETE",
	"WLC_E_PROBRESP_MSG", "WLC_E_P2P_PROBREQ_MSG", "WLC_E_DCS_REQUEST", "WLC_E_FIFO_CREDIT_MAP",
	"WLC_E_ACTION_FRAME_RX", "WLC_E_WAKE_EVENT", "WLC_E_RM_COMPLETE"
};
#endif				/* WL_DBG_LEVEL */

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0))
#define CHAN2G(_channel, _freq, _flags) {			\
	.band			= NL80211_BAND_2GHZ,		\
	.center_freq		= (_freq),			\
	.hw_value		= (_channel),			\
	.flags			= (_flags),			\
	.max_antenna_gain	= 0,				\
	.max_power		= 30,				\
}

#define CHAN5G(_channel, _flags) {				\
	.band			= NL80211_BAND_5GHZ,		\
	.center_freq		= 5000 + (5 * (_channel)),	\
	.hw_value		= (_channel),			\
	.flags			= (_flags),			\
	.max_antenna_gain	= 0,				\
	.max_power		= 30,				\
}
#else
#define CHAN2G(_channel, _freq, _flags) {			\
	.band			= IEEE80211_BAND_2GHZ,		\
	.center_freq		= (_freq),			\
	.hw_value		= (_channel),			\
	.flags			= (_flags),			\
	.max_antenna_gain	= 0,				\
	.max_power		= 30,				\
}

#define CHAN5G(_channel, _flags) {				\
	.band			= IEEE80211_BAND_5GHZ,		\
	.center_freq		= 5000 + (5 * (_channel)),	\
	.hw_value		= (_channel),			\
	.flags			= (_flags),			\
	.max_antenna_gain	= 0,				\
	.max_power		= 30,				\
}
#endif

#define RATE_TO_BASE100KBPS(rate)   (((rate) * 10) / 2)
#define RATETAB_ENT(_rateid, _flags) \
	{								\
		.bitrate	= RATE_TO_BASE100KBPS(_rateid),     \
		.hw_value	= (_rateid),			    \
		.flags	  = (_flags),			     \
	}

static struct ieee80211_rate __wl_rates[] = {
	RATETAB_ENT(DOT11_RATE_1M, 0),
	RATETAB_ENT(DOT11_RATE_2M, IEEE80211_RATE_SHORT_PREAMBLE),
	RATETAB_ENT(DOT11_RATE_5M5, IEEE80211_RATE_SHORT_PREAMBLE),
	RATETAB_ENT(DOT11_RATE_11M, IEEE80211_RATE_SHORT_PREAMBLE),
	RATETAB_ENT(DOT11_RATE_6M, 0),
	RATETAB_ENT(DOT11_RATE_9M, 0),
	RATETAB_ENT(DOT11_RATE_12M, 0),
	RATETAB_ENT(DOT11_RATE_18M, 0),
	RATETAB_ENT(DOT11_RATE_24M, 0),
	RATETAB_ENT(DOT11_RATE_36M, 0),
	RATETAB_ENT(DOT11_RATE_48M, 0),
	RATETAB_ENT(DOT11_RATE_54M, 0)
};

#define wl_a_rates		(__wl_rates + 4)
#define wl_a_rates_size	8
#define wl_g_rates		(__wl_rates + 0)
#define wl_g_rates_size	12

static struct ieee80211_channel __wl_2ghz_channels[] = {
	CHAN2G(1, 2412, 0),
	CHAN2G(2, 2417, 0),
	CHAN2G(3, 2422, 0),
	CHAN2G(4, 2427, 0),
	CHAN2G(5, 2432, 0),
	CHAN2G(6, 2437, 0),
	CHAN2G(7, 2442, 0),
	CHAN2G(8, 2447, 0),
	CHAN2G(9, 2452, 0),
	CHAN2G(10, 2457, 0),
	CHAN2G(11, 2462, 0),
	CHAN2G(12, 2467, 0),
	CHAN2G(13, 2472, 0),
	CHAN2G(14, 2484, 0)
};

static struct ieee80211_channel __wl_5ghz_a_channels[] = {
	CHAN5G(34, 0), CHAN5G(36, 0),
	CHAN5G(38, 0), CHAN5G(40, 0),
	CHAN5G(42, 0), CHAN5G(44, 0),
	CHAN5G(46, 0), CHAN5G(48, 0),
	CHAN5G(52, 0), CHAN5G(56, 0),
	CHAN5G(60, 0), CHAN5G(64, 0),
	CHAN5G(100, 0), CHAN5G(104, 0),
	CHAN5G(108, 0), CHAN5G(112, 0),
	CHAN5G(116, 0), CHAN5G(120, 0),
	CHAN5G(124, 0), CHAN5G(128, 0),
	CHAN5G(132, 0), CHAN5G(136, 0),
	CHAN5G(140, 0), CHAN5G(144, 0),
	CHAN5G(149, 0),	CHAN5G(153, 0),
	CHAN5G(157, 0),	CHAN5G(161, 0),
	CHAN5G(165, 0)
};

static struct ieee80211_supported_band __wl_band_2ghz = {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0))
	.band = NL80211_BAND_2GHZ,
#else
	.band = IEEE80211_BAND_2GHZ,
#endif
	.channels = __wl_2ghz_channels,
	.n_channels = ARRAY_SIZE(__wl_2ghz_channels),
	.bitrates = wl_g_rates,
	.n_bitrates = wl_g_rates_size
};

static struct ieee80211_supported_band __wl_band_5ghz_a = {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0))
	.band = NL80211_BAND_5GHZ,
#else
	.band = IEEE80211_BAND_5GHZ,
#endif
	.channels = __wl_5ghz_a_channels,
	.n_channels = ARRAY_SIZE(__wl_5ghz_a_channels),
	.bitrates = wl_a_rates,
	.n_bitrates = wl_a_rates_size
};

static const u32 __wl_cipher_suites[] = {
	WLAN_CIPHER_SUITE_WEP40,
	WLAN_CIPHER_SUITE_WEP104,
	WLAN_CIPHER_SUITE_TKIP,
	WLAN_CIPHER_SUITE_CCMP,
	WLAN_CIPHER_SUITE_AES_CMAC,
#ifdef BCMWAPI_WPI
	WLAN_CIPHER_SUITE_SMS4,
#endif
#if defined(WLFBT) && defined(WLAN_CIPHER_SUITE_PMK)
	WLAN_CIPHER_SUITE_PMK,
#endif
};

#ifdef WL_SUPPORT_ACS
/*
 * The firmware code required for this feature to work is currently under
 * BCMINTERNAL flag. In future if this is to enabled we need to bring the
 * required firmware code out of the BCMINTERNAL flag.
 */
struct wl_dump_survey {
	u32 obss;
	u32 ibss;
	u32 no_ctg;
	u32 no_pckt;
	u32 tx;
	u32 idle;
};
#endif /* WL_SUPPORT_ACS */
#ifdef  BRCM_RSDB
#ifdef WL_CFG80211_GON_COLLISION
#define BLOCK_GON_REQ_MAX_NUM 5
#endif /* WL_CFG80211_GON_COLLISION */
#endif
#if defined(USE_DYNAMIC_MAXPKT_RXGLOM)
static int maxrxpktglom = 0;
#endif

/* IOCtl version read from targeted driver */
static int ioctl_version;
#ifdef DEBUGFS_CFG80211
#define S_SUBLOGLEVEL 20
static const struct {
	u32 log_level;
	char *sublogname;
} sublogname_map[] = {
	{WL_DBG_ERR, "ERR"},
	{WL_DBG_INFO, "INFO"},
	{WL_DBG_DBG, "DBG"},
	{WL_DBG_SCAN, "SCAN"},
	{WL_DBG_TRACE, "TRACE"},
	{WL_DBG_P2P_ACTION, "P2PACTION"}
};
#endif
#ifdef  BRCM_RSDB
#ifdef CUSTOMER_HW4_DEBUG
uint prev_dhd_console_ms = 0;
u32 prev_wl_dbg_level = 0;
bool wl_scan_timeout_dbg_enabled = 0;
static void wl_scan_timeout_dbg_set(void);
static void wl_scan_timeout_dbg_clear(void);
static void wl_scan_timeout_dbg_set(void)
{
	WL_ERR(("Enter \n"));
	prev_dhd_console_ms = dhd_console_ms;
	prev_wl_dbg_level = wl_dbg_level;

	dhd_console_ms = 1;
	wl_dbg_level |= (WL_DBG_ERR | WL_DBG_P2P_ACTION | WL_DBG_SCAN);

	wl_scan_timeout_dbg_enabled = 1;
}
static void wl_scan_timeout_dbg_clear(void)
{
	WL_ERR(("Enter \n"));
	dhd_console_ms = prev_dhd_console_ms;
	wl_dbg_level = prev_wl_dbg_level;

	wl_scan_timeout_dbg_enabled = 0;
}
#endif /* CUSTOMER_HW4_DEBUG */

/* watchdog timer for disconnecting when fw is not associated for FW_ASSOC_WATCHDOG_TIME ms */
uint32 fw_assoc_watchdog_ms = 0;
bool fw_assoc_watchdog_started = 0;
#define FW_ASSOC_WATCHDOG_TIME 10 * 1000 /* msec */

#ifdef DHD_IFDEBUG

void wl_dump_ifinfo(struct bcm_cfg80211 *cfg)
{
	WL_ERR(("cfg=%p\n", cfg));
	if (cfg) {
		WL_ERR(("cfg->wdev=%p\n", bcmcfg_to_prmry_wdev(cfg)));
		if (bcmcfg_to_prmry_wdev(cfg)) {
			WL_ERR(("cfg->wdev->wiphy=%p\n", bcmcfg_to_wiphy(cfg)));
			WL_ERR(("cfg->wdev->netdev=%p\n", bcmcfg_to_prmry_ndev(cfg)));
		}
	}
}
#endif
static void wl_add_remove_pm_enable_work(struct bcm_cfg80211 *cfg,
	enum wl_pm_workq_act_type type)
{
	u16 wq_duration = 0;

	if (cfg == NULL)
		return;

	mutex_lock(&cfg->pm_sync);
	/*
	 * Make cancel and schedule work part mutually exclusive
	 * so that while cancelling, we are sure that there is no
	 * work getting scheduled.
	 */
	if (delayed_work_pending(&cfg->pm_enable_work)) {
		cancel_delayed_work_sync(&cfg->pm_enable_work);
		DHD_OS_WAKE_UNLOCK(cfg->pub);
	}

	if (type == WL_PM_WORKQ_SHORT) {
		wq_duration = WL_PM_ENABLE_TIMEOUT;
	} else if (type == WL_PM_WORKQ_LONG) {
		wq_duration = (WL_PM_ENABLE_TIMEOUT*2);
	}
	if (wq_duration) {
		DHD_OS_WAKE_LOCK(cfg->pub);
		schedule_delayed_work(&cfg->pm_enable_work,
				msecs_to_jiffies((const unsigned int)wq_duration));
	}
	mutex_unlock(&cfg->pm_sync);
}
#else  /* BRCM_RSDB */

static void wl_add_remove_pm_enable_work(struct bcm_cfg80211 *cfg, bool add_remove,
	enum wl_handler_del_type type)
{
	if (cfg == NULL)
		return;

	if (cfg->pm_enable_work_on) {
		if (add_remove) {
			schedule_delayed_work(&cfg->pm_enable_work,
				msecs_to_jiffies(WL_PM_ENABLE_TIMEOUT));
		} else {
			cancel_delayed_work_sync(&cfg->pm_enable_work);
			switch (type) {
				case WL_HANDLER_MAINTAIN:
					schedule_delayed_work(&cfg->pm_enable_work,
						msecs_to_jiffies(WL_PM_ENABLE_TIMEOUT));
					break;
				case WL_HANDLER_PEND:
					schedule_delayed_work(&cfg->pm_enable_work,
						msecs_to_jiffies(WL_PM_ENABLE_TIMEOUT*2));
					break;
				case WL_HANDLER_DEL:
				default:
					cfg->pm_enable_work_on = false;
					break;
			}
		}
	}
}
#endif /* BRCM_RSDB */
/* Return a new chanspec given a legacy chanspec
 * Returns INVCHANSPEC on error
 */
static chanspec_t
wl_chspec_from_legacy(chanspec_t legacy_chspec)
{
	chanspec_t chspec;

	/* get the channel number */
	chspec = LCHSPEC_CHANNEL(legacy_chspec);

	/* convert the band */
	if (LCHSPEC_IS2G(legacy_chspec)) {
		chspec |= WL_CHANSPEC_BAND_2G;
	} else {
		chspec |= WL_CHANSPEC_BAND_5G;
	}

	/* convert the bw and sideband */
	if (LCHSPEC_IS20(legacy_chspec)) {
		chspec |= WL_CHANSPEC_BW_20;
	} else {
		chspec |= WL_CHANSPEC_BW_40;
		if (LCHSPEC_CTL_SB(legacy_chspec) == WL_LCHANSPEC_CTL_SB_LOWER) {
			chspec |= WL_CHANSPEC_CTL_SB_L;
		} else {
			chspec |= WL_CHANSPEC_CTL_SB_U;
		}
	}

	if (wf_chspec_malformed(chspec)) {
		WL_ERR(("wl_chspec_from_legacy: output chanspec (0x%04X) malformed\n",
		        chspec));
		return INVCHANSPEC;
	}

	return chspec;
}

/* Return a legacy chanspec given a new chanspec
 * Returns INVCHANSPEC on error
 */
static chanspec_t
wl_chspec_to_legacy(chanspec_t chspec)
{
	chanspec_t lchspec;

	if (wf_chspec_malformed(chspec)) {
		WL_ERR(("wl_chspec_to_legacy: input chanspec (0x%04X) malformed\n",
		        chspec));
		return INVCHANSPEC;
	}

	/* get the channel number */
	lchspec = CHSPEC_CHANNEL(chspec);

	/* convert the band */
	if (CHSPEC_IS2G(chspec)) {
		lchspec |= WL_LCHANSPEC_BAND_2G;
	} else {
		lchspec |= WL_LCHANSPEC_BAND_5G;
	}

	/* convert the bw and sideband */
	if (CHSPEC_IS20(chspec)) {
		lchspec |= WL_LCHANSPEC_BW_20;
		lchspec |= WL_LCHANSPEC_CTL_SB_NONE;
	} else if (CHSPEC_IS40(chspec)) {
		lchspec |= WL_LCHANSPEC_BW_40;
		if (CHSPEC_CTL_SB(chspec) == WL_CHANSPEC_CTL_SB_L) {
			lchspec |= WL_LCHANSPEC_CTL_SB_LOWER;
		} else {
			lchspec |= WL_LCHANSPEC_CTL_SB_UPPER;
		}
	} else {
		/* cannot express the bandwidth */
		char chanbuf[CHANSPEC_STR_LEN];
		WL_ERR((
		        "wl_chspec_to_legacy: unable to convert chanspec %s (0x%04X) "
		        "to pre-11ac format\n",
		        wf_chspec_ntoa(chspec, chanbuf), chspec));
		return INVCHANSPEC;
	}

	return lchspec;
}

/* given a chanspec value, do the endian and chanspec version conversion to
 * a chanspec_t value
 * Returns INVCHANSPEC on error
 */
chanspec_t
wl_chspec_host_to_driver(chanspec_t chanspec)
{
	if (ioctl_version == 1) {
		chanspec = wl_chspec_to_legacy(chanspec);
		if (chanspec == INVCHANSPEC) {
			return chanspec;
		}
	}
	chanspec = htodchanspec(chanspec);

	return chanspec;
}

/* given a channel value, do the endian and chanspec version conversion to
 * a chanspec_t value
 * Returns INVCHANSPEC on error
 */
#ifdef  BRCM_RSDB
chanspec_t
wl_ch_host_to_driver(s32 bssidx, u16 channel)
#else
chanspec_t
wl_ch_host_to_driver(u16 channel)
#endif
{

	chanspec_t chanspec;

	chanspec = channel & WL_CHANSPEC_CHAN_MASK;

	if (channel <= CH_MAX_2G_CHANNEL)
		chanspec |= WL_CHANSPEC_BAND_2G;
	else
		chanspec |= WL_CHANSPEC_BAND_5G;

#ifdef  BRCM_RSDB
	chanspec |= wl_cfg80211_ulb_get_min_bw_chspec(NULL, bssidx);
#else
	chanspec |= WL_CHANSPEC_BW_20;
#endif
	chanspec |= WL_CHANSPEC_CTL_SB_NONE;

	return wl_chspec_host_to_driver(chanspec);
}

/* given a chanspec value from the driver, do the endian and chanspec version conversion to
 * a chanspec_t value
 * Returns INVCHANSPEC on error
 */
chanspec_t
wl_chspec_driver_to_host(chanspec_t chanspec)
{
	chanspec = dtohchanspec(chanspec);
	if (ioctl_version == 1) {
		chanspec = wl_chspec_from_legacy(chanspec);
	}

	return chanspec;
}

/*
 * convert ASCII string to MAC address (colon-delimited format)
 * eg: 00:11:22:33:44:55
 */
int
wl_cfg80211_ether_atoe(const char *a, struct ether_addr *n)
{
	char *c = NULL;
	int count = 0;

	memset(n, 0, ETHER_ADDR_LEN);
	for (;;) {
		n->octet[count++] = (uint8)simple_strtoul(a, &c, 16);
		if (!*c++ || count == ETHER_ADDR_LEN)
			break;
		a = c;
	}
	return (count == ETHER_ADDR_LEN);
}
#ifndef  BRCM_RSDB
/* convert hex string buffer to binary */
int
wl_cfg80211_hex_str_to_bin(unsigned char *data, int dlen, char *str)
{
	int count, slen;
	int hvalue;
	char tmp[3] = {0};
	char *ptr = str, *endp = NULL;

	if (!data || !str || !dlen) {
		WL_DBG((" passed buffer is empty \n"));
		return 0;
	}

	slen = strlen(str);
	if (dlen * 2 < slen) {
		WL_DBG((" destination buffer too short \n"));
		return 0;
	}

	if (slen % 2) {
		WL_DBG((" source buffer is of odd length \n"));
		return 0;
	}

	for (count = 0; count < slen; count += 2) {
		memcpy(tmp, ptr, 2);
		hvalue = simple_strtol(tmp, &endp, 16);
		if (*endp != '\0') {
			WL_DBG((" non hexadecimal character encountered \n"));
			return 0;
		}
		*data++ = (unsigned char)hvalue;
		ptr += 2;
	}

	return (slen / 2);
}
#endif
/* There isn't a lot of sense in it, but you can transmit anything you like */
static const struct ieee80211_txrx_stypes
wl_cfg80211_default_mgmt_stypes[NUM_NL80211_IFTYPES] = {
	[NL80211_IFTYPE_ADHOC] = {
		.tx = 0xffff,
		.rx = BIT(IEEE80211_STYPE_ACTION >> 4)
	},
	[NL80211_IFTYPE_STATION] = {
		.tx = 0xffff,
		.rx = BIT(IEEE80211_STYPE_ACTION >> 4) |
		BIT(IEEE80211_STYPE_PROBE_REQ >> 4)
	},
	[NL80211_IFTYPE_AP] = {
		.tx = 0xffff,
		.rx = BIT(IEEE80211_STYPE_ASSOC_REQ >> 4) |
		BIT(IEEE80211_STYPE_REASSOC_REQ >> 4) |
		BIT(IEEE80211_STYPE_PROBE_REQ >> 4) |
		BIT(IEEE80211_STYPE_DISASSOC >> 4) |
		BIT(IEEE80211_STYPE_AUTH >> 4) |
		BIT(IEEE80211_STYPE_DEAUTH >> 4) |
		BIT(IEEE80211_STYPE_ACTION >> 4)
	},
	[NL80211_IFTYPE_AP_VLAN] = {
		/* copy AP */
		.tx = 0xffff,
		.rx = BIT(IEEE80211_STYPE_ASSOC_REQ >> 4) |
		BIT(IEEE80211_STYPE_REASSOC_REQ >> 4) |
		BIT(IEEE80211_STYPE_PROBE_REQ >> 4) |
		BIT(IEEE80211_STYPE_DISASSOC >> 4) |
		BIT(IEEE80211_STYPE_AUTH >> 4) |
		BIT(IEEE80211_STYPE_DEAUTH >> 4) |
		BIT(IEEE80211_STYPE_ACTION >> 4)
	},
	[NL80211_IFTYPE_P2P_CLIENT] = {
		.tx = 0xffff,
		.rx = BIT(IEEE80211_STYPE_ACTION >> 4) |
		BIT(IEEE80211_STYPE_PROBE_REQ >> 4)
	},
	[NL80211_IFTYPE_P2P_GO] = {
		.tx = 0xffff,
		.rx = BIT(IEEE80211_STYPE_ASSOC_REQ >> 4) |
		BIT(IEEE80211_STYPE_REASSOC_REQ >> 4) |
		BIT(IEEE80211_STYPE_PROBE_REQ >> 4) |
		BIT(IEEE80211_STYPE_DISASSOC >> 4) |
		BIT(IEEE80211_STYPE_AUTH >> 4) |
		BIT(IEEE80211_STYPE_DEAUTH >> 4) |
		BIT(IEEE80211_STYPE_ACTION >> 4)
	},
#if defined(WL_CFG80211_P2P_DEV_IF)
	[NL80211_IFTYPE_P2P_DEVICE] = {
		.tx = 0xffff,
		.rx = BIT(IEEE80211_STYPE_ACTION >> 4) |
		BIT(IEEE80211_STYPE_PROBE_REQ >> 4)
	},
#endif /* WL_CFG80211_P2P_DEV_IF */
};

static void swap_key_from_BE(struct wl_wsec_key *key)
{
	key->index = htod32(key->index);
	key->len = htod32(key->len);
	key->algo = htod32(key->algo);
	key->flags = htod32(key->flags);
	key->rxiv.hi = htod32(key->rxiv.hi);
	key->rxiv.lo = htod16(key->rxiv.lo);
	key->iv_initialized = htod32(key->iv_initialized);
}

static void swap_key_to_BE(struct wl_wsec_key *key)
{
	key->index = dtoh32(key->index);
	key->len = dtoh32(key->len);
	key->algo = dtoh32(key->algo);
	key->flags = dtoh32(key->flags);
	key->rxiv.hi = dtoh32(key->rxiv.hi);
	key->rxiv.lo = dtoh16(key->rxiv.lo);
	key->iv_initialized = dtoh32(key->iv_initialized);
}

/* Dump the contents of the encoded wps ie buffer and get pbc value */
static void
wl_validate_wps_ie(char *wps_ie, s32 wps_ie_len, bool *pbc)
{
	#define WPS_IE_FIXED_LEN 6
	u16 len;
	u8 *subel = NULL;
	u16 subelt_id;
	u16 subelt_len;
	u16 val;
	u8 *valptr = (uint8*) &val;
	if (wps_ie == NULL || wps_ie_len < WPS_IE_FIXED_LEN) {
		WL_ERR(("invalid argument : NULL\n"));
		return;
	}
	len = (u16)wps_ie[TLV_LEN_OFF];

	if (len > wps_ie_len) {
		WL_ERR(("invalid length len %d, wps ie len %d\n", len, wps_ie_len));
		return;
	}
	WL_DBG(("wps_ie len=%d\n", len));
	len -= 4;	/* for the WPS IE's OUI, oui_type fields */
	subel = wps_ie + WPS_IE_FIXED_LEN;
	while (len >= 4) {		/* must have attr id, attr len fields */
		valptr[0] = *subel++;
		valptr[1] = *subel++;
		subelt_id = HTON16(val);

		valptr[0] = *subel++;
		valptr[1] = *subel++;
		subelt_len = HTON16(val);

		len -= 4;			/* for the attr id, attr len fields */
#ifdef BCM_PATCH_GSCAN
		if (len < subelt_len) {
			WL_ERR(("not enough data, len %d, subelt_len %d\n", len,
				subelt_len));
			break;
		}
#endif
		len -= subelt_len;	/* for the remaining fields in this attribute */

		WL_DBG((" subel=%p, subelt_id=0x%x subelt_len=%u\n",
			subel, subelt_id, subelt_len));

		if (subelt_id == WPS_ID_VERSION) {
			WL_DBG(("  attr WPS_ID_VERSION: %u\n", *subel));
		} else if (subelt_id == WPS_ID_REQ_TYPE) {
			WL_DBG(("  attr WPS_ID_REQ_TYPE: %u\n", *subel));
		} else if (subelt_id == WPS_ID_CONFIG_METHODS) {
			valptr[0] = *subel;
			valptr[1] = *(subel + 1);
			WL_DBG(("  attr WPS_ID_CONFIG_METHODS: %x\n", HTON16(val)));
		} else if (subelt_id == WPS_ID_DEVICE_NAME) {
			char devname[100];
#ifdef BCM_PATCH_CVE_2016_0801
			int namelen = MIN(subelt_len, (sizeof(devname) - 1));
			if (namelen) {
				memcpy(devname, subel, namelen);
				devname[namelen] = '\0';
			}
#else
			memcpy(devname, subel, subelt_len);
			devname[subelt_len] = '\0';
#endif
			WL_DBG(("  attr WPS_ID_DEVICE_NAME: %s (len %u)\n",
				devname, subelt_len));
		} else if (subelt_id == WPS_ID_DEVICE_PWD_ID) {
			valptr[0] = *subel;
			valptr[1] = *(subel + 1);
			WL_DBG(("  attr WPS_ID_DEVICE_PWD_ID: %u\n", HTON16(val)));
			*pbc = (HTON16(val) == DEV_PW_PUSHBUTTON) ? true : false;
		} else if (subelt_id == WPS_ID_PRIM_DEV_TYPE) {
			valptr[0] = *subel;
			valptr[1] = *(subel + 1);
			WL_DBG(("  attr WPS_ID_PRIM_DEV_TYPE: cat=%u \n", HTON16(val)));
			valptr[0] = *(subel + 6);
			valptr[1] = *(subel + 7);
			WL_DBG(("  attr WPS_ID_PRIM_DEV_TYPE: subcat=%u\n", HTON16(val)));
		} else if (subelt_id == WPS_ID_REQ_DEV_TYPE) {
			valptr[0] = *subel;
			valptr[1] = *(subel + 1);
			WL_DBG(("  attr WPS_ID_REQ_DEV_TYPE: cat=%u\n", HTON16(val)));
			valptr[0] = *(subel + 6);
			valptr[1] = *(subel + 7);
			WL_DBG(("  attr WPS_ID_REQ_DEV_TYPE: subcat=%u\n", HTON16(val)));
		} else if (subelt_id == WPS_ID_SELECTED_REGISTRAR_CONFIG_METHODS) {
			valptr[0] = *subel;
			valptr[1] = *(subel + 1);
			WL_DBG(("  attr WPS_ID_SELECTED_REGISTRAR_CONFIG_METHODS"
				": cat=%u\n", HTON16(val)));
		} else {
			WL_DBG(("  unknown attr 0x%x\n", subelt_id));
		}

		subel += subelt_len;
	}
}

s32 wl_set_tx_power(struct net_device *dev,
	enum nl80211_tx_power_setting type, s32 dbm)
{
	s32 err = 0;
	s32 disable = 0;
	s32 txpwrqdbm;
	struct bcm_cfg80211 *cfg = g_bcm_cfg;

	if (cfg == NULL)
	{
		WL_ERR(("cfg is NULL, ignore wl_set_tx_power\n"));
		return BCME_ERROR;
	}

	/* Make sure radio is off or on as far as software is concerned */
	disable = WL_RADIO_SW_DISABLE << 16;
	disable = htod32(disable);
	err = wldev_ioctl(dev, WLC_SET_RADIO, &disable, sizeof(disable), true);
	if (unlikely(err)) {
		WL_ERR(("WLC_SET_RADIO error (%d)\n", err));
		return err;
	}

	if (dbm > 0xffff)
		dbm = 0xffff;
	txpwrqdbm = dbm * 4;
#ifdef SUPPORT_WL_TXPOWER
	if (type == NL80211_TX_POWER_AUTOMATIC)
		txpwrqdbm = 127;
	else
		txpwrqdbm |= WL_TXPWR_OVERRIDE;
#endif /* SUPPORT_WL_TXPOWER */
	err = wldev_iovar_setbuf_bsscfg(dev, "qtxpower", (void *)&txpwrqdbm,
		sizeof(txpwrqdbm), cfg->ioctl_buf, WLC_IOCTL_SMLEN, 0,
		&cfg->ioctl_buf_sync);
	if (unlikely(err))
		WL_ERR(("qtxpower error (%d)\n", err));
	else
		WL_ERR(("dBm=%d, txpwrqdbm=0x%x\n", dbm, txpwrqdbm));

	return err;
}

s32 wl_get_tx_power(struct net_device *dev, s32 *dbm)
{
	s32 err = 0;
	s32 txpwrdbm;
	struct bcm_cfg80211 *cfg = g_bcm_cfg;

	if (cfg == NULL)
	{
		WL_ERR(("cfg is NULL, ignore wl_get_tx_power\n"));
		return BCME_ERROR;
	}

	err = wldev_iovar_getbuf_bsscfg(dev, "qtxpower",
		NULL, 0, cfg->ioctl_buf, WLC_IOCTL_SMLEN, 0, &cfg->ioctl_buf_sync);
	if (unlikely(err)) {
		WL_ERR(("error (%d)\n", err));
		return err;
	}

	memcpy(&txpwrdbm, cfg->ioctl_buf, sizeof(txpwrdbm));
	txpwrdbm = dtoh32(txpwrdbm);
	*dbm = (txpwrdbm & ~WL_TXPWR_OVERRIDE) / 4;

	WL_INFORM(("dBm=%d, txpwrdbm=0x%x\n", *dbm, txpwrdbm));

	return err;
}

static chanspec_t wl_cfg80211_get_shared_freq(struct wiphy *wiphy)
{
	chanspec_t chspec;
	int err = 0;
	struct bcm_cfg80211 *cfg = wiphy_priv(wiphy);
	struct net_device *dev = bcmcfg_to_prmry_ndev(cfg);
	struct ether_addr bssid;
	struct wl_bss_info *bss = NULL;
#ifdef  BRCM_RSDB
	s32 bssidx = 0; /* Explicitly set to primary bssidx */
#endif
	char *buf;
	memset(&bssid, 0, sizeof(bssid));
	if ((err = wldev_ioctl(dev, WLC_GET_BSSID, &bssid, sizeof(bssid), false))) {
		/* STA interface is not associated. So start the new interface on a temp
		 * channel . Later proper channel will be applied by the above framework
		 * via set_channel (cfg80211 API).
		 */
		WL_DBG(("Not associated. Return a temp channel. \n"));
#ifdef  BRCM_RSDB
		return wl_ch_host_to_driver(bssidx, WL_P2P_TEMP_CHAN);
#else
		return wl_ch_host_to_driver(WL_P2P_TEMP_CHAN);
#endif
	}
#ifdef  BRCM_RSDB
	buf = kzalloc(WL_EXTRA_BUF_MAX, GFP_KERNEL);
	if (!buf) {
		WL_ERR(("buf alloc failed. use temp channel\n"));
		return wl_ch_host_to_driver(bssidx, WL_P2P_TEMP_CHAN);
#else
	buf = kzalloc(WL_EXTRA_BUF_MAX, GFP_KERNEL);
	if (!buf) {
		WL_ERR(("buf alloc failed. use temp channel\n"));
		return wl_ch_host_to_driver(WL_P2P_TEMP_CHAN);
#endif
	}
	*(u32 *)buf = htod32(WL_EXTRA_BUF_MAX);
	err = wldev_ioctl(dev, WLC_GET_BSS_INFO, buf, WL_EXTRA_BUF_MAX, false);
	if (err) {
		WL_ERR(("Failed to get bss info, use temp channel\n"));
#ifdef BRCM_RSDB
		chspec = wl_ch_host_to_driver(bssidx, WL_P2P_TEMP_CHAN);
#else
		chspec = wl_ch_host_to_driver(WL_P2P_TEMP_CHAN);
#endif
	}
	else {
		bss = (struct wl_bss_info *)(buf + 4);
		chspec = bss->chanspec;
		WL_DBG(("Valid BSS Found. chanspec:%d\n", chspec));
	}
	kfree(buf);
	return chspec;
}

static bcm_struct_cfgdev *
wl_cfg80211_add_monitor_if(char *name)
{
#if defined(WL_ENABLE_P2P_IF) || defined(WL_CFG80211_P2P_DEV_IF)
	WL_INFORM(("wl_cfg80211_add_monitor_if: No more support monitor interface\n"));
	return ERR_PTR(-EOPNOTSUPP);
#else
	struct net_device* ndev = NULL;

	dhd_add_monitor(name, &ndev);
	WL_INFORM(("wl_cfg80211_add_monitor_if net device returned: 0x%p\n", ndev));
	return ndev_to_cfgdev(ndev);
#endif /* WL_ENABLE_P2P_IF || WL_CFG80211_P2P_DEV_IF */
}

static bcm_struct_cfgdev *
wl_cfg80211_add_virtual_iface(struct wiphy *wiphy,
#if defined(WL_CFG80211_P2P_DEV_IF)
	const char *name,
#else
	char *name,
#endif /* WL_CFG80211_P2P_DEV_IF */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0)) && defined(HW_KERNEL_4_0_ADAPTATION)
	unsigned char name_assign_type,
#endif
	enum nl80211_iftype type, u32 *flags,
	struct vif_params *params)
{
	s32 err = -ENODEV;
	s32 timeout = -1;
	s32 wlif_type = -1;
	s32 mode = 0;
	s32 val = 0;
#ifdef  BRCM_RSDB
	s32 cfg_type;
#endif
	s32 dhd_mode = 0;
	chanspec_t chspec;
	struct bcm_cfg80211 *cfg = wiphy_priv(wiphy);
	struct net_device *primary_ndev;
	struct net_device *new_ndev;
	struct ether_addr primary_mac;
#ifdef  BRCM_RSDB
#ifdef WL_VIRTUAL_APSTA
	bcm_struct_cfgdev *new_cfgdev;
#endif /* WL_VIRTUAL_APSTA */
#endif /* BRCM_RSDB */
#ifdef PROP_TXSTATUS_VSDB
#if defined(BCMSDIO)
	s32 up = 1;
	dhd_pub_t *dhd;
	bool enabled;
#endif
#endif /* PROP_TXSTATUS_VSDB */
#ifdef  BRCM_RSDB
	bool hang_required = false;
#endif

	if (!cfg)
		return ERR_PTR(-EINVAL);

#ifdef PROP_TXSTATUS_VSDB
#if defined(BCMSDIO)
	dhd = (dhd_pub_t *)(cfg->pub);
#endif
#endif /* PROP_TXSTATUS_VSDB */


	/* Use primary I/F for sending cmds down to firmware */
	primary_ndev = bcmcfg_to_prmry_ndev(cfg);

	if (unlikely(!wl_get_drv_status(cfg, READY, primary_ndev))) {
		WL_ERR(("device is not ready\n"));
		return ERR_PTR(-ENODEV);
	}

	WL_DBG(("if name: %s, type: %d\n", name, type));
	switch (type) {
	case NL80211_IFTYPE_ADHOC:
#if defined(WLAIBSS_MCHAN) && defined(BRCM_RSDB)
		return bcm_cfg80211_add_ibss_if(wiphy, (char *)name);
#endif /* WLAIBSS_MCHAN && BRCM_RSDB */
	case NL80211_IFTYPE_AP_VLAN:
	case NL80211_IFTYPE_WDS:
	case NL80211_IFTYPE_MESH_POINT:
		WL_ERR(("Unsupported interface type\n"));
		mode = WL_MODE_IBSS;
		return NULL;
	case NL80211_IFTYPE_MONITOR:
		return wl_cfg80211_add_monitor_if((char *)name);
#if defined(WL_CFG80211_P2P_DEV_IF)
	case NL80211_IFTYPE_P2P_DEVICE:
#ifdef  BRCM_RSDB
		cfg->down_disc_if = FALSE;
#endif
		return wl_cfgp2p_add_p2p_disc_if(cfg);
#endif /* WL_CFG80211_P2P_DEV_IF */
	case NL80211_IFTYPE_STATION:
#ifdef  BRCM_RSDB
#ifdef WL_VIRTUAL_APSTA
#ifdef WLAIBSS_MCHAN
		if (cfg->ibss_cfgdev) {
			WL_ERR(("AIBSS is already operational. "
					" AIBSS & DUALSTA can't be used together \n"));
			return ERR_PTR(-ENOMEM);
		}
#endif /* WLAIBSS_MCHAN */
		if (!name) {
			WL_ERR(("Interface name not provided \n"));
			return ERR_PTR(-ENODEV);
		}

		if (wl_cfgp2p_vif_created(cfg)) {
			WL_ERR(("Could not create new iface."
				"Already one p2p interface is running"));
			return ERR_PTR(-ENODEV);
		}
		new_cfgdev = wl_cfg80211_create_iface(cfg->wdev->wiphy,
			NL80211_IFTYPE_STATION, NULL, name);
		if (!new_cfgdev)
			return ERR_PTR(-ENOMEM);
		else
			return new_cfgdev;
#endif /* WL_VIRTUAL_APSTA */
#else  /* BRCM_RSDB */
#ifdef DUAL_STA
		if (cfg->ibss_cfgdev) {
			WL_ERR(("AIBSS is already operational. "
					" AIBSS & DUALSTA can't be used together \n"));
			return NULL;
		}
		if (!name) {
			WL_ERR(("Interface name not provided \n"));
			return NULL;
		}
		return wl_cfg80211_create_iface(cfg->wdev->wiphy,
			NL80211_IFTYPE_STATION, NULL, name);
#endif /* DUAL_STA */
#endif /* BRCM_RSDB */
	case NL80211_IFTYPE_P2P_CLIENT:
		wlif_type = WL_P2P_IF_CLIENT;
		mode = WL_MODE_BSS;
		break;
	case NL80211_IFTYPE_P2P_GO:
	case NL80211_IFTYPE_AP:
		wlif_type = WL_P2P_IF_GO;
		mode = WL_MODE_AP;
		break;
	default:
		WL_ERR(("Unsupported interface type\n"));
#ifndef  BRCM_RSDB
		return NULL;
#else
		return ERR_PTR(-ENODEV);
#endif
		break;
	}

	if (!name) {
		WL_ERR(("name is NULL\n"));
#ifndef  BRCM_RSDB
		return NULL;
#else
		return ERR_PTR(-ENODEV);
#endif
	}
	if (cfg->p2p_supported && (wlif_type != -1)) {
		ASSERT(cfg->p2p); /* ensure expectation of p2p initialization */

#ifdef PROP_TXSTATUS_VSDB
#if defined(BCMSDIO)
		if (!dhd)
			return ERR_PTR(-ENODEV);
#endif
#endif /* PROP_TXSTATUS_VSDB */
		if (!cfg->p2p)
			return ERR_PTR(-ENODEV);
#ifdef  BRCM_RSDB
		if (cfg->cfgdev_bssidx != -1) {
			WL_ERR(("Failed to start p2p, Maximum no of interface reached"));
			return ERR_PTR(-ENODEV);
		}
#endif
		if (cfg->p2p && !cfg->p2p->on && strstr(name, WL_P2P_INTERFACE_PREFIX)) {
			p2p_on(cfg) = true;
			wl_cfgp2p_set_firm_p2p(cfg);
			wl_cfgp2p_init_discovery(cfg);
			get_primary_mac(cfg, &primary_mac);
#ifndef  BRCM_RSDB
			wl_cfgp2p_generate_bss_mac(&primary_mac,
				&cfg->p2p->dev_addr, &cfg->p2p->int_addr);
#else
			wl_cfgp2p_generate_bss_mac(cfg, &primary_mac);
#endif
		}

		strncpy(cfg->p2p->vir_ifname, name, IFNAMSIZ - 1);
		cfg->p2p->vir_ifname[IFNAMSIZ - 1] = '\0';

		wl_cfg80211_scan_abort(cfg);
#ifdef PROP_TXSTATUS_VSDB
#if defined(BCMSDIO)
		if (!cfg->wlfc_on && !disable_proptx) {
			dhd_wlfc_get_enable(dhd, &enabled);
			if (!enabled && dhd->op_mode != DHD_FLAG_HOSTAP_MODE &&
				dhd->op_mode != DHD_FLAG_IBSS_MODE) {
				dhd_wlfc_init(dhd);
				err = wldev_ioctl(primary_ndev, WLC_UP, &up, sizeof(s32), true);
				if (err < 0)
					WL_ERR(("WLC_UP return err:%d\n", err));
			}
			cfg->wlfc_on = true;
		}
#endif
#endif /* PROP_TXSTATUS_VSDB */
#ifdef  BRCM_RSDB
		/* Dual p2p doesn't support multiple P2PGO interfaces,
		 * p2p_go_count is the counter for GO creation
		 * requests.
		 */
		if ((cfg->p2p->p2p_go_count > 0) && (type == NL80211_IFTYPE_P2P_GO)) {
			WL_ERR(("Fw doesnot support  multiple Go"));
			return ERR_PTR(-ENOMEM);
		}
#endif
		/* In concurrency case, STA may be already associated in a particular channel.
		 * so retrieve the current channel of primary interface and then start the virtual
		 * interface on that.
		 */
		 chspec = wl_cfg80211_get_shared_freq(wiphy);

		/* For P2P mode, use P2P-specific driver features to create the
		 * bss: "cfg p2p_ifadd"
		 */
#ifdef  BRCM_RSDB
		if (wl_check_dongle_idle(wiphy) != TRUE) {
			WL_ERR(("FW is busy to add interface"));
			return ERR_PTR(-ENOMEM);
		}
#endif
		wl_set_p2p_status(cfg, IF_ADDING);
		memset(&cfg->if_event_info, 0, sizeof(cfg->if_event_info));
		if (wlif_type == WL_P2P_IF_GO)
			wldev_iovar_setint(primary_ndev, "mpc", 0);
#ifdef  BRCM_RSDB
		cfg_type = wl_cfgp2p_get_conn_idx(cfg);
		if (cfg_type == BCME_ERROR) {
			wl_clr_p2p_status(cfg, IF_ADDING);
			WL_ERR(("Failed to get connection idx for p2p interface"));
			goto fail;
		}
		err = wl_cfgp2p_ifadd(cfg, wl_to_p2p_bss_macaddr(cfg, cfg_type),
			htod32(wlif_type), chspec);
#else
		err = wl_cfgp2p_ifadd(cfg, &cfg->p2p->int_addr, htod32(wlif_type), chspec);
#endif
		if (unlikely(err)) {
			wl_clr_p2p_status(cfg, IF_ADDING);
			WL_ERR((" virtual iface add failed (%d) \n", err));
			return ERR_PTR(-ENOMEM);
		}

		timeout = wait_event_interruptible_timeout(cfg->netif_change_event,
#ifndef  BRCM_RSDB
			(wl_get_p2p_status(cfg, IF_ADDING) == false),
#else
			((wl_get_p2p_status(cfg, IF_ADDING) == false) &&
			(cfg->if_event_info.valid)),
#endif
			msecs_to_jiffies(MAX_WAIT_TIME));

		if (timeout > 0 && !wl_get_p2p_status(cfg, IF_ADDING) && cfg->if_event_info.valid) {
			struct wireless_dev *vwdev;
			int pm_mode = PM_ENABLE;
			wl_if_event_info *event = &cfg->if_event_info;

			/* IF_ADD event has come back, we can proceed to to register
			 * the new interface now, use the interface name provided by caller (thus
			 * ignore the one from wlc)
			 */
#ifndef  BRCM_RSDB
			strncpy(cfg->if_event_info.name, name, IFNAMSIZ - 1);
			new_ndev = wl_cfg80211_allocate_if(cfg, event->ifidx, cfg->p2p->vir_ifname,
				event->mac, event->bssidx);
#else
			new_ndev = wl_cfg80211_allocate_if(cfg, event->ifidx, cfg->p2p->vir_ifname,
				event->mac, event->bssidx, event->name);
#endif
			if (new_ndev == NULL)
				goto fail;
#ifndef  BRCM_RSDB
			wl_to_p2p_bss_ndev(cfg, P2PAPI_BSSCFG_CONNECTION) = new_ndev;
			wl_to_p2p_bss_bssidx(cfg, P2PAPI_BSSCFG_CONNECTION) = event->bssidx;
#else
			wl_to_p2p_bss_ndev(cfg, cfg_type) = new_ndev;
			wl_to_p2p_bss_bssidx(cfg, cfg_type) = event->bssidx;
#endif
			vwdev = kzalloc(sizeof(*vwdev), GFP_KERNEL);
			if (unlikely(!vwdev)) {
				WL_ERR(("Could not allocate wireless device\n"));
#ifdef  BRCM_RSDB
				err = -ENOMEM;
#endif
				goto fail;
			}
			vwdev->wiphy = cfg->wdev->wiphy;
			WL_INFORM(("virtual interface(%s) is created\n", cfg->p2p->vir_ifname));
#ifdef  BRCM_RSDB
			if (type == NL80211_IFTYPE_P2P_GO) {
				cfg->p2p->p2p_go_count++;
			}
#endif
			vwdev->iftype = type;
#if defined(DHD_IFDEBUG) && defined(BRCM_RSDB)
			WL_ERR(("new_ndev: %p\n", new_ndev));
#endif
			vwdev->netdev = new_ndev;
			new_ndev->ieee80211_ptr = vwdev;
			SET_NETDEV_DEV(new_ndev, wiphy_dev(vwdev->wiphy));
			wl_set_drv_status(cfg, READY, new_ndev);
#ifndef  BRCM_RSDB
			cfg->p2p->vif_created = true;
#endif
			wl_set_mode_by_netdev(cfg, new_ndev, mode);

			if (wl_cfg80211_register_if(cfg, event->ifidx, new_ndev) != BCME_OK) {
				wl_cfg80211_remove_if(cfg, event->ifidx, new_ndev);
#ifdef  BRCM_RSDB
				err = -ENODEV;
#endif
				goto fail;
			}
#ifndef  BRCM_RSDB
			wl_alloc_netinfo(cfg, new_ndev, vwdev, mode, pm_mode);
#else
			err = wl_alloc_netinfo(cfg, new_ndev, vwdev, mode, pm_mode, event->bssidx);
			if (unlikely(err != 0)) {
				WL_ERR(("Allocation of netinfo failed (%d) \n", err));
				goto fail;
			}
#endif
			val = 1;
			/* Disable firmware roaming for P2P interface  */
			wldev_iovar_setint(new_ndev, "roam_off", val);
#if defined(WL11ULB) && defined(BRCM_RSDB)
			if (cfg->p2p_wdev && is_p2p_group_iface(new_ndev->ieee80211_ptr)) {
				u32 ulb_bw = wl_cfg80211_get_ulb_bw(cfg->p2p_wdev);
				if (ulb_bw) {
					/* Apply ULB BW settings on the newly spawned interface */
					WL_DBG(("[ULB] Applying ULB BW for the newly"
						"created P2P interface \n"));
					if (wl_cfg80211_set_ulb_bw(new_ndev,
						ulb_bw, new_ndev->name) < 0) {
						/*
						 * If ulb_bw set failed, fail the iface creation.
						 * wl_dealloc_netinfo_by_wdev will be called by the
						 * unregister notifier.
						 */
						wl_cfg80211_remove_if(cfg, event->ifidx, new_ndev);
						err = -EINVAL;
						goto fail;
					}
				}
			}
#endif /* WL11ULB && BRCM_RSDB */

			if (mode != WL_MODE_AP)
				wldev_iovar_setint(new_ndev, "buf_key_b4_m4", 1);

			WL_ERR((" virtual interface(%s) is "
				"created net attach done\n", cfg->p2p->vir_ifname));
			if (mode == WL_MODE_AP)
				wl_set_drv_status(cfg, CONNECTED, new_ndev);
			if (type == NL80211_IFTYPE_P2P_CLIENT)
				dhd_mode = DHD_FLAG_P2P_GC_MODE;
			else if (type == NL80211_IFTYPE_P2P_GO)
				dhd_mode = DHD_FLAG_P2P_GO_MODE;
			DNGL_FUNC(dhd_cfg80211_set_p2p_info, (cfg, dhd_mode));
			/* reinitialize completion to clear previous count */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 13, 0))
			INIT_COMPLETION(cfg->iface_disable);
#else
			init_completion(&cfg->iface_disable);
#endif
			return ndev_to_cfgdev(new_ndev);
		} else {
			wl_clr_p2p_status(cfg, IF_ADDING);
			WL_ERR((" virtual interface(%s) is not created \n", cfg->p2p->vir_ifname));
#ifdef  BRCM_RSDB
			WL_ERR(("left timeout : %d\n", timeout));
			WL_ERR(("IF_ADDING status : %d\n", wl_get_p2p_status(cfg, IF_ADDING)));
			WL_ERR(("event valid : %d\n", cfg->if_event_info.valid));

			wl_clr_p2p_status(cfg, GO_NEG_PHASE);
			wl_set_p2p_status(cfg, IF_DELETING);

			err = wl_cfgp2p_ifdel(cfg, wl_to_p2p_bss_macaddr(cfg, cfg_type));
			if (err == BCME_OK) {
				timeout = wait_event_interruptible_timeout(cfg->netif_change_event,
					((wl_get_p2p_status(cfg, IF_DELETING) == false) &&
					(cfg->if_event_info.valid)),
					msecs_to_jiffies(MAX_WAIT_TIME));
				if (timeout > 0 && !wl_get_p2p_status(cfg, IF_DELETING) &&
					cfg->if_event_info.valid) {
					 /*
					  * Should indicate upper layer this failure case of p2p
					  * interface creation
					  */
					WL_ERR(("IFDEL operation done\n"));
				} else {
					WL_ERR(("IFDEL didn't complete properly\n"));
					hang_required = true;
				}
			} else {
				hang_required = true;
			}

			if (hang_required) {
				struct net_device *ndev = bcmcfg_to_prmry_ndev(cfg);
				dhd_pub_t *dhd = (dhd_pub_t *)(cfg->pub);

				WL_ERR(("p2p_ifdel failed, error %d, sent HANG event to %s\n",
					err, ndev->name));
				dhd->hang_reason = HANG_REASON_P2P_IFACE_DEL_FAILURE;
				net_os_send_hang_message(ndev);
			}

			memset(cfg->p2p->vir_ifname, '\0', IFNAMSIZ);
			wl_to_p2p_bss_bssidx(cfg, cfg_type) = -1;
#else
			memset(cfg->p2p->vir_ifname, '\0', IFNAMSIZ);
			cfg->p2p->vif_created = false;
#endif /* BRCM_RSDB */
#ifdef PROP_TXSTATUS_VSDB
#if defined(BCMSDIO)
			dhd_wlfc_get_enable(dhd, &enabled);
		if (enabled && cfg->wlfc_on && dhd->op_mode != DHD_FLAG_HOSTAP_MODE &&
			dhd->op_mode != DHD_FLAG_IBSS_MODE) {
			dhd_wlfc_deinit(dhd);
			cfg->wlfc_on = false;
		}
#endif
#endif /* PROP_TXSTATUS_VSDB */
#ifdef  BRCM_RSDB
			/*
			* Returns -ENODEV to upperlayer to indicate that DHD
			* failed to create p2p interface
			*/
			err = -ENODEV;
#endif
		}
	}

fail:
	if (wlif_type == WL_P2P_IF_GO)
		wldev_iovar_setint(primary_ndev, "mpc", 1);
#ifndef  BRCM_RSDB
	return ERR_PTR(-ENODEV);
#else
	return ERR_PTR(err);
#endif
}
#ifndef  BRCM_RSDB
static s32
wl_cfg80211_del_virtual_iface(struct wiphy *wiphy, bcm_struct_cfgdev *cfgdev)
{
	struct net_device *dev = NULL;
	struct ether_addr p2p_mac;
	struct bcm_cfg80211 *cfg = wiphy_priv(wiphy);
	s32 timeout = -1;
	s32 ret = 0;
	s32 index = -1;
#ifdef CUSTOM_SET_CPUCORE
	dhd_pub_t *dhd = (dhd_pub_t *)(cfg->pub);
#endif /* CUSTOM_SET_CPUCORE */
	WL_DBG(("Enter\n"));

#ifdef CUSTOM_SET_CPUCORE
	dhd->chan_isvht80 &= ~DHD_FLAG_P2P_MODE;
	if (!(dhd->chan_isvht80))
		dhd_set_cpucore(dhd, FALSE);
#endif /* CUSTOM_SET_CPUCORE */
#if defined(WL_CFG80211_P2P_DEV_IF)
	if (cfgdev->iftype == NL80211_IFTYPE_P2P_DEVICE) {
		return wl_cfgp2p_del_p2p_disc_if(cfgdev, cfg);
	}
#endif /* WL_CFG80211_P2P_DEV_IF */
	dev = cfgdev_to_wlc_ndev(cfgdev, cfg);

	if (cfgdev == cfg->ibss_cfgdev)
		return bcm_cfg80211_del_ibss_if(wiphy, cfgdev);

#ifdef DUAL_STA
	if (cfgdev == cfg->bss_cfgdev)
		return wl_cfg80211_del_iface(wiphy, cfgdev);
#endif /* DUAL_STA */

	if (wl_cfgp2p_find_idx(cfg, dev, &index) != BCME_OK) {
		WL_ERR(("Find p2p index from ndev(%pK) failed\n", dev));
		return BCME_ERROR;
	}
	if (cfg->p2p_supported) {
		memcpy(p2p_mac.octet, cfg->p2p->int_addr.octet, ETHER_ADDR_LEN);

		/* Clear GO_NEG_PHASE bit to take care of GO-NEG-FAIL cases
		 */
		WL_DBG(("P2P: GO_NEG_PHASE status cleared "));
		wl_clr_p2p_status(cfg, GO_NEG_PHASE);
		if (cfg->p2p->vif_created) {
			if (wl_get_drv_status(cfg, SCANNING, dev)) {
				wl_notify_escan_complete(cfg, dev, true, true);
			}
			wldev_iovar_setint(dev, "mpc", 1);
			/* Delete pm_enable_work */
			wl_add_remove_pm_enable_work(cfg, FALSE, WL_HANDLER_DEL);

			/* for GC */
			if (wl_get_drv_status(cfg, DISCONNECTING, dev) &&
				(wl_get_mode_by_netdev(cfg, dev) != WL_MODE_AP)) {
				WL_ERR(("Wait for Link Down event for GC !\n"));
				wait_for_completion_timeout
					(&cfg->iface_disable, msecs_to_jiffies(500));
			}

			memset(&cfg->if_event_info, 0, sizeof(cfg->if_event_info));
			wl_set_p2p_status(cfg, IF_DELETING);
			DNGL_FUNC(dhd_cfg80211_clean_p2p_info, (cfg));

			/* for GO */
			if (wl_get_mode_by_netdev(cfg, dev) == WL_MODE_AP) {
				wl_add_remove_eventmsg(dev, WLC_E_PROBREQ_MSG, false);
				/* disable interface before bsscfg free */
				ret = wl_cfgp2p_ifdisable(cfg, &p2p_mac);
				/* if fw doesn't support "ifdis",
				   do not wait for link down of ap mode
				 */
				if (ret == 0) {
					WL_ERR(("Wait for Link Down event for GO !!!\n"));
					wait_for_completion_timeout(&cfg->iface_disable,
						msecs_to_jiffies(500));
				} else if (ret != BCME_UNSUPPORTED) {
					msleep(300);
				}
			}
			wl_cfgp2p_clear_management_ie(cfg, index);

			if (wl_get_mode_by_netdev(cfg, dev) != WL_MODE_AP)
				wldev_iovar_setint(dev, "buf_key_b4_m4", 0);

			/* delete interface after link down */
			ret = wl_cfgp2p_ifdel(cfg, &p2p_mac);

			if (ret != BCME_OK) {
				struct net_device *ndev = bcmcfg_to_prmry_ndev(cfg);

				WL_ERR(("p2p_ifdel failed, error %d, sent HANG event to %s\n",
					ret, ndev->name));
				#if defined(BCMDONGLEHOST) && (defined(OEM_ANDROID) || defined(HW_HANG_EVENT))
				net_os_send_hang_message(ndev);
				#endif
			} else {
				/* Wait for IF_DEL operation to be finished */
				timeout = wait_event_interruptible_timeout(cfg->netif_change_event,
					(wl_get_p2p_status(cfg, IF_DELETING) == false),
					msecs_to_jiffies(MAX_WAIT_TIME));
				if (timeout > 0 && !wl_get_p2p_status(cfg, IF_DELETING) &&
					cfg->if_event_info.valid) {

					WL_DBG(("IFDEL operation done\n"));
					wl_cfg80211_handle_ifdel(cfg, &cfg->if_event_info, dev);
				} else {
					WL_ERR(("IFDEL didn't complete properly\n"));
				}
			}

			ret = dhd_del_monitor(dev);
			if (wl_get_mode_by_netdev(cfg, dev) == WL_MODE_AP) {
				DHD_OS_WAKE_LOCK_CTRL_TIMEOUT_CANCEL((dhd_pub_t *)(cfg->pub));
			}
		}
	}
	return ret;
}
#else
static s32
wl_cfg80211_del_virtual_iface(struct wiphy *wiphy, bcm_struct_cfgdev *cfgdev)
{
	struct net_device *dev = NULL;
	struct ether_addr p2p_mac;
	struct bcm_cfg80211 *cfg = wiphy_priv(wiphy);
	s32 timeout = -1;
	s32 ret = 0;
	s32 index = -1;
	s32 type = -1;
#ifdef CUSTOM_SET_CPUCORE
	dhd_pub_t *dhd = (dhd_pub_t *)(cfg->pub);
#endif /* CUSTOM_SET_CPUCORE */
	WL_DBG(("Enter\n"));

#ifdef CUSTOM_SET_CPUCORE
	dhd->chan_isvht80 &= ~DHD_FLAG_P2P_MODE;
	if (!(dhd->chan_isvht80))
		dhd_set_cpucore(dhd, FALSE);
#endif /* CUSTOM_SET_CPUCORE */
#ifdef WL_CFG80211_P2P_DEV_IF
	if (cfgdev->iftype == NL80211_IFTYPE_P2P_DEVICE) {
#ifdef HW_NOT_DEL_P2P_WHEN_SUSPEND
		return wl_cfgp2p_del_p2p_disc_if(cfgdev, cfg);
#else
		if (dhd_download_fw_on_driverload) {
			return wl_cfgp2p_del_p2p_disc_if(cfgdev, cfg);
		} else {
			cfg->down_disc_if = TRUE;
			return 0;
		}
#endif
	}
#endif /* WL_CFG80211_P2P_DEV_IF */
	dev = cfgdev_to_wlc_ndev(cfgdev, cfg);

#ifdef WLAIBSS_MCHAN
	if (cfgdev == cfg->ibss_cfgdev)
		return bcm_cfg80211_del_ibss_if(wiphy, cfgdev);
#endif /* WLAIBSS_MCHAN */

#ifdef WL_VIRTUAL_APSTA
	if (cfgdev == cfg->bss_cfgdev)
		return wl_cfg80211_del_iface(wiphy, cfgdev);
#endif /* WL_VIRTUAL_APSTA */
	if ((index = wl_get_bssidx_by_wdev(cfg, cfgdev_to_wdev(cfgdev))) < 0) {
		WL_ERR(("Find p2p index from wdev failed\n"));
		return BCME_ERROR;
	}
	if (wl_check_dongle_idle(wiphy) != TRUE) {
		WL_ERR(("FW is busy to add interface"));
		return BCME_ERROR;
	}
	if (cfg->p2p_supported) {
		if (wl_cfgp2p_find_type(cfg, index, &type) != BCME_OK)
			return BCME_ERROR;
		memcpy(p2p_mac.octet, wl_to_p2p_bss_macaddr(cfg, type).octet, ETHER_ADDR_LEN);

		/* Clear GO_NEG_PHASE bit to take care of GO-NEG-FAIL cases
		 */
		WL_DBG(("P2P: GO_NEG_PHASE status cleared "));
		wl_clr_p2p_status(cfg, GO_NEG_PHASE);
		if (wl_cfgp2p_vif_created(cfg)) {
			if (wl_get_drv_status(cfg, SCANNING, dev)) {
				wl_notify_escan_complete(cfg, dev, true, true);
			}
			wldev_iovar_setint(dev, "mpc", 1);
			/* Delete pm_enable_work */
			wl_add_remove_pm_enable_work(cfg, WL_PM_WORKQ_DEL);

			/* for GC */
			if (wl_get_drv_status(cfg, DISCONNECTING, dev) &&
				(wl_get_mode_by_netdev(cfg, dev) != WL_MODE_AP)) {
				WL_ERR(("Wait for Link Down event for GC !\n"));
				wait_for_completion_timeout
					(&cfg->iface_disable, msecs_to_jiffies(500));
			}

			memset(&cfg->if_event_info, 0, sizeof(cfg->if_event_info));
			wl_set_p2p_status(cfg, IF_DELETING);
			DNGL_FUNC(dhd_cfg80211_clean_p2p_info, (cfg));

			/* for GO */
			if (wl_get_mode_by_netdev(cfg, dev) == WL_MODE_AP) {
				wl_add_remove_eventmsg(dev, WLC_E_PROBREQ_MSG, false);
				cfg->p2p->p2p_go_count--;
				/* disable interface before bsscfg free */
				ret = wl_cfgp2p_ifdisable(cfg, &p2p_mac);
				/* if fw doesn't support "ifdis",
				   do not wait for link down of ap mode
				 */
				if (ret == 0) {
					WL_ERR(("Wait for Link Down event for GO !!!\n"));
					wait_for_completion_timeout(&cfg->iface_disable,
						msecs_to_jiffies(500));
				} else if (ret != BCME_UNSUPPORTED) {
					msleep(300);
				}
			}
			wl_cfg80211_clear_per_bss_ies(cfg, index);

			if (wl_get_mode_by_netdev(cfg, dev) != WL_MODE_AP)
				wldev_iovar_setint(dev, "buf_key_b4_m4", 0);
			memcpy(p2p_mac.octet, wl_to_p2p_bss_macaddr(cfg, type).octet,
			ETHER_ADDR_LEN);
			CFGP2P_INFO(("primary idx %d : cfg p2p_ifdis "MACDBG"\n",
			       dev->ifindex, MAC2STRDBG(p2p_mac.octet)));

			/* delete interface after link down */
#ifdef HW_PCIE_STABILITY
			if (bcmcfg_to_prmry_wdev(cfg)) {
#endif
				ret = wl_cfgp2p_ifdel(cfg, &p2p_mac);
				if (ret != BCME_OK) {
					struct net_device *ndev = bcmcfg_to_prmry_ndev(cfg);
					dhd_pub_t *dhd = (dhd_pub_t *)(cfg->pub);

					WL_ERR(("p2p_ifdel failed, error %d, sent HANG event to %s\n",
						ret, ndev->name));
					dhd->hang_reason = HANG_REASON_P2P_IFACE_DEL_FAILURE;
					net_os_send_hang_message(ndev);
				} else {
					/* Wait for IF_DEL operation to be finished */
					timeout = wait_event_interruptible_timeout(cfg->netif_change_event,
						((wl_get_p2p_status(cfg, IF_DELETING) == false) &&
						(cfg->if_event_info.valid)),
						msecs_to_jiffies(MAX_WAIT_TIME));
					if (timeout > 0 && !wl_get_p2p_status(cfg, IF_DELETING) &&
						cfg->if_event_info.valid) {

						WL_DBG(("IFDEL operation done\n"));
						wl_cfg80211_handle_ifdel(cfg, &cfg->if_event_info, dev);
					} else {
						WL_ERR(("IFDEL didn't complete properly\n"));
					}
				}
#ifdef HW_PCIE_STABILITY
			}
#endif
			ret = dhd_del_monitor(dev);
			if (wl_get_mode_by_netdev(cfg, dev) == WL_MODE_AP) {
				DHD_OS_WAKE_LOCK_CTRL_TIMEOUT_CANCEL((dhd_pub_t *)(cfg->pub));
			}
		}
	}
	return ret;
}
#endif  /* BRCM_RSDB */

static s32
wl_cfg80211_change_virtual_iface(struct wiphy *wiphy, struct net_device *ndev,
	enum nl80211_iftype type, u32 *flags,
	struct vif_params *params)
{
	s32 ap = 0;
	s32 infra = 0;
	s32 ibss = 0;
	s32 wlif_type;
	s32 mode = 0;
	s32 err = BCME_OK;
#ifdef  BRCM_RSDB
	s32 index;
	s32 conn_idx = -1;
#endif
	chanspec_t chspec;
	struct bcm_cfg80211 *cfg = wiphy_priv(wiphy);
#ifdef  BRCM_RSDB
	struct net_device *primary_ndev = bcmcfg_to_prmry_ndev(cfg);
#endif
	dhd_pub_t *dhd = (dhd_pub_t *)(cfg->pub);

	WL_DBG(("Enter type %d\n", type));
	switch (type) {
	case NL80211_IFTYPE_MONITOR:
	case NL80211_IFTYPE_WDS:
	case NL80211_IFTYPE_MESH_POINT:
		ap = 1;
		WL_ERR(("type (%d) : currently we do not support this type\n",
			type));
		break;
	case NL80211_IFTYPE_ADHOC:
		mode = WL_MODE_IBSS;
		ibss = 1;
		break;
	case NL80211_IFTYPE_STATION:
	case NL80211_IFTYPE_P2P_CLIENT:
		mode = WL_MODE_BSS;
		infra = 1;
		break;
	case NL80211_IFTYPE_AP:
#ifdef  BRCM_RSDB
		dhd->op_mode |= DHD_FLAG_HOSTAP_MODE;
		/* intentional fall through */
#endif
	case NL80211_IFTYPE_AP_VLAN:
	case NL80211_IFTYPE_P2P_GO:
		mode = WL_MODE_AP;
		ap = 1;
		break;
	default:
		return -EINVAL;
	}
	if (!dhd)
		return -EINVAL;
#ifdef  BRCM_RSDB
	/* If any scan is going on, abort it */
	if (wl_get_drv_status_all(cfg, SCANNING)) {
		int wait_cnt = MAX_SCAN_ABORT_WAIT_CNT;
		WL_ERR(("Scan in progress. Aborting the scan!\n"));
		wl_cfg80211_scan_abort(cfg);
		while (wl_get_drv_status_all(cfg, SCANNING) && wait_cnt) {
			WL_DBG(("Waiting for SCANNING terminated, wait_cnt: %d\n", wait_cnt));
			wait_cnt--;
			OSL_SLEEP(WAIT_SCAN_ABORT_OSL_SLEEP_TIME);
		}
		if (wl_get_drv_status_all(cfg, SCANNING)) {
			wl_notify_escan_complete(cfg, cfg->escan_info.ndev, true, true);
		}
	}
#endif
	if (ap) {
		wl_set_mode_by_netdev(cfg, ndev, mode);
#ifndef  BRCM_RSDB
		if (cfg->p2p_supported && cfg->p2p->vif_created) {
			WL_DBG(("p2p_vif_created (%d) p2p_on (%d)\n", cfg->p2p->vif_created,
			p2p_on(cfg)));
#else
		if (is_p2p_group_iface(ndev->ieee80211_ptr) &&
			cfg->p2p && wl_cfgp2p_vif_created(cfg)) {
			WL_DBG(("p2p_vif_created p2p_on (%d)\n", p2p_on(cfg)));
			if (wl_check_dongle_idle(wiphy) != TRUE) {
				WL_ERR(("FW is busy to add interface"));
				return -EINVAL;
			}
#endif
			wldev_iovar_setint(ndev, "mpc", 0);
			wl_notify_escan_complete(cfg, ndev, true, true);
#ifdef  BRCM_RSDB
			/* Dual p2p doesn't support multiple P2PGO interfaces,
			 * p2p_go_count is the counter for GO creation
			 * requests.
			 */
			if ((cfg->p2p->p2p_go_count > 0) && (type == NL80211_IFTYPE_P2P_GO)) {
				wl_set_mode_by_netdev(cfg, ndev, WL_MODE_BSS);
				WL_ERR(("Fw doesnot support  multiple GO "));
				return BCME_ERROR;
			}
#endif
			/* In concurrency case, STA may be already associated in a particular
			 * channel. so retrieve the current channel of primary interface and
			 * then start the virtual interface on that.
			 */
			chspec = wl_cfg80211_get_shared_freq(wiphy);
#ifdef  BRCM_RSDB
			index = wl_get_bssidx_by_wdev(cfg, ndev->ieee80211_ptr);
			if (index < 0) {
				WL_ERR(("Find p2p index from ndev(%p) failed\n", ndev));
				return BCME_ERROR;
			}
			if (wl_cfgp2p_find_type(cfg, index, &conn_idx) != BCME_OK)
				return BCME_ERROR;
#endif
			wlif_type = WL_P2P_IF_GO;
#ifndef  BRCM_RSDB
			WL_ERR(("%s : ap (%d), infra (%d), iftype: (%d)\n",
				ndev->name, ap, infra, type));
#else
			WL_DBG(("%s : ap (%d), infra (%d), iftype (%d) conn_idx (%d)\n",
				ndev->name, ap, infra, type, conn_idx));
#endif
			wl_set_p2p_status(cfg, IF_CHANGING);
			wl_clr_p2p_status(cfg, IF_CHANGED);
#ifndef  BRCM_RSDB
			wl_cfgp2p_ifchange(cfg, &cfg->p2p->int_addr, htod32(wlif_type), chspec);
#else
			wl_cfgp2p_ifchange(cfg, wl_to_p2p_bss_macaddr(cfg, conn_idx),
				htod32(wlif_type), chspec, conn_idx);
#endif
			wait_event_interruptible_timeout(cfg->netif_change_event,
				(wl_get_p2p_status(cfg, IF_CHANGED) == true),
				msecs_to_jiffies(MAX_WAIT_TIME));
			wl_set_mode_by_netdev(cfg, ndev, mode);
			dhd->op_mode &= ~DHD_FLAG_P2P_GC_MODE;
			dhd->op_mode |= DHD_FLAG_P2P_GO_MODE;
			wl_clr_p2p_status(cfg, IF_CHANGING);
			wl_clr_p2p_status(cfg, IF_CHANGED);
			if (mode == WL_MODE_AP)
				wl_set_drv_status(cfg, CONNECTED, ndev);
#ifndef  BRCM_RSDB
		} else if (ndev == bcmcfg_to_prmry_ndev(cfg) &&
#else
		} else if (((ndev == primary_ndev) ||
			(ndev == ((struct net_device *)cfgdev_to_ndev(cfg->bss_cfgdev)))) &&
#endif
			!wl_get_drv_status(cfg, AP_CREATED, ndev)) {
			wl_set_drv_status(cfg, AP_CREATING, ndev);
#ifndef  BRCM_RSDB
			if (!cfg->ap_info &&
				!(cfg->ap_info = kzalloc(sizeof(struct ap_info), GFP_KERNEL))) {
				WL_ERR(("struct ap_saved_ie allocation failed\n"));
				return -ENOMEM;
			}
#endif
		} else {
			WL_ERR(("Cannot change the interface for GO or SOFTAP\n"));
			return -EINVAL;
		}
	} else {
#ifdef  BRCM_RSDB
		/* P2P GO interface deletion is handled on the basis of role type (AP).
		 * So avoid changing role for p2p type.
		 */
		if (ndev->ieee80211_ptr->iftype != NL80211_IFTYPE_P2P_GO)
			wl_set_mode_by_netdev(cfg, ndev, mode);
		WL_DBG(("Change_virtual_iface for transition from GO/AP to client/STA"));
#endif
	}

	if (ibss) {
		infra = 0;
		wl_set_mode_by_netdev(cfg, ndev, mode);
		err = wldev_ioctl(ndev, WLC_SET_INFRA, &infra, sizeof(s32), true);
		if (err < 0) {
			WL_ERR(("SET Adhoc error %d\n", err));
			return -EINVAL;
		}
	}

	ndev->ieee80211_ptr->iftype = type;
	return 0;
}

s32
wl_cfg80211_notify_ifadd(int ifidx, char *name, uint8 *mac, uint8 bssidx)
{
	bool ifadd_expected = FALSE;
	struct bcm_cfg80211 *cfg = g_bcm_cfg;

	if (cfg == NULL)
	{
		WL_ERR(("g_bcm_cfg is NULL, ignore wl_cfg80211_notify_ifadd\n"));
		return BCME_ERROR;
	}

	/* P2P may send WLC_E_IF_ADD and/or WLC_E_IF_CHANGE during IF updating ("p2p_ifupd")
	 * redirect the IF_ADD event to ifchange as it is not a real "new" interface
	 */
	if (wl_get_p2p_status(cfg, IF_CHANGING))
		return wl_cfg80211_notify_ifchange(ifidx, name, mac, bssidx);

	/* Okay, we are expecting IF_ADD (as IF_ADDING is true) */
	if (wl_get_p2p_status(cfg, IF_ADDING)) {
		ifadd_expected = TRUE;
		wl_clr_p2p_status(cfg, IF_ADDING);
	} else if (cfg->bss_pending_op) {
		ifadd_expected = TRUE;
		cfg->bss_pending_op = FALSE;
	}

	if (ifadd_expected) {
		wl_if_event_info *if_event_info = &cfg->if_event_info;

		if_event_info->valid = TRUE;
		if_event_info->ifidx = ifidx;
		if_event_info->bssidx = bssidx;
		strncpy(if_event_info->name, name, IFNAMSIZ);
		if_event_info->name[IFNAMSIZ] = '\0';
		if (mac)
			memcpy(if_event_info->mac, mac, ETHER_ADDR_LEN);
		wake_up_interruptible(&cfg->netif_change_event);
		return BCME_OK;
	}

	return BCME_ERROR;
}

s32
wl_cfg80211_notify_ifdel(int ifidx, char *name, uint8 *mac, uint8 bssidx)
{
	bool ifdel_expected = FALSE;
	struct bcm_cfg80211 *cfg = g_bcm_cfg;
	wl_if_event_info *if_event_info = &cfg->if_event_info;

	if (cfg == NULL)
	{
		WL_ERR(("cfg is NULL, ignore wl_cfg80211_notify_ifdel\n"));
		return BCME_ERROR;
	}

	if (wl_get_p2p_status(cfg, IF_DELETING)) {
		ifdel_expected = TRUE;
		wl_clr_p2p_status(cfg, IF_DELETING);
	} else if (cfg->bss_pending_op) {
		ifdel_expected = TRUE;
		cfg->bss_pending_op = FALSE;
	}

	if (ifdel_expected) {
		if_event_info->valid = TRUE;
		if_event_info->ifidx = ifidx;
		if_event_info->bssidx = bssidx;
		wake_up_interruptible(&cfg->netif_change_event);
		return BCME_OK;
	}

	return BCME_ERROR;
}

s32
wl_cfg80211_notify_ifchange(int ifidx, char *name, uint8 *mac, uint8 bssidx)
{
	struct bcm_cfg80211 *cfg = g_bcm_cfg;
	if (cfg == NULL)
	{
		WL_ERR(("cfg is NULL, ignore wl_cfg80211_notify_ifchange\n"));
		return BCME_ERROR;
	}
	if (wl_get_p2p_status(cfg, IF_CHANGING)) {
		wl_set_p2p_status(cfg, IF_CHANGED);
		wake_up_interruptible(&cfg->netif_change_event);
		return BCME_OK;
	}

	return BCME_ERROR;
}

static s32 wl_cfg80211_handle_ifdel(struct bcm_cfg80211 *cfg, wl_if_event_info *if_event_info,
	struct net_device* ndev)
{
	s32 type = -1;
	s32 bssidx = -1;
#ifdef PROP_TXSTATUS_VSDB
#if defined(BCMSDIO)
	dhd_pub_t *dhd =  (dhd_pub_t *)(cfg->pub);
	bool enabled = FALSE;
#endif
#endif /* PROP_TXSTATUS_VSDB */

	bssidx = if_event_info->bssidx;
#ifndef  BRCM_RSDB
	if (bssidx != wl_to_p2p_bss_bssidx(cfg, P2PAPI_BSSCFG_CONNECTION)) {
#else
	if (bssidx != wl_to_p2p_bss_bssidx(cfg, P2PAPI_BSSCFG_CONNECTION1) &&
		bssidx != wl_to_p2p_bss_bssidx(cfg, P2PAPI_BSSCFG_CONNECTION2)) {
#endif
		WL_ERR(("got IF_DEL for if %d, not owned by cfg driver\n", bssidx));
		return BCME_ERROR;
	}
#ifndef  BRCM_RSDB
	if (p2p_is_on(cfg) && cfg->p2p->vif_created) {
#else
	if (p2p_is_on(cfg) && wl_cfgp2p_vif_created(cfg)) {
#endif

		if (cfg->scan_request && (cfg->escan_info.ndev == ndev)) {
			/* Abort any pending scan requests */
			cfg->escan_info.escan_state = WL_ESCAN_STATE_IDLE;
			WL_DBG(("ESCAN COMPLETED\n"));
			wl_notify_escan_complete(cfg, cfg->escan_info.ndev, true, false);
		}

		memset(cfg->p2p->vir_ifname, '\0', IFNAMSIZ);
#ifndef  BRCM_RSDB
		if (wl_cfgp2p_find_type(cfg, bssidx, &type) != BCME_OK) {
			WL_ERR(("Find p2p type from bssidx(%d) failed\n", bssidx));
			return BCME_ERROR;
		}
		wl_clr_drv_status(cfg, CONNECTED, wl_to_p2p_bss_ndev(cfg, type));
		wl_to_p2p_bss_ndev(cfg, type) = NULL;
		wl_to_p2p_bss_bssidx(cfg, type) = WL_INVALID;
		cfg->p2p->vif_created = false;
#else
		if (wl_cfgp2p_find_type(cfg, bssidx, &type) == BCME_OK) {
			/* Update P2P data */
			wl_clr_drv_status(cfg, CONNECTED, wl_to_p2p_bss_ndev(cfg, type));
			wl_to_p2p_bss_ndev(cfg, type) = NULL;
			wl_to_p2p_bss_bssidx(cfg, type) = -1;
		} else if (wl_get_bssidx_by_wdev(cfg, ndev->ieee80211_ptr) < 0) {
			WL_ERR(("bssidx not known for the given ndev as per net_info data \n"));
			return BCME_ERROR;
		}
#endif

#ifdef PROP_TXSTATUS_VSDB
#if defined(BCMSDIO)
		dhd_wlfc_get_enable(dhd, &enabled);
		if (enabled && cfg->wlfc_on && dhd->op_mode != DHD_FLAG_HOSTAP_MODE &&
			dhd->op_mode != DHD_FLAG_IBSS_MODE) {
			dhd_wlfc_deinit(dhd);
			cfg->wlfc_on = false;
		}
#endif
#endif /* PROP_TXSTATUS_VSDB */
	}
#ifdef  BRCM_RSDB
	dhd_net_if_lock(ndev);
#endif
	wl_cfg80211_remove_if(cfg, if_event_info->ifidx, ndev);
#ifdef  BRCM_RSDB
	dhd_net_if_unlock(ndev);
#endif
	return BCME_OK;
}

/* Find listen channel */
static s32 wl_find_listen_channel(struct bcm_cfg80211 *cfg,
	const u8 *ie, u32 ie_len)
{
	wifi_p2p_ie_t *p2p_ie;
	u8 *end, *pos;
	s32 listen_channel;
#ifdef  BRCM_RSDB
/* unfortunately const cast required here - function is
 * a callback so its signature must not be changed
 * and cascade of changing wl_cfgp2p_find_p2pie
 * causes need for const cast in other places
 */
#if defined(STRICT_GCC_WARNINGS) && defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == \
	4 && __GNUC_MINOR__ >= 6))
_Pragma("GCC diagnostic push")
_Pragma("GCC diagnostic ignored \"-Wcast-qual\"")
#endif
#endif
	pos = (u8 *)ie;
#ifdef  BRCM_RSDB
#if defined(STRICT_GCC_WARNINGS) && defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == \
	4 && __GNUC_MINOR__ >= 6))
_Pragma("GCC diagnostic pop")
#endif
#endif
	p2p_ie = wl_cfgp2p_find_p2pie(pos, ie_len);

	if (p2p_ie == NULL)
		return 0;

	pos = p2p_ie->subelts;
	end = p2p_ie->subelts + (p2p_ie->len - 4);

	CFGP2P_DBG((" found p2p ie ! lenth %d \n",
		p2p_ie->len));

	while (pos < end) {
		uint16 attr_len;
		if (pos + 2 >= end) {
			CFGP2P_DBG((" -- Invalid P2P attribute"));
			return 0;
		}
		attr_len = ((uint16) (((pos + 1)[1] << 8) | (pos + 1)[0]));

		if (pos + 3 + attr_len > end) {
			CFGP2P_DBG(("P2P: Attribute underflow "
				   "(len=%u left=%d)",
				   attr_len, (int) (end - pos - 3)));
			return 0;
		}

		/* if Listen Channel att id is 6 and the vailue is valid,
		 * return the listen channel
		 */
		if (pos[0] == 6) {
			/* listen channel subel length format
			 * 1(id) + 2(len) + 3(country) + 1(op. class) + 1(chan num)
			 */
			listen_channel = pos[1 + 2 + 3 + 1];

			if (listen_channel == SOCIAL_CHAN_1 ||
				listen_channel == SOCIAL_CHAN_2 ||
				listen_channel == SOCIAL_CHAN_3) {
				CFGP2P_DBG((" Found my Listen Channel %d \n", listen_channel));
				return listen_channel;
			}
		}
		pos += 3 + attr_len;
	}
	return 0;
}

static void wl_scan_prep(struct wl_scan_params *params, struct cfg80211_scan_request *request)
{
	u32 n_ssids;
	u32 n_channels;
	u16 channel;
	chanspec_t chanspec;
	s32 i = 0, j = 0, offset;
	char *ptr;
	wlc_ssid_t ssid;
	struct bcm_cfg80211 *cfg = g_bcm_cfg;
#ifdef  BRCM_RSDB
	struct wireless_dev *wdev;
#endif
	if (cfg == NULL)
	{
		WL_ERR(("cfg is NULL, ignore wl_scan_prep\n"));
		return;
	}

	memcpy(&params->bssid, &ether_bcast, ETHER_ADDR_LEN);
	params->bss_type = DOT11_BSSTYPE_ANY;
	params->scan_type = 0;
	params->nprobes = -1;

#ifdef HW_WIFI_FIRST_SCAN_OPTIMIZE
	if(FALSE == g_dhd_init_flag){
		params->active_time = 40;
		g_dhd_init_flag = TRUE;
	} else{
		params->active_time = -1;
	}
	params->home_time = 50;
#else
	params->active_time = -1;
	params->home_time = -1;
#endif
	params->passive_time = -1;
	params->channel_num = 0;
	memset(&params->ssid, 0, sizeof(wlc_ssid_t));

#ifdef HW_SCAN_PARAMS_TIME
	params->nprobes = 2;
	params->active_time = 40;
	params->passive_time = 130;
	params->home_time = 45;
#endif

	WL_SCAN(("Preparing Scan request\n"));
	WL_SCAN(("nprobes=%d\n", params->nprobes));
	WL_SCAN(("active_time=%d\n", params->active_time));
	WL_SCAN(("passive_time=%d\n", params->passive_time));
	WL_SCAN(("home_time=%d\n", params->home_time));
	WL_SCAN(("scan_type=%d\n", params->scan_type));

	params->nprobes = htod32(params->nprobes);
	params->active_time = htod32(params->active_time);
	params->passive_time = htod32(params->passive_time);
	params->home_time = htod32(params->home_time);

	/* if request is null just exit so it will be all channel broadcast scan */
	if (!request)
		return;

	n_ssids = request->n_ssids;
	n_channels = request->n_channels;

	/* Copy channel array if applicable */
#ifdef HW_LOG_PATCH1
	hw_dhd_log("### List of channelspecs to scan ###");
	hw_dhd_looplog_start();
	hw_dhd_looplog("Chan: ");
#else
	WL_SCAN(("### List of channelspecs to scan ###\n"));
#endif
	if (n_channels > 0) {
		for (i = 0; i < n_channels; i++) {
			chanspec = 0;
			channel = ieee80211_frequency_to_channel(request->channels[i]->center_freq);
			/* SKIP DFS channels for Secondary interface */
			if ((cfg->escan_info.ndev != bcmcfg_to_prmry_ndev(cfg)) &&
				(request->channels[i]->flags &
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 14, 0))
				(IEEE80211_CHAN_RADAR | IEEE80211_CHAN_PASSIVE_SCAN)))
#else
				(IEEE80211_CHAN_RADAR | IEEE80211_CHAN_NO_IR)))
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(3, 14, 0) */
				continue;
#ifdef  BRCM_RSDB
#if defined(WL_CFG80211_P2P_DEV_IF)
			wdev = request->wdev;
#else
			wdev = request->dev->ieee80211_ptr;
#endif /* WL_CFG80211_P2P_DEV_IF */
			chanspec = wl_cfg80211_ulb_get_min_bw_chspec(wdev, -1);
			if (chanspec == INVCHANSPEC) {
				WL_ERR(("Invalid chanspec! Skipping channel\n"));
				continue;
			}
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0))
			if (request->channels[i]->band == NL80211_BAND_2GHZ) {
#else
			if (request->channels[i]->band == IEEE80211_BAND_2GHZ) {
#endif
#if defined(WL_HOST_BAND_MGMT) && defined(BRCM_RSDB)
				if (cfg->curr_band == WLC_BAND_5G) {
					WL_DBG(("In 5G only mode, omit 2G channel:%d\n", channel));
					continue;
				}
#endif /* WL_HOST_BAND_MGMT */
				chanspec |= WL_CHANSPEC_BAND_2G;
			} else {
#if defined(WL_HOST_BAND_MGMT) && defined(BRCM_RSDB)
				if (cfg->curr_band == WLC_BAND_2G) {
					WL_DBG(("In 2G only mode, omit 5G channel:%d\n", channel));
					continue;
				}
#endif /* WL_HOST_BAND_MGMT */
				chanspec |= WL_CHANSPEC_BAND_5G;
			}
#ifndef  BRCM_RSDB
			chanspec |= WL_CHANSPEC_BW_20;
			chanspec |= WL_CHANSPEC_CTL_SB_NONE;
#endif
			params->channel_list[j] = channel;
			params->channel_list[j] &= WL_CHANSPEC_CHAN_MASK;
			params->channel_list[j] |= chanspec;
#ifdef HW_LOG_PATCH1
			hw_dhd_looplog("%d ", channel);
#else
			WL_SCAN(("Chan : %d, Channel spec: %x \n",
				channel, params->channel_list[j]));
#endif
			params->channel_list[j] = wl_chspec_host_to_driver(params->channel_list[j]);
			j++;
		}
	} else {
		WL_SCAN(("Scanning all channels\n"));
	}
	n_channels = j;
	/* Copy ssid array if applicable */
#ifdef HW_LOG_PATCH1
	hw_dhd_looplog_end();
	hw_dhd_log("### List of SSIDs to scan ###");
	hw_dhd_looplog_start();
	hw_dhd_looplog("scan for: ");
#else
	WL_SCAN(("### List of SSIDs to scan ###\n"));
#endif
	if (n_ssids > 0) {
		offset = offsetof(wl_scan_params_t, channel_list) + n_channels * sizeof(u16);
		offset = roundup(offset, sizeof(u32));
		ptr = (char*)params + offset;
		for (i = 0; i < n_ssids; i++) {
			memset(&ssid, 0, sizeof(wlc_ssid_t));
#ifdef BCM_PATCH_CVE_2016_2493
			ssid.SSID_len = min(request->ssids[i].ssid_len, (u8)DOT11_MAX_SSID_LEN);
#else
			ssid.SSID_len = request->ssids[i].ssid_len;
#endif
			memcpy(ssid.SSID, request->ssids[i].ssid, ssid.SSID_len);
			if (!ssid.SSID_len)
#ifdef HW_LOG_PATCH1
				hw_dhd_looplog("Broadcast scan,");
#else
				WL_SCAN(("%d: Broadcast scan\n", i));
#endif
			else
//#ifdef HW_LOG_PATCH1
//				hw_dhd_looplog("%s,", ssid.SSID);
//#else
				WL_ERR(("%d: scan  for  %s size =%d\n", i,
				ssid.SSID, ssid.SSID_len));
//#endif
			memcpy(ptr, &ssid, sizeof(wlc_ssid_t));
			ptr += sizeof(wlc_ssid_t);
		}
	} else {
#ifdef HW_LOG_PATCH1
		hw_dhd_looplog("Broadcast scan");
#else
		WL_SCAN(("Broadcast scan\n"));
#endif
	}
#ifdef HW_LOG_PATCH1
		hw_dhd_looplog_end();
#endif
	/* Adding mask to channel numbers */
	params->channel_num =
	        htod32((n_ssids << WL_SCAN_PARAMS_NSSID_SHIFT) |
	               (n_channels & WL_SCAN_PARAMS_COUNT_MASK));

	if (n_channels == 1) {
		params->active_time = htod32(WL_SCAN_CONNECT_DWELL_TIME_MS);
		params->nprobes = htod32(params->active_time / WL_SCAN_JOIN_PROBE_INTERVAL_MS);
	}
}

static s32
wl_get_valid_channels(struct net_device *ndev, u8 *valid_chan_list, s32 size)
{
	wl_uint32_list_t *list;
	s32 err = BCME_OK;
	if (valid_chan_list == NULL || size <= 0)
		return -ENOMEM;

	memset(valid_chan_list, 0, size);
	list = (wl_uint32_list_t *)(void *) valid_chan_list;
	list->count = htod32(WL_NUMCHANNELS);
	err = wldev_ioctl(ndev, WLC_GET_VALID_CHANNELS, valid_chan_list, size, false);
	if (err != 0) {
		WL_ERR(("get channels failed with %d\n", err));
	}

	return err;
}

#if defined(USE_INITIAL_2G_SCAN) || defined(USE_INITIAL_SHORT_DWELL_TIME)
#define FIRST_SCAN_ACTIVE_DWELL_TIME_MS 40
bool g_first_broadcast_scan = TRUE;
#endif /* USE_INITIAL_2G_SCAN || USE_INITIAL_SHORT_DWELL_TIME */

static s32
wl_run_escan(struct bcm_cfg80211 *cfg, struct net_device *ndev,
	struct cfg80211_scan_request *request, uint16 action)
{
	scb_val_t scbval;
	s32 err = BCME_OK;
	u32 n_channels;
	u32 n_ssids;
	s32 params_size = (WL_SCAN_PARAMS_FIXED_SIZE + OFFSETOF(wl_escan_params_t, params));
	wl_escan_params_t *params = NULL;
	u8 chan_buf[sizeof(u32)*(WL_NUMCHANNELS + 1)];
	u32 num_chans = 0;
	s32 channel;
#ifndef  BRCM_RSDB
	s32 n_valid_chan;
#else
	u32 n_valid_chan;
#endif
	s32 search_state = WL_P2P_DISC_ST_SCAN;
	u32 i, j, n_nodfs = 0;
	u16 *default_chan_list = NULL;
	wl_uint32_list_t *list;
#ifdef  BRCM_RSDB
	s32 bssidx = -1;
#endif
	struct net_device *dev = NULL;
#ifndef  BRCM_RSDB
	dhd_pub_t *dhd = (dhd_pub_t *)(cfg->pub);
#endif
#if defined(USE_INITIAL_2G_SCAN) || defined(USE_INITIAL_SHORT_DWELL_TIME)
	bool is_first_init_2g_scan = false;
#endif /* USE_INITIAL_2G_SCAN || USE_INITIAL_SHORT_DWELL_TIME */
	p2p_scan_purpose_t	p2p_scan_purpose = P2P_SCAN_PURPOSE_MIN;
	static int cnt = 0;

	WL_DBG(("Enter \n"));

	/* scan request can come with empty request : perform all default scan */
	if (!cfg) {
		err = -EINVAL;
		goto exit;
	}
	if (!cfg->p2p_supported || !p2p_scan(cfg)) {
		/* LEGACY SCAN TRIGGER */
		WL_ERR((" LEGACY E-SCAN START\n"));

#if defined(USE_INITIAL_2G_SCAN) || defined(USE_INITIAL_SHORT_DWELL_TIME)
		if (!request) {
			err = -EINVAL;
			goto exit;
		}
		if (ndev == bcmcfg_to_prmry_ndev(cfg) && g_first_broadcast_scan == true) {
#ifdef USE_INITIAL_2G_SCAN
			struct ieee80211_channel tmp_channel_list[CH_MAX_2G_CHANNEL];
			/* allow one 5G channel to add previous connected channel in 5G */
			bool allow_one_5g_channel = TRUE;
			j = 0;
			for (i = 0; i < request->n_channels; i++) {
				int tmp_chan = ieee80211_frequency_to_channel
					(request->channels[i]->center_freq);
				if (tmp_chan > CH_MAX_2G_CHANNEL) {
					if (allow_one_5g_channel)
						allow_one_5g_channel = FALSE;
					else
						continue;
				}
				if (j > CH_MAX_2G_CHANNEL) {
					WL_ERR(("Index %d exceeds max 2.4GHz channels %d"
						" and previous 5G connected channel\n",
						j, CH_MAX_2G_CHANNEL));
					break;
				}
				bcopy(request->channels[i], &tmp_channel_list[j],
					sizeof(struct ieee80211_channel));
				WL_SCAN(("channel of request->channels[%d]=%d\n", i, tmp_chan));
				j++;
			}
			if ((j > 0) && (j <= CH_MAX_2G_CHANNEL)) {
				for (i = 0; i < j; i++)
					bcopy(&tmp_channel_list[i], request->channels[i],
						sizeof(struct ieee80211_channel));

				request->n_channels = j;
				is_first_init_2g_scan = true;
			}
			else
				WL_ERR(("Invalid number of 2.4GHz channels %d\n", j));

			WL_SCAN(("request->n_channels=%d\n", request->n_channels));
#else /* USE_INITIAL_SHORT_DWELL_TIME */
			is_first_init_2g_scan = true;
#endif /* USE_INITIAL_2G_SCAN */
			g_first_broadcast_scan = false;
		}
#endif /* USE_INITIAL_2G_SCAN || USE_INITIAL_SHORT_DWELL_TIME */

		/* if scan request is not empty parse scan request paramters */
		if (request != NULL) {
			n_channels = request->n_channels;
			n_ssids = request->n_ssids;
			if (n_channels % 2)
				/* If n_channels is odd, add a padd of u16 */
				params_size += sizeof(u16) * (n_channels + 1);
			else
				params_size += sizeof(u16) * n_channels;

			/* Allocate space for populating ssids in wl_escan_params_t struct */
			params_size += sizeof(struct wlc_ssid) * n_ssids;
		}
		params = (wl_escan_params_t *) kzalloc(params_size, GFP_KERNEL);
		if (params == NULL) {
			err = -ENOMEM;
			goto exit;
		}
		wl_scan_prep(&params->params, request);

#if defined(USE_INITIAL_2G_SCAN) || defined(USE_INITIAL_SHORT_DWELL_TIME)
		/* Override active_time to reduce scan time if it's first bradcast scan. */
		if (is_first_init_2g_scan)
			params->params.active_time = FIRST_SCAN_ACTIVE_DWELL_TIME_MS;
#endif /* USE_INITIAL_2G_SCAN || USE_INITIAL_SHORT_DWELL_TIME */

		params->version = htod32(ESCAN_REQ_VERSION);
		params->action =  htod16(action);
		wl_escan_set_sync_id(params->sync_id, cfg);
		wl_escan_set_type(cfg, WL_SCANTYPE_LEGACY);
		if (params_size + sizeof("escan") >= WLC_IOCTL_MEDLEN) {
			WL_ERR(("ioctl buffer length not sufficient\n"));
			kfree(params);
			err = -ENOMEM;
			goto exit;
		}
#ifdef  BRCM_RSDB
		if (cfg->active_scan == PASSIVE_SCAN) {
			params->params.scan_type = DOT11_SCANTYPE_PASSIVE;
			WL_DBG(("Passive scan_type %d \n", params->params.scan_type));
		}

		bssidx = wl_get_bssidx_by_wdev(cfg, ndev->ieee80211_ptr);
#endif
		err = wldev_iovar_setbuf(ndev, "escan", params, params_size,
			cfg->escan_ioctl_buf, WLC_IOCTL_MEDLEN, NULL);
#ifdef  BRCM_RSDB
		WL_ERR(("LEGACY_SCAN sync ID: %d, bssidx: %d\n", params->sync_id, bssidx));
#endif
		if (unlikely(err)) {
			if (err == BCME_EPERM)
				/* Scan Not permitted at this point of time */
				WL_DBG((" Escan not permitted at this time (%d)\n", err));
			else
				WL_ERR((" Escan set error (%d)\n", err));
#ifndef  BRCM_RSDB
		} else {
			DBG_EVENT_LOG(dhd, WIFI_EVENT_DRIVER_SCAN_REQUESTED);
#endif
		}
		kfree(params);
	}
	else if (p2p_is_on(cfg) && p2p_scan(cfg)) {
		/* P2P SCAN TRIGGER */
		s32 _freq = 0;
		n_nodfs = 0;
		if (request && request->n_channels) {
			num_chans = request->n_channels;
			WL_ERR((" chann number : %d\n", num_chans));
			default_chan_list = kzalloc(num_chans * sizeof(*default_chan_list),
				GFP_KERNEL);
			if (default_chan_list == NULL) {
				WL_ERR(("channel list allocation failed \n"));
				err = -ENOMEM;
				goto exit;
			}
			if (!wl_get_valid_channels(ndev, chan_buf, sizeof(chan_buf))) {
#if defined(P2P_SKIP_DFS) && defined(BRCM_RSDB)
				int is_printed = false;
#endif /* P2P_SKIP_DFS */
				list = (wl_uint32_list_t *) chan_buf;
				n_valid_chan = dtoh32(list->count);
#ifdef BCM_PATCH_SECURITY_2017_04
				if (n_valid_chan > WL_NUMCHANNELS) {
					WL_ERR(("wrong n_valid_chan:%d\n",
						n_valid_chan));
					kfree(default_chan_list);
					err = -EINVAL;
					goto exit;
				}
#endif
				for (i = 0; i < num_chans; i++)
				{
#if defined(WL_HOST_BAND_MGMT) && defined(BRCM_RSDB)
					int channel_band = 0;
#endif /* WL_HOST_BAND_MGMT */
					_freq = request->channels[i]->center_freq;
					channel = ieee80211_frequency_to_channel(_freq);
#if defined(WL_HOST_BAND_MGMT) && defined(BRCM_RSDB)
					channel_band = (channel > CH_MAX_2G_CHANNEL) ?
						WLC_BAND_5G : WLC_BAND_2G;
					if ((cfg->curr_band != WLC_BAND_AUTO) &&
						(cfg->curr_band != channel_band) &&
						!IS_P2P_SOCIAL_CHANNEL(channel))
							continue;
#endif /* WL_HOST_BAND_MGMT */

					/* ignore DFS channels */
					if (request->channels[i]->flags &
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0))
						(IEEE80211_CHAN_NO_IR
						| IEEE80211_CHAN_RADAR))
#else
						(IEEE80211_CHAN_RADAR
						| IEEE80211_CHAN_PASSIVE_SCAN))
#endif
						continue;
#if defined(P2P_SKIP_DFS) && defined(BRCM_RSDB)
					if (channel >= 52 && channel <= 144) {
						if (is_printed == false) {
							WL_ERR(("SKIP DFS CHANs(52~144)\n"));
							is_printed = true;
						}
						continue;
					}
#endif /* P2P_SKIP_DFS */

					for (j = 0; j < n_valid_chan; j++) {
						/* allows only supported channel on
						*  current reguatory
						*/
#ifdef BCM_PATCH_SECURITY_2017_04
						if (n_nodfs >= num_chans)
							break;
#endif
						if (channel == (dtoh32(list->element[j])))
							default_chan_list[n_nodfs++] =
								channel;
					}

				}
			}
			if (num_chans == SOCIAL_CHAN_CNT && (
						(default_chan_list[0] == SOCIAL_CHAN_1) &&
						(default_chan_list[1] == SOCIAL_CHAN_2) &&
						(default_chan_list[2] == SOCIAL_CHAN_3))) {
				/* SOCIAL CHANNELS 1, 6, 11 */
				search_state = WL_P2P_DISC_ST_SEARCH;
				p2p_scan_purpose = P2P_SCAN_SOCIAL_CHANNEL;
				WL_INFORM(("P2P SEARCH PHASE START \n"));
#ifndef  BRCM_RSDB
			} else if ((dev = wl_to_p2p_bss_ndev(cfg, P2PAPI_BSSCFG_CONNECTION)) &&
				(wl_get_mode_by_netdev(cfg, dev) == WL_MODE_AP)) {
#else
			} else if (((dev = wl_to_p2p_bss_ndev(cfg, P2PAPI_BSSCFG_CONNECTION1)) &&
				(wl_get_mode_by_netdev(cfg, dev) == WL_MODE_AP)) ||
				((dev = wl_to_p2p_bss_ndev(cfg, P2PAPI_BSSCFG_CONNECTION2)) &&
				(wl_get_mode_by_netdev(cfg, dev) == WL_MODE_AP))) {
#endif
				/* If you are already a GO, then do SEARCH only */
				WL_INFORM(("Already a GO. Do SEARCH Only"));
				search_state = WL_P2P_DISC_ST_SEARCH;
				num_chans = n_nodfs;
				p2p_scan_purpose = P2P_SCAN_NORMAL;

			} else if (num_chans == 1) {
				p2p_scan_purpose = P2P_SCAN_CONNECT_TRY;
			} else if (num_chans == SOCIAL_CHAN_CNT + 1) {
			/* SOCIAL_CHAN_CNT + 1 takes care of the Progressive scan supported by
			 * the supplicant
			 */
				p2p_scan_purpose = P2P_SCAN_SOCIAL_CHANNEL;
			} else {
				WL_INFORM(("P2P SCAN STATE START \n"));
				num_chans = n_nodfs;
				p2p_scan_purpose = P2P_SCAN_NORMAL;
			}
		} else {
			err = -EINVAL;
			goto exit;
		}
#ifndef  BRCM_RSDB
		err = wl_cfgp2p_escan(cfg, ndev, cfg->active_scan, num_chans, default_chan_list,
#else
		err = wl_cfgp2p_escan(cfg, ndev, ACTIVE_SCAN, num_chans, default_chan_list,
#endif
			search_state, action,
			wl_to_p2p_bss_bssidx(cfg, P2PAPI_BSSCFG_DEVICE), NULL,
			p2p_scan_purpose);

		if (!err)
			cfg->p2p->search_state = search_state;

		kfree(default_chan_list);
	}
exit:
	if (unlikely(err)) {
		/* Don't print Error incase of Scan suppress */
		if ((err == BCME_EPERM) && cfg->scan_suppressed)
			WL_DBG(("Escan failed: Scan Suppressed \n"));
		else{
			if (cfg) {
				cnt++;
				WL_ERR(("error (%d), cnt=%d\n", err, cnt));
				if (cnt >= 4) {
					dev = bcmcfg_to_prmry_ndev(cfg);
					memset(&scbval, 0, sizeof(scb_val_t));
					wldev_ioctl(dev, WLC_DISASSOC, &scbval, sizeof(scb_val_t), true);
					WL_ERR(("Send disassoc to break the busy dev=%p\n", dev));
					cnt = 0;
				}
			} else{
				WL_ERR(("cfg is null can not to send disassoc"));
			}
		}
	}
	else {
		cnt = 0;
	}
	return err;
}


static s32
wl_do_escan(struct bcm_cfg80211 *cfg, struct wiphy *wiphy, struct net_device *ndev,
	struct cfg80211_scan_request *request)
{
	s32 err = BCME_OK;
	s32 passive_scan;
#ifdef HW_PATCH_FOR_HANG
	dhd_pub_t *dhd = NULL;
#endif
#ifdef  BRCM_RSDB
	s32 passive_scan_time;
	s32 passive_scan_time_org;
#endif
	wl_scan_results_t *results;
	WL_SCAN(("Enter \n"));
#ifndef  BRCM_RSDB
	mutex_lock(&cfg->usr_sync);
#endif

	results = wl_escan_get_buf(cfg, FALSE);
	results->version = 0;
	results->count = 0;
	results->buflen = WL_SCAN_RESULTS_FIXED_SIZE;

	cfg->escan_info.ndev = ndev;
	cfg->escan_info.wiphy = wiphy;
	cfg->escan_info.escan_state = WL_ESCAN_STATE_SCANING;
	passive_scan = cfg->active_scan ? 0 : 1;
	err = wldev_ioctl(ndev, WLC_SET_PASSIVE_SCAN,
		&passive_scan, sizeof(passive_scan), true);
	if (unlikely(err)) {
		WL_ERR(("error (%d)\n", err));
		goto exit;
	}
#ifdef  BRCM_RSDB
	if (passive_channel_skip) {

		err = wldev_ioctl(ndev, WLC_GET_SCAN_PASSIVE_TIME,
			&passive_scan_time_org, sizeof(passive_scan_time_org), false);
		if (unlikely(err)) {
			WL_ERR(("== error (%d)\n", err));
			goto exit;
		}

		WL_SCAN(("PASSIVE SCAN time : %d \n", passive_scan_time_org));

		passive_scan_time = 0;
		err = wldev_ioctl(ndev, WLC_SET_SCAN_PASSIVE_TIME,
			&passive_scan_time, sizeof(passive_scan_time), true);
		if (unlikely(err)) {
			WL_ERR(("== error (%d)\n", err));
			goto exit;
		}

		WL_SCAN(("PASSIVE SCAN SKIPED!! (passive_channel_skip:%d) \n",
			passive_channel_skip));
	}
#endif
	err = wl_run_escan(cfg, ndev, request, WL_SCAN_ACTION_START);
#ifdef HW_PATCH_FOR_HANG
	if (hw_need_hang_with_scanbusy(err)) {
		hw_need_hang_with_scanbusy(BCME_OK); //reset counter to zero
		dhd = (dhd_pub_t *)(cfg->pub);
		if (dhd) {
			WL_ERR(("SCAN failed with BCME_BUSY. send hang event\n"));
#ifdef BRCM_RSDB
			dhd->hang_reason = HANG_REASON_WLC_MEMORY_LEAK;
#endif
			dhd_os_send_hang_message(dhd);
#ifdef HW_WIFI_DMD_LOG
			hw_wifi_dsm_client_notify(DSM_WIFI_WLC_SCAN_ERROR, "SCAN failed with BCME_BUSY\n");
#endif
		}
		goto exit;
	}
#endif /* HW_PATCH_FOR_HANG */
#ifdef HW_PATCH_SCAN_RETRY_WLC_UP
        if (BCME_NOTUP == err) {
            s32 val = 1;
            s32 e = 0;
            WL_ERR(("scan error for WLC NOTUP(%d), try to force it up.\n", err));
            e = wldev_ioctl(ndev, WLC_UP, &val, sizeof(s32), true);
            if (e < 0) {
                WL_ERR(("WLC_UP error %d\n", e));
                goto exit;
            }
            WL_ERR(("WLC UP ok, retry to scan.\n"));
            err = wl_run_escan(cfg, ndev, request, WL_SCAN_ACTION_START);
            if (BCME_NOTUP == err) {
                dhd_pub_t *dhd = (dhd_pub_t *)(cfg->pub);
                if (dhd) {
                    WL_ERR(("SCAN failed with WLC_DOWN. send hang event\n"));
#ifdef BRCM_RSDB
                    dhd->hang_reason = HANG_REASON_WLC_DOWN;
#endif
                    dhd_os_send_hang_message(dhd);
#ifdef HW_WIFI_DMD_LOG
                    hw_wifi_dsm_client_notify(DSM_WIFI_WLC_SCAN_ERROR, "SCAN failed with WLC_DOWN\n");
#endif
                }
                goto exit;
            }
        }
#endif
#ifdef  BRCM_RSDB
	if (passive_channel_skip) {
		err = wldev_ioctl(ndev, WLC_SET_SCAN_PASSIVE_TIME,
			&passive_scan_time_org, sizeof(passive_scan_time_org), true);
		if (unlikely(err)) {
			WL_ERR(("== error (%d)\n", err));
			goto exit;
		}
	WL_SCAN(("PASSIVE SCAN RECOVERED!! (passive_scan_time_org:%d) \n",
			passive_scan_time_org));
	}
#endif
exit:
#ifndef  BRCM_RSDB
	mutex_unlock(&cfg->usr_sync);
#endif
	return err;
}
#ifdef  BRCM_RSDB
static s32
__wl_cfg80211_scan(struct wiphy *wiphy, struct net_device *ndev,
	struct cfg80211_scan_request *request,
	struct cfg80211_ssid *this_ssid)
{
	struct bcm_cfg80211 *cfg = wiphy_priv(wiphy);
	struct cfg80211_ssid *ssids;
	struct ether_addr primary_mac;
	bool p2p_ssid;
#ifdef WL11U
	bcm_tlv_t *interworking_ie;
#endif
	s32 err = 0;
	s32 bssidx = -1;
	s32 i;

	unsigned long flags;
	static s32 busy_count = 0;
#ifdef WL_CFG80211_VSDB_PRIORITIZE_SCAN_REQUEST
	struct net_device *remain_on_channel_ndev = NULL;
#endif
	uint scan_timer_interval_ms = WL_SCAN_TIMER_INTERVAL_MS;

	/*
	 * Hostapd triggers scan before starting automatic channel selection
	 * to collect channel characteristics. However firmware scan engine
	 * doesn't support any channel characteristics collection along with
	 * scan. Hence return scan success.
	 */
	if (request && (scan_req_iftype(request) == NL80211_IFTYPE_AP)) {
		WL_INFORM(("Scan Command on SoftAP Interface. Ignoring...\n"));
		return 0;
	}

	ndev = ndev_to_wlc_ndev(ndev, cfg);

	if (WL_DRV_STATUS_SENDING_AF_FRM_EXT(cfg)) {
		WL_ERR(("Sending Action Frames. Try it again.\n"));
		return -EAGAIN;
	}

	WL_DBG(("Enter wiphy (%p)\n", wiphy));
	if (wl_get_drv_status_all(cfg, SCANNING)) {
		if (cfg->scan_request == NULL) {
			wl_clr_drv_status_all(cfg, SCANNING);
			WL_DBG(("<<<<<<<<<<<Force Clear Scanning Status>>>>>>>>>>>\n"));
		} else {
			WL_ERR(("Scanning already\n"));
			return -EAGAIN;
		}
	}
	if (wl_get_drv_status(cfg, SCAN_ABORTING, ndev)) {
		WL_ERR(("Scanning being aborted\n"));
		return -EAGAIN;
	}
	if (request && request->n_ssids > WL_SCAN_PARAMS_SSID_MAX) {
		WL_ERR(("request null or n_ssids > WL_SCAN_PARAMS_SSID_MAX\n"));
		return -EOPNOTSUPP;
	}

#ifdef P2P_LISTEN_OFFLOADING
	if (wl_get_p2p_status(cfg, DISC_IN_PROGRESS)) {
		WL_ERR(("P2P_FIND: Discovery offload is in progress\n"));
		return -EAGAIN;
	}
#endif /* P2P_LISTEN_OFFLOADING */

#ifdef WL_CFG80211_VSDB_PRIORITIZE_SCAN_REQUEST
	remain_on_channel_ndev = wl_cfg80211_get_remain_on_channel_ndev(cfg);
	if (remain_on_channel_ndev) {
		WL_DBG(("Remain_on_channel bit is set, somehow it didn't get cleared\n"));
		wl_notify_escan_complete(cfg, remain_on_channel_ndev, true, true);
	}
#endif /* WL_CFG80211_VSDB_PRIORITIZE_SCAN_REQUEST */

#ifdef WES_SUPPORT
#ifdef CUSTOMER_SCAN_TIMEOUT_SETTING
	if ((cfg->custom_scan_channel_time > DEFAULT_SCAN_CHANNEL_TIME) |
			(cfg->custom_scan_home_time > DEFAULT_SCAN_HOME_TIME) |
			(cfg->custom_scan_home_away_time > DEFAULT_SCAN_HOME_AWAY_TIME)) {
		scan_timer_interval_ms = CUSTOMER_WL_SCAN_TIMER_INTERVAL_MS;
	} else {
		scan_timer_interval_ms = WL_SCAN_TIMER_INTERVAL_MS;
	}
	WL_TRACE_HW4(("scan_timer_interval_ms %d\n", scan_timer_interval_ms));
#endif /* CUSTOMER_SCAN_TIMEOUT_SETTING */
#endif /* WES_SUPPORT */

	/* Arm scan timeout timer */
	mod_timer(&cfg->scan_timeout, jiffies + msecs_to_jiffies(scan_timer_interval_ms));
	if (request) {		/* scan bss */
		ssids = request->ssids;
		p2p_ssid = false;
		for (i = 0; i < request->n_ssids; i++) {
#ifdef BCM_PATCH_HIDDEN_DIRECT_SCAN
			if (ssids[i].ssid_len &&
				IS_P2P_SSID(ssids[i].ssid, ssids[i].ssid_len) &&
				request->wdev && (request->wdev->iftype != NL80211_IFTYPE_STATION)) {
#else
			if (ssids[i].ssid_len &&
				IS_P2P_SSID(ssids[i].ssid, ssids[i].ssid_len)) {
#endif
				p2p_ssid = true;
				break;
			}
		}
		if (p2p_ssid) {
			if (cfg->p2p_supported) {
				/* p2p scan trigger */
				if (p2p_on(cfg) == false) {
					/* p2p on at the first time */
					p2p_on(cfg) = true;
					wl_cfgp2p_set_firm_p2p(cfg);
					get_primary_mac(cfg, &primary_mac);
					wl_cfgp2p_generate_bss_mac(cfg, &primary_mac);
#if defined(P2P_IE_MISSING_FIX)
					cfg->p2p_prb_noti = false;
#endif
				}
				wl_clr_p2p_status(cfg, GO_NEG_PHASE);
				WL_DBG(("P2P: GO_NEG_PHASE status cleared \n"));
				p2p_scan(cfg) = true;
			}
		} else {
			/* legacy scan trigger
			 * So, we have to disable p2p discovery if p2p discovery is on
			 */
			if (cfg->p2p_supported) {
				p2p_scan(cfg) = false;
				/* If Netdevice is not equals to primary and p2p is on
				*  , we will do p2p scan using P2PAPI_BSSCFG_DEVICE.
				*/

				if (p2p_scan(cfg) == false) {
					if (wl_get_p2p_status(cfg, DISCOVERY_ON)) {
						err = wl_cfgp2p_discover_enable_search(cfg,
						false);
						if (unlikely(err)) {
							goto scan_out;
						}

					}
				}
			}
			if (!cfg->p2p_supported || !p2p_scan(cfg)) {
				if ((bssidx = wl_get_bssidx_by_wdev(cfg,
					ndev->ieee80211_ptr)) < 0) {
					WL_ERR(("Find p2p index from ndev(%p) failed\n",
						ndev));
					err = BCME_ERROR;
					goto scan_out;
				}
#ifdef WL11U
				if ((interworking_ie = wl_cfg80211_find_interworking_ie(
					(u8 *)request->ie, request->ie_len)) != NULL) {
					err = wl_cfg80211_add_iw_ie(cfg, ndev, bssidx,
					       VNDR_IE_CUSTOM_FLAG, interworking_ie->id,
					       interworking_ie->data, interworking_ie->len);

					if (unlikely(err)) {
						WL_ERR(("Failed to add interworking IE"));
					}
				} else if (cfg->iw_ie_len != 0) {
				/* we have to clear IW IE and disable gratuitous APR */
					wl_cfg80211_add_iw_ie(cfg, ndev, bssidx,
						VNDR_IE_CUSTOM_FLAG,
						DOT11_MNG_INTERWORKING_ID,
						0, 0);

					(void)wldev_iovar_setint_bsscfg(ndev, "grat_arp", 0,
						bssidx);
					cfg->wl11u = FALSE;
					cfg->iw_ie_len = 0;
					memset(cfg->iw_ie, 0, IW_IES_MAX_BUF_LEN);
					/* we don't care about error */
				}
#endif /* WL11U */
				err = wl_cfg80211_set_mgmt_vndr_ies(cfg, ndev_to_cfgdev(ndev),
					bssidx, VNDR_IE_PRBREQ_FLAG, request->ie,
					request->ie_len);

				if (unlikely(err)) {
					goto scan_out;
				}

			}
		}
	} else {		/* scan in ibss */
		ssids = this_ssid;
	}

	if (request && cfg->p2p_supported && !p2p_scan(cfg)) {
		WL_TRACE_HW4(("START SCAN\n"));
		DHD_OS_SCAN_WAKE_LOCK_TIMEOUT((dhd_pub_t *)(cfg->pub),
			SCAN_WAKE_LOCK_TIMEOUT);
		//DHD_DISABLE_RUNTIME_PM((dhd_pub_t *)(cfg->pub));
	}

	if (cfg->p2p_supported) {
		if (p2p_on(cfg) && p2p_scan(cfg)) {

			/* find my listen channel */
			cfg->afx_hdl->my_listen_chan =
				wl_find_listen_channel(cfg, request->ie,
				request->ie_len);
			err = wl_cfgp2p_enable_discovery(cfg, ndev,
			request->ie, request->ie_len);

			if (unlikely(err)) {
				goto scan_out;
			}
		}
	}
	err = wl_do_escan(cfg, wiphy, ndev, request);
	if (likely(!err))
		goto scan_success;
	else
		goto scan_out;

scan_success:
	busy_count = 0;
	cfg->scan_request = request;
	wl_set_drv_status(cfg, SCANNING, ndev);

	return 0;

scan_out:
#ifdef HW_READ_FW_LOG
	dhd_read_console_ex((dhd_pub_t *)cfg->pub);
#endif
	if (err == BCME_BUSY || err == BCME_NOTREADY) {
		WL_ERR(("Scan err = (%d), busy?%d", err, -EBUSY));
		err = -EBUSY;
	} else if ((err == BCME_EPERM) && cfg->scan_suppressed) {
		WL_ERR(("Scan not permitted due to scan suppress\n"));
		err = -EPERM;
	} else {
		/* For all other fw errors, use a generic error code as return
		 * value to cfg80211 stack
		 */
		err = -EAGAIN;
	}

#define SCAN_EBUSY_RETRY_LIMIT 20
	if (err == -EBUSY) {
		if (busy_count++ > SCAN_EBUSY_RETRY_LIMIT) {
			struct ether_addr bssid;
			s32 ret = 0;
#if defined(DHD_DEBUG) && defined(BCMPCIE) && defined(DHD_FW_COREDUMP)
			dhd_pub_t *dhdp = (dhd_pub_t *)(cfg->pub);
#endif /* DHD_DEBUG && BCMPCIE && DHD_FW_COREDUMP */
			busy_count = 0;
			WL_ERR(("Unusual continuous EBUSY error, %d %d %d %d %d %d %d %d %d\n",
				wl_get_drv_status(cfg, SCANNING, ndev),
				wl_get_drv_status(cfg, SCAN_ABORTING, ndev),
				wl_get_drv_status(cfg, CONNECTING, ndev),
				wl_get_drv_status(cfg, CONNECTED, ndev),
				wl_get_drv_status(cfg, DISCONNECTING, ndev),
				wl_get_drv_status(cfg, AP_CREATING, ndev),
				wl_get_drv_status(cfg, AP_CREATED, ndev),
				wl_get_drv_status(cfg, SENDING_ACT_FRM, ndev),
				wl_get_drv_status(cfg, SENDING_ACT_FRM, ndev)));

#if defined(DHD_DEBUG) && defined(BCMPCIE) && defined(DHD_FW_COREDUMP)
			if (dhdp->memdump_enabled) {
				dhdp->memdump_type = DUMP_TYPE_SCAN_BUSY;
				dhd_bus_mem_dump(dhdp);
			}
#endif /* DHD_DEBUG && BCMPCIE && DHD_FW_COREDUMP */

			bzero(&bssid, sizeof(bssid));
			if ((ret = wldev_ioctl(ndev, WLC_GET_BSSID,
				&bssid, ETHER_ADDR_LEN, false)) == 0)
				WL_ERR(("FW is connected with " MACDBG "/n",
					MAC2STRDBG(bssid.octet)));
			else
				WL_ERR(("GET BSSID failed with %d\n", ret));

			wl_cfg80211_scan_abort(cfg);

		} else {
			/* Hold the context for 400msec, so that 10 subsequent scans
			* can give a buffer of 4sec which is enough to
			* cover any on-going scan in the firmware
			*/
			WL_DBG(("Enforcing delay for EBUSY case \n"));
			msleep(500);
		}
	} else {
		busy_count = 0;
	}

	wl_clr_drv_status(cfg, SCANNING, ndev);
	if (timer_pending(&cfg->scan_timeout))
		del_timer_sync(&cfg->scan_timeout);
	DHD_OS_SCAN_WAKE_UNLOCK((dhd_pub_t *)(cfg->pub));
	spin_lock_irqsave(&cfg->cfgdrv_lock, flags);
	cfg->scan_request = NULL;
	spin_unlock_irqrestore(&cfg->cfgdrv_lock, flags);

	return err;
}

#else /* BRCM_RSDB */
static s32
__wl_cfg80211_scan(struct wiphy *wiphy, struct net_device *ndev,
	struct cfg80211_scan_request *request,
	struct cfg80211_ssid *this_ssid)
{
	struct bcm_cfg80211 *cfg = wiphy_priv(wiphy);
	struct cfg80211_ssid *ssids;
	struct ether_addr primary_mac;
	bool p2p_ssid;
#ifdef WL11U
	bcm_tlv_t *interworking_ie;
#endif
	s32 err = 0;
	s32 bssidx = -1;
	s32 i;

	unsigned long flags;
	static s32 busy_count = 0;
#ifdef WL_CFG80211_VSDB_PRIORITIZE_SCAN_REQUEST
	struct net_device *remain_on_channel_ndev = NULL;
#endif

	dhd_pub_t *dhd;

	dhd = (dhd_pub_t *)(cfg->pub);
	/*
	 * Hostapd triggers scan before starting automatic channel selection
	 * also Dump stats IOVAR scans each channel hence returning from here.
	 */
	if (dhd->op_mode & DHD_FLAG_HOSTAP_MODE) {
#ifdef WL_SUPPORT_ACS
		WL_INFORM(("Scan Command at SoftAP mode\n"));
		return 0;
#else
		WL_ERR(("Invalid Scan Command at SoftAP mode\n"));
		return -EINVAL;
#endif /* WL_SUPPORT_ACS */
	}

	ndev = ndev_to_wlc_ndev(ndev, cfg);

	if (WL_DRV_STATUS_SENDING_AF_FRM_EXT(cfg)) {
		WL_ERR(("Sending Action Frames. Try it again.\n"));
		return -EAGAIN;
	}

	WL_DBG(("Enter wiphy (%p)\n", wiphy));
	if (wl_get_drv_status_all(cfg, SCANNING)) {
		if (cfg->scan_request == NULL) {
			wl_clr_drv_status_all(cfg, SCANNING);
			WL_DBG(("<<<<<<<<<<<Force Clear Scanning Status>>>>>>>>>>>\n"));
		} else {
			WL_ERR(("Scanning already\n"));
			return -EAGAIN;
		}
	}
	if (wl_get_drv_status(cfg, SCAN_ABORTING, ndev)) {
		WL_ERR(("Scanning being aborted\n"));
		return -EAGAIN;
	}
	if (request && request->n_ssids > WL_SCAN_PARAMS_SSID_MAX) {
		WL_ERR(("request null or n_ssids > WL_SCAN_PARAMS_SSID_MAX\n"));
		return -EOPNOTSUPP;
	}
#ifdef WL_CFG80211_VSDB_PRIORITIZE_SCAN_REQUEST
	remain_on_channel_ndev = wl_cfg80211_get_remain_on_channel_ndev(cfg);
	if (remain_on_channel_ndev) {
		WL_DBG(("Remain_on_channel bit is set, somehow it didn't get cleared\n"));
		wl_notify_escan_complete(cfg, remain_on_channel_ndev, true, true);
	}
#endif /* WL_CFG80211_VSDB_PRIORITIZE_SCAN_REQUEST */


	/* Arm scan timeout timer */
	mod_timer(&cfg->scan_timeout, jiffies + msecs_to_jiffies(WL_SCAN_TIMER_INTERVAL_MS));
	if (request) {		/* scan bss */
		ssids = request->ssids;
		p2p_ssid = false;
		for (i = 0; i < request->n_ssids; i++) {
#ifdef BCM_PATCH_HIDDEN_DIRECT_SCAN
			if (ssids[i].ssid_len &&
				IS_P2P_SSID(ssids[i].ssid, ssids[i].ssid_len) &&
				request->wdev && (request->wdev->iftype != NL80211_IFTYPE_STATION)) {
#else
			if (ssids[i].ssid_len &&
				IS_P2P_SSID(ssids[i].ssid, ssids[i].ssid_len)) {
#endif
				p2p_ssid = true;
				break;
			}
		}
		if (p2p_ssid) {
			if (cfg->p2p_supported) {
				/* p2p scan trigger */
				if (p2p_on(cfg) == false) {
					/* p2p on at the first time */
					p2p_on(cfg) = true;
					wl_cfgp2p_set_firm_p2p(cfg);
					get_primary_mac(cfg, &primary_mac);
					wl_cfgp2p_generate_bss_mac(&primary_mac,
						&cfg->p2p->dev_addr, &cfg->p2p->int_addr);
				}
				wl_clr_p2p_status(cfg, GO_NEG_PHASE);
				WL_DBG(("P2P: GO_NEG_PHASE status cleared \n"));
				p2p_scan(cfg) = true;
			}
		} else {
			/* legacy scan trigger
			 * So, we have to disable p2p discovery if p2p discovery is on
			 */
			if (cfg->p2p_supported) {
				p2p_scan(cfg) = false;
				/* If Netdevice is not equals to primary and p2p is on
				*  , we will do p2p scan using P2PAPI_BSSCFG_DEVICE.
				*/

				if (p2p_scan(cfg) == false) {
					if (wl_get_p2p_status(cfg, DISCOVERY_ON)) {
						err = wl_cfgp2p_discover_enable_search(cfg,
						false);
						if (unlikely(err)) {
							goto scan_out;
						}

					}
				}
			}
			if (!cfg->p2p_supported || !p2p_scan(cfg)) {

				if (wl_cfgp2p_find_idx(cfg, ndev, &bssidx) != BCME_OK) {
					WL_ERR(("Find p2p index from ndev(%p) failed\n",
						ndev));
					err = BCME_ERROR;
					goto scan_out;
				}
#ifdef WL11U
				if ((interworking_ie = wl_cfg80211_find_interworking_ie(
					(u8 *)request->ie, request->ie_len)) != NULL) {
					err = wl_cfg80211_add_iw_ie(cfg, ndev, bssidx,
					       VNDR_IE_CUSTOM_FLAG, interworking_ie->id,
					       interworking_ie->data, interworking_ie->len);

					if (unlikely(err)) {
						goto scan_out;
					}
				} else if (cfg->iw_ie_len != 0) {
				/* we have to clear IW IE and disable gratuitous APR */
					wl_cfg80211_add_iw_ie(cfg, ndev, bssidx,
						VNDR_IE_CUSTOM_FLAG,
						DOT11_MNG_INTERWORKING_ID,
						0, 0);

					wldev_iovar_setint_bsscfg(ndev, "grat_arp", 0,
						bssidx);
					cfg->wl11u = FALSE;
					/* we don't care about error */
				}
#endif /* WL11U */
				err = wl_cfgp2p_set_management_ie(cfg, ndev, bssidx,
					VNDR_IE_PRBREQ_FLAG, (u8 *)request->ie,
					request->ie_len);

				if (unlikely(err)) {
					goto scan_out;
				}

			}
		}
	} else {		/* scan in ibss */
		ssids = this_ssid;
	}

	cfg->scan_request = request;
	wl_set_drv_status(cfg, SCANNING, ndev);

	if (cfg->p2p_supported) {
		if (p2p_on(cfg) && p2p_scan(cfg)) {

			/* find my listen channel */
			cfg->afx_hdl->my_listen_chan =
				wl_find_listen_channel(cfg, request->ie,
				request->ie_len);
			err = wl_cfgp2p_enable_discovery(cfg, ndev,
			request->ie, request->ie_len);

			if (unlikely(err)) {
				goto scan_out;
			}
		}
	}
	err = wl_do_escan(cfg, wiphy, ndev, request);
	if (likely(!err))
		goto scan_success;
	else
		goto scan_out;

scan_success:
	busy_count = 0;

	return 0;

scan_out:
	if (err == BCME_BUSY || err == BCME_NOTREADY) {
		WL_ERR(("Scan err = (%d), busy?%d", err, -EBUSY));
		err = -EBUSY;
	}

#define SCAN_EBUSY_RETRY_LIMIT 10
	if (err == -EBUSY) {
		if (busy_count++ > SCAN_EBUSY_RETRY_LIMIT) {
			struct ether_addr bssid;
			s32 ret = 0;
			busy_count = 0;
			WL_ERR(("Unusual continuous EBUSY error, %d %d %d %d %d %d %d %d %d\n",
				wl_get_drv_status(cfg, SCANNING, ndev),
				wl_get_drv_status(cfg, SCAN_ABORTING, ndev),
				wl_get_drv_status(cfg, CONNECTING, ndev),
				wl_get_drv_status(cfg, CONNECTED, ndev),
				wl_get_drv_status(cfg, DISCONNECTING, ndev),
				wl_get_drv_status(cfg, AP_CREATING, ndev),
				wl_get_drv_status(cfg, AP_CREATED, ndev),
				wl_get_drv_status(cfg, SENDING_ACT_FRM, ndev),
				wl_get_drv_status(cfg, SENDING_ACT_FRM, ndev)));

			bzero(&bssid, sizeof(bssid));
			if ((ret = wldev_ioctl(ndev, WLC_GET_BSSID,
				&bssid, ETHER_ADDR_LEN, false)) == 0)
				WL_ERR(("FW is connected with " MACDBG "/n",
					MAC2STRDBG(bssid.octet)));
			else
				WL_ERR(("GET BSSID failed with %d\n", ret));

			wl_cfg80211_scan_abort(cfg);

		}
	} else {
		busy_count = 0;
	}

	wl_clr_drv_status(cfg, SCANNING, ndev);
	if (timer_pending(&cfg->scan_timeout))
		del_timer_sync(&cfg->scan_timeout);
	spin_lock_irqsave(&cfg->cfgdrv_lock, flags);
	cfg->scan_request = NULL;
	spin_unlock_irqrestore(&cfg->cfgdrv_lock, flags);

	return err;
}
#endif /* BRCM_RSDB */

static int wifitem_seq_set_current_tem(u32 current_tem);
static u32 wl_get_tempt(struct net_device *dev)
{
	u32 tempt = 0;
	s32 err = 0;
	if(dev == NULL)
	{
		WL_ERR(("wl_get_tempt:dev is NULL\n"));
		return 0;
	}

	err = wldev_iovar_getint(dev, "phy_tempsense", &tempt);
	if (unlikely(err))
	{
		WL_ERR(("wl_get_tempt Failed (%d)\n", err));
		return 0;
	}
	else
	{
		return tempt;
	}
}

#if defined(WL_CFG80211_P2P_DEV_IF)
static s32
wl_cfg80211_scan(struct wiphy *wiphy, struct cfg80211_scan_request *request)
#else
static s32
wl_cfg80211_scan(struct wiphy *wiphy, struct net_device *ndev,
	struct cfg80211_scan_request *request)
#endif /* WL_CFG80211_P2P_DEV_IF */
{
	s32 err = 0;
	u32 tempt = 0;
	struct bcm_cfg80211 *cfg = wiphy_priv(wiphy);
#if defined(WL_CFG80211_P2P_DEV_IF)
	struct net_device *ndev = wdev_to_wlc_ndev(request->wdev, cfg);
#endif /* WL_CFG80211_P2P_DEV_IF */

	WL_DBG(("Enter\n"));
	RETURN_EIO_IF_NOT_UP(cfg);
#ifdef  BRCM_RSDB
	if (ndev == bcmcfg_to_prmry_ndev(cfg)) {
		if (wl_cfg_multip2p_operational(cfg)) {
			WL_ERR(("wlan0 scan failed, p2p devices are operational"));
			 return -ENODEV;
		}
	}

	mutex_lock(&cfg->usr_sync);
#endif
	err = __wl_cfg80211_scan(wiphy, ndev, request, NULL);

	if(0 != wl_get_drv_status(cfg, CONNECTED, ndev))
	{
		tempt = wl_get_tempt(ndev);
	}
	else
	{
		tempt = 0;
	}
	wifitem_seq_set_current_tem(tempt);

#ifndef  BRCM_RSDB
	if (unlikely(err)) {
		if ((err == BCME_EPERM) && cfg->scan_suppressed)
			WL_DBG(("scan not permitted at this time (%d)\n", err));
		else
			WL_ERR(("scan error (%d)\n", err));
		return err;
	}

	return err;
#else
	if (unlikely(err)) {
		WL_ERR(("scan error (%d)\n", err));
	}
	mutex_unlock(&cfg->usr_sync);

	return err;
#endif

}

static s32 wl_set_rts(struct net_device *dev, u32 rts_threshold)
{
	s32 err = 0;

	err = wldev_iovar_setint(dev, "rtsthresh", rts_threshold);
	if (unlikely(err)) {
		WL_ERR(("Error (%d)\n", err));
		return err;
	}
	return err;
}

static s32 wl_set_frag(struct net_device *dev, u32 frag_threshold)
{
	s32 err = 0;

	err = wldev_iovar_setint_bsscfg(dev, "fragthresh", frag_threshold, 0);
	if (unlikely(err)) {
		WL_ERR(("Error (%d)\n", err));
		return err;
	}
	return err;
}

static s32 wl_set_retry(struct net_device *dev, u32 retry, bool l)
{
	s32 err = 0;
	u32 cmd = (l ? WLC_SET_LRL : WLC_SET_SRL);

	retry = htod32(retry);
	err = wldev_ioctl(dev, cmd, &retry, sizeof(retry), true);
	if (unlikely(err)) {
		WL_ERR(("cmd (%d) , error (%d)\n", cmd, err));
		return err;
	}
	return err;
}

static s32 wl_cfg80211_set_wiphy_params(struct wiphy *wiphy, u32 changed)
{
	struct bcm_cfg80211 *cfg = (struct bcm_cfg80211 *)wiphy_priv(wiphy);
	struct net_device *ndev = bcmcfg_to_prmry_ndev(cfg);
	s32 err = 0;

	RETURN_EIO_IF_NOT_UP(cfg);
	WL_DBG(("Enter\n"));
	if (changed & WIPHY_PARAM_RTS_THRESHOLD &&
		(cfg->conf->rts_threshold != wiphy->rts_threshold)) {
		cfg->conf->rts_threshold = wiphy->rts_threshold;
		err = wl_set_rts(ndev, cfg->conf->rts_threshold);
		if (!err)
			return err;
	}
	if (changed & WIPHY_PARAM_FRAG_THRESHOLD &&
		(cfg->conf->frag_threshold != wiphy->frag_threshold)) {
		cfg->conf->frag_threshold = wiphy->frag_threshold;
		err = wl_set_frag(ndev, cfg->conf->frag_threshold);
		if (!err)
			return err;
	}
	if (changed & WIPHY_PARAM_RETRY_LONG &&
		(cfg->conf->retry_long != wiphy->retry_long)) {
		cfg->conf->retry_long = wiphy->retry_long;
		err = wl_set_retry(ndev, cfg->conf->retry_long, true);
		if (!err)
			return err;
	}
	if (changed & WIPHY_PARAM_RETRY_SHORT &&
		(cfg->conf->retry_short != wiphy->retry_short)) {
		cfg->conf->retry_short = wiphy->retry_short;
		err = wl_set_retry(ndev, cfg->conf->retry_short, false);
		if (!err) {
			return err;
		}
	}

	return err;
}
static chanspec_t
channel_to_chanspec(struct wiphy *wiphy, struct net_device *dev, u32 channel, u32 bw_cap)
{
	struct bcm_cfg80211 *cfg = wiphy_priv(wiphy);
	u8 *buf = NULL;
	wl_uint32_list_t *list;
	int err = BCME_OK;
	chanspec_t c = 0, ret_c = 0;
	int bw = 0, tmp_bw = 0;
	int i;
	u32 tmp_c;
	u16 kflags = in_atomic() ? GFP_ATOMIC : GFP_KERNEL;
#define LOCAL_BUF_SIZE	1024
	buf = (u8 *) kzalloc(LOCAL_BUF_SIZE, kflags);
	if (!buf) {
		WL_ERR(("buf memory alloc failed\n"));
		goto exit;
	}
	list = (wl_uint32_list_t *)(void *)buf;
	list->count = htod32(WL_NUMCHANSPECS);
	err = wldev_iovar_getbuf_bsscfg(dev, "chanspecs", NULL,
		0, buf, LOCAL_BUF_SIZE, 0, &cfg->ioctl_buf_sync);
	if (err != BCME_OK) {
		WL_ERR(("get chanspecs failed with %d\n", err));
		goto exit;
	}
	for (i = 0; i < dtoh32(list->count); i++) {
		c = dtoh32(list->element[i]);
		if (channel <= CH_MAX_2G_CHANNEL) {
			if (!CHSPEC_IS20(c))
				continue;
			if (channel == CHSPEC_CHANNEL(c)) {
				ret_c = c;
				bw = 20;
				goto exit;
			}
		}
		tmp_c = wf_chspec_ctlchan(c);
		tmp_bw = bw2cap[CHSPEC_BW(c) >> WL_CHANSPEC_BW_SHIFT];
		if (tmp_c != channel)
			continue;

		if ((tmp_bw > bw) && (tmp_bw <= bw_cap)) {
			bw = tmp_bw;
			ret_c = c;
			if (bw == bw_cap)
				goto exit;
		}
	}
exit:
	if (buf)
		kfree(buf);
#undef LOCAL_BUF_SIZE
	WL_INFORM(("return chanspec %x %d\n", ret_c, bw));
	return ret_c;
}

void
wl_cfg80211_ibss_vsie_set_buffer(vndr_ie_setbuf_t *ibss_vsie, int ibss_vsie_len)
{
	struct bcm_cfg80211 *cfg = g_bcm_cfg;

	if (cfg != NULL && ibss_vsie != NULL) {
		if (cfg->ibss_vsie != NULL) {
			kfree(cfg->ibss_vsie);
		}
		cfg->ibss_vsie = ibss_vsie;
		cfg->ibss_vsie_len = ibss_vsie_len;
	}
}

static void
wl_cfg80211_ibss_vsie_free(struct bcm_cfg80211 *cfg)
{
	/* free & initiralize VSIE (Vendor Specific IE) */
	if (cfg->ibss_vsie != NULL) {
		kfree(cfg->ibss_vsie);
		cfg->ibss_vsie = NULL;
		cfg->ibss_vsie_len = 0;
	}
}

s32
wl_cfg80211_ibss_vsie_delete(struct net_device *dev)
{
	struct bcm_cfg80211 *cfg = g_bcm_cfg;
	char *ioctl_buf = NULL;
	s32 ret = BCME_OK;

	if (cfg != NULL && cfg->ibss_vsie != NULL) {
		ioctl_buf = kmalloc(WLC_IOCTL_MEDLEN, GFP_KERNEL);
		if (!ioctl_buf) {
			WL_ERR(("ioctl memory alloc failed\n"));
			return -ENOMEM;
		}

		/* change the command from "add" to "del" */
		strncpy(cfg->ibss_vsie->cmd, "del", VNDR_IE_CMD_LEN - 1);
		cfg->ibss_vsie->cmd[VNDR_IE_CMD_LEN - 1] = '\0';

		ret = wldev_iovar_setbuf(dev, "ie",
			cfg->ibss_vsie, cfg->ibss_vsie_len,
			ioctl_buf, WLC_IOCTL_MEDLEN, NULL);
		WL_ERR(("ret=%d\n", ret));

		if (ret == BCME_OK) {
			/* free & initiralize VSIE */
			kfree(cfg->ibss_vsie);
			cfg->ibss_vsie = NULL;
			cfg->ibss_vsie_len = 0;
		}

		if (ioctl_buf) {
			kfree(ioctl_buf);
		}
	}

	return ret;
}
#ifndef  BRCM_RSDB
static bcm_struct_cfgdev*
bcm_cfg80211_add_ibss_if(struct wiphy *wiphy, char *name)
{
	int err = 0;
	struct bcm_cfg80211 *cfg = wiphy_priv(wiphy);
	struct wireless_dev* wdev = NULL;
	struct net_device *new_ndev = NULL;
	struct net_device *primary_ndev = NULL;
	s32 timeout;
	wl_aibss_if_t aibss_if;
	wl_if_event_info *event = NULL;

	if (cfg->ibss_cfgdev != NULL) {
		WL_ERR(("IBSS interface %s already exists\n", name));
		return NULL;
	}

	WL_ERR(("Try to create IBSS interface %s\n", name));
	primary_ndev = bcmcfg_to_prmry_ndev(cfg);
	/* generate a new MAC address for the IBSS interface */
	get_primary_mac(cfg, &cfg->ibss_if_addr);
	cfg->ibss_if_addr.octet[4] ^= 0x40;
	memset(&aibss_if, 0, sizeof(aibss_if));
	memcpy(&aibss_if.addr, &cfg->ibss_if_addr, sizeof(aibss_if.addr));
	aibss_if.chspec = 0;
	aibss_if.len = sizeof(aibss_if);

	cfg->bss_pending_op = TRUE;
	memset(&cfg->if_event_info, 0, sizeof(cfg->if_event_info));
	err = wldev_iovar_setbuf(primary_ndev, "aibss_ifadd", &aibss_if,
		sizeof(aibss_if), cfg->ioctl_buf, WLC_IOCTL_MAXLEN, NULL);
	if (err) {
		WL_ERR(("IOVAR aibss_ifadd failed with error %d\n", err));
		goto fail;
	}
	timeout = wait_event_interruptible_timeout(cfg->netif_change_event,
		!cfg->bss_pending_op, msecs_to_jiffies(MAX_WAIT_TIME));
	if (timeout <= 0 || cfg->bss_pending_op)
		goto fail;

	event = &cfg->if_event_info;
	strncpy(event->name, name, IFNAMSIZ - 1);
	/* By calling wl_cfg80211_allocate_if (dhd_allocate_if eventually) we give the control
	 * over this net_device interface to dhd_linux, hence the interface is managed by dhd_liux
	 * and will be freed by dhd_detach unless it gets unregistered before that. The
	 * wireless_dev instance new_ndev->ieee80211_ptr associated with this net_device will
	 * be freed by wl_dealloc_netinfo
	 */
	new_ndev = wl_cfg80211_allocate_if(cfg, event->ifidx, event->name,
		event->mac, event->bssidx);
	if (new_ndev == NULL)
		goto fail;
	wdev = kzalloc(sizeof(*wdev), GFP_KERNEL);
	if (wdev == NULL)
		goto fail;
	wdev->wiphy = wiphy;
	wdev->iftype = NL80211_IFTYPE_ADHOC;
	wdev->netdev = new_ndev;
	new_ndev->ieee80211_ptr = wdev;
	SET_NETDEV_DEV(new_ndev, wiphy_dev(wdev->wiphy));

	/* rtnl lock must have been acquired, if this is not the case, wl_cfg80211_register_if
	* needs to be modified to take one parameter (bool need_rtnl_lock)
	 */
	ASSERT_RTNL();
	if (wl_cfg80211_register_if(cfg, event->ifidx, new_ndev) != BCME_OK)
		goto fail;

	wl_alloc_netinfo(cfg, new_ndev, wdev, WL_MODE_IBSS, PM_ENABLE);
	cfg->ibss_cfgdev = ndev_to_cfgdev(new_ndev);
	WL_ERR(("IBSS interface %s created\n", new_ndev->name));
	return cfg->ibss_cfgdev;

fail:
	WL_ERR(("failed to create IBSS interface %s \n", name));
	cfg->bss_pending_op = FALSE;
	if (new_ndev)
		wl_cfg80211_remove_if(cfg, event->ifidx, new_ndev);
	if (wdev)
		kfree(wdev);
	return NULL;
}

#if defined(DUAL_STA) || defined(DUAL_STA_STATIC_IF)
s32
wl_cfg80211_add_del_bss(struct bcm_cfg80211 *cfg,
	struct net_device *ndev, s32 bsscfg_idx,
	enum nl80211_iftype iface_type, s32 del, u8 *addr)
{
	s32 ret = BCME_OK;
	s32 val = 0;

	struct {
		s32 cfg;
		s32 val;
		struct ether_addr ea;
	} bss_setbuf;

	WL_INFORM(("iface_type:%d del:%d \n", iface_type, del));

	bzero(&bss_setbuf, sizeof(bss_setbuf));

	/* AP=3, STA=2, up=1, down=0, val=-1 */
	if (del) {
		val = -1;
	} else if (iface_type == NL80211_IFTYPE_AP) {
		/* AP Interface */
		WL_DBG(("Adding AP Interface \n"));
		val = 3;
	} else if (iface_type == NL80211_IFTYPE_STATION) {
		WL_DBG(("Adding STA Interface \n"));
		val = 2;
	} else {
		WL_ERR((" add_del_bss NOT supported for IFACE type:0x%x", iface_type));
		return -EINVAL;
	}

	bss_setbuf.cfg = htod32(bsscfg_idx);
	bss_setbuf.val = htod32(val);

	if (addr) {
		memcpy(&bss_setbuf.ea.octet, addr, ETH_ALEN);
	}

	ret = wldev_iovar_setbuf(ndev, "bss", &bss_setbuf, sizeof(bss_setbuf),
		cfg->ioctl_buf, WLC_IOCTL_MAXLEN, &cfg->ioctl_buf_sync);
	if (ret != 0)
		WL_ERR(("'bss %d' failed with %d\n", val, ret));

	return ret;
}

/* Create a Generic Network Interface and initialize it depending up on
 * the interface type
 */
bcm_struct_cfgdev*
wl_cfg80211_create_iface(struct wiphy *wiphy,
	enum nl80211_iftype iface_type,
	u8 *mac_addr, const char *name)
{
	struct bcm_cfg80211 *cfg = wiphy_priv(wiphy);
	struct net_device *new_ndev = NULL;
	struct net_device *primary_ndev = NULL;
	s32 ret = BCME_OK;
	s32 bsscfg_idx = 1;
	u32 timeout;
	wl_if_event_info *event = NULL;
	struct wireless_dev *wdev = NULL;
	u8 addr[ETH_ALEN];

	WL_DBG(("Enter\n"));

	if (!name) {
		WL_ERR(("Interface name not provided\n"));
		return NULL;
	}

	primary_ndev = bcmcfg_to_prmry_ndev(cfg);

	if (likely(!mac_addr)) {
		/* Use primary MAC with the locally administered bit for the Secondary STA I/F */
		memcpy(addr, primary_ndev->dev_addr, ETH_ALEN);
		addr[0] |= 0x02;
	} else {
		/* Use the application provided mac address (if any) */
		memcpy(addr, mac_addr, ETH_ALEN);
	}

	if ((iface_type != NL80211_IFTYPE_STATION) && (iface_type != NL80211_IFTYPE_AP)) {
		WL_ERR(("IFACE type:%d not supported. STA "
					"or AP IFACE is only supported\n", iface_type));
		return NULL;
	}

	cfg->bss_pending_op = TRUE;
	memset(&cfg->if_event_info, 0, sizeof(cfg->if_event_info));

	/* De-initialize the p2p discovery interface, if operational */
	if (p2p_is_on(cfg)) {
		WL_DBG(("Disabling P2P Discovery Interface \n"));
#ifdef WL_CFG80211_P2P_DEV_IF
		ret = wl_cfg80211_scan_stop(bcmcfg_to_p2p_wdev(cfg));
#else
		ret = wl_cfg80211_scan_stop(cfg->p2p_net);
#endif
		if (unlikely(ret < 0)) {
			CFGP2P_ERR(("P2P scan stop failed, ret=%d\n", ret));
		}

		wl_cfgp2p_disable_discovery(cfg);
		wl_to_p2p_bss_bssidx(cfg, P2PAPI_BSSCFG_DEVICE) = 0;
		p2p_on(cfg) = false;
	}

	/*
	 * Intialize the firmware I/F.
	 */
	if ((ret = wl_cfg80211_add_del_bss(cfg, primary_ndev,
		bsscfg_idx, iface_type, 0, addr)) < 0) {
		return NULL;
	}

	/*
	 * Wait till the firmware send a confirmation event back.
	 */
	WL_DBG(("Wait for the FW I/F Event\n"));
	timeout = wait_event_interruptible_timeout(cfg->netif_change_event,
		!cfg->bss_pending_op, msecs_to_jiffies(MAX_WAIT_TIME));
	if (timeout <= 0 || cfg->bss_pending_op) {
		WL_ERR(("ADD_IF event, didn't come. Return \n"));
		goto fail;
	}

	/*
	 * Since FW operation is successful,we can go ahead with the
	 * the host interface creation.
	 */
	event = &cfg->if_event_info;
	strncpy(event->name, name, IFNAMSIZ - 1);
	new_ndev = wl_cfg80211_allocate_if(cfg, event->ifidx,
		event->name, addr, event->bssidx);
	if (!new_ndev) {
		WL_ERR(("I/F allocation failed! \n"));
		goto fail;
	} else
		WL_DBG(("I/F allocation succeeded! ifidx:0x%x bssidx:0x%x \n",
		 event->ifidx, event->bssidx));

	wdev = kzalloc(sizeof(*wdev), GFP_KERNEL);
	if (!wdev) {
		WL_ERR(("wireless_dev alloc failed! \n"));
		goto fail;
	}

	wdev->wiphy = wiphy;
	wdev->iftype = iface_type;
	new_ndev->ieee80211_ptr = wdev;
	SET_NETDEV_DEV(new_ndev, wiphy_dev(wdev->wiphy));

	/* RTNL lock must have been acquired. */
	ASSERT_RTNL();

	/* Set the locally administed mac addr, if not applied already */
	if (memcmp(addr, event->mac, ETH_ALEN) != 0) {
		ret = wldev_iovar_setbuf_bsscfg(primary_ndev, "cur_etheraddr", addr, ETH_ALEN,
			cfg->ioctl_buf, WLC_IOCTL_MAXLEN, event->bssidx, &cfg->ioctl_buf_sync);
		if (unlikely(ret)) {
				WL_ERR(("set cur_etheraddr Error (%d)\n", ret));
				goto fail;
		}
		memcpy(new_ndev->dev_addr, addr, ETH_ALEN);
	}

	if (wl_cfg80211_register_if(cfg, event->ifidx, new_ndev) != BCME_OK) {
		WL_ERR(("IFACE register failed \n"));
		goto fail;
	}

	/* Initialize with the station mode params */
	wl_alloc_netinfo(cfg, new_ndev, wdev,
		(iface_type == NL80211_IFTYPE_STATION) ?
		WL_MODE_BSS : WL_MODE_AP, PM_ENABLE);
	cfg->bss_cfgdev = ndev_to_cfgdev(new_ndev);
	cfg->cfgdev_bssidx = event->bssidx;

	WL_DBG(("Host Network Interface for Secondary I/F created"));

	return cfg->bss_cfgdev;

fail:
	cfg->bss_pending_op = FALSE;
	if (new_ndev)
		wl_cfg80211_remove_if(cfg, event->ifidx, new_ndev);
	if (wdev)
		kfree(wdev);

	return NULL;
}
s32
wl_cfg80211_del_iface(struct wiphy *wiphy, bcm_struct_cfgdev *cfgdev)
{
	struct bcm_cfg80211 *cfg = wiphy_priv(wiphy);
	struct net_device *ndev = NULL;
	struct net_device *primary_ndev = NULL;
	s32 ret = BCME_OK;
	s32 bsscfg_idx = 1;
	u32 timeout;
	enum nl80211_iftype iface_type = NL80211_IFTYPE_STATION;

	WL_DBG(("Enter\n"));

	if (!cfg->bss_cfgdev)
		return 0;

	/* If any scan is going on, abort it */
	if (wl_get_drv_status_all(cfg, SCANNING)) {
		WL_DBG(("Scan in progress. Aborting the scan!\n"));
		wl_notify_escan_complete(cfg, cfg->escan_info.ndev, true, true);
	}

	ndev = (struct net_device *)cfgdev_to_ndev(cfg->bss_cfgdev);
	primary_ndev = bcmcfg_to_prmry_ndev(cfg);

	cfg->bss_pending_op = TRUE;
	memset(&cfg->if_event_info, 0, sizeof(cfg->if_event_info));

	/* Delete the firmware interface */
	if ((ret = wl_cfg80211_add_del_bss(cfg, ndev,
		bsscfg_idx, iface_type, true, NULL)) < 0) {
		WL_ERR(("DEL bss failed ret:%d \n", ret));
		return ret;
	}

	timeout = wait_event_interruptible_timeout(cfg->netif_change_event,
		!cfg->bss_pending_op, msecs_to_jiffies(MAX_WAIT_TIME));
	if (timeout <= 0 || cfg->bss_pending_op) {
		WL_ERR(("timeout in waiting IF_DEL event\n"));
	}

	wl_cfg80211_remove_if(cfg, cfg->if_event_info.ifidx, ndev);
	cfg->bss_cfgdev = NULL;
	cfg->cfgdev_bssidx = -1;
	cfg->bss_pending_op = FALSE;

	WL_DBG(("IF_DEL Done.\n"));

	return ret;
}
#endif /* defined(DUAL_STA) || defined(DUAL_STA_STATIC_IF) */

#else  /* BRCM_RSDB */

#ifdef WLAIBSS_MCHAN
static bcm_struct_cfgdev*
bcm_cfg80211_add_ibss_if(struct wiphy *wiphy, char *name)
{
	int err = 0;
	struct bcm_cfg80211 *cfg = wiphy_priv(wiphy);
	struct wireless_dev* wdev = NULL;
	struct net_device *new_ndev = NULL;
	struct net_device *primary_ndev = NULL;
	s32 timeout;
	wl_aibss_if_t aibss_if;
	wl_if_event_info *event = NULL;

	if (cfg->ibss_cfgdev != NULL) {
		WL_ERR(("IBSS interface %s already exists\n", name));
		return NULL;
	}

	WL_ERR(("Try to create IBSS interface %s\n", name));
	primary_ndev = bcmcfg_to_prmry_ndev(cfg);
	/* generate a new MAC address for the IBSS interface */
	get_primary_mac(cfg, &cfg->ibss_if_addr);
	cfg->ibss_if_addr.octet[4] ^= 0x40;
	memset(&aibss_if, sizeof(aibss_if), 0);
	memcpy(&aibss_if.addr, &cfg->ibss_if_addr, sizeof(aibss_if.addr));
	aibss_if.chspec = 0;
	aibss_if.len = sizeof(aibss_if);

	cfg->bss_pending_op = TRUE;
	memset(&cfg->if_event_info, 0, sizeof(cfg->if_event_info));
	err = wldev_iovar_setbuf(primary_ndev, "aibss_ifadd", &aibss_if,
		sizeof(aibss_if), cfg->ioctl_buf, WLC_IOCTL_MAXLEN, NULL);
	if (err) {
		WL_ERR(("IOVAR aibss_ifadd failed with error %d\n", err));
		goto fail;
	}
	timeout = wait_event_interruptible_timeout(cfg->netif_change_event,
		!cfg->bss_pending_op, msecs_to_jiffies(MAX_WAIT_TIME));
	if (timeout <= 0 || cfg->bss_pending_op)
		goto fail;

	event = &cfg->if_event_info;
	/* By calling wl_cfg80211_allocate_if (dhd_allocate_if eventually) we give the control
	 * over this net_device interface to dhd_linux, hence the interface is managed by dhd_liux
	 * and will be freed by dhd_detach unless it gets unregistered before that. The
	 * wireless_dev instance new_ndev->ieee80211_ptr associated with this net_device will
	 * be freed by wl_dealloc_netinfo
	 */
	new_ndev = wl_cfg80211_allocate_if(cfg, event->ifidx, event->name,
		event->mac, event->bssidx, event->name);
	if (new_ndev == NULL)
		goto fail;
	wdev = kzalloc(sizeof(*wdev), GFP_KERNEL);
	if (wdev == NULL)
		goto fail;
	wdev->wiphy = wiphy;
	wdev->iftype = NL80211_IFTYPE_ADHOC;
	wdev->netdev = new_ndev;
	new_ndev->ieee80211_ptr = wdev;
	SET_NETDEV_DEV(new_ndev, wiphy_dev(wdev->wiphy));

	/* rtnl lock must have been acquired, if this is not the case, wl_cfg80211_register_if
	* needs to be modified to take one parameter (bool need_rtnl_lock)
	 */
	ASSERT_RTNL();
	if (wl_cfg80211_register_if(cfg, event->ifidx, new_ndev) != BCME_OK)
		goto fail;

	wl_alloc_netinfo(cfg, new_ndev, wdev, WL_MODE_IBSS, PM_ENABLE, event->bssidx);
	cfg->ibss_cfgdev = ndev_to_cfgdev(new_ndev);
	WL_ERR(("IBSS interface %s created\n", new_ndev->name));
	return cfg->ibss_cfgdev;

fail:
	WL_ERR(("failed to create IBSS interface %s \n", name));
	cfg->bss_pending_op = FALSE;
	if (new_ndev)
		wl_cfg80211_remove_if(cfg, event->ifidx, new_ndev);
	if (wdev)
		kfree(wdev);
	return NULL;
}

static s32
bcm_cfg80211_del_ibss_if(struct wiphy *wiphy, bcm_struct_cfgdev *cfgdev)
{
	int err = 0;
	struct bcm_cfg80211 *cfg = wiphy_priv(wiphy);
	struct net_device *ndev = NULL;
	struct net_device *primary_ndev = NULL;
	s32 timeout;

	if (!cfgdev || cfg->ibss_cfgdev != cfgdev || ETHER_ISNULLADDR(&cfg->ibss_if_addr.octet))
		return -EINVAL;
	ndev = (struct net_device *)cfgdev_to_ndev(cfg->ibss_cfgdev);
	primary_ndev = bcmcfg_to_prmry_ndev(cfg);

	cfg->bss_pending_op = TRUE;
	memset(&cfg->if_event_info, 0, sizeof(cfg->if_event_info));
	err = wldev_iovar_setbuf(primary_ndev, "aibss_ifdel", &cfg->ibss_if_addr,
		sizeof(cfg->ibss_if_addr), cfg->ioctl_buf, WLC_IOCTL_MAXLEN, NULL);
	if (err) {
		WL_ERR(("IOVAR aibss_ifdel failed with error %d\n", err));
		goto fail;
	}
	timeout = wait_event_interruptible_timeout(cfg->netif_change_event,
		!cfg->bss_pending_op, msecs_to_jiffies(MAX_WAIT_TIME));
	if (timeout <= 0 || cfg->bss_pending_op) {
		WL_ERR(("timeout in waiting IF_DEL event\n"));
		goto fail;
	}

	wl_cfg80211_remove_if(cfg, cfg->if_event_info.ifidx, ndev);
	cfg->ibss_cfgdev = NULL;
	return 0;

fail:
	cfg->bss_pending_op = FALSE;
	return -1;
}
#endif /* WLAIBSS_MCHAN */

s32
wl_cfg80211_interface_ops(struct bcm_cfg80211 *cfg,
	struct net_device *ndev, s32 bsscfg_idx,
	enum nl80211_iftype iface_type, s32 del, u8 *addr)
{
	wl_interface_create_t iface;
	s32 ret;
	wl_interface_info_t *info;

	bzero(&iface, sizeof(wl_interface_create_t));

	iface.ver = WL_INTERFACE_CREATE_VER;

	if (iface_type == NL80211_IFTYPE_AP)
		iface.flags = WL_INTERFACE_CREATE_AP;
	else
		iface.flags = WL_INTERFACE_CREATE_STA;

	if (del) {
		ret = wldev_iovar_setbuf(ndev, "interface_remove",
			NULL, 0, cfg->ioctl_buf, WLC_IOCTL_MEDLEN, NULL);
	} else {
		if (addr) {
			memcpy(&iface.mac_addr.octet, addr, ETH_ALEN);
			iface.flags |= WL_INTERFACE_MAC_USE;
		}
		ret = wldev_iovar_getbuf(ndev, "interface_create",
			&iface, sizeof(wl_interface_create_t),
			cfg->ioctl_buf, WLC_IOCTL_MAXLEN, &cfg->ioctl_buf_sync);
		if (ret == 0) {
			/* success */
			info = (wl_interface_info_t *)cfg->ioctl_buf;
			WL_DBG(("wl interface create success!! bssidx:%d \n",
				info->bsscfgidx));
			ret = info->bsscfgidx;
		}
	}

	if (ret < 0)
		WL_ERR(("Interface %s failed!! ret %d\n",
			del ? "remove" : "create", ret));

	return ret;
}


s32
wl_cfg80211_add_del_bss(struct bcm_cfg80211 *cfg,
	struct net_device *ndev, s32 bsscfg_idx,
	enum nl80211_iftype iface_type, s32 del, u8 *addr)
{
	s32 ret = BCME_OK;
	s32 val = 0;

	struct {
		s32 cfg;
		s32 val;
		struct ether_addr ea;
	} bss_setbuf;

	WL_INFORM(("iface_type:%d del:%d \n", iface_type, del));

	bzero(&bss_setbuf, sizeof(bss_setbuf));

	/* AP=3, STA=2, up=1, down=0, val=-1 */
	if (del) {
		val = -1;
	} else if (iface_type == NL80211_IFTYPE_AP) {
		/* AP Interface */
		WL_DBG(("Adding AP Interface \n"));
		val = 3;
	} else if (iface_type == NL80211_IFTYPE_STATION) {
		WL_DBG(("Adding STA Interface \n"));
		val = 2;
	} else {
		WL_ERR((" add_del_bss NOT supported for IFACE type:0x%x", iface_type));
		return -EINVAL;
	}

	bss_setbuf.cfg = htod32(bsscfg_idx);
	bss_setbuf.val = htod32(val);

	if (addr) {
		memcpy(&bss_setbuf.ea.octet, addr, ETH_ALEN);
	}

	ret = wldev_iovar_setbuf(ndev, "bss", &bss_setbuf, sizeof(bss_setbuf),
		cfg->ioctl_buf, WLC_IOCTL_MAXLEN, &cfg->ioctl_buf_sync);
	if (ret != 0)
		WL_ERR(("'bss %d' failed with %d\n", val, ret));

	return ret;
}

#if defined(WL_VIRTUAL_APSTA) || defined(DUAL_STA_STATIC_IF)
/* Create a Generic Network Interface and initialize it depending up on
 * the interface type
 */
bcm_struct_cfgdev*
wl_cfg80211_create_iface(struct wiphy *wiphy,
	enum nl80211_iftype iface_type,
	u8 *mac_addr, const char *name)
{
	struct bcm_cfg80211 *cfg = wiphy_priv(wiphy);
	struct net_device *new_ndev = NULL;
	struct net_device *primary_ndev = NULL;
	s32 ret = BCME_OK;
	s32 bsscfg_idx = 0;
	u32 timeout;
	wl_if_event_info *event = NULL;
	struct wireless_dev *wdev = NULL;
	u8 addr[ETH_ALEN];

	WL_DBG(("Enter\n"));

	if (!name) {
		WL_ERR(("Interface name not provided\n"));
		return NULL;
	}

	primary_ndev = bcmcfg_to_prmry_ndev(cfg);

#ifdef DHD_IFDEBUG
	WL_ERR(("cfg=%p, primary_ndev=%p, ifname=%s\n", cfg, primary_ndev, name));
#endif

	/* If any scan is going on, abort it */
	if (wl_get_drv_status_all(cfg, SCANNING)) {
		int wait_cnt = MAX_SCAN_ABORT_WAIT_CNT;
		WL_ERR(("Scan in progress. Aborting the scan!\n"));
		wl_cfg80211_scan_abort(cfg);
		while (wl_get_drv_status_all(cfg, SCANNING) && wait_cnt) {
			WL_DBG(("Waiting for SCANNING terminated, wait_cnt: %d\n", wait_cnt));
			wait_cnt--;
			OSL_SLEEP(WAIT_SCAN_ABORT_OSL_SLEEP_TIME);
		}
		if (!wait_cnt && wl_get_drv_status_all(cfg, SCANNING)) {
			WL_ERR(("Failed to abort scan\n"));
			return NULL;
		}
	}

	primary_ndev = bcmcfg_to_prmry_ndev(cfg);
	if (likely(!mac_addr)) {
		/* Use primary MAC with the locally administered bit for the
		 *  Secondary STA I/F
		 */
		memcpy(addr, primary_ndev->dev_addr, ETH_ALEN);
		addr[0] |= 0x02;
	} else {
		/* Use the application provided mac address (if any) */
		memcpy(addr, mac_addr, ETH_ALEN);
	}

	if ((iface_type != NL80211_IFTYPE_STATION) && (iface_type != NL80211_IFTYPE_AP)) {
		WL_ERR(("IFACE type:%d not supported. STA "
					"or AP IFACE is only supported\n", iface_type));
		return NULL;
	}

	cfg->bss_pending_op = TRUE;
	memset(&cfg->if_event_info, 0, sizeof(cfg->if_event_info));

	/* De-initialize the p2p discovery interface, if operational */
	if (p2p_is_on(cfg)) {
		WL_DBG(("Disabling P2P Discovery Interface \n"));
#ifdef WL_CFG80211_P2P_DEV_IF
		ret = wl_cfg80211_scan_stop(bcmcfg_to_p2p_wdev(cfg));
#else
		ret = wl_cfg80211_scan_stop(cfg->p2p_net);
#endif
		if (unlikely(ret < 0)) {
			CFGP2P_ERR(("P2P scan stop failed, ret=%d\n", ret));
		}

#ifdef DHD_IFDEBUG
		WL_ERR(("call wl_cfgp2p_disable_discovery()\n"));
#endif
		wl_cfgp2p_disable_discovery(cfg);
		wl_to_p2p_bss_bssidx(cfg, P2PAPI_BSSCFG_DEVICE) = 0;
		p2p_on(cfg) = false;
	}

	/*
	 * Intialize the firmware I/F.
	 */
	ret = wl_cfg80211_interface_ops(cfg, primary_ndev, bsscfg_idx,
		NL80211_IFTYPE_STATION, 0, addr);
	if (ret == BCME_UNSUPPORTED) {
		/* Use bssidx 1 by default */
		bsscfg_idx = 1;
		if ((ret = wl_cfg80211_add_del_bss(cfg, primary_ndev,
			bsscfg_idx, iface_type, 0, addr)) < 0) {
			return NULL;
		}
	} else if (ret < 0) {
		WL_ERR(("Interface create failed!! ret:%d \n", ret));
		goto fail;
	} else {
		/* Success */
		bsscfg_idx = ret;
	}

	WL_DBG(("Interface created!! bssidx:%d \n", bsscfg_idx));

	/*
	 * Wait till the firmware send a confirmation event back.
	 */
	WL_DBG(("Wait for the FW I/F Event\n"));
	timeout = wait_event_interruptible_timeout(cfg->netif_change_event,
		!cfg->bss_pending_op, msecs_to_jiffies(MAX_WAIT_TIME));
	if (timeout <= 0 || cfg->bss_pending_op) {
		WL_ERR(("ADD_IF event, didn't come. Return \n"));
		goto fail;
	}

	/*
	 * Since FW operation is successful,we can go ahead with the
	 * the host interface creation.
	 */
	event = &cfg->if_event_info;
	new_ndev = wl_cfg80211_allocate_if(cfg, event->ifidx,
		(char*)name, addr, event->bssidx, event->name);
	if (!new_ndev) {
		WL_ERR(("I/F allocation failed! \n"));
		goto fail;
	} else
		WL_DBG(("I/F allocation succeeded! ifidx:0x%x bssidx:0x%x \n",
		 event->ifidx, event->bssidx));

	wdev = kzalloc(sizeof(*wdev), GFP_KERNEL);
	if (!wdev) {
		WL_ERR(("wireless_dev alloc failed! \n"));
		goto fail;
	}

	wdev->wiphy = wiphy;
	wdev->iftype = iface_type;
	new_ndev->ieee80211_ptr = wdev;
	SET_NETDEV_DEV(new_ndev, wiphy_dev(wdev->wiphy));

#ifdef DHD_IFDEBUG
	WL_ERR(("wdev=%p, new_ndev=%p\n", wdev, new_ndev));
#endif

	/* RTNL lock must have been acquired. */
	ASSERT_RTNL();

	/* Set the locally administed mac addr, if not applied already */
	if (memcmp(addr, event->mac, ETH_ALEN) != 0) {
		ret = wldev_iovar_setbuf_bsscfg(primary_ndev, "cur_etheraddr",
			addr, ETH_ALEN, cfg->ioctl_buf, WLC_IOCTL_MAXLEN,
			event->bssidx, &cfg->ioctl_buf_sync);
		if (unlikely(ret)) {
				WL_ERR(("set cur_etheraddr Error (%d)\n", ret));
				goto fail;
		}
		memcpy(new_ndev->dev_addr, addr, ETH_ALEN);
	}

	if (wl_cfg80211_register_if(cfg, event->ifidx, new_ndev) != BCME_OK) {
		WL_ERR(("IFACE register failed \n"));
		goto fail;
	}

	/* Initialize with the station mode params */
	wl_alloc_netinfo(cfg, new_ndev, wdev,
		(iface_type == NL80211_IFTYPE_STATION) ?
		WL_MODE_BSS : WL_MODE_AP, PM_ENABLE, event->bssidx);
	cfg->bss_cfgdev = ndev_to_cfgdev(new_ndev);
	cfg->cfgdev_bssidx = event->bssidx;

	WL_DBG(("Host Network Interface for Secondary I/F created"));

#ifdef DHD_IFDEBUG
	WL_ERR(("cfg->bss_cfgdev=%p\n", cfg->bss_cfgdev));
#endif

	return cfg->bss_cfgdev;

fail:
	cfg->bss_pending_op = FALSE;
	cfg->cfgdev_bssidx = -1;
	if (wdev)
		kfree(wdev);
	if (new_ndev)
		wl_cfg80211_remove_if(cfg, event->ifidx, new_ndev);

#ifdef DHD_IFDEBUG
	WL_ERR(("failed!!!\n"));
#endif

	return NULL;
}
s32
wl_cfg80211_del_iface(struct wiphy *wiphy, bcm_struct_cfgdev *cfgdev)
{
	struct bcm_cfg80211 *cfg = wiphy_priv(wiphy);
	struct net_device *ndev = NULL;
#ifdef DHD_IFDEBUG
	struct net_device *primary_ndev = NULL;
#endif
	s32 ret = BCME_OK;
	s32 bsscfg_idx = 1;
	u32 timeout;
	u32 ifidx;
	enum nl80211_iftype iface_type = NL80211_IFTYPE_STATION;

	WL_ERR(("Enter\n"));

	if (!cfg->bss_cfgdev)
		return 0;

	/* If any scan is going on, abort it */
	if (wl_get_drv_status_all(cfg, SCANNING)) {
		WL_ERR(("Scan in progress. Aborting the scan!\n"));
		wl_notify_escan_complete(cfg, cfg->escan_info.ndev, true, true);
	}

	ndev = (struct net_device *)cfgdev_to_ndev(cfg->bss_cfgdev);
#ifdef DHD_IFDEBUG
	primary_ndev = bcmcfg_to_prmry_ndev(cfg);
	WL_ERR(("cfg->bss_cfgdev=%p, ndev=%p, primary_ndev=%p\n",
		cfg->bss_cfgdev, ndev, primary_ndev));
#endif

	cfg->bss_pending_op = TRUE;
	memset(&cfg->if_event_info, 0, sizeof(cfg->if_event_info));

	/* Delete the firmware interface. "interface_remove" command
	 * should go on the interface to be deleted
	 */
	ret = wl_cfg80211_interface_ops(cfg, ndev, cfg->cfgdev_bssidx,
		NL80211_IFTYPE_STATION, 1, NULL);
	if (ret == BCME_UNSUPPORTED) {
		if ((ret = wl_cfg80211_add_del_bss(cfg, ndev,
			bsscfg_idx, iface_type, true, NULL)) < 0) {
			WL_ERR(("DEL bss failed ret:%d \n", ret));
			goto exit;
		}
	} else if (ret < 0) {
		WL_ERR(("Interface DEL failed ret:%d \n", ret));
		goto exit;
	}

	timeout = wait_event_interruptible_timeout(cfg->netif_change_event,
		!cfg->bss_pending_op, msecs_to_jiffies(MAX_WAIT_TIME));
	if (timeout <= 0 || cfg->bss_pending_op) {
		WL_ERR(("timeout in waiting IF_DEL event\n"));
	}

exit:
	ifidx = dhd_net2idx(((struct dhd_pub *)(cfg->pub))->info, ndev);
	wl_cfg80211_remove_if(cfg, ifidx, ndev);
	cfg->bss_cfgdev = NULL;
	cfg->cfgdev_bssidx = -1;
	cfg->bss_pending_op = FALSE;

	WL_ERR(("IF_DEL Done.\n"));

	return ret;
}
#endif /* defined(WL_VIRTUAL_APSTA) || defined(DUAL_STA_STATIC_IF) */

#endif /* BRCM_RSDB */


#ifndef  BRCM_RSDB
static s32
bcm_cfg80211_del_ibss_if(struct wiphy *wiphy, bcm_struct_cfgdev *cfgdev)
{
	int err = 0;
	struct bcm_cfg80211 *cfg = wiphy_priv(wiphy);
	struct net_device *ndev = NULL;
	struct net_device *primary_ndev = NULL;
	s32 timeout;

	if (!cfgdev || cfg->ibss_cfgdev != cfgdev || ETHER_ISNULLADDR(&cfg->ibss_if_addr.octet))
		return -EINVAL;
	ndev = cfgdev_to_ndev(cfg->ibss_cfgdev);
	primary_ndev = bcmcfg_to_prmry_ndev(cfg);

	cfg->bss_pending_op = TRUE;
	memset(&cfg->if_event_info, 0, sizeof(cfg->if_event_info));
	err = wldev_iovar_setbuf(primary_ndev, "aibss_ifdel", &cfg->ibss_if_addr,
		sizeof(cfg->ibss_if_addr), cfg->ioctl_buf, WLC_IOCTL_MAXLEN, NULL);
	if (err) {
		WL_ERR(("IOVAR aibss_ifdel failed with error %d\n", err));
		goto fail;
	}
	timeout = wait_event_interruptible_timeout(cfg->netif_change_event,
		!cfg->bss_pending_op, msecs_to_jiffies(MAX_WAIT_TIME));
	if (timeout <= 0 || cfg->bss_pending_op) {
		WL_ERR(("timeout in waiting IF_DEL event\n"));
		goto fail;
	}

	wl_cfg80211_remove_if(cfg, cfg->if_event_info.ifidx, ndev);
	cfg->ibss_cfgdev = NULL;
	return 0;

fail:
	cfg->bss_pending_op = FALSE;
	return -1;
}
#endif
static s32
wl_cfg80211_join_ibss(struct wiphy *wiphy, struct net_device *dev,
	struct cfg80211_ibss_params *params)
{
	struct bcm_cfg80211 *cfg = wiphy_priv(wiphy);
	struct cfg80211_bss *bss;
	struct ieee80211_channel *chan;
	struct wl_join_params join_params;
	int scan_suppress;
	struct cfg80211_ssid ssid;
	s32 scan_retry = 0;
	s32 err = 0;
	size_t join_params_size;
	chanspec_t chanspec = 0;
	u32 param[2] = {0, 0};
	u32 bw_cap = 0;
#ifdef  BRCM_RSDB
#if defined(WLAIBSS) && defined(WLAIBSS_PS)
	s32 atim = 10;
#endif /* WLAIBSS & WLAIBSS_PS */
#endif
	WL_TRACE(("In\n"));
	RETURN_EIO_IF_NOT_UP(cfg);
	WL_INFORM(("JOIN BSSID:" MACDBG "\n", MAC2STRDBG(params->bssid)));
#ifdef BCM_PATCH_CVE_2016_2493
	if (!params->ssid || params->ssid_len <= 0 ||
		params->ssid_len > DOT11_MAX_SSID_LEN) {
#else
	if (!params->ssid || params->ssid_len <= 0) {
#endif
		WL_ERR(("Invalid parameter\n"));
		return -EINVAL;
	}
#if defined(WL_CFG80211_P2P_DEV_IF)
	chan = params->chandef.chan;
#else
	chan = params->channel;
#endif /* WL_CFG80211_P2P_DEV_IF */
	if (chan)
		cfg->channel = ieee80211_frequency_to_channel(chan->center_freq);
	if (wl_get_drv_status(cfg, CONNECTED, dev)) {
		struct wlc_ssid *lssid = (struct wlc_ssid *)wl_read_prof(cfg, dev, WL_PROF_SSID);
		u8 *bssid = (u8 *)wl_read_prof(cfg, dev, WL_PROF_BSSID);
		u32 *channel = (u32 *)wl_read_prof(cfg, dev, WL_PROF_CHAN);
		if (!params->bssid || ((memcmp(params->bssid, bssid, ETHER_ADDR_LEN) == 0) &&
			(memcmp(params->ssid, lssid->SSID, lssid->SSID_len) == 0) &&
			(*channel == cfg->channel))) {
			WL_ERR(("Connection already existed to " MACDBG "\n",
				MAC2STRDBG((u8 *)wl_read_prof(cfg, dev, WL_PROF_BSSID))));
			return -EISCONN;
		}
		WL_ERR(("Ignore Previous connecton to %s (" MACDBG ")\n",
			lssid->SSID, MAC2STRDBG(bssid)));
	}

	/* remove the VSIE */
	wl_cfg80211_ibss_vsie_delete(dev);

	bss = cfg80211_get_ibss(wiphy, NULL, params->ssid, params->ssid_len);
	if (!bss) {
		if (IBSS_INITIAL_SCAN_ALLOWED == TRUE) {
			memcpy(ssid.ssid, params->ssid, params->ssid_len);
			ssid.ssid_len = params->ssid_len;
			do {
				if (unlikely
					(__wl_cfg80211_scan(wiphy, dev, NULL, &ssid) ==
					 -EBUSY)) {
					wl_delay(150);
				} else {
					break;
				}
			} while (++scan_retry < WL_SCAN_RETRY_MAX);

			/* rtnl lock code is removed here. don't see why rtnl lock
			 * needs to be released.
			 */

			/* wait 4 secons till scan done.... */
			schedule_timeout_interruptible(msecs_to_jiffies(4000));

			bss = cfg80211_get_ibss(wiphy, NULL,
				params->ssid, params->ssid_len);
		}
	}
	if (bss && ((IBSS_COALESCE_ALLOWED == TRUE) ||
		((IBSS_COALESCE_ALLOWED == FALSE) && params->bssid &&
		!memcmp(bss->bssid, params->bssid, ETHER_ADDR_LEN)))) {
		cfg->ibss_starter = false;
		WL_DBG(("Found IBSS\n"));
	} else {
		cfg->ibss_starter = true;
	}
	if (chan) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0))
		if (chan->band == NL80211_BAND_5GHZ)
			param[0] = WLC_BAND_5G;
		else if (chan->band == NL80211_BAND_2GHZ)
			param[0] = WLC_BAND_2G;
#else
		if (chan->band == IEEE80211_BAND_5GHZ)
			param[0] = WLC_BAND_5G;
		else if (chan->band == IEEE80211_BAND_2GHZ)
			param[0] = WLC_BAND_2G;
#endif
		err = wldev_iovar_getint(dev, "bw_cap", param);
		if (unlikely(err)) {
			WL_ERR(("Get bw_cap Failed (%d)\n", err));
			return err;
		}
		bw_cap = param[0];
		chanspec = channel_to_chanspec(wiphy, dev, cfg->channel, bw_cap);
	}
	/*
	 * Join with specific BSSID and cached SSID
	 * If SSID is zero join based on BSSID only
	 */
	memset(&join_params, 0, sizeof(join_params));
	memcpy((void *)join_params.ssid.SSID, (void *)params->ssid,
		params->ssid_len);
	join_params.ssid.SSID_len = htod32(params->ssid_len);
	if (params->bssid) {
		memcpy(&join_params.params.bssid, params->bssid, ETHER_ADDR_LEN);
		err = wldev_ioctl(dev, WLC_SET_DESIRED_BSSID, &join_params.params.bssid,
			ETHER_ADDR_LEN, true);
		if (unlikely(err)) {
			WL_ERR(("Error (%d)\n", err));
			return err;
		}
	} else
		memset(&join_params.params.bssid, 0, ETHER_ADDR_LEN);
	wldev_iovar_setint(dev, "ibss_coalesce_allowed", IBSS_COALESCE_ALLOWED);

	if (IBSS_INITIAL_SCAN_ALLOWED == FALSE) {
		scan_suppress = TRUE;
		/* Set the SCAN SUPPRESS Flag in the firmware to skip join scan */
		err = wldev_ioctl(dev, WLC_SET_SCANSUPPRESS,
			&scan_suppress, sizeof(int), true);
		if (unlikely(err)) {
			WL_ERR(("Scan Suppress Setting Failed (%d)\n", err));
			return err;
		}
	}

	join_params.params.chanspec_list[0] = chanspec;
	join_params.params.chanspec_num = 1;
	wldev_iovar_setint(dev, "chanspec", chanspec);
	join_params_size = sizeof(join_params);

	/* Disable Authentication, IBSS will add key if it required */
	wldev_iovar_setint(dev, "wpa_auth", WPA_AUTH_DISABLED);
	wldev_iovar_setint(dev, "wsec", 0);
#ifdef  BRCM_RSDB
#ifdef WLAIBSS
	/* Enable custom ibss features */
	err = wldev_iovar_setint(dev, "aibss", TRUE);

	if (unlikely(err)) {
		WL_ERR(("Enable custom IBSS mode failed (%d)\n", err));
		return err;
	}
#ifdef WLAIBSS_PS
	err = wldev_ioctl(dev, WLC_SET_ATIM, &atim, sizeof(int), true);
	if (unlikely(err)) {
		WL_ERR(("Enable custom IBSS ATIM mode failed (%d)\n", err));
		return err;
	}
#endif /* WLAIBSS_PS */
#endif /* WLAIBSS */
#endif /* BRCM_RSDB */

	err = wldev_ioctl(dev, WLC_SET_SSID, &join_params,
		join_params_size, true);
	if (unlikely(err)) {
		WL_ERR(("Error (%d)\n", err));
		return err;
	}

	if (IBSS_INITIAL_SCAN_ALLOWED == FALSE) {
		scan_suppress = FALSE;
		/* Reset the SCAN SUPPRESS Flag */
		err = wldev_ioctl(dev, WLC_SET_SCANSUPPRESS,
			&scan_suppress, sizeof(int), true);
		if (unlikely(err)) {
			WL_ERR(("Reset Scan Suppress Flag Failed (%d)\n", err));
			return err;
		}
	}
	wl_update_prof(cfg, dev, NULL, &join_params.ssid, WL_PROF_SSID);
	wl_update_prof(cfg, dev, NULL, &cfg->channel, WL_PROF_CHAN);
#ifdef  BRCM_RSDB
#ifdef WLAIBSS
	cfg->aibss_txfail_seq = 0;	/* initialize the sequence */
#endif /* WLAIBSS */
	cfg->rmc_event_seq = 0; /* initialize rmcfail sequence */
#endif
	return err;
}

static s32 wl_cfg80211_leave_ibss(struct wiphy *wiphy, struct net_device *dev)
{
	struct bcm_cfg80211 *cfg = wiphy_priv(wiphy);
	s32 err = 0;
	scb_val_t scbval;
	u8 *curbssid;

	RETURN_EIO_IF_NOT_UP(cfg);
	wl_link_down(cfg);

	WL_ERR(("Leave IBSS\n"));
	curbssid = wl_read_prof(cfg, dev, WL_PROF_BSSID);
	wl_set_drv_status(cfg, DISCONNECTING, dev);
	scbval.val = 0;
	memcpy(&scbval.ea, curbssid, ETHER_ADDR_LEN);
	err = wldev_ioctl(dev, WLC_DISASSOC, &scbval,
		sizeof(scb_val_t), true);
	if (unlikely(err)) {
		wl_clr_drv_status(cfg, DISCONNECTING, dev);
		WL_ERR(("error(%d)\n", err));
		return err;
	}

	/* remove the VSIE */
	wl_cfg80211_ibss_vsie_delete(dev);

	return err;
}
#ifdef MFP
static int wl_cfg80211_get_rsn_capa(bcm_tlv_t *wpa2ie, u8* capa)
{
	u16 suite_count;
	wpa_suite_mcast_t *mcast;
	wpa_suite_ucast_t *ucast;
	u16 len;
	wpa_suite_auth_key_mgmt_t *mgmt;

	if (!wpa2ie)
		return -1;

	len = wpa2ie->len;
	mcast = (wpa_suite_mcast_t *)&wpa2ie->data[WPA2_VERSION_LEN];
	if ((len -= WPA_SUITE_LEN) <= 0)
		return BCME_BADLEN;
	ucast = (wpa_suite_ucast_t *)&mcast[1];
	suite_count = ltoh16_ua(&ucast->count);
	if ((suite_count > NL80211_MAX_NR_CIPHER_SUITES) ||
		(len -= (WPA_IE_SUITE_COUNT_LEN +
		(WPA_SUITE_LEN * suite_count))) <= 0)
		return BCME_BADLEN;

	mgmt = (wpa_suite_auth_key_mgmt_t *)&ucast->list[suite_count];
	suite_count = ltoh16_ua(&mgmt->count);

	if ((suite_count > NL80211_MAX_NR_CIPHER_SUITES) ||
		(len -= (WPA_IE_SUITE_COUNT_LEN +
		(WPA_SUITE_LEN * suite_count))) >= RSN_CAP_LEN) {
		capa[0] = *(u8 *)&mgmt->list[suite_count];
		capa[1] = *((u8 *)&mgmt->list[suite_count] + 1);
	} else
		return BCME_BADLEN;

	return 0;
}
#endif /* MFP */


static s32
wl_set_wpa_version(struct net_device *dev, struct cfg80211_connect_params *sme)
{
	struct bcm_cfg80211 *cfg = g_bcm_cfg;
	struct wl_security *sec;
	s32 val = 0;
	s32 err = 0;
	s32 bssidx;

	if (cfg == NULL)
	{
		WL_ERR(("cfg is NULL, ignore wl_set_wpa_version\n"));
		return BCME_ERROR;
	}
#ifndef  BRCM_RSDB
	if (wl_cfgp2p_find_idx(cfg, dev, &bssidx) != BCME_OK) {
		WL_ERR(("Find p2p index from dev(%p) failed\n", dev));
		return BCME_ERROR;
	}
#else
	if ((bssidx = wl_get_bssidx_by_wdev(cfg, dev->ieee80211_ptr)) < 0) {
		WL_ERR(("Find p2p index from wdev(%p) failed\n", dev->ieee80211_ptr));
		return BCME_ERROR;
	}
#endif

	if (sme->crypto.wpa_versions & NL80211_WPA_VERSION_1)
		val = WPA_AUTH_PSK |
#if defined(BCMCCX) && defined(BRCM_RSDB)
			WPA_AUTH_CCKM |
#endif
			WPA_AUTH_UNSPECIFIED;
	else if (sme->crypto.wpa_versions & NL80211_WPA_VERSION_2)
		val = WPA2_AUTH_PSK|
#if defined(BCMCCX) && defined(BRCM_RSDB)
			WPA2_AUTH_CCKM |
#endif
			WPA2_AUTH_UNSPECIFIED;
	else
		val = WPA_AUTH_DISABLED;

	if (is_wps_conn(sme))
		val = WPA_AUTH_DISABLED;

#ifdef BCMWAPI_WPI
	if (sme->crypto.wpa_versions & NL80211_WAPI_VERSION_1) {
		WL_DBG((" * wl_set_wpa_version, set wpa_auth"
			" to WPA_AUTH_WAPI 0x400"));
		val = WAPI_AUTH_PSK | WAPI_AUTH_UNSPECIFIED;
	}
#endif
	WL_DBG(("setting wpa_auth to 0x%0x\n", val));
	err = wldev_iovar_setint_bsscfg(dev, "wpa_auth", val, bssidx);
	if (unlikely(err)) {
		WL_ERR(("set wpa_auth failed (%d)\n", err));
		return err;
	}
	sec = wl_read_prof(cfg, dev, WL_PROF_SEC);
	sec->wpa_versions = sme->crypto.wpa_versions;
	return err;
}

#ifdef BCMWAPI_WPI
static s32
wl_set_set_wapi_ie(struct net_device *dev, struct cfg80211_connect_params *sme)
{
	struct bcm_cfg80211 *cfg = g_bcm_cfg;
	s32 err = 0;
	s32 bssidx;
	if (cfg == NULL)
	{
		WL_ERR(("cfg is NULL, ignore wl_set_set_wapi_ie\n"));
		return BCME_ERROR;
	}
#ifndef  BRCM_RSDB
	if (wl_cfgp2p_find_idx(cfg, dev, &bssidx) != BCME_OK) {
		WL_ERR(("Find p2p index from dev(%p) failed\n", dev));
		return BCME_ERROR;
	}
#else
	if ((bssidx = wl_get_bssidx_by_wdev(cfg, dev->ieee80211_ptr)) < 0) {
		WL_ERR(("Find p2p index from wdev(%p) failed\n", dev->ieee80211_ptr));
		return BCME_ERROR;
	}
#endif

	WL_DBG((" %s \n", __FUNCTION__));

	if (sme->crypto.wpa_versions & NL80211_WAPI_VERSION_1) {
		err = wldev_iovar_setbuf_bsscfg(dev, "wapiie", (void *)sme->ie, sme->ie_len,
			cfg->ioctl_buf, WLC_IOCTL_MAXLEN, bssidx, &cfg->ioctl_buf_sync);

		if (unlikely(err)) {
			WL_ERR(("===> set_wapi_ie Error (%d)\n", err));
			return err;
		}
	} else
		WL_DBG((" * skip \n"));
	return err;
}
#endif /* BCMWAPI_WPI */

static s32
wl_set_auth_type(struct net_device *dev, struct cfg80211_connect_params *sme)
{
	struct bcm_cfg80211 *cfg = g_bcm_cfg;
	struct wl_security *sec;
	s32 val = 0;
	s32 err = 0;
	s32 bssidx;

	if (cfg == NULL)
	{
		WL_ERR(("cfg is NULL, ignore wl_set_auth_type\n"));
		return BCME_ERROR;
	}
#ifndef  BRCM_RSDB
	if (wl_cfgp2p_find_idx(cfg, dev, &bssidx) != BCME_OK) {
		WL_ERR(("Find p2p index from dev(%p) failed\n", dev));
		return BCME_ERROR;
	}
#else
	if ((bssidx = wl_get_bssidx_by_wdev(cfg, dev->ieee80211_ptr)) < 0) {
		WL_ERR(("Find p2p index from wdev(%p) failed\n", dev->ieee80211_ptr));
		return BCME_ERROR;
	}
#endif

	switch (sme->auth_type) {
	case NL80211_AUTHTYPE_OPEN_SYSTEM:
		val = WL_AUTH_OPEN_SYSTEM;
		WL_DBG(("open system\n"));
		break;
	case NL80211_AUTHTYPE_SHARED_KEY:
		val = WL_AUTH_SHARED_KEY;
		WL_DBG(("shared key\n"));
		break;
	case NL80211_AUTHTYPE_AUTOMATIC:
		val = WL_AUTH_OPEN_SHARED;
		WL_DBG(("automatic\n"));
		break;
#if defined(BCMCCX) && defined(BRCM_RSDB)
	case NL80211_AUTHTYPE_NETWORK_EAP:
		WL_DBG(("network eap\n"));
		val = DOT11_LEAP_AUTH;
		break;
#endif
	default:
		val = 2;
		WL_ERR(("invalid auth type (%d)\n", sme->auth_type));
		break;
	}

	err = wldev_iovar_setint_bsscfg(dev, "auth", val, bssidx);
	if (unlikely(err)) {
		WL_ERR(("set auth failed (%d)\n", err));
		return err;
	}
	sec = wl_read_prof(cfg, dev, WL_PROF_SEC);
	sec->auth_type = sme->auth_type;
	return err;
}

static s32
wl_set_set_cipher(struct net_device *dev, struct cfg80211_connect_params *sme)
{
	struct bcm_cfg80211 *cfg = g_bcm_cfg;
	struct wl_security *sec;
	s32 pval = 0;
	s32 gval = 0;
	s32 err = 0;
	s32 wsec_val = 0;
#ifndef  BRCM_RSDB
#ifdef MFP
	s32 mfp = 0;
	bcm_tlv_t *wpa2_ie;
	u8 rsn_cap[2];
#endif /* MFP */
#endif
#ifdef BCMWAPI_WPI
	s32 val = 0;
#endif
	s32 bssidx;

	if (cfg == NULL)
	{
		WL_ERR(("cfg is NULL, ignore wl_set_set_cipher\n"));
		return BCME_ERROR;
	}
#ifndef  BRCM_RSDB
	if (wl_cfgp2p_find_idx(cfg, dev, &bssidx) != BCME_OK) {
		WL_ERR(("Find p2p index from dev(%p) failed\n", dev));
		return BCME_ERROR;
	}
#else
	if ((bssidx = wl_get_bssidx_by_wdev(cfg, dev->ieee80211_ptr)) < 0) {
		WL_ERR(("Find p2p index from wdev(%p) failed\n", dev->ieee80211_ptr));
		return BCME_ERROR;
	}
#endif

	if (sme->crypto.n_ciphers_pairwise) {
		switch (sme->crypto.ciphers_pairwise[0]) {
		case WLAN_CIPHER_SUITE_WEP40:
		case WLAN_CIPHER_SUITE_WEP104:
			pval = WEP_ENABLED;
			break;
		case WLAN_CIPHER_SUITE_TKIP:
			pval = TKIP_ENABLED;
			break;
		case WLAN_CIPHER_SUITE_CCMP:
		case WLAN_CIPHER_SUITE_AES_CMAC:
			pval = AES_ENABLED;
			break;
#ifdef BCMWAPI_WPI
		case WLAN_CIPHER_SUITE_SMS4:
			val = SMS4_ENABLED;
			pval = SMS4_ENABLED;
			break;
#endif
		default:
			WL_ERR(("invalid cipher pairwise (%d)\n",
				sme->crypto.ciphers_pairwise[0]));
			return -EINVAL;
		}
	}
#ifdef  BRCM_RSDB
#if defined(BCMSUP_4WAY_HANDSHAKE) && defined(WLAN_AKM_SUITE_FT_8021X)
	/* Ensure in-dongle supplicant is turned on when FBT wants to do the 4-way
	 * handshake.
	 * Note that the FW feature flag only exists on kernels that support the
	 * FT-EAP AKM suite.
	 */
	if (cfg->wdev->wiphy->features & NL80211_FEATURE_FW_4WAY_HANDSHAKE) {
		if (pval == AES_ENABLED)
			err = wldev_iovar_setint_bsscfg(dev, "sup_wpa", 1, bssidx);
		else
			err = wldev_iovar_setint_bsscfg(dev, "sup_wpa", 0, bssidx);

		if (err) {
			WL_ERR(("FBT: Error setting sup_wpa (%d)\n", err));
			return err;
		}
	}
#endif /* BCMSUP_4WAY_HANDSHAKE && WLAN_AKM_SUITE_FT_8021X */
#endif
	if (sme->crypto.cipher_group) {
		switch (sme->crypto.cipher_group) {
		case WLAN_CIPHER_SUITE_WEP40:
		case WLAN_CIPHER_SUITE_WEP104:
			gval = WEP_ENABLED;
			break;
		case WLAN_CIPHER_SUITE_TKIP:
			gval = TKIP_ENABLED;
			break;
		case WLAN_CIPHER_SUITE_CCMP:
			gval = AES_ENABLED;
			break;
		case WLAN_CIPHER_SUITE_AES_CMAC:
			gval = AES_ENABLED;
			break;
#ifdef BCMWAPI_WPI
		case WLAN_CIPHER_SUITE_SMS4:
			val = SMS4_ENABLED;
			gval = SMS4_ENABLED;
			break;
#endif
		default:
			WL_ERR(("invalid cipher group (%d)\n",
				sme->crypto.cipher_group));
			return -EINVAL;
		}
	}

	WL_DBG(("pval (%d) gval (%d)\n", pval, gval));

	if (is_wps_conn(sme)) {
		if (sme->privacy)
			err = wldev_iovar_setint_bsscfg(dev, "wsec", 4, bssidx);
		else
			/* WPS-2.0 allows no security */
			err = wldev_iovar_setint_bsscfg(dev, "wsec", 0, bssidx);
	} else {
#ifdef BCMWAPI_WPI
		if (sme->crypto.cipher_group == WLAN_CIPHER_SUITE_SMS4) {
			WL_DBG((" NO, is_wps_conn, WAPI set to SMS4_ENABLED"));
			err = wldev_iovar_setint_bsscfg(dev, "wsec", val, bssidx);
		} else {
#endif
			WL_DBG((" NO, is_wps_conn, Set pval | gval to WSEC"));
			wsec_val = pval | gval;
#ifndef  BRCM_RSDB
#ifdef MFP
			if (pval == AES_ENABLED) {
				if (((wpa2_ie = bcm_parse_tlvs((u8 *)sme->ie, sme->ie_len,
					DOT11_MNG_RSN_ID)) != NULL) &&
					(wl_cfg80211_get_rsn_capa(wpa2_ie, rsn_cap) == 0)) {

					if (rsn_cap[0] & RSN_CAP_MFPC) {
						/* MFP Capability advertised by supplicant. Check
						 * whether MFP is supported in the firmware
						 */
						if ((err = wldev_iovar_getint_bsscfg(dev,
								"mfp", &mfp, bssidx)) < 0) {
							WL_ERR(("Get MFP failed! "
								"Check MFP support in FW \n"));
							return -1;
						}

						if ((sme->crypto.n_akm_suites == 1) &&
							((sme->crypto.akm_suites[0] ==
							WL_AKM_SUITE_MFP_PSK) ||
							(sme->crypto.akm_suites[0] ==
							WL_AKM_SUITE_MFP_1X))) {
							wsec_val |= MFP_SHA256;
						} else if (sme->crypto.n_akm_suites > 1) {
							WL_ERR(("Multiple AKM Specified \n"));
							return -EINVAL;
						}

						wsec_val |= MFP_CAPABLE;
						if (rsn_cap[0] & RSN_CAP_MFPR)
							wsec_val |= MFP_REQUIRED;
					}
				}
			}
#endif /* MFP */
#endif
			WL_DBG((" Set WSEC to fW 0x%x \n", wsec_val));
			err = wldev_iovar_setint_bsscfg(dev, "wsec",
				wsec_val, bssidx);
#ifdef BCMWAPI_WPI
		}
#endif
	}
	if (unlikely(err)) {
		WL_ERR(("error (%d)\n", err));
		return err;
	}

	sec = wl_read_prof(cfg, dev, WL_PROF_SEC);
	sec->cipher_pairwise = sme->crypto.ciphers_pairwise[0];
	sec->cipher_group = sme->crypto.cipher_group;

	return err;
}

static s32
wl_set_key_mgmt(struct net_device *dev, struct cfg80211_connect_params *sme)
{
	struct bcm_cfg80211 *cfg = g_bcm_cfg;
	struct wl_security *sec;
	s32 val = 0;
	s32 err = 0;
	s32 bssidx;
#ifndef  BRCM_RSDB
	if (wl_cfgp2p_find_idx(cfg, dev, &bssidx) != BCME_OK) {
		WL_ERR(("Find p2p index from dev(%p) failed\n", dev));
		return BCME_ERROR;
	}
#else
#ifdef MFP
	s32 mfp = WL_MFP_NONE;
	bcm_tlv_t *wpa2_ie;
	u8 rsn_cap[2];
#endif /* MFP */

	if ((bssidx = wl_get_bssidx_by_wdev(cfg, dev->ieee80211_ptr)) < 0) {
		WL_ERR(("Find p2p index from wdev(%p) failed\n", dev->ieee80211_ptr));
		return BCME_ERROR;
	}
#endif

	if (sme->crypto.n_akm_suites) {
		err = wldev_iovar_getint(dev, "wpa_auth", &val);
		if (unlikely(err)) {
			WL_ERR(("could not get wpa_auth (%d)\n", err));
			return err;
		}
		if (val & (WPA_AUTH_PSK |
#if defined(BCMCCX) && defined(BRCM_RSDB)
			WPA_AUTH_CCKM |
#endif
			WPA_AUTH_UNSPECIFIED)) {
			switch (sme->crypto.akm_suites[0]) {
			case WLAN_AKM_SUITE_8021X:
				val = WPA_AUTH_UNSPECIFIED;
				break;
			case WLAN_AKM_SUITE_PSK:
				val = WPA_AUTH_PSK;
				break;
#if defined(BCMCCX) && defined(BRCM_RSDB)
			case WLAN_AKM_SUITE_CCKM:
				val = WPA_AUTH_CCKM;
				break;
#endif
			default:
#ifndef  BRCM_RSDB
				WL_ERR(("invalid cipher group (%d)\n",
					sme->crypto.cipher_group));
#else
				WL_ERR(("invalid akm suite (0x%x)\n",
					sme->crypto.akm_suites[0]));
#endif
				return -EINVAL;
			}
		} else if (val & (WPA2_AUTH_PSK |
#if defined(BCMCCX) && defined(BRCM_RSDB)
			WPA2_AUTH_CCKM |
#endif
			WPA2_AUTH_UNSPECIFIED)) {
			switch (sme->crypto.akm_suites[0]) {
			case WLAN_AKM_SUITE_8021X:
				val = WPA2_AUTH_UNSPECIFIED;
				break;
#ifdef MFP
#ifndef  BRCM_RSDB
			case WL_AKM_SUITE_MFP_1X:
				val = WPA2_AUTH_UNSPECIFIED;
				break;
			case WL_AKM_SUITE_MFP_PSK:
				val = WPA2_AUTH_PSK;
				break;
#else
			case WL_AKM_SUITE_SHA256_1X:
				val = WPA2_AUTH_1X_SHA256;
				break;
			case WL_AKM_SUITE_SHA256_PSK:
				val = WPA2_AUTH_PSK_SHA256;
				break;
#endif /* BRCM_RSDB */
#endif
			case WLAN_AKM_SUITE_PSK:
				val = WPA2_AUTH_PSK;
				break;
#if defined(WLFBT) && defined(WLAN_AKM_SUITE_FT_8021X)
			case WLAN_AKM_SUITE_FT_8021X:
				val = WPA2_AUTH_UNSPECIFIED | WPA2_AUTH_FT;
				break;
#endif
#if defined(WLFBT) && defined(WLAN_AKM_SUITE_FT_PSK)
			case WLAN_AKM_SUITE_FT_PSK:
				val = WPA2_AUTH_PSK | WPA2_AUTH_FT;
				break;
#endif
#ifdef  BRCM_RSDB
#ifdef BCMCCX
			case WLAN_AKM_SUITE_CCKM:
				val = WPA2_AUTH_CCKM;
				break;
#endif
#endif
			default:
#ifndef  BRCM_RSDB
				WL_ERR(("invalid cipher group (%d)\n",
					sme->crypto.cipher_group));
#else
				WL_ERR(("invalid akm suite (0x%x)\n",
					sme->crypto.akm_suites[0]));
#endif
				return -EINVAL;
			}
		}
#ifdef BCMWAPI_WPI
		else if (val & (WAPI_AUTH_PSK | WAPI_AUTH_UNSPECIFIED)) {
			switch (sme->crypto.akm_suites[0]) {
			case WLAN_AKM_SUITE_WAPI_CERT:
				val = WAPI_AUTH_UNSPECIFIED;
				break;
			case WLAN_AKM_SUITE_WAPI_PSK:
				val = WAPI_AUTH_PSK;
				break;
			default:
#ifndef  BRCM_RSDB
				WL_ERR(("invalid cipher group (%d)\n",
					sme->crypto.cipher_group));
#else
				WL_ERR(("invalid akm suite (0x%x)\n",
					sme->crypto.akm_suites[0]));
#endif
				return -EINVAL;
			}
		}
#endif
#ifdef  BRCM_RSDB
#ifdef MFP
		if (((wpa2_ie = bcm_parse_tlvs((u8 *)sme->ie, sme->ie_len,
			DOT11_MNG_RSN_ID)) != NULL) &&
			(wl_cfg80211_get_rsn_capa(wpa2_ie, rsn_cap) == 0)) {
				/* Check for MFP cap in the RSN capability field */
				if (rsn_cap[0] & RSN_CAP_MFPR) {
					mfp = WL_MFP_REQUIRED;
				} else if (rsn_cap[0] & RSN_CAP_MFPC) {
					mfp = WL_MFP_CAPABLE;
				}
		}
		err = wldev_iovar_setint(dev, "mfp", mfp);
		if (unlikely(err)) {
			if (!mfp && (err == BCME_UNSUPPORTED)) {
				/* For non-mfp cases, if firmware doesn't support MFP
				 * ignore the failure and proceed ahead.
				 */
				WL_DBG(("fw doesn't support mfp \n"));
				err = 0;
			} else {
				WL_ERR(("mfp set failed ret:%d \n", err));
				return err;
			}
		} else {
			WL_DBG(("mfp set to 0x%x \n", mfp));
		}
#endif /* MFP */
#endif

		WL_DBG(("setting wpa_auth to 0x%x\n", val));

		err = wldev_iovar_setint_bsscfg(dev, "wpa_auth", val, bssidx);
		if (unlikely(err)) {
			WL_ERR(("could not set wpa_auth (%d)\n", err));
			return err;
		}
	}
	sec = wl_read_prof(cfg, dev, WL_PROF_SEC);
	sec->wpa_auth = sme->crypto.akm_suites[0];

	return err;
}

static s32
wl_set_set_sharedkey(struct net_device *dev,
	struct cfg80211_connect_params *sme)
{
	struct bcm_cfg80211 *cfg = g_bcm_cfg;
	struct wl_security *sec;
	struct wl_wsec_key key;
	s32 val;
	s32 err = 0;
	s32 bssidx;
#ifndef  BRCM_RSDB
	if (wl_cfgp2p_find_idx(cfg, dev, &bssidx) != BCME_OK) {
		WL_ERR(("Find p2p index from dev(%p) failed\n", dev));
		return BCME_ERROR;
	}
#else
	if ((bssidx = wl_get_bssidx_by_wdev(cfg, dev->ieee80211_ptr)) < 0) {
		WL_ERR(("Find p2p index from wdev(%p) failed\n", dev->ieee80211_ptr));
		return BCME_ERROR;
	}
#endif

	WL_DBG(("key len (%d)\n", sme->key_len));
	if (sme->key_len) {
		sec = wl_read_prof(cfg, dev, WL_PROF_SEC);
		WL_DBG(("wpa_versions 0x%x cipher_pairwise 0x%x\n",
			sec->wpa_versions, sec->cipher_pairwise));
		if (!(sec->wpa_versions & (NL80211_WPA_VERSION_1 |
#ifdef BCMWAPI_WPI
			NL80211_WPA_VERSION_2 | NL80211_WAPI_VERSION_1)) &&
#else
			NL80211_WPA_VERSION_2)) &&
#endif
			(sec->cipher_pairwise & (WLAN_CIPHER_SUITE_WEP40 |
#ifdef BCMWAPI_WPI
		WLAN_CIPHER_SUITE_WEP104 | WLAN_CIPHER_SUITE_SMS4)))
#else
		WLAN_CIPHER_SUITE_WEP104)))
#endif
		{
			memset(&key, 0, sizeof(key));
			key.len = (u32) sme->key_len;
			key.index = (u32) sme->key_idx;
			if (unlikely(key.len > sizeof(key.data))) {
				WL_ERR(("Too long key length (%u)\n", key.len));
				return -EINVAL;
			}
			memcpy(key.data, sme->key, key.len);
			key.flags = WL_PRIMARY_KEY;
			switch (sec->cipher_pairwise) {
			case WLAN_CIPHER_SUITE_WEP40:
				key.algo = CRYPTO_ALGO_WEP1;
				break;
			case WLAN_CIPHER_SUITE_WEP104:
				key.algo = CRYPTO_ALGO_WEP128;
				break;
#ifdef BCMWAPI_WPI
			case WLAN_CIPHER_SUITE_SMS4:
				key.algo = CRYPTO_ALGO_SMS4;
				break;
#endif
			default:
				WL_ERR(("Invalid algorithm (%d)\n",
					sme->crypto.ciphers_pairwise[0]));
				return -EINVAL;
			}
			/* Set the new key/index */
			WL_DBG(("key length (%d) key index (%d) algo (%d)\n",
				key.len, key.index, key.algo));
			WL_DBG(("key \"%s\"\n", key.data));
			swap_key_from_BE(&key);
			err = wldev_iovar_setbuf_bsscfg(dev, "wsec_key", &key, sizeof(key),
				cfg->ioctl_buf, WLC_IOCTL_MAXLEN, bssidx, &cfg->ioctl_buf_sync);
			if (unlikely(err)) {
				WL_ERR(("WLC_SET_KEY error (%d)\n", err));
				return err;
			}
			if (sec->auth_type == NL80211_AUTHTYPE_SHARED_KEY) {
				WL_DBG(("set auth_type to shared key\n"));
				val = WL_AUTH_SHARED_KEY;	/* shared key */
				err = wldev_iovar_setint_bsscfg(dev, "auth", val, bssidx);
				if (unlikely(err)) {
					WL_ERR(("set auth failed (%d)\n", err));
					return err;
				}
			}
		}
	}
	return err;
}

#if defined(ESCAN_RESULT_PATCH)
static u8 connect_req_bssid[6];
static u8 broad_bssid[6];
#endif /* ESCAN_RESULT_PATCH */


#if defined(ROAM_CHANNEL_CACHE) && defined(BRCM_RSDB)
#define MAX_ROAM_CACHE_NUM 100
#endif /* ROAM_CHANNEL_CACHE */

#if defined(CUSTOM_SET_CPUCORE) || defined(CONFIG_TCPACK_FASTTX)
static bool wl_get_chan_isvht80(struct net_device *net, dhd_pub_t *dhd)
{
	u32 chanspec = 0;
	bool isvht80 = 0;

	if (wldev_iovar_getint(net, "chanspec", (s32 *)&chanspec) == BCME_OK)
		chanspec = wl_chspec_driver_to_host(chanspec);

	isvht80 = chanspec & WL_CHANSPEC_BW_80;
	WL_INFO(("%s: chanspec(%x:%d)\n", __FUNCTION__, chanspec, isvht80));

	return isvht80;
}
#endif /* CUSTOM_SET_CPUCORE || CONFIG_TCPACK_FASTTX */

#ifdef CONFIG_HW_ABS
static int wl_switch_siso(struct net_device *ndev, uint8 siso_mcs)
{
	int ret = -1;
	char iovbuf[WLC_IOCTL_SMLEN];

	bcm_mkiovar("sisosw", (char *)&siso_mcs, 1, iovbuf, sizeof(iovbuf));
	ret = wldev_ioctl(ndev, WLC_SET_VAR, iovbuf, sizeof(iovbuf), true);
	if (unlikely(ret)) {
		WL_ERR(("%s: Fail to switch siso, ret=%d\n", __FUNCTION__, ret));
		return ret;
	}

	return 0;
}
#endif

static s32
wl_cfg80211_connect(struct wiphy *wiphy, struct net_device *dev,
	struct cfg80211_connect_params *sme)
{
	struct bcm_cfg80211 *cfg = wiphy_priv(wiphy);
	struct ieee80211_channel *chan = sme->channel;
#ifdef CONFIG_HW_ABS
	extern bool g_abs_enabled;
#endif
#ifdef  BRCM_RSDB
	wl_extjoin_params_t *ext_join_params;
#endif
	struct wl_join_params join_params;
#ifndef  BRCM_RSDB
	struct ether_addr bssid;
	wl_extjoin_params_t *ext_join_params;
#endif
	size_t join_params_size;
#if defined(ROAM_ENABLE) && defined(ROAM_AP_ENV_DETECTION)
	dhd_pub_t *dhd =  (dhd_pub_t *)(cfg->pub);
	s32 roam_trigger[2] = {0, 0};
#endif /* ROAM_AP_ENV_DETECTION */
#ifdef  BRCM_RSDB
	s32 err = 0;
	wpa_ie_fixed_t *wpa_ie;
	bcm_tlv_t *wpa2_ie;
	u8* wpaie  = 0;
	u32 wpaie_len = 0;
	u32 chan_cnt = 0;
	struct ether_addr bssid;
	s32 bssidx = -1;
	int ret;
	int wait_cnt;
#else
	u8* wpaie  = 0;
	u8 chan_info[CHAN_INFO_LEN] = {0}, *chan_ptr;
	u32 wpaie_len = 0;
	u32 timeout;
	u32 chan_cnt = 0, i, w_count = 0;
	s32 wait_cnt;
	s32 bssidx;
	s32 err = 0;
	wpa_ie_fixed_t *wpa_ie;
	bcm_tlv_t *wpa2_ie;
	bool use_chan_cache = FALSE;
#endif
#ifdef ROAM_CHANNEL_CACHE
	chanspec_t chanspec_list[MAX_ROAM_CACHE_NUM];
#endif /* ROAM_CHANNEL_CACHE */
	WL_DBG(("In\n"));
#ifdef  BRCM_RSDB
#if defined(SUPPORT_RANDOM_MAC_SCAN)
	wl_cfg80211_set_random_mac(dev, FALSE);
#endif /* SUPPORT_RANDOM_MAC_SCAN */

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 15, 0))
	if (sme->channel_hint) {
		chan = sme->channel_hint;
		WL_DBG(("channel_hint (%d), channel_hint center_freq (%d)\n",
			ieee80211_frequency_to_channel(sme->channel_hint->center_freq),
			sme->channel_hint->center_freq));
	}
	if (sme->bssid_hint) {
		sme->bssid = sme->bssid_hint;
		WL_DBG(("bssid_hint "MACDBG" \n", MAC2STRDBG(sme->bssid_hint)));
	}
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(3, 15, 0) */
#endif /* BRCM_RSDB */
	if (unlikely(!sme->ssid)) {
		WL_ERR(("Invalid ssid\n"));
		return -EOPNOTSUPP;
	}

	if (unlikely(sme->ssid_len > DOT11_MAX_SSID_LEN)) {
		WL_ERR(("Invalid SSID info: SSID=%s, length=%zd\n",
			sme->ssid, sme->ssid_len));
		return -EINVAL;
	}

	RETURN_EIO_IF_NOT_UP(cfg);
#ifndef  BRCM_RSDB
	chan_ptr = chan_info;
#endif
	/*
	 * Cancel ongoing scan to sync up with sme state machine of cfg80211.
	 */
#ifndef  BRCM_RSDB
	if (cfg->scan_request) {
		wl_notify_escan_complete(cfg, dev, true, true);
	}
#else
#if (defined(BCM4334_CHIP) || defined(BCM4359_CHIP) || !defined(ESCAN_RESULT_PATCH))
	if (cfg->scan_request) {
		WL_TRACE_HW4(("Aborting the scan! \n"));
		wl_cfg80211_scan_abort(cfg);
		wait_cnt = MAX_SCAN_ABORT_WAIT_CNT;
		while (wl_get_drv_status(cfg, SCANNING, dev) && wait_cnt) {
			WL_DBG(("Waiting for SCANNING terminated, wait_cnt: %d\n", wait_cnt));
			wait_cnt--;
			OSL_SLEEP(WAIT_SCAN_ABORT_OSL_SLEEP_TIME);
		}
		if (wl_get_drv_status(cfg, SCANNING, dev)) {
			wl_notify_escan_complete(cfg, dev, true, true);
		}
	}
#endif
#endif
#ifdef WL_SCHED_SCAN
	if (cfg->sched_scan_req) {
		wl_cfg80211_sched_scan_stop(wiphy, bcmcfg_to_prmry_ndev(cfg));
	}
#endif
#if defined(WL_CFG80211_GON_COLLISION) && defined(BRCM_RSDB)
	/* init block gon req count  */
	cfg->block_gon_req_tx_count = 0;
	cfg->block_gon_req_rx_count = 0;
#endif /* WL_CFG80211_GON_COLLISION */
#if defined(ESCAN_RESULT_PATCH)
	if (sme->bssid)
		memcpy(connect_req_bssid, sme->bssid, ETHER_ADDR_LEN);
#ifdef BCM_PATCH_CONN_QUICK_SCAN
	else if (sme->bssid_hint)
		memcpy(connect_req_bssid, sme->bssid_hint, ETHER_ADDR_LEN);
#endif
	else
		bzero(connect_req_bssid, ETHER_ADDR_LEN);
	bzero(broad_bssid, ETHER_ADDR_LEN);
#endif
#if defined(USE_DYNAMIC_MAXPKT_RXGLOM)
	maxrxpktglom = 0;
#endif
	bzero(&bssid, sizeof(bssid));
	if (!wl_get_drv_status(cfg, CONNECTED, dev)&&
#ifndef  BRCM_RSDB
		(err = wldev_ioctl(dev, WLC_GET_BSSID, &bssid, ETHER_ADDR_LEN, false)) == 0) {
#else
		(ret = wldev_ioctl(dev, WLC_GET_BSSID, &bssid, ETHER_ADDR_LEN, false)) == 0) {
#endif
		if (!ETHER_ISNULLADDR(&bssid)) {
			scb_val_t scbval;
			wl_set_drv_status(cfg, DISCONNECTING, dev);
			scbval.val = DOT11_RC_DISASSOC_LEAVING;
			memcpy(&scbval.ea, &bssid, ETHER_ADDR_LEN);
			scbval.val = htod32(scbval.val);

			WL_DBG(("drv status CONNECTED is not set, but connected in FW!" MACDBG "/n",
				MAC2STRDBG(bssid.octet)));
			err = wldev_ioctl(dev, WLC_DISASSOC, &scbval,
				sizeof(scb_val_t), true);
			if (unlikely(err)) {
				wl_clr_drv_status(cfg, DISCONNECTING, dev);
				WL_ERR(("error (%d)\n", err));
#ifdef HW_WIFI_DMD_LOG
				hw_wifi_dsm_client_notify(DSM_WIFI_WLC_DISASSOC_ERROR,
				          "error (%d)\n", err);
#endif
				return err;
			}

			/* wait for disconnection termination, upto 200 ms */
#ifndef  BRCM_RSDB
			wait_cnt = 200/10;
#else
			wait_cnt = 500/10;
#endif
			while (wl_get_drv_status(cfg, DISCONNECTING, dev) && wait_cnt) {
				WL_DBG(("Waiting for disconnection terminated, wait_cnt: %d\n",
					wait_cnt));
				wait_cnt--;
				OSL_SLEEP(10);
			}
		} else
			WL_DBG(("Currently not associated!\n"));
#ifndef  BRCM_RSDB
	} else if (wl_get_drv_status(cfg, DISCONNECTING, dev)) {
		timeout = wait_event_interruptible_timeout(cfg->event_sync_wq,
					!wl_get_drv_status(cfg, DISCONNECTING, dev),
					msecs_to_jiffies(MAX_WAIT_TIME/3));
		if 	(timeout <= 0 || wl_get_drv_status(cfg, DISCONNECTING, dev)) {
			WL_ERR(("timeout in waiting disconnect event\n"));
		}
		wl_clr_drv_status(cfg, DISCONNECTING, dev);
#else
	} else {
		/* if status is DISCONNECTING, wait for disconnection terminated max 500 ms */
		wait_cnt = 200/10;
		while (wl_get_drv_status(cfg, DISCONNECTING, dev) && wait_cnt) {
			WL_DBG(("Waiting for disconnection terminated, wait_cnt: %d\n", wait_cnt));
			wait_cnt--;
			OSL_SLEEP(10);
		}
		if (wl_get_drv_status(cfg, DISCONNECTING, dev)) {
			WL_ERR(("Force clear DISCONNECTING status!\n"));
			wl_clr_drv_status(cfg, DISCONNECTING, dev);
		}
#endif
	}

	/* Clean BSSID */
	bzero(&bssid, sizeof(bssid));
	if (!wl_get_drv_status(cfg, DISCONNECTING, dev))
		wl_update_prof(cfg, dev, NULL, (void *)&bssid, WL_PROF_BSSID);

	if (p2p_is_on(cfg) && (dev != bcmcfg_to_prmry_ndev(cfg))) {
#ifndef  BRCM_RSDB
		/* we only allow to connect using virtual interface in case of P2P */
			if (wl_cfgp2p_find_idx(cfg, dev, &bssidx) != BCME_OK) {
				WL_ERR(("Find p2p index from dev(%p) failed\n", dev));
				return BCME_ERROR;
			}
			wl_cfgp2p_set_management_ie(cfg, dev, bssidx,
				VNDR_IE_ASSOCREQ_FLAG, sme->ie, sme->ie_len);
#else
		/* we only allow to connect using virtual interface in case of P2P */
			if ((bssidx = wl_get_bssidx_by_wdev(cfg, dev->ieee80211_ptr)) < 0) {
				WL_ERR(("Find p2p index from wdev(%p) failed\n",
					dev->ieee80211_ptr));
				return BCME_ERROR;
			}
			wl_cfg80211_set_mgmt_vndr_ies(cfg, ndev_to_cfgdev(dev), bssidx,
				VNDR_IE_ASSOCREQ_FLAG, sme->ie, sme->ie_len);
#endif
	} else if (dev == bcmcfg_to_prmry_ndev(cfg)) {
		/* find the RSN_IE */
		if ((wpa2_ie = bcm_parse_tlvs((u8 *)sme->ie, sme->ie_len,
			DOT11_MNG_RSN_ID)) != NULL) {
			WL_DBG((" WPA2 IE is found\n"));
		}
		/* find the WPA_IE */
		if ((wpa_ie = wl_cfgp2p_find_wpaie((u8 *)sme->ie,
			sme->ie_len)) != NULL) {
			WL_DBG((" WPA IE is found\n"));
		}
		if (wpa_ie != NULL || wpa2_ie != NULL) {
			wpaie = (wpa_ie != NULL) ? (u8 *)wpa_ie : (u8 *)wpa2_ie;
			wpaie_len = (wpa_ie != NULL) ? wpa_ie->length : wpa2_ie->len;
			wpaie_len += WPA_RSN_IE_TAG_FIXED_LEN;
#ifndef  BRCM_RSDB
			wldev_iovar_setbuf(dev, "wpaie", wpaie, wpaie_len,
				cfg->ioctl_buf, WLC_IOCTL_MAXLEN, &cfg->ioctl_buf_sync);
#else
			err = wldev_iovar_setbuf(dev, "wpaie", wpaie, wpaie_len,
				cfg->ioctl_buf, WLC_IOCTL_MAXLEN, &cfg->ioctl_buf_sync);
			if (unlikely(err)) {
				WL_ERR(("wpaie set error (%d)\n", err));
				return err;
			}
#endif
		} else {
#ifndef  BRCM_RSDB
			wldev_iovar_setbuf(dev, "wpaie", NULL, 0,
				cfg->ioctl_buf, WLC_IOCTL_MAXLEN, &cfg->ioctl_buf_sync);
		}

		if (wl_cfgp2p_find_idx(cfg, dev, &bssidx) != BCME_OK) {
			WL_ERR(("Find p2p index from dev(%p) failed\n", dev));
			return BCME_ERROR;
		}
		err = wl_cfgp2p_set_management_ie(cfg, dev, bssidx,
			VNDR_IE_ASSOCREQ_FLAG, (u8 *)sme->ie, sme->ie_len);
#else
		err = wldev_iovar_setbuf(dev, "wpaie", NULL, 0,
				cfg->ioctl_buf, WLC_IOCTL_MAXLEN, &cfg->ioctl_buf_sync);
			if (unlikely(err)) {
				WL_ERR(("wpaie set error (%d)\n", err));
				return err;
			}
		}

		if ((bssidx = wl_get_bssidx_by_wdev(cfg, dev->ieee80211_ptr)) < 0) {
			WL_ERR(("Find p2p index from wdev(%p) failed\n", dev->ieee80211_ptr));
			return BCME_ERROR;
		}
		err = wl_cfg80211_set_mgmt_vndr_ies(cfg, ndev_to_cfgdev(dev), bssidx,
			VNDR_IE_ASSOCREQ_FLAG, (const u8 *)sme->ie, sme->ie_len);
#endif
		if (unlikely(err)) {
			return err;
		}
	}
#if defined(ROAM_ENABLE) && defined(ROAM_AP_ENV_DETECTION)
	if (dhd->roam_env_detection) {
		bool is_roamtrig_reset = TRUE;
		bool is_roam_env_ok = (wldev_iovar_setint(dev, "roam_env_detection",
			AP_ENV_DETECT_NOT_USED) == BCME_OK);
#if defined(KEEP_CUSTOM_ROAM_TRIGGER) && defined(BRCM_RSDB)
		roam_trigger[1] = WLC_BAND_2G;
		is_roamtrig_reset =
			(wldev_ioctl(dev, WLC_GET_ROAM_TRIGGER, roam_trigger,
			sizeof(roam_trigger), 0) == BCME_OK) &&
			(roam_trigger[0] == WL_AUTO_ROAM_TRIGGER-10);
#endif /* KEEP_CUSTOM_ROAM_TRIGGER */
		if (is_roamtrig_reset && is_roam_env_ok) {
			roam_trigger[0] = WL_AUTO_ROAM_TRIGGER;
			roam_trigger[1] = WLC_BAND_ALL;
		err = wldev_ioctl(dev, WLC_SET_ROAM_TRIGGER, roam_trigger,
			sizeof(roam_trigger), true);
		if (unlikely(err)) {
				WL_ERR((" failed to restore roam_trigger for auto env"
					" detection\n"));
			}
		}
	}
#endif /* ROAM_ENABLE && ROAM_AP_ENV_DETECTION */
	if (chan) {
		/* If RCC is not enabled, use the channel provided by userspace */
		cfg->channel = ieee80211_frequency_to_channel(chan->center_freq);
		chan_cnt = 1;
		WL_DBG(("channel (%d), center_req (%d), %d channels\n", cfg->channel,
			chan->center_freq, chan_cnt));
	} else {
#ifndef  BRCM_RSDB
#ifdef ROAM_CHANNEL_CACHE
		wlc_ssid_t ssid;
		int band;
		use_chan_cache = TRUE;
		err = wldev_get_band(dev, &band);
		if (!err) {
			set_roam_band(band);
		}

		cfg->channel = 0;
		memcpy(ssid.SSID, sme->ssid, sme->ssid_len);
		ssid.SSID_len = sme->ssid_len;
		chan_cnt = get_roam_channel_list(cfg->channel, chanspec_list, &ssid, ioctl_version);
#else
		cfg->channel = 0;
#endif /* ROAM_CHANNEL_CACHE */
#else  /* BRCM_RSDB */
		/*
		 * No channel information from user space. if RCC is enabled, the RCC
		 * would prepare the channel list, else no channel would be provided
		 * and firmware would need to do a full channel scan.
		 */
		WL_DBG(("No channel info from user space\n"));
		cfg->channel = 0;
#ifdef ROAM_CHANNEL_CACHE
		if (cfg->rcc_enabled) {
			/* Use cached channels */
			wlc_ssid_t ssid;
			int band;

			err = wldev_get_band(dev, &band);
			if (!err) {
				set_roam_band(band);
			}

			if (chan) {
				cfg->channel = ieee80211_frequency_to_channel(chan->center_freq);
			} else {
				cfg->channel = 0;
			}
			memcpy(ssid.SSID, sme->ssid, sme->ssid_len);
			ssid.SSID_len = sme->ssid_len;
			chan_cnt = get_roam_channel_list(cfg->channel, chanspec_list,
				&ssid, ioctl_version);
			WL_DBG(("Using roam cache channel set. channel count:%d \n", chan_cnt));
		}
#endif /* ROAM_CHANNEL_CACHE */
#endif /* BRCM_RSDB */
	}
#ifdef BCMWAPI_WPI
	WL_DBG(("1. enable wapi auth\n"));
	if (sme->crypto.wpa_versions & NL80211_WAPI_VERSION_1) {
		WL_DBG(("2. set wapi ie  \n"));
		err = wl_set_set_wapi_ie(dev, sme);
		if (unlikely(err))
			return err;
	} else
		WL_DBG(("2. Not wapi ie  \n"));
#endif


	WL_DBG(("ie (%p), ie_len (%zd)\n", sme->ie, sme->ie_len));
	WL_DBG(("3. set wpa version \n"));
	err = wl_set_wpa_version(dev, sme);
	if (unlikely(err)) {
		WL_ERR(("Invalid wpa_version\n"));
		return err;
	}
#ifdef BCMWAPI_WPI
#ifndef  BRCM_RSDB
	if (sme->crypto.wpa_versions & NL80211_WAPI_VERSION_1) {
                WL_DBG(("4. WAPI, set auth type as OPEN SYSTEM\n"));
#ifdef HW_WAPI_AUTH_TYPE_BUG
                sme->auth_type = NL80211_AUTHTYPE_OPEN_SYSTEM;
                err = wl_set_auth_type(dev, sme);
                if (unlikely(err)) {
                        WL_ERR(("Invalid auth type\n"));
                        return err;
                }
#endif
        }
#else
	if (sme->crypto.wpa_versions & NL80211_WAPI_VERSION_1)
		WL_DBG(("4. WAPI Dont Set wl_set_auth_type\n"));
#endif
	else {
		WL_DBG(("4. wl_set_auth_type\n"));
#endif
		err = wl_set_auth_type(dev, sme);
		if (unlikely(err)) {
			WL_ERR(("Invalid auth type\n"));
			return err;
		}
#ifdef BCMWAPI_WPI
	}
#endif

	err = wl_set_set_cipher(dev, sme);
	if (unlikely(err)) {
		WL_ERR(("Invalid ciper\n"));
		return err;
	}

	err = wl_set_key_mgmt(dev, sme);
	if (unlikely(err)) {
		WL_ERR(("Invalid key mgmt\n"));
		return err;
	}

	err = wl_set_set_sharedkey(dev, sme);
	if (unlikely(err)) {
		WL_ERR(("Invalid shared key\n"));
		return err;
	}

#ifdef CONFIG_HW_ABS
	if (true == g_abs_enabled) {
		uint8 siso_mcs = 0;
		if (sme->ht_capa.mcs.rx_mask[0] == 0xff && sme->ht_capa.mcs.rx_mask[1] == 0) {
			siso_mcs = 1;
		}
		WL_ERR(("sisosw: %d\n", siso_mcs));
		wl_switch_siso(dev, siso_mcs);
	}
#endif

	/*
	 *  Join with specific BSSID and cached SSID
	 *  If SSID is zero join based on BSSID only
	 */
	join_params_size = WL_EXTJOIN_PARAMS_FIXED_SIZE +
		chan_cnt * sizeof(chanspec_t);
	ext_join_params =  (wl_extjoin_params_t*)kzalloc(join_params_size, GFP_KERNEL);
	if (ext_join_params == NULL) {
		err = -ENOMEM;
		wl_clr_drv_status(cfg, CONNECTING, dev);
		goto exit;
	}
	ext_join_params->ssid.SSID_len = min(sizeof(ext_join_params->ssid.SSID), sme->ssid_len);
	memcpy(&ext_join_params->ssid.SSID, sme->ssid, ext_join_params->ssid.SSID_len);
	wl_update_prof(cfg, dev, NULL, &ext_join_params->ssid, WL_PROF_SSID);
	ext_join_params->ssid.SSID_len = htod32(ext_join_params->ssid.SSID_len);
	/* increate dwell time to receive probe response or detect Beacon
	* from target AP at a noisy air only during connect command
	*/
	ext_join_params->scan.active_time = chan_cnt ? WL_SCAN_JOIN_ACTIVE_DWELL_TIME_MS : -1;
	ext_join_params->scan.passive_time = chan_cnt ? WL_SCAN_JOIN_PASSIVE_DWELL_TIME_MS : -1;
	/* Set up join scan parameters */
	ext_join_params->scan.scan_type = -1;
	ext_join_params->scan.nprobes = chan_cnt ?
		(ext_join_params->scan.active_time/WL_SCAN_JOIN_PROBE_INTERVAL_MS) : -1;
	ext_join_params->scan.home_time = -1;

	if (sme->bssid)
		memcpy(&ext_join_params->assoc.bssid, sme->bssid, ETH_ALEN);
#ifdef BCM_PATCH_CONN_QUICK_SCAN
	else if (sme->bssid_hint)
		memcpy(&ext_join_params->assoc.bssid, sme->bssid_hint, ETH_ALEN);
#endif
	else
		memcpy(&ext_join_params->assoc.bssid, &ether_bcast, ETH_ALEN);

#ifdef WRONG_ACTION_PATCH
	wl_clear_wrong_action_handle(cfg, dev, ext_join_params);
#endif
		
	ext_join_params->assoc.chanspec_num = chan_cnt;
	if (chan_cnt) {
#ifndef  BRCM_RSDB
		if (use_chan_cache) {
			memcpy(ext_join_params->assoc.chanspec_list, chanspec_list,
				sizeof(chanspec_t) * chan_cnt);
			for (i = 0; i < chan_cnt; i++) {
				w_count += snprintf(chan_ptr + w_count, sizeof(chan_info) - w_count, "%d",
					wf_chspec_ctlchan(chanspec_list[i]));
				if (i != chan_cnt - 1) {
					w_count += snprintf(chan_ptr + w_count, sizeof(chan_info) - w_count, ", ");
				}
			}
		} else {
			u16 channel, band, bw, ctl_sb;
			chanspec_t chspec;
			channel = cfg->channel;
			band = (channel <= CH_MAX_2G_CHANNEL) ? WL_CHANSPEC_BAND_2G
				: WL_CHANSPEC_BAND_5G;
			bw = WL_CHANSPEC_BW_20;
#else
		if (cfg->channel) {
			/*
			 * Use the channel provided by userspace
			 */
			u16 channel, band, bw, ctl_sb;
			chanspec_t chspec;
			channel = cfg->channel;
			band = (channel <= CH_MAX_2G_CHANNEL) ? WL_CHANSPEC_BAND_2G
				: WL_CHANSPEC_BAND_5G;

			/* Get min_bw set for the interface */
			bw = wl_cfg80211_ulb_get_min_bw_chspec(dev->ieee80211_ptr, bssidx);
			if (bw == INVCHANSPEC) {
				WL_ERR(("Invalid chanspec \n"));
				kfree(ext_join_params);
				return BCME_ERROR;
			}
#endif
			ctl_sb = WL_CHANSPEC_CTL_SB_NONE;
			chspec = (channel | band | bw | ctl_sb);
			ext_join_params->assoc.chanspec_list[0]  &= WL_CHANSPEC_CHAN_MASK;
			ext_join_params->assoc.chanspec_list[0] |= chspec;
			ext_join_params->assoc.chanspec_list[0] =
				wl_chspec_host_to_driver(ext_join_params->assoc.chanspec_list[0]);
#ifndef  BRCM_RSDB
			snprintf(chan_ptr, sizeof(chan_info), "%d", channel);
		}
	} else {
		snprintf(chan_ptr, sizeof(chan_info), "0");
#else
	}
#ifdef ROAM_CHANNEL_CACHE
		else if (cfg->rcc_enabled) {
			memcpy(ext_join_params->assoc.chanspec_list, chanspec_list,
				sizeof(chanspec_t) * chan_cnt);
		}
#endif /* ROAM_CHANNEL_CACHE */
#endif /* BRCM_RSDB */
	}
	ext_join_params->assoc.chanspec_num = htod32(ext_join_params->assoc.chanspec_num);
	if (ext_join_params->ssid.SSID_len < IEEE80211_MAX_SSID_LEN) {
		WL_INFORM(("ssid \"%s\", len (%d)\n", ext_join_params->ssid.SSID,
			ext_join_params->ssid.SSID_len));
	}
	wl_set_drv_status(cfg, CONNECTING, dev);
#ifdef HW_WIFI_CLEAR_DISCONNECTING_FLAG
	if (wl_get_drv_status(cfg, DISCONNECTING, dev)) {
		WL_ERR(("clear disconnecting flag\n"));
		wl_clr_drv_status(cfg, DISCONNECTING, dev);
	}
#endif
#ifndef  BRCM_RSDB
	if (wl_cfgp2p_find_idx(cfg, dev, &bssidx) != BCME_OK) {
		WL_ERR(("Find p2p index from dev(%p) failed\n", dev));
#else
	if ((bssidx = wl_get_bssidx_by_wdev(cfg, dev->ieee80211_ptr)) < 0) {
		WL_ERR(("Find p2p index from wdev(%p) failed\n", dev->ieee80211_ptr));
		kfree(ext_join_params);
#endif
		return BCME_ERROR;
	}
	err = wldev_iovar_setbuf_bsscfg(dev, "join", ext_join_params, join_params_size,
		cfg->ioctl_buf, WLC_IOCTL_MAXLEN, bssidx, &cfg->ioctl_buf_sync);

#ifndef  BRCM_RSDB
	WL_ERR(("Connecting to " MACDBG " with channel (%s) ssid %s\n",
		MAC2STRDBG((u8*)(&ext_join_params->assoc.bssid)),
		chan_info, ext_join_params->ssid.SSID));
#else
	if (cfg->rcc_enabled) {
		WL_ERR(("Connecting with" MACDBG " ssid \"%s\", len (%d) with rcc channels \n\n",
			MAC2STRDBG((u8*)(&ext_join_params->assoc.bssid)),
			ext_join_params->ssid.SSID, ext_join_params->ssid.SSID_len));
	} else {
		WL_ERR(("Connecting with" MACDBG " ssid \"%s\", len (%d) channel=%d\n\n",
			MAC2STRDBG((u8*)(&ext_join_params->assoc.bssid)),
			ext_join_params->ssid.SSID, ext_join_params->ssid.SSID_len, cfg->channel));
	}
#endif /* BRCM_RSDB */

	kfree(ext_join_params);
	if (err) {
		wl_clr_drv_status(cfg, CONNECTING, dev);
		if (err == BCME_UNSUPPORTED) {
			WL_DBG(("join iovar is not supported\n"));
			goto set_ssid;
		} else {
			WL_ERR(("error (%d)\n", err));
			goto exit;
		}
	} else
		goto exit;

set_ssid:
	memset(&join_params, 0, sizeof(join_params));
	join_params_size = sizeof(join_params.ssid);

	join_params.ssid.SSID_len = min(sizeof(join_params.ssid.SSID), sme->ssid_len);
	memcpy(&join_params.ssid.SSID, sme->ssid, join_params.ssid.SSID_len);
	join_params.ssid.SSID_len = htod32(join_params.ssid.SSID_len);
	wl_update_prof(cfg, dev, NULL, &join_params.ssid, WL_PROF_SSID);
	if (sme->bssid)
		memcpy(&join_params.params.bssid, sme->bssid, ETH_ALEN);
	else
		memcpy(&join_params.params.bssid, &ether_bcast, ETH_ALEN);
#ifndef  BRCM_RSDB
	wl_ch_to_chanspec(cfg->channel, &join_params, &join_params_size);
#else
	if (wl_ch_to_chanspec(dev, cfg->channel, &join_params, &join_params_size) < 0) {
		WL_ERR(("Invalid chanspec\n"));
		return -EINVAL;
	}
#endif /* BRCM_RSDB */
	WL_DBG(("join_param_size %zu\n", join_params_size));

	if (join_params.ssid.SSID_len < IEEE80211_MAX_SSID_LEN) {
		WL_INFORM(("ssid \"%s\", len (%d)\n", join_params.ssid.SSID,
			join_params.ssid.SSID_len));
	}
	wl_set_drv_status(cfg, CONNECTING, dev);
	err = wldev_ioctl(dev, WLC_SET_SSID, &join_params, join_params_size, true);
	if (err) {
		WL_ERR(("error (%d)\n", err));
		wl_clr_drv_status(cfg, CONNECTING, dev);
	}
exit:
	return err;
}

static s32
wl_cfg80211_disconnect(struct wiphy *wiphy, struct net_device *dev,
	u16 reason_code)
{
	struct bcm_cfg80211 *cfg = wiphy_priv(wiphy);
	scb_val_t scbval;
	bool act = false;
	s32 err = 0;
	u8 *curbssid;
#ifdef CUSTOM_SET_CPUCORE
	dhd_pub_t *dhd = (dhd_pub_t *)(cfg->pub);
#endif /* CUSTOM_SET_CPUCORE */
	WL_ERR(("Reason %d\n", reason_code));
	RETURN_EIO_IF_NOT_UP(cfg);
	act = *(bool *) wl_read_prof(cfg, dev, WL_PROF_ACT);
	curbssid = wl_read_prof(cfg, dev, WL_PROF_BSSID);

#ifdef ESCAN_RESULT_PATCH
	if (wl_get_drv_status(cfg, CONNECTING, dev) && curbssid &&
			(memcmp(curbssid, connect_req_bssid, ETHER_ADDR_LEN) == 0)) {
		WL_ERR(("Disconnecting from connecting device: " MACDBG "\n",
			MAC2STRDBG(curbssid)));
		act = true;
	}
#endif /* ESCAN_RESULT_PATCH */

	if (act) {
		/*
		 * Cancel ongoing scan to sync up with sme state machine of cfg80211.
		 */
#ifndef  BRCM_RSDB
		/* Let scan aborted by FW */
		if (cfg->scan_request) {
			wl_notify_escan_complete(cfg, dev, true, true);
		}

		wl_set_drv_status(cfg, DISCONNECTING, dev);
		if (wl_get_drv_status(cfg, CONNECTING, dev)) {
			/* in case of associating status, this will abort assoc procedure */
			wl_notify_escan_complete(cfg, dev, false, true);

			/* send pseudo connection failure event */
			wl_send_event(dev, WLC_E_SET_SSID, WLC_E_STATUS_ABORT, 0);
                } else {
			scbval.val = reason_code;
			memcpy(&scbval.ea, curbssid, ETHER_ADDR_LEN);
			scbval.val = htod32(scbval.val);
			err = wldev_ioctl(dev, WLC_DISASSOC, &scbval,
					sizeof(scb_val_t), true);
			if (unlikely(err)) {
				wl_clr_drv_status(cfg, DISCONNECTING, dev);
				WL_ERR(("error (%d)\n", err));
				return err;
			}
		}
#else
#if (defined(BCM4334_CHIP) || !defined(ESCAN_RESULT_PATCH))
		/* Let scan aborted by F/W */
		if (cfg->scan_request) {
			WL_TRACE_HW4(("Aborting the scan! \n"));
			wl_notify_escan_complete(cfg, dev, true, true);
		}
#endif /* ESCAN_RESULT_PATCH */
		if (wl_get_drv_status(cfg, CONNECTING, dev) ||
			wl_get_drv_status(cfg, CONNECTED, dev)) {
				wl_set_drv_status(cfg, DISCONNECTING, dev);
				scbval.val = reason_code;
				memcpy(&scbval.ea, curbssid, ETHER_ADDR_LEN);
				scbval.val = htod32(scbval.val);
				err = wldev_ioctl(dev, WLC_DISASSOC, &scbval,
						sizeof(scb_val_t), true);
				if (unlikely(err)) {
					wl_clr_drv_status(cfg, DISCONNECTING, dev);
					WL_ERR(("error (%d)\n", err));
					return err;
				}
#if defined(BCM4358_CHIP)
				WL_ERR(("Wait for complete of disconnecting \n"));
				OSL_SLEEP(200);
#endif /* BCM4358_CHIP */
		}
#endif /* BRCM_RSDB */
	}
#ifdef CUSTOM_SET_CPUCORE
	/* set default cpucore */
	if (dev == bcmcfg_to_prmry_ndev(cfg)) {
		dhd->chan_isvht80 &= ~DHD_FLAG_STA_MODE;
		if (!(dhd->chan_isvht80))
			dhd_set_cpucore(dhd, FALSE);
	}
#endif /* CUSTOM_SET_CPUCORE */

	return err;
}

#if defined(WL_CFG80211_P2P_DEV_IF)
static s32
wl_cfg80211_set_tx_power(struct wiphy *wiphy, struct wireless_dev *wdev,
	enum nl80211_tx_power_setting type, s32 mbm)
#else
static s32
wl_cfg80211_set_tx_power(struct wiphy *wiphy,
	enum nl80211_tx_power_setting type, s32 dbm)
#endif /* WL_CFG80211_P2P_DEV_IF */
{

	struct bcm_cfg80211 *cfg = wiphy_priv(wiphy);
	struct net_device *ndev = bcmcfg_to_prmry_ndev(cfg);
	s32 err = 0;
#if defined(WL_CFG80211_P2P_DEV_IF)
	s32 dbm = MBM_TO_DBM(mbm);
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 3, 0)) || \
	defined(WL_COMPAT_WIRELESS) || defined(WL_SUPPORT_BACKPORTED_KPATCHES)
	dbm = MBM_TO_DBM(dbm);
#endif /* WL_CFG80211_P2P_DEV_IF */

	RETURN_EIO_IF_NOT_UP(cfg);
	switch (type) {
	case NL80211_TX_POWER_AUTOMATIC:
		break;
	case NL80211_TX_POWER_LIMITED:
		if (dbm < 0) {
			WL_ERR(("TX_POWER_LIMITTED - dbm is negative\n"));
			return -EINVAL;
		}
		break;
	case NL80211_TX_POWER_FIXED:
		if (dbm < 0) {
			WL_ERR(("TX_POWER_FIXED - dbm is negative..\n"));
			return -EINVAL;
		}
		break;
	}

	err = wl_set_tx_power(ndev, type, dbm);
	if (unlikely(err)) {
		WL_ERR(("error (%d)\n", err));
		return err;
	}

	cfg->conf->tx_power = dbm;

	return err;
}

#if defined(WL_CFG80211_P2P_DEV_IF)
static s32 wl_cfg80211_get_tx_power(struct wiphy *wiphy,
	struct wireless_dev *wdev, s32 *dbm)
#else
static s32 wl_cfg80211_get_tx_power(struct wiphy *wiphy, s32 *dbm)
#endif /* WL_CFG80211_P2P_DEV_IF */
{
	struct bcm_cfg80211 *cfg = wiphy_priv(wiphy);
	struct net_device *ndev = bcmcfg_to_prmry_ndev(cfg);
	s32 err = 0;

	RETURN_EIO_IF_NOT_UP(cfg);
	err = wl_get_tx_power(ndev, dbm);
	if (unlikely(err))
		WL_ERR(("error (%d)\n", err));

	return err;
}

static s32
wl_cfg80211_config_default_key(struct wiphy *wiphy, struct net_device *dev,
	u8 key_idx, bool unicast, bool multicast)
{
	struct bcm_cfg80211 *cfg = wiphy_priv(wiphy);
	u32 index;
	s32 wsec;
	s32 err = 0;
	s32 bssidx;
#ifndef  BRCM_RSDB
	if (wl_cfgp2p_find_idx(cfg, dev, &bssidx) != BCME_OK) {
		WL_ERR(("Find p2p index from dev(%p) failed\n", dev));
		return BCME_ERROR;
	}
#else
	if ((bssidx = wl_get_bssidx_by_wdev(cfg, dev->ieee80211_ptr)) < 0) {
		WL_ERR(("Find p2p index from dev(%p) failed\n", dev->ieee80211_ptr));
		return BCME_ERROR;
	}
#endif

	WL_DBG(("key index (%d)\n", key_idx));
	RETURN_EIO_IF_NOT_UP(cfg);
	err = wldev_iovar_getint_bsscfg(dev, "wsec", &wsec, bssidx);
	if (unlikely(err)) {
		WL_ERR(("WLC_GET_WSEC error (%d)\n", err));
		return err;
	}
	if (wsec == WEP_ENABLED) {
		/* Just select a new current key */
		index = (u32) key_idx;
		index = htod32(index);
		err = wldev_ioctl(dev, WLC_SET_KEY_PRIMARY, &index,
			sizeof(index), true);
		if (unlikely(err)) {
			WL_ERR(("error (%d)\n", err));
		}
	}
	return err;
}

static s32
wl_add_keyext(struct wiphy *wiphy, struct net_device *dev,
	u8 key_idx, const u8 *mac_addr, struct key_params *params)
{
	struct bcm_cfg80211 *cfg = wiphy_priv(wiphy);
	struct wl_wsec_key key;
	s32 err = 0;
	s32 bssidx;
	s32 mode = wl_get_mode_by_netdev(cfg, dev);
#ifndef  BRCM_RSDB
	if (wl_cfgp2p_find_idx(cfg, dev, &bssidx) != BCME_OK) {
		WL_ERR(("Find p2p index from dev(%p) failed\n", dev));
		return BCME_ERROR;
	}
#else
	if ((bssidx = wl_get_bssidx_by_wdev(cfg, dev->ieee80211_ptr)) < 0) {
		WL_ERR(("Find p2p index from wdev(%p) failed\n", dev->ieee80211_ptr));
		return BCME_ERROR;
	}
#endif
	memset(&key, 0, sizeof(key));
	key.index = (u32) key_idx;

	if (!ETHER_ISMULTI(mac_addr))
#ifndef  BRCM_RSDB
		memcpy((char *)&key.ea, (void *)mac_addr, ETHER_ADDR_LEN);
#else
		memcpy((char *)&key.ea, (const void *)mac_addr, ETHER_ADDR_LEN);
#endif
	key.len = (u32) params->key_len;

	/* check for key index change */
	if (key.len == 0) {
		/* key delete */
		swap_key_from_BE(&key);
		err = wldev_iovar_setbuf_bsscfg(dev, "wsec_key", &key, sizeof(key),
			cfg->ioctl_buf, WLC_IOCTL_MAXLEN, bssidx, &cfg->ioctl_buf_sync);
		if (unlikely(err)) {
			WL_ERR(("key delete error (%d)\n", err));
			return err;
		}
	} else {
		if (key.len > sizeof(key.data)) {
			WL_ERR(("Invalid key length (%d)\n", key.len));
			return -EINVAL;
		}
		WL_DBG(("Setting the key index %d\n", key.index));
		memcpy(key.data, params->key, key.len);

		if ((mode == WL_MODE_BSS) &&
			(params->cipher == WLAN_CIPHER_SUITE_TKIP)) {
			u8 keybuf[8];
			memcpy(keybuf, &key.data[24], sizeof(keybuf));
			memcpy(&key.data[24], &key.data[16], sizeof(keybuf));
			memcpy(&key.data[16], keybuf, sizeof(keybuf));
		}

		/* if IW_ENCODE_EXT_RX_SEQ_VALID set */
		if (params->seq && params->seq_len == 6) {
			/* rx iv */
			u8 *ivptr;
			ivptr = (u8 *) params->seq;
			key.rxiv.hi = (ivptr[5] << 24) | (ivptr[4] << 16) |
				(ivptr[3] << 8) | ivptr[2];
			key.rxiv.lo = (ivptr[1] << 8) | ivptr[0];
			key.iv_initialized = true;
		}

		switch (params->cipher) {
		case WLAN_CIPHER_SUITE_WEP40:
			key.algo = CRYPTO_ALGO_WEP1;
			WL_DBG(("WLAN_CIPHER_SUITE_WEP40\n"));
			break;
		case WLAN_CIPHER_SUITE_WEP104:
			key.algo = CRYPTO_ALGO_WEP128;
			WL_DBG(("WLAN_CIPHER_SUITE_WEP104\n"));
			break;
		case WLAN_CIPHER_SUITE_TKIP:
			key.algo = CRYPTO_ALGO_TKIP;
			WL_DBG(("WLAN_CIPHER_SUITE_TKIP\n"));
			break;
		case WLAN_CIPHER_SUITE_AES_CMAC:
			key.algo = CRYPTO_ALGO_AES_CCM;
			WL_DBG(("WLAN_CIPHER_SUITE_AES_CMAC\n"));
			break;
		case WLAN_CIPHER_SUITE_CCMP:
			key.algo = CRYPTO_ALGO_AES_CCM;
			WL_DBG(("WLAN_CIPHER_SUITE_CCMP\n"));
			break;
#ifdef BCMWAPI_WPI
		case WLAN_CIPHER_SUITE_SMS4:
			key.algo = CRYPTO_ALGO_SMS4;
			WL_DBG(("WLAN_CIPHER_SUITE_SMS4\n"));
			break;
#endif
		default:
			WL_ERR(("Invalid cipher (0x%x)\n", params->cipher));
			return -EINVAL;
		}
		swap_key_from_BE(&key);
		/* need to guarantee EAPOL 4/4 send out before set key */
		dhd_wait_pend8021x(dev);
		err = wldev_iovar_setbuf_bsscfg(dev, "wsec_key", &key, sizeof(key),
			cfg->ioctl_buf, WLC_IOCTL_MAXLEN, bssidx, &cfg->ioctl_buf_sync);
		if (unlikely(err)) {
			WL_ERR(("WLC_SET_KEY error (%d)\n", err));
			return err;
		}
	}
	return err;
}

#ifdef BCM_BLOCK_DATA_FRAME
struct bcm_cfg80211 *wl_get_cfg(struct net_device *ndev)
{
	struct wireless_dev *wdev = ndev->ieee80211_ptr;

	if (!wdev) {
		WL_ERR(("%s, invalid parameter\n", __func__));
		return NULL;
	}

	return wiphy_priv(wdev->wiphy);
}

extern void dhd_set_blockframe(dhd_pub_t *dhd, int enable, int timer, int index);
int
wl_cfg80211_set_blockframe(struct net_device *dev, char *command, int len)
{
	int enable = 0;
	struct bcm_cfg80211 *cfg = wl_get_cfg(dev);
	char *p;
	int timer = 0;
	int i;
	dhd_pub_t *dhd =  (dhd_pub_t *)(cfg->pub);

	/* Make sure there are 2 parameters at least */
	if (command == NULL || len < 3) {
		WL_ERR(("%s, invalid command\n", __func__));
		return 0;
	}

	enable = *(command) - '0';
	if (enable == 1) {
		p = command + 2;
		for (i = 2; i < len; i++, p++) {
			if ((*p > '9' ) || (*p < '0')){
				break;
			}
			timer = timer * 10 + (*p - '0');
		}
	}
	else if (enable != 0) {
		WL_ERR(("%s, invalid parameters\n", __func__));
		return 0;
	}
	WL_ERR(("%s: set dhd->blockframe_flag=%x, timer = %d\n", __func__, enable, timer));
	dhd_set_blockframe(dhd, enable, timer, dev->ifindex);
	return 0;
}
#endif

#ifndef  BRCM_RSDB
int
wl_cfg80211_enable_roam_offload(struct net_device *dev, bool enable)
#else
int
wl_cfg80211_enable_roam_offload(struct net_device *dev, int enable)
#endif
{
	int err;
	wl_eventmsg_buf_t ev_buf;

	if (dev != bcmcfg_to_prmry_ndev(g_bcm_cfg)) {
		/* roam offload is only for the primary device */
		return -1;
	}
	err = wldev_iovar_setint(dev, "roam_offload", (int)enable);
	if (err)
		return err;
#ifdef  BRCM_RSDB
	if (enable) {
		err = wldev_iovar_setint(dev, "sup_wpa_tmo", IDSUP_4WAY_HANDSHAKE_TIMEOUT);
		if (err) {
			WL_INFORM(("Setting 'sup_wpa_tmo' failed, err=%d\n", err));
		}
	}
#endif
	bzero(&ev_buf, sizeof(wl_eventmsg_buf_t));
#ifndef  BRCM_RSDB
	wl_cfg80211_add_to_eventbuffer(&ev_buf, WLC_E_PSK_SUP, !enable);
#endif
	wl_cfg80211_add_to_eventbuffer(&ev_buf, WLC_E_ASSOC_REQ_IE, !enable);
	wl_cfg80211_add_to_eventbuffer(&ev_buf, WLC_E_ASSOC_RESP_IE, !enable);
	wl_cfg80211_add_to_eventbuffer(&ev_buf, WLC_E_REASSOC, !enable);
	wl_cfg80211_add_to_eventbuffer(&ev_buf, WLC_E_JOIN, !enable);
	wl_cfg80211_add_to_eventbuffer(&ev_buf, WLC_E_ROAM, !enable);
	err = wl_cfg80211_apply_eventbuffer(dev, g_bcm_cfg, &ev_buf);
	if (!err) {
		g_bcm_cfg->roam_offload = enable;
	}
	return err;
}

#ifdef HW_SET_PM
s32 wl_cfg80211_powerlock(struct net_device *dev, bool enable)
{

	s32 pm = PM_OFF;
	s32 ret = BCME_OK;

	if(enable) {
		pm = PM_OFF;
		ret = wldev_ioctl(dev,
			WLC_SET_PM, &pm, sizeof(pm), true);
		if (unlikely(ret))
			WL_ERR(("SET PM error (%d)\n", ret));

	    wl_cfg80211_update_power_mode(dev);

		dhd_power_lock_status = TRUE;
	} else {
		dhd_power_lock_status = FALSE;

		pm = PM_FAST;
		ret = wldev_ioctl(dev,
			WLC_SET_PM, &pm, sizeof(pm), true);
		if (unlikely(ret))
			WL_ERR(("SET PM error (%d)\n", ret));

		wl_cfg80211_update_power_mode(dev);
	}

	return ret;
}
#endif
#ifdef  BRCM_RSDB
#if defined(WL_VIRTUAL_APSTA)
int
wl_cfg80211_interface_create(struct net_device *dev, char *name)
{
	struct bcm_cfg80211 *cfg = g_bcm_cfg;
	bcm_struct_cfgdev *new_cfgdev;

	new_cfgdev = wl_cfg80211_create_iface(cfg->wdev->wiphy,
			NL80211_IFTYPE_STATION, NULL, name);
	if (!new_cfgdev) {
		return BCME_ERROR;
	}
	else {
		WL_DBG(("Iface %s created successfuly\n", name));
		return BCME_OK;
	}
}

int
wl_cfg80211_interface_delete(struct net_device *dev, char *name)
{
	struct bcm_cfg80211 *cfg = g_bcm_cfg;
	struct net_info *iter, *next;
	int err = BCME_ERROR;

	if (name == NULL) {
		return BCME_ERROR;
	}

	for_each_ndev(cfg, iter, next) {
		if (iter->ndev) {
			if (strcmp(iter->ndev->name, name) == 0) {
				err =  wl_cfg80211_del_iface(cfg->wdev->wiphy, cfg->bss_cfgdev);
				break;
			}
		}
	}
	if (!err) {
		WL_DBG(("Iface %s deleted successfuly", name));
	}
	return err;
}
#endif /* defined (WL_VIRTUAL_APSTA) */
#endif
static s32
wl_cfg80211_add_key(struct wiphy *wiphy, struct net_device *dev,
	u8 key_idx, bool pairwise, const u8 *mac_addr,
	struct key_params *params)
{
	struct wl_wsec_key key;
	s32 val = 0;
	s32 wsec = 0;
	s32 err = 0;
	u8 keybuf[8];
	s32 bssidx = 0;
	struct bcm_cfg80211 *cfg = wiphy_priv(wiphy);
	s32 mode = wl_get_mode_by_netdev(cfg, dev);
	WL_DBG(("key index (%d)\n", key_idx));
	RETURN_EIO_IF_NOT_UP(cfg);
#ifndef  BRCM_RSDB
	if (wl_cfgp2p_find_idx(cfg, dev, &bssidx) != BCME_OK) {
		WL_ERR(("Find p2p index from dev(%p) failed\n", dev));
		return BCME_ERROR;
	}
#else
	if ((bssidx = wl_get_bssidx_by_wdev(cfg, dev->ieee80211_ptr)) < 0) {
		WL_ERR(("Find p2p index from dev(%p) failed\n", dev->ieee80211_ptr));
		return BCME_ERROR;
	}
#endif

	if (mac_addr &&
		((params->cipher != WLAN_CIPHER_SUITE_WEP40) &&
		(params->cipher != WLAN_CIPHER_SUITE_WEP104))) {
			wl_add_keyext(wiphy, dev, key_idx, mac_addr, params);
			goto exit;
	}
	memset(&key, 0, sizeof(key));
#ifdef  BRCM_RSDB
	/* Clear any buffered wep key */
	memset(&cfg->wep_key, 0, sizeof(struct wl_wsec_key));
#endif
	key.len = (u32) params->key_len;
	key.index = (u32) key_idx;

	if (unlikely(key.len > sizeof(key.data))) {
		WL_ERR(("Too long key length (%u)\n", key.len));
		return -EINVAL;
	}
	memcpy(key.data, params->key, key.len);

	key.flags = WL_PRIMARY_KEY;
	switch (params->cipher) {
	case WLAN_CIPHER_SUITE_WEP40:
		key.algo = CRYPTO_ALGO_WEP1;
		val = WEP_ENABLED;
		WL_DBG(("WLAN_CIPHER_SUITE_WEP40\n"));
		break;
	case WLAN_CIPHER_SUITE_WEP104:
		key.algo = CRYPTO_ALGO_WEP128;
		val = WEP_ENABLED;
		WL_DBG(("WLAN_CIPHER_SUITE_WEP104\n"));
		break;
	case WLAN_CIPHER_SUITE_TKIP:
		key.algo = CRYPTO_ALGO_TKIP;
		val = TKIP_ENABLED;
		/* wpa_supplicant switches the third and fourth quarters of the TKIP key */
		if (mode == WL_MODE_BSS) {
			bcopy(&key.data[24], keybuf, sizeof(keybuf));
			bcopy(&key.data[16], &key.data[24], sizeof(keybuf));
			bcopy(keybuf, &key.data[16], sizeof(keybuf));
		}
		WL_DBG(("WLAN_CIPHER_SUITE_TKIP\n"));
		break;
	case WLAN_CIPHER_SUITE_AES_CMAC:
		key.algo = CRYPTO_ALGO_AES_CCM;
		val = AES_ENABLED;
		WL_DBG(("WLAN_CIPHER_SUITE_AES_CMAC\n"));
		break;
	case WLAN_CIPHER_SUITE_CCMP:
		key.algo = CRYPTO_ALGO_AES_CCM;
		val = AES_ENABLED;
		WL_DBG(("WLAN_CIPHER_SUITE_CCMP\n"));
		break;
#ifdef BCMWAPI_WPI
	case WLAN_CIPHER_SUITE_SMS4:
		key.algo = CRYPTO_ALGO_SMS4;
		WL_DBG(("WLAN_CIPHER_SUITE_SMS4\n"));
		val = SMS4_ENABLED;
		break;
#endif /* BCMWAPI_WPI */
#if defined(WLFBT) && defined(WLAN_CIPHER_SUITE_PMK)
	case WLAN_CIPHER_SUITE_PMK: {
		int j;
		wsec_pmk_t pmk;
		char keystring[WSEC_MAX_PSK_LEN + 1];
		char* charptr = keystring;
		uint len;
		struct wl_security *sec;

		sec = wl_read_prof(cfg, dev, WL_PROF_SEC);
		if (sec->wpa_auth == WLAN_AKM_SUITE_8021X) {
			err = wldev_iovar_setbuf(dev, "okc_info_pmk", (void *)params->key,
				WSEC_MAX_PSK_LEN / 2, keystring, sizeof(keystring), NULL);
			if (err) {
				/* could fail in case that 'okc' is not supported */
				WL_INFORM(("Setting 'okc_info_pmk' failed, err=%d\n", err));
			}
		}
		/* copy the raw hex key to the appropriate format */
		for (j = 0; j < (WSEC_MAX_PSK_LEN / 2); j++) {
			charptr += snprintf(charptr, sizeof(keystring), "%02x", params->key[j]);
		}
		len = strlen(keystring);
		pmk.key_len = htod16(len);
		bcopy(keystring, pmk.key, len);
		pmk.flags = htod16(WSEC_PASSPHRASE);
		err = wldev_ioctl(dev, WLC_SET_WSEC_PMK, &pmk, sizeof(pmk), true);
		if (err)
			return err;
	} break;
#endif /* WLFBT && WLAN_CIPHER_SUITE_PMK */
	default:
		WL_ERR(("Invalid cipher (0x%x)\n", params->cipher));
		return -EINVAL;
	}

	/* Set the new key/index */
	if ((mode == WL_MODE_IBSS) && (val & (TKIP_ENABLED | AES_ENABLED))) {
		WL_ERR(("IBSS KEY setted\n"));
		wldev_iovar_setint(dev, "wpa_auth", WPA_AUTH_NONE);
	}
	swap_key_from_BE(&key);
#ifdef  BRCM_RSDB
	if ((params->cipher == WLAN_CIPHER_SUITE_WEP40) ||
		(params->cipher == WLAN_CIPHER_SUITE_WEP104)) {
		/*
		 * For AP role, since we are doing a wl down before bringing up AP,
		 * the plumbed keys will be lost. So for AP once we bring up AP, we
		 * need to plumb keys again. So buffer the keys for future use. This
		 * is more like a WAR. If firmware later has the capability to do
		 * interface upgrade without doing a "wl down" and "wl apsta 0", then
		 * this will not be required.
		 */
		WL_DBG(("Buffering WEP Keys \n"));
		memcpy(&cfg->wep_key, &key, sizeof(struct wl_wsec_key));
	}
#endif
	err = wldev_iovar_setbuf_bsscfg(dev, "wsec_key", &key, sizeof(key), cfg->ioctl_buf,
		WLC_IOCTL_MAXLEN, bssidx, &cfg->ioctl_buf_sync);
	if (unlikely(err)) {
		WL_ERR(("WLC_SET_KEY error (%d)\n", err));
		return err;
	}

exit:
	err = wldev_iovar_getint_bsscfg(dev, "wsec", &wsec, bssidx);
	if (unlikely(err)) {
		WL_ERR(("get wsec error (%d)\n", err));
		return err;
	}

	wsec |= val;
	err = wldev_iovar_setint_bsscfg(dev, "wsec", wsec, bssidx);
	if (unlikely(err)) {
		WL_ERR(("set wsec error (%d)\n", err));
		return err;
	}

	return err;
}

static s32
wl_cfg80211_del_key(struct wiphy *wiphy, struct net_device *dev,
	u8 key_idx, bool pairwise, const u8 *mac_addr)
{
	struct wl_wsec_key key;
	struct bcm_cfg80211 *cfg = wiphy_priv(wiphy);
	s32 err = 0;
	s32 bssidx;
#ifndef  BRCM_RSDB
	if (wl_cfgp2p_find_idx(cfg, dev, &bssidx) != BCME_OK) {
		WL_ERR(("Find p2p index from dev(%p) failed\n", dev));
		return BCME_ERROR;
	}
	WL_DBG(("Enter\n"));
#else
	WL_DBG(("Enter. key_idx: %d\n", key_idx));
	if ((bssidx = wl_get_bssidx_by_wdev(cfg, dev->ieee80211_ptr)) < 0) {
		WL_ERR(("Find p2p index from wdev(%p) failed\n", dev->ieee80211_ptr));
		return BCME_ERROR;
	}
#endif
#ifndef  BRCM_RSDB
#ifndef IEEE80211W
	if ((key_idx >= DOT11_MAX_DEFAULT_KEYS) && (key_idx < DOT11_MAX_DEFAULT_KEYS+2))
		return -EINVAL;
#endif
#else
#ifndef MFP
	if ((key_idx >= DOT11_MAX_DEFAULT_KEYS) && (key_idx < DOT11_MAX_DEFAULT_KEYS+2))
		return -EINVAL;
#endif
#endif /* BRCM_RSDB */

	RETURN_EIO_IF_NOT_UP(cfg);
	memset(&key, 0, sizeof(key));

	key.flags = WL_PRIMARY_KEY;
	key.algo = CRYPTO_ALGO_OFF;
	key.index = (u32) key_idx;

	/* Set the new key/index */
	swap_key_from_BE(&key);
	err = wldev_iovar_setbuf_bsscfg(dev, "wsec_key", &key, sizeof(key), cfg->ioctl_buf,
		WLC_IOCTL_MAXLEN, bssidx, &cfg->ioctl_buf_sync);
	if (unlikely(err)) {
		if (err == -EINVAL) {
			if (key.index >= DOT11_MAX_DEFAULT_KEYS) {
				/* we ignore this key index in this case */
				WL_DBG(("invalid key index (%d)\n", key_idx));
			}
		} else {
			WL_ERR(("WLC_SET_KEY error (%d)\n", err));
		}
		return err;
	}
	return err;
}

static s32
wl_cfg80211_get_key(struct wiphy *wiphy, struct net_device *dev,
	u8 key_idx, bool pairwise, const u8 *mac_addr, void *cookie,
	void (*callback) (void *cookie, struct key_params * params))
{
	struct key_params params;
	struct wl_wsec_key key;
	struct bcm_cfg80211 *cfg = wiphy_priv(wiphy);
	struct wl_security *sec;
	s32 wsec;
	s32 err = 0;
	s32 bssidx;
#ifndef  BRCM_RSDB
	union {
		int32 index;
		uint8 tsc[DOT11_WPA_KEY_RSC_LEN];
	} u;
	if (wl_cfgp2p_find_idx(cfg, dev, &bssidx) != BCME_OK) {
		WL_ERR(("Find p2p index from dev(%p) failed\n", dev));
		return BCME_ERROR;
	}
#else
	if ((bssidx = wl_get_bssidx_by_wdev(cfg, dev->ieee80211_ptr)) < 0) {
		WL_ERR(("Find p2p index from wdev(%p) failed\n", dev->ieee80211_ptr));
		return BCME_ERROR;
	}
#endif
	WL_DBG(("key index (%d)\n", key_idx));
	RETURN_EIO_IF_NOT_UP(cfg);
	memset(&key, 0, sizeof(key));
	key.index = key_idx;
#ifndef  BRCM_RSDB
	swap_key_from_BE(&key);
	if ((err = wldev_ioctl(dev, WLC_GET_KEY, &key, sizeof(key), false))) {
		return err;
	}
#endif
	swap_key_to_BE(&key);
	memset(&params, 0, sizeof(params));
	params.key_len = (u8) min_t(u8, DOT11_MAX_KEY_SIZE, key.len);
#ifndef  BRCM_RSDB
	params.key = key.data;

	memset(&u, 0, sizeof(u));
	u.index = key.index;
	if ((err = wldev_ioctl(dev, WLC_GET_KEY_SEQ, &u, sizeof(u), false))) {
		return err;
	}
	params.seq = u.tsc;
	params.seq_len = DOT11_WPA_KEY_RSC_LEN;
#else
	memcpy((void *)params.key, key.data, params.key_len);
#endif
	err = wldev_iovar_getint_bsscfg(dev, "wsec", &wsec, bssidx);
	if (unlikely(err)) {
		WL_ERR(("WLC_GET_WSEC error (%d)\n", err));
		return err;
	}
#ifndef  BRCM_RSDB
	switch (wsec & ~SES_OW_ENABLED) {
#else
	switch (WSEC_ENABLED(wsec)) {
#endif
		case WEP_ENABLED:
			sec = wl_read_prof(cfg, dev, WL_PROF_SEC);
			ASSERT(sec);
			if (sec->cipher_pairwise & WLAN_CIPHER_SUITE_WEP40) {
				params.cipher = WLAN_CIPHER_SUITE_WEP40;
				WL_DBG(("WLAN_CIPHER_SUITE_WEP40\n"));
			} else if (sec->cipher_pairwise & WLAN_CIPHER_SUITE_WEP104) {
				params.cipher = WLAN_CIPHER_SUITE_WEP104;
				WL_DBG(("WLAN_CIPHER_SUITE_WEP104\n"));
			}
			break;
		case TKIP_ENABLED:
			params.cipher = WLAN_CIPHER_SUITE_TKIP;
			WL_DBG(("WLAN_CIPHER_SUITE_TKIP\n"));
			break;
		case AES_ENABLED:
			params.cipher = WLAN_CIPHER_SUITE_AES_CMAC;
			WL_DBG(("WLAN_CIPHER_SUITE_AES_CMAC\n"));
			break;
#ifdef BCMWAPI_WPI
#ifndef  BRCM_RSDB
		case WLAN_CIPHER_SUITE_SMS4:
			key.algo = CRYPTO_ALGO_SMS4;
#else
		case SMS4_ENABLED:
			params.cipher = WLAN_CIPHER_SUITE_SMS4;
#endif
			WL_DBG(("WLAN_CIPHER_SUITE_SMS4\n"));
			break;
#endif
#if defined(SUPPORT_SOFTAP_WPAWPA2_MIXED)
		/* to connect to mixed mode AP */
		case (AES_ENABLED | TKIP_ENABLED): /* TKIP CCMP */
			params.cipher = WLAN_CIPHER_SUITE_AES_CMAC;
			WL_DBG(("WLAN_CIPHER_SUITE_TKIP\n"));
			break;
#endif
		default:
			WL_ERR(("Invalid algo (0x%x)\n", wsec));
			return -EINVAL;
	}

	callback(cookie, &params);
	return err;
}

static s32
wl_cfg80211_config_default_mgmt_key(struct wiphy *wiphy,
	struct net_device *dev, u8 key_idx)
{
#ifndef  BRCM_RSDB
	WL_INFORM(("Not supported\n"));
	return -EOPNOTSUPP;
#else
#ifdef MFP
	return 0;
#else
	WL_INFORM(("Not supported\n"));
	return -EOPNOTSUPP;
#endif /* MFP */
#endif
}

#ifdef HW_SIGNAL_LOG_PRINT
static s32 hw_get_station_cnt = 0;
static s32 hw_rssi3 = 0;
static s32 hw_rssi2 = 0;
static s32 hw_rssi1 = 0;
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0))
static s32
wl_cfg80211_get_station(struct wiphy *wiphy, struct net_device *dev,
	const u8 *mac, struct station_info *sinfo)
#else
static s32
wl_cfg80211_get_station(struct wiphy *wiphy, struct net_device *dev,
	u8 *mac, struct station_info *sinfo)
#endif

{
	struct bcm_cfg80211 *cfg = wiphy_priv(wiphy);
	scb_val_t scb_val;
	s32 rssi;
	s32 rate = 0;
	s32 err = 0;
	sta_info_t *sta;
	dhd_pub_t *dhd = NULL;
#ifdef  BRCM_RSDB
	u32 dhd_assoc_state = 0;
	bool fw_assoc_state = FALSE;
#endif
#ifdef CONFIG_HW_GET_EXT_SIG
	rrmrep_t chloadrep;
	s32 noise;
	s32 snr;
	s32 chload;
	u32 ekvr_opclass;
	u32 ekvr_freq, band;
	int pm = -1;
#endif
#ifndef  BRCM_RSDB
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 0, 0))
	s8 eabuf[ETHER_ADDR_STR_LEN];
#endif
#else
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 0, 0)) || defined(WL_COMPAT_WIRELESS)
	s8 eabuf[ETHER_ADDR_STR_LEN];
#endif
#endif
	if (cfg == NULL) {
		WL_ERR(("%s : cfg is null\n", __FUNCTION__));
		return BCME_ERROR;
	}
	dhd =  (dhd_pub_t *)(cfg->pub);
	if (dhd == NULL) {
		WL_ERR(("%s : dhd is null\n", __FUNCTION__));
		return BCME_ERROR;
	}
	RETURN_EIO_IF_NOT_UP(cfg);
	WL_DBG(("wl_cfg80211_get_station enter\n")); //For Signal Poll block issue debug ccbc.
	if (wl_get_mode_by_netdev(cfg, dev) == WL_MODE_AP) {
		err = wldev_iovar_getbuf(dev, "sta_info", (struct ether_addr *)mac,
			ETHER_ADDR_LEN, cfg->ioctl_buf, WLC_IOCTL_SMLEN, &cfg->ioctl_buf_sync);
		if (err < 0) {
			WL_ERR(("GET STA INFO failed, %d\n", err));
			return err;
		}
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0)) && defined(HW_KERNEL_4_0_ADAPTATION)
		sinfo->filled = BIT(NL80211_STA_INFO_INACTIVE_TIME);
#else
#ifndef  BRCM_RSDB
		sinfo->filled = STATION_INFO_INACTIVE_TIME;
#else
		sinfo->filled = STA_INFO_BIT(INFO_INACTIVE_TIME);
#endif
#endif
		sta = (sta_info_t *)cfg->ioctl_buf;
		sta->len = dtoh16(sta->len);
		sta->cap = dtoh16(sta->cap);
		sta->flags = dtoh32(sta->flags);
		sta->idle = dtoh32(sta->idle);
		sta->in = dtoh32(sta->in);
		sinfo->inactive_time = sta->idle * 1000;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 0, 0)) || defined(WL_COMPAT_WIRELESS)
		if (sta->flags & WL_STA_ASSOC) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0)) && defined(HW_KERNEL_4_0_ADAPTATION)
			sinfo->filled |= BIT(NL80211_STA_INFO_CONNECTED_TIME);
#else
#ifndef  BRCM_RSDB
			sinfo->filled |= STATION_INFO_CONNECTED_TIME;
#else
			sinfo->filled |= STA_INFO_BIT(INFO_CONNECTED_TIME);
#endif  /* BRCM_RSDB */
#endif
			sinfo->connected_time = sta->in;
		}
		WL_INFORM(("STA %s : idle time : %d sec, connected time :%d ms\n",
			bcm_ether_ntoa((const struct ether_addr *)mac, eabuf), sinfo->inactive_time,
			sta->idle * 1000));
#endif
	} else if (wl_get_mode_by_netdev(cfg, dev) == WL_MODE_BSS ||
		wl_get_mode_by_netdev(cfg, dev) == WL_MODE_IBSS) {
		get_pktcnt_t pktcnt;
		u8 *curmacp;

		if (cfg->roam_offload) {
			struct ether_addr bssid;
			memset(&bssid, 0, sizeof(bssid));
			err = wldev_ioctl(dev, WLC_GET_BSSID, &bssid, ETHER_ADDR_LEN, false);
			if (err) {
				WL_ERR(("Failed to get current BSSID\n"));
			} else {
#ifndef  BRCM_RSDB
				if (memcmp(mac, &bssid.octet, ETHER_ADDR_LEN) != 0) {
#else
				if (!ETHER_ISNULLADDR(&bssid.octet) &&
						memcmp(mac, &bssid.octet, ETHER_ADDR_LEN) != 0) {
#endif
					/* roaming is detected */
					err = wl_cfg80211_delayed_roam(cfg, dev, &bssid);
					if (err)
						WL_ERR(("Failed to handle the delayed roam, "
							"err=%d", err));
					mac = (u8 *)bssid.octet;
				}
			}
		}
#ifndef  BRCM_RSDB
		if (!wl_get_drv_status(cfg, CONNECTED, dev) ||
			(dhd_is_associated(dhd, NULL, &err) == FALSE)) {
#else
		dhd_assoc_state = wl_get_drv_status(cfg, CONNECTED, dev);
		fw_assoc_state = dhd_is_associated(dhd, 0, &err);
		if (!dhd_assoc_state || !fw_assoc_state) {
#endif
			WL_ERR(("NOT assoc\n"));
			if (err == -ERESTARTSYS)
				return err;
#ifdef  BRCM_RSDB
			if (!dhd_assoc_state) {
				WL_TRACE_HW4(("drv state is not connected \n"));
			}
			if (!fw_assoc_state) {
				WL_TRACE_HW4(("fw state is not associated \n"));
			}
			/* Disconnect due to fw is not associated for FW_ASSOC_WATCHDOG_TIME ms.
			* 'err == 0' of dhd_is_associated() and '!fw_assoc_state'
			* means that BSSID is null.
			*/
			if (dhd_assoc_state && !fw_assoc_state && !err) {
				if (!fw_assoc_watchdog_started) {
					fw_assoc_watchdog_ms = OSL_SYSUPTIME();
					fw_assoc_watchdog_started = TRUE;
					WL_TRACE_HW4(("fw_assoc_watchdog_started \n"));
				} else {
					if (OSL_SYSUPTIME() - fw_assoc_watchdog_ms >
						FW_ASSOC_WATCHDOG_TIME) {
						fw_assoc_watchdog_started = FALSE;
						err = -ENODEV;
						WL_TRACE_HW4(("fw is not associated for %d ms \n",
							(OSL_SYSUPTIME() - fw_assoc_watchdog_ms)));
						goto get_station_err;
					}
				}
			}
#endif
			err = -ENODEV;
			return err;
		}
#ifdef  BRCM_RSDB
		fw_assoc_watchdog_started = FALSE;
#endif
		curmacp = wl_read_prof(cfg, dev, WL_PROF_BSSID);
		ASSERT(curmacp);
		if (memcmp(mac, curmacp, ETHER_ADDR_LEN)) {
			WL_ERR(("Wrong Mac address: "MACDBG" != "MACDBG"\n",
				MAC2STRDBG(mac), MAC2STRDBG(curmacp)));
		}

		if (dhd->early_suspended) {
			err = wldev_ioctl(dev, WLC_GET_PM, &pm, sizeof(pm), true);
			if (err) {
				WL_ERR(("Could not get PM (%d)\n", err));
			}
			WL_ERR(("now pm is %d\n",pm));
		}

		/* Report the current tx rate */
		err = wldev_ioctl(dev, WLC_GET_RATE, &rate, sizeof(rate), false);
		if (err) {
			WL_ERR(("Could not get rate (%d)\n", err));
		} else {
#if defined(USE_DYNAMIC_MAXPKT_RXGLOM)
			int rxpktglom;
#endif
			rate = dtoh32(rate);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0)) && defined(HW_KERNEL_4_0_ADAPTATION)
			sinfo->filled |= BIT(NL80211_STA_INFO_TX_BITRATE);
#else
#ifndef  BRCM_RSDB
			sinfo->filled |= STATION_INFO_TX_BITRATE;
#else
			sinfo->filled |= STA_INFO_BIT(INFO_TX_BITRATE);
#endif
#endif
			sinfo->txrate.legacy = rate * 5;
			WL_DBG(("Rate %d Mbps\n", (rate / 2)));
#if defined(USE_DYNAMIC_MAXPKT_RXGLOM)
			rxpktglom = ((rate/2) > 150) ? 20 : 10;

			if (maxrxpktglom != rxpktglom) {
				maxrxpktglom = rxpktglom;
				WL_DBG(("Rate %d Mbps, update bus:maxtxpktglom=%d\n", (rate/2),
					maxrxpktglom));
				err = wldev_iovar_setbuf(dev, "bus:maxtxpktglom",
					(char*)&maxrxpktglom, 4, cfg->ioctl_buf,
					WLC_IOCTL_MAXLEN, NULL);
				if (err < 0) {
					WL_ERR(("set bus:maxtxpktglom failed, %d\n", err));
				}
			}
#endif
		}

		memset(&scb_val, 0, sizeof(scb_val));
		scb_val.val = 0;
		err = wldev_ioctl(dev, WLC_GET_RSSI, &scb_val,
			sizeof(scb_val_t), false);
		if (err) {
			WL_ERR(("Could not get rssi (%d)\n", err));
			goto get_station_err;
		}
		rssi = wl_rssi_offset(dtoh32(scb_val.val));
		rssi = wl_rssi_average(rssi);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0)) && defined(HW_KERNEL_4_0_ADAPTATION)
		sinfo->filled |= BIT(NL80211_STA_INFO_SIGNAL);
#else
#ifndef  BRCM_RSDB
		sinfo->filled |= STATION_INFO_SIGNAL;
#else
		sinfo->filled |= STA_INFO_BIT(INFO_SIGNAL);
#endif
#endif
		sinfo->signal = rssi;
		WL_DBG(("RSSI %d dBm\n", rssi));
#ifdef CONFIG_HW_GET_EXT_SIG
		/*
		 * get noise
		 * normal range is [-92, -70]
		 */
		noise = 0;
		err = wldev_ioctl(dev, WLC_GET_PHY_NOISE, &noise,
			sizeof(int), false);
		if (err) {
			WL_ERR(("Could not get noise (%d)\n", err));
		} else {
			sinfo->filled |= BIT(NL80211_STA_INFO_NOISE);
			sinfo->noise = noise;
		}

		/*
		 * get snr
		 * normal range is [0, 35]
		 */
		snr = -1;
		err = wldev_iovar_getbuf(dev, "snr", NULL, 0, &snr, sizeof(s32), NULL);
		if (err) {
			WL_ERR(("Could not get snr (%d)\n", err));
		} else {
			sinfo->filled |= BIT(NL80211_STA_INFO_SNR);
			sinfo->snr = snr;
		}

		/*
		 * get chload
		 * normal range is [0, 255]
		 */
		chload = -1;
		if (dhd) {
			chload = dhd->chload;
		}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0))
		band = (cfg->channel <= CH_MAX_2G_CHANNEL) ? NL80211_BAND_2GHZ : NL80211_BAND_5GHZ;
#else
		band = (cfg->channel <= CH_MAX_2G_CHANNEL) ? IEEE80211_BAND_2GHZ : IEEE80211_BAND_5GHZ;
#endif
		ekvr_freq = ieee80211_channel_to_frequency(cfg->channel, band);

		/* The number below is about the frequency of Wi-Fi */
		if (ekvr_freq >= 2412 && ekvr_freq <= 2472)
			ekvr_opclass = 81;

		if (ekvr_freq == 2484)
			ekvr_opclass = 82;

		if (ekvr_freq >= 5180 && ekvr_freq <= 5240)
			ekvr_opclass = 115;

		if (ekvr_freq >= 5745 && ekvr_freq <= 5845)
			ekvr_opclass = 125;

		if (ekvr_freq >= 5000 && ekvr_freq <= 5700)
			ekvr_opclass = 121;

		chloadrep.reg           = ekvr_opclass;
		chloadrep.chan          = cfg->channel;
		chloadrep.random_int    = 0;
		chloadrep.dur           = 100;
#ifndef HW_RRM_DISABLE
		WL_ERR(("eKVR_get_bssinfo_type: channel=%d, opclass=%d" , cfg->channel , ekvr_opclass));
		err = wldev_iovar_setbuf(dev, "rrm_chload_req", &chloadrep, sizeof(chloadrep),
			cfg->ioctl_buf, WLC_IOCTL_MAXLEN, NULL);
		if (unlikely(err)) {
			WL_ERR(("set rrm_chload_req error (%d)\n", err));
		} else {
			sinfo->filled |= BIT(NL80211_STA_INFO_CNAHLOAD);
			sinfo->chload = chload;
		}
#endif  /* HW_RRM_DISABLE */
#endif


		memset(&pktcnt, 0, sizeof(pktcnt));
		err = wldev_ioctl(dev, WLC_GET_PKTCNTS, &pktcnt,
			sizeof(pktcnt), false);
		if (!err) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0)) && defined(HW_KERNEL_4_0_ADAPTATION)
			sinfo->filled |= (BIT(NL80211_STA_INFO_RX_PACKETS) |
				BIT(NL80211_STA_INFO_RX_DROP_MISC) |
				BIT(NL80211_STA_INFO_TX_PACKETS) |
				BIT(NL80211_STA_INFO_TX_FAILED));
#else
#ifndef  BRCM_RSDB
			sinfo->filled |= (STATION_INFO_RX_PACKETS |
				STATION_INFO_RX_DROP_MISC |
				STATION_INFO_TX_PACKETS |
				STATION_INFO_TX_FAILED);
#else
			sinfo->filled |= (STA_INFO_BIT(INFO_RX_PACKETS) |
				STA_INFO_BIT(INFO_RX_DROP_MISC) |
				STA_INFO_BIT(INFO_TX_PACKETS) |
				STA_INFO_BIT(INFO_TX_FAILED));
#endif
#endif
			sinfo->rx_packets = pktcnt.rx_good_pkt;
			sinfo->rx_dropped_misc = pktcnt.rx_bad_pkt;
			sinfo->tx_packets = pktcnt.tx_good_pkt;
			sinfo->tx_failed  = pktcnt.tx_bad_pkt;
		}
#ifdef HW_SIGNAL_LOG_PRINT
		hw_get_station_cnt++;
		if ((hw_get_station_cnt & 3) == 0) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0)) && defined(HW_KERNEL_4_0_ADAPTATION)
			if ((sinfo->filled & (BIT(NL80211_STA_INFO_TX_BITRATE) | BIT(NL80211_STA_INFO_RX_PACKETS)))
					== (BIT(NL80211_STA_INFO_TX_BITRATE) | BIT(NL80211_STA_INFO_RX_PACKETS))) {
#else
			if ((sinfo->filled & (STATION_INFO_TX_BITRATE | STATION_INFO_RX_PACKETS))
					== (STATION_INFO_TX_BITRATE | STATION_INFO_RX_PACKETS)) {
#endif
				WL_ERR(("rssi:(%d,%d,%d,%d), rate:%d (%d,%d,%d,%d)\n",
						hw_rssi3, hw_rssi2, hw_rssi1, rssi, rate/2, sinfo->rx_packets,
						sinfo->rx_dropped_misc, sinfo->tx_packets, sinfo->tx_failed));
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0)) && defined(HW_KERNEL_4_0_ADAPTATION)
			} else if ((sinfo->filled & BIT(NL80211_STA_INFO_TX_BITRATE)) == BIT(NL80211_STA_INFO_TX_BITRATE)) {
#else
			} else if ((sinfo->filled & STATION_INFO_TX_BITRATE) == STATION_INFO_TX_BITRATE) {
#endif
				WL_ERR(("rssi:(%d,%d,%d,%d), rate:%d\n", hw_rssi3, hw_rssi2, hw_rssi1, rssi, rate/2));
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0)) && defined(HW_KERNEL_4_0_ADAPTATION)
			} else if ((sinfo->filled & BIT(NL80211_STA_INFO_RX_PACKETS)) == BIT(NL80211_STA_INFO_RX_PACKETS)) {
#else
			} else if ((sinfo->filled & STATION_INFO_RX_PACKETS) == STATION_INFO_RX_PACKETS) {
#endif
				WL_ERR(("rssi:(%d,%d,%d,%d) (%d,%d,%d,%d)\n", hw_rssi3, hw_rssi2, hw_rssi1, rssi,
						sinfo->rx_packets,sinfo->rx_dropped_misc,sinfo->tx_packets,sinfo->tx_failed));
			} else {
				WL_ERR(("rssi:(%d,%d,%d,%d)\n", hw_rssi3, hw_rssi2, hw_rssi1, rssi));
			}
		}
		hw_rssi3 = hw_rssi2;
		hw_rssi2 = hw_rssi1;
		hw_rssi1 = rssi;
#endif /* HW_SIGNAL_LOG_PRINT */
get_station_err:
		if (err && (err != -ERESTARTSYS)) {
			/* Disconnect due to zero BSSID or error to get RSSI */
			WL_ERR(("force cfg80211_disconnected: %d\n", err));
			wl_clr_drv_status(cfg, CONNECTED, dev);
			CFG80211_DISCONNECTED(dev, 0, NULL, 0, false, GFP_KERNEL);
			wl_link_down(cfg);
		}
	}
	else {
		WL_ERR(("Invalid device mode %d\n", wl_get_mode_by_netdev(cfg, dev)));
	}

	return err;
}

#ifdef CONFIG_HW_GET_P2P_TX_RATE
static s32
wl_cfg80211_get_p2p_tx_rate(struct wiphy *wiphy, struct net_device *dev,
	struct station_info *sinfo)
{
	struct bcm_cfg80211 *cfg = wiphy_priv(wiphy);
	s32 rate = 0;
	s32 err = 0;

	RETURN_EIO_IF_NOT_UP(cfg);
	WL_DBG(("wl_cfg80211_get_p2p_tx_rate\n")); //For Signal Poll block issue debug ccbc.

	/* Report the current tx rate */
	err = wldev_ioctl(dev, WLC_GET_RATE, &rate, sizeof(rate), false);
	if (err) {
		WL_ERR(("Could not get rate (%d)\n", err));
	} else {
#if defined(USE_DYNAMIC_MAXPKT_RXGLOM)
		int rxpktglom;
#endif
		rate = dtoh32(rate);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0)) && defined(HW_KERNEL_4_0_ADAPTATION)
		sinfo->filled |= BIT(NL80211_STA_INFO_TX_BITRATE);
#else
#ifndef  BRCM_RSDB
		sinfo->filled |= STATION_INFO_TX_BITRATE;
#else
		sinfo->filled |= STA_INFO_BIT(INFO_TX_BITRATE);
#endif
#endif
		sinfo->txrate.legacy = rate * 5;
		WL_DBG(("Rate %d Mbps\n", (rate / 2)));
#if defined(USE_DYNAMIC_MAXPKT_RXGLOM)
		rxpktglom = ((rate/2) > 150) ? 20 : 10;

		if (maxrxpktglom != rxpktglom) {
			maxrxpktglom = rxpktglom;
			WL_DBG(("Rate %d Mbps, update bus:maxtxpktglom=%d\n", (rate/2),
				maxrxpktglom));
			err = wldev_iovar_setbuf(dev, "bus:maxtxpktglom",
				(char*)&maxrxpktglom, 4, cfg->ioctl_buf,
				WLC_IOCTL_MAXLEN, NULL);
			if (err < 0) {
				WL_ERR(("set bus:maxtxpktglom failed, %d\n", err));
			}
		}
#endif
	}

	return err;
}
#endif /* CONFIG_HW_GET_P2P_TX_RATE */

static s32
wl_cfg80211_set_power_mgmt(struct wiphy *wiphy, struct net_device *dev,
	bool enabled, s32 timeout)
{
	s32 pm;
	s32 err = 0;
	struct bcm_cfg80211 *cfg = wiphy_priv(wiphy);
	struct net_info *_net_info = wl_get_netinfo_by_netdev(cfg, dev);

	RETURN_EIO_IF_NOT_UP(cfg);
	WL_DBG(("Enter\n"));
#ifndef  BRCM_RSDB
	if (cfg->p2p_net == dev || _net_info == NULL || cfg->vsdb_mode ||
		!wl_get_drv_status(cfg, CONNECTED, dev)) {
		return err;
	}

	/* Delete pm_enable_work */
	wl_add_remove_pm_enable_work(cfg, FALSE, WL_HANDLER_PEND);
#else
	if (cfg->p2p_net == dev || _net_info == NULL ||
		!wl_get_drv_status(cfg, CONNECTED, dev) ||
		(wl_get_mode_by_netdev(cfg, dev) != WL_MODE_BSS &&
		wl_get_mode_by_netdev(cfg, dev) != WL_MODE_IBSS)) {
		return err;
	}
#ifndef HW_PM_LOCK_BUGFIX
	/* Enlarge pm_enable_work */
	wl_add_remove_pm_enable_work(cfg, WL_PM_WORKQ_LONG);
#else
	if (enabled) {
		/* Delete pm_enable_work */
		wl_add_remove_pm_enable_work(cfg, WL_PM_WORKQ_DEL);
	} else {
		/* Enlarge pm_enable_work */
		wl_add_remove_pm_enable_work(cfg, WL_PM_WORKQ_LONG);
	}
#endif  //endif HW_PM_LOCK_BUGFIX
#endif

	pm = enabled ? PM_FAST : PM_OFF;
	if (_net_info->pm_block) {
		WL_ERR(("%s:Do not enable the power save for pm_block %d\n",
			dev->name, _net_info->pm_block));
		pm = PM_OFF;
	}
	pm = htod32(pm);
	WL_DBG(("%s:power save %s\n", dev->name, (pm ? "enabled" : "disabled")));
	err = wldev_ioctl(dev, WLC_SET_PM, &pm, sizeof(pm), true);
	if (unlikely(err)) {
		if (err == -ENODEV)
			WL_DBG(("net_device is not ready yet\n"));
		else
			WL_ERR(("error (%d)\n", err));
		return err;
	}
	wl_cfg80211_update_power_mode(dev);
	return err;
}

void wl_cfg80211_update_power_mode(struct net_device *dev)
{
	int err, pm = -1;

	err = wldev_ioctl(dev, WLC_GET_PM, &pm, sizeof(pm), true);
	if (err)
		WL_ERR(("%s:error (%d)\n", __FUNCTION__, err));
	else if (pm != -1 && dev->ieee80211_ptr)
		dev->ieee80211_ptr->ps = (pm == PM_OFF) ? false : true;
}
#ifdef  BRCM_RSDB
void wl_cfg80211_set_passive_scan(struct net_device *dev, char *command)
{
	struct bcm_cfg80211 *cfg = g_bcm_cfg;

	if (strcmp(command, "SCAN-ACTIVE") == 0) {
		cfg->active_scan = 1;
	} else if (strcmp(command, "SCAN-PASSIVE") == 0) {
		cfg->active_scan = 0;
	} else
		WL_ERR(("Unknown command \n"));
}
#endif
static __used u32 wl_find_msb(u16 bit16)
{
	u32 ret = 0;

	if (bit16 & 0xff00) {
		ret += 8;
		bit16 >>= 8;
	}

	if (bit16 & 0xf0) {
		ret += 4;
		bit16 >>= 4;
	}

	if (bit16 & 0xc) {
		ret += 2;
		bit16 >>= 2;
	}

	if (bit16 & 2)
		ret += bit16 & 2;
	else if (bit16)
		ret += bit16;

	return ret;
}

static s32 wl_cfg80211_resume(struct wiphy *wiphy)
{
	struct bcm_cfg80211 *cfg = wiphy_priv(wiphy);
	struct net_device *ndev = bcmcfg_to_prmry_ndev(cfg);
#ifndef  BRCM_RSDB
	s32 err = 0;
#else
	s32 err = BCME_OK;
#endif

	HW_PRINT((WIFI_TAG"%s +\n", __FUNCTION__));
#ifdef HW_WIFI_SUSPEND_ISSUE
	hw_cfg80211_suspend = FALSE;
	HW_PRINT((WIFI_TAG"hw_cfg80211_suspend: %d\n", hw_cfg80211_suspend));
#endif
	if (unlikely(!wl_get_drv_status(cfg, READY, ndev))) {
		WL_INFORM(("device is not ready\n"));
		HW_PRINT((WIFI_TAG"%s -\n", __FUNCTION__));
#ifndef  BRCM_RSDB	
		return 0;
#else
		return err;
#endif
	}

#ifdef HW_SDIO_DPC_SUSPEND
	hw_resched_dpc_ifneed(ndev);
#endif

	HW_PRINT((WIFI_TAG"%s -\n", __FUNCTION__));
	return err;
}


#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 39)) || defined(WL_COMPAT_WIRELESS)
static s32 wl_cfg80211_suspend(struct wiphy *wiphy, struct cfg80211_wowlan *wow)
#else
static s32 wl_cfg80211_suspend(struct wiphy *wiphy)
#endif
{
	s32 err = BCME_OK;
	HW_PRINT((WIFI_TAG"%s +\n", __FUNCTION__));
#ifdef DHD_CLEAR_ON_SUSPEND
	struct bcm_cfg80211 *cfg = wiphy_priv(wiphy);
	struct net_info *iter, *next;
	struct net_device *ndev = bcmcfg_to_prmry_ndev(cfg);
	unsigned long flags;
	if (unlikely(!wl_get_drv_status(cfg, READY, ndev))) {
		WL_INFORM(("device is not ready : status (%d)\n",
			(int)cfg->status));
		HW_PRINT((WIFI_TAG"%s -\n", __FUNCTION__));
		return err;
	}
#ifndef  BRCM_RSDB
	for_each_ndev(cfg, iter, next)
		wl_set_drv_status(cfg, SCAN_ABORTING, iter->ndev);
#else
	for_each_ndev(cfg, iter, next) {
		/* p2p discovery iface doesn't have a ndev associated with it (for kernel > 3.8) */
		if (iter->ndev)
			wl_set_drv_status(cfg, SCAN_ABORTING, iter->ndev);
		}
#endif
	spin_lock_irqsave(&cfg->cfgdrv_lock, flags);
	if (cfg->scan_request) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 8, 0))
		struct cfg80211_scan_info info = {
			.aborted = true,
		};
		cfg80211_scan_done(cfg->scan_request, &info);
#else
		cfg80211_scan_done(cfg->scan_request, true);
#endif
		cfg->scan_request = NULL;
	}
	for_each_ndev(cfg, iter, next) {
#ifndef  BRCM_RSDB
		wl_clr_drv_status(cfg, SCANNING, iter->ndev);
		wl_clr_drv_status(cfg, SCAN_ABORTING, iter->ndev);
#else
		if (iter->ndev) {
			wl_clr_drv_status(cfg, SCANNING, iter->ndev);
			wl_clr_drv_status(cfg, SCAN_ABORTING, iter->ndev);
		}
#endif
	}
	spin_unlock_irqrestore(&cfg->cfgdrv_lock, flags);
	for_each_ndev(cfg, iter, next) {
		if (wl_get_drv_status(cfg, CONNECTING, iter->ndev)) {
			wl_bss_connect_done(cfg, iter->ndev, NULL, NULL, false);
		}
	}
#endif /* DHD_CLEAR_ON_SUSPEND */
#ifdef HW_WIFI_SUSPEND_ISSUE
	hw_cfg80211_suspend = TRUE;
	HW_PRINT((WIFI_TAG"hw_cfg80211_suspend: %d\n", hw_cfg80211_suspend));
#endif
	HW_PRINT((WIFI_TAG"%s -\n", __FUNCTION__));
	return err;
}

static s32
wl_update_pmklist(struct net_device *dev, struct wl_pmk_list *pmk_list,
	s32 err)
{
	int i, j;
	struct bcm_cfg80211 *cfg = g_bcm_cfg;
	struct net_device *primary_dev = bcmcfg_to_prmry_ndev(cfg);

	if (!pmk_list) {
#ifndef  BRCM_RSDB
		printf("pmk_list is NULL\n");
#else
		WL_INFORM(("pmk_list is NULL\n"));
#endif
		return -EINVAL;
	}
	/* pmk list is supported only for STA interface i.e. primary interface
	 * Refer code wlc_bsscfg.c->wlc_bsscfg_sta_init
	 */
	if (primary_dev != dev) {
		WL_INFORM(("Not supporting Flushing pmklist on virtual"
			" interfaces than primary interface\n"));
		return err;
	}

	WL_DBG(("No of elements %d\n", pmk_list->pmkids.npmkid));
	for (i = 0; i < pmk_list->pmkids.npmkid; i++) {
		WL_DBG(("PMKID[%d]: %pM =\n", i,
			&pmk_list->pmkids.pmkid[i].BSSID));
		for (j = 0; j < WPA2_PMKID_LEN; j++) {
			WL_DBG(("%02x\n", pmk_list->pmkids.pmkid[i].PMKID[j]));
		}
	}
	if (likely(!err)) {
		err = wldev_iovar_setbuf(dev, "pmkid_info", (char *)pmk_list,
			sizeof(*pmk_list), cfg->ioctl_buf, WLC_IOCTL_MAXLEN, &cfg->ioctl_buf_sync);
	}

	return err;
}

static s32
wl_cfg80211_set_pmksa(struct wiphy *wiphy, struct net_device *dev,
	struct cfg80211_pmksa *pmksa)
{
	struct bcm_cfg80211 *cfg = wiphy_priv(wiphy);
	s32 err = 0;
	int i;

	RETURN_EIO_IF_NOT_UP(cfg);
	for (i = 0; i < cfg->pmk_list->pmkids.npmkid; i++)
		if (!memcmp(pmksa->bssid, &cfg->pmk_list->pmkids.pmkid[i].BSSID,
			ETHER_ADDR_LEN))
			break;
	if (i < WL_NUM_PMKIDS_MAX) {
		memcpy(&cfg->pmk_list->pmkids.pmkid[i].BSSID, pmksa->bssid,
			ETHER_ADDR_LEN);
		memcpy(&cfg->pmk_list->pmkids.pmkid[i].PMKID, pmksa->pmkid,
			WPA2_PMKID_LEN);
		if (i == cfg->pmk_list->pmkids.npmkid)
			cfg->pmk_list->pmkids.npmkid++;
	} else {
		err = -EINVAL;
	}
	WL_DBG(("set_pmksa,IW_PMKSA_ADD - PMKID: %pM =\n",
		&cfg->pmk_list->pmkids.pmkid[cfg->pmk_list->pmkids.npmkid - 1].BSSID));
	for (i = 0; i < WPA2_PMKID_LEN; i++) {
		WL_DBG(("%02x\n",
			cfg->pmk_list->pmkids.pmkid[cfg->pmk_list->pmkids.npmkid - 1].
			PMKID[i]));
	}

	err = wl_update_pmklist(dev, cfg->pmk_list, err);

	return err;
}

static s32
wl_cfg80211_del_pmksa(struct wiphy *wiphy, struct net_device *dev,
	struct cfg80211_pmksa *pmksa)
{
	struct bcm_cfg80211 *cfg = wiphy_priv(wiphy);
#ifndef  BRCM_RSDB
	struct _pmkid_list pmkid = {0};
#else
	struct _pmkid_list pmkid = {.npmkid = 0};
#endif
	s32 err = 0;
	int i;

	RETURN_EIO_IF_NOT_UP(cfg);
	memcpy(&pmkid.pmkid[0].BSSID, pmksa->bssid, ETHER_ADDR_LEN);
	memcpy(pmkid.pmkid[0].PMKID, pmksa->pmkid, WPA2_PMKID_LEN);

	WL_DBG(("del_pmksa,IW_PMKSA_REMOVE - PMKID: %pM =\n",
		&pmkid.pmkid[0].BSSID));
	for (i = 0; i < WPA2_PMKID_LEN; i++) {
		WL_DBG(("%02x\n", pmkid.pmkid[0].PMKID[i]));
	}

	for (i = 0; i < cfg->pmk_list->pmkids.npmkid; i++)
		if (!memcmp
		    (pmksa->bssid, &cfg->pmk_list->pmkids.pmkid[i].BSSID,
		     ETHER_ADDR_LEN))
			break;

	if ((cfg->pmk_list->pmkids.npmkid > 0) &&
		(i < cfg->pmk_list->pmkids.npmkid)) {
		memset(&cfg->pmk_list->pmkids.pmkid[i], 0, sizeof(pmkid_t));
		for (; i < (cfg->pmk_list->pmkids.npmkid - 1); i++) {
			memcpy(&cfg->pmk_list->pmkids.pmkid[i].BSSID,
				&cfg->pmk_list->pmkids.pmkid[i + 1].BSSID,
				ETHER_ADDR_LEN);
			memcpy(&cfg->pmk_list->pmkids.pmkid[i].PMKID,
				&cfg->pmk_list->pmkids.pmkid[i + 1].PMKID,
				WPA2_PMKID_LEN);
		}
		cfg->pmk_list->pmkids.npmkid--;
	} else {
		err = -EINVAL;
	}

	err = wl_update_pmklist(dev, cfg->pmk_list, err);

	return err;

}

static s32
wl_cfg80211_flush_pmksa(struct wiphy *wiphy, struct net_device *dev)
{
	struct bcm_cfg80211 *cfg = wiphy_priv(wiphy);
	s32 err = 0;
	RETURN_EIO_IF_NOT_UP(cfg);
	memset(cfg->pmk_list, 0, sizeof(*cfg->pmk_list));
	err = wl_update_pmklist(dev, cfg->pmk_list, err);
	return err;

}

static wl_scan_params_t *
wl_cfg80211_scan_alloc_params(int channel, int nprobes, int *out_params_size)
{
	wl_scan_params_t *params;
	int params_size;
	int num_chans;
#ifdef  BRCM_RSDB
	int bssidx = 0;
#endif
	*out_params_size = 0;

	/* Our scan params only need space for 1 channel and 0 ssids */
	params_size = WL_SCAN_PARAMS_FIXED_SIZE + 1 * sizeof(uint16);
	params = (wl_scan_params_t*) kzalloc(params_size, GFP_KERNEL);
	if (params == NULL) {
		WL_ERR(("mem alloc failed (%d bytes)\n", params_size));
		return params;
	}
	memset(params, 0, params_size);
	params->nprobes = nprobes;

	num_chans = (channel == 0) ? 0 : 1;

	memcpy(&params->bssid, &ether_bcast, ETHER_ADDR_LEN);
	params->bss_type = DOT11_BSSTYPE_ANY;
	params->scan_type = DOT11_SCANTYPE_ACTIVE;
	params->nprobes = htod32(1);
	params->active_time = htod32(-1);
	params->passive_time = htod32(-1);
	params->home_time = htod32(10);
	if (channel == -1)
		params->channel_list[0] = htodchanspec(channel);
	else
#ifndef  BRCM_RSDB
		params->channel_list[0] = wl_ch_host_to_driver(channel);
#else
		params->channel_list[0] = wl_ch_host_to_driver(bssidx, channel);
#endif

	/* Our scan params have 1 channel and 0 ssids */
	params->channel_num = htod32((0 << WL_SCAN_PARAMS_NSSID_SHIFT) |
		(num_chans & WL_SCAN_PARAMS_COUNT_MASK));

	*out_params_size = params_size;	/* rtn size to the caller */
	return params;
}

#if defined(WL_CFG80211_P2P_DEV_IF)
static s32
wl_cfg80211_remain_on_channel(struct wiphy *wiphy, bcm_struct_cfgdev *cfgdev,
	struct ieee80211_channel *channel, unsigned int duration, u64 *cookie)
#else
static s32
wl_cfg80211_remain_on_channel(struct wiphy *wiphy, bcm_struct_cfgdev *cfgdev,
	struct ieee80211_channel * channel,
	enum nl80211_channel_type channel_type,
	unsigned int duration, u64 *cookie)
#endif /* WL_CFG80211_P2P_DEV_IF */
{
	s32 target_channel;
#ifndef  BRCM_RSDB
	u32 id;
#endif
	s32 err = BCME_OK;
	struct ether_addr primary_mac;
	struct net_device *ndev = NULL;
	struct bcm_cfg80211 *cfg = wiphy_priv(wiphy);

	ndev = cfgdev_to_wlc_ndev(cfgdev, cfg);

	WL_DBG(("Enter, channel: %d, duration ms (%d) SCANNING ?? %s \n",
		ieee80211_frequency_to_channel(channel->center_freq),
		duration, (wl_get_drv_status(cfg, SCANNING, ndev)) ? "YES":"NO"));

	if (!cfg->p2p) {
		WL_ERR(("cfg->p2p is not initialized\n"));
		err = BCME_ERROR;
		goto exit;
	}
#ifdef  BRCM_RSDB
#ifdef P2P_LISTEN_OFFLOADING
	if (wl_get_p2p_status(cfg, DISC_IN_PROGRESS)) {
		WL_ERR(("P2P_FIND: Discovery offload is in progress\n"));
		return -EAGAIN;
	}
#endif /* P2P_LISTEN_OFFLOADING */
#endif
#ifndef WL_CFG80211_VSDB_PRIORITIZE_SCAN_REQUEST
	if (wl_get_drv_status_all(cfg, SCANNING)) {
		wl_notify_escan_complete(cfg, cfg->escan_info.ndev, true, true);
	}
#endif /* not WL_CFG80211_VSDB_PRIORITIZE_SCAN_REQUEST */

	target_channel = ieee80211_frequency_to_channel(channel->center_freq);
	memcpy(&cfg->remain_on_chan, channel, sizeof(struct ieee80211_channel));
#if defined(WL_ENABLE_P2P_IF)
	cfg->remain_on_chan_type = channel_type;
#endif /* WL_ENABLE_P2P_IF */
#ifndef  BRCM_RSDB
	id = ++cfg->last_roc_id;
	if (id == 0)
		id = ++cfg->last_roc_id;
	*cookie = id;
#else
	*cookie = wl_cfg80211_get_new_roc_id(cfg);
#endif
#ifdef WL_CFG80211_VSDB_PRIORITIZE_SCAN_REQUEST
	if (wl_get_drv_status(cfg, SCANNING, ndev)) {
		struct timer_list *_timer;
		WL_DBG(("scan is running. go to fake listen state\n"));
#ifdef  BRCM_RSDB
		if (duration > LONG_LISTEN_TIME) {
			wl_cfg80211_scan_abort(cfg);
		} else {
#endif
		wl_set_drv_status(cfg, FAKE_REMAINING_ON_CHANNEL, ndev);

		if (timer_pending(&cfg->p2p->listen_timer)) {
			WL_DBG(("cancel current listen timer \n"));
			del_timer_sync(&cfg->p2p->listen_timer);
		}

		_timer = &cfg->p2p->listen_timer;
		wl_clr_p2p_status(cfg, LISTEN_EXPIRED);

		INIT_TIMER(_timer, wl_cfgp2p_listen_expired, duration, 0);

		err = BCME_OK;
		goto exit;
#ifdef  BRCM_RSDB
		}
#endif
	}
#endif /* WL_CFG80211_VSDB_PRIORITIZE_SCAN_REQUEST */

#ifdef WL_CFG80211_SYNC_GON
	if (wl_get_drv_status_all(cfg, WAITING_NEXT_ACT_FRM_LISTEN)) {
		/* do not enter listen mode again if we are in listen mode already for next af.
		 * remain on channel completion will be returned by waiting next af completion.
		 */
#ifdef WL_CFG80211_VSDB_PRIORITIZE_SCAN_REQUEST
		wl_set_drv_status(cfg, FAKE_REMAINING_ON_CHANNEL, ndev);
#else
		wl_set_drv_status(cfg, REMAINING_ON_CHANNEL, ndev);
#endif /* WL_CFG80211_VSDB_PRIORITIZE_SCAN_REQUEST */
		goto exit;
	}
#endif /* WL_CFG80211_SYNC_GON */
	if (cfg->p2p && !cfg->p2p->on) {
		/* In case of p2p_listen command, supplicant send remain_on_channel
		 * without turning on P2P
		 */
		get_primary_mac(cfg, &primary_mac);
#ifndef  BRCM_RSDB
		wl_cfgp2p_generate_bss_mac(&primary_mac, &cfg->p2p->dev_addr, &cfg->p2p->int_addr);
#else
		wl_cfgp2p_generate_bss_mac(cfg, &primary_mac);
#endif
		p2p_on(cfg) = true;
	}

	if (p2p_is_on(cfg)) {
		err = wl_cfgp2p_enable_discovery(cfg, ndev, NULL, 0);
		if (unlikely(err)) {
			goto exit;
		}
#ifndef WL_CFG80211_VSDB_PRIORITIZE_SCAN_REQUEST
		wl_set_drv_status(cfg, REMAINING_ON_CHANNEL, ndev);
#endif /* not WL_CFG80211_VSDB_PRIORITIZE_SCAN_REQUEST */
		err = wl_cfgp2p_discover_listen(cfg, target_channel, duration);

#ifdef WL_CFG80211_VSDB_PRIORITIZE_SCAN_REQUEST
		if (err == BCME_OK) {
			wl_set_drv_status(cfg, REMAINING_ON_CHANNEL, ndev);
		} else {
			/* if failed, firmware may be internal scanning state.
			 * so other scan request shall not abort it
			 */
			wl_set_drv_status(cfg, FAKE_REMAINING_ON_CHANNEL, ndev);
		}
#endif /* WL_CFG80211_VSDB_PRIORITIZE_SCAN_REQUEST */
		/* WAR: set err = ok to prevent cookie mismatch in wpa_supplicant
		 * and expire timer will send a completion to the upper layer
		 */
		err = BCME_OK;
	}

exit:
	if (err == BCME_OK) {
		WL_INFORM(("Success\n"));
#if defined(WL_CFG80211_P2P_DEV_IF)
		cfg80211_ready_on_channel(cfgdev, *cookie, channel,
			duration, GFP_KERNEL);
#else
		cfg80211_ready_on_channel(cfgdev, *cookie, channel,
			channel_type, duration, GFP_KERNEL);
#endif /* WL_CFG80211_P2P_DEV_IF */
	} else {
		WL_ERR(("Fail to Set (err=%d cookie:%llu)\n", err, *cookie));
	}
	return err;
}

static s32
wl_cfg80211_cancel_remain_on_channel(struct wiphy *wiphy,
	bcm_struct_cfgdev *cfgdev, u64 cookie)
{
	s32 err = 0;
#ifdef  BRCM_RSDB
	struct bcm_cfg80211 *cfg = g_bcm_cfg;

#ifdef P2PLISTEN_AP_SAMECHN
	struct net_device *dev;
#endif /* P2PLISTEN_AP_SAMECHN */

	RETURN_EIO_IF_NOT_UP(cfg);
#endif
#if defined(WL_CFG80211_P2P_DEV_IF)
	if (cfgdev->iftype == NL80211_IFTYPE_P2P_DEVICE) {
		WL_DBG((" enter ) on P2P dedicated discover interface\n"));
	}
#else
	WL_DBG((" enter ) netdev_ifidx: %d \n", cfgdev->ifindex));
#endif /* WL_CFG80211_P2P_DEV_IF */
#ifdef  BRCM_RSDB
#ifdef P2PLISTEN_AP_SAMECHN
	if (cfg && cfg->p2p_resp_apchn_status) {
		dev = bcmcfg_to_prmry_ndev(cfg);
		wl_cfg80211_set_p2p_resp_ap_chn(dev, 0);
		cfg->p2p_resp_apchn_status = false;
		WL_DBG(("p2p_resp_apchn_status Turn OFF \n"));
	}
#endif /* P2PLISTEN_AP_SAMECHN */

	if (cfg->last_roc_id == cookie) {
		wl_cfgp2p_set_p2p_mode(cfg, WL_P2P_DISC_ST_SCAN, 0, 0,
			wl_to_p2p_bss_bssidx(cfg, P2PAPI_BSSCFG_DEVICE));
	} else {
		WL_ERR(("%s : ignore, request cookie(%llu) is not matched. (cur : %llu)\n",
			__FUNCTION__, cookie, cfg->last_roc_id));
	}
#endif
	return err;
}

static void
wl_cfg80211_afx_handler(struct work_struct *work)
{
	struct afx_hdl *afx_instance;
	struct bcm_cfg80211 *cfg = g_bcm_cfg;
	s32 ret = BCME_OK;
#ifndef  BRCM_RSDB
	afx_instance = container_of(work, struct afx_hdl, work);
#else
	BCM_SET_CONTAINER_OF(afx_instance, work, struct afx_hdl, work);
#endif
	if (afx_instance != NULL && cfg->afx_hdl->is_active) {
		if (cfg->afx_hdl->is_listen && cfg->afx_hdl->my_listen_chan) {
			ret = wl_cfgp2p_discover_listen(cfg, cfg->afx_hdl->my_listen_chan,
				(100 * (1 + (RANDOM32() % 3)))); /* 100ms ~ 300ms */
		} else {
			ret = wl_cfgp2p_act_frm_search(cfg, cfg->afx_hdl->dev,
				cfg->afx_hdl->bssidx, cfg->afx_hdl->peer_listen_chan,
				NULL);
		}
		if (unlikely(ret != BCME_OK)) {
			WL_ERR(("ERROR occurred! returned value is (%d)\n", ret));
			if (wl_get_drv_status_all(cfg, FINDING_COMMON_CHANNEL))
				complete(&cfg->act_frm_scan);
		}
	}
}

static s32
wl_cfg80211_af_searching_channel(struct bcm_cfg80211 *cfg, struct net_device *dev)
{
	u32 max_retry = WL_CHANNEL_SYNC_RETRY;
#ifdef  BRCM_RSDB
	bool is_p2p_gas = false;
#endif
	if (dev == NULL)
		return -1;

	WL_DBG((" enter ) \n"));

	wl_set_drv_status(cfg, FINDING_COMMON_CHANNEL, dev);
	cfg->afx_hdl->is_active = TRUE;
#ifdef  BRCM_RSDB
	if (cfg->afx_hdl->pending_tx_act_frm) {
		wl_action_frame_t *action_frame;
		action_frame = &(cfg->afx_hdl->pending_tx_act_frm->action_frame);
		if (wl_cfgp2p_is_p2p_gas_action(action_frame->data, action_frame->len))
			is_p2p_gas = true;
	}
#endif
	/* Loop to wait until we find a peer's channel or the
	 * pending action frame tx is cancelled.
	 */
	while ((cfg->afx_hdl->retry < max_retry) &&
		(cfg->afx_hdl->peer_chan == WL_INVALID)) {
		cfg->afx_hdl->is_listen = FALSE;
		wl_set_drv_status(cfg, SCANNING, dev);
		WL_DBG(("Scheduling the action frame for sending.. retry %d\n",
			cfg->afx_hdl->retry));
		/* search peer on peer's listen channel */
		schedule_work(&cfg->afx_hdl->work);
		wait_for_completion_timeout(&cfg->act_frm_scan,
			msecs_to_jiffies(WL_AF_SEARCH_TIME_MAX));

		if ((cfg->afx_hdl->peer_chan != WL_INVALID) ||
			!(wl_get_drv_status(cfg, FINDING_COMMON_CHANNEL, dev)))
#ifdef  BRCM_RSDB
			break;

		if (is_p2p_gas)
#endif
			break;

		if (cfg->afx_hdl->my_listen_chan) {
			WL_DBG(("Scheduling Listen peer in my listen channel = %d\n",
				cfg->afx_hdl->my_listen_chan));
			/* listen on my listen channel */
			cfg->afx_hdl->is_listen = TRUE;
			schedule_work(&cfg->afx_hdl->work);
			wait_for_completion_timeout(&cfg->act_frm_scan,
				msecs_to_jiffies(WL_AF_SEARCH_TIME_MAX));
		}
		if ((cfg->afx_hdl->peer_chan != WL_INVALID) ||
			!(wl_get_drv_status(cfg, FINDING_COMMON_CHANNEL, dev)))
			break;

		cfg->afx_hdl->retry++;

		WL_AF_TX_KEEP_PRI_CONNECTION_VSDB(cfg);
	}

	cfg->afx_hdl->is_active = FALSE;

	wl_clr_drv_status(cfg, SCANNING, dev);
	wl_clr_drv_status(cfg, FINDING_COMMON_CHANNEL, dev);

	return (cfg->afx_hdl->peer_chan);
}

struct p2p_config_af_params {
	s32 max_tx_retry;	/* max tx retry count if tx no ack */
	/* To make sure to send successfully action frame, we have to turn off mpc
	 * 0: off, 1: on,  (-1): do nothing
	 */
	s32 mpc_onoff;
#ifdef  BRCM_RSDB
#ifdef WL_CFG80211_GON_COLLISION
	/* drop tx go nego request if go nego collision occurs */
	bool drop_tx_req;
#endif
#endif
#ifdef WL_CFG80211_SYNC_GON
	bool extra_listen;
#endif
	bool search_channel;	/* 1: search peer's channel to send af */
};

static s32
wl_cfg80211_config_p2p_pub_af_tx(struct wiphy *wiphy,
	wl_action_frame_t *action_frame, wl_af_params_t *af_params,
	struct p2p_config_af_params *config_af_params)
{
	s32 err = BCME_OK;
	struct bcm_cfg80211 *cfg = wiphy_priv(wiphy);
	wifi_p2p_pub_act_frame_t *act_frm =
		(wifi_p2p_pub_act_frame_t *) (action_frame->data);

	/* initialize default value */
#ifdef  BRCM_RSDB
#ifdef WL_CFG80211_GON_COLLISION
	config_af_params->drop_tx_req = false;
#endif
#endif
#ifdef WL_CFG80211_SYNC_GON
	config_af_params->extra_listen = true;
#endif
	config_af_params->search_channel = false;
	config_af_params->max_tx_retry = WL_AF_TX_MAX_RETRY;
	config_af_params->mpc_onoff = -1;
	cfg->next_af_subtype = P2P_PAF_SUBTYPE_INVALID;

	switch (act_frm->subtype) {
	case P2P_PAF_GON_REQ: {
		WL_DBG(("P2P: GO_NEG_PHASE status set \n"));
		wl_set_p2p_status(cfg, GO_NEG_PHASE);

		config_af_params->mpc_onoff = 0;
#ifndef HW_P2P_CANNOT_CONNECT
		/* solve the p2p can't connect to eachother issue */
		config_af_params->search_channel = true;
#endif
		cfg->next_af_subtype = act_frm->subtype + 1;

		/* increase dwell time to wait for RESP frame */
		af_params->dwell_time = WL_MED_DWELL_TIME;
#ifdef  BRCM_RSDB
#ifdef WL_CFG80211_GON_COLLISION
		config_af_params->drop_tx_req = true;
#endif /* WL_CFG80211_GON_COLLISION */
#endif
		break;
	}
	case P2P_PAF_GON_RSP: {
		cfg->next_af_subtype = act_frm->subtype + 1;
		/* increase dwell time to wait for CONF frame */
		af_params->dwell_time = WL_MED_DWELL_TIME + 100;
		break;
	}
	case P2P_PAF_GON_CONF: {
		/* If we reached till GO Neg confirmation reset the filter */
		WL_DBG(("P2P: GO_NEG_PHASE status cleared \n"));
		wl_clr_p2p_status(cfg, GO_NEG_PHASE);

		/* turn on mpc again if go nego is done */
		config_af_params->mpc_onoff = 1;

		/* minimize dwell time */
		af_params->dwell_time = WL_MIN_DWELL_TIME;
#ifdef  BRCM_RSDB
#ifdef WL_CFG80211_GON_COLLISION
		/* if go nego formation done, clear it */
		cfg->block_gon_req_tx_count = 0;
		cfg->block_gon_req_rx_count = 0;
#endif /* WL_CFG80211_GON_COLLISION */
#endif
#ifdef WL_CFG80211_SYNC_GON
		config_af_params->extra_listen = false;
#endif /* WL_CFG80211_SYNC_GON */
		break;
	}
	case P2P_PAF_INVITE_REQ: {
		config_af_params->search_channel = true;
		cfg->next_af_subtype = act_frm->subtype + 1;

		/* increase dwell time */
		af_params->dwell_time = WL_MED_DWELL_TIME;
		break;
	}
	case P2P_PAF_INVITE_RSP:
		/* minimize dwell time */
		af_params->dwell_time = WL_MIN_DWELL_TIME;
#ifdef WL_CFG80211_SYNC_GON
		config_af_params->extra_listen = false;
#endif /* WL_CFG80211_SYNC_GON */
		break;
	case P2P_PAF_DEVDIS_REQ: {
		if (IS_ACTPUB_WITHOUT_GROUP_ID(&act_frm->elts[0],
			action_frame->len)) {
			config_af_params->search_channel = true;
		}

		cfg->next_af_subtype = act_frm->subtype + 1;
		/* maximize dwell time to wait for RESP frame */
		af_params->dwell_time = WL_LONG_DWELL_TIME;
		break;
	}
	case P2P_PAF_DEVDIS_RSP:
		/* minimize dwell time */
		af_params->dwell_time = WL_MIN_DWELL_TIME;
#ifdef WL_CFG80211_SYNC_GON
		config_af_params->extra_listen = false;
#endif /* WL_CFG80211_SYNC_GON */
		break;
	case P2P_PAF_PROVDIS_REQ: {
		if (IS_ACTPUB_WITHOUT_GROUP_ID(&act_frm->elts[0],
			action_frame->len)) {
			config_af_params->search_channel = true;
		}

		config_af_params->mpc_onoff = 0;
		cfg->next_af_subtype = act_frm->subtype + 1;
		/* increase dwell time to wait for RESP frame */
		af_params->dwell_time = WL_MED_DWELL_TIME;
		break;
	}
	case P2P_PAF_PROVDIS_RSP: {
		cfg->next_af_subtype = P2P_PAF_GON_REQ;
		af_params->dwell_time = WL_MIN_DWELL_TIME;
#ifdef WL_CFG80211_SYNC_GON
		config_af_params->extra_listen = false;
#endif /* WL_CFG80211_SYNC_GON */
		break;
	}
	default:
		WL_DBG(("Unknown p2p pub act frame subtype: %d\n",
			act_frm->subtype));
		err = BCME_BADARG;
	}
	return err;
}

#ifdef WL11U
static bool
wl_cfg80211_check_DFS_channel(struct bcm_cfg80211 *cfg, wl_af_params_t *af_params,
	void *frame, u16 frame_len)
{
	struct wl_scan_results *bss_list;
	struct wl_bss_info *bi = NULL;
	bool result = false;
	s32 i;
	chanspec_t chanspec;

	/* If DFS channel is 52~148, check to block it or not */
	if (af_params &&
		(af_params->channel >= 52 && af_params->channel <= 148)) {
		if (!wl_cfgp2p_is_p2p_action(frame, frame_len)) {
			bss_list = cfg->bss_list;
			bi = next_bss(bss_list, bi);
			for_each_bss(bss_list, bi, i) {
				chanspec = wl_chspec_driver_to_host(bi->chanspec);
				if (CHSPEC_IS5G(chanspec) &&
					((bi->ctl_ch ? bi->ctl_ch : CHSPEC_CHANNEL(chanspec))
					== af_params->channel)) {
					result = true;	/* do not block the action frame */
					break;
				}
			}
		}
	}
	else {
		result = true;
	}

	WL_DBG(("result=%s", result?"true":"false"));
	return result;
}
#endif /* WL11U */
#ifdef  BRCM_RSDB
static bool
wl_cfg80211_check_dwell_overflow(int32 requested_dwell, ulong dwell_jiffies)
{
	if ((requested_dwell & CUSTOM_RETRY_MASK) &&
			(jiffies_to_msecs(jiffies - dwell_jiffies) >
			 (requested_dwell & ~CUSTOM_RETRY_MASK))) {
		WL_ERR(("Action frame TX retry time over dwell time!\n"));
		return true;
	}
	return false;
}
#endif
static bool
wl_cfg80211_send_action_frame(struct wiphy *wiphy, struct net_device *dev,
	bcm_struct_cfgdev *cfgdev, wl_af_params_t *af_params,
	wl_action_frame_t *action_frame, u16 action_frame_len, s32 bssidx)
{
#ifdef WL11U
	struct net_device *ndev = NULL;
#endif /* WL11U */
	struct bcm_cfg80211 *cfg = wiphy_priv(wiphy);
	bool ack = false;
	u8 category, action;
	s32 tx_retry;
	struct p2p_config_af_params config_af_params;
	int ret_tmp = 0;
#ifdef VSDB
	ulong off_chan_started_jiffies = 0;
#endif
#ifdef  BRCM_RSDB
	struct net_info *netinfo;
	ulong dwell_jiffies = 0;
	bool dwell_overflow = false;
	int32 requested_dwell = af_params->dwell_time;
#endif
	dhd_pub_t *dhd = (dhd_pub_t *)(cfg->pub);


	/* Add the default dwell time
	 * Dwell time to stay off-channel to wait for a response action frame
	 * after transmitting an GO Negotiation action frame
	 */
	af_params->dwell_time = WL_DWELL_TIME;

#ifdef WL11U
#if defined(WL_CFG80211_P2P_DEV_IF)
	ndev = dev;
#else
	ndev = ndev_to_cfgdev(cfgdev);
#endif /* WL_CFG80211_P2P_DEV_IF */
#endif /* WL11U */

	category = action_frame->data[DOT11_ACTION_CAT_OFF];
	action = action_frame->data[DOT11_ACTION_ACT_OFF];

	/* initialize variables */
	tx_retry = 0;
	cfg->next_af_subtype = P2P_PAF_SUBTYPE_INVALID;
	config_af_params.max_tx_retry = WL_AF_TX_MAX_RETRY;
	config_af_params.mpc_onoff = -1;
	config_af_params.search_channel = false;
#ifdef  BRCM_RSDB
#ifdef WL_CFG80211_GON_COLLISION
	config_af_params.drop_tx_req = false;
#endif
#endif
#ifdef WL_CFG80211_SYNC_GON
	config_af_params.extra_listen = false;
#endif

	/* config parameters */
	/* Public Action Frame Process - DOT11_ACTION_CAT_PUBLIC */
	if (category == DOT11_ACTION_CAT_PUBLIC) {
		if ((action == P2P_PUB_AF_ACTION) &&
			(action_frame_len >= sizeof(wifi_p2p_pub_act_frame_t))) {
			/* p2p public action frame process */
			if (BCME_OK != wl_cfg80211_config_p2p_pub_af_tx(wiphy,
				action_frame, af_params, &config_af_params)) {
				WL_DBG(("Unknown subtype.\n"));
			}
#ifdef  BRCM_RSDB
#ifdef WL_CFG80211_GON_COLLISION
			if (config_af_params.drop_tx_req) {
				if (cfg->block_gon_req_tx_count) {
					/* drop gon req tx action frame */
					WL_DBG(("Drop gon req tx action frame: count %d\n",
						cfg->block_gon_req_tx_count));
					goto exit;
				}
			}
#endif /* WL_CFG80211_GON_COLLISION */
#endif
		} else if (action_frame_len >= sizeof(wifi_p2psd_gas_pub_act_frame_t)) {
			/* service discovery process */
			if (action == P2PSD_ACTION_ID_GAS_IREQ ||
				action == P2PSD_ACTION_ID_GAS_CREQ) {
				/* configure service discovery query frame */

				config_af_params.search_channel = true;

				/* save next af suptype to cancel remained dwell time */
				cfg->next_af_subtype = action + 1;

				af_params->dwell_time = WL_MED_DWELL_TIME;
#ifdef  BRCM_RSDB
				if (requested_dwell & CUSTOM_RETRY_MASK) {
					config_af_params.max_tx_retry =
						(requested_dwell & CUSTOM_RETRY_MASK) >> 24;
					af_params->dwell_time =
						(requested_dwell & ~CUSTOM_RETRY_MASK);
					WL_DBG(("Custom retry(%d) and dwell time(%d) is set.\n",
						config_af_params.max_tx_retry,
						af_params->dwell_time));
				}
#endif
			} else if (action == P2PSD_ACTION_ID_GAS_IRESP ||
				action == P2PSD_ACTION_ID_GAS_CRESP) {
				/* configure service discovery response frame */
				af_params->dwell_time = WL_MIN_DWELL_TIME;
			} else {
				WL_DBG(("Unknown action type: %d\n", action));
			}
		} else {
			WL_DBG(("Unknown Frame: category 0x%x, action 0x%x, length %d\n",
				category, action, action_frame_len));
	}
	} else if (category == P2P_AF_CATEGORY) {
		/* do not configure anything. it will be sent with a default configuration */
	} else {
		WL_DBG(("Unknown Frame: category 0x%x, action 0x%x\n",
			category, action));
		if (dhd->op_mode & DHD_FLAG_HOSTAP_MODE) {
			wl_clr_drv_status(cfg, SENDING_ACT_FRM, dev);
			return false;
		}
	}

	/* To make sure to send successfully action frame, we have to turn off mpc */
	if (config_af_params.mpc_onoff == 0) {
		wldev_iovar_setint(dev, "mpc", 0);
	}

#ifdef  BRCM_RSDB
	netinfo = wl_get_netinfo_by_bssidx(cfg, bssidx);
	/* validate channel and p2p ies */
	if (config_af_params.search_channel && IS_P2P_SOCIAL(af_params->channel) &&
		netinfo && netinfo->bss.ies.probe_req_ie_len) {
#else
	/* validate channel and p2p ies */
	if (config_af_params.search_channel && IS_P2P_SOCIAL(af_params->channel) &&
		wl_to_p2p_bss_saved_ie(cfg, P2PAPI_BSSCFG_DEVICE).p2p_probe_req_ie_len) {
#endif
		config_af_params.search_channel = true;
	} else {
		config_af_params.search_channel = false;
	}
#ifdef WL11U
	if (ndev == bcmcfg_to_prmry_ndev(cfg))
		config_af_params.search_channel = false;
#endif /* WL11U */

#ifdef VSDB
	/* if connecting on primary iface, sleep for a while before sending af tx for VSDB */
	if (wl_get_drv_status(cfg, CONNECTING, bcmcfg_to_prmry_ndev(cfg))) {
		OSL_SLEEP(50);
	}
#endif

	/* if scan is ongoing, abort current scan. */
	if (wl_get_drv_status_all(cfg, SCANNING)) {
		wl_notify_escan_complete(cfg, cfg->escan_info.ndev, true, true);
	}
#ifdef  BRCM_RSDB
	/* Abort P2P listen */
	if (discover_cfgdev(cfgdev, cfg)) {
		if (cfg->p2p_supported && cfg->p2p) {
			wl_cfgp2p_set_p2p_mode(cfg, WL_P2P_DISC_ST_SCAN, 0, 0,
				wl_to_p2p_bss_bssidx(cfg, P2PAPI_BSSCFG_DEVICE));
		}
	}
#endif
#ifdef WL11U
	/* handling DFS channel exceptions */
	if (!wl_cfg80211_check_DFS_channel(cfg, af_params, action_frame->data, action_frame->len)) {
		return false;	/* the action frame was blocked */
	}
#endif /* WL11U */

	/* set status and destination address before sending af */
	if (cfg->next_af_subtype != P2P_PAF_SUBTYPE_INVALID) {
		/* set this status to cancel the remained dwell time in rx process */
		wl_set_drv_status(cfg, WAITING_NEXT_ACT_FRM, dev);
	}
	wl_set_drv_status(cfg, SENDING_ACT_FRM, dev);
	memcpy(cfg->afx_hdl->tx_dst_addr.octet,
		af_params->action_frame.da.octet,
		sizeof(cfg->afx_hdl->tx_dst_addr.octet));

	/* save af_params for rx process */
	cfg->afx_hdl->pending_tx_act_frm = af_params;
#ifdef  BRCM_RSDB
	if (wl_cfgp2p_is_p2p_gas_action(action_frame->data, action_frame->len)) {
		WL_DBG(("Set GAS action frame config.\n"));
		config_af_params.search_channel = false;
		config_af_params.max_tx_retry = 1;
	}
#endif
	/* search peer's channel */
	if (config_af_params.search_channel) {
		/* initialize afx_hdl */
#ifndef  BRCM_RSDB
		if (wl_cfgp2p_find_idx(cfg, dev, &cfg->afx_hdl->bssidx) != BCME_OK) {
			WL_ERR(("Find p2p index from dev(%p) failed\n", dev));
#else
		if ((cfg->afx_hdl->bssidx = wl_get_bssidx_by_wdev(cfg, dev->ieee80211_ptr)) < 0) {
			WL_ERR(("Find p2p index from wdev(%p) failed\n", dev->ieee80211_ptr));
#endif
			goto exit;
		}
		cfg->afx_hdl->dev = dev;
		cfg->afx_hdl->retry = 0;
		cfg->afx_hdl->peer_chan = WL_INVALID;

		if (wl_cfg80211_af_searching_channel(cfg, dev) == WL_INVALID) {
			WL_ERR(("couldn't find peer's channel.\n"));
			wl_cfgp2p_print_actframe(true, action_frame->data, action_frame->len,
				af_params->channel);
			goto exit;
		}

		wl_clr_drv_status(cfg, SCANNING, cfg->afx_hdl->dev);
		/*
		 * Abort scan even for VSDB scenarios. Scan gets aborted in firmware
		 * but after the check of piggyback algorithm.
		 * To take care of current piggback algo, lets abort the scan here itself.
		 */
		wl_notify_escan_complete(cfg, dev, true, true);
		/* Suspend P2P discovery's search-listen to prevent it from
		 * starting a scan or changing the channel.
		 */
#ifndef  BRCM_RSDB
		wl_cfgp2p_discover_enable_search(cfg, false);
#else
		if ((wl_cfgp2p_discover_enable_search(cfg, false)) < 0) {
			WL_ERR(("Can not disable discovery mode\n"));
			goto exit;
		}
#endif

		/* update channel */
		af_params->channel = cfg->afx_hdl->peer_chan;
	}

#ifdef VSDB
	off_chan_started_jiffies = jiffies;
#endif /* VSDB */

	wl_cfgp2p_print_actframe(true, action_frame->data, action_frame->len, af_params->channel);
#ifdef  BRCM_RSDB
	wl_cfgp2p_need_wait_actfrmae(cfg, action_frame->data, action_frame->len, true);

	dwell_jiffies = jiffies;
#endif
	/* Now send a tx action frame */
	ret_tmp = wl_cfgp2p_tx_action_frame(cfg, dev, af_params, bssidx);
	ack = ret_tmp? false: true;
	WL_ERR(("wl_cfgp2p_tx_action_frame return %d", ret_tmp));
#ifdef  BRCM_RSDB
	dwell_overflow = wl_cfg80211_check_dwell_overflow(requested_dwell, dwell_jiffies);

	/* if failed, retry it. tx_retry_max value is configure by .... */
	while ((ack == false) && (tx_retry++ < config_af_params.max_tx_retry) &&
			!dwell_overflow) {
#else
	/* if failed, retry it. tx_retry_max value is configure by .... */
	while ((ack == false) && (tx_retry++ < config_af_params.max_tx_retry)) {
#endif
#ifdef VSDB
		if (af_params->channel) {
			if (jiffies_to_msecs(jiffies - off_chan_started_jiffies) >
				OFF_CHAN_TIME_THRESHOLD_MS) {
				WL_AF_TX_KEEP_PRI_CONNECTION_VSDB(cfg);
				off_chan_started_jiffies = jiffies;
			} else
				OSL_SLEEP(AF_RETRY_DELAY_TIME);
		}
#endif /* VSDB */
		ack = wl_cfgp2p_tx_action_frame(cfg, dev, af_params, bssidx) ?
			false : true;
#ifdef  BRCM_RSDB
		dwell_overflow = wl_cfg80211_check_dwell_overflow(requested_dwell, dwell_jiffies);
#endif
	}

	if (ack == false) {
		WL_ERR(("Failed to send Action Frame(retry %d)\n", tx_retry));
	}
	WL_DBG(("Complete to send action frame\n"));
exit:
	/* Clear SENDING_ACT_FRM after all sending af is done */
	wl_clr_drv_status(cfg, SENDING_ACT_FRM, dev);

#ifdef WL_CFG80211_SYNC_GON
	/* WAR: sometimes dongle does not keep the dwell time of 'actframe'.
	 * if we coundn't get the next action response frame and dongle does not keep
	 * the dwell time, go to listen state again to get next action response frame.
	 */
	if (ack && config_af_params.extra_listen &&
#ifdef  BRCM_RSDB
#ifdef WL_CFG80211_GON_COLLISION
		!cfg->block_gon_req_tx_count &&
#endif /* WL_CFG80211_GON_COLLISION */
#endif
		wl_get_drv_status_all(cfg, WAITING_NEXT_ACT_FRM) &&
		cfg->af_sent_channel == cfg->afx_hdl->my_listen_chan) {
		s32 extar_listen_time;

		extar_listen_time = af_params->dwell_time -
			jiffies_to_msecs(jiffies - cfg->af_tx_sent_jiffies);

		if (extar_listen_time > 50) {
			wl_set_drv_status(cfg, WAITING_NEXT_ACT_FRM_LISTEN, dev);
			WL_DBG(("Wait more time! actual af time:%d,"
				"calculated extar listen:%d\n",
				af_params->dwell_time, extar_listen_time));
			if (wl_cfgp2p_discover_listen(cfg, cfg->af_sent_channel,
				extar_listen_time + 100) == BCME_OK) {
				wait_for_completion_timeout(&cfg->wait_next_af,
					msecs_to_jiffies(extar_listen_time + 100 + 300));
			}
			wl_clr_drv_status(cfg, WAITING_NEXT_ACT_FRM_LISTEN, dev);
		}
	}
#endif /* WL_CFG80211_SYNC_GON */
	wl_clr_drv_status(cfg, WAITING_NEXT_ACT_FRM, dev);

	if (cfg->afx_hdl->pending_tx_act_frm)
		cfg->afx_hdl->pending_tx_act_frm = NULL;

	WL_INFORM(("-- sending Action Frame is %s, listen chan: %d\n",
		(ack) ? "Succeeded!!":"Failed!!", cfg->afx_hdl->my_listen_chan));
#ifdef  BRCM_RSDB
#ifdef WL_CFG80211_GON_COLLISION
	if (cfg->block_gon_req_tx_count) {
		cfg->block_gon_req_tx_count--;
		/* if ack is ture, supplicant will wait more time(100ms).
		 * so we will return it as a success to get more time .
		 */
		ack = true;
	}
#endif /* WL_CFG80211_GON_COLLISION */
#endif
	/* if all done, turn mpc on again */
	if (config_af_params.mpc_onoff == 1) {
		wldev_iovar_setint(dev, "mpc", 1);
	}

	return ack;
}

#define MAX_NUM_OF_ASSOCIATED_DEV       64
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0))
static s32
wl_cfg80211_mgmt_tx(struct wiphy *wiphy, bcm_struct_cfgdev *cfgdev,
	struct cfg80211_mgmt_tx_params *params, u64 *cookie)
#else
static s32
wl_cfg80211_mgmt_tx(struct wiphy *wiphy, bcm_struct_cfgdev *cfgdev,
	struct ieee80211_channel *channel, bool offchan,
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(3, 7, 0))
	enum nl80211_channel_type channel_type,
	bool channel_type_valid,
#endif /* LINUX_VERSION_CODE <= KERNEL_VERSION(3, 7, 0) */
	unsigned int wait, const u8* buf, size_t len,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 2, 0)) || defined(WL_COMPAT_WIRELESS)
	bool no_cck,
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 3, 0)) || defined(WL_COMPAT_WIRELESS)
	bool dont_wait_for_ack,
#endif
	u64 *cookie)
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0) */
{
	wl_action_frame_t *action_frame;
	wl_af_params_t *af_params;
	scb_val_t scb_val;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0))
	struct ieee80211_channel *channel = params->chan;
	const u8 *buf = params->buf;
	size_t len = params->len;
#endif
	const struct ieee80211_mgmt *mgmt;
	struct bcm_cfg80211 *cfg = wiphy_priv(wiphy);
	struct net_device *dev = NULL;
	s32 err = BCME_OK;
	s32 bssidx = 0;
	u32 id;
	bool ack = false;
	s8 eabuf[ETHER_ADDR_STR_LEN];

	WL_DBG(("Enter \n"));

	if (len > (ACTION_FRAME_SIZE + DOT11_MGMT_HDR_LEN)) {
		WL_ERR(("bad length:%zu\n", len));
		return BCME_BADARG;
	}
	dev = cfgdev_to_wlc_ndev(cfgdev, cfg);
#ifdef  BRCM_RSDB
	if (!dev) {
		WL_ERR(("dev is NULL\n"));
		return -EINVAL;
	}
#endif
	/* set bsscfg idx for iovar (wlan0: P2PAPI_BSSCFG_PRIMARY, p2p: P2PAPI_BSSCFG_DEVICE)	*/
	if (discover_cfgdev(cfgdev, cfg)) {
#ifdef BCM_PATCH_REBOOT_FOR_P2PNULL
		if (!cfg->p2p_supported || !cfg->p2p) {
			WL_ERR(("P2P doesn't setup completed yet\n"));
			return -EINVAL;
		}
#endif
		bssidx = wl_to_p2p_bss_bssidx(cfg, P2PAPI_BSSCFG_DEVICE);
	}
	else {
#ifndef  BRCM_RSDB
		if (wl_cfgp2p_find_idx(cfg, dev, &bssidx) != BCME_OK) {
			WL_ERR(("Find p2p index from dev(%p) failed\n", dev));
			return BCME_ERROR;
#else
		if ((bssidx = wl_get_bssidx_by_wdev(cfg, cfgdev_to_wdev(cfgdev))) < 0) {
			WL_ERR(("Find p2p index failed\n"));
			return BCME_ERROR;
#endif
		}
	}

	WL_DBG(("TX target bssidx=%d\n", bssidx));

	if (p2p_is_on(cfg)) {
		/* Suspend P2P discovery search-listen to prevent it from changing the
		 * channel.
		 */
		if ((err = wl_cfgp2p_discover_enable_search(cfg, false)) < 0) {
			WL_ERR(("Can not disable discovery mode\n"));
			return -EFAULT;
		}
	}
	*cookie = 0;
	id = cfg->send_action_id++;
	if (id == 0)
		id = cfg->send_action_id++;
	*cookie = id;
	mgmt = (const struct ieee80211_mgmt *)buf;
	if (ieee80211_is_mgmt(mgmt->frame_control)) {
		if (ieee80211_is_probe_resp(mgmt->frame_control)) {
			s32 ie_offset =  DOT11_MGMT_HDR_LEN + DOT11_BCN_PRB_FIXED_LEN;
			s32 ie_len = len - ie_offset;
#ifndef  BRCM_RSDB
			if ((dev == bcmcfg_to_prmry_ndev(cfg)) && cfg->p2p)
				bssidx = wl_to_p2p_bss_bssidx(cfg, P2PAPI_BSSCFG_DEVICE);
				wl_cfgp2p_set_management_ie(cfg, dev, bssidx,
				VNDR_IE_PRBRSP_FLAG, (u8 *)(buf + ie_offset), ie_len);
			cfg80211_mgmt_tx_status(cfgdev, *cookie, buf, len, true, GFP_KERNEL);
#else
			if ((dev == bcmcfg_to_prmry_ndev(cfg)) && cfg->p2p) {
				bssidx = wl_to_p2p_bss_bssidx(cfg, P2PAPI_BSSCFG_DEVICE);
			}
			wl_cfg80211_set_mgmt_vndr_ies(cfg, ndev_to_cfgdev(dev), bssidx,
				VNDR_IE_PRBRSP_FLAG, (const u8 *)(buf + ie_offset), ie_len);
			cfg80211_mgmt_tx_status(cfgdev, *cookie, buf, len, true, GFP_KERNEL);
#if defined(P2P_IE_MISSING_FIX)
			if (!cfg->p2p_prb_noti) {
				cfg->p2p_prb_noti = true;
				WL_DBG(("%s: TX 802_1X Probe Response first time.\n",
					__FUNCTION__));
			}
#endif
#endif
			goto exit;
		} else if (ieee80211_is_disassoc(mgmt->frame_control) ||
			ieee80211_is_deauth(mgmt->frame_control)) {
			char mac_buf[MAX_NUM_OF_ASSOCIATED_DEV *
				sizeof(struct ether_addr) + sizeof(uint)] = {0};
			int num_associated = 0;
			struct maclist *assoc_maclist = (struct maclist *)mac_buf;
			if (!bcmp((const uint8 *)BSSID_BROADCAST,
				(const struct ether_addr *)mgmt->da, ETHER_ADDR_LEN)) {
				assoc_maclist->count = MAX_NUM_OF_ASSOCIATED_DEV;
				err = wldev_ioctl(dev, WLC_GET_ASSOCLIST,
					assoc_maclist, sizeof(mac_buf), false);
				if (err < 0)
					WL_ERR(("WLC_GET_ASSOCLIST error %d\n", err));
				else
					num_associated = assoc_maclist->count;
			}
			memcpy(scb_val.ea.octet, mgmt->da, ETH_ALEN);
			scb_val.val = mgmt->u.disassoc.reason_code;
			err = wldev_ioctl(dev, WLC_SCB_DEAUTHENTICATE_FOR_REASON, &scb_val,
				sizeof(scb_val_t), true);
			if (err < 0)
				WL_ERR(("WLC_SCB_DEAUTHENTICATE_FOR_REASON error %d\n", err));
			WL_ERR(("Disconnect STA : %s scb_val.val %d\n",
				bcm_ether_ntoa((const struct ether_addr *)mgmt->da, eabuf),
				scb_val.val));

			if (num_associated > 0 && ETHER_ISBCAST(mgmt->da))
				wl_delay(400);

			cfg80211_mgmt_tx_status(cfgdev, *cookie, buf, len, true, GFP_KERNEL);
			goto exit;

		} else if (ieee80211_is_action(mgmt->frame_control)) {
			/* Abort the dwell time of any previous off-channel
			* action frame that may be still in effect.  Sending
			* off-channel action frames relies on the driver's
			* scan engine.  If a previous off-channel action frame
			* tx is still in progress (including the dwell time),
			* then this new action frame will not be sent out.
			*/
/* Do not abort scan for VSDB. Scan will be aborted in firmware if necessary.
 * And previous off-channel action frame must be ended before new af tx.
 */
#ifndef WL_CFG80211_VSDB_PRIORITIZE_SCAN_REQUEST
			wl_notify_escan_complete(cfg, dev, true, true);
#endif /* not WL_CFG80211_VSDB_PRIORITIZE_SCAN_REQUEST */
		}

	} else {
		WL_ERR(("Driver only allows MGMT packet type\n"));
		goto exit;
	}

	af_params = (wl_af_params_t *) kzalloc(WL_WIFI_AF_PARAMS_SIZE, GFP_KERNEL);

	if (af_params == NULL)
	{
		WL_ERR(("unable to allocate frame\n"));
		return -ENOMEM;
	}

	action_frame = &af_params->action_frame;

	/* Add the packet Id */
	action_frame->packetId = *cookie;
	WL_DBG(("action frame %d\n", action_frame->packetId));
	/* Add BSSID */
	memcpy(&action_frame->da, &mgmt->da[0], ETHER_ADDR_LEN);
	memcpy(&af_params->BSSID, &mgmt->bssid[0], ETHER_ADDR_LEN);

	/* Add the length exepted for 802.11 header  */
	action_frame->len = len - DOT11_MGMT_HDR_LEN;
	WL_DBG(("action_frame->len: %d\n", action_frame->len));

	/* Add the channel */
	af_params->channel =
		ieee80211_frequency_to_channel(channel->center_freq);
	/* Save listen_chan for searching common channel */
	cfg->afx_hdl->peer_listen_chan = af_params->channel;
	WL_DBG(("channel from upper layer %d\n", cfg->afx_hdl->peer_listen_chan));

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0))
	af_params->dwell_time = params->wait;
#else
	af_params->dwell_time = wait;
#endif

	memcpy(action_frame->data, &buf[DOT11_MGMT_HDR_LEN], action_frame->len);

	ack = wl_cfg80211_send_action_frame(wiphy, dev, cfgdev, af_params,
		action_frame, action_frame->len, bssidx);
	cfg80211_mgmt_tx_status(cfgdev, *cookie, buf, len, ack, GFP_KERNEL);

	kfree(af_params);
exit:
	return err;
}


static void
wl_cfg80211_mgmt_frame_register(struct wiphy *wiphy, bcm_struct_cfgdev *cfgdev,
	u16 frame_type, bool reg)
{

	WL_DBG(("frame_type: %x, reg: %d\n", frame_type, reg));

	if (frame_type != (IEEE80211_FTYPE_MGMT | IEEE80211_STYPE_PROBE_REQ))
		return;

	return;
}


static s32
wl_cfg80211_change_bss(struct wiphy *wiphy,
	struct net_device *dev,
	struct bss_parameters *params)
{
	s32 err = 0;
	s32 ap_isolate = 0;

	if (params->use_cts_prot >= 0) {
	}

	if (params->use_short_preamble >= 0) {
	}

	if (params->use_short_slot_time >= 0) {
	}

	if (params->basic_rates) {
	}

	if (params->ap_isolate >= 0) {
		ap_isolate = params->ap_isolate;
		err = wldev_iovar_setint(dev, "ap_isolate", ap_isolate);
		if (unlikely(err))
		{
			WL_ERR(("set ap_isolate Error (%d)\n", err));
		}
	}

	if (params->ht_opmode >= 0) {
	}


	return 0;
}

static s32
wl_cfg80211_set_channel(struct wiphy *wiphy, struct net_device *dev,
	struct ieee80211_channel *chan,
	enum nl80211_channel_type channel_type)
{
	s32 _chan;
	chanspec_t chspec = 0;
	chanspec_t fw_chspec = 0;
	u32 bw = WL_CHANSPEC_BW_20;
#ifdef HW_SOFTAP_HT40
	int channel = 0;
	int center_chan;
#endif
#ifdef  BRCM_RSDB
#ifdef WL11ULB
	u32 ulb_bw = wl_cfg80211_get_ulb_bw(dev->ieee80211_ptr);
#endif /* WL11ULB */
#endif

	s32 err = BCME_OK;
	s32 bw_cap = 0;
	struct {
		u32 band;
		u32 bw_cap;
	} param = {0, 0};
	struct bcm_cfg80211 *cfg = wiphy_priv(wiphy);
#ifdef CUSTOM_SET_CPUCORE
	dhd_pub_t *dhd =  (dhd_pub_t *)(cfg->pub);
#endif /* CUSTOM_SET_CPUCORE */

	dev = ndev_to_wlc_ndev(dev, cfg);
	_chan = ieee80211_frequency_to_channel(chan->center_freq);
	WL_ERR(("netdev_ifidx(%d), chan_type(%d) target channel(%d) \n",
		dev->ifindex, channel_type, _chan));
#ifdef  BRCM_RSDB
#ifdef NOT_YET
	switch (channel_type) {
		case NL80211_CHAN_HT40MINUS:
			/* secondary channel is below the control channel */
			chspec = CH40MHZ_CHSPEC(channel, WL_CHANSPEC_CTL_SB_UPPER);
			break;
		case NL80211_CHAN_HT40PLUS:
			/* secondary channel is above the control channel */
			chspec = CH40MHZ_CHSPEC(channel, WL_CHANSPEC_CTL_SB_LOWER);
			break;
		default:
			chspec = CH20MHZ_CHSPEC(channel);

	}
#endif /* NOT_YET */

#ifdef WL11ULB
	if (ulb_bw) {
		WL_DBG(("[ULB] setting AP/GO BW to ulb_bw 0x%x \n", ulb_bw));
		bw = wl_cfg80211_ulbbw_to_ulbchspec(ulb_bw);
		goto set_channel;
	}
#endif /* WL11ULB */
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0))
	if (chan->band == NL80211_BAND_5GHZ) {
#else
	if (chan->band == IEEE80211_BAND_5GHZ) {
#endif
		param.band = WLC_BAND_5G;
		err = wldev_iovar_getbuf(dev, "bw_cap", &param, sizeof(param),
			cfg->ioctl_buf, WLC_IOCTL_SMLEN, &cfg->ioctl_buf_sync);
		if (err) {
			if (err != BCME_UNSUPPORTED) {
				WL_ERR(("bw_cap failed, %d\n", err));
				return err;
			} else {
				err = wldev_iovar_getint(dev, "mimo_bw_cap", &bw_cap);
				if (err) {
					WL_ERR(("error get mimo_bw_cap (%d)\n", err));
				}
				if (bw_cap != WLC_N_BW_20ALL)
					bw = WL_CHANSPEC_BW_40;
			}
		} else {
			if (WL_BW_CAP_80MHZ(cfg->ioctl_buf[0]))
				bw = WL_CHANSPEC_BW_80;
			else if (WL_BW_CAP_40MHZ(cfg->ioctl_buf[0]))
				bw = WL_CHANSPEC_BW_40;
			else
				bw = WL_CHANSPEC_BW_20;

		}
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0))
	} else if (chan->band == NL80211_BAND_2GHZ)
#else
	} else if (chan->band == IEEE80211_BAND_2GHZ)
#endif
		bw = WL_CHANSPEC_BW_20;
set_channel:
#ifdef HW_SOFTAP_HT40
        channel = _chan;
        WL_ERR(("softap support 40MHz & 20MHz, channel:%d\n", channel));
        switch (channel) {
                case 1:
                case 2:
                case 3:
                case 4:
                case 5:
                case 6:
                    center_chan = channel + CH_10MHZ_APART;
                    chspec = CH40MHZ_CHSPEC(center_chan, WL_CHANSPEC_CTL_SB_LOWER);
                    break;
                case 7:
                case 8:
                case 9:
                case 10:
                case 11:
                case 12:
                case 13:
                    center_chan = channel - CH_10MHZ_APART;
                    chspec = CH40MHZ_CHSPEC(center_chan, WL_CHANSPEC_CTL_SB_UPPER);
                    break;
                default:
                    chspec = CH20MHZ_CHSPEC(channel);
                break;
        }
#else
	chspec = wf_channel2chspec(_chan, bw);
#endif
	if (wf_chspec_valid(chspec)) {
		fw_chspec = wl_chspec_host_to_driver(chspec);
		if (fw_chspec != INVCHANSPEC) {
			if ((err = wldev_iovar_setint(dev, "chanspec",
				fw_chspec)) == BCME_BADCHAN) {
				if (bw == WL_CHANSPEC_BW_80)
					goto change_bw;
				err = wldev_ioctl(dev, WLC_SET_CHANNEL,
					&_chan, sizeof(_chan), true);
				if (err < 0) {
					WL_ERR(("WLC_SET_CHANNEL error %d"
					"chip may not be supporting this channel\n", err));
				}
			} else if (err) {
				WL_ERR(("failed to set chanspec error %d\n", err));
			}
		} else {
			WL_ERR(("failed to convert host chanspec to fw chanspec\n"));
			err = BCME_ERROR;
		}
	} else {
change_bw:
		if (bw == WL_CHANSPEC_BW_80)
			bw = WL_CHANSPEC_BW_40;
		else if (bw == WL_CHANSPEC_BW_40)
			bw = WL_CHANSPEC_BW_20;
		else
			bw = 0;
		if (bw)
			goto set_channel;
		WL_ERR(("Invalid chanspec 0x%x\n", chspec));
		err = BCME_ERROR;
	}
#ifndef  BRCM_RSDB
#ifdef CUSTOM_SET_CPUCORE
	if (dhd->op_mode == DHD_FLAG_HOSTAP_MODE) {
		WL_DBG(("SoftAP mode do not need to set cpucore\n"));
	} else if ((dev == wl_to_p2p_bss_ndev(cfg, P2PAPI_BSSCFG_CONNECTION)) &&
		(chspec & WL_CHANSPEC_BW_80)) {
		/* If GO is vht80 */
		dhd->chan_isvht80 |= DHD_FLAG_P2P_MODE;
		dhd_set_cpucore(dhd, TRUE);
	}
#endif /* CUSTOM_SET_CPUCORE */
#else
#ifdef CUSTOM_SET_CPUCORE
	if (dhd->op_mode == DHD_FLAG_HOSTAP_MODE) {
		WL_DBG(("SoftAP mode do not need to set cpucore\n"));
	} else if (chspec & WL_CHANSPEC_BW_80) {
		/* SoftAp only mode do not need to set cpucore */
		if ((dev->ieee80211_ptr->iftype == NL80211_IFTYPE_AP) &&
			dev != bcmcfg_to_prmry_ndev(cfg)) {
			/* Soft AP on virtual Iface (AP+STA case) */
			dhd->chan_isvht80 |= DHD_FLAG_HOSTAP_MODE;
			dhd_set_cpucore(dhd, TRUE);
		} else if (is_p2p_group_iface(dev->ieee80211_ptr)) {
			/* If P2P IF is vht80 */
			dhd->chan_isvht80 |= DHD_FLAG_P2P_MODE;
			dhd_set_cpucore(dhd, TRUE);
		}
	}
#endif /* CUSTOM_SET_CPUCORE */
	if (!err && (wl_get_mode_by_netdev(cfg, dev) == WL_MODE_AP)) {
		/* Update AP/GO operating channel */
		cfg->ap_oper_channel = _chan;
	}
#endif /* BRCM_RSDB */

	return err;
}

#ifdef WL_CFG80211_VSDB_PRIORITIZE_SCAN_REQUEST
struct net_device *
wl_cfg80211_get_remain_on_channel_ndev(struct bcm_cfg80211 *cfg)
{
	struct net_info *_net_info, *next;
	list_for_each_entry_safe(_net_info, next, &cfg->net_list, list) {
		if (_net_info->ndev &&
			test_bit(WL_STATUS_REMAINING_ON_CHANNEL, &_net_info->sme_state))
			return _net_info->ndev;
	}
	return NULL;
}
#endif /* WL_CFG80211_VSDB_PRIORITIZE_SCAN_REQUEST */
#ifndef  BRCM_RSDB
static s32
wl_validate_opensecurity(struct net_device *dev, s32 bssidx)
#else
static s32
wl_validate_opensecurity(struct net_device *dev, s32 bssidx, bool privacy)
#endif
{
	s32 err = BCME_OK;
#ifdef  BRCM_RSDB
	u32 wpa_val;
	s32 wsec = 0;
#endif
	/* set auth */
	err = wldev_iovar_setint_bsscfg(dev, "auth", 0, bssidx);
	if (err < 0) {
		WL_ERR(("auth error %d\n", err));
		return BCME_ERROR;
	}
#ifdef  BRCM_RSDB
	if (privacy) {
		/* If privacy bit is set in open mode, then WEP would be enabled */
		wsec = WEP_ENABLED;
		WL_DBG(("Setting wsec to %d for WEP \n", wsec));
	}
#endif
	/* set wsec */
#ifndef  BRCM_RSDB
	err = wldev_iovar_setint_bsscfg(dev, "wsec", 0, bssidx);
#else
	err = wldev_iovar_setint_bsscfg(dev, "wsec", wsec, bssidx);
#endif
	if (err < 0) {
		WL_ERR(("wsec error %d\n", err));
		return BCME_ERROR;
	}

	/* set upper-layer auth */
#ifdef  BRCM_RSDB
	if (dev->ieee80211_ptr->iftype == NL80211_IFTYPE_ADHOC)
		wpa_val = WPA_AUTH_NONE;
	else
		wpa_val = WPA_AUTH_DISABLED;
	err = wldev_iovar_setint_bsscfg(dev, "wpa_auth", wpa_val, bssidx);
#else
	err = wldev_iovar_setint_bsscfg(dev, "wpa_auth", WPA_AUTH_NONE, bssidx);
#endif
	if (err < 0) {
		WL_ERR(("wpa_auth error %d\n", err));
		return BCME_ERROR;
	}

	return 0;
}

static s32
wl_validate_wpa2ie(struct net_device *dev, bcm_tlv_t *wpa2ie, s32 bssidx)
{
	s32 len = 0;
	s32 err = BCME_OK;
	u16 auth = 0; /* d11 open authentication */
	u32 wsec;
	u32 pval = 0;
	u32 gval = 0;
	u32 wpa_auth = 0;
	wpa_suite_mcast_t *mcast;
	wpa_suite_ucast_t *ucast;
	wpa_suite_auth_key_mgmt_t *mgmt;
#ifdef  BRCM_RSDB
	wpa_pmkid_list_t *pmkid;
	int cnt = 0;
#ifdef MFP
	int mfp = 0;
	struct bcm_cfg80211 *cfg = g_bcm_cfg;
#endif /* MFP */
#endif
	u16 suite_count;
	u8 rsn_cap[2];
	u32 wme_bss_disable;

	if (wpa2ie == NULL)
		goto exit;

	WL_DBG(("Enter \n"));
#ifndef  BRCM_RSDB
	len =  wpa2ie->len;
#else
	len =  wpa2ie->len - WPA2_VERSION_LEN;
#endif
	/* check the mcast cipher */
	mcast = (wpa_suite_mcast_t *)&wpa2ie->data[WPA2_VERSION_LEN];
	switch (mcast->type) {
		case WPA_CIPHER_NONE:
			gval = 0;
			break;
		case WPA_CIPHER_WEP_40:
		case WPA_CIPHER_WEP_104:
			gval = WEP_ENABLED;
			break;
		case WPA_CIPHER_TKIP:
			gval = TKIP_ENABLED;
			break;
		case WPA_CIPHER_AES_CCM:
			gval = AES_ENABLED;
			break;
#ifdef BCMWAPI_WPI
		case WAPI_CIPHER_SMS4:
			gval = SMS4_ENABLED;
			break;
#endif
		default:
			WL_ERR(("No Security Info\n"));
			break;
	}
	if ((len -= WPA_SUITE_LEN) <= 0)
		return BCME_BADLEN;

	/* check the unicast cipher */
	ucast = (wpa_suite_ucast_t *)&mcast[1];
	suite_count = ltoh16_ua(&ucast->count);
	switch (ucast->list[0].type) {
		case WPA_CIPHER_NONE:
			pval = 0;
			break;
		case WPA_CIPHER_WEP_40:
		case WPA_CIPHER_WEP_104:
			pval = WEP_ENABLED;
			break;
		case WPA_CIPHER_TKIP:
			pval = TKIP_ENABLED;
			break;
		case WPA_CIPHER_AES_CCM:
			pval = AES_ENABLED;
			break;
#ifdef BCMWAPI_WPI
		case WAPI_CIPHER_SMS4:
			pval = SMS4_ENABLED;
			break;
#endif
		default:
			WL_ERR(("No Security Info\n"));
	}
	if ((len -= (WPA_IE_SUITE_COUNT_LEN + (WPA_SUITE_LEN * suite_count))) <= 0)
		return BCME_BADLEN;

	/* FOR WPS , set SEC_OW_ENABLED */
	wsec = (pval | gval | SES_OW_ENABLED);
	/* check the AKM */
	mgmt = (wpa_suite_auth_key_mgmt_t *)&ucast->list[suite_count];
#ifndef  BRCM_RSDB
	suite_count = ltoh16_ua(&mgmt->count);
	switch (mgmt->list[0].type) {
		case RSN_AKM_NONE:
			wpa_auth = WPA_AUTH_NONE;
			break;
		case RSN_AKM_UNSPECIFIED:
			wpa_auth = WPA2_AUTH_UNSPECIFIED;
			break;
		case RSN_AKM_PSK:
			wpa_auth = WPA2_AUTH_PSK;
			break;
		default:
			WL_ERR(("No Key Mgmt Info\n"));
#else
	suite_count = cnt = ltoh16_ua(&mgmt->count);
	while (cnt--) {
		switch (mgmt->list[cnt].type) {
		case RSN_AKM_NONE:
			wpa_auth |= WPA_AUTH_NONE;
			break;
		case RSN_AKM_UNSPECIFIED:
			wpa_auth |= WPA2_AUTH_UNSPECIFIED;
			break;
		case RSN_AKM_PSK:
			wpa_auth |= WPA2_AUTH_PSK;
			break;
#ifdef MFP
		case RSN_AKM_MFP_PSK:
			wpa_auth |= WPA2_AUTH_PSK_SHA256;
			break;
		case RSN_AKM_MFP_1X:
			wpa_auth |= WPA2_AUTH_1X_SHA256;
			break;
#endif /* MFP */
		default:
			WL_ERR(("No Key Mgmt Info\n"));
		}
#endif
	}

	if ((len -= (WPA_IE_SUITE_COUNT_LEN + (WPA_SUITE_LEN * suite_count))) >= RSN_CAP_LEN) {
		rsn_cap[0] = *(u8 *)&mgmt->list[suite_count];
		rsn_cap[1] = *((u8 *)&mgmt->list[suite_count] + 1);

		if (rsn_cap[0] & (RSN_CAP_16_REPLAY_CNTRS << RSN_CAP_PTK_REPLAY_CNTR_SHIFT)) {
			wme_bss_disable = 0;
		} else {
			wme_bss_disable = 1;
		}
#ifdef  BRCM_RSDB
#ifdef MFP
	if (rsn_cap[0] & RSN_CAP_MFPR) {
		WL_DBG(("MFP Required \n"));
		mfp = WL_MFP_REQUIRED;
		/* Our firmware has requirement that WPA2_AUTH_PSK/WPA2_AUTH_UNSPECIFIED
		 * be set, if SHA256 OUI is to be included in the rsn ie.
		 */
		if (wpa_auth & WPA2_AUTH_PSK_SHA256) {
			wpa_auth |= WPA2_AUTH_PSK;
		} else if (wpa_auth & WPA2_AUTH_1X_SHA256) {
			wpa_auth |= WPA2_AUTH_UNSPECIFIED;
		}
	} else if (rsn_cap[0] & RSN_CAP_MFPC) {
		WL_DBG(("MFP Capable \n"));
		mfp = WL_MFP_CAPABLE;
	}
#endif /* MFP */
#endif
		/* set wme_bss_disable to sync RSN Capabilities */
		err = wldev_iovar_setint_bsscfg(dev, "wme_bss_disable", wme_bss_disable, bssidx);
		if (err < 0) {
			WL_ERR(("wme_bss_disable error %d\n", err));
			return BCME_ERROR;
		}
	} else {
		WL_DBG(("There is no RSN Capabilities. remained len %d\n", len));
	}
#ifdef  BRCM_RSDB
	len -= RSN_CAP_LEN;
	if (len >= WPA2_PMKID_COUNT_LEN) {
		pmkid = (wpa_pmkid_list_t *)((u8 *)&mgmt->list[suite_count] + RSN_CAP_LEN);
		cnt = ltoh16_ua(&pmkid->count);
		if (cnt != 0) {
			WL_ERR(("AP has non-zero PMKID count. Wrong!\n"));
			return BCME_ERROR;
		}
		/* since PMKID cnt is known to be 0 for AP, */
		/* so don't bother to send down this info to firmware */
	}

#ifdef MFP
	len -= WPA2_PMKID_COUNT_LEN;
	if (len >= WPA_SUITE_LEN) {
		cfg->bip_pos = (u8 *)&mgmt->list[suite_count] + RSN_CAP_LEN + WPA2_PMKID_COUNT_LEN;
	} else {
		cfg->bip_pos = NULL;
	}
#endif
#endif
	/* set auth */
	err = wldev_iovar_setint_bsscfg(dev, "auth", auth, bssidx);
	if (err < 0) {
		WL_ERR(("auth error %d\n", err));
		return BCME_ERROR;
	}
	/* set wsec */
	err = wldev_iovar_setint_bsscfg(dev, "wsec", wsec, bssidx);
	if (err < 0) {
		WL_ERR(("wsec error %d\n", err));
		return BCME_ERROR;
	}
#ifdef  BRCM_RSDB
#ifdef MFP
	cfg->mfp_mode = mfp;
#endif /* MFP */
#endif
	/* set upper-layer auth */
	err = wldev_iovar_setint_bsscfg(dev, "wpa_auth", wpa_auth, bssidx);
	if (err < 0) {
		WL_ERR(("wpa_auth error %d\n", err));
		return BCME_ERROR;
	}
exit:
	return 0;
}

static s32
wl_validate_wpaie(struct net_device *dev, wpa_ie_fixed_t *wpaie, s32 bssidx)
{
	wpa_suite_mcast_t *mcast;
	wpa_suite_ucast_t *ucast;
	wpa_suite_auth_key_mgmt_t *mgmt;
	u16 auth = 0; /* d11 open authentication */
	u16 count;
	s32 err = BCME_OK;
	s32 len = 0;
	u32 i;
	u32 wsec;
	u32 pval = 0;
	u32 gval = 0;
	u32 wpa_auth = 0;
	u32 tmp = 0;

	if (wpaie == NULL)
		goto exit;
	WL_DBG(("Enter \n"));
	len = wpaie->length;    /* value length */
	len -= WPA_IE_TAG_FIXED_LEN;
	/* check for multicast cipher suite */
	if (len < WPA_SUITE_LEN) {
		WL_INFORM(("no multicast cipher suite\n"));
		goto exit;
	}

	/* pick up multicast cipher */
	mcast = (wpa_suite_mcast_t *)&wpaie[1];
	len -= WPA_SUITE_LEN;
	if (!bcmp(mcast->oui, WPA_OUI, WPA_OUI_LEN)) {
		if (IS_WPA_CIPHER(mcast->type)) {
			tmp = 0;
			switch (mcast->type) {
				case WPA_CIPHER_NONE:
					tmp = 0;
					break;
				case WPA_CIPHER_WEP_40:
				case WPA_CIPHER_WEP_104:
					tmp = WEP_ENABLED;
					break;
				case WPA_CIPHER_TKIP:
					tmp = TKIP_ENABLED;
					break;
				case WPA_CIPHER_AES_CCM:
					tmp = AES_ENABLED;
					break;
				default:
					WL_ERR(("No Security Info\n"));
			}
			gval |= tmp;
		}
	}
	/* Check for unicast suite(s) */
	if (len < WPA_IE_SUITE_COUNT_LEN) {
		WL_INFORM(("no unicast suite\n"));
		goto exit;
	}
	/* walk thru unicast cipher list and pick up what we recognize */
	ucast = (wpa_suite_ucast_t *)&mcast[1];
	count = ltoh16_ua(&ucast->count);
	len -= WPA_IE_SUITE_COUNT_LEN;
	for (i = 0; i < count && len >= WPA_SUITE_LEN;
		i++, len -= WPA_SUITE_LEN) {
		if (!bcmp(ucast->list[i].oui, WPA_OUI, WPA_OUI_LEN)) {
			if (IS_WPA_CIPHER(ucast->list[i].type)) {
				tmp = 0;
				switch (ucast->list[i].type) {
					case WPA_CIPHER_NONE:
						tmp = 0;
						break;
					case WPA_CIPHER_WEP_40:
					case WPA_CIPHER_WEP_104:
						tmp = WEP_ENABLED;
						break;
					case WPA_CIPHER_TKIP:
						tmp = TKIP_ENABLED;
						break;
					case WPA_CIPHER_AES_CCM:
						tmp = AES_ENABLED;
						break;
					default:
						WL_ERR(("No Security Info\n"));
				}
				pval |= tmp;
			}
		}
	}
	len -= (count - i) * WPA_SUITE_LEN;
	/* Check for auth key management suite(s) */
	if (len < WPA_IE_SUITE_COUNT_LEN) {
		WL_INFORM((" no auth key mgmt suite\n"));
		goto exit;
	}
	/* walk thru auth management suite list and pick up what we recognize */
	mgmt = (wpa_suite_auth_key_mgmt_t *)&ucast->list[count];
	count = ltoh16_ua(&mgmt->count);
	len -= WPA_IE_SUITE_COUNT_LEN;
	for (i = 0; i < count && len >= WPA_SUITE_LEN;
		i++, len -= WPA_SUITE_LEN) {
		if (!bcmp(mgmt->list[i].oui, WPA_OUI, WPA_OUI_LEN)) {
			if (IS_WPA_AKM(mgmt->list[i].type)) {
				tmp = 0;
				switch (mgmt->list[i].type) {
					case RSN_AKM_NONE:
						tmp = WPA_AUTH_NONE;
						break;
					case RSN_AKM_UNSPECIFIED:
						tmp = WPA_AUTH_UNSPECIFIED;
						break;
					case RSN_AKM_PSK:
						tmp = WPA_AUTH_PSK;
						break;
					default:
						WL_ERR(("No Key Mgmt Info\n"));
				}
				wpa_auth |= tmp;
			}
		}

	}
	/* FOR WPS , set SEC_OW_ENABLED */
	wsec = (pval | gval | SES_OW_ENABLED);
	/* set auth */
	err = wldev_iovar_setint_bsscfg(dev, "auth", auth, bssidx);
	if (err < 0) {
		WL_ERR(("auth error %d\n", err));
		return BCME_ERROR;
	}
	/* set wsec */
	err = wldev_iovar_setint_bsscfg(dev, "wsec", wsec, bssidx);
	if (err < 0) {
		WL_ERR(("wsec error %d\n", err));
		return BCME_ERROR;
	}
	/* set upper-layer auth */
	err = wldev_iovar_setint_bsscfg(dev, "wpa_auth", wpa_auth, bssidx);
	if (err < 0) {
		WL_ERR(("wpa_auth error %d\n", err));
		return BCME_ERROR;
	}
exit:
	return 0;
}

#if defined(SUPPORT_SOFTAP_WPAWPA2_MIXED)
static u32 wl_get_cipher_type(uint8 type)
{
	u32 ret = 0;
	switch (type) {
		case WPA_CIPHER_NONE:
			ret = 0;
			break;
		case WPA_CIPHER_WEP_40:
		case WPA_CIPHER_WEP_104:
			ret = WEP_ENABLED;
			break;
		case WPA_CIPHER_TKIP:
			ret = TKIP_ENABLED;
			break;
		case WPA_CIPHER_AES_CCM:
			ret = AES_ENABLED;
			break;
#ifdef BCMWAPI_WPI
		case WAPI_CIPHER_SMS4:
			ret = SMS4_ENABLED;
			break;
#endif
		default:
			WL_ERR(("No Security Info\n"));
	}
	return ret;
}
#ifndef  BRCM_RSDB
static u32 wl_get_suite_auth_key_mgmt_type(uint8 type)
{
	u32 ret = 0;
	switch (type) {
		case RSN_AKM_NONE:
			ret = WPA_AUTH_NONE;
			break;
		case RSN_AKM_UNSPECIFIED:
			ret = WPA_AUTH_UNSPECIFIED;
			break;
		case RSN_AKM_PSK:
			ret = WPA_AUTH_PSK;
			break;
		default:
			WL_ERR(("No Key Mgmt Info\n"));
	}
	return ret;
}
#else
static u32 wl_get_suite_auth_key_mgmt_type(uint8 type, wpa_suite_mcast_t *mcast)
{
	u32 ret = 0;
	u32 is_wpa2 = 0;

	if (!bcmp(mcast->oui, WPA2_OUI, WPA2_OUI_LEN)) {
		is_wpa2 = 1;
	}

	WL_INFORM(("%s, type = %d\n", is_wpa2 ? "WPA2":"WPA", type));
	switch (type) {
		case RSN_AKM_NONE:
			/* For WPA and WPA2, AUTH_NONE is common */
			ret = WPA_AUTH_NONE;
			break;
		case RSN_AKM_UNSPECIFIED:
			if (is_wpa2) {
				ret = WPA2_AUTH_UNSPECIFIED;
			} else {
				ret = WPA_AUTH_UNSPECIFIED;
			}
			break;
		case RSN_AKM_PSK:
			if (is_wpa2) {
				ret = WPA2_AUTH_PSK;
			} else {
				ret = WPA_AUTH_PSK;
			}
			break;
		default:
			WL_ERR(("No Key Mgmt Info\n"));
	}

	return ret;
}
#endif  /* BRCM_RSDB */

static s32
wl_validate_wpaie_wpa2ie(struct net_device *dev, wpa_ie_fixed_t *wpaie,
	bcm_tlv_t *wpa2ie, s32 bssidx)
{
	wpa_suite_mcast_t *mcast;
	wpa_suite_ucast_t *ucast;
	wpa_suite_auth_key_mgmt_t *mgmt;
	u16 auth = 0; /* d11 open authentication */
	u16 count;
	s32 err = BCME_OK;
	u32 wme_bss_disable;
	u16 suite_count;
	u8 rsn_cap[2];
	s32 len = 0;
	u32 i;
	u32 wsec1, wsec2, wsec;
	u32 pval = 0;
	u32 gval = 0;
	u32 wpa_auth = 0;
	u32 wpa_auth1 = 0;
	u32 wpa_auth2 = 0;
	u8* ptmp;

	if (wpaie == NULL || wpa2ie == NULL)
		goto exit;

	WL_DBG(("Enter \n"));
	len = wpaie->length;    /* value length */
	len -= WPA_IE_TAG_FIXED_LEN;
	/* check for multicast cipher suite */
	if (len < WPA_SUITE_LEN) {
		WL_INFORM(("no multicast cipher suite\n"));
		goto exit;
	}

	/* pick up multicast cipher */
	mcast = (wpa_suite_mcast_t *)&wpaie[1];
	len -= WPA_SUITE_LEN;
	if (!bcmp(mcast->oui, WPA_OUI, WPA_OUI_LEN)) {
		if (IS_WPA_CIPHER(mcast->type)) {
			gval |= wl_get_cipher_type(mcast->type);
		}
	}
	WL_ERR(("\nwpa ie validate\n"));
	WL_ERR(("wpa ie mcast cipher = 0x%X\n", gval));

	/* Check for unicast suite(s) */
	if (len < WPA_IE_SUITE_COUNT_LEN) {
		WL_INFORM(("no unicast suite\n"));
		goto exit;
	}

	/* walk thru unicast cipher list and pick up what we recognize */
	ucast = (wpa_suite_ucast_t *)&mcast[1];
	count = ltoh16_ua(&ucast->count);
	len -= WPA_IE_SUITE_COUNT_LEN;
	for (i = 0; i < count && len >= WPA_SUITE_LEN;
		i++, len -= WPA_SUITE_LEN) {
		if (!bcmp(ucast->list[i].oui, WPA_OUI, WPA_OUI_LEN)) {
			if (IS_WPA_CIPHER(ucast->list[i].type)) {
				pval |= wl_get_cipher_type(ucast->list[i].type);
			}
		}
	}
	WL_ERR(("wpa ie ucast count =%d, cipher = 0x%X\n", count, pval));

	/* FOR WPS , set SEC_OW_ENABLED */
	wsec1 = (pval | gval | SES_OW_ENABLED);
	WL_ERR(("wpa ie wsec = 0x%X\n", wsec1));

	len -= (count - i) * WPA_SUITE_LEN;
	/* Check for auth key management suite(s) */
	if (len < WPA_IE_SUITE_COUNT_LEN) {
		WL_INFORM((" no auth key mgmt suite\n"));
		goto exit;
	}
	/* walk thru auth management suite list and pick up what we recognize */
	mgmt = (wpa_suite_auth_key_mgmt_t *)&ucast->list[count];
	count = ltoh16_ua(&mgmt->count);
	len -= WPA_IE_SUITE_COUNT_LEN;
	for (i = 0; i < count && len >= WPA_SUITE_LEN;
		i++, len -= WPA_SUITE_LEN) {
		if (!bcmp(mgmt->list[i].oui, WPA_OUI, WPA_OUI_LEN)) {
			if (IS_WPA_AKM(mgmt->list[i].type)) {
#ifndef  BRCM_RSDB
				wpa_auth1 |= wl_get_suite_auth_key_mgmt_type(mgmt->list[i].type);
#else
				wpa_auth1 |= wl_get_suite_auth_key_mgmt_type(mgmt->list[i].type, mcast);
#endif
			}
		}

	}
	WL_ERR(("wpa ie wpa_suite_auth_key_mgmt count=%d, key_mgmt = 0x%X\n", count, wpa_auth1));
	WL_ERR(("\nwpa2 ie validate\n"));

	pval = 0;
	gval = 0;
	len =  wpa2ie->len;
	/* check the mcast cipher */
	mcast = (wpa_suite_mcast_t *)&wpa2ie->data[WPA2_VERSION_LEN];
	ptmp = mcast->oui;
	gval = wl_get_cipher_type(ptmp[DOT11_OUI_LEN]);

	WL_ERR(("wpa2 ie mcast cipher = 0x%X\n", gval));
	if ((len -= WPA_SUITE_LEN) <= 0)
	{
		WL_ERR(("P:wpa2 ie len[%d]", len));
		return BCME_BADLEN;
	}

	/* check the unicast cipher */
	ucast = (wpa_suite_ucast_t *)&mcast[1];
	suite_count = ltoh16_ua(&ucast->count);
	WL_ERR((" WPA2 ucast cipher count=%d\n", suite_count));
	pval |= wl_get_cipher_type(ucast->list[0].type);

	if ((len -= (WPA_IE_SUITE_COUNT_LEN + (WPA_SUITE_LEN * suite_count))) <= 0)
		return BCME_BADLEN;

	WL_ERR(("wpa2 ie ucast cipher = 0x%X\n", pval));

	/* FOR WPS , set SEC_OW_ENABLED */
	wsec2 = (pval | gval | SES_OW_ENABLED);
	WL_ERR(("wpa2 ie wsec = 0x%X\n", wsec2));

	/* check the AKM */
	mgmt = (wpa_suite_auth_key_mgmt_t *)&ucast->list[suite_count];
	suite_count = ltoh16_ua(&mgmt->count);
	ptmp = (u8 *)&mgmt->list[0];
#ifndef  BRCM_RSDB
	wpa_auth2 = wl_get_suite_auth_key_mgmt_type(ptmp[DOT11_OUI_LEN]);
#else
	wpa_auth2 = wl_get_suite_auth_key_mgmt_type(ptmp[DOT11_OUI_LEN], mcast);
#endif
	WL_ERR(("wpa ie wpa_suite_auth_key_mgmt count=%d, key_mgmt = 0x%X\n", count, wpa_auth2));

	if ((len -= (WPA_IE_SUITE_COUNT_LEN + (WPA_SUITE_LEN * suite_count))) >= RSN_CAP_LEN) {
		rsn_cap[0] = *(u8 *)&mgmt->list[suite_count];
		rsn_cap[1] = *((u8 *)&mgmt->list[suite_count] + 1);
		if (rsn_cap[0] & (RSN_CAP_16_REPLAY_CNTRS << RSN_CAP_PTK_REPLAY_CNTR_SHIFT)) {
			wme_bss_disable = 0;
		} else {
			wme_bss_disable = 1;
		}
		WL_DBG(("P:rsn_cap[0]=[0x%X]:wme_bss_disabled[%d]\n", rsn_cap[0], wme_bss_disable));

		/* set wme_bss_disable to sync RSN Capabilities */
		err = wldev_iovar_setint_bsscfg(dev, "wme_bss_disable", wme_bss_disable, bssidx);
		if (err < 0) {
			WL_ERR(("wme_bss_disable error %d\n", err));
			return BCME_ERROR;
		}
	} else {
		WL_DBG(("There is no RSN Capabilities. remained len %d\n", len));
	}

	wsec = (wsec1 | wsec2);
	wpa_auth = (wpa_auth1 | wpa_auth2);
	WL_ERR(("wpa_wpa2 wsec=0x%X wpa_auth=0x%X\n", wsec, wpa_auth));

	/* set auth */
	err = wldev_iovar_setint_bsscfg(dev, "auth", auth, bssidx);
	if (err < 0) {
		WL_ERR(("auth error %d\n", err));
		return BCME_ERROR;
	}
	/* set wsec */
	err = wldev_iovar_setint_bsscfg(dev, "wsec", wsec, bssidx);
	if (err < 0) {
		WL_ERR(("wsec error %d\n", err));
		return BCME_ERROR;
	}
	/* set upper-layer auth */
	err = wldev_iovar_setint_bsscfg(dev, "wpa_auth", wpa_auth, bssidx);
	if (err < 0) {
		WL_ERR(("wpa_auth error %d\n", err));
		return BCME_ERROR;
	}
exit:
	return 0;
}
#endif /* SUPPORT_SOFTAP_WPAWPA2_MIXED */
#ifndef  BRCM_RSDB
static s32
wl_cfg80211_bcn_validate_sec(
	struct net_device *dev,
	struct parsed_ies *ies,
	u32 dev_role,
	s32 bssidx)
{
	struct bcm_cfg80211 *cfg = g_bcm_cfg;

	if (dev_role == NL80211_IFTYPE_P2P_GO && (ies->wpa2_ie)) {
		/* For P2P GO, the sec type is WPA2-PSK */
		WL_DBG(("P2P GO: validating wpa2_ie"));
		if (wl_validate_wpa2ie(dev, ies->wpa2_ie, bssidx)  < 0)
			return BCME_ERROR;

	} else if (dev_role == NL80211_IFTYPE_AP) {

		WL_DBG(("SoftAP: validating security"));
		/* If wpa2_ie or wpa_ie is present validate it */

#if defined(SUPPORT_SOFTAP_WPAWPA2_MIXED)
		if ((ies->wpa_ie != NULL && ies->wpa2_ie != NULL)) {
			if (wl_validate_wpaie_wpa2ie(dev, ies->wpa_ie, ies->wpa2_ie, bssidx)  < 0) {
				cfg->ap_info->security_mode = false;
				return BCME_ERROR;
			}
		}
		else {
#endif /* SUPPORT_SOFTAP_WPAWPA2_MIXED */
		if ((ies->wpa2_ie || ies->wpa_ie) &&
			((wl_validate_wpa2ie(dev, ies->wpa2_ie, bssidx)  < 0 ||
			wl_validate_wpaie(dev, ies->wpa_ie, bssidx) < 0))) {
			cfg->ap_info->security_mode = false;
			return BCME_ERROR;
		}

		cfg->ap_info->security_mode = true;
		if (cfg->ap_info->rsn_ie) {
			kfree(cfg->ap_info->rsn_ie);
			cfg->ap_info->rsn_ie = NULL;
		}
		if (cfg->ap_info->wpa_ie) {
			kfree(cfg->ap_info->wpa_ie);
			cfg->ap_info->wpa_ie = NULL;
		}
		if (cfg->ap_info->wps_ie) {
			kfree(cfg->ap_info->wps_ie);
			cfg->ap_info->wps_ie = NULL;
		}
		if (ies->wpa_ie != NULL) {
			/* WPAIE */
			cfg->ap_info->rsn_ie = NULL;
			cfg->ap_info->wpa_ie = kmemdup(ies->wpa_ie,
				ies->wpa_ie->length + WPA_RSN_IE_TAG_FIXED_LEN,
				GFP_KERNEL);
		} else if (ies->wpa2_ie != NULL) {
			/* RSNIE */
			cfg->ap_info->wpa_ie = NULL;
			cfg->ap_info->rsn_ie = kmemdup(ies->wpa2_ie,
				ies->wpa2_ie->len + WPA_RSN_IE_TAG_FIXED_LEN,
				GFP_KERNEL);
		}
#if defined(SUPPORT_SOFTAP_WPAWPA2_MIXED)
		}
#endif /* SUPPORT_SOFTAP_WPAWPA2_MIXED */
		if (!ies->wpa2_ie && !ies->wpa_ie) {
			wl_validate_opensecurity(dev, bssidx);
			cfg->ap_info->security_mode = false;
		}

		if (ies->wps_ie) {
			cfg->ap_info->wps_ie = kmemdup(ies->wps_ie, ies->wps_ie_len, GFP_KERNEL);
		}
	}

	return 0;

}
#else
static s32
wl_cfg80211_bcn_validate_sec(
	struct net_device *dev,
	struct parsed_ies *ies,
	u32 dev_role,
	s32 bssidx,
	bool privacy)
{
	struct bcm_cfg80211 *cfg = g_bcm_cfg;
	wl_cfgbss_t *bss = wl_get_cfgbss_by_wdev(cfg, dev->ieee80211_ptr);

	if (!bss) {
		WL_ERR(("cfgbss is NULL \n"));
		return BCME_ERROR;
	}

	if (dev_role == NL80211_IFTYPE_P2P_GO && (ies->wpa2_ie)) {
		/* For P2P GO, the sec type is WPA2-PSK */
		WL_DBG(("P2P GO: validating wpa2_ie"));
		if (wl_validate_wpa2ie(dev, ies->wpa2_ie, bssidx)  < 0)
			return BCME_ERROR;

	} else if (dev_role == NL80211_IFTYPE_AP) {

		WL_DBG(("SoftAP: validating security"));
		/* If wpa2_ie or wpa_ie is present validate it */

#if defined(SUPPORT_SOFTAP_WPAWPA2_MIXED)
		if ((ies->wpa_ie != NULL && ies->wpa2_ie != NULL)) {
			if (wl_validate_wpaie_wpa2ie(dev, ies->wpa_ie, ies->wpa2_ie, bssidx)  < 0) {
				bss->security_mode = false;
				return BCME_ERROR;
			}
		}
		else {
#endif /* SUPPORT_SOFTAP_WPAWPA2_MIXED */
		if ((ies->wpa2_ie || ies->wpa_ie) &&
			((wl_validate_wpa2ie(dev, ies->wpa2_ie, bssidx)  < 0 ||
			wl_validate_wpaie(dev, ies->wpa_ie, bssidx) < 0))) {
			bss->security_mode = false;
			return BCME_ERROR;
		}

		bss->security_mode = true;
		if (bss->rsn_ie) {
			kfree(bss->rsn_ie);
			bss->rsn_ie = NULL;
		}
		if (bss->wpa_ie) {
			kfree(bss->wpa_ie);
			bss->wpa_ie = NULL;
		}
		if (bss->wps_ie) {
			kfree(bss->wps_ie);
			bss->wps_ie = NULL;
		}
		if (ies->wpa_ie != NULL) {
			/* WPAIE */
			bss->rsn_ie = NULL;
			bss->wpa_ie = kmemdup(ies->wpa_ie,
				ies->wpa_ie->length + WPA_RSN_IE_TAG_FIXED_LEN,
				GFP_KERNEL);
		} else if (ies->wpa2_ie != NULL) {
			/* RSNIE */
			bss->wpa_ie = NULL;
			bss->rsn_ie = kmemdup(ies->wpa2_ie,
				ies->wpa2_ie->len + WPA_RSN_IE_TAG_FIXED_LEN,
				GFP_KERNEL);
		}
#if defined(SUPPORT_SOFTAP_WPAWPA2_MIXED)
		}
#endif /* SUPPORT_SOFTAP_WPAWPA2_MIXED */
		if (!ies->wpa2_ie && !ies->wpa_ie) {
			wl_validate_opensecurity(dev, bssidx, privacy);
			bss->security_mode = false;
		}

		if (ies->wps_ie) {
			bss->wps_ie = kmemdup(ies->wps_ie, ies->wps_ie_len, GFP_KERNEL);
		}
	}

	return 0;

}
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 4, 0)) || defined(WL_COMPAT_WIRELESS)
static s32 wl_cfg80211_bcn_set_params(
	struct cfg80211_ap_settings *info,
	struct net_device *dev,
	u32 dev_role, s32 bssidx)
{
	struct bcm_cfg80211 *cfg = g_bcm_cfg;
	s32 err = BCME_OK;

	WL_DBG(("interval (%d) \ndtim_period (%d) \n",
		info->beacon_interval, info->dtim_period));

	if (info->beacon_interval) {
		if ((err = wldev_ioctl(dev, WLC_SET_BCNPRD,
			&info->beacon_interval, sizeof(s32), true)) < 0) {
			WL_ERR(("Beacon Interval Set Error, %d\n", err));
			return err;
		}
	}

	if (info->dtim_period) {
		if ((err = wldev_ioctl(dev, WLC_SET_DTIMPRD,
			&info->dtim_period, sizeof(s32), true)) < 0) {
			WL_ERR(("DTIM Interval Set Error, %d\n", err));
			return err;
		}
	}

	if ((info->ssid) && (info->ssid_len > 0) &&
#ifdef BCM_PATCH_CVE_2016_2493
		(info->ssid_len <= DOT11_MAX_SSID_LEN)) {
#else
		(info->ssid_len <= 32)) {
#endif
		WL_DBG(("SSID (%s) len:%zd \n", info->ssid, info->ssid_len));
		if (dev_role == NL80211_IFTYPE_AP) {
			/* Store the hostapd SSID */
#ifdef BCM_PATCH_CVE_2016_2493
			memset(cfg->hostapd_ssid.SSID, 0x00, DOT11_MAX_SSID_LEN);
#else
			memset(cfg->hostapd_ssid.SSID, 0x00, 32);
#endif
			memcpy(cfg->hostapd_ssid.SSID, info->ssid, info->ssid_len);
			cfg->hostapd_ssid.SSID_len = info->ssid_len;
		} else {
				/* P2P GO */
#ifdef BCM_PATCH_CVE_2016_2493
			memset(cfg->p2p->ssid.SSID, 0x00, DOT11_MAX_SSID_LEN);
#else
			memset(cfg->p2p->ssid.SSID, 0x00, 32);
#endif
			memcpy(cfg->p2p->ssid.SSID, info->ssid, info->ssid_len);
			cfg->p2p->ssid.SSID_len = info->ssid_len;
		}
	}

	if (info->hidden_ssid) {
		if ((err = wldev_iovar_setint(dev, "closednet", 1)) < 0)
			WL_ERR(("failed to set hidden : %d\n", err));
		WL_DBG(("hidden_ssid_enum_val: %d \n", info->hidden_ssid));
	}

	return err;
}
#endif

static s32
wl_cfg80211_parse_ies(u8 *ptr, u32 len, struct parsed_ies *ies)
{
	s32 err = BCME_OK;

	memset(ies, 0, sizeof(struct parsed_ies));

	/* find the WPSIE */
	if ((ies->wps_ie = wl_cfgp2p_find_wpsie(ptr, len)) != NULL) {
		WL_DBG(("WPSIE in beacon \n"));
		ies->wps_ie_len = ies->wps_ie->length + WPA_RSN_IE_TAG_FIXED_LEN;
	} else {
		WL_ERR(("No WPSIE in beacon \n"));
	}

	/* find the RSN_IE */
	if ((ies->wpa2_ie = bcm_parse_tlvs(ptr, len,
		DOT11_MNG_RSN_ID)) != NULL) {
		WL_DBG((" WPA2 IE found\n"));
		ies->wpa2_ie_len = ies->wpa2_ie->len;
	}

	/* find the WPA_IE */
	if ((ies->wpa_ie = wl_cfgp2p_find_wpaie(ptr, len)) != NULL) {
		WL_DBG((" WPA found\n"));
		ies->wpa_ie_len = ies->wpa_ie->length;
	}

	return err;

}

#define MAX_AP_LINK_WAIT_TIME   10000
static s32
wl_cfg80211_bcn_bringup_ap(
	struct net_device *dev,
	struct parsed_ies *ies,
	u32 dev_role, s32 bssidx)
{
	struct bcm_cfg80211 *cfg = g_bcm_cfg;
	struct wl_join_params join_params;
#ifdef  BRCM_RSDB
	struct wiphy *wiphy;
#endif
	bool is_bssup = false;
	s32 infra = 1;
	s32 join_params_size = 0;
	s32 ap = 1;
#ifdef  BRCM_RSDB
	s32 pm;
	s32 wsec;
#endif
#ifdef DISABLE_11H_SOFTAP
	s32 spect = 0;
#endif /* DISABLE_11H_SOFTAP */
#ifdef MAX_GO_CLIENT_CNT
	s32 bss_maxassoc = MAX_GO_CLIENT_CNT;
#endif
	s32 err = BCME_OK;
#ifndef  BRCM_RSDB

	WL_DBG(("Enter dev_role: %d\n", dev_role));

	/* Common code for SoftAP and P2P GO */
#else
#ifdef SOFTAP_UAPSD_OFF
	uint32 wme_apsd = 0;
#endif /* SOFTAP_UAPSD_OFF */
	s32 is_rsdb_supported = BCME_ERROR;
	u32 timeout;
#if defined(DHD_DEBUG) && defined(BCMPCIE) && defined(DHD_FW_COREDUMP)
	dhd_pub_t *dhdp = (dhd_pub_t *)(cfg->pub);
#endif /* DHD_DEBUG && BCMPCIE && DHD_FW_COREDUMP */

	is_rsdb_supported = DHD_OPMODE_SUPPORTED(cfg->pub, DHD_FLAG_RSDB_MODE);
	if (is_rsdb_supported < 0)
		return (-ENODEV);

	WL_DBG(("Enter dev_role:%d bssidx:%d\n", dev_role, bssidx));

	/* Common code for SoftAP and P2P GO */
	wiphy = bcmcfg_to_wiphy(cfg);
	if (wl_check_dongle_idle(wiphy) != TRUE) {
		WL_ERR(("FW is busy to add interface"));
		return -EINVAL;
	}
#endif /*BRCM_RSDB*/
	wldev_iovar_setint(dev, "mpc", 0);
#ifdef  BRCM_RSDB
	wl_clr_drv_status(cfg, AP_CREATED, dev);
#endif
	if (dev_role == NL80211_IFTYPE_P2P_GO) {
		is_bssup = wl_cfgp2p_bss_isup(dev, bssidx);
		if (!is_bssup && (ies->wpa2_ie != NULL)) {

			err = wldev_ioctl(dev, WLC_SET_INFRA, &infra, sizeof(s32), true);
			if (err < 0) {
				WL_ERR(("SET INFRA error %d\n", err));
				goto exit;
			}

			err = wldev_iovar_setbuf_bsscfg(dev, "ssid", &cfg->p2p->ssid,
				sizeof(cfg->p2p->ssid), cfg->ioctl_buf, WLC_IOCTL_MAXLEN,
				bssidx, &cfg->ioctl_buf_sync);
			if (err < 0) {
				WL_ERR(("GO SSID setting error %d\n", err));
				goto exit;
			}

			/* Do abort scan before creating GO */
			wl_cfg80211_scan_abort(cfg);

			if ((err = wl_cfgp2p_bss(cfg, dev, bssidx, 1)) < 0) {
				WL_ERR(("GO Bring up error %d\n", err));
				goto exit;
			}
#ifdef MAX_GO_CLIENT_CNT
			err = wldev_iovar_setint_bsscfg(dev, "bss_maxassoc", bss_maxassoc, bssidx);
			if (unlikely(err)) {
				WL_ERR(("bss_maxassoc error (%d)\n", err));
				goto exit;
			}
#endif
		} else
			WL_DBG(("Bss is already up\n"));
	} else if ((dev_role == NL80211_IFTYPE_AP) &&
		(wl_get_drv_status(cfg, AP_CREATING, dev))) {
		/* Device role SoftAP */
#ifdef  BRCM_RSDB
		WL_DBG(("Creating AP bssidx:%d dev_role:%d\n", bssidx, dev_role));

		/* Clear the status bit after use */
		wl_clr_drv_status(cfg, AP_CREATING, dev);

		/* AP on primary Interface */
		if (bssidx == 0) {
			if (is_rsdb_supported) {
				if ((err = wl_cfg80211_add_del_bss(cfg, dev, bssidx,
					NL80211_IFTYPE_AP, 0, NULL)) < 0) {
					WL_ERR(("wl add_del_bss returned error:%d\n", err));
					goto exit;
				}
			} else if (is_rsdb_supported == 0) {
#endif
			/* AP mode switch not supported. Try setting up AP explicitly */
				err = wldev_ioctl(dev, WLC_DOWN, &ap, sizeof(s32), true);
				if (err < 0) {
					WL_ERR(("WLC_DOWN error %d\n", err));
					goto exit;
				}
#ifdef  BRCM_RSDB
				err = wldev_iovar_setint(dev, "apsta", 0);
				if (err < 0) {
					WL_ERR(("wl apsta 0 error %d\n", err));
					goto exit;
				}

				if ((err = wldev_ioctl(dev,
					WLC_SET_AP, &ap, sizeof(s32), true)) < 0) {
					WL_ERR(("setting AP mode failed %d \n", err));
					goto exit;
				}

			}

			pm = 0;
			if ((err = wldev_ioctl(dev, WLC_SET_PM, &pm, sizeof(pm), true)) != 0) {
				WL_ERR(("wl PM 0 returned error:%d\n", err));
				goto exit;
			}
#endif
			err = wldev_ioctl(dev, WLC_SET_INFRA, &infra, sizeof(s32), true);
			if (err < 0) {
				WL_ERR(("SET INFRA error %d\n", err));
				goto exit;
			}
#ifdef  BRCM_RSDB
		} else if (cfg->cfgdev_bssidx && (bssidx == cfg->cfgdev_bssidx)) {

			WL_DBG(("Bringup SoftAP on virtual Interface bssidx:%d \n", bssidx));

			if ((err = wl_cfg80211_add_del_bss(cfg, dev,
				bssidx, NL80211_IFTYPE_AP, 0, NULL)) < 0) {
				WL_ERR(("wl bss ap returned error:%d\n", err));
				goto exit;
			}
		}
#else
	if ((err = wldev_ioctl(dev, WLC_SET_AP, &ap, sizeof(s32), true)) < 0) {
			WL_ERR(("setting AP mode failed %d \n", err));
			goto exit;
		}
#endif
#ifdef DISABLE_11H_SOFTAP
		err = wldev_ioctl(dev, WLC_SET_SPECT_MANAGMENT, &spect, sizeof(s32), true);
		if (err < 0) {
			WL_ERR(("SET SPECT_MANAGMENT error %d\n", err));
			goto exit;
		}
#endif /* DISABLE_11H_SOFTAP */
#ifdef  BRCM_RSDB
#ifdef SOFTAP_UAPSD_OFF
		err = wldev_iovar_setbuf_bsscfg(dev, "wme_apsd", &wme_apsd, sizeof(wme_apsd),
			cfg->ioctl_buf, WLC_IOCTL_SMLEN, bssidx, &cfg->ioctl_buf_sync);
		if (err < 0) {
			WL_ERR(("failed to disable uapsd, error=%d\n", err));
		}
#endif /* SOFTAP_UAPSD_OFF */
#endif /* BRCM_RSDB */
		err = wldev_ioctl(dev, WLC_UP, &ap, sizeof(s32), true);
		if (unlikely(err)) {
			WL_ERR(("WLC_UP error (%d)\n", err));
			goto exit;
		}
#ifdef  BRCM_RSDB
#ifdef MFP
		if (cfg->bip_pos) {
			err = wldev_iovar_setbuf_bsscfg(dev, "bip",
				(void *)(cfg->bip_pos), WPA_SUITE_LEN, cfg->ioctl_buf,
				WLC_IOCTL_SMLEN, bssidx, &cfg->ioctl_buf_sync);
			if (err < 0) {
				WL_ERR(("bip set error %d\n", err));
				goto exit;
			}
		}
#endif /* MFP */

		err = wldev_iovar_getint(dev, "wsec", (s32 *)&wsec);
		if (unlikely(err)) {
			WL_ERR(("Could not get wsec %d\n", err));
			goto exit;
		}
		if ((wsec == WEP_ENABLED) && cfg->wep_key.len) {
			WL_DBG(("Applying buffered WEP KEY \n"));
			err = wldev_iovar_setbuf_bsscfg(dev, "wsec_key", &cfg->wep_key,
				sizeof(struct wl_wsec_key), cfg->ioctl_buf,
				WLC_IOCTL_MAXLEN, bssidx, &cfg->ioctl_buf_sync);
			/* clear the key after use */
			memset(&cfg->wep_key, 0, sizeof(struct wl_wsec_key));
			if (unlikely(err)) {
				WL_ERR(("WLC_SET_KEY error (%d)\n", err));
				goto exit;
			}
		}

#ifdef MFP
		if (cfg->mfp_mode) {
			/* This needs to go after wsec otherwise the wsec command will
			 * overwrite the values set by MFP
			 */
			err = wldev_iovar_setint_bsscfg(dev, "mfp", cfg->mfp_mode, bssidx);
			if (err < 0) {
				WL_ERR(("MFP Setting failed. ret = %d \n", err));
				goto exit;
			}
		}
#endif /* MFP */
#endif /* BRCM_RSDB */
		memset(&join_params, 0, sizeof(join_params));
		/* join parameters starts with ssid */
		join_params_size = sizeof(join_params.ssid);
#ifdef BCM_PATCH_CVE_2016_2493
		join_params.ssid.SSID_len = MIN(cfg->hostapd_ssid.SSID_len,
			(uint32)DOT11_MAX_SSID_LEN);
#endif
		memcpy(join_params.ssid.SSID, cfg->hostapd_ssid.SSID,
#ifdef BCM_PATCH_CVE_2016_2493
			join_params.ssid.SSID_len);
		join_params.ssid.SSID_len = htod32(join_params.ssid.SSID_len);
#else
			cfg->hostapd_ssid.SSID_len);
		join_params.ssid.SSID_len = htod32(cfg->hostapd_ssid.SSID_len);
#endif
#ifndef  BRCM_RSDB
		/* create softap */
		if ((err = wldev_ioctl(dev, WLC_SET_SSID, &join_params,
			join_params_size, true)) == 0) {
			WL_DBG(("SoftAP set SSID (%s) success\n", join_params.ssid.SSID));
			wl_clr_drv_status(cfg, AP_CREATING, dev);
			wl_set_drv_status(cfg, AP_CREATED, dev);
		}
	}
#else
		/* create softap */
		if ((err = wldev_ioctl(dev, WLC_SET_SSID, &join_params,
			join_params_size, true)) != 0) {
			WL_ERR(("SoftAP/GO set ssid failed! \n"));
			goto exit;
		} else {
			WL_DBG((" SoftAP SSID \"%s\" \n", join_params.ssid.SSID));
		}

		if (bssidx != 0) {
			/* AP on Virtual Interface */
			if ((err = wl_cfgp2p_bss(cfg, dev, bssidx, 1)) < 0) {
				WL_ERR(("GO Bring up error %d\n", err));
				goto exit;
			}
		}

	}
	/* Wait for Linkup event to mark successful AP/GO bring up */
	timeout = wait_event_interruptible_timeout(cfg->netif_change_event,
		wl_get_drv_status(cfg, AP_CREATED, dev), msecs_to_jiffies(MAX_AP_LINK_WAIT_TIME));
	if (timeout <= 0 || !wl_get_drv_status(cfg, AP_CREATED, dev)) {
		WL_ERR(("Link up didn't come for AP interface. AP/GO creation failed! \n"));
#if defined(DHD_DEBUG) && defined(BCMPCIE) && defined(DHD_FW_COREDUMP)
		if (dhdp->memdump_enabled) {
			dhdp->memdump_type = DUMP_TYPE_AP_LINKUP_FAILURE;
			dhd_bus_mem_dump(dhdp);
		}
#endif /* DHD_DEBUG && BCMPCIE && DHD_FW_COREDUMP */
		err = -ENODEV;
		goto exit;
	}
#endif

exit:
#ifdef  BRCM_RSDB
	if (cfg->wep_key.len) {
		memset(&cfg->wep_key, 0, sizeof(struct wl_wsec_key));
	}

#ifdef MFP
	if (cfg->mfp_mode) {
		cfg->mfp_mode = 0;
	}

	if (cfg->bip_pos) {
		cfg->bip_pos = NULL;
	}
#endif /* MFP */
#endif /* BRCM_RSDB */
	return err;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 4, 0)) || defined(WL_COMPAT_WIRELESS)
s32
wl_cfg80211_parse_ap_ies(
	struct net_device *dev,
	struct cfg80211_beacon_data *info,
	struct parsed_ies *ies)
{
	struct parsed_ies prb_ies;
	struct bcm_cfg80211 *cfg = g_bcm_cfg;
	dhd_pub_t *dhd = (dhd_pub_t *)(cfg->pub);
	u8 *vndr = NULL;
	u32 vndr_ie_len = 0;
	s32 err = BCME_OK;

	/* Parse Beacon IEs */
	if (wl_cfg80211_parse_ies((u8 *)info->tail,
		info->tail_len, ies) < 0) {
		WL_ERR(("Beacon get IEs failed \n"));
		err = -EINVAL;
		goto fail;
	}

	vndr = (u8 *)info->proberesp_ies;
	vndr_ie_len = info->proberesp_ies_len;

	if (dhd->op_mode & DHD_FLAG_HOSTAP_MODE) {
		/* SoftAP mode */
		struct ieee80211_mgmt *mgmt;
		mgmt = (struct ieee80211_mgmt *)info->probe_resp;
		if (mgmt != NULL) {
			vndr = (u8 *)&mgmt->u.probe_resp.variable;
			vndr_ie_len = info->probe_resp_len -
				offsetof(struct ieee80211_mgmt, u.probe_resp.variable);
		}
	}

	/* Parse Probe Response IEs */
	if (wl_cfg80211_parse_ies(vndr, vndr_ie_len, &prb_ies) < 0) {
		WL_ERR(("PROBE RESP get IEs failed \n"));
		err = -EINVAL;
	}

fail:

	return err;
}

s32
wl_cfg80211_set_ies(
	struct net_device *dev,
	struct cfg80211_beacon_data *info,
	s32 bssidx)
{
	struct bcm_cfg80211 *cfg = g_bcm_cfg;
	dhd_pub_t *dhd = (dhd_pub_t *)(cfg->pub);
	u8 *vndr = NULL;
	u32 vndr_ie_len = 0;
	s32 err = BCME_OK;

	/* Set Beacon IEs to FW */
#ifndef  BRCM_RSDB
	if ((err = wl_cfgp2p_set_management_ie(cfg, dev, bssidx,
		VNDR_IE_BEACON_FLAG, (u8 *)info->tail,
#else
	if ((err = wl_cfg80211_set_mgmt_vndr_ies(cfg, ndev_to_cfgdev(dev), bssidx,
		VNDR_IE_BEACON_FLAG, (const u8 *)info->tail,
#endif
		info->tail_len)) < 0) {
		WL_ERR(("Set Beacon IE Failed \n"));
	} else {
		WL_DBG(("Applied Vndr IEs for Beacon \n"));
	}

	vndr = (u8 *)info->proberesp_ies;
	vndr_ie_len = info->proberesp_ies_len;

	if (dhd->op_mode & DHD_FLAG_HOSTAP_MODE) {
		/* SoftAP mode */
		struct ieee80211_mgmt *mgmt;
		mgmt = (struct ieee80211_mgmt *)info->probe_resp;
		if (mgmt != NULL) {
			vndr = (u8 *)&mgmt->u.probe_resp.variable;
			vndr_ie_len = info->probe_resp_len -
				offsetof(struct ieee80211_mgmt, u.probe_resp.variable);
		}
	}

	/* Set Probe Response IEs to FW */
#ifndef  BRCM_RSDB
	if ((err = wl_cfgp2p_set_management_ie(cfg, dev, bssidx,
#else
	if ((err = wl_cfg80211_set_mgmt_vndr_ies(cfg, ndev_to_cfgdev(dev), bssidx,
#endif
		VNDR_IE_PRBRSP_FLAG, vndr, vndr_ie_len)) < 0) {
		WL_ERR(("Set Probe Resp IE Failed \n"));
	} else {
		WL_DBG(("Applied Vndr IEs for Probe Resp \n"));
	}

	return err;
}
#endif

static s32 wl_cfg80211_hostapd_sec(
	struct net_device *dev,
	struct parsed_ies *ies,
	s32 bssidx)
{
	bool update_bss = 0;
	struct bcm_cfg80211 *cfg = g_bcm_cfg;

#ifndef  BRCM_RSDB
	if (ies->wps_ie) {
		if (cfg->ap_info->wps_ie &&
			memcmp(cfg->ap_info->wps_ie, ies->wps_ie, ies->wps_ie_len)) {
			WL_DBG((" WPS IE is changed\n"));
			kfree(cfg->ap_info->wps_ie);
			cfg->ap_info->wps_ie = kmemdup(ies->wps_ie, ies->wps_ie_len, GFP_KERNEL);
		} else if (cfg->ap_info->wps_ie == NULL) {
			WL_DBG((" WPS IE is added\n"));
			cfg->ap_info->wps_ie = kmemdup(ies->wps_ie, ies->wps_ie_len, GFP_KERNEL);
		}

#if defined(SUPPORT_SOFTAP_WPAWPA2_MIXED)
		if (ies->wpa_ie != NULL && ies->wpa2_ie != NULL) {
			WL_ERR(("update bss - wpa_ie and  wpa2_ie is not null\n"));
			if (!cfg->ap_info->security_mode) {
				/* change from open mode to security mode */
				update_bss = true;
				cfg->ap_info->wpa_ie =
					kmemdup(ies->wpa_ie,
					ies->wpa_ie->length + WPA_RSN_IE_TAG_FIXED_LEN,
					GFP_KERNEL);
				cfg->ap_info->rsn_ie =
					kmemdup(ies->wpa2_ie,
					ies->wpa2_ie->len + WPA_RSN_IE_TAG_FIXED_LEN,
					GFP_KERNEL);
			} else {
				/* change from (WPA or WPA2 or WPA/WPA2) to WPA/WPA2 mixed mode */
				if (cfg->ap_info->wpa_ie) {
					if (memcmp(cfg->ap_info->wpa_ie,
					ies->wpa_ie, ies->wpa_ie->length +
					WPA_RSN_IE_TAG_FIXED_LEN)) {
						kfree(cfg->ap_info->wpa_ie);
						update_bss = true;
						cfg->ap_info->wpa_ie = kmemdup(ies->wpa_ie,
						ies->wpa_ie->length + WPA_RSN_IE_TAG_FIXED_LEN,
						GFP_KERNEL);
					}
				}
				else {
					update_bss = true;
					cfg->ap_info->wpa_ie =
						kmemdup(ies->wpa_ie,
						ies->wpa_ie->length + WPA_RSN_IE_TAG_FIXED_LEN,
						GFP_KERNEL);
				}
				if (cfg->ap_info->rsn_ie) {
					if (memcmp(cfg->ap_info->rsn_ie,
					ies->wpa2_ie,
					ies->wpa2_ie->len + WPA_RSN_IE_TAG_FIXED_LEN)) {
						update_bss = true;
						kfree(cfg->ap_info->rsn_ie);
						cfg->ap_info->rsn_ie =
							kmemdup(ies->wpa2_ie,
							ies->wpa2_ie->len +
							WPA_RSN_IE_TAG_FIXED_LEN,
							GFP_KERNEL);
					}
				}
				else {
					update_bss = true;
					cfg->ap_info->rsn_ie =
						kmemdup(ies->wpa2_ie,
						ies->wpa2_ie->len + WPA_RSN_IE_TAG_FIXED_LEN,
						GFP_KERNEL);
				}
			}
			WL_ERR(("update_bss=%d\n", update_bss));
			if (update_bss) {
				cfg->ap_info->security_mode = true;
				wl_cfgp2p_bss(cfg, dev, bssidx, 0);
				if (wl_validate_wpaie_wpa2ie(dev, ies->wpa_ie,
					ies->wpa2_ie, bssidx)  < 0) {
					return BCME_ERROR;
				}
				wl_cfgp2p_bss(cfg, dev, bssidx, 1);
			}

		}
		else
#endif /* SUPPORT_SOFTAP_WPAWPA2_MIXED */
		if ((ies->wpa_ie != NULL || ies->wpa2_ie != NULL)) {
			if (!cfg->ap_info->security_mode) {
				/* change from open mode to security mode */
				update_bss = true;
				if (ies->wpa_ie != NULL) {
					cfg->ap_info->wpa_ie = kmemdup(ies->wpa_ie,
					ies->wpa_ie->length + WPA_RSN_IE_TAG_FIXED_LEN,
					GFP_KERNEL);
				} else {
					cfg->ap_info->rsn_ie = kmemdup(ies->wpa2_ie,
					ies->wpa2_ie->len + WPA_RSN_IE_TAG_FIXED_LEN,
					GFP_KERNEL);
				}
			} else if (cfg->ap_info->wpa_ie) {
				/* change from WPA2 mode to WPA mode */
				if (ies->wpa_ie != NULL) {
					update_bss = true;
					kfree(cfg->ap_info->rsn_ie);
					cfg->ap_info->rsn_ie = NULL;
					cfg->ap_info->wpa_ie = kmemdup(ies->wpa_ie,
					ies->wpa_ie->length + WPA_RSN_IE_TAG_FIXED_LEN,
					GFP_KERNEL);
				} else if (memcmp(cfg->ap_info->rsn_ie,
					ies->wpa2_ie, ies->wpa2_ie->len
					+ WPA_RSN_IE_TAG_FIXED_LEN)) {
					update_bss = true;
					kfree(cfg->ap_info->rsn_ie);
					cfg->ap_info->rsn_ie = kmemdup(ies->wpa2_ie,
					ies->wpa2_ie->len + WPA_RSN_IE_TAG_FIXED_LEN,
					GFP_KERNEL);
					cfg->ap_info->wpa_ie = NULL;
				}
			}
			if (update_bss) {
				cfg->ap_info->security_mode = true;
#else
	wl_cfgbss_t *bss = wl_get_cfgbss_by_wdev(cfg, dev->ieee80211_ptr);

	if (!bss) {
		WL_ERR(("cfgbss is NULL \n"));
		return -EINVAL;
	}

	if (ies->wps_ie) {
		if (bss->wps_ie &&
			memcmp(bss->wps_ie, ies->wps_ie, ies->wps_ie_len)) {
			WL_DBG((" WPS IE is changed\n"));
			kfree(bss->wps_ie);
			bss->wps_ie = kmemdup(ies->wps_ie, ies->wps_ie_len, GFP_KERNEL);
		} else if (bss->wps_ie == NULL) {
			WL_DBG((" WPS IE is added\n"));
			bss->wps_ie = kmemdup(ies->wps_ie, ies->wps_ie_len, GFP_KERNEL);
		}

#if defined(SUPPORT_SOFTAP_WPAWPA2_MIXED)
		if (ies->wpa_ie != NULL && ies->wpa2_ie != NULL) {
			WL_ERR(("update bss - wpa_ie and  wpa2_ie is not null\n"));
			if (!bss->security_mode) {
				/* change from open mode to security mode */
				update_bss = true;
				bss->wpa_ie =
					kmemdup(ies->wpa_ie,
					ies->wpa_ie->length + WPA_RSN_IE_TAG_FIXED_LEN,
					GFP_KERNEL);
				bss->rsn_ie =
					kmemdup(ies->wpa2_ie,
					ies->wpa2_ie->len + WPA_RSN_IE_TAG_FIXED_LEN,
					GFP_KERNEL);
			} else {
				/* change from (WPA or WPA2 or WPA/WPA2) to WPA/WPA2 mixed mode */
				if (bss->wpa_ie) {
					if (memcmp(bss->wpa_ie,
					ies->wpa_ie, ies->wpa_ie->length +
					WPA_RSN_IE_TAG_FIXED_LEN)) {
						kfree(bss->wpa_ie);
						update_bss = true;
						bss->wpa_ie = kmemdup(ies->wpa_ie,
						ies->wpa_ie->length + WPA_RSN_IE_TAG_FIXED_LEN,
						GFP_KERNEL);
					}
				}
				else {
					update_bss = true;
					bss->wpa_ie =
						kmemdup(ies->wpa_ie,
						ies->wpa_ie->length + WPA_RSN_IE_TAG_FIXED_LEN,
						GFP_KERNEL);
				}
				if (bss->rsn_ie) {
					if (memcmp(bss->rsn_ie,
					ies->wpa2_ie,
					ies->wpa2_ie->len + WPA_RSN_IE_TAG_FIXED_LEN)) {
						update_bss = true;
						kfree(bss->rsn_ie);
						bss->rsn_ie =
							kmemdup(ies->wpa2_ie,
							ies->wpa2_ie->len +
							WPA_RSN_IE_TAG_FIXED_LEN,
							GFP_KERNEL);
					}
				}
				else {
					update_bss = true;
					bss->rsn_ie =
						kmemdup(ies->wpa2_ie,
						ies->wpa2_ie->len + WPA_RSN_IE_TAG_FIXED_LEN,
						GFP_KERNEL);
				}
			}
			WL_ERR(("update_bss=%d\n", update_bss));
			if (update_bss) {
				bss->security_mode = true;
				wl_cfgp2p_bss(cfg, dev, bssidx, 0);
				if (wl_validate_wpaie_wpa2ie(dev, ies->wpa_ie,
					ies->wpa2_ie, bssidx)  < 0) {
					return BCME_ERROR;
				}
				wl_cfgp2p_bss(cfg, dev, bssidx, 1);
			}

		}
		else
#endif /* SUPPORT_SOFTAP_WPAWPA2_MIXED */
		if ((ies->wpa_ie != NULL || ies->wpa2_ie != NULL)) {
			if (!bss->security_mode) {
				/* change from open mode to security mode */
				update_bss = true;
				if (ies->wpa_ie != NULL) {
					bss->wpa_ie = kmemdup(ies->wpa_ie,
					ies->wpa_ie->length + WPA_RSN_IE_TAG_FIXED_LEN,
					GFP_KERNEL);
				} else {
					bss->rsn_ie = kmemdup(ies->wpa2_ie,
					ies->wpa2_ie->len + WPA_RSN_IE_TAG_FIXED_LEN,
					GFP_KERNEL);
				}
			} else if (bss->wpa_ie) {
				/* change from WPA2 mode to WPA mode */
				if (ies->wpa_ie != NULL) {
					update_bss = true;
					kfree(bss->rsn_ie);
					bss->rsn_ie = NULL;
					bss->wpa_ie = kmemdup(ies->wpa_ie,
					ies->wpa_ie->length + WPA_RSN_IE_TAG_FIXED_LEN,
					GFP_KERNEL);
				} else if (memcmp(bss->rsn_ie,
					ies->wpa2_ie, ies->wpa2_ie->len
					+ WPA_RSN_IE_TAG_FIXED_LEN)) {
					update_bss = true;
					kfree(bss->rsn_ie);
					bss->rsn_ie = kmemdup(ies->wpa2_ie,
					ies->wpa2_ie->len + WPA_RSN_IE_TAG_FIXED_LEN,
					GFP_KERNEL);
					bss->wpa_ie = NULL;
				}
			}
			if (update_bss) {
				bss->security_mode = true;
#endif /* BRCM_RSDB */
				wl_cfgp2p_bss(cfg, dev, bssidx, 0);
				if (wl_validate_wpa2ie(dev, ies->wpa2_ie, bssidx)  < 0 ||
					wl_validate_wpaie(dev, ies->wpa_ie, bssidx) < 0) {
					return BCME_ERROR;
				}
				wl_cfgp2p_bss(cfg, dev, bssidx, 1);
			}
		}
	} else {
		WL_ERR(("No WPSIE in beacon \n"));
	}
	return 0;
}

#if defined(WL_SUPPORT_BACKPORTED_KPATCHES) || (LINUX_VERSION_CODE >= KERNEL_VERSION(3, \
	2, 0))
#ifndef  BRCM_RSDB
static s32
wl_cfg80211_del_station(
	struct wiphy *wiphy,
	struct net_device *ndev,
#ifdef HW_KERNEL_4_0_ADAPTATION
	struct station_del_parameters *params)
#else
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0))
	const
#endif
	u8 *mac_addr)
#endif /* HW_KERNEL_4_0_ADAPTATION */
#else
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 19, 0))
static s32
wl_cfg80211_del_station(
		struct wiphy *wiphy, struct net_device *ndev,
		struct station_del_parameters *params)
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0))
static s32
wl_cfg80211_del_station(
	struct wiphy *wiphy,
	struct net_device *ndev,
	const u8* mac_addr)
#else
static s32
wl_cfg80211_del_station(
	struct wiphy *wiphy,
	struct net_device *ndev,
	u8* mac_addr)
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 19, 0)) */
#endif  /* BRCM_RSDB */
{
#ifdef HW_KERNEL_4_0_ADAPTATION
	const u8 *mac_addr = ((NULL != params)? params->mac: NULL);
#endif
	struct net_device *dev;
	struct bcm_cfg80211 *cfg = wiphy_priv(wiphy);
	scb_val_t scb_val;
	s8 eabuf[ETHER_ADDR_STR_LEN];
	int err;
	char mac_buf[MAX_NUM_OF_ASSOCIATED_DEV *
		sizeof(struct ether_addr) + sizeof(uint)] = {0};
	struct maclist *assoc_maclist = (struct maclist *)mac_buf;
	int num_associated = 0;

	WL_DBG(("Entry\n"));
	if (mac_addr == NULL) {
		WL_DBG(("mac_addr is NULL ignore it\n"));
		return 0;
	}

	dev = ndev_to_wlc_ndev(ndev, cfg);

	if (p2p_is_on(cfg)) {
		/* Suspend P2P discovery search-listen to prevent it from changing the
		 * channel.
		 */
		if ((wl_cfgp2p_discover_enable_search(cfg, false)) < 0) {
			WL_ERR(("Can not disable discovery mode\n"));
			return -EFAULT;
		}
	}

	assoc_maclist->count = MAX_NUM_OF_ASSOCIATED_DEV;
	err = wldev_ioctl(ndev, WLC_GET_ASSOCLIST,
		assoc_maclist, sizeof(mac_buf), false);
	if (err < 0)
		WL_ERR(("WLC_GET_ASSOCLIST error %d\n", err));
	else
		num_associated = assoc_maclist->count;

	memcpy(scb_val.ea.octet, mac_addr, ETHER_ADDR_LEN);
	scb_val.val = DOT11_RC_DEAUTH_LEAVING;
	err = wldev_ioctl(dev, WLC_SCB_DEAUTHENTICATE_FOR_REASON, &scb_val,
		sizeof(scb_val_t), true);
	if (err < 0)
		WL_ERR(("WLC_SCB_DEAUTHENTICATE_FOR_REASON err %d\n", err));
	WL_ERR(("Disconnect STA : %s scb_val.val %d\n",
		bcm_ether_ntoa((const struct ether_addr *)mac_addr, eabuf),
		scb_val.val));

	if (num_associated > 0 && ETHER_ISBCAST(mac_addr))
		wl_delay(400);

	return 0;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0))
static s32
wl_cfg80211_change_station(
	struct wiphy *wiphy,
	struct net_device *dev,
	const u8 *mac,
	struct station_parameters *params)
#else
static s32
wl_cfg80211_change_station(
	struct wiphy *wiphy,
	struct net_device *dev,
	u8 *mac,
	struct station_parameters *params)
#endif
{
	int err;
#ifdef  BRCM_RSDB
#ifdef DHD_LOSSLESS_ROAMING
	struct bcm_cfg80211 *cfg = wiphy_priv(wiphy);
#endif
#endif
	WL_DBG(("SCB_AUTHORIZE mac_addr:"MACDBG" sta_flags_mask:0x%x "
				"sta_flags_set:0x%x iface:%s \n", MAC2STRDBG(mac),
				params->sta_flags_mask, params->sta_flags_set, dev->name));

	/* Processing only authorize/de-authorize flag for now */
	if (!(params->sta_flags_mask & BIT(NL80211_STA_FLAG_AUTHORIZED))) {
		WL_ERR(("WLC_SCB_AUTHORIZE sta_flags_mask not set \n"));
		return -ENOTSUPP;
	}

	if (!(params->sta_flags_set & BIT(NL80211_STA_FLAG_AUTHORIZED))) {
#ifndef  BRCM_RSDB
		err = wldev_ioctl(dev, WLC_SCB_DEAUTHORIZE, (void *)mac, ETH_ALEN, true);
#else
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0))
		err = wldev_ioctl(dev, WLC_SCB_DEAUTHORIZE, (u8 *)mac, ETH_ALEN, true);
#else
		err = wldev_ioctl(dev, WLC_SCB_DEAUTHORIZE, mac, ETH_ALEN, true);
#endif
#endif
		if (err)
			WL_ERR(("WLC_SCB_DEAUTHORIZE error (%d)\n", err));
		return err;
	}

#ifndef  BRCM_RSDB
	err = wldev_ioctl(dev, WLC_SCB_AUTHORIZE, (void *)mac, ETH_ALEN, true);
#else
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0))
	err = wldev_ioctl(dev, WLC_SCB_AUTHORIZE, (u8 *)mac, ETH_ALEN, true);
#else
	err = wldev_ioctl(dev, WLC_SCB_AUTHORIZE, mac, ETH_ALEN, true);
#endif
#endif
	if (err)
		WL_ERR(("WLC_SCB_AUTHORIZE error (%d)\n", err));
#ifdef  BRCM_RSDB
#ifdef DHD_LOSSLESS_ROAMING
	wl_del_roam_timeout(cfg);
#endif
#endif
	return err;
}
#endif /* WL_SUPPORT_BACKPORTED_KPATCHES || KERNEL_VER >= KERNEL_VERSION(3, 2, 0)) */
#ifdef  BRCM_RSDB
static s32
wl_cfg80211_set_scb_timings(
	struct bcm_cfg80211 *cfg,
	struct net_device *dev)
{
	int err;
	u32 ps_pretend;
	wl_scb_probe_t scb_probe;

	bzero(&scb_probe, sizeof(wl_scb_probe_t));
	scb_probe.scb_timeout = WL_SCB_TIMEOUT;
	scb_probe.scb_activity_time = WL_SCB_ACTIVITY_TIME;
	scb_probe.scb_max_probe = WL_SCB_MAX_PROBE;
	err = wldev_iovar_setbuf(dev, "scb_probe", (void *)&scb_probe,
		sizeof(wl_scb_probe_t), cfg->ioctl_buf, WLC_IOCTL_SMLEN,
		&cfg->ioctl_buf_sync);
	if (unlikely(err)) {
		WL_ERR(("set 'scb_probe' failed, error = %d\n", err));
		return err;
	}

	ps_pretend = MAX(WL_SCB_MAX_PROBE / 2, WL_MIN_PSPRETEND_THRESHOLD);
	err = wldev_iovar_setint(dev, "pspretend_threshold", ps_pretend);
	if (unlikely(err)) {
		if (err == BCME_UNSUPPORTED) {
			/* Ignore error if fw doesn't support the iovar */
			WL_DBG(("wl pspretend_threshold %d set error %d\n",
				ps_pretend, err));
		} else {
			WL_ERR(("wl pspretend_threshold %d set error %d\n",
				ps_pretend, err));
			return err;
		}
	}

	return 0;
}
#endif /* BRCM_RSDB */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 4, 0)) || defined(WL_COMPAT_WIRELESS)
static s32
wl_cfg80211_start_ap(
	struct wiphy *wiphy,
	struct net_device *dev,
	struct cfg80211_ap_settings *info)
{
	struct bcm_cfg80211 *cfg = wiphy_priv(wiphy);
	s32 err = BCME_OK;
	struct parsed_ies ies;
	s32 bssidx = 0;
	u32 dev_role = 0;
#ifdef  BRCM_RSDB
	dhd_pub_t *dhd = (dhd_pub_t *)(cfg->pub);
	s32 ap_channel = 0;
#endif
	WL_DBG(("Enter \n"));
#ifndef  BRCM_RSDB
	if (dev == bcmcfg_to_prmry_ndev(cfg)) {
		WL_DBG(("Start AP req on primary iface: Softap\n"));
		dev_role = NL80211_IFTYPE_AP;
		if (!cfg->ap_info) {
			if ((cfg->ap_info = kzalloc(sizeof(struct ap_info), GFP_KERNEL))) {
				WL_ERR(("%s: struct ap_info re-allocated\n", __FUNCTION__));
			} else {
				WL_ERR(("%s: struct ap_info re-allocation failed\n", __FUNCTION__));
				err = -ENOMEM;
				goto fail;
			}
		}
#else
#if defined(SUPPORT_RANDOM_MAC_SCAN)
	wl_cfg80211_set_random_mac(dev, FALSE);
#endif /* SUPPORT_RANDOM_MAC_SCAN */

	if ((dev == bcmcfg_to_prmry_ndev(cfg)) ||
		(dev == ((struct net_device *)cfgdev_to_ndev(cfg->bss_cfgdev)))) {
		WL_DBG(("Start AP req on iface: %s \n", dev->name));
		dev_role = NL80211_IFTYPE_AP;
#endif
	}
#if defined(WL_ENABLE_P2P_IF)
	else if (dev == cfg->p2p_net) {
		/* Group Add request on p2p0 */
		WL_DBG(("Start AP req on P2P iface: GO\n"));
		dev = bcmcfg_to_prmry_ndev(cfg);
		dev_role = NL80211_IFTYPE_P2P_GO;
	}
#endif /* WL_ENABLE_P2P_IF */
#ifndef  BRCM_RSDB
	if (wl_cfgp2p_find_idx(cfg, dev, &bssidx) != BCME_OK) {
		WL_ERR(("Find p2p index from dev(%p) failed\n", dev));
		return BCME_ERROR;
	}
	if (p2p_is_on(cfg) &&
		(bssidx == wl_to_p2p_bss_bssidx(cfg,
		P2PAPI_BSSCFG_CONNECTION))) {
		dev_role = NL80211_IFTYPE_P2P_GO;
		WL_DBG(("Start AP req on P2P connection iface\n"));
	}
	if (dev_role == NL80211_IFTYPE_AP && !cfg->ap_info) {
		WL_ERR(("ap_info is NULL!!!!"));
		err = BCME_ERROR;
		goto fail;
	}

	if (!check_dev_role_integrity(cfg, dev_role))
		goto fail;
#else
	if ((bssidx = wl_get_bssidx_by_wdev(cfg, dev->ieee80211_ptr)) < 0) {
		WL_ERR(("Find p2p index from wdev(%p) failed\n", dev->ieee80211_ptr));
		return BCME_ERROR;
	}

	if (p2p_is_on(cfg) && (dev->ieee80211_ptr->iftype == NL80211_IFTYPE_P2P_GO)) {
		dev_role = NL80211_IFTYPE_P2P_GO;
	} else if (dev_role == NL80211_IFTYPE_AP) {
		dhd->op_mode |= DHD_FLAG_HOSTAP_MODE;
		/*
		 * Enabling Softap is causing issues with STA NDO operations
		 * as NDO is not interface specific. So disable NDO while
		 * Softap is enabled
		 */
		err = dhd_ndo_enable(dhd, FALSE);
		WL_DBG(("%s: Disabling NDO on Hostapd mode %d\n", __FUNCTION__, err));
		if (err) {
			/* Non fatal error. */
			WL_ERR(("%s: Disabling NDO Failed %d\n", __FUNCTION__, err));
		} else {
			cfg->revert_ndo_disable = true;
		}

#ifdef PKT_FILTER_SUPPORT
		/* Disable packet filter */
		if (dhd->early_suspended) {
			WL_ERR(("Disable pkt_filter\n"));
			dhd_enable_packet_filter(0, dhd);
		}
#endif /* PKT_FILTER_SUPPORT */
#ifdef ARP_OFFLOAD_SUPPORT
		/* IF SoftAP is enabled, disable arpoe */
		if (dhd->op_mode & DHD_FLAG_STA_MODE) {
			dhd_arp_offload_set(dhd, 0);
			dhd_arp_offload_enable(dhd, FALSE);
		}
#endif /* ARP_OFFLOAD_SUPPORT */
		if ((dhd->op_mode & DHD_FLAG_STA_MODE) && wl_cfg80211_is_roam_offload()) {
			WL_ERR(("Cleare roam_offload_bssid_list at STA-SoftAP MODE.\n"));
			//wl_android_set_roam_offload_bssid_list(dev, "0");
		}
	} else {
		/* only AP or GO role need to be handled here. */
		err = -EINVAL;
		goto fail;
	}

	if (!check_dev_role_integrity(cfg, dev_role)) {
		err = -EINVAL;
		goto fail;
	}
#endif /* BRCM_RSDB */

#if ((LINUX_VERSION_CODE >= KERNEL_VERSION(3, 6, 0)) && !defined(WL_COMPAT_WIRELESS))
	if ((err = wl_cfg80211_set_channel(wiphy, dev,
		dev->ieee80211_ptr->preset_chandef.chan,
		NL80211_CHAN_HT20) < 0)) {
		WL_ERR(("Set channel failed \n"));
		goto fail;
	}
#endif

	if ((err = wl_cfg80211_bcn_set_params(info, dev,
		dev_role, bssidx)) < 0) {
		WL_ERR(("Beacon params set failed \n"));
		goto fail;
	}

	/* Parse IEs */
	if ((err = wl_cfg80211_parse_ap_ies(dev, &info->beacon, &ies)) < 0) {
		WL_ERR(("Set IEs failed \n"));
		goto fail;
	}
#ifndef  BRCM_RSDB
	if ((wl_cfg80211_bcn_validate_sec(dev, &ies,
		dev_role, bssidx)) < 0)
#else
	if ((err = wl_cfg80211_bcn_validate_sec(dev, &ies,
		dev_role, bssidx, info->privacy)) < 0)
#endif
	{
		WL_ERR(("Beacon set security failed \n"));
		goto fail;
	}

	if ((err = wl_cfg80211_bcn_bringup_ap(dev, &ies,
		dev_role, bssidx)) < 0) {
		WL_ERR(("Beacon bring up AP/GO failed \n"));
#ifdef  BRCM_RSDB
		goto fail;
	}

	/* Set GC/STA SCB expiry timings. */
	if ((err = wl_cfg80211_set_scb_timings(cfg, dev))) {
		WL_ERR(("scb setting failed \n"));
#endif
		goto fail;
	}
#ifdef BRCM_RSDB
//only 4359 support ap powersave mode
	printf("#### set radio ps\n");
	//Enable Ap reduce power consumption for duty cycle
#ifdef HW_AP_POWERSAVE
	wldev_iovar_setint(dev, "radio_pwrsave_enable", 0);
#else
	wldev_iovar_setint(dev, "radio_pwrsave_enable", 1);
#endif
	//Above 10s, from duty cycle to alive
	wldev_iovar_setint(dev, "radio_pwrsave_pps", 10);
	//If not receive package for 1 second, form alive to duty cycle
	wldev_iovar_setint(dev, "radio_pwrsave_quiet_time", 1);
	//duty cycle level
	wldev_iovar_setint(dev, "radio_pwrsave_level", 8);
	wldev_iovar_setint(dev, "radio_pwrsave_stas_assoc_check", 1);


	err = wldev_ioctl(dev, WLC_GET_CHANNEL, &ap_channel, sizeof(s32), false);
	if (err < 0) {
		WL_ERR(("GET channel err %d\n", err));
		goto fail;
	}
	printf("#### channel=%d\n", ap_channel);
	// If ap channel is 2g or v project
	if((ap_channel <= 14) || (dhd->rsdb_mode == FALSE)) {
		//If current mimo mode ,then cut back mimo mode when stop_ap
		wldev_iovar_getint(dev, "txchain", &g_pre_chain);
		printf("#### set anntenna to 1\n");
		wldev_iovar_setint(dev, "txchain", 1);
		wldev_iovar_setint(dev, "rxchain", 1);
		//set siso mode
#ifdef HW_P2PGO_2G_SISO
		g_p2pgo_siso = 1;
#endif
	}
#endif
	WL_DBG(("** AP/GO Created **\n"));

#ifdef WL_CFG80211_ACL
	/* Enfoce Admission Control. */
	if ((err = wl_cfg80211_set_mac_acl(wiphy, dev, info->acl)) < 0) {
		WL_ERR(("Set ACL failed\n"));
	}
#endif /* WL_CFG80211_ACL */

	/* Set IEs to FW */
	if ((err = wl_cfg80211_set_ies(dev, &info->beacon, bssidx)) < 0)
		WL_ERR(("Set IEs failed \n"));

	/* Enable Probe Req filter, WPS-AP certification 4.2.13 */
	if ((dev_role == NL80211_IFTYPE_AP) && (ies.wps_ie != NULL)) {
		bool pbc = 0;
		wl_validate_wps_ie((char *) ies.wps_ie, ies.wps_ie_len, &pbc);
		if (pbc) {
			WL_DBG(("set WLC_E_PROBREQ_MSG\n"));
			wl_add_remove_eventmsg(dev, WLC_E_PROBREQ_MSG, true);
		}
	}

fail:
	if (err) {
		WL_ERR(("ADD/SET beacon failed\n"));
		wldev_iovar_setint(dev, "mpc", 1);
#ifdef  BRCM_RSDB
		if (dev_role == NL80211_IFTYPE_AP) {
			dhd->op_mode &= ~DHD_FLAG_HOSTAP_MODE;

#ifdef PKT_FILTER_SUPPORT
			/* Enable packet filter */
			if (dhd->early_suspended) {
				WL_ERR(("Enable pkt_filter\n"));
				dhd_enable_packet_filter(1, dhd);
			}
#endif /* PKT_FILTER_SUPPORT */
#ifdef ARP_OFFLOAD_SUPPORT
			/* IF SoftAP is disabled, enable arpoe back for STA mode. */
			if (dhd->op_mode & DHD_FLAG_STA_MODE) {
				dhd_arp_offload_set(dhd, dhd_arp_mode);
				dhd_arp_offload_enable(dhd, TRUE);
			}
#endif /* ARP_OFFLOAD_SUPPORT */
		}
#endif /* BRCM_RSDB */
	}

	return err;
}
#ifndef  BRCM_RSDB
static s32
wl_cfg80211_stop_ap(
	struct wiphy *wiphy,
	struct net_device *dev)
{
	int err = 0;
	u32 dev_role = 0;
	int infra = 0;
	int ap = 0;
	s32 bssidx = 0;
	struct bcm_cfg80211 *cfg = wiphy_priv(wiphy);

	WL_DBG(("Enter \n"));
	if (dev == bcmcfg_to_prmry_ndev(cfg)) {
		dev_role = NL80211_IFTYPE_AP;
	}
#if defined(WL_ENABLE_P2P_IF)
	else if (dev == cfg->p2p_net) {
		/* Group Add request on p2p0 */
		dev = bcmcfg_to_prmry_ndev(cfg);
		dev_role = NL80211_IFTYPE_P2P_GO;
	}
#endif /* WL_ENABLE_P2P_IF */
	if (wl_cfgp2p_find_idx(cfg, dev, &bssidx) != BCME_OK) {
		WL_ERR(("Find p2p index from dev(%p) failed\n", dev));
		return BCME_ERROR;
	}
	if (p2p_is_on(cfg) &&
		(bssidx == wl_to_p2p_bss_bssidx(cfg,
		P2PAPI_BSSCFG_CONNECTION))) {
		dev_role = NL80211_IFTYPE_P2P_GO;
	}

	if (!check_dev_role_integrity(cfg, dev_role))
		goto exit;

	if (dev_role == NL80211_IFTYPE_AP) {
		/* SoftAp on primary Interface.
		 * Shut down AP and turn on MPC
		 */
		if ((err = wldev_ioctl(dev, WLC_SET_AP, &ap, sizeof(s32), true)) < 0) {
			WL_ERR(("setting AP mode failed %d \n", err));
			err = -ENOTSUPP;
			goto exit;
		}
		err = wldev_ioctl(dev, WLC_SET_INFRA, &infra, sizeof(s32), true);
		if (err < 0) {
			WL_ERR(("SET INFRA error %d\n", err));
			err = -ENOTSUPP;
			goto exit;
		}

		err = wldev_ioctl(dev, WLC_UP, &ap, sizeof(s32), true);
		if (unlikely(err)) {
			WL_ERR(("WLC_UP error (%d)\n", err));
			err = -EINVAL;
			goto exit;
		}

		wl_clr_drv_status(cfg, AP_CREATED, dev);
		/* Turn on the MPC */
		wldev_iovar_setint(dev, "mpc", 1);
		if (cfg->ap_info) {
			kfree(cfg->ap_info->wpa_ie);
			kfree(cfg->ap_info->rsn_ie);
			kfree(cfg->ap_info->wps_ie);
			kfree(cfg->ap_info);
			cfg->ap_info = NULL;
		}
	} else {
		WL_DBG(("Stopping P2P GO \n"));
		DHD_OS_WAKE_LOCK_CTRL_TIMEOUT_ENABLE((dhd_pub_t *)(cfg->pub),
			DHD_EVENT_TIMEOUT_MS*3);
		DHD_OS_WAKE_LOCK_TIMEOUT((dhd_pub_t *)(cfg->pub));
	}

exit:
	return err;
}
#else
static s32
wl_cfg80211_stop_ap(
	struct wiphy *wiphy,
	struct net_device *dev)
{
	int err = 0;
	u32 dev_role = 0;
	int infra = 0;
	int ap = 0;
	s32 bssidx = 0;
	struct bcm_cfg80211 *cfg = wiphy_priv(wiphy);
	s32 is_rsdb_supported = BCME_ERROR;
	dhd_pub_t *dhd = (dhd_pub_t *)(cfg->pub);

	WL_DBG(("Enter \n"));

	is_rsdb_supported = DHD_OPMODE_SUPPORTED(cfg->pub, DHD_FLAG_RSDB_MODE);
	if (is_rsdb_supported < 0)
		return (-ENODEV);

	wl_clr_drv_status(cfg, AP_CREATING, dev);
	wl_clr_drv_status(cfg, AP_CREATED, dev);
	cfg->ap_oper_channel = 0;

	if (dev->ieee80211_ptr->iftype == NL80211_IFTYPE_AP) {
		dev_role = NL80211_IFTYPE_AP;
		WL_DBG(("stopping AP operation\n"));
	} else if (dev->ieee80211_ptr->iftype == NL80211_IFTYPE_P2P_GO) {
		dev_role = NL80211_IFTYPE_P2P_GO;
		WL_DBG(("stopping P2P GO operation\n"));
	} else {
		WL_ERR(("no AP/P2P GO interface is operational.\n"));
		return -EINVAL;
	}

	if ((bssidx = wl_get_bssidx_by_wdev(cfg, dev->ieee80211_ptr)) < 0) {
		WL_ERR(("find p2p index from wdev(%p) failed\n", dev->ieee80211_ptr));
		return BCME_ERROR;
	}

	if (!check_dev_role_integrity(cfg, dev_role)) {
		WL_ERR(("role integrity check failed \n"));
		err = -EINVAL;
		goto exit;
	}

	if ((err = wl_cfgp2p_bss(cfg, dev, bssidx, 0)) < 0) {
		WL_ERR(("bss down error %d\n", err));
	}

	if (dev_role == NL80211_IFTYPE_AP) {
		if (cfg->revert_ndo_disable == true) {
			err = dhd_ndo_enable(dhd, TRUE);
			WL_DBG(("%s: Enabling back NDO on Softap turn off %d\n",
				__FUNCTION__, err));
			if (err) {
				WL_ERR(("%s: Enabling NDO Failed %d\n", __FUNCTION__, err));
			}
			cfg->revert_ndo_disable = false;
		}

#ifdef PKT_FILTER_SUPPORT
		/* Enable packet filter */
		if (dhd->early_suspended) {
			WL_ERR(("Enable pkt_filter\n"));
			dhd_enable_packet_filter(1, dhd);
		}
#endif /* PKT_FILTER_SUPPORT */
#ifdef ARP_OFFLOAD_SUPPORT
		/* IF SoftAP is disabled, enable arpoe back for STA mode. */
		if (dhd->op_mode & DHD_FLAG_STA_MODE) {
			dhd_arp_offload_set(dhd, dhd_arp_mode);
			dhd_arp_offload_enable(dhd, TRUE);
		}
#endif /* ARP_OFFLOAD_SUPPORT */
		/*
		 * Bring down the AP interface by changing role to STA.
		 * Don't do a down or "WLC_SET_AP 0" since the shared
		 * interface may be still running
		 */
		if (is_rsdb_supported) {
			if ((err = wl_cfg80211_add_del_bss(cfg, dev,
				bssidx, NL80211_IFTYPE_STATION, 0, NULL)) < 0) {
				if ((err = wldev_ioctl(dev, WLC_SET_AP, &ap, sizeof(s32),
					true)) < 0) {
					WL_ERR(("setting AP mode failed %d \n", err));
					err = -ENOTSUPP;
					goto exit;
				}
			}
		} else if (is_rsdb_supported == 0) {
			/* SoftAp on primary Interface.
             		 * Shut down AP and turn on MPC
             		 */
			err = wldev_ioctl(dev, WLC_SET_AP, &ap, sizeof(s32), true);
			if (err  < 0) {
				WL_ERR(("setting AP mode failed %d \n", err));
				err = -ENOTSUPP;
				goto exit;
			}
			err = wldev_ioctl(dev, WLC_SET_INFRA, &infra, sizeof(s32), true);
			if (err < 0) {
				WL_ERR(("SET INFRA error %d\n", err));
				err = -ENOTSUPP;
				goto exit;
			}
			err = wldev_ioctl(dev, WLC_UP, &ap, sizeof(s32), true);
			if (unlikely(err)) {
				WL_ERR(("WLC_UP error (%d)\n", err));
				err = -EINVAL;
				goto exit;
			}
		}

		/* Turn on the MPC */
		wldev_iovar_setint(dev, "mpc", 1);

		 wl_cfg80211_clear_per_bss_ies(cfg, bssidx);
	} else {
		s32 ap_channel = 0;
		err = wldev_ioctl(dev, WLC_GET_CHANNEL, &ap_channel, sizeof(s32), false);
		if (err < 0) {
			WL_ERR(("GET channel err %d\n", err));
			goto exit;
		}
		printf("#### channel=%d\n", ap_channel);
		if((ap_channel <= 14) || (dhd->rsdb_mode == FALSE)) {
			if (3 == g_pre_chain) {
				printf("#### set anntenna to 3\n");
				wldev_iovar_setint(dev, "txchain", 3);
				wldev_iovar_setint(dev, "rxchain", 3);
				g_pre_chain = 0;
			}
#ifdef HW_P2PGO_2G_SISO
			g_p2pgo_siso = 0;
#endif
		}
		WL_DBG(("Stopping P2P GO \n"));
		DHD_OS_WAKE_LOCK_CTRL_TIMEOUT_ENABLE((dhd_pub_t *)(cfg->pub),
			DHD_EVENT_TIMEOUT_MS*3);
		DHD_OS_WAKE_LOCK_TIMEOUT((dhd_pub_t *)(cfg->pub));
	}

exit:

	if (dev_role == NL80211_IFTYPE_AP) {
		/* clear the AP mode */
		dhd->op_mode &= ~DHD_FLAG_HOSTAP_MODE;
	}
	return err;
}
#endif /* BRCM_RSDB */

static s32
wl_cfg80211_change_beacon(
	struct wiphy *wiphy,
	struct net_device *dev,
	struct cfg80211_beacon_data *info)
{
	s32 err = BCME_OK;
	struct bcm_cfg80211 *cfg = wiphy_priv(wiphy);
	struct parsed_ies ies;
	u32 dev_role = 0;
	s32 bssidx = 0;
	bool pbc = 0;

	WL_DBG(("Enter \n"));

	if (dev == bcmcfg_to_prmry_ndev(cfg)) {
		dev_role = NL80211_IFTYPE_AP;
	}
#if defined(WL_ENABLE_P2P_IF)
	else if (dev == cfg->p2p_net) {
		/* Group Add request on p2p0 */
		dev = bcmcfg_to_prmry_ndev(cfg);
		dev_role = NL80211_IFTYPE_P2P_GO;
	}
#endif /* WL_ENABLE_P2P_IF */
#ifndef  BRCM_RSDB
	if (wl_cfgp2p_find_idx(cfg, dev, &bssidx) != BCME_OK) {
		WL_ERR(("Find p2p index from dev(%p) failed\n", dev));
		return BCME_ERROR;
	}
	if (p2p_is_on(cfg) &&
		(bssidx == wl_to_p2p_bss_bssidx(cfg,
		P2PAPI_BSSCFG_CONNECTION))) {
		dev_role = NL80211_IFTYPE_P2P_GO;
	}

	if (!check_dev_role_integrity(cfg, dev_role))
		goto fail;
#else
	if ((bssidx = wl_get_bssidx_by_wdev(cfg, dev->ieee80211_ptr)) < 0) {
		WL_ERR(("Find p2p index from wdev(%p) failed\n", dev->ieee80211_ptr));
		return BCME_ERROR;
	}

	if (dev->ieee80211_ptr->iftype == NL80211_IFTYPE_P2P_GO) {
		dev_role = NL80211_IFTYPE_P2P_GO;
	}

	if (!check_dev_role_integrity(cfg, dev_role)) {
		err = -EINVAL;
		goto fail;
	}
#endif  /* BRCM_RSDB */

	if ((dev_role == NL80211_IFTYPE_P2P_GO) && (cfg->p2p_wdev == NULL)) {
		WL_ERR(("P2P already down status!\n"));
		err = BCME_ERROR;
		goto fail;
	}

	/* Parse IEs */
	if ((err = wl_cfg80211_parse_ap_ies(dev, info, &ies)) < 0) {
		WL_ERR(("Parse IEs failed \n"));
		goto fail;
	}

	/* Set IEs to FW */
	if ((err = wl_cfg80211_set_ies(dev, info, bssidx)) < 0) {
		WL_ERR(("Set IEs failed \n"));
		goto fail;
	}

	if (dev_role == NL80211_IFTYPE_AP) {
		if (wl_cfg80211_hostapd_sec(dev, &ies, bssidx) < 0) {
			WL_ERR(("Hostapd update sec failed \n"));
			err = -EINVAL;
			goto fail;
		}
		/* Enable Probe Req filter, WPS-AP certification 4.2.13 */
		if ((dev_role == NL80211_IFTYPE_AP) && (ies.wps_ie != NULL)) {
			wl_validate_wps_ie((char *) ies.wps_ie, ies.wps_ie_len, &pbc);
			WL_DBG((" WPS AP, wps_ie is exists pbc=%d\n", pbc));
			if (pbc)
				wl_add_remove_eventmsg(dev, WLC_E_PROBREQ_MSG, true);
			else
				wl_add_remove_eventmsg(dev, WLC_E_PROBREQ_MSG, false);
		}
	}

fail:
	return err;
}
#else
static s32
wl_cfg80211_add_set_beacon(struct wiphy *wiphy, struct net_device *dev,
	struct beacon_parameters *info)
{
	s32 err = BCME_OK;
	struct bcm_cfg80211 *cfg = wiphy_priv(wiphy);
	s32 ie_offset = 0;
	s32 bssidx = 0;
	u32 dev_role = NL80211_IFTYPE_AP;
	struct parsed_ies ies;
	bcm_tlv_t *ssid_ie;
	bool pbc = 0;
#ifdef  BRCM_RSDB
	bool privacy;
	bool is_bss_up = 0;
	dhd_pub_t *dhd = (dhd_pub_t *)(cfg->pub);
#endif
	WL_DBG(("interval (%d) dtim_period (%d) head_len (%d) tail_len (%d)\n",
		info->interval, info->dtim_period, info->head_len, info->tail_len));

	if (dev == bcmcfg_to_prmry_ndev(cfg)) {
		dev_role = NL80211_IFTYPE_AP;
	}
#if defined(WL_ENABLE_P2P_IF)
	else if (dev == cfg->p2p_net) {
		/* Group Add request on p2p0 */
		dev = bcmcfg_to_prmry_ndev(cfg);
		dev_role = NL80211_IFTYPE_P2P_GO;
	}
#endif /* WL_ENABLE_P2P_IF */
#ifndef  BRCM_RSDB
	if (wl_cfgp2p_find_idx(cfg, dev, &bssidx) != BCME_OK) {
		WL_ERR(("Find p2p index from dev(%p) failed\n", dev));
		return BCME_ERROR;
	}
	if (p2p_is_on(cfg) &&
		(bssidx == wl_to_p2p_bss_bssidx(cfg,
		P2PAPI_BSSCFG_CONNECTION))) {
		dev_role = NL80211_IFTYPE_P2P_GO;
	}

	if (!check_dev_role_integrity(cfg, dev_role))
		goto fail;
#else
	if ((bssidx = wl_get_bssidx_by_wdev(cfg, dev->ieee80211_ptr)) < 0) {
		WL_ERR(("Find p2p index from wdev(%p) failed\n", dev->ieee80211_ptr));
		return BCME_ERROR;
	}

	if (dev->ieee80211_ptr->iftype == NL80211_IFTYPE_P2P_GO) {
		dev_role = NL80211_IFTYPE_P2P_GO;
	} else if (dev->ieee80211_ptr->iftype == NL80211_IFTYPE_AP) {
		dhd->op_mode |= DHD_FLAG_HOSTAP_MODE;
	}

	if (!check_dev_role_integrity(cfg, dev_role)) {
		err = -ENODEV;
		goto fail;
	}
#endif

	if ((dev_role == NL80211_IFTYPE_P2P_GO) && (cfg->p2p_wdev == NULL)) {
		WL_ERR(("P2P already down status!\n"));
		err = BCME_ERROR;
		goto fail;
	}

	ie_offset = DOT11_MGMT_HDR_LEN + DOT11_BCN_PRB_FIXED_LEN;
	/* find the SSID */
	if ((ssid_ie = bcm_parse_tlvs((u8 *)&info->head[ie_offset],
		info->head_len - ie_offset,
		DOT11_MNG_SSID_ID)) != NULL) {
		if (dev_role == NL80211_IFTYPE_AP) {
			/* Store the hostapd SSID */
#ifdef BCM_PATCH_CVE_2016_2493
			memset(&cfg->hostapd_ssid.SSID[0], 0x00, DOT11_MAX_SSID_LEN);
			cfg->hostapd_ssid.SSID_len = MIN(ssid_ie->len, DOT11_MAX_SSID_LEN);
			memcpy(&cfg->hostapd_ssid.SSID[0], ssid_ie->data,
				cfg->hostapd_ssid.SSID_len);
#else
			memset(&cfg->hostapd_ssid.SSID[0], 0x00, 32);
			memcpy(&cfg->hostapd_ssid.SSID[0], ssid_ie->data, ssid_ie->len);
			cfg->hostapd_ssid.SSID_len = ssid_ie->len;
#endif
		} else {
				/* P2P GO */
#ifdef BCM_PATCH_CVE_2016_2493
			memset(&cfg->p2p->ssid.SSID[0], 0x00, DOT11_MAX_SSID_LEN);
			cfg->p2p->ssid.SSID_len = MIN(ssid_ie->len, DOT11_MAX_SSID_LEN);
			memcpy(cfg->p2p->ssid.SSID, ssid_ie->data,
				cfg->p2p->ssid.SSID_len);
#else
			memset(&cfg->p2p->ssid.SSID[0], 0x00, 32);
			memcpy(cfg->p2p->ssid.SSID, ssid_ie->data, ssid_ie->len);
			cfg->p2p->ssid.SSID_len = ssid_ie->len;
#endif
		}
	}

	if (wl_cfg80211_parse_ies((u8 *)info->tail,
		info->tail_len, &ies) < 0) {
		WL_ERR(("Beacon get IEs failed \n"));
		err = -EINVAL;
		goto fail;
	}
#ifndef  BRCM_RSDB
	if (wl_cfgp2p_set_management_ie(cfg, dev, bssidx,
		VNDR_IE_BEACON_FLAG, (u8 *)info->tail,
		info->tail_len) < 0) {
		WL_ERR(("Beacon set IEs failed \n"));
		goto fail;
	} else {
		WL_DBG(("Applied Vndr IEs for Beacon \n"));
	}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 2, 0))
	if (wl_cfgp2p_set_management_ie(cfg, dev, bssidx,
		VNDR_IE_PRBRSP_FLAG, (u8 *)info->proberesp_ies,
		info->proberesp_ies_len) < 0) {
		WL_ERR(("ProbeRsp set IEs failed \n"));
		goto fail;
	} else {
		WL_DBG(("Applied Vndr IEs for ProbeRsp \n"));
	}
#endif

	if (!wl_cfgp2p_bss_isup(dev, bssidx) &&
		(wl_cfg80211_bcn_validate_sec(dev, &ies, dev_role, bssidx) < 0))
	{
		WL_ERR(("Beacon set security failed \n"));
		goto fail;
	}
#else
	if ((err = wl_cfg80211_set_mgmt_vndr_ies(cfg, ndev_to_cfgdev(dev), bssidx,
		VNDR_IE_BEACON_FLAG, (u8 *)info->tail,
		info->tail_len)) < 0) {
		WL_ERR(("Beacon set IEs failed \n"));
		goto fail;
	} else {
		WL_DBG(("Applied Vndr IEs for Beacon \n"));
	}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 2, 0))
	if ((err = wl_cfg80211_set_mgmt_vndr_ies(cfg, ndev_to_cfgdev(dev), bssidx,
		VNDR_IE_PRBRSP_FLAG, (u8 *)info->proberesp_ies,
		info->proberesp_ies_len)) < 0) {
		WL_ERR(("ProbeRsp set IEs failed \n"));
		goto fail;
	} else {
		WL_DBG(("Applied Vndr IEs for ProbeRsp \n"));
	}
#endif

	is_bss_up = wl_cfgp2p_bss_isup(dev, bssidx);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 2, 0))
	privacy = info->privacy;
#else
	privacy = 0;
#endif
	if (!is_bss_up &&
		(wl_cfg80211_bcn_validate_sec(dev, &ies, dev_role, bssidx, privacy) < 0))
	{
		WL_ERR(("Beacon set security failed \n"));
		err = -EINVAL;
		goto fail;
	}
#endif /* BRCM_RSDB */
	/* Set BI and DTIM period */
	if (info->interval) {
		if ((err = wldev_ioctl(dev, WLC_SET_BCNPRD,
			&info->interval, sizeof(s32), true)) < 0) {
			WL_ERR(("Beacon Interval Set Error, %d\n", err));
			return err;
		}
	}
	if (info->dtim_period) {
		if ((err = wldev_ioctl(dev, WLC_SET_DTIMPRD,
			&info->dtim_period, sizeof(s32), true)) < 0) {
			WL_ERR(("DTIM Interval Set Error, %d\n", err));
			return err;
		}
	}

#ifndef  BRCM_RSDB
	if (wl_cfg80211_bcn_bringup_ap(dev, &ies, dev_role, bssidx) < 0) {
		WL_ERR(("Beacon bring up AP/GO failed \n"));
		goto fail;
	}
#else
	/* If bss is already up, skip bring up */
	if (!is_bss_up &&
		(err = wl_cfg80211_bcn_bringup_ap(dev, &ies, dev_role, bssidx)) < 0)
	{
		WL_ERR(("Beacon bring up AP/GO failed \n"));
		goto fail;
	}

	/* Set GC/STA SCB expiry timings. */
	if ((err = wl_cfg80211_set_scb_timings(cfg, dev))) {
		WL_ERR(("scb setting failed \n"));
		goto fail;
	}
#endif

	if (wl_get_drv_status(cfg, AP_CREATED, dev)) {
		/* Soft AP already running. Update changed params */
		if (wl_cfg80211_hostapd_sec(dev, &ies, bssidx) < 0) {
			WL_ERR(("Hostapd update sec failed \n"));
			err = -EINVAL;
			goto fail;
		}
	}

	/* Enable Probe Req filter */
	if (((dev_role == NL80211_IFTYPE_P2P_GO) ||
		(dev_role == NL80211_IFTYPE_AP)) && (ies.wps_ie != NULL)) {
		wl_validate_wps_ie((char *) ies.wps_ie, ies.wps_ie_len, &pbc);
		if (pbc)
			wl_add_remove_eventmsg(dev, WLC_E_PROBREQ_MSG, true);
	}

	WL_DBG(("** ADD/SET beacon done **\n"));

fail:
	if (err) {
		WL_ERR(("ADD/SET beacon failed\n"));
		wldev_iovar_setint(dev, "mpc", 1);
#ifdef  BRCM_RSDB
		if (dev_role == NL80211_IFTYPE_AP) {
			/* clear the AP mode */
			dhd->op_mode &= ~DHD_FLAG_HOSTAP_MODE;
		}
#endif
	}
	return err;

}
#endif

#ifdef WL_SCHED_SCAN
#define PNO_TIME		30
#define PNO_REPEAT		4
#define PNO_FREQ_EXPO_MAX	2
static bool
is_ssid_in_list(struct cfg80211_ssid *ssid, struct cfg80211_ssid *ssid_list, int count)
{
	int i;

	if (!ssid || !ssid_list)
		return FALSE;

	for (i = 0; i < count; i++) {
		if (ssid->ssid_len == ssid_list[i].ssid_len) {
			if (strncmp(ssid->ssid, ssid_list[i].ssid, ssid->ssid_len) == 0)
				return TRUE;
		}
	}
	return FALSE;
}

static int
wl_cfg80211_sched_scan_start(struct wiphy *wiphy,
                             struct net_device *dev,
                             struct cfg80211_sched_scan_request *request)
{
	ushort pno_time = PNO_TIME;
	int pno_repeat = PNO_REPEAT;
	int pno_freq_expo_max = PNO_FREQ_EXPO_MAX;
	wlc_ssid_ext_t ssids_local[MAX_PFN_LIST_COUNT];
	struct bcm_cfg80211 *cfg = wiphy_priv(wiphy);
	struct cfg80211_ssid *ssid = NULL;
	struct cfg80211_ssid *hidden_ssid_list = NULL;
	int ssid_cnt = 0;
	int i;
	int ret = 0;
#ifdef  BRCM_RSDB
	if (!request) {
		WL_ERR(("Sched scan request was NULL\n"));
		return -EINVAL;
	}
#endif
	WL_DBG(("Enter \n"));
#ifndef  BRCM_RSDB
	WL_ERR((">>> SCHED SCAN START\n"));
#else
	WL_PNO((">>> SCHED SCAN START\n"));
#endif
	WL_PNO(("Enter n_match_sets:%d   n_ssids:%d \n",
		request->n_match_sets, request->n_ssids));
	WL_PNO(("ssids:%d pno_time:%d pno_repeat:%d pno_freq:%d \n",
		request->n_ssids, pno_time, pno_repeat, pno_freq_expo_max));

#ifndef  BRCM_RSDB
	if (!request || !request->n_ssids || !request->n_match_sets) {
#else
	if (!request->n_ssids || !request->n_match_sets) {
#endif
		WL_ERR(("Invalid sched scan req!!"));
		return -EINVAL;
	}

	memset(&ssids_local, 0, sizeof(ssids_local));

	if (request->n_ssids > 0)
		hidden_ssid_list = request->ssids;

	for (i = 0; i < request->n_match_sets && ssid_cnt < MAX_PFN_LIST_COUNT; i++) {
		ssid = &request->match_sets[i].ssid;
		/* No need to include null ssid */
		if (ssid->ssid_len) {
#ifdef BCM_PATCH_CVE_2016_2493
			ssids_local[ssid_cnt].SSID_len = MIN(ssid->ssid_len,
				(uint32)DOT11_MAX_SSID_LEN);
			memcpy(ssids_local[ssid_cnt].SSID, ssid->ssid,
				ssids_local[ssid_cnt].SSID_len);
#else
			memcpy(ssids_local[ssid_cnt].SSID, ssid->ssid, ssid->ssid_len);
			ssids_local[ssid_cnt].SSID_len = ssid->ssid_len;
#endif
			if (is_ssid_in_list(ssid, hidden_ssid_list, request->n_ssids)) {
				ssids_local[ssid_cnt].hidden = TRUE;
				WL_PNO((">>> PNO hidden SSID (%s) \n", ssid->ssid));
			} else {
				ssids_local[ssid_cnt].hidden = FALSE;
				WL_PNO((">>> PNO non-hidden SSID (%s) \n", ssid->ssid));
			}
#ifndef  BRCM_RSDB
			if (request->match_sets[i].rssi_thold != NL80211_SCAN_RSSI_THOLD_OFF) {
				ssids_local[ssid_cnt].rssi_thresh =
				      (int8)request->match_sets[i].rssi_thold;
			}
#endif
			ssid_cnt++;
		}
	}

	if (ssid_cnt) {
		if ((ret = dhd_dev_pno_set_for_ssid(dev, ssids_local, ssid_cnt, pno_time,
		        pno_repeat, pno_freq_expo_max, NULL, 0)) < 0) {
			WL_ERR(("PNO setup failed!! ret=%d \n", ret));
			return -EINVAL;
		}
		cfg->sched_scan_req = request;
	} else {
		return -EINVAL;
	}

	return 0;
}

static int
wl_cfg80211_sched_scan_stop(struct wiphy *wiphy, struct net_device *dev)
{
	struct bcm_cfg80211 *cfg = wiphy_priv(wiphy);

	WL_DBG(("Enter \n"));
#ifndef  BRCM_RSDB
	WL_ERR((">>> SCHED SCAN STOP\n"));
#else
	WL_PNO((">>> SCHED SCAN STOP\n"));
#endif
	if (dhd_dev_pno_stop_for_ssid(dev) < 0)
		WL_ERR(("PNO Stop for SSID failed"));

	if (cfg->scan_request && cfg->sched_scan_running) {
		WL_PNO((">>> Sched scan running. Aborting it..\n"));
		wl_notify_escan_complete(cfg, dev, true, true);
	}

	 cfg->sched_scan_req = NULL;
	 cfg->sched_scan_running = FALSE;

	return 0;
}
#endif /* WL_SCHED_SCAN */

#ifdef WL_SUPPORT_ACS
/*
 * Currently the dump_obss IOVAR is returning string as output so we need to
 * parse the output buffer in an unoptimized way. Going forward if we get the
 * IOVAR output in binary format this method can be optimized
 */
static int wl_parse_dump_obss(char *buf, struct wl_dump_survey *survey)
{
	int i;
	char *token;
	char delim[] = " \n";

	token = strsep(&buf, delim);
	while (token != NULL) {
		if (!strcmp(token, "OBSS")) {
			for (i = 0; i < OBSS_TOKEN_IDX; i++)
				token = strsep(&buf, delim);
			survey->obss = simple_strtoul(token, NULL, 10);
		}

		if (!strcmp(token, "IBSS")) {
			for (i = 0; i < IBSS_TOKEN_IDX; i++)
				token = strsep(&buf, delim);
			survey->ibss = simple_strtoul(token, NULL, 10);
		}

		if (!strcmp(token, "TXDur")) {
			for (i = 0; i < TX_TOKEN_IDX; i++)
				token = strsep(&buf, delim);
			survey->tx = simple_strtoul(token, NULL, 10);
		}

		if (!strcmp(token, "Category")) {
			for (i = 0; i < CTG_TOKEN_IDX; i++)
				token = strsep(&buf, delim);
			survey->no_ctg = simple_strtoul(token, NULL, 10);
		}

		if (!strcmp(token, "Packet")) {
			for (i = 0; i < PKT_TOKEN_IDX; i++)
				token = strsep(&buf, delim);
			survey->no_pckt = simple_strtoul(token, NULL, 10);
		}

		if (!strcmp(token, "Opp(time):")) {
			for (i = 0; i < IDLE_TOKEN_IDX; i++)
				token = strsep(&buf, delim);
			survey->idle = simple_strtoul(token, NULL, 10);
		}

		token = strsep(&buf, delim);
	}

	return 0;
}

static int wl_dump_obss(struct net_device *ndev, cca_msrmnt_query req,
	struct wl_dump_survey *survey)
{
	cca_stats_n_flags *results;
	char *buf;
	int retry, err;

	buf = kzalloc(sizeof(char) * WLC_IOCTL_MAXLEN, GFP_KERNEL);
	if (unlikely(!buf)) {
		WL_ERR(("%s: buf alloc failed\n", __func__));
		return -ENOMEM;
	}

	retry = IOCTL_RETRY_COUNT;
	while (retry--) {
		err = wldev_iovar_getbuf(ndev, "dump_obss", &req, sizeof(req),
			buf, WLC_IOCTL_MAXLEN, NULL);
		if (err >=  0) {
			break;
		}
		WL_DBG(("attempt = %d, err = %d, \n",
			(IOCTL_RETRY_COUNT - retry), err));
	}

	if (retry <= 0)	{
		WL_ERR(("failure, dump_obss IOVAR failed\n"));
#ifndef  BRCM_RSDB
		err = -BCME_ERROR;
#else
		err = -EINVAL;
#endif
		goto exit;
	}

	results = (cca_stats_n_flags *)(buf);
	wl_parse_dump_obss(results->buf, survey);
	kfree(buf);

	return 0;
exit:
	kfree(buf);
	return err;
}

static int wl_cfg80211_dump_survey(struct wiphy *wiphy, struct net_device *ndev,
	int idx, struct survey_info *info)
{
	struct bcm_cfg80211 *cfg = wiphy_priv(wiphy);
	struct wl_dump_survey *survey;
	struct ieee80211_supported_band *band;
	struct ieee80211_channel*chan;
	cca_msrmnt_query req;
	int val, err, noise = 0, retry;

	dhd_pub_t *dhd = (dhd_pub_t *)(cfg->pub);
	if (!(dhd->op_mode & DHD_FLAG_HOSTAP_MODE)) {
		return -ENOENT;
	}
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0))
	band = wiphy->bands[NL80211_BAND_2GHZ];
#else
	band = wiphy->bands[IEEE80211_BAND_2GHZ];
#endif
	if (band && idx >= band->n_channels) {
		idx -= band->n_channels;
		band = NULL;
	}

	if (!band || idx >= band->n_channels) {
		/* Move to 5G band */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0))
		band = wiphy->bands[NL80211_BAND_5GHZ];
#else
		band = wiphy->bands[IEEE80211_BAND_5GHZ];
#endif
		if (idx >= band->n_channels) {
			return -ENOENT;
		}
	}

	chan = &band->channels[idx];
	/* Setting current channel to the requested channel */
	if ((err = wl_cfg80211_set_channel(wiphy, ndev, chan,
		NL80211_CHAN_HT20) < 0)) {
		WL_ERR(("Set channel failed \n"));
	}

	if (!idx) {
		/* Disable mpc */
		val = 0;
		err = wldev_iovar_setbuf_bsscfg(ndev, "mpc", (void *)&val,
			sizeof(val), cfg->ioctl_buf, WLC_IOCTL_SMLEN, 0,
			&cfg->ioctl_buf_sync);
		if (err < 0) {
			WL_ERR(("set 'mpc' failed, error = %d\n", err));
		}

		/* Set interface up, explicitly. */
		val = 1;
		err = wldev_ioctl(ndev, WLC_UP, (void *)&val, sizeof(val), true);
		if (err < 0) {
			WL_ERR(("set interface up failed, error = %d\n", err));
		}
	}

	/* Get noise value */
	retry = IOCTL_RETRY_COUNT;
	while (retry--) {
		err = wldev_ioctl(ndev, WLC_GET_PHY_NOISE, &noise,
			sizeof(noise), false);
		if (err >=  0) {
			break;
		}
		WL_DBG(("attempt = %d, err = %d, \n",
			(IOCTL_RETRY_COUNT - retry), err));
	}

	if (retry <= 0)	{
		WL_ERR(("Get Phy Noise failed, error = %d\n", err));
		noise = CHAN_NOISE_DUMMY;
	}

	survey = (struct wl_dump_survey *) kzalloc(sizeof(struct wl_dump_survey),
		GFP_KERNEL);
	if (unlikely(!survey)) {
		WL_ERR(("%s: alloc failed\n", __func__));
		return -ENOMEM;
	}

	/* Start Measurement for obss stats on current channel */
	req.msrmnt_query = 0;
	req.time_req = ACS_MSRMNT_DELAY;
	if ((err = wl_dump_obss(ndev, req, survey)) < 0) {
		goto exit;
	}

	/*
	 * Wait for the meaurement to complete, adding a buffer value of 10 to take
	 * into consideration any delay in IOVAR completion
	 */
	msleep(ACS_MSRMNT_DELAY + 10);

	/* Issue IOVAR to collect measurement results */
	req.msrmnt_query = 1;
	if ((err = wl_dump_obss(ndev, req, survey)) < 0) {
		goto exit;
	}

	info->channel = chan;
	info->noise = noise;
	info->channel_time = ACS_MSRMNT_DELAY;
	info->channel_time_busy = ACS_MSRMNT_DELAY - survey->idle;
	info->channel_time_rx = survey->obss + survey->ibss + survey->no_ctg +
		survey->no_pckt;
	info->channel_time_tx = survey->tx;
	info->filled = SURVEY_INFO_NOISE_DBM |SURVEY_INFO_CHANNEL_TIME |
		SURVEY_INFO_CHANNEL_TIME_BUSY |	SURVEY_INFO_CHANNEL_TIME_RX |
		SURVEY_INFO_CHANNEL_TIME_TX;
	kfree(survey);

	return 0;
exit:
	kfree(survey);
	return err;
}
#endif /* WL_SUPPORT_ACS */

static struct cfg80211_ops wl_cfg80211_ops = {
	.add_virtual_intf = wl_cfg80211_add_virtual_iface,
	.del_virtual_intf = wl_cfg80211_del_virtual_iface,
	.change_virtual_intf = wl_cfg80211_change_virtual_iface,
#if defined(WL_CFG80211_P2P_DEV_IF)
	.start_p2p_device = wl_cfgp2p_start_p2p_device,
	.stop_p2p_device = wl_cfgp2p_stop_p2p_device,
#endif /* WL_CFG80211_P2P_DEV_IF */
#ifdef CONFIG_HW_GET_P2P_TX_RATE
	.get_p2p_tx_rate = wl_cfg80211_get_p2p_tx_rate,
#endif /* CONFIG_HW_GET_P2P_TX_RATE */
	.scan = wl_cfg80211_scan,
	.set_wiphy_params = wl_cfg80211_set_wiphy_params,
	.join_ibss = wl_cfg80211_join_ibss,
	.leave_ibss = wl_cfg80211_leave_ibss,
	.get_station = wl_cfg80211_get_station,
	.set_tx_power = wl_cfg80211_set_tx_power,
	.get_tx_power = wl_cfg80211_get_tx_power,
	.add_key = wl_cfg80211_add_key,
	.del_key = wl_cfg80211_del_key,
	.get_key = wl_cfg80211_get_key,
	.set_default_key = wl_cfg80211_config_default_key,
	.set_default_mgmt_key = wl_cfg80211_config_default_mgmt_key,
	.set_power_mgmt = wl_cfg80211_set_power_mgmt,
	.connect = wl_cfg80211_connect,
	.disconnect = wl_cfg80211_disconnect,
	.suspend = wl_cfg80211_suspend,
	.resume = wl_cfg80211_resume,
	.set_pmksa = wl_cfg80211_set_pmksa,
	.del_pmksa = wl_cfg80211_del_pmksa,
	.flush_pmksa = wl_cfg80211_flush_pmksa,
	.remain_on_channel = wl_cfg80211_remain_on_channel,
	.cancel_remain_on_channel = wl_cfg80211_cancel_remain_on_channel,
	.mgmt_tx = wl_cfg80211_mgmt_tx,
	.mgmt_frame_register = wl_cfg80211_mgmt_frame_register,
	.change_bss = wl_cfg80211_change_bss,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 6, 0)) || defined(WL_COMPAT_WIRELESS)
	.set_channel = wl_cfg80211_set_channel,
#endif /* ((LINUX_VERSION < VERSION(3, 6, 0)) || WL_COMPAT_WIRELESS */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 4, 0)) && !defined(WL_COMPAT_WIRELESS)
	.set_beacon = wl_cfg80211_add_set_beacon,
	.add_beacon = wl_cfg80211_add_set_beacon,
#else
	.change_beacon = wl_cfg80211_change_beacon,
	.start_ap = wl_cfg80211_start_ap,
	.stop_ap = wl_cfg80211_stop_ap,
#endif /* LINUX_VERSION < KERNEL_VERSION(3,4,0) && !WL_COMPAT_WIRELESS */
#ifdef WL_SCHED_SCAN
	.sched_scan_start = wl_cfg80211_sched_scan_start,
	.sched_scan_stop = wl_cfg80211_sched_scan_stop,
#endif /* WL_SCHED_SCAN */
#if defined(WL_SUPPORT_BACKPORTED_KPATCHES) || (LINUX_VERSION_CODE >= KERNEL_VERSION(3, \
	2, 0))
	.del_station = wl_cfg80211_del_station,
	.change_station = wl_cfg80211_change_station,
	.mgmt_tx_cancel_wait = wl_cfg80211_mgmt_tx_cancel_wait,
#endif /* WL_SUPPORT_BACKPORTED_KPATCHES || KERNEL_VERSION >= (3,2,0) */
#if (LINUX_VERSION_CODE > KERNEL_VERSION(3, 2, 0)) || defined(WL_COMPAT_WIRELESS)
#ifdef  BRCM_RSDB
	.tdls_mgmt = wl_cfg80211_tdls_mgmt,
#endif
	.tdls_oper = wl_cfg80211_tdls_oper,
#endif
#ifdef WL_SUPPORT_ACS
	.dump_survey = wl_cfg80211_dump_survey,
#endif /* WL_SUPPORT_ACS */
#ifdef WL_CFG80211_ACL
	.set_mac_acl = wl_cfg80211_set_mac_acl,
#endif /* WL_CFG80211_ACL */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,5,0)) || defined(BCM_PATCH_ABORT_SCAN)
	.abort_scan = wl_cfg80211_abort_scan,
#endif /*(LINUX_VERSION_CODE >= KERNEL_VERSION(4,5,0)) || defined(BCM_PATCH_ABORT_SCAN) */
};

s32 wl_mode_to_nl80211_iftype(s32 mode)
{
	s32 err = 0;

	switch (mode) {
	case WL_MODE_BSS:
		return NL80211_IFTYPE_STATION;
	case WL_MODE_IBSS:
		return NL80211_IFTYPE_ADHOC;
	case WL_MODE_AP:
		return NL80211_IFTYPE_AP;
	default:
		return NL80211_IFTYPE_UNSPECIFIED;
	}

	return err;
}
#ifndef  BRCM_RSDB
#ifdef CONFIG_CFG80211_INTERNAL_REGDB
static void
wl_cfg80211_reg_notifier(
	struct wiphy *wiphy,
	struct regulatory_request *request)
{
	struct bcm_cfg80211 *cfg = (struct bcm_cfg80211 *)wiphy_priv(wiphy);
	int ret = 0;

	if (!request || !cfg) {
		WL_ERR(("Invalid arg\n"));
		return ;
	}

	WL_DBG(("ccode: %c%c Initiator: %d\n",
		request->alpha2[0], request->alpha2[1], request->initiator));

	/* We support only REGDOM_SET_BY_USER as of now */
	if ((request->initiator != NL80211_REGDOM_SET_BY_USER) &&
		(request->initiator != NL80211_REGDOM_SET_BY_COUNTRY_IE)) {
		WL_ERR(("reg_notifier for intiator:%d not supported : set default\n",
			request->initiator));
		/* in case of no supported country by regdb
		     lets driver setup platform default Locale
		*/
	}

	WL_ERR(("Set country code %c%c from %s\n",
		request->alpha2[0], request->alpha2[1],
		((request->initiator == NL80211_REGDOM_SET_BY_COUNTRY_IE) ? " 11d AP" : "User")));

	if ((ret = wldev_set_country(bcmcfg_to_prmry_ndev(cfg), request->alpha2,
		false, (request->initiator == NL80211_REGDOM_SET_BY_USER ? true : false))) < 0) {
		WL_ERR(("set country Failed :%d\n", ret));
	}

	return ;
}
#endif /* CONFIG_CFG80211_INTERNAL_REGDB */
#endif /* BRCM_RSDB */
#ifdef CONFIG_PM
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 6, 0))
static const struct wiphy_wowlan_support brcm_wowlan_support = {
	.flags = WIPHY_WOWLAN_ANY,
};
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(3, 6, 0) */
#ifdef  BRCM_RSDB
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 11, 0))

#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(3, 11, 0) */
#endif /* BRCM_RSDB */
#endif /* CONFIG_PM */

static s32 wl_setup_wiphy(struct wireless_dev *wdev, struct device *sdiofunc_dev, void *context)
{
	s32 err = 0;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 4, 0) || defined(WL_COMPAT_WIRELESS))
	dhd_pub_t *dhd = (dhd_pub_t *)context;
	BCM_REFERENCE(dhd);

	if (!dhd) {
		WL_ERR(("DHD is NULL!!"));
		err = -ENODEV;
		return err;
	}
#endif

	wdev->wiphy =
	    wiphy_new(&wl_cfg80211_ops, sizeof(struct bcm_cfg80211));
	if (unlikely(!wdev->wiphy)) {
		WL_ERR(("Couldn not allocate wiphy device\n"));
		err = -ENOMEM;
		return err;
	}
	set_wiphy_dev(wdev->wiphy, sdiofunc_dev);
	wdev->wiphy->max_scan_ie_len = WL_SCAN_IE_LEN_MAX;
	/* Report  how many SSIDs Driver can support per Scan request */
	wdev->wiphy->max_scan_ssids = WL_SCAN_PARAMS_SSID_MAX;
	wdev->wiphy->max_num_pmkids = WL_NUM_PMKIDS_MAX;
#ifdef WL_SCHED_SCAN
	wdev->wiphy->max_sched_scan_ssids = MAX_PFN_LIST_COUNT;
	wdev->wiphy->max_match_sets = MAX_PFN_LIST_COUNT;
	wdev->wiphy->max_sched_scan_ie_len = WL_SCAN_IE_LEN_MAX;
	wdev->wiphy->flags |= WIPHY_FLAG_SUPPORTS_SCHED_SCAN;
#endif /* WL_SCHED_SCAN */
	wdev->wiphy->interface_modes =
		BIT(NL80211_IFTYPE_STATION)
		| BIT(NL80211_IFTYPE_ADHOC)
#if !defined(WL_ENABLE_P2P_IF) && !defined(WL_CFG80211_P2P_DEV_IF)
		| BIT(NL80211_IFTYPE_MONITOR)
#endif /* !WL_ENABLE_P2P_IF && !WL_CFG80211_P2P_DEV_IF */
#if defined(WL_IFACE_COMB_NUM_CHANNELS) || defined(WL_CFG80211_P2P_DEV_IF)
		| BIT(NL80211_IFTYPE_P2P_CLIENT)
		| BIT(NL80211_IFTYPE_P2P_GO)
#endif /* WL_IFACE_COMB_NUM_CHANNELS || WL_CFG80211_P2P_DEV_IF */
#if defined(WL_CFG80211_P2P_DEV_IF)
		| BIT(NL80211_IFTYPE_P2P_DEVICE)
#endif /* WL_CFG80211_P2P_DEV_IF */
		| BIT(NL80211_IFTYPE_AP);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 0, 0)) && \
	(defined(WL_IFACE_COMB_NUM_CHANNELS) || defined(WL_CFG80211_P2P_DEV_IF))
	WL_DBG(("Setting interface combinations for common mode\n"));
	wdev->wiphy->iface_combinations = common_iface_combinations;
	wdev->wiphy->n_iface_combinations =
		ARRAY_SIZE(common_iface_combinations);
#endif /* LINUX_VER >= 3.0 && (WL_IFACE_COMB_NUM_CHANNELS || WL_CFG80211_P2P_DEV_IF) */

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0))
	wdev->wiphy->bands[NL80211_BAND_2GHZ] = &__wl_band_2ghz;
#else
	wdev->wiphy->bands[IEEE80211_BAND_2GHZ] = &__wl_band_2ghz;
#endif

	wdev->wiphy->signal_type = CFG80211_SIGNAL_TYPE_MBM;
	wdev->wiphy->cipher_suites = __wl_cipher_suites;
	wdev->wiphy->n_cipher_suites = ARRAY_SIZE(__wl_cipher_suites);
	wdev->wiphy->max_remain_on_channel_duration = 5000;
	wdev->wiphy->mgmt_stypes = wl_cfg80211_default_mgmt_stypes;
#ifndef WL_POWERSAVE_DISABLED
	wdev->wiphy->flags |= WIPHY_FLAG_PS_ON_BY_DEFAULT;
#else
	wdev->wiphy->flags &= ~WIPHY_FLAG_PS_ON_BY_DEFAULT;
#endif				/* !WL_POWERSAVE_DISABLED */
	wdev->wiphy->flags |= WIPHY_FLAG_NETNS_OK |
		WIPHY_FLAG_4ADDR_AP |
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 39)) && !defined(WL_COMPAT_WIRELESS)
		WIPHY_FLAG_SUPPORTS_SEPARATE_DEFAULT_KEYS |
#endif
		WIPHY_FLAG_4ADDR_STATION;
#if ((defined(ROAM_ENABLE) || defined(BCMFW_ROAM_ENABLE)) && ((LINUX_VERSION_CODE >= \
	KERNEL_VERSION(3, 2, 0)) || defined(WL_COMPAT_WIRELESS)))
	/*
	 * If FW ROAM flag is advertised, upper layer wouldn't provide
	 * the bssid & freq in the connect command. This will result a
	 * delay in initial connection time due to firmware doing a full
	 * channel scan to figure out the channel & bssid. However kernel
	 * ver >= 3.15, provides bssid_hint & freq_hint and hence kernel
	 * ver >= 3.15 won't have any issue. So if this flags need to be
	 * advertised for kernel < 3.15, suggest to use RCC along with it
	 * to avoid the initial connection delay.
	 */
	wdev->wiphy->flags |= WIPHY_FLAG_SUPPORTS_FW_ROAM;
#endif
#ifdef  BRCM_RSDB
#ifdef UNSET_FW_ROAM_WIPHY_FLAG
	wdev->wiphy->flags &= ~WIPHY_FLAG_SUPPORTS_FW_ROAM;
#endif /* UNSET_FW_ROAM_WIPHY_FLAG */
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 3, 0)) || defined(WL_COMPAT_WIRELESS)
	wdev->wiphy->flags |= WIPHY_FLAG_HAS_REMAIN_ON_CHANNEL |
		WIPHY_FLAG_OFFCHAN_TX;
#endif
#if defined(WL_SUPPORT_BACKPORTED_KPATCHES) || (LINUX_VERSION_CODE >= KERNEL_VERSION(3, \
	4, 0))
	/* From 3.4 kernel ownards AP_SME flag can be advertised
	 * to remove the patch from supplicant
	 */
	wdev->wiphy->flags |= WIPHY_FLAG_HAVE_AP_SME;

#ifdef WL_CFG80211_ACL
	/* Configure ACL capabilities. */
	wdev->wiphy->max_acl_mac_addrs = MAX_NUM_MAC_FILT;
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 4, 0) || defined(WL_COMPAT_WIRELESS))
	/* Supplicant distinguish between the SoftAP mode and other
	 * modes (e.g. P2P, WPS, HS2.0) when it builds the probe
	 * response frame from Supplicant MR1 and Kernel 3.4.0 or
	 * later version. To add Vendor specific IE into the
	 * probe response frame in case of SoftAP mode,
	 * AP_PROBE_RESP_OFFLOAD flag is set to wiphy->flags variable.
	 */
	if (dhd_get_fw_mode(dhd->info) == DHD_FLAG_HOSTAP_MODE) {
		wdev->wiphy->flags |= WIPHY_FLAG_AP_PROBE_RESP_OFFLOAD;
		wdev->wiphy->probe_resp_offload = 0;
	}
#endif
#endif /* WL_SUPPORT_BACKPORTED_KPATCHES) || (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 4, 0)) */
#ifndef  BRCM_RSDB
#ifdef CONFIG_CFG80211_INTERNAL_REGDB
	wdev->wiphy->reg_notifier = wl_cfg80211_reg_notifier;
#endif /* CONFIG_CFG80211_INTERNAL_REGDB */
#endif
#if (LINUX_VERSION_CODE > KERNEL_VERSION(3, 2, 0)) || defined(WL_COMPAT_WIRELESS)
	wdev->wiphy->flags |= WIPHY_FLAG_SUPPORTS_TDLS;
#endif

#if defined(CONFIG_PM) && defined(WL_CFG80211_P2P_DEV_IF)
	/*
	 * From linux-3.10 kernel, wowlan packet filter is mandated to avoid the
	 * disconnection of connected network before suspend. So a dummy wowlan
	 * filter is configured for kernels linux-3.8 and above.
	 */

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 11, 0))
	wdev->wiphy->wowlan = &brcm_wowlan_support;
#else
	wdev->wiphy->wowlan.flags = WIPHY_WOWLAN_ANY;
#ifdef  BRCM_RSDB
	wdev->wiphy->wowlan.n_patterns = WL_WOWLAN_MAX_PATTERNS;
	wdev->wiphy->wowlan.pattern_min_len = WL_WOWLAN_MIN_PATTERN_LEN;
	wdev->wiphy->wowlan.pattern_max_len = WL_WOWLAN_MAX_PATTERN_LEN;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0))
	wdev->wiphy->wowlan.max_pkt_offset = WL_WOWLAN_MAX_PATTERN_LEN;
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0) */
#endif /* BRCM_RSDB */
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(3, 11, 10) */
#endif /* CONFIG_PM && WL_CFG80211_P2P_DEV_IF */

	WL_DBG(("Registering custom regulatory)\n"));
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0))
	wdev->wiphy->regulatory_flags |= REGULATORY_CUSTOM_REG;
#else
	wdev->wiphy->flags |= WIPHY_FLAG_CUSTOM_REGULATORY;
#endif
	wiphy_apply_custom_regulatory(wdev->wiphy, &brcm_regdom);

#if defined(WL_VENDOR_EXT_SUPPORT)
	WL_ERR(("Registering Vendor80211\n"));
#ifndef  BRCM_RSDB
	err = wl_cfgvendor_attach(wdev->wiphy, dhd);
#else
	err = wl_cfgvendor_attach(wdev->wiphy);
#endif
	if (unlikely(err < 0)) {
		WL_ERR(("Couldn not attach vendor commands (%d)\n", err));
	}
#endif /* (LINUX_VERSION_CODE > KERNEL_VERSION(3, 13, 0)) || defined(WL_VENDOR_EXT_SUPPORT) */


	/* Now we can register wiphy with cfg80211 module */
	err = wiphy_register(wdev->wiphy);
	if (unlikely(err < 0)) {
		WL_ERR(("Couldn not register wiphy device (%d)\n", err));
		wiphy_free(wdev->wiphy);
	}

#if ((LINUX_VERSION_CODE >= KERNEL_VERSION(3, 0, 0)) && (LINUX_VERSION_CODE <= \
	KERNEL_VERSION(3, 3, 0))) && defined(WL_IFACE_COMB_NUM_CHANNELS)
	wdev->wiphy->flags &= ~WIPHY_FLAG_ENFORCE_COMBINATIONS;
#endif

	return err;
}

static void wl_free_wdev(struct bcm_cfg80211 *cfg)
{
	struct wireless_dev *wdev = cfg->wdev;
	struct wiphy *wiphy = NULL;
	if (!wdev) {
		WL_ERR(("wdev is invalid\n"));
		return;
	}
#ifndef  BRCM_RSDB
	wiphy = wdev->wiphy;

#if (LINUX_VERSION_CODE > KERNEL_VERSION(3, 13, 0)) || defined(WL_VENDOR_EXT_SUPPORT)
	wl_cfgvendor_detach(wdev->wiphy);
#endif /* if (LINUX_VERSION_CODE > KERNEL_VERSION(3, 13, 0)) || defined(WL_VENDOR_EXT_SUPPORT) */

	wiphy_unregister(wdev->wiphy);
	wdev->wiphy->dev.parent = NULL;

	wl_delete_all_netinfo(cfg);
	wiphy_free(wiphy);
#else
	if (wdev->wiphy) {
		wiphy = wdev->wiphy;

#if defined(WL_VENDOR_EXT_SUPPORT)
		wl_cfgvendor_detach(wdev->wiphy);
#endif /* if defined(WL_VENDOR_EXT_SUPPORT) */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 11, 0))
		/* Reset wowlan & wowlan_config before Unregister to avoid  Kernel Panic */
		WL_DBG(("wl_free_wdev Clearing wowlan Config \n"));
		wdev->wiphy->wowlan = NULL;
		wdev->wiphy->wowlan_config = NULL;
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(3, 11, 0) */
		wiphy_unregister(wdev->wiphy);
		wdev->wiphy->dev.parent = NULL;
		wdev->wiphy = NULL;
	}

	wl_delete_all_netinfo(cfg);
	if (wiphy)
		wiphy_free(wiphy);
#endif /* BRCM_RSDB */
	/* PLEASE do NOT call any function after wiphy_free, the driver's private structure "cfg",
	 * which is the private part of wiphy, has been freed in wiphy_free !!!!!!!!!!!
	 */
}

static s32 wl_inform_bss(struct bcm_cfg80211 *cfg)
{
	struct wl_scan_results *bss_list;
	struct wl_bss_info *bi = NULL;	/* must be initialized */
	s32 err = 0;
	s32 i;
#ifdef HW_LOG_PATCH1
	s32 bss_idx = 0;
	s32 per_bss_line = 6;
#endif

	bss_list = cfg->bss_list;
#ifdef HW_LOG_PATCH1
	hw_dhd_log("scanned AP count (%d)", bss_list->count);
#else
	WL_DBG(("scanned AP count (%d)\n", bss_list->count));
#endif
#ifdef ROAM_CHANNEL_CACHE
#ifndef  BRCM_RSDB
	reset_roam_cache();
#else
	reset_roam_cache(cfg);
#endif
#endif /* ROAM_CHANNEL_CACHE */
	bi = next_bss(bss_list, bi);
	for_each_bss(bss_list, bi, i) {
#ifdef ROAM_CHANNEL_CACHE
	add_roam_cache(bi);
#endif /* ROAM_CHANNEL_CACHE */
#ifdef HW_LOG_PATCH1
	if (bss_idx % per_bss_line == 0) {
		if (bss_idx > 0) {
			hw_dhd_looplog_end();
		}
		hw_dhd_looplog_start();
	}
	bss_idx++;
#endif
		err = wl_inform_single_bss(cfg, bi, false);
		if (unlikely(err))
			break;
	}
#ifdef HW_LOG_PATCH1
	if (bss_idx % per_bss_line > 0) {
		hw_dhd_looplog_end();
	}
#endif
#ifdef ROAM_CHANNEL_CACHE
	/* print_roam_cache(); */
	update_roam_cache(cfg, ioctl_version);
#endif /* ROAM_CHANNEL_CACHE */
	return err;
}

static s32 wl_inform_single_bss(struct bcm_cfg80211 *cfg, struct wl_bss_info *bi, bool roam)
{
	struct wiphy *wiphy = bcmcfg_to_wiphy(cfg);
	struct ieee80211_mgmt *mgmt;
	struct ieee80211_channel *channel;
	struct ieee80211_supported_band *band;
	struct wl_cfg80211_bss_info *notif_bss_info;
	struct wl_scan_req *sr = wl_to_sr(cfg);
	struct beacon_proberesp *beacon_proberesp;
	struct cfg80211_bss *cbss = NULL;
	s32 mgmt_type;
	s32 signal;
	u32 freq;
	s32 err = 0;
	gfp_t aflags;

	if (unlikely(dtoh32(bi->length) > WL_BSS_INFO_MAX)) {
		WL_DBG(("Beacon is larger than buffer. Discarding\n"));
		return err;
	}
	aflags = (in_atomic()) ? GFP_ATOMIC : GFP_KERNEL;
	notif_bss_info = kzalloc(sizeof(*notif_bss_info) + sizeof(*mgmt)
		- sizeof(u8) + WL_BSS_INFO_MAX, aflags);
	if (unlikely(!notif_bss_info)) {
		WL_ERR(("notif_bss_info alloc failed\n"));
		return -ENOMEM;
	}
	mgmt = (struct ieee80211_mgmt *)notif_bss_info->frame_buf;
	notif_bss_info->channel =
		wf_chspec_ctlchan(wl_chspec_driver_to_host(bi->chanspec));

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0))
	if (notif_bss_info->channel <= CH_MAX_2G_CHANNEL)
		band = wiphy->bands[NL80211_BAND_2GHZ];
	else
		band = wiphy->bands[NL80211_BAND_5GHZ];
#else
	if (notif_bss_info->channel <= CH_MAX_2G_CHANNEL)
		band = wiphy->bands[IEEE80211_BAND_2GHZ];
	else
		band = wiphy->bands[IEEE80211_BAND_5GHZ];
#endif
	if (!band) {
		WL_ERR(("No valid band"));
		kfree(notif_bss_info);
		return -EINVAL;
	}
	notif_bss_info->rssi = wl_rssi_offset(dtoh16(bi->RSSI));
	memcpy(mgmt->bssid, &bi->BSSID, ETHER_ADDR_LEN);
	mgmt_type = cfg->active_scan ?
		IEEE80211_STYPE_PROBE_RESP : IEEE80211_STYPE_BEACON;
	if (!memcmp(bi->SSID, sr->ssid.SSID, bi->SSID_len)) {
	    mgmt->frame_control = cpu_to_le16(IEEE80211_FTYPE_MGMT | mgmt_type);
	}
	beacon_proberesp = cfg->active_scan ?
		(struct beacon_proberesp *)&mgmt->u.probe_resp :
		(struct beacon_proberesp *)&mgmt->u.beacon;
	beacon_proberesp->timestamp = 0;
	beacon_proberesp->beacon_int = cpu_to_le16(bi->beacon_period);
	beacon_proberesp->capab_info = cpu_to_le16(bi->capability);
	wl_rst_ie(cfg);
	wl_update_hidden_ap_ie(bi, ((u8 *) bi) + bi->ie_offset, &bi->ie_length, roam);
	wl_mrg_ie(cfg, ((u8 *) bi) + bi->ie_offset, bi->ie_length);
	wl_cp_ie(cfg, beacon_proberesp->variable, WL_BSS_INFO_MAX -
		offsetof(struct wl_cfg80211_bss_info, frame_buf));
	notif_bss_info->frame_len = offsetof(struct ieee80211_mgmt,
		u.beacon.variable) + wl_get_ielen(cfg);
#if LINUX_VERSION_CODE == KERNEL_VERSION(2, 6, 38) && !defined(WL_COMPAT_WIRELESS)
	freq = ieee80211_channel_to_frequency(notif_bss_info->channel);
	(void)band->band;
#else
	freq = ieee80211_channel_to_frequency(notif_bss_info->channel, band->band);
#endif
	if (freq == 0) {
		WL_ERR(("Invalid channel, fail to chcnage channel to freq\n"));
		kfree(notif_bss_info);
		return -EINVAL;
	}
	channel = ieee80211_get_channel(wiphy, freq);
	if (unlikely(!channel)) {
		WL_ERR(("ieee80211_get_channel error\n"));
		kfree(notif_bss_info);
		return -EINVAL;
	}
#ifdef HW_LOG_PATCH1
	hw_dhd_looplog("(%s,%d,%d,%pM);", bi->SSID,
			notif_bss_info->rssi, notif_bss_info->channel, &bi->BSSID);
#else
	WL_DBG(("SSID : \"%s\", rssi %d, channel %d, capability : 0x04%x, bssid %pM"
			"mgmt_type %d frame_len %d\n", bi->SSID,
			notif_bss_info->rssi, notif_bss_info->channel,
			mgmt->u.beacon.capab_info, &bi->BSSID, mgmt_type,
			notif_bss_info->frame_len));
#endif

	signal = notif_bss_info->rssi * 100;
	if (!mgmt->u.probe_resp.timestamp) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 39))
		struct timespec ts;
		get_monotonic_boottime(&ts);
		mgmt->u.probe_resp.timestamp = ((u64)ts.tv_sec*1000000)
				+ ts.tv_nsec / 1000;
#else
		struct timeval tv;
		do_gettimeofday(&tv);
		mgmt->u.probe_resp.timestamp = ((u64)tv.tv_sec*1000000)
				+ tv.tv_usec;
#endif
	}


	cbss = cfg80211_inform_bss_frame(wiphy, channel, mgmt,
		le16_to_cpu(notif_bss_info->frame_len), signal, aflags);
	if (unlikely(!cbss)) {
		WL_ERR(("cfg80211_inform_bss_frame error\n"));
		kfree(notif_bss_info);
		return -EINVAL;
	}
#ifdef  BRCM_RSDB
#ifdef  ROAM_CHANNEL_CACHE
	cbss->channel = channel;
#endif /* ROAM_CHANNEL_CACHE */
#endif /* BRCM_RSDB */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0))
	cfg80211_put_bss(wiphy, cbss);
#else
	cfg80211_put_bss(cbss);
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0) */
	kfree(notif_bss_info);
	return err;
}

static bool wl_is_linkup(struct bcm_cfg80211 *cfg, const wl_event_msg_t *e, struct net_device *ndev)
{
	u32 event = ntoh32(e->event_type);
	u32 status =  ntoh32(e->status);
	u16 flags = ntoh16(e->flags);

	WL_DBG(("event %d, status %d flags %x\n", event, status, flags));
	if (event == WLC_E_SET_SSID) {
		if (status == WLC_E_STATUS_SUCCESS) {
			if (!wl_is_ibssmode(cfg, ndev))
				return true;
		}
	} else if (event == WLC_E_LINK) {
		if (flags & WLC_EVENT_MSG_LINK)
			return true;
	}

	WL_DBG(("wl_is_linkup false\n"));
	return false;
}

static bool wl_is_linkdown(struct bcm_cfg80211 *cfg, const wl_event_msg_t *e)
{
	u32 event = ntoh32(e->event_type);
	u16 flags = ntoh16(e->flags);
#ifdef HW_READ_FW_LOG
	if (NULL != cfg)
		dhd_read_console_ex((dhd_pub_t *)cfg->pub);
#endif
	if (event == WLC_E_DEAUTH_IND ||
	event == WLC_E_DISASSOC_IND ||
	event == WLC_E_DISASSOC ||
	event == WLC_E_DEAUTH) {
#if (WL_DBG_LEVEL > 0)
#ifndef  BRCM_RSDB
	WL_ERR(("Link down Reason : WLC_E_%s reason = %d status = %d\n", wl_dbg_estr[event],
	    ntoh32(e->reason), ntoh32(e->status)));
#else
	WL_ERR(("Link down Reason : WLC_E_%s\n", wl_dbg_estr[event]));
#endif /* BRCM_RSDB */
#endif /* (WL_DBG_LEVEL > 0) */
		return true;
	} else if (event == WLC_E_LINK) {
		if (!(flags & WLC_EVENT_MSG_LINK)) {
#if (WL_DBG_LEVEL > 0)
	WL_ERR(("Link down Reason : WLC_E_%s\n", wl_dbg_estr[event]));
#endif /* (WL_DBG_LEVEL > 0) */
			return true;
		}
	}

	return false;
}

static bool wl_is_nonetwork(struct bcm_cfg80211 *cfg, const wl_event_msg_t *e)
{
	u32 event = ntoh32(e->event_type);
	u32 status = ntoh32(e->status);

	if (event == WLC_E_LINK && status == WLC_E_STATUS_NO_NETWORKS)
		return true;
	if (event == WLC_E_SET_SSID && status != WLC_E_STATUS_SUCCESS)
		return true;

	return false;
}

/* The mainline kernel >= 3.2.0 has support for indicating new/del station
 * to AP/P2P GO via events. If this change is backported to kernel for which
 * this driver is being built, then define WL_CFG80211_STA_EVENT. You
 * should use this new/del sta event mechanism for BRCM supplicant >= 22.
 */
static s32
wl_notify_connect_status_ap(struct bcm_cfg80211 *cfg, struct net_device *ndev,
	const wl_event_msg_t *e, void *data)
{
	s32 err = 0;
	u32 event = ntoh32(e->event_type);
	u32 reason = ntoh32(e->reason);
	u32 len = ntoh32(e->datalen);
#ifdef  BRCM_RSDB
	u32 status = ntoh32(e->status);
#endif
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 2, 0)) && !defined(WL_CFG80211_STA_EVENT)
#ifdef  BRCM_RSDB
	&& !defined(WL_COMPAT_WIRELESS)
#endif
	bool isfree = false;
	u8 *mgmt_frame;
	u8 bsscfgidx = e->bsscfgidx;
	s32 freq;
	s32 channel;
	u8 *body = NULL;
	u16 fc = 0;

	struct ieee80211_supported_band *band;
	struct ether_addr da;
	struct ether_addr bssid;
	struct wiphy *wiphy = bcmcfg_to_wiphy(cfg);
	channel_info_t ci;
#else
	struct station_info sinfo;
#endif

	WL_DBG(("event %d status %d reason %d\n", event, ntoh32(e->status), reason));
	/* if link down, bsscfg is disabled. */
	if (event == WLC_E_LINK && reason == WLC_E_LINK_BSSCFG_DIS &&
		wl_get_p2p_status(cfg, IF_DELETING) && (ndev != bcmcfg_to_prmry_ndev(cfg))) {
		wl_add_remove_eventmsg(ndev, WLC_E_PROBREQ_MSG, false);
		WL_INFORM(("AP mode link down !! \n"));
		complete(&cfg->iface_disable);
		return 0;
	}
#ifdef  BRCM_RSDB
	if ((event == WLC_E_LINK) && (status == WLC_E_STATUS_SUCCESS) &&
		(reason == WLC_E_REASON_INITIAL_ASSOC) &&
		(wl_get_mode_by_netdev(cfg, ndev) == WL_MODE_AP)) {
		if (!wl_get_drv_status(cfg, AP_CREATED, ndev)) {
			/* AP/GO brought up successfull in firmware */
			WL_ERR(("** AP/GO Link up event **\n"));
			wl_set_drv_status(cfg, AP_CREATED, ndev);
			wake_up_interruptible(&cfg->netif_change_event);
			return 0;
		}
	}
#endif
	if (event == WLC_E_DISASSOC_IND || event == WLC_E_DEAUTH_IND || event == WLC_E_DEAUTH) {
		WL_ERR(("event %s(%d) status %d reason %d\n",
		bcmevent_get_name(event), event, ntoh32(e->status), reason));
	}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 2, 0)) && !defined(WL_CFG80211_STA_EVENT)
#ifdef  BRCM_RSDB
	&& !defined(WL_COMPAT_WIRELESS)
#endif
	WL_DBG(("Enter \n"));
	if (!len && (event == WLC_E_DEAUTH)) {
		len = 2; /* reason code field */
		data = &reason;
	}
	if (len) {
		body = kzalloc(len, GFP_KERNEL);

		if (body == NULL) {
			WL_ERR(("wl_notify_connect_status: Failed to allocate body\n"));
			return WL_INVALID;
		}
	}
	memset(&bssid, 0, ETHER_ADDR_LEN);
	WL_DBG(("Enter event %d ndev %p\n", event, ndev));
	if (wl_get_mode_by_netdev(cfg, ndev) == WL_INVALID) {
		kfree(body);
		return WL_INVALID;
	}
	if (len)
		memcpy(body, data, len);

	wldev_iovar_getbuf_bsscfg(ndev, "cur_etheraddr",
		NULL, 0, cfg->ioctl_buf, WLC_IOCTL_SMLEN, bsscfgidx, &cfg->ioctl_buf_sync);
	memcpy(da.octet, cfg->ioctl_buf, ETHER_ADDR_LEN);
	memset(&bssid, 0, sizeof(bssid));
	err = wldev_ioctl(ndev, WLC_GET_BSSID, &bssid, ETHER_ADDR_LEN, false);
	switch (event) {
		case WLC_E_ASSOC_IND:
			fc = FC_ASSOC_REQ;
			break;
		case WLC_E_REASSOC_IND:
			fc = FC_REASSOC_REQ;
			break;
		case WLC_E_DISASSOC_IND:
			fc = FC_DISASSOC;
			break;
		case WLC_E_DEAUTH_IND:
			fc = FC_DISASSOC;
			break;
		case WLC_E_DEAUTH:
			fc = FC_DISASSOC;
			break;
		default:
			fc = 0;
			goto exit;
	}
	memset(&ci, 0, sizeof(ci));
	if ((err = wldev_ioctl(ndev, WLC_GET_CHANNEL, &ci, sizeof(ci), false))) {
		kfree(body);
		return err;
	}

	channel = dtoh32(ci.hw_channel);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0))
	if (channel <= CH_MAX_2G_CHANNEL)
		band = wiphy->bands[NL80211_BAND_2GHZ];
	else
		band = wiphy->bands[NL80211_BAND_5GHZ];
#else
	if (channel <= CH_MAX_2G_CHANNEL)
		band = wiphy->bands[IEEE80211_BAND_2GHZ];
	else
		band = wiphy->bands[IEEE80211_BAND_5GHZ];
#endif
	if (!band) {
		WL_ERR(("No valid band"));
		if (body)
			kfree(body);
		return -EINVAL;
	}
#if LINUX_VERSION_CODE == KERNEL_VERSION(2, 6, 38) && !defined(WL_COMPAT_WIRELESS)
	freq = ieee80211_channel_to_frequency(channel);
	(void)band->band;
#else
	freq = ieee80211_channel_to_frequency(channel, band->band);
#endif

	err = wl_frame_get_mgmt(fc, &da, &e->addr, &bssid,
		&mgmt_frame, &len, body);
	if (err < 0)
		goto exit;
	isfree = true;

	if (event == WLC_E_ASSOC_IND && reason == DOT11_SC_SUCCESS) {
#ifndef  BRCM_RSDB
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 18, 0))
		cfg80211_rx_mgmt(ndev, freq, 0, mgmt_frame, len, 0);
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0))
		cfg80211_rx_mgmt(ndev, freq, 0, mgmt_frame, len, 0, GFP_ATOMIC);
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 4, 0))
		cfg80211_rx_mgmt(ndev, freq, 0, mgmt_frame, len, GFP_ATOMIC);
#else
		cfg80211_rx_mgmt(ndev, freq, mgmt_frame, len, GFP_ATOMIC);
#endif
	} else if (event == WLC_E_DISASSOC_IND) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 18, 0))
		cfg80211_rx_mgmt(ndev, freq, 0, mgmt_frame, len, 0);
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0))
		cfg80211_rx_mgmt(ndev, freq, 0, mgmt_frame, len, 0, GFP_ATOMIC);
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 4, 0))
		cfg80211_rx_mgmt(ndev, freq, 0, mgmt_frame, len, GFP_ATOMIC);
#else
		cfg80211_rx_mgmt(ndev, freq, mgmt_frame, len, GFP_ATOMIC);
#endif
	} else if ((event == WLC_E_DEAUTH_IND) || (event == WLC_E_DEAUTH)) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 18, 0))
		cfg80211_rx_mgmt(ndev, freq, 0, mgmt_frame, len, 0);
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0))
		cfg80211_rx_mgmt(ndev, freq, 0, mgmt_frame, len, 0, GFP_ATOMIC);
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 4, 0))
		cfg80211_rx_mgmt(ndev, freq, 0, mgmt_frame, len, GFP_ATOMIC);
#else
		cfg80211_rx_mgmt(ndev, freq, mgmt_frame, len, GFP_ATOMIC);
#endif
#else  /* BRCM_RSDB */
#if ((LINUX_VERSION_CODE >= KERNEL_VERSION(3, 4, 0)) && (LINUX_VERSION_CODE < \
	KERNEL_VERSION(3, 18, 0))) || defined(WL_COMPAT_WIRELESS)
		cfg80211_rx_mgmt(ndev, freq, 0, mgmt_frame, len, GFP_ATOMIC);

#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 18, 0))
		cfg80211_rx_mgmt(ndev, freq, 0, mgmt_frame, len);
#else
		cfg80211_rx_mgmt(ndev, freq, mgmt_frame, len, GFP_ATOMIC);
#endif /* LINUX_VERSION >= VERSION(3,4,0) || WL_COMPAT_WIRELESS */
	} else if (event == WLC_E_DISASSOC_IND) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 4, 0)) || defined(WL_COMPAT_WIRELESS)
		cfg80211_rx_mgmt(ndev, freq, 0, mgmt_frame, len, GFP_ATOMIC);
#else
		cfg80211_rx_mgmt(ndev, freq, mgmt_frame, len, GFP_ATOMIC);
#endif /* LINUX_VERSION >= VERSION(3,4,0) || WL_COMPAT_WIRELESS */
	} else if ((event == WLC_E_DEAUTH_IND) || (event == WLC_E_DEAUTH)) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 4, 0)) || defined(WL_COMPAT_WIRELESS)
		cfg80211_rx_mgmt(ndev, freq, 0, mgmt_frame, len, GFP_ATOMIC);
#else
		cfg80211_rx_mgmt(ndev, freq, mgmt_frame, len, GFP_ATOMIC);
#endif /* LINUX_VERSION >= VERSION(3,4,0) || WL_COMPAT_WIRELESS */
#endif /* BRCM_RSDB */
	}

exit:
	if (isfree)
		kfree(mgmt_frame);
	if (body)
		kfree(body);
#else /* LINUX_VERSION < VERSION(3,2,0) && !WL_CFG80211_STA_EVENT && !WL_COMPAT_WIRELESS */
	sinfo.filled = 0;
	if (((event == WLC_E_ASSOC_IND) || (event == WLC_E_REASSOC_IND)) &&
		reason == DOT11_SC_SUCCESS) {
#ifndef  BRCM_RSDB
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0)) || !defined(HW_KERNEL_4_0_ADAPTATION)
		sinfo.filled = STATION_INFO_ASSOC_REQ_IES;
#endif
#else
		/* Linux ver >= 4.0 assoc_req_ies_len is used instead of
		 * STATION_INFO_ASSOC_REQ_IES flag
		 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0))
		sinfo.filled = STA_INFO_BIT(INFO_ASSOC_REQ_IES);
#endif /*  (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0)) */
#endif
		if (!data) {
			WL_ERR(("No IEs present in ASSOC/REASSOC_IND"));
			return -EINVAL;
		}
		sinfo.assoc_req_ies = data;
		sinfo.assoc_req_ies_len = len;
		cfg80211_new_sta(ndev, e->addr.octet, &sinfo, GFP_ATOMIC);
	} else if (event == WLC_E_DISASSOC_IND) {
		cfg80211_del_sta(ndev, e->addr.octet, GFP_ATOMIC);
	} else if ((event == WLC_E_DEAUTH_IND) || (event == WLC_E_DEAUTH)) {
		cfg80211_del_sta(ndev, e->addr.octet, GFP_ATOMIC);
	}
#endif
	return err;
}
#ifdef  BRCM_RSDB
#if defined(DHD_ENABLE_BIGDATA_LOGGING)
#define MAX_ASSOC_REJECT_ERR_STATUS 5
int wl_get_connect_failed_status(struct bcm_cfg80211 *cfg, const wl_event_msg_t *e)
{
	u32 status = ntoh32(e->status);

	cfg->assoc_reject_status = 0;

	if (status == WLC_E_STATUS_FAIL) {
		WL_ERR(("auth assoc status event=%d e->status %d e->reason %d \n",
			ntoh32(cfg->event_auth_assoc.event_type),
			(int)ntoh32(cfg->event_auth_assoc.status),
			(int)ntoh32(cfg->event_auth_assoc.reason)));

		switch ((int)ntoh32(cfg->event_auth_assoc.status)) {
			case WLC_E_STATUS_NO_ACK:
				cfg->assoc_reject_status = 1;
				break;
			case WLC_E_STATUS_FAIL:
				cfg->assoc_reject_status = 2;
				break;
			case WLC_E_STATUS_UNSOLICITED:
				cfg->assoc_reject_status = 3;
				break;
			case WLC_E_STATUS_TIMEOUT:
				cfg->assoc_reject_status = 4;
				break;
			case WLC_E_STATUS_ABORT:
				cfg->assoc_reject_status = 5;
				break;
			default:
				break;
		}
		if (cfg->assoc_reject_status) {
			if (ntoh32(cfg->event_auth_assoc.event_type) == WLC_E_ASSOC) {
				cfg->assoc_reject_status += MAX_ASSOC_REJECT_ERR_STATUS;
			}
		}
	}

	WL_ERR(("assoc_reject_status %d \n", cfg->assoc_reject_status));

	return 0;
}

s32 wl_cfg80211_get_connect_failed_status(struct net_device *dev, char* cmd, int total_len)
{
	struct bcm_cfg80211 *cfg = NULL;
	int bytes_written = 0;

	cfg = g_bcm_cfg;

	if (cfg == NULL) {
		return -1;
	}

	memset(cmd, 0, total_len);
	bytes_written = snprintf(cmd, 30, "assoc_reject.status %d", cfg->assoc_reject_status);

	WL_ERR(("cmd: %s \n", cmd));

	return bytes_written;
}
#endif /* DHD_ENABLE_BIGDATA_LOGGING */
#endif
static s32
wl_get_auth_assoc_status(struct bcm_cfg80211 *cfg, struct net_device *ndev,
	const wl_event_msg_t *e)
{
	u32 reason = ntoh32(e->reason);
	u32 event = ntoh32(e->event_type);
	struct wl_security *sec = wl_read_prof(cfg, ndev, WL_PROF_SEC);
	WL_DBG(("event type : %d, reason : %d\n", event, reason));
#ifdef  BRCM_RSDB
#if defined(DHD_ENABLE_BIGDATA_LOGGING)
	memcpy(&cfg->event_auth_assoc, e, sizeof(wl_event_msg_t));
	WL_ERR(("event=%d status %d reason %d \n",
		ntoh32(cfg->event_auth_assoc.event_type),
		ntoh32(cfg->event_auth_assoc.status),
		ntoh32(cfg->event_auth_assoc.reason)));
#endif /* DHD_ENABLE_BIGDATA_LOGGING */
#endif
	if (sec) {
		switch (event) {
		case WLC_E_ASSOC:
		case WLC_E_AUTH:
				sec->auth_assoc_res_status = reason;
		default:
			break;
		}
	} else
		WL_ERR(("sec is NULL\n"));
	return 0;
}

static s32
wl_notify_connect_status_ibss(struct bcm_cfg80211 *cfg, struct net_device *ndev,
	const wl_event_msg_t *e, void *data)
{
	s32 err = 0;
	u32 event = ntoh32(e->event_type);
	u16 flags = ntoh16(e->flags);
	u32 status =  ntoh32(e->status);
	bool active;
#ifndef  BRCM_RSDB
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 18, 0))
	struct ieee80211_channel *chan;
#endif
#else
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 15, 0)
	struct ieee80211_channel *channel = NULL;
	struct wiphy *wiphy = bcmcfg_to_wiphy(cfg);
	u32 chanspec, chan;
	u32 freq, band;
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(3, 15, 0) */
#endif

	if (event == WLC_E_JOIN) {
		WL_DBG(("joined in IBSS network\n"));
	}
	if (event == WLC_E_START) {
		WL_DBG(("started IBSS network\n"));
	}
	if (event == WLC_E_JOIN || event == WLC_E_START ||
		(event == WLC_E_LINK && (flags == WLC_EVENT_MSG_LINK))) {
#ifndef  BRCM_RSDB
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 18, 0))
		chan = ieee80211_get_channel(bcmcfg_to_wiphy(cfg), cfg->channel);
#endif
#else
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 15, 0)
		err = wldev_iovar_getint(ndev, "chanspec", (s32 *)&chanspec);
		if (unlikely(err)) {
			WL_ERR(("Could not get chanspec %d\n", err));
			return err;
		}
		chan = wf_chspec_ctlchan(wl_chspec_driver_to_host(chanspec));
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0))
		band = (chan <= CH_MAX_2G_CHANNEL) ? NL80211_BAND_2GHZ : NL80211_BAND_5GHZ;
#else
		band = (chan <= CH_MAX_2G_CHANNEL) ? IEEE80211_BAND_2GHZ : IEEE80211_BAND_5GHZ;
#endif
		freq = ieee80211_channel_to_frequency(chan, band);
		channel = ieee80211_get_channel(wiphy, freq);
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(3, 15, 0) */
#endif
		if (wl_get_drv_status(cfg, CONNECTED, ndev)) {
			/* ROAM or Redundant */
			u8 *cur_bssid = wl_read_prof(cfg, ndev, WL_PROF_BSSID);
			ASSERT(cur_bssid);
			if (memcmp(cur_bssid, &e->addr, ETHER_ADDR_LEN) == 0) {
				WL_DBG(("IBSS connected event from same BSSID("
					MACDBG "), ignore it\n", MAC2STRDBG(cur_bssid)));
				return err;
			}
			WL_INFORM(("IBSS BSSID is changed from " MACDBG " to " MACDBG "\n",
#ifndef  BRCM_RSDB
				MAC2STRDBG(cur_bssid), MAC2STRDBG((u8 *)&e->addr)));
			wl_get_assoc_ies(cfg, ndev);
			wl_update_prof(cfg, ndev, NULL, (void *)&e->addr, WL_PROF_BSSID);
			wl_update_bss_info(cfg, ndev, false);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 18, 0))
			cfg80211_ibss_joined(ndev, (s8 *)&e->addr, chan, GFP_KERNEL);
#else
			cfg80211_ibss_joined(ndev, (s8 *)&e->addr, GFP_KERNEL);
#endif
		}
		else {
			/* New connection */
			WL_INFORM(("IBSS connected to " MACDBG "\n", MAC2STRDBG((u8 *)&e->addr)));
			wl_link_up(cfg);
			wl_get_assoc_ies(cfg, ndev);
			wl_update_prof(cfg, ndev, NULL, (void *)&e->addr, WL_PROF_BSSID);
			wl_update_bss_info(cfg, ndev, false);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 18, 0))
			cfg80211_ibss_joined(ndev, (s8 *)&e->addr, chan, GFP_KERNEL);
#else
			cfg80211_ibss_joined(ndev, (s8 *)&e->addr, GFP_KERNEL);
#endif
			wl_set_drv_status(cfg, CONNECTED, ndev);
			active = true;
			wl_update_prof(cfg, ndev, NULL, (void *)&active, WL_PROF_ACT);
#else
			MAC2STRDBG(cur_bssid), MAC2STRDBG((const u8 *)&e->addr)));
			wl_get_assoc_ies(cfg, ndev);
			wl_update_prof(cfg, ndev, NULL, (const void *)&e->addr, WL_PROF_BSSID);
			wl_update_bss_info(cfg, ndev, false);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 15, 0)
			cfg80211_ibss_joined(ndev, (const s8 *)&e->addr, channel, GFP_KERNEL);
#else
			cfg80211_ibss_joined(ndev, (const s8 *)&e->addr, GFP_KERNEL);
#endif
		}
		else {
			/* New connection */
			WL_INFORM(("IBSS connected to " MACDBG "\n",
				MAC2STRDBG((const u8 *)&e->addr)));
			wl_link_up(cfg);
			wl_get_assoc_ies(cfg, ndev);
			wl_update_prof(cfg, ndev, NULL, (const void *)&e->addr, WL_PROF_BSSID);
			wl_update_bss_info(cfg, ndev, false);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 15, 0)
			cfg80211_ibss_joined(ndev, (const s8 *)&e->addr, channel, GFP_KERNEL);
#else
			cfg80211_ibss_joined(ndev, (const s8 *)&e->addr, GFP_KERNEL);
#endif
			wl_set_drv_status(cfg, CONNECTED, ndev);
			active = true;
			wl_update_prof(cfg, ndev, NULL, (const void *)&active, WL_PROF_ACT);
#endif
		}
	} else if ((event == WLC_E_LINK && !(flags & WLC_EVENT_MSG_LINK)) ||
		event == WLC_E_DEAUTH_IND || event == WLC_E_DISASSOC_IND) {
		wl_clr_drv_status(cfg, CONNECTED, ndev);
		wl_link_down(cfg);
		wl_init_prof(cfg, ndev);
	}
	else if (event == WLC_E_SET_SSID && status == WLC_E_STATUS_NO_NETWORKS) {
		WL_DBG(("no action - join fail (IBSS mode)\n"));
	}
	else {
		WL_DBG(("no action (IBSS mode)\n"));
}
	return err;
}

#if defined(WLFBT)
static s32
wl_notify_assoc_resp_ie(struct bcm_cfg80211 *cfg, bcm_struct_cfgdev *cfgdev,
								const wl_event_msg_t *e, void *data)
{
	struct net_device *ndev = NULL;

	ndev = cfgdev_to_wlc_ndev(cfgdev, cfg);

	if((e->status == 0) && (ntoh32(e->reason) == 0x111)) {
		u8 *curbssid;
		struct cfg80211_ft_event_params ft_event;

		curbssid = wl_read_prof(cfg, ndev, WL_PROF_BSSID);

		ft_event.ies = data + 6;
		ft_event.ies_len = ntoh32(e->datalen) - 6;
		ft_event.target_ap = (u8 *)&(e->addr);
		ft_event.ric_ies = NULL;
		ft_event.ric_ies_len = 0;

		cfg80211_ft_event(ndev, &ft_event);
	}
	return 0;
}
#endif

#ifdef  BRCM_RSDB
#if defined(DHD_ENABLE_BIGDATA_LOGGING)
#define WiFiALL_OUI         "\x50\x6F\x9A"  /* Wi-FiAll OUI */
#define WiFiALL_OUI_LEN     3
#define WiFiALL_OUI_TYPE    16

int wl_get_bss_info(struct bcm_cfg80211 *cfg, struct net_device *dev, uint8 *mac)
{
	s32 err = 0;
	struct wl_bss_info *bi;
	uint8 eabuf[ETHER_ADDR_LEN];
	u32 rate, channel, freq, supported_rate, nss = 0, mcs_map, mode_80211 = 0;
	char rate_str[4];
	u8 *ie = NULL;
	u32 ie_len;
	struct wiphy *wiphy;
	struct cfg80211_bss *bss;
	bcm_tlv_t *interworking_ie = NULL;
	bcm_tlv_t *tlv_ie = NULL;
	bcm_tlv_t *vht_ie = NULL;
	vndr_ie_t *vndrie;
	int16 ie_11u_rel_num = -1, ie_mu_mimo_cap = -1;
	u32 i, remained_len, count = 0;
	char roam_count_str[4], akm_str[4];
	s32 val = 0;

	/* get BSS information */

	strncpy(cfg->bss_info, "x x x x x x x x x x x x x", GET_BSS_INFO_LEN);

	*(u32 *) cfg->extra_buf = htod32(WL_EXTRA_BUF_MAX);

	err = wldev_ioctl(dev, WLC_GET_BSS_INFO, cfg->extra_buf, WL_EXTRA_BUF_MAX, false);
	if (unlikely(err)) {
		WL_ERR(("Could not get bss info %d\n", err));
		cfg->roam_count = 0;
		return -1;
	}

	if (!mac) {
		WL_ERR(("mac is null \n"));
		cfg->roam_count = 0;
		return -1;
	}

	memcpy(eabuf, mac, ETHER_ADDR_LEN);

	bi = (struct wl_bss_info *)(cfg->extra_buf + 4);
	channel = wf_chspec_ctlchan(bi->chanspec);

#if LINUX_VERSION_CODE == KERNEL_VERSION(2, 6, 38) && !defined(WL_COMPAT_WIRELESS)
	freq = ieee80211_channel_to_frequency(channel);
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0))
	if (channel > 14) {
		freq = ieee80211_channel_to_frequency(channel, NL80211_BAND_5GHZ);
	} else {
		freq = ieee80211_channel_to_frequency(channel, NL80211_BAND_2GHZ);
	}
#else
	if (channel > 14) {
		freq = ieee80211_channel_to_frequency(channel, IEEE80211_BAND_5GHZ);
	} else {
		freq = ieee80211_channel_to_frequency(channel, IEEE80211_BAND_2GHZ);
	}
#endif

	err = wldev_ioctl(dev, WLC_GET_RATE, &rate, sizeof(rate), false);
	if (err) {
		WL_ERR(("Could not get rate (%d)\n", err));
		snprintf(rate_str, sizeof(rate_str), "x"); // Unknown

	} else {
		rate = dtoh32(rate);
		snprintf(rate_str, sizeof(rate_str), "%d", (rate/2));
	}

	//supported maximum rate
	supported_rate = (bi->rateset.rates[bi->rateset.count - 1] & 0x7f) / 2;

	if (supported_rate < 12) {
		mode_80211 = 0; //11b maximum rate is 11Mbps. 11b mode
	} else {
		//It's not HT Capable case.
		if (channel > 14) {
			mode_80211 = 3; // 11a mode
		} else {
			mode_80211 = 1; // 11g mode
		}
	}

	if (bi->n_cap) {
		/* check Rx MCS Map for HT */
		nss = 0;
		mode_80211 = 2;
		for (i = 0; i < MAX_STREAMS_SUPPORTED; i++) {
			int8 bitmap = 0xFF;
			if (i == MAX_STREAMS_SUPPORTED-1) {
				bitmap = 0x7F;
			}
			if (bi->basic_mcs[i] & bitmap) {
				nss++;
			}
		}
	}

	if (bi->vht_cap) {
		nss = 0;
		mode_80211 = 4;
		for (i = 1; i <= VHT_CAP_MCS_MAP_NSS_MAX; i++) {
			mcs_map = VHT_MCS_MAP_GET_MCS_PER_SS(i, dtoh16(bi->vht_rxmcsmap));
			if (mcs_map != VHT_CAP_MCS_MAP_NONE) {
				nss++;
			}
		}
	}

	if (nss) {
		nss = nss - 1;
	}

	wiphy = bcmcfg_to_wiphy(cfg);
	bss = cfg80211_get_bss(wiphy, NULL, eabuf,
			bi->SSID, strlen(bi->SSID), WLAN_CAPABILITY_ESS,
			WLAN_CAPABILITY_ESS);

	if (!bss) {
		WL_ERR(("Could not find the AP\n"));
	} else {
#if defined(WL_CFG80211_P2P_DEV_IF)
		ie = (u8 *)bss->ies->data;
		ie_len = bss->ies->len;
#else
		ie = bss->information_elements;
		ie_len = bss->len_information_elements;
#endif /* WL_CFG80211_P2P_DEV_IF */
	}

	if (ie) {
		ie_mu_mimo_cap = 0;
		ie_11u_rel_num = 0;

		if (bi->vht_cap) {
			if ((vht_ie = bcm_parse_tlvs(ie, (u32)ie_len,
					DOT11_MNG_VHT_CAP_ID)) != NULL) {
				ie_mu_mimo_cap = (vht_ie->data[2] & 0x08) >> 3;
			}
		}

		if ((interworking_ie = bcm_parse_tlvs(ie, (u32)ie_len,
				DOT11_MNG_INTERWORKING_ID)) != NULL) {
			if ((tlv_ie = bcm_parse_tlvs(ie, (u32)ie_len, DOT11_MNG_VS_ID)) != NULL) {
				remained_len = ie_len;

				while (tlv_ie) {
					if (count > MAX_VNDR_IE_NUMBER)
						break;

					if (tlv_ie->id == DOT11_MNG_VS_ID) {
						vndrie = (vndr_ie_t *) tlv_ie;

						if (vndrie->len < (VNDR_IE_MIN_LEN + 1)) {
							WL_ERR(("%s: invalid vndr ie."
								"length is too small %d\n",
								__FUNCTION__, vndrie->len));
							break;
						}

						if (!bcmp(vndrie->oui,
							(u8*)WiFiALL_OUI, WiFiALL_OUI_LEN) &&
							(vndrie->data[0] == WiFiALL_OUI_TYPE))
						{
							WL_ERR(("Found Wi-FiAll OUI oui.\n"));
							ie_11u_rel_num = vndrie->data[1];
							ie_11u_rel_num = (ie_11u_rel_num & 0xf0)>>4;
							ie_11u_rel_num += 1;

							break;
						}
					}
					count++;
					tlv_ie = bcm_next_tlv(tlv_ie, &remained_len);
				}
			}
		}
	}

	for (i = 0; i < bi->SSID_len; i++) {
		if (bi->SSID[i] == ' ') {
			bi->SSID[i] = '_';
		}
	}

	//0 : None, 1 : OKC, 2 : FT, 3 : CCKM
	err = wldev_iovar_getint(dev, "wpa_auth", &val);
	if (unlikely(err)) {
		WL_ERR(("could not get wpa_auth (%d)\n", err));
		snprintf(akm_str, sizeof(akm_str), "x"); // Unknown
	} else {
		WL_ERR(("wpa_auth val %d \n", val));
#if defined(BCMCCX) || defined(BCMEXTCCX)
		if (val & (WPA_AUTH_CCKM | WPA2_AUTH_CCKM)) {
			snprintf(akm_str, sizeof(akm_str), "3");
		} else
#endif  /* BCMCCX || BCMEXTCCX */
		if (val & WPA2_AUTH_FT) {
			snprintf(akm_str, sizeof(akm_str), "2");
		} else if (val & (WPA_AUTH_UNSPECIFIED | WPA2_AUTH_UNSPECIFIED)) {
			snprintf(akm_str, sizeof(akm_str), "1");
		} else {
			snprintf(akm_str, sizeof(akm_str), "0");
		}
	}

	if (cfg->roam_offload) {
		snprintf(roam_count_str, sizeof(roam_count_str), "x"); // Unknown
	} else {
		snprintf(roam_count_str, sizeof(roam_count_str), "%d", cfg->roam_count);
	}
	cfg->roam_count = 0;

	WL_ERR(("BSSID:" MACDBG " SSID %s \n", MAC2STRDBG(eabuf), bi->SSID));
	WL_ERR(("freq:%d, BW:%s, RSSI:%d dBm, Rate:%d Mbps, 11mode:%d, stream:%d,"
				"MU-MIMO:%d, Passpoint:%d, SNR:%d, Noise:%d, \n"
				"akm:%s roam:%s \n",
				freq, wf_chspec_to_bw_str(bi->chanspec),
				dtoh32(bi->RSSI), (rate / 2), mode_80211, nss,
				ie_mu_mimo_cap, ie_11u_rel_num, bi->SNR, bi->phy_noise,
				akm_str, roam_count_str));

	if (ie) {
		snprintf(cfg->bss_info, GET_BSS_INFO_LEN,
				"%02x:%02x:%02x %d %s %d %s %d %d %d %d %d %d %s %s",
				eabuf[0], eabuf[1], eabuf[2],
				freq, wf_chspec_to_bw_str(bi->chanspec),
				dtoh32(bi->RSSI), rate_str, mode_80211, nss,
				ie_mu_mimo_cap, ie_11u_rel_num,
				bi->SNR, bi->phy_noise, akm_str, roam_count_str);
	} else {
		//ie_mu_mimo_cap and ie_11u_rel_num is unknow.
		snprintf(cfg->bss_info, GET_BSS_INFO_LEN,
				"%02x:%02x:%02x %d %s %d %s %d %d x x %d %d %s %s",
				eabuf[0], eabuf[1], eabuf[2],
				freq, wf_chspec_to_bw_str(bi->chanspec),
				dtoh32(bi->RSSI), rate_str, mode_80211, nss,
				bi->SNR, bi->phy_noise, akm_str, roam_count_str);
	}


	return 0;
}

s32 wl_cfg80211_get_bss_info(struct net_device *dev, char* cmd, int total_len)
{
	struct bcm_cfg80211 *cfg = NULL;

	cfg = g_bcm_cfg;

	if (cfg == NULL) {
		return -1;
	}

	memset(cmd, 0, total_len);
	memcpy(cmd, cfg->bss_info, GET_BSS_INFO_LEN);

	WL_ERR(("cmd: %s \n", cmd));

	return GET_BSS_INFO_LEN;
}

#endif /* DHD_ENABLE_BIGDATA_LOGGING */
#endif /* BRCM_RSDB */

#ifdef WL_TIM_EVENT
struct hw_tim_event_chr{
    u32  tim_event_cnt;        /* sum of wifi tim event */
    u32  pm2_sleep_ret;        /* PM threshold */
    u32  bcn_interval;	       /* beacon interval */
    u32  dtim_period;          /* DTIM */
    u32  send_package_cnt;     /* sum of send packages */
    u32  rcv_package_cnt;      /* sum of receive packages */
};

static struct hw_tim_event_chr tim_event_chr;
static struct mutex tim_event_mutex;

static s32
wl_hw_tim_event_handler(struct bcm_cfg80211 *cfg, bcm_struct_cfgdev *cfgdev,
	const wl_event_msg_t *e, void *data)
{
    u32 event = be32_to_cpu(e->event_type);
    u32 status = be32_to_cpu(e->status);
    s32 err = 0;
    struct net_device *ndev =  bcmcfg_to_prmry_ndev(cfg);

    if((WLC_E_TIM_EVENT == event) && (status == WLC_E_STATUS_SUCCESS) &&
	    (wl_get_drv_status(cfg, CONNECTED, bcmcfg_to_prmry_ndev(cfg)))) {
        mutex_lock(&tim_event_mutex);
        tim_event_chr.tim_event_cnt++;

        /* get pm2_sleep_ret */
        err = wldev_iovar_getint(ndev, "pm2_sleep_ret", &tim_event_chr.pm2_sleep_ret);
        if (err) {
            WL_ERR(("Could not get snr (%d)\n", err));
        }
        mutex_unlock(&tim_event_mutex);
    }
    return 0;
}

static int wifitim_seq_show(struct seq_file *seq, void *v)
{
    dhd_pub_t *dhd = wl_cfg80211_get_dhdp();
    WL_DBG(("wifitim_seq_show\n"));
    if (NULL != dhd) {
        tim_event_chr.send_package_cnt = dhd->tx_packets;
        tim_event_chr.rcv_package_cnt = dhd->rx_packets;
    }

    mutex_lock(&tim_event_mutex);
    seq_printf(seq, "tim_event_cnt:%d \n", tim_event_chr.tim_event_cnt);
    seq_printf(seq, "pm2_sleep_ret:%d \n", tim_event_chr.pm2_sleep_ret);
    seq_printf(seq, "bcn_interval:%d \n", tim_event_chr.bcn_interval);
    seq_printf(seq, "dtim_period:%d \n", tim_event_chr.dtim_period);
    seq_printf(seq, "send_package_cnt:%d \n", tim_event_chr.send_package_cnt);
    seq_printf(seq, "rcv_package_cnt:%d \n", tim_event_chr.rcv_package_cnt);
    seq_puts(seq, "\n");
    mutex_unlock(&tim_event_mutex);
    return 0;
}

static int wifitim_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, wifitim_seq_show, NULL);
}

static const struct file_operations wifitim_seq_fops = {
    .owner	 = THIS_MODULE,
    .open	 = wifitim_seq_open,
    .read	 = seq_read,
    .llseek	 = seq_lseek,
    .release	 = single_release,
};
#endif

#ifdef WL_TEM_CTRL
struct hw_tem_ctrl_chr{
    u32  tem_ctrl_cnt;		/* sum of wifi tem control process */
    u32  tem_ctrl_times;	/* duration of wifi tem control process */
    u16  maxtem;		/* max temprature of wifi tem control process */
    u16  mindutycycle_cnt;	/* sum of min dutycycle  of wifi tem control process */
    u32  exceed_warn_tem_cnt;	/* sum of exceed max tem cnt */
    u32  current_tem;
};

static struct hw_tem_ctrl_chr tem_ctrl_chr;
static u32  tem_ctrl_min_duty_cnt = 0;
static u32  tem_ctrl_exceed_tem_cnt = 0;
static struct timeval tem_ctrl_cycle_start;
static struct mutex tem_ctrl_mutex;
/*Called from the PROCfs module. This outputs /proc/net/wifipro_tcp_stat.*/
//u32  tem_ctrl_cnt;		/* sum of wifi tem control process */
//u32  tem_ctrl_times;		/* duration of wifi tem control process */
//u16  maxtem;			/* max temprature of wifi tem control process */
//u16  mindutycycle_cnt;	/* sum of min dutycycle  of wifi tem control process */
//u32  exceed_warn_tem_cnt;	/* sum of exceed max tem cnt */
static int wifitem_seq_show(struct seq_file *seq, void *v)
{
	WL_DBG(("wifitem_snmp_seq_show\n"));
	mutex_lock(&tem_ctrl_mutex);
	seq_printf(seq, "tem_ctrl_cnt:%d \n", tem_ctrl_chr.tem_ctrl_cnt);
	seq_printf(seq, "tem_ctrl_times:%d \n", tem_ctrl_chr.tem_ctrl_times);
	seq_printf(seq, "maxtem:%d \n", tem_ctrl_chr.maxtem);
	seq_printf(seq, "mindutycycle_cnt:%d \n", tem_ctrl_chr.mindutycycle_cnt);
	seq_printf(seq, "exceed_warn_tem_cnt:%d \n", tem_ctrl_chr.exceed_warn_tem_cnt);
	seq_printf(seq, "currnet_tem:%d \n", tem_ctrl_chr.current_tem);
	seq_puts(seq, "\n");
	memset(&tem_ctrl_chr,0,sizeof(struct hw_tem_ctrl_chr));
	mutex_unlock(&tem_ctrl_mutex);
	return 0;
}

static int wifitem_seq_set_current_tem(u32 current_tem)
{
	mutex_lock(&tem_ctrl_mutex);
	tem_ctrl_chr.current_tem = current_tem;
	mutex_unlock(&tem_ctrl_mutex);
	return 0;
}

static int wifitem_seq_open(struct inode *inode, struct file *file)
{
	return single_open(file, wifitem_seq_show, NULL);
}

static const struct file_operations wifitem_seq_fops = {
	.owner	 = THIS_MODULE,
	.open	 = wifitem_seq_open,
	.read	 = seq_read,
	.llseek	 = seq_lseek,
	.release = single_release,
};
#endif

#if defined WL_TEM_CTRL || defined WL_TIM_EVENT
struct proc_dir_entry *wifi_dir = NULL;
int wifi_init_proc()
{
    wifi_dir = proc_mkdir("wifi", NULL);
    if (!wifi_dir) {
	WL_ERR(("[WIFI] proc dir create failed\n"));
	return -ENOMEM;
    }
#ifdef WL_TEM_CTRL
    if (!proc_create("wifi_tem_stat",  S_IRUGO, wifi_dir, &wifitem_seq_fops)){
	WL_ERR(("wifi_init_proc error\n"));
	return -ENOMEM;
    }
    mutex_init(&tem_ctrl_mutex);
    mutex_lock(&tem_ctrl_mutex);
    memset(&tem_ctrl_chr,0,sizeof(struct hw_tem_ctrl_chr));
    mutex_unlock(&tem_ctrl_mutex);
#endif
#ifdef WL_TIM_EVENT
    if (!proc_create("wifi_tim_stat",  S_IRUGO, wifi_dir, &wifitim_seq_fops)){
        WL_ERR(("wifitim_init_proc error\n"));
	return -ENOMEM;
    }
    mutex_init(&tim_event_mutex);
    mutex_lock(&tim_event_mutex);
    memset(&tim_event_chr,0,sizeof(struct hw_tim_event_chr));
    mutex_unlock(&tim_event_mutex);
#endif
    WL_DBG(("wifi_init_proc ok\n"));
    return 0;
}

void wifi_exit_proc()
{
	WL_DBG(("wifi_exit_proc ok\n"));
	if (wifi_dir) {
#ifdef WL_TEM_CTRL
		remove_proc_entry("wifi_tem_stat", wifi_dir);
#endif
#ifdef WL_TIM_EVENT
		remove_proc_entry("wifi_tim_stat", wifi_dir);
#endif
		remove_proc_entry("wifi", 0);
		wifi_dir = NULL;
	}
}
#endif

static s32
wl_notify_connect_status(struct bcm_cfg80211 *cfg, bcm_struct_cfgdev *cfgdev,
	const wl_event_msg_t *e, void *data)
{
	bool act;
	struct net_device *ndev = NULL;
	s32 err = 0;
	u32 event = ntoh32(e->event_type);
#ifdef  BRCM_RSDB
	struct wiphy *wiphy = NULL;
	struct cfg80211_bss *bss = NULL;
	struct wlc_ssid *ssid = NULL;
	u8 *bssid = 0;
#endif
	ndev = cfgdev_to_wlc_ndev(cfgdev, cfg);

	if (wl_get_mode_by_netdev(cfg, ndev) == WL_MODE_AP) {
		err = wl_notify_connect_status_ap(cfg, ndev, e, data);
#ifdef HW_P2PGO_2G_SISO
		//previous p2p go already cut siso mode, confirm whether siso mode when p2p connect complete
		if (!err && (g_p2pgo_siso == 1) && p2p_is_on(cfg) &&  \
			(event == WLC_E_ASSOC_IND) && (ndev->ieee80211_ptr->iftype == NL80211_IFTYPE_P2P_GO)) {
			s32 cur_txchain = 0;
			// read mimo/siso capacity
			wldev_iovar_getint(ndev, "txchain", &cur_txchain);
			// 2.4G P2P only connect whether wifi chip auto cut back mimo
			if(cur_txchain == 3) {
				wldev_iovar_setint(ndev, "txchain", 1);
				wldev_iovar_setint(ndev, "rxchain", 1);
				WL_ERR(("%s: current antenna mimo mode when P2P connect, set antenna to 1 \n", __FUNCTION__));
			}
		}
#endif
	} else if (wl_get_mode_by_netdev(cfg, ndev) == WL_MODE_IBSS) {
		err = wl_notify_connect_status_ibss(cfg, ndev, e, data);
	} else if (wl_get_mode_by_netdev(cfg, ndev) == WL_MODE_BSS) {
		WL_DBG(("wl_notify_connect_status : event %d status : %d ndev %p\n",
			ntoh32(e->event_type), ntoh32(e->status), ndev));

#ifdef WRONG_ACTION_PATCH
		if (event == WLC_E_ASSOC) {
            wl_check_assoc_status(cfg, ndev, e);
		}
#endif		
		
		if (event == WLC_E_ASSOC || event == WLC_E_AUTH) {
			wl_get_auth_assoc_status(cfg, ndev, e);
			return 0;
		}

#ifdef WL_TIM_EVENT
		if(ntoh32(e->event_type) == WLC_E_JOIN) {
			dhd_pub_t *dhd = (dhd_pub_t *)(cfg->pub);
			/* read associated AP beacon interval */
			if ((err = dhd_wl_ioctl_cmd(dhd, WLC_GET_BCNPRD, &tim_event_chr.bcn_interval,
				sizeof(tim_event_chr.bcn_interval), FALSE, 0)) < 0) {
				WL_ERR(("%s get beacon failed code %d\n", __FUNCTION__, err));
			}

			/* read associated ap's dtim setup */
			if ((err = dhd_wl_ioctl_cmd(dhd, WLC_GET_DTIMPRD, &tim_event_chr.dtim_period,
				sizeof(tim_event_chr.dtim_period), FALSE, 0)) < 0) {
				WL_ERR(("%s failed code %d\n", __FUNCTION__, err));
			}
		}
#endif

		if (wl_is_linkup(cfg, e, ndev)) {
			wl_link_up(cfg);
			act = true;
			if (!wl_get_drv_status(cfg, DISCONNECTING, ndev)) {

				WL_ERR(("wl_bss_connect_done succeeded with " MACDBG "\n",
						MAC2STRDBG((const u8*)(&e->addr))));
#ifdef HW_PATCH_DISABLE_TCP_TIMESTAMPS
					hw_set_connect_status(ndev, 1);
#endif
#ifdef HW_LP_OVERSEA
				if (event == WLC_E_LINK) {
					dhd_pub_t *tdhd = (dhd_pub_t *)cfg->pub;
					hw_set_pmlock(tdhd);
				}
#endif
#ifdef  BRCM_RSDB
#ifdef DHD_LOSSLESS_ROAMING
				bool is_connected = wl_get_drv_status(cfg, CONNECTED, ndev);
#endif
#endif
				wl_bss_connect_done(cfg, ndev, e, data, true);
				WL_DBG(("joined in BSS network \"%s\"\n",
				((struct wlc_ssid *)
				 wl_read_prof(cfg, ndev, WL_PROF_SSID))->SSID));
#ifdef  BRCM_RSDB
#ifdef DHD_LOSSLESS_ROAMING
					if (event == WLC_E_LINK && is_connected &&
							!cfg->roam_offload) {
						wl_bss_roaming_done(cfg, ndev, e, data);
					}
#endif /* DHD_LOSSLESS_ROAMING */
#endif /* BRCM_RSDB */
				}
			wl_update_prof(cfg, ndev, e, &act, WL_PROF_ACT);
#ifndef  BRCM_RSDB
			wl_update_prof(cfg, ndev, NULL, (void *)&e->addr, WL_PROF_BSSID);
#else
			wl_update_prof(cfg, ndev, NULL, (const void *)&e->addr, WL_PROF_BSSID);
#endif
		} else if (wl_is_linkdown(cfg, e)) {
#ifndef BRCM_RSDB
			if (cfg->scan_request)
				wl_notify_escan_complete(cfg, ndev, true, true);
			if (wl_get_drv_status(cfg, CONNECTED, ndev)) {
				scb_val_t scbval;
				u8 *curbssid = wl_read_prof(cfg, ndev, WL_PROF_BSSID);
				s32 reason = 0;
				if (event == WLC_E_DEAUTH_IND || event == WLC_E_DISASSOC_IND)
					reason = ntoh32(e->reason);
				/* WLAN_REASON_UNSPECIFIED is used for hang up event in Android */
				reason = (reason == WLAN_REASON_UNSPECIFIED)? 0 : reason;

#if defined(DHD_DEBUG) && defined(BCM_PATCH_DEBUG_BEACON_LOSS)
				if (fw_log_open_by_beacon_loss == TRUE) {
					dhd_console_ms = 0;
					fw_log_open_by_beacon_loss = FALSE;
				}
#endif

				WL_ERR(("link down if %s may call cfg80211_disconnected. "
					"event : %d, reason=%d from " MACDBG "\n",
					ndev->name, event, ntoh32(e->reason),
					MAC2STRDBG((u8*)(&e->addr))));
				if (!cfg->roam_offload &&
					memcmp(curbssid, &e->addr, ETHER_ADDR_LEN) != 0) {
					WL_ERR(("BSSID of event is not the connected BSSID"
						"(ignore it) cur: " MACDBG " event: " MACDBG"\n",
						MAC2STRDBG(curbssid), MAC2STRDBG((u8*)(&e->addr))));
					return 0;
				}
#else
#ifdef DHD_LOSSLESS_ROAMING
			wl_del_roam_timeout(cfg);
#endif
#ifdef P2PLISTEN_AP_SAMECHN
			if (ndev == bcmcfg_to_prmry_ndev(cfg)) {
				wl_cfg80211_set_p2p_resp_ap_chn(ndev, 0);
				cfg->p2p_resp_apchn_status = false;
				WL_DBG(("p2p_resp_apchn_status Turn OFF \n"));
			}
#endif /* P2PLISTEN_AP_SAMECHN */
			wl_cfg80211_cancel_scan(cfg);

#if defined(DHD_ENABLE_BIGDATA_LOGGING)
			if (wl_get_drv_status(cfg, CONNECTED, ndev)) {
				wl_get_bss_info(cfg, ndev, (u8*)(&e->addr));
			}
#endif /* DHD_ENABLE_BIGDATA_LOGGING */
			/* Explicitly calling unlink to remove BSS in CFG */
			wiphy = bcmcfg_to_wiphy(cfg);
			ssid = (struct wlc_ssid *)wl_read_prof(cfg, ndev, WL_PROF_SSID);
			bssid = (u8 *)wl_read_prof(cfg, ndev, WL_PROF_BSSID);
			if (ssid && bssid) {
				bss = cfg80211_get_bss(wiphy, NULL, bssid,
					ssid->SSID, ssid->SSID_len, WLAN_CAPABILITY_ESS,
					WLAN_CAPABILITY_ESS);
				if (bss) {
					cfg80211_unlink_bss(wiphy, bss);
				}
			}

			if (wl_get_drv_status(cfg, CONNECTED, ndev)) {
				scb_val_t scbval;
				u8 *curbssid = wl_read_prof(cfg, ndev, WL_PROF_BSSID);
				s32 reason = 0;
				struct ether_addr bssid_dongle;
				struct ether_addr bssid_null = {{0, 0, 0, 0, 0, 0}};

				if (event == WLC_E_DEAUTH_IND || event == WLC_E_DISASSOC_IND)
					reason = ntoh32(e->reason);
				/* WLAN_REASON_UNSPECIFIED is used for hang up event in Android */
				reason = (reason == WLAN_REASON_UNSPECIFIED)? 0 : reason;

				WL_ERR(("link down if %s may call cfg80211_disconnected. "
					"event : %d, reason=%d from " MACDBG "\n",
					ndev->name, event, ntoh32(e->reason),
					MAC2STRDBG((const u8*)(&e->addr))));

				/* roam offload does not sync BSSID always, get it from dongle */
				if (cfg->roam_offload) {
					if (wldev_ioctl(ndev, WLC_GET_BSSID, &bssid_dongle,
							sizeof(bssid_dongle), false) == BCME_OK) {
						/* if not roam case, it would return null bssid */
						if (memcmp(&bssid_dongle, &bssid_null,
								ETHER_ADDR_LEN) != 0) {
							curbssid = (u8 *)&bssid_dongle;
						}
					}
				}
				if (memcmp(curbssid, &e->addr, ETHER_ADDR_LEN) != 0) {
					bool fw_assoc_state = TRUE;
					dhd_pub_t *dhd = (dhd_pub_t *)cfg->pub;
					fw_assoc_state = dhd_is_associated(dhd, e->ifidx, &err);
					if (!fw_assoc_state) {
						WL_ERR(("Even sends up even different BSSID"
							" cur: " MACDBG " event: " MACDBG"\n",
							MAC2STRDBG(curbssid),
							MAC2STRDBG((const u8*)(&e->addr))));
					} else {
						WL_ERR(("BSSID of event is not the connected BSSID"
							"(ignore it) cur: " MACDBG
							" event: " MACDBG"\n",
							MAC2STRDBG(curbssid),
							MAC2STRDBG((const u8*)(&e->addr))));
						return 0;
					}
				}
#endif /*BRCM_RSDB*/
#ifdef HW_PATCH_DISABLE_TCP_TIMESTAMPS
				hw_set_connect_status(ndev, 0);
#endif
				wl_clr_drv_status(cfg, CONNECTED, ndev);
				if (!wl_get_drv_status(cfg, DISCONNECTING, ndev)) {
					/* To make sure disconnect, explictly send dissassoc
					*  for BSSID 00:00:00:00:00:00 issue
					*/
					scbval.val = WLAN_REASON_DEAUTH_LEAVING;

					memcpy(&scbval.ea, curbssid, ETHER_ADDR_LEN);
					scbval.val = htod32(scbval.val);
					err = wldev_ioctl(ndev, WLC_DISASSOC, &scbval,
						sizeof(scb_val_t), true);
					if (err < 0) {
						WL_ERR(("WLC_DISASSOC error %d\n", err));
						err = 0;
					}
#ifndef  BRCM_RSDB
					CFG80211_DISCONNECTED(ndev, reason, NULL, 0, false, GFP_KERNEL);
					wl_link_down(cfg);
					wl_init_prof(cfg, ndev);
				} else {
					wl_clr_drv_status(cfg, DISCONNECTING, ndev);
				}
			} else if (wl_get_drv_status(cfg, CONNECTING, ndev)) {
#else
					CFG80211_DISCONNECTED(ndev, reason, NULL, 0,
							false, GFP_KERNEL);
					wl_link_down(cfg);
					wl_init_prof(cfg, ndev);
				}
			}
			else if (wl_get_drv_status(cfg, CONNECTING, ndev)) {
#endif
				WL_ERR(("link down, during connecting\n"));
#ifdef ESCAN_RESULT_PATCH
				if ((memcmp(connect_req_bssid, broad_bssid, ETHER_ADDR_LEN) == 0) ||
					(memcmp(&e->addr, broad_bssid, ETHER_ADDR_LEN) == 0) ||
					(memcmp(&e->addr, connect_req_bssid, ETHER_ADDR_LEN) == 0))
					/* In case this event comes while associating another AP */
#endif /* ESCAN_RESULT_PATCH */
#ifndef  BRCM_RSDB
					if (!wl_get_drv_status(cfg, DISCONNECTING, ndev))
#endif
						wl_bss_connect_done(cfg, ndev, e, data, false);
			}
			wl_clr_drv_status(cfg, DISCONNECTING, ndev);

			/* if link down, bsscfg is diabled */
			if (ndev != bcmcfg_to_prmry_ndev(cfg))
				complete(&cfg->iface_disable);

		} else if (wl_is_nonetwork(cfg, e)) {
			WL_ERR(("connect failed event=%d e->status %d e->reason %d \n",
				event, (int)ntoh32(e->status), (int)ntoh32(e->reason)));
#ifndef  BRCM_RSDB
			/* Clean up any pending scan request */
			if (cfg->scan_request)
				wl_notify_escan_complete(cfg, ndev, true, true);
			if (wl_get_drv_status(cfg, CONNECTING, ndev) &&
				!wl_get_drv_status(cfg, DISCONNECTING, ndev))
				wl_bss_connect_done(cfg, ndev, e, data, false);
			wl_clr_drv_status(cfg, DISCONNECTING, ndev);
			wl_clr_drv_status(cfg, CONNECTING, ndev);
#else
#if defined(DHD_ENABLE_BIGDATA_LOGGING)
			if (event == WLC_E_SET_SSID) {
				wl_get_connect_failed_status(cfg, e);
			}
#endif /* DHD_ENABLE_BIGDATA_LOGGING */
			/* Clean up any pending scan request */
			wl_cfg80211_cancel_scan(cfg);
			if (wl_get_drv_status(cfg, CONNECTING, ndev))
				wl_bss_connect_done(cfg, ndev, e, data, false);
#endif
		} else {
			WL_DBG(("%s nothing\n", __FUNCTION__));
		}
	}
		else {
		WL_ERR(("Invalid ndev status %d\n", wl_get_mode_by_netdev(cfg, ndev)));
	}
	return err;
}

#ifdef WL_TEM_CTRL
#define HW_MAX_CHIP_TEM 100
#define WLC_E_RESULT_TMPCTL 0
#define WLC_E_RESULT_VOLS 1
#define HW_MAX_CHIP_TEM_SPECIAL (107)
#define HW_TIMESTAMP_STR_SIZE 32
#define DSM_BUFF_SIZE_MAX 1024
#define HW_TEM_CTRL_INCREASE 0
#define HW_TEM_CTRL_DECREASE 1
#define HW_TEM_CTRL_DUTY_START_PCT 50
#define HW_TEM_CTRL_DUTY_END_PCT 100
#define HW_TEM_CTRL_MIN_DUTYCYCLE 10
struct hw_tem_ctrl_event{
    u16  duty;        /* Adjusted to the percentage */
    u16  decrease;    /* Duty cycle increase or decrease(1:decrease, 0:increase) */
    int32  temperature; /* Current chipset temperature */
};

struct hw_tem_ctrl_electric_event {
    int32 elec1;
    int32 elec2;
    int32 elec3;
    int32 elec4;
};

struct hw_tem_ctrl_electric_value {
    int32 elec1;
    int32 elec2;
    int32 elec3;
    int32 elec4;
    struct timeval tv;
};

enum hw_get_time_type {
    TS_DATE,
    TS_TIME,
    TS_DATETIME
};

static char tem_ctrl_start_time[HW_TIMESTAMP_STR_SIZE] = {0};
static u32  tem_ctrl_max_temperature = 0;
static char tem_ctrl_max_temp_time[HW_TIMESTAMP_STR_SIZE] = {0};
static char tem_ctrl_min_duty_time[HW_TIMESTAMP_STR_SIZE] = {0};
static u16  tem_ctrl_pre_duty = HW_TEM_CTRL_DUTY_END_PCT;
static bool tem_ctrl_started = FALSE;
static u32  tem_ctrl_start_temperature = 0;
static u16  tem_ctrl_min_duty = HW_TEM_CTRL_DUTY_END_PCT;
static u64  tem_ctrl_nrate = 0;
static u32  tem_ctrl_rssi = 0;
static int32  tem_ctrl_tempthresh = 85;
static u32  tem_ctrl_band = 2; /* 2: 2.4G, 5: 5G */
static struct hw_tem_ctrl_electric_value tem_ctrl_elec_tmp = {0,0,0,0};
static struct hw_tem_ctrl_electric_value tem_ctrl_elec_start_record = {0,0,0,0};
static struct hw_tem_ctrl_electric_value tem_ctrl_elec_maxtemp_record = {0,0,0,0};
static struct hw_tem_ctrl_electric_value tem_ctrl_elec_invalid = {-1,-1,-1,-1};

static int wl_hw_get_timestamp(char *str, int len, enum hw_get_time_type ts_type)
{
    struct timeval tv;
    struct rtc_time tm;

    if(NULL == str) {
        WL_ERR(("wl_hw_get_timestamp input para is null.\n"));
        return BCME_ERROR;
    }

    do_gettimeofday(&tv);
    tv.tv_sec -= (long)sys_tz.tz_minuteswest * 60;
    rtc_time_to_tm(tv.tv_sec, &tm);

    if (TS_DATE == ts_type)
        snprintf(str, len, "%04d-%02d-%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
    else if (TS_TIME == ts_type)
        snprintf(str, len, "%02d:%02d:%02d", tm.tm_hour, tm.tm_min, tm.tm_sec);
    else
        snprintf(str, len, "%04d-%02d-%02d %02d:%02d:%02d",
            tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

    return strlen(str);
}

#define HW_TEMP_CTRL_MODE_MASK   0x03000000
#define HW_TEMP_CTRL_SGI_MASK    0x00800000
#define HW_TEMP_CTRL_BW_MASK     0x00070000
#define HW_TEMP_CTRL_RATE_MASK   0x000000FF
#define HW_TEMP_CTRL_HTMCS_MASK  0x000000FF
#define HW_TEMP_CTRL_VHTMCS_MASK 0x0000000F
#define HW_TEMP_CTRL_VHTNSS_MASK 0x000000F0
void
wl_hw_tem_ctrl_fill_nrate_info(char *buf, int *len)
{
	u16 mode = 0, sgi = 0, bw = 0, ht_mcs = 0, vht_mcs = 0, vht_nss = 0;
	u64 nrate = tem_ctrl_nrate;
	u32 rate = 0;
	int tmp_len = 0;

	if (NULL == buf || NULL == len) {
		WL_ERR(("wl_hw_tem_ctrl_fill_nrate_info invalid param\n"));
		return;
	}
	tmp_len = *len;
	mode = (nrate & HW_TEMP_CTRL_MODE_MASK) >> 24;
	sgi = (nrate & HW_TEMP_CTRL_SGI_MASK) >> 23;
	bw = (nrate & HW_TEMP_CTRL_BW_MASK) >> 16;
	tmp_len += snprintf(buf+tmp_len, DSM_BUFF_SIZE_MAX-tmp_len, "mode:%u,sgi:%u,bw:%u,", mode, sgi, bw);
	if (0 == mode) {
		rate = nrate & HW_TEMP_CTRL_RATE_MASK;
		tmp_len += snprintf(buf+tmp_len, DSM_BUFF_SIZE_MAX-tmp_len, "rate:%u) ", rate);
	} else if (1 == mode) {
		ht_mcs = nrate & HW_TEMP_CTRL_HTMCS_MASK;
		tmp_len += snprintf(buf+tmp_len, DSM_BUFF_SIZE_MAX-tmp_len, "mcs:%u) ", ht_mcs);
	} else if (2 == mode) {
		vht_mcs = nrate & HW_TEMP_CTRL_VHTMCS_MASK;
		vht_nss = (nrate & HW_TEMP_CTRL_VHTNSS_MASK) >> 4;
		tmp_len += snprintf(buf+tmp_len, DSM_BUFF_SIZE_MAX-tmp_len, "mcs:%u,nss:%u) ", vht_mcs, vht_nss);
	}
	*len = tmp_len;
}

void
wl_hw_temp_dmd_get_start_info(struct net_device *dev) {
    s32 err = 0;
    scb_val_t scb_val;

    /* get rssi */
    memset(&scb_val, 0, sizeof(scb_val));
    err = wldev_ioctl(dev, WLC_GET_RSSI, &scb_val, sizeof(scb_val_t), false);
    tem_ctrl_rssi = wl_rssi_offset(dtoh32(scb_val.val));
    /* get nrate */
    err += wldev_iovar_getbuf(dev, "nrate", NULL, 0, &tem_ctrl_nrate, sizeof(u64), NULL);
    if (err < 0) {
        WL_ERR(("get rssi or nrate err: %d\n", err));
    }
}

void
wl_hw_tem_get_tempthresh_info(struct net_device *dev) {
    s32 err = 0;

    err = wldev_iovar_getint(dev, "phy_tempthresh", &tem_ctrl_tempthresh);
    if (err < 0) {
        WL_ERR(("get tempthresh err: %d\n", err));
        tem_ctrl_tempthresh = 85;
    }
}

void wl_hw_tem_ctrl_event_report(void)
{
    static struct timeval tem_ctrl_cycle_end;
    if(tem_ctrl_started){
#ifdef HW_WIFI_DMD_LOG
        char dsm_buff[DSM_BUFF_SIZE_MAX] = {0};
        int tmp_len = 0;
        tmp_len += snprintf(dsm_buff, DSM_BUFF_SIZE_MAX, "StartTime:%s ", tem_ctrl_start_time);
        tmp_len += snprintf(dsm_buff+tmp_len, DSM_BUFF_SIZE_MAX-tmp_len,
                   "Temp:%d(%d,%d,%d,%d) ", tem_ctrl_start_temperature, tem_ctrl_elec_start_record.elec1,
                    tem_ctrl_elec_start_record.elec2, tem_ctrl_elec_start_record.elec3,
                    tem_ctrl_elec_start_record.elec4);
        tmp_len += snprintf(dsm_buff+tmp_len, DSM_BUFF_SIZE_MAX-tmp_len, "nrate(%d,%dG,", tem_ctrl_rssi, tem_ctrl_band);
        wl_hw_tem_ctrl_fill_nrate_info(dsm_buff, &tmp_len);
        tmp_len += snprintf(dsm_buff+tmp_len, DSM_BUFF_SIZE_MAX-tmp_len, "MaxTemp:%d(%d,%d,%d,%d)@%s ",
                            tem_ctrl_max_temperature, tem_ctrl_elec_maxtemp_record.elec1,
                            tem_ctrl_elec_maxtemp_record.elec2, tem_ctrl_elec_maxtemp_record.elec3,
                            tem_ctrl_elec_maxtemp_record.elec4, tem_ctrl_max_temp_time);
        tmp_len += snprintf(dsm_buff+tmp_len, DSM_BUFF_SIZE_MAX-tmp_len, "MinDuty:%d@%s", tem_ctrl_min_duty, tem_ctrl_min_duty_time);
        hw_wifi_dsm_client_notify(DSM_WIFI_TEM_CTRL_EVENT, dsm_buff);
        WL_ERR(("dsm_buff:%s\n", dsm_buff));
#endif

	    do_gettimeofday(&tem_ctrl_cycle_end);
	    mutex_lock(&tem_ctrl_mutex);
	    tem_ctrl_chr.tem_ctrl_cnt++;
	    tem_ctrl_chr.tem_ctrl_times += (tem_ctrl_cycle_end.tv_sec - tem_ctrl_cycle_start.tv_sec);
	    tem_ctrl_chr.maxtem = (tem_ctrl_chr.maxtem > tem_ctrl_max_temperature) ? tem_ctrl_chr.maxtem : tem_ctrl_max_temperature;
	    tem_ctrl_chr.mindutycycle_cnt = tem_ctrl_min_duty_cnt;
	    tem_ctrl_chr.exceed_warn_tem_cnt = tem_ctrl_exceed_tem_cnt;
	    mutex_unlock(&tem_ctrl_mutex);
        tem_ctrl_started = FALSE;
        tem_ctrl_max_temperature = 0;
        tem_ctrl_start_temperature = 0;
        tem_ctrl_pre_duty = HW_TEM_CTRL_DUTY_END_PCT;
        tem_ctrl_min_duty = HW_TEM_CTRL_DUTY_END_PCT;
	}
}

static void wl_hw_tem_ctrl_get_elec(struct hw_tem_ctrl_electric_value *elec_value) {
    struct timeval tv;
    do_gettimeofday(&tv);
    if (tv.tv_sec - tem_ctrl_elec_tmp.tv.tv_sec <= 1) {
        *elec_value = tem_ctrl_elec_tmp;
    } else {
        *elec_value = tem_ctrl_elec_invalid;
        WL_ERR(("Invalid electric value, now is:%ld, electric time is: %ld\n", tv.tv_sec, tem_ctrl_elec_tmp.tv.tv_sec));
    }
}

static void wl_hw_tem_ctrl_normal_event(struct hw_tem_ctrl_event *tem_ctl) {
    if (NULL == tem_ctl) {
        WL_ERR(("wl_hw_tem_ctrl_normal_event invalid param\n"));
        return;
    }
    /* duty=10 count for chr */
    if (HW_TEM_CTRL_MIN_DUTYCYCLE == tem_ctl->duty) {
        tem_ctrl_min_duty_cnt++;
    }
    /* temperature count for chr when temperature is higher than HW_MAX_CHIP_TEM */
    if (HW_MAX_CHIP_TEM <= tem_ctl->temperature) {
        tem_ctrl_exceed_tem_cnt++;
    }
    /* check the maximum temperature */
    if (tem_ctl->temperature > tem_ctrl_max_temperature) {
        tem_ctrl_max_temperature = tem_ctl->temperature;
        wl_hw_get_timestamp(tem_ctrl_max_temp_time, HW_TIMESTAMP_STR_SIZE, TS_TIME);
        wl_hw_tem_ctrl_get_elec(&tem_ctrl_elec_maxtemp_record);
        /* report when the temperature is higher than HW_MAX_CHIP_TEM and higher than the pre_max temperature */
        if (HW_MAX_CHIP_TEM <= tem_ctl->temperature) {
            char dsm_buff[DSM_BUFF_SIZE_MAX] = {0};
            snprintf(dsm_buff, DSM_BUFF_SIZE_MAX, "MaxTemp reach %d(%d,%d,%d,%d) duty:%d\n", tem_ctl->temperature,
                tem_ctrl_elec_maxtemp_record.elec1, tem_ctrl_elec_maxtemp_record.elec2,
                tem_ctrl_elec_maxtemp_record.elec3, tem_ctrl_elec_maxtemp_record.elec4, tem_ctl->duty);
            WL_ERR(("Reach max temp, dmd buff:%s\n", dsm_buff));
#ifdef HW_WIFI_DMD_LOG
            hw_wifi_dsm_client_notify(DSM_WIFI_CHIPSET_DAMAGE_WARNING, dsm_buff);
#endif
        }
    }
    /* check the minimum duty */
    if (HW_TEM_CTRL_DECREASE == tem_ctl->decrease && tem_ctrl_min_duty > tem_ctl->duty) {
        tem_ctrl_min_duty = tem_ctl->duty;
        wl_hw_get_timestamp(tem_ctrl_min_duty_time, HW_TIMESTAMP_STR_SIZE, TS_TIME);
    }
    /* temperature control stop */
    else if ((HW_TEM_CTRL_DUTY_END_PCT == tem_ctl->duty)&&(HW_TEM_CTRL_INCREASE == tem_ctl->decrease)) {
        WL_ERR(("temperature control stops\n"));
        wl_hw_tem_ctrl_event_report();
    }
    /* update duty */
    tem_ctrl_pre_duty = tem_ctl->duty;
}

static void wl_hw_tem_ctrl_abnormal_event(struct hw_tem_ctrl_event *tem_ctl, struct bcm_cfg80211 *cfg) {
    char dsm_buff[DSM_BUFF_SIZE_MAX] = {0};
    struct hw_tem_ctrl_electric_value tem_ctrl_elec_error_record = {0,0,0,0};
#ifdef HW_WIFI_DMD_LOG
    dhd_pub_t *dhd =  NULL;
#endif
    if (NULL == tem_ctl) {
        WL_ERR(("wl_hw_tem_ctrl_abnormal_event invalid param\n"));
        return;
    }
    wl_hw_tem_ctrl_get_elec(&tem_ctrl_elec_error_record);
    snprintf(dsm_buff, DSM_BUFF_SIZE_MAX, "duty = %d, decrease = %d, temperature = %d(%d,%d,%d,%d)\n",
        tem_ctl->duty, tem_ctl->decrease, tem_ctl->temperature,
        tem_ctrl_elec_error_record.elec1, tem_ctrl_elec_error_record.elec2,
        tem_ctrl_elec_error_record.elec3, tem_ctrl_elec_error_record.elec4);
    WL_ERR(("Error temp_ctrl_event, dmd buff:%s\n", dsm_buff));
#ifdef HW_WIFI_DMD_LOG
    dhd =  (dhd_pub_t *)(cfg->pub);
    if (dhd->op_mode & DHD_FLAG_STA_MODE){
        hw_wifi_dsm_client_notify(DSM_WIFI_WLC_SET_PASSIVE_SCAN_ERROR, dsm_buff);
    }
#endif
}


/*
 * deal with WLC_E_TEM_CTRL_EVENT
 * WLC_E_TEM_CTRL_EVENT is received when duty cycle change or
 * duty cycle down to 10 but the temperature is still rising
 */
static s32
wl_hw_tem_ctrl_event_handler(struct bcm_cfg80211 *cfg, bcm_struct_cfgdev *cfgdev,
	const wl_event_msg_t *e, void *data)
{
    u32 event = be32_to_cpu(e->event_type);
    u32 status = be32_to_cpu(e->status);
    struct hw_tem_ctrl_event *tem_ctl = NULL;
    struct hw_tem_ctrl_electric_event *tem_ctl_elec = NULL;
    struct net_device *ndev = cfgdev_to_wlc_ndev(cfgdev, cfg);

    if ((event == WLC_E_TEM_CTRL_EVENT) && (status == WLC_E_RESULT_TMPCTL)) {
        tem_ctl = data;
        wl_hw_tem_get_tempthresh_info(ndev);

        if ((e->datalen >= sizeof(struct hw_tem_ctrl_event)) && (tem_ctl)) {
            /* Print tem_ctrl_event kernel log */
            WL_ERR(("current duty = %d, decrease=%d, temperature=%d\n",
                tem_ctl->duty, tem_ctl->decrease, tem_ctl->temperature));

            /* temperature control start */
            if ((HW_TEM_CTRL_DUTY_START_PCT == tem_ctl->duty) && (HW_TEM_CTRL_DECREASE == tem_ctl->decrease)
                && (HW_TEM_CTRL_DUTY_END_PCT == tem_ctrl_pre_duty)&&(tem_ctrl_tempthresh <= tem_ctl->temperature)) {
                tem_ctrl_started = TRUE;
                do_gettimeofday(&tem_ctrl_cycle_start);
                wl_hw_get_timestamp(tem_ctrl_start_time, HW_TIMESTAMP_STR_SIZE, TS_DATETIME);
                WL_ERR(("tem_ctrl_start_time:%s\n", tem_ctrl_start_time));
                tem_ctrl_band = (cfg->channel <= 14) ? 2 : 5;
                tem_ctrl_start_temperature = tem_ctl->temperature;
                wl_hw_tem_ctrl_get_elec(&tem_ctrl_elec_start_record);
                wl_hw_temp_dmd_get_start_info(ndev);
            }

            /* normal temperature control process */
            if (tem_ctrl_started) {
                wl_hw_tem_ctrl_normal_event(tem_ctl);
            } else {
                /* abnormal temperature control process */
                wl_hw_tem_ctrl_abnormal_event(tem_ctl, cfg);
            }
        }
    } else if ((event == WLC_E_TEM_CTRL_EVENT) && (status == WLC_E_RESULT_VOLS)) {
        tem_ctl_elec = data;
        tem_ctrl_elec_tmp.elec1 = tem_ctl_elec->elec1;
        tem_ctrl_elec_tmp.elec2 = tem_ctl_elec->elec2;
        tem_ctrl_elec_tmp.elec3 = tem_ctl_elec->elec3;
        tem_ctrl_elec_tmp.elec4 = tem_ctl_elec->elec4;
        do_gettimeofday(&(tem_ctrl_elec_tmp.tv));
    }
    return BCME_OK;
}
#endif


#ifndef  BRCM_RSDB
#ifdef GSCAN_SUPPORT
static s32
wl_handle_roam_exp_event(struct bcm_cfg80211 *cfg, bcm_struct_cfgdev *cfgdev,
	const wl_event_msg_t *e, void *data)
{
	struct net_device *ndev = NULL;
	u32 datalen = be32_to_cpu(e->datalen);

	if (datalen) {
		wl_roam_exp_event_t *evt_data = (wl_roam_exp_event_t *)data;
		if (evt_data->version == ROAM_EXP_EVENT_VERSION) {
			wlc_ssid_t *ssid = &evt_data->cur_ssid;
			struct wireless_dev *wdev;
			ndev = cfgdev_to_wlc_ndev(cfgdev, cfg);
			if (ndev) {
				wdev = ndev->ieee80211_ptr;
				wdev->ssid_len = min(ssid->SSID_len, (uint32)DOT11_MAX_SSID_LEN);
				memcpy(wdev->ssid, ssid->SSID, wdev->ssid_len);
				WL_ERR(("SSID is %s\n", ssid->SSID));
				wl_update_prof(cfg, ndev, NULL, ssid, WL_PROF_SSID);
			} else {
				WL_ERR(("NULL ndev!\n"));
			}
		} else {
			WL_ERR(("Version mismatch %d, expected %d", evt_data->version,
			       ROAM_EXP_EVENT_VERSION));
		}
	}
	return BCME_OK;
}
#endif /* GSCAN_SUPPORT */

static s32 wl_handle_rssi_monitor_event(struct bcm_cfg80211 *cfg, bcm_struct_cfgdev *cfgdev,
	const wl_event_msg_t *e, void *data)
{
	u32 datalen = be32_to_cpu(e->datalen);
	struct net_device *ndev = cfgdev_to_wlc_ndev(cfgdev, cfg);
	struct wiphy *wiphy = bcmcfg_to_wiphy(cfg);

	if (datalen) {
		wl_rssi_monitor_evt_t *evt_data = (wl_rssi_monitor_evt_t *)data;
		if (evt_data->version == RSSI_MONITOR_VERSION) {
			dhd_rssi_monitor_evt_t monitor_data;
			monitor_data.version = DHD_RSSI_MONITOR_EVT_VERSION;
			monitor_data.cur_rssi = evt_data->cur_rssi;
			memcpy(&monitor_data.BSSID, &e->addr, ETHER_ADDR_LEN);
			wl_cfgvendor_send_async_event(wiphy, ndev,
				GOOGLE_RSSI_MONITOR_EVENT,
				&monitor_data, sizeof(monitor_data));
		} else {
			WL_ERR(("Version mismatch %d, expected %d", evt_data->version,
			       RSSI_MONITOR_VERSION));
		}
	}
	return BCME_OK;
}
#else
void wl_cfg80211_set_rmc_pid(int pid)
{
	struct bcm_cfg80211 *cfg = g_bcm_cfg;
	if (pid > 0)
		cfg->rmc_event_pid = pid;
	WL_DBG(("set pid for rmc event : pid=%d\n", pid));
}

#ifdef WLAIBSS
void wl_cfg80211_set_txfail_pid(int pid)
{
	struct bcm_cfg80211 *cfg = g_bcm_cfg;
	if (pid > 0)
		cfg->aibss_txfail_pid = pid;
	WL_DBG(("set pid for aibss fail event : pid=%d\n", pid));
}

static s32
wl_notify_aibss_txfail(struct bcm_cfg80211 *cfg, bcm_struct_cfgdev *cfgdev,
	const wl_event_msg_t *e, void *data)
{
	u32 evt = ntoh32(e->event_type);
	int ret = -1;
#ifdef PCIE_FULL_DONGLE
	dhd_pub_t *dhd = (dhd_pub_t *)(cfg->pub);
	u32 reason = ntoh32(e->reason);
	uint8 ifindex;
	ifindex = (uint8)dhd_ifname2idx(dhd->info, (char*)e->ifname);
#endif
	if (cfg->aibss_txfail_pid != 0) {
#ifdef PCIE_FULL_DONGLE
		if (reason == AIBSS_PEER_FREE) {
			WL_INFORM(("Peer freed. Flow rings delete for peer.\n"));
			dhd_flow_rings_delete_for_peer(dhd, ifindex,
				(void *)&e->addr.octet[0]);
			return 0;
		}
#endif
		ret = wl_netlink_send_msg(cfg->aibss_txfail_pid, AIBSS_EVENT_TXFAIL,
			cfg->aibss_txfail_seq++, (void *)&e->addr, ETHER_ADDR_LEN);
	}

	WL_DBG(("txfail : evt=%d, pid=%d, ret=%d, mac=" MACF "\n",
		evt, cfg->aibss_txfail_pid, ret, ETHERP_TO_MACF(&e->addr)));
	return ret;
}
#endif /* WLAIBSS */
#ifdef WL_RELMCAST
static s32
wl_notify_rmc_status(struct bcm_cfg80211 *cfg, bcm_struct_cfgdev *cfgdev,
	const wl_event_msg_t *e, void *data)
{
	u32 evt = ntoh32(e->event_type);
	u32 reason = ntoh32(e->reason);
	int ret = -1;

	switch (reason) {
		case WLC_E_REASON_RMC_AR_LOST:
		case WLC_E_REASON_RMC_AR_NO_ACK:
			if (cfg->rmc_event_pid != 0) {
				ret = wl_netlink_send_msg(cfg->rmc_event_pid,
					RMC_EVENT_LEADER_CHECK_FAIL,
					cfg->rmc_event_seq++, NULL, 0);
			}
			break;
		default:
			break;
	}
	WL_DBG(("rmcevent : evt=%d, pid=%d, ret=%d\n", evt, cfg->rmc_event_pid, ret));
	return ret;
}
#endif /* WL_RELMCAST */
#endif /* BRCM_RSDB */

static s32
wl_notify_roaming_status(struct bcm_cfg80211 *cfg, bcm_struct_cfgdev *cfgdev,
	const wl_event_msg_t *e, void *data)
{
	bool act;
	struct net_device *ndev = NULL;
	s32 err = 0;
	u32 event = be32_to_cpu(e->event_type);
	u32 status = be32_to_cpu(e->status);
#ifdef WL_TIM_EVENT
	dhd_pub_t *dhd = NULL;
#endif /* WL_TIM_EVENT */
#ifdef  BRCM_RSDB
#ifdef DHD_LOSSLESS_ROAMING
	struct wl_security *sec;
#endif
#endif
	WL_DBG(("Enter \n"));

	ndev = cfgdev_to_wlc_ndev(cfgdev, cfg);

	if ((!cfg->disable_roam_event) && (event == WLC_E_BSSID)) {
		wl_add_remove_eventmsg(ndev, WLC_E_ROAM, false);
		cfg->disable_roam_event = TRUE;
	}

	if ((cfg->disable_roam_event) && (event == WLC_E_ROAM))
		return err;

	if ((event == WLC_E_ROAM || event == WLC_E_BSSID) && status == WLC_E_STATUS_SUCCESS) {
#ifndef  BRCM_RSDB
		if (wl_get_drv_status(cfg, CONNECTED, ndev))
			wl_bss_roaming_done(cfg, ndev, e, data);
		else
			wl_bss_connect_done(cfg, ndev, e, data, true);
		act = true;
		wl_update_prof(cfg, ndev, e, &act, WL_PROF_ACT);
		wl_update_prof(cfg, ndev, NULL, (void *)&e->addr, WL_PROF_BSSID);
	}
#else
		if (wl_get_drv_status(cfg, CONNECTED, ndev)) {
#ifdef DHD_LOSSLESS_ROAMING
			if (cfg->roam_offload) {
				wl_bss_roaming_done(cfg, ndev, e, data);
				wl_del_roam_timeout(cfg);
			}
			else {
				sec = wl_read_prof(cfg, ndev, WL_PROF_SEC);
				/* In order to reduce roaming delay, wl_bss_roaming_done is
				 * early called with WLC_E_LINK event. It is called from
				 * here only if WLC_E_LINK event is blocked for specific
				 * security type.
				 */
				if (IS_AKM_SUITE_FT(sec)) {
					wl_bss_roaming_done(cfg, ndev, e, data);
				}
				/* Roam timer is deleted mostly from wl_cfg80211_change_station
				 * after roaming is finished successfully. We need to delete
				 * the timer from here only for some security types that aren't
				 * using wl_cfg80211_change_station to authorize SCB
				 */
				if (IS_AKM_SUITE_FT(sec) || IS_AKM_SUITE_CCKM(sec)) {
					wl_del_roam_timeout(cfg);
				}
			}
#else
			wl_bss_roaming_done(cfg, ndev, e, data);
#endif /* DHD_LOSSLESS_ROAMING */
		} else {
			wl_bss_connect_done(cfg, ndev, e, data, true);
		}
		act = true;
#ifdef WL_TIM_EVENT
		dhd = (dhd_pub_t *)(cfg->pub);
		/* read associated AP beacon interval */
		if ((err = dhd_wl_ioctl_cmd(dhd, WLC_GET_BCNPRD, &tim_event_chr.bcn_interval,
			sizeof(tim_event_chr.bcn_interval), FALSE, 0)) < 0) {
			WL_ERR(("%s get beacon failed code %d\n", __FUNCTION__, err));
		}

		/* read associated ap's dtim setup */
		if ((err = dhd_wl_ioctl_cmd(dhd, WLC_GET_DTIMPRD, &tim_event_chr.dtim_period,
			sizeof(tim_event_chr.dtim_period), FALSE, 0)) < 0) {
			 WL_ERR(("%s failed code %d\n", __FUNCTION__, err));
		}
#endif
		wl_update_prof(cfg, ndev, e, &act, WL_PROF_ACT);
		wl_update_prof(cfg, ndev, NULL, (const void *)&e->addr, WL_PROF_BSSID);
	}
#ifdef DHD_LOSSLESS_ROAMING
	else if ((event == WLC_E_ROAM || event == WLC_E_BSSID) && status != WLC_E_STATUS_SUCCESS) {
		wl_del_roam_timeout(cfg);
	}
#endif
#endif
	return err;
}
#ifdef  BRCM_RSDB
#ifdef QOS_MAP_SET
/* up range from low to high with up value */
static bool
up_table_set(uint8 *up_table, uint8 up, uint8 low, uint8 high)
{
	int i;

	if (up > 7 || low > high || low >= UP_TABLE_MAX || high >= UP_TABLE_MAX) {
		return FALSE;
	}

	for (i = low; i <= high; i++) {
		up_table[i] = up;
	}

	return TRUE;
}

/* set user priority table */
static void
wl_set_up_table(uint8 *up_table, bcm_tlv_t *qos_map_ie)
{
	uint8 len;

	if (up_table == NULL || qos_map_ie == NULL) {
		return;
	}

	/* clear table to check table was set or not */
	memset(up_table, 0xff, UP_TABLE_MAX);

	/* length of QoS Map IE must be 16+n*2, n is number of exceptions */
	if (qos_map_ie != NULL && qos_map_ie->id == DOT11_MNG_QOS_MAP_ID &&
			(len = qos_map_ie->len) >= QOS_MAP_FIXED_LENGTH &&
			(len % 2) == 0) {
		uint8 *except_ptr = (uint8 *)qos_map_ie->data;
		uint8 except_len = len - QOS_MAP_FIXED_LENGTH;
		uint8 *range_ptr = except_ptr + except_len;
		int i;

		/* fill in ranges */
		for (i = 0; i < QOS_MAP_FIXED_LENGTH; i += 2) {
			uint8 low = range_ptr[i];
			uint8 high = range_ptr[i + 1];
			if (low == 255 && high == 255) {
				continue;
			}

			if (!up_table_set(up_table, i / 2, low, high)) {
				/* clear the table on failure */
				memset(up_table, 0xff, UP_TABLE_MAX);
				return;
			}
		}

		/* update exceptions */
		for (i = 0; i < except_len; i += 2) {
			uint8 dscp = except_ptr[i];
			uint8 up = except_ptr[i+1];

			/* exceptions with invalid dscp/up are ignored */
			up_table_set(up_table, up, dscp, dscp);
		}
	}

	if (wl_dbg_level & WL_DBG_DBG) {
		prhex("UP table", up_table, UP_TABLE_MAX);
	}
}

/* get user priority table */
uint8 *
wl_get_up_table(void)
{
	return (uint8 *)(g_bcm_cfg->up_table);
}
#endif /* QOS_MAP_SET */

#ifdef DHD_LOSSLESS_ROAMING
static s32
wl_notify_roam_prep_status(struct bcm_cfg80211 *cfg, bcm_struct_cfgdev *cfgdev,
	const wl_event_msg_t *e, void *data)
{
	s32 err = 0;
	struct wl_security *sec;
	struct net_device *ndev;
	dhd_pub_t *dhdp = (dhd_pub_t *)(cfg->pub);

	ndev = cfgdev_to_wlc_ndev(cfgdev, cfg);

	sec = wl_read_prof(cfg, ndev, WL_PROF_SEC);
	/* Disable Lossless Roaming for specific AKM suite
	 * Any other AKM suite can be added below if transition time
	 * is delayed because of Lossless Roaming
	 * and it causes any certication failure
	 */
	if (IS_AKM_SUITE_FT(sec)) {
		return err;
	}

	dhdp->dequeue_prec_map = 1 << PRIO_8021D_NC;
	/* Restore flow control  */
	dhd_txflowcontrol(dhdp, ALL_INTERFACES, OFF);

	mod_timer(&cfg->roam_timeout, jiffies + msecs_to_jiffies(WL_ROAM_TIMEOUT_MS));

	return err;
}
#endif /* DHD_LOSSLESS_ROAMING */

static s32
wl_notify_idsup_status(struct bcm_cfg80211 *cfg, bcm_struct_cfgdev *cfgdev,
		const wl_event_msg_t *e, void *data)
{
	s32 err = 0;
#if defined(WL_VENDOR_EXT_SUPPORT)
	u32 idsup_status;
	u32 reason = ntoh32(e->reason);
	struct net_device *ndev = cfgdev_to_wlc_ndev(cfgdev, cfg);
	struct wiphy *wiphy = bcmcfg_to_wiphy(cfg);
#endif /* defined(WL_VENDOR_EXT_SUPPORT) */

	if (cfg->roam_offload) {
#if defined(WL_VENDOR_EXT_SUPPORT)
		switch (reason) {
			case WLC_E_SUP_WPA_PSK_TMO:
				idsup_status = IDSUP_EVENT_4WAY_HANDSHAKE_TIMEOUT;
				break;
			case WLC_E_SUP_OTHER:
				idsup_status = IDSUP_EVENT_SUCCESS;
				break;
			default:
				WL_ERR(("Other type at IDSUP. "
						"event=%d e->status %d e->reason %d \n",
						(int)ntoh32(e->event_type), (int)ntoh32(e->status),
						(int)ntoh32(e->reason)));
				return err;
		}

		err = wl_cfgvendor_send_async_event(wiphy, ndev,
				BRCM_VENDOR_EVENT_IDSUP_STATUS, &idsup_status, sizeof(u32));
#endif /* defined(WL_VENDOR_EXT_SUPPORT) */
	}
	return err;
}

#ifdef CUSTOM_EVENT_PM_WAKE
static s32
wl_check_pmstatus(struct bcm_cfg80211 *cfg, bcm_struct_cfgdev *cfgdev,
		const wl_event_msg_t *e, void *data)
{
	s32 err = 0;
	struct net_device *ndev = NULL;
	u8 *pbuf = NULL;
	ndev = cfgdev_to_wlc_ndev(cfgdev, cfg);

	pbuf = kzalloc(WLC_IOCTL_MEDLEN, GFP_KERNEL);
	if (pbuf == NULL) {
		WL_ERR(("failed to allocate local pbuf\n"));
		return -ENOMEM;
	}

	err = wldev_iovar_getbuf_bsscfg(ndev, "dump",
		"pm", strlen("pm"), pbuf, WLC_IOCTL_MEDLEN, 0, &cfg->ioctl_buf_sync);

	if (err) {
		WL_ERR(("dump ioctl err = %d", err));
	} else {
		WL_ERR(("PM status : %s\n", pbuf));
	}

	if (pbuf) {
		kfree(pbuf);
	}
	return err;
}
#endif /* CUSTOM_EVENT_PM_WAKE */
#endif /* BRCM_RSDB */

#ifdef BCM_BSSID_BLACKLIST
static s32
wl_notify_roam_start_status(struct bcm_cfg80211 *cfg, bcm_struct_cfgdev *cfgdev,
	const wl_event_msg_t *e, void *data)
{
#if (LINUX_VERSION_CODE > KERNEL_VERSION(3, 13, 0)) || defined(WL_VENDOR_EXT_SUPPORT)
	struct net_device *ndev = cfgdev_to_wlc_ndev(cfgdev, cfg);
	struct wiphy *wiphy = bcmcfg_to_wiphy(cfg);
	int event_type;

	event_type = WIFI_EVENT_ROAM_SCAN_STARTED;
	wl_cfgvendor_send_async_event(wiphy, ndev, GOOGLE_ROAM_EVENT_START,
		&event_type, sizeof(int));
#endif /* (LINUX_VERSION_CODE > KERNEL_VERSION(3, 13, 0)) || (WL_VENDOR_EXT_SUPPORT) */

	return BCME_OK;
}
#endif

static s32 wl_get_assoc_ies(struct bcm_cfg80211 *cfg, struct net_device *ndev)
{
	wl_assoc_info_t assoc_info;
	struct wl_connect_info *conn_info = wl_to_conn(cfg);
	s32 err = 0;
#ifdef  BRCM_RSDB
#ifdef QOS_MAP_SET
	bcm_tlv_t * qos_map_ie = NULL;
#endif /* QOS_MAP_SET */
#endif
	WL_DBG(("Enter \n"));
	err = wldev_iovar_getbuf(ndev, "assoc_info", NULL, 0, cfg->extra_buf,
		WL_ASSOC_INFO_MAX, NULL);
	if (unlikely(err)) {
		WL_ERR(("could not get assoc info (%d)\n", err));
		return err;
	}
	memcpy(&assoc_info, cfg->extra_buf, sizeof(wl_assoc_info_t));
	assoc_info.req_len = htod32(assoc_info.req_len);
	assoc_info.resp_len = htod32(assoc_info.resp_len);
	assoc_info.flags = htod32(assoc_info.flags);
#ifdef BCM_PATCH_CVE_2017_13292_13303

	if (assoc_info.req_len >
		(MAX_REQ_LINE + sizeof(struct dot11_assoc_req) +
		((assoc_info.flags & WLC_ASSOC_REQ_IS_REASSOC) ?
		ETHER_ADDR_LEN : 0))) {
		return BCME_BADLEN;
	}
	if ((assoc_info.req_len > 0) &&
	    (assoc_info.req_len < (sizeof(struct dot11_assoc_req) +
		((assoc_info.flags & WLC_ASSOC_REQ_IS_REASSOC) ?
		ETHER_ADDR_LEN : 0)))) {
		return BCME_BADLEN;
	}
	if (assoc_info.resp_len >
		(MAX_REQ_LINE + sizeof(struct dot11_assoc_resp))) {
		return BCME_BADLEN;
	}
	if ((assoc_info.resp_len > 0) &&
		(assoc_info.resp_len < sizeof(struct dot11_assoc_resp))) {
		return BCME_BADLEN;
	}

#endif
	if (conn_info->req_ie_len) {
		conn_info->req_ie_len = 0;
		bzero(conn_info->req_ie, sizeof(conn_info->req_ie));
	}
	if (conn_info->resp_ie_len) {
		conn_info->resp_ie_len = 0;
		bzero(conn_info->resp_ie, sizeof(conn_info->resp_ie));
	}
	if (assoc_info.req_len) {
		err = wldev_iovar_getbuf(ndev, "assoc_req_ies", NULL, 0, cfg->extra_buf,
			WL_ASSOC_INFO_MAX, NULL);
		if (unlikely(err)) {
			WL_ERR(("could not get assoc req (%d)\n", err));
			return err;
		}
		conn_info->req_ie_len = assoc_info.req_len - sizeof(struct dot11_assoc_req);
		if (assoc_info.flags & WLC_ASSOC_REQ_IS_REASSOC) {
			conn_info->req_ie_len -= ETHER_ADDR_LEN;
		}
#ifdef HW_WIFI_SECURITY_PATCH
		if ((conn_info->req_ie_len <= MAX_REQ_LINE) && (conn_info->req_ie_len > 0))
#else
		if (conn_info->req_ie_len <= MAX_REQ_LINE)
#endif
			memcpy(conn_info->req_ie, cfg->extra_buf, conn_info->req_ie_len);
		else {
			WL_ERR(("IE size %d above max %d size \n",
				conn_info->req_ie_len, MAX_REQ_LINE));
			return err;
		}
	} else {
		conn_info->req_ie_len = 0;
	}
	if (assoc_info.resp_len) {
		err = wldev_iovar_getbuf(ndev, "assoc_resp_ies", NULL, 0, cfg->extra_buf,
			WL_ASSOC_INFO_MAX, NULL);
		if (unlikely(err)) {
			WL_ERR(("could not get assoc resp (%d)\n", err));
			return err;
		}
		conn_info->resp_ie_len = assoc_info.resp_len -sizeof(struct dot11_assoc_resp);
#ifdef HW_WIFI_SECURITY_PATCH
		if ((conn_info->resp_ie_len <= MAX_REQ_LINE) && (conn_info->resp_ie_len > 0))
#else
		if (conn_info->resp_ie_len <= MAX_REQ_LINE)
#endif
			memcpy(conn_info->resp_ie, cfg->extra_buf, conn_info->resp_ie_len);
		else {
			WL_ERR(("IE size %d above max %d size \n",
				conn_info->resp_ie_len, MAX_REQ_LINE));
			return err;
		}
#ifdef  BRCM_RSDB
#ifdef QOS_MAP_SET
		/* find qos map set ie */
		if ((qos_map_ie = bcm_parse_tlvs(conn_info->resp_ie, conn_info->resp_ie_len,
				DOT11_MNG_QOS_MAP_ID)) != NULL) {
			WL_DBG((" QoS map set IE found in assoc response\n"));
			if (!cfg->up_table) {
				cfg->up_table = kmalloc(UP_TABLE_MAX, GFP_KERNEL);
			}
			wl_set_up_table(cfg->up_table, qos_map_ie);
		} else {
			kfree(cfg->up_table);
			cfg->up_table = NULL;
		}
#endif /* QOS_MAP_SET */
#endif
	} else {
		conn_info->resp_ie_len = 0;
	}
	WL_DBG(("req len (%d) resp len (%d)\n", conn_info->req_ie_len,
		conn_info->resp_ie_len));

	return err;
}
#ifndef  BRCM_RSDB
static void wl_ch_to_chanspec(int ch, struct wl_join_params *join_params,
        size_t *join_params_size)
{
#ifndef ROAM_CHANNEL_CACHE
	chanspec_t chanspec = 0;
#endif

	if (ch != 0) {
#ifdef ROAM_CHANNEL_CACHE
		int n_channels;

		n_channels = get_roam_channel_list(ch, join_params->params.chanspec_list,
			&join_params->ssid, ioctl_version);
		join_params->params.chanspec_num = htod32(n_channels);
		*join_params_size += WL_ASSOC_PARAMS_FIXED_SIZE +
			join_params->params.chanspec_num * sizeof(chanspec_t);
#else
		join_params->params.chanspec_num = 1;
		join_params->params.chanspec_list[0] = ch;

		if (join_params->params.chanspec_list[0] <= CH_MAX_2G_CHANNEL)
			chanspec |= WL_CHANSPEC_BAND_2G;
		else
			chanspec |= WL_CHANSPEC_BAND_5G;

		chanspec |= WL_CHANSPEC_BW_20;
		chanspec |= WL_CHANSPEC_CTL_SB_NONE;

		*join_params_size += WL_ASSOC_PARAMS_FIXED_SIZE +
			join_params->params.chanspec_num * sizeof(chanspec_t);

		join_params->params.chanspec_list[0]  &= WL_CHANSPEC_CHAN_MASK;
		join_params->params.chanspec_list[0] |= chanspec;
		join_params->params.chanspec_list[0] =
			wl_chspec_host_to_driver(join_params->params.chanspec_list[0]);

		join_params->params.chanspec_num =
			htod32(join_params->params.chanspec_num);
#endif /* ROAM_CHANNEL_CACHE */
		WL_DBG(("join_params->params.chanspec_list[0]= %X, %d channels\n",
			join_params->params.chanspec_list[0],
			join_params->params.chanspec_num));
	}
}
#else
static s32 wl_ch_to_chanspec(struct net_device *dev, int ch, struct wl_join_params *join_params,
        size_t *join_params_size)
{
	struct bcm_cfg80211 *cfg;
	s32 bssidx = -1;
	chanspec_t chanspec = 0, chspec;

	if (ch != 0) {
		cfg = (struct bcm_cfg80211 *)wiphy_priv(dev->ieee80211_ptr->wiphy);
		if (cfg && cfg->rcc_enabled) {
#ifdef ROAM_CHANNEL_CACHE
			/* If ROAM CACHE is enabled, use the cached channel list */
			int n_channels;
			n_channels = get_roam_channel_list(ch, join_params->params.chanspec_list,
				&join_params->ssid, ioctl_version);
			join_params->params.chanspec_num = htod32(n_channels);
			*join_params_size += WL_ASSOC_PARAMS_FIXED_SIZE +
				join_params->params.chanspec_num * sizeof(chanspec_t);
#endif /* ROAM_CHANNEL_CACHE */
		} else {
			join_params->params.chanspec_num = 1;
			join_params->params.chanspec_list[0] = ch;

			if (join_params->params.chanspec_list[0] <= CH_MAX_2G_CHANNEL)
				chanspec |= WL_CHANSPEC_BAND_2G;
			else
				chanspec |= WL_CHANSPEC_BAND_5G;

			/* Get the min_bw set for the interface */
			chspec = wl_cfg80211_ulb_get_min_bw_chspec(dev->ieee80211_ptr, bssidx);
			if (chspec == INVCHANSPEC) {
				WL_ERR(("Invalid chanspec \n"));
				return -EINVAL;
			}
			chanspec |= chspec;
			chanspec |= WL_CHANSPEC_CTL_SB_NONE;

			*join_params_size += WL_ASSOC_PARAMS_FIXED_SIZE +
				join_params->params.chanspec_num * sizeof(chanspec_t);

			join_params->params.chanspec_list[0]  &= WL_CHANSPEC_CHAN_MASK;
			join_params->params.chanspec_list[0] |= chanspec;
			join_params->params.chanspec_list[0] =
				wl_chspec_host_to_driver(join_params->params.chanspec_list[0]);

			join_params->params.chanspec_num =
				htod32(join_params->params.chanspec_num);
		}

		WL_DBG(("join_params->params.chanspec_list[0]= %X, %d channels\n",
			join_params->params.chanspec_list[0],
			join_params->params.chanspec_num));
	}
	return 0;
}
#endif
static s32 wl_update_bss_info(struct bcm_cfg80211 *cfg, struct net_device *ndev, bool roam)
{
#ifndef  BRCM_RSDB
	struct cfg80211_bss *bss = NULL;
#endif
	struct wl_bss_info *bi;
#ifndef  BRCM_RSDB
	struct wlc_ssid *ssid;
#endif
	struct bcm_tlv *tim;
	s32 beacon_interval;
	s32 dtim_period = 0;
	size_t ie_len;
	u8 *ie;
	u8 *curbssid;
	s32 err = 0;
#ifndef  BRCM_RSDB
	struct wiphy *wiphy;
#endif
	u32 channel;
	char *buf;
#ifndef  BRCM_RSDB
#ifdef  ROAM_CHANNEL_CACHE
	struct ieee80211_channel *cur_channel;
	u32 freq, band;
#endif /* ROAM_CHANNEL_CACHE */

	wiphy = bcmcfg_to_wiphy(cfg);

	ssid = (struct wlc_ssid *)wl_read_prof(cfg, ndev, WL_PROF_SSID);
#endif /* BRCM_RSDB */
	curbssid = wl_read_prof(cfg, ndev, WL_PROF_BSSID);
	ASSERT(curbssid);
#ifndef  BRCM_RSDB
        if (ssid) {
	        bss = cfg80211_get_bss(wiphy, NULL, curbssid,
		ssid->SSID, ssid->SSID_len, WLAN_CAPABILITY_ESS,
		WLAN_CAPABILITY_ESS);
        }
#endif
	mutex_lock(&cfg->usr_sync);
	buf = kzalloc(WL_EXTRA_BUF_MAX, GFP_ATOMIC);
	if (!buf) {
		WL_ERR(("buffer alloc failed.\n"));
                mutex_unlock(&cfg->usr_sync);
		return BCME_NOMEM;
	}
	*(u32 *)buf = htod32(WL_EXTRA_BUF_MAX);
	err = wldev_ioctl(ndev, WLC_GET_BSS_INFO,
		buf, WL_EXTRA_BUF_MAX, false);
	if (unlikely(err)) {
		WL_ERR(("Could not get bss info %d\n", err));
		goto update_bss_info_out;
	}
	bi = (struct wl_bss_info *)(buf + 4);
	channel = wf_chspec_ctlchan(wl_chspec_driver_to_host(bi->chanspec));
	wl_update_prof(cfg, ndev, NULL, &channel, WL_PROF_CHAN);
#ifndef  BRCM_RSDB
	if (!bss) {
		WL_DBG(("Could not find the AP\n"));
#endif
		if (memcmp(bi->BSSID.octet, curbssid, ETHER_ADDR_LEN)) {
			WL_ERR(("Bssid doesn't match\n"));
			err = -EIO;
			goto update_bss_info_out;
		}
		err = wl_inform_single_bss(cfg, bi, roam);
		if (unlikely(err))
			goto update_bss_info_out;

		ie = ((u8 *)bi) + bi->ie_offset;
		ie_len = bi->ie_length;
		beacon_interval = cpu_to_le16(bi->beacon_period);
#ifndef  BRCM_RSDB
	} else {
		WL_DBG(("Found the AP in the list - BSSID %pM\n", bss->bssid));
#ifdef  ROAM_CHANNEL_CACHE
#if LINUX_VERSION_CODE == KERNEL_VERSION(2, 6, 38) && !defined(WL_COMPAT_WIRELESS)
		freq = ieee80211_channel_to_frequency(channel);
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0))
		band = (channel <= CH_MAX_2G_CHANNEL) ? NL80211_BAND_2GHZ : NL80211_BAND_5GHZ;
		freq = ieee80211_channel_to_frequency(channel, band);
#else
		band = (channel <= CH_MAX_2G_CHANNEL) ? IEEE80211_BAND_2GHZ : IEEE80211_BAND_5GHZ;
		freq = ieee80211_channel_to_frequency(channel, band);
#endif
		cur_channel = ieee80211_get_channel(wiphy, freq);
		bss->channel = cur_channel;
#endif /* ROAM_CHANNEL_CACHE */
#if defined(WL_CFG80211_P2P_DEV_IF)
		ie = (u8 *)bss->ies->data;
		ie_len = bss->ies->len;
#else
		ie = bss->information_elements;
		ie_len = bss->len_information_elements;
#endif /* WL_CFG80211_P2P_DEV_IF */
		beacon_interval = bss->beacon_interval;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0))
		cfg80211_put_bss(wiphy, bss);
#else
		cfg80211_put_bss(bss);
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0) */
	}
#endif
	tim = bcm_parse_tlvs(ie, ie_len, WLAN_EID_TIM);
	if (tim) {
		dtim_period = tim->data[1];
	} else {
		/*
		* active scan was done so we could not get dtim
		* information out of probe response.
		* so we speficially query dtim information.
		*/
		err = wldev_ioctl(ndev, WLC_GET_DTIMPRD,
			&dtim_period, sizeof(dtim_period), false);
		if (unlikely(err)) {
			WL_ERR(("WLC_GET_DTIMPRD error (%d)\n", err));
			goto update_bss_info_out;
		}
	}

	wl_update_prof(cfg, ndev, NULL, &beacon_interval, WL_PROF_BEACONINT);
	wl_update_prof(cfg, ndev, NULL, &dtim_period, WL_PROF_DTIMPERIOD);

update_bss_info_out:
	if (unlikely(err)) {
		WL_ERR(("Failed with error %d\n", err));
	}
	kfree(buf);
	mutex_unlock(&cfg->usr_sync);
	return err;
}

static s32
wl_bss_roaming_done(struct bcm_cfg80211 *cfg, struct net_device *ndev,
	const wl_event_msg_t *e, void *data)
{
	struct wl_connect_info *conn_info = wl_to_conn(cfg);
	s32 err = 0;
	u8 *curbssid;
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 39)) || defined(WL_COMPAT_WIRELESS)
	struct wiphy *wiphy = bcmcfg_to_wiphy(cfg);
	struct ieee80211_supported_band *band;
	struct ieee80211_channel *notify_channel = NULL;
	u32 *channel;
	u32 freq;
#endif /* LINUX_VERSION > 2.6.39 || WL_COMPAT_WIRELESS */
#ifdef WLFBT
	uint32 data_len = 0;
	if (data)
		data_len = ntoh32(e->datalen);
#endif /* WLFBT */

#ifdef  BRCM_RSDB
	if (memcmp(&cfg->last_roamed_addr, &e->addr, ETHER_ADDR_LEN) == 0) {
		WL_INFORM(("BSSID already updated\n"));
		return err;
	}

#ifndef BCM_PATCH_FOR_ROAM_EVENT
	/* Skip calling cfg80211_roamed If current bssid and
	 * roamed bssid are same. Also clear timer roam_timeout.
	 */
	curbssid = wl_read_prof(cfg, ndev, WL_PROF_BSSID);
	if (memcmp(curbssid, &e->addr, ETHER_ADDR_LEN) == 0) {
		WL_ERR(("BSS already present, Skipping roamed event to upper layer\n"));
#ifdef DHD_LOSSLESS_ROAMING
		wl_del_roam_timeout(cfg);
#endif  /* DHD_LOSSLESS_ROAMING */
		return  err;
	}
#endif /* BCM_PATCH_FOR_ROAM_EVENT */
#endif /* BRCM_RSDB */
	wl_get_assoc_ies(cfg, ndev);
#ifndef  BRCM_RSDB
	wl_update_prof(cfg, ndev, NULL, (void *)(e->addr.octet), WL_PROF_BSSID);
#else
	wl_update_prof(cfg, ndev, NULL, (const void *)(e->addr.octet), WL_PROF_BSSID);
#endif
	curbssid = wl_read_prof(cfg, ndev, WL_PROF_BSSID);
	wl_update_bss_info(cfg, ndev, true);
	wl_update_pmklist(ndev, cfg->pmk_list, err);

#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 39)) || defined(WL_COMPAT_WIRELESS)
	/* channel info for cfg80211_roamed introduced in 2.6.39-rc1 */
	channel = (u32 *)wl_read_prof(cfg, ndev, WL_PROF_CHAN);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0))
	if (*channel <= CH_MAX_2G_CHANNEL)
		band = wiphy->bands[NL80211_BAND_2GHZ];
	else
		band = wiphy->bands[NL80211_BAND_5GHZ];
#else
	if (*channel <= CH_MAX_2G_CHANNEL)
		band = wiphy->bands[IEEE80211_BAND_2GHZ];
	else
		band = wiphy->bands[IEEE80211_BAND_5GHZ];
#endif
	freq = ieee80211_channel_to_frequency(*channel, band->band);
	notify_channel = ieee80211_get_channel(wiphy, freq);
#endif /* LINUX_VERSION > 2.6.39  || WL_COMPAT_WIRELESS */
#ifndef  BRCM_RSDB
	printf("wl_bss_roaming_done succeeded to " MACDBG "\n",
		MAC2STRDBG((u8*)(&e->addr)));
#ifdef PCIE_FULL_DONGLE
	wl_roam_flowring_cleanup(cfg);
#endif /* PCIE_FULL_DONGLE */
#else  /* BRCM_RSDB */
	WL_ERR(("wl_bss_roaming_done succeeded to " MACDBG "\n",
		MAC2STRDBG((const u8*)(&e->addr))));
#endif /* BRCM_RSDB */

#ifdef WLFBT
	/* back up the given FBT key for the further supplicant request,
	 * currently not checking the FBT is enabled for current BSS in DHD,
	 * because the supplicant decides to take it or not.
	 */
	if (data && (data_len == FBT_KEYLEN)) {
		memcpy(cfg->fbt_key, data, FBT_KEYLEN);
	}
#endif /* WLFBT */
	cfg80211_roamed(ndev,
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 39)) || defined(WL_COMPAT_WIRELESS)
		notify_channel,
#endif
		curbssid,
		conn_info->req_ie, conn_info->req_ie_len,
		conn_info->resp_ie, conn_info->resp_ie_len, GFP_KERNEL);
	WL_DBG(("Report roaming result\n"));
#ifdef  BRCM_RSDB
	memcpy(&cfg->last_roamed_addr, (void *)&e->addr, ETHER_ADDR_LEN);
#endif
	wl_set_drv_status(cfg, CONNECTED, ndev);
#ifdef  BRCM_RSDB
#if defined(DHD_ENABLE_BIGDATA_LOGGING)
	cfg->roam_count++;
#endif /* DHD_ENABLE_BIGDATA_LOGGING */
#endif
	return err;
}

static s32
wl_bss_connect_done(struct bcm_cfg80211 *cfg, struct net_device *ndev,
	const wl_event_msg_t *e, void *data, bool completed)
{
	struct wl_connect_info *conn_info = wl_to_conn(cfg);
	struct wl_security *sec = wl_read_prof(cfg, ndev, WL_PROF_SEC);
#if (defined(ROAM_ENABLE) && defined(ROAM_AP_ENV_DETECTION)) || \
	defined(CUSTOM_SET_CPUCORE) || defined(HW_PATCH_FOR_HANG)
	dhd_pub_t *dhd = (dhd_pub_t *)(cfg->pub);
#endif /* (ROAM_ENABLE && ROAM_AP_ENV_DETECTION) || CUSTOM_SET_CPUCORE || HW_PATCH_FOR_HANG */
	s32 err = 0;
	u8 *curbssid = wl_read_prof(cfg, ndev, WL_PROF_BSSID);
#ifdef HW_WIFI_ASSOC_STATUS
	u16 hwstatus = 0;
#endif
	if (!sec) {
		WL_ERR(("sec is NULL\n"));
		return -ENODEV;
	}
	WL_DBG((" enter\n"));
#ifdef ESCAN_RESULT_PATCH
	if (wl_get_drv_status(cfg, CONNECTED, ndev)) {
		if (memcmp(curbssid, connect_req_bssid, ETHER_ADDR_LEN) == 0) {
			WL_DBG((" Connected event of connected device e=%d s=%d, ignore it\n",
				ntoh32(e->event_type), ntoh32(e->status)));
			return err;
		}
	}
	if (memcmp(curbssid, broad_bssid, ETHER_ADDR_LEN) == 0 &&
		memcmp(broad_bssid, connect_req_bssid, ETHER_ADDR_LEN) != 0) {
		WL_DBG(("copy bssid\n"));
		memcpy(curbssid, connect_req_bssid, ETHER_ADDR_LEN);
	}
#ifdef  BRCM_RSDB
#if defined(BCM4334_CHIP)
	if (cfg->scan_request) {
		wl_notify_escan_complete(cfg, ndev, true, true);
	}
#endif
#endif
#else
	if (cfg->scan_request) {
		wl_notify_escan_complete(cfg, ndev, true, true);
	}
#endif /* ESCAN_RESULT_PATCH */
	if (wl_get_drv_status(cfg, CONNECTING, ndev)) {
		wl_cfg80211_scan_abort(cfg);
		wl_clr_drv_status(cfg, CONNECTING, ndev);
		if (completed) {
			wl_get_assoc_ies(cfg, ndev);
#ifndef  BRCM_RSDB
			wl_update_prof(cfg, ndev, NULL, (void *)(e->addr.octet), WL_PROF_BSSID);
#else
			wl_update_prof(cfg, ndev, NULL, (const void *)(e->addr.octet), WL_PROF_BSSID);
#endif
			curbssid = wl_read_prof(cfg, ndev, WL_PROF_BSSID);
			wl_update_bss_info(cfg, ndev, false);
			wl_update_pmklist(ndev, cfg->pmk_list, err);
			wl_set_drv_status(cfg, CONNECTED, ndev);
#if defined(ROAM_ENABLE) && defined(ROAM_AP_ENV_DETECTION)
			if (dhd->roam_env_detection)
				wldev_iovar_setint(ndev, "roam_env_detection",
					AP_ENV_INDETERMINATE);
#endif /* ROAM_AP_ENV_DETECTION */
			if (ndev != bcmcfg_to_prmry_ndev(cfg)) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 13, 0)
				init_completion(&cfg->iface_disable);
#else
				/* reinitialize completion to clear previous count */
				INIT_COMPLETION(cfg->iface_disable);
#endif
			}
#ifdef CUSTOM_SET_CPUCORE
			if (wl_get_chan_isvht80(ndev, dhd)) {
				if (ndev == bcmcfg_to_prmry_ndev(cfg))
					dhd->chan_isvht80 |= DHD_FLAG_STA_MODE; /* STA mode */
#ifndef  BRCM_RSDB
				else if (ndev == wl_to_p2p_bss_ndev(cfg, P2PAPI_BSSCFG_CONNECTION))
#else
				else if (is_p2p_group_iface(ndev->ieee80211_ptr))
#endif
					dhd->chan_isvht80 |= DHD_FLAG_P2P_MODE; /* p2p mode */
				dhd_set_cpucore(dhd, TRUE);
			}
#endif /* CUSTOM_SET_CPUCORE */
#ifdef  BRCM_RSDB
			memset(&cfg->last_roamed_addr, 0, ETHER_ADDR_LEN);
#endif
		}
#ifdef HW_WIFI_ASSOC_STATUS
		hwstatus = (u16)WLAN_STATUS_SUCCESS;
		if (!completed) {
			hwstatus = (u16)WLAN_STATUS_UNSPECIFIED_FAILURE;
			if (sec->auth_assoc_res_status) {
				hwstatus = (u16)sec->auth_assoc_res_status;
			} else if (e && (ntoh32(e->event_type) == WLC_E_SET_SSID) && ntoh32(e->status)) {
				hwstatus = (u16)ntoh32(e->status);
			}
			WL_ERR(("connection failed - status: %u\n", hwstatus));
		}
#ifdef HW_PATCH_FOR_HANG
		if (dhd && hw_need_hang_with_assoc_status(hwstatus)) {
			WL_ERR(("hw_need_hang_with_assoc_status: %u\n", hwstatus));
#ifdef BRCM_RSDB
			dhd->hang_reason = HANG_REASON_WLC_MEMORY_LEAK;
#endif
			dhd_os_send_hang_message(dhd);
#ifdef HW_WIFI_DMD_LOG
			hw_wifi_dsm_client_notify(DSM_WIFI_WLC_SET_SSID_ERROR, "hang for assoc status %d", hwstatus);
#endif
		}
#endif /* HW_PATCH_FOR_HANG */
#endif
		cfg80211_connect_result(ndev,
			curbssid,
			conn_info->req_ie,
			conn_info->req_ie_len,
			conn_info->resp_ie,
			conn_info->resp_ie_len,
#ifdef HW_WIFI_ASSOC_STATUS
			hwstatus,
#else
			completed ? WLAN_STATUS_SUCCESS :
			(sec->auth_assoc_res_status) ?
			sec->auth_assoc_res_status :
			WLAN_STATUS_UNSPECIFIED_FAILURE,
#endif
			GFP_KERNEL);
		if (completed)
			WL_INFORM(("Report connect result - connection succeeded\n"));
		else
			WL_ERR(("Report connect result - connection failed\n"));
	}
#ifdef CONFIG_TCPACK_FASTTX
	if (wl_get_chan_isvht80(ndev, dhd))
		wldev_iovar_setint(ndev, "tcpack_fast_tx", 0);
	else
		wldev_iovar_setint(ndev, "tcpack_fast_tx", 1);
#endif /* CONFIG_TCPACK_FASTTX */

	return err;
}

static s32
wl_notify_mic_status(struct bcm_cfg80211 *cfg, bcm_struct_cfgdev *cfgdev,
	const wl_event_msg_t *e, void *data)
{
	struct net_device *ndev = NULL;
	u16 flags = ntoh16(e->flags);
	enum nl80211_key_type key_type;

	ndev = cfgdev_to_wlc_ndev(cfgdev, cfg);

	mutex_lock(&cfg->usr_sync);
	if (flags & WLC_EVENT_MSG_GROUP)
		key_type = NL80211_KEYTYPE_GROUP;
	else
		key_type = NL80211_KEYTYPE_PAIRWISE;
#ifndef  BRCM_RSDB
	cfg80211_michael_mic_failure(ndev, (u8 *)&e->addr, key_type, -1,
		NULL, GFP_KERNEL);
#else
	cfg80211_michael_mic_failure(ndev, (const u8 *)&e->addr, key_type, -1,
		NULL, GFP_KERNEL);
#endif
	mutex_unlock(&cfg->usr_sync);

	return 0;
}

#ifdef BT_WIFI_HANDOVER
static s32
wl_notify_bt_wifi_handover_req(struct bcm_cfg80211 *cfg, bcm_struct_cfgdev *cfgdev,
	const wl_event_msg_t *e, void *data)
{
	struct net_device *ndev = NULL;
	u32 event = ntoh32(e->event_type);
	u32 datalen = ntoh32(e->datalen);
	s32 err;

	WL_ERR(("wl_notify_bt_wifi_handover_req: event_type : %d, datalen : %d\n", event, datalen));
	ndev = cfgdev_to_wlc_ndev(cfgdev, cfg);
	err = wl_genl_send_msg(ndev, event, data, (u16)datalen, 0, 0);

	return err;
}
#endif /* BT_WIFI_HANDOVER */

#if defined(DHD_DEBUG) && defined(BCM_PATCH_DEBUG_BEACON_LOSS)
static s32 wl_notify_beacon_event(struct bcm_cfg80211 *cfg, bcm_struct_cfgdev *cfgdev,
	const wl_event_msg_t *e, void *data)
{
	struct net_device *ndev = NULL;
	static char iovar_buf[WLC_IOCTL_MAXLEN];
	wl_cnt_t *wl_cnt;
	u32 event = ntoh32(e->event_type);
	u32 reason = ntoh32(e->reason);
	s32 pm = PM_OFF;
	int rssi = 0;
	s32 ret = 0;

	bzero(iovar_buf, WLC_IOCTL_MAXLEN);
	ndev = cfgdev_to_wlc_ndev(cfgdev, cfg);

	WL_ERR(("receive event: %d, reason: %d\n", event, reason));

	if(reason == WLC_E_REASON_BCNS_LOST) {
		/*
		 * get current rssi before open firmware log.
		 * if rssi is lower than -82, beacon loss may because of poor signal.
		 * this only for better signal.
		 */
		ret = wldev_get_rssi(ndev, &rssi);
		if (unlikely(ret)) {
			WL_ERR(("failed to get current rssi, do not dump fw log!\n"));
			return ret;
		}

		WL_ERR(("current rssi: %d\n", rssi));
		if (rssi < -82) {
			WL_ERR(("beacon loss because of poor rssi, do not dump fw log!\n"));
			return ret;
		}

		dhd_console_ms = 100;
		fw_log_open_by_beacon_loss = TRUE;

		/*dump pm status*/
		ret = wldev_ioctl(ndev, WLC_GET_PM, &pm, sizeof(pm), false);
		if (unlikely(ret))
			WL_ERR(("failed to get pm status\n"));
		else
			WL_ERR(("get pm status: %d\n", pm));

		/*dump counters status*/
		ret = wldev_iovar_getbuf(ndev, "counters", NULL, 0,
				iovar_buf, WLC_IOCTL_MAXLEN, NULL);
		if (unlikely(ret)) {
			WL_ERR(("error (%d) - size = %zu\n", ret, sizeof(wl_cnt_t)));
			return ret;
		}
		wl_cnt = (wl_cnt_t *)iovar_buf;
#ifdef HW_LINK_COUNTERS
		hw_counters_hex_dump(wl_cnt);
#endif
	} else if (reason == WLC_E_REASON_BCNS_RECOVERY) {
		if (fw_log_open_by_beacon_loss == TRUE) {
			dhd_console_ms = 0;
			fw_log_open_by_beacon_loss = FALSE;
		}
	}

	return ret;
}
#endif

#ifdef PNO_SUPPORT
static s32
wl_notify_pfn_status(struct bcm_cfg80211 *cfg, bcm_struct_cfgdev *cfgdev,
	const wl_event_msg_t *e, void *data)
{
	struct net_device *ndev = NULL;
#ifndef  BRCM_RSDB
#ifdef GSCAN_SUPPORT
	void *ptr;
	int send_evt_bytes = 0;
	u32 event = be32_to_cpu(e->event_type);
	struct wiphy *wiphy = bcmcfg_to_wiphy(cfg);
#endif /* GSCAN_SUPPORT */
#endif
	WL_ERR((">>> PNO Event\n"));

#ifdef BCM_PATCH_SECURITY_2017_04
	if (!data) {
		WL_ERR(("Data is NULL!\n"));
		return 0;
	}
#endif

	ndev = cfgdev_to_wlc_ndev(cfgdev, cfg);
#ifndef  BRCM_RSDB
#ifdef GSCAN_SUPPORT
	ptr = dhd_dev_process_epno_result(ndev, data, event, &send_evt_bytes);
	if (ptr) {
		wl_cfgvendor_send_async_event(wiphy, ndev,
			GOOGLE_SCAN_EPNO_EVENT, ptr, send_evt_bytes);
		kfree(ptr);
	}
	if (!dhd_dev_is_legacy_pno_enabled(ndev))
		return 0;
#endif /* GSCAN_SUPPORT */
#endif
#ifndef WL_SCHED_SCAN
	mutex_lock(&cfg->usr_sync);
	/* TODO: Use cfg80211_sched_scan_results(wiphy); */
	CFG80211_DISCONNECTED(ndev, 0, NULL, 0, false, GFP_KERNEL);
	mutex_unlock(&cfg->usr_sync);
#else
	/* If cfg80211 scheduled scan is supported, report the pno results via sched
	 * scan results
	 */
	wl_notify_sched_scan_results(cfg, ndev, e, data);
#endif /* WL_SCHED_SCAN */
	return 0;
}
#endif /* PNO_SUPPORT */

#ifdef GSCAN_SUPPORT
static s32
wl_notify_gscan_event(struct bcm_cfg80211 *cfg, bcm_struct_cfgdev *cfgdev,
	const wl_event_msg_t *e, void *data)
{
	s32 err = 0;
	u32 event = be32_to_cpu(e->event_type);
	void *ptr;
	int send_evt_bytes = 0;
#ifdef BCM_PATCH_GSCAN
	int event_type;
#else
	int batch_event_result_dummy = 0;
#endif
	struct net_device *ndev = cfgdev_to_wlc_ndev(cfgdev, cfg);
	struct wiphy *wiphy = bcmcfg_to_wiphy(cfg);
	u32 len = ntoh32(e->datalen);

	switch (event) {
#ifndef BCM_PATCH_SECURITY_2017_07
		case WLC_E_PFN_SWC:
			ptr = dhd_dev_swc_scan_event(ndev, data, &send_evt_bytes);
			if (send_evt_bytes) {
				wl_cfgvendor_send_async_event(wiphy, ndev,
				    GOOGLE_GSCAN_SIGNIFICANT_EVENT, ptr, send_evt_bytes);
				kfree(ptr);
			}
			break;
#endif
		case WLC_E_PFN_BEST_BATCHING:
			err = dhd_dev_retrieve_batch_scan(ndev);
			if (err < 0) {
				WL_ERR(("Batch retrieval already in progress %d\n", err));
			} else {
#ifdef BCM_PATCH_GSCAN
				event_type = WIFI_SCAN_THRESHOLD_NUM_SCANS;
				if (data && len) {
					event_type = *((int *)data);
				}
				wl_cfgvendor_send_async_event(wiphy, ndev,
				    GOOGLE_GSCAN_BATCH_SCAN_EVENT,
				     &event_type, sizeof(int));
#else
				wl_cfgvendor_send_async_event(wiphy, ndev,
				    GOOGLE_GSCAN_BATCH_SCAN_EVENT,
				     &batch_event_result_dummy, sizeof(int));
#endif
			}
			break;
		case WLC_E_PFN_SCAN_COMPLETE:
#ifdef BCM_PATCH_GSCAN
			event_type = WIFI_SCAN_COMPLETE;
			wl_cfgvendor_send_async_event(wiphy, ndev,
				GOOGLE_SCAN_COMPLETE_EVENT,
				&event_type, sizeof(int));
#else
			batch_event_result_dummy = WIFI_SCAN_COMPLETE;
			wl_cfgvendor_send_async_event(wiphy, ndev,
				GOOGLE_SCAN_COMPLETE_EVENT,
				&batch_event_result_dummy, sizeof(int));
#endif
			break;
		case WLC_E_PFN_BSSID_NET_FOUND:
			ptr = dhd_dev_hotlist_scan_event(ndev, data, &send_evt_bytes,
			      HOTLIST_FOUND);
			if (ptr) {
				wl_cfgvendor_send_hotlist_event(wiphy, ndev,
				 ptr, send_evt_bytes, GOOGLE_GSCAN_GEOFENCE_FOUND_EVENT);
				dhd_dev_gscan_hotlist_cache_cleanup(ndev, HOTLIST_FOUND);
#ifndef  BRCM_RSDB
			} else
				err = -ENOMEM;
#else
			}
#endif
			break;
		case WLC_E_PFN_BSSID_NET_LOST:
			/* WLC_E_PFN_BSSID_NET_LOST is conflict shared with WLC_E_PFN_SCAN_ALLGONE
			 * We currently do not use WLC_E_PFN_SCAN_ALLGONE, so if we get it, ignore
			 */
			if (len) {
				ptr = dhd_dev_hotlist_scan_event(ndev, data, &send_evt_bytes,
				            HOTLIST_LOST);
				if (ptr) {
					wl_cfgvendor_send_hotlist_event(wiphy, ndev,
					 ptr, send_evt_bytes, GOOGLE_GSCAN_GEOFENCE_LOST_EVENT);
					dhd_dev_gscan_hotlist_cache_cleanup(ndev, HOTLIST_LOST);
#ifndef  BRCM_RSDB		
				} else
					err = -ENOMEM;
			} else
				err = -EINVAL;
#else
				}
			}
#endif
			break;
		case WLC_E_PFN_GSCAN_FULL_RESULT:
#ifndef BCM_PATCH_SECURITY_2017_07
			ptr = dhd_dev_process_full_gscan_result(ndev, data, &send_evt_bytes);
#else
			ptr = dhd_dev_process_full_gscan_result(ndev, data, len, &send_evt_bytes);
#endif
			if (ptr) {
				wl_cfgvendor_send_async_event(wiphy, ndev,
				    GOOGLE_SCAN_FULL_RESULTS_EVENT, ptr, send_evt_bytes);

				kfree(ptr);
			} 
			break;
#ifndef  BRCM_RSDB	
		case WLC_E_PFN_NET_FOUND:
			ptr = dhd_dev_process_anqpo_result(ndev, data, event, &len);
			if (ptr) {
				wl_cfgvendor_send_async_event(wiphy, ndev,
				    GOOGLE_PNO_HOTSPOT_FOUND_EVENT, ptr, len);
				kfree(ptr);
			} else
				err = -ENOMEM;
			break;
#endif
		default:
#ifndef  BRCM_RSDB
			WL_ERR(("Unknown event %d\n", event));
			break;
#else
			WL_ERR(("%s: Unexpected event! - %d\n", __FUNCTION__, event));
#endif
	}
	return err;
}
#endif /* GSCAN_SUPPORT */

static s32
wl_notify_scan_status(struct bcm_cfg80211 *cfg, bcm_struct_cfgdev *cfgdev,
	const wl_event_msg_t *e, void *data)
{
	struct channel_info channel_inform;
	struct wl_scan_results *bss_list;
	struct net_device *ndev = NULL;
	u32 len = WL_SCAN_BUF_MAX;
	s32 err = 0;
	unsigned long flags;

	WL_DBG(("Enter \n"));
	if (!wl_get_drv_status(cfg, SCANNING, ndev)) {
		WL_ERR(("scan is not ready \n"));
		return err;
	}
	ndev = cfgdev_to_wlc_ndev(cfgdev, cfg);

	mutex_lock(&cfg->usr_sync);
	wl_clr_drv_status(cfg, SCANNING, ndev);
	memset(&channel_inform, 0, sizeof(channel_inform));
	err = wldev_ioctl(ndev, WLC_GET_CHANNEL, &channel_inform,
		sizeof(channel_inform), false);
	if (unlikely(err)) {
		WL_ERR(("scan busy (%d)\n", err));
		goto scan_done_out;
	}
	channel_inform.scan_channel = dtoh32(channel_inform.scan_channel);
	if (unlikely(channel_inform.scan_channel)) {

		WL_DBG(("channel_inform.scan_channel (%d)\n",
			channel_inform.scan_channel));
	}
	cfg->bss_list = cfg->scan_results;
	bss_list = cfg->bss_list;
	memset(bss_list, 0, len);
	bss_list->buflen = htod32(len);
	err = wldev_ioctl(ndev, WLC_SCAN_RESULTS, bss_list, len, false);
	if (unlikely(err) && unlikely(!cfg->scan_suppressed)) {
		WL_ERR(("%s Scan_results error (%d)\n", ndev->name, err));
		err = -EINVAL;
		goto scan_done_out;
	}
	bss_list->buflen = dtoh32(bss_list->buflen);
	bss_list->version = dtoh32(bss_list->version);
	bss_list->count = dtoh32(bss_list->count);

	err = wl_inform_bss(cfg);

scan_done_out:
	del_timer_sync(&cfg->scan_timeout);
	spin_lock_irqsave(&cfg->cfgdrv_lock, flags);
	if (cfg->scan_request) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 8, 0))
		struct cfg80211_scan_info info = {
			.aborted = false,
		};
		cfg80211_scan_done(cfg->scan_request, &info);
#else
		cfg80211_scan_done(cfg->scan_request, false);
#endif
		cfg->scan_request = NULL;
	}
	spin_unlock_irqrestore(&cfg->cfgdrv_lock, flags);
	WL_DBG(("cfg80211_scan_done\n"));
	mutex_unlock(&cfg->usr_sync);
	return err;
}

static s32
wl_frame_get_mgmt(u16 fc, const struct ether_addr *da,
	const struct ether_addr *sa, const struct ether_addr *bssid,
	u8 **pheader, u32 *body_len, u8 *pbody)
{
	struct dot11_management_header *hdr;
	u32 totlen = 0;
	s32 err = 0;
	u8 *offset;
	u32 prebody_len = *body_len;
	switch (fc) {
		case FC_ASSOC_REQ:
			/* capability , listen interval */
			totlen = DOT11_ASSOC_REQ_FIXED_LEN;
			*body_len += DOT11_ASSOC_REQ_FIXED_LEN;
			break;

		case FC_REASSOC_REQ:
			/* capability, listen inteval, ap address */
			totlen = DOT11_REASSOC_REQ_FIXED_LEN;
			*body_len += DOT11_REASSOC_REQ_FIXED_LEN;
			break;
	}
	totlen += DOT11_MGMT_HDR_LEN + prebody_len;
	*pheader = kzalloc(totlen, GFP_KERNEL);
	if (*pheader == NULL) {
		WL_ERR(("memory alloc failed \n"));
		return -ENOMEM;
	}
	hdr = (struct dot11_management_header *) (*pheader);
	hdr->fc = htol16(fc);
	hdr->durid = 0;
	hdr->seq = 0;
	offset = (u8*)(hdr + 1) + (totlen - DOT11_MGMT_HDR_LEN - prebody_len);
	bcopy((const char*)da, (u8*)&hdr->da, ETHER_ADDR_LEN);
	bcopy((const char*)sa, (u8*)&hdr->sa, ETHER_ADDR_LEN);
	bcopy((const char*)bssid, (u8*)&hdr->bssid, ETHER_ADDR_LEN);
	if ((pbody != NULL) && prebody_len)
		bcopy((const char*)pbody, offset, prebody_len);
	*body_len = totlen;
	return err;
}
#ifdef  BRCM_RSDB
#ifdef WL_CFG80211_GON_COLLISION
static void
wl_gon_req_collision(struct bcm_cfg80211 *cfg, wl_action_frame_t *tx_act_frm,
	wifi_p2p_pub_act_frame_t *rx_act_frm, struct net_device *ndev,
	struct ether_addr sa, struct ether_addr da)
{
	if (cfg->afx_hdl->pending_tx_act_frm == NULL)
		return;

	if (tx_act_frm &&
		wl_cfgp2p_is_pub_action(tx_act_frm->data, tx_act_frm->len)) {
		wifi_p2p_pub_act_frame_t *pact_frm;

		pact_frm = (wifi_p2p_pub_act_frame_t *)tx_act_frm->data;

		if (!(pact_frm->subtype == P2P_PAF_GON_REQ &&
			rx_act_frm->subtype == P2P_PAF_GON_REQ)) {
			return;
		}
	}

	WL_ERR((" GO NEGO Request COLLISION !!! \n"));

	/* if sa(peer) addr is less than da(my) addr,
	 * my device will process peer's gon request and block to send my gon req.
	 *
	 * if not (sa addr > da addr),
	 * my device will process gon request and drop gon req of peer.
	 */
	if (memcmp(sa.octet, da.octet, ETHER_ADDR_LEN) < 0) {
		/* block to send tx gon request */
		cfg->block_gon_req_tx_count = BLOCK_GON_REQ_MAX_NUM;
		WL_ERR((" block to send gon req tx !!!\n"));

		/* if we are finding a common channel for sending af,
		 * do not scan more to block to send current gon req
		 */
		if (wl_get_drv_status_all(cfg, FINDING_COMMON_CHANNEL)) {
			wl_clr_drv_status(cfg, FINDING_COMMON_CHANNEL, ndev);
			complete(&cfg->act_frm_scan);
		}
	} else {
		/* drop gon request of peer to process gon request by my device. */
		WL_ERR((" drop to receive gon req rx !!! \n"));
		cfg->block_gon_req_rx_count = BLOCK_GON_REQ_MAX_NUM;
	}

	return;
}
#endif /* WL_CFG80211_GON_COLLISION */
#endif /* BRCM_RSDB */
void
wl_stop_wait_next_action_frame(struct bcm_cfg80211 *cfg, struct net_device *ndev)
{
	if (wl_get_drv_status_all(cfg, FINDING_COMMON_CHANNEL)) {
		if (timer_pending(&cfg->p2p->listen_timer)) {
			del_timer_sync(&cfg->p2p->listen_timer);
		}
		if (cfg->afx_hdl != NULL) {
			if (cfg->afx_hdl->dev != NULL) {
				wl_clr_drv_status(cfg, SCANNING, cfg->afx_hdl->dev);
				wl_clr_drv_status(cfg, FINDING_COMMON_CHANNEL, cfg->afx_hdl->dev);
			}
			cfg->afx_hdl->peer_chan = WL_INVALID;
		}
		complete(&cfg->act_frm_scan);
		WL_DBG(("*** Wake UP ** Working afx searching is cleared\n"));
	} else if (wl_get_drv_status_all(cfg, SENDING_ACT_FRM)) {
		if (!(wl_get_p2p_status(cfg, ACTION_TX_COMPLETED) ||
			wl_get_p2p_status(cfg, ACTION_TX_NOACK)))
			wl_set_p2p_status(cfg, ACTION_TX_COMPLETED);

		WL_DBG(("*** Wake UP ** abort actframe iovar\n"));
		/* if channel is not zero, "actfame" uses off channel scan.
		 * So abort scan for off channel completion.
		 */
		if (cfg->af_sent_channel)
			wl_cfg80211_scan_abort(cfg);
	}
#ifdef WL_CFG80211_SYNC_GON
	else if (wl_get_drv_status_all(cfg, WAITING_NEXT_ACT_FRM_LISTEN)) {
		WL_DBG(("*** Wake UP ** abort listen for next af frame\n"));
		/* So abort scan to cancel listen */
		wl_cfg80211_scan_abort(cfg);
	}
#endif /* WL_CFG80211_SYNC_GON */
}
#ifdef  BRCM_RSDB
#if defined(WLTDLS)
bool wl_cfg80211_is_tdls_tunneled_frame(void *frame, u32 frame_len)
{
	unsigned char *data;

	if (frame == NULL) {
		WL_ERR(("Invalid frame \n"));
		return false;
	}

	if (frame_len < 5) {
		WL_ERR(("Invalid frame length [%d] \n", frame_len));
		return false;
	}

	data = frame;

	if (!memcmp(data, TDLS_TUNNELED_PRB_REQ, 5) ||
		!memcmp(data, TDLS_TUNNELED_PRB_RESP, 5)) {
		WL_DBG(("TDLS Vendor Specific Received type\n"));
		return true;
	}

	return false;
}
#endif /* WLTDLS */

#if defined(CUSTOMER_HW4) && defined(WES_SUPPORT)
static int wes_mode = 0;
int wl_cfg80211_set_wes_mode(int mode)
{
	wes_mode = mode;
	return 0;
}

int wl_cfg80211_get_wes_mode(void)
{
	return wes_mode;
}

bool wl_cfg80211_is_wes(void *frame, u32 frame_len)
{
	unsigned char *data;

	if (frame == NULL) {
		WL_ERR(("Invalid frame \n"));
		return false;
	}

	if (frame_len < 4) {
		WL_ERR(("Invalid frame length [%d] \n", frame_len));
		return false;
	}

	data = frame;

	if (memcmp(data, "\x7f\x00\x00\xf0", 4) == 0) {
		WL_DBG(("Receive WES VS Action Frame \n"));
		return true;
	}

	return false;
}
#endif /* WES_SUPPORT */
#endif /*BRCM_RSDB*/
int wl_cfg80211_get_ioctl_version(void)
{
	return ioctl_version;
}

static s32
wl_notify_rx_mgmt_frame(struct bcm_cfg80211 *cfg, bcm_struct_cfgdev *cfgdev,
	const wl_event_msg_t *e, void *data)
{
	struct ieee80211_supported_band *band;
	struct wiphy *wiphy = bcmcfg_to_wiphy(cfg);
	struct ether_addr da;
	struct ether_addr bssid;
	bool isfree = false;
	s32 err = 0;
	s32 freq;
	struct net_device *ndev = NULL;
	wifi_p2p_pub_act_frame_t *act_frm = NULL;
	wifi_p2p_action_frame_t *p2p_act_frm = NULL;
	wifi_p2psd_gas_pub_act_frame_t *sd_act_frm = NULL;
#ifdef  BRCM_RSDB
#if defined(WLTDLS) && defined(TDLS_MSG_ONLY_WFD)
	dhd_pub_t *dhdp;
#endif /* WLTDLS && TDLS_MSG_ONLY_WFD */
#endif
	wl_event_rx_frame_data_t *rxframe =
		(wl_event_rx_frame_data_t*)data;
	u32 event = ntoh32(e->event_type);
	u8 *mgmt_frame;
	u8 bsscfgidx = e->bsscfgidx;
#ifndef BCM_PATCH_SECURITY_2017_07
	u32 mgmt_frame_len = ntoh32(e->datalen) - sizeof(wl_event_rx_frame_data_t);
	u16 channel = ((ntoh16(rxframe->channel) & WL_CHANSPEC_CHAN_MASK));
#else
	u32 mgmt_frame_len = ntoh32(e->datalen);
	u16 channel = ((ntoh16(rxframe->channel) & WL_CHANSPEC_CHAN_MASK));

	if (mgmt_frame_len < sizeof(wl_event_rx_frame_data_t)) {
		WL_ERR(("wrong datalen:%d\n", mgmt_frame_len));
		return -EINVAL;
	}
	mgmt_frame_len -= sizeof(wl_event_rx_frame_data_t);
#endif
	memset(&bssid, 0, ETHER_ADDR_LEN);

	ndev = cfgdev_to_wlc_ndev(cfgdev, cfg);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0))
	if (channel <= CH_MAX_2G_CHANNEL)
		band = wiphy->bands[NL80211_BAND_2GHZ];
	else
		band = wiphy->bands[NL80211_BAND_5GHZ];
#else
	if (channel <= CH_MAX_2G_CHANNEL)
		band = wiphy->bands[IEEE80211_BAND_2GHZ];
	else
		band = wiphy->bands[IEEE80211_BAND_5GHZ];
#endif
	if (!band) {
		WL_ERR(("No valid band"));
		return -EINVAL;
	}
#if LINUX_VERSION_CODE == KERNEL_VERSION(2, 6, 38) && !defined(WL_COMPAT_WIRELESS)
	freq = ieee80211_channel_to_frequency(channel);
	(void)band->band;
#else
	freq = ieee80211_channel_to_frequency(channel, band->band);
#endif
	if (event == WLC_E_ACTION_FRAME_RX) {
		wldev_iovar_getbuf_bsscfg(ndev, "cur_etheraddr",
			NULL, 0, cfg->ioctl_buf, WLC_IOCTL_SMLEN, bsscfgidx, &cfg->ioctl_buf_sync);

		err = wldev_ioctl(ndev, WLC_GET_BSSID, &bssid, ETHER_ADDR_LEN, false);
		if (err < 0)
			 WL_ERR(("WLC_GET_BSSID error %d\n", err));
		memcpy(da.octet, cfg->ioctl_buf, ETHER_ADDR_LEN);
		err = wl_frame_get_mgmt(FC_ACTION, &da, &e->addr, &bssid,
			&mgmt_frame, &mgmt_frame_len,
			(u8 *)((wl_event_rx_frame_data_t *)rxframe + 1));
		if (err < 0) {
			WL_ERR(("Error in receiving action frame len %d channel %d freq %d\n",
				mgmt_frame_len, channel, freq));
			goto exit;
		}
		isfree = true;
		if (wl_cfgp2p_is_pub_action(&mgmt_frame[DOT11_MGMT_HDR_LEN],
			mgmt_frame_len - DOT11_MGMT_HDR_LEN)) {
			act_frm = (wifi_p2p_pub_act_frame_t *)
					(&mgmt_frame[DOT11_MGMT_HDR_LEN]);
		} else if (wl_cfgp2p_is_p2p_action(&mgmt_frame[DOT11_MGMT_HDR_LEN],
			mgmt_frame_len - DOT11_MGMT_HDR_LEN)) {
			p2p_act_frm = (wifi_p2p_action_frame_t *)
					(&mgmt_frame[DOT11_MGMT_HDR_LEN]);
			(void) p2p_act_frm;
		} else if (wl_cfgp2p_is_gas_action(&mgmt_frame[DOT11_MGMT_HDR_LEN],
			mgmt_frame_len - DOT11_MGMT_HDR_LEN)) {

			sd_act_frm = (wifi_p2psd_gas_pub_act_frame_t *)
					(&mgmt_frame[DOT11_MGMT_HDR_LEN]);
			if (sd_act_frm && wl_get_drv_status_all(cfg, WAITING_NEXT_ACT_FRM)) {
				if (cfg->next_af_subtype == sd_act_frm->action) {
					WL_DBG(("We got a right next frame of SD!(%d)\n",
						sd_act_frm->action));
					wl_clr_drv_status(cfg, WAITING_NEXT_ACT_FRM, ndev);

					/* Stop waiting for next AF. */
					wl_stop_wait_next_action_frame(cfg, ndev);
				}
			}
			(void) sd_act_frm;
#ifdef  BRCM_RSDB
#ifdef WLTDLS
		} else if ((mgmt_frame[DOT11_MGMT_HDR_LEN] == TDLS_AF_CATEGORY) ||
				(wl_cfg80211_is_tdls_tunneled_frame(
				    &mgmt_frame[DOT11_MGMT_HDR_LEN],
				    mgmt_frame_len - DOT11_MGMT_HDR_LEN))) {
			if (mgmt_frame[DOT11_MGMT_HDR_LEN] == TDLS_AF_CATEGORY) {
				WL_ERR((" TDLS Action Frame Received type = %d \n",
					mgmt_frame[DOT11_MGMT_HDR_LEN + 1]));
			}
#ifdef TDLS_MSG_ONLY_WFD
			dhdp = (dhd_pub_t *)(cfg->pub);
			if (!dhdp->tdls_mode) {
				WL_DBG((" TDLS Frame filtered \n"));
				return 0;
			}
#else
			if (mgmt_frame[DOT11_MGMT_HDR_LEN + 1] == TDLS_ACTION_SETUP_RESP) {
				cfg->tdls_mgmt_frame = mgmt_frame;
				cfg->tdls_mgmt_frame_len = mgmt_frame_len;
				cfg->tdls_mgmt_freq = freq;
				return 0;
			}
#endif /* TDLS_MSG_ONLY_WFD */
#endif /* WLTDLS */
#ifdef QOS_MAP_SET
		} else if (mgmt_frame[DOT11_MGMT_HDR_LEN] == DOT11_ACTION_CAT_QOS) {
			/* update QoS map set table */
			bcm_tlv_t * qos_map_ie = NULL;
			if ((qos_map_ie = bcm_parse_tlvs(&mgmt_frame[DOT11_MGMT_HDR_LEN],
					mgmt_frame_len - DOT11_MGMT_HDR_LEN,
					DOT11_MNG_QOS_MAP_ID)) != NULL) {
				WL_DBG((" QoS map set IE found in QoS action frame\n"));
				if (!cfg->up_table) {
					cfg->up_table = kmalloc(UP_TABLE_MAX, GFP_KERNEL);
				}
				wl_set_up_table(cfg->up_table, qos_map_ie);
			} else {
				kfree(cfg->up_table);
				cfg->up_table = NULL;
			}
#endif /* QOS_MAP_SET */
#endif /* BRCM_RSDB */
		} else {
#ifdef  BRCM_RSDB
			/*
			 *  if we got normal action frame and ndev is p2p0,
			 *  we have to change ndev from p2p0 to wlan0
			 */
#if defined(CUSTOMER_HW4) && defined(WES_SUPPORT)
			if (wl_cfg80211_is_wes(&mgmt_frame[DOT11_MGMT_HDR_LEN],
			mgmt_frame_len - DOT11_MGMT_HDR_LEN) && wes_mode == 0) {
			/* Ignore WES VS Action frame */
			goto exit;
			}
#endif /* WES_SUPPORT */
#endif

			if (cfg->next_af_subtype != P2P_PAF_SUBTYPE_INVALID) {
				u8 action = 0;
				if (wl_get_public_action(&mgmt_frame[DOT11_MGMT_HDR_LEN],
					mgmt_frame_len - DOT11_MGMT_HDR_LEN, &action) != BCME_OK) {
					WL_DBG(("Recived action is not public action frame\n"));
				} else if (cfg->next_af_subtype == action) {
					WL_DBG(("Recived action is the waiting action(%d)\n",
						action));
					wl_clr_drv_status(cfg, WAITING_NEXT_ACT_FRM, ndev);

					/* Stop waiting for next AF. */
					wl_stop_wait_next_action_frame(cfg, ndev);
				}
			}
		}

		if (act_frm) {
#ifdef  BRCM_RSDB
#ifdef WL_CFG80211_GON_COLLISION
			if (act_frm->subtype == P2P_PAF_GON_REQ) {
				wl_gon_req_collision(cfg,
					&cfg->afx_hdl->pending_tx_act_frm->action_frame,
					act_frm, ndev, e->addr, da);

				if (cfg->block_gon_req_rx_count) {
					WL_ERR(("drop frame GON Req Rx : count (%d)\n",
						cfg->block_gon_req_rx_count));
					cfg->block_gon_req_rx_count--;
					goto exit;
				}
			} else if (act_frm->subtype == P2P_PAF_GON_CONF) {
				/* if go formation done, clear it */
				cfg->block_gon_req_tx_count = 0;
				cfg->block_gon_req_rx_count = 0;
			}
#endif /* WL_CFG80211_GON_COLLISION */
#endif
			if (wl_get_drv_status_all(cfg, WAITING_NEXT_ACT_FRM)) {
				if (cfg->next_af_subtype == act_frm->subtype) {
					WL_DBG(("We got a right next frame!(%d)\n",
						act_frm->subtype));
					wl_clr_drv_status(cfg, WAITING_NEXT_ACT_FRM, ndev);

					if (cfg->next_af_subtype == P2P_PAF_GON_CONF) {
						OSL_SLEEP(20);
					}

					/* Stop waiting for next AF. */
					wl_stop_wait_next_action_frame(cfg, ndev);
				}
			}
		}

		wl_cfgp2p_print_actframe(false, &mgmt_frame[DOT11_MGMT_HDR_LEN],
			mgmt_frame_len - DOT11_MGMT_HDR_LEN, channel);
		/*
		 * After complete GO Negotiation, roll back to mpc mode
		 */
		if (act_frm && ((act_frm->subtype == P2P_PAF_GON_CONF) ||
			(act_frm->subtype == P2P_PAF_PROVDIS_RSP))) {
			wldev_iovar_setint(ndev, "mpc", 1);
		}
		if (act_frm && (act_frm->subtype == P2P_PAF_GON_CONF)) {
			WL_DBG(("P2P: GO_NEG_PHASE status cleared \n"));
			wl_clr_p2p_status(cfg, GO_NEG_PHASE);
		}
	} else if (event == WLC_E_PROBREQ_MSG) {

		/* Handle probe reqs frame
		 * WPS-AP certification 4.2.13
		 */
		struct parsed_ies prbreq_ies;
		u32 prbreq_ie_len = 0;
		bool pbc = 0;

		WL_DBG((" Event WLC_E_PROBREQ_MSG received\n"));
		mgmt_frame = (u8 *)(data);
		mgmt_frame_len = ntoh32(e->datalen);
#ifdef BCM_PATCH_SECURITY_2017_07
		if (mgmt_frame_len < DOT11_MGMT_HDR_LEN) {
			WL_ERR(("wrong datalen:%d\n", mgmt_frame_len));
			return -EINVAL;
		}
#endif
		prbreq_ie_len = mgmt_frame_len - DOT11_MGMT_HDR_LEN;

		/* Parse prob_req IEs */
		if (wl_cfg80211_parse_ies(&mgmt_frame[DOT11_MGMT_HDR_LEN],
			prbreq_ie_len, &prbreq_ies) < 0) {
			WL_ERR(("Prob req get IEs failed\n"));
			return 0;
		}
		if (prbreq_ies.wps_ie != NULL) {
			wl_validate_wps_ie((char *)prbreq_ies.wps_ie, prbreq_ies.wps_ie_len, &pbc);
			WL_DBG((" wps_ie exist pbc = %d\n", pbc));
			/* if pbc method, send prob_req mgmt frame to upper layer */
			if (!pbc)
				return 0;
		} else
			return 0;
	} else {
		mgmt_frame = (u8 *)((wl_event_rx_frame_data_t *)rxframe + 1);

		/* wpa supplicant use probe request event for restarting another GON Req.
		 * but it makes GON Req repetition.
		 * so if src addr of prb req is same as my target device,
		 * do not send probe request event during sending action frame.
		 */
		if (event == WLC_E_P2P_PROBREQ_MSG) {
			WL_DBG((" Event %s\n", (event == WLC_E_P2P_PROBREQ_MSG) ?
				"WLC_E_P2P_PROBREQ_MSG":"WLC_E_PROBREQ_MSG"));
#ifdef  BRCM_RSDB
#ifdef WL_CFG80211_USE_PRB_REQ_FOR_AF_TX
			if (WL_DRV_STATUS_SENDING_AF_FRM_EXT(cfg) &&
				!memcmp(cfg->afx_hdl->tx_dst_addr.octet, e->addr.octet,
				ETHER_ADDR_LEN)) {
				if (cfg->afx_hdl->pending_tx_act_frm &&
					wl_get_drv_status_all(cfg, FINDING_COMMON_CHANNEL)) {
					s32 channel = CHSPEC_CHANNEL(hton16(rxframe->channel));
					WL_DBG(("PROBE REQUEST : Peer found, channel : %d\n",
						channel));
					cfg->afx_hdl->peer_chan = channel;
					complete(&cfg->act_frm_scan);
				}
			}
#endif /* WL_CFG80211_USE_PRB_REQ_FOR_AF_TX */
#endif
			/* Filter any P2P probe reqs arriving during the
			 * GO-NEG Phase
			 */
			if (cfg->p2p &&
#ifdef  BRCM_RSDB
#if defined(P2P_IE_MISSING_FIX)
				cfg->p2p_prb_noti &&
#endif
#endif
				wl_get_p2p_status(cfg, GO_NEG_PHASE)) {
				WL_DBG(("Filtering P2P probe_req while "
					"being in GO-Neg state\n"));
				return 0;
			}
		}
	}
#ifdef  BRCM_RSDB
	if (discover_cfgdev(cfgdev, cfg))
		WL_DBG(("Rx Managment frame For P2P Discovery Interface \n"));
	else
		WL_DBG(("Rx Managment frame For Iface (%s) \n", ndev->name));
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 18, 0))
	cfg80211_rx_mgmt(cfgdev, freq, 0, mgmt_frame, mgmt_frame_len, 0);
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 13, 0))
	cfg80211_rx_mgmt(cfgdev, freq, 0,  mgmt_frame, mgmt_frame_len, 0, GFP_ATOMIC);
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 4, 0)) || \
	defined(WL_COMPAT_WIRELESS)
	cfg80211_rx_mgmt(cfgdev, freq, 0, mgmt_frame, mgmt_frame_len, GFP_ATOMIC);
#else
	cfg80211_rx_mgmt(cfgdev, freq, mgmt_frame, mgmt_frame_len, GFP_ATOMIC);
#endif /* LINUX_VERSION >= VERSION(3, 14, 0) */

	WL_DBG(("mgmt_frame_len (%d) , e->datalen (%d), channel (%d), freq (%d)\n",
		mgmt_frame_len, ntoh32(e->datalen), channel, freq));
exit:
	if (isfree)
		kfree(mgmt_frame);
	return 0;
}

#ifdef WL_SCHED_SCAN
/* If target scan is not reliable, set the below define to "1" to do a
 * full escan
 */
#define FULL_ESCAN_ON_PFN_NET_FOUND		0
static s32
wl_notify_sched_scan_results(struct bcm_cfg80211 *cfg, struct net_device *ndev,
	const wl_event_msg_t *e, void *data)
{
	int n_pfn_results;
	wl_pfn_net_info_t *netinfo, *pnetinfo;
	struct wiphy *wiphy	= bcmcfg_to_wiphy(cfg);
	int err = 0;
	struct cfg80211_scan_request *request = NULL;
	struct cfg80211_ssid ssid[MAX_PFN_LIST_COUNT];
	struct ieee80211_channel *channel = NULL;
	int channel_req = 0;
	int band = 0;
	struct wl_pfn_scanresults *pfn_result = NULL;

	if (data == NULL) {
		WL_ERR(("%s :data is null\n", __FUNCTION__));
		return BCME_ERROR;
	}

	pfn_result = (struct wl_pfn_scanresults *)data;
	n_pfn_results = pfn_result->count;
	WL_DBG(("Enter\n"));

#ifdef BCM_PATCH_CVE_2016_0801
	if ((e->event_type == WLC_E_PFN_NET_LOST) || !data) {
		WL_PNO(("Do Nothing %d\n", e->event_type));
		return 0;
	}
	if (pfn_result->version != PFN_SCANRESULT_VERSION) {
		WL_ERR(("Incorrect version %d, expected %d\n", pfn_result->version,
		       PFN_SCANRESULT_VERSION));
		return 0;
	}
#else
	if (e->event_type == WLC_E_PFN_NET_LOST) {
		WL_PNO(("PFN NET LOST event. Do Nothing \n"));
		return 0;
	}
#endif
	WL_PNO((">>> PFN NET FOUND event. count:%d \n", n_pfn_results));
	if (n_pfn_results > 0) {
		int i;

		if (n_pfn_results > MAX_PFN_LIST_COUNT)
			n_pfn_results = MAX_PFN_LIST_COUNT;
		pnetinfo = (wl_pfn_net_info_t *)(data + sizeof(wl_pfn_scanresults_t)
				- sizeof(wl_pfn_net_info_t));

		memset(&ssid, 0x00, sizeof(ssid));

		request = kzalloc(sizeof(*request)
			+ sizeof(*request->channels) * n_pfn_results,
			GFP_KERNEL);
		channel = (struct ieee80211_channel *)kzalloc(
			(sizeof(struct ieee80211_channel) * n_pfn_results),
			GFP_KERNEL);
		if (!request || !channel) {
			WL_ERR(("No memory"));
			err = -ENOMEM;
			goto out_err;
		}

		request->wiphy = wiphy;

		for (i = 0; i < n_pfn_results; i++) {
			netinfo = &pnetinfo[i];
			if (!netinfo) {
				WL_ERR(("Invalid netinfo ptr. index:%d", i));
				err = -EINVAL;
				goto out_err;
			}
#ifndef  BRCM_RSDB
			printf(">>> SSID:%s Channel:%d \n",
				netinfo->pfnsubnet.SSID, netinfo->pfnsubnet.channel);
#else
			WL_ERR((">>> SSID:%s Channel:%d \n",
				netinfo->pfnsubnet.SSID, netinfo->pfnsubnet.channel));
#endif
			/* PFN result doesn't have all the info which are required by the supplicant
			 * (For e.g IEs) Do a target Escan so that sched scan results are reported
			 * via wl_inform_single_bss in the required format. Escan does require the
			 * scan request in the form of cfg80211_scan_request. For timebeing, create
			 * cfg80211_scan_request one out of the received PNO event.
			 */
#ifdef BCM_PATCH_CVE_2016_0801
#ifndef  BRCM_RSDB
			ssid[i].ssid_len = MIN(DOT11_MAX_SSID_LEN, netinfo->pfnsubnet.SSID_len);
			memcpy(ssid[i].ssid, netinfo->pfnsubnet.SSID,
			       ssid[i].ssid_len);
#else
			ssid[i].ssid_len = MIN(netinfo->pfnsubnet.SSID_len, DOT11_MAX_SSID_LEN);
			memcpy(ssid[i].ssid, netinfo->pfnsubnet.SSID, ssid[i].ssid_len);
#endif /* BRCM_RSDB */
#else
			memcpy(ssid[i].ssid, netinfo->pfnsubnet.SSID,
				netinfo->pfnsubnet.SSID_len);
			ssid[i].ssid_len = netinfo->pfnsubnet.SSID_len;
#endif
			request->n_ssids++;

			channel_req = netinfo->pfnsubnet.channel;
			band = (channel_req <= CH_MAX_2G_CHANNEL) ? NL80211_BAND_2GHZ
				: NL80211_BAND_5GHZ;
			channel[i].center_freq = ieee80211_channel_to_frequency(channel_req, band);
			channel[i].band = band;
			channel[i].flags |= IEEE80211_CHAN_NO_HT40;
			request->channels[i] = &channel[i];
			request->n_channels++;
		}

		/* assign parsed ssid array */
		if (request->n_ssids)
			request->ssids = &ssid[0];

		if (wl_get_drv_status_all(cfg, SCANNING)) {
			/* Abort any on-going scan */
			wl_notify_escan_complete(cfg, ndev, true, true);
		}

		if (wl_get_p2p_status(cfg, DISCOVERY_ON)) {
			WL_PNO((">>> P2P discovery was ON. Disabling it\n"));
			err = wl_cfgp2p_discover_enable_search(cfg, false);
			if (unlikely(err)) {
				wl_clr_drv_status(cfg, SCANNING, ndev);
				goto out_err;
			}
			p2p_scan(cfg) = false;
		}

		wl_set_drv_status(cfg, SCANNING, ndev);
#ifndef  BRCM_RSDB
#ifdef CUSTOM_SET_SHORT_DWELL_TIME
		net_set_short_dwell_time(ndev, FALSE);
#endif
#endif
#if FULL_ESCAN_ON_PFN_NET_FOUND
		WL_PNO((">>> Doing Full ESCAN on PNO event\n"));
		err = wl_do_escan(cfg, wiphy, ndev, NULL);
#else
		WL_PNO((">>> Doing targeted ESCAN on PNO event\n"));
		err = wl_do_escan(cfg, wiphy, ndev, request);
#endif
		if (err) {
			wl_clr_drv_status(cfg, SCANNING, ndev);
			goto out_err;
		}
		cfg->sched_scan_running = TRUE;
	}
	else {
		WL_ERR(("FALSE PNO Event. (pfn_count == 0) \n"));
	}
out_err:
	if (request)
		kfree(request);
	if (channel)
		kfree(channel);
	return err;
}
#endif /* WL_SCHED_SCAN */

static void wl_init_conf(struct wl_conf *conf)
{
	WL_DBG(("Enter \n"));
	conf->frag_threshold = (u32)-1;
	conf->rts_threshold = (u32)-1;
	conf->retry_short = (u32)-1;
	conf->retry_long = (u32)-1;
	conf->tx_power = -1;
}

static void wl_init_prof(struct bcm_cfg80211 *cfg, struct net_device *ndev)
{
	unsigned long flags;
	struct wl_profile *profile = wl_get_profile_by_netdev(cfg, ndev);

	spin_lock_irqsave(&cfg->cfgdrv_lock, flags);
	memset(profile, 0, sizeof(struct wl_profile));
	spin_unlock_irqrestore(&cfg->cfgdrv_lock, flags);
}

static void wl_init_event_handler(struct bcm_cfg80211 *cfg)
{
	memset(cfg->evt_handler, 0, sizeof(cfg->evt_handler));

	cfg->evt_handler[WLC_E_SCAN_COMPLETE] = wl_notify_scan_status;
	cfg->evt_handler[WLC_E_AUTH] = wl_notify_connect_status;
	cfg->evt_handler[WLC_E_ASSOC] = wl_notify_connect_status;
	cfg->evt_handler[WLC_E_LINK] = wl_notify_connect_status;
	cfg->evt_handler[WLC_E_DEAUTH_IND] = wl_notify_connect_status;
	cfg->evt_handler[WLC_E_DEAUTH] = wl_notify_connect_status;
	cfg->evt_handler[WLC_E_DISASSOC_IND] = wl_notify_connect_status;
	cfg->evt_handler[WLC_E_ASSOC_IND] = wl_notify_connect_status;
	cfg->evt_handler[WLC_E_REASSOC_IND] = wl_notify_connect_status;
	cfg->evt_handler[WLC_E_ROAM] = wl_notify_roaming_status;
	cfg->evt_handler[WLC_E_MIC_ERROR] = wl_notify_mic_status;
	cfg->evt_handler[WLC_E_SET_SSID] = wl_notify_connect_status;
	cfg->evt_handler[WLC_E_ACTION_FRAME_RX] = wl_notify_rx_mgmt_frame;
	cfg->evt_handler[WLC_E_PROBREQ_MSG] = wl_notify_rx_mgmt_frame;
	cfg->evt_handler[WLC_E_P2P_PROBREQ_MSG] = wl_notify_rx_mgmt_frame;
	cfg->evt_handler[WLC_E_P2P_DISC_LISTEN_COMPLETE] = wl_cfgp2p_listen_complete;
	cfg->evt_handler[WLC_E_ACTION_FRAME_COMPLETE] = wl_cfgp2p_action_tx_complete;
	cfg->evt_handler[WLC_E_ACTION_FRAME_OFF_CHAN_COMPLETE] = wl_cfgp2p_action_tx_complete;
	cfg->evt_handler[WLC_E_JOIN] = wl_notify_connect_status;
#if defined(WLFBT)
	cfg->evt_handler[WLC_E_ASSOC_RESP_IE] = wl_notify_assoc_resp_ie;
#endif
	cfg->evt_handler[WLC_E_START] = wl_notify_connect_status;
#ifdef PNO_SUPPORT
	cfg->evt_handler[WLC_E_PFN_NET_FOUND] = wl_notify_pfn_status;
#endif /* PNO_SUPPORT */
#ifdef GSCAN_SUPPORT
	cfg->evt_handler[WLC_E_PFN_BEST_BATCHING] = wl_notify_gscan_event;
	cfg->evt_handler[WLC_E_PFN_SCAN_COMPLETE] = wl_notify_gscan_event;
	cfg->evt_handler[WLC_E_PFN_GSCAN_FULL_RESULT] = wl_notify_gscan_event;
#ifndef BCM_PATCH_SECURITY_2017_07
	cfg->evt_handler[WLC_E_PFN_SWC] = wl_notify_gscan_event;
#endif
	cfg->evt_handler[WLC_E_PFN_BSSID_NET_FOUND] = wl_notify_gscan_event;
	cfg->evt_handler[WLC_E_PFN_BSSID_NET_LOST] = wl_notify_gscan_event;
#ifndef  BRCM_RSDB
#ifdef BCM_PATCH_GSCAN
#else
	cfg->evt_handler[WLC_E_PFN_SSID_EXT] = wl_notify_gscan_event;
#endif
	cfg->evt_handler[WLC_E_GAS_FRAGMENT_RX] = wl_notify_gscan_event;
	cfg->evt_handler[WLC_E_ROAM_EXP_EVENT] = wl_handle_roam_exp_event;
#endif /* BRCM_RSDB */
#endif /* GSCAN_SUPPORT */
#ifndef  BRCM_RSDB
	cfg->evt_handler[WLC_E_RSSI_LQM] = wl_handle_rssi_monitor_event;
#endif
#ifdef WLTDLS
	cfg->evt_handler[WLC_E_TDLS_PEER_EVENT] = wl_tdls_event_handler;
#endif /* WLTDLS */
#ifdef WL_TEM_CTRL
    cfg->evt_handler[WLC_E_TEM_CTRL_EVENT] = wl_hw_tem_ctrl_event_handler;
#endif
#ifdef WL_TIM_EVENT
    cfg->evt_handler[WLC_E_TIM_EVENT] = wl_hw_tim_event_handler;
#endif
#ifdef WRONG_ACTION_PATCH
	cfg->evt_handler[WLC_E_PRUNE] = wl_prune_event_handler;
#endif
	cfg->evt_handler[WLC_E_BSSID] = wl_notify_roaming_status;
#ifdef  BRCM_RSDB
#ifdef WLAIBSS
	cfg->evt_handler[WLC_E_AIBSS_TXFAIL] = wl_notify_aibss_txfail;
#endif /* WLAIBSS */
#ifdef	WL_RELMCAST
	cfg->evt_handler[WLC_E_RMC_EVENT] = wl_notify_rmc_status;
#endif
#ifdef WL_NAN
	cfg->evt_handler[WLC_E_NAN] = wl_cfgnan_notify_nan_status;
	cfg->evt_handler[WLC_E_PROXD] = wl_cfgnan_notify_proxd_status;
#endif /* WL_NAN */
	cfg->evt_handler[WLC_E_CSA_COMPLETE_IND] = wl_csa_complete_ind;
#ifdef DHD_LOSSLESS_ROAMING
	cfg->evt_handler[WLC_E_ROAM_PREP] = wl_notify_roam_prep_status;
#endif
#ifdef BCM_BSSID_BLACKLIST
	cfg->evt_handler[WLC_E_ROAM_START] = wl_notify_roam_start_status;
#endif
	cfg->evt_handler[WLC_E_AP_STARTED] = wl_ap_start_ind;
#ifdef CUSTOM_EVENT_PM_WAKE
	cfg->evt_handler[WLC_E_EXCESS_PM_WAKE_EVENT] = wl_check_pmstatus;
#endif /* CUSTOM_EVENT_PM_WAKE */
	cfg->evt_handler[WLC_E_PSK_SUP] = wl_notify_idsup_status;
#endif /* BRCM_RSDB */
#ifdef BT_WIFI_HANDOVER
	cfg->evt_handler[WLC_E_BT_WIFI_HANDOVER_REQ] = wl_notify_bt_wifi_handover_req;
#endif
#if defined(DHD_DEBUG) && defined(BCM_PATCH_DEBUG_BEACON_LOSS)
	cfg->evt_handler[WLC_E_BEACON_RX] = wl_notify_beacon_event;
#endif
#ifdef CONFIG_HW_VOWIFI
	cfg->evt_handler[WLC_E_RSSI] = wl_notify_vowifi_event;
#endif
#ifdef HW_DFX_TXFAIL_EVENT
	cfg->evt_handler[WLC_E_TXFAIL_THRESH] = wl_notify_txfail_event;
#endif
#ifdef CONFIG_HW_ABS
	cfg->evt_handler[WLC_E_ANT_EVENT] = wl_notify_ant_event;
#endif
}
#ifdef CONFIG_HW_VOWIFI
static s32
wl_notify_vowifi_event(struct bcm_cfg80211 *cfg, bcm_struct_cfgdev *cfgdev,
	const wl_event_msg_t *e, void *data)
{
	struct net_device *ndev = NULL;
	u32 event = ntoh32(e->event_type);
	
	ndev = cfgdev_to_wlc_ndev(cfgdev, cfg);
	
	WL_ERR(("send event WLC_E_RSSI event = %d\n",event));
	/* send WLC_E_SET_SSID event */
	cfg80211_drv_vowifi(ndev, GFP_KERNEL);

	return 0;
}
#endif
#ifdef HW_READ_FW_LOG
extern void dhd_read_console_ex(dhd_pub_t *dhd);
#define TIME_1S_IN_JIFFIES msecs_to_jiffies(1000) /* 1s */
unsigned long timeout;
unsigned long previous_jiffies = 0;	/* The last active tx wakelock time */
#endif

#ifdef HW_DFX_TXFAIL_EVENT
static s32
wl_notify_txfail_event(struct bcm_cfg80211 *cfg, bcm_struct_cfgdev *cfgdev,
    const wl_event_msg_t *e, void *data)
{
	struct net_device *ndev = cfgdev_to_wlc_ndev(cfgdev, cfg);
	dhd_pub_t *dhdp = (dhd_pub_t*)cfg->pub;
	u32 status = ntoh32(e->status);
	int error = 0;
	int txpwrindex = 0;
	int rssi = 0;
	int rate = 0;

#ifdef HW_READ_FW_LOG
	unsigned long timeout;

	timeout = previous_jiffies + TIME_1S_IN_JIFFIES;
	if (time_after(jiffies, timeout) || previous_jiffies == 0) {
		dhd_read_console_ex(dhdp);
	}
	previous_jiffies = jiffies;
#endif

	/* now we have two types of tx fail(no ack from ap):
	WLC_STATUS_ARP_TX_FRAME_FAIL 0x10
	WLC_STATUS_DHCPC_TX_FRAME_FAIL 0x20
	*/
	if (status == WLC_STATUS_ARP_TX_FRAME_FAIL)
		WL_ERR(("notify txfail_event ARP no ack.\n"));
	else if (status == WLC_STATUS_DHCPC_TX_FRAME_FAIL)
		WL_ERR(("notify txfail_event DHCP no ack.\n"));
	else if (status == WLC_STATUS_8021X_TX_FRAME_FAIL)
		WL_ERR(("notify txfail_event 802.1X no ack.\n"));
	else
		WL_ERR(("notify txfail_event unknown status = 0x%x\n", status));

	/* check the tx power (25 ~ 80) */
	error = hw_iovar_int_get(dhdp, "phy_txpwrindex", &txpwrindex);
	WL_ERR(("txfail_event txpowerindex = %d[25~80], error = %d\n", txpwrindex, error));

	/* check the rssi and phy linkspeed */
	error = wldev_get_rssi(ndev, &rssi);
	WL_ERR(("wldev_get_rssi return %d\n", error));
	error = wldev_get_link_speed(ndev, &rate); /*unit: Kbps*/
	WL_ERR(("wldev_get_link_speed return %d\n", error));
	rate = rate/100;  /*unit: 100Kbps*/
	WL_ERR(("txfail_event rssi = %d(dbm), rate = %d(100kbps), error = %d\n", rssi, rate, error));
	return 0;
}
#endif


#if defined(STATIC_WL_PRIV_STRUCT)
static void
wl_init_escan_result_buf(struct bcm_cfg80211 *cfg)
{
#if defined(DUAL_ESCAN_RESULT_BUFFER) && defined(BRCM_RSDB)
	cfg->escan_info.escan_buf[0] = DHD_OS_PREALLOC(cfg->pub,
		DHD_PREALLOC_WIPHY_ESCAN0, ESCAN_BUF_SIZE);
	bzero(cfg->escan_info.escan_buf[0], ESCAN_BUF_SIZE);
	cfg->escan_info.escan_buf[1] = DHD_OS_PREALLOC(cfg->pub,
		DHD_PREALLOC_WIPHY_ESCAN1, ESCAN_BUF_SIZE);
	bzero(cfg->escan_info.escan_buf[1], ESCAN_BUF_SIZE);
	cfg->escan_info.escan_type[0] = 0;
	cfg->escan_info.escan_type[1] = 0;
#else
	cfg->escan_info.escan_buf = DHD_OS_PREALLOC(cfg->pub,
		DHD_PREALLOC_WIPHY_ESCAN0, ESCAN_BUF_SIZE);
	bzero(cfg->escan_info.escan_buf, ESCAN_BUF_SIZE);
#endif /* DUAL_ESCAN_RESULT_BUFFER && BRCM_RSDB */
}

static void
wl_deinit_escan_result_buf(struct bcm_cfg80211 *cfg)
{
#if defined(DUAL_ESCAN_RESULT_BUFFER) && defined(BRCM_RSDB)
	cfg->escan_info.escan_buf[0] = NULL;
	cfg->escan_info.escan_buf[1] = NULL;
	cfg->escan_info.escan_type[0] = 0;
	cfg->escan_info.escan_type[1] = 0;
#else
	cfg->escan_info.escan_buf = NULL;
#endif /* DUAL_ESCAN_RESULT_BUFFER && BRCM_RSDB */

}
#endif /* STATIC_WL_PRIV_STRUCT */

static s32 wl_init_priv_mem(struct bcm_cfg80211 *cfg)
{
	WL_DBG(("Enter \n"));
	cfg->scan_results = (void *)kzalloc(WL_SCAN_BUF_MAX, GFP_KERNEL);
	if (unlikely(!cfg->scan_results)) {
		WL_ERR(("Scan results alloc failed\n"));
		goto init_priv_mem_out;
	}
	cfg->conf = (void *)kzalloc(sizeof(*cfg->conf), GFP_KERNEL);
	if (unlikely(!cfg->conf)) {
		WL_ERR(("wl_conf alloc failed\n"));
		goto init_priv_mem_out;
	}
	cfg->scan_req_int =
	    (void *)kzalloc(sizeof(*cfg->scan_req_int), GFP_KERNEL);
	if (unlikely(!cfg->scan_req_int)) {
		WL_ERR(("Scan req alloc failed\n"));
		goto init_priv_mem_out;
	}
	cfg->ioctl_buf = (void *)kzalloc(WLC_IOCTL_MAXLEN, GFP_KERNEL);
	if (unlikely(!cfg->ioctl_buf)) {
		WL_ERR(("Ioctl buf alloc failed\n"));
		goto init_priv_mem_out;
	}
	cfg->escan_ioctl_buf = (void *)kzalloc(WLC_IOCTL_MAXLEN, GFP_KERNEL);
	if (unlikely(!cfg->escan_ioctl_buf)) {
		WL_ERR(("Ioctl buf alloc failed\n"));
		goto init_priv_mem_out;
	}
	cfg->extra_buf = (void *)kzalloc(WL_EXTRA_BUF_MAX, GFP_KERNEL);
	if (unlikely(!cfg->extra_buf)) {
		WL_ERR(("Extra buf alloc failed\n"));
		goto init_priv_mem_out;
	}
	cfg->pmk_list = (void *)kzalloc(sizeof(*cfg->pmk_list), GFP_KERNEL);
	if (unlikely(!cfg->pmk_list)) {
		WL_ERR(("pmk list alloc failed\n"));
		goto init_priv_mem_out;
	}
#ifndef  BRCM_RSDB
	cfg->sta_info = (void *)kzalloc(sizeof(*cfg->sta_info), GFP_KERNEL);
	if (unlikely(!cfg->sta_info)) {
		WL_ERR(("sta info  alloc failed\n"));
		goto init_priv_mem_out;
	}
#endif
#if defined(STATIC_WL_PRIV_STRUCT)
	cfg->conn_info = (void *)kzalloc(sizeof(*cfg->conn_info), GFP_KERNEL);
	if (unlikely(!cfg->conn_info)) {
		WL_ERR(("cfg->conn_info  alloc failed\n"));
		goto init_priv_mem_out;
	}
	cfg->ie = (void *)kzalloc(sizeof(*cfg->ie), GFP_KERNEL);
	if (unlikely(!cfg->ie)) {
		WL_ERR(("cfg->ie  alloc failed\n"));
		goto init_priv_mem_out;
	}
	wl_init_escan_result_buf(cfg);
#endif /* STATIC_WL_PRIV_STRUCT */
	cfg->afx_hdl = (void *)kzalloc(sizeof(*cfg->afx_hdl), GFP_KERNEL);
	if (unlikely(!cfg->afx_hdl)) {
		WL_ERR(("afx hdl  alloc failed\n"));
		goto init_priv_mem_out;
	} else {
		init_completion(&cfg->act_frm_scan);
		init_completion(&cfg->wait_next_af);

		INIT_WORK(&cfg->afx_hdl->work, wl_cfg80211_afx_handler);
	}
#ifdef  BRCM_RSDB
#ifdef WLTDLS
	if (cfg->tdls_mgmt_frame) {
		kfree(cfg->tdls_mgmt_frame);
		cfg->tdls_mgmt_frame = NULL;
	}
#endif /* WLTDLS */
#endif
	return 0;

init_priv_mem_out:
	wl_deinit_priv_mem(cfg);

	return -ENOMEM;
}

static void wl_deinit_priv_mem(struct bcm_cfg80211 *cfg)
{
	kfree(cfg->scan_results);
	cfg->scan_results = NULL;
	kfree(cfg->conf);
	cfg->conf = NULL;
	kfree(cfg->scan_req_int);
	cfg->scan_req_int = NULL;
	kfree(cfg->ioctl_buf);
	cfg->ioctl_buf = NULL;
	kfree(cfg->escan_ioctl_buf);
	cfg->escan_ioctl_buf = NULL;
	kfree(cfg->extra_buf);
	cfg->extra_buf = NULL;
	kfree(cfg->pmk_list);
	cfg->pmk_list = NULL;
#ifndef  BRCM_RSDB
	kfree(cfg->sta_info);
	cfg->sta_info = NULL;
#endif
#if defined(STATIC_WL_PRIV_STRUCT)
	kfree(cfg->conn_info);
	cfg->conn_info = NULL;
	kfree(cfg->ie);
	cfg->ie = NULL;
	wl_deinit_escan_result_buf(cfg);
#endif /* STATIC_WL_PRIV_STRUCT */
	if (cfg->afx_hdl) {
		cancel_work_sync(&cfg->afx_hdl->work);
		kfree(cfg->afx_hdl);
		cfg->afx_hdl = NULL;
	}
#ifndef  BRCM_RSDB
	if (cfg->ap_info) {
		kfree(cfg->ap_info->wpa_ie);
		kfree(cfg->ap_info->rsn_ie);
		kfree(cfg->ap_info->wps_ie);
		kfree(cfg->ap_info);
		cfg->ap_info = NULL;
	}
#endif
}

static s32 wl_create_event_handler(struct bcm_cfg80211 *cfg)
{
	int ret = 0;
	WL_DBG(("Enter \n"));
	if(NULL == cfg)
		return -ENOMEM;

	/* Do not use DHD in cfg driver */
	cfg->event_tsk.thr_pid = -1;

	PROC_START(wl_event_handler, cfg, &cfg->event_tsk, 0, "wl_event_handler");
	if (cfg->event_tsk.thr_pid < 0)
		ret = -ENOMEM;
	return ret;
}

static void wl_destroy_event_handler(struct bcm_cfg80211 *cfg)
{
	if (cfg->event_tsk.thr_pid >= 0)
		PROC_STOP(&cfg->event_tsk);
}
#ifdef  BRCM_RSDB
void wl_terminate_event_handler(void)
{
	struct bcm_cfg80211 *cfg = g_bcm_cfg;

	if (cfg) {
		wl_destroy_event_handler(cfg);
		wl_flush_eq(cfg);
	}
}
#endif
static void wl_scan_timeout(unsigned long data)
{
	wl_event_msg_t msg;
	struct bcm_cfg80211 *cfg = (struct bcm_cfg80211 *)data;
#ifdef  BRCM_RSDB
	struct wireless_dev *wdev = NULL;
	struct net_device *ndev = NULL;
	struct wl_scan_results *bss_list;
	struct wl_bss_info *bi = NULL;
	s32 i;
	u32 channel;
#if defined(DHD_DEBUG) && defined(BCMPCIE) && defined(DHD_FW_COREDUMP)
	dhd_pub_t *dhdp = (dhd_pub_t *)(cfg->pub);
	uint32 prev_memdump_mode = dhdp->memdump_enabled;
#endif /* DHD_DEBUG && BCMPCIE */
#endif
	if (!(cfg->scan_request)) {
		WL_ERR(("timer expired but no scan request\n"));
		return;
	}
#ifdef  BRCM_RSDB
	bss_list = wl_escan_get_buf(cfg, FALSE);
	if (!bss_list) {
		WL_ERR(("bss_list is null. Didn't receive any partial scan results\n"));
	} else {
		WL_ERR(("scanned AP count (%d)\n", bss_list->count));

		bi = next_bss(bss_list, bi);
		for_each_bss(bss_list, bi, i) {
			channel = wf_chspec_ctlchan(wl_chspec_driver_to_host(bi->chanspec));
			WL_ERR(("SSID :%s  Channel :%d\n", bi->SSID, channel));
		}
	}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 6, 0))
	if (cfg->scan_request->dev)
		wdev = cfg->scan_request->dev->ieee80211_ptr;
#else
	wdev = cfg->scan_request->wdev;
#endif /* LINUX_VERSION < KERNEL_VERSION(3, 6, 0) */
	if (!wdev) {
		WL_ERR(("No wireless_dev present\n"));
		return;
	}
	ndev = wdev_to_wlc_ndev(wdev, cfg);
#endif
	bzero(&msg, sizeof(wl_event_msg_t));
	WL_ERR(("timer expired\n"));
#ifdef  BRCM_RSDB
#if defined(DHD_DEBUG) && defined(BCMPCIE) && defined(DHD_FW_COREDUMP)
	if (dhdp->memdump_enabled) {
		dhdp->memdump_enabled = DUMP_MEMFILE;
		dhdp->memdump_type = DUMP_TYPE_SCAN_TIMEOUT;
		dhd_bus_mem_dump(dhdp);
		dhdp->memdump_enabled = prev_memdump_mode;
	}
#endif /* DHD_DEBUG && BCMPCIE */
#endif
	msg.event_type = hton32(WLC_E_ESCAN_RESULT);
	msg.status = hton32(WLC_E_STATUS_TIMEOUT);
	msg.reason = 0xFFFFFFFF;
#ifndef  BRCM_RSDB
	wl_cfg80211_event(bcmcfg_to_prmry_ndev(cfg), &msg, NULL);
#else
	wl_cfg80211_event(ndev, &msg, NULL);
#ifdef CUSTOMER_HW4_DEBUG
	if (!wl_scan_timeout_dbg_enabled)
		wl_scan_timeout_dbg_set();
#endif /* CUSTOMER_HW4_DEBUG */
#endif
}
#ifndef  BRCM_RSDB
static void wl_send_event(struct net_device *dev, uint32 event_type,
				uint32 status, uint32 reason)
{
	wl_event_msg_t msg;
	bzero(&msg, sizeof(wl_event_msg_t));
	msg.event_type = hton32(event_type);
	msg.status = hton32(status);
	msg.reason = hton32(reason);
	wl_cfg80211_event(dev, &msg, NULL);
}
#else
#ifdef DHD_LOSSLESS_ROAMING
static void wl_del_roam_timeout(struct bcm_cfg80211 *cfg)
{
	dhd_pub_t *dhdp = (dhd_pub_t *)(cfg->pub);

	/* restore prec_map to ALLPRIO */
	dhdp->dequeue_prec_map = ALLPRIO;
	if (timer_pending(&cfg->roam_timeout)) {
		del_timer_sync(&cfg->roam_timeout);
	}

}

static void wl_roam_timeout(unsigned long data)
{
	struct bcm_cfg80211 *cfg = (struct bcm_cfg80211 *)data;
	dhd_pub_t *dhdp = (dhd_pub_t *)(cfg->pub);

	WL_ERR(("roam timer expired\n"));

	/* restore prec_map to ALLPRIO */
	dhdp->dequeue_prec_map = ALLPRIO;
}

#endif /* DHD_LOSSLESS_ROAMING */
#endif
static s32
wl_cfg80211_netdev_notifier_call(struct notifier_block * nb,
	unsigned long state,
	void *ndev)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,1,0)) && defined(HW_KERNEL_4_0_ADAPTATION)
	struct net_device *dev = netdev_notifier_info_to_dev(ndev);
#else
	struct net_device *dev = ndev;
#endif
#ifndef  BRCM_RSDB
	struct wireless_dev *wdev = dev->ieee80211_ptr;
#else
	struct wireless_dev *wdev = ndev_to_wdev(dev);
#endif
	struct bcm_cfg80211 *cfg = g_bcm_cfg;

#ifdef DHD_IFDEBUG
	WL_ERR(("Enter \n"));
#endif

	if (!wdev || !cfg || dev == bcmcfg_to_prmry_ndev(cfg))
		return NOTIFY_DONE;

	switch (state) {
		case NETDEV_DOWN:
		{
#ifndef HW_WIFI_WDEV_CLEANUP_WORK_PANIC
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 11, 0))
			int max_wait_timeout = 2;
			int max_wait_count = 100;
			int refcnt = 0;
			unsigned long limit = jiffies + max_wait_timeout * HZ;
#ifdef DHD_IFDEBUG
			WL_ERR(("NETDEV_DOWN(+) wdev=%p, cfg=%p, dev=%p\n", wdev, cfg, dev));
#endif
			while (work_pending(&wdev->cleanup_work)) {
				if (refcnt%5 == 0) {
					WL_ERR(("[NETDEV_DOWN] wait for "
						"complete of cleanup_work"
						" (%d th)\n", refcnt));
				}
				if (!time_before(jiffies, limit)) {
					WL_ERR(("[NETDEV_DOWN] cleanup_work"
						" of CFG80211 is not"
						" completed in %d sec\n",
						max_wait_timeout));
					break;
				}
				if (refcnt >= max_wait_count) {
					WL_ERR(("[NETDEV_DOWN] cleanup_work"
						" of CFG80211 is not"
						" completed in %d loop\n",
						max_wait_count));
					break;
				}
				set_current_state(TASK_INTERRUPTIBLE);
				(void)schedule_timeout(100);
				set_current_state(TASK_RUNNING);
				refcnt++;
			}
#ifdef DHD_IFDEBUG
			WL_ERR(("NETDEV_DOWN(-) wdev=%p, cfg=%p, dev=%p\n", wdev, cfg, dev));
#endif
#endif /* LINUX_VERSION < VERSION(3, 14, 0) */
#endif /* HW_WIFI_WDEV_CLEANUP_WORK_PANIC */
			break;
		}

		case NETDEV_UNREGISTER:
#ifdef DHD_IFDEBUG
			WL_ERR(("NETDEV_UNREGISTER(+) wdev=%p, cfg=%p, dev=%p\n", wdev, cfg, dev));
#endif
			/* after calling list_del_rcu(&wdev->list) */
#ifndef  BRCM_RSDB
#ifdef HW_KERNEL_4_0_ADAPTATION
			wl_dealloc_netinfo(cfg, dev);
#else
			wl_dealloc_netinfo(cfg, ndev);
#endif
#else
			wl_cfg80211_clear_per_bss_ies(cfg,
			wl_get_bssidx_by_wdev(cfg, wdev));
			wl_dealloc_netinfo_by_wdev(cfg, wdev);
#ifdef DHD_IFDEBUG
			WL_ERR(("NETDEV_UNREGISTER(-) wdev=%p, cfg=%p, dev=%p\n", wdev, cfg, dev));
#endif
#endif /* BRCM_RSDB */
			break;
		case NETDEV_GOING_DOWN:
			/*
			 * At NETDEV_DOWN state, wdev_cleanup_work work will be called.
			 * In front of door, the function checks whether current scan
			 * is working or not. If the scanning is still working,
			 * wdev_cleanup_work call WARN_ON and make the scan done forcibly.
			 */
#ifdef DHD_IFDEBUG
			WL_ERR(("NETDEV_GOING_DOWN wdev=%p, cfg=%p, dev=%p\n", wdev, cfg, dev));
#endif
			if (wl_get_drv_status(cfg, SCANNING, dev))
				wl_notify_escan_complete(cfg, dev, true, true);
			break;
	}
	return NOTIFY_DONE;
}
static struct notifier_block wl_cfg80211_netdev_notifier = {
	.notifier_call = wl_cfg80211_netdev_notifier_call,
};
/* to make sure we won't register the same notifier twice, otherwise a loop is likely to be
 * created in kernel notifier link list (with 'next' pointing to itself)
 */
static bool wl_cfg80211_netdev_notifier_registered = FALSE;
#ifdef  BRCM_RSDB
static void wl_cfg80211_cancel_scan(struct bcm_cfg80211 *cfg)
{
	struct wireless_dev *wdev = NULL;
	struct net_device *ndev = NULL;

	if (!cfg->scan_request)
		return;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 6, 0))
	if (cfg->scan_request->dev)
		wdev = cfg->scan_request->dev->ieee80211_ptr;
#else
	wdev = cfg->scan_request->wdev;
#endif /* LINUX_VERSION < KERNEL_VERSION(3, 6, 0) */

	if (!wdev) {
		WL_ERR(("No wireless_dev present\n"));
		return;
	}

	ndev = wdev_to_wlc_ndev(wdev, cfg);
	wl_notify_escan_complete(cfg, ndev, true, true);
	WL_ERR(("Scan aborted! \n"));
}
#endif
static void wl_cfg80211_scan_abort(struct bcm_cfg80211 *cfg)
{
	wl_scan_params_t *params = NULL;
	s32 params_size = 0;
	s32 err = BCME_OK;
	struct net_device *dev = bcmcfg_to_prmry_ndev(cfg);
	if (!in_atomic()) {
		/* Our scan params only need space for 1 channel and 0 ssids */
		params = wl_cfg80211_scan_alloc_params(-1, 0, &params_size);
		if (params == NULL) {
			WL_ERR(("scan params allocation failed \n"));
			err = -ENOMEM;
		} else {
			/* Do a scan abort to stop the driver's scan engine */
			err = wldev_ioctl(dev, WLC_SCAN, params, params_size, true);
			if (err < 0) {
				WL_ERR(("scan abort  failed \n"));
			}
			kfree(params);
		}
	}
#ifdef  BRCM_RSDB
#ifdef WLTDLS
	if (cfg->tdls_mgmt_frame) {
		kfree(cfg->tdls_mgmt_frame);
		cfg->tdls_mgmt_frame = NULL;
	}
#endif /* WLTDLS */
#endif
}

static s32 wl_notify_escan_complete(struct bcm_cfg80211 *cfg,
	struct net_device *ndev,
	bool aborted, bool fw_abort)
{
	s32 err = BCME_OK;
	unsigned long flags;
	struct net_device *dev;

	WL_DBG(("Enter \n"));

	mutex_lock(&cfg->scan_complete);

	if (!ndev) {
		WL_ERR(("ndev is null\n"));
		err = BCME_ERROR;
		goto out;
	}

	if (cfg->escan_info.ndev != ndev) {
		WL_ERR(("ndev is different %p %p\n", cfg->escan_info.ndev, ndev));
		err = BCME_ERROR;
		goto out;
	}

	if (cfg->scan_request) {
		dev = bcmcfg_to_prmry_ndev(cfg);
#ifndef  BRCM_RSDB
#if defined(WL_ENABLE_P2P_IF)
		if (cfg->scan_request->dev != cfg->p2p_net)
			dev = cfg->scan_request->dev;
#endif /* WL_ENABLE_P2P_IF */
#else  /* BRCM_RSDB */
#if defined(WL_ENABLE_P2P_IF)
		if (cfg->scan_request->dev != cfg->p2p_net)
			dev = cfg->scan_request->dev;
#elif defined(WL_CFG80211_P2P_DEV_IF)
		if (cfg->scan_request->wdev->iftype != NL80211_IFTYPE_P2P_DEVICE) {
#ifdef DHD_IFDEBUG
			WL_ERR(("%s: dev: %p\n", __FUNCTION__, cfg->scan_request->wdev->netdev));
#endif
			dev = cfg->scan_request->wdev->netdev;
		}
#endif /* WL_ENABLE_P2P_IF */
#endif /* BRCM_RSDB */
	}
	else {
		WL_DBG(("cfg->scan_request is NULL may be internal scan."
			"doing scan_abort for ndev %p primary %p",
				ndev, bcmcfg_to_prmry_ndev(cfg)));
		dev = ndev;
	}
	if (fw_abort && !in_atomic())
		wl_cfg80211_scan_abort(cfg);
	if (timer_pending(&cfg->scan_timeout))
		del_timer_sync(&cfg->scan_timeout);
#if defined(ESCAN_RESULT_PATCH)
	if ((likely(cfg->scan_request)||likely(cfg->sched_scan_req))
#ifdef BCM_PATCH_ESCAN_ABORTED_WLINFORMBSS
		&& (!aborted)
#endif /* BCM_PATCH_ESCAN_ABORTED_WLINFORMBSS */
	) {
		cfg->bss_list = wl_escan_get_buf(cfg, aborted);
		wl_inform_bss(cfg);
	}
#endif /* ESCAN_RESULT_PATCH */
	spin_lock_irqsave(&cfg->cfgdrv_lock, flags);
#ifdef WL_SCHED_SCAN
	if (cfg->sched_scan_req && !cfg->scan_request) {
		WL_PNO((">>> REPORTING SCHED SCAN RESULTS \n"));
		if (!aborted)
			cfg80211_sched_scan_results(cfg->sched_scan_req->wiphy);
		cfg->sched_scan_running = FALSE;
		cfg->sched_scan_req = NULL;
	}
#endif /* WL_SCHED_SCAN */
#ifdef HW_CHECK_WIPHY_NULL
	if (likely(cfg->scan_request && cfg->scan_request->wiphy )){
#else
	if (likely(cfg->scan_request)) {
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 8, 0))
		struct cfg80211_scan_info info = {
			.aborted = aborted,
		};
		cfg80211_scan_done(cfg->scan_request, &info);
#else
		cfg80211_scan_done(cfg->scan_request, aborted);
#endif
		cfg->scan_request = NULL;
#ifdef  BRCM_RSDB
		DHD_OS_SCAN_WAKE_UNLOCK((dhd_pub_t *)(cfg->pub));
		//DHD_ENABLE_RUNTIME_PM((dhd_pub_t *)(cfg->pub));
#endif
	}
	if (p2p_is_on(cfg))
		wl_clr_p2p_status(cfg, SCANNING);
	wl_clr_drv_status(cfg, SCANNING, dev);
	spin_unlock_irqrestore(&cfg->cfgdrv_lock, flags);

out:
	mutex_unlock(&cfg->scan_complete);
	return err;
}

#ifdef  BRCM_RSDB
#ifdef ESCAN_BUF_OVERFLOW_MGMT
static void
wl_cfg80211_find_removal_candidate(wl_bss_info_t *bss, removal_element_t *candidate)
{
	int idx;
	for (idx = 0; idx < BUF_OVERFLOW_MGMT_COUNT; idx++) {
		int len = BUF_OVERFLOW_MGMT_COUNT - idx - 1;
		if (bss->RSSI < candidate[idx].RSSI) {
			if (len)
				memcpy(&candidate[idx + 1], &candidate[idx],
					sizeof(removal_element_t) * len);
			candidate[idx].RSSI = bss->RSSI;
			candidate[idx].length = bss->length;
			memcpy(&candidate[idx].BSSID, &bss->BSSID, ETHER_ADDR_LEN);
			return;
		}
	}
}

static void
wl_cfg80211_remove_lowRSSI_info(wl_scan_results_t *list, removal_element_t *candidate,
	wl_bss_info_t *bi)
{
	int idx1, idx2;
	int total_delete_len = 0;
	for (idx1 = 0; idx1 < BUF_OVERFLOW_MGMT_COUNT; idx1++) {
		int cur_len = WL_SCAN_RESULTS_FIXED_SIZE;
		wl_bss_info_t *bss = NULL;
		if (candidate[idx1].RSSI >= bi->RSSI)
			continue;
		for (idx2 = 0; idx2 < list->count; idx2++) {
			bss = bss ? (wl_bss_info_t *)((uintptr)bss + dtoh32(bss->length)) :
				list->bss_info;
			if (!bcmp(&candidate[idx1].BSSID, &bss->BSSID, ETHER_ADDR_LEN) &&
				candidate[idx1].RSSI == bss->RSSI &&
				candidate[idx1].length == dtoh32(bss->length)) {
				u32 delete_len = dtoh32(bss->length);
				WL_DBG(("delete scan info of " MACDBG " to add new AP\n",
					MAC2STRDBG(bss->BSSID.octet)));
				if (idx2 < list->count -1) {
					memmove((u8 *)bss, (u8 *)bss + delete_len,
						list->buflen - cur_len - delete_len);
				}
				list->buflen -= delete_len;
				list->count--;
				total_delete_len += delete_len;
				/* if delete_len is greater than or equal to result length */
				if (total_delete_len >= bi->length) {
					return;
				}
				break;
			}
			cur_len += dtoh32(bss->length);
		}
	}
}
#endif /* ESCAN_BUF_OVERFLOW_MGMT */
#endif
#if defined(DHD_DEBUG) && defined(HW_ESCAN_ERROR_PATCH)
static unsigned int escan_fail_counter = 0;	/* scan abort counter ccbc */
static int dhd_console_ms_save = -1;		/* save origin dhd_console_ms value */
#endif

static s32 wl_escan_handler(struct bcm_cfg80211 *cfg, bcm_struct_cfgdev *cfgdev,
	const wl_event_msg_t *e, void *data)
{
	s32 err = BCME_OK;
	s32 status = ntoh32(e->status);
	wl_bss_info_t *bi;
	wl_escan_result_t *escan_result;
	wl_bss_info_t *bss = NULL;
	wl_scan_results_t *list;
	wifi_p2p_ie_t * p2p_ie;
	struct net_device *ndev = NULL;
	u32 bi_length;
	u32 i;
	u8 *p2p_dev_addr = NULL;

#ifdef HW_LOG_PATCH1
	if (WLC_E_STATUS_PARTIAL != status && WLC_E_STATUS_SUCCESS != status) {
		hw_dhd_log("%s: status: %d", __FUNCTION__, status);
	}
#else
	WL_DBG((" enter event type : %d, status : %d \n",
		ntoh32(e->event_type), ntoh32(e->status)));
#endif

	ndev = cfgdev_to_wlc_ndev(cfgdev, cfg);

	mutex_lock(&cfg->usr_sync);
	/* P2P SCAN is coming from primary interface */
	if (wl_get_p2p_status(cfg, SCANNING)) {
		if (wl_get_drv_status_all(cfg, SENDING_ACT_FRM))
			ndev = cfg->afx_hdl->dev;
		else
			ndev = cfg->escan_info.ndev;

	}
	if (!ndev || (!wl_get_drv_status(cfg, SCANNING, ndev) && !cfg->sched_scan_running)) {
		WL_ERR(("escan is not ready ndev %p drv_status 0x%x e_type %d e_states %d\n",
			ndev, wl_get_drv_status(cfg, SCANNING, ndev),
			ntoh32(e->event_type), ntoh32(e->status)));

#if defined(DHD_DEBUG) && defined(HW_ESCAN_ERROR_PATCH)
		if(escan_fail_counter == 5){
			dhd_console_ms_save = (int)dhd_console_ms;
			dhd_console_ms = 10;	/* Set it to get firmware log ccbc */
			WL_ERR(("escan_fail counter reach 5, set firmware log ms=%d; origin ms=%d\n",
				dhd_console_ms,dhd_console_ms_save));
		}
		escan_fail_counter++;
		if(escan_fail_counter == UINT_MAX)
			escan_fail_counter = 6;
#endif
		goto exit;
	}

#if defined(DHD_DEBUG) && defined(HW_ESCAN_ERROR_PATCH)
	if(dhd_console_ms_save != -1){
		WL_ERR(("escan_fail restore firmware log ms=%d; escan fail counter=%d\n",
			dhd_console_ms_save,escan_fail_counter));
		dhd_console_ms = (unsigned int)dhd_console_ms_save;	/* restore it */
		dhd_console_ms_save = -1;
	}
	escan_fail_counter = 0;
#endif

	escan_result = (wl_escan_result_t *)data;

#ifdef SCAN_TIMEOUT_FIX
	if (status != WLC_E_STATUS_TIMEOUT){
		scan_timeout_counter = 0;
		//WL_ERR(("SCAN TIMEOUT counter clear\n"));
	}
#endif



	if (status == WLC_E_STATUS_PARTIAL) {
		WL_INFORM(("WLC_E_STATUS_PARTIAL \n"));
		if (!escan_result) {
			WL_ERR(("Invalid escan result (NULL pointer)\n"));
			goto exit;
		}
#ifdef BCM_PATCH_SECURITY_2017_07
		if ((dtoh32(escan_result->buflen) > (int)ESCAN_BUF_SIZE) ||
		    (dtoh32(escan_result->buflen) < sizeof(wl_escan_result_t))) {
			WL_ERR(("Invalid escan buffer len:%d\n", dtoh32(escan_result->buflen)));
			goto exit;
		}
#endif
		if (dtoh16(escan_result->bss_count) != 1) {
			WL_ERR(("Invalid bss_count %d: ignoring\n", escan_result->bss_count));
			goto exit;
		}
		bi = escan_result->bss_info;
		if (!bi) {
			WL_ERR(("Invalid escan bss info (NULL pointer)\n"));
			goto exit;
		}
		bi_length = dtoh32(bi->length);
		if (bi_length != (dtoh32(escan_result->buflen) - WL_ESCAN_RESULTS_FIXED_SIZE)) {
			WL_ERR(("Invalid bss_info length %d: ignoring\n", bi_length));
			goto exit;
		}
		if (wl_escan_check_sync_id(status, escan_result->sync_id,
			cfg->escan_info.cur_sync_id) < 0)
			goto exit;

		if (!(bcmcfg_to_wiphy(cfg)->interface_modes & BIT(NL80211_IFTYPE_ADHOC))) {
			if (dtoh16(bi->capability) & DOT11_CAP_IBSS) {
				WL_DBG(("Ignoring IBSS result\n"));
				goto exit;
			}
		}

		if (wl_get_drv_status_all(cfg, FINDING_COMMON_CHANNEL)) {
			p2p_dev_addr = wl_cfgp2p_retreive_p2p_dev_addr(bi, bi_length);
			if (p2p_dev_addr && !memcmp(p2p_dev_addr,
				cfg->afx_hdl->tx_dst_addr.octet, ETHER_ADDR_LEN)) {
				s32 channel = wf_chspec_ctlchan(
					wl_chspec_driver_to_host(bi->chanspec));

				if ((channel > MAXCHANNEL) || (channel <= 0))
					channel = WL_INVALID;
				else
					WL_ERR(("ACTION FRAME SCAN : Peer " MACDBG " found,"
						" channel : %d\n",
						MAC2STRDBG(cfg->afx_hdl->tx_dst_addr.octet),
						channel));

				wl_clr_p2p_status(cfg, SCANNING);
				cfg->afx_hdl->peer_chan = channel;
				complete(&cfg->act_frm_scan);
				goto exit;
			}

		} else {
			int cur_len = WL_SCAN_RESULTS_FIXED_SIZE;
#if defined(ESCAN_BUF_OVERFLOW_MGMT) && defined(BRCM_RSDB)
			removal_element_t candidate[BUF_OVERFLOW_MGMT_COUNT];
			int remove_lower_rssi = FALSE;

			bzero(candidate, sizeof(removal_element_t)*BUF_OVERFLOW_MGMT_COUNT);
#endif /* ESCAN_BUF_OVERFLOW_MGMT */

			list = wl_escan_get_buf(cfg, FALSE);
			if (scan_req_match(cfg)) {
#if defined(WL_HOST_BAND_MGMT) && defined(BRCM_RSDB)
				s32 channel = 0;
				s32 channel_band = 0;
				chanspec_t chspec;
#endif /* WL_HOST_BAND_MGMT */
				/* p2p scan && allow only probe response */
				if ((cfg->p2p->search_state != WL_P2P_DISC_ST_SCAN) &&
					(bi->flags & WL_BSS_FLAGS_FROM_BEACON))
					goto exit;
				if ((p2p_ie = wl_cfgp2p_find_p2pie(((u8 *) bi) + bi->ie_offset,
					bi->ie_length)) == NULL) {
						WL_ERR(("Couldn't find P2PIE in probe"
							" response/beacon\n"));
						goto exit;
				}
#if defined(WL_HOST_BAND_MGMT) && defined(BRCM_RSDB)
				chspec = wl_chspec_driver_to_host(bi->chanspec);
				channel = wf_chspec_ctlchan(chspec);
				channel_band = CHSPEC2WLC_BAND(chspec);

				if ((cfg->curr_band == WLC_BAND_5G) &&
					(channel_band == WLC_BAND_2G)) {
					/* Avoid sending the GO results in band conflict */
					if (wl_cfgp2p_retreive_p2pattrib(p2p_ie,
						P2P_SEID_GROUP_ID) != NULL)
						goto exit;
				}
#endif /* WL_HOST_BAND_MGMT */
			}
#if defined(ESCAN_BUF_OVERFLOW_MGMT) && defined(BRCM_RSDB)
			if (bi_length > ESCAN_BUF_SIZE - list->buflen)
				remove_lower_rssi = TRUE;
#endif /* ESCAN_BUF_OVERFLOW_MGMT */

			for (i = 0; i < list->count; i++) {
				bss = bss ? (wl_bss_info_t *)((uintptr)bss + dtoh32(bss->length))
					: list->bss_info;
#if defined(ESCAN_BUF_OVERFLOW_MGMT) && defined(BRCM_RSDB)
				WL_TRACE(("%s("MACDBG"), i=%d bss: RSSI %d list->count %d\n",
					bss->SSID, MAC2STRDBG(bss->BSSID.octet),
					i, bss->RSSI, list->count));

				if (remove_lower_rssi)
					wl_cfg80211_find_removal_candidate(bss, candidate);
#endif /* ESCAN_BUF_OVERFLOW_MGMT */

				if (!bcmp(&bi->BSSID, &bss->BSSID, ETHER_ADDR_LEN) &&
					(CHSPEC_BAND(wl_chspec_driver_to_host(bi->chanspec))
					== CHSPEC_BAND(wl_chspec_driver_to_host(bss->chanspec))) &&
					bi->SSID_len == bss->SSID_len &&
					!bcmp(bi->SSID, bss->SSID, bi->SSID_len)) {

					/* do not allow beacon data to update
					*the data recd from a probe response
					*/
					if (!(bss->flags & WL_BSS_FLAGS_FROM_BEACON) &&
						(bi->flags & WL_BSS_FLAGS_FROM_BEACON))
						goto exit;
#if !defined(HW_LOG_PATCH1) && defined(HW_SCAN_TRACE)
					if (g_scan_trace == 1) {
						WL_ERR(("%s("MACDBG"), i=%d prev: RSSI %d"
							" flags 0x%x, new: RSSI %d flags 0x%x\n",
							bss->SSID, MAC2STRDBG(bi->BSSID.octet), i,
							bss->RSSI, bss->flags, bi->RSSI, bi->flags));
						g_scan_trace = 0;
					}
#else
					WL_DBG(("%s("MACDBG"), i=%d prev: RSSI %d"
						" flags 0x%x, new: RSSI %d flags 0x%x\n",
						bss->SSID, MAC2STRDBG(bi->BSSID.octet), i,
						bss->RSSI, bss->flags, bi->RSSI, bi->flags));
#endif
					if ((bss->flags & WL_BSS_FLAGS_RSSI_ONCHANNEL) ==
						(bi->flags & WL_BSS_FLAGS_RSSI_ONCHANNEL)) {
						/* preserve max RSSI if the measurements are
						* both on-channel or both off-channel
						*/
						WL_SCAN(("%s("MACDBG"), same onchan"
						", RSSI: prev %d new %d\n",
						bss->SSID, MAC2STRDBG(bi->BSSID.octet),
						bss->RSSI, bi->RSSI));
						bi->RSSI = MAX(bss->RSSI, bi->RSSI);
					} else if ((bss->flags & WL_BSS_FLAGS_RSSI_ONCHANNEL) &&
						(bi->flags & WL_BSS_FLAGS_RSSI_ONCHANNEL) == 0) {
						/* preserve the on-channel rssi measurement
						* if the new measurement is off channel
						*/
						WL_SCAN(("%s("MACDBG"), prev onchan"
						", RSSI: prev %d new %d\n",
						bss->SSID, MAC2STRDBG(bi->BSSID.octet),
						bss->RSSI, bi->RSSI));
						bi->RSSI = bss->RSSI;
						bi->flags |= WL_BSS_FLAGS_RSSI_ONCHANNEL;
					}
					if (dtoh32(bss->length) != bi_length) {
						u32 prev_len = dtoh32(bss->length);

						WL_SCAN(("bss info replacement"
							" is occured(bcast:%d->probresp%d)\n",
							bss->ie_length, bi->ie_length));
						WL_DBG(("%s("MACDBG"), replacement!(%d -> %d)\n",
						bss->SSID, MAC2STRDBG(bi->BSSID.octet),
						prev_len, bi_length));

						if (list->buflen - prev_len + bi_length
							> ESCAN_BUF_SIZE) {
							WL_ERR(("Buffer is too small: keep the"
								" previous result of this AP\n"));
							/* Only update RSSI */
							bss->RSSI = bi->RSSI;
							bss->flags |= (bi->flags
								& WL_BSS_FLAGS_RSSI_ONCHANNEL);
							goto exit;
						}

						if (i < list->count - 1) {
							/* memory copy required by this case only */
							memmove((u8 *)bss + bi_length,
								(u8 *)bss + prev_len,
								list->buflen - cur_len - prev_len);
						}
						list->buflen -= prev_len;
						list->buflen += bi_length;
					}
					list->version = dtoh32(bi->version);
					memcpy((u8 *)bss, (u8 *)bi, bi_length);
					goto exit;
				}
				cur_len += dtoh32(bss->length);
			}
			if (bi_length > ESCAN_BUF_SIZE - list->buflen) {
#if defined(ESCAN_BUF_OVERFLOW_MGMT) && defined(BRCM_RSDB)
				wl_cfg80211_remove_lowRSSI_info(list, candidate, bi);
				if (bi_length > ESCAN_BUF_SIZE - list->buflen) {
					WL_DBG(("RSSI(" MACDBG ") is too low(%d) to add Buffer\n",
						MAC2STRDBG(bi->BSSID.octet), bi->RSSI));
					goto exit;
				}
#else
				WL_ERR(("Buffer is too small: ignoring\n"));
				goto exit;
#endif /* ESCAN_BUF_OVERFLOW_MGMT */
			}

			memcpy(&(((char *)list)[list->buflen]), bi, bi_length);
			list->version = dtoh32(bi->version);
			list->buflen += bi_length;
			list->count++;
#ifdef  BRCM_RSDB
			/*
			 * !Broadcast && number of ssid = 1 && number of channels =1
			 * means specific scan to association
			 */
			if (wl_cfgp2p_is_p2p_specific_scan(cfg->scan_request)) {
				WL_ERR(("P2P assoc scan fast aborted.\n"));
				wl_notify_escan_complete(cfg, cfg->escan_info.ndev, false, true);
				goto exit;
			}
#endif
		}
	}
	else if (status == WLC_E_STATUS_SUCCESS) {
		cfg->escan_info.escan_state = WL_ESCAN_STATE_IDLE;
		wl_escan_print_sync_id(status, cfg->escan_info.cur_sync_id,
			escan_result->sync_id);

		if (wl_get_drv_status_all(cfg, FINDING_COMMON_CHANNEL)) {
			WL_INFORM(("ACTION FRAME SCAN DONE\n"));
			wl_clr_p2p_status(cfg, SCANNING);
			wl_clr_drv_status(cfg, SCANNING, cfg->afx_hdl->dev);
			if (cfg->afx_hdl->peer_chan == WL_INVALID)
				complete(&cfg->act_frm_scan);
		} else if ((likely(cfg->scan_request)) || (cfg->sched_scan_running)) {
#ifdef HW_SCAN_TRACE
			WL_ERR(("ESCAN COMPLETED\n"));
			g_scan_trace = 1;
#else
			WL_INFORM(("ESCAN COMPLETED\n"));
#endif
#ifndef ESCAN_RESULT_PATCH
			cfg->bss_list = wl_escan_get_buf(cfg, FALSE);
			if (!scan_req_match(cfg)) {
				WL_TRACE_HW4(("SCAN COMPLETED: scanned AP count=%d\n",
					cfg->bss_list->count));
			}
			wl_inform_bss(cfg);
#endif
			wl_notify_escan_complete(cfg, ndev, false, false);
		}
		wl_escan_increment_sync_id(cfg, SCAN_BUF_NEXT);

#ifdef CUSTOMER_HW4_DEBUG
		if (wl_scan_timeout_dbg_enabled)
			wl_scan_timeout_dbg_clear();
#endif /* CUSTOMER_HW4_DEBUG */
	} else if ((status == WLC_E_STATUS_ABORT) || (status == WLC_E_STATUS_NEWSCAN) ||
#ifdef BCMCCX
		(status == WLC_E_STATUS_CCXFASTRM) ||
#endif /* BCMCCX */
		(status == WLC_E_STATUS_11HQUIET) || (status == WLC_E_STATUS_CS_ABORT) ||
		(status == WLC_E_STATUS_NEWASSOC)) {
		/* Handle all cases of scan abort */

		cfg->escan_info.escan_state = WL_ESCAN_STATE_IDLE;
		wl_escan_print_sync_id(status, escan_result->sync_id,
			cfg->escan_info.cur_sync_id);
		WL_ERR(("ESCAN ABORT reason: %d\n", status));

		if (wl_get_drv_status_all(cfg, FINDING_COMMON_CHANNEL)) {
			WL_INFORM(("ACTION FRAME SCAN DONE\n"));
			wl_clr_drv_status(cfg, SCANNING, cfg->afx_hdl->dev);
			wl_clr_p2p_status(cfg, SCANNING);
			if (cfg->afx_hdl->peer_chan == WL_INVALID)
				complete(&cfg->act_frm_scan);
		} else if ((likely(cfg->scan_request)) || (cfg->sched_scan_running)) {
			WL_INFORM(("ESCAN ABORTED\n"));
			cfg->bss_list = wl_escan_get_buf(cfg, TRUE);
			if (!scan_req_match(cfg)) {
				WL_TRACE_HW4(("scan_req_match=0: scanned AP count=%d\n",
					cfg->bss_list->count));
			}
#if defined(DUAL_ESCAN_RESULT_BUFFER) && defined(BRCM_RSDB)
			if (escan_result->sync_id != cfg->escan_info.cur_sync_id) {
				/* If sync_id is not matching, then the abort might have
				 * come for the old scan req or for the in-driver initiated
				 * scan. So do abort for scan_req for which sync_id is
				 * matching.
				 */
				WL_INFORM(("sync_id mismatch (%d != %d). "
					"Ignore the scan abort event.\n",
					escan_result->sync_id, cfg->escan_info.cur_sync_id));
				goto exit;
			} else {
				/* sync id is matching, abort the scan */
				WL_INFORM(("scan aborted for sync_id: %d \n",
					cfg->escan_info.cur_sync_id));
				wl_inform_bss(cfg);
				wl_notify_escan_complete(cfg, ndev, true, false);
			}
#else
			wl_inform_bss(cfg);
			wl_notify_escan_complete(cfg, ndev, true, false);
#endif /* DUAL_ESCAN_RESULT_BUFFER && BRCM_RSDB */
#ifdef BRCM_RSDB
			} else {
			/* If there is no pending host initiated scan, do nothing */
			WL_DBG(("ESCAN ABORT: No pending scans. Ignoring event.\n"));
#endif
		}
		wl_escan_increment_sync_id(cfg, SCAN_BUF_CNT);
	} else if (status == WLC_E_STATUS_TIMEOUT) {
		WL_ERR(("WLC_E_STATUS_TIMEOUT : scan_request[%p]\n", cfg->scan_request));
		WL_ERR(("reason[0x%x]\n", e->reason));
		if (e->reason == 0xFFFFFFFF) {
			wl_notify_escan_complete(cfg, cfg->escan_info.ndev, true, true);
		}
#ifdef SCAN_TIMEOUT_FIX
		//continous scan timeout will lead to hang message to recover firmware.
		scan_timeout_counter++;
		WL_ERR(("SCAN TIMEOUT counter %d\n", scan_timeout_counter));
		if ( MAX_SCANTIMEOUT_COUNT == scan_timeout_counter)
		{
			//struct net_device *pdev = bcmcfg_to_prmry_ndev(wl);
			WL_ERR(("SCAN TIMEOUT lead to HANG Notification sent to %s\n", ndev->name));
			net_os_send_hang_message(ndev);
		}
#endif
	} else {
		WL_ERR(("unexpected Escan Event %d : abort\n", status));
		cfg->escan_info.escan_state = WL_ESCAN_STATE_IDLE;
		wl_escan_print_sync_id(status, escan_result->sync_id,
			cfg->escan_info.cur_sync_id);
		if (wl_get_drv_status_all(cfg, FINDING_COMMON_CHANNEL)) {
			WL_INFORM(("ACTION FRAME SCAN DONE\n"));
			wl_clr_p2p_status(cfg, SCANNING);
			wl_clr_drv_status(cfg, SCANNING, cfg->afx_hdl->dev);
			if (cfg->afx_hdl->peer_chan == WL_INVALID)
				complete(&cfg->act_frm_scan);
		} else if ((likely(cfg->scan_request)) || (cfg->sched_scan_running)) {
			cfg->bss_list = wl_escan_get_buf(cfg, TRUE);
			if (!scan_req_match(cfg)) {
				WL_TRACE_HW4(("SCAN ABORTED(UNEXPECTED): "
					"scanned AP count=%d\n",
					cfg->bss_list->count));
			}
			wl_inform_bss(cfg);
			wl_notify_escan_complete(cfg, ndev, true, false);
		}
		wl_escan_increment_sync_id(cfg, 2);
	}
exit:
	mutex_unlock(&cfg->usr_sync);
	return err;
}
#ifndef  BRCM_RSDB
static void wl_cfg80211_concurrent_roam(struct bcm_cfg80211 *cfg, int enable)
{
	u32 connected_cnt  = wl_get_drv_status_all(cfg, CONNECTED);
	struct net_info *iter, *next;
	int err;

	if (!cfg->roamoff_on_concurrent)
		return;
	if (enable && connected_cnt > 1) {
		for_each_ndev(cfg, iter, next) {
			/* Save the current roam setting */
			if ((err = wldev_iovar_getint(iter->ndev, "roam_off",
				(s32 *)&iter->roam_off)) != BCME_OK) {
				WL_ERR(("%s:Failed to get current roam setting err %d\n",
					iter->ndev->name, err));
				continue;
			}
			if ((err = wldev_iovar_setint(iter->ndev, "roam_off", 1)) != BCME_OK) {
				WL_ERR((" %s:failed to set roam_off : %d\n",
					iter->ndev->name, err));
			}
		}
	}
	else if (!enable) {
		for_each_ndev(cfg, iter, next) {
			if (iter->roam_off != WL_INVALID) {
				if ((err = wldev_iovar_setint(iter->ndev, "roam_off",
					iter->roam_off)) == BCME_OK)
					iter->roam_off = WL_INVALID;
				else {
					WL_ERR((" %s:failed to set roam_off : %d\n",
						iter->ndev->name, err));
				}
			}
		}
	}
	return;
}
#else
static void wl_cfg80211_concurrent_roam(struct bcm_cfg80211 *cfg, int enable)
{
	u32 connected_cnt  = wl_get_drv_status_all(cfg, CONNECTED);
	bool p2p_connected  = wl_cfgp2p_vif_created(cfg);
	struct net_info *iter, *next;

	if (!cfg->roamoff_on_concurrent)
		return;
	if (enable && (p2p_connected||(connected_cnt > 1))) {
#if defined(STRICT_GCC_WARNINGS) && defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == \
	4 && __GNUC_MINOR__ >= 6))
_Pragma("GCC diagnostic push")
_Pragma("GCC diagnostic ignored \"-Wcast-qual\"")
#endif
		for_each_ndev(cfg, iter, next) {
			if (iter->ndev && iter->wdev &&
					iter->wdev->iftype == NL80211_IFTYPE_STATION) {
				if (wldev_iovar_setint(iter->ndev, "roam_off", TRUE)
						== BCME_OK) {
					iter->roam_off = TRUE;
				}
				else {
					WL_ERR(("error to enable roam_off\n"));
				}
			}
		}
#if defined(STRICT_GCC_WARNINGS) && defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == \
	4 && __GNUC_MINOR__ >= 6))
_Pragma("GCC diagnostic pop")
#endif
	}
	else if (!enable) {
#if defined(STRICT_GCC_WARNINGS) && defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == \
	4 && __GNUC_MINOR__ >= 6))
_Pragma("GCC diagnostic push")
_Pragma("GCC diagnostic ignored \"-Wcast-qual\"")
#endif
		for_each_ndev(cfg, iter, next) {
			if (iter->ndev && iter->wdev &&
					iter->wdev->iftype == NL80211_IFTYPE_STATION) {
				if (iter->roam_off != WL_INVALID) {
					if (wldev_iovar_setint(iter->ndev, "roam_off", FALSE)
							== BCME_OK) {
						iter->roam_off = FALSE;
					}
					else {
						WL_ERR(("error to disable roam_off\n"));
					}
				}
			}
		}
#if defined(STRICT_GCC_WARNINGS) && defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == \
	4 && __GNUC_MINOR__ >= 6))
_Pragma("GCC diagnostic pop")
#endif
	}
	return;
}
#endif /* BRCM_RSDB */

static void wl_cfg80211_determine_vsdb_mode(struct bcm_cfg80211 *cfg)
{
	struct net_info *iter, *next;
	u32 ctl_chan = 0;
	u32 chanspec = 0;
	u32 pre_ctl_chan = 0;
	u32 connected_cnt  = wl_get_drv_status_all(cfg, CONNECTED);
	cfg->vsdb_mode = false;

	if (connected_cnt <= 1)  {
		return;
	}
#ifdef  BRCM_RSDB
#if defined(STRICT_GCC_WARNINGS) && defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == \
	4 && __GNUC_MINOR__ >= 6))
_Pragma("GCC diagnostic push")
_Pragma("GCC diagnostic ignored \"-Wcast-qual\"")
#endif
#endif
	for_each_ndev(cfg, iter, next) {
#ifdef  BRCM_RSDB
		/* p2p discovery iface ndev could be null */
		if (iter->ndev) {
#endif
		chanspec = 0;
		ctl_chan = 0;
		if (wl_get_drv_status(cfg, CONNECTED, iter->ndev)) {
			if (wldev_iovar_getint(iter->ndev, "chanspec",
				(s32 *)&chanspec) == BCME_OK) {
				chanspec = wl_chspec_driver_to_host(chanspec);
				ctl_chan = wf_chspec_ctlchan(chanspec);
				wl_update_prof(cfg, iter->ndev, NULL,
					&ctl_chan, WL_PROF_CHAN);
			}
			if (!cfg->vsdb_mode) {
				if (!pre_ctl_chan && ctl_chan)
					pre_ctl_chan = ctl_chan;
				else if (pre_ctl_chan && (pre_ctl_chan != ctl_chan)) {
					cfg->vsdb_mode = true;
				}
			}
		}
	}
#ifdef  BRCM_RSDB
	}
#if defined(STRICT_GCC_WARNINGS) && defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == \
	4 && __GNUC_MINOR__ >= 6))
_Pragma("GCC diagnostic pop")
#endif
#endif /* BRCM_RSDB */
	WL_ERR(("%s concurrency is enabled\n", cfg->vsdb_mode ? "Multi Channel" : "Same Channel"));
	return;
}
#ifdef  BRCM_RSDB
#if defined(DISABLE_FRAMEBURST_VSDB) && defined(USE_WFA_CERT_CONF)
extern int g_frameburst;
#endif /* DISABLE_FRAMEBURST_VSDB && USE_WFA_CERT_CONF */
#endif
#ifndef  BRCM_RSDB
static s32 wl_notifier_change_state(struct bcm_cfg80211 *cfg, struct net_info *_net_info,
	enum wl_status state, bool set)
{
	s32 pm = PM_FAST;
	s32 err = BCME_OK;
	u32 mode;
	u32 chan = 0;
	u32 frameburst;
	struct net_info *iter, *next;
	struct net_device *primary_dev = bcmcfg_to_prmry_ndev(cfg);
	WL_DBG(("Enter state %d set %d _net_info->pm_restore %d iface %s\n",
		state, set, _net_info->pm_restore, _net_info->ndev->name));

	mode = wl_get_mode_by_netdev(cfg, _net_info->ndev);
	if (set) {
		if (state == WL_STATUS_CONNECTED) {
			wl_cfg80211_concurrent_roam(cfg, 1);

			if (mode == WL_MODE_AP) {
				if (wl_add_remove_eventmsg(primary_dev, WLC_E_P2P_PROBREQ_MSG, false))
					WL_ERR((" failed to unset WLC_E_P2P_PROPREQ_MSG\n"));
			}
			wl_cfg80211_determine_vsdb_mode(cfg);
			if (cfg->vsdb_mode || _net_info->pm_block) {
				/* Delete pm_enable_work */
				wl_add_remove_pm_enable_work(cfg, FALSE, WL_HANDLER_MAINTAIN);
				/* save PM_FAST in _net_info to restore this
				 * if _net_info->pm_block is false
				 */
				if (!_net_info->pm_block && (mode == WL_MODE_BSS)) {
					_net_info->pm = PM_FAST;
					_net_info->pm_restore = true;
				}
				pm = PM_OFF;
				for_each_ndev(cfg, iter, next) {
					if (iter->pm_restore)
						continue;
					/* Save the current power mode */
					iter->pm = PM_OFF;
					err = wldev_ioctl(iter->ndev, WLC_GET_PM, &iter->pm,
							sizeof(iter->pm), false);
					WL_DBG(("%s:power save %s\n", iter->ndev->name,
								iter->pm ? "enabled" : "disabled"));
					if (!err && iter->pm) {
						iter->pm_restore = true;
					}
				}
				for_each_ndev(cfg, iter, next) {
					if ((err = wldev_ioctl(iter->ndev, WLC_SET_PM, &pm,
									sizeof(pm), true)) != 0) {
						if (err == -ENODEV)
							WL_DBG(("%s:netdev not ready\n", iter->ndev->name));
						else
							WL_ERR(("%s:error (%d)\n", iter->ndev->name, err));
					} else {
						wl_cfg80211_update_power_mode(iter->ndev);
					}
				}
			} else {
				/*
				 * Re-enable PM2 mode for static IP and roaming event
				 */
				pm = PM_FAST;

				for_each_ndev(cfg, iter, next) {
					if ((err = wldev_ioctl(iter->ndev, WLC_SET_PM, &pm,
									sizeof(pm), true)) != 0) {
						if (err == -ENODEV)
							WL_DBG(("%s:netdev not ready\n", iter->ndev->name));
						else
							WL_ERR(("%s:error (%d)\n", iter->ndev->name, err));
					}
				}

				if (cfg->pm_enable_work_on) {
					wl_add_remove_pm_enable_work(cfg, FALSE, WL_HANDLER_DEL);
				}
			}
#if defined(WLTDLS)
#if defined(DISABLE_TDLS_IN_P2P)
			if (cfg->vsdb_mode || p2p_is_on(cfg))
#else
			if (cfg->vsdb_mode)
#endif /* defined(DISABLE_TDLS_IN_P2P) */
			{

				err = wldev_iovar_setint(primary_dev, "tdls_enable", 0);
			}
#endif /* defined(WLTDLS) */
			if (cfg->vsdb_mode) {
				/* disable frameburst on multichannel */
				frameburst = 0;
				if (wldev_ioctl(primary_dev, WLC_SET_FAKEFRAG, &frameburst,
					sizeof(frameburst), true) != 0) {
					WL_DBG(("frameburst set 0 error\n"));
				} else {
					WL_DBG(("Frameburst Disabled\n"));
				}
			}
		}
	} else { /* clear */
		if (state == WL_STATUS_CONNECTED) {
			chan = 0;
			/* clear chan information when the net device is disconnected */
			wl_update_prof(cfg, _net_info->ndev, NULL, &chan, WL_PROF_CHAN);
			wl_cfg80211_determine_vsdb_mode(cfg);
			for_each_ndev(cfg, iter, next) {
				if (iter->pm_restore && iter->pm) {
					WL_DBG(("%s:restoring power save %s\n",
							iter->ndev->name, (iter->pm ? "enabled" : "disabled")));
					err = wldev_ioctl(iter->ndev, WLC_SET_PM, &iter->pm,
								sizeof(iter->pm), true);
					if (unlikely(err)) {
						if (err == -ENODEV)
							WL_DBG(("%s:netdev not ready\n", iter->ndev->name));
						else
							WL_ERR(("%s:error(%d)\n", iter->ndev->name, err));
						break;
					}
					iter->pm_restore = 0;
					wl_cfg80211_update_power_mode(iter->ndev);
				}
			}
			wl_cfg80211_concurrent_roam(cfg, 0);

			if (!cfg->vsdb_mode) {
#if defined(WLTDLS)
				err = wldev_iovar_setint(primary_dev, "tdls_enable", 1);
#endif /* defined(WLTDLS) */
				/* enable frameburst on single channel */
				frameburst = 1;
				if (wldev_ioctl(primary_dev, WLC_SET_FAKEFRAG, &frameburst,
					sizeof(frameburst), true) != 0) {
					WL_DBG(("frameburst set 1 error\n"));
				} else {
					WL_DBG(("Frameburst Enabled\n"));
				}
			}
		} else if (state == WL_STATUS_DISCONNECTING) {
			wake_up_interruptible(&cfg->event_sync_wq);
		}
	}
	return err;
}
#else
static s32 wl_notifier_change_state(struct bcm_cfg80211 *cfg, struct net_info *_net_info,
	enum wl_status state, bool set)
{
	s32 pm = PM_FAST;
	s32 err = BCME_OK;
	u32 mode;
	u32 chan = 0;
	struct net_device *primary_dev = bcmcfg_to_prmry_ndev(cfg);
	dhd_pub_t *dhd =  (dhd_pub_t *)(cfg->pub);
	if (dhd->busstate == DHD_BUS_DOWN) {
		WL_ERR(("%s : busstate is DHD_BUS_DOWN!\n", __FUNCTION__));
		return 0;
	}
	WL_DBG(("Enter state %d set %d _net_info->pm_restore %d iface %s\n",
		state, set, _net_info->pm_restore, _net_info->ndev->name));

	if (state != WL_STATUS_CONNECTED)
		return 0;
	mode = wl_get_mode_by_netdev(cfg, _net_info->ndev);
	if (set) {
		wl_cfg80211_concurrent_roam(cfg, 1);
		wl_cfg80211_determine_vsdb_mode(cfg);
		if (mode == WL_MODE_AP) {
			if (wl_add_remove_eventmsg(primary_dev, WLC_E_P2P_PROBREQ_MSG, false))
				WL_ERR((" failed to unset WLC_E_P2P_PROPREQ_MSG\n"));
		}

		pm = PM_OFF;
		if ((err = wldev_ioctl(_net_info->ndev, WLC_SET_PM, &pm,
				sizeof(pm), true)) != 0) {
			if (err == -ENODEV)
				WL_DBG(("%s:netdev not ready\n",
					_net_info->ndev->name));
			else
				WL_ERR(("%s:error (%d)\n",
					_net_info->ndev->name, err));

			wl_cfg80211_update_power_mode(_net_info->ndev);
		}
		wl_add_remove_pm_enable_work(cfg, WL_PM_WORKQ_SHORT);
#if defined(WLTDLS)
		if (wl_cfg80211_is_concurrent_mode()) {
			err = wldev_iovar_setint(primary_dev, "tdls_enable", 0);
		}
#endif /* defined(WLTDLS) */

#ifdef DISABLE_FRAMEBURST_VSDB
#ifdef USE_WFA_CERT_CONF
		if (g_frameburst)
#endif /* USE_WFA_CERT_CONF */
		{
			if (wl_cfg80211_is_concurrent_mode()) {
				int frameburst = 0;
				if (wldev_ioctl(primary_dev, WLC_SET_FAKEFRAG, &frameburst,
					sizeof(frameburst), true) != 0) {
					WL_DBG(("frameburst set error\n"));
				}
				WL_DBG(("Frameburst Disabled\n"));
			}
		}
#endif /* DISABLE_FRAMEBURST_VSDB */
	} else { /* clear */
		chan = 0;
		/* clear chan information when the net device is disconnected */
		wl_update_prof(cfg, _net_info->ndev, NULL, &chan, WL_PROF_CHAN);
		wl_cfg80211_determine_vsdb_mode(cfg);
		if (primary_dev == _net_info->ndev) {
			pm = PM_FAST;
			if ((err = wldev_ioctl(_net_info->ndev, WLC_SET_PM, &pm,
					sizeof(pm), true)) != 0) {
				if (err == -ENODEV)
					WL_DBG(("%s:netdev not ready\n",
						_net_info->ndev->name));
				else
					WL_ERR(("%s:error (%d)\n",
						_net_info->ndev->name, err));

				wl_cfg80211_update_power_mode(_net_info->ndev);
			}
		}

		wl_cfg80211_concurrent_roam(cfg, 0);
#if defined(WLTDLS)
		if (!wl_cfg80211_is_concurrent_mode()) {
			err = wldev_iovar_setint(primary_dev, "tdls_enable", 1);
		}
#endif /* defined(WLTDLS) */

#ifdef DISABLE_FRAMEBURST_VSDB
#ifdef USE_WFA_CERT_CONF
		if (g_frameburst)
#endif /* USE_WFA_CERT_CONF */
		{
			int frameburst = 1;
			if (wldev_ioctl(primary_dev, WLC_SET_FAKEFRAG, &frameburst,
				sizeof(frameburst), true) != 0) {
				WL_DBG(("frameburst set error\n"));
			}
			WL_DBG(("Frameburst Enabled\n"));
		}
#endif /* DISABLE_FRAMEBURST_VSDB */
	}
	return err;
}
#endif
static s32 wl_init_scan(struct bcm_cfg80211 *cfg)
{
	int err = 0;

	cfg->evt_handler[WLC_E_ESCAN_RESULT] = wl_escan_handler;
	cfg->escan_info.escan_state = WL_ESCAN_STATE_IDLE;
	wl_escan_init_sync_id(cfg);

	/* Init scan_timeout timer */
	init_timer(&cfg->scan_timeout);
	cfg->scan_timeout.data = (unsigned long) cfg;
	cfg->scan_timeout.function = wl_scan_timeout;

	return err;
}

#if defined(DHD_LOSSLESS_ROAMING) && defined(BRCM_RSDB)
static s32 wl_init_roam_timeout(struct bcm_cfg80211 *cfg)
{
	int err = 0;

	/* Init roam timer */
	init_timer(&cfg->roam_timeout);
	cfg->roam_timeout.data = (unsigned long) cfg;
	cfg->roam_timeout.function = wl_roam_timeout;

	return err;
}
#endif /* DHD_LOSSLESS_ROAMING && BRCM_RSDB*/

static s32 wl_init_priv(struct bcm_cfg80211 *cfg)
{
	struct wiphy *wiphy = bcmcfg_to_wiphy(cfg);
	struct net_device *ndev = bcmcfg_to_prmry_ndev(cfg);
	s32 err = 0;

	cfg->scan_request = NULL;
	cfg->pwr_save = !!(wiphy->flags & WIPHY_FLAG_PS_ON_BY_DEFAULT);
	cfg->roam_on = false;
	cfg->active_scan = true;
	cfg->rf_blocked = false;
	cfg->vsdb_mode = false;
#if defined(BCMSDIO)
	cfg->wlfc_on = false;
#endif
	cfg->roamoff_on_concurrent = true;
	cfg->disable_roam_event = false;
#ifdef  BRCM_RSDB
	cfg->cfgdev_bssidx = -1;
#endif
	/* register interested state */
	set_bit(WL_STATUS_CONNECTED, &cfg->interrested_state);
#ifndef  BRCM_RSDB
	set_bit(WL_STATUS_DISCONNECTING, &cfg->interrested_state);
#endif
	spin_lock_init(&cfg->cfgdrv_lock);
#ifdef HW_P2P_SPINLOCK_PATCH
	spin_lock_init(&cfg->p2p_detach_lock);
#endif
	mutex_init(&cfg->ioctl_buf_sync);
	init_waitqueue_head(&cfg->netif_change_event);
#ifndef  BRCM_RSDB
	init_waitqueue_head(&cfg->event_sync_wq);
#endif
	init_completion(&cfg->send_af_done);
	init_completion(&cfg->iface_disable);
	wl_init_eq(cfg);
	err = wl_init_priv_mem(cfg);
	if (err)
		return err;
	if (wl_create_event_handler(cfg))
		return -ENOMEM;
	wl_init_event_handler(cfg);
	mutex_init(&cfg->usr_sync);
	mutex_init(&cfg->event_sync);
	mutex_init(&cfg->scan_complete);
	err = wl_init_scan(cfg);
	if (err)
		return err;
#if defined(DHD_LOSSLESS_ROAMING) && defined(BRCM_RSDB)
	err = wl_init_roam_timeout(cfg);
	if (err) {
		return err;
	}
#endif /* DHD_LOSSLESS_ROAMING */
	wl_init_conf(cfg->conf);
	wl_init_prof(cfg, ndev);
	wl_link_down(cfg);
	DNGL_FUNC(dhd_cfg80211_init, (cfg));

	return err;
}

static void wl_deinit_priv(struct bcm_cfg80211 *cfg)
{
	DNGL_FUNC(dhd_cfg80211_deinit, (cfg));
	wl_destroy_event_handler(cfg);
	wl_flush_eq(cfg);
	wl_link_down(cfg);
	del_timer_sync(&cfg->scan_timeout);
#if defined(DHD_LOSSLESS_ROAMING) && defined(BRCM_RSDB)
	del_timer_sync(&cfg->roam_timeout);
#endif
	wl_deinit_priv_mem(cfg);
	if (wl_cfg80211_netdev_notifier_registered) {
		wl_cfg80211_netdev_notifier_registered = FALSE;
		unregister_netdevice_notifier(&wl_cfg80211_netdev_notifier);
	}
}

#if defined(WL_ENABLE_P2P_IF) || defined(WL_NEWCFG_PRIVCMD_SUPPORT)
static s32 wl_cfg80211_attach_p2p(void)
{
	struct bcm_cfg80211 *cfg = g_bcm_cfg;

	WL_TRACE(("Enter \n"));

	if (wl_cfgp2p_register_ndev(cfg) < 0) {
		WL_ERR(("P2P attach failed. \n"));
		return -ENODEV;
	}

	return 0;
}

static s32  wl_cfg80211_detach_p2p(void)
{
	struct bcm_cfg80211 *cfg = g_bcm_cfg;
#ifndef WL_NEWCFG_PRIVCMD_SUPPORT
	struct wireless_dev *wdev;
#endif /* WL_NEWCFG_PRIVCMD_SUPPORT */

	WL_DBG(("Enter \n"));
	if (!cfg) {
		WL_ERR(("Invalid Ptr\n"));
		return -EINVAL;
#ifndef  BRCM_RSDB
	} else
		wdev = cfg->p2p_wdev;

	if (!wdev) {
		WL_ERR(("Invalid Ptr\n"));
		return -EINVAL;
#else
	}
#ifndef WL_NEWCFG_PRIVCMD_SUPPORT
	else {
		wdev = cfg->p2p_wdev;
		if (!wdev) {
			WL_ERR(("Invalid Ptr\n"));
			return -EINVAL;
		}
	}
#endif /* WL_NEWCFG_PRIVCMD_SUPPORT */
#endif /* BRCM_RSDB */

	wl_cfgp2p_unregister_ndev(cfg);

	cfg->p2p_wdev = NULL;
	cfg->p2p_net = NULL;
#ifndef  BRCM_RSDB
	WL_DBG(("Freeing 0x%08x \n", (unsigned int)wdev));
	kfree(wdev);
#else
#ifndef WL_NEWCFG_PRIVCMD_SUPPORT
	WL_DBG(("Freeing 0x%p \n", wdev));
	kfree(wdev);
#endif /* WL_NEWCFG_PRIVCMD_SUPPORT */
#endif

	return 0;
}
#endif

s32 wl_cfg80211_attach_post(struct net_device *ndev)
{
	struct bcm_cfg80211 * cfg = NULL;
	s32 err = 0;
	s32 ret = 0;
	WL_TRACE(("In\n"));
	if (unlikely(!ndev)) {
		WL_ERR(("ndev is invaild\n"));
		return -ENODEV;
	}
	cfg = g_bcm_cfg;
	if (unlikely(!cfg)) {
		WL_ERR(("cfg is invaild\n"));
		return -EINVAL;
	}
	if (!wl_get_drv_status(cfg, READY, ndev)) {
		if (cfg->wdev) {
			ret = wl_cfgp2p_supported(cfg, ndev);
			if (ret > 0) {
#if !defined(WL_ENABLE_P2P_IF)
				cfg->wdev->wiphy->interface_modes |=
					(BIT(NL80211_IFTYPE_P2P_CLIENT)|
					BIT(NL80211_IFTYPE_P2P_GO));
#endif /* !WL_ENABLE_P2P_IF */
				if ((err = wl_cfgp2p_init_priv(cfg)) != 0)
					goto fail;

#if defined(WL_ENABLE_P2P_IF)
				if (cfg->p2p_net) {
					/* Update MAC addr for p2p0 interface here. */
					memcpy(cfg->p2p_net->dev_addr, ndev->dev_addr, ETH_ALEN);
					cfg->p2p_net->dev_addr[0] |= 0x02;
					WL_ERR(("%s: p2p_dev_addr="MACDBG "\n",
						cfg->p2p_net->name,
						MAC2STRDBG(cfg->p2p_net->dev_addr)));
				} else {
					WL_ERR(("p2p_net not yet populated."
					" Couldn't update the MAC Address for p2p0 \n"));
					return -ENODEV;
				}
#endif /* WL_ENABLE_P2P_IF */
				cfg->p2p_supported = true;
			} else if (ret == 0) {
				if ((err = wl_cfgp2p_init_priv(cfg)) != 0)
					goto fail;
			} else {
				/* SDIO bus timeout */
				err = -ENODEV;
				goto fail;
			}
		}
	}
	wl_set_drv_status(cfg, READY, ndev);
fail:
	return err;
}

s32 wl_cfg80211_attach(struct net_device *ndev, void *context)
{
	struct wireless_dev *wdev;
	struct bcm_cfg80211 *cfg;
	s32 err = 0;
	struct device *dev;

	WL_TRACE(("In\n"));
	if (!ndev) {
		WL_ERR(("ndev is invaild\n"));
		return -ENODEV;
	}
	WL_DBG(("func %p\n", wl_cfg80211_get_parent_dev()));
	dev = wl_cfg80211_get_parent_dev();

	wdev = kzalloc(sizeof(*wdev), GFP_KERNEL);
	if (unlikely(!wdev)) {
		WL_ERR(("Could not allocate wireless device\n"));
		return -ENOMEM;
	}
	err = wl_setup_wiphy(wdev, dev, context);
	if (unlikely(err)) {
		kfree(wdev);
		return -ENOMEM;
	}
	wdev->iftype = wl_mode_to_nl80211_iftype(WL_MODE_BSS);
	cfg = (struct bcm_cfg80211 *)wiphy_priv(wdev->wiphy);
	cfg->wdev = wdev;
	cfg->pub = context;
	INIT_LIST_HEAD(&cfg->net_list);
#ifdef  BRCM_RSDB	
	spin_lock_init(&cfg->net_list_sync);
#endif
	ndev->ieee80211_ptr = wdev;
	SET_NETDEV_DEV(ndev, wiphy_dev(wdev->wiphy));
	wdev->netdev = ndev;
	cfg->state_notifier = wl_notifier_change_state;
#ifndef  BRCM_RSDB
	err = wl_alloc_netinfo(cfg, ndev, wdev, WL_MODE_BSS, PM_ENABLE);
#else
	err = wl_alloc_netinfo(cfg, ndev, wdev, WL_MODE_BSS, PM_ENABLE, 0);
#endif
	if (err) {
		WL_ERR(("Failed to alloc net_info (%d)\n", err));
		goto cfg80211_attach_out;
	}
	err = wl_init_priv(cfg);
	if (err) {
		WL_ERR(("Failed to init iwm_priv (%d)\n", err));
		goto cfg80211_attach_out;
	}

	err = wl_setup_rfkill(cfg, TRUE);
	if (err) {
		WL_ERR(("Failed to setup rfkill %d\n", err));
		goto cfg80211_attach_out;
	}
#ifdef DEBUGFS_CFG80211
	err = wl_setup_debugfs(cfg,ndev);
	if (err) {
		WL_ERR(("Failed to setup debugfs %d\n", err));
		goto cfg80211_attach_out;
	}
#endif
	if (!wl_cfg80211_netdev_notifier_registered) {
		wl_cfg80211_netdev_notifier_registered = TRUE;
		err = register_netdevice_notifier(&wl_cfg80211_netdev_notifier);
		if (err) {
			wl_cfg80211_netdev_notifier_registered = FALSE;
			WL_ERR(("Failed to register notifierl %d\n", err));
			goto cfg80211_attach_out;
		}
	}
#if defined(COEX_DHCP)
	cfg->btcoex_info = wl_cfg80211_btcoex_init(cfg->wdev->netdev);
	if (!cfg->btcoex_info)
		goto cfg80211_attach_out;
#endif
#ifdef  BRCM_RSDB
#if defined(SUPPORT_RANDOM_MAC_SCAN)
	cfg->random_mac_enabled = FALSE;
#endif /* SUPPORT_RANDOM_MAC_SCAN */
#endif
	g_bcm_cfg = cfg;
#ifndef  BRCM_RSDB
#if defined(WL_ENABLE_P2P_IF)
	err = wl_cfg80211_attach_p2p();
	if (err)
		goto cfg80211_attach_out;
#endif
#else
#if defined(WL_ENABLE_P2P_IF) || defined(WL_NEWCFG_PRIVCMD_SUPPORT)
	err = wl_cfg80211_attach_p2p();
	if (err)
		goto cfg80211_attach_out;
#endif /* WL_ENABLE_P2P_IF || WL_NEWCFG_PRIVCMD_SUPPORT */

	INIT_DELAYED_WORK(&cfg->pm_enable_work, wl_cfg80211_work_handler);
	mutex_init(&cfg->pm_sync);
#endif

	return err;

cfg80211_attach_out:
	wl_setup_rfkill(cfg, FALSE);
	wl_free_wdev(cfg);
	return err;
}

void wl_cfg80211_detach(void *para)
{
	struct bcm_cfg80211 *cfg;

	(void)para;
	cfg = g_bcm_cfg;

	WL_TRACE(("In\n"));
#ifndef  BRCM_RSDB
	wl_add_remove_pm_enable_work(cfg, FALSE, WL_HANDLER_DEL);
#else
	wl_add_remove_pm_enable_work(cfg, WL_PM_WORKQ_DEL);
#endif

#if defined(COEX_DHCP)
	wl_cfg80211_btcoex_deinit();
	cfg->btcoex_info = NULL;
#endif

	wl_setup_rfkill(cfg, FALSE);
#ifdef DEBUGFS_CFG80211
	wl_free_debugfs(cfg);
#endif
	if (cfg->p2p_supported) {
		if (timer_pending(&cfg->p2p->listen_timer))
			del_timer_sync(&cfg->p2p->listen_timer);
		wl_cfgp2p_deinit_priv(cfg);
	}

	if (timer_pending(&cfg->scan_timeout))
		del_timer_sync(&cfg->scan_timeout);
#if defined(DHD_LOSSLESS_ROAMING) && defined(BRCM_RSDB)
	if (timer_pending(&cfg->roam_timeout)) {
		del_timer_sync(&cfg->roam_timeout);
	}
#endif /* DHD_LOSSLESS_ROAMING && BRCM_RSDB */
#ifndef  BRCM_RSDB
#if defined(WL_CFG80211_P2P_DEV_IF)
	wl_cfgp2p_del_p2p_disc_if(cfg->p2p_wdev, cfg);
#endif /* WL_CFG80211_P2P_DEV_IF  */
#if defined(WL_ENABLE_P2P_IF)
	wl_cfg80211_detach_p2p();
#endif
#else
#if defined(WL_CFG80211_P2P_DEV_IF)
	if (cfg->p2p_wdev)
		wl_cfgp2p_del_p2p_disc_if(cfg->p2p_wdev, cfg);
#endif /* WL_CFG80211_P2P_DEV_IF  */
#if defined(WL_ENABLE_P2P_IF) || defined(WL_NEWCFG_PRIVCMD_SUPPORT)
	wl_cfg80211_detach_p2p();
#endif /* WL_ENABLE_P2P_IF || WL_NEWCFG_PRIVCMD_SUPPORT */
#endif

	wl_cfg80211_ibss_vsie_free(cfg);
#ifdef  BRCM_RSDB
	wl_cfg80211_clear_mgmt_vndr_ies(cfg);
#endif
	wl_deinit_priv(cfg);
	g_bcm_cfg = NULL;
	wl_cfg80211_clear_parent_dev();
	wl_free_wdev(cfg);
	/* PLEASE do NOT call any function after wl_free_wdev, the driver's private
	 * structure "cfg", which is the private part of wiphy, has been freed in
	 * wl_free_wdev !!!!!!!!!!!
	 */
}

static void wl_wakeup_event(struct bcm_cfg80211 *cfg)
{
#ifndef  BRCM_RSDB
	if (cfg->event_tsk.thr_pid >= 0) {
		DHD_OS_WAKE_LOCK(cfg->pub);
		up(&cfg->event_tsk.sema);
	}
#else
	dhd_pub_t *dhd = (dhd_pub_t *)(cfg->pub);
	if (dhd->up && (cfg->event_tsk.thr_pid >= 0)) {
		up(&cfg->event_tsk.sema);
	}
#endif
}
#ifndef  BRCM_RSDB
static s32 wl_event_handler(void *data)
{
	struct bcm_cfg80211 *cfg = NULL;
	struct wl_event_q *e;
	tsk_ctl_t *tsk = (tsk_ctl_t *)data;
	bcm_struct_cfgdev *cfgdev = NULL;

	cfg = (struct bcm_cfg80211 *)tsk->parent;

	WL_ERR(("tsk Enter, tsk = 0x%p\n", tsk));

	while (down_interruptible (&tsk->sema) == 0) {
		SMP_RD_BARRIER_DEPENDS();
		if (tsk->terminated)
			break;
		while ((e = wl_deq_event(cfg))) {
			WL_DBG(("event type (%d), if idx: %d\n", e->etype, e->emsg.ifidx));
			/* All P2P device address related events comes on primary interface since
			 * there is no corresponding bsscfg for P2P interface. Map it to p2p0
			 * interface.
			 */
#if defined(WL_CFG80211_P2P_DEV_IF)
			if (WL_IS_P2P_DEV_EVENT(e) && (cfg->p2p_wdev)) {
				cfgdev = bcmcfg_to_p2p_wdev(cfg);
			} else {
				struct net_device *ndev = NULL;

				ndev = dhd_idx2net((struct dhd_pub *)(cfg->pub), e->emsg.ifidx);
				if (ndev)
					cfgdev = ndev_to_wdev(ndev);
			}
#elif defined(WL_ENABLE_P2P_IF)
			if (WL_IS_P2P_DEV_EVENT(e) && (cfg->p2p_net)) {
				cfgdev = cfg->p2p_net;
			} else {
				cfgdev = dhd_idx2net((struct dhd_pub *)(cfg->pub),
					e->emsg.ifidx);
			}
#endif /* WL_CFG80211_P2P_DEV_IF */

			if (!cfgdev) {
#if defined(WL_CFG80211_P2P_DEV_IF)
				cfgdev = bcmcfg_to_prmry_wdev(cfg);
#elif defined(WL_ENABLE_P2P_IF)
				cfgdev = bcmcfg_to_prmry_ndev(cfg);
#endif /* WL_CFG80211_P2P_DEV_IF */
			}
			if (e->etype < WLC_E_LAST && cfg->evt_handler[e->etype]) {
				cfg->evt_handler[e->etype] (cfg, cfgdev, &e->emsg, e->edata);
			} else {
				WL_DBG(("Unknown Event (%d): ignoring\n", e->etype));
			}
			wl_put_event(e);
		}
		DHD_OS_WAKE_UNLOCK(cfg->pub);
	}
	WL_ERR(("was terminated\n"));
	complete_and_exit(&tsk->completed, 0);
	return 0;
}
#else
static s32 wl_event_handler(void *data)
{
	struct bcm_cfg80211 *cfg = NULL;
	struct wl_event_q *e;
	tsk_ctl_t *tsk = (tsk_ctl_t *)data;
	struct wireless_dev *wdev = NULL;

	cfg = (struct bcm_cfg80211 *)tsk->parent;

	WL_ERR(("tsk Enter, tsk = 0x%p\n", tsk));

	while (down_interruptible (&tsk->sema) == 0) {
		SMP_RD_BARRIER_DEPENDS();
		if (tsk->terminated) {
			break;
		}
		while ((e = wl_deq_event(cfg))) {
			WL_DBG(("event type (%d), ifidx: %d bssidx: %d \n",
				e->etype, e->emsg.ifidx, e->emsg.bsscfgidx));

			if (e->emsg.ifidx > WL_MAX_IFS) {
				WL_ERR((" Event ifidx not in range. val:%d \n", e->emsg.ifidx));
				goto fail;
			}
			if (!(wdev = wl_get_wdev_by_bssidx(cfg, e->emsg.bsscfgidx))) {
				/* For WLC_E_IF would be handled by wl_host_event */
				if (e->etype != WLC_E_IF)
					WL_ERR(("No wdev corresponding to bssidx: 0x%x found!"
						" Ignoring event.\n", e->emsg.bsscfgidx));
			} else if (e->etype < WLC_E_LAST && cfg->evt_handler[e->etype]) {
				dhd_pub_t *dhd = (struct dhd_pub *)(cfg->pub);
				if (dhd->busstate == DHD_BUS_DOWN) {
					WL_ERR((": BUS is DOWN.\n"));
				} else {
#ifdef DHD_IFDEBUG
					if (cfg->iface_cnt == 0) {
						wl_dump_ifinfo(cfg);
					}
#endif
					cfg->evt_handler[e->etype](cfg, wdev_to_cfgdev(wdev),
						&e->emsg, e->edata);
				}
			} else {
				WL_DBG(("Unknown Event (%d): ignoring\n", e->etype));
			}
fail:
			wl_put_event(e);
#ifdef PCIE_FULL_DONGLE
			DHD_EVENT_WAKE_UNLOCK(cfg->pub);
#endif
		}
	}
	WL_ERR(("was terminated\n"));
	complete_and_exit(&tsk->completed, 0);
	return 0;
}
#endif /* BRCM_RSDB */
void
wl_cfg80211_event(struct net_device *ndev, const wl_event_msg_t * e, void *data)
{
	u32 event_type = ntoh32(e->event_type);
	struct bcm_cfg80211 *cfg = g_bcm_cfg;
#ifdef  BRCM_RSDB
	struct net_info *netinfo;
#endif	
#ifdef HW_P2P_SPINLOCK_PATCH
	unsigned long flags;
#endif

#if (WL_DBG_LEVEL > 0)
	s8 *estr = (event_type <= sizeof(wl_dbg_estr) / WL_DBG_ESTR_MAX - 1) ?
	    wl_dbg_estr[event_type] : (s8 *) "Unknown";
	WL_DBG(("event_type (%d):" "WLC_E_" "%s\n", event_type, estr));
#endif /* (WL_DBG_LEVEL > 0) */
#ifdef  BRCM_RSDB
	if ((cfg == NULL) || (cfg->p2p_supported && cfg->p2p == NULL)) {
		WL_ERR(("Stale event ignored\n"));
		return;
	}

	if (cfg->event_tsk.thr_pid == -1) {
		WL_ERR(("Event handler is not created\n"));
		return;
	}
#endif
#ifdef HW_P2P_SPINLOCK_PATCH
	spin_lock_irqsave(&cfg->p2p_detach_lock, flags);
#endif
	if (wl_get_p2p_status(cfg, IF_CHANGING) || wl_get_p2p_status(cfg, IF_ADDING)) {
#ifdef HW_P2P_SPINLOCK_PATCH
		spin_unlock_irqrestore(&cfg->p2p_detach_lock, flags);
#endif
		WL_ERR(("during IF change, ignore event %d\n", event_type));
		return;
	}
#ifdef HW_P2P_SPINLOCK_PATCH
	spin_unlock_irqrestore(&cfg->p2p_detach_lock, flags);
#endif
#ifndef  BRCM_RSDB
	if (ndev != bcmcfg_to_prmry_ndev(cfg) && cfg->p2p_supported) {
		if (ndev != wl_to_p2p_bss_ndev(cfg, P2PAPI_BSSCFG_CONNECTION) &&
#if defined(WL_ENABLE_P2P_IF)
			(ndev != (cfg->p2p_net ? cfg->p2p_net :
			wl_to_p2p_bss_ndev(cfg, P2PAPI_BSSCFG_DEVICE))) &&
#else
			(ndev != wl_to_p2p_bss_ndev(cfg, P2PAPI_BSSCFG_DEVICE)) &&
#endif /* WL_ENABLE_P2P_IF */
			TRUE) {
			WL_ERR(("ignore event %d, not interested\n", event_type));
			return;
		}
	}
#else
#ifdef DHD_IFDEBUG
	if (event_type != WLC_E_ESCAN_RESULT) {
		WL_ERR(("Event_type %d , status : %d, reason : %d, bssidx:%d \n",
			event_type, ntoh32(e->status), ntoh32(e->reason), e->bsscfgidx));
	}
#endif
	netinfo = wl_get_netinfo_by_bssidx(cfg, e->bsscfgidx);
	if (!netinfo) {
		/* Since the netinfo entry is not there, the netdev entry is not
		 * created via cfg80211 interface. so the event is not of interest
		 * to the cfg80211 layer.
		 */
		WL_ERR(("ignore event %d, not interested\n", event_type));
		return;
	}
#endif

	if (event_type == WLC_E_PFN_NET_FOUND) {
		WL_DBG((" PNOEVENT: PNO_NET_FOUND\n"));
	}
	else if (event_type == WLC_E_PFN_NET_LOST) {
		WL_DBG((" PNOEVENT: PNO_NET_LOST\n"));
	}
#ifdef PCIE_FULL_DONGLE
	DHD_EVENT_WAKE_LOCK(cfg->pub);
	if (likely(!wl_enq_event(cfg, ndev, event_type, e, data))) {
		wl_wakeup_event(cfg);
	} else {
		DHD_EVENT_WAKE_UNLOCK(cfg->pub);
	}
#else
	if (likely(!wl_enq_event(cfg, ndev, event_type, e, data)))
		wl_wakeup_event(cfg);
#endif
}

static void wl_init_eq(struct bcm_cfg80211 *cfg)
{
	wl_init_eq_lock(cfg);
	INIT_LIST_HEAD(&cfg->eq_list);
}

static void wl_flush_eq(struct bcm_cfg80211 *cfg)
{
	struct wl_event_q *e;
	unsigned long flags;

	flags = wl_lock_eq(cfg);
#ifndef  BRCM_RSDB
	while (!list_empty(&cfg->eq_list)) {
		e = list_first_entry(&cfg->eq_list, struct wl_event_q, eq_list);
#else
	while (!list_empty_careful(&cfg->eq_list)) {
		BCM_SET_LIST_FIRST_ENTRY(e, &cfg->eq_list, struct wl_event_q, eq_list);
#endif
		list_del(&e->eq_list);
		kfree(e);
	}
	wl_unlock_eq(cfg, flags);
}

/*
* retrieve first queued event from head
*/

static struct wl_event_q *wl_deq_event(struct bcm_cfg80211 *cfg)
{
	struct wl_event_q *e = NULL;
	unsigned long flags;

	flags = wl_lock_eq(cfg);
	if (likely(!list_empty(&cfg->eq_list))) {
#ifndef  BRCM_RSDB
		e = list_first_entry(&cfg->eq_list, struct wl_event_q, eq_list);
#else
		BCM_SET_LIST_FIRST_ENTRY(e, &cfg->eq_list, struct wl_event_q, eq_list);
#endif
		list_del(&e->eq_list);
	}
	wl_unlock_eq(cfg, flags);

	return e;
}

/*
 * push event to tail of the queue
 */

static s32
wl_enq_event(struct bcm_cfg80211 *cfg, struct net_device *ndev, u32 event,
	const wl_event_msg_t *msg, void *data)
{
	struct wl_event_q *e;
	s32 err = 0;
	uint32 evtq_size;
	uint32 data_len;
	unsigned long flags;
	gfp_t aflags;

	data_len = 0;
	if (data)
		data_len = ntoh32(msg->datalen);
	evtq_size = sizeof(struct wl_event_q) + data_len;
	if (evtq_size < data_len) {
		WL_ERR(("data_len overflow.\n"));
		return -ENOMEM;
	}
	aflags = (in_atomic()) ? GFP_ATOMIC : GFP_KERNEL;
	e = kzalloc(evtq_size, aflags);
	if (unlikely(!e)) {
		WL_ERR(("event alloc failed\n"));
		return -ENOMEM;
	}
	e->etype = event;
	memcpy(&e->emsg, msg, sizeof(wl_event_msg_t));
	if (data)
		memcpy(e->edata, data, data_len);
	flags = wl_lock_eq(cfg);
	list_add_tail(&e->eq_list, &cfg->eq_list);
	wl_unlock_eq(cfg, flags);

	return err;
}

static void wl_put_event(struct wl_event_q *e)
{
	kfree(e);
}

static s32 wl_config_ifmode(struct bcm_cfg80211 *cfg, struct net_device *ndev, s32 iftype)
{
	s32 infra = 0;
	s32 err = 0;
	s32 mode = 0;
	switch (iftype) {
	case NL80211_IFTYPE_MONITOR:
	case NL80211_IFTYPE_WDS:
		WL_ERR(("type (%d) : currently we do not support this mode\n",
			iftype));
		err = -EINVAL;
		return err;
	case NL80211_IFTYPE_ADHOC:
		mode = WL_MODE_IBSS;
		break;
	case NL80211_IFTYPE_STATION:
	case NL80211_IFTYPE_P2P_CLIENT:
		mode = WL_MODE_BSS;
		infra = 1;
		break;
	case NL80211_IFTYPE_AP:
	case NL80211_IFTYPE_P2P_GO:
		mode = WL_MODE_AP;
		infra = 1;
		break;
	default:
		err = -EINVAL;
		WL_ERR(("invalid type (%d)\n", iftype));
		return err;
	}
	infra = htod32(infra);
	err = wldev_ioctl(ndev, WLC_SET_INFRA, &infra, sizeof(infra), true);
	if (unlikely(err)) {
		WL_ERR(("WLC_SET_INFRA error (%d)\n", err));
		return err;
	}

	wl_set_mode_by_netdev(cfg, ndev, mode);

	return 0;
}

void wl_cfg80211_add_to_eventbuffer(struct wl_eventmsg_buf *ev, u16 event, bool set)
{
	if (!ev || (event > WLC_E_LAST))
		return;

	if (ev->num < MAX_EVENT_BUF_NUM) {
		ev->event[ev->num].type = event;
		ev->event[ev->num].set = set;
		ev->num++;
	} else {
		WL_ERR(("evenbuffer doesn't support > %u events. Update"
			" the define MAX_EVENT_BUF_NUM \n", MAX_EVENT_BUF_NUM));
		ASSERT(0);
	}
}

s32 wl_cfg80211_apply_eventbuffer(
	struct net_device *ndev,
	struct bcm_cfg80211 *cfg,
	wl_eventmsg_buf_t *ev)
{
	char eventmask[WL_EVENTING_MASK_LEN];
	int i, ret = 0;
	s8 iovbuf[WL_EVENTING_MASK_LEN + 12];

	if (!ev || (!ev->num))
		return -EINVAL;

	mutex_lock(&cfg->event_sync);

	/* Read event_msgs mask */
	ret = wldev_iovar_getbuf(ndev, "event_msgs", NULL, 0, iovbuf,
				 sizeof(iovbuf), NULL);
	if (unlikely(ret)) {
		WL_ERR(("Get event_msgs error (%d)\n", ret));
		goto exit;
	}
	memcpy(eventmask, iovbuf, WL_EVENTING_MASK_LEN);

	/* apply the set bits */
	for (i = 0; i < ev->num; i++) {
		if (ev->event[i].set)
			setbit(eventmask, ev->event[i].type);
		else
			clrbit(eventmask, ev->event[i].type);
	}

	/* Write updated Event mask */
	ret = wldev_iovar_setbuf(ndev, "event_msgs", eventmask,
				 sizeof(eventmask), iovbuf, sizeof(iovbuf),
				 NULL);
	if (unlikely(ret)) {
		WL_ERR(("Set event_msgs error (%d)\n", ret));
	}

exit:
	mutex_unlock(&cfg->event_sync);
	return ret;
}

s32 wl_add_remove_eventmsg(struct net_device *ndev, u16 event, bool add)
{
	s8 iovbuf[WL_EVENTING_MASK_LEN + 12];
	s8 eventmask[WL_EVENTING_MASK_LEN];
	s32 err = 0;
	struct bcm_cfg80211 *cfg = g_bcm_cfg;

	if (!ndev || !cfg)
		return -ENODEV;

	mutex_lock(&cfg->event_sync);

	/* Setup event_msgs */
	err = wldev_iovar_getbuf(ndev, "event_msgs", NULL, 0, iovbuf,
				 sizeof(iovbuf), NULL);
	if (unlikely(err)) {
		WL_ERR(("Get event_msgs error (%d)\n", err));
		goto eventmsg_out;
	}
	memcpy(eventmask, iovbuf, WL_EVENTING_MASK_LEN);
	if (add) {
		setbit(eventmask, event);
	} else {
		clrbit(eventmask, event);
	}
	err = wldev_iovar_setbuf(ndev, "event_msgs", eventmask,
				 WL_EVENTING_MASK_LEN, iovbuf, sizeof(iovbuf),
				 NULL);
	if (unlikely(err)) {
		WL_ERR(("Set event_msgs error (%d)\n", err));
		goto eventmsg_out;
	}

eventmsg_out:
	mutex_unlock(&cfg->event_sync);
	return err;
}

static int wl_construct_reginfo(struct bcm_cfg80211 *cfg, s32 bw_cap)
{
	struct net_device *dev = bcmcfg_to_prmry_ndev(cfg);
	struct ieee80211_channel *band_chan_arr = NULL;
	wl_uint32_list_t *list;
	u32 i, j, index, n_2g, n_5g, band, channel, array_size;
	u32 *n_cnt = NULL;
	chanspec_t c = 0;
	s32 err = BCME_OK;
	bool update;
	bool ht40_allowed;
	u8 *pbuf = NULL;
	bool dfs_radar_disabled = FALSE;

#define LOCAL_BUF_LEN 1024
	pbuf = kzalloc(LOCAL_BUF_LEN, GFP_KERNEL);

	if (pbuf == NULL) {
		WL_ERR(("failed to allocate local buf\n"));
		return -ENOMEM;
	}
	list = (wl_uint32_list_t *)(void *)pbuf;
	list->count = htod32(WL_NUMCHANSPECS);


	err = wldev_iovar_getbuf_bsscfg(dev, "chanspecs", NULL,
		0, pbuf, LOCAL_BUF_LEN, 0, &cfg->ioctl_buf_sync);
	if (err != 0) {
		WL_ERR(("get chanspecs failed with %d\n", err));
		kfree(pbuf);
		return err;
	}
#undef LOCAL_BUF_LEN

	list = (wl_uint32_list_t *)(void *)pbuf;
	band = array_size = n_2g = n_5g = 0;
	for (i = 0; i < dtoh32(list->count); i++) {
		index = 0;
		update = false;
		ht40_allowed = false;
		c = (chanspec_t)dtoh32(list->element[i]);
		c = wl_chspec_driver_to_host(c);
		channel = wf_chspec_ctlchan(c);

		if (!CHSPEC_IS40(c) && ! CHSPEC_IS20(c)) {
			WL_DBG(("HT80/160/80p80 center channel : %d\n", channel));
			continue;
		}
		if (CHSPEC_IS2G(c) && (channel >= CH_MIN_2G_CHANNEL) &&
			(channel <= CH_MAX_2G_CHANNEL)) {
			band_chan_arr = __wl_2ghz_channels;
			array_size = ARRAYSIZE(__wl_2ghz_channels);
			n_cnt = &n_2g;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0))
			band = NL80211_BAND_2GHZ;
#else
			band = IEEE80211_BAND_2GHZ;
#endif
			ht40_allowed = (bw_cap  == WLC_N_BW_40ALL)? true : false;
		} else if (CHSPEC_IS5G(c) && channel >= CH_MIN_5G_CHANNEL) {
			band_chan_arr = __wl_5ghz_a_channels;
			array_size = ARRAYSIZE(__wl_5ghz_a_channels);
			n_cnt = &n_5g;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0))
			band = NL80211_BAND_5GHZ;
#else
			band = IEEE80211_BAND_5GHZ;
#endif
			ht40_allowed = (bw_cap  == WLC_N_BW_20ALL)? false : true;
		} else {
			WL_ERR(("Invalid channel Sepc. 0x%x.\n", c));
			continue;
		}
		if (!ht40_allowed && CHSPEC_IS40(c))
			continue;
		for (j = 0; (j < *n_cnt && (*n_cnt < array_size)); j++) {
			if (band_chan_arr[j].hw_value == channel) {
				update = true;
				break;
			}
		}
		if (update)
			index = j;
		else
			index = *n_cnt;
		if (index <  array_size) {
#if LINUX_VERSION_CODE == KERNEL_VERSION(2, 6, 38) && !defined(WL_COMPAT_WIRELESS)
			band_chan_arr[index].center_freq =
				ieee80211_channel_to_frequency(channel);
#else
			band_chan_arr[index].center_freq =
				ieee80211_channel_to_frequency(channel, band);
#endif
			band_chan_arr[index].hw_value = channel;

			if (CHSPEC_IS40(c) && ht40_allowed) {
				/* assuming the order is HT20, HT40 Upper,
				 *  HT40 lower from chanspecs
				 */
				u32 ht40_flag = band_chan_arr[index].flags & IEEE80211_CHAN_NO_HT40;
				if (CHSPEC_SB_UPPER(c)) {
					if (ht40_flag == IEEE80211_CHAN_NO_HT40)
						band_chan_arr[index].flags &=
							~IEEE80211_CHAN_NO_HT40;
					band_chan_arr[index].flags |= IEEE80211_CHAN_NO_HT40PLUS;
				} else {
					/* It should be one of
					 * IEEE80211_CHAN_NO_HT40 or IEEE80211_CHAN_NO_HT40PLUS
					 */
					band_chan_arr[index].flags &= ~IEEE80211_CHAN_NO_HT40;
					if (ht40_flag == IEEE80211_CHAN_NO_HT40)
						band_chan_arr[index].flags |=
							IEEE80211_CHAN_NO_HT40MINUS;
				}
			} else {
				band_chan_arr[index].flags = IEEE80211_CHAN_NO_HT40;
				if (!dfs_radar_disabled) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0))
					if (band == NL80211_BAND_2GHZ)
#else
					if (band == IEEE80211_BAND_2GHZ)
#endif
						channel |= WL_CHANSPEC_BAND_2G;
					else
						channel |= WL_CHANSPEC_BAND_5G;
					channel |= WL_CHANSPEC_BW_20;
					channel = wl_chspec_host_to_driver(channel);
					err = wldev_iovar_getint(dev, "per_chan_info", &channel);
					if (!err) {
						if (channel & WL_CHAN_RADAR) {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 14, 0))
							band_chan_arr[index].flags |=
								(IEEE80211_CHAN_RADAR
								| IEEE80211_CHAN_NO_IBSS);
#else
							band_chan_arr[index].flags |=
								IEEE80211_CHAN_RADAR;
#endif
						}

						if (channel & WL_CHAN_PASSIVE)
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 14, 0))
							band_chan_arr[index].flags |=
								IEEE80211_CHAN_PASSIVE_SCAN;
#else
							band_chan_arr[index].flags |=
								IEEE80211_CHAN_NO_IR;
#endif
					} else if (err == BCME_UNSUPPORTED) {
						dfs_radar_disabled = TRUE;
						WL_ERR(("does not support per_chan_info\n"));
					}
				}
			}
			if (!update)
				(*n_cnt)++;
		}

	}
	__wl_band_2ghz.n_channels = n_2g;
	__wl_band_5ghz_a.n_channels = n_5g;
	kfree(pbuf);
	return err;
}

s32 wl_update_wiphybands(struct bcm_cfg80211 *cfg, bool notify)
{
	struct wiphy *wiphy;
	struct net_device *dev;
	u32 bandlist[3];
	u32 nband = 0;
	u32 i = 0;
	s32 err = 0;
	s32 index = 0;
	s32 nmode = 0;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 6, 0)) && defined(BRCM_RSDB)
	u32 j = 0;
	s32 vhtmode = 0;
	s32 txstreams = 0;
	s32 rxstreams = 0;
	s32 ldpc_cap = 0;
	s32 stbc_rx = 0;
	s32 stbc_tx = 0;
	s32 txbf_bfe_cap = 0;
	s32 txbf_bfr_cap = 0;
#endif
	bool rollback_lock = false;
	s32 bw_cap = 0;
	s32 cur_band = -1;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0))
	struct ieee80211_supported_band *bands[NUM_NL80211_BANDS] = {NULL, };
#else
	struct ieee80211_supported_band *bands[IEEE80211_NUM_BANDS] = {NULL, };
#endif

	if (cfg == NULL) {
		cfg = g_bcm_cfg;
		mutex_lock(&cfg->usr_sync);
		rollback_lock = true;
	}
	dev = bcmcfg_to_prmry_ndev(cfg);

	memset(bandlist, 0, sizeof(bandlist));
	err = wldev_ioctl(dev, WLC_GET_BANDLIST, bandlist,
		sizeof(bandlist), false);
	if (unlikely(err)) {
		WL_ERR(("error read bandlist (%d)\n", err));
		goto end_bands;
	}
	err = wldev_ioctl(dev, WLC_GET_BAND, &cur_band,
		sizeof(s32), false);
	if (unlikely(err)) {
		WL_ERR(("error (%d)\n", err));
		goto end_bands;
	}

	err = wldev_iovar_getint(dev, "nmode", &nmode);
	if (unlikely(err)) {
		WL_ERR(("error reading nmode (%d)\n", err));
#ifndef  BRCM_RSDB
	} else {
	/* For nmodeonly  check bw cap */
#else
	}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 6, 0))
	err = wldev_iovar_getint(dev, "vhtmode", &vhtmode);
	if (unlikely(err)) {
		WL_ERR(("error reading vhtmode (%d)\n", err));
	}

	if (vhtmode) {
		err = wldev_iovar_getint(dev, "txstreams", &txstreams);
		if (unlikely(err)) {
			WL_ERR(("error reading txstreams (%d)\n", err));
		}

		err = wldev_iovar_getint(dev, "rxstreams", &rxstreams);
		if (unlikely(err)) {
			WL_ERR(("error reading rxstreams (%d)\n", err));
		}

		err = wldev_iovar_getint(dev, "ldpc_cap", &ldpc_cap);
		if (unlikely(err)) {
			WL_ERR(("error reading ldpc_cap (%d)\n", err));
		}

		err = wldev_iovar_getint(dev, "stbc_rx", &stbc_rx);
		if (unlikely(err)) {
			WL_ERR(("error reading stbc_rx (%d)\n", err));
		}

		err = wldev_iovar_getint(dev, "stbc_tx", &stbc_tx);
		if (unlikely(err)) {
			WL_ERR(("error reading stbc_tx (%d)\n", err));
		}

		err = wldev_iovar_getint(dev, "txbf_bfe_cap", &txbf_bfe_cap);
		if (unlikely(err)) {
			WL_ERR(("error reading txbf_bfe_cap (%d)\n", err));
		}

		err = wldev_iovar_getint(dev, "txbf_bfr_cap", &txbf_bfr_cap);
		if (unlikely(err)) {
			WL_ERR(("error reading txbf_bfr_cap (%d)\n", err));
		}
	}
#endif

	/* For nmode and vhtmode   check bw cap */
	if (nmode ||
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 6, 0))
		vhtmode ||
#endif
		0) {
#endif /* BRCM_RSDB */
		err = wldev_iovar_getint(dev, "mimo_bw_cap", &bw_cap);
		if (unlikely(err)) {
			WL_ERR(("error get mimo_bw_cap (%d)\n", err));
		}
	}

	err = wl_construct_reginfo(cfg, bw_cap);
	if (err) {
		WL_ERR(("wl_construct_reginfo() fails err=%d\n", err));
		if (err != BCME_UNSUPPORTED)
			goto end_bands;
		err = 0;
	}
	wiphy = bcmcfg_to_wiphy(cfg);
	nband = bandlist[0];

	for (i = 1; i <= nband && i < ARRAYSIZE(bandlist); i++) {
		index = -1;
		if (bandlist[i] == WLC_BAND_5G && __wl_band_5ghz_a.n_channels > 0) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0))
			bands[NL80211_BAND_5GHZ] =
				&__wl_band_5ghz_a;
			index = NL80211_BAND_5GHZ;
#else
			bands[IEEE80211_BAND_5GHZ] =
				&__wl_band_5ghz_a;
			index = IEEE80211_BAND_5GHZ;
#endif
#ifndef  BRCM_RSDB
			if (bw_cap == WLC_N_BW_40ALL || bw_cap == WLC_N_BW_20IN2G_40IN5G)
				bands[index]->ht_cap.cap |= IEEE80211_HT_CAP_SGI_40;
#else
			if (nmode && (bw_cap == WLC_N_BW_40ALL || bw_cap == WLC_N_BW_20IN2G_40IN5G))
				bands[index]->ht_cap.cap |= IEEE80211_HT_CAP_SGI_40;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 6, 0))
			/* VHT capabilities. */
			if (vhtmode) {
				/* Supported */
				bands[index]->vht_cap.vht_supported = TRUE;

				for (j = 1; j <= VHT_CAP_MCS_MAP_NSS_MAX; j++) {
					/* TX stream rates. */
					if (j <= txstreams) {
						VHT_MCS_MAP_SET_MCS_PER_SS(j, VHT_CAP_MCS_MAP_0_9,
							bands[index]->vht_cap.vht_mcs.tx_mcs_map);
					} else {
						VHT_MCS_MAP_SET_MCS_PER_SS(j, VHT_CAP_MCS_MAP_NONE,
							bands[index]->vht_cap.vht_mcs.tx_mcs_map);
					}

					/* RX stream rates. */
					if (j <= rxstreams) {
						VHT_MCS_MAP_SET_MCS_PER_SS(j, VHT_CAP_MCS_MAP_0_9,
							bands[index]->vht_cap.vht_mcs.rx_mcs_map);
					} else {
						VHT_MCS_MAP_SET_MCS_PER_SS(j, VHT_CAP_MCS_MAP_NONE,
							bands[index]->vht_cap.vht_mcs.rx_mcs_map);
					}
				}


				/* Capabilities */
				/* 80 MHz is mandatory */
				bands[index]->vht_cap.cap |=
					IEEE80211_VHT_CAP_SHORT_GI_80;

				if (WL_BW_CAP_160MHZ(bw_cap)) {
					bands[index]->vht_cap.cap |=
						IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_160MHZ;
					bands[index]->vht_cap.cap |=
						IEEE80211_VHT_CAP_SHORT_GI_160;
				}

				bands[index]->vht_cap.cap |=
					IEEE80211_VHT_CAP_MAX_MPDU_LENGTH_11454;

				if (ldpc_cap)
					bands[index]->vht_cap.cap |=
						IEEE80211_VHT_CAP_RXLDPC;

				if (stbc_tx)
					bands[index]->vht_cap.cap |=
						IEEE80211_VHT_CAP_TXSTBC;

				if (stbc_rx)
					bands[index]->vht_cap.cap |=
						(stbc_rx << VHT_CAP_INFO_RX_STBC_SHIFT);

				if (txbf_bfe_cap)
					bands[index]->vht_cap.cap |=
						IEEE80211_VHT_CAP_SU_BEAMFORMEE_CAPABLE;

				if (txbf_bfr_cap) {
					bands[index]->vht_cap.cap |=
						IEEE80211_VHT_CAP_SU_BEAMFORMER_CAPABLE;
				}

				if (txbf_bfe_cap || txbf_bfr_cap) {
					bands[index]->vht_cap.cap |=
						(2 << VHT_CAP_INFO_NUM_BMFMR_ANT_SHIFT);
					bands[index]->vht_cap.cap |=
						((txstreams - 1) <<
							VHT_CAP_INFO_NUM_SOUNDING_DIM_SHIFT);
					bands[index]->vht_cap.cap |=
						IEEE80211_VHT_CAP_VHT_LINK_ADAPTATION_VHT_MRQ_MFB;
				}

				/* AMPDU length limit, support max 1MB (2 ^ (13 + 7)) */
				bands[index]->vht_cap.cap |=
					(7 << VHT_CAP_INFO_AMPDU_MAXLEN_EXP_SHIFT);
				WL_INFORM(("%s band[%d] vht_enab=%d vht_cap=%08x "
					"vht_rx_mcs_map=%04x vht_tx_mcs_map=%04x\n",
					__FUNCTION__, index,
					bands[index]->vht_cap.vht_supported,
					bands[index]->vht_cap.cap,
					bands[index]->vht_cap.vht_mcs.rx_mcs_map,
					bands[index]->vht_cap.vht_mcs.tx_mcs_map));
			}
#endif
#endif /* BRCM_RSDB */
		}
		else if (bandlist[i] == WLC_BAND_2G && __wl_band_2ghz.n_channels > 0) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0))
			bands[NL80211_BAND_2GHZ] =
				&__wl_band_2ghz;
			index = NL80211_BAND_2GHZ;
#else
			bands[IEEE80211_BAND_2GHZ] =
				&__wl_band_2ghz;
			index = IEEE80211_BAND_2GHZ;
#endif
			if (bw_cap == WLC_N_BW_40ALL)
				bands[index]->ht_cap.cap |= IEEE80211_HT_CAP_SGI_40;
		}

		if ((index >= 0) && nmode) {
			bands[index]->ht_cap.cap |=
				(IEEE80211_HT_CAP_SGI_20 | IEEE80211_HT_CAP_DSSSCCK40);
			bands[index]->ht_cap.ht_supported = TRUE;
			bands[index]->ht_cap.ampdu_factor = IEEE80211_HT_MAX_AMPDU_64K;
			bands[index]->ht_cap.ampdu_density = IEEE80211_HT_MPDU_DENSITY_16;
			/* An HT shall support all EQM rates for one spatial stream */
			bands[index]->ht_cap.mcs.rx_mask[0] = 0xff;
		}

	}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0))
	wiphy->bands[NL80211_BAND_2GHZ] = bands[NL80211_BAND_2GHZ];
	wiphy->bands[NL80211_BAND_5GHZ] = bands[NL80211_BAND_5GHZ];

	/* check if any bands populated otherwise makes 2Ghz as default */
	if (wiphy->bands[NL80211_BAND_2GHZ] == NULL &&
		wiphy->bands[NL80211_BAND_5GHZ] == NULL) {
		/* Setup 2Ghz band as default */
		wiphy->bands[NL80211_BAND_2GHZ] = &__wl_band_2ghz;
	}
#else
	wiphy->bands[IEEE80211_BAND_2GHZ] = bands[IEEE80211_BAND_2GHZ];
	wiphy->bands[IEEE80211_BAND_5GHZ] = bands[IEEE80211_BAND_5GHZ];

	/* check if any bands populated otherwise makes 2Ghz as default */
	if (wiphy->bands[IEEE80211_BAND_2GHZ] == NULL &&
		wiphy->bands[IEEE80211_BAND_5GHZ] == NULL) {
		/* Setup 2Ghz band as default */
		wiphy->bands[IEEE80211_BAND_2GHZ] = &__wl_band_2ghz;
	}
#endif

	if (notify)
		wiphy_apply_custom_regulatory(wiphy, &brcm_regdom);

	end_bands:
		if (rollback_lock)
			mutex_unlock(&cfg->usr_sync);
	return err;
}

static s32 __wl_cfg80211_up(struct bcm_cfg80211 *cfg)
{
	s32 err = 0;
#if defined(WL_HOST_BAND_MGMT) && defined(BRCM_RSDB)
	s32 ret = 0;
#endif /* WL_HOST_BAND_MGMT && BRCM_RSDB */
	struct net_device *ndev = bcmcfg_to_prmry_ndev(cfg);
	struct wireless_dev *wdev = ndev->ieee80211_ptr;

	WL_DBG(("In\n"));

	err = dhd_config_dongle(cfg);
	if (unlikely(err))
		return err;

	err = wl_config_ifmode(cfg, ndev, wdev->iftype);
	if (unlikely(err && err != -EINPROGRESS)) {
		WL_ERR(("wl_config_ifmode failed\n"));
		if (err == -1) {
			WL_ERR(("return error %d\n", err));
			return err;
		}
	}
	err = wl_update_wiphybands(cfg, true);
	if (unlikely(err)) {
		WL_ERR(("wl_update_wiphybands failed\n"));
		if (err == -1) {
			WL_ERR(("return error %d\n", err));
			return err;
		}
	}
#ifdef  BRCM_RSDB
	err = wl_create_event_handler(cfg);
	if (err) {
		WL_ERR(("wl_create_event_handler failed\n"));
		return err;
	}
	wl_init_event_handler(cfg);

	err = wl_init_scan(cfg);
	if (err) {
		WL_ERR(("wl_init_scan failed\n"));
		return err;
	}
#ifdef DHD_LOSSLESS_ROAMING
	if (timer_pending(&cfg->roam_timeout)) {
		del_timer_sync(&cfg->roam_timeout);
	}
#endif /* DHD_LOSSLESS_ROAMING */
#endif
	err = dhd_monitor_init(cfg->pub);

#if defined(DHCP_SCAN_SUPPRESS)
	/* wlan scan_supp timer and work thread info */
	init_timer(&cfg->scan_supp_timer);
	cfg->scan_supp_timer.data = (ulong)cfg;
	cfg->scan_supp_timer.function = wl_cfg80211_scan_supp_timerfunc;
	INIT_WORK(&cfg->wlan_work, wl_cfg80211_work_handler);
#endif /* DHCP_SCAN_SUPPRESS */
#ifndef  BRCM_RSDB
	INIT_DELAYED_WORK(&cfg->pm_enable_work, wl_cfg80211_work_handler);
#else
#ifdef WL_HOST_BAND_MGMT
	/* By default the curr_band is initialized to BAND_AUTO */
	if ((ret = wl_cfg80211_set_band(ndev, WLC_BAND_AUTO)) < 0) {
		if (ret == BCME_UNSUPPORTED) {
			/* Don't fail the initialization, lets just
			 * fall back to the original method
			 */
			WL_ERR(("WL_HOST_BAND_MGMT defined, "
				"but roam_band iovar not supported \n"));
		} else {
			WL_ERR(("roam_band failed. ret=%d", ret));
			err = -1;
		}
	}
#endif /* WL_HOST_BAND_MGMT */
#if defined(CUSTOMER_HW4) && defined(WES_SUPPORT)
	/* Reset WES mode to 0 */
	wes_mode = 0;
#endif
#endif /* BRCM_RSDB */
	wl_set_drv_status(cfg, READY, ndev);
	return err;
}
#ifndef  BRCM_RSDB
static s32 __wl_cfg80211_down(struct bcm_cfg80211 *cfg)
{
	s32 err = 0;
	unsigned long flags;
	struct net_info *iter, *next;
	struct net_device *ndev = bcmcfg_to_prmry_ndev(cfg);
#if defined(WL_CFG80211) && defined(WL_ENABLE_P2P_IF)
	struct net_device *p2p_net = cfg->p2p_net;
#endif
	u32 bssidx = 0;
#ifdef PROP_TXSTATUS_VSDB
#if defined(BCMSDIO)
	dhd_pub_t *dhd =  (dhd_pub_t *)(cfg->pub);
#endif
#endif /* PROP_TXSTATUS_VSDB */
	WL_DBG(("In\n"));
	/* Delete pm_enable_work */
	wl_add_remove_pm_enable_work(cfg, FALSE, WL_HANDLER_DEL);

	if (cfg->p2p_supported) {
		wl_clr_p2p_status(cfg, GO_NEG_PHASE);
#ifdef PROP_TXSTATUS_VSDB
#if defined(BCMSDIO)
		if (cfg->p2p->vif_created) {
			bool enabled = false;
			dhd_wlfc_get_enable(dhd, &enabled);
			if (enabled && cfg->wlfc_on && dhd->op_mode != DHD_FLAG_HOSTAP_MODE &&
				dhd->op_mode != DHD_FLAG_IBSS_MODE) {
				dhd_wlfc_deinit(dhd);
				cfg->wlfc_on = false;
			}
		}
#endif
#endif /* PROP_TXSTATUS_VSDB */
	}

#if defined(DHCP_SCAN_SUPPRESS)
	/* Force clear of scan_suppress */
	if (cfg->scan_suppressed)
		wl_cfg80211_scan_suppress(ndev, 0);
	if (timer_pending(&cfg->scan_supp_timer))
		del_timer_sync(&cfg->scan_supp_timer);
	cancel_work_sync(&cfg->wlan_work);
#endif /* DHCP_SCAN_SUPPRESS */

	/* If primary BSS is operational (for e.g SoftAP), bring it down */
	if (!(wl_cfgp2p_find_idx(cfg, ndev, &bssidx)) &&
		wl_cfgp2p_bss_isup(ndev, bssidx)) {
		if (wl_cfgp2p_bss(cfg, ndev, bssidx, 0) < 0)
			WL_ERR(("BSS down failed \n"));
	}

	/* Check if cfg80211 interface is already down */
	if (!wl_get_drv_status(cfg, READY, ndev))
		return err;	/* it is even not ready */
	for_each_ndev(cfg, iter, next)
		wl_set_drv_status(cfg, SCAN_ABORTING, iter->ndev);


	spin_lock_irqsave(&cfg->cfgdrv_lock, flags);
	if (cfg->scan_request) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 8, 0))
		struct cfg80211_scan_info info = {
			.aborted = true,
		};
		cfg80211_scan_done(cfg->scan_request, &info);
#else
		cfg80211_scan_done(cfg->scan_request, true);
#endif
		cfg->scan_request = NULL;
	}
	spin_unlock_irqrestore(&cfg->cfgdrv_lock, flags);

	for_each_ndev(cfg, iter, next) {
		wl_clr_drv_status(cfg, READY, iter->ndev);
		wl_clr_drv_status(cfg, SCANNING, iter->ndev);
		wl_clr_drv_status(cfg, SCAN_ABORTING, iter->ndev);
		wl_clr_drv_status(cfg, CONNECTING, iter->ndev);
		wl_clr_drv_status(cfg, CONNECTED, iter->ndev);
		wl_clr_drv_status(cfg, DISCONNECTING, iter->ndev);
		wl_clr_drv_status(cfg, AP_CREATED, iter->ndev);
		wl_clr_drv_status(cfg, AP_CREATING, iter->ndev);
	}
	bcmcfg_to_prmry_ndev(cfg)->ieee80211_ptr->iftype =
		NL80211_IFTYPE_STATION;
#if defined(WL_CFG80211) && defined(WL_ENABLE_P2P_IF)
		if (p2p_net)
			dev_close(p2p_net);
#endif
	wl_flush_eq(cfg);
	wl_link_down(cfg);
	if (cfg->p2p_supported)
		wl_cfgp2p_down(cfg);
	if (cfg->ap_info) {
		kfree(cfg->ap_info->wpa_ie);
		kfree(cfg->ap_info->rsn_ie);
		kfree(cfg->ap_info->wps_ie);
		kfree(cfg->ap_info);
		cfg->ap_info = NULL;
	}
	dhd_monitor_uninit();

#if defined(DUAL_STA) || defined(DUAL_STA_STATIC_IF)
	/* Clean up if not removed already */
	if (cfg->bss_cfgdev)
		wl_cfg80211_del_iface(cfg->wdev->wiphy, cfg->bss_cfgdev);
#endif /* defined (DUAL_STA) || defined (DUAL_STA_STATIC_IF) */

	DNGL_FUNC(dhd_cfg80211_down, (cfg));

	return err;
}
#else
static s32 __wl_cfg80211_down(struct bcm_cfg80211 *cfg)
{
	s32 err = 0;
	unsigned long flags;
	struct net_info *iter, *next;
	struct net_device *ndev = bcmcfg_to_prmry_ndev(cfg);
#if defined(WL_CFG80211) && (defined(WL_ENABLE_P2P_IF)|| \
	defined(WL_NEWCFG_PRIVCMD_SUPPORT))
	struct net_device *p2p_net = cfg->p2p_net;
#endif /* WL_CFG80211 && (WL_ENABLE_P2P_IF || WL_NEWCFG_PRIVCMD_SUPPORT) */
#ifdef PROP_TXSTATUS_VSDB
#if defined(BCMSDIO)
	dhd_pub_t *dhd =  (dhd_pub_t *)(cfg->pub);
#endif
#endif /* PROP_TXSTATUS_VSDB */
	WL_DBG(("In\n"));
	/* Delete pm_enable_work */
	wl_add_remove_pm_enable_work(cfg, WL_PM_WORKQ_DEL);

#ifdef WL_NAN
	wl_cfgnan_stop_handler(ndev, g_bcm_cfg, NULL, 0, NULL);
#endif /* WL_NAN */

	if (cfg->p2p_supported) {
		wl_clr_p2p_status(cfg, GO_NEG_PHASE);
#ifdef PROP_TXSTATUS_VSDB
#if defined(BCMSDIO)
		if (wl_cfgp2p_vif_created(cfg)) {
			bool enabled = false;
			dhd_wlfc_get_enable(dhd, &enabled);
			if (enabled && cfg->wlfc_on && dhd->op_mode != DHD_FLAG_HOSTAP_MODE &&
				dhd->op_mode != DHD_FLAG_IBSS_MODE) {
				dhd_wlfc_deinit(dhd);
				cfg->wlfc_on = false;
			}
		}
#endif
#endif /* PROP_TXSTATUS_VSDB */
	}


	/* If primary BSS is operational (for e.g SoftAP), bring it down */
	if (wl_cfgp2p_bss_isup(ndev, 0)) {
		if (wl_cfgp2p_bss(cfg, ndev, 0, 0) < 0)
			WL_ERR(("BSS down failed \n"));
	}

	/* Check if cfg80211 interface is already down */
	if (!wl_get_drv_status(cfg, READY, ndev))
		return err;	/* it is even not ready */

	/* clear all the security setting on primary Interface */
	wl_cfg80211_clear_security(cfg);

#if defined(STRICT_GCC_WARNINGS) && defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == \
	4 && __GNUC_MINOR__ >= 6))
_Pragma("GCC diagnostic push")
_Pragma("GCC diagnostic ignored \"-Wcast-qual\"")
#endif
	for_each_ndev(cfg, iter, next) {
		if (iter->ndev) /* p2p discovery iface is null */
			wl_set_drv_status(cfg, SCAN_ABORTING, iter->ndev);
	}
#if defined(STRICT_GCC_WARNINGS) && defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == \
	4 && __GNUC_MINOR__ >= 6))
_Pragma("GCC diagnostic pop")
#endif

#ifdef P2P_LISTEN_OFFLOADING
	wl_cfg80211_p2plo_deinit(cfg);
#endif /* P2P_LISTEN_OFFLOADING */

	spin_lock_irqsave(&cfg->cfgdrv_lock, flags);
	if (cfg->scan_request) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 8, 0))
		struct cfg80211_scan_info info = {
			.aborted = true,
		};
		cfg80211_scan_done(cfg->scan_request, &info);
#else
		cfg80211_scan_done(cfg->scan_request, true);
#endif
		cfg->scan_request = NULL;
	}
	spin_unlock_irqrestore(&cfg->cfgdrv_lock, flags);
#if defined(STRICT_GCC_WARNINGS) && defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == \
	4 && __GNUC_MINOR__ >= 6))
_Pragma("GCC diagnostic push")
_Pragma("GCC diagnostic ignored \"-Wcast-qual\"")
#endif
	for_each_ndev(cfg, iter, next) {
		/* p2p discovery iface ndev ptr could be null */
		if (iter->ndev == NULL)
			continue;
		wl_clr_drv_status(cfg, READY, iter->ndev);
		wl_clr_drv_status(cfg, SCANNING, iter->ndev);
		wl_clr_drv_status(cfg, SCAN_ABORTING, iter->ndev);
		wl_clr_drv_status(cfg, CONNECTING, iter->ndev);
		wl_clr_drv_status(cfg, CONNECTED, iter->ndev);
		wl_clr_drv_status(cfg, DISCONNECTING, iter->ndev);
		wl_clr_drv_status(cfg, AP_CREATED, iter->ndev);
		wl_clr_drv_status(cfg, AP_CREATING, iter->ndev);
	}
#if defined(STRICT_GCC_WARNINGS) && defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == \
	4 && __GNUC_MINOR__ >= 6))
_Pragma("GCC diagnostic pop")
#endif
	bcmcfg_to_prmry_ndev(cfg)->ieee80211_ptr->iftype =
		NL80211_IFTYPE_STATION;
#if defined(WL_CFG80211) && (defined(WL_ENABLE_P2P_IF)|| \
	defined(WL_NEWCFG_PRIVCMD_SUPPORT))
#ifdef SUPPORT_DEEP_SLEEP
	if (!trigger_deep_sleep)
#endif /* SUPPORT_DEEP_SLEEP */
		if (p2p_net)
			dev_close(p2p_net);
#endif /* WL_CFG80211 && (WL_ENABLE_P2P_IF || WL_NEWCFG_PRIVCMD_SUPPORT) */

	/* Avoid deadlock from wl_cfg80211_down */
	mutex_unlock(&cfg->usr_sync);
	wl_destroy_event_handler(cfg);
	mutex_lock(&cfg->usr_sync);
	wl_flush_eq(cfg);
	wl_link_down(cfg);
	if (cfg->p2p_supported) {
		if (timer_pending(&cfg->p2p->listen_timer))
			del_timer_sync(&cfg->p2p->listen_timer);
		wl_cfgp2p_down(cfg);
	}

	if (timer_pending(&cfg->scan_timeout)) {
		del_timer_sync(&cfg->scan_timeout);
	}

	DHD_OS_SCAN_WAKE_UNLOCK((dhd_pub_t *)(cfg->pub));

	dhd_monitor_uninit();
#ifdef WLAIBSS_MCHAN
	bcm_cfg80211_del_ibss_if(cfg->wdev->wiphy, cfg->ibss_cfgdev);
#endif /* WLAIBSS_MCHAN */

#if defined(WL_VIRTUAL_APSTA) || defined(DUAL_STA_STATIC_IF)
	/* Clean up if not removed already */
	if (cfg->bss_cfgdev)
		wl_cfg80211_del_iface(cfg->wdev->wiphy, cfg->bss_cfgdev);
#endif /* defined (WL_VIRTUAL_APSTA) || defined (DUAL_STA_STATIC_IF) */

#ifdef WL11U
	/* Clear interworking element. */
	if (cfg->wl11u) {
		cfg->wl11u = FALSE;
		cfg->iw_ie_len = 0;
		memset(cfg->iw_ie, 0, IW_IES_MAX_BUF_LEN);
	}
#endif /* WL11U */

#ifdef CUSTOMER_HW4_DEBUG
	if (wl_scan_timeout_dbg_enabled)
		wl_scan_timeout_dbg_clear();
#endif /* CUSTOMER_HW4_DEBUG */

	cfg->disable_roam_event = false;

	DNGL_FUNC(dhd_cfg80211_down, (cfg));

#ifdef DHD_IFDEBUG
	/* Printout all netinfo entries */
	wl_probe_wdev_all(cfg);
#endif /* DHD_IFDEBUG */

	return err;
}
#endif /* BRCM_RSDB */

s32 wl_cfg80211_up(void *para)
{
	struct bcm_cfg80211 *cfg;
	s32 err = 0;
	int val = 1;
	dhd_pub_t *dhd;
#if defined(DISABLE_PM_BCNRX) && defined(BRCM_RSDB)
	s32 interr = 0;
	uint param = 0;
	s8 iovbuf[WLC_IOCTL_SMLEN];
#endif /* DISABLE_PM_BCNRX && BRCM_RSDB*/

	(void)para;
	WL_DBG(("In\n"));
	cfg = g_bcm_cfg;

	if ((err = wldev_ioctl(bcmcfg_to_prmry_ndev(cfg), WLC_GET_VERSION, &val,
		sizeof(int), false) < 0)) {
		WL_ERR(("WLC_GET_VERSION failed, err=%d\n", err));
		return err;
	}
	val = dtoh32(val);
	if (val != WLC_IOCTL_VERSION && val != 1) {
		WL_ERR(("Version mismatch, please upgrade. Got %d, expected %d or 1\n",
			val, WLC_IOCTL_VERSION));
#ifdef HW_WIFI_DMD_LOG
		hw_wifi_dsm_client_notify(DSM_WIFI_DHD_DEV_INIT_IOCTL_ERROR,
			"WLC_IOCTL_VERSION Version mismatch failed\n");
#endif
		return BCME_VERSION;
	}
	ioctl_version = val;
	WL_TRACE(("WLC_GET_VERSION=%d\n", ioctl_version));

	mutex_lock(&cfg->usr_sync);
	dhd = (dhd_pub_t *)(cfg->pub);
	if (!(dhd->op_mode & DHD_FLAG_HOSTAP_MODE)) {
		err = wl_cfg80211_attach_post(bcmcfg_to_prmry_ndev(cfg));
		if (unlikely(err)) {
			mutex_unlock(&cfg->usr_sync);
			return err;
		}
	}
#ifdef  BRCM_RSDB
#if defined(BCMSUP_4WAY_HANDSHAKE) && defined(WLAN_AKM_SUITE_FT_8021X)
	if (dhd->fw_4way_handshake)
		cfg->wdev->wiphy->features |= NL80211_FEATURE_FW_4WAY_HANDSHAKE;
#endif
#endif
	err = __wl_cfg80211_up(cfg);
	if (unlikely(err))
		WL_ERR(("__wl_cfg80211_up failed\n"));
#ifdef ROAM_CHANNEL_CACHE
#ifndef  BRCM_RSDB
	init_roam(ioctl_version);
#else
	if (init_roam_cache(cfg, ioctl_version) == 0) {
		/* Enable support for Roam cache */
		cfg->rcc_enabled = true;
		WL_ERR(("Roam channel cache enabled\n"));
	} else {
		WL_ERR(("Failed to enable RCC.\n"));
	}
#endif /*BRCM_RSDB*/
#endif
#ifdef  BRCM_RSDB
#if defined(FORCE_DISABLE_SINGLECORE_SCAN)
	dhd_force_disable_singlcore_scan(dhd);
#endif /* FORCE_DISABLE_SINGLECORE_SCAN */

	/* IOVAR configurations with 'up' condition */
#ifdef DISABLE_PM_BCNRX
	bcm_mkiovar("pm_bcnrx", (char *)&param, 4, iovbuf, sizeof(iovbuf));
	interr = wldev_ioctl(bcmcfg_to_prmry_ndev(cfg), WLC_SET_VAR, iovbuf, sizeof(iovbuf), true);
	if (unlikely(interr))
		WL_ERR(("Set pm_bcnrx returned (%d)\n", interr));
#endif /* DISABLE_PM_BCNRX */
#endif
	mutex_unlock(&cfg->usr_sync);

#if defined(WLAIBSS_MCHAN) && defined(BRCM_RSDB)
	bcm_cfg80211_add_ibss_if(cfg->wdev->wiphy, IBSS_IF_NAME);
#endif /* WLAIBSS_MCHAN */

#ifdef DUAL_STA_STATIC_IF
#ifdef WL_VIRTUAL_APSTA
#error "Both DUAL STA and DUAL_STA_STATIC_IF can't be enabled together"
#endif
	/* Static Interface support is currently supported only for STA only builds (without P2P) */
	wl_cfg80211_create_iface(cfg->wdev->wiphy, NL80211_IFTYPE_STATION, NULL, "wlan%d");
#endif /* DUAL_STA_STATIC_IF */

	return err;
}

/* Private Event to Supplicant with indication that chip hangs */
int wl_cfg80211_hang(struct net_device *dev, u16 reason)
{
	struct bcm_cfg80211 *cfg;
#ifdef  BRCM_RSDB
	dhd_pub_t *dhd;
#if defined(SOFTAP_SEND_HANGEVT)
	/* specifc mac address used for hang event */
	uint8 hang_mac[ETHER_ADDR_LEN] = {0x11, 0x11, 0x11, 0x11, 0x11, 0x11};
#endif /* SOFTAP_SEND_HANGEVT */
	if (!g_bcm_cfg) {
		return BCME_ERROR;
	}
#endif
	cfg = g_bcm_cfg;

#ifndef  BRCM_RSDB
	WL_ERR(("In : chip crash eventing\n"));
	wl_add_remove_pm_enable_work(cfg, FALSE, WL_HANDLER_DEL);
	CFG80211_DISCONNECTED(dev, reason, NULL, 0, true, GFP_KERNEL);
#else
	dhd = (dhd_pub_t *)(cfg->pub);

#ifdef DHD_USE_EXTENDED_HANG_REASON
	if (dhd->hang_reason != 0) {
		reason = dhd->hang_reason;
	}
#endif /* DHD_USE_EXTENDED_HANG_REASON */

	WL_ERR(("In : chip crash eventing, reason=0x%x\n", (uint32)(dhd->hang_reason)));
	wl_add_remove_pm_enable_work(cfg, WL_PM_WORKQ_DEL);
#if defined(SOFTAP_SEND_HANGEVT)
	if (dhd->op_mode & DHD_FLAG_HOSTAP_MODE) {
		cfg80211_del_sta(dev, hang_mac, GFP_ATOMIC);
	} else
#endif /* SOFTAP_SEND_HANGEVT */
	{
		CFG80211_DISCONNECTED(dev, reason, NULL, 0, false, GFP_KERNEL);
	}
#endif /* BRCM_RSDB */
	if (cfg != NULL) {
		wl_link_down(cfg);
	}
	return 0;
}

s32 wl_cfg80211_down(void *para)
{
	struct bcm_cfg80211 *cfg;
	s32 err = 0;

	(void)para;
	WL_DBG(("In\n"));
	cfg = g_bcm_cfg;
	mutex_lock(&cfg->usr_sync);
	err = __wl_cfg80211_down(cfg);
	mutex_unlock(&cfg->usr_sync);

	return err;
}

static void *wl_read_prof(struct bcm_cfg80211 *cfg, struct net_device *ndev, s32 item)
{
	unsigned long flags;
	void *rptr = NULL;
	struct wl_profile *profile = wl_get_profile_by_netdev(cfg, ndev);

	if (!profile)
		return NULL;
	spin_lock_irqsave(&cfg->cfgdrv_lock, flags);
	switch (item) {
	case WL_PROF_SEC:
		rptr = &profile->sec;
		break;
	case WL_PROF_ACT:
		rptr = &profile->active;
		break;
	case WL_PROF_BSSID:
		rptr = profile->bssid;
		break;
	case WL_PROF_SSID:
		rptr = &profile->ssid;
		break;
	case WL_PROF_CHAN:
		rptr = &profile->channel;
		break;
	}
	spin_unlock_irqrestore(&cfg->cfgdrv_lock, flags);
	if (!rptr)
		WL_ERR(("invalid item (%d)\n", item));
	return rptr;
}
#ifndef  BRCM_RSDB
static s32
wl_update_prof(struct bcm_cfg80211 *cfg, struct net_device *ndev,
	const wl_event_msg_t *e, void *data, s32 item)
#else
static s32
wl_update_prof(struct bcm_cfg80211 *cfg, struct net_device *ndev,
	const wl_event_msg_t *e, const void *data, s32 item)
#endif
{
	s32 err = 0;
	struct wlc_ssid *ssid;
	unsigned long flags;
	struct wl_profile *profile = wl_get_profile_by_netdev(cfg, ndev);

	if (!profile)
		return WL_INVALID;
	spin_lock_irqsave(&cfg->cfgdrv_lock, flags);
	switch (item) {
	case WL_PROF_SSID:
#ifndef  BRCM_RSDB
		ssid = (wlc_ssid_t *) data;
#else
		ssid = (wlc_ssid_t *) data;
#endif
		memset(profile->ssid.SSID, 0,
			sizeof(profile->ssid.SSID));
#ifdef BCM_PATCH_CVE_2016_2493
		profile->ssid.SSID_len = MIN(ssid->SSID_len, DOT11_MAX_SSID_LEN);
		memcpy(profile->ssid.SSID, ssid->SSID, profile->ssid.SSID_len);
#else
		memcpy(profile->ssid.SSID, ssid->SSID, ssid->SSID_len);
		profile->ssid.SSID_len = ssid->SSID_len;
#endif
		break;
	case WL_PROF_BSSID:
		if (data)
			memcpy(profile->bssid, data, ETHER_ADDR_LEN);
		else
			memset(profile->bssid, 0, ETHER_ADDR_LEN);
		break;
	case WL_PROF_SEC:
		memcpy(&profile->sec, data, sizeof(profile->sec));
		break;
	case WL_PROF_ACT:
#ifndef  BRCM_RSDB
		profile->active = *(bool *)data;
#else
		profile->active = *(const bool *)data;
#endif
		break;
	case WL_PROF_BEACONINT:
#ifndef  BRCM_RSDB
		profile->beacon_interval = *(u16 *)data;
#else
		profile->beacon_interval = *(const u16 *)data;
#endif
		break;
	case WL_PROF_DTIMPERIOD:
#ifndef  BRCM_RSDB
		profile->dtim_period = *(u8 *)data;
#else
		profile->dtim_period = *(const u8 *)data;
#endif
		break;
	case WL_PROF_CHAN:
#ifndef  BRCM_RSDB
		profile->channel = *(u32*)data;
#else
		profile->channel = *(const u32*)data;
#endif
		break;
	default:
		err = -EOPNOTSUPP;
		break;
	}
	spin_unlock_irqrestore(&cfg->cfgdrv_lock, flags);

	if (err == -EOPNOTSUPP)
		WL_ERR(("unsupported item (%d)\n", item));

	return err;
}

void wl_cfg80211_dbg_level(u32 level)
{
	/*
	* prohibit to change debug level
	* by insmod parameter.
	* eventually debug level will be configured
	* in compile time by using CONFIG_XXX
	*/
	/* wl_dbg_level = level; */
}

static bool wl_is_ibssmode(struct bcm_cfg80211 *cfg, struct net_device *ndev)
{
	return wl_get_mode_by_netdev(cfg, ndev) == WL_MODE_IBSS;
}

static __used bool wl_is_ibssstarter(struct bcm_cfg80211 *cfg)
{
	return cfg->ibss_starter;
}

static void wl_rst_ie(struct bcm_cfg80211 *cfg)
{
	struct wl_ie *ie = wl_to_ie(cfg);

	ie->offset = 0;
}

static __used s32 wl_add_ie(struct bcm_cfg80211 *cfg, u8 t, u8 l, u8 *v)
{
	struct wl_ie *ie = wl_to_ie(cfg);
	s32 err = 0;

	if (unlikely(ie->offset + l + 2 > WL_TLV_INFO_MAX)) {
		WL_ERR(("ei crosses buffer boundary\n"));
		return -ENOSPC;
	}
	ie->buf[ie->offset] = t;
	ie->buf[ie->offset + 1] = l;
	memcpy(&ie->buf[ie->offset + 2], v, l);
	ie->offset += l + 2;

	return err;
}
#ifndef  BRCM_RSDB
static void wl_update_hidden_ap_ie(struct wl_bss_info *bi, u8 *ie_stream, u32 *ie_size, bool roam)
#else
static void wl_update_hidden_ap_ie(struct wl_bss_info *bi, const u8 *ie_stream, u32 *ie_size,
	bool roam)
#endif
{
	u8 *ssidie;
#ifdef BCM_PATCH_CVE_2016_2493
	int32 ssid_len = min(bi->SSID_len, (uint8)DOT11_MAX_SSID_LEN);
	int32 remaining_ie_buf_len, available_buffer_len;
#endif
#ifdef  BRCM_RSDB
	/* cfg80211_find_ie defined in kernel returning const u8 */
#if defined(STRICT_GCC_WARNINGS) && defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == \
	4 && __GNUC_MINOR__ >= 6))
_Pragma("GCC diagnostic push")
_Pragma("GCC diagnostic ignored \"-Wcast-qual\"")
#endif
#endif /* BRCM_RSDB */
	ssidie = (u8 *)cfg80211_find_ie(WLAN_EID_SSID, ie_stream, *ie_size);
#ifdef  BRCM_RSDB
#if defined(STRICT_GCC_WARNINGS) && defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == \
	4 && __GNUC_MINOR__ >= 6))
_Pragma("GCC diagnostic pop")
#endif
#endif  /* BRCM_RSDB */
#ifdef BCM_PATCH_CVE_2016_2493
	/* ERROR out if
	 * 1. No ssid IE is FOUND or
	 * 2. New ssid length is > what was allocated for existing ssid (as
	 * we do not want to overwrite the rest of the IEs) or
	 * 3. If in case of erroneous buffer input where ssid length doesnt match the space
	 * allocated to it.
	 */
#endif
	if (!ssidie)
		return;
#ifdef BCM_PATCH_CVE_2016_2493
	if (ssid_len != ssidie[1]) {
		if (WL_EXTRA_BUF_MAX < *ie_size + (ssid_len - ssidie[1]) + 4) {
			return;
		}
	}
	available_buffer_len = ((int)(*ie_size)) - (ssidie + 2 - ie_stream);
	remaining_ie_buf_len = available_buffer_len - (int)ssidie[1];
	if (ssidie[1] > available_buffer_len) {
		return;
		}

#endif

#ifdef BCM_PATCH_CVE_2016_2493
	if (ssidie[1] != ssid_len) {
#else
	if (ssidie[1] != bi->SSID_len) {
#endif
		if (ssidie[1]) {
			WL_ERR(("%s: Wrong SSID len: %d != %d\n",
				__FUNCTION__, ssidie[1], bi->SSID_len));
		}
		if (roam) {
			WL_ERR(("Changing the SSID Info.\n"));
#ifdef BCM_PATCH_CVE_2016_2493
			memmove(ssidie + ssid_len + 2,
#else
			memmove(ssidie + bi->SSID_len + 2,
#endif
				(ssidie + 2) + ssidie[1],
#ifdef BCM_PATCH_CVE_2016_2493
				remaining_ie_buf_len);
			memcpy(ssidie + 2, bi->SSID, ssid_len);
			*ie_size = *ie_size + ssid_len - ssidie[1];
			ssidie[1] = ssid_len;
#else
				*ie_size - (ssidie + 2 + ssidie[1] - ie_stream));
			memcpy(ssidie + 2, bi->SSID, bi->SSID_len);
			*ie_size = *ie_size + bi->SSID_len - ssidie[1];
			ssidie[1] = bi->SSID_len;
#endif
		}
		return;
	}
	if (*(ssidie + 2) == '\0')
#ifdef BCM_PATCH_CVE_2016_2493
		memcpy(ssidie + 2, bi->SSID, ssid_len);
#else
		 memcpy(ssidie + 2, bi->SSID, bi->SSID_len);
#endif
	return;
}

static s32 wl_mrg_ie(struct bcm_cfg80211 *cfg, u8 *ie_stream, u16 ie_size)
{
	struct wl_ie *ie = wl_to_ie(cfg);
	s32 err = 0;

	if (unlikely(ie->offset + ie_size > WL_TLV_INFO_MAX)) {
		WL_ERR(("ei_stream crosses buffer boundary\n"));
		return -ENOSPC;
	}
	memcpy(&ie->buf[ie->offset], ie_stream, ie_size);
	ie->offset += ie_size;

	return err;
}

static s32 wl_cp_ie(struct bcm_cfg80211 *cfg, u8 *dst, u16 dst_size)
{
	struct wl_ie *ie = wl_to_ie(cfg);
	s32 err = 0;

	if (unlikely(ie->offset > dst_size)) {
		WL_ERR(("dst_size is not enough\n"));
		return -ENOSPC;
	}
	memcpy(dst, &ie->buf[0], ie->offset);

	return err;
}

static u32 wl_get_ielen(struct bcm_cfg80211 *cfg)
{
	struct wl_ie *ie = wl_to_ie(cfg);

	return ie->offset;
}

static void wl_link_up(struct bcm_cfg80211 *cfg)
{
	cfg->link_up = true;
}

static void wl_link_down(struct bcm_cfg80211 *cfg)
{
	struct wl_connect_info *conn_info = wl_to_conn(cfg);

	WL_DBG(("In\n"));
	cfg->link_up = false;
	conn_info->req_ie_len = 0;
	conn_info->resp_ie_len = 0;
}

static unsigned long wl_lock_eq(struct bcm_cfg80211 *cfg)
{
	unsigned long flags;

	spin_lock_irqsave(&cfg->eq_lock, flags);
	return flags;
}

static void wl_unlock_eq(struct bcm_cfg80211 *cfg, unsigned long flags)
{
	spin_unlock_irqrestore(&cfg->eq_lock, flags);
}

static void wl_init_eq_lock(struct bcm_cfg80211 *cfg)
{
	spin_lock_init(&cfg->eq_lock);
}

static void wl_delay(u32 ms)
{
	if (in_atomic() || (ms < jiffies_to_msecs(1))) {
		OSL_DELAY(ms*1000);
	} else {
		OSL_SLEEP(ms);
	}
}

s32 wl_cfg80211_get_p2p_dev_addr(struct net_device *net, struct ether_addr *p2pdev_addr)
{
	struct bcm_cfg80211 *cfg = g_bcm_cfg;
#ifndef  BRCM_RSDB
	struct ether_addr p2pif_addr;
	struct ether_addr primary_mac;
	if (!cfg->p2p)
		return -1;
	if (!p2p_is_on(cfg)) {
		get_primary_mac(cfg, &primary_mac);
		wl_cfgp2p_generate_bss_mac(&primary_mac, p2pdev_addr, &p2pif_addr);
	} else {
		memcpy(p2pdev_addr->octet,
			cfg->p2p->dev_addr.octet, ETHER_ADDR_LEN);
	}
#else
	struct ether_addr primary_mac;
	if (!cfg->p2p)
		return -1;
	if (!p2p_is_on(cfg)) {
		get_primary_mac(cfg, &primary_mac);
		wl_cfgp2p_generate_bss_mac(cfg, &primary_mac);
	} else {
		memcpy(p2pdev_addr->octet, wl_to_p2p_bss_macaddr(cfg, P2PAPI_BSSCFG_DEVICE).octet,
			ETHER_ADDR_LEN);
	}
#endif

	return 0;
}
s32 wl_cfg80211_set_p2p_noa(struct net_device *net, char* buf, int len)
{
	struct bcm_cfg80211 *cfg;

	cfg = g_bcm_cfg;

	return wl_cfgp2p_set_p2p_noa(cfg, net, buf, len);
}

s32 wl_cfg80211_get_p2p_noa(struct net_device *net, char* buf, int len)
{
	struct bcm_cfg80211 *cfg;
	cfg = g_bcm_cfg;

	return wl_cfgp2p_get_p2p_noa(cfg, net, buf, len);
}

s32 wl_cfg80211_set_p2p_ps(struct net_device *net, char* buf, int len)
{
	struct bcm_cfg80211 *cfg;
	cfg = g_bcm_cfg;

	return wl_cfgp2p_set_p2p_ps(cfg, net, buf, len);
}
#ifdef  BRCM_RSDB
s32 wl_cfg80211_set_p2p_ecsa(struct net_device *net, char* buf, int len)
{
	struct bcm_cfg80211 *cfg;
	cfg = g_bcm_cfg;

	return wl_cfgp2p_set_p2p_ecsa(cfg, net, buf, len);
}

#ifdef P2PLISTEN_AP_SAMECHN
s32 wl_cfg80211_set_p2p_resp_ap_chn(struct net_device *net, s32 enable)
{
	s32 ret = wldev_iovar_setint(net, "p2p_resp_ap_chn", enable);

	if ((ret == 0) && enable) {
		/* disable PM for p2p responding on infra AP channel */
		s32 pm = PM_OFF;

		ret = wldev_ioctl(net, WLC_SET_PM, &pm, sizeof(pm), true);
	}

	return ret;
}
#endif /* P2PLISTEN_AP_SAMECHN */
#endif
s32 wl_cfg80211_channel_to_freq(u32 channel)
{
	int freq = 0;

#if LINUX_VERSION_CODE == KERNEL_VERSION(2, 6, 38) && !defined(WL_COMPAT_WIRELESS)
	freq = ieee80211_channel_to_frequency(channel);
#else
	{
		u16 band = 0;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0))
		if (channel <= CH_MAX_2G_CHANNEL)
			band = NL80211_BAND_2GHZ;
		else
			band = NL80211_BAND_5GHZ;
#else
		if (channel <= CH_MAX_2G_CHANNEL)
			band = IEEE80211_BAND_2GHZ;
		else
			band = IEEE80211_BAND_5GHZ;
#endif
		freq = ieee80211_channel_to_frequency(channel, band);
	}
#endif
	return freq;
}


#ifdef WLTDLS
static s32
wl_tdls_event_handler(struct bcm_cfg80211 *cfg, bcm_struct_cfgdev *cfgdev,
	const wl_event_msg_t *e, void *data) {

	struct net_device *ndev = NULL;
	u32 reason = ntoh32(e->reason);
	s8 *msg = NULL;

	ndev = cfgdev_to_wlc_ndev(cfgdev, cfg);

	switch (reason) {
	case WLC_E_TDLS_PEER_DISCOVERED :
		msg = " TDLS PEER DISCOVERD ";
		break;
	case WLC_E_TDLS_PEER_CONNECTED :
#ifdef PCIE_FULL_DONGLE
		dhd_tdls_update_peer_info(ndev, TRUE, (uint8 *)&e->addr.octet[0]);
#endif /* PCIE_FULL_DONGLE */
#ifdef  BRCM_RSDB
		if (cfg->tdls_mgmt_frame) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 18, 0))
			cfg80211_rx_mgmt(cfgdev, cfg->tdls_mgmt_freq, 0,
				cfg->tdls_mgmt_frame, cfg->tdls_mgmt_frame_len,
				0);
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 12, 0))
			cfg80211_rx_mgmt(cfgdev, cfg->tdls_mgmt_freq, 0,
				cfg->tdls_mgmt_frame, cfg->tdls_mgmt_frame_len,
				0, GFP_ATOMIC);
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 4, 0)) || \
	defined(WL_COMPAT_WIRELESS)
			cfg80211_rx_mgmt(cfgdev, cfg->tdls_mgmt_freq, 0,
				cfg->tdls_mgmt_frame, cfg->tdls_mgmt_frame_len,
				GFP_ATOMIC);
#else
			cfg80211_rx_mgmt(cfgdev, cfg->tdls_mgmt_freq,
				cfg->tdls_mgmt_frame, cfg->tdls_mgmt_frame_len,
				GFP_ATOMIC);
#endif /* LINUX_VERSION >= VERSION(3, 12, 0) */
		}
#endif
		msg = " TDLS PEER CONNECTED ";
		break;
	case WLC_E_TDLS_PEER_DISCONNECTED :
#ifdef PCIE_FULL_DONGLE
		dhd_tdls_update_peer_info(ndev, FALSE, (uint8 *)&e->addr.octet[0]);
#endif /* PCIE_FULL_DONGLE */
#ifdef  BRCM_RSDB
		if (cfg->tdls_mgmt_frame) {
			kfree(cfg->tdls_mgmt_frame);
			cfg->tdls_mgmt_frame = NULL;
			cfg->tdls_mgmt_freq = 0;
		}
#endif
		msg = "TDLS PEER DISCONNECTED ";
		break;
	}
	if (msg) {
		WL_ERR(("%s: " MACDBG " on %s ndev\n", msg, MAC2STRDBG((u8*)(&e->addr)),
			(bcmcfg_to_prmry_ndev(cfg) == ndev) ? "primary" : "secondary"));
	}
	return 0;

}
#endif  /* WLTDLS */
#ifdef  BRCM_RSDB
#if (LINUX_VERSION_CODE > KERNEL_VERSION(3, 2, 0)) || defined(WL_COMPAT_WIRELESS)
static s32
#if (defined(CONFIG_ARCH_MSM) && defined(TDLS_MGMT_VERSION2)) || (LINUX_VERSION_CODE < \
	KERNEL_VERSION(3, 16, 0) && LINUX_VERSION_CODE >= KERNEL_VERSION(3, 15, 0))
wl_cfg80211_tdls_mgmt(struct wiphy *wiphy, struct net_device *dev,
        u8 *peer, u8 action_code, u8 dialog_token, u16 status_code,
        u32 peer_capability, const u8 *data, size_t len)
#elif ((LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0)) && \
		(LINUX_VERSION_CODE < KERNEL_VERSION(3, 18, 0)))
wl_cfg80211_tdls_mgmt(struct wiphy *wiphy, struct net_device *dev,
	const u8 *peer, u8 action_code, u8 dialog_token, u16 status_code,
	u32 peer_capability, const u8 *data, size_t len)
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 18, 0))
wl_cfg80211_tdls_mgmt(struct wiphy *wiphy, struct net_device *dev,
	const u8 *peer, u8 action_code, u8 dialog_token, u16 status_code,
	u32 peer_capability, bool initiator, const u8 *data, size_t len)
#else
wl_cfg80211_tdls_mgmt(struct wiphy *wiphy, struct net_device *dev,
	u8 *peer, u8 action_code, u8 dialog_token, u16 status_code, const u8 *data,
	size_t len)
#endif  /* CONFIG_ARCH_MSM && TDLS_MGMT_VERSION2 */
{
	s32 ret = 0;
#ifdef WLTDLS
	struct bcm_cfg80211 *cfg;
	tdls_wfd_ie_iovar_t info;
	memset(&info, 0, sizeof(tdls_wfd_ie_iovar_t));
	cfg = g_bcm_cfg;

#if defined(CONFIG_ARCH_MSM) && defined(TDLS_MGMT_VERSION2)
	/* Some customer platform back ported this feature from kernel 3.15 to kernel 3.10
	* and that cuases build error
	*/
	BCM_REFERENCE(peer_capability);
#endif  /* CONFIG_ARCH_MSM && TDLS_MGMT_VERSION2 */

	switch (action_code) {
	/* We need to set TDLS Wifi Display IE to firmware
	 * using tdls_wfd_ie iovar
	 */
	case WLAN_TDLS_SET_PROBE_WFD_IE:
		WL_ERR(("%s WLAN_TDLS_SET_PROBE_WFD_IE\n", __FUNCTION__));
		info.mode = TDLS_WFD_PROBE_IE_TX;
		if (len > TDLS_WFD_IE_SIZE) {
			return -EINVAL;
		}
		memcpy(&info.data, data, len);
		info.length = len;
		break;
	case WLAN_TDLS_SET_SETUP_WFD_IE:
		WL_ERR(("%s WLAN_TDLS_SET_SETUP_WFD_IE\n", __FUNCTION__));
		info.mode = TDLS_WFD_IE_TX;
		if (len > TDLS_WFD_IE_SIZE) {
			return -EINVAL;
		}
		memcpy(&info.data, data, len);
		info.length = len;
		break;
	case WLAN_TDLS_SET_WFD_ENABLED:
		WL_ERR(("%s WLAN_TDLS_SET_MODE_WFD_ENABLED\n", __FUNCTION__));
		dhd_tdls_set_mode((dhd_pub_t *)(cfg->pub), true);
		goto out;
	case WLAN_TDLS_SET_WFD_DISABLED:
		WL_ERR(("%s WLAN_TDLS_SET_MODE_WFD_DISABLED\n", __FUNCTION__));
		dhd_tdls_set_mode((dhd_pub_t *)(cfg->pub), false);
		goto out;
	default:
		WL_ERR(("Unsupported action code : %d\n", action_code));
		goto out;
	}

	ret = wldev_iovar_setbuf(dev, "tdls_wfd_ie", &info, sizeof(info),
		cfg->ioctl_buf, WLC_IOCTL_MAXLEN, &cfg->ioctl_buf_sync);

	if (ret) {
		WL_ERR(("tdls_wfd_ie error %d\n", ret));
	}
out:
#endif /* WLTDLS */
	return ret;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0))
static s32
wl_cfg80211_tdls_oper(struct wiphy *wiphy, struct net_device *dev,
	const u8 *peer, enum nl80211_tdls_operation oper)
#else
static s32
wl_cfg80211_tdls_oper(struct wiphy *wiphy, struct net_device *dev,
	u8 *peer, enum nl80211_tdls_operation oper)
#endif
{
	s32 ret = 0;
#ifdef WLTDLS
	struct bcm_cfg80211 *cfg;
	tdls_iovar_t info;
	dhd_pub_t *dhdp;
	bool tdls_auto_mode = false;
	cfg = g_bcm_cfg;
	dhdp = (dhd_pub_t *)(cfg->pub);
	memset(&info, 0, sizeof(tdls_iovar_t));
	if (peer) {
		memcpy(&info.ea, peer, ETHER_ADDR_LEN);
	} else {
		return -1;
	}
	switch (oper) {
	case NL80211_TDLS_DISCOVERY_REQ:
		/* If the discovery request is broadcast then we need to set
		 * info.mode to Tunneled Probe Request
		 */
		if (memcmp(peer, (const uint8 *)BSSID_BROADCAST, ETHER_ADDR_LEN) == 0) {
			info.mode = TDLS_MANUAL_EP_WFD_TPQ;
			WL_ERR(("%s TDLS TUNNELED PRBOBE REQUEST\n", __FUNCTION__));
		} else {
			info.mode = TDLS_MANUAL_EP_DISCOVERY;
		}
		break;
	case NL80211_TDLS_SETUP:
		if (dhdp->tdls_mode == true) {
			info.mode = TDLS_MANUAL_EP_CREATE;
			tdls_auto_mode = false;
			ret = dhd_tdls_enable(dev, false, tdls_auto_mode, NULL);
			if (ret < 0) {
				return ret;
			}
		} else {
			tdls_auto_mode = true;
		}
		break;
	case NL80211_TDLS_TEARDOWN:
		info.mode = TDLS_MANUAL_EP_DELETE;
		break;
	default:
		WL_ERR(("Unsupported operation : %d\n", oper));
		goto out;
	}
	/* turn on TDLS */
	ret = dhd_tdls_enable(dev, true, tdls_auto_mode, NULL);
	if (ret < 0) {
		return ret;
	}
	if (info.mode) {
		ret = wldev_iovar_setbuf(dev, "tdls_endpoint", &info, sizeof(info),
			cfg->ioctl_buf, WLC_IOCTL_MAXLEN, &cfg->ioctl_buf_sync);
		if (ret) {
			WL_ERR(("tdls_endpoint error %d\n", ret));
		}
	}
out:
#endif /* WLTDLS */
	return ret;
}
#endif /* LINUX_VERSION > VERSION(3,2,0) || WL_COMPAT_WIRELESS */

s32 wl_cfg80211_set_wps_p2p_ie(struct net_device *ndev, char *buf, int len,
	enum wl_management_type type)
{
	struct bcm_cfg80211 *cfg;
	s32 ret = 0;
	struct ether_addr primary_mac;
	s32 bssidx = 0;
	s32 pktflag = 0;
	cfg = g_bcm_cfg;

	if (wl_get_drv_status(cfg, AP_CREATING, ndev)) {
		/* Vendor IEs should be set to FW
		 * after SoftAP interface is brought up
		 */
		WL_DBG(("Skipping set IE since AP is not up \n"));
		goto exit;
	} else  if (ndev == bcmcfg_to_prmry_ndev(cfg)) {
		/* Either stand alone AP case or P2P discovery */
		if (wl_get_drv_status(cfg, AP_CREATED, ndev)) {
			/* Stand alone AP case on primary interface */
			WL_DBG(("Apply IEs for Primary AP Interface \n"));
			bssidx = 0;
		} else {
			/* P2P Discovery case (p2p listen) */
			if (!cfg->p2p->on) {
				/* Turn on Discovery interface */
				get_primary_mac(cfg, &primary_mac);
				wl_cfgp2p_generate_bss_mac(cfg, &primary_mac);
				p2p_on(cfg) = true;
				ret = wl_cfgp2p_enable_discovery(cfg, ndev, NULL, 0);
				if (unlikely(ret)) {
					WL_ERR(("Enable discovery failed \n"));
					goto exit;
				}
			}
			WL_DBG(("Apply IEs for P2P Discovery Iface \n"));
			ndev = wl_to_p2p_bss_ndev(cfg, P2PAPI_BSSCFG_PRIMARY);
			bssidx = wl_to_p2p_bss_bssidx(cfg, P2PAPI_BSSCFG_DEVICE);
		}
	} else {
		/* Virtual AP/ P2P Group Interface */
		WL_DBG(("Apply IEs for iface:%s\n", ndev->name));
		bssidx = wl_get_bssidx_by_wdev(cfg, ndev->ieee80211_ptr);
	}

	if (ndev != NULL) {
		switch (type) {
			case WL_BEACON:
				pktflag = VNDR_IE_BEACON_FLAG;
				break;
			case WL_PROBE_RESP:
				pktflag = VNDR_IE_PRBRSP_FLAG;
				break;
			case WL_ASSOC_RESP:
				pktflag = VNDR_IE_ASSOCRSP_FLAG;
				break;
		}
		if (pktflag) {
			ret = wl_cfg80211_set_mgmt_vndr_ies(cfg,
				ndev_to_cfgdev(ndev), bssidx, pktflag, buf, len);
		}
	}
exit:
	return ret;
}

#else   /* BRCM_RSDB */
#if (LINUX_VERSION_CODE > KERNEL_VERSION(3, 2, 0))
static s32
wl_cfg80211_tdls_oper(struct wiphy *wiphy, struct net_device *dev,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0))
	const
#endif
	u8 *peer, enum nl80211_tdls_operation oper)
{
	s32 ret = 0;
#ifdef WLTDLS
	struct bcm_cfg80211 *cfg;
	tdls_iovar_t info;
	cfg = g_bcm_cfg;
	memset(&info, 0, sizeof(tdls_iovar_t));
	if (peer)
		memcpy(&info.ea, peer, ETHER_ADDR_LEN);
	switch (oper) {
	case NL80211_TDLS_DISCOVERY_REQ:
		/* turn on TDLS */
		ret = dhd_tdls_enable(dev, true, false, NULL);
		if (ret < 0)
			return ret;
		info.mode = TDLS_MANUAL_EP_DISCOVERY;
		break;
	case NL80211_TDLS_SETUP:
		/* auto mode on */
		ret = dhd_tdls_enable(dev, true, true, (struct ether_addr *)peer);
		if (ret < 0)
			return ret;
		break;
	case NL80211_TDLS_TEARDOWN:
		info.mode = TDLS_MANUAL_EP_DELETE;
		/* auto mode off */
		ret = dhd_tdls_enable(dev, true, false, (struct ether_addr *)peer);
		if (ret < 0)
			return ret;
		break;
	default:
		WL_ERR(("Unsupported operation : %d\n", oper));
		goto out;
	}
	if (info.mode) {
		ret = wldev_iovar_setbuf(dev, "tdls_endpoint", &info, sizeof(info),
			cfg->ioctl_buf, WLC_IOCTL_MAXLEN, &cfg->ioctl_buf_sync);
		if (ret) {
			WL_ERR(("tdls_endpoint error %d\n", ret));
		}
	}
out:
#endif /* WLTDLS */
	return ret;
}
#endif


s32 wl_cfg80211_set_wps_p2p_ie(struct net_device *net, char *buf, int len,
	enum wl_management_type type)
{
	struct bcm_cfg80211 *cfg;
	struct net_device *ndev = NULL;
	struct ether_addr primary_mac;
	s32 ret = 0;
	s32 bssidx = 0;
	s32 pktflag = 0;
	cfg = g_bcm_cfg;

	if (wl_get_drv_status(cfg, AP_CREATING, net)) {
		/* Vendor IEs should be set to FW
		 * after SoftAP interface is brought up
		 */
		goto exit;
	} else if (wl_get_drv_status(cfg, AP_CREATED, net)) {
		ndev = net;
		bssidx = 0;
	} else if (cfg->p2p) {
		net = ndev_to_wlc_ndev(net, cfg);
		if (!cfg->p2p->on) {
			get_primary_mac(cfg, &primary_mac);
			wl_cfgp2p_generate_bss_mac(&primary_mac, &cfg->p2p->dev_addr,
				&cfg->p2p->int_addr);
			/* In case of p2p_listen command, supplicant send remain_on_channel
			* without turning on P2P
			*/

			p2p_on(cfg) = true;
			ret = wl_cfgp2p_enable_discovery(cfg, net, NULL, 0);

			if (unlikely(ret)) {
				goto exit;
			}
		}
		if (net  != bcmcfg_to_prmry_ndev(cfg)) {
			if (wl_get_mode_by_netdev(cfg, net) == WL_MODE_AP) {
				ndev = wl_to_p2p_bss_ndev(cfg, P2PAPI_BSSCFG_CONNECTION);
				bssidx = wl_to_p2p_bss_bssidx(cfg, P2PAPI_BSSCFG_CONNECTION);
			}
		} else {
				ndev = wl_to_p2p_bss_ndev(cfg, P2PAPI_BSSCFG_PRIMARY);
				bssidx = wl_to_p2p_bss_bssidx(cfg, P2PAPI_BSSCFG_DEVICE);
		}
	}
	if (ndev != NULL) {
		switch (type) {
			case WL_BEACON:
				pktflag = VNDR_IE_BEACON_FLAG;
				break;
			case WL_PROBE_RESP:
				pktflag = VNDR_IE_PRBRSP_FLAG;
				break;
			case WL_ASSOC_RESP:
				pktflag = VNDR_IE_ASSOCRSP_FLAG;
				break;
		}
		if (pktflag)
			ret = wl_cfgp2p_set_management_ie(cfg, ndev, bssidx, pktflag, buf, len);
	}
exit:
	return ret;
}
#endif /*BRCM_RSDB*/

#ifdef WL_SUPPORT_AUTO_CHANNEL
static s32
wl_cfg80211_set_auto_channel_scan_state(struct net_device *ndev)
{
	u32 val = 0;
	s32 ret = BCME_ERROR;
	struct bcm_cfg80211 *cfg = g_bcm_cfg;
#ifdef  BRCM_RSDB
	struct wiphy *wiphy;
#endif
	/* Disable mpc, to avoid automatic interface down. */
	val = 0;
#ifdef  BRCM_RSDB
	wiphy = bcmcfg_to_wiphy(cfg);
	if (wl_check_dongle_idle(wiphy) != TRUE) {
		WL_ERR(("FW is busy to add interface"));
		return ret;
	}
#endif
	ret = wldev_iovar_setbuf_bsscfg(ndev, "mpc", (void *)&val,
		sizeof(val), cfg->ioctl_buf, WLC_IOCTL_SMLEN, 0,
		&cfg->ioctl_buf_sync);
	if (ret < 0) {
		WL_ERR(("set 'mpc' failed, error = %d\n", ret));
		goto done;
	}

	/* Set interface up, explicitly. */
	val = 1;

	ret = wldev_ioctl(ndev, WLC_UP, (void *)&val, sizeof(val), true);
	if (ret < 0) {
		WL_ERR(("set interface up failed, error = %d\n", ret));
		goto done;
	}

	/* Stop all scan explicitly, till auto channel selection complete. */
	wl_set_drv_status(cfg, SCANNING, ndev);
	if (cfg->escan_info.ndev == NULL) {
		ret = BCME_OK;
		goto done;
	}
	ret = wl_notify_escan_complete(cfg, ndev, true, true);
	if (ret < 0) {
		WL_ERR(("set scan abort failed, error = %d\n", ret));
		goto done;
	}

done:
	return ret;
}
#ifndef  BRCM_RSDB
static bool
wl_cfg80211_valid_chanspec_p2p(chanspec_t chanspec)
{
	bool valid = false;
	char chanbuf[CHANSPEC_STR_LEN];

	/* channel 1 to 14 */
	if ((chanspec >= 0x2b01) && (chanspec <= 0x2b0e)) {
		valid = true;
	}
	/* channel 36 to 48 */
	else if ((chanspec >= 0x1b24) && (chanspec <= 0x1b30)) {
		valid = true;
	}
	/* channel 149 to 161 */
	else if ((chanspec >= 0x1b95) && (chanspec <= 0x1ba1)) {
		valid = true;
	}
	else {
		valid = false;
		WL_INFORM(("invalid P2P chanspec, chanspec = %s\n",
			wf_chspec_ntoa_ex(chanspec, chanbuf)));
	}

	return valid;
}

static s32
#else
static bool
wl_cfg80211_valid_channel_p2p(int channel)
{
	bool valid = false;

	/* channel 1 to 14 */
	if ((channel >= 1) && (channel <= 14)) {
		valid = true;
	}
	/* channel 36 to 48 */
	else if ((channel >= 36) && (channel <= 48)) {
		valid = true;
	}
	/* channel 149 to 161 */
	else if ((channel >= 149) && (channel <= 161)) {
		valid = true;
	}
	else {
		valid = false;
		WL_INFORM(("invalid P2P chanspec, channel = %d\n", channel));
	}

	return valid;
}

s32
#endif
wl_cfg80211_get_chanspecs_2g(struct net_device *ndev, void *buf, s32 buflen)
{
	s32 ret = BCME_ERROR;
	struct bcm_cfg80211 *cfg = NULL;
	wl_uint32_list_t *list = NULL;
	chanspec_t chanspec = 0;

	memset(buf, 0, buflen);

	cfg = g_bcm_cfg;
	list = (wl_uint32_list_t *)buf;
	list->count = htod32(WL_NUMCHANSPECS);

	/* Restrict channels to 2.4GHz, 20MHz BW, no SB. */
	chanspec |= (WL_CHANSPEC_BAND_2G | WL_CHANSPEC_BW_20 |
		WL_CHANSPEC_CTL_SB_NONE);
	chanspec = wl_chspec_host_to_driver(chanspec);

	ret = wldev_iovar_getbuf_bsscfg(ndev, "chanspecs", (void *)&chanspec,
		sizeof(chanspec), buf, buflen, 0, &cfg->ioctl_buf_sync);
	if (ret < 0) {
		WL_ERR(("get 'chanspecs' failed, error = %d\n", ret));
	}

	return ret;
}
#ifndef  BRCM_RSDB
static s32
#else
s32
#endif
wl_cfg80211_get_chanspecs_5g(struct net_device *ndev, void *buf, s32 buflen)
{
	u32 channel = 0;
	s32 ret = BCME_ERROR;
	s32 i = 0;
	s32 j = 0;
	struct bcm_cfg80211 *cfg = NULL;
	wl_uint32_list_t *list = NULL;
	chanspec_t chanspec = 0;

	memset(buf, 0, buflen);

	cfg = g_bcm_cfg;
	list = (wl_uint32_list_t *)buf;
	list->count = htod32(WL_NUMCHANSPECS);

	/* Restrict channels to 5GHz, 20MHz BW, no SB. */
	chanspec |= (WL_CHANSPEC_BAND_5G | WL_CHANSPEC_BW_20 |
		WL_CHANSPEC_CTL_SB_NONE);
	chanspec = wl_chspec_host_to_driver(chanspec);

	ret = wldev_iovar_getbuf_bsscfg(ndev, "chanspecs", (void *)&chanspec,
		sizeof(chanspec), buf, buflen, 0, &cfg->ioctl_buf_sync);
	if (ret < 0) {
		WL_ERR(("get 'chanspecs' failed, error = %d\n", ret));
		goto done;
	}

	/* Skip DFS and inavlid P2P channel. */
	for (i = 0, j = 0; i < dtoh32(list->count); i++) {
		chanspec = (chanspec_t) dtoh32(list->element[i]);
		channel = CHSPEC_CHANNEL(chanspec);

		ret = wldev_iovar_getint(ndev, "per_chan_info", &channel);
		if (ret < 0) {
			WL_ERR(("get 'per_chan_info' failed, error = %d\n", ret));
			goto done;
		}

		if (CHANNEL_IS_RADAR(channel) ||
#ifndef  BRCM_RSDB
			!(wl_cfg80211_valid_chanspec_p2p(chanspec))) {
#else
			!(wl_cfg80211_valid_channel_p2p(CHSPEC_CHANNEL(chanspec)))) {
#endif
			continue;
		} else {
			list->element[j] = list->element[i];
		}

		j++;
	}

	list->count = j;

done:
	return ret;
}

static s32
wl_cfg80211_get_best_channel(struct net_device *ndev, int *channel)
{
	s32 ret = BCME_ERROR;
	int chosen = 0;
	int retry = 0;

	/* Start auto channel selection scan. */
	ret = wldev_ioctl(ndev, WLC_START_CHANNEL_SEL, NULL, 0, true);
	if (ret < 0) {
		WL_ERR(("can't start auto channel scan, error = %d\n", ret));
		*channel = 0;
		goto done;
	}

	/* Wait for auto channel selection, worst case possible delay is 5250ms. */
	retry = CHAN_SEL_RETRY_COUNT;

	while (retry--) {
		OSL_SLEEP(CHAN_SEL_IOCTL_DELAY);

		ret = wldev_ioctl(ndev, WLC_GET_CHANNEL_SEL, &chosen, sizeof(chosen),
			false);
		if ((ret == 0) && (dtoh32(chosen) != 0)) {
			*channel = (u16)(chosen & 0x00FF);
			WL_INFORM(("selected channel = %d\n", *channel));
			break;
		}
		WL_INFORM(("attempt = %d, ret = %d, chosen = %d\n",
			(CHAN_SEL_RETRY_COUNT - retry), ret, dtoh32(chosen)));
	}

	if (retry <= 0)	{
		WL_ERR(("failure, auto channel selection timed out\n"));
		*channel = 0;
		ret = BCME_ERROR;
	}

done:
	return ret;
}

static s32
wl_cfg80211_restore_auto_channel_scan_state(struct net_device *ndev)
{
	u32 val = 0;
	s32 ret = BCME_ERROR;
	struct bcm_cfg80211 *cfg = g_bcm_cfg;

	/* Clear scan stop driver status. */
	wl_clr_drv_status(cfg, SCANNING, ndev);

	/* Enable mpc back to 1, irrespective of initial state. */
	val = 1;

	ret = wldev_iovar_setbuf_bsscfg(ndev, "mpc", (void *)&val,
		sizeof(val), cfg->ioctl_buf, WLC_IOCTL_SMLEN, 0,
		&cfg->ioctl_buf_sync);
	if (ret < 0) {
		WL_ERR(("set 'mpc' failed, error = %d\n", ret));
	}

	return ret;
}

s32
wl_cfg80211_get_best_channels(struct net_device *dev, char* cmd, int total_len)
{
	int channel = 0;
	s32 ret = BCME_ERROR;
	u8 *buf = NULL;
	char *pos = cmd;
	struct bcm_cfg80211 *cfg = NULL;
	struct net_device *ndev = NULL;

	memset(cmd, 0, total_len);

	buf = kmalloc(CHANSPEC_BUF_SIZE, GFP_KERNEL);
	if (buf == NULL) {
		WL_ERR(("failed to allocate chanspec buffer\n"));
		return -ENOMEM;
	}

	/*
	 * Always use primary interface, irrespective of interface on which
	 * command came.
	 */
	cfg = g_bcm_cfg;
	ndev = bcmcfg_to_prmry_ndev(cfg);

	/*
	 * Make sure that FW and driver are in right state to do auto channel
	 * selection scan.
	 */
	ret = wl_cfg80211_set_auto_channel_scan_state(ndev);
	if (ret < 0) {
		WL_ERR(("can't set auto channel scan state, error = %d\n", ret));
		goto done;
	}

	/* Best channel selection in 2.4GHz band. */
	ret = wl_cfg80211_get_chanspecs_2g(ndev, (void *)buf, CHANSPEC_BUF_SIZE);
	if (ret < 0) {
		WL_ERR(("can't get chanspecs in 2.4GHz, error = %d\n", ret));
		goto done;
	}

	ret = wl_cfg80211_get_best_channel(ndev, &channel);
	if (ret < 0) {
		WL_ERR(("can't select best channel scan in 2.4GHz, error = %d\n", ret));
		goto done;
	}

	if (CHANNEL_IS_2G(channel)) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0))
		channel = ieee80211_channel_to_frequency(channel, NL80211_BAND_2GHZ);
#else
		channel = ieee80211_channel_to_frequency(channel, IEEE80211_BAND_2GHZ);
#endif
	} else {
		WL_ERR(("invalid 2.4GHz channel, channel = %d\n", channel));
		channel = 0;
	}
#ifndef  BRCM_RSDB
	sprintf(pos, "%04d ", channel);
	pos += 5;
#else
	pos += snprintf(pos, total_len, "%04d ", channel);
#endif

	/* Best channel selection in 5GHz band. */
	ret = wl_cfg80211_get_chanspecs_5g(ndev, (void *)buf, CHANSPEC_BUF_SIZE);
	if (ret < 0) {
		WL_ERR(("can't get chanspecs in 5GHz, error = %d\n", ret));
		goto done;
	}

	ret = wl_cfg80211_get_best_channel(ndev, &channel);
	if (ret < 0) {
		WL_ERR(("can't select best channel scan in 5GHz, error = %d\n", ret));
		goto done;
	}

	if (CHANNEL_IS_5G(channel)) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0))
		channel = ieee80211_channel_to_frequency(channel, NL80211_BAND_5GHZ);
#else
		channel = ieee80211_channel_to_frequency(channel, IEEE80211_BAND_5GHZ);
#endif
	} else {
		WL_ERR(("invalid 5GHz channel, channel = %d\n", channel));
		channel = 0;
	}
#ifndef  BRCM_RSDB
	sprintf(pos, "%04d ", channel);
	pos += 5;
#else
	pos += snprintf(pos, total_len, "%04d ", channel);
#endif

	/* Set overall best channel same as 5GHz best channel. */
#ifndef  BRCM_RSDB
	sprintf(pos, "%04d ", channel);
	pos += 5;
#else
	pos += snprintf(pos, total_len, "%04d ", channel);
#endif

done:
	if (NULL != buf) {
		kfree(buf);
	}

	/* Restore FW and driver back to normal state. */
	ret = wl_cfg80211_restore_auto_channel_scan_state(ndev);
	if (ret < 0) {
		WL_ERR(("can't restore auto channel scan state, error = %d\n", ret));
	}

	return (pos - cmd);
}
#endif /* WL_SUPPORT_AUTO_CHANNEL */

static const struct rfkill_ops wl_rfkill_ops = {
	.set_block = wl_rfkill_set
};

static int wl_rfkill_set(void *data, bool blocked)
{
	struct bcm_cfg80211 *cfg = (struct bcm_cfg80211 *)data;

	WL_DBG(("Enter \n"));
	WL_DBG(("RF %s\n", blocked ? "blocked" : "unblocked"));

	if (!cfg)
		return -EINVAL;

	cfg->rf_blocked = blocked;

	return 0;
}

static int wl_setup_rfkill(struct bcm_cfg80211 *cfg, bool setup)
{
	s32 err = 0;

	WL_DBG(("Enter \n"));
	if (!cfg)
		return -EINVAL;
	if (setup) {
		cfg->rfkill = rfkill_alloc("brcmfmac-wifi",
			wl_cfg80211_get_parent_dev(),
			RFKILL_TYPE_WLAN, &wl_rfkill_ops, (void *)cfg);

		if (!cfg->rfkill) {
			err = -ENOMEM;
			goto err_out;
		}

		err = rfkill_register(cfg->rfkill);

		if (err)
			rfkill_destroy(cfg->rfkill);
	} else {
		if (!cfg->rfkill) {
			err = -ENOMEM;
			goto err_out;
		}

		rfkill_unregister(cfg->rfkill);
		rfkill_destroy(cfg->rfkill);
	}

err_out:
	return err;
}

#ifdef DEBUGFS_CFG80211
/**
* Format : echo "SCAN:1 DBG:1" > /sys/kernel/debug/dhd/debug_level
* to turn on SCAN and DBG log.
* To turn off SCAN partially, echo "SCAN:0" > /sys/kernel/debug/dhd/debug_level
* To see current setting of debug level,
* cat /sys/kernel/debug/dhd/debug_level
*/
static ssize_t
wl_debuglevel_write(struct file *file, const char __user *userbuf,
	size_t count, loff_t *ppos)
{
	char tbuf[S_SUBLOGLEVEL * ARRAYSIZE(sublogname_map)], sublog[S_SUBLOGLEVEL];
	char *params, *token, *colon;
	uint i, tokens, log_on = 0;
	memset(tbuf, 0, sizeof(tbuf));
	memset(sublog, 0, sizeof(sublog));
#ifndef  BRCM_RSDB
	if (copy_from_user(&tbuf, userbuf, min_t(size_t, sizeof(tbuf), count)))
#else
	if (copy_from_user(&tbuf, userbuf, min_t(size_t, (sizeof(tbuf) - 1), count)))
#endif
		return -EFAULT;

	params = &tbuf[0];
	colon = strchr(params, '\n');
	if (colon != NULL)
		*colon = '\0';
	while ((token = strsep(&params, " ")) != NULL) {
		memset(sublog, 0, sizeof(sublog));
		if (token == NULL || !*token)
			break;
		if (*token == '\0')
			continue;
		colon = strchr(token, ':');
		if (colon != NULL) {
			*colon = ' ';
		}
		tokens = sscanf(token, "%s %u", sublog, &log_on);
		if (colon != NULL)
			*colon = ':';

		if (tokens == 2) {
				for (i = 0; i < ARRAYSIZE(sublogname_map); i++) {
					if (!strncmp(sublog, sublogname_map[i].sublogname,
						strlen(sublogname_map[i].sublogname))) {
						if (log_on)
							wl_dbg_level |=
							(sublogname_map[i].log_level);
						else
							wl_dbg_level &=
							~(sublogname_map[i].log_level);
					}
				}
		} else
			WL_ERR(("%s: can't parse '%s' as a "
			       "SUBMODULE:LEVEL (%d tokens)\n",
			       tbuf, token, tokens));


	}
	return count;
}

static ssize_t
wl_debuglevel_read(struct file *file, char __user *user_buf,
	size_t count, loff_t *ppos)
{
	char *param;
	char tbuf[S_SUBLOGLEVEL * ARRAYSIZE(sublogname_map)];
	uint i;
	memset(tbuf, 0, sizeof(tbuf));
	param = &tbuf[0];
	for (i = 0; i < ARRAYSIZE(sublogname_map); i++) {
		param += snprintf(param, sizeof(tbuf) - 1, "%s:%d ",
			sublogname_map[i].sublogname,
			(wl_dbg_level & sublogname_map[i].log_level) ? 1 : 0);
	}
	*param = '\n';
	return simple_read_from_buffer(user_buf, count, ppos, tbuf, strlen(&tbuf[0]));

}
/*
*  trigger wifi Recovery
*
*  echo 1 > sys/kernel/debug/bcmdhd/recovery_level
*/
static ssize_t
wl_debugRecovery_write(struct file *file, const char __user *userbuf,
	size_t count, loff_t *ppos)
{
	char tbuf[S_SUBLOGLEVEL];
	uint recover = 0;

	struct net_device *ndev = file->private_data;
	memset(tbuf, 0, sizeof(tbuf));
	if (copy_from_user(&tbuf, userbuf, min_t(size_t, sizeof(tbuf), count))){
		WL_ERR(("copy string from user error!"));
		return -EFAULT;
	}

	if ( sscanf(tbuf, "%d",&recover) ) {
		if (recover == 1){
			net_os_send_hang_message(ndev);
		}

	} else{
		WL_ERR(("%s: can't parse",tbuf));
	}

	return count;
}

static int get_data_debugrecovery_open(struct inode *inode,struct file *filp)
{
	filp->private_data = inode->i_private;
	return 0;
}
static int get_data_debugrecovery_close(struct inode *inode,struct file *filp)
{
	filp->private_data = NULL;
	return 0;
}

static const struct file_operations fops_debuglevel = {
	.open = NULL,
	.write = wl_debuglevel_write,
	.read = wl_debuglevel_read,
	.owner = THIS_MODULE,
	.llseek = NULL,
};
static const struct file_operations fops_debugrecovery = {
	.open = get_data_debugrecovery_open,
	.release = get_data_debugrecovery_close,
	.write = wl_debugRecovery_write,
	.owner = THIS_MODULE,
	.llseek = NULL,
};

#ifdef HW_SDIO_QUALITY_STATISTIC
static int wl_debug_sdio_quality_open(struct inode *inode,struct file *filp)
{
	filp->private_data = inode->i_private;
	return 0;
}
static int wl_debug_sdio_quality_close(struct inode *inode,struct file *filp)
{
	filp->private_data = NULL;
	return 0;
}

static ssize_t
wl_debug_sdio_quality_read(struct file *file, char __user *user_buf,
	size_t count, loff_t *ppos)
{
	char *param;
	static char tbuf[8192];
	uint cnt = 0;
	sdio_quality_t *entry;
	struct bcm_cfg80211 *cfg = (struct bcm_cfg80211 *)file->private_data;
	dhd_pub_t *dhdp = (dhd_pub_t*)cfg->pub;
	sdio_quality_t *sdiop = (sdio_quality_t *)dhdp->sdio_quality;
	if (sdiop == NULL)
		return 0;

	memset(tbuf, 0, sizeof(tbuf));
	param = &tbuf[0];
	cnt = snprintf(param, sizeof(tbuf) -1,
		"cmd_type                dir     request_count      actual_count\n");
	param += cnt;
	list_for_each_entry(entry, &sdiop->list, list) {
		cnt = snprintf(param, sizeof(tbuf) - 1, "%-20s  %5s  %16u  %16u\n",
						sdio_command_to_string(entry->type),
						entry->dir?"WIRTE":"READ",
						entry->request_count,
						entry->actual_count);
		param += cnt;
		if(strlen(tbuf) + cnt >= sizeof(tbuf)) {
			printf("not enough buffer\n");
			break;
		}
	}
	*param = '\n';

	return simple_read_from_buffer(user_buf, count, ppos, tbuf, strlen(&tbuf[0]));
}


static const struct file_operations fops_debug_sdio_quality = {
	.open = wl_debug_sdio_quality_open,
	.release = wl_debug_sdio_quality_close,
	.read = wl_debug_sdio_quality_read,
	.owner = THIS_MODULE,
	.llseek = NULL,
};
#endif

#ifdef HW_WL_COUNTERS_STATISTIC
static int wl_debug_counters_fs_open(struct inode *inode,struct file *filp)
{
	filp->private_data = inode->i_private;
	return 0;
}
static int wl_debug_counters_fs_close(struct inode *inode,struct file *filp)
{
	filp->private_data = NULL;
	return 0;
}
static ssize_t
wl_debug_counters_fs_read(struct file *file, char __user *user_buf,
	size_t count, loff_t *ppos)
{
#define COUNTER_ITEM_CNT (sizeof(wl_cnt_t) / sizeof(uint32))
	static char iovar_buf[WLC_IOCTL_MAXLEN];
	char *param;
	static char tbuf[8192];
	uint32 *p;
	int err = 0;
	uint cnt = 0;
	wl_cnt_t *wl_cnt;
	int i = 1;

	struct bcm_cfg80211 *cfg = (struct bcm_cfg80211 *)file->private_data;
	dhd_pub_t *dhdp = (dhd_pub_t*)cfg->pub;
	if(dhdp == NULL ||  dhdp->up == 0) {
		WL_ERR(("%s: wifi is close return avoid panic\n", __FUNCTION__));
		return 0;
	}
	memset(tbuf, 0, sizeof(tbuf));
	param = &tbuf[0];

	bzero(iovar_buf, WLC_IOCTL_MAXLEN);

	err = wldev_iovar_getbuf(bcmcfg_to_prmry_ndev(cfg), "counters", NULL, 0,
		iovar_buf, WLC_IOCTL_MAXLEN, NULL);
	if (unlikely(err)) {
		WL_ERR(("error (%d) - size = %zu\n", err, sizeof(wl_cnt_t)));
		return err;
	}
	wl_cnt = (wl_cnt_t *)iovar_buf;

	if (wl_cnt->version != WL_CNT_T_VERSION)
		return 0;

	p = (uint32*)iovar_buf;
	p = p + 1;

	for(; i < COUNTER_ITEM_CNT; i++) {
		cnt = snprintf(param, sizeof(tbuf) - 1, "%s:%u ",
						counters_command_to_string(txframe + i - 1),
						*p++);
		param += cnt;
		if (i % 6 == 0) {
			cnt = snprintf(param, sizeof(tbuf) - 1, "\n");
			param += cnt;
		}
	}
	*param = '\n';

	return simple_read_from_buffer(user_buf, count, ppos, tbuf, strlen(&tbuf[0]));
}


static const struct file_operations fops_debug_wl_counters = {
	.open = wl_debug_counters_fs_open,
	.release = wl_debug_counters_fs_close,
	.read = wl_debug_counters_fs_read,
	.owner = THIS_MODULE,
	.llseek = NULL,
};
#endif

#ifdef HW_OTP_PATCH
static ssize_t
wl_debug_otp_fs_write(struct file *file, const char __user *userbuf,
	size_t count, loff_t *ppos)
{
#define OTP_SIZE (1150)
#define BUF_SIZE (32)
#define PARAM_BUF_SIZE (1280)
#define CMD_FLASH "29345123456"
#define OTP_FILE_PATH "/vendor/firmware/bcm43596_otp.bin"

	int err;
	ssize_t filesize = 0;
	mm_segment_t old_fs;
	struct file* filp = NULL;
	char *otpdatap;
	char tbuf[BUF_SIZE];
	size_t ret = count;
	char *parambuf = NULL;
	char *ioctlbuf = NULL;

	cis_rw_t cish;
	struct bcm_cfg80211 *cfg = NULL;
	if (NULL == file || NULL == userbuf || NULL == ppos) {
		WL_ERR(("%s: parameters error", __func__));
		return -EFAULT;
	}

	cfg = (struct bcm_cfg80211 *)file->private_data;
	if (NULL == cfg) {
		WL_ERR(("%s: cfg is NULL", __func__));
		return -EFAULT;
	}

	memset(tbuf, 0, sizeof(tbuf));



	if (copy_from_user(&tbuf, userbuf, min_t(size_t, (sizeof(tbuf) - 1), count))) {
		WL_ERR(("%s: copy_from_user error", __func__));
		return -EFAULT;
	}

	if (strncmp(tbuf, CMD_FLASH, strlen(CMD_FLASH))) {
		WL_ERR(("%s: not handle", tbuf));
		return count;
	}

	cish.source = 0;
	cish.byteoff = 0;
	cish.flags = 0;
	cish.nbytes = OTP_SIZE;
	parambuf = kmalloc(PARAM_BUF_SIZE, GFP_KERNEL);
	if(NULL == parambuf){
		WL_ERR(("failed to allocate parambuf\n"));
		return -EFAULT;
	}
	memset(parambuf, 0, PARAM_BUF_SIZE);
	memcpy(parambuf, (char*)&cish, sizeof(cish));

	filp = filp_open(OTP_FILE_PATH, O_RDONLY, 0);
	if (IS_ERR(filp)) {
		WL_ERR(("open file error %s\n", OTP_FILE_PATH));
		ret = -1;
		goto out;
	}
	otpdatap = parambuf + sizeof(cish);

	old_fs = get_fs();
	set_fs(KERNEL_DS);
	filp->f_pos = 0;
	filesize = vfs_read(filp, otpdatap, (PARAM_BUF_SIZE - sizeof(cish)), &filp->f_pos);
	filp_close(filp, NULL);
	set_fs(old_fs);

	if (OTP_SIZE != filesize) {
		WL_ERR(("otp file size %zu\n", filesize));
		goto out;
	}

	ioctlbuf = kmalloc(PARAM_BUF_SIZE, GFP_KERNEL);
	if(NULL == ioctlbuf){
		WL_ERR(("failed to allocate ioctlbuf\n"));
		ret = -EFAULT;
		goto out;
	}
	memset(ioctlbuf, 0, PARAM_BUF_SIZE);
	err = wldev_iovar_setbuf(bcmcfg_to_prmry_ndev(cfg), "ciswrite", parambuf,
			OTP_SIZE + sizeof(cish), ioctlbuf, PARAM_BUF_SIZE, NULL);
	if (err)
		WL_ERR(("ciswrite error :%d\n", err));

	WL_ERR(("%s finish\n", __func__));
	ret = count;
out:
	if(parambuf){
		kfree(parambuf);
		parambuf = NULL;
	}
	if(ioctlbuf){
		kfree(ioctlbuf);
		ioctlbuf = NULL;
	}
	return ret;
}

static int wl_debug_otp_fs_open(struct inode *inode,struct file *filp) {
	if (NULL != filp) {
		filp->private_data = inode->i_private;
	}
	return 0;
}
static int wl_debug_otp_fs_close(struct inode *inode,struct file *filp) {
	if (NULL != filp) {
		filp->private_data = NULL;
	}
	return 0;
}

static ssize_t
wl_debug_otp_fs_read(struct file *file, char __user *user_buf,
	size_t count, loff_t *ppos)
{
	int err = 0;
	cis_rw_t cish;
	char* outbuf;
	static char ioctlbuf[WLC_IOCTL_MAXLEN];
	struct bcm_cfg80211 *cfg = NULL;

	if (NULL == file || NULL == user_buf || NULL == ppos) {
		WL_ERR(("%s: parameters error", __func__));
		return -EFAULT;
	}
	cfg = (struct bcm_cfg80211 *)file->private_data;
	if (NULL == cfg) {
		WL_ERR(("%s: cfg is NULL", __func__));
		return -EFAULT;
	}

	bzero(&cish, sizeof(cish));
	bzero(ioctlbuf, WLC_IOCTL_MAXLEN);

	err = wldev_iovar_getbuf(bcmcfg_to_prmry_ndev(cfg), "cisdump", &cish, sizeof(cish),
		ioctlbuf, WLC_IOCTL_MAXLEN, NULL);
	if (unlikely(err)) {
		WL_ERR(("error (%d) - size = %zu\n", err, sizeof(cish)));
		return err;
	}
	memcpy((char*)&cish, ioctlbuf, sizeof(cish));
	if ((cish.nbytes + sizeof(cish)) >= WLC_IOCTL_MAXLEN) {
		WL_ERR(("wl_debug_otp_fs_read nbytes %d\n", cish.nbytes));
		return 0;
	}

	outbuf = ioctlbuf + sizeof(cish);
	return simple_read_from_buffer(user_buf, count, ppos, outbuf, cish.nbytes);
}

static const struct file_operations fops_debug_otp = {
	.owner = THIS_MODULE,
	.llseek = NULL,
	.read = wl_debug_otp_fs_read,
	.write = wl_debug_otp_fs_write,
	.open = wl_debug_otp_fs_open,
	.release = wl_debug_otp_fs_close,
};
#endif

static s32 wl_setup_debugfs(struct bcm_cfg80211 *cfg,struct net_device *ndev)
{
	s32 err = 0;
	struct dentry *_dentry;
	if (!cfg)
		return -EINVAL;
	cfg->debugfs = debugfs_create_dir(KBUILD_MODNAME, NULL);
	if (!cfg->debugfs || IS_ERR(cfg->debugfs)) {
		if (cfg->debugfs == ERR_PTR(-ENODEV))
			WL_ERR(("Debugfs is not enabled on this kernel\n"));
		else
			WL_ERR(("Can not create debugfs directory\n"));
		cfg->debugfs = NULL;
		goto exit;

	}
	_dentry = debugfs_create_file("debug_level", S_IRUSR | S_IWUSR,
		cfg->debugfs, cfg, &fops_debuglevel);
	if (!_dentry || IS_ERR(_dentry)) {
		WL_ERR(("failed to create debug_level debug file\n"));
		wl_free_debugfs(cfg);
	}
	_dentry = debugfs_create_file("debug_recovery", S_IWUSR,
		cfg->debugfs, ndev, &fops_debugrecovery);
	if (!_dentry || IS_ERR(_dentry)) {
		WL_ERR(("failed to create debug_level debug file\n"));
		wl_free_debugfs(cfg);
	}

#ifdef HW_SDIO_QUALITY_STATISTIC
	_dentry = debugfs_create_file("debug_sdio_quality", S_IRUSR,
		cfg->debugfs, cfg, &fops_debug_sdio_quality);
	if (!_dentry || IS_ERR(_dentry)) {
		WL_ERR(("failed to create debug_sdio_quality debug file\n"));
		wl_free_debugfs(cfg);
	}
#endif

#ifdef HW_WL_COUNTERS_STATISTIC
	_dentry = debugfs_create_file("debug_wl_counters", S_IRUSR,
		cfg->debugfs, cfg, &fops_debug_wl_counters);
	if (!_dentry || IS_ERR(_dentry)) {
		WL_ERR(("failed to create debug_wl_counters debug file\n"));
		wl_free_debugfs(cfg);
	}
#endif

#ifdef HW_OTP_PATCH
	_dentry = debugfs_create_file("debug_otp", S_IRUSR | S_IWUSR,
		cfg->debugfs, cfg, &fops_debug_otp);
	if (!_dentry || IS_ERR(_dentry)) {
		WL_ERR(("failed to create debug_otp debug file\n"));
		wl_free_debugfs(cfg);
	}
#endif

exit:
	return err;
}
static s32 wl_free_debugfs(struct bcm_cfg80211 *cfg)
{
	if (!cfg)
		return -EINVAL;
	if (cfg->debugfs)
		debugfs_remove_recursive(cfg->debugfs);
	cfg->debugfs = NULL;
	return 0;
}
#endif /* DEBUGFS_CFG80211 */

struct device *wl_cfg80211_get_parent_dev(void)
{
	return cfg80211_parent_dev;
}

void wl_cfg80211_set_parent_dev(void *dev)
{
	cfg80211_parent_dev = dev;
}

static void wl_cfg80211_clear_parent_dev(void)
{
	cfg80211_parent_dev = NULL;
}

void get_primary_mac(struct bcm_cfg80211 *cfg, struct ether_addr *mac)
{
	wldev_iovar_getbuf_bsscfg(bcmcfg_to_prmry_ndev(cfg), "cur_etheraddr", NULL,
		0, cfg->ioctl_buf, WLC_IOCTL_SMLEN, 0, &cfg->ioctl_buf_sync);
	memcpy(mac->octet, cfg->ioctl_buf, ETHER_ADDR_LEN);
}
static bool check_dev_role_integrity(struct bcm_cfg80211 *cfg, u32 dev_role)
{
	dhd_pub_t *dhd = (dhd_pub_t *)(cfg->pub);
	if (((dev_role == NL80211_IFTYPE_AP) &&
		!(dhd->op_mode & DHD_FLAG_HOSTAP_MODE)) ||
		((dev_role == NL80211_IFTYPE_P2P_GO) &&
		!(dhd->op_mode & DHD_FLAG_P2P_GO_MODE)))
	{
#ifndef  BRCM_RSDB
		WL_ERR(("device role select failed\n"));
#else
		WL_ERR(("device role select failed role:%d op_mode:%d \n", dev_role, dhd->op_mode));
#endif
		return false;
	}
	return true;
}

int wl_cfg80211_do_driver_init(struct net_device *net)
{
	struct bcm_cfg80211 *cfg = *(struct bcm_cfg80211 **)netdev_priv(net);

	if (!cfg || !cfg->wdev)
		return -EINVAL;

	if (dhd_do_driver_init(cfg->wdev->netdev) < 0)
		return -1;

	return 0;
}

void wl_cfg80211_enable_trace(bool set, u32 level)
{
	if (set)
		wl_dbg_level = level & WL_DBG_LEVEL;
	else
		wl_dbg_level |= (WL_DBG_LEVEL & level);
}
#if defined(WL_SUPPORT_BACKPORTED_KPATCHES) || (LINUX_VERSION_CODE >= KERNEL_VERSION(3, \
	2, 0))
static s32
wl_cfg80211_mgmt_tx_cancel_wait(struct wiphy *wiphy,
	bcm_struct_cfgdev *cfgdev, u64 cookie)
{
	/* CFG80211 checks for tx_cancel_wait callback when ATTR_DURATION
	 * is passed with CMD_FRAME. This callback is supposed to cancel
	 * the OFFCHANNEL Wait. Since we are already taking care of that
	 *  with the tx_mgmt logic, do nothing here.
	 */

	return 0;
}
#endif /* WL_SUPPORT_BACKPORTED_PATCHES || KERNEL >= 3.2.0 */

#ifdef WL11U
bcm_tlv_t *
wl_cfg80211_find_interworking_ie(u8 *parse, u32 len)
{
	bcm_tlv_t *ie;

	while ((ie = bcm_parse_tlvs(parse, (u32)len, DOT11_MNG_INTERWORKING_ID))) {
			return (bcm_tlv_t *)ie;
	}
	return NULL;
}


static s32
wl_cfg80211_add_iw_ie(struct bcm_cfg80211 *cfg, struct net_device *ndev, s32 bssidx, s32 pktflag,
            uint8 ie_id, uint8 *data, uint8 data_len)
{
	s32 err = BCME_OK;
	s32 buf_len;
	s32 iecount;
	ie_setbuf_t *ie_setbuf;

	if (ie_id != DOT11_MNG_INTERWORKING_ID)
		return BCME_UNSUPPORTED;

#ifdef BCM_PATCH_SECURITY_2017_04
	if (data_len > IW_IES_MAX_BUF_LEN) {
		WL_ERR(("wrong data_len:%d\n", data_len));
		return BCME_BADARG;
	}
#endif

	/* Validate the pktflag parameter */
	if ((pktflag & ~(VNDR_IE_BEACON_FLAG | VNDR_IE_PRBRSP_FLAG |
	            VNDR_IE_ASSOCRSP_FLAG | VNDR_IE_AUTHRSP_FLAG |
	            VNDR_IE_PRBREQ_FLAG | VNDR_IE_ASSOCREQ_FLAG|
	            VNDR_IE_CUSTOM_FLAG))) {
		WL_ERR(("cfg80211 Add IE: Invalid packet flag 0x%x\n", pktflag));
		return -1;
	}

	/* use VNDR_IE_CUSTOM_FLAG flags for none vendor IE . currently fixed value */
	pktflag = htod32(pktflag);

	buf_len = sizeof(ie_setbuf_t) + data_len - 1;
	ie_setbuf = (ie_setbuf_t *) kzalloc(buf_len, GFP_KERNEL);

	if (!ie_setbuf) {
		WL_ERR(("Error allocating buffer for IE\n"));
		return -ENOMEM;
	}

	if (cfg->iw_ie_len == data_len && !memcmp(cfg->iw_ie, data, data_len)) {
		WL_INFORM(("Previous IW IE is equals to current IE\n"));
		err = BCME_OK;
		goto exit;
	}

	strncpy(ie_setbuf->cmd, "add", VNDR_IE_CMD_LEN - 1);
	ie_setbuf->cmd[VNDR_IE_CMD_LEN - 1] = '\0';

	/* Buffer contains only 1 IE */
	iecount = htod32(1);
	memcpy((void *)&ie_setbuf->ie_buffer.iecount, &iecount, sizeof(int));
	memcpy((void *)&ie_setbuf->ie_buffer.ie_list[0].pktflag, &pktflag, sizeof(uint32));

	/* Now, add the IE to the buffer */
	ie_setbuf->ie_buffer.ie_list[0].ie_data.id = ie_id;

	/* if already set with previous values, delete it first */
	if (cfg->iw_ie_len != 0) {
		WL_DBG(("Different IW_IE was already set. clear first\n"));

		ie_setbuf->ie_buffer.ie_list[0].ie_data.len = 0;

		err = wldev_iovar_setbuf_bsscfg(ndev, "ie", ie_setbuf, buf_len,
			cfg->ioctl_buf, WLC_IOCTL_MAXLEN, bssidx, &cfg->ioctl_buf_sync);

		if (err != BCME_OK)
			goto exit;
	}

	ie_setbuf->ie_buffer.ie_list[0].ie_data.len = data_len;
	memcpy((uchar *)&ie_setbuf->ie_buffer.ie_list[0].ie_data.data[0], data, data_len);

	err = wldev_iovar_setbuf_bsscfg(ndev, "ie", ie_setbuf, buf_len,
		cfg->ioctl_buf, WLC_IOCTL_MAXLEN, bssidx, &cfg->ioctl_buf_sync);

	if (err == BCME_OK) {
		memcpy(cfg->iw_ie, data, data_len);
		cfg->iw_ie_len = data_len;
		cfg->wl11u = TRUE;

		err = wldev_iovar_setint_bsscfg(ndev, "grat_arp", 1, bssidx);
	}

exit:
	if (ie_setbuf)
		kfree(ie_setbuf);
	return err;
}
#endif /* WL11U */
#ifdef  BRCM_RSDB
#ifdef WL_HOST_BAND_MGMT
s32
wl_cfg80211_set_band(struct net_device *ndev, int band)
{
	struct bcm_cfg80211 *cfg = g_bcm_cfg;
	int ret = 0;
	char ioctl_buf[50];

	if ((band < WLC_BAND_AUTO) || (band > WLC_BAND_2G)) {
		WL_ERR(("Invalid band\n"));
		return -EINVAL;
	}

	if ((ret = wldev_iovar_setbuf(ndev, "roam_band", &band,
		sizeof(int), ioctl_buf, sizeof(ioctl_buf), NULL)) < 0) {
		WL_ERR(("seting roam_band failed code=%d\n", ret));
		return ret;
	}

	WL_DBG(("Setting band to %d\n", band));
	cfg->curr_band = band;

	return 0;
}
#endif /* WL_HOST_BAND_MGMT */

#if 0
s32
wl_cfg80211_dfs_ap_move(struct net_device *ndev, char *data, char *command, int total_len)
{
	struct bcm_cfg80211 *cfg = g_bcm_cfg;
	char ioctl_buf[50];
	int err = 0;
	uint32 val = 0;
	chanspec_t chanspec = 0;
	int abort;
	int bytes_written = 0;
	wl_dfs_ap_move_status_t *status;
	char chanbuf[CHANSPEC_STR_LEN];
	const char *dfs_state_str[DFS_SCAN_S_MAX] = {
		"Radar Free On Channel",
		"Radar Found On Channel",
		"Radar Scan In Progress",
		"Radar Scan Aborted",
		"RSDB Mode switch in Progress For Scan"
	};
	if (ndev->ieee80211_ptr->iftype != NL80211_IFTYPE_AP) {
		bytes_written = snprintf(command, total_len, "AP is not UP\n");
		return bytes_written;
	}
	if (!*data) {
		if ((err = wldev_iovar_getbuf(ndev, "dfs_ap_move", NULL, 0,
				cfg->ioctl_buf, WLC_IOCTL_MAXLEN,  &cfg->ioctl_buf_sync))) {
			WL_ERR(("setting dfs_ap_move failed with err=%d \n", err));
			return err;
		}
		status = (wl_dfs_ap_move_status_t *)cfg->ioctl_buf;

		if (status->version != WL_DFS_AP_MOVE_VERSION) {
			err = BCME_UNSUPPORTED;
			WL_ERR(("err=%d version=%d\n", err, status->version));
			return err;
		}

		if (status->move_status != (int8) DFS_SCAN_S_IDLE) {
			chanspec = wl_chspec_driver_to_host(status->chanspec);
			if (chanspec != 0 && chanspec != INVCHANSPEC) {
				wf_chspec_ntoa(chanspec, chanbuf);
				bytes_written = snprintf(command, total_len,
					"AP Target Chanspec %s (0x%x)\n", chanbuf, chanspec);
			}
			bytes_written += snprintf(command + bytes_written, total_len,
					 "%s\n", dfs_state_str[status->move_status]);
			return bytes_written;
		} else {
			bytes_written = snprintf(command, total_len, "dfs AP move in IDLE state\n");
			return bytes_written;
		}

	}

	abort = bcm_atoi(data);
	if (abort == -1) {
		if ((err = wldev_iovar_setbuf(ndev, "dfs_ap_move", &abort,
				sizeof(int), ioctl_buf, sizeof(ioctl_buf), NULL)) < 0) {
			WL_ERR(("seting dfs_ap_move failed with err %d\n", err));
			return err;
		}
	} else {
		chanspec = wf_chspec_aton(data);
		if (chanspec != 0) {
			val = wl_chspec_host_to_driver(chanspec);
			if (val != INVCHANSPEC) {
				if ((err = wldev_iovar_setbuf(ndev, "dfs_ap_move", &val,
					sizeof(int), ioctl_buf, sizeof(ioctl_buf), NULL)) < 0) {
					WL_ERR(("seting dfs_ap_move failed with err %d\n", err));
					return err;
				}
				WL_DBG((" set dfs_ap_move successfull"));
			} else {
				err = BCME_USAGE_ERROR;
			}
		}
	}
	return err;
}


s32
wl_cfg80211_wbtext_config(struct net_device *ndev, char *data, char *command, int total_len)
{
	uint i = 0;
	struct bcm_cfg80211 *cfg = g_bcm_cfg;
	wl_roam_prof_band_t *rp;
	int err = -EINVAL, bytes_written = 0;
	size_t len = strlen(data);
	int rp_len = 0;
	data[len] = '\0';
	rp = (wl_roam_prof_band_t *) kzalloc(sizeof(*rp)
			* WL_MAX_ROAM_PROF_BRACKETS, GFP_KERNEL);
	if (unlikely(!rp)) {
		WL_ERR(("%s: failed to allocate memory\n", __func__));
		err =  -ENOMEM;
		goto exit;
	}

	rp->ver = WL_MAX_ROAM_PROF_VER;
	if (*data && (!strncmp(data, "b", 1))) {
		rp->band = WLC_BAND_2G;
	} else if (*data && (!strncmp(data, "a", 1))) {
		rp->band = WLC_BAND_5G;
	} else {
		err = snprintf(command, total_len, "Missing band\n");
		goto exit;
	}
	data++;
	rp->len = 0;
	/* Getting roam profile  from fw */
	if ((err = wldev_iovar_getbuf(ndev, "roam_prof", rp, sizeof(*rp),
		cfg->ioctl_buf, WLC_IOCTL_MAXLEN,  &cfg->ioctl_buf_sync))) {
		WL_ERR(("Getting roam_profile failed with err=%d \n", err));
		goto exit;
	}
	memcpy(rp, cfg->ioctl_buf, sizeof(*rp) * WL_MAX_ROAM_PROF_BRACKETS);
	/* roam_prof version get */
	if (rp->ver != WL_MAX_ROAM_PROF_VER) {
		WL_ERR(("bad version (=%d) in return data\n", rp->ver));
		err = -EINVAL;
		goto exit;
	}
	if ((rp->len % sizeof(wl_roam_prof_t)) != 0) {
		WL_ERR(("bad length (=%d) in return data\n", rp->len));
		err = -EINVAL;
		goto exit;
	}

	if (!*data) {
		for (i = 0; i < WL_MAX_ROAM_PROF_BRACKETS; i++) {
			/* printing contents of roam profile data from fw and exits
			 * if code hits any of one of the below condtion. If remaining
			 * length of buffer is less than roam profile size or
			 * if there is no valid entry.
			 */
			if (((i * sizeof(wl_roam_prof_t)) > rp->len) ||
				(rp->roam_prof[i].fullscan_period == 0)) {
				break;
			}
			bytes_written += snprintf(command+bytes_written,
					total_len, "RSSI[%d,%d] CU(trigger:%d%%: duration:%ds)\n",
					rp->roam_prof[i].roam_trigger, rp->roam_prof[i].rssi_lower,
					rp->roam_prof[i].channel_usage,
					rp->roam_prof[i].cu_avg_calc_dur);
		}
		err = bytes_written;
		goto exit;
	} else {
		for (i = 0; i < WL_MAX_ROAM_PROF_BRACKETS; i++) {
			/* reading contents of roam profile data from fw and exits
			 * if code hits any of one of the below condtion, If remaining
			 * length of buffer is less than roam profile size or if there
			 * is no valid entry.
			 */
			if (((i * sizeof(wl_roam_prof_t)) > rp->len) ||
				(rp->roam_prof[i].fullscan_period == 0)) {
				break;
			}
		}
		/* Do not set roam_prof from upper layer if fw doesn't have 2 rows */
		if (i != 2) {
			WL_ERR(("FW must have 2 rows to fill roam_prof\n"));
			err = -EINVAL;
			goto exit;
		}
		/* setting roam profile to fw */
		data++;
		for (i = 0; i < WL_MAX_ROAM_PROF_BRACKETS; i++) {
			rp->roam_prof[i].roam_trigger = simple_strtol(data, &data, 10);
			data++;
			rp->roam_prof[i].rssi_lower = simple_strtol(data, &data, 10);
			data++;
			rp->roam_prof[i].channel_usage = simple_strtol(data, &data, 10);
			data++;
			rp->roam_prof[i].cu_avg_calc_dur = simple_strtol(data, &data, 10);

			rp_len += sizeof(wl_roam_prof_t);
			if (*data == '\0') {
				break;
			}
			data++;
		}
		if (i != 1) {
			WL_ERR(("Only two roam_prof rows supported.\n"));
			err = -EINVAL;
			goto exit;
		}
		rp->len = rp_len;
		if ((err = wldev_iovar_setbuf(ndev, "roam_prof", rp,
				sizeof(*rp), cfg->ioctl_buf, WLC_IOCTL_MEDLEN, NULL)) < 0) {
			WL_ERR(("seting roam_profile failed with err %d\n", err));
		}
	}
exit:
	if (rp) {
		kfree(rp);
	}
	return err;
}

int wl_cfg80211_wbtext_weight_config(struct net_device *ndev, char *data,
		char *command, int total_len)
{
	struct bcm_cfg80211 *cfg = g_bcm_cfg;
	int bytes_written = 0, err = -EINVAL, argc = 0;
	char rssi[5], band[5], weight[5];
	char *endptr = NULL;
	wnm_bss_select_weight_cfg_t *bwcfg;

	bwcfg = kzalloc(sizeof(*bwcfg), GFP_KERNEL);
	if (unlikely(!bwcfg)) {
		WL_ERR(("%s: failed to allocate memory\n", __func__));
		err = -ENOMEM;
		goto exit;
	}
	bwcfg->version =  WNM_BSSLOAD_MONITOR_VERSION;
	bwcfg->type = 0;
	bwcfg->weight = 0;

	argc = sscanf(data, "%s %s %s", rssi, band, weight);

	if (!strcasecmp(rssi, "rssi"))
		bwcfg->type = WNM_BSS_SELECT_TYPE_RSSI;
	else if (!strcasecmp(rssi, "cu"))
		bwcfg->type = WNM_BSS_SELECT_TYPE_CU;
	else {
		/* Usage DRIVER WBTEXT_WEIGHT_CONFIG <rssi/cu> <band> <weight> */
		WL_ERR(("%s: Command usage error\n", __func__));
		goto exit;
	}

	if (!strcasecmp(band, "a"))
		bwcfg->band = WLC_BAND_5G;
	else if (!strcasecmp(band, "b"))
		bwcfg->band = WLC_BAND_2G;
	else if (!strcasecmp(band, "all"))
		bwcfg->band = WLC_BAND_ALL;
	else {
		WL_ERR(("%s: Command usage error\n", __func__));
		goto exit;
	}

	if (argc == 2) {
		/* If there is no data after band, getting wnm_bss_select_weight from fw */
		if (bwcfg->band == WLC_BAND_ALL) {
			WL_ERR(("band option \"all\" is for set only, not get\n"));
			goto exit;
		}
		if ((err = wldev_iovar_getbuf(ndev, "wnm_bss_select_weight", bwcfg,
				sizeof(*bwcfg),
				cfg->ioctl_buf, WLC_IOCTL_MAXLEN,  &cfg->ioctl_buf_sync))) {
			WL_ERR(("Getting wnm_bss_select_weight failed with err=%d \n", err));
			goto exit;
		}
		memcpy(bwcfg, cfg->ioctl_buf, sizeof(*bwcfg));
		bytes_written = snprintf(command, total_len, "%s %s weight = %d\n",
			(bwcfg->type == WNM_BSS_SELECT_TYPE_RSSI) ? "RSSI" : "CU",
			(bwcfg->band == WLC_BAND_2G) ? "2G" : "5G", bwcfg->weight);
		err = bytes_written;
		goto exit;
	} else {
		/* if weight is non integer returns command usage error */
		bwcfg->weight = simple_strtol(weight, &endptr, 0);
		if (*endptr != '\0') {
			WL_ERR(("%s: Command usage error", __func__));
			goto exit;
		}
		/* setting weight for iovar wnm_bss_select_weight to fw */
		if ((err = wldev_iovar_setbuf(ndev, "wnm_bss_select_weight", bwcfg,
				sizeof(*bwcfg),
				cfg->ioctl_buf, WLC_IOCTL_MAXLEN,  &cfg->ioctl_buf_sync))) {
			WL_ERR(("Getting wnm_bss_select_weight failed with err=%d\n", err));
		}
	}
exit:
	if (bwcfg) {
		kfree(bwcfg);
	}
	return err;
}

/* WBTEXT_TUPLE_MIN_LEN_CHECK :strlen(low)+" "+strlen(high)+" "+strlen(factor) */
#define WBTEXT_TUPLE_MIN_LEN_CHECK 5

int wl_cfg80211_wbtext_table_config(struct net_device *ndev, char *data,
	char *command, int total_len)
{
	struct bcm_cfg80211 *cfg = g_bcm_cfg;
	int bytes_written = 0, err = -EINVAL;
	char rssi[5], band[5];
	int btcfg_len = 0, i = 0, parsed_len = 0;
	wnm_bss_select_factor_cfg_t *btcfg;
	size_t slen = strlen(data);
	char *start_addr = NULL;
	data[slen] = '\0';

	btcfg = kzalloc((sizeof(*btcfg) + sizeof(*btcfg) *
			WL_FACTOR_TABLE_MAX_LIMIT), GFP_KERNEL);
	if (unlikely(!btcfg)) {
		WL_ERR(("%s: failed to allocate memory\n", __func__));
		err = -ENOMEM;
		goto exit;
	}

	btcfg->version = WNM_BSS_SELECT_FACTOR_VERSION;
	btcfg->band = WLC_BAND_AUTO;
	btcfg->type = 0;
	btcfg->count = 0;

	sscanf(data, "%s %s", rssi, band);

	if (!strcasecmp(rssi, "rssi")) {
		btcfg->type = WNM_BSS_SELECT_TYPE_RSSI;
	}
	else if (!strcasecmp(rssi, "cu")) {
		btcfg->type = WNM_BSS_SELECT_TYPE_CU;
	}
	else {
		WL_ERR(("%s: Command usage error\n", __func__));
		goto exit;
	}

	if (!strcasecmp(band, "a")) {
		btcfg->band = WLC_BAND_5G;
	}
	else if (!strcasecmp(band, "b")) {
		btcfg->band = WLC_BAND_2G;
	}
	else if (!strcasecmp(band, "all")) {
		btcfg->band = WLC_BAND_ALL;
	}
	else {
		WL_ERR(("%s: Command usage, Wrong band\n", __func__));
		goto exit;
	}

	if ((slen - 1) == (strlen(rssi) + strlen(band))) {
		/* Getting factor table using iovar 'wnm_bss_select_table' from fw */
		if ((err = wldev_iovar_getbuf(ndev, "wnm_bss_select_table", btcfg,
				sizeof(*btcfg),
				cfg->ioctl_buf, WLC_IOCTL_MAXLEN,  &cfg->ioctl_buf_sync))) {
			WL_ERR(("Getting wnm_bss_select_table failed with err=%d \n", err));
			goto exit;
		}
		memcpy(btcfg, cfg->ioctl_buf, sizeof(*btcfg));
		memcpy(btcfg, cfg->ioctl_buf, (btcfg->count+1) * sizeof(*btcfg));

		bytes_written += snprintf(command + bytes_written, total_len,
					"No of entries in table: %d\n", btcfg->count);
		bytes_written += snprintf(command + bytes_written, total_len, "%s factor table\n",
				(btcfg->type == WNM_BSS_SELECT_TYPE_RSSI) ? "RSSI" : "CU");
		bytes_written += snprintf(command + bytes_written, total_len,
					"low\thigh\tfactor\n");
		for (i = 0; i <= btcfg->count-1; i++) {
			bytes_written += snprintf(command + bytes_written, total_len,
				"%d\t%d\t%d\n", btcfg->params[i].low, btcfg->params[i].high,
				btcfg->params[i].factor);
		}
		err = bytes_written;
		goto exit;
	} else {
		memset(btcfg->params, 0, sizeof(*btcfg) * WL_FACTOR_TABLE_MAX_LIMIT);
		data += (strlen(rssi) + strlen(band) + 2);
		start_addr = data;
		slen = slen - (strlen(rssi) + strlen(band) + 2);
		for (i = 0; i < WL_FACTOR_TABLE_MAX_LIMIT; i++) {
			if (parsed_len + WBTEXT_TUPLE_MIN_LEN_CHECK <= slen) {
				btcfg->params[i].low = simple_strtol(data, &data, 10);
				data++;
				btcfg->params[i].high = simple_strtol(data, &data, 10);
				data++;
				btcfg->params[i].factor = simple_strtol(data, &data, 10);
				btcfg->count++;
				if (*data == '\0') {
					break;
				}
				data++;
				parsed_len = data - start_addr;
			} else {
				WL_ERR(("%s:Command usage:less no of args\n", __func__));
				goto exit;
			}
		}
		btcfg_len = sizeof(*btcfg) + ((btcfg->count) * sizeof(*btcfg));
		if ((err = wldev_iovar_setbuf(ndev, "wnm_bss_select_table", btcfg, btcfg_len,
				cfg->ioctl_buf, WLC_IOCTL_MEDLEN, NULL)) < 0) {
			WL_ERR(("seting wnm_bss_select_table failed with err %d\n", err));
			goto exit;
		}
	}
exit:
	if (btcfg) {
		kfree(btcfg);
	}
	return err;
}

s32
wl_cfg80211_wbtext_delta_config(struct net_device *ndev, char *data, char *command, int total_len)
{
	uint i = 0;
	struct bcm_cfg80211 *cfg = g_bcm_cfg;
	int err = -EINVAL, bytes_written = 0, argc = 0, val, len = 0;
	char delta[5], band[5], *endptr = NULL;
	wl_roam_prof_band_t *rp;

	rp = (wl_roam_prof_band_t *) kzalloc(sizeof(*rp)
			* WL_MAX_ROAM_PROF_BRACKETS, GFP_KERNEL);
	if (unlikely(!rp)) {
		WL_ERR(("%s: failed to allocate memory\n", __func__));
		err = -ENOMEM;
		goto exit;
	}

	argc = sscanf(data, "%s %s", band, delta);
	if (!strcasecmp(band, "a"))
		rp->band = WLC_BAND_5G;
	else if (!strcasecmp(band, "b"))
		rp->band = WLC_BAND_2G;
	else {
		WL_ERR(("%s: Missing band\n", __func__));
		goto exit;
	}
	/* Getting roam profile  from fw */
	if ((err = wldev_iovar_getbuf(ndev, "roam_prof", rp, sizeof(*rp),
		cfg->ioctl_buf, WLC_IOCTL_MAXLEN,  &cfg->ioctl_buf_sync))) {
		WL_ERR(("Getting roam_profile failed with err=%d \n", err));
		goto exit;
	}
	memcpy(rp, cfg->ioctl_buf, sizeof(wl_roam_prof_band_t));
	if (rp->ver != WL_MAX_ROAM_PROF_VER) {
		WL_ERR(("bad version (=%d) in return data\n", rp->ver));
		err = -EINVAL;
		goto exit;
	}
	if ((rp->len % sizeof(wl_roam_prof_t)) != 0) {
		WL_ERR(("bad length (=%d) in return data\n", rp->len));
		err = -EINVAL;
		goto exit;
	}

	if (argc == 2) {
		/* if delta is non integer returns command usage error */
		val = simple_strtol(delta, &endptr, 0);
		if (*endptr != '\0') {
			WL_ERR(("%s: Command usage error", __func__));
			goto exit;
		}
		for (i = 0; i < WL_MAX_ROAM_PROF_BRACKETS; i++) {
		/*
		 * Checking contents of roam profile data from fw and exits
		 * if code hits below condtion. If remaining length of buffer is
		 * less than roam profile size or if there is no valid entry.
		 */
			if (((i * sizeof(wl_roam_prof_t)) > rp->len) ||
				(rp->roam_prof[i].fullscan_period == 0)) {
				break;
			}
			if (rp->roam_prof[i].channel_usage != 0) {
				rp->roam_prof[i].roam_delta = val;
			}
			len += sizeof(wl_roam_prof_t);
		}
	}
	else {
		if (rp->roam_prof[i].channel_usage != 0) {
			bytes_written = snprintf(command, total_len,
				"%s Delta %d\n", (rp->band == WLC_BAND_2G) ? "2G" : "5G",
				rp->roam_prof[0].roam_delta);
		}
		err = bytes_written;
		goto exit;
	}
	rp->len = len;
	if ((err = wldev_iovar_setbuf(ndev, "roam_prof", rp,
			sizeof(*rp), cfg->ioctl_buf, WLC_IOCTL_MEDLEN, NULL)) < 0) {
		WL_ERR(("seting roam_profile failed with err %d\n", err));
	}
exit :
	if (rp) {
		kfree(rp);
	}
	return err;
}
#endif
#endif /* BRCM_RSDB */

#if defined(DHCP_SCAN_SUPPRESS)
static void wl_cfg80211_scan_supp_timerfunc(ulong data)
{
	struct bcm_cfg80211 *cfg = (struct bcm_cfg80211 *)data;

	WL_DBG(("Enter \n"));
	schedule_work(&cfg->wlan_work);
}

int wl_cfg80211_scan_suppress(struct net_device *dev, int suppress)
{
	int ret = 0;
	struct wireless_dev *wdev;
	struct bcm_cfg80211 *cfg;
	if (!dev || ((suppress != 0) && (suppress != 1))) {
		ret = -EINVAL;
		goto exit;
	}
	wdev = ndev_to_wdev(dev);
	if (!wdev) {
		ret = -EINVAL;
		goto exit;
	}
	cfg = (struct bcm_cfg80211 *)wiphy_priv(wdev->wiphy);
	if (!cfg) {
		ret = -EINVAL;
		goto exit;
	}

	if (suppress == cfg->scan_suppressed) {
		WL_DBG(("No change in scan_suppress state. Ignoring cmd..\n"));
		return 0;
	}

	if (timer_pending(&cfg->scan_supp_timer))
		del_timer_sync(&cfg->scan_supp_timer);

	if ((ret = wldev_ioctl(dev, WLC_SET_SCANSUPPRESS,
		&suppress, sizeof(int), true)) < 0) {
		WL_ERR(("Scan suppress setting failed ret:%d \n", ret));
	} else {
		WL_DBG(("Scan suppress %s \n", suppress ? "Enabled" : "Disabled"));
		cfg->scan_suppressed = suppress;
	}

	/* If scan_suppress is set, Start a timer to monitor it (just incase) */
	if (cfg->scan_suppressed) {
		if (ret) {
			WL_ERR(("Retry scan_suppress reset at a later time \n"));
			mod_timer(&cfg->scan_supp_timer,
				jiffies + msecs_to_jiffies(WL_SCAN_SUPPRESS_RETRY));
		} else {
			WL_DBG(("Start wlan_timer to clear of scan_suppress \n"));
			mod_timer(&cfg->scan_supp_timer,
				jiffies + msecs_to_jiffies(WL_SCAN_SUPPRESS_TIMEOUT));
		}
	}
exit:
	return ret;
}
#endif /* DHCP_SCAN_SUPPRESS */

int wl_cfg80211_scan_stop(bcm_struct_cfgdev *cfgdev)
{
	struct bcm_cfg80211 *cfg = NULL;
	struct net_device *ndev = NULL;
	unsigned long flags;
	int clear_flag = 0;
	int ret = 0;

	WL_TRACE(("Enter\n"));

	cfg = g_bcm_cfg;
	if (!cfg)
		return -EINVAL;

	ndev = cfgdev_to_wlc_ndev(cfgdev, cfg);

	spin_lock_irqsave(&cfg->cfgdrv_lock, flags);
#ifdef WL_CFG80211_P2P_DEV_IF
	if (cfg->scan_request && cfg->scan_request->wdev == cfgdev) {
#else
	if (cfg->scan_request && cfg->scan_request->dev == cfgdev) {
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 8, 0))
		struct cfg80211_scan_info info = {
			.aborted = true,
		};
		cfg80211_scan_done(cfg->scan_request, &info);
#else
		cfg80211_scan_done(cfg->scan_request, true);
#endif
		cfg->scan_request = NULL;
		clear_flag = 1;
	}
	spin_unlock_irqrestore(&cfg->cfgdrv_lock, flags);

	if (clear_flag)
		wl_clr_drv_status(cfg, SCANNING, ndev);

	return ret;
}
#ifndef  BRCM_RSDB
bool wl_cfg80211_is_vsdb_mode(void)
{
	return (g_bcm_cfg && g_bcm_cfg->vsdb_mode);
}
#else
bool wl_cfg80211_is_concurrent_mode(void)
{
	if ((g_bcm_cfg) && (wl_get_drv_status_all(g_bcm_cfg, CONNECTED) > 1)) {
		return true;
	} else {
		return false;
	}
}
#endif /* BRCM_RSDB */
void* wl_cfg80211_get_dhdp()
{
	struct bcm_cfg80211 *cfg = g_bcm_cfg;

	return cfg->pub;
}

bool wl_cfg80211_is_p2p_active(void)
{
	return (g_bcm_cfg && g_bcm_cfg->p2p);
}
#ifdef  BRCM_RSDB
bool wl_cfg80211_is_roam_offload(void)
{
	return (g_bcm_cfg && g_bcm_cfg->roam_offload);
}

bool wl_cfg80211_is_event_from_connected_bssid(const wl_event_msg_t *e, int ifidx)
{
	dhd_pub_t *dhd = NULL;
	struct net_device *ndev = NULL;
	u8 *curbssid = NULL;

	dhd = (dhd_pub_t *)(g_bcm_cfg->pub);

	if (dhd) {
		ndev = dhd_idx2net(dhd, ifidx);
	}

	if (!dhd || !ndev) {
		return false;
	}

	curbssid = wl_read_prof(g_bcm_cfg, ndev, WL_PROF_BSSID);

	return memcmp(curbssid, &e->addr, ETHER_ADDR_LEN) == 0;
}
#endif
static void wl_cfg80211_work_handler(struct work_struct * work)
{
	struct bcm_cfg80211 *cfg = NULL;
	struct net_info *iter, *next;
	s32 err = BCME_OK;
	s32 pm = PM_FAST;
#ifndef  BRCM_RSDB
	cfg = container_of(work, struct bcm_cfg80211, pm_enable_work.work);
	WL_DBG(("Enter \n"));
	if (cfg->pm_enable_work_on) {
		cfg->pm_enable_work_on = false;
		for_each_ndev(cfg, iter, next) {
			if (!wl_get_drv_status(cfg, CONNECTED, iter->ndev) ||
				(wl_get_mode_by_netdev(cfg, iter->ndev) != WL_MODE_BSS))
				continue;
			if (iter->ndev) {
				if ((err = wldev_ioctl(iter->ndev, WLC_SET_PM,
					&pm, sizeof(pm), true)) != 0) {
					if (err == -ENODEV)
						WL_DBG(("%s:netdev not ready\n", iter->ndev->name));
					else
						WL_ERR(("%s:error (%d)\n", iter->ndev->name, err));
				} else
					wl_cfg80211_update_power_mode(iter->ndev);
			}
		}
	}
#if defined(DHCP_SCAN_SUPPRESS)
	else if (cfg->scan_suppressed) {
		/* There is pending scan_suppress. Clean it */
		WL_ERR(("Clean up from timer after %d msec\n", WL_SCAN_SUPPRESS_TIMEOUT));
		wl_cfg80211_scan_suppress(bcmcfg_to_prmry_ndev(cfg), 0);
	}
#endif /* DHCP_SCAN_SUPPRESS */
#else /* BRCM_RSDB */
	BCM_SET_CONTAINER_OF(cfg, work, struct bcm_cfg80211, pm_enable_work.work);
	WL_DBG(("Enter \n"));
#if defined(STRICT_GCC_WARNINGS) && defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == \
	4 && __GNUC_MINOR__ >= 6))
_Pragma("GCC diagnostic push")
_Pragma("GCC diagnostic ignored \"-Wcast-qual\"")
#endif
	for_each_ndev(cfg, iter, next) {
		/* p2p discovery iface ndev could be null */
		if (iter->ndev) {
			if (!wl_get_drv_status(cfg, CONNECTED, iter->ndev) ||
				(wl_get_mode_by_netdev(cfg, iter->ndev) != WL_MODE_BSS &&
				wl_get_mode_by_netdev(cfg, iter->ndev) != WL_MODE_IBSS))
				continue;
			if (iter->ndev) {
				if ((err = wldev_ioctl(iter->ndev, WLC_SET_PM,
						&pm, sizeof(pm), true)) != 0) {
					if (err == -ENODEV)
						WL_DBG(("%s:netdev not ready\n",
							iter->ndev->name));
					else
						WL_ERR(("%s:error (%d)\n",
							iter->ndev->name, err));
				} else
					wl_cfg80211_update_power_mode(iter->ndev);
			}
		}
	}
#if defined(STRICT_GCC_WARNINGS) && defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == \
	4 && __GNUC_MINOR__ >= 6))
	_Pragma("GCC diagnostic pop")
#endif
	DHD_OS_WAKE_UNLOCK(cfg->pub);
#endif /* BRCM_RSDB */
}

u8
wl_get_action_category(void *frame, u32 frame_len)
{
	u8 category;
	u8 *ptr = (u8 *)frame;
	if (frame == NULL)
		return DOT11_ACTION_CAT_ERR_MASK;
	if (frame_len < DOT11_ACTION_HDR_LEN)
		return DOT11_ACTION_CAT_ERR_MASK;
	category = ptr[DOT11_ACTION_CAT_OFF];
	WL_INFORM(("Action Category: %d\n", category));
	return category;
}

int
wl_get_public_action(void *frame, u32 frame_len, u8 *ret_action)
{
	u8 *ptr = (u8 *)frame;
	if (frame == NULL || ret_action == NULL)
		return BCME_ERROR;
	if (frame_len < DOT11_ACTION_HDR_LEN)
		return BCME_ERROR;
	if (DOT11_ACTION_CAT_PUBLIC != wl_get_action_category(frame, frame_len))
		return BCME_ERROR;
	*ret_action = ptr[DOT11_ACTION_ACT_OFF];
	WL_INFORM(("Public Action : %d\n", *ret_action));
	return BCME_OK;
}

#ifdef WRONG_ACTION_PATCH
s32
wl_cfg80211_set_nmode(struct net_device *ndev, u8 nmode)
{
    s32 err;
    s32 val = 0;
#ifdef HW_PATCH_SET_NMODE_RETRY_WLC_UP
    s32 retry = 0;
    struct bcm_cfg80211 *wl = g_bcm_cfg;

    WL_ERR(("wl_cfg80211_set_nmode %d\n", nmode));
    if (NULL == wl) {
        WL_ERR(("wl_cfg80211_set_nmode error for wl pointer NULL.\n"));
        return BCME_ERROR;
    }

    mutex_lock(&wl->usr_sync);

    val = 1;
    err = wldev_ioctl(ndev, WLC_DOWN, &val, sizeof(s32), true);
    if (err < 0) {
        WL_ERR(("WLC_DOWN error %d, retry down.\n", err));
        err = wldev_ioctl(ndev, WLC_DOWN, &val, sizeof(s32), true);
        if (err < 0) {
            WL_ERR(("WLC_DOWN retry error %d, quit.\n", err));
            goto exit;
        }
    }

    if ((err = wldev_iovar_setint(ndev, "nmode", nmode)) != BCME_OK) {
        WL_ERR((" Failed to set nmode %d, error %d\n", nmode, err));
        goto exit;
    }

    val = 1;
    err = wldev_ioctl(ndev, WLC_UP, &val, sizeof(s32), true);
    if (err < 0) {
        WL_ERR(("WLC_UP error %d\n", err));
    }

exit:
    val = 0; //init val to 0
    /* WLC_GET_UP maybe get 'UP' value if chip is txing or rxing
       maybe get 'DOWN' if chip not txing or rxing (even loop 50 times is not enough)
       or get 'DOWN' if chip is DOWN
    */
    err = wldev_ioctl(ndev, WLC_GET_UP, &val, sizeof(s32), false);
    WL_ERR(("WLC_GET_UP = %d, err = %d.\n", val, err));
    if (!val) {
        WL_ERR(("wl is not up, force it up!\n"));
        retry = 3;
        do {
            val = 1;
            err = wldev_ioctl(ndev, WLC_UP, &val, sizeof(s32), true);
            WL_ERR(("set WLC_UP = %d, err = %d, remain times = %d\n", val, err, retry));
        } while (err && (retry-- > 0));
    }

    mutex_unlock(&wl->usr_sync);
    WL_ERR(("wl_cfg80211_set_nmode %d exit with err = %d.\n", nmode, err));

#else
	WL_ERR(("wl_cfg80211_set_nmode %d\n", nmode));
	val = 1;
	err = wldev_ioctl(ndev, WLC_DOWN, &val, sizeof(s32), true);
	if (err < 0) {
		WL_ERR(("WLC_DOWN error %d\n", err));
		return err;
	}

	if ((err = wldev_iovar_setint(ndev, "nmode", nmode)) != BCME_OK) {
			WL_ERR((" Failed to set nmode %d, error %d\n", nmode, err));
			return err;
	}		

	err = wldev_ioctl(ndev, WLC_UP, &val, sizeof(s32), true);
	if (err < 0) {
		WL_ERR(("WLC_UP error %d\n", err));
		return err;
	}
#endif

	return 0;
}

static bool
wl_cfg80211_get_nmode(struct net_device *ndev)
{
    s32 err;
    s32 nmode;
    
	err = wldev_iovar_getint(ndev, "nmode", &nmode);
	if (err < 0) {
		WL_ERR(("nmode get error %d\n", err));
		return err;
	}

	return (nmode == 1? TRUE: FALSE);
}


#define WLAN_STATUS_ASSOC_DENIED_NO_HT 27
static s32
wl_check_assoc_status(struct bcm_cfg80211 *wl, struct net_device *ndev,
	const wl_event_msg_t *e)
{
	u32 reason = ntoh32(e->reason);
	dhd_pub_t *dhd;

	dhd = (dhd_pub_t *)(wl->pub);

    //it's for g mode conect nmode only ap, need check if recovery nmode
    if (reason == WLAN_STATUS_ASSOC_DENIED_RATES ||
          reason == WLAN_STATUS_ASSOC_DENIED_NO_HT) {
        WL_ERR(("%s enter %d \n", __FUNCTION__, reason));
        if (FALSE == dhd->nmode_disable && FALSE == wl_cfg80211_get_nmode(ndev)) {
            wl_cfg80211_set_nmode(ndev, 1);
            dhd->nonly_ap = TRUE;
        }
    }
    
	return 0;
}

static s32 
wl_prune_event_handler(struct bcm_cfg80211 *wl, bcm_struct_cfgdev *cfgdev,
	const wl_event_msg_t *e, void *data)
{
	struct net_device *ndev = NULL;
	u32 reason = ntoh32(e->reason);
	dhd_pub_t *dhd;

	dhd = (dhd_pub_t *)(wl->pub);	

    ndev = cfgdev_to_wlc_ndev(cfgdev, wl);
    // only check for the wlan0
    if (ndev != bcmcfg_to_prmry_ndev(wl)) {
        WL_ERR(("%s skipped, only handler for primary dev \n", __FUNCTION__));
        return 0;
    }

    WL_ERR(("%s reason:%d, n only:%d, flag:%d \n", __FUNCTION__,
            reason, dhd->nonly_ap, dhd->wrong_action_flag));
    // recevied wrong action from AP
	if(reason == WLC_E_PRUNE_WRONG_ACTION && FALSE == dhd->nonly_ap
            && FALSE == dhd->wrong_action_flag) {
    	wlc_ssid_t *event_ssid = (wlc_ssid_t *)data;

        //update the last fail ssid
        dhd->last_fail_ssid.SSID_len = event_ssid->SSID_len;
        memcpy(&dhd->last_fail_ssid.SSID, &event_ssid->SSID, event_ssid->SSID_len);
        
		//update the last fail bssid
        memcpy(&dhd->last_fail_bssid, &e->addr ,ETHER_ADDR_LEN);
        /*
        ndev = cfgdev_to_wlc_ndev(cfgdev, wl);
        if (TRUE == wl_cfg80211_get_nmode(ndev)) {
            WL_ERR(("%s disable nmode\n", __FUNCTION__));
            wl_cfg80211_set_nmode(ndev, 0);
        }
        */
         dhd->wrong_action_flag = TRUE;
#ifdef BCM_PATCH_FOR_AMPDU_TEAR_DOWN
    }else if (WLC_E_PRUNE_AMPDU_TEARDOWN == reason) {
        u32 *para_t = NULL;
        u32 para = 0;
        int initiator = 0;
        int recv_flag = 0;
        int tid = 0;
        if (data == NULL) {
            WL_ERR(("%s, AMPDU_TEARDOWN, firmware give wrong param.\n", __FUNCTION__));
            return 0;
        }

        para_t = (u32 *)data;
        para = *para_t;
	/* use bit 12-15 to indicates the tid */
        tid = (para >> 12) & 0x0F;
        /*use bit 11 to indicates whether the originator of ampdu is the one who send delba frame or not*/
        initiator = (para >> 11) & 0x01;
        /*use bit 10 to indicates the delba frame is recived from ap or not*/
        recv_flag = (para >> 10) & 0x01;

        WL_ERR(("%s, AMPDU_TEARDOWN, the delba frame is send by %s, the originator of ampdu is %s. Tid = %d\n",
            __FUNCTION__, recv_flag? "AP" : "PHONE", (recv_flag^initiator)? "PHONE":"AP", tid));
#endif
    }
    return 0;
}

static s32 
wl_clear_wrong_action_handle(struct bcm_cfg80211 *wl, struct net_device *cfgdev, wl_extjoin_params_t *ext_join_params)
{
	struct net_device *ndev = NULL;
	dhd_pub_t *dhd;

	dhd = (dhd_pub_t *)(wl->pub);	
	dhd->nonly_ap = FALSE;
    dhd->wrong_action_flag = FALSE;
    ndev = cfgdev;
    // only check for the wlan0
    if (ndev != bcmcfg_to_prmry_ndev(wl)) {
        WL_ERR(("%s skipped, only clear for primary dev \n", __FUNCTION__));
        return 0;
    }

    // check if n mode disabled
   	if(FALSE == dhd->nmode_disable && FALSE == wl_cfg80211_get_nmode(ndev)) {

        // check if the ssid is not the same
        if (dhd->last_fail_ssid.SSID_len != ext_join_params->ssid.SSID_len ||
              memcmp(&dhd->last_fail_ssid.SSID, &ext_join_params->ssid.SSID, dhd->last_fail_ssid.SSID_len ) != 0) {
                WL_ERR(("%s re enable nmode\n", __FUNCTION__));	
            	wl_cfg80211_set_nmode(ndev, 1);
    	}
    }
    
    return 0;
}

int wl_get_wrong_action_flag(void) {
    bool nmode_enable;
    dhd_pub_t *dhd;
    struct net_device *ndev;
    struct bcm_cfg80211 *wl = g_bcm_cfg;

    if(wl && wl->pub) {
        dhd = (dhd_pub_t *)(wl->pub);
        if(dhd->busstate == DHD_BUS_DOWN) {
            WL_ERR(("%s bus is down, do nothing\n", __func__));
            return 0;
        }
        ndev = bcmcfg_to_prmry_ndev(wl);
        nmode_enable = wl_cfg80211_get_nmode(ndev);
        WL_ERR(("%s wrong_flag:%d, %d\n", __func__, dhd->wrong_action_flag, nmode_enable));
        if((TRUE == nmode_enable) && (FALSE == dhd->nonly_ap) ) {
            return dhd->wrong_action_flag;
        }
    }

    return 0;
}

int wl_trigger_disable_nmode(void) {
    dhd_pub_t *dhd;
    struct net_device *ndev;
    struct bcm_cfg80211 *wl = g_bcm_cfg;

    if(wl&& wl->pub) {
        dhd = (dhd_pub_t *)(wl->pub);
        if(dhd->busstate == DHD_BUS_DOWN) {
            WL_ERR(("%s bus is down, do nothing\n", __func__));
            return 0;
        }
        ndev = bcmcfg_to_prmry_ndev(wl);
        if (dhd->wrong_action_flag
                && FALSE == dhd->nonly_ap
                && TRUE == wl_cfg80211_get_nmode(ndev)) {
            WL_ERR(("%s disable nmode\n", __func__));
            wl_cfg80211_set_nmode(ndev, 0);
        }
    }
    return 0;
}
#else
int wl_get_wrong_action_flag(void) {
    return 0;
}

int wl_trigger_disable_nmode(void) {
    return 0;
}
#endif

#ifdef WLFBT
void
wl_cfg80211_get_fbt_key(uint8 *key)
{
	memcpy(key, g_bcm_cfg->fbt_key, FBT_KEYLEN);
}
#endif /* WLFBT */

static int
wl_cfg80211_delayed_roam(struct bcm_cfg80211 *cfg, struct net_device *ndev,
	const struct ether_addr *bssid)
{
	s32 err;
	wl_event_msg_t e;

	bzero(&e, sizeof(e));
#ifndef  BRCM_RSDB
	e.event_type = cpu_to_be32(WLC_E_ROAM);
#else
	e.event_type = cpu_to_be32(WLC_E_BSSID);
#endif
	memcpy(&e.addr, bssid, ETHER_ADDR_LEN);
	/* trigger the roam event handler */
#ifdef  BRCM_RSDB
	WL_INFORM(("Delayed roam to " MACDBG "\n", MAC2STRDBG((u8*)(bssid))));
#endif
	err = wl_notify_roaming_status(cfg, ndev_to_cfgdev(ndev), &e, NULL);

	return err;
}
#ifdef  BRCM_RSDB
static s32
wl_cfg80211_parse_vndr_ies(u8 *parse, u32 len,
    struct parsed_vndr_ies *vndr_ies)
{
	s32 err = BCME_OK;
	vndr_ie_t *vndrie;
	bcm_tlv_t *ie;
	struct parsed_vndr_ie_info *parsed_info;
	u32 count = 0;
	s32 remained_len;

	remained_len = (s32)len;
	memset(vndr_ies, 0, sizeof(*vndr_ies));

	WL_INFORM(("---> len %d\n", len));
	ie = (bcm_tlv_t *) parse;
	if (!bcm_valid_tlv(ie, remained_len))
		ie = NULL;
	while (ie) {
		if (count >= MAX_VNDR_IE_NUMBER)
			break;
		if (ie->id == DOT11_MNG_VS_ID) {
			vndrie = (vndr_ie_t *) ie;
			/* len should be bigger than OUI length + one data length at least */
			if (vndrie->len < (VNDR_IE_MIN_LEN + 1)) {
				WL_ERR(("%s: invalid vndr ie. length is too small %d\n",
					__FUNCTION__, vndrie->len));
				goto end;
			}
			/* if wpa or wme ie, do not add ie */
			if (!bcmp(vndrie->oui, (u8*)WPA_OUI, WPA_OUI_LEN) &&
				((vndrie->data[0] == WPA_OUI_TYPE) ||
				(vndrie->data[0] == WME_OUI_TYPE))) {
				CFGP2P_DBG(("Found WPA/WME oui. Do not add it\n"));
				goto end;
			}

			parsed_info = &vndr_ies->ie_info[count++];

			/* save vndr ie information */
			parsed_info->ie_ptr = (char *)vndrie;
			parsed_info->ie_len = (vndrie->len + TLV_HDR_LEN);
			memcpy(&parsed_info->vndrie, vndrie, sizeof(vndr_ie_t));
			vndr_ies->count = count;

			WL_DBG(("\t ** OUI %02x %02x %02x, type 0x%02x len:%d\n",
			parsed_info->vndrie.oui[0], parsed_info->vndrie.oui[1],
			parsed_info->vndrie.oui[2], parsed_info->vndrie.data[0],
			parsed_info->ie_len));
		}
end:
		ie = bcm_next_tlv(ie, &remained_len);
	}
	return err;
}

s32
wl_cfg80211_clear_per_bss_ies(struct bcm_cfg80211 *cfg, s32 bssidx)
{
	s32 index;
	struct net_info *netinfo;
	s32 vndrie_flag[] = {VNDR_IE_BEACON_FLAG, VNDR_IE_PRBRSP_FLAG,
		VNDR_IE_ASSOCRSP_FLAG, VNDR_IE_PRBREQ_FLAG, VNDR_IE_ASSOCREQ_FLAG};

	netinfo = wl_get_netinfo_by_bssidx(cfg, bssidx);
	if (!netinfo || !netinfo->wdev) {
		WL_ERR(("netinfo or netinfo->wdev is NULL\n"));
		return -1;
	}

	WL_DBG(("clear management vendor IEs for bssidx:%d \n", bssidx));
	/* Clear the IEs set in the firmware so that host is in sync with firmware */
	for (index = 0; index < ARRAYSIZE(vndrie_flag); index++) {
		if (wl_cfg80211_set_mgmt_vndr_ies(cfg, wdev_to_cfgdev(netinfo->wdev),
			bssidx, vndrie_flag[index], NULL, 0) < 0)
			WL_ERR(("vndr_ies clear failed. Ignoring.. \n"));
	}

	return 0;
}

s32
wl_cfg80211_clear_mgmt_vndr_ies(struct bcm_cfg80211 *cfg)
{
	struct net_info *iter, *next;

	WL_DBG(("clear management vendor IEs \n"));
#if defined(STRICT_GCC_WARNINGS) && defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == \
	4 && __GNUC_MINOR__ >= 6))
_Pragma("GCC diagnostic push")
_Pragma("GCC diagnostic ignored \"-Wcast-qual\"")
#endif
	for_each_ndev(cfg, iter, next) {
		wl_cfg80211_clear_per_bss_ies(cfg, iter->bssidx);
	}
#if defined(STRICT_GCC_WARNINGS) && defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == \
	4 && __GNUC_MINOR__ >= 6))
_Pragma("GCC diagnostic pop")
#endif
	return 0;
}

#define WL_VNDR_IE_MAXLEN 2048
static s8 g_mgmt_ie_buf[WL_VNDR_IE_MAXLEN];
int
wl_cfg80211_set_mgmt_vndr_ies(struct bcm_cfg80211 *cfg, bcm_struct_cfgdev *cfgdev,
	s32 bssidx, s32 pktflag, const u8 *vndr_ie, u32 vndr_ie_len)
{
	struct net_device *ndev = NULL;
	s32 ret = BCME_OK;
	u8  *curr_ie_buf = NULL;
	u8  *mgmt_ie_buf = NULL;
	u32 mgmt_ie_buf_len = 0;
	u32 *mgmt_ie_len = 0;
	u32 del_add_ie_buf_len = 0;
	u32 total_ie_buf_len = 0;
	u32 parsed_ie_buf_len = 0;
	struct parsed_vndr_ies old_vndr_ies;
	struct parsed_vndr_ies new_vndr_ies;
	s32 i;
	u8 *ptr;
	s32 remained_buf_len;
	wl_bss_vndr_ies_t *ies = NULL;
	struct net_info *netinfo;

	WL_DBG(("Enter. pktflag:0x%x bssidx:%x vnd_ie_len:%d \n",
		pktflag, bssidx, vndr_ie_len));

	ndev = cfgdev_to_wlc_ndev(cfgdev, cfg);

	if (bssidx > WL_MAX_IFS) {
		WL_ERR(("bssidx > supported concurrent Ifaces \n"));
		return -EINVAL;
	}

	netinfo = wl_get_netinfo_by_bssidx(cfg, bssidx);
	if (!netinfo) {
		WL_ERR(("net_info ptr is NULL \n"));
		return -EINVAL;
	}

	/* Clear the global buffer */
	memset(g_mgmt_ie_buf, 0, sizeof(g_mgmt_ie_buf));
	curr_ie_buf = g_mgmt_ie_buf;
	ies = &netinfo->bss.ies;

	switch (pktflag) {
		case VNDR_IE_PRBRSP_FLAG :
			mgmt_ie_buf = ies->probe_res_ie;
			mgmt_ie_len = &ies->probe_res_ie_len;
			mgmt_ie_buf_len = sizeof(ies->probe_res_ie);
			break;
		case VNDR_IE_ASSOCRSP_FLAG :
			mgmt_ie_buf = ies->assoc_res_ie;
			mgmt_ie_len = &ies->assoc_res_ie_len;
			mgmt_ie_buf_len = sizeof(ies->assoc_res_ie);
			break;
		case VNDR_IE_BEACON_FLAG :
			mgmt_ie_buf = ies->beacon_ie;
			mgmt_ie_len = &ies->beacon_ie_len;
			mgmt_ie_buf_len = sizeof(ies->beacon_ie);
			break;
		case VNDR_IE_PRBREQ_FLAG :
			mgmt_ie_buf = ies->probe_req_ie;
			mgmt_ie_len = &ies->probe_req_ie_len;
			mgmt_ie_buf_len = sizeof(ies->probe_req_ie);
			break;
		case VNDR_IE_ASSOCREQ_FLAG :
			mgmt_ie_buf = ies->assoc_req_ie;
			mgmt_ie_len = &ies->assoc_req_ie_len;
			mgmt_ie_buf_len = sizeof(ies->assoc_req_ie);
			break;
		default:
			mgmt_ie_buf = NULL;
			mgmt_ie_len = NULL;
			WL_ERR(("not suitable packet type (%d)\n", pktflag));
			return BCME_ERROR;
	}

	if (vndr_ie_len > mgmt_ie_buf_len) {
		WL_ERR(("extra IE size too big\n"));
		ret = -ENOMEM;
	} else {
		/* parse and save new vndr_ie in curr_ie_buff before comparing it */
		if (vndr_ie && vndr_ie_len && curr_ie_buf) {
			ptr = curr_ie_buf;
/* must discard vndr_ie constness, attempt to change vndr_ie arg to non-const
 * causes cascade of errors in other places, fix involves const casts there
 */
#if defined(STRICT_GCC_WARNINGS) && defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == \
	4 && __GNUC_MINOR__ >= 6))
_Pragma("GCC diagnostic push")
_Pragma("GCC diagnostic ignored \"-Wcast-qual\"")
#endif
			if ((ret = wl_cfg80211_parse_vndr_ies((u8 *)vndr_ie,
				vndr_ie_len, &new_vndr_ies)) < 0) {
				WL_ERR(("parse vndr ie failed \n"));
				goto exit;
			}
#if defined(STRICT_GCC_WARNINGS) && defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == \
	4 && __GNUC_MINOR__ >= 6))
_Pragma("GCC diagnostic pop")
#endif
			for (i = 0; i < new_vndr_ies.count; i++) {
				struct parsed_vndr_ie_info *vndrie_info =
					&new_vndr_ies.ie_info[i];

				if ((parsed_ie_buf_len + vndrie_info->ie_len) > WL_VNDR_IE_MAXLEN) {
					WL_ERR(("IE size is too big (%d > %d)\n",
						parsed_ie_buf_len, WL_VNDR_IE_MAXLEN));
					ret = -EINVAL;
					goto exit;
				}

				memcpy(ptr + parsed_ie_buf_len, vndrie_info->ie_ptr,
					vndrie_info->ie_len);
				parsed_ie_buf_len += vndrie_info->ie_len;
			}
		}

		if (mgmt_ie_buf != NULL) {
			if (parsed_ie_buf_len && (parsed_ie_buf_len == *mgmt_ie_len) &&
				(memcmp(mgmt_ie_buf, curr_ie_buf, parsed_ie_buf_len) == 0)) {
				WL_INFORM(("Previous mgmt IE is equals to current IE"));
				goto exit;
			}

			/* parse old vndr_ie */
			if ((ret = wl_cfg80211_parse_vndr_ies(mgmt_ie_buf, *mgmt_ie_len,
				&old_vndr_ies)) < 0) {
				WL_ERR(("parse vndr ie failed \n"));
				goto exit;
			}
			/* make a command to delete old ie */
			for (i = 0; i < old_vndr_ies.count; i++) {
				struct parsed_vndr_ie_info *vndrie_info =
				&old_vndr_ies.ie_info[i];

				WL_INFORM(("DELETED ID : %d, Len: %d , OUI:%02x:%02x:%02x\n",
					vndrie_info->vndrie.id, vndrie_info->vndrie.len,
					vndrie_info->vndrie.oui[0], vndrie_info->vndrie.oui[1],
					vndrie_info->vndrie.oui[2]));

				del_add_ie_buf_len = wl_cfgp2p_vndr_ie(cfg, curr_ie_buf,
					pktflag, vndrie_info->vndrie.oui,
					vndrie_info->vndrie.id,
					vndrie_info->ie_ptr + VNDR_IE_FIXED_LEN,
					vndrie_info->ie_len - VNDR_IE_FIXED_LEN,
					"del");

				curr_ie_buf += del_add_ie_buf_len;
				total_ie_buf_len += del_add_ie_buf_len;
			}
		}

		*mgmt_ie_len = 0;
		/* Add if there is any extra IE */
		if (mgmt_ie_buf && parsed_ie_buf_len) {
			ptr = mgmt_ie_buf;

			remained_buf_len = mgmt_ie_buf_len;

			/* make a command to add new ie */
			for (i = 0; i < new_vndr_ies.count; i++) {
				struct parsed_vndr_ie_info *vndrie_info =
					&new_vndr_ies.ie_info[i];

				WL_INFORM(("ADDED ID : %d, Len: %d(%d), OUI:%02x:%02x:%02x\n",
					vndrie_info->vndrie.id, vndrie_info->vndrie.len,
					vndrie_info->ie_len - 2,
					vndrie_info->vndrie.oui[0], vndrie_info->vndrie.oui[1],
					vndrie_info->vndrie.oui[2]));

				del_add_ie_buf_len = wl_cfgp2p_vndr_ie(cfg, curr_ie_buf,
					pktflag, vndrie_info->vndrie.oui,
					vndrie_info->vndrie.id,
					vndrie_info->ie_ptr + VNDR_IE_FIXED_LEN,
					vndrie_info->ie_len - VNDR_IE_FIXED_LEN,
					"add");

				/* verify remained buf size before copy data */
				if (remained_buf_len >= vndrie_info->ie_len) {
					remained_buf_len -= vndrie_info->ie_len;
				} else {
					WL_ERR(("no space in mgmt_ie_buf: pktflag = %d, "
					"found vndr ies # = %d(cur %d), remained len %d, "
					"cur mgmt_ie_len %d, new ie len = %d\n",
					pktflag, new_vndr_ies.count, i, remained_buf_len,
					*mgmt_ie_len, vndrie_info->ie_len));
					break;
				}

				/* save the parsed IE in cfg struct */
				memcpy(ptr + (*mgmt_ie_len), vndrie_info->ie_ptr,
					vndrie_info->ie_len);
				*mgmt_ie_len += vndrie_info->ie_len;
				curr_ie_buf += del_add_ie_buf_len;
				total_ie_buf_len += del_add_ie_buf_len;
			}
		}

		if (total_ie_buf_len && cfg->ioctl_buf != NULL) {
			ret  = wldev_iovar_setbuf_bsscfg(ndev, "vndr_ie", g_mgmt_ie_buf,
				total_ie_buf_len, cfg->ioctl_buf, WLC_IOCTL_MAXLEN,
				bssidx, &cfg->ioctl_buf_sync);
			if (ret)
				WL_ERR(("vndr ie set error : %d\n", ret));
		}
	}
exit:

return ret;
}
#endif /* BRCM_RSDB */

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,5,0)) || defined(BCM_PATCH_ABORT_SCAN)
static int __wl_cfg80211_abort_scan(struct wiphy *wiphy, struct wireless_dev *wdev)
{
	struct bcm_cfg80211 *cfg = wiphy_priv(wiphy);
	struct net_device *ndev = bcmcfg_to_prmry_ndev(cfg);
	int err;

	if (wl_get_drv_status(cfg, SCAN_ABORTING, ndev)) {
		WL_ERR(("Scanning being aborted\n"));
		return -1;
	}
	if (wl_get_drv_status_all(cfg, SCANNING)) {
		if (cfg->scan_request == NULL) {
			wl_clr_drv_status_all(cfg, SCANNING);
			WL_DBG(("<<<<<<<<<<<Force Clear Scanning Status>>>>>>>>>>>\n"));
		}
		err = wldev_iovar_setbuf_bsscfg(ndev, "scanabort",
			NULL, 0, cfg->ioctl_buf, WLC_IOCTL_SMLEN, 0, &cfg->ioctl_buf_sync);
		if (unlikely(err)) {
			WL_ERR(("error (%d)\n", err));
			return -1;
		}
	}

	return 0;
}
static void wl_cfg80211_abort_scan(struct wiphy *wiphy, struct wireless_dev *wdev)
{
	__wl_cfg80211_abort_scan(wiphy, wdev);
}
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(4,5,0)) || defined(BCM_PATCH_ABORT_SCAN)*/


#ifdef WL_CFG80211_ACL
static int
wl_cfg80211_set_mac_acl(struct wiphy *wiphy, struct net_device *cfgdev,
	const struct cfg80211_acl_data *acl)
{
	int i;
	int ret = 0;
	int macnum = 0;
	int macmode = MACLIST_MODE_DISABLED;
	struct maclist *list;

	/* get the MAC filter mode */
	if (acl && acl->acl_policy == NL80211_ACL_POLICY_DENY_UNLESS_LISTED) {
		macmode = MACLIST_MODE_ALLOW;
	} else if (acl && acl->acl_policy == NL80211_ACL_POLICY_ACCEPT_UNLESS_LISTED &&
	acl->n_acl_entries) {
		macmode = MACLIST_MODE_DENY;
	}

	/* if acl == NULL, macmode is still disabled.. */
	if (macmode == MACLIST_MODE_DISABLED) {
		if ((ret = wl_android_set_ap_mac_list(cfgdev, macmode, NULL)) != 0)
			WL_ERR(("%s : Setting MAC list failed error=%d\n", __FUNCTION__, ret));

		return ret;
	}

	macnum = acl->n_acl_entries;
	if (macnum < 0 || macnum > MAX_NUM_MAC_FILT) {
		WL_ERR(("%s : invalid number of MAC address entries %d\n",
			__FUNCTION__, macnum));
		return -1;
	}

	/* allocate memory for the MAC list */
	list = (struct maclist*)kmalloc(sizeof(int) +
		sizeof(struct ether_addr) * macnum, GFP_KERNEL);
	if (!list) {
		WL_ERR(("%s : failed to allocate memory\n", __FUNCTION__));
		return -1;
	}

	/* prepare the MAC list */
	list->count = htod32(macnum);
	for (i = 0; i < macnum; i++) {
		memcpy(&list->ea[i], &acl->mac_addrs[i], ETHER_ADDR_LEN);
	}
	/* set the list */
	if ((ret = wl_android_set_ap_mac_list(cfgdev, macmode, list)) != 0)
		WL_ERR(("%s : Setting MAC list failed error=%d\n", __FUNCTION__, ret));

	kfree(list);

	return ret;
}
#endif /* WL_CFG80211_ACL */
#ifdef  BRCM_RSDB
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 5, 0))
int wl_chspec_chandef(chanspec_t chanspec,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 8, 0))
	struct cfg80211_chan_def *chandef,
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION (3, 5, 0) && (LINUX_VERSION_CODE <= (3, 7, \
	\
	0)))
	struct chan_info *chaninfo,
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 5, 0)) */
struct wiphy *wiphy)

{
	uint16 freq = 0;
	int chan_type = 0;
	int channel = 0;
	struct ieee80211_channel *chan;

	if (!chandef) {
		return -1;
	}
	channel = CHSPEC_CHANNEL(chanspec);

	switch (CHSPEC_BW(chanspec)) {
		case WL_CHANSPEC_BW_20:
			chan_type = NL80211_CHAN_HT20;
			break;
		case WL_CHANSPEC_BW_40:
		{
			if (CHSPEC_SB_UPPER(chanspec)) {
				channel += CH_10MHZ_APART;
			} else {
				channel -= CH_10MHZ_APART;
			}
		}
			chan_type = NL80211_CHAN_HT40PLUS;
			break;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION (3, 8, 0))
		case WL_CHANSPEC_BW_80:
		case WL_CHANSPEC_BW_8080:
		{
			uint16 sb = CHSPEC_CTL_SB(chanspec);

			if (sb == WL_CHANSPEC_CTL_SB_LL) {
				channel -= (CH_10MHZ_APART + CH_20MHZ_APART);
			} else if (sb == WL_CHANSPEC_CTL_SB_LU) {
				channel -= CH_10MHZ_APART;
			} else if (sb == WL_CHANSPEC_CTL_SB_UL) {
				channel += CH_10MHZ_APART;
			} else {
				/* WL_CHANSPEC_CTL_SB_UU */
				channel += (CH_10MHZ_APART + CH_20MHZ_APART);
			}

			if (sb == WL_CHANSPEC_CTL_SB_LL || sb == WL_CHANSPEC_CTL_SB_LU)
				chan_type = NL80211_CHAN_HT40MINUS;
			else if (sb == WL_CHANSPEC_CTL_SB_UL || sb == WL_CHANSPEC_CTL_SB_UU)
				chan_type = NL80211_CHAN_HT40PLUS;
		}
			break;
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION (3, 8, 0)) */
		default:
			chan_type = NL80211_CHAN_HT20;
			break;

	}

	if (CHSPEC_IS5G(chanspec))
		freq = ieee80211_channel_to_frequency(channel, NL80211_BAND_5GHZ);
	else
		freq = ieee80211_channel_to_frequency(channel, NL80211_BAND_2GHZ);

	chan = ieee80211_get_channel(wiphy, freq);
	WL_DBG(("channel:%d freq:%d chan_type: %d chan_ptr:%p \n",
		channel, freq, chan_type, chan));

	if (unlikely(!chan)) {
		/* fw and cfg80211 channel lists are not in sync */
		WL_ERR(("Couldn't find matching channel in wiphy channel list \n"));
		ASSERT(0);
		return -EINVAL;
	}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION (3, 8, 0))
	cfg80211_chandef_create(chandef, chan, chan_type);
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION (3, 5, 0) && (LINUX_VERSION_CODE <= (3, 7, \
	\
	0)))
	chaninfo->freq = freq;
	chaninfo->chan_type = chan_type;
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION (3, 8, 0)) */
	return 0;
}

void
wl_cfg80211_ch_switch_notify(struct net_device *dev, uint16 chanspec, struct wiphy *wiphy)
{
	u32 freq;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION (3, 8, 0))
	struct cfg80211_chan_def chandef;
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION (3, 5, 0) && (LINUX_VERSION_CODE <= (3, 7, \
	\
	0)))
	struct chan_info chaninfo;
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION (3, 8, 0)) */

	if (!wiphy) {
		WL_ERR(("wiphy is null\n"));
		return;
	}
#ifndef ALLOW_CHSW_EVT
	/* Channel switch support is only for AP/GO/ADHOC/MESH */
	if (dev->ieee80211_ptr->iftype == NL80211_IFTYPE_STATION ||
		dev->ieee80211_ptr->iftype == NL80211_IFTYPE_P2P_CLIENT) {
		WL_ERR(("No channel switch notify support for STA/GC\n"));
		return;
	}
#endif /* !ALLOW_CHSW_EVT */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION (3, 8, 0))
	if (wl_chspec_chandef(chanspec, &chandef, wiphy)) {
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION (3, 5, 0) && (LINUX_VERSION_CODE <= (3, 7, \
	\
	0)))
	if (wl_chspec_chandef(chanspec, &chaninfo, wiphy)) {
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION (3, 8, 0)) */

		WL_ERR(("chspec_chandef failed\n"));
		return;
	}
#if (LINUX_VERSION_CODE >= KERNEL_VERSION (3, 8, 0))
	freq = chandef.chan ? chandef.chan->center_freq : chandef.center_freq1;
	cfg80211_ch_switch_notify(dev, &chandef);
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION (3, 5, 0) && (LINUX_VERSION_CODE <= (3, 7, \
	\
	0)))
	freq = chan_info.freq;
	cfg80211_ch_switch_notify(dev, chan_info.freq, chan_info.chan_type);
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION (3, 8, 0)) */

	WL_ERR(("Channel switch notification for freq: %d chanspec: 0x%x\n", freq, chanspec));
	return;
}
#endif /* LINUX_VERSION_CODE >= (3, 5, 0) */

#ifdef WL11ULB
s32
wl_cfg80211_set_ulb_mode(struct net_device *dev, int mode)
{
	int ret;
	int cur_mode;

	ret = wldev_iovar_getint(dev, "ulb_mode", &cur_mode);
	if (unlikely(ret)) {
		WL_ERR(("[ULB] ulb_mode get failed. ret:%d \n", ret));
		return ret;
	}

	if (cur_mode == mode) {
		/* If request mode is same as that of the current mode, then
		 * do nothing (Avoid unnecessary wl down and up).
		 */
		WL_INFORM(("[ULB] No change in ulb_mode. Do nothing.\n"));
		return 0;
	}

	/* setting of ulb_mode requires wl to be down */
	ret = wldev_ioctl(dev, WLC_DOWN, NULL, 0, true);
	if (unlikely(ret)) {
		WL_ERR(("[ULB] WLC_DOWN command failed:[%d]\n", ret));
		return ret;
	}

	if (mode >= MAX_SUPP_ULB_MODES) {
		WL_ERR(("[ULB] unsupported ulb_mode :[%d]\n", mode));
		return -EINVAL;
	}

	ret = wldev_iovar_setint(dev, "ulb_mode", mode);
	if (unlikely(ret)) {
		WL_ERR(("[ULB] ulb_mode set failed. ret:%d \n", ret));
		return ret;
	}

	ret = wldev_ioctl(dev, WLC_UP, NULL, 0, true);
	if (unlikely(ret)) {
		WL_ERR(("[ULB] WLC_DOWN command failed:[%d]\n", ret));
		return ret;
	}

	WL_DBG(("[ULB] ulb_mode set to %d successfully \n", mode));

	return ret;
}

static s32
wl_cfg80211_ulbbw_to_ulbchspec(u32 bw)
{
	if (bw == ULB_BW_DISABLED) {
		return WL_CHANSPEC_BW_20;
	} else if (bw == ULB_BW_10MHZ) {
		return WL_CHANSPEC_BW_10;
	} else if (bw == ULB_BW_5MHZ) {
		return WL_CHANSPEC_BW_5;
	} else if (bw == ULB_BW_2P5MHZ) {
		return WL_CHANSPEC_BW_2P5;
	} else {
		WL_ERR(("[ULB] unsupported value for ulb_bw \n"));
		return -EINVAL;
	}
}

static chanspec_t
wl_cfg80211_ulb_get_min_bw_chspec(struct wireless_dev *wdev, s32 bssidx)
{
	struct bcm_cfg80211 *cfg = g_bcm_cfg;
	struct net_info *_netinfo;

	/*
	 *  Return the chspec value corresponding to the
	 *  BW setting for a particular interface
	 */
	if (wdev) {
		/* if wdev is provided, use it */
		_netinfo = wl_get_netinfo_by_wdev(cfg, wdev);
	} else if (bssidx >= 0) {
		/* if wdev is not provided, use it */
		_netinfo = wl_get_netinfo_by_bssidx(cfg, bssidx);
	} else {
		WL_ERR(("[ULB] wdev/bssidx not provided\n"));
		return INVCHANSPEC;
	}

	if (unlikely(!_netinfo)) {
		WL_ERR(("[ULB] net_info is null \n"));
		return INVCHANSPEC;
	}

	if (_netinfo->ulb_bw) {
		WL_DBG(("[ULB] wdev_ptr:%p ulb_bw:0x%x \n", _netinfo->wdev, _netinfo->ulb_bw));
		return wl_cfg80211_ulbbw_to_ulbchspec(_netinfo->ulb_bw);
	} else {
		return WL_CHANSPEC_BW_20;
	}
}

static s32
wl_cfg80211_get_ulb_bw(struct wireless_dev *wdev)
{
	struct bcm_cfg80211 *cfg = g_bcm_cfg;
	struct net_info *_netinfo = wl_get_netinfo_by_wdev(cfg, wdev);

	/*
	 *  Return the ulb_bw setting for a
	 *  particular interface
	 */
	if (unlikely(!_netinfo)) {
		WL_ERR(("[ULB] net_info is null \n"));
		return -1;
	}

	return _netinfo->ulb_bw;
}

s32
wl_cfg80211_set_ulb_bw(struct net_device *dev,
	u32 ulb_bw,  char *ifname)
{
	struct bcm_cfg80211 *cfg = g_bcm_cfg;
	int ret;
	int mode;
	struct net_info *_netinfo = NULL, *iter, *next;
	u32 bssidx;

	if (!ifname)
		return -EINVAL;

	WL_DBG(("[ULB] Enter. bw_type:%d \n", ulb_bw));

	ret = wldev_iovar_getint(dev, "ulb_mode", &mode);
	if (unlikely(ret)) {
		WL_ERR(("[ULB] ulb_mode not supported \n"));
		return ret;
	}

	if (mode != ULB_MODE_STD_ALONE_MODE) {
		WL_ERR(("[ULB] ulb bw modification allowed only in stand-alone mode\n"));
		return -EINVAL;
	}

	if (ulb_bw >= MAX_SUPP_ULB_BW) {
		WL_ERR(("[ULB] unsupported value (%d) for ulb_bw \n", ulb_bw));
		return -EINVAL;
	}

#ifdef WL_CFG80211_P2P_DEV_IF
	if (strcmp(ifname, "p2p-dev-wlan0") == 0) {
		/* Use wdev corresponding to the dedicated p2p discovery interface */
		if (likely(cfg->p2p_wdev)) {
			_netinfo = wl_get_netinfo_by_wdev(cfg, cfg->p2p_wdev);
		} else {
			return -ENODEV;
		}
	}
#endif /* WL_CFG80211_P2P_DEV_IF */
	if (!_netinfo) {
		for_each_ndev(cfg, iter, next) {
			if (iter->ndev) {
				if (strncmp(iter->ndev->name, ifname, strlen(ifname)) == 0) {
					_netinfo = wl_get_netinfo_by_netdev(cfg, iter->ndev);
				}
			}
		}
	}

	if (!_netinfo)
		return -ENODEV;
	bssidx = _netinfo->bssidx;
	_netinfo->ulb_bw = ulb_bw;


	WL_DBG(("[ULB] Applying ulb_bw:%d for bssidx:%d \n", ulb_bw, bssidx));
	ret = wldev_iovar_setbuf_bsscfg(dev, "ulb_bw", (void *)&ulb_bw, 4,
		cfg->ioctl_buf, WLC_IOCTL_SMLEN, bssidx,
		&cfg->ioctl_buf_sync);
	if (unlikely(ret)) {
		WL_ERR(("[ULB] ulb_bw set failed. ret:%d \n", ret));
		return ret;
	}

	return ret;
}
#endif /* WL11ULB */

static void
wl_ap_channel_ind(struct bcm_cfg80211 *cfg,
	struct net_device *ndev,
	chanspec_t chanspec)
{
	u32 channel = LCHSPEC_CHANNEL(chanspec);

	WL_DBG(("(%s) AP channel:%d chspec:0x%x \n",
		ndev->name, channel, chanspec));
	if (cfg->ap_oper_channel && (cfg->ap_oper_channel != channel)) {
		/*
		 * If cached channel is different from the channel indicated
		 * by the event, notify user space about the channel switch.
		 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 5, 0))
		wl_cfg80211_ch_switch_notify(ndev, chanspec, bcmcfg_to_wiphy(cfg));
#endif /* LINUX_VERSION_CODE >= (3, 5, 0) */
		cfg->ap_oper_channel = channel;
	}
}

static s32
wl_ap_start_ind(struct bcm_cfg80211 *cfg, bcm_struct_cfgdev *cfgdev,
const wl_event_msg_t *e, void *data)
{
	struct net_device *ndev = NULL;
	chanspec_t chanspec;

	WL_DBG(("Enter\n"));
	if (unlikely(e->status)) {
		WL_ERR(("status:0x%x \n", e->status));
		return -1;
	}

	if (!data) {
		return -EINVAL;
	}

	if (likely(cfgdev)) {
		ndev = cfgdev_to_wlc_ndev(cfgdev, cfg);
		chanspec = *((chanspec_t *)data);

		if (wl_get_mode_by_netdev(cfg, ndev) == WL_MODE_AP) {
			/* For AP/GO role */
			wl_ap_channel_ind(cfg, ndev, chanspec);
		}
	}

	return 0;
}

static s32
wl_csa_complete_ind(struct bcm_cfg80211 *cfg, bcm_struct_cfgdev *cfgdev,
const wl_event_msg_t *e, void *data)
{
	int error = 0;
	u32 chanspec = 0;
	struct net_device *ndev = NULL;

	WL_DBG(("Enter\n"));
	if (unlikely(e->status)) {
		WL_ERR(("status:0x%x \n", e->status));
		return -1;
	}

	if (likely(cfgdev)) {
		ndev = cfgdev_to_wlc_ndev(cfgdev, cfg);
		error = wldev_iovar_getint(ndev, "chanspec", &chanspec);
		if (unlikely(error)) {
			WL_ERR(("Get chanspec error: %d \n", error));
			return -1;
		}

		if (wl_get_mode_by_netdev(cfg, ndev) == WL_MODE_AP) {
			/* For AP/GO role */
			wl_ap_channel_ind(cfg, ndev, chanspec);
		} else {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 5, 0))
			wl_cfg80211_ch_switch_notify(ndev, chanspec, bcmcfg_to_wiphy(cfg));
#endif /* LINUX_VERSION_CODE >= (3, 5, 0) */
		}

	}

	return 0;
}

#ifdef WL_NAN
int
wl_cfg80211_nan_cmd_handler(struct net_device *ndev, char *cmd, int cmd_len)
{
	return wl_cfgnan_cmd_handler(ndev, g_bcm_cfg, cmd, cmd_len);
}
#endif /* WL_NAN */

void wl_cfg80211_clear_security(struct bcm_cfg80211 *cfg)
{
	struct net_device *dev = bcmcfg_to_prmry_ndev(cfg);
	int err;

	/* Clear the security settings on the primary Interface */
	err = wldev_iovar_setint(dev, "wsec", 0);
	if (unlikely(err)) {
		WL_ERR(("wsec clear failed \n"));
	}
	err = wldev_iovar_setint(dev, "auth", 0);
	if (unlikely(err)) {
		WL_ERR(("auth clear failed \n"));
	}
	err = wldev_iovar_setint(dev, "wpa_auth", WPA_AUTH_DISABLED);
	if (unlikely(err)) {
		WL_ERR(("wpa_auth clear failed \n"));
	}
}

#ifdef WL_CFG80211_P2P_DEV_IF
void wl_cfg80211_del_p2p_wdev(void)
{
	struct bcm_cfg80211 *cfg = g_bcm_cfg;
	struct wireless_dev *wdev = NULL;

	WL_DBG(("Enter \n"));
	if (!cfg) {
		WL_ERR(("Invalid Ptr\n"));
		return;
	} else {
		wdev = cfg->p2p_wdev;
	}

	if (wdev && cfg->down_disc_if) {
		wl_cfgp2p_del_p2p_disc_if(wdev, cfg);
		cfg->down_disc_if = FALSE;
	}
}
#endif /* WL_CFG80211_P2P_DEV_IF */

#if defined(WL_SUPPORT_AUTO_CHANNEL)
int
wl_cfg80211_set_spect(struct net_device *dev, int spect)
{
	struct bcm_cfg80211 *cfg = g_bcm_cfg;
	int down = 1;
	int up = 1;
	int err = BCME_OK;

	if (!wl_get_drv_status_all(cfg, CONNECTED)) {
		err = wldev_ioctl(dev, WLC_DOWN, &down, sizeof(down), true);
		if (err) {
			WL_ERR(("%s: WLC_DOWN failed: code: %d\n", __func__, err));
			return err;
		}

		err = wldev_ioctl(dev, WLC_SET_SPECT_MANAGMENT, &spect, sizeof(spect), true);
		if (err) {
			WL_ERR(("%s: error setting spect: code: %d\n", __func__, err));
			return err;
		}

		err = wldev_ioctl(dev, WLC_UP, &up, sizeof(up), true);
		if (err) {
			WL_ERR(("%s: WLC_UP failed: code: %d\n", __func__, err));
			return err;
		}
	}
	return err;
}

int
wl_cfg80211_get_sta_channel(void)
{
	struct net_device *ndev = bcmcfg_to_prmry_ndev(g_bcm_cfg);
	int channel = 0;

	if (wl_get_drv_status(g_bcm_cfg, CONNECTED, ndev)) {
		channel = g_bcm_cfg->channel;
	}
	return channel;
}
#endif /* WL_SUPPORT_AUTO_CHANNEL */
#ifdef P2P_LISTEN_OFFLOADING
s32
wl_cfg80211_p2plo_deinit(struct bcm_cfg80211 *cfg)
{
	s32 bssidx;
	int ret = 0;
	int p2plo_pause = 0;
	if (!cfg || !cfg->p2p) {
		WL_ERR(("Wl %p or cfg->p2p %p is null\n",
			cfg, cfg ? cfg->p2p : 0));
		return 0;
	}

	bssidx = wl_to_p2p_bss_bssidx(cfg, P2PAPI_BSSCFG_DEVICE);
	ret = wldev_iovar_setbuf_bsscfg(bcmcfg_to_prmry_ndev(cfg),
			"p2po_stop", (void*)&p2plo_pause, sizeof(p2plo_pause),
			cfg->ioctl_buf, WLC_IOCTL_SMLEN, bssidx, NULL);
	if (ret < 0) {
		WL_ERR(("p2po_stop Failed :%d\n", ret));
	}

	return  ret;
}
s32
wl_cfg80211_p2plo_listen_start(struct net_device *dev, u8 *buf,	int len)
{
	struct bcm_cfg80211 *cfg = g_bcm_cfg;
	s32 bssidx = wl_to_p2p_bss_bssidx(cfg, P2PAPI_BSSCFG_DEVICE);
	wl_p2plo_listen_t p2plo_listen;
	int ret = -EAGAIN;
	int channel = 0;
	int period = 0;
	int interval = 0;
	int count = 0;

	if (WL_DRV_STATUS_SENDING_AF_FRM_EXT(cfg)) {
		WL_ERR(("Sending Action Frames. Try it again.\n"));
		goto exit;
	}

	if (wl_get_drv_status_all(cfg, SCANNING)) {
		WL_ERR(("Scanning already\n"));
		goto exit;
	}

	if (wl_get_drv_status(cfg, SCAN_ABORTING, dev)) {
		WL_ERR(("Scanning being aborted\n"));
		goto exit;
	}

	if (wl_get_p2p_status(cfg, DISC_IN_PROGRESS)) {
		WL_ERR(("p2p listen offloading already running\n"));
		goto exit;
	}

	/* Just in case if it is not enabled */
	if ((ret = wl_cfgp2p_enable_discovery(cfg, dev, NULL, 0)) < 0) {
		WL_ERR(("cfgp2p_enable discovery failed"));
		goto exit;
	}

	bzero(&p2plo_listen, sizeof(wl_p2plo_listen_t));

	if (len) {
		sscanf(buf, " %10d %10d %10d %10d", &channel, &period, &interval, &count);
		if ((channel == 0) || (period == 0) ||
			(interval == 0) || (count == 0)) {
			WL_ERR(("Wrong argument %d/%d/%d/%d \n",
				channel, period, interval, count));
			ret = -EAGAIN;
			goto exit;
		}
		p2plo_listen.period = period;
		p2plo_listen.interval = interval;
		p2plo_listen.count = count;

		WL_ERR(("channel:%d period:%d, interval:%d count:%d\n",
				channel, period, interval, count));
	} else {
		WL_ERR(("Argument len is wrong.\n"));
		ret = -EAGAIN;
		goto exit;
	}

	if ((ret = wldev_iovar_setbuf_bsscfg(dev, "p2po_listen_channel", (void*)&channel,
			sizeof(channel), cfg->ioctl_buf, WLC_IOCTL_SMLEN,
			bssidx, &cfg->ioctl_buf_sync)) < 0) {
		WL_ERR(("p2po_listen_channel Failed :%d\n", ret));
		goto exit;
	}

	if ((ret = wldev_iovar_setbuf_bsscfg(dev, "p2po_listen", (void*)&p2plo_listen,
			sizeof(wl_p2plo_listen_t), cfg->ioctl_buf, WLC_IOCTL_SMLEN,
			bssidx, &cfg->ioctl_buf_sync)) < 0) {
		WL_ERR(("p2po_listen Failed :%d\n", ret));
		goto exit;
	}

	wl_set_p2p_status(cfg, DISC_IN_PROGRESS);
	cfg->last_roc_id = P2PO_COOKIE;
exit :
	return ret;
}
s32
wl_cfg80211_p2plo_listen_stop(struct net_device *dev)
{
	struct bcm_cfg80211 *cfg = g_bcm_cfg;
	s32 bssidx = wl_to_p2p_bss_bssidx(cfg, P2PAPI_BSSCFG_DEVICE);
	int ret = -EAGAIN;

	if ((ret = wldev_iovar_setbuf_bsscfg(dev, "p2po_stop", NULL,
			0, cfg->ioctl_buf, WLC_IOCTL_SMLEN,
			bssidx, &cfg->ioctl_buf_sync)) < 0) {
		WL_ERR(("p2po_stop Failed :%d\n", ret));
		goto exit;
	}

exit:
	return ret;
}
#endif /* P2P_LISTEN_OFFLOADING */
u64
wl_cfg80211_get_new_roc_id(struct bcm_cfg80211 *cfg)
{
	u64 id = 0;
	id = ++cfg->last_roc_id;
#ifdef  P2P_LISTEN_OFFLOADING
	if (id == P2PO_COOKIE) {
		id = ++cfg->last_roc_id;
	}
#endif /* P2P_LISTEN_OFFLOADING */
	if (id == 0)
		id = ++cfg->last_roc_id;
	return id;
}

#if defined(SUPPORT_RANDOM_MAC_SCAN)
int
wl_cfg80211_set_random_mac(struct net_device *dev, bool enable)
{
	struct bcm_cfg80211 *cfg = g_bcm_cfg;
	int ret;

	if (cfg->random_mac_enabled == enable) {
		WL_ERR(("Random MAC already %s\n", enable ? "Enabled" : "Disabled"));
		return BCME_OK;
	}

	if (enable) {
		ret = wl_cfg80211_random_mac_enable(dev);
	} else {
		ret = wl_cfg80211_random_mac_disable(dev);
	}

	if (!ret) {
		cfg->random_mac_enabled = enable;
	}

	return ret;
}

int
wl_cfg80211_random_mac_enable(struct net_device *dev)
{
	u8 current_mac[ETH_ALEN] = {0, };
	s32 err = BCME_ERROR;
	uint8 buffer[20] = {0, };
	wl_scanmac_t *sm = NULL;
	int len = 0;
	wl_scanmac_enable_t *sm_enable = NULL;
	wl_scanmac_config_t *sm_config = NULL;
	struct bcm_cfg80211 *cfg = g_bcm_cfg;

	if (wl_get_drv_status_all(cfg, CONNECTED) || wl_get_drv_status_all(cfg, CONNECTING) ||
	    wl_get_drv_status_all(cfg, AP_CREATED) || wl_get_drv_status_all(cfg, AP_CREATING)) {
		WL_ERR(("Fail to Set random mac, current state is wrong\n"));
		return err;
	}

	/* Read current mac address */
	err = wldev_iovar_getbuf_bsscfg(dev, "cur_etheraddr",
		NULL, 0, cfg->ioctl_buf, WLC_IOCTL_SMLEN, 0, &cfg->ioctl_buf_sync);

	if (err != BCME_OK) {
		WL_ERR(("failed to get current dongle mac address\n"));
		return err;
	}

	memcpy(current_mac, cfg->ioctl_buf, ETH_ALEN);

	/* Enable scan mac */
	sm = (wl_scanmac_t *)buffer;
	sm_enable = (wl_scanmac_enable_t *)sm->data;
	sm->len = sizeof(*sm_enable);
	sm_enable->enable = 1;
	len = OFFSETOF(wl_scanmac_t, data) + sm->len;
	sm->subcmd_id = WL_SCANMAC_SUBCMD_ENABLE;

	err = wldev_iovar_setbuf_bsscfg(dev, "scanmac",
		sm, len, cfg->ioctl_buf, WLC_IOCTL_SMLEN, 0, &cfg->ioctl_buf_sync);

	if (err != BCME_OK) {
		WL_ERR(("failed to enable scanmac, err=%d\n", err));
		return err;
	}

	/* Configure scanmac */
	memset(buffer, 0x0, sizeof(buffer));
	sm_config = (wl_scanmac_config_t *)sm->data;
	sm->len = sizeof(*sm_config);
	sm->subcmd_id = WL_SCANMAC_SUBCMD_CONFIG;
	sm_config->scan_bitmap = WL_SCANMAC_SCAN_UNASSOC;

	/* Set current mac address */
	memcpy(&sm_config->mac.octet, current_mac, ETH_ALEN);
	sm_config->mac.octet[3] = 0x0;
	sm_config->mac.octet[4] = 0x0;
	sm_config->mac.octet[5] = 0x0;

	/* Set randomize mac address(last 3bytes) */
	memset(&sm_config->random_mask.octet, 0x0, ETH_ALEN);
	sm_config->random_mask.octet[3] = 0xff;
	sm_config->random_mask.octet[4] = 0xff;
	sm_config->random_mask.octet[5] = 0xff;

	len = OFFSETOF(wl_scanmac_t, data) + sm->len;

	err = wldev_iovar_setbuf_bsscfg(dev, "scanmac",
		sm, len, cfg->ioctl_buf, WLC_IOCTL_SMLEN, 0, &cfg->ioctl_buf_sync);

	if (err != BCME_OK) {
		WL_ERR(("failed scanmac configuration\n"));

		/* Disable scan mac for clean-up */
		wl_cfg80211_random_mac_disable(dev);
		return err;
	}

	WL_ERR(("random MAC enable done"));
	return err;
}

int
wl_cfg80211_random_mac_disable(struct net_device *dev)
{
	s32 err = BCME_ERROR;
	uint8 buffer[20] = {0, };
	wl_scanmac_t *sm = NULL;
	int len = 0;
	wl_scanmac_enable_t *sm_enable = NULL;
	struct bcm_cfg80211 *cfg = g_bcm_cfg;

	sm = (wl_scanmac_t *)buffer;
	sm_enable = (wl_scanmac_enable_t *)sm->data;
	sm->len = sizeof(*sm_enable);
	sm_enable->enable = 0;
	len = OFFSETOF(wl_scanmac_t, data) + sm->len;

	sm->subcmd_id = WL_SCANMAC_SUBCMD_ENABLE;

	err = wldev_iovar_setbuf_bsscfg(dev, "scanmac",
		sm, len, cfg->ioctl_buf, WLC_IOCTL_SMLEN, 0, &cfg->ioctl_buf_sync);

	if (err != BCME_OK) {
		WL_ERR(("failed to disable scanmac, err=%d\n", err));
		return err;
	}

	WL_ERR(("random MAC disable done\n"));
	return err;
}
#endif /* SUPPORT_RANDOM_MAC_SCAN */

int
wl_cfg80211_iface_count(void)
{
	struct bcm_cfg80211 *cfg = g_bcm_cfg;
	struct net_info *iter, *next;
	int iface_count = 0;

	for_each_ndev(cfg, iter, next) {
		if (iter->ndev) {
			iface_count++;
		}
	}
	return iface_count;
}

#ifdef DHD_LOG_DUMP
struct bcm_cfg80211*
wl_get_bcm_cfg80211_ptr(void)
{
	return g_bcm_cfg;
}
#endif /* DHD_LOG_DUMP */

#define CHECK_DONGLE_IDLE_TIME	50
#define CHECK_DONGLE_IDLE_CNT	100
int
wl_check_dongle_idle(struct wiphy *wiphy)
{
	int error = 0;
	struct bcm_cfg80211 *cfg = wiphy_priv(wiphy);
	struct net_device *primary_ndev;
	int retry = 0;
	struct channel_info ci;
	if (!cfg)
		return FALSE;
	/* Use primary I/F for sending cmds down to firmware */
	primary_ndev = bcmcfg_to_prmry_ndev(cfg);

	while (retry++ < CHECK_DONGLE_IDLE_CNT) {
		error = wldev_ioctl(primary_ndev, WLC_GET_CHANNEL, &ci, sizeof(ci), false);
		if (error != BCME_OK || ci.scan_channel != 0) {
			if (error == -ENODEV) {
				WL_ERR(("Firmware is not ready, return TRUE\n"));
				return TRUE;
			}
			WL_ERR(("Firmware is busy(err:%d scan channel:%d). wait %dms\n",
				error, ci.scan_channel, CHECK_DONGLE_IDLE_TIME));
		} else {
			break;
		}
		wl_delay(CHECK_DONGLE_IDLE_TIME);
	}
	if (retry >= CHECK_DONGLE_IDLE_CNT) {
		WL_ERR(("DONGLE is BUSY too long\n"));
		return FALSE;
	}
	WL_DBG(("DONGLE is idle\n"));
	return TRUE;
}

#ifdef WES_SUPPORT
#ifdef CUSTOMER_SCAN_TIMEOUT_SETTING
s32 wl_cfg80211_custom_scan_time(enum wl_custom_scan_time_type type, int time)
{
	struct bcm_cfg80211 *cfg = NULL;

	cfg = g_bcm_cfg;

	if (cfg == NULL) {
		return FALSE;
	}

	switch (type) {
		case WL_CUSTOM_SCAN_CHANNEL_TIME :
			WL_ERR(("Scan Channel Time %d\n", time));
			cfg->custom_scan_channel_time = time;
			break;
		case WL_CUSTOM_SCAN_HOME_TIME :
			WL_ERR(("Scan Home Time %d\n", time));
			cfg->custom_scan_home_time = time;
			break;
		case WL_CUSTOM_SCAN_HOME_AWAY_TIME :
			WL_ERR(("Scan Home Away Time %d\n", time));
			cfg->custom_scan_home_away_time = time;
			break;
		default:
			return FALSE;
	}
	return TRUE;
}
#endif /* CUSTOMER_SCAN_TIMEOUT_SETTING */
#endif /* WES_SUPPORT */

#endif

#ifdef CONFIG_HW_ABS
static s32
wl_notify_ant_event(struct bcm_cfg80211 *cfg, bcm_struct_cfgdev *cfgdev,
const wl_event_msg_t *e, void *data)
{
	struct net_device *ndev = NULL;

	ndev = cfgdev_to_wlc_ndev(cfgdev, cfg);

	WL_ERR(("%s : Handle WLC_E_ANT_EVENT\n", __FUNCTION__));
	cfg80211_drv_ant_grab(ndev, GFP_KERNEL);
	return 0;
}
#endif
