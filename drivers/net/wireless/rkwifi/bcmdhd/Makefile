# bcmdhd
# WL_ENABLE_P2P_IF must be remove if kernel version is 3.8 later

MODULE_NAME = bcmdhd
#CONFIG_RKWIFI = m
CONFIG_CFG80211 = y
CONFIG_BCMDHD_OOB = y

DHDCFLAGS = -Wall -Wstrict-prototypes -Dlinux -DBCMDRIVER                 \
	-DBCMDONGLEHOST -DUNRELEASEDCHIP -DBCMDMA32 -DWLBTAMP -DBCMFILEIMAGE  \
	-DDHDTHREAD -DDHD_DEBUG -DSDTEST -DBDC -DTOE                          \
	-DDHD_BCMEVENTS -DSHOW_EVENTS -DPROP_TXSTATUS -DBCMDBG                \
	-DMMC_SDIO_ABORT -DBCMSDIO -DBCMLXSDMMC -DBCMPLATFORM_BUS             \
	-DWIFI_ACT_FRAME -DARP_OFFLOAD_SUPPORT                                \
	-DKEEP_ALIVE -DPKT_FILTER_SUPPORT                                     \
	-DEMBEDDED_PLATFORM -DENABLE_INSMOD_NO_FW_LOAD -DPNO_SUPPORT          \
	-DDHD_USE_IDLECOUNT -DSET_RANDOM_MAC_SOFTAP -DVSDB                    \
	-DWL_CFG80211_VSDB_PRIORITIZE_SCAN_REQUEST -DSDIO_CRC_ERROR_FIX       \
	-DESCAN_RESULT_PATCH -DHT40_GO -DPASS_ARP_PACKET -DSUPPORT_PM2_ONLY   \
	-DDHD_DONOT_FORWARD_BCMEVENT_AS_NETWORK_PKT -DAMPDU_HOSTREORDER       \
	-DCUSTOM_SDIO_F2_BLKSIZE=128 -DWL_SDO -DWLTDLS                        \
	-DWL_SUPPORT_BACKPORTED_KPATCHES -DGET_CUSTOM_MAC_ENABLE \
	-DWL_IFACE_COMB_NUM_CHANNELS \
	-Idrivers/net/wireless/rkwifi/bcmdhd -Idrivers/net/wireless/rkwifi/bcmdhd/include
#-DWL_IFACE_COMB_NUM_CHANNELS
DHDOFILES = aiutils.o bcmsdh_sdmmc_linux.o dhd_linux.o siutils.o bcmutils.o   \
	dhd_linux_sched.o bcmwifi_channels.o dhd_sdio.o bcmevent.o dhd_bta.o hndpmu.o  \
	bcmsdh.o dhd_cdc.o bcmsdh_linux.o dhd_common.o dhd_wlfc.o dhd_ip.o linux_osl.o \
	bcmsdh_sdmmc.o dhd_custom_gpio.o sbutils.o wldev_common.o wl_android.o \
	dhd_pno.o dhd_config.o

$(MODULE_NAME)-y += $(DHDOFILES)

DHDCFLAGS += -DCUSTOMER_HW2

ifeq ($(CONFIG_BCMDHD_OOB),y)
DHDCFLAGS += -DOOB_INTR_ONLY -DHW_OOB
#-DCUSTOMER_OOB
else
DHDCFLAGS += -DSDIO_ISR_THREAD
endif

ifeq ($(CONFIG_BCMDHD_POWER_OFF_IN_SUSPEND),y)
DHDCFLAGS += -DPOWER_OFF_IN_SUSPEND
endif

#ifeq ($(CONFIG_BCMDHD_AG),y)
DHDCFLAGS += -DBAND_AG
#endif

ifneq ($(CONFIG_WIRELESS_EXT),)
$(MODULE_NAME)-y += wl_iw.o
DHDCFLAGS += -DSOFTAP -DWL_WIRELESS_EXT -DUSE_IW
endif
ifneq ($(CONFIG_CFG80211),)
$(MODULE_NAME)-y += wl_cfg80211.o wl_cfgp2p.o wl_linux_mon.o dhd_cfg80211.o
DHDCFLAGS += -DWL_CFG80211 -DWLP2P -DWL_CFG80211_STA_EVENT -DWL_ENABLE_P2P_IF
DHDCFLAGS += -DCUSTOM_ROAM_TRIGGER_SETTING=-65
DHDCFLAGS += -DCUSTOM_ROAM_DELTA_SETTING=15
DHDCFLAGS += -DCUSTOM_KEEP_ALIVE_SETTING=28000
DHDCFLAGS += -DCUSTOM_PNO_EVENT_LOCK_xTIME=7
endif
ifneq ($(CONFIG_DHD_USE_SCHED_SCAN),)
DHDCFLAGS += -DWL_SCHED_SCAN
endif
EXTRA_CFLAGS = $(DHDCFLAGS)
ifeq ($(CONFIG_BCMDHD),m)
EXTRA_LDFLAGS += --strip-debug
endif

$(MODULE_NAME)-y += rkversion.o

obj-$(CONFIG_RKWIFI) += $(MODULE_NAME).o

EXTRA_CFLAGS = $(DHDCFLAGS)
ifeq ($(CONFIG_RKWIFI),m)
EXTRA_LDFLAGS += --strip-debug
endif

KERNEL_DIR = /home/duke/jb_4.2/kernel

all:
	make -j4 -C $(KERNEL_DIR) M=`pwd` modules
	mv wlan.ko rkwifi.oob.ko
clean:
	rm -fr *.mod.c *.mod *.o .*.cmd *.ko *~
	rm .tmp_versions -fr ; rm Module.symvers -fr
	rm -fr Module.markers ; rm -fr modules.order
