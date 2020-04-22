# slimbus driver
subdir-ccflags-y += -Wall -Werror
ccflags-y += -Idrivers/devkit/audiokit/

EXTRA_CFLAGS += -Idrivers/hisi/slimbus/vendor/candance/include
EXTRA_CFLAGS +=	-Idrivers/hisi/slimbus/vendor/candance
EXTRA_CFLAGS +=	-Isound/soc/hisilicon
EXTRA_CFLAGS += -Idrivers/hisi/mntn/blackbox/platform_hifi/

hisilicon-slimbus-platform-objs		:= vendor/candance/cps_bm.o vendor/candance/src/csmi.o vendor/candance/slimbus_drv.o
hisilicon-slimbus-platform-objs		+= slimbus.o slimbus_utils.o slimbus_64xx.o slimbus_6403.o 

ifeq ($(CONFIG_SND_SOC_HI6402),y)
EXTRA_CFLAGS += -DHI6402_SUPPORT
hisilicon-slimbus-platform-objs += slimbus_6402.o
endif

ifeq ($(CONFIG_SND_SOC_HI6405),y)
hisilicon-slimbus-platform-objs += slimbus_6405.o
endif


ifeq ($(CONFIG_HISI_AUDIO_DEBUG),y)
ifneq ($(kcov),true)
hisilicon-slimbus-platform-objs += slimbus_debug.o
endif
endif
obj-$(CONFIG_HISILICON_PLATFORM_SLIMBUS)		+= hisilicon-slimbus-platform.o
