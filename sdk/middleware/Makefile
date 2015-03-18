#/**************************************************************************
# *                                                                        *
# *         Copyright (c) 2008 by Sunplus mMedia Inc.                      *
# *                                                                        *
# *  This software is copyrighted by and is the property of Sunplus        *
# *  mMedia Ltd. All rights are reserved by Sunplus mMedia Inc.            *
# *  This software may only be used in accordance with the                 *
# *  corresponding license agreement. Any unauthorized use, duplication,   *
# *  distribution, or disclosure of this software is expressly forbidden.  *
# *                                                                        *
# *  This Copyright notice MUST not be removed or modified without prior   *
# *  written consent of Sunplus mMedia Inc.                                *
# *                                                                        *
# *  Sunplus mMedia Inc. reserves the right to modify this software        *
# *  without notice.                                                       *
# *                                                                        *
# *  Sunplus mMedia Inc.                                                   *
# *  19, Innovation First Road, Science-Based Industrial Park,             *
# *  Hsin-Chu, Taiwan, R.O.C.                                              *
# *                                                                        *
# *                                                                        *
# **************************************************************************/
TOPDIR := ../../
include $(TOPDIR)sdk/build/core/config.mak

ifeq ($(SYSCONFIG_ARCH), GPL32900)
export USE_XDMA_PORT01 = yes
endif

SUBDIRS += libicver
SUBDIRS += AudioDecoder
SUBDIRS += AudioEncoder
SUBDIRS += libdiag
SUBDIRS += libfilter
SUBDIRS += libboard
SUBDIRS += libceva
SUBDIRS += libmem
SUBDIRS += libieh
SUBDIRS += libchunkmem
SUBDIRS += libgimage
SUBDIRS += libarecord
SUBDIRS += libOpenVG
SUBDIRS += libres
SUBDIRS += libipm
SUBDIRS += libg3d
SUBDIRS += libcrc
SUBDIRS += libencdec
SUBDIRS += libresample
SUBDIRS += libspu
SUBDIRS += libaudiomixer
SUBDIRS += libmcpvo
SUBDIRS += libmcpao
SUBDIRS += MultiMediaParser
SUBDIRS += mediaSystemAPI
SUBDIRS += libcsi
SUBDIRS += libAEC
SUBDIRS += libcdsp

ifeq ($(SYSCONFIG_ARCH), SPMP8050)
SUBDIRS += lib2d
endif

# *** ppu lib ***
ifneq ($(SYSCONFIG_PPU), 0)
SUBDIRS += libppu
SUBDIRS += libppudisp
endif

ifeq ($(SYSCONFIG_PPU_TV),y)
SUBDIRS += libtv1
endif


SUBDIRS += libstream
SUBDIRS += libacodec
SUBDIRS += libvcodec
SUBDIRS += libpng
SUBDIRS += libbmp
SUBDIRS += libtiff
SUBDIRS += libjpeg
SUBDIRS += libgif
SUBDIRS += libexif
SUBDIRS += libImageIF
SUBDIRS += libflashjpeg

ifeq ($(SYSCONFIG_FLASH),y)
#Please put this in the end of building
SUBDIRS += libflash
endif

ifeq ($(SYSCONFIG_LIBFUSE),y)
SUBDIRS += libfuse
endif

ifeq ($(SYSCONFIG_EXFAT),y)
ifeq ($(SYSCONFIG_LIBFUSE),n)
SUBDIRS += libfuse
endif
SUBDIRS += libposix
endif

ifeq ($(SYSCONFIG_ARCH), GPL32900B)
SUBDIRS += libHantro
SUBDIRS += libgpon2
endif

ifeq ($(SYSCONFIG_ARCH), GPL32900B)
SUBDIRS += libMcpThread
SUBDIRS += libAviParser
SUBDIRS += libQtffParser
SUBDIRS += libMultiMediaParser
SUBDIRS += libVideoDecoder
SUBDIRS += libVideoMuxer
endif

ifeq ($(SYSCONFIG_LIB_VIDSTREAM), y)
SUBDIRS += gp_encode_stream
endif

all:
ifeq ($(SYSCONFIG_LIBFUSE),y)
	cd libfuse;chmod +x ./configure;./configure --host=arm-none-linux-gnueabi
endif
ifeq ($(SYSCONFIG_EXFAT),y)
ifeq ($(SYSCONFIG_LIBFUSE),n)
	cd libfuse;./configure --host=arm-none-linux-gnueabi
endif
endif
	+$(COMMAND)

clean:
	+$(COMMAND)

.PHONY: release
release:
	+$(COMMAND)
