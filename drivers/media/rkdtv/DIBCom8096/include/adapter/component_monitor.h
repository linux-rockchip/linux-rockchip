#ifndef ADAPTER_COMPONENT_MONITOR_H
#define ADAPTER_COMPONENT_MONITOR_H

#include "common.h"
#include "demod.h"

#ifdef __cplusplus
extern "C" {
#endif

/* for the MAC */
struct dibMacMonitor {
    uint32_t ad_sample_lock;
};

/* for the Channel decoder */
struct dibChannelDecoderMonitor {
    int type; //see common.h struct dibChannel for possible types
    union {
        struct {
            double   sh_per;
            uint16_t sh_PS;
            uint16_t sh_nb_iter;
            uint16_t sh_overflow;
            uint16_t sh_lock;
            uint16_t sh_pha_sh_frame;
            uint16_t sh_crc_header_ok;
            uint16_t sh_padding_err;
            uint16_t dvb_sh;
            uint16_t sh_standard;
            uint16_t sh_common_mult;
            uint16_t sh_tc_punct_id;
            uint16_t sh_hrch;
            uint16_t sh_select_hp;
            uint16_t sh_Nqam;
            uint16_t sh_alpha;
            uint16_t sh_nfft;
            uint16_t mux;
            uint16_t misc;
        } sh;
        struct {
            uint32_t sl_num;
            uint32_t err_flag;
            double per;
            uint32_t syndrome_first_iter;
            uint32_t nb_iteration;
            uint32_t nb_max_iteration;
            uint32_t bloc_error;
            uint32_t overflow_cnt;
            uint32_t rs_table_count;
            uint32_t rs_table_error_count;
        } cmmb;
        struct {
            struct {
                uint8_t vit;
                uint8_t fec_mpeg;
                uint8_t fec_frm;
            } locks;
            uint16_t viterbi_syndrome;
            uint16_t PacketErrors;
            uint16_t PacketErrorCount;
            double ber;
        } dvbt;
        struct {
            struct {
                uint8_t vit;
                uint8_t vit_b;
                uint8_t vit_c;
                uint8_t fec_mpeg;
                uint8_t fec_mpeg_b;
                uint8_t fec_mpeg_c;
            } locks;

            uint8_t can_display_ber_several_layers;
            uint32_t ber_raw_A;
            uint32_t ber_raw_B;
            uint32_t ber_raw_C;
            uint16_t PacketErrors_A;
            uint16_t PacketErrors_B;
            uint16_t PacketErrors_C;
            double berA;
            double berB;
            double berC;

            uint16_t viterbi_syndrome;
            /*specific ISDBT*/
            uint16_t viterbi_syndrome_b;
            uint16_t viterbi_syndrome_c;

            uint16_t PacketErrorCount;
        } isdbt;
        struct {
            uint32_t syn              ;
            uint32_t syn_fic          ;
            uint32_t syn_msc          ;
            uint32_t syn_subc         ;
            uint32_t ber              ;
            uint32_t subc_staddr      ;
            uint32_t subc_sz          ;
            uint32_t subc_staddr_prev ;
            uint32_t subc_sz_prev     ;
            uint32_t subc_form        ;
            uint32_t subc_index       ;
            uint32_t subc_L1punct     ;
            uint32_t subc_L2punct     ;
            uint32_t subc_PI1         ;
            uint32_t subc_PI2         ;
            uint32_t subc_I           ;
            uint32_t fec_ber_rs       ;
            uint32_t fec_ber_rsnperr  ;
            uint32_t fec_ber_rscnt    ;
            uint32_t fec_newber       ;
            uint32_t fec_mpfail_cnt   ;
            uint32_t dmb_lock_frm     ;
            uint32_t fec_lock_frm     ;
            uint32_t fec_lock_mpeg    ;
            uint32_t fec_mpcor        ;
            uint32_t fec_mpfail       ;
            uint32_t TimeDtlv_overflow;
            uint32_t subc_id          ;
            uint32_t isMSC            ;
            uint32_t recfg            ;
            uint32_t subc_new         ;
            uint32_t cifcounter       ;
        } dab;
    } u;
};

            
/* for the Demod */
struct dibDemodMonitor {
/* raw part */

	struct {
            uint8_t agc;
            uint8_t corm;
            uint8_t coff;
            uint8_t coff_cpil;
            uint8_t lmod4;
            uint8_t pha3;
            uint8_t equal;
            uint8_t vit;
            uint8_t fec_frm;
            uint8_t fec_mpeg;
            uint8_t tps_dec;
            uint8_t tps_sync;
            uint8_t tps_data;
            uint8_t tps_cellid;
            uint8_t dvsy;
        /*specific ISDBT locks; for layer a, using old ones*/
            uint8_t vit_b;
            uint8_t vit_c;
            uint8_t fec_mpeg_b;
            uint8_t fec_mpeg_c;
            uint8_t tmcc_dec;
            uint8_t tmcc_sync;
            uint8_t tmcc_data;
        /*specific TDMB/DAB locks*/
            uint8_t corm_lock_dmb;
            uint8_t ndec_tmode_lock;
            uint8_t ndec_frame_lock;
            uint8_t dabcoff_lock;
	} locks;

	int16_t iq_misgain;
	int16_t iq_misphi;

	uint32_t equal_noise_mant;
	int16_t equal_noise_exp;
        double equal_noise_dB;

	uint32_t equal_signal_mant;
	int16_t equal_signal_exp;
        double equal_signal_dB;

	uint32_t sig_fp;
	uint32_t wgn_fp;

	uint16_t mer_mant;
	int16_t mer_exp;

	uint32_t ber_raw;

	uint16_t PacketErrors;
	uint16_t PacketErrorCount;

	int timing_offset;
	uint32_t timf_current;
	uint32_t timf_default;
	uint32_t current_bandwidth;

	int dds_freq;
	uint8_t invspec;
	int p_dds_freq;

	uint16_t viterbi_syndrome;
        /*specific ISDBT*/
        uint16_t viterbi_syndrome_b;
        uint16_t viterbi_syndrome_c;

#ifndef NO_DVBCHANNEL_BACKWARD_COMPAT
	struct dibDVBChannel cur_channel;
#endif
    struct dibChannel cur_digital_channel;

	uint8_t way0_ok;
	uint8_t way1_ok;
	uint8_t divin_fifo_overflow;
	uint16_t divin_nberr_sym;
	uint16_t dvsy_delay;
	uint8_t dvsy_first_arrived;

	uint8_t adc_monitor_only;

	uint32_t adc_power_i;
	uint32_t adc_power_q;

	uint16_t rf_power;

	uint16_t agc1_raw;
	uint16_t agc2_raw;
	uint16_t agc_wbd_raw;
	uint8_t  agc_split;

	int16_t adc_lnpower_raw;
	uint8_t adsample_lock;

	uint8_t tuner_is_baseband;
	int tun_max_gain;
	int tun_ref_level;
	int lna_gain_step;

	int internal_clk;

	double ber;
	double viewable_dds_freq;

	double I_adc_power;  // in dBVrms
	double Q_adc_power;  // in dBVrms

	double rf_power_dbm; // in dBm
	double agc_db;       // in dB

	double agc_global;
	double agc_wbd;

	double agc1;         // in V
	double agc2;         // in V

	double adc_lnpower;

	int freq_offset;
	double timing_offset_ppm; // in ppm
	double carrier_offset;

	double iq_gain_mismatch;
	double iq_phase_mismatch;

	double CoN;

	double mer;

	uint8_t n_adc_mode;
	uint8_t n_2dpa_monitoring;

	int cti_def_raw[4];
	double cti_def[4];

	uint8_t n_2dpa_mu_int;
	uint8_t n_2dpa_num_fir;

        struct {
            uint8_t nb_bit_quant;
            uint8_t nb_bit_reverse_quant;
            uint8_t tdint_full_internal;
            uint8_t tmode;
            uint8_t ndec_tmode;
            uint8_t ndec_cor_max;
            uint8_t dmb_lock_frm;

            double  small_freqoff;
            int16_t large_freqoff;

            uint8_t vit_syn_subc_id;
            uint8_t vit_syn_mode;
            int16_t syn;
            int16_t syn_fic;
            int16_t syn_msc;
            int16_t syn_subc;
            int8_t  syn_subid;

            uint16_t subc_id;
            uint16_t isMSC;
            uint16_t recfg;
            uint16_t subc_new;
            uint16_t cifcounter;
            uint16_t subc_staddr;
            uint16_t subc_sz;
            uint16_t subc_staddr_prev;
            uint16_t subc_sz_prev;
            uint16_t subc_form;
            uint16_t subc_index;
            uint16_t subc_L1punct;
            uint16_t subc_L2punct;
            uint16_t subc_PI1;
            uint16_t subc_PI2;
            uint16_t subc_I;

            uint16_t fec_state;
            uint8_t fec_subid[12];
        } dab;

	double iqc_dcoff_i;
	double iqc_dcoff_q;
        uint8_t isdbt_err_sec_layerA;
        uint8_t isdbt_err_sec_layerB;
        uint8_t isdbt_err_sec_layerC;

	uint8_t mpefec_available;
	uint8_t mpefec_request;

	struct {
		/*  DVB-H Monitoring */
		uint16_t pid;
		uint32_t ulTotalTables;
		uint32_t ulErrorTablesBeforeFec;
		uint32_t ulErrorTablesAfterFec;
		uint32_t ulTotalPacketData;
		uint32_t ulErrorPacketDataBeforeFec;
		uint32_t ulErrorPacketDataAfterFec;
		uint8_t MPEPadding;
		uint8_t MPEPuncturing;
		uint16_t uiTableSize;
		uint16_t usDeltaT_Stream;
		uint16_t usDeltaT_Detected;
		uint16_t usBurst_Stream;
		uint16_t usBurst_Detected;
		uint16_t usPowerUpTime;
		uint32_t Debug_Value_1;
		uint32_t Debug_Value_2;
		uint8_t tsChannelId;
	} mpefec;

    uint8_t can_display_ber_several_layers;
	uint32_t ber_raw_A;
	uint32_t ber_raw_B;
	uint32_t ber_raw_C;
	uint16_t PacketErrors_A;
	uint16_t PacketErrors_B;
	uint16_t PacketErrors_C;
    double berA;
    double berB;
    double berC;

    uint8_t do_not_display_chandec;

};

struct dibServiceMonitor {
	uint8_t type;	/*  STANDARD_UNKNOWN  0
						STANDARD_DVBT     1
						STANDARD_ISDBT    2
						STANDARD_DAB      3
						STANDARD_FM       4  */

	uint8_t service_monit_available;

	union {
		struct {
			/*  DVB-H Monitoring */
				uint16_t pid;
				uint32_t ulTotalTables;
				uint32_t ulErrorTablesBeforeFec;
				uint32_t ulErrorTablesAfterFec;
				uint8_t MPEPadding;
				uint8_t MPEPuncturing;
				uint16_t uiTableSize;
				uint16_t usDeltaT_Stream;
				uint16_t usDeltaT_Detected;
				uint16_t usBurst_Stream;
				uint16_t usBurst_Detected;
				uint16_t usPowerUpTime;
		} mpe_fec;
	} u;
};

/* for calculation */
struct dibDemodStatisticData {
	double CoN_lin_part_s;
	double CoN_lin_part_n;
	double CoN;
};

struct dibDemodStatistic {
	double CoN_max;
	double CoN_min;
	double CoN_log_mean;
	double CoN_lin_mean;
};

extern void demod_monitor_post_process(struct dibDemodMonitor *);
extern void demod_monitor_store_statistic(struct dibDemodStatisticData *, struct dibDemodMonitor *);
extern void demod_monitor_calc_statistic(struct dibDemodStatistic *, struct dibDemodStatisticData[], int);
struct dibDemodChannelProfile {
	double profile[512];
	uint8_t fft;
	uint16_t pha_shift;
};

extern void demod_channel_profile_calc(int16_t re[128], int16_t im[128], struct dibDemodChannelProfile *);
extern uint32_t demod_ber_stabilization_time(struct dibFrontend *);
extern uint32_t channel_decoder_stabilization_time(struct dibFrontend *fe, uint16_t sh_ncrc_per);

struct dibAnalogVideoMonitor {

	double longmin_db;
	uint16_t longmin_int_val;
	double longmax_db;
	uint16_t longmax_int_val;

	double agc_global;
	double agc1;
	double agc2;

	uint8_t sync_lock;
	uint8_t car_lock;

	double lev_est;
	double pha_est;
	double dci_est;
	double dcq_est;

	uint8_t sync_mode;
	uint8_t div_first;
	uint8_t div_best_path;
	uint16_t div_diff_lock;

	uint16_t n_div_sync_mode;
	uint16_t n_div_comb_mode;
	uint16_t n_dout_cfg;

	double n_Pout_coef_A;
	double n_Pout_coef_B;
	double n_Pout_coef_C;
	double n_Pout_coef_D;

	uint8_t transparent_mode;
};

struct dibAnalogAudioMonitor {

	uint8_t transparent_mode;
	uint8_t mode;

	double err_pha_db;
	int dec_nic_err_rate;
	int dec_nic_ctr_bit;
	int nicam_frame_lock;

	double am_dc;

};

struct dibTunerMonitor {

	int gain_is_frozen;

};

extern void dump_digital_channel_params(struct dibChannel *);

#ifdef __cplusplus
}
#endif

#endif
