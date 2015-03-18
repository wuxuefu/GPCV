include $(TOPDIR)/out/$(PRODUCT)/config/sysconfig.mak
#
# Makefile for the touch panel
#

ifeq ($(SYSCONFIG_TOUCH_PANEL_DEVICE),tp_ft5302_1024x600_module)
	obj-m += tp_ft5302_1024x600_module.o
	tp_ft5302_1024x600_module-objs := tp_ft5302.o
endif

ifeq ($(SYSCONFIG_TOUCH_PANEL_DEVICE),tp_ft5302_800x480_module)
	obj-m += tp_ft5302_800x480_module.o
	tp_ft5302_800x480_module-objs := tp_ft5302_800x480.o
endif

ifeq ($(SYSCONFIG_TOUCH_PANEL_DEVICE),tp_IT7260_480x272_module)
	obj-m += tp_IT7260_480x272_module.o
	tp_IT7260_480x272_module-objs := tp_IT7260_480x272.o
endif

ifeq ($(SYSCONFIG_TOUCH_PANEL_DEVICE),gp_gslx680_800x480_module)
	obj-m += gp_gslx680_800x480_module.o
	gp_gslx680_800x480_module-objs := gp_gslx680_800x480.o
endif

ifeq ($(SYSCONFIG_TOUCH_PANEL_DEVICE),tp_IT7256_480x272_module)
	obj-m += tp_IT7256_480x272_module.o
	tp_IT7256_480x272_module-objs := tp_IT7256_480x272.o
endif
