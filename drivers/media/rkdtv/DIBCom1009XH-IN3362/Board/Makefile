#
# Makefile for the kernel multimedia device drivers.
#

obj-$(CONFIG_DIBCOM1009XH_CONTROL)	+= DibBoard29090.o  DibBoard9080.o  DibBoardMod29098SHA.o  DibBoardNim3006xxx.o  DibBoardNim9080md.o  DibBoardNim9090md.o   DibBoardSelection.o    DibFirmwareSelection.o DibBoard29098.o  DibBoard9090.o  DibBoardNim1009xxx.o   DibBoardNim3009xxx.o  DibBoardNim9090H.o   DibBoardParameters.o  DibBoardStk29098md4.o


USER 	= drivers/media/rkdtv/DIBCom1009XH/User
DRIVER  = drivers/media/rkdtv/DIBCom1009XH/Driver
DEMOD   = drivers/media/rkdtv/DIBCom1009XH/Board
FIRMWARE= drivers/media/rkdtv/DIBCom1009XH/Firmware
BRIDGE	= drivers/media/rkdtv/DIBCom1009XH/Bridge
DRIVER_CHIP_PATH 	= $(DRIVER)/Common/Dragonfly
BRIDGE_CHIP_PATH	= $(BRIDGE)/Common/Dragonfly

INCLUDE	= 	-I$(USER)/Common/ \
		-I$(DEMOD)/ \
		-I$(DRIVER)/Common/ \
		-I$(DRIVER)/Common/Messages/ \
		-I$(DRIVER)/Driver2Bridge/ \
		-I$(BRIDGE)/Common/ \
		-I$(BRIDGE)/LinuxKernelSpi/ \
		-I$(DRIVER_CHIP_PATH)/ \
		-I$(BRIDGE_CHIP_PATH)/ \



EXTRA_CFLAGS += $(INCLUDE) 




