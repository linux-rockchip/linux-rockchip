ifndef V
	SILENT=@
endif
ifndef SILENT
	SILENT=
endif

# target specific part
ifndef TARGET
	ERR=$(error No target defined; please specify one with $0 TARGET=[i386|st8815|mx31|mx28|pxa300|pxa310|user|kernel|i2c])
endif

# Include all files
CHIP_SELECT=Firefly Dragonfly

# This define can be used for shared files between sdk and firmware, to distinguish between the two compilation
DEFINES = -DDIBVERBOSE
DEFINES += -DDRIVER_AND_BRIDGE_MERGED=0

# HOOK TARGET
ifeq ($(TARGET), i386)
	DIB_ARCH=i386
	BRIDGE_DIR=LinuxKernelDib07x0
	DRIVER_DIR=LinuxU2KBridge
	MK=Makefiles/Makefile.i386
endif

ifeq ($(TARGET), kernel)
	DIB_ARCH=i386
	BRIDGE_DIR=LinuxKernelSdioSram
	DRIVER_DIR=LinuxU2KBridge
	MK=Makefiles/Makefile.kernel
endif

ifeq ($(TARGET), sh4)
	BRIDGE_DIR=LinuxKernelSpiSt7106
	DRIVER_DIR=LinuxU2KBridge
	DIB_CC=sh4-linux-gcc
	DIB_AR=sh4-linux-ar
	DIB_RANLIB=sh4-linux-ranlib
	DIB_ARCH=sh
	CROSS_COMPILE=sh4-linux-
#	CFLAGS = -ARCH=sh
	MK=Makefiles/Makefile.sh4
endif

# MX31 SDIO
ifeq ($(TARGET), mx31)
	DIB_ARCH=arm
	BRIDGE_DIR=LinuxKernelSdioMx31
	DRIVER_DIR=LinuxU2KBridge
	MK=Makefiles/Makefile.mx31
endif

# MX28 SDIO
ifeq ($(TARGET), mx28)
	DIB_ARCH=arm
	BRIDGE_DIR=LinuxKernelSdioMx28
	DRIVER_DIR=LinuxU2KBridge
	CROSS_COMPILE=/home/users/firefly/software/tools/Toolchains_customers/Linux/Freescale/gcc-4.4.4-glibc-2.11.1-multilib-1.0/arm-fsl-linux-gnueabi/bin/arm-linux-
	MK=Makefiles/Makefile.mx28
endif


# Monahans
ifeq ($(TARGET), pxa300)
	DIB_ARCH=arm
	BRIDGE_DIR=LinuxKernelSramMonahans
	DRIVER_DIR=LinuxU2KBridge
	MK=Makefiles/Makefile.pxa300
endif

ifeq ($(TARGET), st8815)
	DIB_ARCH=arm
	BRIDGE_DIR=LinuxKernelSramSt8815
	DRIVER_DIR=LinuxU2KBridge
	MK=Makefiles/Makefile.st8815
endif

ifeq ($(TARGET), pxa310)
	DIB_ARCH=arm
	BRIDGE_DIR=LinuxKernelSdioMonahans
	DRIVER_DIR=LinuxU2KBridge
	MK=Makefiles/Makefile.pxa310
endif

# KERNEL SRAM/SDIO EXAMPLE TARGET
ifeq ($(TARGET), kernel)
	DIB_ARCH=i386
	BRIDGE_DIR=LinuxKernelSdioSram
	DRIVER_DIR=LinuxU2KBridge
	MK=Makefiles/Makefile.i386
endif

ifndef MK
	ERR=$(error unknown target defined; please specify one with $0 TARGET=[i386|st8815|mx31|pxa300|pxa310|user|kernel|i2c])
clean: ERROR
all: ERROR
default: ERROR
endif

include $(MK)

ERROR:
	$(SILENT)$(ERR)
