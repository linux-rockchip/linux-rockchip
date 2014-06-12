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

ifndef DIB_CC
  	BOOTSTRAP = bootstrap
endif

USER    = ../../User
DRIVER  = ../../Driver
DEMOD   = ../../Board
FIRMWARE= ../../Firmware

# Only if first Makefile
ifndef DIB_CC
# Target specific part
ifndef TARGET
	ERR=$(error No Target defined; please specify one with $0 TARGET=[i386|st8815|mx31|mx28|pxa300|pxa310|kernel])
endif

# HOOK TARGET
ifeq ($(TARGET), imx28)
	BRIDGE_DIR=LinuxKernelSdioMx28
	DRIVER_DIR=LinuxU2KBridge
	DIB_CC=/home/users/firefly/software/tools/Toolchains_customers/Linux/Freescale/gcc-4.4.4-glibc-2.11.1-multilib-1.0/arm-fsl-linux-gnueabi/bin/arm-linux-gcc
	DIB_AR=/home/users/firefly/software/tools/Toolchains_customers/Linux/Freescale/gcc-4.4.4-glibc-2.11.1-multilib-1.0/arm-fsl-linux-gnueabi/bin/arm-linux-ar
	DIB_RANLIB=/home/users/firefly/software/tools/Toolchains_customers/Linux/Freescale/gcc-4.4.4-glibc-2.11.1-multilib-1.0/arm-fsl-linux-gnueabi/bin/arm-linux-ranlib
	DIB_ARCH=arm

endif



ifeq ($(TARGET), i386)
	BRIDGE_DIR=LinuxKernelDib07x0
	DRIVER_DIR=LinuxU2KBridge
	DIB_CC=gcc
	DIB_AR=ar
	DIB_RANLIB=ranlib
	CFLAGS = -ggdb3
endif

# HOOK TARGET MERGE MODE
ifeq ($(TARGET), i386-merge)
	BRIDGE_DIR=LinuxUserDib07x0
	DRIVER_DIR=Driver2Bridge
	DIB_CC=gcc
	DIB_AR=ar
	DIB_RANLIB=ranlib
	BRIDGE=../../Bridge
	CFLAGS = -DDRIVER_AND_BRIDGE_MERGED -ggdb3
	CFLAGS += -I$(BRIDGE)/Target/include
endif

ifeq ($(TARGET), i386-shared)
	BRIDGE_DIR=LinuxUserDib07x0
	DRIVER_DIR=Driver2Bridge
	DIB_CC=gcc
	DIB_AR=ar
	DIB_RANLIB=ranlib
	BRIDGE=../../Bridge
	CFLAGS = -DDRIVER_AND_BRIDGE_MERGED -ggdb3 -fPIC
	CFLAGS += -I$(BRIDGE)/Target/include
endif

# MX31 SDIO
ifeq ($(TARGET), mx31)
	BRIDGE_DIR=LinuxKernelSdioMx31
	DRIVER_DIR=LinuxU2KBridge
	DIB_CC=/home/users/firefly/software/tools/Toolchains_customers/Linux/ARM/gcc-4.1.1-glibc-2.4-nptl-2/arm-none-linux-gnueabi/bin/arm-none-linux-gnueabi-gcc
	DIB_AR=/home/users/firefly/software/tools/Toolchains_customers/Linux/ARM/gcc-4.1.1-glibc-2.4-nptl-2/arm-none-linux-gnueabi/bin/arm-none-linux-gnueabi-ar
	DIB_RANLIB=/home/users/firefly/software/tools/Toolchains_customers/Linux/ARM/gcc-4.1.1-glibc-2.4-nptl-2/arm-none-linux-gnueabi/bin/arm-none-linux-gnueabi-ranlib
	DIB_ARCH=arm
	CFLAGS = -ARCH=arm
endif

# Monahans sram
ifeq ($(TARGET), pxa300)
	BRIDGE_DIR=LinuxKernelSramMonahans
	DRIVER_DIR=LinuxU2KBridge
	DIB_CC=/home/users/firefly/software/tools/Toolchains_customers/Linux/Intel/arm-xscale-linux-cs344-2.3/bin/arm-none-linux-gnueabi-gcc
	DIB_AR=/home/users/firefly/software/tools/Toolchains_customers/Linux/Intel/arm-xscale-linux-cs344-2.3/bin/arm-none-linux-gnueabi-ar
	DIB_RANLIB=/home/users/firefly/software/tools/Toolchains_customers/Linux/Intel/arm-xscale-linux-cs344-2.3/bin/arm-none-linux-gnueabi-ranlib
	DIB_ARCH=arm
	CFLAGS = -ARCH=arm
endif

ifeq ($(TARGET), pxa310)
	BRIDGE_DIR=LinuxKernelSdioMonahans
	DRIVER_DIR=LinuxU2KBridge
	DIB_CC=/home/users/firefly/software/tools/Toolchains_customers/Linux/Marvell/arm-linux-4.1.1/bin/arm-linux-gcc
	DIB_AR=/home/users/firefly/software/tools/Toolchains_customers/Linux/Marvell/arm-linux-4.1.1/bin/arm-linux-ar
	DIB_RANLIB=/home/users/firefly/software/tools/Toolchains_customers/Linux/Marvell/arm-linux-4.1.1/bin/arm-linux-ranlib
	DIB_ARCH=arm
	CFLAGS = -ARCH=arm
endif

# ST NOMADIK SRAM
ifeq ($(TARGET), st8815)
	BRIDGE_DIR=LinuxKernelSramSt8815
	DRIVER_DIR=LinuxU2KBridge
	DIB_CC=/usr/local/8815/karakum/release430/utils/toolchain/bin/arm-poky-linux-gnueabi-gcc
	DIB_AR=/usr/local/8815/karakum/release430/utils/toolchain/bin/arm-poky-linux-gnueabi-ar
	DIB_RANLIB=/usr/local/8815/karakum/release430/utils/toolchain/bin/arm-poky-linux-gnueabi-ranlib
	DIB_ARCH=arm
	CFLAGS = -ARCH=arm
endif


# KERNEL SRAM/SDIO  EXAMPLE
ifeq ($(TARGET), kernel)
	BRIDGE_DIR=LinuxKernelSdioSram
	DRIVER_DIR=LinuxU2KBridge
	DIB_CC=gcc
	DIB_AR=ar
	DIB_RANLIB=ranlib
endif

# USER SRAM/SDIO  EXAMPLE
ifeq ($(TARGET), user)
	BRIDGE_DIR=LinuxUserSdioSram
	DRIVER_DIR=Driver2Bridge
	DIB_CC=gcc
	DIB_AR=ar
	DIB_RANLIB=ranlib
	CFLAGS = -DDRIVER_AND_BRIDGE_MERGED
	BRIDGE=../../Bridge
endif

ifeq ($(TARGET), kernel)
	BRIDGE_DIR=LinuxKernelSdioSram
	DRIVER_DIR=LinuxU2KBridge
	DIB_CC=gcc
	DIB_AR=ar
	DIB_RANLIB=ranlib
	CFLAGS = -ggdb3
endif

#endif of DIB_CC
endif

# UNCOMMENT THIS LINE TO HAVE C AND ASM MIXED
#CFLAGS += -Wa,-a,-aln=$(<:%.c=%.S)

ifndef DIB_CC
	ERR=$(error unknown Target $(TARGET) defined; please specify one with $0 TARGET=[i386|i386-merge|i386-shared|st8815|mx31|pxa300|pxa310|user|kernel|i2c])
clean: ERROR
all: ERROR
default: ERROR
endif

DRIVER_CHIP_PATH = $(addprefix $(DRIVER)/Common/,Dragonfly Firefly)
BRIDGE_CHIP_PATH = $(addprefix $(BRIDGE)/Common/,Dragonfly Firefly)


SOURCES = $(wildcard $(DRIVER)/Common/*.c \
                     $(DRIVER)/Common/Messages/*.c \
                     $(DRIVER)/Target/*.c \
                     $(DEMOD)/*.c \
                     $(USER)/Common/DibExtAPI.c \
                     $(BRIDGE)/Common/*.c \
                     $(addsuffix /*.c, $(DRIVER_CHIP_PATH)) \
                     $(addsuffix /*.c, $(BRIDGE_CHIP_PATH)) \
                     $(BRIDGE)/Target/*.c)

OBJECTS = $(SOURCES:%.c=%.o)

HEADERS = $(wildcard *.h \
                     $(DRIVER)/Common/*.h \
                     $(DRIVER)/Common/Messages/*.h \
                     $(DRIVER)/Target/*.h \
                     $(DEMOD)/*.h \
                     $(USER)/Common/*.h \
                     $(USER)/Target/*.h \
                     $(BRIDGE)/Common/*.h \
                     $(addsuffix /*.h, $(DRIVER_CHIP_PATH)) \
                     $(addsuffix /*.h, $(BRIDGE_CHIP_PATH)) \
                     $(BRIDGE)/Target/*.h)

TARGET_LIBRARY = libDibDriverLibrary.a

INCLUDE =          -I$(USER)/Common \
				   -I$(DEMOD) \
                   -I$(DRIVER)/Common \
                   -I$(DRIVER)/Common/Messages \
                   -I$(DRIVER)/Target \
                   -I$(BRIDGE)/Common \
                   -I$(BRIDGE)/Target \
                   $(addprefix -I,$(DRIVER_CHIP_PATH)) \
                   $(addprefix -I,$(BRIDGE_CHIP_PATH)) \

# version string to generate proper library
VER_MAJ = $(shell perl -ne 'if (/DIB_SET_VER\((.*?)\)/) { split(/\s*,\s*/, $$1); print $$_[0]; }' ../../Driver/Common/DibDriverConstants.h )
VER_MID = $(shell perl -ne 'if (/DIB_SET_VER\((.*?)\)/) { split(/\s*,\s*/, $$1); print $$_[1]; }' ../../Driver/Common/DibDriverConstants.h )
VER_MIN = $(shell perl -ne 'if (/DIB_SET_VER\((.*?)\)/) { split(/\s*,\s*/, $$1); print $$_[2]; }' ../../Driver/Common/DibDriverConstants.h )

TARGET_SHARED_LIBRARY = libDibDriverLibrary.so.$(VER_MAJ).$(VER_MID).$(VER_MIN)

ifndef DIB_CC
	ERR=$(error unknown Target $(TARGET) defined; please specify one with $0 TARGET=[i386|i386-merge|st8815|mx31|pxa300|pxa310|user|kernel|i2c])
endif

CFLAGS += -g -Wall

ifeq ($(TARGET), i386-shared)
all: ERROR $(TARGET_SHARED_LIBRARY)
else
all: ERROR $(TARGET_LIBRARY)
endif

ERROR:
	$(SILENT)$(ERR)

../../Board/DibBoard%.o : ../../Board/DibBoard%.c $(HEADERS) $(FIRMWARE)/*.h
	$(SILENT)$(ECHO) "Compiling $@...."
	$(SILENT)$(DIB_CC) $(CFLAGS) $(DEFINES) $(INCLUDE) -c -o $@ $<

%.o:%.c $(HEADERS)
	$(SILENT)$(ECHO) "Compiling $@ in $(@:%.o=%.S)...."
	$(SILENT)$(DIB_CC) $(CFLAGS) $(DEFINES) $(INCLUDE) -c -o $@ $<


$(TARGET_LIBRARY) : $(BOOTSTRAP) $(OBJECTS)
	$(SILENT)$(ECHO) "Creating $@...."
	$(SILENT)$(DIB_AR) cr $(TARGET_LIBRARY) $(OBJECTS)
	$(SILENT)$(DIB_RANLIB) $(TARGET_LIBRARY)

# create the shared library
$(TARGET_SHARED_LIBRARY): $(OBJECTS)
	@echo "Creating $@...."
	@gcc -shared -Wl,-soname,libDibDriverLibrary.so.$(VER_MAJ) -o $@ $^
	@ln -sf $@ libDibDriverLibrary.so.$(VER_MAJ)
	@ln -sf libDibDriverLibrary.so.$(VER_MAJ) libDibDriverLibrary.so

clean:
	$(SILENT)-rm -f *.o *.a drv_firefly *~ \
	$(DRIVER)/Common/*.o \
	$(DRIVER)/Common/Messages/*.o \
	$(USER)/Common/*.o \
	$(DRIVER)/Target/*.o \
	$(DEMOD)/*.o \
	$(TARGET_LIBRARY) \
	$(TARGET_SHARED_LIBRARY) \
	$(BRIDGE)/Common/*.o \
	$(addsuffix /*.o, $(DRIVER_CHIP_PATH)) \
	$(addsuffix /*.o, $(BRIDGE_CHIP_PATH)) \
	$(BRIDGE)/Target/*.o> /dev/null 2>&1

default: all

bootstrap:
	$(SILENT)$(EXTRA_LINK)
	$(SILENT)./bootstrap.sh $(BRIDGE_DIR) $(DRIVER_DIR)

update_tag_table:
	$(SILENT)$(ECHO) update tag table
	$(SILENT)-ctags $(SOURCES) $(HEADERS)

update_scope_table:
	$(SILENT)$(ECHO) update scope table
	$(SILENT)-cscope -b $(SOURCES) $(HEADERS)

static_test:
	$(SILENT)$(ECHO) static test on source code
	$(SILENT)-splint $(SOURCES) $(INCLUDE) +bounds >& splint.log

print:
	@echo "DRIVER_CHIP_PATH=$(DRIVER_CHIP_PATH)"
	@echo "BRIDGE_CHIP_PATH=$(BRIDGE_CHIP_PATH)"
	@echo "SOURCES=$(SOURCES)"
	@echo "OBJECTS=$(OBJECTS)"
	@echo "HEADERS=$(HEADERS)"
