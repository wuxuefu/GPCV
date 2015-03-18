#
# Makefile for the storage interface
#

ifeq ($(CONFIG_PM_SPEEDY),y)
obj-y += gp_speedy_boot.o
endif

obj-m += gp_storage_api.o
gp_gp_storage_api-objs := gp_storage_api.o

obj-m += gp_snapshot.o

ifeq ($(CONFIG_PM_SPEEDY),y)
gp_snapshot-objs := snapshot.o
else
gp_snapshot-objs := warp_snapshot.o
endif
