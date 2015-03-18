TOPDIR := ../../../../../
include $(TOPDIR)sdk/build/core/config.mak


GOAL     = car_recorder


CFLAGS += -I$(SDK_DIR)/include
CFLAGS += -I$(SDK_DIR)/middleware/libVideoDecoder/include
CFLAGS += -I$(PRODUCT_DIR)
CFLAGS += -I.
CFLAGS += -I./include
CFLAGS += -rdynamic -ggdb -g -DNO_CPP_DEMANGLE
CFLAGS += -DCONFIG_ARCH_GPL32900B

LDFLAGS += -ldl -lrt -lm -lpthread -rdynamic -ggdb -g
LDFLAGS += -L$(SDK_DIR)/lib  \
           -lchunkmem -ldiag \
		   -licver \
		   -lgpon2 -lgpon2dec \
		   -lresample

OBJS := \
	bmp.o \
	disp.o \
	font.o \
	cvr_pipe.o \
	public_pipe.o \
	dv_service.o \
	dc_service.o \
	playback_service.o \
	ap_state_config.o \
	playback_pipe.o \
	uvc_pipe.o \
	CloudDog_drv.o \
	CloudDog_app.o \
	move_string.o \
	main.o 

LIBS := \
	$(SDK_DIR)/lib/libdiag.a \
	$(SDK_DIR)/lib/libboard.a
	
.PHONY: all
all: $(GOAL)
	$(MSG_SPLIT_LINE)
	@$(MKDIR) -p $(PRODUCT_DIR)/system/app
	@$(CP) -u $(GOAL) $(PRODUCT_DIR)/system/app/$(GOAL)

$(GOAL): $(OBJS) $(LIBS)
	$(MSG_SPLIT_LINE)
	$(MSG_LINKING)
	@$(TARGET_CC) -o $@ $(OBJS) $(LIBS) $(LDFLAGS)
	@$(TARGET_STRIP) $@
	$(MSG_GOAL_OK)


.PHONY: clean
clean:
	$(MSG_SPLIT_LINE)
	$(RM) -rf $(GOAL) $(OBJS) $(OBJS:.o=.d)
	$(RM) -f $(PRODUCT_DIR)/system/app/$(GOAL)


-include $(OBJS:.o=.d)
