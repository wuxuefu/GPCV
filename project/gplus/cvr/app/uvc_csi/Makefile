TOPDIR := ../../../../../
include $(TOPDIR)sdk/build/core/config.mak


GOAL = uvc_csi
OBJS += uvc_csi.o
OBJS += uvc_pipe.o
#OBJS += cdsp_config.o


CFLAGS += -I$(SDK_DIR)/include
CFLAGS += -I$(PRODUCT_DIR)
CFLAGS += -I.
CFLAGS += -g
CFLAGS += -D$(SYSCONFIG_ARCH)

LDFLAGS += -lpthread -lrt -L$(SDK_DIR)/lib -licver
ifeq ($(SYSCONFIG_ARCH), GPL32900B)
LDFLAGS += -ldl -lm -L$(SDK_DIR)/lib -lgpon2 -lcdsp
endif

ifeq ($(SYSCONFIG_ARCH), GPL32900)
LIBS += $(SDK_DIR)/lib/libceva.a
endif
LIBS += $(SDK_DIR)/lib/libchunkmem.a
#LIBS += $(SDK_DIR)/lib/libros.a
LIBS += $(SDK_DIR)/lib/libdiag.a
LIBS += $(SDK_DIR)/lib/libcsi.a

.PHONY: all
all: $(GOAL)
	$(MSG_SPLIT_LINE)
	@$(MKDIR) -p $(PRODUCT_DIR)/system/app
	@$(CP) -u $(GOAL) $(PRODUCT_DIR)/system/app/$(GOAL)

$(GOAL): $(OBJS) $(LIBS) $(EXTLIB)
	$(MSG_SPLIT_LINE)
	$(MSG_LINKING)
	@$(TARGET_CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)
	$(MSG_GOAL_OK)

.PHONY: clean
clean:
	$(MSG_SPLIT_LINE)
	$(RM) -rf $(GOAL) $(OBJS) $(OBJS:.o=.d)
	$(RM) -f $(PRODUCT_DIR)/system/app/$(GOAL)


-include $(OBJS:.o=.d)
