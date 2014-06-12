#include "dib8000p_priv.h"

int dib8000p_cfg_gpio(struct dib8000p_state *st, uint8_t num, uint8_t dir, uint8_t val)
{
	st->cfg.gpio_dir = dib8000p_read_word(st, 1029);
	st->cfg.gpio_dir &= ~(1 << num);         /* reset the direction bit */
	st->cfg.gpio_dir |=  (dir & 0x1) << num; /* set the new direction */
	dib8000p_write_word(st, 1029, st->cfg.gpio_dir);

	st->cfg.gpio_val = dib8000p_read_word(st, 1030);
	st->cfg.gpio_val &= ~(1 << num);          /* reset the direction bit */
	st->cfg.gpio_val |=  (val & 0x01) << num; /* set the new value */
	dib8000p_write_word(st, 1030, st->cfg.gpio_val);

	dbgpl(&dib8000p_dbg, "gpio dir: %x: gpio val: %x",
		st->cfg.gpio_dir, st->cfg.gpio_val);

	return DIB_RETURN_SUCCESS;
}

int dib8000p_reset_gpio(struct dib8000p_state *st)
{
	/* reset the GPIOs */
	dib8000p_write_word(st, 1029, st->cfg.gpio_dir);
	dib8000p_write_word(st, 1030, st->cfg.gpio_val);

	/* TODO 782 is P_gpio_od */

	dib8000p_write_word(st, 1032, st->cfg.gpio_pwm_pos);

	dib8000p_write_word(st, 1037, st->cfg.pwm_freq_div);
	return DIB_RETURN_SUCCESS;
}

int dib8000p_set_pwm_gpio(struct dibFrontend *fe, uint8_t num, uint8_t id, uint8_t freq, uint32_t rate)
{
    struct dib8000p_state *state = fe->demod_priv;
    uint16_t value;
    uint8_t bit = (3 - id)*4;

    if (id > 4)
        return DIB_RETURN_ERROR;

    dib8000p_cfg_gpio(state, num, 0, 0);

    value  = dib8000p_read_word(state, 1032);
    value &= ~(0xf << bit);
    value |=  (num << bit);
    dib8000p_write_word(state, 1032, value);
    dib8000p_write_word(state, 1033 + id, (uint16_t)rate);
    dib8000p_write_word(state, 1037, freq);

    return DIB_RETURN_SUCCESS;
}

