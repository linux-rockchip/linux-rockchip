#ifndef _NMIDTV_H_
#define _NMIDTV_H_

/********************************************
	Function VTable
********************************************/
typedef struct {
	int (*nmi_config_tuner)(void *);
	int (*nmi_config_demod)(void *);
	void (*nmi_config_mac)(void *);
	void (*nmi_get_snr)(void *);
	void (*nmi_get_ber)(void *);
	void (*nmi_get_per)(void *);
	void (*nmi_track_agc)(void *);
	uint32_t (*nmi_get_chipid)(void);
	uint32_t (*nmi_get_formal_chipid)(void);
	void (*nmi_rst_ber)(void);
	void (*nmi_rst_per)(void);
	void (*nmi_rf_write_reg)(uint32_t, uint8_t);
	uint8_t (*nmi_rf_read_reg)(uint32_t);
	void (*nmi_soft_reset)(void);
	void (*nmi_fec_lock)(void *);
	void (*nmi_handle_intr)(void *);

	/* dvb */
	void (*nmi_get_dvb_sta)(void *);
	void (*nmi_get_dvb_mode)(void *);
	void (*nmi_dvb_rst_lnk)(void);
	void (*nmi_dvb_rst_lnk_cnt)(void);
	void (*nmi_dvb_get_lnk_sta)(void *);
	void (*nmi_dvb_track_alg)(void);
	uint32_t (*nmi_dvb_get_ber_count)(void);
	uint32_t (*nmi_dvb_get_ber_period)(void);
	uint32_t (*nmi_dvb_get_per_count)(void);
	uint32_t (*nmi_dvb_get_per_period)(void);
	void (*nmi_dvb_band_scan)(void *);
	void (*nmi_dvb_get_psisi_tbl)(void *);
	int (*nmi_agc_lock)(void);
	void (*nmi_syr_lock)(void);
	void (*nmi_tps_lock)(void);
	void (*nmi_chc_lock)(void);
	void (*nmi_dvb_ts_shutdown)(void *);
	void (*nmi_dvb_ts_wakeup)(void *);
	uint32_t (*nmi_dvb_ts_delta_t)(int);
	void (*nmi_dvb_set_gio)(void *);
	void (*nmi_dvb_set_pid)(void *);
	void (*nmi_dvb_add_pid)(uint32_t);
	void (*nmi_dvb_remove_pid)(uint32_t);
	int (*nmi_dvb_get_sq)(void);
	void (*nmi_dvb_set_agc_thold)(void *);
} NMIDTVVTBL;

typedef struct {
	int (*nmi_write_reg)(uint32_t, uint8_t *, uint32_t);
	int (*nmi_read_reg)(uint32_t, uint8_t *, uint32_t);
	void (*nmi_delay)(uint32_t);
	uint32_t (*nmi_get_tick)(void);
	void (*nmi_log)(char *);
	int (*nmi_dma_read)(void *);
	int (*nmi_dvb_read_tbl)(void *);
} NMIHLPVTBL;

#endif
