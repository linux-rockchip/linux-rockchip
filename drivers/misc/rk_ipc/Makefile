#
# Makefile for the Rockchip IPC module
#

# Object file lists.

#obj-m := ipc.o ipc_mbox.o ipc_serial.o baseband.o
obj-$(CONFIG_RK_IPC) +=ipc.o ipc_mbox.o ipc_serial.o baseband.o  
ipc-objs	:= rk_ipc_mem.o rk_ipcmsg_queue.o rk_ipc.o rk_ipc_pipe.o
ipc_mbox-objs	:= rk_ipc_mbox.o rk3190_mbox.o
ipc_serial-objs	:= rk_ipc_serial.o
baseband-objs := rk_baseband.o 

module-objs := $(ipc-objs)

KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)
default:
	$(MAKE) -C $(KERNELDIR)  M=$(PWD) modules
clean:
	rm *.o *.mod.c .*.cmd .tmp_versions *~ modules.order *.ko Module.symvers -rf
