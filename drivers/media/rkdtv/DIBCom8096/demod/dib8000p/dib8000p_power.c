#include "dib8000p_priv.h"
/*
P_int_force_pwdn_cfg      0             0     1     0     0     1     1280  15    15
unused                    0             0     1     0     0     1     1280  14    14 
unused                    0             0     1     0     0     1     1280  13    13 
unused                    0             0     1     0     0     1     1280  12    12 

P_force_pwdn_sdram        0             0     1     0     0     1     1280  11    11
P_force_pwdn_i2c          0             0     1     0     0     1     1280  10    10
P_force_pwdn_Dout         0             0     1     0     0     1     1280  9     9 
P_force_pwdn_gpio         0             0     1     0     0     1     1280  8     8 

P_force_pwdn_geninterf    0             0     1     0     0     1     1280  7     7 
P_int_restart_cfg         0             0     1     0     0     1     1280  6     6 
unused                    0             0     1     0     0     1     1280  5     5 
unused                    0             0     1     0     0     1     1280  4     4 

unused                    0             0     1     0     0     1     1280  3     3 
P_restart_sdram           0             0     1     0     0     1     1280  2     2 
P_restart_i2c             0             0     1     0     0     1     1280  1     1 
P_restart_Dout            0             0     1     0     0     1     1280  0     0 
*/

void dib8000p_set_power_mode(struct dib8000p_state *state, enum dib8000p_power_mode mode)
{
    /* by default everything is going to be powered off */
    uint16_t reg_774 = 0x3fff;
    uint16_t reg_775 = 0xffff;
    uint16_t reg_776 = 0xffff;
    uint16_t reg_900  = (dib8000p_read_word(state, 900)  & 0xfffc) | 0x3;
    uint16_t reg_1280 = (dib8000p_read_word(state, 1280) & 0x707f) | 0x8f80;

    dbgpl_advanced(&dib8000p_dbg, "powermode : 774 : 0x%04x ; 775 : 0x%04x; 776 : 0x%04x ; 900 : 0x%04x; 1280 : 0x%04x", reg_774, reg_775, reg_776, reg_900, reg_1280);
    /* now, depending on the requested mode, we power on */
    switch (mode) {
        case DIB8000P_POWER_ALL:
            reg_774 = 0x0000;
            reg_775 = 0x0000;
            reg_776 = 0x0000;
            reg_900 &= 0xfffc;
            reg_1280 &= 0x707f;
            break;
        case DIB8000P_POWER_INTERFACE_ONLY:
            reg_1280 &= 0xfa7b;
            break;
    }

    dbgpl_advanced(&dib8000p_dbg, "powermode : 774 : 0x%04x ; 775 : 0x%04x; 776 : 0x%04x ; 900 : 0x%04x; 1280 : 0x%04x", reg_774, reg_775, reg_776, reg_900, reg_1280);
    dib8000p_write_word(state,  774, reg_774);
    dib8000p_write_word(state,  775, reg_775);
    dib8000p_write_word(state,  776, reg_776);
    dib8000p_write_word(state,  900, reg_900);
    dib8000p_write_word(state, 1280, reg_1280);
}

int dib8000p_enable_vbg_voltage(struct dibFrontend *fe)
{
    struct dib8000p_state *state = fe->demod_priv;
    dbgpl(&dib8000p_dbg, "enabling VBG voltage in the ADC");
    /* P_dual_adc_cfg0 */
    dib8000p_write_word(state, 907, 0x0000);
    /* P_dual_adc_cfg1 = 3, P_sar_adc_cfg = 2 */
    dib8000p_write_word(state, 908, (3 << 2) | (2 << 0));

    return DIB_RETURN_SUCCESS;
}
