#ifndef DEMOD_DIB7000P_PRIV_H
#define DEMOD_DIB7000P_PRIV_H

#include <adapter/frontend.h>
#include <demod/dib7000p.h>

#include <demod/dibx000_i2c_common.h>

#define SOC7090 0x7090

struct dib7000p_state {
	struct dibDemodInfo info;
    struct dibFrontend *fe;
	struct dib7000p_config cfg;

	uint32_t  timf;

	struct dibx000_i2c_master i2c_master;

	uint16_t wbd_ref;

/* store the current values to restore the state after a wakeup */
	uint16_t gpio_dir;
	uint16_t gpio_val;

	uint8_t adc_mode;
	uint8_t current_band;

	uint8_t div_force_off : 1;
	uint8_t div_state : 1;
	uint16_t div_sync_wait;

	const struct dibx000_agc_config *current_agc;

	uint8_t sfn_workaround_active : 1;
        uint16_t version;

	uint32_t  timf_default;
	uint8_t pha3_alpha_default;
	uint16_t pha_shift;
};

extern const struct dibDebugObject dib7000p_dbg;

#define dib7000p_read(st, subaddress, buf, len)  data_bus_client_read(demod_get_data_bus_client(st->fe), subaddress, buf, len)
#define dib7000p_write(st, subaddress, buf, len) data_bus_client_write(demod_get_data_bus_client(st->fe), subaddress, buf, len)
#define dib7000p_read_word(st, subaddress)       data_bus_client_read16(demod_get_data_bus_client(st->fe), subaddress)
#define dib7000p_write_word(st, subaddress,v)    data_bus_client_write16(demod_get_data_bus_client(st->fe), subaddress, v)
#define dib7000p_read32(st, address)              data_bus_client_read32(demod_get_data_bus_client(st->fe), address)

extern int dib7000p_reset_gpio(struct dib7000p_state *);

enum dib7000p_power_mode {
	DIB7000P_POWER_ALL = 0,
	DIB7000P_POWER_INTERFACE_ONLY,
	DIB7000P_POWER_ANALOG_ADC
};
extern int dib7000p_set_power_mode(struct dib7000p_state *, enum dib7000p_power_mode);

extern void dib7000p_set_adc_state(struct dib7000p_state *state, enum dibx000_adc_states no);
extern void dib7000p_update_timf(struct dib7000p_state *state);
extern int dib7000p_set_bandwidth(struct dib7000p_state *state, uint32_t bw);

/* functions for the operation */
extern int dib7000p_set_diversity_in(struct dibFrontend *, int);
extern int dib7000p_set_output_mode(struct dib7000p_state *, int);
extern int dib7000p_pid_filter_ctrl(struct dibFrontend *, uint8_t onoff);
extern int dib7000p_pid_filter(struct dibFrontend *, uint8_t id, uint16_t pid, uint8_t onoff);

extern int dib7000p_autosearch_is_irq(struct dibFrontend *);
extern int dib7000p_tune(struct dibFrontend *, struct dibChannel *);
extern int dib7000p_autosearch_start(struct dibFrontend *, struct dibChannel *);

extern void dib7000p_set_frequency_offset(struct dib7000p_state *state, struct dibChannel *ch);
extern void dib7000p_set_dds(struct dib7000p_state *state, int32_t offset_khz);
extern int dib7000p_get_internal_freq(struct dib7000p_state *state);

/* extended monitoring */
extern int dib7000p_generic_monitoring(struct dibFrontend *, uint32_t, uint8_t *, uint32_t, uint32_t);
#endif
