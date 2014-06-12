#ifndef ADAPTER_FRONTEND_H
#define ADAPTER_FRONTEND_H

#include "common.h"

struct dibFrontend;

#include "demod.h"
#include "tuner.h"
#include "sip.h"
#include "channel_decoder.h"
#include "mac.h"
#include "antenna.h"

#ifdef CONFIG_STANDARD_ANALOG
#include "videodecoder.h"
#include "audiodecoder.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

enum restart_tune_process {
    FE_RESTART_TUNE_PROCESS_FROM_TUNER = 0,
    FE_RESTART_TUNE_PROCESS_FROM_AGC,
    FE_RESTART_TUNE_PROCESS_FROM_DEMOD,
    FE_SHUTDOWN,
};

enum frontend_tune_state {
    CT_STANDBY = 0,

    CT_START = 1,

    CT_TUNER_START = 10,
    CT_TUNER_STEP_0,
    CT_TUNER_STEP_1,
    CT_TUNER_STEP_2,
    CT_TUNER_STEP_3,
    CT_TUNER_STEP_4,
    CT_TUNER_STEP_5,
    CT_TUNER_STEP_6,
    CT_TUNER_STEP_7,
    CT_TUNER_STOP,

    CT_AGC_START = 20,
    CT_AGC_STEP_0,
    CT_AGC_STEP_1,
    CT_AGC_STEP_2,
    CT_AGC_STEP_3,
    CT_AGC_STEP_4,
    CT_AGC_STOP,

    CT_DEMOD_START = 30,
    CT_DEMOD_STEP_1,
    CT_DEMOD_STEP_2,
    CT_DEMOD_STEP_3,
    CT_DEMOD_STEP_4,
    CT_DEMOD_STEP_5,
    CT_DEMOD_STEP_6,
    CT_DEMOD_STEP_7,
    CT_DEMOD_STEP_8,
    CT_DEMOD_STEP_9,
    CT_DEMOD_STEP_10,
    CT_DEMOD_STEP_11,
    CT_DEMOD_SEARCH_NEXT = 51,
    CT_DEMOD_STEP_LOCKED,
    CT_DEMOD_STOP,

    CT_CHAN_DEC_START = 60,
    CT_CHAN_DEC_STEP_0,
    CT_CHAN_DEC_STEP_1,
    CT_CHAN_DEC_STEP_2,
    CT_CHAN_DEC_STEP_3,
    CT_CHAN_DEC_STEP_4,
    CT_CHAN_DEC_STEP_LOCKED,
    CT_CHAN_DEC_STOP,
//LOLOLO    
    CT_CAL_START = 70,
    CT_CAL_STEP_0,
    CT_CAL_STEP_1,
    CT_CAL_STEP_2,
    CT_CAL_STEP_3,
    CT_CAL_STEP_4,
    CT_CAL_STEP_5,
    CT_CAL_STEP_6,
    CT_CAL_STOP,
    
    CT_MEAS_START = 80,
    CT_MEAS_STEP_0,
    CT_MEAS_STEP_1,
    CT_MEAS_STEP_2,
    CT_MEAS_STEP_3,
    CT_MEAS_STOP,
    
    CT_DONE = 100,
    CT_SHUTDOWN,
};

/**
 * This structure represents a generic instance of a frontend. One (or more) instances of this (depending on the board) are existing per board.
 * A frontend combines a RF tuner and base-band demodulator for Digital TV, Analog Video and Analog Audio decoder.
 */
struct dibFrontend
{
    uint8_t id; /*! a unique ID of this frontend in its board */

    char description[MAX_INFO_NAME_LENGTH];

    struct dibDemodInfo *demod_info; /*! the demod interface of a frontend */
    struct dibTunerInfo *tuner_info; /*! the tuner interface of a frontend */
    struct dibAntInfo *antenna_info; /*! the antenna interface of a frontend */

#ifdef CONFIG_CORE_CHANDEC
    struct dibChannelDecoderInfo *channel_decoder_info; /*! the channel decoder interface of a frontend */
#endif

    uint8_t tuner_is_shared; /*! this field indicates whether the tuner of this frontend is shared with another frontend */

#ifdef CONFIG_STANDARD_ANALOG
    struct dibVideoDecoderInfo *video_info;
    struct dibAudioDecoderInfo *audio_info;
#endif

    struct dibAdapter  *current_adapter; /* adapter this frontend is currently plugged to */
    struct dibFrontend *input; /*! next frontend in the data chain */
    struct dibFrontend *valid_inputs[MAX_NB_OF_VALID_FRONTENDS]; /*! list of frontends that can be plugged to this frontend as input - valid data interfaces */

#ifndef NO_FE_COMPONENT_BACKWARD_COMPAT
    /* backward compatibility pointer for existing applications */
    struct dibFrontend *video;
    struct dibFrontend *audio;
    struct dibFrontend *demod;
    struct dibFrontend *tuner;

    struct dibFrontend *channel_decoder;


#define COMPONENT_LAYER_RELEASE   0x1
#define SIP_LAYER_RELEASE         0x2
    uint8_t release_compat_fe;
#endif

    struct dibLayout *layout; /*! storing the pointer of the board the component is mounted on. */

    struct dibSIPInfo *sip; /*! reference to a SIP in case it is present */
    struct dibMac     *mac; /*! reference to a MAC in case it is present */

    /* component's private memory */
    void *demod_priv;
    void *tuner_priv;
    void *video_priv;
    void *audio_priv;
#ifdef CONFIG_CORE_CHANDEC
    void *channel_decoder_priv;
#endif
    void *antenna_priv;

    enum frontend_tune_state tune_state;
    uint32_t current_rf;
    uint32_t current_bw;
    uint32_t valid_inputs_caps[MAX_NB_OF_VALID_FRONTENDS]; /*! list of capabilities of the frontend */

/* one of the following status is returned by the tune-function or more than zero which means are delay is required before calling it again */
#define FE_STATUS_TUNE_FAILED          0 // No channel was found in the given time
#define FE_STATUS_TUNE_TIMED_OUT      -1 // Tune process is not finished in the given time : continue the process later with some more time
#define FE_STATUS_TUNE_TIME_TOO_SHORT -2 // The given time was to short to make the tune process says something about the channel
#define FE_STATUS_TUNE_PENDING        -3 // Tune process is pending
#define FE_STATUS_STD_SUCCESS         -4 // A channel has been found, we know the standard, we can continue
#define FE_STATUS_FFT_SUCCESS         -5 // A channel has been found, we know the main params (Ie: FFT, Guard...) to continue
#define FE_STATUS_DEMOD_SUCCESS       -6 // A channel has been found, we are able to receive all channel params (TPS, TMCC, FIC..)
#define FE_STATUS_LOCKED              -7 // The found channel is now completly and successfully locked, but may not be stable
#define FE_STATUS_DATA_LOCKED         -8 // The found channel is now completly and successfully locked, and datas may output correctly
    int32_t status;

    /* bit masks of components, see common.h */
    uint8_t components;

    struct dibChannel *autosearch_channel_compat;
    uint32_t time_steady;
    uint8_t status_has_changed;
    uint8_t chip_Id;
    int8_t antenna_Id;
    int8_t diversity_id;
    int8_t diversity_cnt;
};

#ifndef FE_CALLBACK_TIME_NEVER
#define FE_CALLBACK_TIME_NEVER 0xffffffff
#endif

#ifdef CONFIG_STANDARD_ANALOG
extern void frontend_register_audio_decoder(struct dibFrontend *fe, struct dibAudioDecoderInfo *info, const struct dibAudioDecoderInfo *src, void *priv);
extern void frontend_unregister_audio_decoder(struct dibFrontend *fe);
extern void frontend_register_video_decoder(struct dibFrontend *fe, struct dibVideoDecoderInfo *info, const struct dibVideoDecoderInfo *src, void *priv);
extern void frontend_unregister_video_decoder(struct dibFrontend *fe);
#endif

extern void frontend_register_demod(struct dibFrontend *fe, struct dibDemodInfo *info, const struct dibDemodInfo *src, void *priv);
extern void frontend_unregister_demod(struct dibFrontend *fe);

extern void frontend_register_tuner(struct dibFrontend *fe, struct dibTunerInfo *info, const struct dibTunerInfo *src, void *priv);
extern void frontend_unregister_tuner(struct dibFrontend *fe);

extern void frontend_register_antenna(struct dibFrontend *fe, struct dibAntInfo *info, const struct dibAntInfo *src, void *priv);
extern void frontend_unregister_antenna(struct dibFrontend *fe);

extern void frontend_register_channel_decoder(struct dibFrontend *fe, struct dibChannelDecoderInfo *info, const struct dibChannelDecoderInfo *src, void *priv);
extern void frontend_unregister_channel_decoder(struct dibFrontend *fe);

extern void frontend_register_sip(struct dibFrontend *fe, struct dibSIPInfo *info, const struct dibSIPInfo *src, void *priv);
extern void frontend_unregister_sip(struct dibFrontend *fe);

/* unregisters every component of a frontend: demod, tuner, audio, video, sip, but not the MAC */
extern void frontend_unregister_components(struct dibFrontend *);

extern int frontend_is_component_present(struct dibFrontend *, uint8_t component);

extern uint32_t frontend_capabilities(struct dibFrontend *);

extern int frontend_reset(struct dibFrontend *);

extern void frontend_init(struct dibFrontend *);
#ifdef CONFIG_BUILD_HOST
extern void frontend_set_description (struct dibFrontend *, const char [MAX_INFO_NAME_LENGTH]);
#else
#define frontend_set_description(...)
#endif
extern void frontend_set_id(struct dibFrontend *, uint8_t id);

/* store a MAC in the dibFrontend  */
extern void frontend_set_mac(struct dibFrontend *fe, struct dibMac *);

extern void frontend_tune_restart(struct dibFrontend *fe, enum restart_tune_process, struct dibChannel *ch);

/* returns always the time */
extern void frontend_tune(struct dibFrontend *, struct dibChannel *);
extern int frontend_get_status(struct dibFrontend *fe);

extern uint32_t frontend_callback_time(struct dibFrontend *fe);
extern int component_ready(uint32_t t, enum frontend_tune_state st0, enum frontend_tune_state st1);

extern void frontend_sleep(struct dibFrontend *);
extern void frontend_deep_sleep(struct dibFrontend *);

extern void frontend_get_channel(struct dibFrontend *fe, struct dibChannel *ch);

/* API for shared tuners on different frontends */
extern int frontend_try_inc_tuner_use_count(struct dibFrontend *, int, int);
extern int frontend_inc_tuner_use_count(struct dibFrontend *, int);
extern int frontend_dec_tuner_use_count(struct dibFrontend *, int);

extern struct dibFrontend * shared_tuner_register(struct dibFrontend *fe, struct dibFrontend *shared_tuner);
extern struct dibFrontend * clone_tuner(struct dibFrontend *fe, struct dibFrontend *cloned_tuner);

/* generic frontend monitoring - common for tuner/demod etc */
extern int frontend_generic_monitoring(struct dibFrontend *fe, uint32_t type, uint8_t *buf, uint32_t size, uint32_t offset);
extern uint32_t frontend_get_tune_time_estimation(struct dibFrontend *fe, struct dibChannel * channel, int tune_time_type);
extern void frontend_set_steady_timestamp(struct dibFrontend *fe, uint32_t timestamp);

/* adapter API */
extern void frontend_add_valid_frontend(struct dibFrontend *, struct dibFrontend *);
extern void frontend_add_valid_frontend_caps(struct dibFrontend *, struct dibFrontend *, int caps);
extern void frontend_remove_caps(struct dibFrontend *, int caps);

extern uint16_t frontend_get_isdbt_sb_channels(struct dibFrontend *fe[], uint32_t freq , uint32_t bw, int num, struct dibChannel ch[]);

/* finite divisions with integers - used for monitoring */
extern uint32_t uint32_finite_div(uint32_t n, uint32_t d, uint32_t prec);
#define uint32_finite_to_double(d, p) (((double) (d)) / (double) (1 << (p)))

extern void frontend_wakeup(struct dibFrontend *fe);

extern int ABS(int x);

#ifdef __cplusplus
}
#endif

#endif
