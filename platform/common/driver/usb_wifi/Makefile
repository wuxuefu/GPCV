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
include $(TOPDIR)/out/$(PRODUCT)/config/sysconfig.mak

ifeq ($(SYSCONFIG_USB_WIFI),8192cu)
	obj-m += rtl8192CU_linux_v2.0.1180.20101116/
endif

ifeq ($(SYSCONFIG_USB_WIFI),8188eu)
	obj-m += rtl8188EUS_rtl8189ES_linux_v4.1.6_7546.20130521/
endif

ifeq ($(SYSCONFIG_USB_WIFI),AR9271)
	obj-m += compat-wireless-2.6.39-1/
endif

all:
	+$(COMMAND)

clean:
	+$(COMMAND)

.PHONY: release
release:
	+$(COMMAND)
