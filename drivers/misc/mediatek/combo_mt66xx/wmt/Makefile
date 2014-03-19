MTK_WCN_CMB_SDIO_EINT := y

ccflags-y += -I$(src)/linux/include -I$(src)/core/include -I$(src)/include -I$(src)/
ccflags-y += -DWMT_PLAT_APEX=1
ccflags-y += -DWMT_PLAT_ALPS=0
ccflags-y += -DWMT_UART_RX_MODE_WORK=1 # 1. work thread 0. tasklet
ccflags-y += -DREMOVE_MK_NODE=1
ifeq ($(strip $(MTK_WCN_CMB_SDIO_EINT)), y)
ccflags-y += -DMTK_CMB_SDIO_EINT
endif

#stub to build-in
obj-y += platform/vendor/mtk_wcn_cmb_stub.o

#Common SDIO driver for WIFI and STP(mtk_hif_sdio.ko)
obj-$(CONFIG_MTK_COMBO_WMT) += mtk_hif_sdio.o
mtk_hif_sdio-objs := 	linux/hif_sdio.o \
						linux/hif_sdio_chrdev.o \
						linux/osal.o

ifeq ($(MTK_WCN_CMB_SDIO_EINT), y)
mtk_hif_sdio-objs	+= linux/hif_sdio_eint.o
endif

# WMT/STP DRIVER(mtk_stp_wmt.ko)
obj-$(CONFIG_MTK_COMBO_WMT) += mtk_stp_wmt.o
mtk_stp_wmt-objs := core/wmt_core.o \
					core/wmt_ctrl.o  \
					core/wmt_func.o \
					core/wmt_ic_6620.o \
					core/wmt_lib.o \
					core/wmt_conf.o \
					core/wmt_dbg.o \
					core/wmt_exp.o \
					core/wmt_ic_6628.o \
					linux/wmt_dev.o \
					linux/wmt_tm.o \
					platform/vendor/wmt_plat.o \
					platform/vendor/wmt_plat_stub.o \
					platform/vendor/mtk_wcn_cmb_hw.o \
					core/stp_exp.o \
					core/stp_core.o \
					core/psm_core.o \
					core/btm_core.o \
					linux/stp_dbg.o
ifeq ($(CONFIG_MTK_COMBO_WMT), m)
mtk_stp_wmt-objs += linux/osal.o 
endif

#WMT/STP use UART interface(mtk_stp_uart.ko)
obj-$(CONFIG_MTK_COMBO_WMT) += mtk_stp_uart.o
mtk_stp_uart-objs	:= linux/stp_uart.o

#WMT/STP use SDIO interface(mtk_stp_sdio.ko)
obj-$(CONFIG_MTK_COMBO_WMT) += mtk_stp_sdio.o
mtk_stp_sdio-objs :=	linux/stp_sdio.o \

ifeq ($(CONFIG_MTK_COMBO_WMT), m)
mtk_stp_sdio-objs += linux/osal.o 
endif

#BT character device driver
obj-$(CONFIG_MTK_COMBO_BT) += mtk_stp_bt.o
mtk_stp_bt-objs		:= linux/stp_chrdev_bt.o

#GPS character device driver
obj-$(CONFIG_MTK_COMBO_GPS) += mtk_stp_gps.o
mtk_stp_gps-objs	:= linux/stp_chrdev_gps.o

#WIFI character device driver
obj-$(CONFIG_MTK_COMBO_WIFI) += mtk_wmt_wifi.o
mtk_wmt_wifi-objs       := linux/wmt_chrdev_wifi.o 

#FM don't need such character device

