TOPDIR := ../../../../../
include $(TOPDIR)sdk/build/core/config.mak
OUT_SDK_LIB = 
MW_LIBS = $(TOPDIR)sdk/lib
MW_INC = $(TOPDIR)sdk/middleware

GOAL   = cvrplayer

CFLAGS += -I$(SDK_DIR)/include
CFLAGS += -I$(SDK_DIR)/middleware/libVideoDecoder/include
CFLAGS += -I$(PRODUCT_DIR)
CFLAGS += -I$(MW_INC)
CFLAGS += -I.
CFLAGS += -g
#CFLAGS += -Wall  
CFLAGS += -I./include
CFLAGS += -I$(OUT_SDK_DIR)/include/libjpeg
CFLAGS += -DSPECIAL_MEM_MALLOC -DNO_CPP_DEMANGLE
CFLAGS += -DCONFIG_ARCH_GPL32900B
#CFLAGS += -O2

LDFLAGS += -L$(MW_LIBS)
LDFLAGS += -ldl -lrt -lpthread  -rdynamic -ggdb -g
LDFLAGS += -L$(SDK_DIR)/lib -licver 
#LDFLAGS += -L$(SDK_DIR)/lib -ljpeg -lImageIF -lexif -lros
#LDFLAGS += -L$(SDK_DIR)/lib -lgpstream -ldiag
LDFLAGS += -L$(SDK_DIR)/lib -lmcpthread -lvideodecoder -lmultimediaparser -laviparser -lqtffparser 
LDFLAGS += -L$(SDK_DIR)/lib -lgpon2 -lgpon2dec -lresample -lm

#---------------------------------------------------------------------------
# Intermediate objects
#---------------------------------------------------------------------------
# Define source directory
SRCS := $(shell find . -name '*.c')
SRCS := $(sort $(SRCS))
OBJS += $(SRCS:%.c=%.o)


LIBS += $(SDK_DIR)/lib/libchunkmem.a
LIBS += $(SDK_DIR)/lib/libgpvcodec.a
LIBS += $(SDK_DIR)/lib/libjpeg.a
LIBS += $(SDK_DIR)/lib/libdiag.a
LIBS += $(SDK_DIR)/lib/libVideoMuxer.a
#LIBS += $(SDK_DIR)/lib/wav_dec_lib_ARM.a
#LIBS += $(SDK_DIR)/lib/mp3_dec_lib_ARM.a
#LIBS += $(SDK_DIR)/lib/aac_dec_lib_ARM.a

#---------------------------------------------------------------------------
# Explicit ruls
#---------------------------------------------------------------------------

all: $(GOAL)
	$(MSG_SPLIT_LINE)
	@$(MKDIR) -p $(PRODUCT_DIR)/system/app
	@$(CP) -u $(GOAL) $(PRODUCT_DIR)/system/app/$(GOAL)
	
$(GOAL): $(OBJS) $(LIBS)
	@$(TARGET_CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)
	@$(TARGET_STRIP) $@
	$(MSG_GOAL_OK)
	
clean:
	$(MSG_SPLIT_LINE)
	$(RM) -rf $(GOAL) $(OBJS) $(OBJS:.o=.d)

-include $(OBJS:.o=.d)
