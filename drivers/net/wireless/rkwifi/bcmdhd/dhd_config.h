
#ifndef _dhd_config_
#define _dhd_config_

#include <bcmdevs.h>
#include <dngl_stats.h>
#include <dhd.h>
#include <wlioctl.h>
#ifdef POWER_OFF_IN_SUSPEND
#include <wl_android.h>
#include <bcmsdbus.h>
#include <linux/mmc/sdio_func.h>
#endif

#define FW_PATH_AUTO_SELECT 1
extern char firmware_path[MOD_PARAM_PATHLEN];

/* channel list */
typedef struct wl_channel_list {
	/* in - # of channels, out - # of entries */
	uint32 count;
	/* variable length channel list */
	uint32 channel[WL_NUMCHANNELS];
} wl_channel_list_t;

typedef struct dhd_conf {
	char fw_path[MOD_PARAM_PATHLEN];		/* Firmware path */
	char nv_path[MOD_PARAM_PATHLEN];		/* NVRAM path */
	uint band;			/* Band, b:2.4G only, otherwise for auto */
	wl_country_t cspec;		/* Country */
	wl_channel_list_t channels;	/* Support channels */
	uint roam_off;		/* Roaming, 0:enable, 1:disable */
	uint roam_off_suspend;		/* Roaming in suspend, 0:enable, 1:disable */
	int roam_trigger[2];		/* The RSSI threshold to trigger roaming */
	int roam_scan_period[2];	/* Roaming scan period */
	int roam_delta[2];			/* Roaming candidate qualification delta */
	int fullroamperiod;			/* Full Roaming period */
	uint filter_out_all_packets;	/* Filter out all packets in early suspend */
	uint keep_alive_period;		/* The perioid to send keep alive packet */
} dhd_conf_t;

void dhd_conf_set_fw_name_by_chip(dhd_pub_t *dhd, char *dst, char *src);
void dhd_conf_set_hw_oob_intr(bcmsdh_info_t *sdh, uint chip);
void dhd_conf_set_fw_path(dhd_pub_t *dhd, char *fw_path);
void dhd_conf_set_nv_path(dhd_pub_t *dhd, char *nv_path);
int dhd_conf_set_band(dhd_pub_t *dhd);
uint dhd_conf_get_band(dhd_pub_t *dhd);
int dhd_conf_set_country(dhd_pub_t *dhd);
int dhd_conf_get_country(dhd_pub_t *dhd);
bool dhd_conf_match_channel(dhd_pub_t *dhd, uint32 channel);
int dhd_conf_set_roam(dhd_pub_t *dhd);
void dhd_conf_set_bw(dhd_pub_t *dhd);
int dhd_conf_download_config(dhd_pub_t *dhd);
int dhd_conf_preinit(dhd_pub_t *dhd);
int dhd_conf_attach(dhd_pub_t *dhd);
void dhd_conf_detach(dhd_pub_t *dhd);

extern void *bcmsdh_get_drvdata(void);

#ifdef POWER_OFF_IN_SUSPEND
extern struct net_device *g_netdev;
#if defined(CONFIG_HAS_EARLYSUSPEND)
extern int g_wifi_on;
void wl_cfg80211_stop(void);
void wl_cfg80211_send_disconnect(void);
void wl_cfg80211_user_sync(bool lock);
#endif
void dhd_conf_wifi_suspend(struct sdio_func *func);
void dhd_conf_register_wifi_suspend(struct sdio_func *func);
void dhd_conf_unregister_wifi_suspend(struct sdio_func *func);
#endif

#endif /* _dhd_config_ */
