#include <adapter/frontend.h>
#include <tuner/dib0090.h>
#include "dib0090_priv.h"

void dib0090_write_reg(struct dib0090_state *st, uint32_t r, uint16_t v)
{
    data_bus_client_write16(tuner_get_data_bus_client(st->fe), r, v);
}

uint16_t dib0090_read_reg(struct dib0090_state *st, uint32_t reg)
{
    return data_bus_client_read16(tuner_get_data_bus_client(st->fe), reg);
}

#ifdef CONFIG_DIB0090_USE_PWM_AGC
void dib0090_write_regs(struct dib0090_state *state, uint8_t r, const uint16_t *b, uint8_t c)
{
    do {
        dib0090_write_reg(state, r++, *b++);
    } while (--c);
}
#endif
