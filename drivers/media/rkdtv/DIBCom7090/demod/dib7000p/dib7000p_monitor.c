#include <adapter/demod.h>
#include <adapter/component_monitor.h>

#include <demod/dib7000p.h>
#include "dib7000p_priv.h"

static int dib7000p_get_demod_monitor(struct dib7000p_state *state, struct dibDemodMonitor *m)
{
	uint16_t val = 0;
	uint32_t v32;

	m->adc_power_i = (dib7000p_read32(state, 388) & 0xfffff);
	m->adc_power_q = (dib7000p_read32(state, 390) & 0xfffff);

	m->rf_power = dib7000p_read_word(state, 394);

	val = dib7000p_read_word(state, 509);
	m->locks.agc        = (val >> 15) & 1;
	m->locks.corm       = (val >> 14) & 1;
	m->locks.coff       = (val >> 13) & 1;
	m->locks.coff_cpil  = (val >> 12) & 1;
	m->locks.lmod4      = (val >> 11) & 1;
	m->locks.pha3       = (val >> 10) & 1;
	m->locks.equal      = (val >>  9) & 1;
	m->locks.vit        = (val >>  8) & 1;
	m->locks.fec_frm    = (val >>  7) & 1;
	m->locks.fec_mpeg   = (val >>  6) & 1;
	m->locks.tps_dec    = (val >>  5) & 1;
	m->locks.tps_sync   = (val >>  4) & 1;
	m->locks.tps_data   = (val >>  3) & 1;
	m->locks.tps_cellid = (val >>  1) & 1;
	m->locks.dvsy       = (val >>  0) & 1;

	m->iq_misphi = dib7000p_read_word(state, 399);
	m->iq_misgain = dib7000p_read_word(state, 400);
	if (m->iq_misphi & 0x800)
		m->iq_misphi -= 0x1000;

	val = dib7000p_read_word(state, 479);
	m->equal_noise_mant = (val >> 4) & 0xFF;
	m->equal_noise_exp = ((val & 0xF) << 2);

	val = dib7000p_read_word(state, 480);
	m->equal_noise_exp += ((val >> 14) & 0x3);
	if ((m->equal_noise_exp & 0x20) != 0)
		m->equal_noise_exp -= 0x40;

	m->equal_signal_mant = (val >> 6) & 0xFF;
	m->equal_signal_exp  = (val & 0x3F);
    if ((m->equal_signal_exp & 0x20) != 0)
		m->equal_signal_exp -= 0x40;

	val = dib7000p_read_word(state, 481);
	m->mer_mant = (val >> 6) & 0xFF;
	m->mer_exp = (val & 0x3F);
	if ((m->mer_exp & 0x20) == 0x20)
		m->mer_exp -= 0x40;

	m->ber_raw = dib7000p_read32(state, 500);

	m->PacketErrors     = dib7000p_read_word(state, 506);
	m->PacketErrorCount = dib7000p_read_word(state, 508);

	// Timing offset read
	m->timing_offset = (dib7000p_read32(state, 429) >> 1);
	if (m->timing_offset & 0x20000000)
		m->timing_offset -= 0x40000000;

	m->timf_current = dib7000p_read32(state, 427);
	m->timf_default = state->timf_default;

	m->dds_freq = (dib7000p_read32(state, 425) & 0x3ffffff); //(0,0,26)
	if (m->dds_freq & 0x2000000)
		m->dds_freq -= 0x4000000;

	v32 = dib7000p_read32(state, 21);
	m->invspec = (uint8_t) (v32 >> 25) & 0x01;
	m->p_dds_freq = v32 & 0x1ffffff;       //(0,-1,26)

	if (m->invspec > 0)
		m->dds_freq = (1 << 26) - m->dds_freq;

	m->viterbi_syndrome = dib7000p_read_word(state, 488);

	val = dib7000p_read_word(state, 463);
	INIT_DVB_CHANNEL(&m->cur_channel);
	m->cur_channel.nfft = (val >> 8) & 0x3;
	m->cur_channel.guard = val & 0x3;
	m->cur_channel.nqam = (val >> 14) & 0x3;
	m->cur_channel.vit_hrch = (val >> 13) & 0x1;
	m->cur_channel.vit_alpha = (val >> 10) & 0x7;
	m->cur_channel.vit_code_rate_hp = (val >> 5) & 0x7;
	m->cur_channel.vit_code_rate_lp = (val >> 2) & 0x7;
	val = dib7000p_read_word(state, 464);
	m->cur_channel.intlv_native = (val >> 5) & 0x1;
	CONVERT_DVB_TO_DIGITAL_CHANNEL(&m->cur_channel, &m->cur_digital_channel);

	val = dib7000p_read_word(state, 485);
	m->way0_ok = (val >> 1) & 1;
	m->way1_ok = (val     ) & 1;

	val = dib7000p_read_word(state, 484);
	m->divin_fifo_overflow = val >> 14;
	m->divin_nberr_sym = val & 0x3fff;

	m->agc1_raw = dib7000p_read_word(state, 392);
	m->agc2_raw = dib7000p_read_word(state, 393);
	m->agc_wbd_raw = dib7000p_read_word(state, 397);

	m->tuner_is_baseband = state->cfg.tuner_is_baseband;
	m->tun_max_gain = state->cfg.tun_max_gain;
	m->tun_ref_level = state->cfg.tun_ref_level;
	m->lna_gain_step = state->cfg.lna_gain_step;
        m->internal_clk = dib7000p_get_internal_freq(state);


	m->n_adc_mode = state->adc_mode;
#ifdef DIBCOM_EXTENDED_MONITORING
	demod_monitor_post_process(m);
#endif
	return 0;
}

static int dib7000p_reset_pe_count(struct dib7000p_state *st)
{
	uint8_t b[4], nb[4] = { 0, 0, 0, 1 };

	dib7000p_read(st,  18,  b, 4);
	dib7000p_write(st, 18, nb, 4);
	dib7000p_write(st, 18,  b, 4);

	return 0;
}

static int dib7000p_set_viterbi(struct dib7000p_state *state, int onoff)
{
    /* Now, nothing is forced here, to play with viterbi params on registry editor while ploting */
    return 0; //dib7000p_write_word(state, 212, 0x169 | ((!onoff) << 2) );   // freeze or thaw Viterbi
}

#ifdef DIBCOM_EXTENDED_MONITORING

#include <math.h>
#define to_log(exp,num) \
( \
	10 * log10( \
		( (exp) & 0x8 ? \
			((num) << ((exp) - 0x8)) : \
			((num) << ((exp) + 0x8)) ) \
	/ 16777216.0) \
)

/* gives noise and channel power of carrier freq + the following 2 */
static int dib7000p_get_carrier_power(struct dib7000p_state *state, int carrier, double hchan[3], double noise[3])
{
	uint8_t buf[6];
	uint8_t chan[12];
	uint16_t reg520, reg521, reg522, reg523, reg524, reg525;

	buf[0] = (carrier >> 8) & 0xff;
	buf[1] =  carrier       & 0xff;
	carrier++;
	buf[2] = (carrier >> 8) & 0xff;
	buf[3] =  carrier       & 0xff;
	carrier++;
	buf[4] = (carrier >> 8) & 0xff;
	buf[5] =  carrier       & 0xff;

	dib7000p_write(state, 219, buf, 6);
	DibMSleep(1);
	dib7000p_read(state, 494, chan, 12);

	reg520 = (chan[0]  << 8) | chan[1];
	reg521 = (chan[2]  << 8) | chan[3];
	reg522 = (chan[4]  << 8) | chan[5];
	reg523 = (chan[6]  << 8) | chan[7];
	reg524 = (chan[8]  << 8) | chan[9];
	reg525 = (chan[10] << 8) | chan[11];

	hchan[0] = to_log(reg520 & 0xf, reg520 >> 4);
	hchan[1] = to_log(reg522 & 0xf, reg522 >> 4);
	hchan[2] = to_log(reg524 & 0xf, reg524 >> 4);

	noise[0] = to_log(reg521 & 0xf, reg521 >> 4);
	noise[1] = to_log(reg523 & 0xf, reg523 >> 4);
	noise[2] = to_log(reg525 & 0xf, reg525 >> 4);
	return DIB_RETURN_SUCCESS;
}

static int dib7000p_get_channel_profile(struct dib7000p_state *state, struct dibDemodChannelProfile *profile)
{
	int i;
	uint8_t b[4];
	int16_t re[128], im[128];

	profile->pha_shift = dib7000p_read_word(state, 417) & 0x1fff;

	// adp_freeze, rst_monit, select_fir
	dib7000p_write_word(state, 193, (1 << 3));
	dib7000p_write_word(state, 193, (1 << 3) | (1 << 2) | (2 << 0));
	dib7000p_write_word(state, 193, (1 << 3) | (0 << 2) | (2 << 0));

	for (i = 0; i < 128; i++) {
		dib7000p_read(state, 473, b, 4);
		re[i] = ((b[0] << 8) | b[1]) & 0x7fff;
		im[i] = (b[2] << 8) | b[3];
		if (b[0] & 0x40)
			re[i] -= 0x8000;
		if (b[2] & 0x40)
			im[i] -= 0x8000;
	}

	dib7000p_write_word(state, 193, 0);

	demod_channel_profile_calc(re,im,profile);
	return DIB_RETURN_SUCCESS;
}
#endif

static uint32_t dib7000p_get_snr(struct dib7000p_state *state)
{
    uint32_t s,n,t;
    int8_t e;

    /* SNR */
    t = dib7000p_read_word(state, 479);
    n = t >> 4;

    e = (int8_t)((t & 0xf) << 2);
    t = dib7000p_read_word(state, 480);
    e |= t >> 14;
    if (e & 0x20)
        e -= 0x40;
    n <<= e + 16;

    s = (t >> 6) & 0xff;
    e = (uint8_t) (t & 0x3f);
    if (e & 0x20)
        e -= 0x40;
    s <<= e + 16;

    return uint32_finite_div(s, n, 16);
}

static void dib7000p_set_timf(struct dib7000p_state *state, uint32_t timf)
{
    dib7000p_write_word(state, 23, (uint16_t) (timf >> 16) & 0xffff);
    dib7000p_write_word(state, 24, (uint16_t)  timf        & 0xffff);
}

static int dib7000p_get_timf(struct dib7000p_state *state)
{
    uint16_t buf[2];
    data_bus_client_read_multi16(demod_get_data_bus_client(state->fe), 23, buf, 2);
    return (( buf[0] << 16) | buf[1]);
}

#ifdef DIBCOM_EXTENDED_MONITORING
int dib7000p_get_spectrum_powmean(struct dibFrontend *fe, uint8_t *buf)
{
    struct dib7000p_state *state = fe->demod_priv;
    uint32_t cur_timf, original_timf, original_dds, powmean;
    uint16_t restart, res_khz, buf16[2];
    int16_t dds_offset_khz;

    uint16_t iqc   = dib7000p_read_word(state, 36);
    uint16_t pha   = dib7000p_read_word(state, 32);
    uint16_t pana  = dib7000p_read_word(state, 116);
    uint16_t reg_0 = dib7000p_read_word(state, 0);
    uint8_t iq_path;

    /*if P_guard == 3 (1/4) temporary write P_guard = 0 => workarround 1/4 specific thing inside HW timrec/pow_mean */
    if(((reg_0>>5)&0x3) == 3) {
        dib7000p_write_word(state, 0, (reg_0&~(0x3<<5)));
	dbgpl(&dib7000p_dbg, "P_gaurd = %d\n",((reg_0>>5)&0x3));
    }

    /* get original timf */
    original_timf = dib7000p_get_timf(state);

    /* get original dds */
    data_bus_client_read_multi16(demod_get_data_bus_client(state->fe), 21, buf16, 2);
    original_dds  = ((buf16[0] << 16)&0x1ff) | buf16[1];

    /* get buf instruction */
    res_khz = (buf[3] << 8) | buf[4];
    dds_offset_khz = (buf[0] << 8) | buf[1];

    iq_path = buf[2];

    /* compute timf Vs requested resolution KHz */
    cur_timf = (original_timf / (fe->current_bw)) * res_khz;
    dbgpl(&dib7000p_dbg, "fe->current_bw = %d\n",fe->current_bw);

    dib7000p_set_timf(state, cur_timf);

    /* compute dds */
    dib7000p_set_dds(state, dds_offset_khz);

    /*select IQ path */
    switch (iq_path) {
        case 0:
            dib7000p_write_word(state, 34, (dib7000p_read_word(state, 34) & (1 << 15)) | (1<<14) | (1<<13) | (3<<11) | 1024);
            break;
        case 1:
            dib7000p_write_word(state, 34, (dib7000p_read_word(state, 34) & (1 << 15)) | (1<<14) | (0<<13) | (3<<11) | 1024);
            break;
        case 2:
            dib7000p_write_word(state, 34, (dib7000p_read_word(state, 34) & (1 << 15)) | (0<<14) | (1<<13) | (3<<11) | 1024);
            break;
    }

    /* freeze IQ corr */
    dib7000p_write_word(state, 36, iqc | (3 << 11)); /* Discard the IqC block */
    dib7000p_write_word(state, 32, pha | (1<<4));
    dib7000p_write_word(state, 116, 2); /* p_ana_gain = 2*/

    /* start measurement */
    /* restart palf */
    restart = dib7000p_read_word(state, 770);
    dib7000p_write_word(state, 770, restart | (1<<7));
    dib7000p_write_word(state, 770, restart);

    DibMSleep(78125 / res_khz * 0.2);

    /* get powmean */
    data_bus_client_read_multi16(demod_get_data_bus_client(state->fe), 413, buf16, 2);
    powmean =  (( buf16[0] << 16) | buf16[1]);

    /* set back original params */
    dib7000p_write_word(state, 36, iqc);
    dib7000p_write_word(state, 32, pha);
    dib7000p_write_word(state, 116, pana);

    /* set back original timf */
    dib7000p_set_timf(state, original_timf);

    /* set back original dds */
    dib7000p_write_word(state, 21, (original_dds >> 16));
    dib7000p_write_word(state, 22, (original_dds & 0xffff));

    /* set back real P_guard in case it was modified */
    if(((reg_0>>5)&0x3) == 3)
      dib7000p_write_word(state, 0, reg_0);

    return powmean;

}
#endif

#ifdef DIBCOM_GENERIC_MONITORING
int dib7000p_generic_monitoring(struct dibFrontend *fe, uint32_t cmd, uint8_t *b, uint32_t size, uint32_t offset)
{
	struct dib7000p_state *state = fe->demod_priv;
    int ret = DIB_RETURN_NOT_SUPPORTED;

    switch (cmd & ~GENERIC_MONIT_REQUEST_SIZE) {
        case GENERIC_MONIT_COMPAT_EXTENDED:
            if(!(cmd & GENERIC_MONIT_REQUEST_SIZE))
                dib7000p_get_demod_monitor(state, (struct dibDemodMonitor *) b);
            ret = sizeof(struct dibDemodMonitor);
        break;
        case GENERIC_MONIT_SET_VITERBI:
            if(!(cmd & GENERIC_MONIT_REQUEST_SIZE))
                dib7000p_set_viterbi(state, offset);
            ret = 0;
        break;
        case GENERIC_MONIT_RESET_PE_COUNT:
            if(!(cmd & GENERIC_MONIT_REQUEST_SIZE))
                dib7000p_reset_pe_count(state);
            ret = 0;
        break;

#ifdef DIBCOM_EXTENDED_MONITORING
        case GENERIC_MONIT_COMPAT_SPECTRUM:
            if(!(cmd & GENERIC_MONIT_REQUEST_SIZE)) {
                double *s = (double *) b;
                dib7000p_get_carrier_power(state, offset, &s[0], &s[3]);
            }
            ret = sizeof(double) * 6;
        break;
        case GENERIC_MONIT_COMPAT_PROFILE:
            if(!(cmd & GENERIC_MONIT_REQUEST_SIZE))
                dib7000p_get_channel_profile(state, (struct dibDemodChannelProfile *) b);
            ret = sizeof(struct dibDemodChannelProfile);
        break;
#endif

        case GENERIC_MONIT_SNR:
        case GENERIC_MONIT_SNR_COMBINED:
            if(!(cmd & GENERIC_MONIT_REQUEST_SIZE)) {
                uint32_t snr = dib7000p_get_snr(state);

                if (cmd == GENERIC_MONIT_SNR_COMBINED &&
                    fe->input != NULL) {
                    uint8_t i[4] = { 0 };
                    frontend_generic_monitoring(fe->input, GENERIC_MONIT_SNR_COMBINED, i, 4, 0);
                    snr += (i[0] << 24) | (i[1] << 16) | (i[2] << 8) | i[3];
                }

                b[0] = (uint8_t) (snr >> 24);
                b[1] = (uint8_t) (snr >> 16);
                b[2] = (uint8_t) (snr >>  8);
                b[3] = (uint8_t) (snr      );
            }
            ret = 4;
    break;
#ifdef DIBCOM_EXTENDED_MONITORING
	 case GENERIC_MONIT_GET_SPECTRUM_POWMEAN:
        if(!(cmd & GENERIC_MONIT_REQUEST_SIZE))
			  ret = dib7000p_get_spectrum_powmean(fe, b);
		 else
	        ret = 0;
    break;
#endif
    default:
    break;
    }
    return ret;
}
#endif

