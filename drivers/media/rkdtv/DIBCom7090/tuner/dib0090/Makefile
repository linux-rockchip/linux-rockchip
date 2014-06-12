#
# Makefile for the kernel multimedia device drivers.
#
obj-$(CONFIG_DIBCOM7090_CONTROL)	+= dib0090.o dib0090_reset.o dib0090_agc_pwm.o  dib0090_agc_soft.o dib0090_cal.o dib0090_efuse.o dib0090_io.o

EXTRA_CFLAGS += -Idrivers/media/dvb/dvb-core/  -Idrivers/media/dvb/frontends -Idrivers/media/dvb/rkdtv/DIBCom7090/include/
