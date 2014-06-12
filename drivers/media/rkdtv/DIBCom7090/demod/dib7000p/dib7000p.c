#include "dib7000p_priv.h"

static int dib7000p_sad_calib(struct dib7000p_state *state)
{
#ifdef CONFIG_DEMOD_DIB7000P_DEBUG
    dbgpl(&dib7000p_dbg," Calibrate Slow ADC");
#endif

/* internal */
//	dib7000p_write_word(state, 72, (3 << 14) | (1 << 12) | (524 << 0)); // sampling clock of the SAD is writting in set_bandwidth

    dib7000p_write_word(state, 73, (0 << 1) | (0 << 0)); /* P_sad_ext_calib = 0 P_sad_calibration = 0 */

    if(state->version == SOC7090)
        dib7000p_write_word(state, 74, 2048); // P_sad_calib_value = (0.9/1.8)*4096
    else
        dib7000p_write_word(state, 74, 776); // P_sad_calib_value = 0.625*3.3 / 4096

    /* do the calibration */
    dib7000p_write_word(state, 73, 1); /* P_sad_calibration  = 1 */
    dib7000p_write_word(state, 73, 0); /* P_sad_calibration  = 0 */

    DibMSleep(1);

	return DIB_RETURN_SUCCESS;
}

static const int32_t lut_1000ln_mant[] = //1000*log(1000:2000:50)
{
    6908,6956,7003,7047,7090,7131,7170,7208,7244,7279,7313,7346,7377,7408,7438,7467,7495,7523,7549,7575,7600
};

int32_t dib7000p_get_adc_power(struct dibFrontend *fe)
{
    struct dib7000p_state *state = fe->demod_priv;
    uint32_t tmp_val =0, exp = 0, mant = 0;
    int32_t pow_i;
    uint16_t buf[2];
    uint8_t ix = 0;

    data_bus_client_read_multi16(demod_get_data_bus_client(state->fe), 0x184, buf, 2);
    pow_i = (buf[0] << 16) | buf[1];
    dbgpl(NULL," raw pow_i = %d", pow_i);

    /* ln_agcpower calc using mant-exp conversion and mantis look up table */
    tmp_val = pow_i;
    while(tmp_val >>= 1)
        exp++;

    mant = (pow_i * 1000 / (1<<exp));
    dbgpl(NULL," mant = %d exp = %d", mant/1000, exp);

    ix = (uint8_t)((mant-1000)/100); /* index of the LUT */
    dbgpl(NULL," ix = %d", ix);

    pow_i = (lut_1000ln_mant[ix] + 693*(exp-20) - 6908); /* 1000 * ln(adcpower_real) ; 693 = 1000ln(2) ; 6908 = 1000*ln(1000) ; 20 comes from adc_real = adc_pow_int / 2**20 */
    pow_i = (pow_i<<8)/1000; /* quantify as agc_ln_power should be : 1signed-4int-8fract */
    dbgpl(NULL," pow_i = %d", pow_i);

    return pow_i;
}

extern void dib7000p_set_dds(struct dib7000p_state *state, int32_t offset_khz)
{
    uint32_t internal = dib7000p_get_internal_freq(state);
    int32_t unit_khz_dds_val = 67108864 / (internal); /* 2**26 / Fsampling is the unit 1KHz offset */
    uint32_t abs_offset_khz = ABS(offset_khz);
    uint32_t dds = state->cfg.bw->ifreq & 0x1ffffff;
    uint8_t invert = !!(state->cfg.bw->ifreq & (1 << 25));
    dbgpl(&dib7000p_dbg, "setting a frequency offset of %dkHz internal freq = %d invert = %d", offset_khz, internal,invert);

    if (offset_khz < 0)
        unit_khz_dds_val *= -1;

    /* IF tuner */
    if (invert)
        dds -= (abs_offset_khz * unit_khz_dds_val); /* /100 because of /100 on the unit_khz_dds_val line calc for better accuracy */
    else
        dds += (abs_offset_khz * unit_khz_dds_val);

    if(abs_offset_khz <= (internal/2)) { /* Max dds offset is the half of the demod freq */
        dib7000p_write_word(state, 21, (uint16_t)(((dds >> 16) & 0x1ff) | (0 << 10) | (invert << 9)));
        dib7000p_write_word(state, 22, (uint16_t)(dds & 0xffff));
    }
}

extern void dib7000p_set_frequency_offset(struct dib7000p_state *state, struct dibChannel *ch)
{
    int total_dds_offset_khz = -((int)ch->RF_kHz - (int)state->fe->current_rf);

    /* Auto compensate all tuner freq offsets (diversity offset and/or low-if offset )*/
    dbgpl(&dib7000p_dbg,"FE %d : %dkhz tuner offset (ch->RF_kHz = %dKHz & state->fe->current_rf = %dKHz) total_dds_offset_khz = %d", state->fe->id, ch->RF_kHz - state->fe->current_rf, ch->RF_kHz, state->fe->current_rf, total_dds_offset_khz);

    /* apply dds offset now */
    dib7000p_set_dds(state, total_dds_offset_khz);
}

/* return the internal KHz freq */
extern int dib7000p_get_internal_freq(struct dib7000p_state *state)
{
    return dib7000p_read32(state, 18) /1000;
}

int dib7000p_update_pll(struct dibFrontend *fe, struct dibx000_bandwidth_config * bw)
{
    struct dib7000p_state *state = fe->demod_priv;
    uint16_t reg_1857, reg_1856 = dib7000p_read_word(state, 1856);
    uint8_t loopdiv, prediv;
    uint32_t internal, xtal;

    /* get back old values */
    prediv = reg_1856 & 0x3f;
    loopdiv = (reg_1856 >> 6) & 0x3f;

    if((bw != NULL) && (bw->pll_prediv != prediv || bw->pll_ratio != loopdiv)) {
        dbgpl(&dib7000p_dbg,"Updating pll (prediv: old =  %d new = %d ; loopdiv : old = %d new = %d)", prediv, bw->pll_prediv, loopdiv, bw->pll_ratio);
        reg_1856 &= 0xf000;
        reg_1857 = dib7000p_read_word(state, 1857);
        dib7000p_write_word(state, 1857, reg_1857 & ~(1<<15)); // desable pll

        dib7000p_write_word(state, 1856, reg_1856 | ((bw->pll_ratio & 0x3f) << 6 ) | (bw->pll_prediv & 0x3f));

        /* write new system clk into P_sec_len */
        internal = dib7000p_get_internal_freq(state);
        xtal = (internal / loopdiv) * prediv;
        internal = 1000 * (xtal/bw->pll_prediv) * bw->pll_ratio; /* new internal */
        dib7000p_write_word(state, 18, (uint16_t) ((internal >> 16) & 0xffff));
        dib7000p_write_word(state, 19, (uint16_t) (internal & 0xffff));

        dib7000p_write_word(state, 1857, reg_1857 | (1<<15)); // enable pll

        while(((dib7000p_read_word(state, 1856)>>15)&0x1) != 1) {
            dbgpl(NULL,"Waiting for PLL to lock");
        }

    /* verify
        reg_1856 = dib7000p_read_word(state, 1856);
        dbgpl(&dib7000p_dbg,"Updating pll (old prediv = %d ; old loopdiv = %d)", reg_1856&0x3f, (reg_1856>>6)&0x3f);
    */
        return DIB_RETURN_SUCCESS;
    }
    return DIB_RETURN_ERROR;
}

static void dib7000p_reset_pll(struct dib7000p_state *state)
{
    const struct dibx000_bandwidth_config *bw = &state->cfg.bw[0];
    uint16_t clk_cfg0, reg_1857;

    if(state->version == SOC7090) {
        dib7000p_write_word(state, 1856, (!bw->pll_reset<<13) | (bw->pll_range<<12) | (bw->pll_ratio<<6) | (bw->pll_prediv));

        while(((dib7000p_read_word(state, 1856)>>15)&0x1) != 1) {
#ifdef CONFIG_DEMOD_DIB7000P_DEBUG
            dbgpl(NULL,"Waiting for PLL to lock");
#endif
        }

#ifdef CONFIG_DEMOD_DIB7000P_DEBUG
        dbgpl(NULL," PLL lock = %d ",(dib7000p_read_word(state, 1856)>>15));
#endif

        reg_1857 = dib7000p_read_word(state, 1857);
        dib7000p_write_word(state, 1857, reg_1857|(!bw->pll_bypass<<15));
    } else {
        /* force PLL bypass */
        clk_cfg0 = (1 << 15) | ((bw->pll_ratio & 0x3f) << 9) |
                (bw->modulo << 7) | (bw->ADClkSrc << 6) | (bw->IO_CLK_en_core << 5) |
                (bw->bypclk_div << 2) | (bw->enable_refdiv << 1) | (0 << 0);

        dib7000p_write_word(state, 900, clk_cfg0);

        /* P_pll_cfg */
        dib7000p_write_word(state, 903, (bw->pll_prediv << 5) | (((bw->pll_ratio >> 6) & 0x3) << 3) | (bw->pll_range << 1) | bw->pll_reset);
        clk_cfg0 = (bw->pll_bypass << 15) | (clk_cfg0 & 0x7fff);
        dib7000p_write_word(state, 900, clk_cfg0);
    }

    dib7000p_write_word(state, 18, (uint16_t) (((bw->internal*1000) >> 16) & 0xffff));
    dib7000p_write_word(state, 19, (uint16_t) ( (bw->internal*1000       ) & 0xffff));
    dib7000p_write_word(state, 21, (uint16_t) ( (bw->ifreq          >> 16) & 0xffff));
    dib7000p_write_word(state, 22, (uint16_t) ( (bw->ifreq               ) & 0xffff));

    dib7000p_write_word(state, 72, bw->sad_cfg);

}

static const uint16_t dib7000p_defaults[] = {
	// auto search configuration
	3, 2,
		0x0004,
	        (1<<3)|(1<<11)|(1<<12)|(1<<13),//0x1000,
		0x0814, /* Equal Lock */

	12, 6,
		0x001b,
		0x7740,
		0x005b,
		0x8d80,
		0x01c9,
		0xc380,
		0x0000,
		0x0080,
		0x0000,
		0x0090,
		0x0001,
		0xd4c0,

	1, 26,
		0x6680, // P_timf_alpha=6, P_corm_alpha=6, P_corm_thres=128 default: 6,4,26

	/* set ADC level to -16 */
	11, 79,
		(1 << 13) - 825 - 117,
		(1 << 13) - 837 - 117,
		(1 << 13) - 811 - 117,
		(1 << 13) - 766 - 117,
		(1 << 13) - 737 - 117,
		(1 << 13) - 693 - 117,
		(1 << 13) - 648 - 117,
		(1 << 13) - 619 - 117,
		(1 << 13) - 575 - 117,
		(1 << 13) - 531 - 117,
		(1 << 13) - 501 - 117,

	1, 142,
		0x0410, // P_palf_filter_on=1, P_palf_filter_freeze=0, P_palf_alpha_regul=16

	/* disable power smoothing */
	8, 145,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,

	1, 154,
		1 << 13, // P_fft_freq_dir=1, P_fft_nb_to_cut=0

	1, 168,
		0x0ccd, // P_pha3_thres, default 0x3000

//	1, 169,
//		0x0010, // P_cti_use_cpe=0, P_cti_use_prog=0, P_cti_win_len=16, default: 0x0010

	1, 183,
		0x200f, // P_cspu_regul=512, P_cspu_win_cut=15, default: 0x2005
        1, 212,
	        0x169,  // P_vit_ksi_dwn = 5 P_vit_ksi_up = 5       0x1e1, // P_vit_ksi_dwn = 4 P_vit_ksi_up = 7

	5, 187,
		0x023d, // P_adp_regul_cnt=573, default: 410
		0x00a4, // P_adp_noise_cnt=
		0x00a4, // P_adp_regul_ext
		0x7ff0, // P_adp_noise_ext
		0x3ccc, // P_adp_fil

	1, 198,
		0x800, // P_equal_thres_wgn

	1, 222,
		0x0010, // P_fec_ber_rs_len=2

	0,
};

static void dib7000p_write_tab(struct dib7000p_state *state, const uint16_t *buf)
{
	uint16_t l = 0, r;
    const uint16_t *n;
	n = buf;
	l = *n++;
	while (l) {
		r = *n++;

		do {
			dib7000p_write_word(state, r, *n++);
			r++;
		} while (--l);
		l = *n++;
	}
}

int dib7000pc_detection(struct dibDataBusHost *host)
{
	uint16_t rx;
	struct dibDataBusClient client;

	data_bus_client_init(&client, &dibx000_i2c_data_bus_client, host);

	data_bus_client_set_device_id(&client, 18);
	rx = data_bus_client_read16(&client, 0x0300);
	if (rx == 0x01b3) {
		dbgpl(&dib7000p_dbg, "DiB7000PC detected");
		return 1;
	}

	data_bus_client_set_device_id(&client, 0x80);
	rx = data_bus_client_read16(&client, 0x0300);
	if (rx == 0x01b3) {
		dbgpl(&dib7000p_dbg, "DiB7000PC detected");
		return 1;
	}

	dbgpl(&dib7000p_dbg, "DiB7000PC not detected");
	return 0;
}

int dib7000p_set_bandwidth(struct dib7000p_state *state, uint32_t bw)
{
	uint32_t timf;

	if (state->timf == 0) {
		dbgpl(&dib7000p_dbg, "using default timf");
		timf = state->cfg.bw->timf;
	} else {
		dbgpl(&dib7000p_dbg, "using updated timf");
		timf = state->timf;
	}

	timf = timf * (bw / 50) / 160;

	dib7000p_write_word(state, 23, (uint16_t) ((timf >> 16) & 0xffff));
	dib7000p_write_word(state, 24, (uint16_t) ((timf      ) & 0xffff));

	return DIB_RETURN_SUCCESS;
}

static int dib7000p_wakeup(struct dibFrontend *fe)
{
	struct dib7000p_state *state = fe->demod_priv;
	dib7000p_set_power_mode(state, DIB7000P_POWER_ALL);
	dib7000p_set_adc_state(state, DIBX000_SLOW_ADC_ON);

    /* for the dib7090, force a new SAD calibration */
    if(state->version == SOC7090)
        dib7000p_sad_calib(state);
    return DIB_RETURN_SUCCESS;
}

static int dib7000p_sleep(struct dibFrontend *fe, uint8_t deep)
{
	struct dib7000p_state *state = fe->demod_priv;
    if (deep) {
        switch (data_bus_host_type( demod_get_data_bus_client(fe)->host )) {
            case DATA_BUS_SDIO:
            case DATA_BUS_MMIO_SRAM_LIKE: data_bus_client_set_speed(demod_get_data_bus_client(fe), 0); break;
            case DATA_BUS_I2C:            data_bus_client_set_speed(demod_get_data_bus_client(fe), 340); break;
        }
        dibx000_deep_sleep_i2c_master(&state->i2c_master);
    } else
        return dib7000p_set_output_mode(state, OUTPUT_MODE_OFF) | dib7000p_set_power_mode(state, DIB7000P_POWER_INTERFACE_ONLY);
    return DIB_RETURN_SUCCESS;
}

static int dib7000p_get_signal_status(struct dibFrontend *fe, struct dibDVBSignalStatus *status)
{
	struct dib7000p_state *st = fe->demod_priv;
	uint16_t v = dib7000p_read_word(st,509);

	status->tps_data_lock  = (v >> 3) & 1;

	// for the 7000P equal_lock is required for div_synchronization - viterbi is stopping
	status->mpeg_data_lock = (v >>  6) & 1;

	if (status->mpeg_data_lock)
		status->bit_error_rate = dib7000p_read32(st,500);
	else
		status->bit_error_rate = 2097151;

	v = dib7000p_read_word(st, 394);
	status->signal_strength = (100 - ((v * 100) / 65535));

	dbgpl(&dib7000p_dbg, "SIGNAL STRENGTH: %d", status->signal_strength);

	return 0;
}

static int dib7000p_demod_output_mode(struct dibFrontend *fe, int mode)
{
	struct dib7000p_state *state = fe->demod_priv;
	return dib7000p_set_output_mode(state, mode);
}

static int dib7000p_release(struct dibFrontend *fe)
{
	struct dib7000p_state *st = fe->demod_priv;
	dibx000_exit_i2c_master(&st->i2c_master);
	if (st != NULL)
		MemFree(st,sizeof(struct dib7000p_state));
    if (fe->release_compat_fe & COMPONENT_LAYER_RELEASE)
        MemFree(fe, sizeof(struct dibFrontend));
   return 0;
}

static uint16_t dib7000p_get_version_id(struct dibDataBusClient *client)
{
        uint16_t version = data_bus_client_read16(client, 897);
#ifdef CONFIG_DEMOD_DIB7000P_DEBUG
        dbgpl(&dib7000p_dbg, "Dib7000p version id = 0x%04x", version);
#endif
        return version;
}

static uint16_t dib7000p_identify(struct dibDataBusClient *client)
{
	uint16_t value;
	dbgpl(&dib7000p_dbg, "checking demod on I2C address: %d (%x)", data_bus_client_get_device_id(client), data_bus_client_get_device_id(client));
	if ((value = data_bus_client_read16(client, 768)) != 0x01b3) {
		dbgpl(&dib7000p_dbg, "wrong Vendor ID (read=0x%x)", value);
		return 0;
	}

	if ((value = data_bus_client_read16(client, 769)) != 0x4000) {
		dbgpl(&dib7000p_dbg, "wrong Device ID (%x)", value);
		return 0;
	}
#ifdef CONFIG_DEMOD_DIB7000P_DEBUG
        dbgpl(&dib7000p_dbg, "Device id = %x", value);
#endif
	return value;
}

/* reset is only after waking up the demod from deep-sleep, not after a software-sleep */
static int dib7000p_reset(struct dibFrontend *fe)
{
    struct dib7000p_state *state = fe->demod_priv;

    /* Uncomment the following line if SRAM mode has to be used on dib7000p chip
       Warning: This write should not be done with SOC7090 because registry changed. */
    //dib7000p_write_word(state, 1287, 0x0003); /* sram lead in, rdy */

    if (dib7000p_identify(demod_get_data_bus_client(fe)) != 0x4000)
        return DIB_RETURN_ERROR;

        state->version = dib7000p_get_version_id(demod_get_data_bus_client(fe));

	dib7000p_set_power_mode(state, DIB7000P_POWER_ALL);

        if(state->version == SOC7090) /* mainly for multi chips diversity/PVR SOC 7090 boards */
            dibx000_reset_i2c_master(&state->i2c_master);

	dib7000p_set_adc_state(state, DIBX000_VBG_ENABLE);

	/* restart all parts */
	dib7000p_write_word(state,  770, 0xffff);
	dib7000p_write_word(state,  771, 0xffff);
	dib7000p_write_word(state,  772, 0x001f);

	/* except i2c, sdio, gpio - control interfaces */
	//dib7000p_write_word(state, 1280, 0x01fc - ((1 << 7) | (1 << 6) | (1 << 5)) );
        /* DIB 7090 SOC */

        dib7000p_write_word(state, 1280, 0x001f - ((1 << 4) | (1 << 3)));

	dib7000p_write_word(state,  770, 0);
	dib7000p_write_word(state,  771, 0);
	dib7000p_write_word(state,  772, 0);
	dib7000p_write_word(state, 1280, 0);

        if(state->version != SOC7090) {
            dib7000p_write_word(state,  898, 0x0003);
            dib7000p_write_word(state,  898, 0); /* P_restart_a2r , P_restart_bist */
        }

	/* default */
	dib7000p_reset_pll(state);

        if (dib7000p_reset_gpio(state) != DIB_RETURN_SUCCESS)
            dbgpl(&dib7000p_dbg, "GPIO reset was not successful.");

        if(state->version == SOC7090) {
            //dib7000p_write_word(state, 1925, (4 << 6) | (1<2));
            dib7000p_write_word(state, 899, 0);

            /* impulse noise */
            dib7000p_write_word(state, 42, (1<<5) | 3); /* P_iqc_thsat_ipc = 1 ; P_iqc_win2 = 3 */
            dib7000p_write_word(state, 43, 0x2d4); /*-300 fag P_iqc_dect_min = -280 */
            dib7000p_write_word(state, 44, 300); /* 300 fag P_iqc_dect_min = +280 */
            dib7000p_write_word(state, 273, (0<<6) | 30); /* P_vit_inoise_sel = 0, P_vit_inoise_gain = 30 */

        }

        if (dib7000p_set_output_mode(state, OUTPUT_MODE_OFF) != DIB_RETURN_SUCCESS)
            dbgpl(&dib7000p_dbg, "OUTPUT_MODE could not be reset.");

        state->current_agc = NULL;

        dib7000p_set_adc_state(state, DIBX000_SLOW_ADC_ON);
        dib7000p_sad_calib(state);
        dib7000p_set_adc_state(state, DIBX000_SLOW_ADC_OFF);

        /* unforce divstr regardless whether i2c enumeration was done or not */
        dib7000p_write_word(state, 1285, dib7000p_read_word(state, 1285) & ~(1 << 1) );

        dib7000p_set_bandwidth(state, 8000); /* SOC 7090 */

	if(state->version == SOC7090) {
            dib7000p_write_word(state, 36, 0x0755);/* P_iqc_impnc_on =1 & P_iqc_corr_inh = 1 for impulsive noise */
	} else { // P_iqc_alpha_pha, P_iqc_alpha_amp_dcc_alpha, ...
	  if(state->cfg.tuner_is_baseband)
            dib7000p_write_word(state, 36,0x0755);
	  else
            dib7000p_write_word(state, 36,0x1f55);
	}

	dib7000p_write_tab(state, dib7000p_defaults);
        /* those followings writes were in dib7000p_defaults but should not be written in SOC7090 case */
        if(state->version != SOC7090) {
            dib7000p_write_word(state, 901, 0x0006); // P_clk_cfg1
            dib7000p_write_word(state, 902, (3 << 10) | (1 << 6)); // P_divclksel=3 P_divbitsel=1
            dib7000p_write_word(state, 905, 0x2c8e); // Tuner IO bank: max drive (14mA) + divout pads max drive
        }

    if (state->cfg.forward_erronous_mpeg_packet)
		dib7000p_write_word(state, 235, 0x0022); // P_smo_mode, P_smo_fifo_flush, P_smo_pid_parse, P_smo_error_discard
    else
		dib7000p_write_word(state, 235, 0x0062); // P_smo_mode, P_smo_rs_discard, P_smo_fifo_flush, P_smo_pid_parse, P_smo_error_discard

	dib7000p_set_power_mode(state, DIB7000P_POWER_INTERFACE_ONLY);

	fe->demod_info->ber_rs_len = 2;

        if(state->version != SOC7090) {
            // high speed i2c is possible after reset
            switch (data_bus_host_type( demod_get_data_bus_client(fe)->host )) {
                case DATA_BUS_MMIO_SRAM_LIKE: data_bus_client_set_speed(demod_get_data_bus_client(fe), 1); break;
                case DATA_BUS_I2C:            data_bus_client_set_speed(demod_get_data_bus_client(fe), 340); break;
            }
        }

	return DIB_RETURN_SUCCESS;
}

struct dibDataBusHost * dib7000p_get_i2c_master(struct dibFrontend *fe, enum dibx000_i2c_interface intf, int gating)
{
    struct dib7000p_state *st = fe->demod_priv;
    return dibx000_get_i2c_adapter(&st->i2c_master, intf, gating);
}

/* first_addr is 0x80 for the DiB7000P */
int dib7000p_i2c_enumeration(struct dibDataBusHost *host, int no_of_demods, uint8_t default_addr, uint8_t first_addr)
{
    int k;
    uint8_t new_addr;
    struct dibDataBusClient client;

    data_bus_client_init(&client, &dibx000_i2c_data_bus_client, host);
    data_bus_client_set_speed(&client, 100);

    /* i2c init com + demod identification */
    for (k = no_of_demods-1; k >= 0; k--) {
        /* designated i2c address */
        new_addr = first_addr + (k << 1);

        data_bus_client_set_device_id(&client, new_addr);
        /* Uncomment the following line if SRAM mode has to be used on dib7000p chip
           Warning: This write should not be done with SOC7090 because registry changed. */
        //data_bus_client_write16(&client, 1287, 0x0003);

        if (dib7000p_identify(&client) == 0) {
            data_bus_client_set_device_id(&client, default_addr);
            /* Uncomment the following line if SRAM mode has to be used on dib7000p chip
               Warning: This write should not be done with SOC7090 because registry changed. */
            //data_bus_client_write16(&client, 1287, 0x0003);

            if (dib7000p_identify(&client) == 0) {
                dbgpl(&dib7000p_dbg, "IC #%d: not identified", k);
                return DIB_RETURN_ERROR;
            }
        }

        /* turn on div_out */
        data_bus_client_write16(&client, 1286, (1 << 10) | (4 << 6));

        // set new i2c address and force divstr (Bit 1) to value 0 (Bit 0)
        data_bus_client_write16(&client, 1285, (new_addr << 2) | 0x2);
        dbgpl(&dib7000p_dbg, "IC #%d initialized (to i2c_address 0x%x)", k, new_addr);
    }

    for (k = 0; k < no_of_demods; k++) {
        new_addr = first_addr | (k << 1);
        data_bus_client_set_device_id(&client, new_addr);

        // unforce divstr
        data_bus_client_write16(&client, 1285, new_addr << 2);

        /* turn off div_out */
        data_bus_client_write16(&client, 1286, 0);
    }
    return DIB_RETURN_SUCCESS;
}

static int dib7000p_set_wbd_ref(struct dibFrontend *fe, uint16_t value)
{
    struct dib7000p_state *state = fe->demod_priv;
    if (value > 4095)
        value = 4095;
    state->wbd_ref = value;
    return dib7000p_write_word(state, 105, (dib7000p_read_word(state, 105) & 0xf000) | value);
}

static uint16_t dib7000p_get_wbd_ref(struct dibFrontend *fe)
{
    struct dib7000p_state *state = fe->demod_priv;
    return state->wbd_ref;
}

static uint32_t dib7000p_ctrl_timf(struct dibFrontend *fe, uint8_t op, uint32_t timf)
{
    struct dib7000p_state *state = fe->demod_priv;
    switch (op) {
        case DEMOD_TIMF_SET: state->timf = state->timf_default = timf; break;
        case DEMOD_TIMF_UPDATE: dib7000p_update_timf(state); break;
        case DEMOD_TIMF_GET: break;
    }
    dib7000p_set_bandwidth(state, fe->current_bw);
    return state->timf;
}

#if 0
#warning should not be enabled
static void dump_reg(struct dibFrontend *fe)
{
    struct dib7000p_state *st = fe->demod_priv;

    int i;
    uint16_t v;
    char filename[1024];
    sprintf(filename, "./dump/dump_7090p_RF_%d_ko.txt", fe->current_rf);

    FILE * f = fopen(filename, "w");
    for ( i = 0; i <= 1987; i++) {
        v = dib7000p_read_word(st, i);
	fprintf(f, "%04d = %04x (%5d)\n", i, v, v);
        //dbgpl(&dib7000p_dbg, "%04d = %04x (%5d)", i, v, v);
    }

    for (i = 0 ; i < 24 ; i++) {
        v = dibx090p_tuner_read_serpar(fe, i);
	fprintf(f, "serpar %d = %04x (%5d)\n", i, v, v);
        //dbgpl(&dibx090p_dbg, "separ %d = 0x%04x", i, word);
    }

    fclose(f);
}
#endif

static int dib7000p_dvb_t_get_channel_description(struct dibFrontend *fe, struct dibChannel *cd)
{
    struct dib7000p_state *state = fe->demod_priv;
    uint16_t v = dib7000p_read_word(state, 463);

    cd->u.dvbt.constellation = (v >> 14) & 0x3;
    cd->u.dvbt.hrch          = (v >> 13) & 0x1;
    if (cd->u.dvbt.select_hp == -1)
        cd->u.dvbt.select_hp = 1;
    cd->u.dvbt.alpha         = (v >> 10) & 0x7;
    cd->u.dvbt.nfft          = (v >>  8) & 0x3;
    cd->u.dvbt.code_rate_hp  = (v >>  5) & 0x7;
    cd->u.dvbt.code_rate_lp  = (v >>  2) & 0x7;
    cd->u.dvbt.guard         =  v        & 0x3;

    v = dib7000p_read_word(state, 464);
    cd->u.dvbt.intlv_native  = (v >>  5) & 0x1;

    return DIB_RETURN_SUCCESS;
}

int dib7000p_bad_sfn_workaround(struct dibFrontend *fe, uint8_t enable)
{
    struct dib7000p_state *state = fe->demod_priv;
    state->sfn_workaround_active = enable;
    return DIB_RETURN_SUCCESS;
}

void dib7000p_close_gate(struct dibFrontend *fe)
{
    struct dib7000p_state *state = fe->demod_priv;
    dib7000p_write_word(state, 1025, 1 << 8);
}

static const struct dibDemodInfo dib7000p_info = {
    "DiBcom DiB7000PC", // name
    MODE_DVBT,          // caps
    32,                 // pid_filter_count

    {                 // ops
    dib7000p_reset,   // reset
    dib7000p_wakeup,  // init
    dib7000p_sleep,   // sleep

    dib7000p_agc_startup,   // agc_restart
    dib7000p_tune,          // tune

    dib7000p_autosearch_start,  // autosearch_start
    dib7000p_autosearch_is_irq, // autosearch_irq
    dib7000p_dvb_t_get_channel_description, // get_channel_desc

    dib7000p_set_diversity_in,  // set_div_in
    dib7000p_demod_output_mode, // set_output_mode
    dib7000p_pid_filter_ctrl,   // pid_filter_ctrl
    dib7000p_pid_filter,        // pid_filter

    dib7000p_set_wbd_ref,
    dib7000p_get_wbd_ref,

    dib7000p_set_gpio,          // set_gpio
    dib7000p_get_signal_status, // get_signal_status

    dib7000p_ctrl_timf,            // ctrl_timf

#ifdef DIBCOM_GENERIC_MONITORING
    dib7000p_generic_monitoring,   // generic_monitoring
#else
    NULL,
#endif
    dib7000p_release, // release
    }
};

struct dibFrontend * dib7000p_register(struct dibFrontend *fe, struct dibDataBusHost *host, uint8_t default_addr, const struct dib7000p_config *cfg)
{
    struct dib7000p_state *state = MemAlloc(sizeof(struct dib7000p_state));
    struct dibDataBusClient *client;
    if (state == NULL)
        return NULL;
    DibZeroMemory(state,sizeof(struct dib7000p_state));

    frontend_register_demod(fe, &state->info, &dib7000p_info, state);
    fe->release_compat_fe &= ~COMPONENT_LAYER_RELEASE;
    memcpy(&state->cfg, cfg, sizeof(struct dib7000p_config));
    state->gpio_val = cfg->gpio_val;
    state->gpio_dir = cfg->gpio_dir;

    state->timf_default = cfg->bw->timf;

    state->fe = fe;

    client = demod_get_data_bus_client(fe);

    if (state->cfg.agc_config_count < 1 || state->cfg.agc_config_count > 3) {
        dbgpl(&dib7000p_dbg, "not more than 3 and not less than 1 AGC configurations are supported.");
        state->cfg.agc_config_count = 1;
    }

    switch (data_bus_host_type(host)) {
        case DATA_BUS_I2C:
            data_bus_client_init(client, &dibx000_i2c_data_bus_client, host);
            data_bus_client_set_speed(client, 340);
            break;
        case DATA_BUS_MMIO_SRAM_LIKE:
        case DATA_BUS_SDIO:
            data_bus_client_init(client, &dibx000_mmio_data_bus_client, host);
            data_bus_client_set_speed(client, 0);
            break;
    }

    data_bus_client_set_device_id(client, default_addr);
    dibx000_init_i2c_master(&state->i2c_master, DIB7000P, client);

#ifdef CONFIG_DEMOD_DIB7000P_SMART_SFN_CORRECTION
    state->pha3_alpha_default = 0xff; /* set the value to -1 (unset) */
#endif
    return fe;
}

int dib7000p_attach(struct dibDataBusHost *i2c_adap, int no_of_demods, uint8_t default_addr, uint8_t do_i2c_enum, const struct dib7000p_config cfg[], struct dibDemod *demod[])
{
    int i, num = 0;

    if (no_of_demods < 1)
        return DIB_RETURN_ERROR;

    /* preparing the structures */
    for (i = 0; i < no_of_demods ; i++) {
        demod[i] = MemAlloc(sizeof(struct dibFrontend));
        frontend_init(demod[i]);
        if (do_i2c_enum)
            demod[i] = dib7000p_register(demod[i], i2c_adap, 0x80+2*i, &cfg[i]);
        else
            demod[i] = dib7000p_register(demod[i], i2c_adap, default_addr, &cfg[i]);
        demod[i]->release_compat_fe |= COMPONENT_LAYER_RELEASE;
        num++;
        if (demod[i] == NULL)
            goto error;
    }

    if (do_i2c_enum) {
        if (dib7000p_i2c_enumeration(i2c_adap,no_of_demods,default_addr,0x80) != DIB_RETURN_SUCCESS)
            goto error;
    } else {
        data_bus_client_set_device_id(demod_get_data_bus_client(demod[0]), default_addr);
        if (dib7000p_identify(demod_get_data_bus_client(demod[0])) !=0)
            goto error;
    }

    // do basic resets and power down
    for (i = 0; i < no_of_demods; i++) {
        dib7000p_reset(demod[i]);

    }
    return 0;

error:
    for (i = 0; i < num; i++) {
        MemFree(demod[i]->demod_priv,sizeof(struct dib7000p_state));
        demod[i] = NULL;
    }

    return DIB_RETURN_FRONT_END_COM_ERROR;
}

const struct dibDebugObject dib7000p_dbg = {
	DEBUG_DEMOD,
	"DiB7000P",
};
