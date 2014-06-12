#include <adapter/frontend.h>
int tuner_reset(struct dibFrontend *fe)
{
    if (FE_TUNER_FUNC_CHECK(fe, reset))
        return FE_TUNER_FUNC_CALL(fe, reset)(fe);
    else return DIB_RETURN_SUCCESS;
}

int tuner_sleep(struct dibFrontend *fe)
{
    if (FE_TUNER_FUNC_CHECK(fe, sleep))
        return FE_TUNER_FUNC_CALL(fe, sleep)(fe);
    else return DIB_RETURN_SUCCESS;
}

#ifdef CONFIG_STANDARD_ANALOG
int tuner_tune_analog(struct dibFrontend *fe, struct dibAnalogChannel *ch)
{
    if (FE_TUNER_FUNC_CHECK(fe, tune_analog))
        return FE_TUNER_FUNC_CALL(fe, tune_analog)(fe,ch);
    else
        return DIB_RETURN_SUCCESS;
}
#endif

void tuner_release(struct dibFrontend *fe)
{
    if (FE_TUNER_FUNC_CHECK(fe, release))
        fe->tuner_info->ops.release(fe);
}

struct dibDataBusClient * tuner_get_data_bus_client(struct dibFrontend *fe)
{
    if ((fe != NULL) && (fe->tuner_info != NULL))
        return &fe->tuner_info->data_bus_client;
    else
        return NULL;
}

int tuner_wakeup(struct dibFrontend *fe)
{
    if (FE_TUNER_FUNC_CHECK(fe, wakeup))
        return FE_TUNER_FUNC_CALL(fe, wakeup)(fe);

    return DIB_RETURN_SUCCESS;
}

#ifndef NO_SHARED_TUNER
int tuner_try_inc_use_count(struct dibFrontend *tuner, int count)
{
	if ((tuner->tuner_info->use_count + count) > TUNER_MAX_USE_COUNT)
		return DIB_RETURN_ERROR;
	return DIB_RETURN_SUCCESS;
}

int tuner_inc_use_count(struct dibFrontend *tuner, int count)
{
	if (tuner_try_inc_use_count(tuner,count) != DIB_RETURN_SUCCESS)
		return DIB_RETURN_ERROR;

	tuner->tuner_info->use_count += count;
	return DIB_RETURN_SUCCESS;
}

int tuner_dec_use_count(struct dibFrontend *tuner, int count)
{
	tuner->tuner_info->use_count -= count;
	if (tuner->tuner_info->use_count < 0) {
        tuner->tuner_info->use_count = 0; //To be modified ASAP
        DibDbgPrint("-E-  tuner use_count is below zero - something is wrong\n");
		return DIB_RETURN_ERROR;
	}
	return DIB_RETURN_SUCCESS;
}

int tuner_use_count(struct dibFrontend *fe)
{
    if ((fe != NULL) && (fe->tuner_info != NULL))
        return fe->tuner_info->use_count;
    else
        return 4;
}
#endif

#ifndef NO_DVBCHANNEL_BACKWARD_COMPAT
int tuner_tune_digital(struct dibFrontend* tuner,struct dibDVBChannel* ch)
{
    struct dibChannel digitch;
    CONVERT_DVB_TO_DIGITAL_CHANNEL(ch, &digitch);
    return tuner_tune(tuner, &digitch);
}
#endif

int tuner_tune_fast(struct dibFrontend *fe, struct dibChannel *ch)
{
    if (fe->tuner_info) {
        if (fe->tuner_info->ops.tune_digital) {
            if (fe->tuner_info->caps & TUNER_CAN_FAST_TUNE) {
                if (component_ready(fe->tuner_info->callback_time, fe->tune_state, CT_TUNER_START)) {

                    uint32_t callback_time = fe->tuner_info->ops.tune_digital(fe, ch);

                    if (callback_time == FE_CALLBACK_TIME_NEVER)
                        fe->tuner_info->callback_time = FE_CALLBACK_TIME_NEVER;
                    else
                        fe->tuner_info->callback_time  = callback_time + systime();
                    //dbgpl(NULL,"fe%d->tuner_info->callback_time = %d  systime = %d ", fe->id, fe->tuner_info->callback_time, syst);
                }
                return DIB_RETURN_SUCCESS;
            }
#ifndef NO_SYNC_INTERFACE
            else {
                if (fe->tune_state == CT_TUNER_START) {

                    fe->tuner_info->callback_time = FE_CALLBACK_TIME_NEVER;
                    fe->tune_state = CT_TUNER_STOP;

                    return fe->tuner_info->ops.tune_digital(fe, ch);
                } else
                    return DIB_RETURN_SUCCESS;
            }
#endif
        } else
            fe->tuner_info->callback_time = FE_CALLBACK_TIME_NEVER;
    }
    return DIB_RETURN_NOT_SUPPORTED;
}

/* backward compat */
int tuner_tune(struct dibFrontend *fe, struct dibChannel *ch)
{
    if (fe->tuner_info && fe->tuner_info->ops.tune_digital) {
        if (fe->tuner_info->caps & TUNER_CAN_FAST_TUNE) {
            int ret;
            fe->tune_state = CT_TUNER_START;
            do {
                ret = fe->tuner_info->ops.tune_digital(fe, ch);
                DibUSleep(ret * 100);
            } while (fe->tune_state != CT_TUNER_STOP);
            return DIB_RETURN_SUCCESS;
        } else
            return fe->tuner_info->ops.tune_digital(fe, ch);
    }
    return DIB_RETURN_SUCCESS;
}

#ifndef NO_ADVANCED_FEATURES
int tuner_get_frequency(struct dibFrontend *fe)
{
    uint8_t f[4];
    if (frontend_generic_monitoring(fe, GENERIC_MONIT_RADIO_FREQUENCY, f, 4, 0) == 4)
        return f[0] << 24 | f[1] << 16 | f[2] << 8 | f[3];
    return 0;
}
#endif
