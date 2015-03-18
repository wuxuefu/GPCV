include $(TOPDIR)/out/$(PRODUCT)/config/sysconfig.mak
# ###############################################################
# LCD panels
# ###############################################################

ifeq ($(SYSCONFIG_DISP0_PANEL),panel_lcd_auo_A043FL01)
	obj-m += panel_lcd_auo_A043FL01.o
	panel_lcd_auo_A043FL01-objs := panel_common.o lcd_auo_A043FL01.o
endif

ifeq ($(SYSCONFIG_DISP0_PANEL),panel_lcd_hannstar_HSD070IDW1)
	obj-m += panel_lcd_hannstar_HSD070IDW1.o
	panel_lcd_hannstar_HSD070IDW1-objs := panel_common.o lcd_hannstar_HSD070IDW1.o
endif

ifeq ($(SYSCONFIG_DISP0_PANEL),panel_lcd_kd_50g2650)
	obj-m += panel_lcd_kd_50g2650.o
	panel_lcd_kd_50g2650-objs := panel_common.o lcd_kd_50g2650.o
endif

ifeq ($(SYSCONFIG_DISP0_PANEL),panel_lcd_CLAP070NJ02CW)
	obj-m += panel_lcd_CLAP070NJ02CW.o
	panel_lcd_CLAP070NJ02CW-objs := panel_common.o lcd_cpt_CLAP070NJ02CW.o
endif

ifeq ($(SYSCONFIG_DISP0_PANEL),panel_lcd_OLMS_50)
	obj-m += panel_lcd_OLMS_50.o
	panel_lcd_OLMS_50-objs := panel_common.o lcd_OLMS_50.o
endif

ifeq ($(SYSCONFIG_DISP0_PANEL),panel_lcd_ILI8961_27)
	obj-m += panel_lcd_ILI8961_27.o
	panel_lcd_ILI8961_27-objs := panel_common.o lcd_ILI8961_27.o
endif
