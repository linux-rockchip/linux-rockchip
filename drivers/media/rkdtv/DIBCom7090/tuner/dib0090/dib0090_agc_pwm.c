#include <adapter/frontend.h>
#include <tuner/dib0090.h>
#include "dib0090_priv.h"

#ifdef CONFIG_DIB0090_USE_PWM_AGC

/* PWM (& PWM-like) AGC function */
static void dib0090_set_rframp_pwm(struct dib0090_state *state, const uint16_t *cfg)
{
    state->rf_ramp = cfg;
    dib0090_write_reg(state, RF_RAMP1, 0xffff); //(state->config->io.clock_khz / cfg[1]) * 100);

    dbgp("total RF gain: %ddB, step: %d\n", (uint32_t) cfg[0], dib0090_read_reg(state, RF_RAMP1));

    dib0090_write_regs(state, RF_RAMP3, cfg + 3, 6);
    dib0090_write_regs(state, GAIN4_1,  cfg + 9, 2);
}

static void dib0090_set_bbramp_pwm(struct dib0090_state *state, const uint16_t *cfg)
{
    state->bb_ramp = cfg;

    if(!state->identity.in_soc)
        dib0090_set_boost(state, cfg[0] > 500); /* we want the boost if the gain is higher than 50dB */

    dib0090_write_reg(state, BB_RAMP1, 0xffff); //(state->config->io.clock_khz / cfg[1]) * 100);
    dbgp("total BB gain: %ddB, step: %d\n", (uint32_t) cfg[0], dib0090_read_reg(state, BB_RAMP1));
    dib0090_write_regs(state, BB_RAMP3, cfg + 3, 4);
}

/* PWM ramp definition for SOCs */
#ifdef CONFIG_TUNER_DIB0090_P1G_SOCX090_SUPPORT
static const uint16_t bb_ramp_pwm_normal_socs[] =
{
    550, /* max BB gain in 10th of dB */
    (1<<9) | 8, /* ramp_slope = 1dB of gain -> clock_ticks_per_db = clk_khz / ramp_slope -> BB_RAMP2 */
    440,
    (4  << 9) | 0, /* BB_RAMP3 = 26dB */
    (0  << 9) | 208, /* BB_RAMP4 */
    (4  << 9) | 208, /* BB_RAMP5 = 29dB */
    (0  << 9) | 440, /* BB_RAMP6 */
};

static const uint16_t rf_ramp_pwm_cband_7090p[] =
{
    280, /* max RF gain in 10th of dB */
    18, /* ramp_slope = 1dB of gain -> clock_ticks_per_db = clk_khz / ramp_slope -> RF_RAMP2 */
    504, /* ramp_max = maximum X used on the ramp */
    (29 << 10) | 364, /* RF_RAMP5, LNA 1 = 8dB */
    (0  << 10) | 504, /* RF_RAMP6, LNA 1 */
    (60 << 10) | 228, /* RF_RAMP7, LNA 2 = 7.7dB */
    (0  << 10) | 364, /* RF_RAMP8, LNA 2 */
    (34 << 10) | 109, /* GAIN_4_1, LNA 3 = 6.8dB */
    (0  << 10) | 228, /* GAIN_4_2, LNA 3 */
    (37 << 10) | 0, /* RF_RAMP3, LNA 4 = 6.2dB */
    (0  << 10) | 109, /* RF_RAMP4, LNA 4 */
};

static const uint16_t rf_ramp_pwm_cband_7090e_sensitivity[] =
{
    186, /* max RF gain in 10th of dB */
    40, /* ramp_slope = 1dB of gain -> clock_ticks_per_db = clk_khz / ramp_slope -> RF_RAMP2 */
    746, /* ramp_max = maximum X used on the ramp */
    (10 << 10) | 345, /* RF_RAMP5, LNA 1 = 10dB */
    (0  << 10) | 746, /* RF_RAMP6, LNA 1 */
    (0 << 10) | 0, /* RF_RAMP7, LNA 2 = 0 dB */
    (0  << 10) | 0, /* RF_RAMP8, LNA 2 */
    (28 << 10) | 200, /* GAIN_4_1, LNA 3 = 6.8dB */ /* 3.61 dB */
    (0  << 10) | 345, /* GAIN_4_2, LNA 3 */
    (20 << 10) | 0, /* RF_RAMP3, LNA 4 = 6.2dB */ /* 4.96 dB */
    (0  << 10) | 200, /* RF_RAMP4, LNA 4 */
};

static const uint16_t rf_ramp_pwm_cband_7090e_aci[] =
{
    86, /* max RF gain in 10th of dB */
    40, /* ramp_slope = 1dB of gain -> clock_ticks_per_db = clk_khz / ramp_slope -> RF_RAMP2 */
    345, /* ramp_max = maximum X used on the ramp */
    (0 << 10) | 0, /* RF_RAMP5, LNA 1 = 8dB */ /* 7.47 dB */
    (0 << 10) | 0, /* RF_RAMP6, LNA 1 */
    (0 << 10) | 0, /* RF_RAMP7, LNA 2 = 0 dB */
    (0 << 10) | 0, /* RF_RAMP8, LNA 2 */
    (28 << 10) | 200, /* GAIN_4_1, LNA 3 = 6.8dB */ /* 3.61 dB */
    (0  << 10) | 345, /* GAIN_4_2, LNA 3 */
    (20 << 10) | 0, /* RF_RAMP3, LNA 4 = 6.2dB */ /* 4.96 dB */
    (0  << 10) | 200, /* RF_RAMP4, LNA 4 */
};

static const uint16_t rf_ramp_pwm_cband_8090[] =
{
    345, /* max RF gain in 10th of dB */
    29, /* ramp_slope = 1dB of gain -> clock_ticks_per_db = clk_khz / ramp_slope -> RF_RAMP2 */
    1000, /* ramp_max = maximum X used on the ramp */
    (35 << 10) | 772, /* RF_RAMP3, LNA 1 = 8dB */
    (0  << 10) | 1000, /* RF_RAMP4, LNA 1 */
    (58 << 10) | 496, /* RF_RAMP5, LNA 2 = 9.5dB */
    (0  << 10) | 772, /* RF_RAMP6, LNA 2 */
    (27 << 10) | 200, /* RF_RAMP7, LNA 3 = 10.5dB */
    (0  << 10) | 496, /* RF_RAMP8, LNA 3 */
    (40 << 10) | 0, /* GAIN_4_1, LNA 4 = 7dB */
    (0  << 10) | 200, /* GAIN_4_2, LNA 4 */
};

static const uint16_t rf_ramp_pwm_uhf_7090[] = {
    407, /* max RF gain in 10th of dB */
    13, /* ramp_slope = 1dB of gain -> clock_ticks_per_db = clk_khz / ramp_slope -> RF_RAMP2 */
    529, /* ramp_max = maximum X used on the ramp */
    (23 << 10) | 0, /* RF_RAMP3, LNA 1 = 14.7dB */
    (0  << 10) | 176, /* RF_RAMP4, LNA 1 */
    (63 << 10) | 400, /* RF_RAMP5, LNA 2 = 8dB */
    (0  << 10) | 529, /* RF_RAMP6, LNA 2 */
    (48 << 10) | 316, /* RF_RAMP7, LNA 3 = 6.8dB */
    (0  << 10) | 400, /* RF_RAMP8, LNA 3 */
    (29 << 10) | 176, /* GAIN_4_1, LNA 4 = 11.5dB */
    (0  << 10) | 316, /* GAIN_4_2, LNA 4 */
};

static const uint16_t rf_ramp_pwm_uhf_8090[] = {
    388, /* max RF gain in 10th of dB */
    26, /* ramp_slope = 1dB of gain -> clock_ticks_per_db = clk_khz / ramp_slope -> RF_RAMP2 */
    1008, /* ramp_max = maximum X used on the ramp */
    (11 << 10) | 0, /* RF_RAMP3, LNA 1 = 14.7dB */
    (0  << 10) | 369, /* RF_RAMP4, LNA 1 */
    (41 << 10) | 809, /* RF_RAMP5, LNA 2 = 8dB */
    (0  << 10) | 1008, /* RF_RAMP6, LNA 2 */
    (27 << 10) | 659, /* RF_RAMP7, LNA 3 = 6dB */
    (0  << 10) | 809, /* RF_RAMP8, LNA 3 */
    (14 << 10) | 369, /* GAIN_4_1, LNA 4 = 11.5dB */
    (0  << 10) | 659, /* GAIN_4_2, LNA 4 */
};
#endif

/* GENERAL PWM ramp definition for all other Krosus */
static const uint16_t bb_ramp_pwm_normal[] =
{
    500, /* max BB gain in 10th of dB */
    8, /* ramp_slope = 1dB of gain -> clock_ticks_per_db = clk_khz / ramp_slope -> BB_RAMP2 */
    400,
    (2  << 9) | 0, /* BB_RAMP3 = 21dB */
    (0  << 9) | 168, /* BB_RAMP4 */
    (2  << 9) | 168, /* BB_RAMP5 = 29dB */
    (0  << 9) | 400, /* BB_RAMP6 */
};

static const uint16_t bb_ramp_pwm_boost[] =
{
    550, /* max BB gain in 10th of dB */
    8, /* ramp_slope = 1dB of gain -> clock_ticks_per_db = clk_khz / ramp_slope -> BB_RAMP2 */
    440,
    (2  << 9) | 0, /* BB_RAMP3 = 26dB */
    (0  << 9) | 208, /* BB_RAMP4 */
    (2  << 9) | 208, /* BB_RAMP5 = 29dB */
    (0  << 9) | 440, /* BB_RAMP6 */
};

static const uint16_t rf_ramp_pwm_cband[] =
{
    314, /* max RF gain in 10th of dB */
    33, /* ramp_slope = 1dB of gain -> clock_ticks_per_db = clk_khz / ramp_slope -> RF_RAMP2 */
    1023, /* ramp_max = maximum X used on the ramp */
    (8  << 10) | 743, /* RF_RAMP3, LNA 1 = 0dB */
    (0  << 10) | 1023, /* RF_RAMP4, LNA 1 */
    (15 << 10) | 469, /* RF_RAMP5, LNA 2 = 0dB */
    (0  << 10) | 742, /* RF_RAMP6, LNA 2 */
    (9  << 10) | 234, /* RF_RAMP7, LNA 3 = 0dB */
    (0  << 10) | 468, /* RF_RAMP8, LNA 3 */
    (9  << 10) | 0, /* GAIN_4_1, LNA 4 = 0dB */
    (0  << 10) | 233, /* GAIN_4_2, LNA 4 */
};

static const uint16_t rf_ramp_pwm_vhf[] = {
    398, /* max RF gain in 10th of dB */
    24, /* ramp_slope = 1dB of gain -> clock_ticks_per_db = clk_khz / ramp_slope -> RF_RAMP2 */
    954, /* ramp_max = maximum X used on the ramp */
    (7  << 10) | 0, /* RF_RAMP3, LNA 1 = 13.2dB */
    (0  << 10) | 290, /* RF_RAMP4, LNA 1 */
    (16 << 10) | 699, /* RF_RAMP5, LNA 2 = 10.5dB */
    (0  << 10) | 954, /* RF_RAMP6, LNA 2 */
    (17 << 10) | 580, /* RF_RAMP7, LNA 3 = 5dB */
    (0  << 10) | 699, /* RF_RAMP8, LNA 3 */
    (7  << 10) | 290, /* GAIN_4_1, LNA 4 = 12.5dB */
    (0  << 10) | 580, /* GAIN_4_2, LNA 4 */
};

static const uint16_t rf_ramp_pwm_uhf[] = {
    398, /* max RF gain in 10th of dB */
    24, /* ramp_slope = 1dB of gain -> clock_ticks_per_db = clk_khz / ramp_slope -> RF_RAMP2 */
    954, /* ramp_max = maximum X used on the ramp */
    (7  << 10) | 0, /* RF_RAMP3, LNA 1 = 13.2dB */
    (0  << 10) | 290, /* RF_RAMP4, LNA 1 */
    (16 << 10) | 699, /* RF_RAMP5, LNA 2 = 10.5dB */
    (0  << 10) | 954, /* RF_RAMP6, LNA 2 */
    (17 << 10) | 580, /* RF_RAMP7, LNA 3 = 5dB */
    (0  << 10) | 699, /* RF_RAMP8, LNA 3 */
    (7  << 10) | 290, /* GAIN_4_1, LNA 4 = 12.5dB */
    (0  << 10) | 580, /* GAIN_4_2, LNA 4 */
};

static const uint16_t rf_ramp_pwm_sband[] =
{
    253, /* max RF gain in 10th of dB */
    38, /* ramp_slope = 1dB of gain -> clock_ticks_per_db = clk_khz / ramp_slope -> RF_RAMP2 */
    961,
    (4  << 10) | 0, /* RF_RAMP3, LNA 1 = 14.1dB */
    (0  << 10) | 508, /* RF_RAMP4, LNA 1 */
    (9  << 10) | 508, /* RF_RAMP5, LNA 2 = 11.2dB */
    (0  << 10) | 961, /* RF_RAMP6, LNA 2 */
    (0  << 10) | 0, /* RF_RAMP7, LNA 3 = 0dB */
    (0  << 10) | 0, /* RF_RAMP8, LNA 3 */
    (0  << 10) | 0, /* GAIN_4_1, LNA 4 = 0dB */
    (0  << 10) | 0, /* GAIN_4_2, LNA 4 */
};

int dib0090_update_rframp_7090(struct dibFrontend *fe, uint8_t cfg_sensitivity)
{
#ifdef CONFIG_TUNER_DIB0090_P1G_SOCX090_SUPPORT
    struct dib0090_state *state = fe->tuner_priv;

    if((!state->identity.p1g) || (!state->identity.in_soc) || ((state->identity.version != SOC_7090_P1G_21R1) && (state->identity.version != SOC_7090_P1G_11R1))) {
        dbgpHal("dib0090_update_rframp_7090() function can only be used for dib7090P\n");
        return DIB_RETURN_ERROR;
    }

    if (cfg_sensitivity)
      state->rf_ramp = (const uint16_t *)&rf_ramp_pwm_cband_7090e_sensitivity;
    else
        state->rf_ramp = (const uint16_t *)&rf_ramp_pwm_cband_7090e_aci;
    dib0090_pwm_gain_reset(fe, NULL);

    return DIB_RETURN_SUCCESS;
#else
    dbgpHal("dib0090_update_rframp_7090() function can only be used for dib7090P\n");
    return DIB_RETURN_ERROR;
#endif
}

extern void dib0090_set_dc_servo(struct dibFrontend *fe, uint8_t DC_servo_cutoff)
{
    struct dib0090_state *state = fe->tuner_priv;
    if(DC_servo_cutoff < 4)
      dib0090_write_reg(state, CTRL_BB_4, DC_servo_cutoff); /*0 = 1KHz ; 1 = 50Hz ; 2 = 150Hz ; 3 = 50KHz ; 4 = servo fast*/
}

extern void dib0090_pwm_gain_reset(struct dibFrontend *fe, struct dibChannel *ch)
{
    struct dib0090_state *state = fe->tuner_priv;
    uint16_t * bb_ramp = (uint16_t *)&bb_ramp_pwm_normal; /* default baseband config */
    uint16_t * rf_ramp = NULL;
    uint8_t en_pwm_rf_mux = 1;

    /* reset the AGC */
    if (state->config->use_pwm_agc) {
#ifdef CONFIG_BAND_SBAND
        if (state->current_band == BAND_SBAND) {
            bb_ramp = (uint16_t *)&bb_ramp_pwm_boost;
            rf_ramp = (uint16_t *)&rf_ramp_pwm_sband;
        } else
#endif

#ifdef CONFIG_BAND_CBAND
	if (state->current_band == BAND_CBAND) {
#ifdef CONFIG_TUNER_DIB0090_P1G_SOCX090_SUPPORT
            if(state->identity.in_soc) {
                bb_ramp = (uint16_t *)&bb_ramp_pwm_normal_socs;
                if(state->identity.version == SOC_8090_P1G_11R1 || state->identity.version == SOC_8090_P1G_21R1)
                    rf_ramp = (uint16_t *)&rf_ramp_pwm_cband_8090;
                else if(state->identity.version == SOC_7090_P1G_11R1 || state->identity.version == SOC_7090_P1G_21R1) {
                    if (state->config->is_dib7090e) {
                        if (state->rf_ramp == NULL)
                            rf_ramp = (uint16_t *)&rf_ramp_pwm_cband_7090e_sensitivity;
                        else
                            rf_ramp = (uint16_t *)state->rf_ramp;
                    } else
                        rf_ramp = (uint16_t *)&rf_ramp_pwm_cband_7090p;
                }
            } else
#endif
                rf_ramp = (uint16_t *)&rf_ramp_pwm_cband;
        } else
#endif

#ifdef CONFIG_BAND_VHF
	if (state->current_band == BAND_VHF) {
#ifdef CONFIG_TUNER_DIB0090_P1G_SOCX090_SUPPORT
            if(state->identity.in_soc) {
                bb_ramp = (uint16_t *)&bb_ramp_pwm_normal_socs;
                //rf_ramp = &rf_ramp_pwm_vhf_socs; /* TODO */
            } else
#endif
                rf_ramp = (uint16_t *)&rf_ramp_pwm_vhf;
        } else
#endif

#ifdef CONFIG_BAND_UHF
        if (state->current_band == BAND_UHF) {
#ifdef CONFIG_TUNER_DIB0090_P1G_SOCX090_SUPPORT
            if(state->identity.in_soc) {
                bb_ramp = (uint16_t *)&bb_ramp_pwm_normal_socs;
                if(state->identity.version == SOC_8090_P1G_11R1 || state->identity.version == SOC_8090_P1G_21R1)
                    rf_ramp = (uint16_t *)&rf_ramp_pwm_uhf_8090;
                else if(state->identity.version == SOC_7090_P1G_11R1 || state->identity.version == SOC_7090_P1G_21R1)
                    rf_ramp = (uint16_t *)&rf_ramp_pwm_uhf_7090;
            } else
#endif
            rf_ramp = (uint16_t *)&rf_ramp_pwm_uhf;
        }
#endif
        if(rf_ramp)
            dib0090_set_rframp_pwm(state, rf_ramp);
        dib0090_set_bbramp_pwm(state, bb_ramp);

        /* activate the ramp generator using PWM control */
        dbgpHal("ramp RF gain = %d BAND = %s version = %d\n",state->rf_ramp[0], (state->current_band == BAND_CBAND)?"CBAND":"NOT CBAND", state->identity.version & 0x1f);

        if((state->rf_ramp[0] == 0) || (state->current_band == BAND_CBAND && (state->identity.version & 0x1f) <= P1D_E_F)) {
            dbgpHal("DE-Engage mux for direct gain reg control\n");
            en_pwm_rf_mux = 0;
        } else
            dbgpHal("Engage mux for PWM control\n");

        dib0090_write_reg(state, RF_RAMP9, (en_pwm_rf_mux << 12) | (en_pwm_rf_mux << 11));

        /* Set fast servo cutoff to start AGC; 0 = 1KHz ; 1 = 50Hz ; 2 = 150Hz ; 3 = 50KHz ; 4 = servo fast*/
        if(state->identity.version == SOC_7090_P1G_11R1 || state->identity.version == SOC_7090_P1G_21R1)
            dib0090_write_reg(state, CTRL_BB_4, 3);
        else
            dib0090_write_reg(state, CTRL_BB_4, 1);
	dib0090_write_reg(state, BB_RAMP7, (1<< 10));// 0 gain by default

    }
}

#endif
