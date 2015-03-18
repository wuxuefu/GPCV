#
# Makefile for the linux kernel.
#

# Object file lists.
obj-y	+= clk_src.o mgr.o gp_clk_core.o

# Clock  manager source code
mgr-objs := mgr/clk.o mgr/clk-divider.o mgr/clk-fixed-factor.o mgr/clk-fixed-rate.o mgr/clk-gate.o mgr/clk-mux.o mgr/gp_clk_gate.o
# Clock source code 
clk_src-objs := clk_src/gp_pll.o clk_src/gp_ref_clk.o clk_src/gp_bus_clk.o clk_src/gp_scua.o clk_src/gp_scub.o clk_src/gp_scuc.o