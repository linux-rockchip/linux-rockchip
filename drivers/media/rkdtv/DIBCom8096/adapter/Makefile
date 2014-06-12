#
# Makefile for the kernel multimedia device drivers.
#
obj-$(CONFIG_DIBCOM8096_CONTROL)	+= antenna.o channel.o databus.o demod.o frontend_tune.o frontend.o mac.o tuner.o #component_monitor.o

EXTRA_CFLAGS += -Idrivers/media/dvb/dvb-core/  -Idrivers/media/dvb/frontends -Idrivers/media/dvb/rkdtv/DIBCom8096/include/
