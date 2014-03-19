#
# Makefile for RK2928 HDMI linux kernel module.
#

ccflags-$(CONFIG_RK_HDMI_DEBUG) = -DDEBUG -DHDMI_DEBUG -DHDCP_DEBUG

obj-$(CONFIG_HDMI_RK3028A) += rk3028a_hdmi_hw.o rk3028a_hdmi.o
obj-$(CONFIG_HDCP_RK3028A) += rk3028a_hdmi_hdcp.o rk3028a_hdcp.o
obj-$(CONFIG_CEC_RK3028A) += rk3028a_hdmi_cec.o