# ###############################################################
# Driver modules
# ###############################################################
ifneq ($(KERNELRELEASE),)
include $(TOPDIR)/out/$(PRODUCT)/config/sysconfig.mak
endif

ifneq (,$(findstring lcd,$(SYSCONFIG_DISP0_PANEL)))
	obj-m += lcd/$(SYSCONFIG_ARCH)/
else
	ifneq (,$(findstring lcm,$(SYSCONFIG_DISP0_PANEL)))
		obj-m += lcm/$(SYSCONFIG_ARCH)/
	endif
endif

ifneq ($(SYSCONFIG_SENSOR_DRIVER_NUM),0)
	obj-m += sensor/
endif

ifneq (,$(findstring lcd,$(SYSCONFIG_DISP1_PANEL)))
	obj-m += lcd1/
else
	ifneq (,$(findstring lcm,$(SYSCONFIG_DISP1_PANEL)))
		obj-m += lcm1/
	endif
endif

ifneq ($(SYSCONFIG_G_SENSOR),n)
	obj-m += gsensor/
endif

ifneq ($(SYSCONFIG_TOUCHPAD),n)
	obj-m += touchpad/
endif

ifneq ($(SYSCONFIG_KEYBOARD),n)
	obj-m += keyboard/
endif

ifneq ($(SYSCONFIG_PS2MOUSE),n)
	obj-m += mouse/
endif

ifneq ($(SYSCONFIG_USB_WIFI),None)
	obj-m += usb_wifi/
endif

ifneq ($(SYSCONFIG_TOUCH_PANEL),n)
	obj-m += touch_panel/
endif

ifneq ($(SYSCONFIG_A_SENSOR),n)
	obj-m += asensor/
endif
# ###############################################################
# Rules for build kernel modules
# ###############################################################
ifeq ($(KERNELRELEASE),)
TOPDIR := ../../../
include $(TOPDIR)sdk/build/core/config.mak

USBWIFI_DIR = usb_wifi
COMPAT_DIR = compat-wireless-2.6.39-1

.PHONY: all
all: modules

.PHONY: clean
clean: modules_clean

.PHONY: modules
modules:
	@$(MAKE) TOPDIR=$(shell cd $(TOPDIR); pwd) -C $(KERNEL_HEADERS) M=$(shell pwd) modules
	@$(MKDIR) -p $(KERNEL_OUT_MODULES)/lib/modules/common/
	@$(FIND) . -type f -regex ".*\.ko$$" ! -regex ".*\.svn.*" -exec $(CP) -u -f {} $(KERNEL_OUT_MODULES)/lib/modules/common/ \;
ifeq ($(SYSCONFIG_USB_WIFI), AR9271)
	# compat wireless driver build 
	usb_wifi/make_compat.sh make
	# copy all wifi driver to output folder
	@$(MKDIR) -p $(KERNEL_OUT_MODULES)/lib/modules/common/compat_wireless
	@$(FIND) $(USBWIFI_DIR)/$(COMPAT_DIR) -name *.ko -exec $(CP) -u -f {} $(KERNEL_OUT_MODULES)/lib/modules/common/compat_wireless \;
endif

.PHONY: modules_clean
modules_clean:
	@$(MAKE) TOPDIR=$(shell cd $(TOPDIR); pwd) -C $(KERNEL_HEADERS) M=$(shell pwd) clean
	@$(RM) -rf $(KERNEL_OUT_MODULES)/lib/modules/common/
ifeq ($(SYSCONFIG_USB_WIFI), AR9271)
	$(USBWIFI_DIR)/make_compat.sh clean 
endif

endif
