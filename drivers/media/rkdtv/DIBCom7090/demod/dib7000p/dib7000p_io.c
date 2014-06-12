#include "dib7000p_priv.h"

extern int dib7000p_set_diversity_in(struct dibFrontend *fe, int onoff)
{
    struct dib7000p_state *state = fe->demod_priv;
    
    /* P_dvsy_sync_mode = 0, P_dvsy_sync_enable=1, P_dvcb_comb_mode=2 */
    if (state->div_force_off) {
        dbgpl(&dib7000p_dbg, "diversity combination deactivated - forced by COFDM parameters");
        onoff = 0;
        dib7000p_write_word(state, 207, 0);
    } else
        dib7000p_write_word(state, 207, (state->div_sync_wait << 4) | (1 << 2) | (2 << 0));
    state->div_state = (uint8_t)onoff;

    switch (onoff) {
        case 0: /* only use the internal way - not the diversity input */
            dib7000p_write_word(state, 204, 1);
            dib7000p_write_word(state, 205, 0);
            break;
        case 1: /* both ways */
            dib7000p_write_word(state, 204, 6);
            dib7000p_write_word(state, 205, 16);
            break;
        case 2: /* only the diversity input */
            dib7000p_write_word(state, 204, 0);
            dib7000p_write_word(state, 205, 1);
            break;
    }

    return DIB_RETURN_SUCCESS;
}

int dib7000p_set_acquisition_mode(struct dibFrontend *fe, uint8_t agc_disable)
{
    struct dib7000p_state *state = fe->demod_priv;

    if(state->version != SOC7090) { /* only valid for dib7000p not for SOC7090 */
        state->adc_mode = 1 + agc_disable; // 1 = on+agc, 2 = on w/o agc

        dib7000p_set_power_mode(state, DIB7000P_POWER_ANALOG_ADC);

        //Set AGC inactive (in case another device (dib5000) controls AGC tuner : PWM 1 ACG pad in tri state mode)
        if (agc_disable) {
            uint16_t reg_905 = dib7000p_read_word(state, 905) | (1<<15);

            dib7000p_write_word(state, 107, 0xffff);
            dib7000p_write_word(state, 108, 0xffff);
            dib7000p_write_word(state, 109, 0xffff);
            dib7000p_write_word(state, 110, 0xffff);
            dib7000p_write_word(state, 905, reg_905);
        }

        dib7000p_set_output_mode(state, OUTPUT_MODE_ANALOG_ADC);
        dib7000p_set_adc_state(state, DIBX000_SLOW_ADC_ON);
        dib7000p_set_adc_state(state, DIBX000_ADC_ON);
    }
    return DIB_RETURN_SUCCESS;
}

int dib7000p_set_output_mode(struct dib7000p_state *state, int mode)
{
	int    ret = 0;
	uint16_t outreg, fifo_threshold, smo_mode;

	outreg = 0;
	fifo_threshold = 1792;
	smo_mode = (dib7000p_read_word(state, 235) & 0x0050) | (1 << 1);

	dbgpl(&dib7000p_dbg, "setting output mode for demod %p to %d", state->fe, mode);

	switch (mode) {
		case OUTPUT_MODE_TS_PARALLEL_GATED:   // STBs with parallel gated clock
			outreg = (1 << 10);  /* 0x0400 */
			break;
		case OUTPUT_MODE_TS_PARALLEL_CONT:    // STBs with parallel continues clock
			outreg = (1 << 10) | (1 << 6); /* 0x0440 */
			break;
		case OUTPUT_MODE_TS_SERIAL:          // STBs with serial input
			outreg = (1 << 10) | (2 << 6) | (0 << 1); /* 0x0480 */
			break;
		case OUTPUT_MODE_DIVERSITY:
			if (state->cfg.hostbus_diversity)
				outreg = (1 << 10) | (4 << 6); /* 0x0500 */
			else
				outreg = (1 << 11);
			break;
		case OUTPUT_MODE_TS_FIFO:            // e.g. USB feeding
			smo_mode |= (3 << 1);
			fifo_threshold = 512;
			outreg = (1 << 10) | (5 << 6);
			break;
		case OUTPUT_MODE_ANALOG_ADC:
			outreg = (1 << 10) | (3 << 6);
			break;
		case OUTPUT_MODE_OFF:  // disable
			outreg = 0;
			break;
		default:
			dbgpl(&dib7000p_dbg, "Unhandled output_mode passed to be set for demod %p",state->fe);
			return DIB_RETURN_ERROR;
	}

	if (state->cfg.output_mpeg2_in_188_bytes)
		smo_mode |= (1 << 5) ;

	ret |= dib7000p_write_word(state,  235, smo_mode);
	ret |= dib7000p_write_word(state,  236, fifo_threshold); /* synchronous fread */

        if(state->version != SOC7090)
            ret |= dib7000p_write_word(state, 1286, outreg);         /* P_Div_active */

	return ret;
}

int dib7000p_pid_filter_ctrl(struct dibFrontend *fe, uint8_t onoff)
{
	struct dib7000p_state *state = fe->demod_priv;
	uint16_t val = dib7000p_read_word(state, 235) & 0xffef;
	val |= (onoff & 0x1) << 4;
	dbgpl(&dib7000p_dbg, "PID filter enabled %d", onoff);
	return dib7000p_write_word(state, 235, val);
}

int dib7000p_pid_filter(struct dibFrontend *fe, uint8_t id, uint16_t pid, uint8_t onoff)
{
	struct dib7000p_state *state = fe->demod_priv;
	dbgpl(&dib7000p_dbg, "PID filter: index %x, PID %d, OnOff %d", id, pid, onoff);
	return dib7000p_write_word(state, 241 + id, onoff ? (1 << 13) | pid : 0);
}

