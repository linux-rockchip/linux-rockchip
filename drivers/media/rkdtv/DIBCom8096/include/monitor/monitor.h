#ifndef MONITOR_H
#define MONITOR_H

#include "../adapter/component_monitor.h"

#ifdef CONFIG_STANDARD_ANALOG
#ifdef ANALOG_DECODER_CX2584X_MONITOR
#include <analog_decoder/cx2584x_monitor.h>
#endif
#endif

#ifdef CONFIG_TUNER_XC3028
#include <tuner/xc3028.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct dibBoardMonitor {
	int dib7000_count;
	struct dibDemodMonitor dib7000[4];

#ifdef CONFIG_STANDARD_ANALOG
#ifdef ANALOG_DECODER_CX2584X_MONITOR
	int cx2584x_count;
	struct cx2584x_monitor cx2584x;
#endif
#endif

#ifdef CONFIG_TUNER_XC3028
	int xc3028_count;
	struct xc3028_monitor xc3028;
#endif

#ifdef CONFIG_STANDARD_ANALOG
	int dib5000_count;
	struct dibAnalogVideoMonitor dib5000_video[3];
	struct dibAnalogAudioMonitor dib5000_audio[3];
#endif

	int dib0070_count;
	struct dibTunerMonitor dib0070[4];

	int channel_decoder_count;
    struct dibChannelDecoderMonitor dib_channel_decoder[2];

	int mac_count;
    struct dibMacMonitor dib_mac[2];
};

extern void dib7000_print_monitor(struct dibDemodMonitor *m, struct dibDemodStatistic s[], int stat_count, int num);
extern void channel_decoder_print_monitor(struct dibChannelDecoderMonitor *m, int num);
extern void mac_print_monitor(struct dibMacMonitor m_mac[], struct dibDemodMonitor m_demod[], int num);

struct cx2584x_monitor;
extern void cx2584x_print_monitor(struct cx2584x_monitor *m, int num);

struct xc3028_monitor;
extern void xc3028_print_monitor(struct xc3028_monitor *m, int num);

extern void dib5000_video_print_monitor(struct dibAnalogVideoMonitor *m, int num);
extern void dib5000_audio_print_monitor(struct dibAnalogAudioMonitor *m, int num);

extern void dib0070_tuner_print_monitor(struct dibTunerMonitor *m, int num);

#ifdef __cplusplus
}
#endif

#endif
