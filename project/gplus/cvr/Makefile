TOPDIR := ../../../
include $(TOPDIR)sdk/build/core/config.mak

SUBDIRS += ../../common/sysinit
SUBDIRS += ../../common/initgama
SUBDIRS += app/car_recorder
#SUBDIRS += app/extended_app
#SUBDIRS += app/dvr
SUBDIRS += app/camcorder
#SUBDIRS += app/dc
#SUBDIRS += app/disp
SUBDIRS += app/cvr_player
SUBDIRS += app/uvc_csi
SUBDIRS += app/v4l2

.PHONY: all
all:
	+$(COMMAND)

.PHONY: clean
clean:
	+$(COMMAND)

.PHONY: release
release:
	+$(COMMAND)

.PHONY: project_post_all
project_post_all:


# ###############################################################
# Collect platform root directory
# ###############################################################
.PHONY: collect_rootfs
collect_rootfs:
#	Copy common/rootfs/* to /out/rootfs
	@chmod a+x $(TOPDIR)project/common/$(SYSCONFIG_ARCH)/rootfs/usr/bin/*
	@$(CP) -R -f -u $(TOPDIR)project/common/$(SYSCONFIG_ARCH)/rootfs/* $(PRODUCT_DIR)/rootfs/

#	Copy <PROJECT_DIR>/rootfs/* to /out/rootfs
	@$(CP) -R -f -u ./rootfs/* $(PRODUCT_DIR)/rootfs/
	
	@-$(RM) -f $(PRODUCT_DIR)/rootfs/usr/bin/color_bar
	@-$(RM) -f $(PRODUCT_DIR)/rootfs/usr/bin/disktype
	@-$(RM) -f $(PRODUCT_DIR)/rootfs/usr/bin/gpioread
	@-$(RM) -f $(PRODUCT_DIR)/rootfs/usr/bin/gpiowrite
	@-$(RM) -f $(PRODUCT_DIR)/rootfs/usr/bin/pintest
	@-$(RM) -f $(PRODUCT_DIR)/rootfs/usr/bin/pwm.x
	@-$(RM) -f $(PRODUCT_DIR)/rootfs/usr/bin/regutil

#	Remove .svn folder
	@$(FIND) $(PRODUCT_DIR)/rootfs/ -name "\.svn" -type d | xargs --no-run-if-empty $(RM) -fR

.PHONY: collect_system
collect_system:
#	// cp common/system
	@$(CP) -R -f -u $(TOPDIR)project/common/system/* $(PRODUCT_DIR)/system
	
#	// cp project/xx/xx/system
	@$(CP) -R -f -u ./system/* $(PRODUCT_DIR)/system

#	// cp <PROJECT_DIR>/system
	@$(CP) -R -f -u $(KERNEL_OUT_MODULES)/* $(PRODUCT_DIR)/system
	@-$(RM) -r -f $(PRODUCT_DIR)/system/lib/modules/2.6.32.94/kernel/arch/arm/mach-gpl32900b
	@-$(RM) -r -f $(PRODUCT_DIR)/system/lib/modules/2.6.32.94/kernel/fs
	@$(MKDIR) -p $(PRODUCT_DIR)/system/lib/modules/2.6.32.94/kernel/arch/arm/mach-gpl32900b
	@$(CP) -R -f -u $(KERNEL_OUT_MODULES)/lib/modules/2.6.32.94/kernel/arch/arm/mach-gpl32900b/usb $(PRODUCT_DIR)/system/lib/modules/2.6.32.94/kernel/arch/arm/mach-gpl32900b
	
#	Remove .svn folder
	@$(FIND) $(PRODUCT_DIR)/system/ -name "\.svn" -type d | xargs --no-run-if-empty $(RM) -fR

#	Copy WiFi system folder to target file system
ifneq ($(SYSCONFIG_USB_WIFI),None)
	@$(CP) -R -f -u $(TOPDIR)project/common/wifi/system/* $(PRODUCT_DIR)/system
endif
	@-$(RM) -f $(PRODUCT_DIR)/system/bin/fancybrowser
	@-$(RM) -f $(PRODUCT_DIR)/system/bin/gdbserver
	@-$(RM) -f $(PRODUCT_DIR)/system/bin/hostapd
	@-$(RM) -f $(PRODUCT_DIR)/system/bin/hostapd_cli
	@-$(RM) -f $(PRODUCT_DIR)/system/bin/dosfsck
	
	@-$(RM) -f $(PRODUCT_DIR)/system/lib/libcrypt.so.1
	@-$(RM) -f $(PRODUCT_DIR)/system/lib/libnss_files.so.2
	@-$(RM) -f $(PRODUCT_DIR)/system/lib/libnss_dns.so.2
	@-$(RM) -f $(PRODUCT_DIR)/system/lib/libresolv.so.2
	@-$(RM) -f $(PRODUCT_DIR)/system/lib/libstdc++.so.6
	@-$(RM) -f $(PRODUCT_DIR)/system/lib/libthread_db.so.1
	@-$(RM) -f $(PRODUCT_DIR)/system/lib/libutil.so.1
#	@-$(RM) -f $(PRODUCT_DIR)/system/lib/libwavenc.so
	@-$(CP) -R -f -u $(TOPDIR)/sdk/lib/libImageIF.so $(PRODUCT_DIR)/system/lib/

	@-$(RM) -R -f $(PRODUCT_DIR)/rootfs/lib/modules/2.6.32.94/kernel/arch/arm/mach-gpl32900b/dc2dc

ifeq ($(SYSCONFIG_TOOLCHAIN_SEL), uclibc)
	@-$(RM) -f $(PRODUCT_DIR)/system/lib/librt.so.1
	@-$(RM) -f $(PRODUCT_DIR)/system/lib/libpthread.so.0
	@-$(RM) -f $(PRODUCT_DIR)/rootfs/lib/ld-linux.so.3
	@-$(RM) -f $(PRODUCT_DIR)/rootfs/lib/libc.so.6
	@-$(RM) -f $(PRODUCT_DIR)/rootfs/lib/libdl.so.2
	@-$(RM) -f $(PRODUCT_DIR)/rootfs/lib/libm.so.6
	@-$(RM) -f $(PRODUCT_DIR)/rootfs/lib/libgcc_s.so.1
	@$(CP) -R -f -u $(TOPDIR)project/common/uClibc/* $(PRODUCT_DIR)/rootfs/lib/
endif
ifeq ($(SYSCONFIG_TOOLCHAIN_SEL), musl)
	@-$(RM) -f $(PRODUCT_DIR)/system/lib/librt.so.1
	@-$(RM) -f $(PRODUCT_DIR)/system/lib/libpthread.so.0
	@-$(RM) -f $(PRODUCT_DIR)/rootfs/lib/ld-linux.so.3
	@-$(RM) -f $(PRODUCT_DIR)/rootfs/lib/libc.so.6
	@-$(RM) -f $(PRODUCT_DIR)/rootfs/lib/libdl.so.2
	@-$(RM) -f $(PRODUCT_DIR)/rootfs/lib/libm.so.6
	@-$(RM) -f $(PRODUCT_DIR)/rootfs/lib/libgcc_s.so.1
endif
	@-$(RM) -f $(PRODUCT_DIR)/system/lib/libgp_video_stream.so
	@-$(RM) -f $(PRODUCT_DIR)/system/lib/modules/common/panel_lcd_auo_A043FL01.ko
	@-$(RM) -f $(PRODUCT_DIR)/system/lib/modules/dhd.ko
	@-$(RM) -f $(PRODUCT_DIR)/system/lib/modules/gp_joystick_driver.ko
#	@-$(RM) -f $(PRODUCT_DIR)/system/lib/modules/key_driver.ko
	@-$(RM) -f $(PRODUCT_DIR)/system/lib/modules/touch_driver.ko
	@-$(RM) -f $(PRODUCT_DIR)/system/lib/modules/board_config.ko
	
	@-$(RM) -f $(PRODUCT_DIR)/system/app/double_stream_test.x
	@-$(RM) -f $(PRODUCT_DIR)/system/app/Decode_demo_test.x
	@-$(RM) -f $(PRODUCT_DIR)/system/app/sensor_attr.x
	@-$(RM) -f $(PRODUCT_DIR)/system/app/csi_md_test.x
	@-$(RM) -f $(PRODUCT_DIR)/system/app/audio_test.x
	@-$(RM) -f $(PRODUCT_DIR)/system/app/dc.x
	
