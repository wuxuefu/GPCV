include $(TOPDIR)/out/$(PRODUCT)/config/sysconfig.mak

# ###############################################################
# Sensors
# ###############################################################

ifneq ($(SYSCONFIG_SENSOR0),None)
	obj-m += $(SYSCONFIG_SENSOR0).o
endif

ifneq ($(SYSCONFIG_SENSOR1),None)
	obj-m += $(SYSCONFIG_SENSOR1).o
endif

ifneq ($(SYSCONFIG_SENSOR2),None)
	obj-m += $(SYSCONFIG_SENSOR2).o
endif