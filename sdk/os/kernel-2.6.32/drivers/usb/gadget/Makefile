#
# USB peripheral controller drivers
#
ifeq ($(CONFIG_USB_GADGET_DEBUG),y)
	EXTRA_CFLAGS		+= -DDEBUG
endif

obj-$(CONFIG_USB_DUMMY_HCD)	+= dummy_hcd.o
obj-$(CONFIG_USB_NET2280)	+= net2280.o
obj-$(CONFIG_USB_AMD5536UDC)	+= amd5536udc.o
obj-$(CONFIG_USB_PXA25X)	+= pxa25x_udc.o
obj-$(CONFIG_USB_PXA27X)	+= pxa27x_udc.o
obj-$(CONFIG_USB_IMX)		+= imx_udc.o
obj-$(CONFIG_USB_GOKU)		+= goku_udc.o
obj-$(CONFIG_USB_OMAP)		+= omap_udc.o
obj-$(CONFIG_USB_LH7A40X)	+= lh7a40x_udc.o
obj-$(CONFIG_USB_S3C2410)	+= s3c2410_udc.o
obj-$(CONFIG_USB_AT91)		+= at91_udc.o
obj-$(CONFIG_USB_ATMEL_USBA)	+= atmel_usba_udc.o
obj-$(CONFIG_USB_FSL_USB2)	+= fsl_usb2_udc.o
fsl_usb2_udc-objs		:= fsl_udc_core.o
ifeq ($(CONFIG_ARCH_MXC),y)
fsl_usb2_udc-objs		+= fsl_mx3_udc.o
endif
obj-$(CONFIG_USB_M66592)	+= m66592-udc.o
obj-$(CONFIG_USB_R8A66597)	+= r8a66597-udc.o
obj-$(CONFIG_USB_FSL_QE)	+= fsl_qe_udc.o
obj-$(CONFIG_USB_CI13XXX)	+= ci13xxx_udc.o
obj-$(CONFIG_USB_S3C_HSOTG)	+= s3c-hsotg.o
obj-$(CONFIG_USB_LANGWELL)	+= langwell_udc.o
ifeq ($(CONFIG_ARCH_GPL32900B),y)
obj-$(CONFIG_USB_SPMP8000)	+= spmp_udc_b.o
else
obj-$(CONFIG_USB_SPMP8000)	+= spmp_udc.o
endif
obj-$(CONFIG_USB_SPMP8000_ETH) += spmp_udc_eth.o
#obj-$(CONFIG_USB_GP329XXB_UDC)	+= gp329xxb_udc.o
obj-$(CONFIG_USB_GP329XXB_CAM)	+= gp329xxb_udc.o
#
# USB gadget drivers
#
g_zero-objs			:= zero.o
g_audio-objs			:= audio.o
g_ether-objs			:= ether.o
g_serial-objs			:= serial.o
g_midi-objs			:= gmidi.o
gadgetfs-objs			:= inode.o
g_file_storage-objs		:= file_storage.o
ifeq ($(CONFIG_ARCH_GPL32900B),y)
g_spmp_storage-objs		:= s_file_storage_b.o gp_usb_vendor_cmd.o skip_vfs_file_storage.o gp_usb_color_cal_cmd.o
else
g_spmp_storage-objs		:= s_file_storage.o gp_usb_vendor_cmd.o skip_vfs_file_storage.o
endif
ifdef CONFIG_USB_GP329XXB_FSG
gp329xxb_storage-objs     := gp329xxb_file_storage.o gp_usb_vendor_cmd.o skip_vfs_file_storage.o
endif
ifdef CONFIG_USB_GP329XXB_CAM
gp329xxb_cam_gadget-objs     := gp329xxb_cam.o
endif
g_printer-objs			:= printer.o
g_cdc-objs			:= cdc2.o

androids-objs			:= android.o f_adb.o f_mass_storage.o
#androids-objs			:= android.o 

obj-$(CONFIG_USB_ZERO)		+= g_zero.o
obj-$(CONFIG_USB_AUDIO)		+= g_audio.o
obj-$(CONFIG_USB_ETH)		+= g_ether.o
obj-$(CONFIG_USB_GADGETFS)	+= gadgetfs.o
obj-$(CONFIG_USB_FILE_STORAGE)	+= g_file_storage.o
obj-$(CONFIG_USB_G_SERIAL)	+= g_serial.o
obj-$(CONFIG_USB_G_PRINTER)	+= g_printer.o
obj-$(CONFIG_USB_MIDI_GADGET)	+= g_midi.o
obj-$(CONFIG_USB_CDC_COMPOSITE) += g_cdc.o
obj-$(CONFIG_USB_ANDROID)	+= androids.o
#obj-$(CONFIG_USB_ANDROID)	+= f_adb.o
#obj-$(CONFIG_USB_ANDROID)	+= f_mass_storage.o
obj-$(CONFIG_USB_ANDROID_ACM)	+= f_acm.o u_serial.o
#obj-$(CONFIG_USB_ANDROID_ADB)	+= f_adb.o 
#obj-$(CONFIG_USB_ANDROID_MASS_STORAGE)	+= f_mass_storage.o
obj-$(CONFIG_USB_ANDROID_RNDIS)	+= f_rndis.o u_ether.o
obj-$(CONFIG_USB_SPMP)	+= g_spmp_storage.o
obj-$(CONFIG_USB_GP329XXB_CAM)	+= gp329xxb_cam_gadget.o
