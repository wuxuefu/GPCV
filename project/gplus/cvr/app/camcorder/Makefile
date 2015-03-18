TOPDIR := ../../../../../
include $(TOPDIR)sdk/build/core/config.mak

CFLAGS += -I.
CFLAGS += -g  -O2
CFLAGS += -I$(SDK_DIR)/include
#CFLAGS += -I./include
CFLAGS += -I$(PRODUCT_DIR)
CFLAGS += -I $(SDK_DIR)/middleware/gp_encode_stream/include/public
#CFLAGS += -DDISABLE_AUDIO_STREAM

LDFLAGS += -lrt -ldl -lm
#LDFLAGS += -L$(SDK_DIR)/lib -lgpon2 -lgp_video_stream -lpthread
#LDFLAGS += -L$(SDK_DIR)/lib -lgpon2 -lpthread
LDFLAGS += -L$(SDK_DIR)/lib -lpthread -lgpon2enc -lcdsp
#LDFLAGS += -lgpon2dec

LIBS += $(SDK_DIR)/lib/gp_video_stream.a
LIBS += $(SDK_DIR)/lib/libchunkmem.a
LIBS += $(SDK_DIR)/lib/libaacenc.a
LIBS +=$(SDK_DIR)/lib/libG726G711.a
LIBS +=$(SDK_DIR)/lib/libVideoMuxer.a
#LIBS +=$(OUT_SDK_DIR)/lib/libpng.a
#LIBS +=$(OUT_SDK_DIR)/lib/libz.a
LIBS += $(SDK_DIR)/lib/libboard.a
LIBS += $(SDK_DIR)/lib/libdiag.a

GOAL = camcorder

OBJS += main.o
OBJS += dv_main.o
OBJS += dc_main.o
#OBJS += Bit_operation.o
OBJS += loop_packer.o
#OBJS += vid_mp4_packer.o
#OBJS += avi_mux.o
OBJS += storage.o
OBJS += dvr_pipe.o
OBJS += folder.o

.PHONY: all
all: $(GOAL)
	$(MSG_SPLIT_LINE)
	@$(MKDIR) -p $(PRODUCT_DIR)/system/app
	@$(CP) -u $(GOAL) $(PRODUCT_DIR)/system/app/$(GOAL)

$(GOAL): $(OBJS) $(LIBS) $(EXTLIB)
	$(MSG_SPLIT_LINE)
	$(MSG_LINKING)
	@$(TARGET_CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)
#	@$(TARGET_CXX) $(CFLAGS) $(OBJS) $(OBJS_ICGI) -o $(GOAL) $(LDFLAGS) $(LIBS)
	@$(TARGET_STRIP) $(GOAL)
	$(MSG_GOAL_OK)
	
.PHONY: clean
clean:
	$(MSG_SPLIT_LINE)
	@$(RM) -rf $(GOAL) $(OBJS) $(OBJS:.o=.d)
	@$(RM) -f $(PRODUCT_DIR)/system/app/$(GOAL)


-include $(OBJS:.o=.d)
