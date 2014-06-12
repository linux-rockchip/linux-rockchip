#ifndef ADAPTER_DEMOD_H
#define ADAPTER_DEMOD_H

#include "frontend.h"
#include "databus.h"

#ifdef __cplusplus
extern "C" {
#endif

//! Short Status struct giving Informations about the Signal
struct dibDVBSignalStatus
{
	uint8_t     mpeg_data_lock;
	uint8_t     signal_strength;
	uint8_t     tps_dec_lock;           /* r535:  5 */
	uint8_t     tps_sync_lock;          /* r535:  4 */
	uint8_t     tps_data_lock;          /* r535:  3 */
	uint8_t     tps_cellid_lock;        /* r535:  1 */

	uint32_t    bit_error_rate;

	uint32_t    wgn_fp;                  /* dB = 10*log10(wgn_fp/2^32)    */
	uint32_t    sig_fp;                  /* dB = 10*log10(sig_fp/2^32)    */

	uint16_t    V_agc_1;
	uint16_t    V_agc_2;
};
struct dibDemodMonitor;
struct dibDemodChannelProfile;
struct dibFrontend;
struct dibChannel;
struct dibDVBChannel;

//! generic description of a Demodulator
/**
 * This struct describes a demodulator in a generic way. Each demodulator-driver has to implement one.
 */
struct dibDemodInfo {
    const char name[MAX_INFO_NAME_LENGTH]; /*! Keeps a human-readable name as a string. */
	uint32_t caps; /*! Defines the capabilities (currently unused). */
    uint16_t pid_filter_count; /*! Defines the maximum number of PIDs which can be filtered by the demod */

	struct {
        int (*reset) (struct dibFrontend *);

		int (*wakeup) (struct dibFrontend *);
		int (*sleep) (struct dibFrontend *, uint8_t deep);

		int (*agc_startup) (struct dibFrontend *, struct dibChannel *channel);
		int (*tune) (struct dibFrontend *, struct dibChannel *channel);

		int (*autosearch_start) (struct dibFrontend *, struct dibChannel *channel);
		int (*autosearch_irq) (struct dibFrontend *);
		int (*get_channel) (struct dibFrontend *, struct dibChannel *);

		int (*set_diversity_in) (struct dibFrontend *, int onoff);
		int (*set_output_mode) (struct dibFrontend *, int);
		int (*pid_filter_ctrl) (struct dibFrontend *, uint8_t onoff);
		int (*pid_filter) (struct dibFrontend *, uint8_t id, uint16_t pid, uint8_t onoff);

		int (*set_wbd_ref) (struct dibFrontend *, uint16_t);
		uint16_t (*get_wbd_ref) (struct dibFrontend *);

		int (*set_gpio) (struct dibFrontend *, uint8_t num, uint8_t dir, uint8_t val);

		int (*get_signal_status) (struct dibFrontend *, struct dibDVBSignalStatus *);

#define DEMOD_TIMF_SET    0x00
#define DEMOD_TIMF_GET    0x01
#define DEMOD_TIMF_UPDATE 0x02
		uint32_t (*ctrl_timf) (struct dibFrontend *, uint8_t ctrl, uint32_t timf);

		int (*generic_monitoring) (struct dibFrontend *, uint32_t type, uint8_t *buff, uint32_t size, uint32_t offset);

		int (*release) (struct dibFrontend *);
	} ops;

    struct dibDataBusClient data_bus_client; /*! structure the demod uses to read from and write to itself (how to access the register) */

    uint8_t ber_rs_len;

    uint32_t callback_time;
    uint32_t callback_time_agc; /* historically the AGC is in the demod */
};

#define demod_get_name(fe) fe->demod_info->name
#define demod_get_caps(fe) fe->demod_info->caps

/* stable API for dibDemodOps - use this instead of the function pointers directly */
#define FE_DEMOD_FUNC_CHECK(obj, func)  FE_FUNC_CHECK(obj, demod_info, func)
#ifndef FE_DEMOD_FUNC_CALL
#define FE_DEMOD_FUNC_CALL(obj, func)   FE_FUNC_CALL(obj,  demod_info, func)
#endif

//! Reset a demod
extern int demod_reset(struct dibFrontend *);

//! Wake up a demod
/**
 * Has to be called after sleep and before each tune - to put the demod into a defined state
 *
 * \param d pointer to a valid struct dibFrontend
 */
extern int demod_wakeup(struct dibFrontend *);
#define demod_init(d) demod_wakeup(d)

//! Software-suspend a demod.
/**
 * Put the demod to sleep per software. Note that the power consumption is
 * still higher than doing it by hardware. Temporary low-power-mode during
 * runtime.
 *
 * \param d pointer to a valid struct dibFrontend
 */
extern int demod_sleep(struct dibFrontend *);

//! Deep-sleep a demod.
/**
 * When the external power supply is shutdown one needs to call this
 * function to tell the demod-driver to do the right thing in order to
 * be able to wake up again. After resuming from a deep-sleep the
 * demod has to be reset.
 *
 * \param d pointer to a valid struct dibFrontend
 */
extern int demod_deep_sleep(struct dibFrontend *);

//! Set the COFDM bandwidth to the demod. (obsolete)
/**
 * This function is obsolete, it is only kept for
 * backward-compatibility. It is doing nothing on the hardware.
 *
 * \param d pointer to a valid struct dibFrontend
 * \param b bandwidth index see common.h for INDEX_BW_*-defines
 */
extern int demod_set_bandwidth(struct dibFrontend *, uint32_t);

//! Restart the AGCs
/**
 * Prepare the demod to restart the AGCs. Call this function before entering the AGC-startup loop (see demod_agc_startup).
 *
 * \param d pointer to a valid struct dibFrontend
 */
#define demod_agc_restart(d) do { if ((d) != NULL && (d)->demod_info != NULL) (d)->tune_state = CT_AGC_START; } while (0)

//! Stepped AGCs start
/**
 * One has to call this function in a loop until it returns -1. A
 * value different to -1 is representing the time in hundreds of
 * micro-seconds (1 == 100usec). This is the time you need to wait at
 * least before calling the function again with the same dibFrontend.
 *
 * By doing that the tuning time is reduced when having more than one demod
 * to tune (diversity). Call this function after the frequency was set on
 * the tuner.
 *
 * \param d pointer to a valid struct dibFrontend
 * \param c pointer to a valid struct dibChannel
 */
extern int demod_agc_startup_ex(struct dibFrontend *fe, struct dibChannel *ch);


extern int demod_agc_startup_fast(struct dibFrontend *fe, struct dibChannel *ch);

//! Stepped AGCs start (obsolete)
/**
 * This function is obsolete, it is only kept for
 * backward-compatibility. Please use "demod_agc_startup_ex" instead.
 *
 * \param d pointer to a valid struct dibFrontend
 * \param c pointer to a valid struct dibDVBChannel
 */
extern int demod_agc_startup(struct dibFrontend * fe , struct dibDVBChannel * ch);

//! Do a fast-tune
/**
 * This tune-function combines channel search and tune functionality.
 * It also does not sleep during the process but returns the time it
 * needs to do a step, because being called again to do the next step.
 * It is recommended to call frontend_tune to tune the whole frontend
 * (not only the demod).
 *
 * \param fe pointer to a valid struct dibFrontend
 * \param ch pointer to a valid struct dibChannel
 *
 * \return >= 0 tune is pending, call this function again, but wait
 * the returned value before doing that (milli-seconds, -1 = tune
 * failed, -2 = successfully completed
 */
extern int demod_tune_fast(struct dibFrontend *, struct dibChannel *);

//! Tune and autosearch the demod
/**
 * Tune the demod to a channel with known parameters. See common.h for all fields of struct dibChannel.
 *
 * \param demod pointer to a valid struct dibFrontend
 * \param ch pointer to a valid struct dibChannel
 */
extern int demod_tune_ex(struct dibFrontend * fe, struct dibChannel *ch);

//! Tune the demod (obsolete)
/**
 * This function is obsolete, it is only kept for
 * backward-compatibility. Please use "demod_tune_ex" instead.
 *
 * \param fe pointer to a valid struct dibFrontend
 * \param ch pointer to a valid struct dibDVBChannel
 */
extern int demod_tune(struct dibFrontend * fe, struct dibDVBChannel *ch);

//! Start the channel parameter autosearch
/**
 * Start the autosearch on a demod. This forces the demodulator to try to find a signal (of standard "type") on the tuned RF channel.
 *
 * \param d pointer to a valid struct dibFrontend
 * \param c pointer to a valid struct dibChannel. Except RF_kHz, bandwidth_kHz and type, all fields in channel can be unknown.
 */
extern int demod_autosearch_start_ex(struct dibFrontend *, struct dibChannel *);

//! Start the channel parameter autosearch (obsolete)
/**
 * This function is obsolete, it is only kept for
 * backward-compatibility. Please use "demod_autosearch_start_ex" instead.
 *
 * \param d pointer to a valid struct dibFrontend
 * \param c pointer to a valid struct dibDVBChannel. Except RF_kHz and Bw all fields in channel can be unknown.
 */
extern int demod_autosearch_start(struct dibFrontend *, struct dibDVBChannel *);

//! Poll autosearch completion
/**
 * After calling autosearch_start check if the autosearch is completed.
 *
 * \param d pointer to a valid struct dibFrontend
 *
 * \return 0 = not completed, 1 = autosearch failed, 2 = successfully completed
 */
extern int demod_autosearch_irq(struct dibFrontend *);

//! Request the channel parameters (TPS, TMCC, other)
/**
 * After successfully finishing the autosearch (or at any time after
 * tuning successfully) the channel parameters can be querried calling
 * this function. Note that RF_kHz and the bandwidth are not returned everytime.
 *
 * \param d pointer to a valid struct dibFrontend
 * \param c pointer to a valid struct dibChannel. Except RF_kHz and bandwidth_kHz all fields will be overwritten.
 */
extern int demod_get_channel(struct dibFrontend *, struct dibChannel *);

//! Request the channel parameters (TPS) (obsolete)
/**
 * This function is obsolete, it is only kept for
 * backward-compatibility. Please use "demod_get_channel" instead.
 *
 * \param d pointer to a valid struct dibFrontend
 * \param c pointer to a valid struct dibDVBChannel. Except RF_kHz and Bw all fields will be overwritten.
 */
extern int demod_get_channel_desc(struct dibFrontend *, struct dibDVBChannel *);

//! Control diversity input
/**
 * Turns on or off the diversity input combination.
 *
 * \param d pointer to a valid struct dibFrontend
 * \param m 0 = deactivate diversity input, 1 = activate combination and use both reception ways, 2 = only use diversity input
 */
extern int demod_set_diversity_in(struct dibFrontend *, int);

//! Set the output mode of a demod
/**
 * Set up the output_mode of a demod. Valid modes can be found in common.h . (e.g. OUTMODE_DIVERSITY, OUTMODE_MPEG2_SERIAL).
 *
 * \param d pointer to a valid struct dibFrontend
 * \param m output mode (default: OUTMODE_MPEG2_FIFO)
 */
extern int demod_set_output_mode(struct dibFrontend *, int);

//! PID filter control
/**
 * Enables or disabled the internal PID filter of a demod. If it is disabled,
 * the complete MPEG2 Transport is put out otherwise only selected PIDs are
 * passed through (see demod_pid_filter).
 *
 * \param d pointer to a valid struct dibFrontend
 * \param o 0 = disable PID filter (default), 1 = enable
 */
extern int demod_pid_filter_ctrl(struct dibFrontend *, uint8_t);

//! PID filter table
/**
 * Enable a PID to be filtered. Disable the consideration of a position of the internal table.
 *
 * \param d pointer to a valid struct dibFrontend
 * \param i index of the internal table
 * \param p PID to be filtered
 * \param o activate de-activate the given table index
 */
extern int demod_pid_filter(struct dibFrontend *, uint8_t, uint16_t, uint8_t);

//! PID filter count
extern int demod_pid_filter_count(struct dibFrontend *fe);

//! Set a target value for the wideband-detector
/**
 * If you use the wideband-detector you want to give a target value to make the
 * algorithm work best. If this target value needs to be defined at run-time,
 * one can use this function to set it. See the data sheet of your
 * tuner/demodulator how to compute the appropriate value.
 *
 * \param d pointer to a valid struct dibFrontend
 * \param v wbd target value
 */
extern int demod_set_wbd_ref(struct dibFrontend *, uint16_t);

//! Get the current wideband-detector target
/**
 * Get the (previously set) reference value for the wideband detector.
 *
 * \param d pointer to a valid struct dibFrontend
 * \return wideband-detector target value
 */
extern int demod_get_wbd_ref(struct dibFrontend *);

//! Get tuning information
/**
 * To retrieve very basic information about the tuned signal this function can be used.
 *
 * \param d pointer to a valid struct dibFrontend
 * \param s pointer to a valid struct dibDVBSignalStatus
 */
extern int demod_get_signal_status(struct dibFrontend *, struct dibDVBSignalStatus *);

//! Manipulate the timing frequency
/**
 * Controlling the timing frequency of a demod. This function can be used to
 * set, get and update the timing frequency. Having an accurate timing
 * frequency can improved the tuning speed of your system. The timing
 * frequency is in relation to the accuracy of the crystal for the demod.
 *
 * \param d pointer to a valid struct dibFrontend
 * \param op can be DEMOD_TIMF_SET (use t as timf for now), DEMOD_TIMF_GET (the
 * current (corrected) timf value is returned), DEMOD_TIMF_UPDATE (force an
 * update of the timf (best called before tuning to another channel while being
 * tuned)
 * \param t timf-value when calling with DEMOD_TIMF_SET
 * \return corrected timf when calling with DEMOD_TIMF_GET
 */
extern int demod_ctrl_timing_frequency(struct dibFrontend *, uint8_t, uint32_t);

//! Manipulate the GPIO of a demod
/**
 * Use this function to manipulate the state of a GPIO of the demodulator.
 *
 * \param d pointer to a valid struct dibFrontend
 * \param num number of GPIO to control (if in doubt do not touch at least GPIO1,2 and GPIO3,4)
 * \param dir GPIO direction 0 = OUT, 1 = IN
 * \param val state of the GPIO (valid only in direction out) -> 0 = low, 1 = high
 */
extern int demod_set_gpio(struct dibFrontend *, uint8_t, uint8_t, uint8_t);

/* backward compatiblity layer for extended monitoring */
extern int demod_get_monitoring(struct dibFrontend *, struct dibDemodMonitor *);
extern int demod_set_viterbi(struct dibFrontend *, int);
extern int demod_get_carrier_power(struct dibFrontend *, int, double[3], double[3]);
extern int demod_get_channel_profile(struct dibFrontend *, struct dibDemodChannelProfile *);
extern int demod_reset_packet_error_count(struct dibFrontend *);

extern void demod_set_ber_monitored_layer(struct dibFrontend *, int);
extern int demod_get_ber_monitored_layer(struct dibFrontend *);

extern int demod_agc_state(struct dibFrontend *);

extern void demod_set_ber_rs_len(struct dibFrontend *, uint8_t);

//! Free demod memory
/**
 * Calling the function cleans up and frees private memory.
 *
 * \param d pointer to a valid struct dibFrontend
 */
extern void demod_release(struct dibFrontend *);

extern struct dibDataBusClient * demod_get_data_bus_client(struct dibFrontend *);
extern struct dibDataBusClient * demod_get_register_access(struct dibFrontend *);

extern int demod_post_tune(struct dibFrontend *fe, struct dibChannel *ch);
extern void demod_gpio_function(struct dibFrontend *, struct dibGPIOFunction *);
#ifdef __cplusplus
}
#endif

#endif
