#include "dib8000p_priv.h"

static int dib8000p_set_bus_speed(struct dibFrontend *fe, uint16_t bus_type, uint32_t speed)
{
    struct dib8000p_state *state = fe->demod_priv;

    if (data_bus_host_type( demod_get_data_bus_client(fe)->host ) == bus_type) {
        if ((bus_type == DATA_BUS_I2C) && (dib8000p_get_internal_freq(state)/2 < 57000) && (speed > 400))
            data_bus_client_set_speed(demod_get_data_bus_client(fe), 400);
        else
            data_bus_client_set_speed(demod_get_data_bus_client(fe), speed);
    }
    return DIB_RETURN_SUCCESS;
}

static int dib8000p_sad_calib(struct dib8000p_state *state)
{
    uint8_t sad_sel = 3;
    dbgpl(&dib8000p_dbg, "FE %d dib8000p_sad_calib", state->fe->id);

    dib8000p_write_word(state, 922, (sad_sel<<2)); /*sal sel to 3 */ 
    dib8000p_write_word(state, 923, 2048); // 0.9*1.8/4096

    dib8000p_write_word(state, 922, (sad_sel<<2) | 0x1);
    dib8000p_write_word(state, 922, (sad_sel<<2));

    DibMSleep(1);
    return DIB_RETURN_SUCCESS;
}

static int dib8000p_wakeup(struct dibFrontend *fe)
{
    struct dib8000p_state *state = fe->demod_priv;
    dbgpl(&dib8000p_dbg, "Wakeup demod");
    dib8000p_set_power_mode(state, DIB8000P_POWER_ALL);

    dib8000p_set_adc_state(state, DIBX000_ADC_ON);
    if (dib8000p_set_adc_state(state, DIBX000_SLOW_ADC_ON) != DIB_RETURN_SUCCESS)
        dbgpl_advanced(&dib8000p_dbg, "FE %d Can not start Slow ADC", fe->id);

    dib8000p_sad_calib(state);

    return DIB_RETURN_SUCCESS;
}

static int dib8000p_sleep(struct dibFrontend *fe, uint8_t deep)
{
    struct dib8000p_state *st = fe->demod_priv;
    if (!deep) {
        if ((data_bus_host_type( demod_get_data_bus_client(fe)->host) == DATA_BUS_I2C))
            data_bus_client_set_speed(demod_get_data_bus_client(fe), 340);

        dib8000p_set_power_mode(st, DIB8000P_POWER_INTERFACE_ONLY);
        return dib8000p_set_adc_state(st, DIBX000_SLOW_ADC_OFF) |
        dib8000p_set_adc_state(st, DIBX000_ADC_OFF);
    } else {
        dibx000_deep_sleep_i2c_master(&st->i2c_master);
        return DIB_RETURN_SUCCESS;
    }
}

void dib8000p_set_timf(struct dibFrontend *fe, uint32_t timf)
{
    struct dib8000p_state *state = fe->demod_priv;
    state->timf_default = timf;
    dib8000p_write_word(state, 29, (uint16_t) (timf >> 16) & 0xffff);
    dib8000p_write_word(state, 30, (uint16_t)  timf        & 0xffff);
}

int dib8000p_get_timf(struct dibFrontend *fe)
{
    struct dib8000p_state *state = fe->demod_priv;
    uint16_t buf[2];
    data_bus_client_read_multi16(demod_get_data_bus_client(state->fe), 29, buf, 2);
    return ((buf[0] << 16) | buf[1]);
}

void dib8000p_set_dds(struct dib8000p_state *state, int32_t offset_khz)
{
    int32_t unit_khz_dds_val = (1<<26) / (dib8000p_get_internal_freq(state)); /* 2**26 / Fsampling is the unit 1KHz offset */
    uint32_t abs_offset_khz = ABS(offset_khz);
    uint32_t dds = state->cfg.pll->ifreq & 0x1ffffff;
    uint8_t invert = !!(state->cfg.pll->ifreq & (1 << 25));

    if (offset_khz < 0)
        dds = (1<<26) -(abs_offset_khz * unit_khz_dds_val);
    else
        dds = (abs_offset_khz * unit_khz_dds_val);

    if(invert)
        dds = (1<<26) -dds;

    dbgpl(&dib8000p_dbg, "FE %d: Setting a DDS frequency offset of %s %dkHz (%d : %d)", state->fe->id, invert ? "inverted spectrum" : "non inverted spectrum", offset_khz, abs_offset_khz, dds);

    if (abs_offset_khz <= (uint32_t)(dib8000p_get_internal_freq(state)/4)) { /* Max dds offset is the half of the demod freq */
        dib8000p_write_word(state, 26, invert);
        dib8000p_write_word(state, 27, (uint16_t) (dds >> 16) & 0x1ff);
        dib8000p_write_word(state, 28, (uint16_t) (dds      ) & 0xffff);
    }
}

extern void dib8000p_set_frequency_offset(struct dib8000p_state *state, struct dibChannel *ch)
{
    int i;
    int total_dds_offset_khz = -((int)ch->RF_kHz - (int)state->fe->current_rf);

    if (ch->u.isdbt.sb_mode) {
        state->subchannel = ch->u.isdbt.sb_subchannel;

        i = dib8000p_read_word(state, 26) & 1; /* P_dds_invspec */
        dib8000p_write_word(state, 26, ch->u.isdbt.spectrum_inversion^i);

        if (state->cfg.pll->ifreq == 0) { /* low if tuner */
            if ((ch->u.isdbt.spectrum_inversion^i) == 0)
                dib8000p_write_word(state, 26, dib8000p_read_word(state, 26) | 1);
        } else {
            if ((ch->u.isdbt.spectrum_inversion^i) == 0)
                total_dds_offset_khz *= -1;
        }
    }

    /* Auto compensate all tuner freq offsets (diversity offset and/or low-if offset )*/
    dbgpl(&dib8000p_dbg,"FE %d : %dkhz tuner offset (ch->RF_kHz = %dKHz & state->fe->current_rf = %dKHz) total_dds_offset_khz = %d", state->fe->id, ch->RF_kHz - state->fe->current_rf, ch->RF_kHz, state->fe->current_rf, total_dds_offset_khz);

    /* apply dds offset now */
    dib8000p_set_dds(state, total_dds_offset_khz);
}

static int dib8000p_get_signal_status(struct dibFrontend *fe, struct dibDVBSignalStatus *status)
{
    struct dib8000p_state *st = fe->demod_priv;
    uint16_t v = dib8000p_read_word(st, 570);

    status->tps_data_lock  = (v >> 1) & 1; /* TMCC Data lock */
    status->mpeg_data_lock = !!((v >> 5) & 7); /* if one of the fec_locks is one, we say data is present */

    if (status->mpeg_data_lock)
        status->bit_error_rate = dib8000p_read32(st, 562);
    else
        status->bit_error_rate = 2097151;

    v = dib8000p_read_word(st, 390);
    status->signal_strength = (100 - ((v * 100) / 65535));
	
    return DIB_RETURN_SUCCESS;
}

static int dib8000p_set_wbd_ref(struct dibFrontend *fe, uint16_t value)
{
    struct dib8000p_state *state = fe->demod_priv;
    if (value > 4095)
        value = 4095;
    state->wbd_ref = value;
    return dib8000p_write_word(state, 106, value);
}

static uint16_t dib8000p_get_wbd_ref(struct dibFrontend *fe)
{
    struct dib8000p_state *state = fe->demod_priv;
    return state->wbd_ref;
}

static const int32_t lut_1000ln_mant[] =
{
    6908,7003,7090,7170,7244,7313,7377,7438,7495,7549,7600
};

int32_t dib8000p_get_adc_power(struct dibFrontend *fe, uint8_t mode)
{
    struct dib8000p_state *state = fe->demod_priv;
    uint32_t ix =0, tmp_val =0, exp = 0, mant = 0;
    int32_t val;

    val = dib8000p_read32(state, 384);
    /* mode = 1 : ln_agcpower calc using mant-exp conversion and mantis look up table */
    if(mode) {
        tmp_val = val;
        while(tmp_val>>=1)
            exp++;
        mant = (val * 1000 / (1<<exp));
        ix = (uint8_t)((mant-1000)/100); /* index of the LUT */
        val = (lut_1000ln_mant[ix] + 693*(exp-20) - 6908); /* 1000 * ln(adcpower_real) ; 693 = 1000ln(2) ; 6908 = 1000*ln(1000) ; 20 comes from adc_real = adc_pow_int / 2**20 */
        val = (val*256)/1000;
    }
    return val;
}


static uint32_t dib8000p_ctrl_timf(struct dibFrontend *fe, uint8_t op, uint32_t timf)
{
    struct dib8000p_state *state = fe->demod_priv;
    switch (op) {
        case DEMOD_TIMF_SET: state->timf = timf; break;
        case DEMOD_TIMF_UPDATE: dib8000p_update_timf(state); break;
        case DEMOD_TIMF_GET: break;
    }
    dib8000p_set_bandwidth(state, fe->current_bw);
    return state->timf;
}

static int dib8000p_get_channel(struct dibFrontend *fe, struct dibChannel *cd)
{
    struct dib8000p_state *state = fe->demod_priv;
    uint16_t v;
    int i;

    cd->type = STANDARD_ISDBT;

    v = (dib8000p_read_word(state, 572) & 0x30 ) >> 4; // read mode
    switch (v) {
        case 1: cd->u.isdbt.nfft = FFT_2K; break;
        case 2: cd->u.isdbt.nfft = FFT_4K; break;
        default:
        case 3: cd->u.isdbt.nfft = FFT_8K; break;
    }
    v = dib8000p_read_word(state, 572) & 0x3;

    switch (v) {
        case 0: cd->u.isdbt.guard = GUARD_INTERVAL_1_32; break;
        case 1: cd->u.isdbt.guard = GUARD_INTERVAL_1_16; break;
        case 2: cd->u.isdbt.guard = GUARD_INTERVAL_1_8 ; break;
        default:
        case 3: cd->u.isdbt.guard = GUARD_INTERVAL_1_4 ; break;
    }
    cd->u.isdbt.partial_reception  =  dib8000p_read_word(state, 505) & 0x1;
    cd->u.isdbt.sb_mode            =  dib8000p_read_word(state, 508) & 0x1;
    cd->u.isdbt.spectrum_inversion = (dib8000p_read_word(state, 572) & 0x40) >> 6;

    for (i=0; i<3; i++) {
        cd->u.isdbt.layer[i].code_rate = dib8000p_read_word(state, 481+i) & 0x7;

        switch (dib8000p_read_word(state, 487+i)) {
            case 0: cd->u.isdbt.layer[i].constellation = QAM_DQPSK; break;
            case 1: cd->u.isdbt.layer[i].constellation = QAM_QPSK ; break;
            case 2: cd->u.isdbt.layer[i].constellation = QAM_16QAM; break;
            default :
            case 3: cd->u.isdbt.layer[i].constellation = QAM_64QAM; break;
        }
        cd->u.isdbt.layer[i].nb_segments = dib8000p_read_word(state, 493+i) & 0xf;
        cd->u.isdbt.layer[i].time_intlv  = dib8000p_read_word(state, 499+i) & 0x3;
    }
    cd->u.isdbt.sb_subchannel = state->subchannel;

    return DIB_RETURN_SUCCESS;
}

static int dib8000p_release(struct dibFrontend *fe)
{
    struct dib8000p_state *st = fe->demod_priv;
    dibx000_exit_i2c_master(&st->i2c_master);
    if (st != NULL)
        MemFree(st,sizeof(struct dib8000p_state));
    if (fe->release_compat_fe & COMPONENT_LAYER_RELEASE)
        MemFree(fe, sizeof(struct dibFrontend));
    return 0;
}

/* FT 8090P adapt done */
static uint16_t dib8000p_identify(struct dibDataBusClient *client)
{
    uint16_t value;

    //because of glitches sometimes
    value = data_bus_client_read16(client, 896);

    if ((value = data_bus_client_read16(client, 896)) != 0x01b3) {
        dbgpl(&dib8000p_dbg, "wrong Vendor ID (read=0x%x)", value);
        return 0;
    }

    value = data_bus_client_read16(client, 897);
    if (value != 0x8090) {
        dbgpl(&dib8000p_dbg, "wrong Device ID (0x%x)", value);
        return 0;
    } else
        dbgpl(&dib8000p_dbg, "found DIB8090P");

    return value;
}

/* exported functions */
void dib8000p_set_hostbus_diversity(struct dibFrontend *fe, uint8_t onoff)
{
    struct dib8000p_state *st = fe->demod_priv;
    st->cfg.hostbus_diversity = onoff;
}

static int dib8000p_set_gpio(struct dibFrontend *fe, uint8_t num, uint8_t dir, uint8_t val)
{
    struct dib8000p_state *state = fe->demod_priv;
    return dib8000p_cfg_gpio(state, num, dir, val);
}

int dib8000p_set_bandwidth(struct dib8000p_state *state, uint32_t bw)
{
    uint32_t timf;

    if (bw == 0)
        bw = 6000;

    if (state->timf == 0) {
        dbgpl(&dib8000p_dbg, "FE %d is going to use the default TIMF value %d", state->fe->id, state->timf_default);
        timf = state->timf_default;
    } else {
        dbgpl(&dib8000p_dbg, "FE %d is going to use the updated TIMF %d value", state->fe->id, state->timf);
        timf = state->timf;
    }

    timf = (timf/120) * (bw/50);
    dbgpl(&dib8000p_dbg, "FE %d timf Vs banwidth(%d) = %d", state->fe->id, bw, timf);

    dib8000p_set_timf(state->fe, timf);
    return DIB_RETURN_SUCCESS;
}

/* return the internal KHz freq */
extern uint32_t dib8000p_get_internal_freq(struct dib8000p_state *state)
{
    return dib8000p_read32(state, 23) /1000;
}

extern void dib8000p_set_internal_freq(struct dib8000p_state *state, uint32_t internal)
{
    dbgpl(&dib8000p_dbg,"FE %d: writing %d On P_secLen", state->fe->id, internal);
    dib8000p_write_word(state, 23, (uint16_t) ((internal >> 16) & 0xffff));
    dib8000p_write_word(state, 24, (uint16_t) (internal & 0xffff));
    dbgpl(&dib8000p_dbg,"FE %d: read back %d On P_secLen", state->fe->id, dib8000p_get_internal_freq(state));
}

static void dib8000p_reset_pll_common(struct dib8000p_state *state, const struct dibx000_bandwidth_config *bw)
{
    dbgpl(&dib8000p_dbg, "FE %d ifreq: %d %x, inversion: %d", state->fe->id, bw->ifreq, bw->ifreq, bw->ifreq >> 25);
    dbgpl(&dib8000p_dbg, "FE %d bw->internal = %d", state->fe->id, bw->internal);
    dib8000p_set_internal_freq(state, (bw->internal/2*1000));
    dib8000p_write_word(state, 27, (uint16_t)(( bw->ifreq          >> 16) & 0x01ff));
    dib8000p_write_word(state, 28,   (uint16_t)(bw->ifreq                 & 0xffff));
    dib8000p_write_word(state, 26,  (uint16_t)((bw->ifreq          >> 25) & 0x0003));
    //dib8000p_write_word(state, 921, bw->sad_cfg);
}

static void dib8000p_reset_pll(struct dib8000p_state *state)
{
    const struct dibx000_bandwidth_config * pll = state->cfg.pll;
    uint16_t reg =0;

    dbgpl(NULL, "FE%d : reset pll", state->fe->id);

    dib8000p_write_word(state, 1856, (!pll->pll_reset<<13) | (pll->pll_range<<12) | (pll->pll_ratio<<6) | (pll->pll_prediv));

#ifdef ADVANCED_DEBUG_DIB8000P
    while(((dib8000p_read_word(state, 1856)>>15)&0x1) != 1) {
            dbgpl_advanced(NULL,"Waiting for PLL to lock");
    }
    dbgpl_advanced(NULL," PLL lock = %d ",(dib8000p_read_word(state, 1856)>>15));
#endif

    reg = dib8000p_read_word(state, 1857);
    dib8000p_write_word(state, 1857, reg|(!pll->pll_bypass<<15));

    reg = dib8000p_read_word(state, 1858); /* Force clk out pll /2 */
    dib8000p_write_word(state, 1858, reg | 1);

    dib8000p_write_word(state, 904, (pll->modulo << 8));
    dib8000p_reset_pll_common(state, pll);
}

int dib8000p_update_pll(struct dibFrontend *fe, struct dibx000_bandwidth_config * pll)
{
    struct dib8000p_state *state = fe->demod_priv;
    uint16_t reg_1857, reg_1856 = dib8000p_read_word(state, 1856);
    uint8_t loopdiv, prediv;
    uint32_t internal, xtal;

    /* get back old values */
    prediv = reg_1856 & 0x3f;
    loopdiv = (reg_1856 >> 6) & 0x3f;

    dib8000p_set_bus_speed(fe, DATA_BUS_I2C, 200);

    if((pll != NULL) && (pll->pll_prediv != prediv || pll->pll_ratio != loopdiv)) {
        dbgpl(&dib8000p_dbg,"FE%d : Updating pll (prediv: old =  %d new = %d ; loopdiv : old = %d new = %d)", fe->id, prediv, pll->pll_prediv, loopdiv, pll->pll_ratio);
        reg_1856 &= 0xf000;
        reg_1857 = dib8000p_read_word(state, 1857);
        dib8000p_write_word(state, 1857, reg_1857 & ~(1<<15)); // desable pll

        dib8000p_write_word(state, 1856, reg_1856 | ((pll->pll_ratio & 0x3f) << 6 ) | (pll->pll_prediv & 0x3f));

        /* write new system clk into P_sec_len */
        internal = dib8000p_get_internal_freq(state); /*dib8090p system clk = 2 * demod clk*/
	dbgpl(&dib8000p_dbg,"FE%d : Old Internal = %d", fe->id, internal);
        xtal = 2 * (internal / loopdiv) * prediv;
        internal = 1000 * (xtal/pll->pll_prediv) * pll->pll_ratio; /* new internal */
	dbgpl(&dib8000p_dbg,"FE%d : Xtal = %d , New Fmem = %d New Fdemod = %d, New Fsampling = %d", fe->id, xtal, internal/1000, internal/2000, internal/8000);
        dbgpl(&dib8000p_dbg,"FE%d : New Internal = %d", fe->id, internal);
        
        dib8000p_set_internal_freq(state, internal/2);
        dib8000p_write_word(state, 1857, reg_1857 | (1<<15)); // enable pll

        while(((dib8000p_read_word(state, 1856)>>15)&0x1) != 1) {
            dbgpl(&dib8000p_dbg,"FE%d : Waiting for PLL to lock", fe->id);
        }

	dib8000p_set_bus_speed(fe, DATA_BUS_I2C, 1000);

	/* verify */ 
        reg_1856 = dib8000p_read_word(state, 1856);
        dbgpl(&dib8000p_dbg,"FE%d : PLL Updated with prediv = %d and loopdiv = %d", fe->id, reg_1856&0x3f, (reg_1856>>6)&0x3f);

        return DIB_RETURN_SUCCESS;
    }
    return DIB_RETURN_ERROR;
}

static const uint16_t dib8000p_defaults[] = {
    /* auto search configuration - lock0 by default waiting
       for cpil_lock; lock1 cpil_lock; lock2 tmcc_sync_lock */

    3, 7,
    	0x0004,
    	0x0400,
    	0x0814,

    12, 11,
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

    /*1, 32,
    	0x6680 // P_corm_thres Lock algorithms configuration */

    11, 80, /* set ADC level to -16 */
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

    4, 108,
            0,
            0,
            0,
            0,

    /* Imp noise config */
#ifdef CONFIG_DIB8000p_USE_IMPULSIVE_NOISE_COMPENSATION
    4, 141,
        219, /* inh, rst_burst & P_imp_Tpow*/
        219, /* P_imp_Taccpow */
        183, /* P_imp_Tparam */
        18, /* P_imp_sum_gamma_length & P_imp_Tdelta */
    1, 151,
        810, /* P_imp_satin_var */
    1, 156,
        113, /* P_imp_mkv_log_prior_0 */
    1, 161,
        113, /* P_imp_mkv_log_prior_1 */
    7, 166,
        113, /* P_imp_mkv_log_prior_2 */
        1164, /* P_imp_mkvl_alpha_3 */
        16382, /* P_imp_mkvl_1_alpha_3 */
        1459,  /* P_imp_mkvl_beta_3 */
        16363, /* P_imp_mkvl_1_beta_3 */
        113,   /* P_imp_mkv_log_prior_3 */
        64512, /* P_imp_mkv_log_prior_4 */
#endif

    1, 175,
    	0x0410,
    1, 179,
    	8192, // P_fft_nb_to_cut

    6, 181,
    	0x2800, // P_coff_corthres_ ( 2k 4k 8k ) 0x2800
    	0x2800,
    	0x2800,
    	0x2800, // P_coff_cpilthres_ ( 2k 4k 8k ) 0x2800
    	0x2800,
    	0x2800,

    2, 193,
    	0x0666, // P_pha3_thres
    	0x0000, // P_cti_use_cpe, P_cti_use_prog

    2, 205,
    	0x200f, // P_cspu_regul, P_cspu_win_cut
    	0x000f, // P_des_shift_work

    5, 215,
    	0x023d, // P_adp_regul_cnt
    	0x00a4, // P_adp_noise_cnt
    	0x00a4, // P_adp_regul_ext
    	0x7ff0, // P_adp_noise_ext
    	0x3ccc, // P_adp_fil

    1, 230,
    	0x0000, // P_2d_byp_ti_num

    1, 263,
    	0x800, //P_equal_thres_wgn

    1, 268,
    	(2 << 9) | 39, // P_equal_ctrl_synchro, P_equal_speedmode

    1, 270,
    	0x0001, // P_div_lock0_wait
    1, 285,
    	0x0020, //p_fec_
    1, 338,
    	(1 << 12) | // P_ctrl_corm_thres4pre_freq_inh=1
        (1 << 10) | // P_ctrl_pre_freq_mode_sat=1
        (0 <<  9) | // P_ctrl_pre_freq_inh=0
        (3 <<  5) | // P_ctrl_pre_freq_step=3
        (1 <<  0),  // P_pre_freq_win_len=1
    0,
};

static int dib8000p_reset(struct dibFrontend *fe)
{
    struct dib8000p_state *state = fe->demod_priv;
    uint16_t l = 0, r;
    const uint16_t *n;

    if ((state->revision = dib8000p_identify( demod_get_data_bus_client(fe) )) == 0)
        return DIB_RETURN_ERROR;

    dib8000p_set_power_mode(state, DIB8000P_POWER_ALL);

    dibx000_reset_i2c_master(&state->i2c_master);
    dib8000p_set_adc_state(state, DIBX000_ADC_OFF);

    /* restart all parts */
    dib8000p_write_word(state, 770, 0xffff);
    dib8000p_write_word(state, 771, 0xffff);
    dib8000p_write_word(state, 772, 0xfffc);
    dib8000p_write_word(state, 1280, 0x0045);
    dib8000p_write_word(state, 1281, 0x000c);

    dib8000p_write_word(state, 770, 0x0000);
    dib8000p_write_word(state, 771, 0x0000);
    dib8000p_write_word(state, 772, 0x0000);
    dib8000p_write_word(state, 898, 0x0004); // sad
    dib8000p_write_word(state, 1280, 0x0000);
    dib8000p_write_word(state, 1281, 0x0000);

    /* drives */
/*
    if (state->cfg.drives)
        dib8000p_write_word(state, 906, state->cfg.drives);
    else {
        dbgpl(&dib8000p_dbg, "FE %d : Using standard PAD-drive-settings, please adjust settings in config-struct to be optimal.",fe->id);
        dib8000p_write_word(state, 906, 0x2d98); // min drive SDRAM - not optimal - adjust
    }
*/

    dib8000p_reset_pll(state);
    dib8000p_set_bus_speed(fe, DATA_BUS_I2C, 1000);

    if (dib8000p_reset_gpio(state) != DIB_RETURN_SUCCESS)
        dbgpl_advanced(&dib8000p_dbg, "FE %d : GPIO reset was not successful.",fe->id);

    state->current_agc = NULL;

    // P_iqc_alpha_pha, P_iqc_alpha_amp, P_iqc_dcc_alpha, ...
    /* P_iqc_ca2 = 0; P_iqc_impnc_on = 0; P_iqc_mode = 0;*/
    if(state->cfg.pll->ifreq == 0)
        dib8000p_write_word(state, 40, 0x0755); /* P_iqc_corr_inh = 0 enable IQcorr block*/
    else
        dib8000p_write_word(state, 40, 0x1f55); /* P_iqc_corr_inh = 1 disable IQcorr block*/

    n = dib8000p_defaults;
    l = *n++;
    while (l) {
        r = *n++;
        do {
            dib8000p_write_word(state, r, *n++);
            r++;
        } while (--l);
        l = *n++;
    }

    if (state->cfg.forward_erronous_mpeg_packet)
		dib8000p_write_word(state, 299, 0x0022); // P_smo_mode, P_smo_fifo_flush, P_smo_pid_parse, P_smo_error_discard
    else
		dib8000p_write_word(state, 299, 0x0062); // P_smo_mode, P_smo_rs_discard, P_smo_fifo_flush, P_smo_pid_parse, P_smo_error_discard

    state->isdbt_cfg_loaded = 0;

    /* unforce divstr regardless whether i2c enumeration was done or not */
    dib8000p_write_word(state, 1285, dib8000p_read_word(state, 1285) & ~(1 << 1) );

    dib8000p_set_bandwidth(state, fe->current_bw);
    dib8000p_set_adc_state(state, DIBX000_SLOW_ADC_ON);
    dib8000p_sad_calib(state);
    dib8000p_set_power_mode(state, DIB8000P_POWER_INTERFACE_ONLY);

    return DIB_RETURN_SUCCESS;
}

#ifdef DEBUG_ESDRAM
static void dib8090p_write_sdram(struct dib8000p_state *state, uint32_t addr,uint32_t data, uint32_t size) {
    uint16_t reg_1293;

    dib8000p_write_word(state, 1296 ,addr);
    reg_1293 = dib8000p_read_word(state, 1293)& 0x9fff;

    if (size == 0) { 
        if ( (addr&0x1) == 1)
            reg_1293 |= (1<<14);
        else
            reg_1293 |= (1<<13);
        dib8000p_write_word(state, 1293, reg_1293);
    }

    dib8000p_write_word(state, 1300, data);
}

static uint16_t dib8090p_read_sdram (struct dib8000p_state *state, uint32_t addr) {
    uint16_t reg_1293;
    dib8000p_write_word(state, 1296 ,addr);
    reg_1293 = dib8000p_read_word(state, 1293)& 0x9fff;
    reg_1293 |= (1<<14);
    dib8000p_write_word(state, 1293, reg_1293);

   /* Dummy write to launch read access */
    dib8000p_write_word(state, 1301, 0);

   /* Read value */
    return dib8000p_read_word(state, 1301);
}
#endif

#if 0
static void dib8090p_sdram_bus_drive(struct dib8000p_state *state, uint8_t drive)
{
    uint16_t reg;
    drive &= 0x7;
    dbgpl(&dib8000p_dbg, "Set SDRAM BUS DRIVE to %d", drive);

    /* drive dram bus 0 */
    reg = dib8000p_read_word(state, 1803) &~(0x7<<8);
    reg|= (drive<<8);
    dib8000p_write_word(state, 1803, reg);

    /* drive host bus 1,2,3 */
    //dbgpl(&dib8000p_dbg, "reg 1804 = 0x%04x", dib8000p_write_word(state, 1804)); // For test
    reg = dib8000p_read_word(state, 1804) &~((0x7)|(0x7 << 6)|(0x7<<12));
    reg|= (drive<<12) | (drive<<6) | drive;
    dib8000p_write_word(state, 1804, reg);
    //dbgpl(&dib8000p_dbg, "reg 1804 = 0x%04x", dib8000p_write_word(state, 1804)); // For test

    /* drive dram bus 4,5 */
    reg = dib8000p_read_word(state, 1805) &~((0x7 << 2)|(0x7<<8));
    reg|= (drive<<8) | (drive<<2);
    dib8000p_write_word(state, 1805, reg);

    /* drive host bus 6,7,8 */
    reg = dib8000p_read_word(state, 1806) &~((0x7)|(0x7 << 6)|(0x7<<12));
    reg|= (drive<<12) | (drive<<6) | drive;
    dib8000p_write_word(state, 1806, reg);

    /* drive dram bus 9,10 */
    reg = dib8000p_read_word(state, 1807) &~((0x7 << 2)|(0x7<<8));
    reg|= (drive<<8) | (drive<<2);
    dib8000p_write_word(state, 1807, reg);

    /* drive host bus 11,12,13 */
    reg = dib8000p_read_word(state, 1808) &~((0x7)|(0x7 << 6)|(0x7<<12));
    reg|= (drive<<12) | (drive<<6) | drive;
    dib8000p_write_word(state, 1808, reg);

    /* drive dram bus 14,15 */
     reg = dib8000p_read_word(state, 1809) &~((0x7 << 2)|(0x7<<8));
    reg|= (drive<<8) | (drive<<2);
    dib8000p_write_word(state, 1809, reg);

    /* drive host bus 16,17,18 */
    reg = dib8000p_read_word(state, 1810) &~((0x7)|(0x7 << 6)|(0x7<<12));
    reg|= (drive<<12) | (drive<<6) | drive;
    dib8000p_write_word(state, 1810, reg);

    /* drive dram bus 19,20 */
    reg = dib8000p_read_word(state, 1811) &~((0x7 << 2)|(0x7<<8));
    reg|= (drive<<8) | (drive<<2);
    dib8000p_write_word(state, 1811, reg);

    /* drive host bus 21,22,23 */
    reg = dib8000p_read_word(state, 1812) &~((0x7)|(0x7 << 6)|(0x7<<12));
    reg|= (drive<<12) | (drive<<6) | drive;
    dib8000p_write_word(state, 1812, reg);

    /* drive dram bus 24,25 */
    reg = dib8000p_read_word(state, 1813) &~((0x7 << 2)|(0x7<<8));
    reg|= (drive<<8) | (drive<<2);
    dib8000p_write_word(state, 1813, reg);

    /* drive host bus 26,27,28 */
    reg = dib8000p_read_word(state, 1814) &~((0x7)|(0x7 << 6)|(0x7<<12));
    reg|= (drive<<12) | (drive<<6) | drive;
    dib8000p_write_word(state, 1814, reg);

    /* drive dram bus 29,30 */
    reg = dib8000p_read_word(state, 1815) &~((0x7 << 2)|(0x7<<8));
    reg|= (drive<<8) | (drive<<2);
    dib8000p_write_word(state, 1815, reg);

    /* drive host bus 31,32,33 */
    reg = dib8000p_read_word(state, 1816) &~((0x7)|(0x7 << 6)|(0x7<<12));
    reg|= (drive<<12) | (drive<<6) | drive;
    dib8000p_write_word(state, 1816, reg);

    /* drive dram bus 34,35 */
    reg = dib8000p_read_word(state, 1817) &~((0x7 << 2)|(0x7<<8));
    reg|= (drive<<8) | (drive<<2);
    dib8000p_write_word(state, 1817, reg);

    /* drive host bus 36 */
    reg = dib8000p_read_word(state, 1818) &~((0x7));
    reg|= drive;
    dib8000p_write_word(state, 1818, reg);
}
#endif

extern void dib8000p_init_sdram(struct dib8000p_state *state) {
    uint16_t reg = 0; 
    dbgpl(&dib8000p_dbg, "Init sdram");

    /* Force DRAM BANK IO DRIVE */
    //dib8090p_sdram_bus_drive(state, 1);

    reg = dib8000p_read_word(state, 274)&0xfff0;
    dib8000p_write_word(state, 274, reg | 0x7); /* P_dintlv_delay_ram = 7 because of MobileSdram */ 

    dib8000p_write_word(state, 1803, (7<<2));

    reg = dib8000p_read_word(state, 1280);
    dib8000p_write_word(state, 1280,  reg | (1<<2)); /* force restart P_restart_sdram */

    dib8000p_write_word(state, 1280,  reg); /* release restart P_restart_sdram */

#ifdef DEBUG_ESDRAM
    /* SDRAM R/W test */
    dib8000p_write_word(state, 1289, 2);
    reg = dib8000p_read_word(state, 1302);
    dib8000p_write_word(state, 1302, reg | (1<<4));
    dib8000p_write_word(state, 1302, reg);

    int i =0;
    for(i = 0 ; i < 10 ; i++)
        dib8090p_write_sdram(state, i, i, 1);

    for(i = 0 ; i < 10 ; i++)
        dbgpl_advanced(&dib8000p_dbg, "esdram addr %d = 0x%04x ", i , dib8090p_read_sdram(state, i));

    dib8000p_write_word(state, 1289, 0);
#endif

}

/* first_addr is 0x80 for DiB8000p */
int dib8000p_i2c_enumeration(struct dibDataBusHost *host, int no_of_demods, uint8_t default_addr, uint8_t first_addr)
{
    int k = 0;
    uint8_t new_addr = 0;
    struct dibDataBusClient client;

    data_bus_client_init(&client, &dibx000_i2c_data_bus_client, host);
    data_bus_client_set_speed(&client, 200);

    for (k = no_of_demods-1; k >= 0; k--) {
        /* designated i2c address */
        new_addr = first_addr + (k << 1);

        data_bus_client_set_device_id(&client, new_addr);
        //data_bus_client_write16(&client, 1287, 0x0003); /* TODO 1287 is no longer sram lead in and rdy in 8090P!!! */ 
        if (dib8000p_identify(&client) == 0) {
            //data_bus_client_write16(&client, 1287, 0x0003); /* TODO 1287 is no longer sram lead in and rdy 8090P !!! */ 
            data_bus_client_set_device_id(&client, default_addr);
            if (dib8000p_identify(&client) == 0) {
                dbgpl(&dib8000p_dbg, "#%d: not identified", k);
                return DIB_RETURN_ERROR;
            }
        }

        /* start diversity to pull_down div_str - just for i2c-enumeration */
        data_bus_client_write16(&client, 1286, (1 << 10) | (4 << 6));

        /* set new i2c address and force divstart */
        data_bus_client_write16(&client, 1285, (new_addr << 2) | 0x2);
        data_bus_client_set_device_id(&client, new_addr);
        dib8000p_identify(&client);
        dbgpl(&dib8000p_dbg, "IC %d initialized (to i2c_address 0x%x)", k, new_addr);
    }

    for (k = 0; k < no_of_demods; k++) {
        new_addr = first_addr | (k << 1);
        data_bus_client_set_device_id(&client, new_addr);

        /* unforce divstr */
        data_bus_client_write16(&client, 1285, new_addr << 2);

        /* deactivate div - it was just for i2c-enumeration */
        data_bus_client_write16(&client, 1286, 0);
    }

    return DIB_RETURN_SUCCESS;
}

struct dibDataBusHost * dib8000p_get_i2c_master(struct dibFrontend *fe, enum dibx000_i2c_interface intf, int gating)
{
    struct dib8000p_state *st = fe->demod_priv;
    return dibx000_get_i2c_adapter(&st->i2c_master, intf, gating);
}

void dib8000p_close_gate(struct dibFrontend *fe)
{
    struct dib8000p_state *state = fe->demod_priv;
    dib8000p_write_word(state, 1025, 1 << 8);
}

void dib8000p_enable_output_clock(struct dibFrontend *fe)
{
    struct dib8000p_state *state = fe->demod_priv;
    dib8000p_reset_pll(state);
}

/* i2c_master */
void dib8000p_reset_sync_pha(struct dibFrontend* fe, uint32_t loop_factor)
{
    struct dib8000p_state *state = fe->demod_priv;
    struct dibDataBusClient *client = demod_get_data_bus_client(state->fe);
    uint32_t i,j;
    uint16_t val;

    if (state->diversity_onoff == 0) {
        dbgpl(&dib8000p_dbg, "FE %d : Diversity is not active", fe->id);
        return;
    }
    for (i = 0; i < loop_factor; i++) {
        uint8_t div_errors[128];
	uint16_t error = 0;
        data_bus_client_read_attr( client, 549, DATA_BUS_ACCESS_MODE_NO_ADDRESS_INCREMENT | DATA_BUS_ACCESS_MODE_16BIT, div_errors, 128);
        for (j=0;j<128;j+=2) {
            error = ((div_errors[j]<<8) + (div_errors[j+1])) & 0x3fff;
            if (error)
                break;
        }
        if (error != 0) {
            dbgpl_advanced(&dib8000p_dbg, "errors : %d",div_errors[j]);
            val = dib8000p_read_word(state, 904);
            dib8000p_write_word(state, 904, (val & 0xfcff) | 0x100);
            dib8000p_write_word(state, 904, val);
            val = dib8000p_read_word(state, 770) & 0xbfff;
            dib8000p_write_word(state, 770, val| (1<<14));
            dib8000p_write_word(state, 770, val);
            dbgpl_advanced(&dib8000p_dbg, "reset");
            error = 0;
        }
    }
}

void dib8000p_set_low_thres(struct dibFrontend* fe)
{
    struct dib8000p_state *state = fe->demod_priv;
    state->saved_registers[0] = dib8000p_read_word(state, 183);
    state->saved_registers[1] = dib8000p_read_word(state, 182);
    state->saved_registers[2] = dib8000p_read_word(state, 181);
    state->saved_registers[3] = dib8000p_read_word(state, 180);
    state->saved_registers[4] = dib8000p_read_word(state, 188);
    state->saved_registers[5] = dib8000p_read_word(state, 9);
    dib8000p_write_word(state, 183, 0); //p_coff_corthres_2K = 0
    dib8000p_write_word(state, 182, 0); //p_coff_corthres_4K = 0
    dib8000p_write_word(state, 181, 0); //p_coff_corthres_8K = 0
    dib8000p_write_word(state, 180, 0x400); //default value exept p_coff_thres_lock = 0
    dib8000p_write_word(state, 188, 0x200); //default value except lmod4_thres_lock = 0
    dib8000p_write_word(state,   9, 0x14); //p_lock_div_mask = coff_lock + lmod4
    dib8000p_write_word(state, 36, dib8000p_read_word(state, 36) |1); //freeze pha shift
    dib8000p_write_word(state, 31, dib8000p_read_word(state, 31) |(1<<8)); //inh timf
}

void dib8000p_reset_thres(struct dibFrontend* fe)
{
    struct dib8000p_state *state = fe->demod_priv;
    dib8000p_write_word(state, 183, state->saved_registers[0]); //previous value
    dib8000p_write_word(state, 182, state->saved_registers[1]); //previous value
    dib8000p_write_word(state, 181, state->saved_registers[2]); //previous value
    dib8000p_write_word(state, 180, state->saved_registers[3]); //previous value
    dib8000p_write_word(state, 188, state->saved_registers[4]); //previous value
    dib8000p_write_word(state,   9, state->saved_registers[5]); //previous value
    dib8000p_write_word(state, 36, dib8000p_read_word(state, 36) & 0xfffe); //defreeze pha shift
    dib8000p_write_word(state, 31, dib8000p_read_word(state, 31) & 0xfeff); //deinh timf
}

void dib8000p_tune_reset_fes(struct dibFrontend *fe[] , uint32_t num_fe)
{
    int i;
    if (num_fe > 0) {
        dbgpl_advanced(&dib8000p_dbg ,"Trying to reset %d",num_fe);
        dib8000p_set_low_thres(fe[num_fe]);
        for (i=num_fe-1; i >= 0; i--) {
            if (fe[i]->input != NULL) {
                dbgpl_advanced(&dib8000p_dbg ,"fe%d",i);
                dib8000p_reset_sync_pha(fe[i], 1000);
            }
        }
        dib8000p_reset_thres(fe[num_fe]);
    }
}

static const struct dibDemodInfo dib8000p_info = {
    "DiBcom 8000P", // name
    MODE_ISDBT | FE_CAN_FAST_TUNE,// caps
    32,              // pid_filter_count

    { // ops
    dib8000p_reset,

    dib8000p_wakeup,        // wakeup
    dib8000p_sleep,         // sleep

    dib8000p_agc_startup,   // agc_startup
    dib8000p_tune,          // tune

    NULL,  // autosearch_start
    NULL, // autosearch_irq
    dib8000p_get_channel,       // get_channel

    dib8000p_set_diversity_in,      // set_diversity_in
    NULL, // set_output_mode
    dib8000p_pid_filter_ctrl,       // pid_filter_ctrl
    dib8000p_pid_filter,            // pid_filter

    dib8000p_set_wbd_ref,         // set_wdb_ref
    dib8000p_get_wbd_ref,         // set_wdb_ref

    dib8000p_set_gpio,            // set_gpio

    dib8000p_get_signal_status,   // get_signal_status

    dib8000p_ctrl_timf,           // ctrl_timf

#ifdef DIBCOM_GENERIC_MONITORING
    dib8000p_generic_monitoring,   // get_monitoring
#else
    NULL,
#endif
    dib8000p_release, //release
    }
};
/* wakeup function actually */

struct dibFrontend * dib8000p_register(struct dibFrontend *fe, struct dibDataBusHost *i2c_adap, uint8_t i2c_addr, const struct dib8000p_config *cfg)
{
    struct dib8000p_state *st = MemAlloc(sizeof(struct dib8000p_state));
    struct dibDataBusClient *client;
    if (st == NULL)
        return NULL;
    DibZeroMemory(st,sizeof(struct dib8000p_state));
    memcpy(&st->cfg, cfg, sizeof(struct dib8000p_config));

    st->fe = fe;
    frontend_register_demod(fe, &st->info, &dib8000p_info, st);
    fe->release_compat_fe &= ~COMPONENT_LAYER_RELEASE;
    st->timf_default = st->cfg.pll->timf;
    st->diversity_onoff = 0;

    client = demod_get_data_bus_client(fe);

    if (data_bus_host_type(i2c_adap) == DATA_BUS_I2C) {
        data_bus_client_init(client, &dibx000_i2c_data_bus_client, i2c_adap);
        data_bus_client_set_speed(client, 340);
    }
    data_bus_client_set_device_id(client, i2c_addr);

    dibx000_init_i2c_master(&st->i2c_master, DIB8000, demod_get_data_bus_client(fe));

    return fe;
}

const struct dibDebugObject dib8000p_dbg = {
    DEBUG_DEMOD,
    "DiB8000p"
};
