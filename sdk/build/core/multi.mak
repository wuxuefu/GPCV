
include $(TOPDIR)sdk/build/core/config.mak

################################################################
# Functions
################################################################
rename_folder = \
	@$(ECHO) "rename folder $(2)_$(1)" ; \
	if [ -d $(2)_$(1) ] ; then \
		$(RM) -fr $(2)_$(1) ; \
	fi ;\
	$(MV) $(2) $(2)_$(1) ; \
	
install_platform = \
	@$(MAKE) PRODUCT=$(SYSCONFIG_$(1)) -C $(TOPDIR) -j16 ; \
	$(CP) -fr $(TOPDIR)out/$(SYSCONFIG_$(1))/rootfs $(PRODUCT_DIR)/ ; \
	$(CP) -r $(TOPDIR)out/$(SYSCONFIG_$(1))/system $(PRODUCT_DIR)/ ; \
	$(CP) -u $(TOPDIR)out/$(SYSCONFIG_$(1))/Image $(PRODUCT_DIR)/Image_$(1) ; \
	$(CP) -u $(TOPDIR)out/$(SYSCONFIG_$(1))/cmdline.txt $(PRODUCT_DIR)/cmdline_$(1).txt ; \
#	$(MAKE) PRODUCT=$(SYSCONFIG_$(1)) -C $(TOPDIR) clean ; \
#	$(MAKE) PRODUCT=$(SYSCONFIG_$(1)) -C $(TOPDIR) kernel_clean ; \

# ###############################################################
# all
# ###############################################################
.PHONY: all
all: product

.PHONY: clean
clean: product_clean

# ###############################################################
# product
# ###############################################################
.PHONY: product
product:
	@$(MKDIR) -p $(PRODUCT_DIR)/rootfs
	@$(MKDIR) -p $(PRODUCT_DIR)/system
	$(call install_platform,GPL32900)
	$(call rename_folder,GPL32900,$(PRODUCT_DIR)/rootfs/lib/modules)
	$(call rename_folder,GPL32900,$(PRODUCT_DIR)/system/lib/modules)
	$(call rename_folder,GPL32900,$(PRODUCT_DIR)/system/app)
	$(call install_platform,GPL32900B)
	$(call rename_folder,GPL32900B,$(PRODUCT_DIR)/rootfs/lib/modules)
	$(call rename_folder,GPL32900B,$(PRODUCT_DIR)/system/lib/modules)
	$(call rename_folder,GPL32900B,$(PRODUCT_DIR)/system/app)
	@$(MAKE) oImage

.PHONY: product_clean
product_clean: oImage_clean
	@$(MAKE) PRODUCT=$(SYSCONFIG_GPL32900) -C $(TOPDIR) clean
	@$(MAKE) PRODUCT=$(SYSCONFIG_GPL32900B) -C $(TOPDIR) clean

# ###############################################################
# Product initramfs
# ###############################################################
.PHONY: initramfs
initramfs:
	@$(ECHO) "  Create initramfs"
#	// cp initramfs.cpio from base.cpio
	@$(CP) -f $(TOPDIR)project/common/base/base.cpio $(PRODUCT_DIR)/initramfs.cpio

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
	
# ###############################################################
# Images and initramfs
# ###############################################################
.PHONY: oImage
oImage: initramfs
	@$(ECHO) "  Packing images"
#	// pack kernel, initramfs_oImage, and cmdline
	@$(KERNEL_MULTI_PACKER) kernel_GPL32900=$(PRODUCT_DIR)/Image_GPL32900 cmdline_GPL32900=$(PRODUCT_DIR)/cmdline_GPL32900.txt kernel_GPL32900B=$(PRODUCT_DIR)/Image_GPL32900B cmdline_GPL32900B=$(PRODUCT_DIR)/cmdline_GPL32900B.txt initrd=$(INITRAMFS_OIMAGE) out=$(PRODUCT_DIR)/packed_oImage.bin
#	// concat u-boot(padding to 256KB) and packed_oImage.bin
	@$(PADDING) $(SDK_DIR)/prebuild/bootloader/u-boot_ram_GPL32900B.bin $(PRODUCT_DIR)/oImage 262144
	@$(CAT) $(PRODUCT_DIR)/packed_oImage.bin >> $(PRODUCT_DIR)/oImage
	@$(LS) -l $(PRODUCT_DIR)/oImage

#	// pack kernel, initramfs, and cmdline
	@$(KERNEL_MULTI_PACKER) kernel_GPL32900=$(PRODUCT_DIR)/Image_GPL32900 cmdline_GPL32900=$(PRODUCT_DIR)/cmdline_GPL32900.txt kernel_GPL32900B=$(PRODUCT_DIR)/Image_GPL32900B cmdline_GPL32900B=$(PRODUCT_DIR)/cmdline_GPL32900B.txt initrd=$(INITRAMFS_IMAGE) out=$(PRODUCT_DIR)/packed.bin
#	// concat u-boot(padding to 256KB) and packed.bin
	@$(PADDING) $(SDK_DIR)/prebuild/bootloader/u-boot_ram_GPL32900B.bin $(PRODUCT_DIR)/kernel.bin 262144
	@$(CAT) $(PRODUCT_DIR)/packed.bin >> $(PRODUCT_DIR)/kernel.bin
	@$(LS) -l $(PRODUCT_DIR)/kernel.bin
	
	@$(MKCRAMFS) ./system ./system.bin
	
.PHONY: oImage_clean
oImage_clean:
#	// rm generated files
	@-$(RM) -fr $(PRODUCT_DIR)/rootfs/
	@-$(RM) -fr $(PRODUCT_DIR)/system/
	@-$(RM) -f $(PRODUCT_DIR)/cmdline_*.txt
	@-$(RM) -f $(PRODUCT_DIR)/Image_*
	@-$(RM) -f $(PRODUCT_DIR)/initramfs.cpio
	@-$(RM) -f $(PRODUCT_DIR)/initramfs_oImage.igz
	@-$(RM) -f $(PRODUCT_DIR)/packed_oImage.bin
	@-$(RM) -f $(PRODUCT_DIR)/oImage
	@-$(RM) -f $(PRODUCT_DIR)/initramfs.igz
	@-$(RM) -f $(PRODUCT_DIR)/packed.bin
	@-$(RM) -f $(PRODUCT_DIR)/kernel.bin
	@-$(RM) -f $(PRODUCT_DIR)/system.bin
