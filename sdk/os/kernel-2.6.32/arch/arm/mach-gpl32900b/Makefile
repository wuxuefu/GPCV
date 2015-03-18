#
# Makefile for the linux kernel.
#

# Object file lists.
obj-$(CONFIG_MACH_GPL32900B) += hal/hal_ppu.o
obj-$(CONFIG_MACH_GPL32900B) += hal/hal_disp.o
obj-$(CONFIG_MACH_GPL32900B) += hal/hal_disp1.o
obj-$(CONFIG_MACH_GPL32900B) += hal/hal_disp2.o
obj-$(CONFIG_MACH_GPL32900B) += timer.o
obj-$(CONFIG_MACH_GPL32900B) += gp_version.o

# pmpmci.o sd_core.o
obj-$(CONFIG_MACH_GPL32900B) += dlmalloc.o gp_chunkmem.o
obj-$(CONFIG_MACH_GPL32900B) += gp_reg.o
obj-$(CONFIG_MACH_GPL32900B) += gp_cache.o
obj-$(CONFIG_MACH_GPL32900B) += gp_fiq.o

obj-$(CONFIG_MACH_GPL32900B) += hal/hal_dac.o
obj-$(CONFIG_MACH_GPL32900B) += hal/hal_i2s.o
obj-$(CONFIG_MACH_GPL32900B) += hal/hal_i2s_hdmi.o
obj-$(CONFIG_MACH_GPL32900B) += hal/hal_apbdma1.o

obj-$(CONFIG_MACH_GPL32900B) += hal/hal_gpio.o gp_gpio.o 
#obj-$(CONFIG_MACH_GPL32900B) += hal/hal_2d.o gp_graphic.o
obj-$(CONFIG_MACH_GPL32900B) += hal/hal_clock.o
obj-$(CONFIG_MACH_GPL32900B) += hal/hal_usb.o
obj-$(CONFIG_MACH_GPL32900B) += hal/hal_vic.o

obj-$(CONFIG_MACH_GPL32900B) += gp_diag.o

obj-$(CONFIG_PM) += sleep.o gp_sram.o pm.o
obj-$(CONFIG_MACH_GPL32900B) += hal/hal_uart.o gp_uart.o

obj-$(CONFIG_MACH_GPL32900B)	+= rtc/
obj-$(CONFIG_MACH_GPL32900B)	+= display/
obj-$(CONFIG_MACH_GPL32900B)	+= i2c-bus/
obj-$(CONFIG_MACH_GPL32900B)	+= ti2c/
obj-$(CONFIG_MACH_GPL32900B)	+= scale/
obj-$(CONFIG_MACH_GPL32900B)	+= scale2/
obj-$(CONFIG_MACH_GPL32900B)	+= watchdog/
obj-$(CONFIG_MACH_GPL32900B)	+= timer/
obj-$(CONFIG_MACH_GPL32900B)	+= pwm/
obj-$(CONFIG_MACH_GPL32900B)	+= spi/
obj-$(CONFIG_MACH_GPL32900B)	+= spi_nor/
obj-$(CONFIG_MACH_GPL32900B)	+= usb/
obj-$(CONFIG_MACH_GPL32900B)	+= apbdma0/
obj-$(CONFIG_MACH_GPL32900B)	+= sensor_mgr/
obj-$(CONFIG_MACH_GPL32900B)	+= csi/
obj-$(CONFIG_MACH_GPL32900B)	+= tv/
obj-$(CONFIG_MACH_GPL32900B)	+= ppu/
obj-$(CONFIG_MACH_GPL32900B)	+= ppu_simple/
obj-$(CONFIG_MACH_GPL32900B)	+= mipi/
obj-$(CONFIG_MACH_GPL32900B)	+= aeawb/
obj-$(CONFIG_MACH_GPL32900B)	+= cdsp/
obj-$(CONFIG_MACH_GPL32900B)	+= aes/
obj-$(CONFIG_MACH_GPL32900B)	+= gpl32900b.o devices.o
obj-$(CONFIG_MACH_GPL32900B)	+= gp_partition.o
obj-$(CONFIG_MACH_GPL32900B)	+= spu/
obj-$(CONFIG_MACH_GPL32900B)	+= sdma/

#obj-$(CONFIG_MACH_GPL32900B)	+= hal/hal_sar.o gp_adc.o
obj-$(CONFIG_MACH_GPL32900B)	+= adc/

obj-$(CONFIG_MACH_GPL32900B) += board/board.o
#obj-$(CONFIG_MACH_GPL32900B) += os/
#obj-$(CONFIG_MACH_GPL32900B) += clock_mgr/
obj-$(CONFIG_MACH_GPL32900B) += clk/
#obj-$(CONFIG_MACH_GPL32900B)	+= sdio/
obj-$(CONFIG_MACH_GPL32900B) += sd/
obj-$(CONFIG_MACH_GPL32900B) += audiodrv/
#obj-$(CONFIG_MACH_GPL32900B) += audio/
obj-$(CONFIG_MACH_GPL32900B)	+= gp_nand/
obj-$(CONFIG_MACH_GPL32900B)	+= dc2dc/
obj-$(CONFIG_MACH_GPL32900B)	+= spi1/

obj-$(CONFIG_MACH_GPL32900B) += cpu_pwr_down.o
obj-$(CONFIG_MACH_GPL32900B) += gp_suspend.o
obj-$(CONFIG_MACH_GPL32900B) += pwr_down_asm.o

#ifeq ($(CONFIG_PM_GPFB),y)
	#obj-m += snapshot.o
#endif
#obj-$(CONFIG_PM_GPFB) += snapshot.o

ifeq ($(CONFIG_PM_SPEEDY),y)
obj-$(CONFIG_MACH_GPL32900B)	+= fastboot/
endif

obj-$(CONFIG_MACH_GPL32900B) += idle.o
obj-$(CONFIG_MACH_GPL32900B)	+= hantro/

ifeq ($(CONFIG_NET_ETHERNET), y)
obj-$(CONFIG_MACH_GPL32900B) += ethernet/
endif

obj-$(CONFIG_MACH_GPL32900B) += lbp/
