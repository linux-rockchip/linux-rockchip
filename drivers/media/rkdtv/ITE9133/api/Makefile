#
# Makefile for the kernel multimedia device drivers.
#
obj-$(CONFIG_ITE9133_CONTROL)	+= cmd.o IT9133.o Omega.o standard.o tuner.o user.o

EXTRA_CFLAGS += -Idrivers/media/dvb/dvb-core/  -Idrivers/media/dvb/frontends -Idrivers/media/dvb/rkdtv/ -Idrivers/media/dvb/rkdtv/ITE9133/ -Idrivers/media/dvb/rkdtv/ITE9133/api/ -Idrivers/media/dvb/rkdtv/ITE9133/firmware/
