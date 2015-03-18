# Product general makefile
include $(TOPDIR)sdk/build/core/config.mak

################################################################
# Pattern rules
################################################################
%.ko %.order: sysconfig.h
#	@$(ECHO) "COPY $@ to rootfs"
	@files=`find $(KERNEL_OUT_MODULES) -name "$@" -type f -print` && \
	if [ -z $$files ]; then \
		$(ECHO) "$@ is missing"; \
	else \
		for src in $$files; do \
			base=`basename $$src` && \
			dst=`echo $$src | sed "s%^$(KERNEL_OUT_MODULES)/%$(PRODUCT_DIR)/rootfs/%"` && \
			$(INSTALL) -D -m 644 $$src $$dst; \
		done; \
	fi
	
# ###############################################################
# all
# ###############################################################
.PHONY: all
all: product

.PHONY: clean
clean: product_clean

# ###############################################################
# Copy sysconfig.h to local
# ###############################################################
sysconfig.h: $(PRODUCT_DIR)/config/sysconfig.h
	@$(CP) -f $< $@

# ###############################################################
# product
# ###############################################################
.PHONY: product
product:
	$(MAKE) local_dir
	$(MAKE) project_platform_sdk_install
	$(MAKE) oImage
	$(MAKE) firmware

.PHONY: product_clean
product_clean: project_clean platform_clean local_dir_clean oImage_clean firmware_clean

.PHONY: local_dir
local_dir:
	@$(MKDIR) -p rootfs
	@$(MKDIR) -p system

.PHONY: local_dir_clean
local_dir_clean:
	@-$(RM) -rf rootfs
	@-$(RM) -rf system

# ###############################################################
# project, platform and sdk_install simultaneously
# ###############################################################
.PHONY: project_platform_sdk_install
project_platform_sdk_install : project platform sdk_install


.PHONY: sdk_install
sdk_install:
ifeq ($(SYSCONFIG_LUA), y)
	$(MKDIR) -p $(PRODUCT_DIR)/system/lua
	$(CP) -u -R $(OUT_DIR)/lua/* $(PRODUCT_DIR)/system/lua/
endif

ifeq ($(SYSCONFIG_SDL), y)
	$(MKDIR) -p $(PRODUCT_DIR)/system/lib
	$(CP) -u $(OUT_SDK_DIR)/lib/libSDL.so $(PRODUCT_DIR)/system/lib/
ifeq ($(SYSCONFIG_SDL_GFX), y)
	$(CP) -u $(OUT_SDK_DIR)/lib/libSDL_gfx.so $(PRODUCT_DIR)/system/lib/
endif
ifeq ($(SYSCONFIG_SDL_IMAGE), y)
	$(CP) -u $(OUT_SDK_DIR)/lib/libSDL_image.so $(PRODUCT_DIR)/system/lib/
endif
ifeq ($(SYSCONFIG_SDL_TTF), y)
	$(CP) -u $(OUT_SDK_DIR)/lib/libSDL_ttf.so $(PRODUCT_DIR)/system/lib/
endif
endif

ifeq ($(SYSCONFIG_LIBZ), y)
	$(MKDIR) -p $(PRODUCT_DIR)/system/lib
	$(CP) -u $(OUT_SDK_DIR)/lib/libz.so $(PRODUCT_DIR)/system/lib/
endif

ifeq ($(SYSCONFIG_PNG), y)
	$(MKDIR) -p $(PRODUCT_DIR)/system/lib
	$(CP) -u $(OUT_SDK_DIR)/lib/libpng.so $(PRODUCT_DIR)/system/lib/
endif

ifeq ($(SYSCONFIG_LIBID3TAG), y)
	$(MKDIR) -p $(PRODUCT_DIR)/system/lib
	$(CP) -u $(OUT_SDK_DIR)/lib/libid3tag.so $(PRODUCT_DIR)/system/lib/
endif

ifeq ($(SYSCONFIG_JPEG), y)
	$(MKDIR) -p $(PRODUCT_DIR)/system/lib
	$(CP) -u $(OUT_SDK_DIR)/lib/libjpeg.so $(PRODUCT_DIR)/system/lib/
endif

ifeq ($(SYSCONFIG_TIFF), y)
	$(MKDIR) -p $(PRODUCT_DIR)/system/lib
	$(CP) -u $(OUT_SDK_DIR)/lib/libtiff.so $(PRODUCT_DIR)/system/lib/
endif

ifeq ($(SYSCONFIG_FREETYPE2), y)
	$(MKDIR) -p $(PRODUCT_DIR)/system/lib
	$(CP) -u $(OUT_SDK_DIR)/lib/libfreetype2.so $(PRODUCT_DIR)/system/lib/
endif

ifeq ($(SYSCONFIG_TS), y)
	$(MKDIR) -p $(PRODUCT_DIR)/system/lib
	$(CP) -u $(OUT_SDK_DIR)/lib/libts.so $(PRODUCT_DIR)/system/lib/
	$(CP) -u $(OUT_SDK_DIR)/lib/ts_dejitter.so $(PRODUCT_DIR)/system/lib/
	$(CP) -u $(OUT_SDK_DIR)/lib/ts_input.so $(PRODUCT_DIR)/system/lib/
	$(CP) -u $(OUT_SDK_DIR)/lib/ts_linear.so $(PRODUCT_DIR)/system/lib/
	$(CP) -u $(OUT_SDK_DIR)/lib/ts_pthres.so $(PRODUCT_DIR)/system/lib/
	$(CP) -u $(OUT_SDK_DIR)/lib/ts_variance.so $(PRODUCT_DIR)/system/lib/
endif

ifeq ($(SYSCONFIG_LZO), y)
	$(MKDIR) -p $(PRODUCT_DIR)/system/lib
	$(CP) -u $(OUT_SDK_DIR)/lib/liblzo.so $(PRODUCT_DIR)/system/lib/
endif

ifeq ($(SYSCONFIG_EXPAT), y)
	$(MKDIR) -p $(PRODUCT_DIR)/system/lib
	$(CP) -u $(OUT_SDK_DIR)/lib/libexpat.so $(PRODUCT_DIR)/system/lib/
endif

ifeq ($(SYSCONFIG_AUDIOFILE), y)
	$(MKDIR) -p $(PRODUCT_DIR)/system/lib
	$(CP) -u $(OUT_SDK_DIR)/lib/libaudiofile.so $(PRODUCT_DIR)/system/lib/
endif

ifeq ($(SYSCONFIG_ESOUND), y)
	$(MKDIR) -p $(PRODUCT_DIR)/system/lib
	$(CP) -u $(OUT_SDK_DIR)/lib/libesd.so $(PRODUCT_DIR)/system/lib/
endif

ifeq ($(SYSCONFIG_OPENVG), y)
	$(MKDIR) -p $(PRODUCT_DIR)/system/lib
	$(CP) -u $(SDK_DIR)/lib/libOpenVG.so $(PRODUCT_DIR)/system/lib/
endif

ifeq ($(SYSCONFIG_CEVA), y)
	$(MKDIR) -p $(PRODUCT_DIR)/system/lib
	$(CP) -u $(SDK_DIR)/lib/libceva.so $(PRODUCT_DIR)/system/lib/
endif

ifeq ($(SYSCONFIG_ON2), y)
	$(MKDIR) -p $(PRODUCT_DIR)/system/lib
	$(CP) -u $(SDK_DIR)/lib/libgpon2.so $(PRODUCT_DIR)/system/lib/
	$(CP) -u $(SDK_DIR)/lib/libgpon2dec.so $(PRODUCT_DIR)/system/lib/
	$(CP) -u $(SDK_DIR)/lib/libgpon2enc.so $(PRODUCT_DIR)/system/lib/
endif

ifeq ($(SYSCONFIG_FLASH), y)
	$(MKDIR) -p $(PRODUCT_DIR)/system/bin/
	$(CP) -u $(SDK_DIR)/bin/flashplayer/bin/flashplayer $(PRODUCT_DIR)/system/bin/flashplayer
	chmod +x $(PRODUCT_DIR)/system/bin/flashplayer
	$(MKDIR) -p $(PRODUCT_DIR)/system/lib/
	-$(CP) -u $(SDK_DIR)/bin/flashplayer/lib/* $(PRODUCT_DIR)/system/lib/
	$(MKDIR) -p $(PRODUCT_DIR)/system/flash/xse/
	-$(CP) -uR $(SDK_DIR)/lib/libflash/* $(PRODUCT_DIR)/system/flash/xse/
	chmod +x -R $(PRODUCT_DIR)/system/flash/xse/*
	$(MKDIR) -p $(PRODUCT_DIR)/system/flash/xse/cgi/services
	-$(CP) -uR $(SDK_DIR)/bin/ebook/* $(PRODUCT_DIR)/system/flash/xse/cgi/services
	-$(CP) -uR $(SDK_DIR)/bin/emu/common/* $(PRODUCT_DIR)/system/flash/xse/cgi/services
	-$(CP) -f $(SDK_DIR)/bin/emu/$(SYSCONFIG_PLATFORM)/* $(PRODUCT_DIR)/system/flash/xse/cgi/services
#	$(MKDIR) -p $(PRODUCT_DIR)/system/usr/
#	$(CP) -R -u $(SDK_DIR)/bin/flashplayer/$(SYSCONFIG_ARCH)/usr/* $(PRODUCT_DIR)/system/usr/
	@-$(FIND) $(PRODUCT_DIR)/system/flash -iname ".svn" | xargs rm -rf
	$(CP) -u $(SDK_DIR)/lib/libflashjpeg.so $(PRODUCT_DIR)/system/lib/
	$(CP) -u $(SDK_DIR)/lib/libImageIF.so $(PRODUCT_DIR)/system/lib/
ifeq ($(SYSCONFIG_FLASH_EXTERNAL_INTERFACE),y)
	$(CP) -f $(SDK_DIR)/lib/libextinterface.so $(PRODUCT_DIR)/system/lib/
	$(CP) -u $(SDK_DIR)/middleware/libflash/ExternalInterface/GpSystemFunction/src/btcilent/btplaypipe  $(PRODUCT_DIR)/system/bin/btplaypipe
endif
endif

ifeq ($(SYSCONFIG_CODEC_VIDEO), y)
	$(MKDIR) -p $(PRODUCT_DIR)/system/lib/image
	-$(CP) -uR $(SDK_DIR)/lib/codec/$(SYSCONFIG_ARCH)/video/* $(PRODUCT_DIR)/system/lib/image/
endif

ifeq ($(SYSCONFIG_CODEC_IMAGE), y)
	$(MKDIR) -p $(PRODUCT_DIR)/system/lib/image
	-$(CP) -uR $(SDK_DIR)/lib/codec/$(SYSCONFIG_ARCH)/image/* $(PRODUCT_DIR)/system/lib/image/
endif

ifeq ($(SYSCONFIG_BTPLAY), y)
	$(MKDIR) -p $(PRODUCT_DIR)/system/btplay/
	$(CP) -u $(SDK_DIR)/bin/btplay/btplay $(PRODUCT_DIR)/system/btplay/btplay
	$(CP) -u $(SDK_DIR)/bin/btplay/btplay_test $(PRODUCT_DIR)/system/btplay/btplay_test
	chmod +x $(PRODUCT_DIR)/system/btplay/btplay	
	chmod +x $(PRODUCT_DIR)/system/btplay/btplay_test	
endif

ifeq ($(SYSCONFIG_PULSEAUDIO), y)
	$(MKDIR) -p $(PRODUCT_DIR)/system/bin/
	-$(CP) -uR $(SDK_DIR)/bin/pulseaudio/* $(PRODUCT_DIR)/system/bin/
	$(MKDIR) -p $(PRODUCT_DIR)/system/lib/
	-$(CP) -uR $(SDK_DIR)/lib/pulseaudio/* $(PRODUCT_DIR)/system/lib/
endif

ifeq ($(SYSCONFIG_AUDIOMIXER), y)
	$(MKDIR) -p $(PRODUCT_DIR)/system/bin/
	-$(CP) -uR $(SDK_DIR)/bin/audiomixer/* $(PRODUCT_DIR)/system/bin/
	$(MKDIR) -p $(PRODUCT_DIR)/system/lib
	$(CP) -u $(SDK_DIR)/lib/libaudiomixer.so $(PRODUCT_DIR)/system/lib/
endif

ifeq ($(SYSCONFIG_RESAMPLE), y)
	$(MKDIR) -p $(PRODUCT_DIR)/system/lib
	$(CP) -u $(SDK_DIR)/lib/libresample.so $(PRODUCT_DIR)/system/lib/
endif


ifeq ($(SYSCONFIG_MICROWINDOWS), y)
	$(MKDIR) -p $(PRODUCT_DIR)/system/lib
	$(CP) -u $(OUT_SDK_DIR)/lib/libmicrowindows.so $(PRODUCT_DIR)/system/lib/
endif

ifeq ($(SYSCONFIG_QT), y)
	$(MKDIR) -p $(PRODUCT_DIR)/system/QT
	$(CP) -rf $(SDK_DIR)/external/QT-4.8.5/* $(PRODUCT_DIR)/system/QT/
endif
# 
# Copy Audio Decoder Dynamic Libraries
# 
ifeq ($(SYSCONFIG_LIB_AUDIO), y)
	$(MKDIR) -p $(PRODUCT_DIR)/system/lib
	$(CP) -u $(SDK_DIR)/lib/audio/*.so $(PRODUCT_DIR)/system/lib
endif

ifeq ($(SYSCONFIG_LIB_MCP), y)
	$(MKDIR) -p $(PRODUCT_DIR)/system/lib
	$(CP) -u $(SDK_DIR)/lib/libgpmcpvo.so $(PRODUCT_DIR)/system/lib
	$(CP) -u $(SDK_DIR)/lib/libgpmcpao.so $(PRODUCT_DIR)/system/lib
endif

ifeq ($(SYSCONFIG_LIB_FD), y)
	$(MKDIR) -p $(PRODUCT_DIR)/system/lib
ifeq ($(SYSCONFIG_TOOLCHAIN_SEL), musl)
	$(CP) -u $(SDK_DIR)/lib/libfacedetect_musl.so $(PRODUCT_DIR)/system/lib
else
	$(CP) -u $(SDK_DIR)/lib/libfacedetect.so $(PRODUCT_DIR)/system/lib
endif
endif
	
ifeq ($(SYSCONFIG_LIB_VIDDEC), y)
	$(MKDIR) -p $(PRODUCT_DIR)/system/lib
	$(CP) -u $(SDK_DIR)/lib/libaviparser.so 		$(PRODUCT_DIR)/system/lib	
	$(CP) -u $(SDK_DIR)/lib/libqtffparser.so 		$(PRODUCT_DIR)/system/lib
	$(CP) -u $(SDK_DIR)/lib/libmultimediaparser.so 	$(PRODUCT_DIR)/system/lib
	$(CP) -u $(SDK_DIR)/lib/libvideodecoder.so		$(PRODUCT_DIR)/system/lib
	$(CP) -u $(SDK_DIR)/lib/libmcpthread.so			$(PRODUCT_DIR)/system/lib
	$(CP) -u $(SDK_DIR)/lib/libresample.so			$(PRODUCT_DIR)/system/lib
endif

ifeq ($(SYSCONFIG_LIB_VIDSTREAM), y)
	@$(CP) -u $(SDK_DIR)/lib/libgp_video_stream.so $(PRODUCT_DIR)/system/lib
endif

	@$(CP) -u $(SDK_DIR)/lib/libicver.so $(PRODUCT_DIR)/system/lib
	@$(MKDIR) -p $(PRODUCT_DIR)/rootfs/lib
	@$(CP) -u $(SDK_DIR)/lib/libicver.so $(PRODUCT_DIR)/rootfs/lib
	
	@$(CP) -u $(SDK_DIR)/lib/libcdsp.so $(PRODUCT_DIR)/system/lib
	
# ###############################################################
# Project
# ###############################################################
.PHONY: project
project:
	@$(ECHO) "  Building Project"
	@$(MAKE) PRODUCT=$(PRODUCT) -C $(PROJECT_DIR) all

.PHONY: project_clean
project_clean:
	@$(MAKE) PRODUCT=$(PRODUCT) -C $(PROJECT_DIR) clean

# ###############################################################
# Platform
# ###############################################################
.PHONY: platform
platform:
	@$(ECHO) "  Building Platform"
	@$(MAKE) PRODUCT=$(PRODUCT) SYSCONFIG_ARCH=$(SYSCONFIG_ARCH) -C $(PLATFORM_DIR) all

.PHONY: platform_clean
platform_clean:
	@$(MAKE) PRODUCT=$(PRODUCT) SYSCONFIG_ARCH=$(SYSCONFIG_ARCH) -C $(PLATFORM_DIR) clean

# ###############################################################
# Product collect_rootfs and collect_system
# ###############################################################

rootfs_modules=
ifneq ($(SYSCONFIG_SIMULATOR), y)
	rootfs_modules += modules.order
	rootfs_modules += cramfs.ko
	rootfs_modules += squashfs.ko
	rootfs_modules += gp_board.ko
	rootfs_modules += gp_timer_module.ko
	rootfs_modules += gp_pwm_module.ko
	rootfs_modules += board_config.ko
	rootfs_modules += gp_apbdma0_module.ko
	rootfs_modules += gp_i2c_bus_module.ko
	rootfs_modules += gp_ti2c_bus_module.ko
	rootfs_modules += gp_adc_module.ko
	rootfs_modules += gp_dc2dc_module.ko
ifneq ($(SYSCONFIG_MAINSTORAGE), None)
ifeq ($(SYSCONFIG_MAINSTORAGE), gp_usb_disk) #usb as main storage
	rootfs_modules += usbcore.ko
	rootfs_modules += ohci-hcd.ko
	rootfs_modules += ehci-hcd.ko
	rootfs_modules += usb-storage.ko
	rootfs_modules += spmp_udc.ko
	rootfs_modules += gp_usb.ko
else
ifeq ($(SYSCONFIG_MAINSTORAGE), gp_nand_module) #NAND as main storage
	rootfs_modules += nand_hal.ko
	rootfs_modules += gp_nand_module.ko
else #other storage device
	rootfs_modules += $(SYSCONFIG_MAINSTORAGE).ko
endif
endif
endif

ifeq ($(SYSCONFIG_MODULE_ALLIN_ROOTFS), y)
	rootfs_modules += gp_scale_module.ko
	rootfs_modules += gp_scale2_module.ko
	rootfs_modules += hx170dec.ko
	rootfs_modules += hx280enc.ko
	rootfs_modules += memalloc.ko
	rootfs_modules += gp_lbp_module.ko
	rootfs_modules += gp_wdt_module.ko
	rootfs_modules += gp_adc_module.ko
	rootfs_modules += key_driver.ko
ifeq ($(SYSCONFIG_SFLASH), y)
	rootfs_modules += gp_spi_module.ko
endif
ifeq ($(SYSCONFIG_AUDIO), y)
	rootfs_modules += gp_audio.ko
	rootfs_modules += gp_mixer.ko
endif
ifeq ($(SYSCONFIG_SD), y)
	rootfs_modules += gp_sd.ko
endif
ifeq ($(SYSCONFIG_CSI1), y)
	rootfs_modules += sensor_mgr_module.ko
	rootfs_modules += gp_mipi_module.ko
	rootfs_modules += gp_csi1_module.ko
ifneq ($(SYSCONFIG_SENSOR0), None)
	rootfs_modules += $(SYSCONFIG_SENSOR0).ko
endif
endif
ifeq ($(SYSCONFIG_CSI2), y)
	rootfs_modules += sensor_mgr_module.ko
	rootfs_modules += gp_aeawb_module.ko
	rootfs_modules += gp_mipi_module.ko
	rootfs_modules += gp_cdsp_module.ko
ifneq ($(SYSCONFIG_SENSOR0), None)
	rootfs_modules += $(SYSCONFIG_SENSOR0).ko
endif
endif
ifeq ($(SYSCONFIG_A_SENSOR), y)
ifneq ($(SYSCONFIG_A_SENSOR_DEVICE), None)
	rootfs_modules += $(SYSCONFIG_A_SENSOR_DEVICE).ko
endif
endif

endif

ifeq ($(SYSCONFIG_DISP0), y)
ifneq ($(SYSCONFIG_DISP0_PANEL), None)
	rootfs_modules += $(SYSCONFIG_DISP0_PANEL).ko
endif
	rootfs_modules += gp_display.ko
	rootfs_modules += tv_ntsc.ko
	rootfs_modules += gp_fb.ko
	rootfs_modules += gp_tv.ko
	rootfs_modules += gp_hdmi.ko
endif

ifeq ($(SYSCONFIG_DISP1), y)
ifneq ($(SYSCONFIG_DISP1_PANEL), None)
	rootfs_modules += $(SYSCONFIG_DISP1_PANEL).ko
endif
	rootfs_modules += gp_display1.ko
	rootfs_modules += gp_fb1.ko
endif

ifeq ($(SYSCONFIG_DISP2), y)
	rootfs_modules += gp_display2.ko
	rootfs_modules += tv1.ko
	rootfs_modules += gp_fb2.ko
endif
endif

.PHONY: collect_rootfs
collect_rootfs: $(rootfs_modules)
#	Collect project rootfs files
	@$(MAKE) PRODUCT=$(PRODUCT) -C $(PROJECT_DIR) collect_rootfs

#	Collect platform rootfs files
	@$(MAKE) PRODUCT=$(PRODUCT) -C $(PLATFORM_DIR) collect_rootfs
	
#	Remove .svn folder
	@$(FIND) $(PRODUCT_DIR)/rootfs/ -name "\.svn" -type d | xargs --no-run-if-empty $(RM) -fR	

# ###############################################################
# collect_system
# ###############################################################

.PHONY: collect_system
collect_system:
#	//collect_project_system
	@$(MAKE) PRODUCT=$(PRODUCT) -C $(PROJECT_DIR) collect_system
	
#	//collect_platform_system
	@$(MAKE) PRODUCT=$(PRODUCT) -C $(PLATFORM_DIR) collect_system
	
#	//copy_kernel_modules:
ifneq ($(SYSCONFIG_MODULE_ALLIN_ROOTFS), y)
	@$(CP) -R -f $(KERNEL_OUT_MODULES)/* $(PRODUCT_DIR)/system/
endif	
#	Remove .svn folder
	@$(FIND) $(PRODUCT_DIR)/system/ -name "\.svn" -type d | xargs --no-run-if-empty $(RM) -fR

# ###############################################################
# Product initramfs
# ###############################################################
.PHONY: initramfs
initramfs: sysconfig.h collect_rootfs collect_system
ifneq ($(SYSCONFIG_SIMULATOR), y)
	@$(ECHO) "  Create initramfs"
#	// cp initramfs.cpio from base.cpio
ifeq ($(SYSCONFIG_TOOLCHAIN_SEL), uclibc)
	@$(CP) -f $(TOPDIR)project/common/base/base_lite_uclibc.cpio $(PRODUCT_DIR)/initramfs.cpio
endif
ifeq ($(SYSCONFIG_TOOLCHAIN_SEL), musl)
	@$(CP) -f $(TOPDIR)project/common/base/base_lite_musl.cpio $(PRODUCT_DIR)/initramfs.cpio
endif
ifeq ($(SYSCONFIG_TOOLCHAIN_SEL), glibc)
ifeq ($(SYSCONFIG_LITE_BUSYBOX), y)
	@$(CP) -f $(TOPDIR)project/common/base/base_lite.cpio $(PRODUCT_DIR)/initramfs.cpio
else
	@$(CP) -f $(TOPDIR)project/common/base/base.cpio $(PRODUCT_DIR)/initramfs.cpio
endif
endif

#	// cpio <PRODUCT_DIR>/rootfs/*
	@cd ./rootfs; \
	$(FIND) . ! -regex ".*\.svn.*" | $(CPIO) -H newc -o --owner root:root --append -F $(INITRAMFS_CPIO)

#	// compress initramfs for kernel.bin
	@$(CAT) $(INITRAMFS_CPIO) | gzip > $(INITRAMFS_IMAGE)

#	// cpio <PRODUCT_DIR>/system
	@$(FIND) ./system ! -regex ".*\.svn.*" | $(CPIO) -H newc -o --owner root:root --append -F $(INITRAMFS_CPIO)

#	// compress initramfs for oImage
	@$(CAT) $(INITRAMFS_CPIO) | gzip > $(INITRAMFS_OIMAGE)
	@$(LS) -l $(PRODUCT_DIR)/initramfs*
endif

# ###############################################################
# Images and initramfs
# ###############################################################
.PHONY: oImage
oImage: initramfs
ifneq ($(SYSCONFIG_SIMULATOR), y)
	@$(ECHO) "  Packing images"
#	// kernel cmdline
	@$(CP) -f $(PROJECT_DIR)/config/kernel_bootparam.txt $(PRODUCT_DIR)/cmdline.txt
	@$(ECHO) " $(SYSCONFIG_KERNEL_CMDLINE)" >> $(PRODUCT_DIR)/cmdline.txt
#	// pack kernel, initramfs_oImage, and cmdline
	@$(KERNEL_PACKER) kernel=$(KERNEL_OUT_IMAGE) initrd=$(INITRAMFS_OIMAGE) cmdline=$(PRODUCT_DIR)/cmdline.txt out=$(PRODUCT_DIR)/packed_oImage.bin
#	// concat u-boot(padding to 256KB) and packed_oImage.bin
	@$(PADDING) $(SDK_DIR)/prebuild/bootloader/u-boot_ram_$(SYSCONFIG_ARCH).bin $(PRODUCT_DIR)/oImage 262144
	@$(CAT) $(PRODUCT_DIR)/packed_oImage.bin >> $(PRODUCT_DIR)/oImage
	@$(LS) -l $(PRODUCT_DIR)/oImage

#	// pack kernel, initramfs, and cmdline
	@$(KERNEL_PACKER) kernel=$(KERNEL_OUT_IMAGE) initrd=$(INITRAMFS_IMAGE) cmdline=$(PRODUCT_DIR)/cmdline.txt out=$(PRODUCT_DIR)/packed.bin
#	// concat u-boot(padding to 256KB) and packed.bin
	@$(PADDING) $(SDK_DIR)/prebuild/bootloader/u-boot_ram_$(SYSCONFIG_ARCH).bin $(PRODUCT_DIR)/kernel.bin 262144
	@$(CAT) $(PRODUCT_DIR)/packed.bin >> $(PRODUCT_DIR)/kernel.bin
	@$(LS) -l $(PRODUCT_DIR)/kernel.bin
	
ifeq ($(SYSCONFIG_RM_UBOOT), y)
	@$(CP) -f $(PROJECT_DIR)/config/kernel_bootparam.txt $(PRODUCT_DIR)/
	@$(CP) -f $(PROJECT_DIR)/packer/loader_12b.bin $(PRODUCT_DIR)/
	@$(CP) -f $(PROJECT_DIR)/packer/uclinux_packer $(PRODUCT_DIR)/
	
	@-$(RM) -f $(PRODUCT_DIR)/oImage
	cd $(PRODUCT_DIR)/
	./uclinux_packer -k ./Image -r ./initramfs_oImage.igz -c ./kernel_bootparam.txt -l ./loader_12b.bin -i ./oImage -sm 32 -cm 96
endif

endif

.PHONY: oImage_clean
oImage_clean:
ifneq ($(SYSCONFIG_SIMULATOR), y)
#	// rm generated files
	@-$(RM) -f $(PRODUCT_DIR)/cmdline.txt
	@-$(RM) -f $(PRODUCT_DIR)/initramfs.cpio
	@-$(RM) -f $(PRODUCT_DIR)/initramfs_oImage.igz
	@-$(RM) -f $(PRODUCT_DIR)/packed_oImage.bin
	@-$(RM) -f $(PRODUCT_DIR)/oImage
	@-$(RM) -f $(PRODUCT_DIR)/initramfs.igz
	@-$(RM) -f $(PRODUCT_DIR)/packed.bin
	@-$(RM) -f $(PRODUCT_DIR)/kernel.bin
endif

.PHONY: firmware
firmware: 
ifneq ($(SYSCONFIG_SIMULATOR), y)
#	// copy modules
ifneq ($(SYSCONFIG_MODULE_ALLIN_ROOTFS), y)
	$(CP) -R -u $(KERNEL_OUT_MODULES)/* $(PRODUCT_DIR)/system
endif
#	@-$(FIND) ./system_image -type d -regex ".*\.svn" -exec rm -rf {} \;
	@-$(FIND) ./system -iname ".svn" | xargs rm -rf

ifeq ($(SYSCONFIG_MAIN_FILESYSTEM), squashfs)
	@$(MKSQUASHFS) ./system ./system.bin -noappend -all-root
else
	@$(MKCRAMFS) ./system ./system.bin
endif
	@$(LS) -l $(PRODUCT_DIR)/system.bin	

	@$(FIRMWARE_PACKER) kernel=./kernel.bin system=./system.bin out=./firmware.bin	
	@$(LS) -l $(PRODUCT_DIR)/firmware.bin
	
	@$(MAKE) PRODUCT=$(PRODUCT) -C $(PROJECT_DIR) project_post_all
endif

.PHONY: firmware_clean
firmware_clean:
ifneq ($(SYSCONFIG_SIMULATOR), y)
#	// rm generated files
	@-$(RM) -f $(PRODUCT_DIR)/system.bin
	@-$(RM) -f $(PRODUCT_DIR)/firmware.bin
endif

.PHONY: firmware_pack
firmware_pack:
ifneq ($(SYSCONFIG_SIMULATOR), y)
ifeq ($(SYSCONFIG_MAIN_FILESYSTEM), squashfs)
	@$(MKSQUASHFS) ./system ./system.bin -noappend -all-root
else
	@$(MKCRAMFS) ./system ./system.bin
endif
	@$(LS) -l $(PRODUCT_DIR)/system.bin	

	@$(FIRMWARE_PACKER) kernel=./kernel.bin system=./system.bin out=./firmware.bin	
	@$(LS) -l $(PRODUCT_DIR)/firmware.bin
	
	@$(MAKE) PRODUCT=$(PRODUCT) -C $(PROJECT_DIR) project_post_all
endif
