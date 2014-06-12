#ifndef ADAPTER_AUDIODECODER_H
#define ADAPTER_AUDIODECODER_H

#include <adapter/common.h>
#include <adapter/databus.h>

#ifdef __cplusplus
extern "C" {
#endif

enum dibAudioStandard {
    AUDIO_STANDARD_NONE = 0,
	AUDIO_STANDARD_AUTO,
    AUDIO_STANDARD_BTSC,
    AUDIO_STANDARD_EIAJ,
    AUDIO_STANDARD_NICAM_BG,
    AUDIO_STANDARD_NICAM_DK,
    AUDIO_STANDARD_NICAM_L,
    AUDIO_STANDARD_NICAM_I,
    AUDIO_STANDARD_A2_M,
    AUDIO_STANDARD_FM,
	AUDIO_STANDARD_AM_L,
};

/*this value is a bit mask*/
enum dibAudioInput {
   AUDIO_TV_INPUT = 0x1,
   AUDIO_FM_INPUT = 0x2,
   AUDIO_TUNER_INPUT = AUDIO_FM_INPUT | AUDIO_TV_INPUT,
   AUDIO_INPUT_LINE = 0x4,
   AUDIO_MUTE = 0x8,
};


struct dibAudioStatus
{
	enum dibAudioStandard standard;
	uint32_t                mode;
    uint8_t                 locked;
#define FREQUENCY_LOCKED    0
#define FREQUENCY_TOO_LOW   1
#define FREQUENCY_TOO_HIGH  2
    uint8_t                 PLL_offset;
};

struct dibAnalogAudioMonitor;

struct dibAudioDecoderInfo
{
	const char *name;
	struct {
        int  (*reset)          (struct dibFrontend *);

		int  (*wakeup)         (struct dibFrontend *);
		int  (*sleep)          (struct dibFrontend *);

		int  (*set_input)      (struct dibFrontend *, enum dibAudioInput);
		int  (*set_standard)   (struct dibFrontend *, enum dibAudioStandard);
		int  (*set_mode)       (struct dibFrontend *, uint32_t audio_mode);
		int  (*set_smpl_freq)  (struct dibFrontend *, uint32_t frequency);

		void (*get_status)     (struct dibFrontend *, struct dibAudioStatus *);
		void (*get_mode)       (struct dibFrontend *, uint32_t *audio_mode);

		void (*notify)         (struct dibFrontend *, enum dibChannelChange);

		int  (*get_monitoring) (struct dibFrontend *, struct dibAnalogAudioMonitor *);

		void (*release)        (struct dibFrontend *);
	} ops;

    uint8_t  powered_up;
	uint8_t  muted;
	uint32_t standard;
	uint32_t tv_audio_mode;
	enum dibAudioInput input;
	uint32_t frequency;

	struct dibDataBusClient data_bus_client;
};

extern void audio_decoder_init(struct dibFrontend *);

#define audio_decoder_get_name(fe) fe->audio_info->name

#define FE_AUDIO_FUNC_CHECK(obj, func)  FE_FUNC_CHECK(obj, audio_info, func)
#define FE_AUDIO_FUNC_CALL(obj, func)   FE_FUNC_CALL(obj,  audio_info, func)

#define audio_decoder_reset(a) (FE_AUDIO_FUNC_CHECK(a, reset) ? FE_AUDIO_FUNC_CALL(a, reset)(a) : DIB_RETURN_SUCCESS)

#define audio_decoder_wake_up(a) (FE_AUDIO_FUNC_CHECK(a, wakeup) ? FE_AUDIO_FUNC_CALL(a, wakeup)(a) : DIB_RETURN_ERROR)
#define audio_decoder_wakeup audio_decoder_wake_up

#define audio_decoder_sleep(a) (FE_AUDIO_FUNC_CHECK(a, sleep) ? FE_AUDIO_FUNC_CALL(a,sleep)(a) : DIB_RETURN_ERROR)

#define audio_decoder_set_input(a, i) (FE_AUDIO_FUNC_CHECK(a,set_input) ? FE_AUDIO_FUNC_CALL(a,set_input)(a, i) : DIB_RETURN_ERROR)
#define audio_decoder_set_standard(a, s) (FE_AUDIO_FUNC_CHECK(a, set_standard) ? FE_AUDIO_FUNC_CALL(a,set_standard)(a, s) : DIB_RETURN_ERROR)
#define audio_decoder_set_mode(a, m) (FE_AUDIO_FUNC_CHECK(a,set_mode) ? FE_AUDIO_FUNC_CALL(a,set_mode)(a, m) : DIB_RETURN_ERROR)
#define audio_decoder_set_sampling_frequency(a, f) (FE_AUDIO_FUNC_CHECK(a, set_smpl_freq) ? FE_AUDIO_FUNC_CALL(a, set_smpl_freq)(a, f) : DIB_RETURN_ERROR)

#define audio_decoder_get_status(a, s) do { if (FE_AUDIO_FUNC_CHECK(a, get_status)) FE_AUDIO_FUNC_CALL(a,get_status)(a, s); } while (0)
#define audio_decoder_get_mode(a, m) do { if (FE_AUDIO_FUNC_CHECK(a, get_mode)) FE_AUDIO_FUNC_CALL(a,get_mode)(a, m); } while (0)

#define audio_decoder_notify_channel_change(a, c) do { if (FE_AUDIO_FUNC_CHECK(a, notify)) FE_AUDIO_FUNC_CALL(a, notify)(a, c); } while(0)

#define audio_decoder_get_monitoring(a, m) (FE_AUDIO_FUNC_CHECK(a, get_monitoring) ? FE_AUDIO_FUNC_CALL(a, get_monitoring)(a, m) : DIB_RETURN_ERROR)

#define audio_decoder_release(a) do { if (FE_AUDIO_FUNC_CHECK(a,release)) FE_AUDIO_FUNC_CALL(a, release)(a); } while (0)

#ifdef __cplusplus
}
#endif

#endif
