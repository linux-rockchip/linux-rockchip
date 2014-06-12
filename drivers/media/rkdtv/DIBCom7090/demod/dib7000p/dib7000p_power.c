#include "dib7000p_priv.h"
int dib7000p_set_power_mode(struct dib7000p_state *state, enum dib7000p_power_mode mode)
{
	/* by default everything is powered off */
	uint16_t reg_774 = 0x3fff, reg_775 = 0xffff, reg_776 = 0x0007, reg_899  = 0x0003,
            reg_1280 = (0xfe00) | (dib7000p_read_word(state, 1280) & 0x01ff);

        if (state->version != SOC7090)
            reg_1280 = (0xfe00) | (dib7000p_read_word(state, 1280) & 0x01ff);
#ifdef CONFIG_DEMOD_DIB7000P_DEBUG
        else {
            dbgpl(&dib7000p_dbg, "Power setting specific to dib7000p for SOC 7090");
            //reg_1280 = (0xffe0) | (dib7000p_read_word(state, 1280) & 0x001f);
        }
#endif

	/* now, depending on the requested mode, we power on */
	switch (mode) {
		/* power up everything in the demod */
		case DIB7000P_POWER_ALL:
                        reg_774 = 0x0000; reg_775 = 0x0000; reg_776 = 0x0; reg_899 = 0x0;
                        if (state->version == SOC7090)
                            reg_1280 &= 0x001f;
                        else
                            reg_1280 &= 0x01ff;
			break;

		case DIB7000P_POWER_ANALOG_ADC:
			/* dem, cfg, iqc, sad, agc */
			reg_774 &= ~((1 << 15) | (1 << 14) | (1 << 11) | (1 << 10) | (1 << 9));
			/* nud */
			reg_776 &= ~((1 << 0));


			/* Dout */
                        if (state->version != SOC7090) /* NOT DIB 7090 SOC */
                            reg_1280 &= ~((1 << 11));

 			reg_1280 &= ~((1 << 6));
			/* fall through wanted to enable the interfaces */

		/* just leave power on the control-interfaces: GPIO and (I2C or SDIO) */
		case DIB7000P_POWER_INTERFACE_ONLY: /* TODO power up either SDIO or I2C */
                        if (state->version == SOC7090)
                            reg_1280 &= ~((1 << 7) | (1 << 5));
                        else
                            reg_1280 &= ~((1 << 14) | (1 << 13) | (1 << 12) | (1 << 10));
			break;

/* TODO following stuff is just converted from the dib7000-driver - check when is used what */
#if 0
		case DIB7000_POWER_LEVEL_INTERF_ANALOG_AGC:
			/* dem, cfg, iqc, sad, agc */
			reg_774  &= ~((1 << 15) | (1 << 14) | (1 << 11) | (1 << 10) | (1 << 9));
			/* sdio, i2c, gpio */
			reg_1280 &= ~((1 << 13) | (1 << 12) | (1 << 10));
			break;
		case DIB7000_POWER_LEVEL_DOWN_COR4_DINTLV_ICIRM_EQUAL_CFROD:
			reg_774   = 0;
			/* power down: cor4 dintlv equal */
			reg_775   = (1 << 15) | (1 << 6) | (1 << 5);
			reg_776   = 0;
			reg_899   = 0;
			reg_1280 &= 0x01ff;
			break;
		case DIB7000_POWER_LEVEL_DOWN_COR4_CRY_ESRAM_MOUT_NUD:
			reg_774   = 0;
			/* power down: cor4 */
			reg_775   = (1 << 15);
			/* nud */
			reg_776   = (1 <<  0);
			reg_899   = 0;
			reg_1280 &= 0x01ff;
			break;
#endif
	}

	dib7000p_write_word(state,  774,  reg_774);
	dib7000p_write_word(state,  775,  reg_775);
	dib7000p_write_word(state,  776,  reg_776);
	dib7000p_write_word(state, 1280, reg_1280);

        if (state->version != SOC7090)
            dib7000p_write_word(state,  899,  reg_899); /* no longer P_force_pwdn_a2r &  P_force_pwdn_bist in SOC7090 */

	return DIB_RETURN_SUCCESS;
}

int dib7000p_enable_vbg_voltage(struct dibFrontend *fe)
{
    struct dib7000p_state *state = fe->demod_priv;
    if (state->version != SOC7090) {
        dbgpl(&dib7000p_dbg, "enabling VBG voltage in the ADC");
        /* P_dual_adc_cfg0 */
        dib7000p_write_word(state, 908, (state->cfg.enable_current_mirror & 1) << 7);
        /* P_dual_adc_cfg1 = 3, P_sar_adc_cfg = 2 */
        dib7000p_write_word(state, 909, ((state->cfg.disable_sample_and_hold & 1) << 4) | (3 << 2) | (2 << 0));
    } else {
        dbgpl(&dib7000p_dbg, "WARNING : Can not enable VBG voltage in the ADC for SOC7090");
    }
    return 0;
}
