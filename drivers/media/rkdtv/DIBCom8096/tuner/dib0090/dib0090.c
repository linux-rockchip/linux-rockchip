#include <adapter/frontend.h>
#include <tuner/dib0090.h>
#include "dib0090_priv.h"

#define EFUSE
#define LPF	100
#ifndef POWER_SAVING
#define POWER_SAVING 0
#endif

DEBUG_OBJECT(dib0090_dbg, DEBUG_TUNER, "DiB0090")

static int dib0090_release(struct dibFrontend *fe) /* MERGE done */
{
    struct dib0090_state *st = fe->tuner_priv;
    MemFree(st,sizeof(struct dib0090_state));
    return 0;
}

/* wakeup from sleep */
static int dib0090_wakeup(struct dibFrontend *fe)
{
    struct dib0090_state *state = fe->tuner_priv;
    if (state->config->sleep)
        state->config->sleep(fe, 0);
#if POWER_SAVING == 1
    else
        dib0090_write_reg(state, TUNER_EN, 0x22ed);
#endif

    /* enable dataTX in case we have been restarted in the wrong moment */
    if(!state->identity.in_soc)
        dib0090_write_reg(state, DIG_CFG, dib0090_read_reg(state, DIG_CFG) | (1<<14));

	return DIB_RETURN_SUCCESS;
}

int dib0090_sleep(struct dibFrontend *fe)
{
    struct dib0090_state *state = fe->tuner_priv;

#if POWER_SAVING == 1
    /* disable the fast interface */
    if(!state->identity.in_soc)
        dib0090_write_reg(state, DIG_CFG, dib0090_read_reg(state, DIG_CFG) & ~(1<<14));
#endif

    if (state->config->sleep)
        state->config->sleep(fe, 1);
#if POWER_SAVING == 1
    else {
        // only keep pll
        dib0090_write_reg(state, TUNER_EN, 1 << 5);
    }
#endif
	return DIB_RETURN_SUCCESS;
}

uint32_t dib0090_get_slow_adc_val(struct dib0090_state *state)
{
    uint16_t adc_val = dib0090_read_reg(state, ADCVAL);
    if (state->identity.in_soc) {
#ifdef CONFIG_TUNER_DIB0090_DEBUG
        //dbgpHal(" *** The 10bits Sad has been shifted\n");
#endif
        adc_val >>= 2;
    }
#ifdef CONFIG_TUNER_DIB0090_DEBUG
    //  dbgpHal(" Slow ADC val = %d,%d\n", adc_val & 0xfff,(adc_val & 0xfff) * 1.8 / 4096);
    //  dbgpHal(" Slow ADC val = %d (%d)\n", adc_val>>12, adc_val);
#endif
    return adc_val;
}

extern void dib0090_set_boost(struct dib0090_state *state, int onoff)
{
    state->bb_1_def &= 0xdfff;
    state->bb_1_def |= onoff << 13;
}

int dib0090_get_digital_clk_out(struct dibFrontend *fe)
{
    struct dib0090_state *state = fe->tuner_priv;
    return (state->config->io.clock_khz * state->config->io.pll_loopdiv) / state->config->io.pll_prediv;
}

void dib0090_set_captrim(struct dibFrontend *fe)
{
   struct dib0090_state *state = fe->tuner_priv;
   state->calibrate = CAPTRIM_CAL; /* captrim serach now */
}

/* Changes the baseband filter settings */
static void dib0090_set_bandwidth(struct dib0090_state *state, struct dibChannel *ch)
{
    uint16_t tmp;
    if (ch->bandwidth_kHz <= 5000)
        tmp = (3 << 14);
    else if (ch->bandwidth_kHz <= 6000)
        tmp = (2 << 14);
    else if (ch->bandwidth_kHz <= 7000)
        tmp = (1 << 14);
    else
        tmp = (0 << 14);

    state->bb_1_def &= 0x3fff;
    state->bb_1_def |= tmp;

    dib0090_write_reg(state, CTRL_BB_1, state->bb_1_def); /* be sure that we have the right bb-filter */

#ifdef CONFIG_TUNER_DIB0090_DEBUG
        tmp = dib0090_read_reg(state, CTRL_BB_2);
        dbgpHal("CTRL_BB_2 = 0x%04x\n", tmp);
        dbgpHal("CTRL_BB_2_cap_value            = %d (<<  0)\n", (tmp >>  0) &0x3f);
        dbgpHal("CTRL_BB_2_res_hr               = %d (<<  6)\n", (tmp >>  6) &0x1f);
        dbgpHal("CTRL_BB_2_res_np               = %d (<< 11)\n", (tmp >> 11) &0xf);
        dbgpHal("CTRL_BB_2_nc                   = %d (<< 15)\n", (tmp >> 15) &0x1);

        tmp = dib0090_read_reg(state, CTRL_BB_3);
        dbgpHal("*** CTRL_BB_3 = 0x%04x\n", tmp);
        dbgpHal("CTRL_BB_3_filter2_cutoff_freq  = %d (<<  0)\n", (tmp >> 0)  &0x1f);
        dbgpHal("CTRL_BB_3_filt2missmatch_trim  = %d (<<  5)\n", (tmp >> 5)  &0x1f);
        dbgpHal("CTRL_BB_3_filter2_gm1_trim     = %d (<< 10)\n", (tmp >> 10) &0xf);
        dbgpHal("CTRL_BB_3_vcm3_trim            = %d (<< 14)\n", (tmp >> 14) &0x3);

        tmp = dib0090_read_reg(state, CTRL_BB_4);
        dbgpHal("*** CTRL_BB_4 = 0x%04x\n", tmp);
        dbgpHal("CTRL_BB_4_servo_cutoff         = %d (<<  0)\n", (tmp >>  0)& 0x3);
        dbgpHal("CTRL_BB_4_servo_fast           = %d (<<  2)\n", (tmp >>  2)& 0x1);
        dbgpHal("CTRL_BB_4_servo_ibias_trim     = %d (<<  3)\n", (tmp >>  3)& 0x7);
        dbgpHal("CTRL_BB_4_servo_dc_offset_curr = %d (<<  6)\n", (tmp >>  6)& 0xf);
        dbgpHal("CTRL_BB_4_en_DAB               = %d (<< 10)\n", (tmp >> 10)& 0x1);
        dbgpHal("CTRL_BB_4_servo_hold           = %d (<< 11)\n", (tmp >> 11)& 0x1);
        dbgpHal("CTRL_BB_4_servo_off            = %d (<< 12)\n", (tmp >> 12)& 0x1);
        dbgpHal("CTRL_BB_4_adc_test_buffer      = %d (<< 13)\n", (tmp >> 13)& 0x1);
        dbgpHal("CTRL_BB_4_vcm_out1             = %d (<< 14)\n", (tmp >> 14)& 0x3);

        tmp = dib0090_read_reg(state, CTRL_BB_5);
        dbgpHal("*** CTRL_BB_5 = 0x%04x\n", tmp);
        dbgpHal("CTRL_BB_5_filter13_gm2_ibias_t = %d (<<  0)\n", (tmp >>  0) &0xf);
        dbgpHal("CTRL_BB_5_obuf_ibias_trim      = %d (<<  4)\n", (tmp >>  4) &0x7);
        dbgpHal("CTRL_BB_5_ext_gm_trim          = %d (<<  7)\n", (tmp >>  7) &0x1);
        dbgpHal("CTRL_BB_5_nc                   = %d (<<  8)\n", (tmp >>  8) &0xf);
        dbgpHal("CTRL_BB_5_input_stage_ibias_tr = %d (<< 12)\n", (tmp >> 12) &0x3);
        dbgpHal("CTRL_BB_5_attenuator_ibias_tri = %d (<< 14)\n", (tmp >> 14) &0x3);
#endif

#if defined CONFIG_STANDARD_DAB || defined CONFIG_STANDARD_ISDBT
    if ((
#if defined CONFIG_STANDARD_ISDBT
        (ch->type == STANDARD_ISDBT && ch->u.isdbt.sb_mode == 1) ||
#else
        0 ||
#endif
#if defined CONFIG_STANDARD_DAB
        (ch->type == STANDARD_DAB)
#else
        0
#endif
        ) && state->identity.p1g) { //if 1 seg or dab and P1G

#ifdef CONFIG_TUNER_DIB0090_DEBUG
        dbgpHal("*** Narrow Bandwidth requested, what a luck we have a P1G : Bandwidth =%d ***\n",ch->bandwidth_kHz);
#endif
        dib0090_write_reg(state, CTRL_BB_2, (15 << 11) | (31 << 6) | 63); /* cap value = 63 : narrow BB filter */

        if(state->config->is_nautilus == 1)
            dib0090_write_reg(state, CTRL_BB_4, ( 1 << 10) | (15 << 6) | 1); /* DAB En, DCoffset set to 8, DC cutoff servo = 1*/
        else
            dib0090_write_reg(state, CTRL_BB_4, ( 1 << 10) | (15 << 6) | 3); /* DAB En, DCoffset set to 8, DC cutoff servo = 3*/

        tmp = dib0090_read_reg(state, CTRL_BB_3) & (~(0xf<<10)); //Clear the bit
        dib0090_write_reg(state, CTRL_BB_3, tmp);

        tmp = dib0090_read_reg(state, CTRL_BB_5) | (7<<9) | 0xf;
        dib0090_write_reg(state, CTRL_BB_5, tmp);
    } else {
       // dbgpHal("*** Set default bandwidth setting ***\n");

#ifndef FIRMWARE_FIREFLY
    tmp = (state->config->cap_value != 0)? state->config->cap_value : 22;
#else
    tmp = 22; /* cap_value = 22 */
#endif
        dib0090_write_reg(state, CTRL_BB_3, 0x6008); /* = 0x6008 : vcm3_trim = 1 ; filter2_gm1_trim = 8 ; filter2_cutoff_freq = 0 */
        dib0090_write_reg(state, CTRL_BB_4, 0x1);    /* 0 = 1KHz ; 1 = 50Hz ; 2 = 150Hz ; 3 = 50KHz ; 4 = servo fast*/

        if(state->identity.in_soc) {
            dib0090_write_reg(state, CTRL_BB_5, 0x9bcf); /* attenuator_ibias_tri = 2 ; input_stage_ibias_tr = 1 ; nc = 11 ; ext_gm_trim = 1 ; obuf_ibias_trim = 4 ; filter13_gm2_ibias_t = 15 */
        } else {
            dib0090_write_reg(state, CTRL_BB_2, (5 << 11) | (8 << 6) | (tmp & 0x3f)); /* tmp = cap_value */
            dib0090_write_reg(state, CTRL_BB_5, 0xabcd); /* = 0xabcd : attenuator_ibias_tri = 2 ; input_stage_ibias_tr = 2 ; nc = 11 ; ext_gm_trim = 1 ; obuf_ibias_trim = 4 ; filter13_gm2_ibias_t = 13 */
        }

    }
#endif

}

static const struct dib0090_wbd_slope dib0090_wbd_table_default[] = {
//  { 0,      0,   0, 130, 130 },
    { 470,    0, 250, 0,   100 ,4},
    { 860,   51, 866, 21,  375 ,4},
    {1700,    0, 800, 0,   850, 4}, //LBAND Predefinition , to calibrate
    {2900,    0, 250, 0,   100, 6}, //SBAND Predefinition , NOT tested Yet
    { 0xFFFF, 0,   0, 0,   0   ,0},
};

/********************************************* dib0090 P1G SOC 7090 tuning tables **************************************/
#ifdef CONFIG_TUNER_DIB0090_P1G_SOCX090_SUPPORT
/* SOC 7090 is P1G type */
static const struct dib0090_tuning dib0090_tuning_table_cband_7090[] =
{
    //max_freq, switch_trim, lna_tune, lna_bias, v2i, mix, load, tuner_enable;
#ifdef CONFIG_BAND_CBAND
  //{ 184000,  4,  3, 0x018F, 0x2c0, 0x2d12, 0xb9ce, EN_CAB }, // 0x81ce 0x8190 Good perf but higher current //0x4187 Low current
    { 300000,  4,  3,  0x018F, 0x2c0, 0x2d12, 0xb9ce, EN_CAB }, // 0x018F
    { 380000,  4,  10, 0x018F, 0x2c0, 0x2d12, 0xb9ce, EN_CAB }, //0x4187
    { 570000,  4,  10, 0x8190, 0x2c0, 0x2d12, 0xb9ce, EN_CAB },
    { 858000,  4,  5,  0x8190, 0x2c0, 0x2d12, 0xb9ce, EN_CAB },
#endif
};

#endif //CONFIG_TUNER_DIB0090_P1G_SOCX090_SUPPORT
static const struct dib0090_tuning dib0090_tuning_table_cband_7090e_sensitivity[] =
{
    //max_freq, switch_trim, lna_tune, lna_bias, v2i, mix, load, tuner_enable;
#ifdef CONFIG_BAND_CBAND
  { 300000,  0 ,  3,  0x8105, 0x2c0, 0x2d12, 0xb84e, EN_CAB }, // 0x018F */
  { 380000,  0 ,  10, 0x810F, 0x2c0, 0x2d12, 0xb84e, EN_CAB }, //0x4187 */
  { 600000,  0 ,  10, 0x815E, 0x280, 0x2d12, 0xb84e, EN_CAB },
  { 660000,  0 ,  5,  0x85E3, 0x280, 0x2d12, 0xb84e, EN_CAB },
  { 720000,  0 ,  5,  0x852E, 0x280, 0x2d12, 0xb84e, EN_CAB },
  { 860000,  0 ,  4,  0x85E5, 0x280, 0x2d12, 0xb84e, EN_CAB },

#endif
};

/********************************************* dib0090 P1G tuning tables **************************************/
#ifdef CONFIG_TUNER_DIB0090_P1G_SUPPORT
static const struct dib0090_pll dib0090_p1g_pll_table[] =
{
#ifdef CONFIG_BAND_CBAND
      { 34000, 	1, 13, 96, 6},  // CAB
      { 42000, 	0, 12, 64, 4},  // CAB
      { 46000, 	1, 12, 64, 4},  // CAB
      { 57000, 	0, 11, 48, 6},  // CAB
      { 70000, 	1, 11, 48, 6},  // CAB
      { 86000, 	0, 10, 32, 4},  // CAB
      { 105000,	1, 10, 32, 4}, // FM
      { 115000,	0, 9, 24, 6}, // FM
      { 140000,	1, 9, 24, 6}, // MID FM VHF
      { 170000,	0, 8, 16, 4}, // MID FM VHF
#endif
#ifdef CONFIG_BAND_VHF
      { 200000, 1, 8, 16, 4}, // VHF
      { 230000, 0, 7, 12, 6}, // VHF
      { 280000, 1, 7, 12, 6}, // MID VHF UHF
      { 340000, 0, 6, 8, 4},  // MID VHF UHF
      { 380000, 1, 6, 8, 4},  // MID VHF UHF
      { 455000, 0, 5, 6, 6},  // MID VHF UHF
#endif
#ifdef CONFIG_BAND_UHF
      { 580000, 1, 5,  6, 6}, // UHF
      { 680000, 0, 4,  4, 4}, // UHF
      { 860000, 1, 4,  4, 4}, // UHF
#endif
#ifdef CONFIG_BAND_LBAND
      { 1800000, 1, 2,  2, 4}, // LBD
#endif
#ifdef CONFIG_BAND_SBAND
      { 2900000, 0, 1, 1, 6}, // SBD
#endif
};

static const struct dib0090_pll dib7090e_p1g_pll_table[] =
{
#ifdef CONFIG_BAND_CBAND
      { 34000, 	1, 13, 96, 6},  // CAB
      { 42000, 	0, 12, 64, 4},  // CAB
      { 46000, 	1, 12, 64, 4},  // CAB
      { 57000, 	0, 11, 48, 6},  // CAB
      { 70000, 	1, 11, 48, 6},  // CAB
      { 86000, 	0, 10, 32, 4},  // CAB
      { 105000,	1, 10, 32, 4}, // FM
      { 115000,	0, 9, 24, 6}, // FM
      { 140000,	1, 9, 24, 6}, // MID FM VHF
      { 170000,	0, 8, 16, 4}, // MID FM VHF
#endif
#ifdef CONFIG_BAND_VHF
      { 200000, 1, 8, 16, 4}, // VHF
      { 235000, 0, 7, 12, 6}, // VHF
      { 280000, 1, 7, 12, 6}, // MID VHF UHF
      { 340000, 0, 6, 8, 4},  // MID VHF UHF
      { 380000, 1, 6, 8, 4},  // MID VHF UHF
      { 470000, 0, 5, 6, 6},  // MID VHF UHF
#endif
#ifdef CONFIG_BAND_UHF
      { 580000, 1, 5,  6, 6}, // UHF
      { 680000, 0, 4,  4, 4}, // UHF
      { 860000, 1, 4,  4, 4}, // UHF
#endif
#ifdef CONFIG_BAND_LBAND
      { 1800000, 1, 2,  2, 4}, // LBD
#endif
#ifdef CONFIG_BAND_SBAND
      { 2900000, 0, 1, 1, 6}, // SBD
#endif
};

static const struct dib0090_tuning dib0090_p1g_tuning_table_fm_vhf_on_cband[] =
{
    //max_freq, switch_trim, lna_tune, lna_bias, v2i, mix, load, tuner_enable;
#ifdef CONFIG_BAND_CBAND
    { 380000,  4, 3, 0x818f, 0x2c0, 0x2d12, 0xb8ce, EN_CAB }, // VHF EN_CAB
#endif
#ifdef CONFIG_BAND_UHF
    { 520000,  2, 0, 15, 0x300, 0x1d12, 0xb9ce, EN_UHF }, // UHF
    { 550000,  2, 2, 15, 0x300, 0x1d12, 0xb9ce, EN_UHF }, // UHF
    { 650000,  2, 3, 15, 0x300, 0x1d12, 0xb9ce, EN_UHF }, // UHF
    { 750000,  2, 5, 15, 0x300, 0x1d12, 0xb9ce, EN_UHF }, // UHF
    { 850000,  2, 6, 15, 0x300, 0x1d12, 0xb9ce, EN_UHF }, // UHF
    { 900000,  2, 7, 15, 0x300, 0x1d12, 0xb9ce, EN_UHF }, // UHF
#endif
#ifdef CONFIG_BAND_LBAND
    { 1500000, 4, 0, 20, 0x300, 0x1912, 0x82c9, EN_LBD }, // LBD EN_LBD
    { 1600000, 4, 1, 20, 0x300, 0x1912, 0x82c9, EN_LBD }, // LBD EN_LBD
    { 1800000, 4, 3, 20, 0x300, 0x1912, 0x82c9, EN_LBD }, // LBD EN_LBD
#endif
#ifdef CONFIG_BAND_SBAND
    { 2300000, 1, 4, 20, 0x300, 0x2d2A, 0x82c7, EN_SBD }, // SBD EN_SBD
    { 2900000, 1, 7, 20, 0x280, 0x2deb, 0x8347, EN_SBD }, // SBD EN_SBD
#endif
};

static const struct dib0090_tuning dib0090_p1g_tuning_table[] =
{
    //max_freq, switch_trim, lna_tune, lna_bias, v2i, mix, load, tuner_enable;
#ifdef CONFIG_BAND_CBAND
    { 170000,  4, 1, 0x820f, 0x300, 0x2d12, 0x82cb, EN_CAB }, // FM EN_CAB
#endif
#ifdef CONFIG_BAND_VHF
    { 184000,  1, 1, 15, 0x300, 0x4d12, 0xb94e, EN_VHF }, // VHF EN_VHF
    { 227000,  1, 3, 15, 0x300, 0x4d12, 0xb94e, EN_VHF }, // VHF EN_VHF
    { 380000,  1, 7, 15, 0x300, 0x4d12, 0xb94e, EN_VHF }, // VHF EN_VHF
#endif
#ifdef CONFIG_BAND_UHF
    { 510000,  2, 0, 15, 0x300, 0x1d12, 0xb9ce, EN_UHF }, // UHF
    { 540000,  2, 1, 15, 0x300, 0x1d12, 0xb9ce, EN_UHF }, // UHF
    { 600000,  2, 3, 15, 0x300, 0x1d12, 0xb9ce, EN_UHF }, // UHF
    { 630000,  2, 4, 15, 0x300, 0x1d12, 0xb9ce, EN_UHF }, // UHF
    { 680000,  2, 5, 15, 0x300, 0x1d12, 0xb9ce, EN_UHF }, // UHF
    { 720000,  2, 6, 15, 0x300, 0x1d12, 0xb9ce, EN_UHF }, // UHF
    { 900000,  2, 7, 15, 0x300, 0x1d12, 0xb9ce, EN_UHF }, // UHF
#endif
#ifdef CONFIG_BAND_LBAND
    { 1500000, 4, 0, 20, 0x300, 0x1912, 0x82c9, EN_LBD }, // LBD EN_LBD
    { 1600000, 4, 1, 20, 0x300, 0x1912, 0x82c9, EN_LBD }, // LBD EN_LBD
    { 1800000, 4, 3, 20, 0x300, 0x1912, 0x82c9, EN_LBD }, // LBD EN_LBD
#endif
#ifdef CONFIG_BAND_SBAND
    { 2300000, 1, 4, 20, 0x300, 0x2d2A, 0x82c7, EN_SBD }, // SBD EN_SBD
    { 2900000, 1, 7, 20, 0x280, 0x2deb, 0x8347, EN_SBD }, // SBD EN_SBD
#endif
};
#endif //CONFIG_TUNER_DIB0090_P1G_SUPPORT

/********************************************* dib0090 P1C_F tuning tables **************************************/
#ifdef CONFIG_TUNER_DIB0090_P1C_F_SUPPORT
static const struct dib0090_pll dib0090_pll_table[] =
{
#ifdef CONFIG_BAND_CBAND
    { 56000, 0, 9, 48, 6},  // CAB
    { 70000, 1, 9, 48, 6},  // CAB
    { 87000, 0, 8, 32, 4},  // CAB
    { 105000, 1, 8, 32, 4}, // FM
    { 115000, 0, 7, 24, 6}, // FM
    { 140000, 1, 7, 24, 6}, // MID FM VHF
    { 170000, 0, 6, 16, 4}, // MID FM VHF
#endif
#ifdef CONFIG_BAND_VHF
    { 200000, 1, 6, 16, 4}, // VHF
    { 230000, 0, 5, 12, 6}, // VHF
    { 280000, 1, 5, 12, 6}, // MID VHF UHF
    { 340000, 0, 4, 8, 4},  // MID VHF UHF
    { 380000, 1, 4, 8, 4},  // MID VHF UHF
    { 450000, 0, 3, 6, 6},  // MID VHF UHF
#endif
#ifdef CONFIG_BAND_UHF
    { 580000, 1, 3,  6, 6}, // UHF
    { 700000, 0, 2,  4, 4}, // UHF
    { 860000, 1, 2,  4, 4}, // UHF
#endif
#ifdef CONFIG_BAND_LBAND
    { 1800000, 1, 0,  2, 4}, // LBD
#endif
#ifdef CONFIG_BAND_SBAND
    { 2900000, 0, 14, 1, 4}, // SBD
#endif
};

static const struct dib0090_tuning dib0090_tuning_table_fm_vhf_on_cband[] =
{
    //max_freq, switch_trim, lna_tune, lna_bias, v2i, mix, load, tuner_enable;
#ifdef CONFIG_BAND_CBAND
    { 184000,  4, 1, 15, 0x280, 0x2912, 0xb94e, EN_CAB }, // FM EN_CAB
    { 227000,  4, 3, 15, 0x280, 0x2912, 0xb94e, EN_CAB }, // FM EN_CAB
    { 380000,  4, 7, 15, 0x280, 0x2912, 0xb94e, EN_CAB }, // FM EN_CAB
#endif
#ifdef CONFIG_BAND_UHF
    { 520000,  2, 0, 15, 0x300, 0x1d12, 0xb9ce, EN_UHF }, // UHF
    { 550000,  2, 2, 15, 0x300, 0x1d12, 0xb9ce, EN_UHF }, // UHF
    { 650000,  2, 3, 15, 0x300, 0x1d12, 0xb9ce, EN_UHF }, // UHF
    { 750000,  2, 5, 15, 0x300, 0x1d12, 0xb9ce, EN_UHF }, // UHF
    { 850000,  2, 6, 15, 0x300, 0x1d12, 0xb9ce, EN_UHF }, // UHF
    { 900000,  2, 7, 15, 0x300, 0x1d12, 0xb9ce, EN_UHF }, // UHF
#endif
#ifdef CONFIG_BAND_LBAND
    { 1500000, 4, 0, 20, 0x300, 0x1912, 0x82c9, EN_LBD }, // LBD EN_LBD
    { 1600000, 4, 1, 20, 0x300, 0x1912, 0x82c9, EN_LBD }, // LBD EN_LBD
    { 1800000, 4, 3, 20, 0x300, 0x1912, 0x82c9, EN_LBD }, // LBD EN_LBD
#endif
#ifdef CONFIG_BAND_SBAND
    { 2300000, 1, 4, 20, 0x300, 0x2d2A, 0x82c7, EN_SBD }, // SBD EN_SBD
    { 2900000, 1, 7, 20, 0x280, 0x2deb, 0x8347, EN_SBD }, // SBD EN_SBD
#endif
};

static const struct dib0090_tuning dib0090_tuning_table[] =
{
    //max_freq, switch_trim, lna_tune, lna_bias, v2i, mix, load, tuner_enable;
#ifdef CONFIG_BAND_CBAND
    { 170000,  4, 1, 15, 0x280, 0x2912, 0xb94e, EN_CAB }, // FM EN_CAB
#endif
#ifdef CONFIG_BAND_VHF
    { 184000,  1, 1, 15, 0x300, 0x4d12, 0xb94e, EN_VHF }, // VHF EN_VHF
    { 227000,  1, 3, 15, 0x300, 0x4d12, 0xb94e, EN_VHF }, // VHF EN_VHF
    { 380000,  1, 7, 15, 0x300, 0x4d12, 0xb94e, EN_VHF }, // VHF EN_VHF
#endif
#ifdef CONFIG_BAND_UHF
    { 520000,  2, 0, 15, 0x300, 0x1d12, 0xb9ce, EN_UHF }, // UHF
    { 550000,  2, 2, 15, 0x300, 0x1d12, 0xb9ce, EN_UHF }, // UHF
    { 650000,  2, 3, 15, 0x300, 0x1d12, 0xb9ce, EN_UHF }, // UHF
    { 750000,  2, 5, 15, 0x300, 0x1d12, 0xb9ce, EN_UHF }, // UHF
    { 850000,  2, 6, 15, 0x300, 0x1d12, 0xb9ce, EN_UHF }, // UHF
    { 900000,  2, 7, 15, 0x300, 0x1d12, 0xb9ce, EN_UHF }, // UHF
#endif
#ifdef CONFIG_BAND_LBAND
    { 1500000, 4, 0, 20, 0x300, 0x1912, 0x82c9, EN_LBD }, // LBD EN_LBD
    { 1600000, 4, 1, 20, 0x300, 0x1912, 0x82c9, EN_LBD }, // LBD EN_LBD
    { 1800000, 4, 3, 20, 0x300, 0x1912, 0x82c9, EN_LBD }, // LBD EN_LBD
#endif
#ifdef CONFIG_BAND_SBAND
    { 2300000, 1, 4, 20, 0x300, 0x2d2A, 0x82c7, EN_SBD }, // SBD EN_SBD
    { 2900000, 1, 7, 20, 0x280, 0x2deb, 0x8347, EN_SBD }, // SBD EN_SBD
#endif
};
#endif //CONFIG_TUNER_DIB0090_P1C_F_SUPPORT

static int dib0090_tune(struct dibFrontend *fe, struct dibChannel *ch)
{
    struct dib0090_state *state = fe->tuner_priv;
    const struct dib0090_tuning *tune = state->current_tune_table_index;
    const struct dib0090_pll *pll = state->current_pll_table_index;
    enum frontend_tune_state *tune_state = &fe->tune_state;

    uint16_t lo5, lo6, Den;
    uint32_t FBDiv, Rest, FREF, VCOF_kHz = 0;
    uint16_t tmp;
    int ret = 10; /* 1ms is the default delay most of the time */
    uint8_t c, i;

    /************************* VCO ***************************/
    /* Default values for FG                                 */
    /* from these are needed :                               */
    /* Cp,HFdiv,VCOband,SD,Num,Den,FB and REFDiv             */

    //dbgpHal("fe%hd: Tuning for Band: %d (%d kHz); tune_state = %d\n",fe->id, state->current_band, rf, fe->tune_state);
    //dbgpHal("FE %d TUNER STEP %d callback time = %d\n",fe->id, fe->tune_state,fe->tuner_info->callback_time);

    /* in any case we first need to do a calibration if needed */
    if (*tune_state == CT_TUNER_START) {
        /* deactivate DataTX before some calibrations */
        if (state->calibrate & (DC_CAL | TEMP_CAL | WBD_CAL)) {
            if(!state->identity.in_soc)
                dib0090_write_reg(state, DIG_CFG, dib0090_read_reg(state, DIG_CFG) & ~(1<<14));
        } else          /* Activate DataTX in case a calibration has been done before */
            if (state->config->analog_output == 0) {
                if(!state->identity.in_soc)
                    dib0090_write_reg(state, DIG_CFG, dib0090_read_reg(state, DIG_CFG) | (1<<14));
            }
    }

    if ((*tune_state >= CT_TUNER_START) && (*tune_state <= CT_TUNER_STOP)) {
        if (state->calibrate & DC_CAL)
            return dib0090_dc_offset_calibration(state, tune_state);
        else if (state->calibrate & WBD_CAL) {
            if (state->current_rf == 0) {
                //dbgpHal(****** current_rf = 0, Be carefull Wideband Calibration will be wrong****\n");
                state->current_rf = ch-> RF_kHz;
            }
            return dib0090_wbd_calibration(state, tune_state);
        }
        else if (state->calibrate & TEMP_CAL)
            return dib0090_get_temperature(state, tune_state);
        else if (state->calibrate & CAPTRIM_CAL)
            return dib0090_captrim_search(state, tune_state);
    }

    if (*tune_state == CT_TUNER_START) {

        /* if soc and AGC pwm control, disengage mux to be able to R/W access to CTRL_BB_1 register to set the right filter (cutoff_freq_select) during the tune sequence, otherwise, SOC SERPAR error when accessing to CTRL_BB_1 */
        if (state->config->use_pwm_agc && state->identity.in_soc) {
            tmp = dib0090_read_reg(state, BB_RAMP7);
            if((tmp>>10)&0x1)
                dib0090_write_reg(state, BB_RAMP7, tmp & ~(1<< 10));// disengage mux : en_mux_bb1 = 0
        }

        /* select the tuner input using current_band */
        state->current_band = (uint8_t)channel_frequency_band(ch->RF_kHz);

        /* first select the frequency offset */
        state->rf_request = ch->RF_kHz + (state->current_band == BAND_UHF ? state->config->freq_offset_khz_uhf : state->config->freq_offset_khz_vhf);

        /* force CBAND if requested by user: force_cband_input */
        if ( (state->current_band & BAND_CBAND || state->current_band & BAND_FM || state->current_band & BAND_VHF || state->current_band & BAND_UHF) &&
                state->config->force_cband_input) /* Use the CBAND input for all band */
            state->current_band = BAND_CBAND;
        else /* or force CBAND for certain products */

#ifdef CONFIG_TUNER_DIB0090_P1G_SOCX090_SUPPORT
            if (state->identity.in_soc) {
                if (state->rf_request < 380000) {
                    state->current_band = BAND_CBAND;
#ifdef CONFIG_TUNER_DIB0090_DEBUG
                    dbgpHal("*** 7090 : Force FM, VHF in CBD input ***\n");
#endif
                }
            } else
#endif

#if defined (CONFIG_TUNER_DIB0090_P1C_F_SUPPORT) || defined (CONFIG_TUNER_DIB0090_P1G_SUPPORT)
            {
                tmp = (state->identity.version >> 5)&0x7;
                if(tmp == 0x4 || tmp == 0x7) {
                    /* CBAND tuner version for VHF */
                    if(state->rf_request <= 380000) {
                        state->current_band = BAND_CBAND; /* Force CBAND */
#ifdef CONFIG_TUNER_DIB0090_DEBUG
                        dbgpHal("*** 7090 : Force FM, VHF in CBD input ***\n");
#endif
                    }
                }
            }
#endif

#ifdef CONFIG_STANDARD_ISDBT
        /* in ISDB-T 1seg we shift tuning frequency */
        if ((ch->type == STANDARD_ISDBT && ch->u.isdbt.sb_mode == 1 && ch->u.isdbt.partial_reception == 0)) {
            const struct dib0090_low_if_offset_table *LUT_offset = state->config->low_if;
            uint8_t found_offset = 0;
            uint32_t margin_khz = 100;

            if(LUT_offset != NULL) {
                while(LUT_offset->RF_freq != 0xffff) {
#ifdef CONFIG_TUNER_DIB0090_DEBUG
                    dbgpHal("rf = %d fe->current_rf = %d offset->RF_freq = %d offset->offset_khz = %d\n", state->rf_request, fe->current_rf, LUT_offset->RF_freq, LUT_offset->offset_khz);
#endif
                    if(((state->rf_request > (LUT_offset->RF_freq - margin_khz)) && (state->rf_request < (LUT_offset->RF_freq + margin_khz))) && LUT_offset->std == ch->type) {
                        state->rf_request += LUT_offset->offset_khz;
                        found_offset = 1;
                        break;
                    }
                    LUT_offset++;
                }
            }

            if(found_offset == 0)
                state->rf_request +=400;
        }
#endif

        if(ch->type == STANDARD_CTTB)
           state->rf_request += 50;

        if (fe->current_rf != state->rf_request  || (state->current_standard != ch->type)) {
            state->tuner_is_tuned = 0;
            fe->current_rf = 0;
            state->current_standard = 0;

            /* select the tune-table */
#ifdef CONFIG_TUNER_DIB0090_P1C_F_SUPPORT
            tune = dib0090_tuning_table;
#endif
#ifdef CONFIG_TUNER_DIB0090_P1G_SUPPORT
            if(state->identity.p1g)
                tune = dib0090_p1g_tuning_table;
#endif

            tmp = (state->identity.version >> 5)&0x7;

#ifdef CONFIG_TUNER_DIB0090_P1G_SOCX090_SUPPORT
            if (state->identity.in_soc) {
                if (state->config->is_dib7090e)
                    tune = dib0090_tuning_table_cband_7090e_sensitivity;
                else if(state->rf_request <= 380000)
                    tune = dib0090_tuning_table_cband_7090;
            } else
#endif

#if defined (CONFIG_TUNER_DIB0090_P1C_F_SUPPORT) || defined (CONFIG_TUNER_DIB0090_P1G_SUPPORT)
                if(tmp == 0x4 || tmp == 0x7) {
                    /* CBAND tuner version for VHF */
                    if(state->rf_request <= 380000) {
#ifdef CONFIG_TUNER_DIB0090_P1C_F_SUPPORT
                        tune = dib0090_tuning_table_fm_vhf_on_cband;
#endif
#ifdef CONFIG_TUNER_DIB0090_P1G_SUPPORT
                        if(state->identity.p1g)
                            tune = dib0090_p1g_tuning_table_fm_vhf_on_cband;
                    }
                }
#endif
#endif

#ifdef CONFIG_TUNER_DIB0090_P1C_F_SUPPORT
            pll = dib0090_pll_table;
#endif
#ifdef CONFIG_TUNER_DIB0090_P1G_SUPPORT
            if(state->identity.p1g)
                pll = dib0090_p1g_pll_table;
#endif

            /* Look for the interval */
            while (state->rf_request > tune->max_freq)
                tune++;
            while (state->rf_request > pll->max_freq)
                pll++;

            state->current_tune_table_index = tune;
            state->current_pll_table_index = pll;

            // select internal switch
            dib0090_write_reg(state, CTRL_RF_SW, 0xb800 | (tune->switch_trim));

            // Find the VCO frequency in MHz
            VCOF_kHz = (pll->hfdiv * state->rf_request) * 2;

            FREF = state->config->io.clock_khz; // REFDIV is 1FREF Has to be as Close as possible to 10MHz
#ifndef FIRMWARE_FIREFLY
            if (state->config->fref_clock_ratio != 0)
                FREF /= state->config->fref_clock_ratio;
#endif

            // Determine the FB divider
            // The reference is 10MHz, Therefore the FBdivider is on the first digits
            FBDiv = (VCOF_kHz / pll->topresc / FREF);
            Rest  = (VCOF_kHz / pll->topresc) - FBDiv * FREF; //in kHz

            // Avoid Spurs in the loopfilter bandwidth
            if (Rest < LPF)                   Rest = 0;
            else if (Rest < 2 * LPF)          Rest = 2 * LPF;
            else if (Rest > (FREF - LPF))   { Rest = 0 ; FBDiv += 1; } //Go to the next FB
            else if (Rest > (FREF - 2 * LPF)) Rest = FREF - 2 * LPF;
            Rest = (Rest * 6528) / (FREF / 10);
            state->rest = Rest;

            /* external loop filter, otherwise:
             * lo5 = (0 << 15) | (0 << 12) | (0 << 11) | (3 << 9) | (4 << 6) | (3 << 4) | 4;
             * lo6 = 0x0e34 */

#ifdef CONFIG_TUNER_DIB0090_P1C_F_SUPPORT
             if (Rest == 0) {
                 if (pll->vco_band)
                     lo5 = 0x049f;
                 //else if (state->config->analog_output)
                 //    lo5 = 0x041f;
                 else
                     lo5 = 0x041f;
             } else {
                 if (pll->vco_band)
                     lo5 = 0x049e;
                 else if (state->config->analog_output)
                     lo5 = 0x041d;
                 else
                     lo5 = 0x041c;
             }
#endif

#ifdef CONFIG_TUNER_DIB0090_P1G_SUPPORT
           if(state->identity.p1g) { /* Bias is done automatically in P1G */
#ifdef CONFIG_TUNER_DIB0090_P1G_SOCX090_SUPPORT
                if(state->identity.in_soc) {
                    if(state->identity.version == SOC_8090_P1G_11R1)
                        lo5 = 0x46f;
                    else
                        lo5 = 0x42f;
                }
                else
#endif
                   lo5 = 0x42c; //BIAS Lo set to 4 by default in case of the Captrim search does not take care of the VCO Bias
           }
#endif

            lo5 |= (pll->hfdiv_code << 11) | (pll->vco_band << 7); /* bit 15 is the split to the slave, we do not do it here */
#ifdef CONFIG_TUNER_DIB0090_DEBUG
            dbgpHal("***lo5 is loaded = 0x%04x *******\n", lo5);
#endif
            //Internal loop filter set...
            if(!state->config->io.pll_int_loop_filt) {
                if (state->identity.in_soc)
                    lo6 = 0xff98;
                else if(state->identity.p1g || (Rest == 0))
                    lo6 = 0xfff8;
                else
                    lo6 = 0xff28;
            } else
                lo6 = (state->config->io.pll_int_loop_filt << 3); // take the loop filter value given by the layout
            //dbgpHal("FE %d lo6 = 0x%04x\n", (uint32_t)fe->id, (uint32_t)lo6);

            Den = 1;
#ifdef CONFIG_TUNER_DIB0090_DEBUG
            dbgpHal("***Rest value = %d\n", Rest );
#endif

            if (Rest > 0) {
                if (state->config->analog_output)
                    lo6 |= (1 << 2) | 2; //SigmaDelta and Dither
                else {
                    if(state->identity.in_soc)
                        lo6 |= (1 << 2) | 2; //SigmaDelta and Dither
                    else
                        lo6 |= (1 << 2) | 1; //SigmaDelta and Dither
                }
                Den = 255;
            }

#ifdef CONFIG_BAND_SBAND
            if (state->current_band == BAND_SBAND)
                lo6 &= 0xfffb; //Remove the Dither for SBAND
#endif

            // Now we have to define the Num and Denum
            // LO1 gets the FBdiv
            dib0090_write_reg(state, CTRL_LO_1, (uint16_t) FBDiv);
            // LO2 gets the REFDiv
#ifndef FIRMWARE_FIREFLY
            if (state->config->fref_clock_ratio != 0)
                dib0090_write_reg(state, CTRL_LO_2, (Den << 8) | state->config->fref_clock_ratio);
            else
#endif
            dib0090_write_reg(state, CTRL_LO_2, (Den << 8) | 1);
            // LO3 for the Numerator
            dib0090_write_reg(state, CTRL_LO_3, (uint16_t)Rest);
            // VCO and HF DIV
            dib0090_write_reg(state, CTRL_LO_5, lo5);
            // SIGMA Delta
            dib0090_write_reg(state, CTRL_LO_6, lo6);


            // Check if the 0090 is analogged configured
            //Disable ADC and DigPLL =0xFF9F, 0xffbf for test purposes.
            //Enable The Outputs of the BB on DATA_Tx
            lo6 = tune->tuner_enable;
            if (state->config->analog_output)
                lo6 = (lo6 & 0xff9f) | 0x2;

            dib0090_write_reg(state, TUNER_EN, lo6 | EN_LO
#ifdef CONFIG_DIB0090_USE_PWM_AGC
                    | state->config->use_pwm_agc * EN_CRYSTAL
#endif
                    );

        }

        fe->current_rf = state->rf_request;
        state->current_standard = ch->type;

        ret = 20;
        state->calibrate = CAPTRIM_CAL; /* captrim serach now */
    }

    else if (*tune_state == CT_TUNER_STEP_0) {   /* Warning : because of captrim cal, if you change this step, change it also in _cal.c file because it is the step following captrim cal state machine */
        const struct dib0090_wbd_slope *wbd = state->current_wbd_table;

//        if(!state->identity.p1g) {
              while (fe->current_rf/1000 > wbd->max_freq)
                  wbd++;
//        }

              if(!state->identity.in_soc)
                  dib0090_write_reg(state, ADCCLK, 0x07ff);
		#if 0
        dbgpHal("FE %d Final Captrim: %d\n",(uint32_t)fe->id, (uint32_t)state->fcaptrim);
        dbgpHal("FE %d HFDIV code: %d\n",(uint32_t)fe->id, (uint32_t)pll->hfdiv_code);
        dbgpHal("FE %d VCO = %d\n",(uint32_t)fe->id, (uint32_t)pll->vco_band);
        dbgpHal("FE %d VCOF in kHz: %d ((%d*%d) << 1))\n",(uint32_t)fe->id, (uint32_t)((pll->hfdiv * state->rf_request) * 2), (uint32_t)pll->hfdiv, (uint32_t)state->rf_request);
        dbgpHal("FE %d REFDIV: %d, FREF: %d\n", (uint32_t)fe->id, (uint32_t)1, (uint32_t)state->config->io.clock_khz);
        dbgpHal("FE %d FBDIV: %d, Rest: %d\n",(uint32_t)fe->id, (uint32_t)dib0090_read_reg(state, CTRL_LO_1), (uint32_t)dib0090_read_reg(state, CTRL_LO_3));
        dbgpHal("FE %d Num: %d, Den: %d, SD: %d\n",(uint32_t)fe->id, (uint32_t)dib0090_read_reg(state, CTRL_LO_3), (uint32_t)(dib0090_read_reg(state, CTRL_LO_2) >> 8), (uint32_t)dib0090_read_reg(state, CTRL_LO_6) & 0x3);
		#endif

#define WBD     0x781 /* 1 1 1 1 0000 0 0 1 */
        c=4;
        i=3; //wbdmux_bias
#if defined(CONFIG_BAND_LBAND) || defined(CONFIG_BAND_SBAND)
        if ((state->current_band==BAND_LBAND) || (state->current_band==BAND_SBAND)) {
            c=2;
            i=2;
        }
#endif

	if (wbd->wbd_gain!=0) //&& !state->identity.p1g)
            c = wbd->wbd_gain;

        //Store wideband mux register.
        state->wbdmux=(c << 13) | (i << 11) | (WBD
#ifdef CONFIG_DIB0090_USE_PWM_AGC
                |(state->config->use_pwm_agc<<1)
#endif
                );
#ifdef CONFIG_TUNER_DIB0090_DEBUG
        dbgpHal("in WbdMux:%x with Gain %d\n",state->wbdmux,c);
#endif
        dib0090_write_reg(state, CTRL_WBDMUX, state->wbdmux);

#ifdef CONFIG_TUNER_DIB0090_P1G_SUPPORT
        if ((tune->tuner_enable == EN_CAB) && state->identity.p1g) {
            dbgpHal("P1G : The cable band is selected and lna_tune = %d\n",tune->lna_tune);
            dib0090_write_reg(state, CTRL_RXRF2, tune->lna_bias);
            dib0090_write_reg(state, CTRL_RF_SW, 0xb800 | (tune->lna_tune << 6) | (tune->switch_trim));
        } else
#endif
            dib0090_write_reg(state, CTRL_RXRF2, (tune->lna_tune << 5) | tune->lna_bias);

        dib0090_write_reg(state, CTRL_RF_V2I, tune->v2i);
        dib0090_write_reg(state, CTRL_RF_MIX, tune->mix);
        dib0090_write_reg(state, CTRL_RF_LOAD, tune->load);
        *tune_state = CT_TUNER_STEP_1;

    } else if (*tune_state == CT_TUNER_STEP_1) {
        /* initialize the lt gain register */
        state->rf_lt_def = 0x7c00;
        // dib0090_write_reg(state, CTRL_RF_LT, state->rf_lt_def);

        dib0090_set_bandwidth(state, ch);
	state->tuner_is_tuned = 1;

//       if(!state->identity.p1g)
	 state->calibrate |= WBD_CAL; // TODO: only do the WBD calibration for new tune
//
        state->calibrate |= TEMP_CAL; // Force the Temperature to be remesured at next TUNE.
        *tune_state = CT_TUNER_STOP;
    } else
        ret = FE_CALLBACK_TIME_NEVER;
    return ret;
}

#define pgm_read_word(w) (*w)
static const uint16_t dib0090_defaults[] =
{
    // RF INITIALISATION + BB
    25, CTRL_BB_1, // nb of values, start register
        0x0000, // 0x01  CTRL_BB_1
        0x99a0,   // 0x02  CTRL_BB_2 Captrim
        0x6008, // 0x03  CTRL_BB_3
        0x0000, // 0x04  CTRL_BB_4 DC servo loop 1KHz
        0x8bcb, // 0x05  CTRL_BB_5
        0x0000, // 0x06  CTRL_BB_6
        0x0405, // 0x07  CTRL_BB_7
        0x0000, // 0x08  CTRL_RF_1
        0x0000, // 0x09  CTRL_RF_2 Bias and Tune not set
        0x0000, // 0x0a	 CTRL_RF_3
        0xb802, // 0x0b	 CTRL_RF_SW
        0x0300, // 0x0c  CTRL_RF_V2I
        0x2d12, // 0x0d	 CTRL_RF_MIX
        0xbac0, // 0x0e  CTRL_LOAD
        0x7c00, // 0x0f	 CTRL_LT //CAB
        0xdbb9, // 0x10  CTRL_WBDMUX   // WBD MUX TOWARD BB
        0x0954, // 0x11  CTRL_TX
        0x0743,	// 0x12  IQ_ADC
        0x8000,	// 0x13	 CTRL_BIAS 0x4106 Internal bias
        0x0002, // 0x14  CTRL_CRYSTAL
        0x0040, // 0x15  CTRL_LO_1
        0x0100, // 0x16  CTRL_LO_2
        0x0000, // 0x17  CTRL_LO_3
        0xe910, // 0x18  CTRL_LO_4
        0x149e, // 0x19  CTRL_LO_5 1734

    1, CTRL_LO_6,
        0xff2d,	// 0x1a  CTRL_LO_6

    1, BB_RAMP7,
        0x0000,  // deactivate the ramp and ramp_mux to adjust the gain manually

    1, TUNER_EN,
        EN_UHF | EN_CRYSTAL, // 0x33 TUNER_EN

    0
};

static const uint16_t not_soc_dib0090_additionnal_defaults[] =
{
    2, ADCCLK,
        0x07FF,	// ADC_CLK
        0x0007, // VGA_MODE

    2, PWM1_REG,
        0x3ff, // pwm accu period
        0x111, // (alpha_bb<<8)|(resetn_pwm<<4)|(alpha_rf)
    0
};

#ifdef CONFIG_TUNER_DIB0090_P1G_SUPPORT
static const uint16_t dib0090_p1g_additionnal_defaults[] =
{
    // additionnal INITIALISATION for p1g to be written after dib0090_defaults
    1, CTRL_BB_5, // nb of values, start register
        0xabcd, // 0x05  CTRL_BB_5

    1, CTRL_TX, // nb of values, start register
	0x00b4, // 0x11  CTRL_LO_7 Now prepare for auto captrim

    1, CTRL_LO_6,
	0xfffd, // 0x19  CTRL_LO_6

    1, CAPTRIM_CFG,
	0x108,
    0
};
#endif

uint8_t dib0090_is_SH_capable(struct dibFrontend *slave)
{

    struct dib0090_state *state = slave->tuner_priv;

    if((dib0090_read_reg(state,EFUSE_1)&0xf) < 0xf) {
        dbgpHal("This device can not demodulate DVB-SH\n");
        return 0;
    }
    else
        return 1;
}

static void dib0090_set_default_config(struct dib0090_state *state, const uint16_t * n)
{
    uint16_t l, r;

    l = pgm_read_word(n++);
    while (l) {
        r = pgm_read_word(n++);
        do {
          /* DEBUG_TUNER */
	  //dbgpHal("%d, %d, %x\n", l, r, pgm_read_word(n));
            dib0090_write_reg(state, r, pgm_read_word(n++));
            r++;
        } while (--l);
	    l = pgm_read_word(n++);
    }
}

static void dib0090_pwm_agc_freeze(struct dibFrontend *fe, int freeze)
{
    struct dib0090_state *state = fe->tuner_priv;
    uint16_t reg_bb_ramp7, reg_rf_ramp9;
    uint8_t en_mux = 1 ;

    dbgpHal("%s\n", (freeze == 1)?"Freeze PWM AGC":"Unfreeze PWM AGC");

    if(freeze == 1 )
        en_mux = 0;

    reg_bb_ramp7 = (dib0090_read_reg(state, BB_RAMP7) & ~(1<<10)) | (en_mux << 10);
    dib0090_write_reg(state, BB_RAMP7, reg_bb_ramp7);

    reg_rf_ramp9 = (dib0090_read_reg(state, RF_RAMP9) & ~(0x3 << 11)) | ((en_mux << 11) | (en_mux << 12));
    dib0090_write_reg(state, RF_RAMP9, reg_rf_ramp9);
}

static int dib0090_reset(struct dibFrontend *fe) /* MERGE done */
{
    struct dib0090_state *state = fe->tuner_priv;
    dib0090_reset_digital(fe, state->config);

    /* Revision definition */
    if (dib0090_identify(fe) == DIB_RETURN_ERROR)
        return DIB_RETURN_ERROR;

#ifdef CONFIG_TUNER_DIB0090_P1B_SUPPORT
    if (!(state->identity.version & 0x1)) /* it is P1B - reset is already done */
        return DIB_RETURN_SUCCESS;
#endif

    /* Diversity/single sleep mode configuration :
         - do not turn off the BB/IQADC/BIAS of tuner in case of diversity die because of PVR mode issue
    */
    if(!state->identity.in_soc) {
        if((dib0090_read_reg(state, DIG_CFG_RO)>>5) & 0x2) {/* this is a diversity krosus die */
            dbgpHal("Diversity tuner die detected DIG_CFG_RO = 0x%04x\n", dib0090_read_reg(state, DIG_CFG_RO));
            dib0090_write_reg(state, SLEEP_EN, (EN_IQADC | EN_BB | EN_BIAS | EN_DIGCLK | EN_PLL | EN_CRYSTAL));
        }
        else { /* Single die : optimize Sleep mode power consumption */
            dbgpHal("Single tuner die detected DIG_CFG_RO = 0x%04x\n", dib0090_read_reg(state, DIG_CFG_RO));
            dib0090_write_reg(state, SLEEP_EN, (EN_DIGCLK | EN_PLL | EN_CRYSTAL));
        }
    }

    /* Upload the default values */
    dib0090_set_default_config(state, dib0090_defaults);
    if(!state->identity.in_soc)
        dib0090_set_default_config(state, not_soc_dib0090_additionnal_defaults);

    if(state->identity.in_soc)
      dib0090_write_reg(state, CTRL_LO_4, 0x2910); /* CTRL_LO_4_charge_pump_current = 0 */


#ifdef CONFIG_TUNER_DIB0090_P1G_SUPPORT
    if(state->identity.p1g)
        dib0090_set_default_config(state, dib0090_p1g_additionnal_defaults);
#endif

#ifdef EFUSE
    if (((state->identity.version & 0x1f) >= P1D_E_F) || (state->identity.in_soc)) /* Update the efuse : Only available for KROSUS > P1C  and SOC as well*/
      dib0090_set_EFUSE(state);

#endif

#ifndef FIRMWARE_FIREFLY
    if (state->config->cap_value != 0)
        dib0090_write_reg(state, CTRL_BB_2, (dib0090_read_reg(state, CTRL_BB_2)&0xffc0)|(state->config->cap_value&0x3f));
#endif

    /* Congigure in function of the crystal */
    if (state->config->force_crystal_mode != 0)
        dib0090_write_reg(state, CTRL_CRYSTAL, state->config->force_crystal_mode & 3);
    else if (state->config->is_nautilus == 1)
        dib0090_write_reg(state, CTRL_CRYSTAL, 2);
    else if (state->config->io.clock_khz >= 24000)
        dib0090_write_reg(state, CTRL_CRYSTAL, 1);
    else
        dib0090_write_reg(state, CTRL_CRYSTAL, 2);

#ifdef CONFIG_TUNER_DIB0090_DEBUG
    dbgpHal("Pll lock : %d\n", (dib0090_read_reg(state, DIG_CFG_RO)>>11)&0x1);
#endif

    state->calibrate = DC_CAL | WBD_CAL | TEMP_CAL; /* enable iq-offset-calibration and wbd-calibration when tuning next time */
    return DIB_RETURN_SUCCESS;
}

#ifdef DIBCOM_EXTENDED_MONITORING
#include <adapter/component_monitor.h>
#endif

#if !defined(FIRMWARE_FIREFLY)
static int dib0090_generic_monitoring(struct dibFrontend *fe, uint32_t cmd, uint8_t *b, uint32_t size, uint32_t offset)
{
    struct dib0090_state *state = fe->tuner_priv;
    int ret = DIB_RETURN_NOT_SUPPORTED;

    switch (cmd & ~GENERIC_MONIT_REQUEST_SIZE) {
    case GENERIC_MONIT_AGC_FREEZE:
        if(state->config->use_pwm_agc)
            dib0090_pwm_agc_freeze(fe, offset);
        else
            state->agc_freeze = offset;
        ret = 0;
        break;
    case GENERIC_MONIT_FORCE_MAX_GAIN:
        state->force_max_gain = offset;
        break;
#ifdef DIBCOM_EXTENDED_MONITORING
    case GENERIC_MONIT_COMPAT_EXTENDED:
        ret = sizeof(struct dibDemodMonitor);
        if(!(cmd & GENERIC_MONIT_REQUEST_SIZE) && (state->rf_ramp != NULL) && (state->bb_ramp != NULL)) {
            struct dibDemodMonitor *m = (struct dibDemodMonitor *) b;
            m->rf_power   = (((uint32_t) (state->current_gain >> state->gain_alpha) * 0xffff) / (state->rf_ramp[0] + state->bb_ramp[0]));
            m->agc1       = (((uint32_t) state->gain[0] * 0xffff) / state->rf_ramp[0]) / 65536.0;
            m->agc2       = (((uint32_t) state->gain[1] * 0xffff) / state->bb_ramp[0]) / 65536.0;
            m->agc_wbd    = dib0090_get_slow_adc_val(state) / 1024.0;
                    //(uint16_t) (((state->rf_gain_limit >> WBD_ALPHA) * 0xff) / state->rf_ramp[0]);
        }
        ret = sizeof(struct dibDemodMonitor);
        break;
#endif


    default:
        break;
    }

    return ret;
}
#endif

int dib0090_set_vga(struct dibFrontend *fe, uint8_t onoff)
{
    struct dib0090_state *state = fe->tuner_priv;

#ifdef CONFIG_TUNER_DIB0090_DEBUG
    dbgpHal("set the vga to %d\n", onoff);
#endif
    dib0090_write_reg(state, CTRL_RXRF2, (dib0090_read_reg(state, CTRL_RXRF2) & 0x7fff) | ((onoff & 1) << 15));
    return DIB_RETURN_SUCCESS;
}

#ifdef CONFIG_TUNER_DIB0090_P1G_SOCX090_SUPPORT
int dib0090_update_tuning_table_7090(struct dibFrontend *fe, uint8_t cfg_sensitivity)
{
#ifdef CONFIG_TUNER_DIB0090_P1G_SOCX090_SUPPORT
    struct dib0090_state *state = fe->tuner_priv;

    const struct dib0090_tuning *tune = dib0090_tuning_table_cband_7090e_sensitivity;
    const struct dib0090_tuning dib0090_tuning_table_cband_7090e_aci[] =
    {
        //max_freq, switch_trim, lna_tune, lna_bias, v2i, mix, load, tuner_enable;
#ifdef CONFIG_BAND_CBAND
        { 300000,  0 ,  3,  0x8165, 0x2c0, 0x2d12, 0xb84e, EN_CAB }, // 0x018F
        { 650000,  0 ,  4,  0x815B, 0x280, 0x2d12, 0xb84e, EN_CAB },
        { 860000,  0 ,  5,  0x84EF, 0x280, 0x2d12, 0xb84e, EN_CAB },
#endif
    };

    if((!state->identity.p1g) || (!state->identity.in_soc) || ((state->identity.version != SOC_7090_P1G_21R1) && (state->identity.version != SOC_7090_P1G_11R1))) {
        dbgpHal("dib0090_update_tuning_table_7090() function can only be used for dib7090\n");
        return DIB_RETURN_ERROR;
    }

#ifdef CONFIG_TUNER_DIB0090_DEBUG
    dbgpHal("update tuning table for %s\n", (cfg_sensitivity?"sensitivity":"aci"));
#endif

    if (cfg_sensitivity)
        tune = dib0090_tuning_table_cband_7090e_sensitivity;
    else
        tune = dib0090_tuning_table_cband_7090e_aci;

    while (state->rf_request > tune->max_freq)
        tune++;

    dib0090_write_reg(state, CTRL_RXRF2, (dib0090_read_reg(state, CTRL_RXRF2) & 0x8000) | (tune->lna_bias & 0x7fff));
    dib0090_write_reg(state, CTRL_RF_SW, (dib0090_read_reg(state, CTRL_RF_SW) & 0xf83f) | ((tune->lna_tune << 6) & 0x07c0));
    return DIB_RETURN_SUCCESS;
#else
    dbgpHal("dib0090_update_tuning_table_7090() function can only be used for dib7090\n");
    return DIB_RETURN_ERROR;
#endif
}
#endif

int dib0090_set_cg(struct dibFrontend *fe)
{
    struct dib0090_state *state = fe->tuner_priv;

#ifdef CONFIG_TUNER_DIB0090_DEBUG
    dbgpHal("set the cg : high level\n");
#endif
    dib0090_write_reg(state, CTRL_RXRF2, (dib0090_read_reg(state, CTRL_RXRF2) & 0xf83f) | ((1) << 6));
    return DIB_RETURN_SUCCESS;
}

int dib0090_set_switch(struct dibFrontend *fe, uint8_t sw1, uint8_t sw2, uint8_t sw3)
{
    struct dib0090_state *state = fe->tuner_priv;

#ifdef CONFIG_TUNER_DIB0090_DEBUG
    dbgpHal("set the switch to: sw1=%d sw2=%d sw3=%d\n", sw1, sw2, sw3);
#endif
    dib0090_write_reg(state, CTRL_RF_SW, (dib0090_read_reg(state, CTRL_RF_SW) & 0xfff8) | ((sw3 & 1) << 2) | ((sw2 & 1) << 1) | (sw1 & 1));

    return DIB_RETURN_SUCCESS;
}

static const struct dibTunerInfo dib0090_info = {
    INFO_TEXT("DiBcom DiB0090 (Kroesus)"),   // name

    TUNER_CAN_VHF | TUNER_CAN_UHF | TUNER_CAN_LBAND | TUNER_CAN_SBAND | TUNER_CAN_FAST_TUNE, // caps

    { // ops
        dib0090_reset,   // reset
        dib0090_wakeup,  // wakeup
        dib0090_sleep,   // sleep

        dib0090_tune,  // tune_digital

        NULL,
#if !defined(FIRMWARE_FIREFLY)
        dib0090_generic_monitoring,
#else
        NULL,
#endif

        dib0090_release, // release
    }
};

struct dibFrontend * dib0090_register(struct dibFrontend *fe, struct dibDataBusHost *data, const struct dib0090_config *config)
{
	struct dib0090_state *st = (struct dib0090_state *)MemAlloc(sizeof(struct dib0090_state));
	struct dibDataBusClient *client;
	if (st == NULL)
		return NULL;
	DibZeroMemory(st,sizeof(struct dib0090_state));

#ifdef CONFIG_DIB0090_USE_PWM_AGC
        if(!config->use_pwm_agc) {
#endif

    if (fe->demod_info == NULL)
        dbgpHal("this driver should have a demodulator in the same frontend - not registering the GAIN control functions\n");
    else
        fe->demod_info->ops.agc_startup = dib0090_gain_control;

#ifdef CONFIG_DIB0090_USE_PWM_AGC
        }
#endif
    if ((config->_unused0 != 0) || (config->force_cband_input > 1)) {
        dbgpHal("The config is not correct: _unused0=%i force_cband_input=%i\n", config->_unused0, config->force_cband_input);
        return NULL;
    }
    frontend_register_tuner(fe, &st->info, &dib0090_info, st);

    client = tuner_get_data_bus_client(fe);

    switch(data_bus_host_type(data)) {
#if !defined(CONFIG_BUILD_LEON) && defined(CONFIG_DIB0090_USE_PWM_AGC)
        case DATA_BUS_I2C:
            data_bus_client_init(client, &data_bus_client_template_i2c_8b_16b, data);
            data_bus_client_set_speed(client, 340);
            data_bus_client_set_device_id(client, config->i2c_address);
            break;
#endif
        case DATA_BUS_DIBCTRL_IF:
            data_bus_client_init(client, &data_bus_client_template_8b_16b, data);
            break;
        default:
            dbgpHal("data_bus host type not handled\n");
    }

    st->config = config;
    st->fe = fe;
    st->tune = dib0090_tune;
    st->rf_request = 0;


    if (config->wbd == NULL) {
#ifdef CONFIG_TUNER_DIB0090_DEBUG
      dbgpHal ("******* Default wideband table is used ******* \n");
#endif
      st->current_wbd_table = dib0090_wbd_table_default;
    }
    else {
#ifdef CONFIG_TUNER_DIB0090_DEBUG
      dbgpHal ("******* Layout wideband table is used ******* \n");
#endif
      st->current_wbd_table = config->wbd;
    }
    return fe;
}

