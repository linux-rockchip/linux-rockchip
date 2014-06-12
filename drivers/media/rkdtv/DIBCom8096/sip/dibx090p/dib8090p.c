#include <adapter/frontend.h>
#include <demod/dib8000p.h>
#include <tuner/dib0090.h>
#include <sip/dibx090p.h>
#include "dibx090p_priv.h"

#ifdef CONFIG_BUILD_HOST
//#define DYN_DEBUG_FULL_RF_LIST     // ONLY FOR DRIVER DEBUG : Use of double and printf to log prediv/loopdiv found couples !!!
#endif

static const struct dibDebugObject dib8090p_dbg = {
    DEBUG_SIP,
    "DiB8090P"
};

static int dib8090p_get_adc_power(struct dibFrontend *fe)
{
    return dib8000p_get_adc_power(fe, 1);
}

#ifdef CONFIG_SIP_DIB8090P_AUTO_ADC_FREQ
static int dib8090_get_best_sampling(struct dibFrontend *fe , struct dibChannel *ch, struct dibx090p_best_adc *adc)
{
    uint8_t spur = 0, prediv = 0, loopdiv = 0, min_prediv = 1, max_prediv = 1;
    struct dibx090p_state *state = fe->sip->priv;

    uint16_t xtal= state->cfg->clock_khz;
    uint16_t fcp_min = 1900;  /* PLL, Minimum Frequency of phase comparator (KHz) */
    uint16_t fcp_max = 20000; /* PLL, Maximum Frequency of phase comparator (KHz) */
    uint32_t fmem_max = 140000; /* 140MHz max SDRAM freq */
    uint32_t fdem_min = 66000;
    uint32_t fcp = 0, fs = 0, fdem = 0, fmem = 0;
    uint32_t harmonic_id = 0;

	 adc->timf = 0;
    adc->pll_loopdiv = loopdiv;
    adc->pll_prediv = prediv;

#ifdef DYN_DEBUG_FULL_RF_LIST
    int bw;
    uint32_t channel_spacing = 6000;
    for(bw = 6000 ; bw <= 8000 ; bw += 1000) {
        ch->bandwidth_kHz = bw;
#endif
        dbgpl(&dib8090p_dbg, "FE%d : ch->bandwidth_kHz = %d", fe->id, ch->bandwidth_kHz);

        /* Find Min and Max prediv */
        while((xtal/max_prediv) >= fcp_min)
            max_prediv++;

        max_prediv--;
        min_prediv = max_prediv;
        while((xtal/min_prediv) <= fcp_max) {
            min_prediv--;
            if(min_prediv == 1)
                break;
        }
        dbgpl(&dib8090p_dbg, "FE%d : MIN prediv = %d : MAX prediv = %d", fe->id, min_prediv, max_prediv);

        min_prediv = 1;

#ifdef DYN_DEBUG_FULL_RF_LIST
        ch->RF_kHz = 93143 - channel_spacing;
        printf("ch->RF_kHz KHz; bw KHz ; Xtal KHz;fmem KHz;fdem KHz; fs KHz;prediv;loopdiv; Computed Timf; Timf ppm\n");

        while (ch->RF_kHz < 803143) {
            ch->RF_kHz += channel_spacing;
            if(ch->RF_kHz == (159143 + channel_spacing))
                ch->RF_kHz = 167143;
            else if(ch->RF_kHz == (191143 + channel_spacing))
                ch->RF_kHz = 195143;
            else if(ch->RF_kHz == (375143 + channel_spacing))
                ch->RF_kHz = 381143;
            else if(ch->RF_kHz == (465143 + channel_spacing))
                ch->RF_kHz = 473143;
#endif

            for(prediv = min_prediv ; prediv < max_prediv ; prediv ++) {
                fcp = xtal / prediv;
                if(fcp > fcp_min && fcp < fcp_max) {
                    for(loopdiv = 1 ; loopdiv < 64 ; loopdiv++) {
                        fmem = ((xtal/prediv) * loopdiv);
                        fdem = fmem / 2;
                        fs   = fdem / 4;

                        /* test min/max system restrictions */
                        if((fdem >= fdem_min) && (fmem <= fmem_max) && (fs >= ch->bandwidth_kHz)) {
                            spur = 0;
                            /* test fs harmonics positions */
                            for(harmonic_id = (ch->RF_kHz / fs) ;  harmonic_id <= ((ch->RF_kHz / fs)+1) ; harmonic_id++) {
                                if(((fs*harmonic_id) >= (ch->RF_kHz - (ch->bandwidth_kHz/2))) &&  ((fs*harmonic_id) <= (ch->RF_kHz + (ch->bandwidth_kHz/2)))) {
                                    spur = 1;
                                    break;
                                }
                            }

                            if(!spur) {
                                adc->pll_loopdiv = loopdiv;
                                adc->pll_prediv = prediv;
                                adc->timf = (4260880253U/fdem)*(1<<8); //4260880253 = (2000*512/63)*2**18 ; 1<<8 = 2**(26-18)
                                adc->timf+= ((4260880253U%fdem)<< 8)/fdem;

#ifndef DYN_DEBUG_FULL_RF_LIST
                                dbgpl(&dib8090p_dbg, "FE%d : RF %6d; BW %6d; Xtal %6d; Fmem %6d; Fdem %6d; Fs %6d; Prediv %2d; Loopdiv %2d; Timf %8d;",fe->id, ch->RF_kHz, ch->bandwidth_kHz, xtal, fmem, fdem, fs, prediv, loopdiv, adc->timf);
                                break;
#else
                                double timf = (2.0*(512.0/63.0)/(double)(fdem/1000.0))*(1<<26);
                                double ppm = 1e6 * (((double) adc->timf / timf) - 1);
                                printf("%6d; %6d; %6d; %6d; %6d; %6d; %2d; %2d; %8d; %5.2f;\n",ch->RF_kHz, ch->bandwidth_kHz, xtal, fmem, fdem, fs, prediv, loopdiv, adc->timf, ppm);
#endif
                            }
                        }
                    }
                }
#ifndef DYN_DEBUG_FULL_RF_LIST
                if(!spur)
                    break;
#endif
            }

#ifdef DYN_DEBUG_FULL_RF_LIST
        }
    }
#endif
    if(adc->pll_loopdiv == 0 && adc->pll_prediv == 0) {
        //printf("%d ; %d ; %d : FAILLED !!!\n",ch->RF_kHz, prediv, loopdiv);
        return DIB_RETURN_ERROR;
    }
    else
        return DIB_RETURN_SUCCESS;
}
#else
struct dibx090p_adc dib8090p_adc_tab[] = {
    { 50000, 17043521, 16, 3}, /* 64 MHz */
/*
    { 58000, 17043521, 16, 3}, // 64 MHz
    { 66000, 18179756,  5, 1}, // 60 MHz
    { 74000, 18179756,  5, 1}, // 60 MHz
    { 82000, 18179756,  5, 1}, // 60 MHz
    { 90000, 17043521, 16, 3}, // 64 MHz
    { 98000, 18179756,  5, 1}, // 60 MHz
    {106000, 17043521, 16, 3}, // 64 MHz
    {114000, 17043521, 16, 3}, // 64 MHz
    {122000, 17043521, 16, 3}, // 64 MHz
    {130000, 18179756,  5, 1}, // 60 MHz
    {138000, 18179756,  5, 1}, // 60 MHz
    {146000, 17043521, 16, 3}, // 64 MHz
    {154000, 17043521, 16, 3}, // 64 MHz
    {162000, 18179756,  5, 1}, // 60 MHz
    {170000, 17043521, 16, 3}, // 64 MHz
    {178000, 17043521, 16, 3}, // 64 MHz
    {186000, 17043521, 16, 3}, // 64 MHz
    {194000, 18179756,  5, 1}, // 60 MHz
    {202000, 18179756,  5, 1}, // 60 MHz
    {210000, 17043521, 16, 3}, // 64 MHz
    {218000, 18179756,  5, 1}, // 60 MHz
    {226000, 18179756,  5, 1}, // 60 MHz
    {234000, 17043521, 16, 3}, // 64 MHz
    {242000, 17043521, 16, 3}, // 64 MHz
    {250000, 18179756,  5, 1}, // 60 MHz
    {258000, 18179756,  5, 1}, // 60 MHz
    {266000, 17043521, 16, 3}, // 64 MHz
    {274000, 17043521, 16, 3}, // 64 MHz
    {282000, 18179756,  5, 1}, // 60 MHz
    {290000, 18179756,  5, 1}, // 60 MHz
    {298000, 17043521, 16, 3}, // 64 MHz
    {306000, 18179756,  5, 1}, // 60 MHz
    {314000, 18179756,  5, 1}, // 60 MHz
    {322000, 18179756,  5, 1}, // 60 MHz
    {330000, 18179756,  5, 1}, // 60 MHz
    {338000, 18179756,  5, 1}, // 60 MHz
    {346000, 18179756,  5, 1}, // 60 MHz
    {354000, 18179756,  5, 1}, // 60 MHz
    {362000, 17043521, 16, 3}, // 64 MHz
    {370000, 18179756,  5, 1}, // 60 MHz
*/
    {878000, 16527051, 11, 1}, /* 66 MHz */
    {0xffffffff,    0,  0, 0}, /* 60 MHz */
};
#endif

static int dib8090p_agc_startup(struct dibFrontend *fe, struct dibChannel *channel)
{
    struct dibx000_bandwidth_config pll;
    uint16_t target;
#ifdef CONFIG_SIP_DIB8090P_AUTO_ADC_FREQ
    struct dibx090p_best_adc adc;
#else
    int better_sampling_freq = 0;
    struct dibx090p_adc *adc_table = &dib8090p_adc_tab[0];
#endif

   DibZeroMemory(&pll, sizeof(struct dibx000_bandwidth_config));

    if (fe->tune_state == CT_AGC_START) {
        dib0090_pwm_gain_reset(fe, channel);
	/* dib0090_get_wbd_target is returning any possible temperature compensated wbd-target */
	target = (dib0090_get_wbd_target(fe, channel->RF_kHz) * 8  + 1) / 2;
	demod_set_wbd_ref(fe, target);

#ifdef CONFIG_SIP_DIB8090P_AUTO_ADC_FREQ
        if(dib8090_get_best_sampling(fe , channel, &adc) == DIB_RETURN_SUCCESS) {
            pll.pll_ratio  = adc.pll_loopdiv;
            pll.pll_prediv = adc.pll_prediv;
            dib8000p_update_pll(fe, &pll);
            fe->demod_info->ops.ctrl_timf(fe, DEMOD_TIMF_SET, adc.timf);
        }
#else
        while(channel->RF_kHz > adc_table->freq) {
            better_sampling_freq = 1;
            adc_table++;
        }

        if((adc_table->freq != 0xffffffff) && better_sampling_freq) {
            pll.pll_ratio  = adc_table->pll_loopdiv;
            pll.pll_prediv = adc_table->pll_prediv;
            dib8000p_update_pll(fe, &pll);
            fe->demod_info->ops.ctrl_timf(fe, DEMOD_TIMF_SET, adc_table->timf);
        }
#endif
    }
    return dib8000p_agc_startup(fe, channel);
}

static const struct dib0090_wbd_slope dib8090p_wbd_table[] = {
    { 380,   81, 850, 64, 540 ,4},
    { 860,   51, 866, 21,  375 ,4},
    {1700,    0, 250, 0,   100, 6}, //LBAND Predefinition , NOT tested Yet
    {2600,    0, 250, 0,   100, 6}, //SBAND Predefinition , NOT tested Yet
    { 0xFFFF, 0,   0, 0,   0   ,0},
};

const struct dibx000_agc_config dib8090p_agc_config[2] = {
    {
        BAND_UHF,
        /* P_agc_use_sd_mod1=0, P_agc_use_sd_mod2=0, P_agc_freq_pwm_div=1, P_agc_inv_pwm1=0, P_agc_inv_pwm2=0,
         * P_agc_inh_dc_rv_est=0, P_agc_time_est=3, P_agc_freeze=0, P_agc_nb_est=5, P_agc_write=0 */
        (0 << 15) | (0 << 14) | (5 << 11) | (0 << 10) | (0 << 9) | (0 << 8) | (3 << 5) | (0 << 4) | (5 << 1) | (0 << 0), // setup

        684,// inv_gain = 1/ 95.7dB // no boost, lower gain due to ramp quantification
        10,  // time_stabiliz

        0,  // alpha_level
        118,  // thlock

        0,     // wbd_inv
        1200,  // wbd_ref
        3,     // wbd_sel
        5,     // wbd_alpha

        65535,  // agc1_max
        0,      // agc1_min

        32767,  // agc2_max  : half of the full range because of soc HW bug
        0,      // agc2_min

        0,      // agc1_pt1
        0,      // agc1_pt2
        105,     // agc1_pt3  // 38.8dB
        0,      // agc1_slope1
        156,    // agc1_slope2 = Dy/Dx * 2**6 * 2**8 = 1/105 * 2**6 *2**8 : Dy = 1
        105,     // agc2_pt1
        255,    // agc2_pt2
        54,     // agc2_slope1 = Dy/Dx * 2**6 * 2**8 = 1/(255-105) * 2**6 *2**8 : Dy = 0.5 because of soc HW bug
        0,      // agc2_slope2

        28,  // alpha_mant // 5Hz with 90.2dB
        26,  // alpha_exp
        31,  // beta_mant
        51,  // beta_exp

        0,  // perform_agc_softsplit
    } , {
        BAND_FM | BAND_VHF | BAND_CBAND,
        /* P_agc_use_sd_mod1=0, P_agc_use_sd_mod2=0, P_agc_freq_pwm_div=1, P_agc_inv_pwm1=0, P_agc_inv_pwm2=0,
         * P_agc_inh_dc_rv_est=0, P_agc_time_est=3, P_agc_freeze=0, P_agc_nb_est=5, P_agc_write=0 */
        (0 << 15) | (0 << 14) | (5 << 11) | (0 << 10) | (0 << 9) | (0 << 8) | (3 << 5) | (0 << 4) | (5 << 1) | (0 << 0), // setup

        732,// inv_gain = 1/ 89.5dB // no boost, lower gain due to ramp quantification
        10,  // time_stabiliz

        0,  // alpha_level
        118,  // thlock

        0,     // wbd_inv
        1200,  // wbd_ref
        3,     // wbd_sel
        5,     // wbd_alpha

        65535,  // agc1_max : 1
        0,      // agc1_min

        32767,  // agc2_max :0.5  half of the full range because of soc HW bug
        0,      // agc2_min

        0,      // agc1_pt1
        0,      // agc1_pt2
        98,     // agc1_pt3  // 34.5dB CBAND P1G + 55dB BB boost = 89.5dB
        0,      // agc1_slope1
        167,    // agc1_slope2 = Dy/Dx * 2**6 * 2**8 = 1/85 * 2**6 *2**8 : Dy = 1
        98,     // agc2_pt1
        255,    // agc2_pt2
        52,     // agc2_slope1 = Dy/Dx * 2**6 * 2**8 = 0.5/(255-85) * 2**6 *2**8 : Dy = 0.5
        0,      // agc2_slope2

        28,  // alpha_mant // 5Hz with 89.5dB
        26,  // alpha_exp
        31,  // beta_mant
        51,  // beta_exp

        0,  // perform_agc_softsplit
    }
};

static const struct dib8000p_config default_dib8000p_config = {
    0, // output_mpeg2_in_188_bytes
    0, // hostbus_diversity
    dibx090p_update_lna, // update_lna

    2, // agc_config_count
    dib8090p_agc_config,
    NULL,

    DIB8000P_GPIO_DEFAULT_DIRECTIONS, // gpio_dir
    DIB8000P_GPIO_DEFAULT_VALUES,  // gpio_val
    DIB8000P_GPIO_DEFAULT_PWM_POS, // gpio_pwm_pos
    0,

    NULL,
};

static const struct dib0090_config default_dib0090_config= {
    {
        12000,
    },

    dibx090p_tuner_sleep,
    dibx090p_tuner_sleep,

    /*  offset in kHz */
    0, //freq_offset_khz_uhf; => copy from dib8090p_config into register function
    0, //freq_offset_khz_vhf; => copy from dib8090p_config into register function

    dib8090p_get_adc_power,

    1, //clkouttobamse : 1; /* activate or deactivate clock output */
    0, //analog_output;

    0, //i2c_address;

    0, //_unused0; /* was wbd-value - now unused */
    0, // force_cband_input
    1, //use_pwm_agc;
    0, //clkoutdrive;

    1, //fref_clock_ratio;
    0,// cap_value;

    dib8090p_wbd_table, //Wbd table

    0,
    0,
    dibx090p_get_dc_power,
    NULL,
    1, /* to signify from layout/sip that the tuner is inside a SOC */
};

static const struct dibx000_bandwidth_config dib8090p_clock_config_12_mhz = {
	108000, 13500, // internal, sampling
	1, 9, 1, 0, 0, // pll_cfg: prediv, ratio, range, reset, bypass
	0, 0, 0, 0, 2, // misc: refdiv, bypclk_div, IO_CLK_en_core, ADClkSrc, modulo
	(3 << 14) | (1 << 12) | (524 << 0), // sad_cfg: refsel, sel, freq_15k
	(0 << 25) | 0, // ifreq = 0.000000 MHz
	20199729, // timf
	12000000, // xtal_hz
};

static const struct dibx000_bandwidth_config dib8090p_clock_config_30_mhz = {
	60000, 15000, // internal, sampling
	3, 6, 0, 1, 0, // pll_cfg: prediv, ratio, range, reset, bypass
	0, 0, 1, 1, 2, // misc: refdiv, bypclk_div, IO_CLK_en_core, ADClkSrc, modulo
	(3 << 14) | (1 << 12) | (524 << 0), // sad_cfg: refsel, sel, freq_15k
	(0 << 25) | 0, // ifreq = 0.000000 MHz
	20452225, // timf
	15000000, // xtal_hz
};

static const struct dibSIPInfo dib8090p_info = {
    "DiBcom DiB8090pMB",
    {
        dibx090p_release
    }
};

struct dibFrontend * dib8090p_sip_register(struct dibFrontend *fe, struct dibDataBusHost *host, uint8_t addr, const struct dibx090p_config *cfg)
{
    struct dibx090p_state *state;
    struct dib8000p_config dib8000p_cfg;

    state = MemAlloc(sizeof(struct dibx090p_state));
    if (state == NULL)
        return NULL;
    DibZeroMemory(state, sizeof(struct dibx090p_state));

    frontend_register_sip(fe, &state->info, &dib8090p_info, state);

    state->cfg = cfg;
    state->fe = fe;
    state->version = 0x8090;

    memcpy(&dib8000p_cfg, &default_dib8000p_config, sizeof(struct dib8000p_config));

    switch (cfg->clock_khz) {
        case 30000: dib8000p_cfg.pll = &dib8090p_clock_config_30_mhz; break;
        case 12000:
        default:    dib8000p_cfg.pll = &dib8090p_clock_config_12_mhz; break;
    }

    dib8000p_cfg.agc = dib8090p_agc_config;

    dib8000p_cfg.output_mpeg2_in_188_bytes = cfg->output_mpeg2_in_188_bytes;
    dib8000p_cfg.hostbus_diversity         = 1;
    dib8000p_cfg.gpio_dir                  = cfg->gpio_dir;
    dib8000p_cfg.gpio_val                  = cfg->gpio_val;
    dib8000p_cfg.gpio_pwm_pos              = cfg->gpio_pwm_pos;
    dib8000p_cfg.diversity_delay           = cfg->diversity_delay;
    dib8000p_cfg.forward_erronous_mpeg_packet = cfg->forward_erronous_mpeg_packet;

    memcpy(&state->dib0090_cfg, &default_dib0090_config, sizeof(struct dib0090_config));
    state->dib0090_cfg.freq_offset_khz_uhf       = cfg->dib0090_freq_offset_khz_uhf;
    state->dib0090_cfg.freq_offset_khz_vhf       = cfg->dib0090_freq_offset_khz_vhf;

    if (dib8000p_register(fe, host, addr, &dib8000p_cfg) == NULL)
        goto error;

    state->dib0090_cfg.get_adc_power = dib8090p_get_adc_power;
    state->dib0090_cfg.get_dc_power = dibx090p_get_dc_power;

    /* callback of set_diversity_in */
    state->set_diversity_in = state->fe->demod_info->ops.set_diversity_in;
    state->fe->demod_info->ops.set_diversity_in = dibx090p_set_diversity_in;

    /* callback of set output mode */
    state->set_output_mode = state->fe->demod_info->ops.set_output_mode;
    state->fe->demod_info->ops.set_output_mode = dibx090p_set_output_mode;


    data_bus_host_init(&state->tuner_access, DATA_BUS_DIBCTRL_IF, dibx090p_tuner_xfer, state);
    if (dib0090_register(fe, &state->tuner_access, &state->dib0090_cfg) == NULL)
        goto tuner_error;

    /* custom AGC start for DiB8090P */
    fe->demod_info->ops.agc_startup = dib8090p_agc_startup;

    return fe;

tuner_error:
    frontend_unregister_demod(fe);
error:
    MemFree(state, sizeof(struct dib8090p_state));

    return NULL;
}

