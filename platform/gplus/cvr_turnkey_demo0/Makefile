# ###############################################################
# Driver modules
# ###############################################################
obj-m += board_config.o
obj-m += key_driver.o
obj-m += touch_driver.o
#obj-m += external_rtc.o
obj-m += gp_joystick_driver.o

# ###############################################################
# Rules for build kernel modules
# ###############################################################
ifeq ($(KERNELRELEASE),)
TOPDIR := ../../../
include $(TOPDIR)sdk/build/core/config.mak

CFLAGS += -I$(SDK_DIR)/include
CFLAGS += -I$(SDK_DIR)/middleware/libVideoDecoder/include
CFLAGS += -I$(PRODUCT_DIR)
CFLAGS += -I.
CFLAGS += -I./include
CFLAGS += -rdynamic -ggdb -g -DNO_CPP_DEMANGLE
CFLAGS += -DCONFIG_ARCH_GPL32900B

################################################################
# Utilities
################################################################

copydir = \
	@if [ -d $(1) ]; then \
		cd $(1); \
		$(FIND) -type d | while read DIR; \
		do \
			$(MKDIR) -p "$(2)/$$DIR"; \
		done; \
		$(FIND) -type f | while read FILE; \
		do \
			$(CP) -f "$$FILE" "$(2)/$$FILE"; \
		done; \
	fi; \
	@$(FIND) $(2) -name "\.svn" -type d | xargs --no-run-if-empty $(RM) -fR

.PHONY: all
all: sysconfig.h modules platform_common


.PHONY: clean
clean: modules_clean platform_common_clean
	@$(RM) -f sysconfig.h


# ###############################################################
# Copy sysconfig.h to local
# ###############################################################
sysconfig.h: $(PRODUCT_DIR)/config/sysconfig.h
	@$(CP) -f $< $@

# ###############################################################
# Common
# ###############################################################
#include $(BUILD_SYSTEM)/platform_common.mak
.PHONY: platform_common
platform_common:
	@$(MKDIR) -p $(KERNEL_OUT_MODULES)/lib/modules/common
	@$(MAKE) -C $(TOPDIR)platform/common/driver

.PHONY: platform_common_clean
platform_common_clean:
	@$(MAKE) -C $(TOPDIR)platform/common/driver clean
	@$(RM) -rf $(KERNEL_OUT_MODULES)/lib/modules/common

# ###############################################################
# Build modules for board_config
# ###############################################################
.PHONY: modules
modules:
	@$(MAKE) -C $(KERNEL_HEADERS) M=$(shell pwd) modules
	@$(MKDIR) -p $(KERNEL_OUT_MODULES)/lib/modules
	@$(CP) -f *.ko $(KERNEL_OUT_MODULES)/lib/modules/

.PHONY: modules_clean
modules_clean:
	@$(MAKE) -C $(KERNEL_HEADERS) M=$(shell pwd) clean
	@$(RM) -f $(KERNEL_OUT_MODULES)/lib/modules/*.ko

# ###############################################################
# Collect platform root directory
# ###############################################################
.PHONY: collect_rootfs
collect_rootfs:
	$(call copydir,rootfs,$(PRODUCT_DIR_FULL)/rootfs)

# ###############################################################
# Collect platform system directory
# ###############################################################
.PHONY: collect_system
collect_system:
	$(call copydir,system,$(PRODUCT_DIR_FULL)/system)

endif
