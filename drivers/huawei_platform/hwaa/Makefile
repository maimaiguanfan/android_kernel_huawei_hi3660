COMILE_BY_MODULE := online
#COMILE_BY_MODULE := offline

ifeq ($(COMILE_BY_MODULE),offline)
CONFIG_HWAA := m
endif

obj-$(CONFIG_HWAA) += src/base/hwaa_utils.o \
		      src/base/hwaa_list.o

obj-$(CONFIG_HWAA) += src/fek/hwaa_fs_callbacks.o

obj-$(CONFIG_HWAA) += src/ioctl/hwaa_ioctl.o \
		      src/ioctl/hwaa_ioctl_impl.o \
		      src/ioctl/hwaa_user_key.o

obj-$(CONFIG_HWAA) += src/tee/hwaa_adapter.o \
		      src/tee/base_alg.o \
		      src/tee/hwaa_alg.o \
		      src/tee/hwaa_tee.o

EXTRA_CFLAGS := -I../../include/ \
		-Iinc/base \
		-Iinc/fek \
		-Iinc/ioctl \
		-Iinc/tee \
                -Idrivers/hisi/tzdriver \
		-Idrivers/hisi/tzdriver/libhwsecurec

ifeq ($(COMILE_BY_MODULE),offline)
PWD := $(shell pwd)

ANDROID_ROOT_DIR := $(PWD)/../../../../../..

KERNEL_SRC_DIR := $(ANDROID_ROOT_DIR)/kernel/linux-4.9

COMPILER_DIR := $(ANDROID_ROOT_DIR)/prebuilts/gcc/linux-x86/aarch64/aarch64-linux-android-4.9

CC := $(COMPILER_DIR)/bin/aarch64-linux-android-gcc
LD := $(COMPILER_DIR)/bin/aarch64-linux-android-ld

KERNEL_OUT := $(ANDROID_ROOT_DIR)/out/target/product/kirin980/obj/KERNEL_OBJ

default:
	$(MAKE) M=$(PWD) CC=$(CC) LD=$(LD) ARCH=arm64 CROSS_COMPILE=aarch64-linux-android- \
	-C $(KERNEL_SRC_DIR) modules V=1 O=$(KERNEL_OUT)

clean:
	$(MAKE) M=$(PWD) -C $(KERNEL_SRC_DIR) clean
endif
