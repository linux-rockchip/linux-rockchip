#
# Makefile for the kernel multimedia device drivers.
#Messages/ not build
obj-$(CONFIG_DIBCOM1009XH_CONTROL)	+= Dragonfly/ DibBridge.o  DibBridgeDataCmmb.o  DibBridgeDataHbm.o     DibBridgeDataMpeIFec.o   DibBridgeDataPes.o    DibBridgeMailboxHandler.o   DibBridgeRawTs.o DibBridgeData.o  DibBridgeDataFig.o   DibBridgeDataMpeFec.o  DibBridgeDataMscAudio.o  DibBridgeDataSiPsi.o  DibBridgePayloadCheckers.o  DibBridgeTestIf.o


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
