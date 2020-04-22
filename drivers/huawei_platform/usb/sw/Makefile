#
# Makefile for single wire devices
#

ifeq ($(CONFIG_HW_SW_DEVICES), m)
KERNEL_DIR := $(srctree)
ccflags-y += -Ifs/
ccflags-y += -I$(KERNEL_DIR)/
ccflags-y += -fno-pic
endif

sw-objs :=  sw_device.o \
            sw_core.o \
            sw_hid_vendor.o \
            sw_hid.o \
            sw_debug.o \
            sw_utils.o \
            sw_detect.o \
            sw_detect_kb.o \
            sw_detect_chg.o

sw-objs +=  keyboard/keyboard_cmr.o \
            keyboard/keyboard_scm.o

obj-$(CONFIG_HW_SW_DEVICES) := sw.o
