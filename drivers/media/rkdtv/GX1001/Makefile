#
# Makefile for the kernel multimedia device drivers.
#
obj-$(CONFIG_GX1001_CONTROL)	+= tda18250/ GX1001_i2c_app.o GX1001_V8.2.5.o AlPSTDQE3_V8.2.5.o rf_alps_tdae_v8.2.3.o GX1001_app.o

EXTRA_CFLAGS += -Idrivers/media/dvb/dvb-core/  -Idrivers/media/dvb/frontends -Idrivers/media/dvb/rkdtv/ -Idrivers/media/dvb/rkdtv/GX1001/ -Idrivers/media/dvb/rkdtv/GX1001/tda18250/

