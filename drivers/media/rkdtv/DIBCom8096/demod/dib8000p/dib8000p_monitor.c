#include <adapter/frontend.h>
#include <adapter/component_monitor.h>

#include <demod/dib8000p.h>
#include "dib8000p_priv.h"

static int dib8000p_get_demod_monitor(struct dib8000p_state *state, struct dibDemodMonitor *m)
{
    uint16_t val = 0,v;
    uint16_t i;

    val = dib8000p_read_word(state, 570);
    m->locks.agc        = (val >> 14) & 1;
    m->locks.corm       = (val >> 13) & 1;
    m->locks.coff       = (val >> 12) & 1;
    m->locks.coff_cpil  = (val >> 11) & 1;
    m->locks.lmod4      = (val >> 10) & 1;
    m->locks.pha3       = (val >>  9) & 1;
    m->locks.equal      = (val >>  8) & 1;
    m->locks.fec_mpeg   = (val >> 7) & 1;
    m->locks.fec_mpeg_b = (val >> 6) & 1;
    m->locks.fec_mpeg_c = (val >> 5) & 1;
    m->locks.tmcc_dec   = (val >> 4) & 1;
    m->locks.tmcc_sync  = (val >> 3) & 1;
    m->locks.tmcc_data  = (val >> 1) & 1;
    m->locks.dvsy       = (val >>  0) & 1;

    m->locks.vit        = dib8000p_read_word(state, 556) & 1;
    m->locks.vit_b      = dib8000p_read_word(state, 557) & 1;
    m->locks.vit_c      = dib8000p_read_word(state, 558) & 1;

    m->iq_misgain = dib8000p_read_word(state, 396);
    m->iq_misphi = dib8000p_read_word(state, 395);
    if (m->iq_misphi & 0x800)
        m->iq_misphi -= 0x1000;

    val = dib8000p_read_word(state, 544);
    m->equal_noise_mant = (val >> 6) & 0xFF;
    m->equal_noise_exp = ((val & 0x3F));

    val = dib8000p_read_word(state, 545);
    m->equal_signal_mant = (val >> 6) & 0xFF;
    m->equal_signal_exp  = (val & 0x3F);

    if ((m->equal_noise_exp & 0x20) != 0)
        m->equal_noise_exp -= 0x40;
    if ((m->equal_signal_exp & 0x20) != 0)
        m->equal_signal_exp -= 0x40;

    val = dib8000p_read_word(state, 546);
    m->mer_mant = (val >> 6) & 0xFF;
    m->mer_exp = (val & 0x3F);
    if ((m->mer_exp & 0x20) == 0x20)
        m->mer_exp -= 0x40;

    m->can_display_ber_several_layers = 1;

    m->ber_raw_A        = dib8000p_read32(state, 562);
    m->ber_raw_B        = dib8000p_read32(state, 578);
    m->ber_raw_C        = dib8000p_read32(state, 583);
    m->PacketErrors_A   = dib8000p_read_word(state, 564);
    m->PacketErrors_B   = dib8000p_read_word(state, 580);
    m->PacketErrors_C   = dib8000p_read_word(state, 585);

    if (state->ber_monitored_layer == LAYER_C)
    {
        m->ber_raw      = m->ber_raw_C;
        m->PacketErrors = m->PacketErrors_C;
    }
    else if (state->ber_monitored_layer == LAYER_B)
    {
        m->ber_raw      = m->ber_raw_B;
        m->PacketErrors = m->PacketErrors_B;
    }
    else // layer A by default
    {
        m->ber_raw      = m->ber_raw_A;
        m->PacketErrors = m->PacketErrors_A;
    }

    m->PacketErrorCount = dib8000p_read_word(state, 569);
    m->isdbt_err_sec_layerA = dib8000p_read_word(state, 553) != 0;
    m->isdbt_err_sec_layerB = dib8000p_read_word(state, 554) != 0;
    m->isdbt_err_sec_layerC = dib8000p_read_word(state, 555) != 0;

    // Timing offset read
    m->timing_offset = (dib8000p_read32(state, 437) >> 1);
    if (m->timing_offset  & 0x20000000)
        m->timing_offset -= 0x40000000;

    m->timf_current = dib8000p_read32(state, 435);
    m->timf_default = state->timf_default;

    m->dds_freq = dib8000p_read32(state, 433);

    if (m->dds_freq  & 0x2000000)
        m->dds_freq -= 0x4000000;

    if ((m->invspec = dib8000p_read_word(state, 26)&0x1) == 1)
        m->dds_freq = (1 << 26) - m->dds_freq;

    m->p_dds_freq = dib8000p_read32(state, 27) & 0x1ffffff;

    m->viterbi_syndrome = dib8000p_read_word(state, 559);
    m->viterbi_syndrome_b = dib8000p_read_word(state, 560);
    m->viterbi_syndrome_c = dib8000p_read_word(state, 561);

    INIT_CHANNEL(&m->cur_digital_channel, STANDARD_ISDBT);
    m->cur_digital_channel.type = STANDARD_ISDBT;

    /* for the DiB8000pA we need to read the config */
    switch ((dib8000p_read_word(state, 0) >> 13) & 0x3) {
        case 1: m->cur_digital_channel.u.isdbt.nfft = FFT_2K; break;
        case 2: m->cur_digital_channel.u.isdbt.nfft = FFT_4K; break;
        default: m->cur_digital_channel.u.isdbt.nfft = FFT_8K; break;
    }
    m->cur_digital_channel.u.isdbt.guard = dib8000p_read_word(state, 1) & 0x3;

    m->cur_digital_channel.u.isdbt.sb_mode = dib8000p_read_word(state, 508) & 0x3;
    m->cur_digital_channel.u.isdbt.partial_reception = (dib8000p_read_word(state, 505)) & 0x1;

    for (i = 0; i < 3; i++) {
        m->cur_digital_channel.u.isdbt.layer[i].nb_segments = dib8000p_read_word(state, 493+i) & 0xF;
        m->cur_digital_channel.u.isdbt.layer[i].code_rate   = dib8000p_read_word(state, 481+i) & 0x7;
        m->cur_digital_channel.u.isdbt.layer[i].time_intlv  = dib8000p_read_word(state, 499+i) & 0x7;

        val = dib8000p_read_word(state, 487+i) & 0x3;
        switch(val) {
        case 0: m->cur_digital_channel.u.isdbt.layer[i].constellation = QAM_DQPSK; break;
        case 1: m->cur_digital_channel.u.isdbt.layer[i].constellation = QAM_QPSK; break;
        case 2: m->cur_digital_channel.u.isdbt.layer[i].constellation = QAM_16QAM; break;
        case 3:
        default : m->cur_digital_channel.u.isdbt.layer[i].constellation = QAM_64QAM; break;
        }
    }

    //val = dib8000p_read_word(state, 481);
    //m->cur_channel.intlv_native = (val >> 5) & 0x1;

    val = dib8000p_read_word(state, 550);
    m->way0_ok = (val >> 1) & 1;
    m->way1_ok = (val     ) & 1;
    m->dvsy_delay = ((val >> 2) & 0x7ff) * 2;
    m->dvsy_first_arrived = (val >> 14) & 1;

    val = dib8000p_read_word(state, 549);
    m->divin_fifo_overflow = val >> 14;
    m->divin_nberr_sym = val & 0x3fff;

    m->adc_power_i = dib8000p_read32(state, 384) & 0xfffff;

    m->adc_monitor_only = 0;
    m->adc_power_q = dib8000p_read32(state, 386) & 0xfffff;

    m->rf_power = dib8000p_read_word(state, 390);

    m->agc1_raw = dib8000p_read_word(state, 388);
    m->agc2_raw = 2 * dib8000p_read_word(state, 389); /* SOC8090P : 2 times because of HW bug in demod pwm cmd mapping / tuner input cmd. AGC2 uses only half of the range from 0->0.5 so just multiply it by 2 here to make it goes from 0->1 */
    m->agc_wbd_raw = dib8000p_read_word(state, 393);
    m->agc_split = (uint8_t) dib8000p_read_word(state, 392);

    m->internal_clk = dib8000p_read32(state, 23) /1000;
    m->n_2dpa_monitoring = 1;

    val = dib8000p_read_word(state, 538);
    m->n_2dpa_mu_int  = (val >> 3) & 0x7f;
    m->n_2dpa_num_fir = (val >> 0) & 0x07;

    for (v = 0; v < 3; v++) {
        uint32_t v32 = dib8000p_read32(state, 461 + v*2);
        m->cti_def_raw[v] = v32 & 0xffff;
        if((v32 >> 16 )&0x1)
            m->cti_def_raw[v] -= (1 << 16);
    }

#if 0
#include <math.h>

    dib8000p_write_word(state, 588, 1); // P_csic_rst_mnt_pilerr
    dib8000p_write_word(state, 588, 0);


    FILE *f = fopen("/tmp/pil_err", "w");
    uint16_t pil[1870], b[4];

    data_bus_client_read_multi16_attr(demod_get_data_bus_client(state->fe), 590, DATA_BUS_ACCESS_MODE_NO_ADDRESS_INCREMENT, pil, 1870);

    for (i = 1; i < 1870 * 3 + 1; i++) {
        double mant, err, chan;
        int16_t e;

        fprintf(f, "%d ", i);
        dib8000p_write_word(state, 589, i);

        while (!dib8000p_read_word(state, 595)) // n_csic_mnt_chan2_rdy

            data_bus_client_read_multi16(demod_get_data_bus_client(state->fe), 591, b, 4);

#if 0
        mant = b[0]  / 256.0;
        e = b[1];
        if (e & 0x8)
            e -= 0x10;
        chan = 10*log10(mant * pow(2.0, e) / 256.0);
        fprintf(f, "%f ", -45.0 + chan );
#endif

        mant = b[2]  / 256.0;
        e = b[3];
        if (e & 0x8)
            e -= 0x10;
        chan = 10*log10(mant * pow(2.0, e) / 256.0);
        fprintf(f, "%f ", chan );

#if 1
        mant = (pil[i/3] & 0x7f) / 128.0;

        e =  (pil[i/3] >> 7);
        if (e & 0x10)
            e -= 0x20;

        err = 10*log10(mant * pow(2.0, e - 15));


        fprintf(f, "%f", err );
#endif
        fprintf(f, "\n");
    }
    fclose(f);
#endif

    /*
       dbgpl(&dib8000p_dbg, "n_imp_cnt_burst  = %d", rd(406) );
       dbgpl(&dib8000p_dbg, "n_imp_cnt_burst1 = %d", rd(407) );
       dbgpl(&dib8000p_dbg, "n_imp_cnt_burst2 = %d", rd(408) );
       dbgpl(&dib8000p_dbg, "n_imp_cnt_burst3 = %d", rd(409) );
       dbgpl(&dib8000p_dbg, "P_imp:             %x", rd(137) );
       DibMSleep(200);
     */

#ifdef DIBCOM_EXTENDED_MONITORING
    demod_monitor_post_process(m);
#endif
    return DIB_RETURN_SUCCESS;
}

static void dib8000p_set_ber_monitored_layer(struct dib8000p_state *state, int layer)
{
        state->ber_monitored_layer = layer;
}

static int dib8000p_get_ber_monitored_layer(struct dib8000p_state *state)
{
    return state->ber_monitored_layer;
}

static int dib8000p_reset_pe_count(struct dib8000p_state *st)
{
    uint8_t b[4], nb[4] = { 0, 0, 0, 1 };

    dib8000p_read(st,  23,  b, 4);
    dib8000p_write(st, 23, nb, 4);
    dib8000p_write(st, 23,  b, 4);

    return 0;
}

static void dib8000p_get_snr_discrete(struct dib8000p_state *state, uint32_t *s, uint32_t *n)
{
    uint32_t t;
    int8_t exp;

    /* SNR */
    t = dib8000p_read_word(state, 544);
    *n = (t >> 6) & 0xff;
    exp = (uint8_t) (t & 0x3f);
    if (exp & 0x20)
        exp -= 0x40;
    *n <<= exp + 16;

    t = dib8000p_read_word(state, 545);
    *s = (t >> 6) & 0xff;
    exp = (uint8_t) (t & 0x3f);
    if (exp & 0x20)
        exp -= 0x40;
    *s <<= exp + 16;

}

static uint32_t dib8000p_get_snr(struct dib8000p_state *state)
{
    uint32_t s,n;
    dib8000p_get_snr_discrete(state, &s, &n);
    return uint32_finite_div(s, n, 16);
}

#ifdef DIBCOM_EXTENDED_MONITORING
#include <math.h>
static double isdbt_tolog(uint16_t mant, uint16_t expo, uint16_t quant_fract_mant, uint16_t int_quant_exp, uint8_t signed_exp, uint8_t noise)
{
   int expo_s = expo;

    //dbgpl(NULL,"expo = %d int_quant_exp =%d signed_exp = %d\n", expo_s, int_quant_exp, signed_exp);

    if(signed_exp && (expo & (1 << (int_quant_exp)) )) {
        expo_s -= (1<< (int_quant_exp +1));
        //dbgpl(NULL,"Negative out expo = %d\n", expo_s);
    }

    if(noise)
        expo_s -=7;

    return 10*log10(((double)(mant/pow(2.0, quant_fract_mant)) * pow(2.0, expo_s)) / 256.0);
}

/* viterbi spectrum */
static int dib8000p_get_carrier_power(struct dib8000p_state *state, int carrier, double * hchan, double * noise)
{
    uint16_t cnt, n=0, noise_mant, noise_exp, sig_mant, sig_exp;
    uint16_t nb_pil = carrier / 3;
    uint16_t *pil = (uint16_t *) malloc(nb_pil * sizeof(uint16_t));
    uint16_t ochan2[2];

    /* get noise */
    dib8000p_write_word(state, 588, 1); /* P_csic_rst_mnt_pilerr */
    dib8000p_write_word(state, 588, 0); /* P_csic_rst_mnt_pilerr */
    DibMSleep(10);

    data_bus_client_read_multi16_attr(demod_get_data_bus_client(state->fe), 590, DATA_BUS_ACCESS_MODE_NO_ADDRESS_INCREMENT, pil, nb_pil);

    for(cnt = 0 ; cnt < carrier ; cnt+=3) {
        noise_mant = pil[n]&0x7f;
        noise_exp = (pil[n]>>7)&0x1f;
        noise[cnt] = isdbt_tolog(noise_mant, noise_exp, 7, 4, 1, 1);

        if((cnt+1) < carrier)
            noise[cnt+1] = noise[cnt];
        if((cnt+2) < carrier)
            noise[cnt+2] = noise[cnt];
        n++;
    }


    for(cnt = 0 ; cnt < carrier ; cnt++) {
        dib8000p_write_word(state, 589, cnt);

        while(dib8000p_read_word(state, 595) != 1); /* wait for rdy signal */
        data_bus_client_read_multi16(demod_get_data_bus_client(state->fe), 593, ochan2, 2);

        sig_mant = ochan2[0]&0xff;
        sig_exp = ochan2[1]&0xf;
        hchan[cnt] = isdbt_tolog(sig_mant, sig_exp, 8, 3, 1, 0);
    }

    return DIB_RETURN_SUCCESS;
}

int dib8000p_get_spectrum_powmean(struct dibFrontend *fe, uint8_t *buf)
{
    struct dib8000p_state *state = fe->demod_priv;
    uint32_t cur_timf, original_timf, original_dds, powmean;
    uint16_t res_khz, buf16[2];
    int16_t dds_offset_khz;

    uint16_t iqc = dib8000p_read_word(state, 40);
    uint16_t pha = dib8000p_read_word(state, 36);
    uint16_t pana =dib8000p_read_word(state, 116);

    uint8_t iq_path;

    /* get original timf */
    original_timf = dib8000p_get_timf(fe);
    /* get original dds */
    data_bus_client_read_multi16(demod_get_data_bus_client(state->fe), 27, buf16, 2);
    original_dds  = (buf16[0] << 16) | buf16[1];

    /* get buf instruction */
    res_khz = (buf[3] << 8) | buf[4];
    dds_offset_khz = (buf[0] << 8) | buf[1];

    iq_path = buf[2];

    /* compute timf Vs requested resolution KHz */
    cur_timf = (original_timf / (fe->current_bw)) * res_khz;

    dib8000p_set_timf(fe, cur_timf);

    /* compute dds */
    dib8000p_set_dds(state, dds_offset_khz);

    /*select IQ path */
    switch (iq_path) {
        case 0:
            dib8000p_write_word(state, 38, (1<<14)|(1<<13)|(3<<11)|1024);
            break;
        case 1:
            dib8000p_write_word(state, 38, (1<<14)|(0<<13)|(3<<11)|1024);
            break;
        case 2:
            dib8000p_write_word(state, 38, (0<<14)|(1<<13)|(3<<11)|1024);
            break;
    }

    /* freeze IQ corr */
    dib8000p_write_word(state, 40, iqc | (3 << 11)); /* Discard the IqC block */
    dib8000p_write_word(state, 36, pha | 1);
    dib8000p_write_word(state, 116, 3); /* p_ana_gain */

    DibMSleep(78125 / res_khz * 0.2);

    /* get powmean */
    data_bus_client_read_multi16(demod_get_data_bus_client(state->fe), 422, buf16, 2); /* read n_corm_pow */
    powmean =  (( buf16[0] << 16) | buf16[1]);

    /* set back original params */
    dib8000p_write_word(state, 40, iqc);
    dib8000p_write_word(state, 36, pha);
    dib8000p_write_word(state, 116, pana);

    /* set back original timf */
    dib8000p_set_timf(fe, original_timf);

    /* set back original dds */
    dib8000p_write_word(state, 27, (original_dds >> 16) & 0x3ff);
    dib8000p_write_word(state, 28, (original_dds & 0xffff));

    return powmean;

}

/* TODO : REG 528 MAPPING IS DIFFERENT FROM 8002 !!! */
static int dib8000p_get_channel_profile(struct dib8000p_state *state, struct dibDemodChannelProfile *profile)
{
    int i;
    uint8_t b[4];
    int16_t re[128], im[128];

    profile->pha_shift = dib8000p_read_word(state, 426) & 0x1fff;

    // adp_freeze, rst_monit, select_fir
    dib8000p_write_word(state, 221, (1 << 3));
    dib8000p_write_word(state, 221, (1 << 3) | (1 << 2) | (2 << 0));
    dib8000p_write_word(state, 221, (1 << 3) | (0 << 2) | (2 << 0));

    for (i = 0; i < 128; i++) {
        dib8000p_read(state, 528, b, 4); /* TODO : REG 528 MAPPING IS DIFFERENT FROM 8002 !!! */
        re[i] = (b[0] << 8) | b[1];
        im[i] = (b[2] << 8) | b[3];
        if (b[0] & 0x40)
            re[i] -= 0x8000;
        if (b[2] & 0x40)
            im[i] -= 0x8000;
    }

    dib8000p_write_word(state, 221, 0);

    demod_channel_profile_calc(re,im,profile);
    return DIB_RETURN_SUCCESS;
}
#endif

#ifdef DIBCOM_GENERIC_MONITORING
int dib8000p_generic_monitoring(struct dibFrontend *fe, uint32_t cmd, uint8_t *b, uint32_t size, uint32_t offset)
{
    struct dib8000p_state *state = fe->demod_priv;
    int ret = DIB_RETURN_NOT_SUPPORTED;

    switch (cmd & ~GENERIC_MONIT_REQUEST_SIZE) {
        case GENERIC_MONIT_COMPAT_EXTENDED:
            ret = sizeof(struct dibDemodMonitor);
            if(!(cmd & GENERIC_MONIT_REQUEST_SIZE))
                dib8000p_get_demod_monitor(state, (struct dibDemodMonitor *) b);
            break;

        case GENERIC_MONIT_RESET_PE_COUNT:
            ret = 0;
            if(!(cmd & GENERIC_MONIT_REQUEST_SIZE))
                dib8000p_reset_pe_count(state);
            break;

#ifdef DIBCOM_EXTENDED_MONITORING
        case GENERIC_MONIT_COMPAT_PROFILE:
            ret = sizeof(struct dibDemodChannelProfile);
            if(!(cmd & GENERIC_MONIT_REQUEST_SIZE))
                dib8000p_get_channel_profile(state, (struct dibDemodChannelProfile *) b);
            break;
#endif
        case GENERIC_MONIT_SNR_DISCRETE:
            if(!(cmd & GENERIC_MONIT_REQUEST_SIZE)) {
                uint32_t s,n;
                dib8000p_get_snr_discrete(state, &s, &n);
                b[0] = (uint8_t) (s >> 24);
                b[1] = (uint8_t) (s >> 16);
                b[2] = (uint8_t) (s >>  8);
                b[3] = (uint8_t) (s      );
                b[4] = (uint8_t) (n >> 24);
                b[5] = (uint8_t) (n >> 16);
                b[6] = (uint8_t) (n >>  8);
                b[7] = (uint8_t) (n      );
            }
            ret = 8;
            break;

        case GENERIC_MONIT_SNR:
        case GENERIC_MONIT_SNR_COMBINED:
            ret = 4;

            if(!(cmd & GENERIC_MONIT_REQUEST_SIZE)) {
                uint32_t snr = dib8000p_get_snr(state);

                if (cmd == GENERIC_MONIT_SNR_COMBINED && fe->input != NULL) {
                    uint8_t i[4] = { 0 };
                    frontend_generic_monitoring(fe->input, GENERIC_MONIT_SNR_COMBINED, i, 4, 0);
                    snr += (i[0] << 24) | (i[1] << 16) | (i[2] << 8) | i[3];
                }

                b[0] = (uint8_t) (snr >> 24);
                b[1] = (uint8_t) (snr >> 16);
                b[2] = (uint8_t) (snr >>  8);
                b[3] = (uint8_t) (snr      );
            }
            break;

        case GENERIC_MONIT_SET_BER_LAYER:
            ret = 0;
            if(!(cmd & GENERIC_MONIT_REQUEST_SIZE))
                dib8000p_set_ber_monitored_layer(state, offset);
            break;

        case GENERIC_MONIT_BER_LAYER:
            ret = 0;
            if(!(cmd & GENERIC_MONIT_REQUEST_SIZE))
                ret = dib8000p_get_ber_monitored_layer(state);
            break;

        case GENERIC_MONIT_SET_BER_RS_LEN:
            ret = 0;
            if(!(cmd & GENERIC_MONIT_REQUEST_SIZE))
                ret = dib8000p_write_word(state, 285, (dib8000p_read_word(state, 285) & ~0x60) | ((uint16_t) ((offset&0x3) << 5)) );
             break;

#ifdef DIBCOM_EXTENDED_MONITORING
    case GENERIC_MONIT_SPECTRUM:
        if(!(cmd & GENERIC_MONIT_REQUEST_SIZE)) {
            double *s = (double *) b;
            dib8000p_get_carrier_power(state, offset, &s[0], &s[offset]);
        }
        ret = sizeof(double) * 2 * offset;
        break;

    case GENERIC_MONIT_GET_SPECTRUM_POWMEAN:
        ret = 0;
        if(!(cmd & GENERIC_MONIT_REQUEST_SIZE))
            ret = dib8000p_get_spectrum_powmean(fe, b);
        break;
#endif
    default:
    break;
    }
    return ret;
}
#endif

#if 0
double dump_cti_carrier(struct dibFrontend *demod, int selected_carrier, int average_nb)
{
    struct dib8000p_state *state = demod->demod_priv;
    int i;
    uint32_t r;
    int16_t re_, im_;
    double power = 0;


    for (i = 0; i < average_nb; i++) {
        dib8000p_write_word(state, 212, selected_carrier);

        r   = dib8000p_read32(state, 456);
        re_ = (uint16_t) ((r >> 10) & 0x3ff);
        im_ = (uint16_t) ((r      ) & 0x3ff);

        if (re_ & 0x200)
            re_ -= 0x400;
        if (im_ & 0x200)
            im_ -= 0x400;

        power += pow(re_ / (double) 0x200, 2) + pow(im_ / (double) 0x200, 2);
    }
    power /= (double) average_nb;
    power = 10*log10(power + 1e-10);

    return power;
}
#endif

