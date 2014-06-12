#include "dib7000p_priv.h"

//static int dib7000p_cfg_gpio(struct dib7000p_state *st, uint8_t num, uint8_t dir, uint8_t val)
int dib7000p_cfg_gpio(struct dib7000p_state *st, uint8_t num, uint8_t dir, uint8_t val)
{
	st->gpio_dir = dib7000p_read_word(st, 1029);
	st->gpio_dir &= ~(1 << num);         /* reset the direction bit */
	st->gpio_dir |=  (dir & 0x1) << num; /* set the new direction */
	dib7000p_write_word(st, 1029, st->gpio_dir);

	st->gpio_val = dib7000p_read_word(st, 1030);
	st->gpio_val &= ~(1 << num);          /* reset the direction bit */
	st->gpio_val |=  (val & 0x01) << num; /* set the new value */
	dib7000p_write_word(st, 1030, st->gpio_val);

	return DIB_RETURN_SUCCESS;
}

/* exported function for the user */
int dib7000p_set_gpio(struct dibFrontend *fe, uint8_t num, uint8_t dir, uint8_t val)
{
	struct dib7000p_state *state = fe->demod_priv;
	return dib7000p_cfg_gpio(state, num, dir, val);
}

uint8_t dib7000p_get_gpio(struct dibFrontend *fe, uint8_t num)
{
    struct dib7000p_state *state = fe->demod_priv;
    uint16_t val = dib7000p_read_word(state, 1038);

    return (val >> num)&0x1;
}

uint8_t dib7000p_get_gpio_dir(struct dibFrontend *fe, uint8_t num)
{
    struct dib7000p_state *state = fe->demod_priv;
    uint16_t dir = dib7000p_read_word(state, 1029);

    return (dir >> num)&0x1;
}

int dib7000p_reset_gpio(struct dib7000p_state *st)
{
        dbgpl(&dib7000p_dbg, "Reset GPIO");

	/* reset the GPIOs */
	dbgpl(&dib7000p_dbg, "gpio dir: %x: val: %x, pwm_pos: %x",st->gpio_dir, st->gpio_val,st->cfg.gpio_pwm_pos);

	dib7000p_write_word(st, 1029, st->gpio_dir);
	dib7000p_write_word(st, 1030, st->gpio_val);

	/* TODO 1031 is P_gpio_od */
	dib7000p_write_word(st, 1032, st->cfg.gpio_pwm_pos);

	dib7000p_write_word(st, 1037, st->cfg.pwm_freq_div);

	return DIB_RETURN_SUCCESS;
}
