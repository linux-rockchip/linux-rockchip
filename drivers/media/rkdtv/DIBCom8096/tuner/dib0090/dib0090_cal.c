#include <adapter/frontend.h>
#include <tuner/dib0090.h>
#include "dib0090_priv.h"

/********************DC Calib ***************************/
#define steps(u) (((u)>15)?((u)-16):(u))
#define INTERN_WAIT 10

//#define DEBUG_DC_CAL
//#define DC_CAL_OFFSET_READY //Do not use get_dc_power function because timing is not correct : define it after validation if needed

#ifdef CONFIG_TUNER_DIB0090_P1G_SUPPORT /* specific config for p1G */
static const struct dc_calibration dc_p1g_table[] =
{
    /* Step1 BB gain1= 26 with boost 1, gain 2 = 0 */
    /* addr ; trim reg offset ; pga ; CTRL_BB1 value ; i or q */
    { CTRL_BB_6,  5, 1, (1 << 13) | (0  << 8) | (15 << 3), 1 }, // offset_trim2_i_chann  0   0   5    0    0    1    6     9    5
    { CTRL_BB_7, 11, 1, (1 << 13) | (0  << 8) | (15 << 3), 0 }, // offset_trim2_q_chann  0   0   5    0    0    1    7     15   11
    /* Step 2 BB gain 1 = 26 with boost = 1 & gain 2 = 29 */
    { CTRL_BB_6,  0, 0, (1 << 13) | (29 << 8) | (15 << 3), 1 }, // offset_trim1_i_chann  0   0   5    0    0    1    6     4    0
    { CTRL_BB_6, 10, 0, (1 << 13) | (29 << 8) | (15 << 3), 0 }, // offset_trim1_q_chann  0   0   5    0    0    1    6     14   10
    { 0 },
};
#endif

#ifdef CONFIG_TUNER_DIB0090_P1C_F_SUPPORT
static const struct dc_calibration dc_table[] =
{
    /* Step1 BB gain1= 26 with boost 1, gain 2 = 0 */
    { CTRL_BB_6,  5, 1, (1 << 13) | (0  << 8) | (26 << 3), 1 }, // offset_trim2_i_chann  0   0   5    0    0    1    6     9    5
    { CTRL_BB_7, 11, 1, (1 << 13) | (0  << 8) | (26 << 3), 0 }, // offset_trim2_q_chann  0   0   5    0    0    1    7     15   11
    /* Step 2 BB gain 1 = 26 with boost = 1 & gain 2 = 29 */
    { CTRL_BB_6,  0, 0, (1 << 13) | (29 << 8) | (26 << 3), 1 }, // offset_trim1_i_chann  0   0   5    0    0    1    6     4    0
    { CTRL_BB_6, 10, 0, (1 << 13) | (29 << 8) | (26 << 3), 0 }, // offset_trim1_q_chann  0   0   5    0    0    1    6     14   10
    { 0 },
};
#endif

#ifdef CONFIG_TUNER_DIB0090_DEBUG
static void dib0090_print_WBDMUX_conf(struct dib0090_state *state)
{
#if 0 //def CONFIG_TUNER_DIB0090_DEBUG
    uint16_t wbd_mux = dib0090_read_reg(state, CTRL_WBDMUX);

    dbgpHal("CTRL_WBDMUX_adc_output_enable  = %d\n", (wbd_mux >>  0) & 0x1);
    dbgpHal("CTRL_WBDMUX_mux_output_enable  = %d\n", (wbd_mux >>  1) & 0x1);
    dbgpHal("CTRL_WBDMUX_nc                 = %d\n", (wbd_mux >>  2) & 0x1);

    switch((wbd_mux >>  3) & 0xf) {
        default :
        case 0x0 : dbgpHal("CTRL_WBDMUX_test_input         = NC\n");      break;
        case 0x1 : dbgpHal("CTRL_WBDMUX_test_input         = WBD_rfin \n"); break;
        case 0x2 : dbgpHal("CTRL_WBDMUX_test_input         = Mixer CMV\n"); break;
        case 0x3 : dbgpHal("CTRL_WBDMUX_test_input         = V2I Vdd  \n"); break;
        case 0x4 : dbgpHal("CTRL_WBDMUX_test_input         = V2I      \n"); break;
        case 0x5 : dbgpHal("CTRL_WBDMUX_test_input         = LNA      \n"); break;
        case 0x6 : dbgpHal("CTRL_WBDMUX_test_input         = FG       \n"); break;
        case 0x7 : dbgpHal("CTRL_WBDMUX_test_input         = BB       \n"); break;
        case 0xf : dbgpHal("CTRL_WBDMUX_test_input         = bias     \n"); break;
     }

    dbgpHal("CTRL_WBDMUX_wbd_log            = %d\n", (wbd_mux >>  7) & 0x1);
    dbgpHal("CTRL_WBDMUX_filter_short       = %d\n", (wbd_mux >>  8) & 0x1);
    dbgpHal("CTRL_WBDMUX_wbd_out            = %d\n", (wbd_mux >>  9) & 0x1);
    dbgpHal("CTRL_WBDMUX_wbd_enable         = %d\n", (wbd_mux >> 10) & 0x1);
    dbgpHal("CTRL_WBDMUX_wbd_bias           = %d\n", (wbd_mux >> 11) & 0x3);
    dbgpHal("CTRL_WBDMUX_wbd_gain           = %d\n", (wbd_mux >> 13) & 0x7);
#endif
}
#endif

static int dib0090_get_offset(struct dib0090_state *state, enum frontend_tune_state *tune_state)
{
    int ret = INTERN_WAIT * 10;

    switch (*tune_state) {
	  case CT_TUNER_STEP_2:
#ifdef DEBUG_DC_CAL
          dbgpHal("FE %d : Select WBDMUX_p as Slow ADC input\n", state->fe->id);
#endif
	      dib0090_write_reg(state, VGA_MODE, 0x7); /* Turns to positive */
	      *tune_state = CT_TUNER_STEP_3;
	      break;

	  case CT_TUNER_STEP_3:
	      state->adc_diff = dib0090_get_slow_adc_val(state);
#ifdef DEBUG_DC_CAL
          dbgpHal("FE %d : adc diff = %d, Now select WBDMUX_n as Slow ADC input\n", state->fe->id, state->adc_diff);
#endif
	      dib0090_write_reg(state, VGA_MODE, 0x4); /* Turns to negative */
	      *tune_state = CT_TUNER_STEP_4;
	      break;

	  case CT_TUNER_STEP_4:
	      state->adc_diff -= dib0090_get_slow_adc_val(state);
#ifdef DEBUG_DC_CAL
              dbgpl(&dib0090_dbg, "FE %d : adc diff = %d", state->fe->id, state->adc_diff);
#endif
	      *tune_state = CT_TUNER_STEP_5;
	      ret = 0;
	      break;
	default:
	    break;
    }
    // dbgpHal("Pos=%.3f;Neg=%.3f;Delta=%.3f\n", wbdp*1.8/1024, wbdn*1.8/1024,(wbdp-wbdn)*1.8/1024);
    return ret;
}

static void dib0090_set_trim(struct dib0090_state *state)
{
    uint16_t *val;

    if (state->dc->addr == CTRL_BB_7)
        val = &state->bb7;
    else
        val = &state->bb6;

    *val &= ~(0x1f << state->dc->offset);
    *val |= state->step << state->dc->offset;

#ifdef DEBUG_DC_CAL
    switch(state->dc->addr) {
        case CTRL_BB_6:
            if(state->dc->offset == 0)
                dbgpl(&dib0090_dbg,"Set trim1_I_chann = %d", state->step);
            else if(state->dc->offset == 5)
                dbgpl(&dib0090_dbg,"Set trim2_I_chann = %d", state->step);
            else if(state->dc->offset == 10)
                dbgpl(&dib0090_dbg,"Set trim1_Q_chann = %d", state->step);
            break;
        case CTRL_BB_7:
            if(state->dc->offset == 11)
                dbgpl(&dib0090_dbg,"Set trim2_Q_chann = %d", state->step);
            break;
    }
#endif

    dib0090_write_reg(state, state->dc->addr, *val);
}

int dib0090_captrim_search(struct dib0090_state *state, enum frontend_tune_state *tune_state)
{
    int ret = 0;
    uint16_t lo4 = 0xe900;

    int16_t adc_target;
    uint16_t adc;
    int8_t step_sign;
    uint8_t force_soft_search = 0;

    if(state->identity.version == SOC_8090_P1G_11R1 || state->identity.version == SOC_8090_P1G_21R1)
        force_soft_search = 1;

    if (*tune_state == CT_TUNER_START) {
        dbgpHal("Start Captrim search : %s\n", (force_soft_search == 1)?"FORCE SOFT SEARCH":"AUTO");
        dib0090_write_reg(state, CTRL_WBDMUX, 0x2B1);
        if(!state->identity.in_soc)
            dib0090_write_reg(state, ADCCLK, 0x0032);

#ifdef CONFIG_TUNER_DIB0090_DEBUG
        dib0090_print_WBDMUX_conf(state);
        dbgpl(&dib0090_dbg, "*** rf_request = %d current_rf = %d  state->tuner_is_tuned = %d***", state->rf_request, state->fe->current_rf, state->tuner_is_tuned);
#endif
        if (!state->tuner_is_tuned) {
            /* prepare a complete captrim */
            if(!state->identity.p1g || force_soft_search)
                state->step = state->captrim = state->fcaptrim = 64;

            state->fe->current_rf = state->rf_request;
        } else { /* we are already tuned to this frequency - the configuration is correct  */
            if(!state->identity.p1g || force_soft_search) {
                /* do a minimal captrim even if the frequency has not changed */
                state->step = 4;
                state->captrim = state->fcaptrim;
            }
        }
        state->adc_diff = 3000; // start with a unreachable high number : only set for KROSUS < P1G */
        *tune_state = CT_TUNER_STEP_0;

    } else if (*tune_state == CT_TUNER_STEP_0) {
#ifdef CONFIG_TUNER_DIB0090_P1G_SUPPORT
        if(state->identity.p1g && !force_soft_search) {
            // 30MHz => Code 15 for the ration => 128us to lock. Giving approximately
            uint8_t ratio = 31; // (state->config->io.clock_khz / 1024 + 1) & 0x1f;

#ifdef CONFIG_TUNER_DIB0090_DEBUG
            dbgpHal("***Auto Captrim search***\n");
            dbgpHal("***Code to the RATIO is %x***\n", ratio);
#endif

            dib0090_write_reg(state, CAPTRIM_CFG, (3<<7) | (ratio <<2) | (1<<1) | 1);
            dib0090_read_reg(state, CAPTRIM_CFG);
            //dib0090_write_reg(state, CAPTRIM_CFG, (3<<7) | ((((state->config->io.clock_khz >> 11)+1) & 0x1f)<<2) | (1<<1) | 1);
            ret = 50;
        } else
#endif
        {
            state->step /= 2;
            dib0090_write_reg(state, CTRL_LO_4, lo4 | state->captrim);

            if (state->identity.in_soc)
                ret = 25;
        }
        *tune_state = CT_TUNER_STEP_1;

    } else if (*tune_state == CT_TUNER_STEP_1) {
#ifdef CONFIG_TUNER_DIB0090_P1G_SUPPORT
        if(state->identity.p1g && !force_soft_search) {
            dib0090_write_reg(state, CAPTRIM_CFG, 0x18c | (0<<1) | 0);
            dib0090_read_reg(state, CAPTRIM_CFG);

#ifdef CONFIG_TUNER_DIB0090_DEBUG
            if (dib0090_read_reg(state, CALIBRATE) & (0x1<<3))
                dbgpHal("***Captrim Done\n");
            else
                dbgpHal("***Captrim is Not Finished\n");
#endif

            state->fcaptrim = dib0090_read_reg(state, CTRL_LO_4) & 0x7F;
            dbgpHal("***Final Captrim= 0x%x\n",state->fcaptrim);
            *tune_state = CT_TUNER_STEP_3;

        } else
#endif
        {
            /* MERGE for all krosus before P1G */
            adc = dib0090_get_slow_adc_val(state);
            dbgpHal("FE %d CAPTRIM=%d; ADC = %d (ADC) & %dmV\n", (uint32_t)state->fe->id, (uint32_t)state->captrim, (uint32_t)adc, (uint32_t) (adc)*(uint32_t)1800/(uint32_t)1024);

            if (state->rest == 0 || state->identity.in_soc)  {/* Just for 8090P SOCS where auto captrim HW bug : TO CHECK IN ACI for SOCS !!! if 400 for 8090p SOC => tune issue !!! */
                adc_target = 200;
            }
	    else
                adc_target = 400;

            if (adc >= adc_target) {
                adc -= adc_target;
                step_sign = -1;
            } else {
                adc = adc_target - adc;
                step_sign = 1;
            }

            if (adc < state->adc_diff) {
                dbgpHal("FE %d CAPTRIM=%d is closer to target (%d/%d)\n", (uint32_t)state->fe->id, (uint32_t)state->captrim, (uint32_t)adc, (uint32_t)state->adc_diff);
                state->adc_diff = adc;
                state->fcaptrim = state->captrim;
                //we could break here, to save time, if we reached a close-enough value
                //e.g.: if (state->adc_diff < 20)
                //break;
            }

            state->captrim += step_sign * state->step;
            if (state->step >= 1)
                *tune_state = CT_TUNER_STEP_0;
            else
                *tune_state = CT_TUNER_STEP_2;

            ret = 25; //LOLO changed from 15
        }
    } else if (*tune_state == CT_TUNER_STEP_2) {   /* this step is only used by krosus < P1G */
        /*write the final cptrim config*/
        dib0090_write_reg(state, CTRL_LO_4, lo4 | state->fcaptrim);

#ifdef CONFIG_TUNER_DIB0090_CAPTRIM_MEMORY
        state->memory[state->memory_index].cap=state->fcaptrim; //Store the cap value in the short memory
#endif
        *tune_state = CT_TUNER_STEP_3;

    } else if (*tune_state == CT_TUNER_STEP_3) {
        state->calibrate &= ~CAPTRIM_CAL;
        *tune_state = CT_TUNER_STEP_0;
    }

    return ret;
}

int dib0090_dc_offset_calibration(struct dib0090_state *state, enum frontend_tune_state *tune_state)
{
    int ret = 0;
#ifdef CONFIG_TUNER_DIB0090_P1G_SOCX090_SUPPORT
    uint16_t reg;
#endif

    switch (*tune_state) {
        case CT_TUNER_START:
        dbgpHal("Start DC offset calibration\n");

        /* force vcm2 = 0.8V */
        state->bb6 = 0;
        state->bb7 = 0x040d;

#ifdef CONFIG_TUNER_DIB0090_P1G_SOCX090_SUPPORT
        /* the LNA AND LO are off */
        reg = dib0090_read_reg(state, TUNER_EN) & 0x0ffb; /* shutdown lna and lo */
        dib0090_write_reg(state, TUNER_EN, reg);

         state->wbdmux = dib0090_read_reg(state, CTRL_WBDMUX);
         dib0090_write_reg(state, CTRL_WBDMUX, (state->wbdmux & ~(0xff << 3)) | (0x7 << 3) | 0x3); // connect BB, disable WDB enable*
#else
        /* the LNA AND LO are off */
        dib0090_write_reg(state, TUNER_EN, 0x02e9); /* MERGE : code from dib0090_p1g.c ; was 0x02ed in dib0090.c only LNA was off */
#endif
        if(!state->identity.in_soc)
            dib0090_write_reg(state, DIG_CFG, dib0090_read_reg(state, DIG_CFG) & ~(1<<14)); //Discard the DataTX

#ifdef CONFIG_TUNER_DIB0090_P1C_F_SUPPORT
        state->dc = dc_table;
#endif

#ifdef CONFIG_TUNER_DIB0090_P1G_SUPPORT
        if(state->identity.p1g)
            state->dc = dc_p1g_table;
#endif
        *tune_state = CT_TUNER_STEP_0;

        /* fall through */

    case CT_TUNER_STEP_0:
        dbgpHal("Sart/continue DC calibration for %s path\n", (state->dc->i == 1)?"I":"Q");
        dib0090_write_reg(state, CTRL_BB_1, state->dc->bb1);
        dib0090_write_reg(state, CTRL_BB_7, state->bb7 | (state->dc->i << 7));

        state->step = 0;
        state->min_adc_diff = 1023;
        *tune_state = CT_TUNER_STEP_1;
        ret = 50;
        break;

    case CT_TUNER_STEP_1:
        dib0090_set_trim(state);
#ifdef DC_CAL_OFFSET_READY
	if (state->config->get_dc_power != NULL)
            ret = 10;
#endif
        *tune_state = CT_TUNER_STEP_2;
        break;

    case CT_TUNER_STEP_2:
    case CT_TUNER_STEP_3:
    case CT_TUNER_STEP_4:
#if defined(CONFIG_TUNER_DIB0090_P1G_SUPPORT) && defined(DC_CAL_OFFSET_READY)
        if(state->identity.p1g && (state->config->get_dc_power != 0)) { //The DC power is not available
            //dbgpHal("********************************** Use Iqc_dcoff DC calibration\n");
            state->adc_diff = state->config->get_dc_power(state->fe, state->dc->i);
            ret = 50;
            *tune_state = CT_TUNER_STEP_5;
	} else
#endif
            ret = dib0090_get_offset(state, tune_state);
        break;

    case CT_TUNER_STEP_5: /* found an offset */
        dbgpHal("FE%d: adc_diff = %d, current step= %d\n", state->fe->id, (uint32_t) state->adc_diff, state->step);
	if (state->step == 0 && state->adc_diff < 0 ) {
            state->min_adc_diff = -1023;
            dbgpHal("Change of sign of the minimum adc diff\n");
	}

	dbgpHal("FE%d: adc_diff = %d, min_adc_diff = %d current_step = %d\n", state->fe->id, state->adc_diff,state->min_adc_diff, state->step);

        /* first turn for this frequency */
        if (state->step == 0) {
            if (state->dc->pga && state->adc_diff < 0)
                state->step = 0x10;
            if (state->dc->pga == 0 && state->adc_diff > 0)
                state->step = 0x10;
        }

	/* Look for a change of Sign in the Adc_diff.min_adc_diff is used to STORE the setp N-1 */
	if ((state->adc_diff & 0x8000) == (state->min_adc_diff & 0x8000) && steps(state->step) < 15 && state->adc_diff != 0) {
            /* stop search when the delta the sign is changing and Steps =15 and Step=0 is force for continuance */
            state->step++;
            state->min_adc_diff = state->adc_diff; //min is used as N-1
            *tune_state = CT_TUNER_STEP_1;
        } else {
            /* the minimum was what we have seen in the step before */
	    if (ABS(state->adc_diff) > ABS(state->min_adc_diff)) { //Come back to the previous state since the delta was better
                dbgpHal("Since adc_diff N = %d  > adc_diff step N-1 = %d, Come back one step\n", state->adc_diff,state->min_adc_diff);
                state->step--;
	    }

            dib0090_set_trim(state);
            dbgpHal("FE%d: BB Offset Cal, BBreg=%hd,Offset=%hd,Value Set=%hd\n", state->fe->id, state->dc->addr, state->adc_diff, state->step);

            state->dc++;
            if (state->dc->addr == 0) /* done */
                *tune_state = CT_TUNER_STEP_6;
            else
                *tune_state = CT_TUNER_STEP_0;

        }
        break;

    case CT_TUNER_STEP_6:
        dib0090_write_reg(state, CTRL_BB_7, state->bb7 & ~0x0008); //Force the test bus to be off
        dib0090_write_reg(state, VGA_MODE, 0x7);
        *tune_state = CT_TUNER_START; /* reset done -> real tuning can now begin */
        state->calibrate &= ~DC_CAL;
    default:
        break;
    }
    return ret;
}
/********************End DC Calib ***************************/
int dib0090_wbd_calibration(struct dib0090_state *state, enum frontend_tune_state *tune_state)
{
    uint8_t wbd_gain;
    const struct dib0090_wbd_slope *wbd = state->current_wbd_table;

    switch (*tune_state) {
        case CT_TUNER_START:
#ifdef CONFIG_TUNER_DIB0090_DEBUG
            dbgpHal("Start Wideband calibration\n");
#endif
            while (state->current_rf/1000 > wbd->max_freq)
                wbd++;
            if (wbd->wbd_gain!=0)
                wbd_gain = wbd->wbd_gain;
            else {
                wbd_gain = 4;
#if defined(CONFIG_BAND_LBAND) || defined(CONFIG_BAND_SBAND)
                if ((state->current_band == BAND_LBAND) || (state->current_band == BAND_SBAND))
                    wbd_gain = 2;
#endif
            }

            if (wbd_gain == state->wbd_calibration_gain) { /* the WBD calibration has already been done */
                *tune_state = CT_TUNER_START;
                state->calibrate &= ~WBD_CAL;
                return 0;
            }

            dib0090_write_reg(state, CTRL_WBDMUX, 0x1b81 | (1 << 10) | (wbd_gain << 13) | (1 << 3)); // Force: WBD enable,gain to 4, mux to WBD
#ifdef CONFIG_TUNER_DIB0090_DEBUG
            dib0090_print_WBDMUX_conf(state);
#endif

#ifdef CONFIG_TUNER_DIB0090_DEBUG
            dbgpHal("in WbdMux:%x with Gain %d for a frequency = %d\n", 0x1b81 | (1<<10) | (wbd_gain<<13) | (0<<3), wbd_gain, state->current_rf);
            /*
               dbgpHal("in v2i:%x\n", dib0090_read_reg(state, CTRL_RF_V2I));
               dbgpHal("in mix:%x\n", dib0090_read_reg(state, CTRL_RF_MIX));
               dbgpHal("in load:%x\n",dib0090_read_reg(state, CTRL_RF_LOAD));
             */
#endif

            dib0090_write_reg(state, TUNER_EN, ((EN_UHF & 0x0fff) | (1<<1))); //Discard all LNA but crystal !!!
            *tune_state = CT_TUNER_STEP_0;
            state->wbd_calibration_gain = wbd_gain;
            return 90; /* wait for the WBDMUX to switch and for the ADC to sample */

        case CT_TUNER_STEP_0:
            state->wbd_offset = dib0090_get_slow_adc_val(state);
            dbgpHal("WBD calibration offset = %d\n", state->wbd_offset);
            *tune_state = CT_TUNER_START; /* reset done -> real tuning can now begin */
            state->calibrate &= ~WBD_CAL;
            break;

        default:
            break;
    }
    return 0;
}

int dib0090_get_temperature(struct dib0090_state *state, enum frontend_tune_state *tune_state)
{
    int ret = 30;
    int16_t val;

    //The assumption is that the AGC is not active
    switch (*tune_state) {
        case CT_TUNER_START:
#ifdef CONFIG_TUNER_DIB0090_DEBUG
            dbgpHal("Start Temperature calibration\n");
#endif
            state->wbdmux = dib0090_read_reg(state, CTRL_WBDMUX);
            dib0090_write_reg(state, CTRL_WBDMUX, (state->wbdmux & ~(0xff << 3)) | (0x8 << 3)); //Move to the bias and clear the wbd enable
#ifdef CONFIG_TUNER_DIB0090_DEBUG
            dib0090_print_WBDMUX_conf(state);
#endif

            state->bias   = dib0090_read_reg(state, CTRL_BIAS);
            dib0090_write_reg(state, CTRL_BIAS, state->bias | (0x3 << 8)); //Move to the Ref

            *tune_state = CT_TUNER_STEP_0;
            /* wait for the WBDMUX to switch and for the ADC to sample */
            break;

        case CT_TUNER_STEP_0:
            state->adc_diff = dib0090_get_slow_adc_val(state); // Get the value for the Ref
            dib0090_write_reg(state, CTRL_BIAS, (state->bias & ~(0x3 << 8)) | (0x2 << 8)); //Move to the Ptat
            *tune_state = CT_TUNER_STEP_1;
            break;

        case CT_TUNER_STEP_1:
            val = dib0090_get_slow_adc_val(state);// Get the value for the Ptat
            dbgpHal("temperature: %d C\n", state->temperature - 30);
            //dbgpHal("PTAT=: %d C\n", val);
            state->temperature = ((int16_t)((val - state->adc_diff - state->ptat_ref_offset) * 180) >> 8) + 55 ; // +55 is defined as = -30deg
            //dbgpHal("temperature=: %d C\n", state->temperature - 30);
            //state->temperature = 55; //25deg
            dbgpHal("temperature: %d C\n", state->temperature - 30);

#ifdef CONFIG_TUNER_DIB0090_DEBUG
            dib0090_write_reg(state,CTRL_BIAS, (state->bias & ~(0x3 << 8)) | (0x1 << 8)); //Move to the BGap
#endif
            *tune_state = CT_TUNER_STEP_2;
            break;

        case CT_TUNER_STEP_2:
#ifdef CONFIG_TUNER_DIB0090_DEBUG
            val = dib0090_get_slow_adc_val(state);// Get the value for the Bgap
            dbgpHal("***************************BANDGAP== %d\n", val);
            state->vdd = ((uint32_t) (1250 << 10) / val) - 1800; //Offset from 1.8V
            dbgpHal("Vdd Delta to 1.8V value is %d\n", state->vdd);
#endif
            //Reload the start values.
            dib0090_write_reg(state, CTRL_BIAS, state->bias);
            dib0090_write_reg(state, CTRL_WBDMUX, state->wbdmux); /* write back original WBDMUX */

#ifdef CONFIG_TUNER_DIB0090_DEBUG
            dib0090_print_WBDMUX_conf(state);
#endif

            *tune_state = CT_TUNER_START;
            state->calibrate &= ~TEMP_CAL;
			if (state->config->analog_output==0) {
                if(!state->identity.in_soc)
                    dib0090_write_reg(state,DIG_CFG,dib0090_read_reg(state, DIG_CFG) | (1<<14)); //Set the DataTX
            }

            break;

        default:
            ret = 0;
            break;
    }
    return ret;
}

