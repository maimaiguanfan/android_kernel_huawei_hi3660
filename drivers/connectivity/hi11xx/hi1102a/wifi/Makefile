HI1102_DRIVER_BUILTIN_PATH ?= $(subst ",,$(CONFIG_HI11XX_DRIVER_PATH))
$(warning HI1102_DRIVER_BUILTIN_PATH $(HI1102_DRIVER_BUILTIN_PATH))
WIFI_DRIVER_FLODER =
PLAT_DRIVER_FLODER = ../platform
ANDROID_WIFI_DRIVER_FLODER = wifi
ANDROID_PLAT_DRIVER_FLODER = platform

ifeq ($(HI110X_DRIVER_VERSION),1102)
export _PRE_PRODUCT_VERSION  ?= 1102
export HI1102_BOARD_VERSION ?= v8r2
else
export _PRE_PRODUCT_VERSION  ?= 1102a
export HI1102_BOARD_VERSION ?= default
endif

export PLAT_DEFCONFIG_FILE ?= wifi_$(_PRE_PRODUCT_VERSION)_$(HI1102_BOARD_VERSION)_defconfig

HI1102_COMM_DEFCONFIG := hi$(_PRE_PRODUCT_VERSION)_comm_defconfig

CONFIG_HI110X_KERNEL_MODULES_BUILD_SUPPORT ?= no

ifneq ($(KERNELRELEASE),)

ifeq ($(HI1102_MAKE_FLAG),)
#android floder
KERNEL_DIR := $(srctree)
HI1102_DRIVER_SRC_PATH=$(KERNEL_DIR)/$(HI1102_DRIVER_BUILTIN_PATH)/hi$(_PRE_PRODUCT_VERSION)
WIFI_DRIVER_PATH=$(HI1102_DRIVER_SRC_PATH)/$(ANDROID_WIFI_DRIVER_FLODER)
PLAT_DRIVER_PATH=$(HI1102_DRIVER_SRC_PATH)/$(ANDROID_PLAT_DRIVER_FLODER)
else
#local driver floder
WIFI_DRIVER_PATH =$(HI1102_DRIVER_SRC_PATH)
PLAT_DRIVER_PATH =$(HI1102_DRIVER_SRC_PATH)/$(PLAT_DRIVER_FLODER)
endif

ifeq ($(HI110X_DRIVER_VERSION),1102)
include $(HI1102_DRIVER_SRC_PATH)/../1102/platform/$(HI1102_COMM_DEFCONFIG)
else
include $(PLAT_DRIVER_PATH)/$(HI1102_COMM_DEFCONFIG)
endif
include $(WIFI_DRIVER_PATH)/$(PLAT_DEFCONFIG_FILE)

$(warning defconfig: $(WIFI_DRIVER_PATH)/$(PLAT_DEFCONFIG_FILE))

hmac-objs := hmac_main.o hmac_rx_data.o hmac_tx_amsdu.o hmac_tx_data.o hmac_user.o hmac_vap.o hmac_mgmt_bss_comm.o hmac_mgmt_classifier.o hmac_fsm.o hmac_mgmt_ap.o hmac_mgmt_sta.o hmac_encap_frame_ap.o hmac_encap_frame_sta.o hmac_encap_frame.o hmac_sme_sta.o hmac_11i.o hmac_uapsd.o hmac_config.o hmac_protection.o hmac_frag.o hmac_crypto_tkip.o hmac_chan_mgmt.o hmac_m2u.o hmac_smps.o hmac_data_acq.o hmac_rx_filter.o hmac_proxy_arp.o hmac_hcc_adapt.o hmac_reset.o hmac_scan.o hmac_p2p.o hmac_device.o hmac_resource.o  hmac_wapi_wpi.o hmac_wapi_sms4.o hmac_wapi.o hmac_cali_mgmt.o hmac_cali_dpd.o
hmac-objs += hmac_custom_security.o hmac_scan.o hmac_blockack.o hmac_dfs.o hmac_psm_ap.o hmac_dfx.o hmac_edca_opt.o hmac_arp_offload.o hmac_verify/hmac_test_main.o  hmac_tcp_opt.o hmac_roam_alg.o hmac_roam_connect.o hmac_roam_main.o
hmac-objs += hmac_traffic_classify.o hmac_proxysta.o hmac_wmmac.o hmac_statistic_data_flow.o
hmac-objs += hmac_btcoex.o hmac_11v.o
hmac-objs := $(addprefix hmac/,$(hmac-objs))

mac-objs := mac_device.o mac_frame.o mac_user.o mac_vap.o mac_regdomain.o mac_ie.o mac_resource.o mac_board.o mac_pm.o mac_data.o
mac-objs := $(addprefix mac_ec/,$(mac-objs))
mac-ram-objs := mac_board_ram.o mac_data_ram.o mac_device_ram.o mac_frame_ram.o mac_ie_ram.o mac_pm_ram.o mac_regdomain_ram.o mac_resource_ram.o mac_user_ram.o mac_vap_ram.o
mac-ram-objs := $(addprefix mac_ram/,$(mac-ram-objs))

hmac-objs += $(mac-objs)
hmac-objs += $(mac-ram-objs)

wal-objs := wal_config.o wal_config_debug.o wal_linux_bridge.o  wal_main.o wal_linux_scan.o wal_linux_rx_rsp.o wal_linux_event.o wal_regdb.o wal_config_acs.o wal_linux_flowctl.o  wal_linux_ioctl.o wal_linux_ioctl_debug.o wal_linux_atcmdsrv.o
wal-objs += wal_linux_cfg80211.o wal_dfx.o wal_linux_cfgvendor.o
wal-objs := $(addprefix wal/,$(wal-objs))

main-objs := main.o
main-objs := $(addprefix main/,$(main-objs))

#wifi.ko
wifi-objs := $(wal-objs)

wifi-objs += $(main-objs)

wifi-objs += $(hmac-objs)

wifi_$(_PRE_PRODUCT_VERSION)-objs = $(wifi-objs)

#main.ko
ifeq ($(CONFIG_HI110X_KERNEL_MODULES_BUILD_SUPPORT), yes)
obj-m += wifi.o
else
obj-y += wifi_$(_PRE_PRODUCT_VERSION).o
endif


HI1102_INCLUDES := -I$(WIFI_DRIVER_PATH)/inc

ifeq ($(HI110X_DRIVER_VERSION),1102)
HI1102_INCLUDES += -I$(HI1102_DRIVER_SRC_PATH)/../1102/platform/inc
HI1102_INCLUDES += -I$(HI1102_DRIVER_SRC_PATH)/../1102/platform/board
else
HI1102_INCLUDES += -I$(PLAT_DRIVER_PATH)/inc
HI1102_INCLUDES += -I$(PLAT_DRIVER_PATH)/board
endif

HI1102_INCLUDES += -I$(WIFI_DRIVER_PATH)/inc/hmac
HI1102_INCLUDES += -I$(WIFI_DRIVER_PATH)/inc/wal
HI1102_INCLUDES += -I$(WIFI_DRIVER_PATH)/hmac
HI1102_INCLUDES += -I$(WIFI_DRIVER_PATH)/wal


#This is not good
HI1102_INCLUDES += -I$(KERNEL_DIR)

ifeq ($(HI110X_DRIVER_VERSION),1102)
HI1102_INCLUDES += -I$(HI1102_DRIVER_SRC_PATH)/../1102/platform/inc/frw
HI1102_INCLUDES += -I$(HI1102_DRIVER_SRC_PATH)/../1102/platform/inc/oal
HI1102_INCLUDES += -I$(HI1102_DRIVER_SRC_PATH)/../1102/platform/inc/oal/linux
HI1102_INCLUDES += -I$(HI1102_DRIVER_SRC_PATH)/../1102/platform/inc/pm
HI1102_INCLUDES += -I$(HI1102_DRIVER_SRC_PATH)/../1102/platform/inc/oam
HI1102_INCLUDES += -I$(HI1102_DRIVER_SRC_PATH)/../1102/platform/oam
HI1102_INCLUDES += -I$(HI1102_DRIVER_SRC_PATH)/../1102/platform/pm
HI1102_INCLUDES += -I$(HI1102_DRIVER_SRC_PATH)/../1102/platform/oal
HI1102_INCLUDES += -I$(HI1102_DRIVER_SRC_PATH)/../1102/platform/frw
HI1102_INCLUDES += -I$(HI1102_DRIVER_SRC_PATH)/../1102/platform/sdt
HI1102_INCLUDES += -I$(HI1102_DRIVER_SRC_PATH)/../1102/common/customize
HI1102_INCLUDES += -I$(HI1102_DRIVER_SRC_PATH)/../1102/common/inc
HI1102_INCLUDES += -I$(HI1102_DRIVER_SRC_PATH)/../1102/common/chr_log
HI1102_INCLUDES += -I$(HI1102_DRIVER_SRC_PATH)/../1102/common/oneimage
else
HI1102_INCLUDES += -I$(PLAT_DRIVER_PATH)/inc/frw
HI1102_INCLUDES += -I$(PLAT_DRIVER_PATH)/inc/oal
HI1102_INCLUDES += -I$(PLAT_DRIVER_PATH)/inc/oal/linux
HI1102_INCLUDES += -I$(PLAT_DRIVER_PATH)/inc/pm
HI1102_INCLUDES += -I$(PLAT_DRIVER_PATH)/inc/oam
HI1102_INCLUDES += -I$(PLAT_DRIVER_PATH)/oam
HI1102_INCLUDES += -I$(PLAT_DRIVER_PATH)/pm
HI1102_INCLUDES += -I$(PLAT_DRIVER_PATH)/oal
HI1102_INCLUDES += -I$(PLAT_DRIVER_PATH)/frw
HI1102_INCLUDES += -I$(PLAT_DRIVER_PATH)/sdt
HI1102_INCLUDES += -I$(PLAT_DRIVER_PATH)/../common/customize
HI1102_INCLUDES += -I$(PLAT_DRIVER_PATH)/../common/inc
HI1102_INCLUDES += -I$(PLAT_DRIVER_PATH)/../common/chr_log
HI1102_INCLUDES += -I$(PLAT_DRIVER_PATH)/../common/oneimage
endif

HI1102_INCLUDES += -I$(WIFI_DRIVER_PATH)/inc/dmac
HI1102_INCLUDES += -I$(WIFI_DRIVER_PATH)/inc/hal
HI1102_INCLUDES += -I$(WIFI_DRIVER_PATH)/hal

HI1102_INCLUDES += -I$(WIFI_DRIVER_PATH)/hal/hal_verify/
HI1102_INCLUDES += -I$(WIFI_DRIVER_PATH)/hmac/hmac_verify/
HI1102_INCLUDES += -I$(WIFI_DRIVER_PATH)/dmac/dmac_verify/
HI1102_INCLUDES += -I$(WIFI_DRIVER_PATH)/alg/alg_verify
HI1102_INCLUDES += -I$(WIFI_DRIVER_PATH)/mac_ec
HI1102_INCLUDES += -I$(WIFI_DRIVER_PATH)/alg

#$(warning INCLUDE: $(HI1102_INCLUDES))

ifneq ($(TARGET_BUILD_VARIANT),user)
KBUILD_CFLAGS += -DPLATFORM_DEBUG_ENABLE
endif

EXTRA_CFLAGS = $(HI1102_INCLUDES) $(COPTS)
EXTRA_CFLAGS += $(COPTS)
EXTRA_CFLAGS += $(COMM_COPTS)
EXTRA_CFLAGS += -Werror
EXTRA_CFLAGS +=  -fno-pic
MODFLAGS = -fno-pic
else
#make modules
export HI1102_MAKE_FLAG=MODULES
export HI1102_DRIVER_SRC_PATH ?= $(shell pwd)

ANDROID_PRODUCT=hi6210sft
ifeq ($(ARCH),arm64)
ANDROID_PATH ?= /home/zhouxinfeng/v8/v8r2-sft-bbit
MODULE_PARAM ?= ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu-
else
ANDROID_PATH ?= /home/zhouxinfeng/1102/v8r2-checkin
MODULE_PARAM ?= ARCH=arm CROSS_COMPILE=arm-eabi-
endif

KERNEL_DIR ?= $(ANDROID_PATH)/out/target/product/$(ANDROID_PRODUCT)/obj/KERNEL_OBJ
CROSS_DIR ?= $(ANDROID_PATH)/prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin:$(ANDROID_PATH)/prebuilts/gcc/linux-x86/arm/gcc-linaro-aarch64-linux-gnu-4.8/bin
PATH := $(CROSS_DIR):$(PATH)

CONFIG_HI110X_KERNEL_MODULES_BUILD_SUPPORT ?= yes

default:
	$(MAKE) -C $(KERNEL_DIR) $(MODULE_PARAM)  M=$(HI1102_DRIVER_SRC_PATH) modules

clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(HI1102_DRIVER_SRC_PATH) clean
	
endif
