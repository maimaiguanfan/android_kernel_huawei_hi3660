/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2018. All rights reserved.
 * Description: the hw_rscan_whitelist.h for set default root procs
 * Author: ducuixia <ducuixia@huawei.com>
 * Create: 2018-01-19
 */

#ifndef _HW_RSCAN_WHITELIST_H_
#define _HW_RSCAN_WHITELIST_H_
#define RPROC_WHITE_LIST_STR                                                  \
	"/init:/sbin/charger:/sbin/cust_init:/sbin/hdbd:"                     \
	"/sbin/hwservicemanager:/sbin/hw_ueventd:/sbin/ueventd:"              \
	"/sbin/logctl_service:/sbin/oeminfo_nvm_server:/sbin/xlogcat-early:"  \
	"/system/bin/app_process:/system/bin/app_process32:"                  \
	"/system/bin/app_process64:/system/bin/atrace:/system/bin/bastetd:"   \
	"/system/bin/bootstat:/system/bin/bugreport:/system/bin/debuggerd:"   \
	"/system/bin/debuggerd64:/system/bin/dubaid:/system/bin/dumpsys:"     \
	"/system/bin/emcomd:/system/bin/factory_log_service:"                 \
	"/system/bin/filebackup:/system/bin/gzip:/system/bin/hdbd:"           \
	"/system/bin/healthd:/system/bin/hiview:/system/bin/hwnffserver:"     \
	"/system/bin/hwpged:/system/bin/hwpged_m:/system/bin/imonitor:"       \
	"/system/bin/installd:/system/bin/install-recovery.sh:"               \
	"/system/bin/ip:/system/bin/ip6tables:/system/bin/iptables:"          \
	"/system/bin/limit_current:/system/bin/lmkd:/system/bin/logcat:"      \
	"/system/bin/logd:/system/bin/netd:/system/bin/NORMAL:"               \
	"/system/bin/racoon:/system/bin/screencap:/system/bin/screenrecord:"  \
	"/system/bin/sh:/system/bin/shs:/system/bin/statusd:"                 \
	"/system/bin/storaged:/system/bin/system_teecd:"                      \
	"/system/bin/thermalserviced:/system/bin/toolbox:/system/bin/toybox:" \
	"/system/bin/tui_daemon:/system/bin/uncrypt:"                         \
	"/system/bin/update_engine:/system/bin/vdc:/system/bin/vold:"         \
	"/system/bin/xlogcat:/vendor/bin/aptouch_daemon:"                     \
	"/vendor/bin/atcmdserver:/vendor/bin/blkid:/vendor/bin/dhcp6s:"       \
	"/vendor/bin/exfatfsck:/vendor/bin/fmd:/vendor/bin/fsck_msdos:"       \
	"/vendor/bin/gpsdaemon:/vendor/bin/hinetmanager:"                     \
	"/vendor/bin/hw/android.hardware.usb@1.0-service:"                    \
	"/vendor/bin/hw/rild:"                                                \
	"/vendor/bin/hw/vendor.huawei.hardware.hwfs@1.0-service:"             \
	"/vendor/bin/iked:/vendor/bin/irqbalance:"                            \
	"/vendor/bin/modemlogcat_lte:/vendor/bin/oeminfo_nvm_server:"         \
	"/vendor/bin/radvd:/vendor/bin/smithloader:"                          \
	"/vendor/bin/storage_info:/vendor/bin/teecd:/vendor/bin/unrmd:"       \
	"/vendor/vin/ntfs/3g:/eng/init:"                                      \
	"/vendor/bin/hw/vendor.huawei.hardware.hwhiview@1.0-service:"         \
	"/vendor/bin/isplogcat:/vendor/bin/tlogcat:"                          \
	"/sbin/teecd:/system/bin/check_longpress:/system/bin/cust_init:"      \
	"/system/bin/do_ddrtest:/system/bin/dumpstate:"                       \
	"/system/bin/goldeneye:/system/bin/hw_cdmamodem_service:"             \
	"/system/bin/logserver:/system/bin/mobicache:/system/bin/pmom_cat:"   \
	"/system/bin/powerlogd:/system/bin/sample:/system/bin/usbd:"          \
	"/system/vendor/bin/aptouch_daemon:/system/vendor/bin/cs-set:"        \
	"/system/vendor/bin/hilogcat:/system/vendor/bin/hinetmanager:"        \
	"/system/vendor/bin/hiscoutmanager:/system/vendor/bin/iked:"          \
	"/system/vendor/bin/imonitor:/system/vendor/bin/shs:"                 \
	"/vendor/bin/diagserver:/vendor/bin/fcs:/vendor/bin/gcovd:"           \
	"/vendor/bin/hwemerffu:/vendor/bin/modemchr:"                         \
	"/vendor/bin/modemlogcat_via:/vendor/bin/statusd:"                    \
	"/system/bin/tee_auth_daemon"
#endif

