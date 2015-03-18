include $(TOPDIR)/out/$(PRODUCT)/config/sysconfig.mak
#
# Makefile for the ps2 mouse
#

ifeq ($(SYSCONFIG_PS2MOUSE_DEVICE),gp_ps2_uart_mouse_module)
	obj-m += gp_ps2_uart_mouse_module.o
	#gp_ps2_uart_mouse_module-objs := ../hal/hal_mouse.o gp_ps2_uart_mouse.o
	gp_ps2_uart_mouse_module-objs :=  gp_ps2_uart_mouse.o
endif
