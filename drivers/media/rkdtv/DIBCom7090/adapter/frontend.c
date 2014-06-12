#include <adapter/frontend.h>

#ifndef NO_SHARED_TUNER
#include <adapter/adapter.h>
#endif

void frontend_register_demod(struct dibFrontend *fe, struct dibDemodInfo *info, const struct dibDemodInfo *src, void *priv)
{
    fe->demod_info = info;
	memcpy(info, src, sizeof(struct dibDemodInfo));
    fe->demod_priv = priv;
#ifndef NO_FE_COMPONENT_BACKWARD_COMPAT
    fe->demod = fe;
#endif
    fe->components |= FE_COMPONENT_DEMOD;
}

void frontend_unregister_demod(struct dibFrontend *fe)
{
    demod_release(fe); fe->demod_info = fe->demod_priv = NULL;
}

void frontend_register_tuner(struct dibFrontend *fe, struct dibTunerInfo *info, const struct dibTunerInfo *src, void *priv)
{
    fe->tuner_info = info;
	memcpy(info, src, sizeof(struct dibTunerInfo));
    fe->tuner_priv = priv;
#ifndef NO_FE_COMPONENT_BACKWARD_COMPAT
    fe->tuner = fe;
#endif
    fe->components |= FE_COMPONENT_TUNER;
}

void frontend_unregister_tuner(struct dibFrontend *fe)
{
    if (fe->tuner_is_shared)
        fe->tuner_is_shared--;
    else
        tuner_release(fe);
    fe->tuner_info = NULL; fe->tuner_priv = NULL;
}

#ifdef CONFIG_CORE_ANTENNA
void frontend_register_antenna(struct dibFrontend *fe, struct dibAntInfo *info, const struct dibAntInfo *src, void *priv)
{
    fe->antenna_info = info;
	memcpy(info, src, sizeof(struct dibAntInfo));
    fe->antenna_priv = priv;
    fe->components |= FE_COMPONENT_ANTENNA;
}

void frontend_unregister_antenna(struct dibFrontend *fe)
{
    antenna_release(fe);
    fe->antenna_info = NULL; fe->antenna_priv = NULL;
}
#endif

#ifdef CONFIG_CORE_CHANDEC
void frontend_register_channel_decoder(struct dibFrontend *fe, struct dibChannelDecoderInfo *info, const struct dibChannelDecoderInfo *src, void *priv)
{

    fe->channel_decoder_info = info;
        memcpy(info, src, sizeof(struct dibChannelDecoderInfo));
    fe->channel_decoder_priv = priv;
    fe->components |= FE_COMPONENT_CHANDEC;
}

void frontend_unregister_channel_decoder(struct dibFrontend *fe)
{
    channel_decoder_release(fe); fe->channel_decoder_info = fe->channel_decoder_priv = NULL;
}
#endif

#ifdef CONFIG_STANDARD_ANALOG
void frontend_register_audio_decoder(struct dibFrontend *fe, struct dibAudioDecoderInfo *info, const struct dibAudioDecoderInfo *src, void *priv)
{
    fe->audio_info = info;
	memcpy(info, src, sizeof(struct dibAudioDecoderInfo));
    fe->audio_priv = priv;
#ifndef NO_FE_COMPONENT_BACKWARD_COMPAT
    fe->audio = fe;
#endif
#if 0
    fe->components |= FE_COMPONENT_AUDIO;
#endif
}

void frontend_unregister_audio_decoder(struct dibFrontend *fe)
{
    audio_decoder_release(fe); fe->audio_info = NULL; fe->audio_priv = NULL;
}

void frontend_register_video_decoder(struct dibFrontend *fe, struct dibVideoDecoderInfo *info, const struct dibVideoDecoderInfo *src, void *priv)
{
    fe->video_info = info;
	memcpy(info, src, sizeof(struct dibVideoDecoderInfo));
    fe->video_priv = priv;
#ifndef NO_FE_COMPONENT_BACKWARD_COMPAT
    fe->video = fe;
#endif
#if 0
    fe->components |= FE_COMPONENT_VIDEO;
#endif
}

void frontend_unregister_video_decoder(struct dibFrontend *fe)
{
    video_decoder_release(fe); fe->video_info = NULL; fe->video_priv = NULL;
}
#endif

void frontend_register_sip(struct dibFrontend *fe, struct dibSIPInfo *sip, const struct dibSIPInfo *src, void *priv)
{
    fe->sip = sip;
	memcpy(sip, src, sizeof(struct dibSIPInfo));
    fe->sip->priv = priv;
}

void frontend_unregister_sip(struct dibFrontend *fe)
{
    sip_release(fe); fe->sip = NULL;
}

void frontend_init(struct dibFrontend *fe)
{
	DibZeroMemory(fe,sizeof(struct dibFrontend));
}

uint32_t frontend_capabilities(struct dibFrontend *fe)
{
    uint32_t c = 0;

    if (fe->demod_info)
        c |= fe->demod_info->caps;

#ifdef CONFIG_CORE_CHANDEC
    if (fe->channel_decoder_info)
        c |= fe->channel_decoder_info->caps;
#endif

    /* if (fe->tuner_info)
        c |= FE_CAPS_; */
#ifdef CONFIG_STANDARD_ANALOG
    if (fe->video_info)
        c |= MODE_ANALOG_VIDEO;

    if (fe->audio_info)
        c |= MODE_ANALOG_AUDIO;
#endif
    return c;
}

#ifdef CONFIG_BUILD_HOST
void frontend_set_description(struct dibFrontend *fe, const char name[MAX_INFO_NAME_LENGTH])
{
    strncpy(fe->description, name, MAX_INFO_NAME_LENGTH);
}
#endif

void frontend_set_id(struct dibFrontend *fe, uint8_t id)
{
    fe->id = id;
}

void frontend_wakeup(struct dibFrontend *fe)
{
    tuner_wakeup(fe);
    demod_wakeup(fe);
#ifdef CONFIG_CORE_CHANDEC
    channel_decoder_wakeup(fe);
#endif

#ifdef CONFIG_STANDARD_ANALOG
    video_decoder_wakeup(fe);
    audio_decoder_wakeup(fe);
#endif
}

void frontend_sleep(struct dibFrontend *fe)
{
    tuner_sleep(fe);
    demod_sleep(fe);
#ifdef CONFIG_CORE_CHANDEC
    channel_decoder_sleep(fe);
#endif
    mac_sleep(fe->mac,0);
#ifdef CONFIG_STANDARD_ANALOG
    video_decoder_sleep(fe);
    audio_decoder_sleep(fe);
#endif
    fe->status = FE_STATUS_TUNE_PENDING;
    fe->tune_state = CT_STANDBY;
}

#ifdef CONFIG_BUILD_HOST
void frontend_deep_sleep(struct dibFrontend *fe)
{
    //tuner_deep_sleep(fe);
    demod_deep_sleep(fe);
#ifdef CONFIG_CORE_CHANDEC
    channel_decoder_deep_sleep(fe);
#endif
	mac_sleep(fe->mac,1);
#ifdef CONFIG_STANDARD_ANALOG
    video_decoder_sleep(fe);
    audio_decoder_sleep(fe);
#endif
    fe->current_rf = 0;
    fe->current_bw = 0;
}

void frontend_unregister_components(struct dibFrontend *fe)
{
    frontend_unregister_sip(fe);
    frontend_unregister_demod(fe);
    frontend_unregister_tuner(fe);

#ifdef CONFIG_CORE_CHANDEC
    frontend_unregister_channel_decoder(fe);
#endif

#ifdef CONFIG_STANDARD_ANALOG
    frontend_unregister_audio_decoder(fe);
    frontend_unregister_video_decoder(fe);
#endif
}
#endif

int frontend_reset(struct dibFrontend *fe)
{
    if (demod_reset(fe) != DIB_RETURN_SUCCESS)
        return DIB_RETURN_ERROR;
    if (tuner_reset(fe) != DIB_RETURN_SUCCESS)
        return DIB_RETURN_ERROR;
#ifdef CONFIG_CORE_CHANDEC
    if (channel_decoder_reset(fe) != DIB_RETURN_SUCCESS)
        return DIB_RETURN_ERROR;
#endif
#ifdef CONFIG_STANDARD_ANALOG
    if (audio_decoder_reset(fe) != DIB_RETURN_SUCCESS)
        return DIB_RETURN_ERROR;
    if (video_decoder_reset(fe) != DIB_RETURN_SUCCESS)
        return DIB_RETURN_ERROR;
#endif
    return DIB_RETURN_SUCCESS;
}

void frontend_set_mac(struct dibFrontend *fe, struct dibMac *mac)
{
    fe->mac = mac;
}

void frontend_tune_restart(struct dibFrontend *fe, enum restart_tune_process option, struct dibChannel * ch)
{
    struct dibChannelFEInfo *info = channel_frontend_info(ch, fe->id);

    fe->status = FE_STATUS_TUNE_PENDING;
    switch(option) {
        default:
        case FE_RESTART_TUNE_PROCESS_FROM_TUNER:
            info->tune_time_locked = info->tune_time_estimation[TUNE_TIME_LOCKED] = ch->context.tune_time_estimation[TUNE_TIME_LOCKED];
            info->tune_time_data = info->tune_time_estimation[TUNE_TIME_DATA] = ch->context.tune_time_estimation[TUNE_TIME_DATA];

            info->tune_start_time = fe->time_steady = systime();
            dbgpl(NULL,"Full restart of fe %d (from TUNER)",fe->id);
            fe->tune_state = CT_START;

            if (fe->demod_info)
                fe->demod_info->callback_time_agc = FE_CALLBACK_TIME_NEVER;
            if (fe->demod_info)
                fe->demod_info->callback_time = FE_CALLBACK_TIME_NEVER;
#ifdef CHANDEC_SH_FEC
            if (fe->channel_decoder_info)
                fe->channel_decoder_info->callback_time = FE_CALLBACK_TIME_NEVER;
#endif

            break;
        case FE_RESTART_TUNE_PROCESS_FROM_AGC:
            dbgpl(NULL,"Partial restart of fe %d (from AGC only)",fe->id);

            if (fe->tune_state > CT_AGC_START)
                fe->tune_state = CT_AGC_START;

            if (fe->demod_info)
                fe->demod_info->callback_time = FE_CALLBACK_TIME_NEVER;
#ifdef CHANDEC_SH_FEC
            if (fe->channel_decoder_info)
                fe->channel_decoder_info->callback_time = FE_CALLBACK_TIME_NEVER;
#endif

            break;
        case FE_RESTART_TUNE_PROCESS_FROM_DEMOD:
            dbgpl(NULL,"Partial restart of fe %d (from DEMOD only)",fe->id);

            if (fe->tune_state > CT_DEMOD_START)
                fe->tune_state = CT_DEMOD_START;

#ifdef CHANDEC_SH_FEC
            if (fe->channel_decoder_info)
                fe->channel_decoder_info->callback_time = FE_CALLBACK_TIME_NEVER;
#endif
            break;
        case FE_SHUTDOWN:
            fe->tune_state = CT_SHUTDOWN;
            break;
    }
}

#if 0
static void frontend_set_bw(struct dibFrontend *fe, struct dibChannel *ch)
{
    if (fe->current_bw != ch->bandwidth_kHz) {
        tuner_set_bandwidth_ex(fe, ch);
        fe->current_bw = ch->bandwidth_kHz;
	//demod_set_bandwidth_and_update_timf(TIMF_SET_BANDWIDTH);
    }
}
#endif

void frontend_get_channel(struct dibFrontend *fe, struct dibChannel *ch)
{
    demod_get_channel(fe,ch);
    ch->context.status = CHANNEL_STATUS_PARAMETERS_SET;
}

int frontend_get_status(struct dibFrontend *fe)
{
    return fe->status;
}

uint32_t frontend_get_tune_time_estimation(struct dibFrontend *fe, struct dibChannel * channel, int tune_time_type)
{
    struct dibChannelFEInfo *info = channel_frontend_info(channel, fe->id);
    return info->tune_time_estimation[tune_time_type];
}

void frontend_set_steady_timestamp(struct dibFrontend *fe, uint32_t timestamp)
{
    if(timestamp > fe->time_steady)
        fe->time_steady = timestamp;
    //dbgpl(NULL,"feID = %d, fe->time_steady = %u", fe->id, fe->time_steady);
}

int component_ready(uint32_t t, enum frontend_tune_state st0, enum frontend_tune_state st1)
{
    if (st0 == st1 || (t <= systime() && st0 > st1) || (st0 == CT_SHUTDOWN) )
        return 1;

    return 0;
}

uint32_t frontend_callback_time(struct dibFrontend *fe)
{
    uint32_t ret;
    switch (fe->tune_state) {
    case CT_TUNER_START:
    case CT_AGC_START:
    case CT_DEMOD_START:
    case CT_CHAN_DEC_START:
        return 0;
    case CT_STANDBY:
        return FE_CALLBACK_TIME_NEVER;
    default:
        break;
    }

    ret = FE_CALLBACK_TIME_NEVER;

    if (fe->tuner_info && fe->tuner_info->callback_time < ret) {
        ret = fe->tuner_info->callback_time;
        //dbgpl(NULL,"fe %d_callback_time : Tuner is answering : %d",fe->id, ret);
    }
    if (fe->demod_info && fe->demod_info->callback_time_agc < ret) {
        ret = fe->demod_info->callback_time_agc;
        //dbgpl(NULL,"fe %d_callback_time : Demod AGC is answering : %d",fe->id,ret);
    }
    if (fe->demod_info && fe->demod_info->callback_time < ret) {
        ret = fe->demod_info->callback_time;
        //dbgpl(NULL,"fe %d_callback_time : Demod is answering : %d",fe->id,ret);
    }
#ifdef CONFIG_CORE_CHANDEC
    if (fe->channel_decoder_info && fe->channel_decoder_info->callback_time < ret) {
        ret = fe->channel_decoder_info->callback_time;
        //dbgpl(NULL,"fe %d_callback_time : Channel decoder is answering : %d",fe->id, ret);
    }
#endif
    //dbgpl(NULL, "now: %d, cb: %d", systime(), ret);
    return ret;
}

void frontend_tune(struct dibFrontend *fe, struct dibChannel *ch)
{
    int ret = 0;
    struct dibChannelFEInfo *fe_info = channel_frontend_info(ch, fe->id);
    uint8_t alpha = 5;
    int16_t current_status = frontend_get_status(fe);
#ifdef WAVE_ADAPT
    enum frontend_tune_state *tune_state = &fe->tune_state;
#endif

    /* start the tuning process */
    if (fe->tune_state == CT_START) {
        frontend_wakeup(fe);
        fe->tune_state = CT_TUNER_START;
        fe->current_bw = ch->bandwidth_kHz;
    }

    /* nothing to do */
    if (frontend_callback_time(fe) > systime()) {
        //dbgpl(NULL,"fe%d tune state = %d  : nothing to do ! %x", fe->id, fe->tune_state, frontend_callback_time(fe));
        return;
    }

    /* TUNER */
    ret = tuner_tune_fast(fe, ch);
    if (ret == DIB_RETURN_NOT_SUPPORTED)
        if (fe->tune_state >= CT_TUNER_START && fe->tune_state < CT_DEMOD_START)
            fe->tune_state = CT_DEMOD_START; /* skip the TUNER */

    /* AGC */
    ret = demod_agc_startup_fast(fe, ch);
    if (ret == DIB_RETURN_NOT_SUPPORTED)
        if (fe->tune_state >= CT_AGC_START && fe->tune_state < CT_DEMOD_START)
            fe->tune_state = CT_DEMOD_START; /* directly go to DEMOD */

    /* DEMOD */
    ret = demod_tune_fast(fe, ch);
    if (ret == DIB_RETURN_NOT_SUPPORTED)
        if (fe->tune_state >= CT_DEMOD_START && fe->tune_state < CT_CHAN_DEC_START)
#ifdef CONFIG_CORE_CHANDEC
            fe->tune_state = CT_CHAN_DEC_START; /* directly go to CHANDEC */

    /* CHANDEC */
    ret = channel_decoder_tune(fe, ch);
    if (ret == DIB_RETURN_NOT_SUPPORTED)
        if (fe->tune_state >= CT_CHAN_DEC_START && fe->tune_state < CT_DONE)
#endif
            fe->tune_state = CT_DONE; /* done */

    if(fe->status == FE_STATUS_LOCKED)
        fe_info->tune_time_locked = systime() - fe_info->tune_start_time;

    if(fe->status == FE_STATUS_DATA_LOCKED && (fe->tune_state == CT_DEMOD_STOP || fe->tune_state == CT_CHAN_DEC_STOP))
        fe_info->tune_time_data = systime() - fe_info->tune_start_time;

    /* Increase smoothly estimated time (10ms step) if FE_STATUS_TUNE_FAILED */
    if((fe->tune_state == CT_DEMOD_STOP || fe->tune_state == CT_CHAN_DEC_STEP_LOCKED) && fe->status == FE_STATUS_TUNE_FAILED) {
        if(fe_info->tune_time_locked < channel_get_default_tune_time(ch->type))
            fe_info->tune_time_locked+= 100;
        if(fe_info->tune_time_data < channel_get_default_tune_time(ch->type))
            fe_info->tune_time_data+= 100;
    }

    if((fe->tune_state == CT_DEMOD_STOP || fe->tune_state == CT_CHAN_DEC_STOP) && (fe->status == FE_STATUS_DATA_LOCKED || fe->status == FE_STATUS_LOCKED)) {
        alpha = 8;
        fe_info->tune_time_estimation[TUNE_TIME_LOCKED] = 400 + ((10-alpha) * fe_info->tune_time_estimation[TUNE_TIME_LOCKED] + (alpha * fe_info->tune_time_locked))/10; /* 40ms static margin */
        fe_info->tune_time_estimation[TUNE_TIME_DATA] = 400 + ((10-alpha) * fe_info->tune_time_estimation[TUNE_TIME_DATA] + (alpha * fe_info->tune_time_data))/10; /* 40ms static margin */

        /* saturations of tune_time_ estimation */
        if(fe_info->tune_time_estimation[TUNE_TIME_LOCKED] < 300)
            fe_info->tune_time_estimation[TUNE_TIME_LOCKED] = 300; /* tune can not take less than 30 ms*/
        if(fe_info->tune_time_estimation[TUNE_TIME_LOCKED] > 100000)
            fe_info->tune_time_estimation[TUNE_TIME_LOCKED] = 100000; /* tune can not take more than 10s*/

        /* saturations of tune_time_ estimation */
        if(fe_info->tune_time_estimation[TUNE_TIME_DATA] < fe_info->tune_time_estimation[TUNE_TIME_LOCKED])
            fe_info->tune_time_estimation[TUNE_TIME_DATA] = fe_info->tune_time_estimation[TUNE_TIME_LOCKED]; /* tune can not take less than the time to lock */
        if(fe_info->tune_time_estimation[TUNE_TIME_DATA] > 100000)
            fe_info->tune_time_estimation[TUNE_TIME_DATA] = 100000; /* tune can not take more than 10s*/

#ifdef WAVE_ADAPT
    WaveNetUpdate(NET_tune_time_est_locked_fe_0+fe->id, fe_info->tune_time_estimation[TUNE_TIME_LOCKED]);
    WaveNetUpdate(NET_tune_time_est_data_fe_0+fe->id, fe_info->tune_time_estimation[TUNE_TIME_DATA]);
#endif

    }
#ifdef WAVE_ADAPT
    WaveNetUpdate(NET_tune_state_fe_0+fe->id, *tune_state);
#endif

    /* activate the next component */
    if (fe->tune_state == CT_TUNER_STOP)
        fe->tune_state = CT_AGC_START;
    if (fe->tune_state == CT_AGC_STOP)
        fe->tune_state = CT_DEMOD_START;
    if (fe->tune_state == CT_DEMOD_STOP)
#ifdef CONFIG_CORE_CHANDEC
        fe->tune_state = CT_CHAN_DEC_START;
    if (fe->tune_state == CT_CHAN_DEC_STOP)
#endif
        fe->tune_state = CT_DONE;

    if (current_status ==  frontend_get_status(fe)) {
        fe->status_has_changed = 0;
    } else {
        //Trace2Host("status of FE%d has changed %d -> %d\n", fe->id, current_status, frontend_get_status(fe));
        fe->status_has_changed = 1;
    }
}

#ifndef NO_SHARED_TUNER

#define ADAPTER_MODE_TO_COUNT(mode) \
	((mode) == MODE_ANALOG_VIDEO || (mode) == MODE_ANALOG_AUDIO) ? 1 : \
	((mode) & MODE_DIGITAL) || (((mode) & MODE_ANALOG) == MODE_ANALOG) ? 2 : 99

int frontend_try_inc_tuner_use_count(struct dibFrontend *frontend, int mode, int force)
{
    int return_value;

	if (frontend == NULL)
		return DIB_RETURN_SUCCESS;

    if (force)
        tuner_dec_use_count(frontend, ADAPTER_MODE_TO_COUNT(frontend->current_adapter->current_mode));

    return_value = tuner_try_inc_use_count(frontend, ADAPTER_MODE_TO_COUNT(mode));

    if (force)
        tuner_inc_use_count(frontend, ADAPTER_MODE_TO_COUNT(frontend->current_adapter->current_mode));

    return return_value;
}

int frontend_inc_tuner_use_count(struct dibFrontend *frontend, int adap_mode)
{
	if (frontend)
		return tuner_inc_use_count(frontend, ADAPTER_MODE_TO_COUNT(adap_mode));
	else
		return DIB_RETURN_SUCCESS;
}

int frontend_dec_tuner_use_count(struct dibFrontend *frontend, int adap_mode)
{
	if (frontend)
		return tuner_dec_use_count(frontend, ADAPTER_MODE_TO_COUNT(adap_mode));
	else
		return DIB_RETURN_SUCCESS;
}

struct dibFrontend * shared_tuner_register(struct dibFrontend *fe, struct dibFrontend *shared_tuner)
{
    if (shared_tuner->tuner_info == NULL)
        return NULL;

    fe->tuner_info = shared_tuner->tuner_info;
    fe->tuner_priv = shared_tuner->tuner_priv;
    fe->tuner_is_shared ++;
    return fe;
}
#endif

static int tuner_release_mem(struct dibFrontend *fe) {

    MemFree(fe->tuner_info, sizeof(struct dibTunerInfo));
    return DIB_RETURN_SUCCESS;
}

struct dibFrontend *clone_tuner(struct dibFrontend *fe, struct dibFrontend *cloned_tuner) {

    fe->tuner_info = MemAlloc(sizeof(struct dibTunerInfo));
    memcpy(fe->tuner_info, cloned_tuner->tuner_info, sizeof(struct dibTunerInfo));
    fe->tuner_priv = cloned_tuner->tuner_priv;
    fe->tuner_info->ops.reset = NULL;
    fe->tuner_info->ops.release = tuner_release_mem;
    if ((fe->demod_info !=NULL) && (fe->demod_info->ops.agc_startup == NULL))
        fe->demod_info->ops.agc_startup = cloned_tuner->demod_info->ops.agc_startup;

    fe->components |= FE_COMPONENT_TUNER;

    return fe;
}

int frontend_generic_monitoring(struct dibFrontend *fe, uint32_t type, uint8_t *buf, uint32_t size, uint32_t offset)
{
    int ret = DIB_RETURN_NOT_SUPPORTED;
    if (fe == NULL)
        return ret;

    if (fe->demod_info && fe->demod_info->ops.generic_monitoring)
		ret = fe->demod_info->ops.generic_monitoring(fe, type, buf, size, offset);
    if (ret < 0 && fe->tuner_info && fe->tuner_info->ops.generic_monitoring)
		ret = fe->tuner_info->ops.generic_monitoring(fe, type, buf, size, offset);
#ifdef CONFIG_CORE_CHANDEC
    if (ret < 0 && fe->channel_decoder_info && fe->channel_decoder_info->ops.generic_monitoring)
        ret = fe->channel_decoder_info->ops.generic_monitoring(fe, type, buf, size, offset);
#endif
    return ret;
}

/* calculation tools */
uint32_t uint32_finite_div(uint32_t n, uint32_t d, uint32_t p)
{
    if (d > 0) {
        uint32_t t = (n/d) << p; /* finite precision of 1/2^prec */
        return t + ((n << p) - d*t) / d;
    }
    return 0xffffffff;
}

#define MAX_AUTOSEARCH_ITERATIONS 10000
#ifdef CONFIG_STANDARD_ISDBT
uint16_t frontend_get_isdbt_sb_channels(struct dibFrontend *fe[], uint32_t freq , uint32_t bw, int num, struct dibChannel ch[])
{
    uint16_t seg_bw_khz = 429;
    int16_t rf_offset_khz = 0;
    uint16_t total_segment_number = (uint16_t)((bw / seg_bw_khz) - 1); // 1 is the freq guard
    int segment_index = 0, channel_index = 0, subch_id = -1;
    int i;

    dbgpl(NULL, "will search SB on %d potential segment(s) inside a bandwidth of %dkhz arround FR freq %dkhz", total_segment_number, bw, freq);

    for (i = 0; i < num; i++) {
        if (demod_init(fe[i]) != 0) {
            DibDbgPrint("-E-  Tuner init failed\n");
            return 0;
        }
        if (tuner_init(fe[i]) != 0) {
            DibDbgPrint("-E-  Tuner init failed\n");
            return 0;
        }
    }

    for (segment_index = 1; segment_index <= total_segment_number; segment_index++) {
        int success, fe_fail_count;

        channel_init(&ch[channel_index], STANDARD_ISDBT);

        /* compute segment center freq */
        rf_offset_khz = seg_bw_khz * (segment_index - (total_segment_number/2) - (total_segment_number%2));
        dbgpl(NULL,"rf_offset_khz = %d",rf_offset_khz);
        /* add a half of seg BW offset is total number of connected seg is even */
        if((total_segment_number%2) == 0) {
            dbgpl(NULL,"compensate for 1/2 seg (%d khz) tune freq because sb_conn_total_seg is even = %d",seg_bw_khz, total_segment_number);
            rf_offset_khz-=(seg_bw_khz/2);
        }
        dbgpl(NULL,"rf_offset_khz = %d",rf_offset_khz);

        ch[channel_index].RF_kHz = freq + rf_offset_khz;
        ch[channel_index].bandwidth_kHz = bw;
        ch[channel_index].u.isdbt.sb_mode = 1;

        if (subch_id != -1)
            subch_id += 3;

        ch[channel_index].u.isdbt.sb_subchannel = subch_id;

        dbgpl(NULL,"---------------------- Start search on segment #%d center freq = %dkhz (%d+%d, %d)--------------------- ",
                segment_index,ch[channel_index].RF_kHz, freq, rf_offset_khz, subch_id);

        for (i = 0; i < num; i++)
            frontend_tune_restart(fe[i], FE_RESTART_TUNE_PROCESS_FROM_TUNER, &ch[channel_index]);

        do {
            success = 0;
            fe_fail_count = 0;
            for (i = 0; i < num; i++) {
                frontend_tune(fe[i], &ch[channel_index]);

                if (fe[i]->status_has_changed && (frontend_get_status(fe[i]) == FE_STATUS_DEMOD_SUCCESS || frontend_get_status(fe[i]) == FE_STATUS_FFT_SUCCESS)) {
                    //dbgpl(&adapter_dbg, "Autosearch succeeded on FE %d", fe[i]->id);

                    frontend_get_channel(fe[i], &ch[channel_index]); /* we read the channel parameters from the frontend which was successful */

                    if (ch[channel_index].u.isdbt.partial_reception == 1) {
                        ch[channel_index].bandwidth_kHz = seg_bw_khz * 3; /* bandwidth is for 3 segments */
                        segment_index++; /* no need to check the next segment */
                    } else
                        ch[channel_index].bandwidth_kHz = seg_bw_khz;

#ifdef DIBCOM_EXTENDED_MONITORING
                        dump_digital_channel_params(&ch[channel_index]);
#endif
                }

                if (frontend_get_status(fe[i]) == FE_STATUS_LOCKED) {
                    subch_id = ch[channel_index].u.isdbt.sb_subchannel;
                    channel_index++;
                    success = 1;
                    break;
                } else if(frontend_get_status(fe[i]) == FE_STATUS_TUNE_FAILED) {
                    fe_fail_count++;

                }
            }
        } while (!success && fe_fail_count != num);

        if (success)
            DibDbgPrint("-I-  Autosearch succeeded for demod %d channel_index = %d - done.\n",i, channel_index);
        else
            DibDbgPrint("-I-  Autosearch failled for demod %d channel_index = %d - done. %d/%d\n",i, channel_index, fe_fail_count, num);
     }

    return channel_index;
}

#endif

int frontend_is_component_present(struct dibFrontend *fe, uint8_t component)
{
    return !!(fe->components & component);
}

int ABS(int x)
{
    return x < 0 ? -x : x;
}

