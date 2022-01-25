ccflags-y += -Idrivers/media/huawei/camera

obj-y += hicam_buf.o

# VERSIO and PATCHLEVEL is linux version defined in linux-4.xx/Makefile
CAM_ION_IS_MODERN := $(shell [ $(VERSION) -gt 4 -o $(VERSION) -eq 4 -a $(PATCHLEVEL) -ge 14 ] && echo "true" || echo "no")
#$(info "Modern ION:$(CAM_ION_IS_MODERN)")

ifeq ($(CAM_ION_IS_MODERN), true)
obj-y += hicam_buf_priv.o
else
obj-y += hicam_buf_priv_legacy.o
endif
