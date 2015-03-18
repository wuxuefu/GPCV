#
# Makefile for the i2c bus driver
#

obj-m := gp_i2c_bus_module.o
#gp_i2c_bus_module-objs := gp_i2c_bus.o ../hal/hal_i2c_bus.o
#gp_i2c_bus_module-objs := gp_i2c_bus.o gpio_i2c_bus.o
gp_i2c_bus_module-objs := gp_i2c_bus.o  gpio_i2c_bus.o ../hal/hal_i2c_bus.o

obj-y := gp_i2c_internal_module.o
gp_i2c_internal_module-objs := gp_i2c_internal.o ../hal/hal_i2c_internal.o
