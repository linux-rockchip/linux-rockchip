#include <adapter/frontend.h>
#include <demod/dib7000p.h>
#include <tuner/dib0090.h>
#include <sip/dibx090p.h>
#include "dibx090p_priv.h"

#ifdef CONFIG_BUILD_HOST
//#define DYN_DEBUG_FULL_RF_LIST     // ONLY FOR DRIVER DEBUG : Use of double and printf to log prediv/loopdiv found couples !!!
#endif

static const struct dibDebugObject dib7090p_dbg = {
    DEBUG_SIP,
    "DiB7090p"
};

extern int dib7090p_get_adc_power(struct dibFrontend *fe)
{
    return dib7000p_get_adc_power(fe);
}

#ifdef CONFIG_SIP_DIB7090P_AUTO_ADC_FREQ
static int dib7090p_get_best_sampling(struct dibFrontend *fe , struct dibChannel *ch, struct dibx090p_best_adc *adc)
{
    uint8_t spur = 0, prediv = 0, loopdiv = 0, min_prediv = 1, max_prediv = 1;
    struct dibx090p_state *state = fe->sip->priv;

    uint16_t xtal= state->cfg->clock_khz;
    uint32_t fcp_min = 1900;  /* PLL Minimum Frequency comparator KHz */
    uint32_t fcp_max = 20000; /* PLL Maximum Frequency comparator KHz */
    uint32_t fdem_max = 76000; //76000;//70000;
    uint32_t fdem_min = 69500; //69500;//61500;
    uint32_t fcp = 0, fs = 0, fdem = 0;
    uint32_t harmonic_id = 0;

#ifdef DYN_DEBUG_FULL_RF_LIST
    int bw;
    uint32_t channel_spacing = 8000;
#endif

    adc->pll_loopdiv = loopdiv;
    adc->pll_prediv = prediv;
    adc->timf = 0;

#ifdef DYN_DEBUG_FULL_RF_LIST
    for(bw = 5000 ; bw <= 8000 ; bw += 1000) {
        ch->bandwidth_kHz = bw;
#endif
        dbgpl(&dib7090p_dbg, "ch->bandwidth_kHz = %d fdem_min =%d", ch->bandwidth_kHz, fdem_min);

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
        dbgpl(&dib7090p_dbg, "MIN prediv = %d : MAX prediv = %d", min_prediv, max_prediv);

        min_prediv = 2;

#ifdef DYN_DEBUG_FULL_RF_LIST
        ch->RF_kHz = 114000;
        printf("ch->RF_kHz KHz; bw KHz ; Xtal KHz;fdem KHz; fs KHz;prediv;loopdiv; Computed Timf; Timf ppm\n");
        while (ch->RF_kHz <= 858000) {
            ch->RF_kHz += channel_spacing;
#endif
            for(prediv = min_prediv ; prediv < max_prediv ; prediv ++) {
                fcp = xtal / prediv;
                if(fcp > fcp_min && fcp < fcp_max) {
                    for(loopdiv = 1 ; loopdiv < 64 ; loopdiv++) {
                        fdem = ((xtal/prediv) * loopdiv);
                        fs   = fdem / 4;
                        /* test min/max system restrictions */
                        //   dbgpl(&dib7090p_dbg, "fdem kHz = %d fdem_min =%d", fdem, fdem_min);

                        if((fdem >= fdem_min) && (fdem <= fdem_max) && (fs >= ch->bandwidth_kHz)) {
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
                                adc->timf = (2396745143U/fdem)*(1<<9); //4793490286 = (1000* 2 * 64/7)*2**17 ; 1<<9 = 2**(26-17)
                                adc->timf+= ((2396745143U%fdem)<< 9)/fdem;

#ifndef DYN_DEBUG_FULL_RF_LIST
                                dbgpl(&dib7090p_dbg, "RF %6d; BW %6d; Xtal %6d; Dem %6d; Fs %6d; Prediv %2d; Loopdiv %2d; Timf %8d;",ch->RF_kHz, ch->bandwidth_kHz, xtal, fdem, fs, prediv, loopdiv, adc->timf);
                                break;
#else
                                double timf = (2.0*(64.0/7.0)/(double)(fdem/1000.0))*(1<<26);
                                double ppm = 1e6 * (((double) adc->timf / timf) - 1);
                                printf("%6d; %6d; %6d; %6d; %6d; %2d; %2d; %8d; %5.2f;\n",ch->RF_kHz, ch->bandwidth_kHz, xtal, fdem, fs, prediv, loopdiv, adc->timf, ppm);
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
struct dibx090p_adc dib7090p_adc_tab[] = {
    {    56000, 16146493, 19, 3},  /* 76 MHz */
    {    58000, 17631228, 29, 5},  /* 69 MHz */
    {    67000, 17043521, 12, 2},  /* 72 MHz */
    {    73000, 16146493, 19, 3},  /* 76 MHz */
    {    76000, 17631228, 29, 5},  /* 69 MHz */
    {    85000, 17043521, 12, 2},  /* 72 MHz */
    {    91000, 16146493, 19, 3},  /* 76 MHz */
    {    94000, 17631228, 29, 5},  /* 69 MHz */
    {   103000, 17043521, 12, 2},  /* 72 MHz */
    {   109000, 16146493, 19, 3},  /* 76 MHz */
    {   112000, 17631228, 29, 5},  /* 69 MHz */
    {   121000, 17043521, 12, 2},  /* 72 MHz */
    {   128000, 16146493, 19, 3},  /* 76 MHz */
    {   130000, 17631228, 29, 5},  /* 69 MHz */
    {   139000, 17043521, 12, 2},  /* 72 MHz */
    {   147000, 16146493, 19, 3},  /* 76 MHz */
    {   148000, 17631228, 29, 5},  /* 69 MHz */
    {   157000, 17043521, 12, 2},  /* 72 MHz */
    {   166000, 16146493, 19, 3},  /* 76 MHz */
    {   175000, 17043521, 12, 2},  /* 72 MHz */
    {   184000, 16146493, 19, 3},  /* 76 MHz */
    {   193000, 17043521, 12, 2},  /* 72 MHz */
    {   194000, 16361780, 25, 4},  /* 75 MHz */
    {   202000, 16146493, 19, 3},  /* 76 MHz */
    {   211000, 17043521, 12, 2},  /* 72 MHz */
    {   213000, 16361780, 25, 4},  /* 75 MHz */
    {   220000, 16146493, 19, 3},  /* 76 MHz */
    {   229000, 17043521, 12, 2},  /* 72 MHz */
    {   232000, 16361780, 25, 4},  /* 75 MHz */
    {   238000, 16146493, 19, 3},  /* 76 MHz */
    {   247000, 17043521, 12, 2},  /* 72 MHz */
    {   251000, 16361780, 25, 4},  /* 75 MHz */
    {   256000, 16146493, 19, 3},  /* 76 MHz */
    {   265000, 17043521, 12, 2},  /* 72 MHz */
    {   266000, 17631228, 29, 5},  /* 69 MHz */
    {   270000, 16361780, 25, 4},  /* 75 MHz */
    {   274000, 16146493, 19, 3},  /* 76 MHz */
    {   283000, 17043521, 12, 2},  /* 72 MHz */
    {   285000, 17631228, 29, 5},  /* 69 MHz */
    {   289000, 16361780, 25, 4},  /* 75 MHz */
    {   292000, 16146493, 19, 3},  /* 76 MHz */
    {   301000, 17043521, 12, 2},  /* 72 MHz */
    {   304000, 17631228, 29, 5},  /* 69 MHz */
    {   308000, 16361780, 25, 4},  /* 75 MHz */
    {   310000, 16146493, 19, 3},  /* 76 MHz */
    {   319000, 17043521, 12, 2},  /* 72 MHz */
    {   322000, 17631228, 29, 5},  /* 69 MHz */
    {   327000, 16361780, 25, 4},  /* 75 MHz */
    {   328000, 16146493, 19, 3},  /* 76 MHz */
    {   337000, 17043521, 12, 2},  /* 72 MHz */
    {   341000, 17631228, 29, 5},  /* 69 MHz */
    {   346000, 16361780, 25, 4},  /* 75 MHz */
    {   355000, 17043521, 12, 2},  /* 72 MHz */
    {   356000, 16146493, 19, 3},  /* 76 MHz */
    {   360000, 17631228, 29, 5},  /* 69 MHz */
    {   364000, 16361780, 25, 4},  /* 75 MHz */
    {   373000, 17043521, 12, 2},  /* 72 MHz */
    {   375000, 16146493, 19, 3},  /* 76 MHz */
    {   378000, 17631228, 29, 5},  /* 69 MHz */
    {   379000, 16493730, 31, 5},  /* 74 MHz */
    {   382000, 16361780, 25, 4},  /* 75 MHz */
    {   391000, 17043521, 12, 2},  /* 72 MHz */
    {   394000, 16146493, 19, 3},  /* 76 MHz */
    {   396000, 17631228, 29, 5},  /* 69 MHz */
    {   397000, 16493730, 31, 5},  /* 74 MHz */
    {   400000, 16361780, 25, 4},  /* 75 MHz */
    {   409000, 17043521, 12, 2},  /* 72 MHz */
    {   413000, 16146493, 19, 3},  /* 76 MHz */
    {   416000, 16493730, 31, 5},  /* 74 MHz */
    {   418000, 16361780, 25, 4},  /* 75 MHz */
    {   427000, 17043521, 12, 2},  /* 72 MHz */
    {   432000, 16146493, 19, 3},  /* 76 MHz */
    {   435000, 16493730, 31, 5},  /* 74 MHz */
    {   436000, 16361780, 25, 4},  /* 75 MHz */
    {   445000, 17043521, 12, 2},  /* 72 MHz */
    {   451000, 16146493, 19, 3},  /* 76 MHz */
    {   454000, 16493730, 31, 5},  /* 74 MHz */
    {   463000, 17043521, 12, 2},  /* 72 MHz */
    {   470000, 16146493, 19, 3},  /* 76 MHz */
    {   472000, 16493730, 31, 5},  /* 74 MHz */
    {   481000, 17043521, 12, 2},  /* 72 MHz */
    {   489000, 16146493, 19, 3},  /* 76 MHz */
    {   490000, 16493730, 31, 5},  /* 74 MHz */
    {   499000, 17043521, 12, 2},  /* 72 MHz */
    {   508000, 16146493, 19, 3},  /* 76 MHz */
    {   517000, 17043521, 12, 2},  /* 72 MHz */
    {   526000, 16146493, 19, 3},  /* 76 MHz */
    {   535000, 17043521, 12, 2},  /* 72 MHz */
    {   536000, 16361780, 25, 4},  /* 75 MHz */
    {   544000, 16146493, 19, 3},  /* 76 MHz */
    {   553000, 17043521, 12, 2},  /* 72 MHz */
    {   555000, 16361780, 25, 4},  /* 75 MHz */
    {   562000, 16146493, 19, 3},  /* 76 MHz */
    {   571000, 17043521, 12, 2},  /* 72 MHz */
    {   574000, 16361780, 25, 4},  /* 75 MHz */
    {   580000, 16146493, 19, 3},  /* 76 MHz */
    {   589000, 17043521, 12, 2},  /* 72 MHz */
    {   593000, 16361780, 25, 4},  /* 75 MHz */
    {   598000, 16146493, 19, 3},  /* 76 MHz */
    {   607000, 17043521, 12, 2},  /* 72 MHz */
    {   612000, 16361780, 25, 4},  /* 75 MHz */
    {   616000, 16146493, 19, 3},  /* 76 MHz */
    {   625000, 17043521, 12, 2},  /* 72 MHz */
    {   631000, 16361780, 25, 4},  /* 75 MHz */
    {   634000, 16146493, 19, 3},  /* 76 MHz */
    {   643000, 17043521, 12, 2},  /* 72 MHz */
    {   650000, 16361780, 25, 4},  /* 75 MHz */
    {   652000, 16146493, 19, 3},  /* 76 MHz */
    {   661000, 17043521, 12, 2},  /* 72 MHz */
    {   669000, 16361780, 25, 4},  /* 75 MHz */
    {   670000, 16146493, 19, 3},  /* 76 MHz */
    {   679000, 17043521, 12, 2},  /* 72 MHz */
    {   688000, 16361780, 25, 4},  /* 75 MHz */
    {   697000, 17043521, 12, 2},  /* 72 MHz */
    {   698000, 16146493, 19, 3},  /* 76 MHz */
    {   706000, 16361780, 25, 4},  /* 75 MHz */
    {   715000, 17043521, 12, 2},  /* 72 MHz */
    {   717000, 16146493, 19, 3},  /* 76 MHz */
    {   724000, 16361780, 25, 4},  /* 75 MHz */
    {   733000, 17043521, 12, 2},  /* 72 MHz */
    {   736000, 16146493, 19, 3},  /* 76 MHz */
    {   742000, 16361780, 25, 4},  /* 75 MHz */
    {   751000, 17043521, 12, 2},  /* 72 MHz */
    {   755000, 16146493, 19, 3},  /* 76 MHz */
    {   760000, 16361780, 25, 4},  /* 75 MHz */
    {   769000, 17043521, 12, 2},  /* 72 MHz */
    {   774000, 16146493, 19, 3},  /* 76 MHz */
    {   778000, 16361780, 25, 4},  /* 75 MHz */
    {   787000, 17043521, 12, 2},  /* 72 MHz */
    {   793000, 16146493, 19, 3},  /* 76 MHz */
    {   796000, 16361780, 25, 4},  /* 75 MHz */
    {   805000, 17043521, 12, 2},  /* 72 MHz */
    {   812000, 16146493, 19, 3},  /* 76 MHz */
    {   814000, 16361780, 25, 4},  /* 75 MHz */
    {   823000, 17043521, 12, 2},  /* 72 MHz */
    {   831000, 16146493, 19, 3},  /* 76 MHz */
    {   832000, 16361780, 25, 4},  /* 75 MHz */
    {   841000, 17043521, 12, 2},  /* 72 MHz */
    {   850000, 16146493, 19, 3},  /* 76 MHz */
    {   0xffffffff, 17043521, 12, 2},  /* 72 MHz */
};
#endif

static int dibx090p_agc_startup(struct dibFrontend *fe, struct dibChannel *channel)
{
    struct dibx000_bandwidth_config pll;
    uint16_t target;

#ifdef CONFIG_SIP_DIB7090P_AUTO_ADC_FREQ
    struct dibx090p_best_adc adc;
#else
    int better_sampling_freq = 0;
    struct dibx090p_adc *adc_table = &dib7090p_adc_tab[0];
#endif

   DibZeroMemory(&pll, sizeof(struct dibx000_bandwidth_config));
   if (fe->tune_state == CT_AGC_START) {
       dib0090_pwm_gain_reset(fe, channel);
        /* dib0090_get_wbd_target is returning any possible temperature compensated wbd-target */
        target = (dib0090_get_wbd_target(fe, channel->RF_kHz) * 8  + 1) / 2;
        demod_set_wbd_ref(fe, target);

#ifdef CONFIG_SIP_DIB7090P_AUTO_ADC_FREQ
        if(dib7090p_get_best_sampling(fe , channel, &adc) == DIB_RETURN_SUCCESS) {
            pll.pll_ratio  = adc.pll_loopdiv;
            pll.pll_prediv = adc.pll_prediv;

            dib7000p_update_pll(fe, &pll);
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
            dib7000p_update_pll(fe, &pll);
            fe->demod_info->ops.ctrl_timf(fe, DEMOD_TIMF_SET, adc_table->timf);
        }
#endif
    }
    return dib7000p_agc_startup(fe, channel);
}

static const struct dib0090_wbd_slope dibx090p_wbd_table[] = {
    { 380,   81, 850, 64, 540 ,4},
    { 860,   51, 866, 21,  375 ,4},
    {1700,    0, 250, 0,   100, 6}, //LBAND Predefinition , NOT tested Yet
    {2600,    0, 250, 0,   100, 6}, //SBAND Predefinition , NOT tested Yet
    { 0xFFFF, 0,   0, 0,   0   ,0},
};

static const struct dib0090_wbd_slope dib7090e_wbd_table[] = {
    { 380,   81, 850, 64, 540 ,4},
    { 700,   51, 866, 21,  320 ,4},
    { 860,   48, 666, 18,  330 ,6},
    {1700,    0, 250, 0,   100, 6}, //LBAND Predefinition , NOT tested Yet
    {2600,    0, 250, 0,   100, 6}, //SBAND Predefinition , NOT tested Yet
    { 0xFFFF, 0,   0, 0,   0   ,0},
};

const struct dibx000_agc_config dibx090p_agc_config[2] = {
    {
        BAND_UHF,
        /* P_agc_use_sd_mod1=0, P_agc_use_sd_mod2=0, P_agc_freq_pwm_div=1, P_agc_inv_pwm1=0, P_agc_inv_pwm2=0,
         * P_agc_inh_dc_rv_est=0, P_agc_time_est=3, P_agc_freeze=0, P_agc_nb_est=5, P_agc_write=0 */
        (0 << 15) | (0 << 14) | (5 << 11) | (0 << 10) | (0 << 9) | (0 << 8) | (3 << 5) | (0 << 4) | (5 << 1) | (0 << 0), // setup

        687,// inv_gain = 1/ 95.4dB // no boost, lower gain due to ramp quantification
        10,  // time_stabiliz

        0,  // alpha_level
        118,  // thlock

        0,     // wbd_inv
        1200,  //3530,  // wbd_ref
        3,     // wbd_sel
        5,     // wbd_alpha

        65535,  // agc1_max
        0,      // agc1_min

        65535,  // agc2_max
        0,      // agc2_min

        0,      // agc1_pt1
        32,     // agc1_pt2
        114,    // agc1_pt3  // 40.4dB
        143,    // agc1_slope1
        144,    // agc1_slope2
        114,    // agc2_pt1
        227,    // agc2_pt2
        116,    // agc2_slope1
        117,    // agc2_slope2

        18,  // alpha_mant
        0,   // alpha_exp
        20,  // beta_mant
        59,  // beta_exp

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

        65535,  // agc2_max
        0,      // agc2_min

        0,      // agc1_pt1
        0,      // agc1_pt2
        98,     // agc1_pt3  // 34.5dB CBAND P1G + 55dB BB boost = 89.5dB
        0,      // agc1_slope1
        167,    // agc1_slope2 = Dy/Dx * 2**6 * 2**8 = 1/98 * 2**6 *2**8 : Dy = 1
        98,     // agc2_pt1
        255,    // agc2_pt2
        104,    // agc2_slope1 = Dy/Dx * 2**6 * 2**8 = 1/(255-98) * 2**6 *2**8
        0,      // agc2_slope2

        18,  // alpha_mant
        0,   // alpha_exp
        20,  // beta_mant
        59,  // beta_exp

        0,  // perform_agc_softsplit
    }
};

static const struct dib7000p_config default_dib7000_config = {
	0,    // output_mpeg2_in_188_bytes
	0,    // hostbus_diversity
	1,    // tuner_is_baseband
	dibx090p_update_lna, // update_lna

	2,  // agc_config_count
	dibx090p_agc_config,

	NULL, // bw
	0,    // lna_gain_step
	60,   // tun_max_gain
	-14,  // tun_ref_level

	DIB7000P_GPIO_DEFAULT_DIRECTIONS,
	DIB7000P_GPIO_DEFAULT_VALUES,
	DIB7000P_GPIO_DEFAULT_PWM_POS,

	0,

	dibx090p_agc_restart,

	0,   // spur_protect
        0,   // disable_sample_and_hold
        0,   // enable_current_mirror : 1;
        0,   // diversity_delay;
};

static const struct dib0090_config default_dib0090_config= {
    {
        12000,

        0, //pll_bypass

        0, //pll_range OutFreq_Khz 100MHz : pll_range = 1 else = 0
        3, //pll_prediv
        6, //pll_loopdiv


        0, //adc_clock_ratio : no longer used in 7090p
        0 //pll_int_loop_filt
    },

    dibx090p_tuner_sleep,
    dibx090p_tuner_sleep,

    /*  offset in kHz */
    0, //freq_offset_khz_uhf;
    0, //freq_offset_khz_vhf;

    dib7090p_get_adc_power,

    1, //clkouttobamse : 1; /* activate or deactivate clock output */
    0, //analog_output;

    0, //i2c_address;

    0, //_unused0; /* was wbd-value - now unused */
    0, // force_cband_input
    1, //use_pwm_agc;
    0, //clkoutdrive;

    0, //fref_clock_ratio;
    0,// cap_value;

    dibx090p_wbd_table, //Wbd table

    0,
    0,
    dibx090p_get_dc_power,
    NULL,
    1, /* to signify from layout/sip that the tuner is inside a SOC */
};

static const struct dib0090_config dib7090e_config = {
    {
        12000,

        0, //pll_bypass

        0, //pll_range OutFreq_Khz 100MHz : pll_range = 1 else = 0
        3, //pll_prediv
        6, //pll_loopdiv


        0, //adc_clock_ratio : no longer used in 7090p
        0 //pll_int_loop_filt
    },

    dibx090p_tuner_sleep,
    dibx090p_tuner_sleep,

    /*  offset in kHz */
    0, //freq_offset_khz_uhf;
    0, //freq_offset_khz_vhf;

    dib7090p_get_adc_power,

    1, //clkouttobamse : 1; /* activate or deactivate clock output */
    0, //analog_output;

    0, //i2c_address;

    0, //_unused0; /* was wbd-value - now unused */
    1, // force_cband_input
    1, //use_pwm_agc;
    0, //clkoutdrive;

    0, //fref_clock_ratio;
    0,// cap_value;

    dib7090e_wbd_table, //Wbd table

    0,
    0,
    dibx090p_get_dc_power,
    NULL,
    1, /* to signify from layout/sip that the tuner is inside a SOC */
    0,
    0,
    1, // is_dib7090e
};


static const struct dibx000_bandwidth_config dibx090p_clock_config_12_mhz = {
	60000, 15000, // internal, sampling
	1, 5, 0, 0, 0, // pll_cfg: prediv, ratio, range, reset, bypass
	0, 0, 1, 1, 2, // misc: refdiv, bypclk_div, IO_CLK_en_core, ADClkSrc, modulo
	(3 << 14) | (1 << 12) | (524 << 0), // sad_cfg: refsel, sel, freq_15k
	(0 << 25) | 0, // ifreq = 0.000000 MHz
	20452225, // timf
	15000000, // xtal_hz
};

static const struct dibx000_bandwidth_config dibx090p_clock_config_30_mhz = {
	60000, 15000, // internal, sampling
	3, 6, 0, 0, 0, // pll_cfg: prediv, ratio, range, reset, bypass
	0, 0, 1, 1, 2, // misc: refdiv, bypclk_div, IO_CLK_en_core, ADClkSrc, modulo
	(3 << 14) | (1 << 12) | (524 << 0), // sad_cfg: refsel, sel, freq_15k
	(0 << 25) | 0, // ifreq = 0.000000 MHz
	20452225, // timf
	15000000, // xtal_hz
};

static const struct dibSIPInfo dib7090p_info = {
    "DiBcom DiB7090pMB",

    {
        dibx090p_release
    }
};

static struct dibFrontend * dib7090x_sip_register(struct dibFrontend *fe, struct dibDataBusHost *host, uint8_t addr, const struct dibx090p_config *cfg, uint8_t is_7090e)
{
    struct dibx090p_state *state;
    struct dib7000p_config dib7000p_cfg;

    state = MemAlloc(sizeof(struct dibx090p_state));
    if (state == NULL)
        return NULL;
    DibZeroMemory(state, sizeof(struct dibx090p_state));

    frontend_register_sip(fe, &state->info, &dib7090p_info, state);

    state->cfg = cfg;
    state->fe = fe;
    state->version = 0x7090;

    memcpy(&dib7000p_cfg, &default_dib7000_config, sizeof(struct dib7000p_config));

    switch (cfg->clock_khz) {
        case 30000: dib7000p_cfg.bw = &dibx090p_clock_config_30_mhz; break;
        case 12000:
        default:    dib7000p_cfg.bw = &dibx090p_clock_config_12_mhz; break;
    }

    dib7000p_cfg.agc = dibx090p_agc_config;

    dib7000p_cfg.output_mpeg2_in_188_bytes = cfg->output_mpeg2_in_188_bytes;
    dib7000p_cfg.hostbus_diversity         = 1;
    dib7000p_cfg.gpio_dir                  = cfg->gpio_dir;
    dib7000p_cfg.gpio_val                  = cfg->gpio_val;
    dib7000p_cfg.gpio_pwm_pos              = cfg->gpio_pwm_pos;
    dib7000p_cfg.diversity_delay           = cfg->diversity_delay;
    dib7000p_cfg.forward_erronous_mpeg_packet = cfg->forward_erronous_mpeg_packet;
    dib7000p_cfg.invert_spectrum           = cfg->invert_spectrum;

    if (is_7090e)
        memcpy(&state->dib0090_cfg, &dib7090e_config, sizeof(struct dib0090_config));
    else
        memcpy(&state->dib0090_cfg, &default_dib0090_config, sizeof(struct dib0090_config));
    state->dib0090_cfg.freq_offset_khz_uhf       = cfg->dib0090_freq_offset_khz_uhf;
    state->dib0090_cfg.freq_offset_khz_vhf       = cfg->dib0090_freq_offset_khz_vhf;

    state->dib0090_cfg.force_crystal_mode = cfg->force_crystal_mode;

    if (dib7000p_register(fe, host, addr, &dib7000p_cfg) == NULL)
        goto error;

    state->dib0090_cfg.get_adc_power = dib7090p_get_adc_power;
    state->dib0090_cfg.get_dc_power = dibx090p_get_dc_power;

    /* callback of set_diversity */
    state->set_diversity_in = state->fe->demod_info->ops.set_diversity_in;
    state->fe->demod_info->ops.set_diversity_in = dibx090p_set_diversity_in;

    /* callback of set_output_mode */
    state->fe->demod_info->ops.set_output_mode = dibx090p_set_output_mode;

    data_bus_host_init(&state->tuner_access, DATA_BUS_DIBCTRL_IF, dibx090p_tuner_xfer, state);
    if (dib0090_register(fe, &state->tuner_access, &state->dib0090_cfg) == NULL)
        goto tuner_error;

    /* custom AGC start for DiBx090p */
    fe->demod_info->ops.agc_startup = dibx090p_agc_startup;

    return fe;

tuner_error:
    frontend_unregister_demod(fe);
error:
    MemFree(state, sizeof(struct dibx090p_state));

    return NULL;
}

struct dibFrontend * dib7090e_sip_register(struct dibFrontend *fe, struct dibDataBusHost *host, uint8_t addr, const struct dibx090p_config *cfg) {
    return dib7090x_sip_register(fe, host, addr, cfg, 1);
}

struct dibFrontend * dib7090p_sip_register(struct dibFrontend *fe, struct dibDataBusHost *host, uint8_t addr, const struct dibx090p_config *cfg) {
    return dib7090x_sip_register(fe, host, addr, cfg, 0);
}

