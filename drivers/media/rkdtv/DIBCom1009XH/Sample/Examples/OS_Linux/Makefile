# Enable debug monitor. Internal use
I386_DEBUG=yes
SHELL=/bin/sh

ifndef V
	SILENT=@
endif
ifndef SILENT
	SILENT=
endif

ifeq ($(MAKELEVEL), 0)
	ECHO=echo
else
	ECHO := echo -n "    ";$(ECHO)
endif
export ECHO

# enable firefly compilation by default

# target specific part
ifndef TARGET
	ERR=$(error No target defined; please specify one with $0 TARGET=[i386|i386-merge|st8815|mx31|pxa300|pxa310|user|kernel|i2c])
endif

# HOOK TARGET
ifeq ($(TARGET), i386)
	DEBUG=$(I386_DEBUG)
	BRIDGE_DIR=LinuxKernelDib07x0
	DRIVER_DIR=LinuxU2KBridge
	DIB_CC=gcc
	DIB_AR=ar
	DIB_RANLIB=ranlib
	CFLAGS = -DDRIVER_AND_BRIDGE_MERGED=0 -ggdb3 -ansi
endif

# HOOK TARGET MERGE MODE
ifeq ($(TARGET), i386-merge)
	DEBUG=$(I386_DEBUG)
	BRIDGE_DIR=LinuxUserDib07x0
	DRIVER_DIR=Driver2Bridge
	DIB_CC=gcc
	DIB_AR=ar
	DIB_RANLIB=ranlib
	CFLAGS = -DDRIVER_AND_BRIDGE_MERGED=1 -ansi -Wcast-align
	BRIDGE=../../Bridge
endif

# MX31 SDIO
ifeq ($(TARGET), mx31)
	BRIDGE_DIR=LinuxKernelSdioMx31
	DRIVER_DIR=LinuxU2KBridge
	DIB_CC=/home/users/firefly/software/tools/Toolchains_customers/Linux/ARM/gcc-4.1.1-glibc-2.4-nptl-2/arm-none-linux-gnueabi/bin/arm-none-linux-gnueabi-gcc
	DIB_AR=/home/users/firefly/software/tools/Toolchains_customers/Linux/ARM/gcc-4.1.1-glibc-2.4-nptl-2/arm-none-linux-gnueabi/bin/arm-none-linux-gnueabi-ar
	DIB_RANLIB=/home/users/firefly/software/tools/Toolchains_customers/Linux/ARM/gcc-4.1.1-glibc-2.4-nptl-2/arm-none-linux-gnueabi/bin/arm-none-linux-gnueabi-ranlib
	DIB_ARCH=arm
	CFLAGS = -DDRIVER_AND_BRIDGE_MERGED=0 -ARCH=arm
endif

# Monahans sram
ifeq ($(TARGET), pxa300)
	BRIDGE_DIR=LinuxKernelSramMonahans
	DRIVER_DIR=LinuxU2KBridge
	DIB_CC=/home/users/firefly/software/tools/Toolchains_customers/Linux/Intel/arm-xscale-linux-cs344-2.3/bin/arm-none-linux-gnueabi-gcc
	DIB_AR=/home/users/firefly/software/tools/Toolchains_customers/Linux/Intel/arm-xscale-linux-cs344-2.3/bin/arm-none-linux-gnueabi-ar
	DIB_RANLIB=/home/users/firefly/software/tools/Toolchains_customers/Linux/Intel/arm-xscale-linux-cs344-2.3/bin/arm-none-linux-gnueabi-ranlib
	DIB_ARCH=arm
	CFLAGS = -DDRIVER_AND_BRIDGE_MERGED=0 -ARCH=arm
endif

ifeq ($(TARGET), pxa310)
	BRIDGE_DIR=LinuxKernelSdioMonahans
	DRIVER_DIR=LinuxU2KBridge
	DIB_CC=/home/users/firefly/software/tools/Toolchains_customers/Linux/Marvell/arm-linux-4.1.1/bin/arm-linux-gcc
	DIB_AR=/home/users/firefly/software/tools/Toolchains_customers/Linux/Marvell/arm-linux-4.1.1/bin/arm-linux-ar
	DIB_RANLIB=/home/users/firefly/software/tools/Toolchains_customers/Linux/Marvell/arm-linux-4.1.1/bin/arm-linux-ranlib
	DIB_ARCH=arm
	CFLAGS = -DDRIVER_AND_BRIDGE_MERGED=0 -ARCH=arm
endif

ifeq ($(TARGET), st8815)
	BRIDGE_DIR=LinuxKernelSramSt8815
	DRIVER_DIR=LinuxU2KBridge
	DIB_CC=/home/users/firefly/software/tools/BSPs/ST/Nomadik/Linux/ARM_NHK15_4.3.0/linux_nhk15/toolchain/bin/arm-poky-linux-gnueabi-gcc
	DIB_AR=/home/users/firefly/software/tools/BSPs/ST/Nomadik/Linux/ARM_NHK15_4.3.0/linux_nhk15/toolchain/bin/arm-poky-linux-gnueabi-ar
	DIB_RANLIB=/home/users/firefly/software/tools/BSPs/ST/Nomadik/Linux/ARM_NHK15_4.3.0/linux_nhk15/toolchain/bin/arm-poky-linux-gnueabi-ranlib
#DIB_CC=/usr/local/8815/karakum/release430/utils/toolchain/bin/arm-poky-linux-gnueabi-gcc
#   DIB_AR=/usr/local/8815/karakum/release430/utils/toolchain/bin/arm-poky-linux-gnueabi-ar
#	DIB_RANLIB=/usr/local/8815/karakum/release430/utils/toolchain/bin/arm-poky-linux-gnueabi-ranlib
	DIB_ARCH=arm
	CFLAGS = -DDRIVER_AND_BRIDGE_MERGED=0 -ARCH=arm
endif

# KERNEL SRAM/SDIO  EXAMPLE
ifeq ($(TARGET), kernel)
	BRIDGE_DIR=LinuxKernelSdioSram
	DRIVER_DIR=LinuxU2KBridge
	DIB_CC=gcc
	DIB_AR=ar
	DIB_RANLIB=ranlib
	CFLAGS = -DDRIVER_AND_BRIDGE_MERGED=0
endif

# USER SRAM/SDIO  EXAMPLE
ifeq ($(TARGET), user)
	BRIDGE_DIR=LinuxUserSdioSram
	DRIVER_DIR=Driver2Bridge
	DIB_CC=gcc
	DIB_AR=ar
	DIB_RANLIB=ranlib
	CFLAGS = -DDRIVER_AND_BRIDGE_MERGED=1
	BRIDGE=../../Bridge
endif

# USER SRAM/SDIO  EXAMPLE
ifeq ($(TARGET), i2c)
	BRIDGE_DIR=LinuxUserI2C
	DRIVER_DIR=Driver2Bridge
	DIB_CC=gcc
	DIB_AR=ar
	DIB_RANLIB=ranlib
	CFLAGS = -DDRIVER_AND_BRIDGE_MERGED=1
	BRIDGE=../../Bridge
endif


ifeq ($(DEBUG_LIB),yes)
OBJECT_USER+=../Debug/Debug.a
CFLAGS     += -DDEBUG_LIB
else
OBJECT_USER=
endif

ifeq ($(DEBUG),yes)
CFLAGS += -DDEBUG
endif


ifndef DIB_CC
	ERR=$(error unknown target $(TARGET) defined; please specify one with $0 TARGET=[i386|i386-merge|st8815|mx31|pxa300|pxa310|user|kernel|i2c])
clean: ERROR
all: ERROR
default: ERROR
endif

export DIB_CC DIB_AR DIB_RANLIB SILENT DIB_ARCH CFLAGS BRIDGE

USER =../../../User/

SOURCES = $(wildcard ../*.c ) $(wildcard *.c )
OBJECTS = $(SOURCES:%.c=%.o)
HEADERS = $(wildcard ../*.h) $(wildcard *.h)

DRIVER_LIBRARY_DIR=../../../Sample/LinuxDriverLibrary
DRIVER_LIBRARY=$(DRIVER_LIBRARY_DIR)/libDibDriverLibrary.a

CFLAGS += -g -Wall
LIBRARY = -lm -lpthread -L$(DRIVER_LIBRARY_DIR) -lDibDriverLibrary
INCLUDE = -I$(USER)/Common -I./

default : all

bootstrap:
	$(SILENT)$(EXTRA_LINK)
	$(SILENT)$(SHELL) ./bootstrap.sh $(BRIDGE_DIR) $(DRIVER_DIR) ".."

clean_bootstrap:
	$(SILENT)$(SHELL) ./bootstrap.sh $(BRIDGE_DIR) $(DRIVER_DIR) ".." delete

%.o : %.c $(HEADERS)
	$(SILENT)$(ECHO) "Compiling $@...."
	$(SILENT)$(DIB_CC) $(CFLAGS) $(INCLUDE) -c -o $@ $<

SampleDvbt : bootstrap $(DRIVER_LIBRARY)
	$(SILENT)$(ECHO) "Creating $@...."
	$(SILENT)$(DIB_CC) $(INCLUDE) ../SampleDvbt.c $(LIBRARY) -o SampleDvbt

SampleIsdbt : bootstrap $(DRIVER_LIBRARY)
	$(SILENT)$(ECHO) "Creating $@...."
	$(SILENT)$(DIB_CC) $(INCLUDE) ../SampleIsdbt.c $(LIBRARY) -o SampleIsdbt

SampleIsdbtsb : bootstrap $(DRIVER_LIBRARY)
	$(SILENT)$(ECHO) "Creating $@...."
	$(SILENT)$(DIB_CC) $(INCLUDE) ../SampleIsdbtsb.c $(LIBRARY) -o SampleIsdbtsb

SampleIsdbtBcas : bootstrap $(DRIVER_LIBRARY)
	$(SILENT)$(ECHO) "Creating $@...."
	$(SILENT)$(DIB_CC) $(INCLUDE) ../SampleIsdbtBcas.c $(LIBRARY) -o SampleIsdbtBcas

SampleCmmb : bootstrap $(DRIVER_LIBRARY)
	$(SILENT)$(ECHO) "Creating $@...."
	$(SILENT)$(DIB_CC) $(INCLUDE) ../SampleCmmb.c $(LIBRARY) -o SampleCmmb

SampleCmmbNagra : bootstrap $(DRIVER_LIBRARY)
	$(SILENT)$(ECHO) "Creating $@...."
	$(SILENT)$(DIB_CC) $(INCLUDE) ../SampleCmmbNagra.c $(LIBRARY) -o SampleCmmbNagra

SampleCmmbMbbms : bootstrap $(DRIVER_LIBRARY)
	$(SILENT)$(ECHO) "Creating $@...."
	$(SILENT)$(DIB_CC) $(INCLUDE) ../SampleCmmbMbbms.c $(LIBRARY) -o SampleCmmbMbbms

SampleDab : bootstrap $(DRIVER_LIBRARY)
	$(SILENT)$(ECHO) "Creating $@...."
	$(SILENT)$(DIB_CC) $(INCLUDE) ../SampleDab.c $(LIBRARY) -o SampleDab

SampleDvbtExtendedApi : bootstrap $(DRIVER_LIBRARY)
	$(SILENT)$(ECHO) "Creating $@...."
	$(SILENT)$(DIB_CC) $(INCLUDE) ../SampleDvbtExtendedApi.c $(LIBRARY) -o SampleDvbtExtendedApi

SampleDabExtendedApi : bootstrap $(DRIVER_LIBRARY)
	$(SILENT)$(ECHO) "Creating $@...."
	$(SILENT)$(DIB_CC) $(INCLUDE) ../SampleDabExtendedApi.c $(LIBRARY) -o SampleDabExtendedApi

SampleIsdbtExtendedApiDiv : bootstrap $(DRIVER_LIBRARY)
	$(SILENT)$(ECHO) "Creating $@...."
	$(SILENT)$(DIB_CC) $(INCLUDE) ../SampleIsdbtExtendedApiDiv.c $(LIBRARY) -o SampleIsdbtExtendedApiDiv

SampleIsdbtExtendedApiDiv2+2 : bootstrap $(DRIVER_LIBRARY)
	$(SILENT)$(ECHO) "Creating $@...."
	$(SILENT)$(DIB_CC) $(INCLUDE) ../SampleIsdbtExtendedApiDiv2+2.c $(LIBRARY) -o SampleIsdbtExtendedApiDiv2+2

all : SampleDvbt SampleIsdbt SampleIsdbtBcas SampleIsdbtsb SampleCmmb SampleCmmbNagra SampleCmmbMbbms SampleDab SampleDvbtExtendedApi SampleDabExtendedApi SampleIsdbtExtendedApiDiv SampleIsdbtExtendedApiDiv2+2

ERROR:
	$(SILENT)$(ERR)

.PHONY: $(DRIVER_LIBRARY)
$(DRIVER_LIBRARY) :
	$(SILENT)cd $(DRIVER_LIBRARY_DIR) && make --no-print-directory
.PHONY: clean_driver_lib
clean_driver_lib :
	$(SILENT)cd $(DRIVER_LIBRARY_DIR) && make --no-print-directory clean
clean : clean_driver_lib clean_bootstrap
	$(SILENT)-rm -f $(DEMOD)/*.o $(USER)/Common/*.o $(USER)/Target/*.o dta *.o ../*.o *~ ../*~ > /dev/null 2>&1
