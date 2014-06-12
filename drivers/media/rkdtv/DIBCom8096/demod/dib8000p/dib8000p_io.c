#include "dib8000p_priv.h"

extern int dib8000p_set_diversity_in(struct dibFrontend *fe, int onoff)
{
    struct dib8000p_state *state = fe->demod_priv;
    uint16_t tmp = dib8000p_read_word(state, 273) & 0xfff0;

    if (!state->differential_constellation) {
        dib8000p_write_word(state, 272, 1 << 9); //dvsy_off_lmod4 = 1
        dib8000p_write_word(state, 273, tmp | (1 << 2) | 2); // sync_enable = 1; comb_mode = 2
    } else {
        dib8000p_write_word(state, 272, 0); //dvsy_off_lmod4 = 0
        dib8000p_write_word(state, 273, tmp); // sync_enable = 0; comb_mode = 0
    }
    state->diversity_onoff = onoff;

    switch (onoff) {
    case 0: /* only use the internal way - not the diversity input */
        dib8000p_write_word(state, 270, 1);
        dib8000p_write_word(state, 271, 0);
        break;
    case 1: /* both ways */
        dib8000p_write_word(state, 270, 6);
        dib8000p_write_word(state, 271, 6);
        break;
    case 2: /* only the diversity input */
        dib8000p_write_word(state, 270, 0);
        dib8000p_write_word(state, 271, 1);
        break;
    }

    dbgpl(&dib8000p_dbg,"fe %d state->revision = 0x%04x",fe->id, state->revision);
    if (state->revision == 0x8002) {
        tmp = dib8000p_read_word(state, 903);
        dib8000p_write_word(state, 903, tmp & ~(1 << 3));
        DibMSleep(30);
        dib8000p_write_word(state, 903, tmp | (1 << 3));
    }

    return DIB_RETURN_SUCCESS;
}

static void dib8000p_set_acquisition_mode(struct dib8000p_state *state)
{
    uint16_t nud = dib8000p_read_word(state, 298);
    dbgpl(&dib8000p_dbg, "acquisition mode activated");
    if (state->revision >= 0x8002)
        nud |= (1<<5) | (1<<2);
    else
        nud |= (1<<3) | (1<<0);
    dib8000p_write_word(state, 298, nud);
}

int dib8000p_set_output_mode(struct dib8000p_state *state, int mode)
{
    uint16_t outreg, fifo_threshold, smo_mode,
             sram = 0x0205; /* by default SDRAM deintlv is enabled */

    outreg = 0;
    fifo_threshold = 1792;
    smo_mode = (dib8000p_read_word(state, 299) & 0x0050) | (1 << 1);

    dbgpl(&dib8000p_dbg, "-I-  Setting output mode for demod %p to %d", state->fe, mode);
    state->output_mode = mode;

    switch (mode) {
    case OUTPUT_MODE_TS_PARALLEL_GATED:   // STBs with parallel gated clock
        outreg = (1 << 10);  /* 0x0400 */
        break;
    case OUTPUT_MODE_TS_PARALLEL_CONT:    // STBs with parallel continues clock
        outreg = (1 << 10) | (1 << 6); /* 0x0440 */
        break;
    case OUTPUT_MODE_TS_SERIAL:          // STBs with serial input
        outreg = (1 << 10) | (2 << 6) | (0 << 1); /* 0x0482 */
        break;
    case OUTPUT_MODE_DIVERSITY:
        if (state->cfg.hostbus_diversity) {
            outreg = (1 << 10) | (4 << 6); /* 0x0500 */
            sram &= 0xfdff;
        } else
            sram |= 0x0c00;
        break;
    case OUTPUT_MODE_TS_FIFO:            // e.g. USB feeding
        smo_mode |= (3 << 1);
        fifo_threshold = 512;
        outreg = (1 << 10) | (5 << 6);
        break;
    case OUTPUT_MODE_OFF:  // disable
        outreg = 0;
        break;

    case OUTPUT_MODE_ANALOG_ADC:
        outreg = (1 << 10) | (3 << 6);
        dib8000p_set_acquisition_mode(state);
        break;

    default:
        dbgpl(&dib8000p_dbg, "Unhandled output_mode passed to be set for demod %p", state->fe);
        return DIB_RETURN_ERROR;
    }

    if (state->cfg.output_mpeg2_in_188_bytes)
        smo_mode |= (1 << 5) ;

    dib8000p_write_word(state,  299, smo_mode);
    dib8000p_write_word(state,  300, fifo_threshold); /* synchronous fread */
    dib8000p_write_word(state, 1286, outreg);
    dib8000p_write_word(state, 1291, sram);

    return DIB_RETURN_SUCCESS;
}

int dib8000p_pid_filter_ctrl(struct dibFrontend *fe, uint8_t onoff)
{
    struct dib8000p_state *state = fe->demod_priv;
    uint16_t val = dib8000p_read_word(state, 299) & 0xffef;
    val |= (onoff & 0x1) << 4;

    dbgpl(&dib8000p_dbg, "pid filter enabled %d", onoff);
    return dib8000p_write_word(state, 299, val);
}

int dib8000p_pid_filter(struct dibFrontend *fe, uint8_t id, uint16_t pid, uint8_t onoff)
{
    struct dib8000p_state *state = fe->demod_priv;
    dbgpl(&dib8000p_dbg, "Index %x, PID %d, OnOff %d", id, pid, onoff);
    return dib8000p_write_word(state, 305 + id, onoff ? (1 << 13) | pid : 0);
}
