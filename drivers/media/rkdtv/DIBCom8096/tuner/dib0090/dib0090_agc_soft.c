#include <adapter/frontend.h>
#include <tuner/dib0090.h>
#include "dib0090_priv.h"

//#define DEBUG_AGC

/* average the RF max gain with 2^WBD_ALPHA normally max=5 (target = -640 or
 * -100) => 10 bits signed => WBD_ALPHA <= 5
 *
 * we use 6, but it's risky, but it can work -> because we never have less than
 * 61dBm (610-100 = 510 == 9bits)
 */
/* included in [0:962] => 10 bit unsigned => GAIN_ALPHA <= 5 */
#define WBD_ALPHA 6

//static struct slope dib0090_wbd_slopes[] =
//{
//	{  0, 1000,642 },
//	{  70, 550,35 }, //Between Offset and Offset+70 the power is between -100dBm and -55dBm
//	{ 768, 300, 117}, //Between Offset+70 and Offset+768 the power is between -55dBm and -30dBm
//	{ 1024, 0,0 },//Between Offset+768 and Offset+1024 the power is between -30dBm and -10dBm
//};

/* this array only covers the variable part of the wbd values, above the floor */
static const struct slope dib0090_wbd_slopes[3] =
{
    {  66, 120 }, /* -64,-52: offset -   65 */
    { 600, 170 }, /* -52,-35: 65     -  665 */
    { 170, 250 }, /* -45,-10: 665    - 835 */
};

void dib0090_get_current_gain(struct dibFrontend *fe, uint16_t *rf, uint16_t *bb, uint16_t *rf_gain_limit, uint16_t *rflt)
{
    struct dib0090_state *state = fe->tuner_priv;
    if (rf)
        *rf = state->gain[0];
    if (bb)
        *bb = state->gain[1];
    if (rf_gain_limit)
        *rf_gain_limit = state->rf_gain_limit;
    if (rflt)
        *rflt = (state->rf_lt_def >> 10) & 0x7;
}

static uint16_t slopes_to_scale(const struct slope *slopes, uint8_t num, int16_t val)
{
    uint8_t i;
    uint16_t rest;
    uint16_t ret = 0;
    for (i = 0; i < num; i++) {
        if (val  > slopes[i].range)
            rest = slopes[i].range;
        else
            rest = val;
        ret += (rest * slopes[i].slope) / slopes[i].range;
        val -= rest;
    }
    return ret;
}

static int16_t dib0090_wbd_to_db(struct dib0090_state *state, uint16_t wbd)
{
    if(wbd > 0x3ff) {
        //dbgp("wbd = %d > 1023 ! Saturating wdb to 1023\n", wbd);
        wbd = 0x3ff;
    }

    if (wbd < state->wbd_offset)
        wbd = 0;
    else
        wbd -= state->wbd_offset;
    /* -64dB is the floor */
    return -640 + (int16_t) slopes_to_scale(dib0090_wbd_slopes, ARRAY_SIZE(dib0090_wbd_slopes), wbd);
}

#ifdef DEBUG_AGC // Force the Wideband target for test purposes
void dib0090_force_wbd_target(struct dibFrontend *fe, uint16_t forced_wbd_offset)
{
    struct dib0090_state *state = fe->tuner_priv;
    enum frontend_tune_state *tune_state = &fe->tune_state;
    state->wbd_target = dib0090_wbd_to_db(state, state->wbd_offset + forced_wbd_offset); // get the value in dBm from the offset
    dbgpHal("wbd-target: %d dB\n", (uint32_t) state->wbd_target);
    dbgpHal("offset applied is %d\n", forced_wbd_offset);
    *tune_state = CT_AGC_STEP_0;
}
#endif

uint16_t dib0090_get_wbd_target(struct dibFrontend * fe, uint32_t rf)
{
    struct dib0090_state *state = fe->tuner_priv;
    uint32_t f_MHz = rf / 1000;
    int32_t current_temp = state->temperature;
    int32_t wbd_thot, wbd_tcold;
    const struct dib0090_wbd_slope *wbd = state->current_wbd_table;

    while (f_MHz > wbd->max_freq)
        wbd++;

    dbgpHal("using wbd-table-entry with max freq %d\n", wbd->max_freq);

    if (current_temp < 0)
        current_temp = 0;
    if (current_temp > 128)
        current_temp = 128;

    //What Wbd gain to apply for this range of frequency
    state->wbdmux &= ~(7<<13);
    if (wbd->wbd_gain!=0)
        state->wbdmux |=(wbd->wbd_gain<<13);
    else
        state->wbdmux |= (4<<13); // 4 is the default WBD gain

    dib0090_write_reg(state, CTRL_WBDMUX, state->wbdmux);

#if 0
    dib0090_print_WBDMUX_conf(state);
#endif

#ifdef CONFIG_TUNER_DIB0090_DEBUG
    dbgpHal("The gain applied should be %d which gives WBDMUX %x\n",wbd->wbd_gain, state->wbdmux);
#endif

    //All the curves are linear with slope*f/64+offset
    wbd_thot  = wbd->offset_hot  - (((uint32_t) wbd->slope_hot  * f_MHz) >> 6);
    wbd_tcold = wbd->offset_cold - (((uint32_t) wbd->slope_cold * f_MHz) >> 6);

    // Iet assumes that thot-tcold = 130 equiv 128, current temperature ref is -30deg
#ifdef CONFIG_TUNER_DIB0090_DEBUG
    dbgpHal("The wbd_hot %d and wbd_cold %d\n", wbd_thot, wbd_tcold);
#endif
    wbd_tcold += ( (wbd_thot - wbd_tcold) * current_temp ) >> 7;

#ifdef CONFIG_TUNER_DIB0090_DEBUG
    dbgpHal("The wbd_compensated set to %d at temp %d degrees\n", wbd_tcold, current_temp - 30);
#endif
    //for (offset = 0; offset < 1000; offset += 4)
    //    dbgpHal("offset = %d -> %d\n", offset, dib0090_wbd_to_db(state, offset));
    state->wbd_target = dib0090_wbd_to_db(state, state->wbd_offset + wbd_tcold); // get the value in dBm from the offset
    dbgpHal("wbd-target: %d dB\n", (uint32_t) state->wbd_target);
    dbgpHal("offset applied is %d\n", wbd_tcold);

    return state->wbd_offset + wbd_tcold;
}

uint16_t dib0090_get_wbd_offset(struct dibFrontend *tuner)
{
    struct dib0090_state *st = tuner->tuner_priv;
    return st->wbd_offset;
}

int dib0090_set_wbd_table(struct dibFrontend *fe, const struct dib0090_wbd_slope *wbd)
{
	struct dib0090_state *st = fe->tuner_priv;

	if (wbd != NULL)
		st->current_wbd_table = wbd;
	return DIB_RETURN_SUCCESS;
}

void dib0090_dcc_freq(struct dibFrontend *fe,uint8_t fast)
{
  struct dib0090_state *state = fe->tuner_priv;
  if(fast)
    dib0090_write_reg(state, CTRL_BB_4, 0);//1kHz
  else
    dib0090_write_reg(state, CTRL_BB_4, 1);//almost frozen
}

static void dib0090_set_rframp(struct dib0090_state *state, const uint16_t *cfg)
{
    state->rf_ramp = cfg;
}

static void dib0090_set_bbramp(struct dib0090_state *state, const uint16_t *cfg)
{
    state->bb_ramp = cfg;
    dib0090_set_boost(state, cfg[0] > 500); /* we want the boost if the gain is higher that 50dB */
}

/*******************************************************  General software ramp definition *****************************************/
static const uint16_t rf_ramp_uhf[] =
{
    407, /* Calibrated at 600MHz order has been changed g1-g4-g3-g2 */
    144, 144, 127, /* LNA1  : */
    110, 407, 255, /* LNA2  : */
     71, 297, 127, /* LNA3  : */
     82, 226, 127, /* LNA4  : */
      0,   0, 127, /* CBAND : */
  };

static const uint16_t rf_ramp_cband_broadmatching[] =/* for p1G only */
{
  314, /* Calibrated at 200MHz order has been changed g4-g3-g2-g1 */
  84, 314, 127, /* LNA1 */
  80, 230, 255, /* LNA2 */
  80, 150, 127, /* LNA3  It was measured 12dB, do not lock if 120*/
  70,  70, 127, /* LNA4 */
  0,   0, 127, /* CBAND */
};

static const uint16_t rf_ramp_vhf[] = /* for krosus before p1G */
{
    466, /* Calibrated at 200MHz order has been changed g1-g4-g3-g2 */
    146, 146, 127, /* LNA1 */
    118, 466, 255, /* LNA2 */
     95, 348, 127, /* LNA3  It was measured 12dB, do not lock if 120*/
    107, 253, 127, /* LNA4 */
      0,   0, 127, /* CBAND */
};

static const uint16_t rf_ramp_cband[] = /* for krosus before p1G */
{
   332, /* max RF gain in 10th of dB */
   132, 132, 127, /* LNA1,  dB */
   80,  332, 255, /* LNA2,  dB */
   0,   0,   127, /* LNA3,  dB */
   0,   0,   127, /* LNA4,  dB */
   120, 242, 127, /* LT1 CBAND */
};

static const uint16_t rf_ramp_lband[] =
{
    213, /* max RF gain in 10th of dB */
    132, 132, 127, /* LNA1,  13.2dB */
     81, 213, 255, /* LNA2,  8.1dB */
      0,   0, 127, /* LNA3,  0dB */
      0,   0, 127, /* LNA4,  0dB */
      0,   0, 127, /* CBAND, 0dB */
};

static const uint16_t rf_ramp_sband[] =
{
    253, /* max RF gain in 10th of dB */
    141, 141, 127, /* LNA1,  14.1dB */
    112, 253, 255, /* LNA2,  11.2dB */
      0,   0, 127, /* LNA3,  0dB */
      0,   0, 127, /* LNA4,  0dB */
      0,   0, 127, /* CBAND, 0dB */
};

static const uint16_t bb_ramp_boost[] =
{
    550, /* max BB gain in 10th of dB */
    260, 260,  26, /* BB1, 26dB */
    290, 550,  29, /* BB2, 29dB */
};

static const uint16_t bb_ramp_normal[] =
{
    500, /* max BB gain in 10th of dB */
    210, 210,  21, /* BB1, 21dB */
    290, 500,  29, /* BB2, 29dB */
};

static const int gain_reg_addr[4] =
{
    CTRL_RXRF1, CTRL_RXRF3, CTRL_RF_LT, CTRL_BB_1
};

//#define DEBUG_RF_GLITCH
#ifdef DEBUG_RF_GLITCH
#define GLITCH_PERIOD 1000 /* ms */
unsigned int cnt=0;
unsigned first_tune=1;
#endif

static void dib0090_gain_apply(struct dib0090_state *state, int16_t gain_delta, int16_t top_delta, uint8_t force)
{
    uint16_t rf, bb, ref;
    uint16_t i, v, gain_reg[4] = { 0 }, gain;
    const uint16_t *g;

#ifdef DEBUG_AGC
    int16_t adc_error = gain_delta;
#endif

    if (top_delta < -511)
        top_delta = -511;
    if (top_delta > 511)
        top_delta = 511;

    if (force) {
        top_delta *= (1 << WBD_ALPHA);
        gain_delta *= (1 << state->gain_alpha);
    }

    if(top_delta >= ((int16_t)(state->rf_ramp[0] << WBD_ALPHA) - state->rf_gain_limit)) /* overflow*/
        state->rf_gain_limit =state->rf_ramp[0] << WBD_ALPHA;
    else
        state->rf_gain_limit += top_delta;

    /* minimal AGC RF gain */
    int cap_ratio = 0;
#define AGC_RF_GAIN_LIMIT
#ifdef AGC_RF_GAIN_LIMIT
    if (state->config->is_nautilus) {
        if (state->current_band == BAND_UHF)
            cap_ratio = 52;
        else if ((state->current_band == BAND_CBAND) || (state->current_band == BAND_VHF))
            cap_ratio = 0;
    }
#endif
    if (state->rf_gain_limit <= (state->rf_ramp[0] << WBD_ALPHA)*cap_ratio/100)
        state->rf_gain_limit = (state->rf_ramp[0] << WBD_ALPHA)*cap_ratio/100;

    /* use gain as a temporary variable and correct current_gain */
    gain = ((state->rf_gain_limit >> WBD_ALPHA) + state->bb_ramp[0]) << state->gain_alpha;
    if(gain_delta >= ((int16_t)gain - state->current_gain)) /* overflow*/
        state->current_gain = gain ;
    else
        state->current_gain += gain_delta;

    /* cannot be less than 0 (only if gain_delta is less than 0 we can have current_gain < 0) */
    if (state->current_gain < 0)
        state->current_gain = 0;

    /* now split total gain to rf and bb gain */
    gain = state->current_gain >> state->gain_alpha;

    /* requested gain is bigger than rf gain limit - ACI/WBD adjustment */
    if (gain > (state->rf_gain_limit >> WBD_ALPHA)) {
        rf = state->rf_gain_limit >> WBD_ALPHA;
        bb = gain - rf;
        if (bb > state->bb_ramp[0])
            bb = state->bb_ramp[0];
    } else { /* high signal level -> all gains put on RF */
        rf = gain;
        bb = 0;
    }
 
    state->gain[0] = rf;
    state->gain[1] = bb;

    /* software ramp */
    /* Start with RF gains */
    g = state->rf_ramp + 1; /* point on RF LNA1 max gain */
    ref = rf;

    for (i = 0; i < 7; i++) { /* Go over all amplifiers => 5RF amps + 2 BB amps = 7 amps */
        if (g[0] == 0 || ref < (g[1] - g[0])) /* if total gain of the current amp is null or this amp is not concerned because it starts to work from an higher gain value */
            v = 0; /* force the gain to write for the current amp to be null */
        else if (ref >= g[1]) /* Gain to set is higher than the high working point of this amp */
            v = g[2]; /* force this amp to be full gain */
        else /* compute the value to set to this amp because we are somewhere in his range */
            v = ((ref - (g[1] - g[0])) * g[2]) / g[0];

        //dbgpHal( "amplifier %d : g[0] = %d, g[1] = %d, g[2] = %d : v = %d, ref = %d\n", i, g[0], g[1], g[2], v, ref);

        if (i == 0) /* LNA 1 reg mapping */
            gain_reg[0] = v;
        else if(i == 1) /* LNA 2 reg mapping */
            gain_reg[0] |= v << 7;
        else if(i == 2) /* LNA 3 reg mapping */
            gain_reg[1] = v;
        else if(i == 3) /* LNA 4 reg mapping */
            gain_reg[1] |= v << 7;
        else if(i == 4) /* CBAND LNA reg mapping */
            gain_reg[2] = v | state->rf_lt_def;
        else if(i == 5) /* BB gain 1 reg mapping */
            gain_reg[3] = v << 3;
        else if(i == 6) /* BB gain 2 reg mapping */
            gain_reg[3] |= v << 8;

        g += 3; /* go to next gain bloc */

        /* When RF is finished, start with BB */
        if (i == 4) {
            g = state->bb_ramp + 1; /* point on BB gain 1 max gain */
            ref = bb;
        }
    }
    gain_reg[3] |= state->bb_1_def;
    gain_reg[3] |= ((bb % 10) * 100) / 125;

#ifdef DEBUG_AGC
    dbgp("%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d\n", systime(), state->gain_alpha, adc_error, rf, bb, rf + bb, gain_reg[0]&0x7f, (gain_reg[0]>>7)&0xff, gain_reg[1]&0x7f, (gain_reg[1]>>7)&0x7f, (gain_reg[3]>>3)&0x1f, (gain_reg[3]>>8)&0x1f);

#endif

    /* Write the amplifier regs */
    for (i = 0; i < 4; i++) {
        v = gain_reg[i];
        //dbgpHal("writing 0x%04x @ 0x%04x\n", v, gain_reg_addr[i]);
        if (force || state->gain_reg[i] != v) {
            state->gain_reg[i]  = v;
            dib0090_write_reg(state, gain_reg_addr[i], v);
        }
    }

#ifdef DEBUG_RF_GLITCH
    cnt ++;
/* glitch delayed of 1s for CMMB reception of ts0 safe */
    if (cnt >= GLITCH_PERIOD+(1000*first_tune))
    {
       dib0090_write_reg(state, gain_reg_addr[0], 0);
       dib0090_write_reg(state, gain_reg_addr[1], 0);
       dib0090_write_reg(state, gain_reg_addr[2], 0);
       dib0090_write_reg(state, gain_reg_addr[3], 0);
    }
    if (cnt == GLITCH_PERIOD+ 25 +(1000*first_tune))
    {
       dib0090_write_reg(state, gain_reg_addr[0], state->gain_reg[0]);
       dib0090_write_reg(state, gain_reg_addr[1], state->gain_reg[1]);
       dib0090_write_reg(state, gain_reg_addr[2], state->gain_reg[1]);
       dib0090_write_reg(state, gain_reg_addr[3], state->gain_reg[1]);
       cnt = 0;
       first_tune = 0;
    }
#endif
}

static void dib0090_monitoring_refresh(struct dib0090_state *state, struct dibChannelMonitor *mon, uint16_t wbd_val)
{
    /* monitoring refresh was requested */
#ifndef FIRMWARE_FIREFLY /* AGC monitoring on firefly takes 450 Bytes */
    if (mon->refresh & FE_COMPONENT_TUNER) {
#endif
        mon->agc_global    = (uint16_t) (((uint32_t) (state->current_gain >> state->gain_alpha) * 0xffff) / (state->rf_ramp[0] + state->bb_ramp[0]));
        mon->agc_rf        = (uint16_t) (((uint32_t) state->gain[0] * 0xffff) / state->rf_ramp[0]);
        mon->agc_bb        = (uint16_t) (((uint32_t) state->gain[1] * 0xffff) / state->bb_ramp[0]);
        mon->agc_wbd       = wbd_val << 2; /* here wbd is on 10 bits - previously it was on 12 */
        mon->agc_wbd_split = (uint16_t) (((state->rf_gain_limit >> WBD_ALPHA) * 0xff) / state->rf_ramp[0]);
#ifndef FIRMWARE_FIREFLY
        mon->refresh &= ~FE_COMPONENT_TUNER;
    }
#endif
}

/*  ADC target BW adaptation array :
    10*log10(bw/8);
    Ie: 10*log10(5/8) = -2.04 => -20
    Order by BW KHz                    [DAB_1500, 3500, 4500, 5000, 6000, 7000, 8000] BW adaptation */
static const int adcTargetBwAdapt[8] = {     -72,  -36,  -25,  -20,  -12,   -5,    0};
static void dib0090_adapt_adc_taget(struct dib0090_state *state, struct dibChannel *ch)
{
    int32_t target, target_offset = 0;

    if(state->config->is_nautilus == 1)
       target = -190;
    else
       target = -220;

    switch(ch->bandwidth_kHz) {
        case 3500: target_offset = adcTargetBwAdapt[1]; break;
        case 4500: target_offset = adcTargetBwAdapt[2]; break;
        case 5000: target_offset = adcTargetBwAdapt[3]; break;
        case 6000: target_offset = adcTargetBwAdapt[4]; break;
        case 7000: target_offset = adcTargetBwAdapt[5]; break;
        default:
        case 8000: target_offset = adcTargetBwAdapt[6]; break;
    }

#ifdef CONFIG_STANDARD_DAB
    if (ch->type == STANDARD_DAB)
        target_offset = adcTargetBwAdapt[0];
#endif

#ifdef CONFIG_STANDARD_DVBT
    if ((state->config->is_nautilus == 0) && (ch->type == STANDARD_DVBT) && (ch->u.dvbt.constellation == QAM_64QAM || ch->u.dvbt.constellation == QAM_16QAM))
    {
       target_offset += 60;
    }
#endif

#ifdef CONFIG_STANDARD_ISDBT
    if ((state->config->is_nautilus == 0)
        &&(ch->type == STANDARD_ISDBT)
        &&(((ch->u.isdbt.layer[0].nb_segments > 0)
        &&((ch->u.isdbt.layer[0].constellation == QAM_64QAM)||(ch->u.isdbt.layer[0].constellation == QAM_16QAM)))
        ||((ch->u.isdbt.layer[1].nb_segments > 0)
        &&((ch->u.isdbt.layer[1].constellation == QAM_64QAM)||(ch->u.isdbt.layer[1].constellation == QAM_16QAM)))
        ||((ch->u.isdbt.layer[2].nb_segments > 0)
        &&((ch->u.isdbt.layer[2].constellation == QAM_64QAM)||(ch->u.isdbt.layer[2].constellation == QAM_16QAM)))))
        {
           target_offset += 60;
        }
#endif

    /* VOYAGER OR NAUTILUS dib0090_config struct sips or layouts should use adc_target_apply_3dB = 1 because get_adc_power returns IQ ANAFE power */
    if(state->config->adc_target_apply_3dB)
        target_offset += 30;

    state->adc_target = target + target_offset;
    //dbgpl(&dib0090_dbg,"FE%d : state->adc_target = %d initial target = %d ; target_offset = %d", state->fe->id, state->adc_target,target,target_offset);
}

int dib0090_gain_control(struct dibFrontend *fe, struct dibChannel *ch)
{
    struct dib0090_state *state = fe->tuner_priv;
    enum frontend_tune_state *tune_state = &fe->tune_state;
    int ret = 10;
    struct dibChannelMonitor *mon = channel_frontend_monitoring(ch, fe->id);
    uint16_t wbd_val = 0;
    uint8_t apply_gain_immediatly = 1;
    int16_t wbd_error = 0, adc_error = 0;
    uint8_t ltg2;
    int dab_max_adc_error = 10;

    if (*tune_state == CT_AGC_START) {
        state->bb4 = dib0090_read_reg(state, CTRL_BB_4);

#ifdef CONFIG_BUILD_HOST
        state->gain_alpha = 3;
#ifdef CONFIG_STANDARD_DAB
        if(ch->type == STANDARD_DAB)
            state->gain_alpha = 1; /* because adc_error is saturated to +/-1dB in DAB only */
#endif
#else // !CONFIG_BUILD_HOST
        state->gain_alpha = 5;
#ifdef CONFIG_STANDARD_DAB
        if(ch->type == STANDARD_DAB)
            state->gain_alpha = 3; /* because adc_error is saturated to +/-1dB in DAB only */
#endif
#endif

#ifdef DEBUG_AGC
        dbgp("%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;\n","Systime", "gain_alpha", "adc error", "rf","bb", "rf+bb", "LNA1","LNA2","LNA3", "LNA4","BB1", "BB2");
#endif

        state->agc_freeze = 0;

#ifdef CONFIG_BAND_LBAND
        if (state->current_band == BAND_LBAND) {
            dib0090_set_rframp(state, rf_ramp_lband);
            dib0090_set_bbramp(state, bb_ramp_boost);
        } else
#endif

#ifdef CONFIG_BAND_SBAND
        if (state->current_band == BAND_SBAND) {
            dib0090_set_rframp(state, rf_ramp_sband);
            dib0090_set_bbramp(state, bb_ramp_boost);
        } else
#endif

#ifdef CONFIG_BAND_VHF
        if (state->current_band == BAND_VHF && !state->identity.p1g) {
            dib0090_set_rframp(state, rf_ramp_vhf);
            dib0090_set_bbramp(state, bb_ramp_boost);
        } else
#endif

#ifdef CONFIG_BAND_CBAND
        if (state->current_band == BAND_CBAND && !state->identity.p1g) {
            dib0090_set_rframp(state, rf_ramp_cband);
            dib0090_set_bbramp(state, bb_ramp_boost);
        } else

#ifdef CONFIG_TUNER_DIB0090_P1G_SUPPORT
	if ((state->current_band == BAND_CBAND || state->current_band == BAND_VHF) && state->identity.p1g) {
            dib0090_set_rframp(state, rf_ramp_cband_broadmatching);
            dib0090_set_bbramp(state, bb_ramp_normal);
        } else
#endif
#endif
        {
            dib0090_set_rframp(state, rf_ramp_uhf);

            /* use bb_ramp_normal for Nautilus in UHF for DVBT and ISDBT(but 1SEG) */
            if ((state->config->is_nautilus == 1)
            && ((ch->type == STANDARD_DVBT)
            || (ch->type == STANDARD_ISDBT && ch->u.isdbt.sb_mode == 0)))
            {
                dib0090_set_bbramp(state, bb_ramp_normal);
            } else {
                dib0090_set_bbramp(state, bb_ramp_boost);
            }
        }
        // deactivate the ramp generator using PWM control
        dib0090_write_reg(state, RF_RAMP9, 0);
        dib0090_write_reg(state, BB_RAMP7, 0);

        dib0090_get_wbd_target(fe, fe->current_rf);

        state->rf_gain_limit = state->rf_ramp[0] << WBD_ALPHA;
        state->current_gain = ((state->rf_ramp[0] + state->bb_ramp[0]) / 2) << state->gain_alpha;

        *tune_state = CT_AGC_STEP_0;
    } else if (!state->agc_freeze) {
        int16_t wbd = 0, i, cnt;
        int adc;
        wbd_val = dib0090_get_slow_adc_val(state);

        if (*tune_state == CT_AGC_STEP_0)
            cnt = 5;
        else
            cnt = 1;

        for (i = 0; i < cnt; i++) {
            wbd_val  = dib0090_get_slow_adc_val(state);
            wbd += dib0090_wbd_to_db(state, wbd_val);
        }
        wbd /= cnt;
        wbd_error = state->wbd_target - wbd;

        if (*tune_state == CT_AGC_STEP_0) {

            if ((wbd_error < 0) && (state->rf_gain_limit > 0) && !state->identity.p1g) {
#ifdef CONFIG_BAND_CBAND
                /* in case of CBAND tune reduce first the lt_gain2 before adjusting the RF gain */

                 ltg2 = (state->rf_lt_def >> 10) & 0x7;
                 if (state->current_band == BAND_CBAND && ltg2) {
                     ltg2 >>= 1;
                     state->rf_lt_def &= ltg2 << 10; /* reduce in 3 steps from 7 to 0 */
                 }
#endif
             } else {
                 state->agc_step = 0;
                 dib0090_write_reg(state, CTRL_BB_4, (state->bb4 | 0x3)); /* force fast DC compensation setting for AGC spartup sequence */
                 *tune_state = CT_AGC_STEP_1;
             }
        } else {
            /* calc the adc power */
            adc = state->config->get_adc_power(fe);
            adc = (adc * ((int32_t) 355774) + (((int32_t)1) << 20)) >> 21;  /* included in [0:-700] */
            dib0090_adapt_adc_taget(state, ch);
            adc_error = (int16_t)(state->adc_target - adc);

            if (*tune_state == CT_AGC_STEP_1) { /* quickly go to the correct range of the ADC power */
                if (ABS(adc_error) < 50 || state->agc_step++ > 5) {
#ifndef EFUSE
#ifdef CONFIG_TUNER_DIB0090_DEBUG
                    dbgpHal("***************EFuse overwritten***************\n");
#endif
                    dib0090_write_reg(state, CTRL_BB_2, (1<<15)|(3<<11)|(6<<6)|(32));
#endif
                    dib0090_write_reg(state, CTRL_BB_4, state->bb4); /* reset back required STD DC servo cutoff after startup */
                    *tune_state = CT_AGC_STOP;
                }
            } else {
                /* everything higher than or equal to CT_AGC_STOP means tracking */
                if (ch->type == STANDARD_DAB) { /* suppresion glitch symbol NULL sur adc error en DAB */
                    if (adc_error > dab_max_adc_error)
                        adc_error =  dab_max_adc_error;
                    if (adc_error < -dab_max_adc_error)
                        adc_error = -dab_max_adc_error;
                }
                ret = 10; /* 1ms interval */
                apply_gain_immediatly = 0;
            }
        }
#ifdef DEBUG_AGC
        dbgpHal("FE: %d, tune state %d, ADC = %3ddB (ADC err %3d) WBD %3ddB (WBD err %3d, WBD val SADC: %4d), RFGainLimit (TOP): %3d, signal: %3ddBm\n",
        (uint32_t) fe->id, (uint32_t) *tune_state, (uint32_t) adc, (uint32_t) adc_error, (uint32_t) wbd, (uint32_t) wbd_error, (uint32_t) wbd_val,
        (uint32_t) state->rf_gain_limit >> WBD_ALPHA, (int32_t) 200 + adc - (state->current_gain >> state->gain_alpha));
#endif
    }

    dib0090_monitoring_refresh(state, mon, wbd_val);

    /* apply gain */
    if (!state->agc_freeze)
        dib0090_gain_apply(state, adc_error, wbd_error, apply_gain_immediatly);

    return ret;
}
