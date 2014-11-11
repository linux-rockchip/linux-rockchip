#
# Makefile for HDMI linux kernel module.
#

ccflags-$(CONFIG_RK_HDMI_DEBUG) = -DDEBUG -DHDMI_DEBUG

obj-$(CONFIG_RK_HDMI) += hdmi-core.o  hdmi-lcdc.o hdmi-edid.o hdmi-sysfs.o hdmi-cec.o
obj-$(CONFIG_HDMI_RK3288) += rk3288/
