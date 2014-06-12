#include "dib7000p_priv.h"

void dib7000p_set_adc_state(struct dib7000p_state *state, enum dibx000_adc_states no)
{
    uint16_t reg, reg_908 = 0, reg_909 = 0;

    if(state->version != SOC7090) {
        reg_908 = dib7000p_read_word(state, 908);
        reg_909 = dib7000p_read_word(state, 909);
    }

    switch (no) {
        case DIBX000_SLOW_ADC_ON:
#ifdef CONFIG_DEMOD_DIB7000P_DEBUG
            dbgpl(&dib7000p_dbg, "Slow ADC ON");
#endif

            if(state->version == SOC7090) {
                reg = dib7000p_read_word(state, 1925);

                dib7000p_write_word(state, 1925, reg | (1<<4) | (1<<2)); /* en_slowAdc = 1 & reset_sladc = 1 */

                reg = dib7000p_read_word(state, 1925); /* read acces to make it works... strange ... */
                DibMSleep(1);
                dib7000p_write_word(state, 1925, reg & ~(1<<4)); /* en_slowAdc = 1 & reset_sladc = 0 */

                reg = dib7000p_read_word(state, 72) & ~((0x3 << 14) | (0x3 << 12));
                dib7000p_write_word(state, 72, reg | (1 << 14) | (3 << 12) | 524); /* ref = Vin1 => Vbg ; sel = Vin0 or Vin3 ; (Vin2 = Vcm) */

#ifdef CONFIG_DEMOD_DIB7000P_DEBUG
                /* debug */
                reg = dib7000p_read_word(state, 1925);
                dbgpl(&dib7000p_dbg, "en_slowAdc  = %d reset_sladc =%d P_adcValidEdge =%d P_clk_div =%d", (reg>>2)&0x1, (reg>>4)&0x1,  (reg>>5)&0x1, (reg>>6)&0x3);
                reg = dib7000p_read_word(state, 72);
                dbgpl(&dib7000p_dbg, "P_sad_ref_sel  = %d P_sad_sel =%d P_sad_freq_15k =%d", (reg>>14)&0x3, (reg>>12)&0x3,  reg &0xfff);

                reg = dib7000p_read_word(state, 73);
                dbgpl(&dib7000p_dbg, "P_sad_ext_calib = %d P_sad_calibration =%d", (reg>>1)&0x1, reg&0x1);

                reg = dib7000p_read_word(state, 74);
                dbgpl(&dib7000p_dbg, "P_sad_calib_value = %d", reg>>12);

                reg = dib7000p_read_word(state, 106);
                dbgpl(&dib7000p_dbg, "reg 106 = 0x%04x", reg);
#endif


            } else {
                reg_909 |= (1 << 1) | (1 << 0);
                dib7000p_write_word(state, 909, reg_909);
                reg_909 &= ~(1 << 1);
            }
            break;

        case DIBX000_SLOW_ADC_OFF:
            dbgpl(&dib7000p_dbg, "Slow ADC OFF");
            if(state->version == SOC7090) {
                reg = dib7000p_read_word(state, 1925);
                dib7000p_write_word(state, 1925, (reg & ~(1<<2)) | (1<<4)); /* reset_sladc = 1 en_slowAdc = 0 */

                /* debug */
                reg = dib7000p_read_word(state, 1925);

#ifdef CONFIG_DEMOD_DIB7000P_DEBUG
                dbgpl(&dib7000p_dbg, "en_slowAdc  = %d reset_sladc =%d P_adcValidEdge =%d P_clk_div =%d", (reg>>2)&0x1, (reg>>4)&0x1,  (reg>>5)&0x1, (reg>>6)&0x3);
                reg = dib7000p_read_word(state, 72);
                dbgpl(&dib7000p_dbg, "P_sad_ref_sel  = %d P_sad_sel =%d P_sad_freq_15k =%d", (reg>>14)&0x3, (reg>>12)&0x3,  reg &0xfff);
                reg = dib7000p_read_word(state, 73);
                dbgpl(&dib7000p_dbg, "P_sad_ext_calib = %d P_sad_calibration =%d", (reg>>1)&0x1, reg&0x1);
                reg = dib7000p_read_word(state, 74);
                dbgpl(&dib7000p_dbg, "P_sad_calib_value = %d", reg>>12);
#endif

            } else
                reg_909 |=  (1 << 1) | (1 << 0);
            break;

        case DIBX000_ADC_ON:
            reg_908 &= 0x0fff;
            reg_909 &= 0x0003;
            break;

        case DIBX000_ADC_OFF: // leave the VBG voltage on
            reg_908 |= (1 << 14) | (1 << 13) | (1 << 12);
            reg_909 |= (1 << 5) | (1 << 4) | (1 << 3) | (1 << 2);
            break;

        case DIBX000_VBG_ENABLE:
            reg_908 &= ~(1 << 15);
            break;

        case DIBX000_VBG_DISABLE:
            reg_908 |= (1 << 15);
            break;

        default:
            break;
    }
    reg_909 |= (state->cfg.disable_sample_and_hold & 1) << 4;
    reg_908 |= (state->cfg.enable_current_mirror & 1) << 7;
    // dbgpl(&dib7000p_dbg, "908: %x, 909: %x\n", reg_908, reg_909);

    if(state->version != SOC7090) {
        dib7000p_write_word(state, 908, reg_908);
        dib7000p_write_word(state, 909, reg_909);
    }
}

/* this function sets up a filter to eliminate harmonics of the xtal */
static void dib7000p_spur_protect(struct dib7000p_state *state, uint32_t rf_khz, uint32_t bw)
{
    static int16_t notch[]={16143, 14402, 12238, 9713, 6902, 3888, 759, -2392};
    static uint8_t sine [] ={0, 2, 3, 5, 6, 8, 9, 11, 13, 14, 16, 17, 19, 20, 22,
    24, 25, 27, 28, 30, 31, 33, 34, 36, 38, 39, 41, 42, 44, 45, 47, 48, 50, 51,
    53, 55, 56, 58, 59, 61, 62, 64, 65, 67, 68, 70, 71, 73, 74, 76, 77, 79, 80,
    82, 83, 85, 86, 88, 89, 91, 92, 94, 95, 97, 98, 99, 101, 102, 104, 105,
    107, 108, 109, 111, 112, 114, 115, 117, 118, 119, 121, 122, 123, 125, 126,
    128, 129, 130, 132, 133, 134, 136, 137, 138, 140, 141, 142, 144, 145, 146,
    147, 149, 150, 151, 152, 154, 155, 156, 157, 159, 160, 161, 162, 164, 165,
    166, 167, 168, 170, 171, 172, 173, 174, 175, 177, 178, 179, 180, 181, 182,
    183, 184, 185, 186, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198,
    199, 200, 201, 202, 203, 204, 205, 206, 207, 207, 208, 209, 210, 211, 212,
    213, 214, 215, 215, 216, 217, 218, 219, 220, 220, 221, 222, 223, 224, 224,
    225, 226, 227, 227, 228, 229, 229, 230, 231, 231, 232, 233, 233, 234, 235,
    235, 236, 237, 237, 238, 238, 239, 239, 240, 241, 241, 242, 242, 243, 243,
    244, 244, 245, 245, 245, 246, 246, 247, 247, 248, 248, 248, 249, 249, 249,
    250, 250, 250, 251, 251, 251, 252, 252, 252, 252, 253, 253, 253, 253, 254,
    254, 254, 254, 254, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255};

    uint32_t xtal = state->cfg.bw->xtal_hz / 1000;
    int f_rel = ( (rf_khz + xtal/2) / xtal) * xtal - rf_khz;
    int k;
    int coef_re[8],coef_im[8];
    int bw_khz = bw;
    uint32_t pha;

    dbgpl(&dib7000p_dbg, "relative position of the Spur: %dk (RF: %dk, XTAL: %dk)", f_rel, rf_khz, xtal);


    if (f_rel < -bw_khz/2 || f_rel > bw_khz/2)
    	return;

    bw_khz /= 100;

    dib7000p_write_word(state, 142 ,0x0610);

    for (k = 0; k < 8; k++) {
        pha = ((f_rel * (k+1) * 112 * 80/bw_khz) /1000) & 0x3ff;

        if (pha==0) {
            coef_re[k] = 256;
            coef_im[k] = 0;
        } else if(pha < 256) {
            coef_re[k] = sine[256-(pha&0xff)];
            coef_im[k] = sine[pha&0xff];
        } else if (pha == 256) {
            coef_re[k] = 0;
            coef_im[k] = 256;
        } else if (pha < 512) {
            coef_re[k] = -sine[pha&0xff];
            coef_im[k] = sine[256 - (pha&0xff)];
        } else if (pha == 512) {
            coef_re[k] = -256;
            coef_im[k] = 0;
        } else if (pha < 768) {
            coef_re[k] = -sine[256-(pha&0xff)];
            coef_im[k] = -sine[pha&0xff];
        } else if (pha == 768) {
            coef_re[k] = 0;
            coef_im[k] = -256;
        } else {
            coef_re[k] = sine[pha&0xff];
            coef_im[k] = -sine[256 - (pha&0xff)];
        }

        coef_re[k] *= notch[k];
        coef_re[k] += (1<<14);
        if (coef_re[k] >= (1<<24))
            coef_re[k]  = (1<<24) - 1;
        coef_re[k] /= (1<<15);

        coef_im[k] *= notch[k];
        coef_im[k] += (1<<14);
        if (coef_im[k] >= (1<<24))
            coef_im[k]  = (1<<24)-1;
        coef_im[k] /= (1<<15);

        dbgpl(&dib7000p_dbg, "PALF COEF: %d re: %d im: %d", k, coef_re[k], coef_im[k]);

        dib7000p_write_word(state, 143, (0 << 14) | (k << 10) | (coef_re[k] & 0x3ff));
        dib7000p_write_word(state, 144, coef_im[k] & 0x3ff);
        dib7000p_write_word(state, 143, (1 << 14) | (k << 10) | (coef_re[k] & 0x3ff));
    }
    dib7000p_write_word(state,143 ,0);
}

void dib7000p_update_timf(struct dib7000p_state *state)
{
    uint32_t timf = dib7000p_read32(state, 427);

    state->timf = timf * 160 / (state->fe->current_bw / 50);
    dib7000p_write_word(state, 23, (uint16_t) (timf >> 16));
    dib7000p_write_word(state, 24, (uint16_t) (timf & 0xffff));
    dbgpl(&dib7000p_dbg, "updated timf_frequency: %d (default: %d)",state->timf, state->cfg.bw->timf);
}

static void dib7000p_pll_clk_cfg(struct dib7000p_state *state)
{
    uint16_t tmp = 0;
    tmp = dib7000p_read_word(state, 903);
    dib7000p_write_word(state, 903, (tmp | 0x1));   //pwr-up pll
    tmp = dib7000p_read_word(state, 900);
    dib7000p_write_word(state, 900, (tmp & 0x7fff) | (1 << 6));     //use High freq clock
}

void dib7000p_restart_agc(struct dibFrontend *fe)
{
    struct dib7000p_state *state = fe->demod_priv;

    dib7000p_write_word(state, 770, (1 << 11) | (1 << 9));
    dib7000p_write_word(state, 770, 0x0000);
}

static int dib7000p_update_lna(struct dibFrontend *fe)
{
    struct dib7000p_state *state = fe->demod_priv;
    uint16_t dyn_gain;

    // when there is no LNA to program return immediatly
    if (state->cfg.update_lna) {
        // read dyn_gain here (because it is demod-dependent and not tuner)
        dyn_gain = dib7000p_read_word(state, 394);
        if (state->cfg.update_lna(fe, dyn_gain)) { // LNA has changed
            dib7000p_restart_agc(fe);
            return 1;
        }
    }
    return 0;
}

static int dib7000p_set_agc_config(struct dib7000p_state *state, uint8_t band)
{
    const struct dibx000_agc_config *agc = NULL;
    int i;

    if (state->current_band == band && state->current_agc != NULL)
        return DIB_RETURN_SUCCESS;

    state->current_band = band;

    for (i = 0; i < state->cfg.agc_config_count; i++) {
        if (state->cfg.agc[i].band_caps & band) {
            agc = &state->cfg.agc[i];
            break;
        }
    }

    if (agc == NULL) {
        dbgpl(&dib7000p_dbg, "no valid AGC configuration found for band 0x%02x",band);
	return DIB_RETURN_ERROR;
    }

    state->current_agc = agc;

    /* AGC */
    dib7000p_write_word(state, 75 ,  agc->setup );
    dib7000p_write_word(state, 76 ,  agc->inv_gain );
    dib7000p_write_word(state, 77 ,  agc->time_stabiliz );
    dib7000p_write_word(state, 100, (agc->alpha_level << 12) | agc->thlock);

    // Demod AGC loop configuration
    dib7000p_write_word(state, 101, (agc->alpha_mant << 5) | agc->alpha_exp);
    dib7000p_write_word(state, 102, (agc->beta_mant << 6)  | agc->beta_exp);

    /* AGC continued */
    dbgpl(&dib7000p_dbg, "WBD: ref: %d, sel: %d, active: %d, alpha: %d",
    state->wbd_ref != 0 ? state->wbd_ref : agc->wbd_ref, agc->wbd_sel, !agc->perform_agc_softsplit, agc->wbd_sel);

    if (state->wbd_ref != 0)
        dib7000p_write_word(state, 105, (agc->wbd_inv << 12) | state->wbd_ref);
    else
        dib7000p_write_word(state, 105, (agc->wbd_inv << 12) | agc->wbd_ref);

    dib7000p_write_word(state, 106, (agc->wbd_sel << 13) | (agc->wbd_alpha << 9) | (agc->perform_agc_softsplit << 8));

    dib7000p_write_word(state, 107,  agc->agc1_max);
    dib7000p_write_word(state, 108,  agc->agc1_min);
    dib7000p_write_word(state, 109,  agc->agc2_max);
    dib7000p_write_word(state, 110,  agc->agc2_min);
    dib7000p_write_word(state, 111, (agc->agc1_pt1    << 8) | agc->agc1_pt2);
    dib7000p_write_word(state, 112,  agc->agc1_pt3);
    dib7000p_write_word(state, 113, (agc->agc1_slope1 << 8) | agc->agc1_slope2);
    dib7000p_write_word(state, 114, (agc->agc2_pt1    << 8) | agc->agc2_pt2);
    dib7000p_write_word(state, 115, (agc->agc2_slope1 << 8) | agc->agc2_slope2);

#if 0
    dbgpl(&dib7000p_dbg, "reg_107 = 0x%04x ", dib7000p_read_word(state, 107));
    dbgpl(&dib7000p_dbg, "reg_108 = 0x%04x ", dib7000p_read_word(state, 108));
    dbgpl(&dib7000p_dbg, "reg_109 = 0x%04x ", dib7000p_read_word(state, 109));
    dbgpl(&dib7000p_dbg, "reg_110 = 0x%04x ", dib7000p_read_word(state, 110));
    dbgpl(&dib7000p_dbg, "reg_111 = 0x%04x ", dib7000p_read_word(state, 111));
    dbgpl(&dib7000p_dbg, "reg_112 = 0x%04x ", dib7000p_read_word(state, 112));
    dbgpl(&dib7000p_dbg, "reg_113 = 0x%04x ", dib7000p_read_word(state, 113));
    dbgpl(&dib7000p_dbg, "reg_114 = 0x%04x ", dib7000p_read_word(state, 114));
    dbgpl(&dib7000p_dbg, "reg_115 = 0x%04x ", dib7000p_read_word(state, 115));
#endif

    return DIB_RETURN_SUCCESS;
}

int dib7000p_set_agc1_min(struct dibFrontend *fe, uint16_t v) {
    struct dib7000p_state *state = fe->demod_priv;
    return dib7000p_write_word(state, 108,  v);
}

int dib7000p_agc_startup(struct dibFrontend *fe, struct dibChannel *ch)
{
    struct dib7000p_state *state = fe->demod_priv;
    enum frontend_tune_state *tune_state = &fe->tune_state;
    int ret = 10;
    uint8_t agc_split;
    uint32_t upd_demod_gain_period = 0x1000;
    uint16_t reg;

    switch (*tune_state) {
        case CT_AGC_START:
            // set power-up level: interf+analog+AGC
            dib7000p_set_power_mode(state, DIB7000P_POWER_ALL);

            if(state->version == SOC7090) {
                reg = dib7000p_read_word(state, 0x79b)&0xff00;
                dib7000p_write_word(state, 0x79a, upd_demod_gain_period & 0xFFFF); /* lsb */
                dib7000p_write_word(state, 0x79b, reg | (1<<14) | ((upd_demod_gain_period >> 16) & 0xFF)); // bit 14 = enDemodGain

                /* enable adc i & q */
                reg = dib7000p_read_word(state, 0x780);
                dib7000p_write_word(state, 0x780, (reg | (0x3))&(~(1<<7)));
            } else {
                dib7000p_set_adc_state(state, DIBX000_ADC_ON);
                dib7000p_pll_clk_cfg(state);
            }

            if (dib7000p_set_agc_config(state, (uint8_t)(BAND_OF_FREQUENCY(ch->RF_kHz))) != DIB_RETURN_SUCCESS) {
                fe->tune_state = CT_AGC_STOP;
                fe->status = FE_STATUS_TUNE_FAILED;
                break;
            }

            dib7000p_set_frequency_offset(state, ch);

            ret = 70;
            *tune_state = CT_AGC_STEP_0;
            break;

        case CT_AGC_STEP_0:
            // AGC initialization
            if (state->cfg.agc_control)
                state->cfg.agc_control(fe, 1);

            dib7000p_write_word(state, 78, 32768);
            if (!state->current_agc->perform_agc_softsplit) {
                /* we are using the wbd - so slow AGC startup */
                /* force 0 split on WBD and restart AGC */
                dib7000p_write_word(state, 106, (state->current_agc->wbd_sel << 13) | (state->current_agc->wbd_alpha << 9) | (1 << 8));
                *tune_state = CT_AGC_STEP_1;
                ret = 50;
            } else {
                /* default AGC startup */
                fe->tune_state = CT_AGC_STEP_3;
                /* wait AGC rough lock time */
                ret = 70;
            }

            dib7000p_restart_agc(fe);
            break;

        case CT_AGC_STEP_1: /* fast split search path after 5sec */
            dib7000p_write_word(state,  75, state->current_agc->setup | (1 << 4)); /* freeze AGC loop */
            dib7000p_write_word(state, 106, (state->current_agc->wbd_sel << 13) | (2 << 9) | (0 << 8)); /* fast split search 0.25kHz */
            ret = 360;
            *tune_state = CT_AGC_STEP_2;
        break;

        case CT_AGC_STEP_2: /* split search ended */
            agc_split = (uint8_t)dib7000p_read_word(state, 396); /* store the split value for the next time */
            dib7000p_write_word(state, 78, dib7000p_read_word(state, 394)); /* set AGC gain start value */

            dib7000p_write_word(state, 75,  state->current_agc->setup);   /* std AGC loop */
            dib7000p_write_word(state, 106, (state->current_agc->wbd_sel << 13) | (state->current_agc->wbd_alpha << 9) | agc_split);

            dib7000p_restart_agc(fe);

            dbgpl(&dib7000p_dbg, "SPLIT %p: %d", fe, agc_split);

            ret = 70;
            *tune_state = CT_AGC_STEP_3;
            break;

        case CT_AGC_STEP_3: /* LNA startup */
            if (dib7000p_update_lna(fe)) {
                ret = 120;
                dib7000p_restart_agc(fe);
            } else
                ret = 10;

            *tune_state = CT_AGC_STEP_4;
            break;

        case CT_AGC_STEP_4:
            if (state->cfg.agc_control)
                state->cfg.agc_control(fe, 0);
            *tune_state = CT_AGC_STOP;
        default: /* fall through */
            ret = FE_CALLBACK_TIME_NEVER;
            break;
    }
    return ret;
}

/* make sure there is no garbage in struct dibChannel */
static void dib7000p_set_channel(struct dib7000p_state *state, struct dibChannel *ch, uint8_t seq)
{
    uint16_t value, est[4];

    if ((state->cfg.invert_spectrum) || (ch->u.dvbt.spectrum_inversion == 1)) {
        dbgpl(&dib7000p_dbg, "fe%i invert spectrum", state->fe->id);
        dib7000p_write_word(state, 34, dib7000p_read_word(state, 34) | (1 << 15));
    }

    dib7000p_set_bandwidth(state, ch->bandwidth_kHz);

    /* nfft, guard, qam, alpha */
    value = 0;
    switch (ch->u.dvbt.nfft) {
        case FFT_2K: value |= (0 << 7); break;
        case FFT_4K: value |= (2 << 7); break;
        default:
        case FFT_8K: value |= (1 << 7); break;
    }
    switch (ch->u.dvbt.guard) {
        case GUARD_INTERVAL_1_32: value |= (0 << 5); break;
        case GUARD_INTERVAL_1_16: value |= (1 << 5); break;
        case GUARD_INTERVAL_1_4:  value |= (3 << 5); break;
        default:
        case GUARD_INTERVAL_1_8:  value |= (2 << 5); break;
    }
    switch (ch->u.dvbt.constellation) {
        case QAM_QPSK:  value |= (0 << 3); break;
        case QAM_16QAM: value |= (1 << 3); break;
        default:
        case QAM_64QAM: value |= (2 << 3); break;
    }
    switch (ch->u.dvbt.alpha) {
        case VIT_ALPHA_2: value |= 2; break;
        case VIT_ALPHA_4: value |= 4; break;
        default:
        case VIT_ALPHA_1: value |= 1; break;
    }
    dib7000p_write_word(state, 0, value);
    dib7000p_write_word(state, 5, (seq << 4) | 1); /* do not force tps, search list 0 */

    /* P_dintl_native, P_dintlv_inv, P_hrch, P_code_rate, P_select_hp */
    value = 0;
    if (ch->u.dvbt.intlv_native != 0)
        value |= (1 << 6);
    if (ch->u.dvbt.hrch == 1)
        value |= (1 << 4);
    if (ch->u.dvbt.select_hp == 1)
        value |= 1;
    switch ((ch->u.dvbt.hrch == 0 || ch->u.dvbt.select_hp == 1) ? ch->u.dvbt.code_rate_hp : ch->u.dvbt.code_rate_lp) {
        case VIT_CODERATE_2_3: value |= (2 << 1); break;
        case VIT_CODERATE_3_4: value |= (3 << 1); break;
        case VIT_CODERATE_5_6: value |= (5 << 1); break;
        case VIT_CODERATE_7_8: value |= (7 << 1); break;
        default:
        case VIT_CODERATE_1_2: value |= (1 << 1); break;
    }
    dib7000p_write_word(state, 208, value);

    /* offset loop parameters */
    dib7000p_write_word(state, 26, 0x6680); // timf(6xxx)
    dib7000p_write_word(state, 32, 0x0003); // pha_off_max(xxx3)
    dib7000p_write_word(state, 29, 0x1273); // isi
    dib7000p_write_word(state, 33, 0x0005); // sfreq(xxx5)

    /* P_dvsy_sync_wait */
    switch (ch->u.dvbt.nfft) {
        case FFT_8K: value = 256; break;
        case FFT_4K: value = 128; break;
        case FFT_2K:
        default: value = 64; break;
    }
    switch (ch->u.dvbt.guard) {
        case GUARD_INTERVAL_1_16: value *= 2; break;
        case GUARD_INTERVAL_1_8:  value *= 4; break;
        case GUARD_INTERVAL_1_4:  value *= 8; break;
        default:
        case GUARD_INTERVAL_1_32: value *= 1; break;
    }
    if (state->cfg.diversity_delay == 0)
        state->div_sync_wait = (value * 3) / 2 + 48; // add 50% SFN margin + compensate for one DVSY-fifo
    else
        state->div_sync_wait = (value * 3) / 2 + state->cfg.diversity_delay; // add 50% SFN margin + compensate for one DVSY-fifo

    /* deactive the possibility of diversity reception if extended interleaver */
    state->div_force_off = !ch->u.dvbt.intlv_native && ch->u.dvbt.nfft != FFT_8K;
    dib7000p_set_diversity_in(state->fe, state->div_state);

    /* channel estimation fine configuration */
    switch (ch->u.dvbt.constellation) {
        case QAM_64QAM:
            est[0] = 0x0148;       /* P_adp_regul_cnt 0.04 */
            est[1] = 0xfff0;       /* P_adp_noise_cnt -0.002 */
            est[2] = 0x00a4;       /* P_adp_regul_ext 0.02 */
            est[3] = 0xfff8;       /* P_adp_noise_ext -0.001 */
            break;
    	case QAM_16QAM:
            est[0] = 0x023d;       /* P_adp_regul_cnt 0.07 */
            est[1] = 0xffdf;       /* P_adp_noise_cnt -0.004 */
            est[2] = 0x00a4;       /* P_adp_regul_ext 0.02 */
            est[3] = 0xfff0;       /* P_adp_noise_ext -0.002 */
            break;
    	default:
            est[0] = 0x099a;       /* P_adp_regul_cnt 0.3 */
            est[1] = 0xffae;       /* P_adp_noise_cnt -0.01 */
            est[2] = 0x0333;       /* P_adp_regul_ext 0.1 */
            est[3] = 0xfff8;       /* P_adp_noise_ext -0.002 */
            break;
    }

    for (value = 0; value < 4; value++)
        dib7000p_write_word(state, 187 + value, est[value]);
}

int dib7000p_autosearch_is_irq(struct dibFrontend *fe)
{
    uint8_t b[2];
    if (data_bus_client_read(demod_get_data_bus_client(fe), 1284, b, 2) != DIB_RETURN_SUCCESS)
        return 1;

	if (b[1] & 0x1) // failed
		return 1;

	if (b[1] & 0x2) // succeeded
		return 2;

    return 0; // still pending
}

#if 0
#warning should not be enabled
static void dump_reg(struct dib7000p_state *st)
{
    int i;
    uint16_t v;

    for ( i = 0; i <= 1288; i++) {
        v = dib7000p_read_word(st, i);
        dbgpl(&dib7000p_dbg, "%04d = %04x (%5d)", i, v, v);
    }
}
#endif

/* scan function */
int dib7000p_autosearch_start(struct dibFrontend *fe, struct dibChannel *ch)
{
    struct dib7000p_state *state = fe->demod_priv;
    struct dibChannel schan;
    uint32_t value, factor;
    uint32_t internal = dib7000p_get_internal_freq(state);

    INIT_CHANNEL(&schan, STANDARD_DVBT);
    schan = *ch;
    schan.u.dvbt.constellation = QAM_64QAM;
    schan.u.dvbt.guard         = GUARD_INTERVAL_1_32;
    schan.u.dvbt.nfft          = FFT_8K;
    schan.u.dvbt.alpha         = VIT_ALPHA_1;
    schan.u.dvbt.select_hp     = 1;
    schan.u.dvbt.code_rate_hp  = VIT_CODERATE_2_3;
    schan.u.dvbt.code_rate_lp  = VIT_CODERATE_3_4;
    schan.u.dvbt.hrch          = 0;
    schan.u.dvbt.intlv_native  = 1;

    dib7000p_set_channel(state, &schan, 7);

    factor = ch->bandwidth_kHz;
    if (factor >= 5000)
    	factor = 2;
    else
    	factor = 6;

    // always use the setting for 8MHz here lock_time for 7,6 MHz are longer
    value = 30 * internal * factor;
    dib7000p_write_word(state, 6,  (uint16_t) ((value >> 16) & 0xffff)); // lock0 wait time
    dib7000p_write_word(state, 7,  (uint16_t)  (value        & 0xffff)); // lock0 wait time
    value = 100 * internal * factor;
    dib7000p_write_word(state, 8,  (uint16_t) ((value >> 16) & 0xffff)); // lock1 wait time
    dib7000p_write_word(state, 9,  (uint16_t)  (value        & 0xffff)); // lock1 wait time
    value = 500 * internal * factor;
    dib7000p_write_word(state, 10, (uint16_t) ((value >> 16) & 0xffff)); // lock2 wait time
    dib7000p_write_word(state, 11, (uint16_t)  (value        & 0xffff)); // lock2 wait time

    value = dib7000p_read_word(state, 0);
    dib7000p_write_word(state, 0, (uint16_t) ((1 << 9) | value));
    dib7000p_read_word(state, 1284);
    dib7000p_write_word(state, 0, (uint16_t) value);

    return DIB_RETURN_SUCCESS;
}

#if 1
void dib7000p_dump_tuner_dig(struct dibFrontend *fe)
{
    struct dibDataBusClient * client = demod_get_data_bus_client(fe);
    struct dib7000p_state *state = fe->demod_priv;
    int reg;

    for(reg = 1920 ; reg < 1948 ; reg ++)
        dbgpl(NULL, "Reg %d = 0x%04x", reg, data_bus_client_read16(client, reg));

    for(reg = 1984 ; reg < 1987 ; reg ++)
        dbgpl(NULL, "Reg %d = 0x%04x", reg, data_bus_client_read16(client, reg));


    dbgpl(NULL, "stepBbGain                  = %d", (dib7000p_read_word(state, 0x786)       )         );
    dbgpl(NULL, "startGlobalGainBb           = %d", (dib7000p_read_word(state, 0x787)       ) &0x1ff  );
    dbgpl(NULL, "incrBbGain                  = %d", (dib7000p_read_word(state, 0x787) >>  9 ) &0x7f   );
    dbgpl(NULL, "bb1min                      = %d", (dib7000p_read_word(state, 0x788)       ) &0x1ff  );
    dbgpl(NULL, "bb1Scale                    = %d", (dib7000p_read_word(state, 0x788) >>  9 ) &0x3f   );
    dbgpl(NULL, "bb1max                      = %d", (dib7000p_read_word(state, 0x789)       ) &0x1ff  );
    dbgpl(NULL, "bb1Offs                     = %d", (dib7000p_read_word(state, 0x789) >>  9 ) &0x3f   );
    dbgpl(NULL, "bb2min                      = %d", (dib7000p_read_word(state, 0x78a)       ) &0x1ff  );
    dbgpl(NULL, "bb2Scale                    = %d", (dib7000p_read_word(state, 0x78a) >>  9 ) &0x3f   );
    dbgpl(NULL, "bb2max                      = %d", (dib7000p_read_word(state, 0x78b)       ) &0x1ff  );
    dbgpl(NULL, "bb2Offs                     = %d", (dib7000p_read_word(state, 0x78b) >>  9 ) &0x3f   );
    dbgpl(NULL, "targetGlobalGainBb          = %d", (dib7000p_read_word(state, 0x78c)       ) &0x1ff  );
    dbgpl(NULL, "enBbRamp                    = %d", (dib7000p_read_word(state, 0x78c) >>  9 ) &0x1    );
    dbgpl(NULL, "en_mux_bb1                  = %d", (dib7000p_read_word(state, 0x78c) >> 10 ) &0x1    );
    dbgpl(NULL, "currentGlobalGainRf         = %d", (dib7000p_read_word(state, 0x78d) >>  0 ) &0xffff );
    dbgpl(NULL, "currentGlobalGainBb         = %d", (dib7000p_read_word(state, 0x78e) >>  0 ) &0xffff );
    dbgpl(NULL, "stepRfGain                  = %d", (dib7000p_read_word(state, 0x78f) >>  0 ) &0xffff );
    dbgpl(NULL, "startGlobalGainRf           = %d", (dib7000p_read_word(state, 0x790) >>  0 ) &0x3ff  );
    dbgpl(NULL, "incrRfGain_5_0              = %d", (dib7000p_read_word(state, 0x790) >> 10 ) &0x3f   );
    dbgpl(NULL, "lna1min                     = %d", (dib7000p_read_word(state, 0x791) >>  0 ) &0x3ff  );
    dbgpl(NULL, "lna1Scale                   = %d", (dib7000p_read_word(state, 0x791) >> 10 ) &0x3f   );
    dbgpl(NULL, "lna1max                     = %d", (dib7000p_read_word(state, 0x792) >>  0 ) &0x3ff  );
    dbgpl(NULL, "lna1Offs                    = %d", (dib7000p_read_word(state, 0x792) >> 10 ) &0x3f   );
    dbgpl(NULL, "lna2min                     = %d", (dib7000p_read_word(state, 0x793) >>  0 ) &0x3ff  );
    dbgpl(NULL, "lna2Scale                   = %d", (dib7000p_read_word(state, 0x793) >> 10 ) &0x3f   );
    dbgpl(NULL, "lna2max                     = %d", (dib7000p_read_word(state, 0x794) >>  0 ) &0x3ff  );
    dbgpl(NULL, "lna2Offs                    = %d", (dib7000p_read_word(state, 0x794) >> 10 ) &0x3f   );
    dbgpl(NULL, "rfvgamin                    = %d", (dib7000p_read_word(state, 0x795) >>  0 ) &0x3ff  );
    dbgpl(NULL, "rfvgaScale                  = %d", (dib7000p_read_word(state, 0x795) >> 10 ) &0x3f   );
    dbgpl(NULL, "rfvgamax                    = %d", (dib7000p_read_word(state, 0x796) >>  0 ) &0x3ff  );
    dbgpl(NULL, "rfvgaOffs                   = %d", (dib7000p_read_word(state, 0x796) >> 10 ) &0x3f   );
    dbgpl(NULL, "targetGlobalGainRf          = %d", (dib7000p_read_word(state, 0x797) >>  0 ) &0x3ff  );
    dbgpl(NULL, "enRfRamp                    = %d", (dib7000p_read_word(state, 0x797) >> 10 ) &0x1    );
    dbgpl(NULL, "en_mux_rxrf1                = %d", (dib7000p_read_word(state, 0x797) >> 11 ) &0x1    );
    dbgpl(NULL, "en_mux_rxrf3                = %d", (dib7000p_read_word(state, 0x797) >> 12 ) &0x1    );
    dbgpl(NULL, "incrRfGain_6                = %d", (dib7000p_read_word(state, 0x797) >> 15 ) &0x1    );
    dbgpl(NULL, "gain4min                    = %d", (dib7000p_read_word(state, 0x798) >>  0 ) &0x3ff  );
    dbgpl(NULL, "gain4Scale                  = %d", (dib7000p_read_word(state, 0x798) >> 10 ) &0x3f   );
    dbgpl(NULL, "gain4max                    = %d", (dib7000p_read_word(state, 0x799) >>  0 ) &0x3ff  );
    dbgpl(NULL, "gain4Offs                   = %d", (dib7000p_read_word(state, 0x799) >> 10 ) &0x3f   );
    dbgpl(NULL, "update_gain_period          = 0x%06x", ((dib7000p_read_word(state, 0x79b)&0xff) << 16) | (dib7000p_read_word(state, 0x79a)));
    dbgpl(NULL, "alpha_rf_filter             = %d", (dib7000p_read_word(state, 0x79b) >>  8 ) &0x7    );
    dbgpl(NULL, "alpha_bb_filter             = %d", (dib7000p_read_word(state, 0x79b) >> 11 ) &0x7    );
    dbgpl(NULL, "enDemodGain                 = %d", (dib7000p_read_word(state, 0x79b) >> 14 ) &0x1    );

}
#endif

#ifdef CONFIG_DEMOD_DIB7000P_SMART_SFN_CORRECTION
#define DIB7000P_SMART_SFN_NBR_TPS_FRAME 2
#define DIB7000P_SMART_SFN_TPS_LOWER_LIMIT 1
#define DIB7000P_SMART_SFN_TPS_UPPER_LIMIT 15
int dib7000p_update_FFT_window(struct dibFrontend *fe, uint8_t nb_tps_window)
{
    struct dib7000p_state *state = fe->demod_priv;
    uint16_t tps_symbol_number, reg_167 = dib7000p_read_word(state, 167);
    uint8_t bad_TPS_window, change_window = 1, nbr_window = 0, pha3_alpha_bad_window = 0xf;
    uint32_t now;

    dbgpl(&dib7000p_dbg, "fe%i: update FFT window", fe->id);
#ifdef CONFIG_DEMOD_DIB7000P_DEBUG_SMART_SFN_CORRECTION
    if (state->pha_shift != 0) {
        uint16_t pha_shift = dib7000p_read_word(state, 417) & 0x1fff;

        dbgpl(&dib7000p_dbg, "fe%i: pha shift delta=%i pha shift=%i", fe->id, state->pha_shift - pha_shift, pha_shift);
    }
#endif

    tps_symbol_number = (dib7000p_read_word(state, 462) >> 2) & 0x7f;
    /* do not take into account the first window */
    if ((tps_symbol_number < DIB7000P_SMART_SFN_TPS_UPPER_LIMIT) && (tps_symbol_number >= DIB7000P_SMART_SFN_TPS_LOWER_LIMIT))
        bad_TPS_window = 0;
    else
        bad_TPS_window = 1;

    /* lmod4 not locked ==> there is no signal */
    if (((dib7000p_read_word(state, 509) >> 11) & 1) == 0)
        return DIB_RETURN_ERROR;

    now = systime();
    do {
        if (nbr_window >= nb_tps_window)
            break;

        /* timeout */
        if ((systime() - now) > 6400)
            break;

#ifdef CONFIG_DEMOD_DIB7000P_DEBUG_SMART_SFN_CORRECTION
        dbgpl(&dib7000p_dbg, "fe%i: TPS symbol number = %i nbr_window = %i", fe->id, tps_symbol_number, nbr_window);
#endif
        if ((tps_symbol_number < DIB7000P_SMART_SFN_TPS_UPPER_LIMIT)
                && (tps_symbol_number >= DIB7000P_SMART_SFN_TPS_LOWER_LIMIT)
                && (bad_TPS_window == 1)) {
            bad_TPS_window = 0;
            change_window = 1;
            nbr_window++;
        } else if (((tps_symbol_number >= DIB7000P_SMART_SFN_TPS_UPPER_LIMIT) || (tps_symbol_number < DIB7000P_SMART_SFN_TPS_LOWER_LIMIT))
                && (bad_TPS_window == 0)) {
            bad_TPS_window = 1;
            change_window = 1;
        } else
            change_window = 0;
        if (change_window) {
#ifdef CONFIG_DEMOD_DIB7000P_DEBUG_SMART_SFN_CORRECTION
            dbgpl(&dib7000p_dbg, "fe%i: bad TPS window=%i ==> update pha3 alpha to %i", fe->id, bad_TPS_window, bad_TPS_window ? pha3_alpha_bad_window : state->pha3_alpha_default);
#endif
            /* set the pha3 alpha */
            if (bad_TPS_window)
                dib7000p_write_word(state, 167, (pha3_alpha_bad_window << 4) | (reg_167 & 0xf));
            else
                dib7000p_write_word(state, 167, (state->pha3_alpha_default << 4) | (reg_167 & 0xf));
        }
#ifdef CONFIG_DEMOD_DIB7000P_DEBUG_SMART_SFN_CORRECTION
        /* dbgpl(&dib7000p_dbg, "fe%i: pha3 like=%i TPS symbol number=%i syndrome=%i pha shift=%i\n", state->fe->id,
                dib7000p_read_word(state, 454) & 0x7fff, tps_symbol_number, dib7000p_read_word(state, 488),
                dib7000p_read_word(state, 417) & 0x1fff); */
        dbgpl(&dib7000p_dbg, "fe%i: TPS symbol number = %i pha_shift=%i isi pha=%i bad TPS window=%i",
                fe->id, tps_symbol_number, dib7000p_read_word(state, 417), dib7000p_read_word(state, 458), bad_TPS_window);
#endif
        tps_symbol_number = (dib7000p_read_word(state, 462) >> 2) & 0x7f;
    } while ((nbr_window < (nb_tps_window-1)) || (tps_symbol_number < DIB7000P_SMART_SFN_TPS_UPPER_LIMIT));
    dib7000p_write_word(state, 167, (pha3_alpha_bad_window << 4) | (reg_167 & 0xf)); /* after the update of the FFT window, leave the pha3 alpha to the maximum */

#ifdef CONFIG_DEMOD_DIB7000P_DEBUG_SMART_SFN_CORRECTION
    dbgpl(&dib7000p_dbg, "fe%i: pha shift=%i isi pha=%i", state->fe->id, dib7000p_read_word(state, 417) & 0x1fff, dib7000p_read_word(state, 458) & 0x1fff);
    state->pha_shift = dib7000p_read_word(state, 417) & 0x1fff;
#endif

    return DIB_RETURN_SUCCESS;
}
#endif

int dib7000p_tune(struct dibFrontend *fe, struct dibChannel *ch)
{
    struct dib7000p_state *state = fe->demod_priv;
    uint16_t tmp = 0;
#ifdef CONFIG_DEMOD_DIB7000P_SMART_SFN_CORRECTION
    uint16_t reg_770;
#endif

    if (ch != NULL)
        dib7000p_set_channel(state, ch, 0);
    else
        return DIB_RETURN_ERROR;

    // restart demod
    dib7000p_write_word(state, 770, 0x4000);
    dib7000p_write_word(state, 770, 0x0000);
    DibMSleep(45);

    /* P_ctrl_inh_cor=0, P_ctrl_alpha_cor=4, P_ctrl_inh_isi=0, P_ctrl_alpha_isi=3, P_ctrl_inh_cor4=1, P_ctrl_alpha_cor4=3 */
    tmp = (0 << 14) | (4 << 10) | (0 << 9) | (3 << 5) | (1 << 4) | (0x3);
    if (state->sfn_workaround_active) {
        dbgpl(&dib7000p_dbg, "SFN workaround is active");
        tmp |= (1 << 9);
        dib7000p_write_word(state, 166, 0x4000); // P_pha3_force_pha_shift
    } else {
        dib7000p_write_word(state, 166, 0x0000); // P_pha3_force_pha_shift
    }
    dib7000p_write_word(state, 29, tmp);

    // never achieved a lock with that bandwidth so far - wait for osc-freq to update
    if (state->timf == 0)
        DibMSleep(200);

    /* offset loop parameters */

    /* P_timf_alpha, P_corm_alpha=6, P_corm_thres=0x80 */
    tmp = (6 << 8) | 0x80;
    switch (ch->u.dvbt.nfft) {
        case FFT_2K: tmp |= (2 << 12); break;
        case FFT_4K: tmp |= (3 << 12); break;
        default:
        case FFT_8K: tmp |= (4 << 12); break;
    }
    dib7000p_write_word(state, 26, tmp);  /* timf_a(6xxx) */

    /* P_ctrl_freeze_pha_shift=0, P_ctrl_pha_off_max */
    tmp = (0 << 4);
    switch (ch->u.dvbt.nfft) {
        case FFT_2K: tmp |= 0x6; break;
        case FFT_4K: tmp |= 0x7; break;
        default:
        case FFT_8K: tmp |= 0x8; break;
    }
    dib7000p_write_word(state, 32,  tmp);

    /* P_ctrl_sfreq_inh=0, P_ctrl_sfreq_step */
    tmp = (0 << 4);
    switch (ch->u.dvbt.nfft) {
        case FFT_2K: tmp |= 0x6; break;
        case FFT_4K: tmp |= 0x7; break;
        default:
        case FFT_8K: tmp |= 0x8; break;
    }
    dib7000p_write_word(state, 33,  tmp);

    tmp = dib7000p_read_word(state,509);
    if (!((tmp >> 6) & 0x1)) {
        /* restart the fec */
        tmp = dib7000p_read_word(state,771);
        dib7000p_write_word(state, 771, tmp | (1 << 1));
        dib7000p_write_word(state, 771, tmp);
        DibMSleep(40);
        tmp = dib7000p_read_word(state,509);
    }

    // we achieved a lock - it's time to update the osc freq
    if ((tmp >> 6) & 0x1) {
        dib7000p_update_timf(state);
         /* P_timf_alpha += 2 */
        tmp = dib7000p_read_word(state, 26);
        dib7000p_write_word(state, 26, (tmp&~(0xf<<12)) | ((((tmp>>12)&0xf)+5)<<12));
    }

    if (state->cfg.spur_protect)
        dib7000p_spur_protect(state, ch->RF_kHz, ch->bandwidth_kHz);

    dib7000p_set_bandwidth(state, ch->bandwidth_kHz);

#ifdef CONFIG_DEMOD_DIB7000P_SMART_SFN_CORRECTION
    if  ((state->sfn_workaround_active == 0) && ((dib7000p_read_word(state, 509) >> 11) & 0x1) == 1) {
#if 0
        uint32_t since;

        since = systime();
        do {
            dbgpl(&dib7000p_dbg, "fe%i: TPS symbol number = %i pha3 like=%i pha shift=%i", state->fe->id,
                    (dib7000p_read_word(state, 462) >> 2) & 0x7f, dib7000p_read_word(state, 454) & 0x7fff, dib7000p_read_word(state, 417) & 0x1fff);
        } while ((systime() - since) < 30000);

#endif
        /* the demod locks */
        if (ch->u.dvbt.guard != 3) {
            /* guard interval not 1/4 ==> desactivate pha3 */
            dbgpl(&dib7000p_dbg, "fe%i: guard interval is 1/%i ==> desactive pha3", state->fe->id, 32 / (1 << ch->u.dvbt.guard));
            dib7000p_write_word(state, 166, 0x4000); /* P_pha3_force_pha_shift = 1 */
            dib7000p_write_word(state, 29, dib7000p_read_word(state, 29) | (1 << 9)); /* P_ctrl_inh_isi = 1 */

            /* restart ctrl */
            reg_770 = dib7000p_read_word(state, 770);
            dib7000p_write_word(state, 770, reg_770 | (1 << 12));
            DibMSleep(10);
            dib7000p_write_word(state, 770, reg_770 & ~(1 << 12));
        } else {
            dbgpl(&dib7000p_dbg, "fe%i: guard interval is 1/%i: update FFT position", state->fe->id, 32 / (1 << ch->u.dvbt.guard));
            if (state->pha3_alpha_default == 0xff)
                state->pha3_alpha_default = (dib7000p_read_word(state, 167) >> 4) & 0xf; /* get the default value of the pha3 alpha only during the first tune */
            dib7000p_update_FFT_window(state->fe, DIB7000P_SMART_SFN_NBR_TPS_FRAME);
        }
    }
    dib7000p_write_word(state, 200, dib7000p_read_word(state, 200) | (1 << 7));
#endif

#if 0
    if(state->version == SOC7090)
        dib7000p_dump_tuner_dig(fe);
#endif

    return DIB_RETURN_SUCCESS;
}

void dib7000p_get_agc_values(struct dibFrontend *fe, uint16_t *agc_global, uint16_t *agc1, uint16_t *agc2, uint16_t *wbd)
{
    struct dib7000p_state *state = fe->demod_priv;
    if (agc_global != NULL)
        *agc_global = dib7000p_read_word(state, 394);
    if (agc1 != NULL)
        *agc1 = dib7000p_read_word(state, 392);
    if (agc2 != NULL)
        *agc2 = dib7000p_read_word(state, 393);
    if (wbd != NULL)
        *wbd = dib7000p_read_word(state, 397);
}

void dib7000p_get_sad_data(struct dibFrontend *fe, uint16_t *sad0, uint16_t *sad1, uint16_t *sad2,  uint16_t *sad3)
{
    struct dib7000p_state *state = fe->demod_priv;

    if (sad0 != NULL)
        *sad0 = dib7000p_read_word(state, 384);
    if (sad1 != NULL)
        *sad1 = dib7000p_read_word(state, 385);
    if (sad2 != NULL)
        *sad2 = dib7000p_read_word(state, 386);
    if (sad3 != NULL)
        *sad3 = dib7000p_read_word(state, 387);
}

void dib7000p_set_agc_range(struct dibFrontend *fe, uint8_t agc, uint16_t min, uint16_t max)
{
    struct dib7000p_state *state = fe->demod_priv;
    state->current_band = 0xff;
    if (agc == 0) {
        dib7000p_write_word(state, 107, max);
        dib7000p_write_word(state, 108, min);
    } else {
        dib7000p_write_word(state, 109, min);
        dib7000p_write_word(state, 110, max);
    }
}
