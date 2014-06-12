#include <adapter/component_monitor.h>
#include <adapter/frontend.h>

#include <math.h>

void demod_monitor_store_statistic(struct dibDemodStatisticData *stat, struct dibDemodMonitor *m)
{
	stat->CoN_lin_part_n = m->equal_noise_dB;
	stat->CoN_lin_part_s = m->equal_signal_dB;
	stat->CoN = m->CoN;
}

#if 0
static const int16_t log_10_100[256] = { /* log10(i / 256.0) * 100 */
	-2408, -2107, -1931, -1806, -1709, -1630, -1563, -1505, -1454, -1408,
	-1367, -1329, -1294, -1262, -1232, -1204, -1178, -1153, -1129, -1107,
	-1086, -1066, -1047, -1028, -1010, -993, -977, -961, -946, -931,
	-917, -903, -890, -877, -864, -852, -840, -828, -817, -806, -795,
	-785, -775, -765, -755, -745, -736, -727, -718, -709, -701, -692,
	-684, -676, -668, -660, -652, -645, -637, -630, -623, -616, -609,
	-602, -595, -589, -582, -576, -569, -563, -557, -551, -545, -539,
	-533, -527, -522, -516, -511, -505, -500, -494, -489, -484, -479,
	-474, -469, -464, -459, -454, -449, -444, -440, -435, -431, -426,
	-421, -417, -413, -408, -404, -400, -395, -391, -387, -383, -379,
	-375, -371, -367, -363, -359, -355, -351, -348, -344, -340, -336,
	-333, -329, -325, -322, -318, -315, -311, -308, -304, -301, -298,
	-294, -291, -288, -284, -281, -278, -275, -272, -268, -265, -262,
	-259, -256, -253, -250, -247, -244, -241, -238, -235, -232, -229,
	-226, -224, -221, -218, -215, -212, -210, -207, -204, -201, -199,
	-196, -193, -191, -188, -186, -183, -180, -178, -175, -173, -170,
	-168, -165, -163, -160, -158, -155, -153, -151, -148, -146, -143,
	-141, -139, -136, -134, -132, -129, -127, -125, -123, -120, -118,
	-116, -114, -112, -109, -107, -105, -103, -101, -99, -96, -94,
	-92, -90, -88, -86, -84, -82, -80, -78, -76, -74, -72,
	-70, -68, -66, -64, -62, -60, -58, -56, -54, -52, -50,
	-48, -47, -45, -43, -41, -39, -37, -35, -33, -32, -30,
	-28, -26, -24, -23, -21, -19, -17, -16, -14, -12, -10,
	-9, -7, -5, -3, -2,
};
#endif

void demod_monitor_calc_statistic(struct dibDemodStatistic *stat, struct dibDemodStatisticData data[], int num)
{
	int i;
	double CoN_lin_mean_s = 0;
	double CoN_lin_mean_n = 0;
	double CoN_log_mean = 0;

	stat->CoN_min = 1000;
	stat->CoN_max = 0;

	for (i = 0; i < num; i++) {
		CoN_lin_mean_s += pow(10.0,data[i].CoN_lin_part_s/10.0);
		CoN_lin_mean_n += pow(10.0,data[i].CoN_lin_part_n/10.0);
		CoN_log_mean += data[i].CoN;

		if (data[i].CoN < stat->CoN_min)
			stat->CoN_min = data[i].CoN;

		if (data[i].CoN > stat->CoN_max)
			stat->CoN_max = data[i].CoN;
	}

	stat->CoN_lin_mean = 10.0 * log10( CoN_lin_mean_s / CoN_lin_mean_n );
	stat->CoN_log_mean = CoN_log_mean / ((double) num);
}

void demod_monitor_post_process(struct dibDemodMonitor *m)
{
	uint8_t k;
	double tmp0;

	tmp0 = (double) m->adc_power_i;
	if (tmp0 == 0.0)
		tmp0 = 1.0;
	tmp0 /= (double) (1 << 20);
	m->I_adc_power = 10 * log10(tmp0);

	tmp0 = (double) m->adc_power_q;
	if (tmp0 == 0.0)
		tmp0 = 1.0;
	tmp0 /= (double) (1 << 20);
	m->Q_adc_power = 10 * log10(tmp0);

// make it relative values in per cent
	if (m->adc_monitor_only) {
//		m->adc_lnpower = (10 * m->adc_lnpower_raw / 256.0 / log(10));
		m->adc_lnpower = 10.0 * m->adc_lnpower_raw / 256;
	} else {
		tmp0 = m->adc_power_i + m->adc_power_q;
		if (m->tuner_is_baseband)
			tmp0 /= 2.0;
		if (tmp0 == 0.0)
			tmp0 = 1.0;
		tmp0 /= (1 << 20);
		tmp0 = 10 * log10(tmp0);

		tmp0 = tmp0 - (-14.0);
		m->rf_power_dbm = (-m->tun_max_gain * (double) m->rf_power / (double) (1 << 16) + (double) m->tun_ref_level) + tmp0 +
				(m->lna_gain_step * 0); // 0 = current_lna_status - we don't have it in most cases
		m->agc_db = (double) m->tun_max_gain * (double) m->rf_power / (double) (1 << 16);

		m->agc1 = (m->agc1_raw / 65536.0);
		m->agc2 = (m->agc2_raw / 65536.0);

		m->agc_wbd = m->agc_wbd_raw / 4096.0;
	}
	m->iq_gain_mismatch  = 10 * log10((double) m->iq_misgain / 1024.0);
	m->iq_phase_mismatch = 180 * (m->iq_misphi / 2048.0);

        m->equal_signal_dB = 10 * log10((m->equal_signal_mant+1e-10) * pow(2.0,m->equal_signal_exp));
        m->equal_noise_dB = 10 * log10((m->equal_noise_mant+1e-10) * pow(2.0,m->equal_noise_exp));

        m->CoN = m->equal_signal_dB - m->equal_noise_dB;

	m->mer = -10.0 * log10( ((double) m->mer_mant) * pow(2.0, ((double) m->mer_exp) - 8.0) );

	/*
	int16_t ps = log_10_100[m->equal_signal_mant] + m->equal_signal_exp * 301,
		  pn = log_10_100[m->equal_noise_mant] + m->equal_noise_exp * 301;
	m->mer = (ps - pn + 5) / 10;
	*/

	m->viewable_dds_freq = (double) (m->p_dds_freq  / (1 << 26) * (m->internal_clk/1000));
	m->freq_offset = m->dds_freq - m->p_dds_freq;

	if (m->cur_digital_channel.type == STANDARD_DVBT)
		m->carrier_offset = (m->tuner_is_baseband ? 0 : 60000) - ((double) m->freq_offset / (double) (1 << 26) * (double) m->internal_clk);
	else if (m->cur_digital_channel.type == STANDARD_ISDBT)
		m->carrier_offset = ((double) m->freq_offset / (double) (1 << 26) * (double) m->internal_clk);

	//LB did not put back this modification because on TFE8076MD4, monitored timf was false after
	/*if (m->current_bandwidth !=0)
          m->timf_current = m->timf_current * 80 / (m->current_bandwidth / 100);*/
	m->timing_offset = (int) m->timf_current - (int) m->timf_default;
	m->timing_offset_ppm = 1e6 * (((double) m->timf_current / (double) m->timf_default) - 1);

	m->ber = ((double) m->ber_raw) / 1e8;
    if (m->cur_digital_channel.type == STANDARD_ISDBT) {
        m->berA = ((double) m->ber_raw_A) / 1e8;
        m->berB = ((double) m->ber_raw_B) / 1e8;
        m->berC = ((double) m->ber_raw_C) / 1e8;
    }

	if (m->n_2dpa_monitoring)
		for (k = 0; k < 4; k++)
			m->cti_def[k] = 10 * log10(fabs(m->cti_def_raw[k] / (double)(1 << 18)));
}

static void fft (double *data_re, double *data_im, uint16_t n)
{
	int mmax,m,j=0,istep,i;

	double wtemp,theta;
	double temp_re,temp_im,w_re,w_im,wp_re,wp_im;

	j = 0;
	for (i = 0; i < n; i++) {// Bit reverse the index (in situ)
		if (j > i) {
			temp_re = data_re[i];
			temp_im = data_im[i];
			data_re[i]= data_re[j];
			data_im[i]= data_im[j];
			data_re[j]= temp_re;
			data_im[j]= temp_im;
		}
		m = n >> 1;
		while (m > 0 && j >= m) {
			j -= m;
			m /= 2;
		}
		j+=m;
	}
	mmax=1;
	while (mmax < n) { // Routine proper
		istep = mmax * 2;
		theta = 6.2831853071796 / istep;
		wtemp = sin(0.5 * theta);
		wp_re = -2.0 * wtemp * wtemp;
		wp_im = sin(theta);
		w_re  = 1.0;
		w_im  = 0.0;
		for (m =0; m < mmax; m++) {
			for (i = m; i < n; i += istep) {
				j=i+mmax;
				temp_re= w_re*data_re[j] - w_im*data_im[j];
				temp_im= w_re*data_im[j] + w_im*data_re[j];
				data_re[j]= data_re[i] - temp_re;
				data_im[j]= data_im[i] - temp_im;
				data_re[i]= data_re[i] + temp_re;
				data_im[i]= data_im[i] + temp_im;
			}
			temp_re = w_re + w_re*wp_re - w_im*wp_im;
			temp_im = w_im + w_re*wp_im + w_im*wp_re;
			w_re = temp_re;
			w_im = temp_im;
		}
		mmax=istep;
	}
}

void demod_channel_profile_calc(int16_t re[128], int16_t im[128], struct dibDemodChannelProfile *profile)
{
	int lu = 8192 / (profile->fft == 0 ? 4 : profile->fft);

	int i,j;
	double bmwin[128], tmp_re[512] = { 0.0 }, tmp_im[512] = { 0.0 };

	for (i = 0; i < 128; i++) {
		double p = (i - 127) / (128.0*2.0) * 6.2831853071796;
		double tmp = ( 0.40217 - 0.49703 * cos(p) + 0.09892 * cos(p * 2) - 0.00183 * cos(p * 3));
		bmwin[i] = tmp * (569 / (569.0 - i));
	}

	//--- symetrisation
	for (i = 0; i < 128; i++) {
		tmp_re[i] = re[i] / 8192.0 * bmwin[i];
		tmp_im[i] = im[i] / 8192.0 * bmwin[i];

		if (i > 0) {
			tmp_re[512-i] = tmp_re[i];
			tmp_im[512-i] = -tmp_im[i];
		}
	}

	fft(tmp_re,tmp_im,512);

	i = (profile->pha_shift % (lu / 3)) * 512 / (lu / 3);
//	DibDbgPrint("-D-  profile shift: %d %d\n", profile->pha_shift, i);

	j = 0;
	while (j < 512) {
		profile->profile[j] = 10 * log10(fabs(tmp_re[i]) / 512.0);

		if (++i >= 512)
			i = 0;

		j++;
	}
}



