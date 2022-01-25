# Hisilicon HISPFD Makefile
#
ccflags-y += -Idrivers/hisi/tzdriver/libhwsecurec

obj-$(CONFIG_HISPFD) += hispfd.o

ifeq ($(CONFIG_HISPFD_V130),y)
    ifeq ($(chip_type),es)
        EXTRA_CFLAGS += -DHIFD_V120
    else
        EXTRA_CFLAGS += -DHIFD_V130
    endif
else
    EXTRA_CFLAGS += -DHIFD_V110
endif

