#ifndef ADAPTER_VIDEODECODER_H
#define ADAPTER_VIDEODECODER_H

#include <adapter/common.h>
#include <adapter/databus.h>

#ifdef __cplusplus
extern "C" {
#endif

enum dibPictureProperty {
	BRIGHTNESS,
	CONTRAST,
	HUE,
	SATURATION,
	SHARPNESS,
	MAX_PROPERTY_NUM
};

enum dibVideoInput {
   VIDEO_TUNER_INPUT = 0,
   VIDEO_COMPOSITE_INPUT,
   VIDEO_SVIDEO_INPUT,
   VIDEO_MUTE

};

struct dibVideoStatus {
	uint8_t	horiz_lock;
	uint32_t video_standard;
	uint16_t line_count;
	uint8_t	copy_protected;
};

struct dibFrontend;
struct dibAnalogVideoMonitor;

struct dibVideoDecoderInfo
{
	const char *name;
	struct {
        int  (*reset) (struct dibFrontend *);
		int  (*wakeup) (struct dibFrontend *);
		int  (*sleep) (struct dibFrontend *);

		int  (*set_input) (struct dibFrontend *, enum dibVideoInput);
		int  (*set_standard) (struct dibFrontend *, uint32_t new_standard);
		int  (*set_pic_prop) (struct dibFrontend *, enum dibPictureProperty, int32_t);

		int32_t (*get_pic_prop) (struct dibFrontend *, enum dibPictureProperty);
		void (*get_status) (struct dibFrontend *, struct dibVideoStatus *);

		void (*notify_channel_change) (struct dibFrontend *, enum dibChannelChange);
		int  (*set_resolution) (struct dibFrontend *, struct dibVideoConfig *);
		int  (*set_diversity_in) (struct dibFrontend *, int onoff);
		int  (*set_output_mode) (struct dibFrontend *, int output_mode);
		int  (*get_monitoring) (struct dibFrontend *, struct dibAnalogVideoMonitor *);

		void (*release) (struct dibFrontend *);
	} ops;

	/* if min_value = max_value = 0, the property is not handled by the component */
	struct dibVideoPropertyCapability {
		int32_t min_value;
		int32_t max_value;
		int32_t default_value;
	} property_fields[MAX_PROPERTY_NUM];

    uint8_t  powered_up;
	uint8_t  muted;

	uint32_t standard;
	enum dibVideoInput input;

	int32_t current_property[MAX_PROPERTY_NUM];

	struct dibDataBusClient data_bus_client;
};

#define FE_VIDEO_FUNC_CHECK(obj, func)  FE_FUNC_CHECK(obj, video_info, func)
#define FE_VIDEO_FUNC_CALL(obj, func)   FE_FUNC_CALL(obj,  video_info, func)

extern void video_decoder_init(struct dibFrontend *);

#define video_decoder_get_name(fe) fe->video_info->name

#define video_decoder_reset(v) (FE_VIDEO_FUNC_CHECK(v, reset) ? FE_VIDEO_FUNC_CALL(v, reset)(v) : DIB_RETURN_SUCCESS)

#define video_decoder_wake_up(v) (FE_VIDEO_FUNC_CHECK(v, wakeup) ? FE_VIDEO_FUNC_CALL(v, wakeup)(v) : DIB_RETURN_ERROR)

#define video_decoder_wakeup video_decoder_wake_up

#define video_decoder_sleep(v) (FE_VIDEO_FUNC_CHECK(v,sleep) ? FE_VIDEO_FUNC_CALL(v, sleep)(v) : DIB_RETURN_ERROR)

#define video_decoder_set_input(v, i) (FE_VIDEO_FUNC_CHECK(v,set_input) ? FE_VIDEO_FUNC_CALL(v, set_input)(v, i) : DIB_RETURN_ERROR)

#define video_decoder_set_standard(v, s) (FE_VIDEO_FUNC_CHECK(v,set_standard) ? FE_VIDEO_FUNC_CALL(v, set_standard)(v, s) : DIB_RETURN_ERROR)

#define video_decoder_set_picture_property(v, p, i) (FE_VIDEO_FUNC_CHECK(v,set_pic_prop) ? FE_VIDEO_FUNC_CALL(v, set_pic_prop)(v, p, i) : DIB_RETURN_ERROR)

#define video_decoder_get_picture_property(v, p) (FE_VIDEO_FUNC_CHECK(v,get_pic_prop) ? FE_VIDEO_FUNC_CALL(v, get_pic_prop)(v, p) : DIB_RETURN_ERROR)

#define video_decoder_get_status(v, s) do { if (FE_VIDEO_FUNC_CHECK(v,get_status)) FE_VIDEO_FUNC_CALL(v, get_status)(v, s); } while (0)

#define video_decoder_notify_channel_change(v, c) do { if (FE_VIDEO_FUNC_CHECK(v,notify_channel_change)) FE_VIDEO_FUNC_CALL(v, notify_channel_change)(v, c); } while (0)

#define video_decoder_set_resolution(v, c) (FE_VIDEO_FUNC_CHECK(v,set_resolution) ? FE_VIDEO_FUNC_CALL(v, set_resolution)(v, c) : DIB_RETURN_ERROR)

#define video_decoder_set_diversity_in(v,o) (FE_VIDEO_FUNC_CHECK(v,set_diversity_in) ? FE_VIDEO_FUNC_CALL(v, set_diversity_in)(v, o) : DIB_RETURN_ERROR)

#define video_decoder_set_output_mode(v,o) (FE_VIDEO_FUNC_CHECK(v,set_output_mode) ? FE_VIDEO_FUNC_CALL(v, set_output_mode)(v, o) : DIB_RETURN_ERROR)

#define video_decoder_set_transparent_mode(v) (FE_VIDEO_FUNC_CHECK(v,set_transparent_mode) ? FE_VIDEO_FUNC_CALL(v, set_transparent_mode)(v) : DIB_RETURN_ERROR)

#define video_decoder_get_monitoring(v, m) (FE_VIDEO_FUNC_CHECK(v,get_monitoring) ? FE_VIDEO_FUNC_CALL(v, get_monitoring)(v, m) : DIB_RETURN_ERROR)

#define video_decoder_release(v) do { if (FE_VIDEO_FUNC_CHECK(v,release)) FE_VIDEO_FUNC_CALL(v, release)(v); } while (0)

#define video_decoder_property_min_value(fe, idx) (fe != NULL && fe->video_info != NULL ? (fe)->video_info->property_fields[idx].min_value : 0)
#define video_decoder_property_max_value(fe, idx) (fe != NULL && fe->video_info != NULL ? (fe)->video_info->property_fields[idx].max_value : 0)

#define video_decoder_get_register_access(v) video_decoder_get_data_bus_client(v)
#define video_decoder_get_data_bus_client(v) &(v)->video_info->data_bus_client


#ifdef __cplusplus
}
#endif

#endif
