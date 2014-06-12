#
# Makefile for the kernel multimedia device drivers.
#
obj-$(CONFIG_DIBCOM7090_CONTROL)	+= adapter/ demod/ interface/ monitor/ platform/ sip/ tuner/ DiBcom_i2c_app.o dibcom7090_app.o

EXTRA_CFLAGS += -Idrivers/media/dvb/dvb-core/  -Idrivers/media/dvb/frontends -Idrivers/media/dvb/rkdtv/ -Idrivers/media/dvb/rkdtv/DIBCom7090/

HAVE_DIB7090 = 1

HAVE_DIB8000 = 1

HAVE_DIB0090 = 1

HAVE_COMMON = 1

