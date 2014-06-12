#ifndef ADAPTER_COMMON_H
#define ADAPTER_COMMON_H

#include "../platform/platform.h"
#include "debug.h"

#ifdef __cplusplus
extern "C" {
#endif

/* be carrefull to change those defines : used as array index in the code */
#define TUNE_TIME_LOCKED 0
#define TUNE_TIME_DATA 1

#define DIB_RETURN_SUCCESS                   0
#define DIB_RETURN_ERROR                    -1
#define DIB_RETURN_NOT_SUPPORTED            -2
#define DIB_RETURN_RESOURCES                -3
#define DIB_RETURN_TIMEOUT                  -4
#define DIB_RETURN_INVALID_PARAMETER        -5
#define DIB_RETURN_FRONT_END_COM_ERROR      -6
#define DIB_RETURN_FRONT_END_DEV_ERROR      -7

#define MAX_NB_OF_FRONTENDS_IN_CHAIN 5
#define MAX_NB_OF_VALID_FRONTENDS    10
#define MAX_NB_OF_FRONTENDS          40


#ifndef MAX_INFO_NAME_LENGTH
#define MAX_INFO_NAME_LENGTH 60
#endif


/* used in frontend->component and channel_frontend-monitoring */
#define FE_COMPONENT_TUNER   0x01
#define FE_COMPONENT_DEMOD   0x02
#define FE_COMPONENT_CHANDEC 0x04
#define FE_COMPONENT_ANTENNA 0x08
#if 0
#define FE_COMPONENT_VIDEO   0x08
#define FE_COMPONENT_AUDIO   0x10
#endif
#define CHANNEL_MONITORING_REFRESH_REQUESTED 0x20

#define BAND_LBAND 0x01
#define BAND_UHF   0x02
#define BAND_VHF   0x04
#define BAND_SBAND 0x08
#define BAND_FM	   0x10
#define BAND_CBAND 0x20

extern int channel_frequency_band(uint32_t freq_kHz);
#define BAND_OF_FREQUENCY(freq_kHz) channel_frequency_band(freq_kHz)

/* capabilities of a frontend, mode of an adapter */
#define MODE_DVBT         (1 <<  0)
#define MODE_ISDBT        (1 <<  1)
#define MODE_ATSC         (1 <<  2)
#define MODE_DAB_TDMB     (1 <<  3)
#define MODE_DVBSH_DEMOD  (1 <<  8)
#define MODE_DVBSH        (1 <<  9)
#define MODE_DVBSHA2_DEMOD (1 <<  10)
#define MODE_DVBSHA2      (1 <<  11)
#define MODE_CMMB         (1 << 12)
#define MODE_DIGITAL      (MODE_DVBT | MODE_ATSC | MODE_ISDBT | MODE_DAB_TDMB | MODE_DVBSH | MODE_DVBSHA2 | MODE_DVBSH_DEMOD | MODE_DVBSHA2_DEMOD | MODE_CMMB)

#define MODE_ANALOG_AUDIO (1 <<  4)
#define MODE_ANALOG_VIDEO (1 <<  5)
#define MODE_ANALOG       (MODE_ANALOG_VIDEO | MODE_ANALOG_AUDIO)

#define MODE_ANALOG_ADC   (1 <<  6)
#define FE_CAN_FAST_TUNE  (1 <<  7)


/* additional IDs for GENERIC Monitoring */
#define GENERIC_MONIT_REQUEST_SIZE     (1 << 31)

/* new way to read the viterbi spectrum */
#define GENERIC_MONIT_SPECTRUM         (      1)
#define GENERIC_MONIT_SPECTRUM_SIZE    (GENERIC_MONIT_SPECTRUM | GENERIC_MONIT_REQUEST_SIZE)

/* all COMPAT-requests are not endian-aware - if there is a difference
 * between host and device - it will create a problem. For the others
 * we are big-endian */
/* compatible way to read the viterbi spectrum */
#define GENERIC_MONIT_SET_VITERBI      (      2)
#define GENERIC_MONIT_COMPAT_SPECTRUM  (      3)

/* getting the channel profile - see component_monitor.h */
#define GENERIC_MONIT_COMPAT_PROFILE   (      4)
/* big demod-monitor-structure - see component_monitor.h */
#define GENERIC_MONIT_COMPAT_EXTENDED  (      5)

/* resetting the packet error count */
#define GENERIC_MONIT_RESET_PE_COUNT   (      6)

/* mainly used for ISDB-T */
#define GENERIC_MONIT_SET_BER_LAYER    (      7)
#define GENERIC_MONIT_BER_LAYER        (      8)

#define LAYER_ALL -1
#define LAYER_A   1
#define LAYER_B   2
#define LAYER_C   3

/* SNR */
#define GENERIC_MONIT_SNR              (      9)
/* combined SNR - same as SNR but taking the SNR of the input into account */
#define GENERIC_MONIT_SNR_COMBINED     (     10)

/* a request to the tuner to get the exact tuned frequency - 32bit integer */
#define GENERIC_MONIT_RADIO_FREQUENCY  (     11)

/* PER - 16bit integer */
#define GENERIC_MONIT_PACKET_ERROR_RATE (    12)

/* Data lock (standard independent) - 8bit integer */
#define GENERIC_MONIT_LOCK_DATA        (     13)

/* SH_FEC complete monitoring */
#define GENERIC_MONIT_CHANDEC  (     14)
#define GENERIC_MONIT_SH_FEC_EXTERNDED  GENERIC_MONIT_CHANDEC

/* SNR with signal and noise separate - 64bit integer */
#define GENERIC_MONIT_SNR_DISCRETE      (    15)

/* change ber rs len - 0..2 into offset*/
#define GENERIC_MONIT_SET_BER_RS_LEN    (    16)

/* FPGA_SHA2 complete monitoring */
#define GENERIC_MONIT_FPGA_SHA2_EXTENDED (   17)

/* request to freeze the AGC - (offset = 1 -> freeze) */
#define GENERIC_MONIT_AGC_FREEZE         (   18)

/* handle services when using the SDK wrapper */
#define GENERIC_MONIT_ADD_SERVICE        (   19)
#define GENERIC_MONIT_RM_SERVICE         (   20)
#define GENERIC_MONIT_SERVICE            (   21)
#define GENERIC_MONIT_SET_POWER_SAVING   (   22)

#define GENERIC_MONIT_CHANDEC_FROM_DEMOD (   23)
#define GENERIC_MONIT_GET_SPECTRUM_POWMEAN ( 24)

/* DAB-T-DMB */
#define GENERIC_MONIT_TDMB_NB_CURRENT_TS (   25)
#define GENERIC_MONIT_DISABLE_FRAME_CFG  (   26)
#define GENERIC_MONIT_SET_DAB_QUANT      (   27)
#define GENERIC_MONIT_IS_LOCKED          (   28)
#define GENERIC_MONIT_TDMB_SYNC_CIF      (   29)
#define GENERIC_MONIT_TDMB_CONFIG        (   30)
#define GENERIC_MONIT_SET_TDMB_FEC       (   31)
#define GENERIC_MONIT_IS_RECONFIG_DONE   (   32)

#define GENERIC_MONIT_CONNECT_ME         (   33)
#define GENERIC_MONIT_FORCE_MAX_GAIN     (   34)

/* output mode for frontends depending on the capabilities of the streaming interface */
#define OUTPUT_MODE_OFF               (0 << 0)
#define OUTPUT_MODE_ANALOG_ADC        (1 << 0)
#define OUTPUT_MODE_TS_FIFO           (1 << 1)
#define OUTPUT_MODE_TS_PARALLEL_GATED (1 << 2)
#define OUTPUT_MODE_TS_PARALLEL_CONT  (1 << 3)
#define OUTPUT_MODE_TS_SERIAL         (1 << 4)
#define OUTPUT_MODE_ITU               (1 << 5)
#define OUTPUT_MODE_I2S               (1 << 6)
#define OUTPUT_MODE_DIVERSITY         (1 << 7)
#define OUTPUT_MODE_DAB               (1 << 8)

#define FFT_AUTO	-1
#define FFT_2K		0
#define FFT_8K		1
#define FFT_4K		2
#define FFT_1K		3

#define GUARD_INTERVAL_AUTO -1
#define GUARD_INTERVAL_1_32  0
#define GUARD_INTERVAL_1_16  1
#define GUARD_INTERVAL_1_8   2
#define GUARD_INTERVAL_1_4   3

#define QAM_AUTO   -1
#define QAM_QPSK    0
#define QAM_16QAM   1
#define QAM_64QAM   2
#define QAM_DQPSK   3
#define QAM_256QAM  4
#define VSB_8       10
#define VSB_16      11
#define QAM_BPSK    12

#define INTLV_NATIVE_AUTO -1
#define INTLV_NATIVE_ON    1
#define INTLV_NATIVE_OFF   0

#define VIT_HRCH_AUTO -1
#define VIT_HRCH_OFF   0
#define VIT_HRCH_ON    1

#define VIT_PRIORITY_AUTO -1
#define VIT_PRIORITY_LP    0
#define VIT_PRIORITY_HP    1

#define VIT_ALPHA_AUTO -1
#define VIT_ALPHA_1     1
#define VIT_ALPHA_2     2
#define VIT_ALPHA_4     4

// VITERBI DECODER DVB-T/H code rates
#define VIT_CODERATE_AUTO -1
#define VIT_CODERATE_1_2   1
#define VIT_CODERATE_2_3   2
#define VIT_CODERATE_3_4   3
#define VIT_CODERATE_5_6   5
#define VIT_CODERATE_7_8   7

//TURBO DECODER DVB-SH code rates
#define TURBO_CODERATE_AUTO -1
#define TURBO_CODERATE_STD_1_5   0
#define TURBO_CODERATE_STD_2_9   1
#define TURBO_CODERATE_STD_1_4   2
#define TURBO_CODERATE_STD_2_7   3
#define TURBO_CODERATE_STD_1_3   4
#define TURBO_CODERATE_CPL_1_3   5
#define TURBO_CODERATE_STD_2_5   6
#define TURBO_CODERATE_CPL_2_5   7
#define TURBO_CODERATE_STD_1_2   8
#define TURBO_CODERATE_CPL_1_2   9
#define TURBO_CODERATE_STD_2_3   10
#define TURBO_CODERATE_CPL_2_3   11

#define DAB_TMODE_AUTO -1
#define DAB_TMODE_I	0
#define DAB_TMODE_II	1
#define DAB_TMODE_III	2
#define DAB_TMODE_IV	3

#define DAB_FAST_LOCK   0
#define DAB_ROBUST_LOCK	1
#define DAB_MAX_SUBC	64

struct dibChannel {
    uint32_t RF_kHz;
    uint32_t bandwidth_kHz;

#define STANDARD_UNKNOWN	  0
#define STANDARD_DVBT		  1
#define STANDARD_ISDBT		  2
#define STANDARD_DAB		  3
#define STANDARD_ANALOG_AUDIO 4
#define STANDARD_ANALOG       5
#define STANDARD_ATSC         6
#define STANDARD_DVBSH        7
#define STANDARD_DVBSHA2      8
#define STANDARD_CMMB         9
#define STANDARD_ISDBT_1SEG	10
#define STANDARD_CTTB         11
#define STANDARD_MISC         20
    uint8_t type;

    union std {

#ifdef CONFIG_STANDARD_DVBT
        struct dibDVBTChannel {
            int8_t spectrum_inversion;

            int8_t nfft;
            int8_t guard;
            int8_t constellation;

            int8_t hrch;
            int8_t alpha;
            int8_t code_rate_hp;
            int8_t code_rate_lp;
            int8_t select_hp;

            int8_t intlv_native;
        } dvbt;
#endif

#ifdef CONFIG_STANDARD_ISDBT
        struct dibISDBTChannel {
            int8_t sb_mode; /* 0 = 13seg, 1 = 1 or 3seg */
            int8_t partial_reception; /* sb_mode=0 -> 0=no, 1=yes, sb_mode=1 -> 0=1seg, 1=3seg */

            int8_t spectrum_inversion; /* always AUTO */

            int8_t nfft;  /* can be AUTO */
            int8_t guard; /* can be AUTO */

            int8_t sb_conn_total_seg;
            int8_t sb_wanted_seg;
            int8_t sb_subchannel;

            struct dibISDBTLayer {
                int8_t nb_segments;
                int8_t constellation;
                int8_t code_rate;
                int8_t time_intlv;
            } layer[3];
        } isdbt;
#endif

#ifdef CONFIG_STANDARD_DAB
        struct dibDABChannel {
            int8_t tmode;
            uint8_t spectrum_inversion;
            uint8_t nb_active_sub;
            uint8_t reconfig;
            int16_t cif_occurence_change;

            struct dibDABsubChannel {
                uint8_t  subChid;
                uint16_t start_address;
                uint16_t sub_size;

                uint8_t  form;
                uint8_t  table_index;
                uint8_t  option;
                uint8_t  protection_level;

                uint8_t is_ts;
                uint8_t subc_new;
                uint16_t addr_URAM;

                uint8_t requested_fec;
                uint8_t dummy0;
                uint16_t dummy1; /* binary backward compat since size prev and start adress prev have been removed */

            } subChannel[DAB_MAX_SUBC];
        } dab;
#endif

#ifdef CONFIG_STANDARD_DVBT
        struct dibDVBSHChannel {
            struct dibDVBTChannel dvb_common;
            int8_t dvb_sh;
            int8_t common_mult;
            int8_t nof_late_taps;
            int8_t nof_slices;
            int8_t slice_distance;
            int8_t non_late_incr;
        } dvbsh;

#endif

#ifdef CONFIG_STANDARD_ANALOG
        struct dibANALOGChannel {
            uint32_t  TunerStandard;
            uint32_t  CountryCode;
            uint32_t  Cable;
        } analog;
#endif

#ifdef CONFIG_STANDARD_ATSC
        struct dibATSCChannel {
			int8_t spectrum_inversion;
            int8_t modulation;
        } atsc;
#endif

        struct dibCMMBChannel
        {
            int8_t nfft;
            uint32_t config_changed_timestamp;
            uint8_t  parameters_changed; //Christophe doit un cafe a Luc si le parametre change et devient "par time slot"
            struct time_slot
            {
                struct channel_config
                {
#define RS_CRATE_1      0
#define RS_CRATE_14_15  1
#define RS_CRATE_12_15  2
#define RS_CRATE_11_15  3
                    int8_t rs_code_rate;  /* 0:240/240, 1:224/240, 2:192/240, 3:176/240    */
                    int8_t intlv_mode;    /* 1:Mode 1, 2:Mode 2, 3:Mode 3                  */
                    int8_t ldpc_cr;       /* 0:1/2, 1:3/4                                  */
                    int8_t constellation; /* 0:BPSK, 1:QPSK, 2:16-QAM                      */
                    int8_t scrambling;    /* 0:Mode 0, 1:Mode 1 ... 7:Mode 7               */
                } parameters;
                int8_t mf_id;            /* MF_IF of MF this time slot is assigned to      */
                int8_t slot_mask;
            } ts_array[40];              /* Physical layer level: Broadcast Channel Frame  */
        } cmmb;

        struct dibMISC {
            uint32_t pattern;
        } misc;

    } u;

    // private information per frontend and per channel
    struct dibChannelFEInfo {
        uint32_t tune_time_locked;  /* the time it took for this frontend to acquired mpeg lock or sh lock */
        uint32_t tune_time_data;    /* the time it took for this frontend to receiver data */

        uint32_t tune_start_time;
        uint32_t tune_time_estimation[2];

        uint32_t priv[11]; /* memory for a frontend to store some private about this channel */
    } fe_info[MAX_NB_OF_FRONTENDS_IN_CHAIN];

    struct dibChannelContext {
#define CHANNEL_STATUS_PARAMETERS_UNKNOWN   0x01
#define CHANNEL_STATUS_PARAMETERS_SET       0x02
        uint8_t status;
        uint32_t tune_time_estimation[2]; /* averaged real tuning time */
        int tps_available;
        uint16_t tps[9];
    } context;

#define MAX_MONITORING_AGE          50000
#define MIN_MONITORING_REFRESH_RATE 500
    struct dibChannelMonitor {
        uint8_t refresh; /* internal field to request a refresh of the monitoring - used by the adapter */
        uint32_t timestamp;

        /* common frontend information */
        uint32_t adc_power;
        uint16_t agc_global;
        uint16_t agc_rf;
        uint16_t agc_bb;

        uint16_t agc_wbd;        /* NV = AgcWbd/2^12           */
        uint16_t agc_wbd_split;  /* NV = AgcSplitOffset/2^8    */

        /* demod */
        uint32_t noise_power;    /* Power of Noise  - dB = 10*log10(noise_power/2^32)    */
        uint32_t signal_power;   /* Power of Signal - dB = 10*log10(signal_power/2^32)    */

        uint8_t quality;
        uint8_t quality_fast;

        /* demod locks - different represenation of depending on the standard */
        /* DVB-T:
         * [15] AgcLock     | [14] CormLock    | [13] CoffLock      | [12] CoffCpilLock |
         * [11] Lmod4Lock   | [10] Pha3Lock    | [ 9] EqualLock     | [ 8] VitLock      |
         * [ 7] FecLockFrm  | [ 6] FecLockMpeg | [ 5] TpsDecLock    | [ 4] TpsSyncLock  |
         * [ 3] TpsDataLock | [ 2] TpsBchUncor | [ 1] TpsCellIdLock | [ 0] DvsyLock     |
         DVB-SH:
         * [15] Reserved    | [14] CormLock     | [13] CoffLock      | [12] CoffCpilLock |
         * [11] Lmod4Lock   | [10] Pha3Lock     | [ 9] EqualLock     | [ 8] SHLock       |
         * [ 7] Reserved    | [ 6] Reserved     | [ 5] TpsDecLock    | [ 4] TpsSyncLock  |
         * [ 3] TpsDataLock | [ 2] TpsBchUncor  | [ 1] TpsCellIdLock | [ 0] DvsyLock     |
         TDM-B/DAB:
         * [15] Reserved      | [14] CormLockDmb  | [13] DabCoffLock   | [12] NdecTmode_Lock      |
         * [11] NdecFrameLock | [10] xxx          | [ 9] xxx           | [ 8] xxx                 |
         * [ 7] FecLockFrm    | [ 6] FecLockMpeg  | [ 5] xxx           | [ 4] xxx                 |
         * [ 3] xxx           | [ 2] xxx          | [ 1] xxx           | [ 0] tdint_full_internal |
         * ISDB-T
         * [15] AgcLock     | [14] xxx         | [13] CoffLock      | [12] xxx          |
         * [11] vit_lock_0  | [10] vit_lock_1  | [ 9] vit_lock_2    | [ 8] xxx          |
         * [ 7] mpeg_lock_0 | [ 6] mpeg_lock_1 | [ 5] mpeg_lock_2   | [ 4] xxx          |
         * [ 3] xxx         | [ 2] xxx         | [ 1] xxx           | [ 0] DvsyLock     |
         * CMMB
         * [15] xxx         | [14] xxx         | [13] xxx           | [12] xxx          |
         * [11] xxx         | [10] xxx         | [ 9] xxx           | [ 8] xxx          |
         * [ 7] xxx         | [ 6] xxx         | [ 5] xxx           | [ 4] xxx          |
         * [ 3] xxx         | [ 2] xxx         | [ 1] xxx           | [ 0] coff lock    |
         */

        uint16_t locks;
        uint16_t tmp_tune_state;
        uint16_t tmp_status;

        union {
            struct {
                uint32_t ber;
                uint16_t packet_errors;
            } dvbt;
            struct {
                uint16_t per;
            } dvbsh;
            struct {
                uint32_t ber;
                uint16_t packet_errors;
                uint16_t dmb_lock_frame;

/* Not supported for now
                uint8_t tmode;

                uint16_t syn_fic;
                uint16_t syn_msc;
                uint16_t syn_subc;
                uint16_t syn;
                uint16_t syn_subid;

                uint16_t fec_state;
                uint8_t fec_subid[12];
*/
            } dab;
            struct {
                uint32_t per;
                uint32_t packet_error;
                uint32_t syndrome_first_iter;
                uint8_t time_slot_num;
               /* [ 7] octopus ov  | [ 6] octopus ov  | [ 5] octopus ov    | [ 4] octopus ov   |
                * [ 3] octopus ov  | [ 2] octopus ov  | [ 1] reflex in ov  | [ 0] ldcp ov      |
                */
                uint8_t overflow;

            } cmmb;

            struct {
                uint32_t ber[3];
                uint16_t packet_errors[3];
            } isdbt;
        } u;
    } fe_monitor[MAX_NB_OF_FRONTENDS_IN_CHAIN];

    int chid_to_feid[MAX_NB_OF_FRONTENDS_IN_CHAIN];
};

#define CHANNEL_FE_INFO_PRIV_SIZE ( sizeof(((struct dibChannelFEInfo *) (NULL))->priv ) )


struct dibSignalQuality {
    /* syndrome is used to compute the quality - syndrome is valid on the master only - in diversity */
    uint8_t syndrome_is_set : 1; /* 0:not set, 1:set */
    uint8_t synd_sample_count; // after x samples we are adding this one to the fir_filter
    int16_t synd_mean;
    int16_t synd_std;

    uint8_t fir_synd_ptr;
    int16_t fir_synd_mean[8];
    int16_t fir_synd_std[8];
};

struct MsgHeader;
extern void                 channel_init(struct dibChannel *, uint8_t type);
struct dibChannelFEInfo *   channel_frontend_info(struct dibChannel *, int id);
struct dibChannelMonitor *  channel_frontend_monitoring(struct dibChannel *, int id);
extern uint8_t              channel_get_status(struct dibChannel *);
extern uint32_t             channel_get_tune_time_estimation(struct dibChannel *, int tune_time_type);
extern uint16_t             channel_get_default_tune_time(uint32_t type);
extern const uint16_t *     channel_get_tps_info(struct dibChannel *);
extern int                  channel_tps_available(struct dibChannel *);
extern int                  channel_monitoring_available(struct dibChannel *, int);
extern int                  channel_standard_to_mode(struct dibChannel *);

extern int                  get_chip_id(void);
extern void                 set_mac_address(uint32_t add);
extern uint32_t             get_mac_frequency(void);
extern int                  feid_to_chid(struct dibChannel* ch, int feid);

extern void                 nautilus_generic_process_async(void);
extern void                 nautilus_generic_dibctrl_irq_handler(void);
extern void                 nautilus_generic_process_host_msg(struct MsgHeader * head);
extern void                 nautilus_generic_process_slave_msg(struct MsgHeader * head, uint32_t * msg);
extern void *               nautilus_generic_get_slave_bus(void);
extern void *               nautilus_generic_get_slave_mailbox(void);

extern int                  slave_mailbox_write(void *slave_mailbox, uint32_t *b, uint16_t len_bytes);

extern void                 slave_bus_write_buf(void * slave_bus, uint32_t address, uint32_t attribute, const uint8_t *tx, uint32_t txlen);
extern void                 slave_bus_read_buf(void * slave_bus, uint32_t address, uint32_t attribute,
                                        const uint8_t *tx, uint32_t txlen, uint8_t *rx, uint32_t rxlen);


struct dibService {
	uint8_t type;

	union {
		struct {
			uint16_t pid;
			uint32_t rows;
            uint32_t burst_period;
			uint32_t max_burst_duration;
		} mpe_fec;
	} u;
};


/* absolute value */

extern uint8_t CHANNELS_EQUAL(struct dibChannel *ch1, struct dibChannel *ch2);

/* signal status - returns the status of the reception when being tuned. Not to mix up with channel_monitoring */
struct dibSignalStatus {
	/* be careful this structure is for a temprary use, we will not provide backward compatibility for this */
    uint16_t todo;
/* TODO
	uint16_t locks;
	uint32_t packet_error_rate; // has to be divided by 2^5
	uint32_t bit_error_rate;    //has to be divided by 1e8

	uint16_t equal_noise_exp; //noise = noise_mant * 2^noise_exp
	uint16_t equal_noise_mant;
	uint16_t equal_signal_mant; //signal = signal_mant * 2^signal_exp
	uint16_t equal_signal_exp;

    uint16_t tps_buf[5]; */
};


#define TYPE_TO_STRING(v) ( \
    (v) == 0  ? "STANDARD_UNKNOWN" : \
    (v) == 1  ? "STANDARD_DVBT" :    \
    (v) == 4  ? "STANDARD_FM" :      \
    (v) == 2  ? "STANDARD_ISDBT" :   \
    (v) == 3  ? "STANDARD_DAB" :     \
    (v) == 5  ? "STANDARD_ANALOG" :  \
    (v) == 20 ? "STANDARD_MISC" : "UNK" \
)

#define DVBT_FFT_TO_STRING(v) ( \
	(v) == FFT_1K ? "1K" : \
	(v) == FFT_2K ? "2K" : \
	(v) == FFT_4K ? "4K" : \
	(v) == FFT_8K ? "8K" : "UNK" \
)

#define ISDBT_FFT_TO_STRING(v) ( \
	(v) == FFT_2K ? "Mode1 (2K)" : \
	(v) == FFT_4K ? "Mode2 (4K)" : \
	(v) == FFT_8K ? "Mode3 (8K)" : "UNK" \
)

#define DVBT_FFT_TO_INDEX(v) ( \
	(v) == FFT_1K ? 3 : \
	(v) == FFT_2K ? 0 : \
	(v) == FFT_8K ? 1 : \
	(v) == FFT_4K ? 2 : -1 \
)

#define DVBT_INDEX_TO_FFT(v) ( \
	(v) == 3 ? FFT_1K : \
	(v) == 0 ? FFT_2K : \
	(v) == 1 ? FFT_8K : \
	(v) == 2 ? FFT_4K : FFT_AUTO \
)

#define ISDBT_FFT_TO_INDEX(v) ( \
	(v) == FFT_2K ? 0 : \
	(v) == FFT_4K ? 1 : \
	(v) == FFT_8K ? 2 : -1 \
)

#define ISDBT_INDEX_TO_FFT(v) ( \
	(v) == 0 ? FFT_2K : \
	(v) == 1 ? FFT_4K : \
	(v) == 2 ? FFT_8K : FFT_AUTO \
)

#define DAB_TMODE_TO_INDEX(v) ( \
	(v) == DAB_TMODE_I ? 0 : \
	(v) == DAB_TMODE_II ? 1 : \
	(v) == DAB_TMODE_III ? 2 : \
	(v) == DAB_TMODE_IV ? 3 : -1 \
)

#define DAB_INDEX_TO_TMODE(v) ( \
	(v) == 0 ? DAB_TMODE_I : \
	(v) == 1 ? DAB_TMODE_II : \
	(v) == 2 ? DAB_TMODE_III : \
	(v) == 3 ? DAB_TMODE_IV : FFT_AUTO \
)

#define DAB_TMODE_TO_STRING(v) ( \
	(v) == DAB_TMODE_I ? "T-Mode I (2K)" : \
	(v) == DAB_TMODE_II ? "T-Mode II (0.5K)" : \
	(v) == DAB_TMODE_III ? "T-Mode III (0.25K)" : \
	(v) == DAB_TMODE_IV ? "T-Mode IV (1K)" : "UNK" \
)

#define DAB_INDEX_TO_STRING(v) ( \
	(v) == 0 ? DAB_TMODE_I : \
	(v) == 1 ? DAB_TMODE_II : \
	(v) == 2 ? DAB_TMODE_III : \
	(v) == 3 ? DAB_TMODE_IV : FFT_AUTO \
)

#define CONSTELLATION_TO_STRING(v) (   \
	(v) == QAM_QPSK   ? "QPSK"   : \
	(v) == QAM_16QAM  ? "16QAM"  : \
	(v) == QAM_64QAM  ? "64QAM"  : \
	(v) == QAM_DQPSK  ? "DQPSK"  : \
	(v) == QAM_256QAM ? "256QAM" : \
	(v) == VSB_8      ? "VSB_8"  : \
	(v) == VSB_16     ? "VSB_16" : "UNK" \
)

#define DVBT_CONSTELLATION_TO_INDEX(v) ( \
	(v) == QAM_QPSK  ? 0 : \
	(v) == QAM_16QAM ? 1 : \
	(v) == QAM_64QAM ? 2 : -1 \
)

#define DVBT_INDEX_TO_CONSTELLATION(v) ( \
	(v) == 0 ? QAM_QPSK  : \
	(v) == 1 ? QAM_16QAM : \
	(v) == 2 ? QAM_64QAM : QAM_AUTO \
)

#define ISDBT_CONSTELLATION_TO_INDEX(v) ( \
	(v) == QAM_DQPSK ? 0 : \
	(v) == QAM_QPSK  ? 1 : \
	(v) == QAM_16QAM ? 2 : \
	(v) == QAM_64QAM ? 3 : -1 \
)

#define ISDBT_INDEX_TO_CONSTELLATION(v) ( \
	(v) == 0 ? QAM_DQPSK : \
	(v) == 1 ? QAM_QPSK  : \
	(v) == 2 ? QAM_16QAM : \
	(v) == 3 ? QAM_64QAM : QAM_AUTO \
)

#define GUARD_INTERVAL_TO_STRING(v) ( \
	(v) == GUARD_INTERVAL_1_32 ? "1/32" : \
	(v) == GUARD_INTERVAL_1_16 ? "1/16" : \
	(v) == GUARD_INTERVAL_1_8  ? "1/8" : \
	(v) == GUARD_INTERVAL_1_4  ? "1/4" : "UNK" \
)

#define DVBT_GUARD_INTERVAL_TO_INDEX(v) ( \
	(v) == GUARD_INTERVAL_1_32 ? 0 : \
	(v) == GUARD_INTERVAL_1_16 ? 1 : \
	(v) == GUARD_INTERVAL_1_8  ? 2 : \
	(v) == GUARD_INTERVAL_1_4  ? 3 : - 1 \
)

#define DVBT_INDEX_TO_GUARD_INTERVAL(v) ( \
	(v) == 0 ? GUARD_INTERVAL_1_32 : \
	(v) == 1 ? GUARD_INTERVAL_1_16 : \
	(v) == 2 ? GUARD_INTERVAL_1_8  : \
	(v) == 3 ? GUARD_INTERVAL_1_4  : GUARD_INTERVAL_AUTO \
)

#define ALPHA_TO_STRING(v) ( \
	(v) == VIT_ALPHA_1 ? "1" : \
	(v) == VIT_ALPHA_2 ? "2" : \
	(v) == VIT_ALPHA_4 ? "4" : "UNK" \
)

#define DVBT_ALPHA_TO_INDEX(v) ( \
	(v) == VIT_ALPHA_1 ? 0 : \
	(v) == VIT_ALPHA_2 ? 1 : \
	(v) == VIT_ALPHA_4 ? 2 : -1 \
)

#define DVBT_INDEX_TO_ALPHA(v) ( \
	(v) == 0 ? VIT_ALPHA_1 : \
	(v) == 1 ? VIT_ALPHA_2 : \
	(v) == 2 ? VIT_ALPHA_4 : VIT_ALPHA_AUTO \
)

#define CODERATE_TO_STRING(v) ( \
	(v) == VIT_CODERATE_1_2 ? "1/2" : \
	(v) == VIT_CODERATE_2_3 ? "2/3" : \
	(v) == VIT_CODERATE_3_4 ? "3/4" : \
	(v) == VIT_CODERATE_5_6 ? "5/6" : \
	(v) == VIT_CODERATE_7_8 ? "7/8" : "UNK" \
)

#define DVBT_CODERATE_TO_INDEX(v) ( \
	(v) == VIT_CODERATE_1_2 ? 0 : \
	(v) == VIT_CODERATE_2_3 ? 1 : \
	(v) == VIT_CODERATE_3_4 ? 2 : \
	(v) == VIT_CODERATE_5_6 ? 3 : \
	(v) == VIT_CODERATE_7_8 ? 4 : -1 \
)

#define DVBT_INDEX_TO_CODERATE(v) ( \
	(v)  == 0 ? VIT_CODERATE_1_2 : \
	(v)  == 1 ? VIT_CODERATE_2_3 : \
	(v)  == 2 ? VIT_CODERATE_3_4 : \
	(v)  == 3 ? VIT_CODERATE_5_6 : \
	(v)  == 4 ? VIT_CODERATE_7_8 : VIT_CODERATE_AUTO \
)

#define ISDBT_TIME_INTLV_TO_INDEX(v) ( \
	(v) == 0 ? 0 : \
	(v) == 1 ? 1 : \
	(v) == 2 ? 2 : \
	(v) == 3 ? 3 : -1\
)

#define ISDBT_INDEX_TO_TIME_INTLV(v) ( \
	(v) == 0 ? 0 : \
	(v) == 1 ? 1 : \
	(v) == 2 ? 2 : \
	(v) == 3 ? 3 : -1 \
)

#define DVBSH_CODERATE_TO_STRING(v) ( \
	(v) == TURBO_CODERATE_STD_1_5 ? "STD_1/5" : \
	(v) == TURBO_CODERATE_STD_2_9 ? "STD_2/9" : \
	(v) == TURBO_CODERATE_STD_1_4 ? "STD_1/4" : \
	(v) == TURBO_CODERATE_STD_2_7 ? "STD_2/7" : \
	(v) == TURBO_CODERATE_STD_1_3 ? "STD_1/3" : \
	(v) == TURBO_CODERATE_CPL_1_3 ? "CPL_1/3" : \
	(v) == TURBO_CODERATE_STD_2_5 ? "STD_2/5" : \
	(v) == TURBO_CODERATE_CPL_2_5 ? "CPL_2/5" : \
	(v) == TURBO_CODERATE_STD_1_2 ? "STD_1/2" : \
	(v) == TURBO_CODERATE_CPL_1_2 ? "CPL_1/2" : \
	(v) == TURBO_CODERATE_STD_2_3 ? "STD_2/3" : \
	(v) == TURBO_CODERATE_CPL_2_3 ? "CPL_2/3" : \
        (v) == TURBO_CODERATE_AUTO    ? "TCR_UNK" : "TCR_UNK" \
)

#define DVBSH_CODERATE_TO_INDEX(v) ( \
	(v) == TURBO_CODERATE_STD_1_5 ? 0 : \
	(v) == TURBO_CODERATE_STD_2_9 ? 1 : \
	(v) == TURBO_CODERATE_STD_1_4 ? 2 : \
	(v) == TURBO_CODERATE_STD_2_7 ? 3 : \
	(v) == TURBO_CODERATE_STD_1_3 ? 4 : \
	(v) == TURBO_CODERATE_CPL_1_3 ? 5 : \
	(v) == TURBO_CODERATE_STD_2_5 ? 6 : \
	(v) == TURBO_CODERATE_CPL_2_5 ? 7 : \
	(v) == TURBO_CODERATE_STD_1_2 ? 8 : \
	(v) == TURBO_CODERATE_CPL_1_2 ? 9 : \
	(v) == TURBO_CODERATE_STD_2_3 ? 10 : \
	(v) == TURBO_CODERATE_CPL_2_3 ? 11 : -1\
)

#define DVBSH_INDEX_TO_CODERATE(v) ( \
	(v) == 0 ? TURBO_CODERATE_STD_1_5 : \
	(v) == 1 ? TURBO_CODERATE_STD_2_9 : \
	(v) == 2 ? TURBO_CODERATE_STD_1_4 : \
	(v) == 3 ? TURBO_CODERATE_STD_2_7 : \
	(v) == 4 ? TURBO_CODERATE_STD_1_3 : \
	(v) == 5 ? TURBO_CODERATE_CPL_1_3 : \
	(v) == 6 ? TURBO_CODERATE_STD_2_5 : \
	(v) == 7 ? TURBO_CODERATE_CPL_2_5 : \
	(v) == 8 ? TURBO_CODERATE_STD_1_2 : \
	(v) == 9 ? TURBO_CODERATE_CPL_1_2 : \
	(v) == 10 ? TURBO_CODERATE_STD_2_3 : \
	(v) == 11 ? TURBO_CODERATE_CPL_2_3 : TURBO_CODERATE_AUTO \
)

enum dibChannelChange {
	PRE_CHANNEL_CHANGE,
	POST_CHANNEL_CHANGE
};

#define INIT_ANALOG_CHANNEL(ch) do {\
	(ch)->RF_kHz        = 0; \
	(ch)->TunerStandard = AnalogVideo_None; \
	(ch)->CountryCode   = 0; \
	(ch)->Cable         = 0; \
} while (0)

struct dibAnalogChannel {
	uint32_t RF_kHz;
	uint32_t  TunerStandard;
	uint32_t  CountryCode;
	uint32_t  Cable;
};

#define ANALOG_VIDEO_STANDARD_TO_STRING(s) \
	((s) == AnalogVideo_None)        ? "UNKNOWN" : \
	((s) == AnalogVideo_NTSC_M)      ? "NTSC_M" : \
	((s) == AnalogVideo_NTSC_M_J)    ? "NTSC_M_J" : \
	((s) == AnalogVideo_NTSC_433)    ? "NTSC_433" : \
	((s) == AnalogVideo_PAL_B)       ? "PAL_B" : \
	((s) == AnalogVideo_PAL_D)       ? "PAL_D" : \
	((s) == AnalogVideo_PAL_G)       ? "PAL_G" : \
	((s) == AnalogVideo_PAL_H)       ? "PAL_H" : \
	((s) == AnalogVideo_PAL_I)       ? "PAL_I" : \
	((s) == AnalogVideo_PAL_M)       ? "PAL_M" : \
	((s) == AnalogVideo_PAL_N)       ? "PAL_N" : \
	((s) == AnalogVideo_PAL_60)      ? "PAL_60" : \
	((s) == AnalogVideo_SECAM_B)     ? "SECAM_B" : \
	((s) == AnalogVideo_SECAM_D)     ? "SECAM_D" : \
	((s) == AnalogVideo_SECAM_G)     ? "SECAM_G" : \
	((s) == AnalogVideo_SECAM_H)     ? "SECAM_H" : \
	((s) == AnalogVideo_SECAM_K)     ? "SECAM_K" : \
	((s) == AnalogVideo_SECAM_K1)    ? "SECAM_K1" : \
	((s) == AnalogVideo_SECAM_L)     ? "SECAM_L" : \
	((s) == AnalogVideo_SECAM_L1)    ? "SECAM_L1" : \
	((s) == AnalogVideo_PAL_N_COMBO) ? "PAL_N_COMBO" : \
	((s) == AnalogVideo_PAL_B_NICAM) ? "PAL_B_NICAM" : \
	((s) == AnalogVideo_PAL_G_NICAM) ? "PAL_G_NICAM" : \
	((s) == AnalogVideo_PAL_I1)      ? "PAL_I1" : \
	((s) == AnalogVideo_NTSC_Mask)   ? "NTSC" : \
	((s) == AnalogVideo_PAL_Mask)    ? "PAL" : \
	((s) == AnalogVideo_SECAM_Mask)  ? "SECAM" : "Undefined"

/* useful tools */
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(v)  sizeof(v)/sizeof(*(v))
#endif

#define MAX_INTEL_HEX_RECORD_LENGTH 16
typedef struct {
	uint8_t  Length;
	uint16_t Address;
	uint8_t  Type;
	uint8_t  Data[MAX_INTEL_HEX_RECORD_LENGTH];
	uint8_t  CheckSum;
} INTEL_HEX_RECORD, *PINTEL_HEX_RECORD;

struct dibVideoConfig {
   uint32_t HwVbiLineSize;
   uint32_t HwVideoLineSize;
   uint32_t HwNbActiveLines;
   uint32_t Standard;
};

struct dibVideoTransitions {
	uint32_t StartOfOddField;
	uint32_t StartOfEvenField;
	uint32_t StartOfActiveOdd;
	uint32_t EndOfActiveOdd;
	uint32_t StartOfActiveEven;
	uint32_t EndOfActiveEven;
	uint32_t TotalNumberOfLines;
};

struct dibVideoFraming {
	uint32_t VideoLineSize;
	uint32_t VbiLineSize;
	uint32_t VbiLines;
	uint32_t VbiF0Start;
	uint32_t VbiF0Stop;
	uint32_t VideoF0Start;
	uint32_t VideoF0Stop;
	uint32_t VbiF1Start;
	uint32_t VbiF1Stop;
	uint32_t VideoF1Start;
	uint32_t VideoF1Stop;
};

struct dibGPIOFunction {
#define BOARD_GPIO_COMPONENT_BUS_ADAPTER 1
#define BOARD_GPIO_COMPONENT_DEMOD       2
	uint8_t component;

#define BOARD_GPIO_FUNCTION_BOARD_ON      1
#define BOARD_GPIO_FUNCTION_BOARD_OFF     2
#define BOARD_GPIO_FUNCTION_COMPONENT_ON  3
#define BOARD_GPIO_FUNCTION_COMPONENT_OFF 4
#define BOARD_GPIO_FUNCTION_SUBBAND_PWM   5
#define BOARD_GPIO_FUNCTION_SUBBAND_GPIO   6
	uint8_t function;

/* mask, direction and value are used specify which GPIO to change GPIO0
 * is LSB and possible GPIO31 is MSB.  The same bit-position as in the
 * mask is used for the direction and the value. Direction == 1 is OUT,
 * 0 == IN. For direction "OUT" value is either 1 or 0, for direction IN
 * value has no meaning.
 *
 * In case of BOARD_GPIO_FUNCTION_PWM mask is giving the GPIO to be
 * used to do the PWM. Direction gives the PWModulator to be used.
 * Value gives the PWM value in device-dependent scale.
 */
	uint32_t mask;
	uint32_t direction;
	uint32_t value;
};

#define MAX_NB_SUBBANDS   8
struct dibSubbandSelection {
	uint8_t  size; /* Actual number of subbands. */
	struct {
		uint16_t f_mhz;
        struct dibGPIOFunction gpio;
	} subband[MAX_NB_SUBBANDS];
};

struct dibFeGPIODesc {
    uint8_t feId;
    uint8_t num;
    uint8_t dir;
    uint8_t val;
};

#define FEGA_OR 1
#define FEGA_AND 0

#define FEGA_CT_ALL      0xff
#define FEGA_CT_SHUTDOWN 0x1
#define FEGA_CT_STANDBY  0x2
#define FEGA_CT_TUNSTART 0x4

struct dibFeGPIOAction {
    struct dibFeGPIODesc gpio;
    uint16_t feId_trigger; // bitfield
    uint8_t tune_state;    // bitfield
    uint8_t condition;
    uint8_t outside_band;
    uint16_t freq_MHz[2];
    uint8_t threshold_dir; // 1 : (gain > threshold) 0: (gain <= threshold)
    int16_t threshold_val; //gain threshold for this action
};

#define MAX_GPIO_ACTION_COUNT 12
/* backward compatibility for dib<Component> */
#ifndef NO_FE_COMPONENT_BACKWARD_COMPAT

#define dibDemod dibFrontend
#define dibTuner dibFrontend
#define dibAudioDecoder dibFrontend
#define dibVideoDecoder dibFrontend
#define dibSIP   dibFrontend
struct dibDemodInfo;
struct dibTunerInfo;
struct dibSIPInfo;
#ifdef CONFIG_STANDARD_ANALOG
struct dibAudioDecoderInfo;
struct dibVideoDecoderInfo;
#endif
struct dibFrontend;


/* Chip output mode. */
#define OUTMODE_HIGH_Z                      OUTPUT_MODE_OFF
#define OUTMODE_MPEG2_PAR_GATED_CLK         OUTPUT_MODE_TS_PARALLEL_GATED
#define OUTMODE_MPEG2_PAR_CONT_CLK          OUTPUT_MODE_TS_PARALLEL_CONT
#define OUTMODE_MPEG2_SERIAL                OUTPUT_MODE_TS_SERIAL
#define OUTMODE_DIVERSITY                   OUTPUT_MODE_DIVERSITY
#define OUTMODE_MPEG2_FIFO                  OUTPUT_MODE_TS_FIFO
#define OUTMODE_ANALOG_ADC                  OUTPUT_MODE_ANALOG_ADC
#define OUTMODE_DAB                         OUTPUT_MODE_DAB

#define ANALOG_VIDEO MODE_ANALOG_VIDEO
#define ANALOG_AUDIO MODE_ANALOG_AUDIO
#define ANALOG      (ANALOG_VIDEO | ANALOG_AUDIO)
#define DVB          MODE_DVBT
#define RAW_DATA     MODE_ANALOG_ADC

extern void INIT_CHANNEL(struct dibChannel *ch, uint8_t type);

#endif

/* little helpers */
#define FE_FIELD_CHECK(obj, info)      ((obj) != NULL && (obj)->info != NULL)

#ifndef FE_FUNC_CHECK
#define FE_FUNC_CHECK(obj, info, func) (FE_FIELD_CHECK(obj, info) && (obj)->info->ops.func != NULL)
#endif

#define FE_FUNC_CALL(obj, info, func)  (obj)->info->ops.func


#ifndef NO_DVBCHANNEL_BACKWARD_COMPAT
/* backward compat for DVBChannel */

/* Cofdm spectrum bandwidth index */
#define INDEX_BW_8_0_MHZ                    0
#define INDEX_BW_7_0_MHZ                    1
#define INDEX_BW_6_0_MHZ                    2
#define INDEX_BW_5_0_MHZ                    3
#define INDEX_BW_4_5_MHZ                    4
#define INDEX_BW_3_5_MHZ                    5
#define INDEX_BW_3_0_MHZ                    6
#define INDEX_BW_1_75_MHZ                   7
#define INDEX_BW_1_5_MHZ                    8
#define INDEX_BW_AUTO                       9

#define BW_INDEX_TO_KHZ(v) ( (v) == INDEX_BW_8_0_MHZ  ? 8000 : \
                             (v) == INDEX_BW_7_0_MHZ  ? 7000 : \
							 (v) == INDEX_BW_6_0_MHZ  ? 6000 : \
							 (v) == INDEX_BW_5_0_MHZ  ? 5000 : \
							 (v) == INDEX_BW_4_5_MHZ  ? 4500 : \
							 (v) == INDEX_BW_3_5_MHZ  ? 3500 : \
							 (v) == INDEX_BW_3_0_MHZ  ? 3000 : \
							 (v) == INDEX_BW_1_75_MHZ ? 1750 : \
							 (v) == INDEX_BW_1_5_MHZ  ? 1500 : 0 )

#define BW_MHZ_TO_INDEX(v) BW_KHZ_TO_INDEX(v*1000)

#define BW_KHZ_TO_INDEX(v) ( (v) == 8000  ? INDEX_BW_8_0_MHZ : \
							 (v) == 7000  ? INDEX_BW_7_0_MHZ : \
							 (v) == 6000  ? INDEX_BW_6_0_MHZ : \
							 (v) == 5000  ? INDEX_BW_5_0_MHZ : \
							 (v) == 4500  ? INDEX_BW_4_5_MHZ : \
							 (v) == 3500  ? INDEX_BW_3_5_MHZ : \
							 (v) == 3000  ? INDEX_BW_3_0_MHZ : \
							 (v) == 1750  ? INDEX_BW_1_75_MHZ : \
							 (v) == 1500  ? INDEX_BW_1_5_MHZ : INDEX_BW_8_0_MHZ )

struct dibDVBChannel {
	uint32_t RF_kHz;
	uint8_t Bw;
	int16_t nfft;
	int16_t guard;
	int16_t invspec;
	int16_t nqam;
	int16_t intlv_native;
	int16_t vit_hrch;
	int16_t vit_select_hp;
	int16_t vit_alpha;
	int16_t vit_code_rate_hp;
	int16_t vit_code_rate_lp;
};

extern void CONVERT_DVB_TO_DIGITAL_CHANNEL(struct dibDVBChannel *, struct dibChannel *);
extern void CONVERT_DIGITAL_TO_DVB_CHANNEL(struct dibChannel *, struct dibDVBChannel *);

#define INIT_DVB_CHANNEL(ch) do { \
	(ch)->RF_kHz                = 0;  \
	(ch)->Bw               = 0;  \
	(ch)->nfft             = FFT_AUTO; \
	(ch)->guard            = GUARD_INTERVAL_AUTO; \
	(ch)->invspec          = -1; \
	(ch)->nqam             = QAM_AUTO; \
	(ch)->intlv_native     = INTLV_NATIVE_AUTO; \
	(ch)->vit_hrch         = VIT_HRCH_AUTO; \
	(ch)->vit_select_hp    = VIT_PRIORITY_AUTO; \
	(ch)->vit_alpha        = VIT_ALPHA_AUTO; \
	(ch)->vit_code_rate_hp = VIT_CODERATE_AUTO; \
	(ch)->vit_code_rate_lp = VIT_CODERATE_AUTO; \
} while (0)

#define DVB_CHANNELS_EQUAL(ch1,ch2) ( \
	(ch1)->RF_kHz                == (ch2)->RF_kHz && \
	(ch1)->Bw               == (ch2)->Bw && \
	(ch1)->nfft             == (ch2)->nfft && \
	(ch1)->guard            == (ch2)->guard && \
	(ch1)->nqam             == (ch2)->nqam && \
	(ch1)->intlv_native     == (ch2)->intlv_native && \
	(ch1)->vit_hrch         == (ch2)->vit_hrch && \
	(ch1)->vit_alpha        == (ch2)->vit_alpha && \
	(ch1)->vit_code_rate_hp == (ch2)->vit_code_rate_hp && \
	( ((ch1)->vit_hrch == 1 && ((ch1)->vit_code_rate_lp == (ch2)->vit_code_rate_lp)) || (ch1)->vit_hrch != 1) \
)

#endif // NO_DVBCHANNEL_BACKWARD_COMPAT

#define assert_static(e) do { enum { assert_static_failed__ = 1/(e) }; } while (0)


#define REQUEST_FIRMWARE_OPTION_MASK         0xF0000000

extern
int platform_request_firmware(const char *name,
                              int options,
                              int size,
                              int * off,
                              int (*platform_firmware_indication)(void * ctx, uint8_t * buf, uint32_t len, uint32_t off, uint32_t done),
                              void *ctx);


#ifdef __cplusplus
}
#endif


#endif
