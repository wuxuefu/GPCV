TOPDIR :=
include $(TOPDIR)sdk/build/core/config.mak
PLATFORM_VER := $(shell ./gen_svn_ver ${TOPDIR})
KERNEL_VER := $(shell ./gen_svn_ver ${KERNEL_SRC})

# ###############################################################
# all
# ###############################################################
.PHONY: all
all: 
	@$(MAKE) buildspec
ifeq ($(PRODUCT), multiplatform)
	@$(MAKE) -C $(PRODUCT_DIR)
else
	@$(MAKE) kernel_sdk
	@$(MAKE) product
endif

.PHONY: buildspec
buildspec:
	@$(ECHO) "======================================================"
	@$(ECHO) "== PRODUCT          : $(PRODUCT)"
	@$(ECHO) "== TARGET           : $(SYSCONFIG_TARGET)"
	@$(ECHO) "== HOST             : $(SYSCONFIG_HOST)"
	@$(ECHO) "== KERNEL           : $(KERNEL_HEADERS)"
#	@$(ECHO) -n "== PROJECT SVN " & svn info -r COMMITTED | grep Revision
	@$(ECHO) "== KERNEL Revision  : $(KERNEL_VER)"
	@$(ECHO) "== PLATFORM Revision: $(PLATFORM_VER)"
	@$(ECHO) "======================================================"
#generate version.h
#	@$(ECHO) "#ifndef __VERSION_H__" >ver_tmp
#	@$(ECHO) "#define __VERSION_H__" >>ver_tmp
#	@$(ECHO) "#define __PLATFORM_VER__ $(PLATFORM_VER)" >>ver_tmp
#ifeq ($(KERNEL_VER), Unknow)
#	@$(ECHO) "#define __KERNEL_VER__ 0" >>ver_tmp
#else
#	@$(ECHO) "#define __KERNEL_VER__ $(KERNEL_VER)" >>ver_tmp
#endif
#	@$(ECHO) "#endif /*__VERSION_H__*/" >>ver_tmp
#	@$(MV) ver_tmp $(SDK_DIR)/include/version.h -f

.PHONY: clean
clean: sdk_clean product_clean

# ###############################################################
# Config
# ###############################################################
.PHONY: config
config:
	@$(MKCONFIG)

.PHONY: config_clean
config_clean:
	@$(RM) -rf out/
	@$(RM) -f product_config.mak

.PHONY: switch
switch:
	@$(MKSWITCH)

.PHONY: multiplatform
multiplatform:
	@$(MKMULTI)

# ###############################################################
# Kernel and SDK simultaneously
# ###############################################################
.PHONY: kernel_sdk
kernel_sdk : kernel sdk

# ###############################################################
# Kernel
# ###############################################################
# install kernel/modules
.PHONY: kernel
kernel:
ifneq ($(KERNEL_SRC),)
	@$(KERNEL_CONFIG_SETUP) $(KERNEL_SRC) $(SYSCONFIG_ARCH) $(SYSCONFIG_DEFCONFIG_FILE)
	$(MAKE) -C $(KERNEL_SRC)
	@$(RM) -rf $(KERNEL_OUT_DIR)
	@$(MKDIR) -p $(KERNEL_OUT_DIR)
	@$(CP) -f $(KERNEL_BUILD_IMAGE) $(KERNEL_OUT_IMAGE)
	@$(MAKE) -C $(KERNEL_SRC) modules_install INSTALL_MOD_PATH=$(TOP_DIR_FULL)/$(KERNEL_OUT_DIR)/modules_install
	@$(RM) $(KERNEL_OUT_DIR)/modules_install/lib/modules/*/build
	@$(RM) $(KERNEL_OUT_DIR)/modules_install/lib/modules/*/source
endif
ifneq ($(KERNEL_HEADERS),)
	@$(LS) -l $(KERNEL_OUT_IMAGE)
else
	@$(ECHO) "Error: kernel source or prebuild not found in sdk/os/ folder!"
endif

# clean kernel
.PHONY: kernel_clean
kernel_clean:
ifneq ($(KERNEL_SRC),)
	$(MAKE) -C $(KERNEL_SRC) clean
	@$(RM) -f $(KERNEL_OUT_IMAGE)
	@$(RM) -rf $(KERNEL_OUT_DIR)
endif

# prebuild kernel
.PHONY: kernel_prebuild
kernel_prebuild:
ifneq ($(KERNEL_SRC),)
	$(CP) -R $(KERNEL_OUT_DIR)/* $(KERNEL_PREBUILD_DIR)/
	$(CP) -f $(KERNEL_SRC)/Module.symvers $(KERNEL_PREBUILD_HEADERS)/Module.symvers
	cd $(TOPDIR)sdk/os/kernel-2.6.32; \
	$(FIND) ./arch/arm/mach-spmp8050/include -type f ! -regex ".*\.svn.*" -exec $(CP) -u -f {} ../kernel-2.6.32-headers/{} \;
endif


# find out if kernel base source exist
#test_kernel_base_mk := $(strip $(wildcard $(TOPDIR)sdk/os/kernel-2.6.32-base/Makefile))
#ifneq ($(test_kernel_base_mk),)
#	KERNEL_SRC_BASE := $(patsubst %/,%,$(dir $(test_kernel_base_mk)))
#endif

.PHONY: kernel_unpack
kernel_unpack:
	@cd sdk/os; tar xzf kernel_patch.tgz
	@cd sdk/os/kernel_patch; ./patch_kernel.sh
	@$(RM) -rf sdk/os/kernel_patch

# ###############################################################
# SDK
# ###############################################################
.PHONY: sdk
sdk:
	@$(ECHO) "  Building SDK"
	@$(MAKE) -C $(SDK_DIR) all

.PHONY: sdk_clean
sdk_clean:
	@$(MAKE) -C $(SDK_DIR) clean

# ###############################################################
# Product (project/platform)
# ###############################################################
.PHONY: product
product:
	@$(MAKE) -C $(PRODUCT_DIR) product

.PHONY: product_clean
product_clean:
	@$(MAKE) -C $(PRODUCT_DIR) product_clean

.PHONY: project
project:
	@$(MAKE) -C $(PRODUCT_DIR) project

.PHONY: project_clean
project_clean:
	@$(MAKE) -C $(PRODUCT_DIR) project_clean

.PHONY: platform
platform:
	@$(MAKE) -C $(PRODUCT_DIR) platform

.PHONY: platform_clean
platform_clean:
	@$(MAKE) -C $(PRODUCT_DIR) platform_clean

# ###############################################################
# Image
# ###############################################################
#oImage: kernel sdk
#	@$(MAKE) -C $(PRODUCT_DIR) oImage
.PHONY: oImage
oImage:
	@$(ECHO) 'Please use "make" only to build oImage'

.PHONY: oImage_clean
oImage_clean:
#	@$(MAKE) -C $(PRODUCT_DIR) oImage_clean

.PHONY: firmware_pack
firmware_pack:
	@$(MAKE) -C $(PRODUCT_DIR) firmware_pack
