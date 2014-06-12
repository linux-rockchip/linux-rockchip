#include <adapter/frontend.h>
#include <adapter/component_monitor.h>

#ifndef NO_DVBCHANNEL_BACKWARD_COMPAT

int demod_tune(struct dibFrontend *fe, struct dibDVBChannel *ch)
{
    fe->current_bw = BW_INDEX_TO_KHZ(ch->Bw);
	if (FE_DEMOD_FUNC_CHECK(fe, tune)) {
        struct dibChannel digitch;
        CONVERT_DVB_TO_DIGITAL_CHANNEL(ch, &digitch);
		return FE_DEMOD_FUNC_CALL(fe, tune)(fe, &digitch);
	} else
		return DIB_RETURN_ERROR;
}

int demod_agc_startup(struct dibFrontend* d, struct dibDVBChannel* ch)
{
    struct dibChannel digitch;
    CONVERT_DVB_TO_DIGITAL_CHANNEL(ch, &digitch);
    return demod_agc_startup_ex(d, &digitch);
}

int demod_autosearch_start(struct dibFrontend* d, struct dibDVBChannel *ch)
{
    struct dibChannel digitch;
	d->current_bw = BW_INDEX_TO_KHZ(ch->Bw);
    CONVERT_DVB_TO_DIGITAL_CHANNEL(ch, &digitch);
    return demod_autosearch_start_ex(d, &digitch);
}

int demod_get_channel_desc(struct dibFrontend* d, struct dibDVBChannel*ch)
{
    struct dibChannel digitch;
    int ret = demod_get_channel(d, &digitch);
    CONVERT_DIGITAL_TO_DVB_CHANNEL(&digitch, ch);
	return ret;
}

#endif

int demod_reset(struct dibFrontend *fe)
{
    if (FE_DEMOD_FUNC_CHECK(fe, reset))
        return FE_DEMOD_FUNC_CALL(fe, reset)(fe);
    else return DIB_RETURN_SUCCESS;
}

int demod_wakeup(struct dibFrontend *fe)
{
    if (FE_DEMOD_FUNC_CHECK(fe, wakeup))
        return FE_DEMOD_FUNC_CALL(fe, wakeup)(fe);
    else return DIB_RETURN_ERROR;
}

int demod_sleep(struct dibFrontend *fe)
{
    if (FE_DEMOD_FUNC_CHECK(fe, sleep))
        return FE_DEMOD_FUNC_CALL(fe, sleep)(fe, 0);
    return DIB_RETURN_ERROR;
}

int demod_deep_sleep(struct dibFrontend *fe)
{
    struct dibDataBusClient *client = demod_get_data_bus_client(fe);
    if (client)
        data_bus_host_reset(data_bus_client_get_data_bus(client));

    if (FE_DEMOD_FUNC_CHECK(fe, sleep))
        return FE_DEMOD_FUNC_CALL(fe, sleep)(fe, 1);
    return DIB_RETURN_SUCCESS;
}


#ifndef NO_SYNC_INTERFACE
int demod_set_bandwidth(struct dibFrontend *fe, uint32_t bw)
{
    return DIB_RETURN_SUCCESS;
}


int demod_tune_ex(struct dibFrontend *fe, struct dibChannel *ch)
{
    int ret;
    fe->current_bw = ch->bandwidth_kHz;
    if (FE_DEMOD_FUNC_CHECK(fe, tune)) {
        if (frontend_capabilities(fe) & FE_CAN_FAST_TUNE) {
            do {
                ret = FE_DEMOD_FUNC_CALL(fe, tune)(fe, ch);

                if (fe->status == FE_STATUS_LOCKED ||
                    fe->status == FE_STATUS_TUNE_FAILED)
                    break;
                DibMSleep(ret/10);
            } while (1);
            ret = DIB_RETURN_SUCCESS;
        }
        else {
            frontend_tune_restart(fe, FE_RESTART_TUNE_PROCESS_FROM_DEMOD, ch);
            fe->current_bw = ch->bandwidth_kHz;
            ret = FE_DEMOD_FUNC_CALL(fe, tune)(fe, ch);
        }
    } else
        ret = DIB_RETURN_SUCCESS;
    return ret;
}
#endif

int demod_tune_fast(struct dibFrontend *fe, struct dibChannel *ch)
{
    uint32_t delay;

    if (FE_DEMOD_FUNC_CHECK(fe, tune)) {
        if (frontend_capabilities(fe) & FE_CAN_FAST_TUNE) {
            if (component_ready(fe->demod_info->callback_time, fe->tune_state, CT_DEMOD_START)) {
                delay = FE_DEMOD_FUNC_CALL(fe, tune)(fe, ch);

                if (delay == FE_CALLBACK_TIME_NEVER)
                    fe->demod_info->callback_time = FE_CALLBACK_TIME_NEVER;
                else
                    fe->demod_info->callback_time = delay + systime();

                //dbgpl(NULL,"fe%d->demod_info->callback_time = %d  systime = %d ", fe->id, fe->demod_info->callback_time, systime());
            }
        }
#ifndef NO_SYNC_INTERFACE
        else { /* this demod does not support FAST_TUNE */

            if (component_ready(fe->demod_info->callback_time, fe->tune_state, CT_DEMOD_START)) {
                delay = 0;
                switch (fe->tune_state) {
                case CT_DEMOD_START:
                    if (channel_get_status(ch) == CHANNEL_STATUS_PARAMETERS_UNKNOWN) {
                        demod_autosearch_start_ex(fe, ch);
                        fe->tune_state = CT_DEMOD_STEP_1;
                    } else
                        fe->tune_state = CT_DEMOD_STEP_2;
                break;

                case CT_DEMOD_STEP_1: /* polling autosearch */
                    delay = 10;
                    switch (demod_autosearch_irq(fe)) {
                    case 0: /* pending */
                    break;
                    case 2:  /* success */
                        //frontend_get_channel(fe, ch);
                        fe->status = FE_STATUS_DEMOD_SUCCESS;
                        fe->tune_state = CT_DEMOD_STEP_2;
                    break;
                    case 1: /* failed */
                    default:
                        fe->status = FE_STATUS_TUNE_FAILED;
                        fe->tune_state = CT_DEMOD_STOP;
                        break;
					}
                break;

                case CT_DEMOD_STEP_2: /* tune */
                    FE_DEMOD_FUNC_CALL(fe, tune)(fe, ch);
                    fe->status = FE_STATUS_DATA_LOCKED;
                    fe->tune_state = CT_DEMOD_STOP;
                break;
                default:
                    delay = FE_CALLBACK_TIME_NEVER;
                break;
				}

                if (delay == FE_CALLBACK_TIME_NEVER)
                    fe->demod_info->callback_time = FE_CALLBACK_TIME_NEVER;
                else
                    fe->demod_info->callback_time = systime() + delay;
			}
		}
#endif
        return DIB_RETURN_SUCCESS;
	}
    return DIB_RETURN_NOT_SUPPORTED; /* failed */
}

int demod_agc_startup_fast(struct dibFrontend *fe, struct dibChannel *ch)
{
    if (FE_DEMOD_FUNC_CHECK(fe, agc_startup)) {
        if (component_ready(fe->demod_info->callback_time_agc, fe->tune_state, CT_AGC_START)) {
#ifdef CONFIG_BUILD_HOST /* on the HOST we only do AGC tracking after being tuned */
            if (fe->tune_state < CT_AGC_STOP || fe->tune_state >= CT_DONE)
#endif
            {
                uint32_t delay = FE_DEMOD_FUNC_CALL(fe, agc_startup)(fe, ch);
                if (delay == FE_CALLBACK_TIME_NEVER)
                    fe->demod_info->callback_time_agc = FE_CALLBACK_TIME_NEVER;
                else
                    fe->demod_info->callback_time_agc = systime() + delay;

            }
        }
        return DIB_RETURN_SUCCESS;
    }
    return DIB_RETURN_NOT_SUPPORTED; /* failed */
}

#ifndef NO_SYNC_INTERFACE
int demod_agc_startup_ex(struct dibFrontend *fe, struct dibChannel *ch)
{
    int ret=-1;
    if (FE_DEMOD_FUNC_CHECK(fe, agc_startup))
    {
        if (fe->tune_state < CT_AGC_STOP || fe->tune_state >= CT_DONE)
            ret = FE_DEMOD_FUNC_CALL(fe, agc_startup)(fe, ch);
        if (fe->tune_state >= CT_AGC_STOP)
            ret=-1;
    }
    return ret;
}

int demod_autosearch_start_ex(struct dibFrontend *fe, struct dibChannel *ch)
{
    if (frontend_capabilities(fe) & FE_CAN_FAST_TUNE) {
        fe->status = FE_STATUS_TUNE_PENDING;
        fe->tune_state = CT_DEMOD_START;
        fe->autosearch_channel_compat = ch;
        return DIB_RETURN_SUCCESS;
    } else {
        fe->current_bw = (uint16_t)ch->bandwidth_kHz;
        if (FE_DEMOD_FUNC_CHECK(fe, autosearch_start))
            return FE_DEMOD_FUNC_CALL(fe, autosearch_start)(fe, ch);
    }
    return DIB_RETURN_ERROR;
}

int demod_autosearch_irq(struct dibFrontend *fe)
{
    if (frontend_capabilities(fe) & FE_CAN_FAST_TUNE) {
#ifndef CONFIG_NO_TUNE_ASYNC
        if (component_ready(fe->demod_info->callback_time, fe->tune_state, CT_DEMOD_START)) {
            fe->demod_info->callback_time = systime() +
#endif
                FE_DEMOD_FUNC_CALL(fe, tune)(fe, fe->autosearch_channel_compat);
            if (fe->status <= FE_STATUS_FFT_SUCCESS)
                return 2;
            else if (fe->status == FE_STATUS_TUNE_PENDING)
                return 0;
            else
                return 1;
#ifndef CONFIG_NO_TUNE_ASYNC
        }
#endif
    } else
        if (FE_DEMOD_FUNC_CHECK(fe, autosearch_irq))
            return FE_DEMOD_FUNC_CALL(fe, autosearch_irq)(fe);
    return 1;
}
#endif

int demod_get_channel(struct dibFrontend *fe, struct dibChannel *ch)
{
    if (FE_DEMOD_FUNC_CHECK(fe, get_channel))
        return FE_DEMOD_FUNC_CALL(fe, get_channel)(fe, ch);
    return DIB_RETURN_ERROR;
}

int demod_set_diversity_in(struct dibFrontend *fe, int onoff)
{
    if (FE_DEMOD_FUNC_CHECK(fe, set_diversity_in))
        return FE_DEMOD_FUNC_CALL(fe, set_diversity_in)(fe, onoff);
    else return DIB_RETURN_ERROR;
}

int demod_set_output_mode(struct dibFrontend *fe, int mode)
{
    if (FE_DEMOD_FUNC_CHECK(fe, set_output_mode))
        return FE_DEMOD_FUNC_CALL(fe, set_output_mode)(fe, mode);
    else return DIB_RETURN_ERROR;
}

#ifndef NO_ADVANCED_FEATURES
int demod_pid_filter_ctrl(struct dibFrontend *fe, uint8_t onoff)
{
    if (FE_DEMOD_FUNC_CHECK(fe, pid_filter_ctrl))
        return FE_DEMOD_FUNC_CALL(fe, pid_filter_ctrl)(fe, onoff);
    else return DIB_RETURN_ERROR;
}

int demod_pid_filter(struct dibFrontend *fe, uint8_t id, uint16_t pid, uint8_t onoff)
{
    if (FE_DEMOD_FUNC_CHECK(fe, pid_filter))
        return FE_DEMOD_FUNC_CALL(fe, pid_filter)(fe, id, pid, onoff);
        else return DIB_RETURN_ERROR;
}

int demod_pid_filter_count(struct dibFrontend *fe)
{
    if ((fe != NULL) && (fe->demod_info != NULL))
        return fe->demod_info->pid_filter_count;
    else return 0;
}

int demod_set_wbd_ref(struct dibFrontend *fe, uint16_t value)
{
    if (FE_DEMOD_FUNC_CHECK(fe, set_wbd_ref))
        return FE_DEMOD_FUNC_CALL(fe, set_wbd_ref)(fe, value);
    else return DIB_RETURN_ERROR;
}

int demod_get_wbd_ref(struct dibFrontend *fe)
{
    if (FE_DEMOD_FUNC_CHECK(fe, get_wbd_ref))
        return FE_DEMOD_FUNC_CALL(fe, get_wbd_ref)(fe);
    return DIB_RETURN_SUCCESS;
}

int demod_ctrl_timing_frequency(struct dibFrontend *fe, uint8_t op, uint32_t timf)
{
    if (FE_DEMOD_FUNC_CHECK(fe, ctrl_timf))
        return FE_DEMOD_FUNC_CALL(fe, ctrl_timf)(fe, op, timf);
    else return DIB_RETURN_SUCCESS;
}

void demod_set_ber_rs_len(struct dibFrontend *fe, uint8_t ber_rs_len)
{
    if ((fe != NULL) && (fe->demod_info != NULL)) {
		if (frontend_generic_monitoring(fe, GENERIC_MONIT_SET_BER_RS_LEN, NULL, 0, ber_rs_len) != DIB_RETURN_NOT_SUPPORTED)
			fe->demod_info->ber_rs_len = ber_rs_len;
	}
}

#endif


int demod_get_signal_status(struct dibFrontend *fe, struct dibDVBSignalStatus *status)
{
#ifndef NO_ADVANCED_FEATURES
   if (FE_DEMOD_FUNC_CHECK(fe, get_signal_status))
      return FE_DEMOD_FUNC_CALL(fe, get_signal_status)(fe, status);
#endif
   return DIB_RETURN_ERROR;
}

struct dibDataBusClient * demod_get_data_bus_client(struct dibFrontend *fe)
{
    if ((fe != NULL) && (fe->demod_info != NULL))
        return &fe->demod_info->data_bus_client;
    else
        return NULL;
}

#ifndef NO_ADVANCED_FEATURES
struct dibDataBusClient * demod_get_register_access(struct dibFrontend *fe)
{
    return demod_get_data_bus_client(fe);
}

int demod_set_viterbi(struct dibFrontend *fe, int onoff)
{
	if (frontend_generic_monitoring(fe, GENERIC_MONIT_SET_VITERBI, NULL, 0, onoff) == DIB_RETURN_NOT_SUPPORTED)
		return DIB_RETURN_ERROR;
	else
		return DIB_RETURN_SUCCESS;
}

int demod_get_carrier_power(struct dibFrontend *fe, int carrier, double hchan[3], double noise[3])
{
    double t[6];
    int ret = frontend_generic_monitoring(fe, GENERIC_MONIT_COMPAT_SPECTRUM, (uint8_t *) t, sizeof(t), carrier);
    memcpy(hchan, &t[0], sizeof(double) * 3);
    memcpy(noise, &t[3], sizeof(double) * 3);
    return ret;
}

int demod_get_channel_profile(struct dibFrontend *fe, struct dibDemodChannelProfile *profile)
{
	if (frontend_generic_monitoring(fe, GENERIC_MONIT_COMPAT_PROFILE, (uint8_t *) profile, sizeof(*profile), 0) == DIB_RETURN_NOT_SUPPORTED)
		return DIB_RETURN_ERROR;
	else
		return DIB_RETURN_SUCCESS;
}

int demod_reset_packet_error_count(struct dibFrontend *fe)
{
	if ( frontend_generic_monitoring(fe, GENERIC_MONIT_RESET_PE_COUNT, NULL, 0, 0) == DIB_RETURN_NOT_SUPPORTED)
		return DIB_RETURN_ERROR;
	else
		return DIB_RETURN_SUCCESS;
}

void demod_set_ber_monitored_layer(struct dibFrontend* fe, int layer)
{
	frontend_generic_monitoring(fe, GENERIC_MONIT_SET_BER_LAYER, NULL, 0, layer);
}

int demod_get_ber_monitored_layer(struct dibFrontend *fe)
{
    return frontend_generic_monitoring(fe, GENERIC_MONIT_BER_LAYER, NULL, 0, 0);
}

int demod_get_monitoring(struct dibFrontend *fe, struct dibDemodMonitor *m)
{
    m->current_bandwidth = fe->current_bw;
	if (frontend_generic_monitoring(fe, GENERIC_MONIT_COMPAT_EXTENDED, (uint8_t *) m, sizeof(*m), 0) == DIB_RETURN_NOT_SUPPORTED)
		return DIB_RETURN_ERROR;
	else
		return DIB_RETURN_SUCCESS;
}

void demod_gpio_function(struct dibFrontend *fe, struct dibGPIOFunction *f)
{
	uint8_t i;
	if (f == NULL)
		return;

	for (i = 0; i < 32; i ++)
		if ((1 << i) & f->mask)
			demod_set_gpio(fe, i, (uint8_t)((f->direction >> i) & 0x1), (uint8_t)((f->value >> i) & 0x1));
}

#endif

int demod_set_gpio(struct dibFrontend *fe, uint8_t num, uint8_t dir, uint8_t val)
{
   if (FE_DEMOD_FUNC_CHECK(fe, set_gpio))
      return FE_DEMOD_FUNC_CALL(fe, set_gpio)(fe, num, dir, val);
   return DIB_RETURN_SUCCESS;
}

void demod_release(struct dibFrontend *fe)
{
   if (FE_DEMOD_FUNC_CHECK(fe, release))
      FE_DEMOD_FUNC_CALL(fe, release)(fe);
}

