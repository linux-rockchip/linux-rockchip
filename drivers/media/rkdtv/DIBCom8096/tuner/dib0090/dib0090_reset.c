#include <adapter/frontend.h>
#include <tuner/dib0090.h>

#include "dib0090_priv.h"

struct dib0090_fw_state
{
    const struct dib0090_config *cfg;
    struct dibTunerInfo info;
    struct dib0090_identity identity;
};

static int dib0090_identify_common(struct dibFrontend *fe, struct dib0090_identity *identity)
{
    struct dibDataBusClient *client = tuner_get_data_bus_client(fe);
    uint16_t v = data_bus_client_read16(client, DIG_CFG_RO);

    identity->p1g = 0;
    identity->in_soc = 0;

    dbgpHal("FE %d : Tuner identification (Version = 0x%04x)\n", fe->id, v);

#ifdef FIRMWARE_FIREFLY
    /* pll is not locked locked */
    if (!(v & KROSUS_PLL_LOCKED))
        dbgpHal("Identification : pll is not yet locked\n");
#endif

    /* without PLL lock info */
    v &= ~KROSUS_PLL_LOCKED;
#ifdef CONFIG_TUNER_DIB0090_DEBUG
    dbgpHal("Version without PLL lock info = 0x%04x:\n", v);
#endif

    identity->version = v&0xff;
    identity->product = (v >> 8)&0xf;
#ifdef CONFIG_TUNER_DIB0090_DEBUG
    dbgpHal("Version read : 0x%04x (version_id = 0x%02x & product_id = 0x%01x)\n", v, identity->version, identity->product);
#endif

    if(identity->product != KROSUS)
        goto identification_error;

    //From the SOC the version definition has changed

    if ((identity->version & 0x3) == SOC) {
        identity->in_soc = 1;
        switch(identity->version) {
            case SOC_8090_P1G_11R1: dbgpHal("SOC 8090 P1-G11R1 Has been detected\n"); identity->p1g = 1; break;
            case SOC_8090_P1G_21R1: dbgpHal("SOC 8090 P1-G21R1 Has been detected\n"); identity->p1g = 1; break;
            case SOC_7090_P1G_11R1: dbgpHal("SOC 7090 P1-G11R1 Has been detected\n"); identity->p1g = 1; break;
            case SOC_7090_P1G_21R1: dbgpHal("SOC 7090 P1-G21R1 Has been detected\n"); identity->p1g = 1; break;
            default :
                goto identification_error;
        }
    } else {
        switch ((identity->version >> 5)&0x7) {
            case MP001: dbgpHal("MP001 : 9090/8096\n"); break;
            case MP005: dbgpHal("MP005 : Single Sband\n"); break;
            case MP008: dbgpHal("MP008 : diversity VHF-UHF-LBAND\n"); break;
            case MP009: dbgpHal("MP009 : diversity 29098 CBAND-UHF-LBAND-SBAND\n"); break;
            default :
                goto identification_error;
        }

        switch(identity->version & 0x1f) {
            case P1G_21R2: dbgpHal("P1G_21R2 detected\n"); identity->p1g = 1; break;
            case      P1G: dbgpHal("P1G detected\n");      identity->p1g = 1; break;
            case  P1D_E_F: dbgpHal("P1D/E/F detected\n");  break;
            case      P1C: dbgpHal("P1C detected\n");      break;
            case    P1A_B:
#ifdef CONFIG_TUNER_DIB0090_P1B_SUPPORT
                dbgpHal("P1-A/B detected: driver is deactivated - not available\n");
                dib0090_p1b_register(fe);
#else
                dbgpHal("P1-A/B detected: driver is deactivated - not available\n");
                goto identification_error;
#endif
                break;
            default :
                goto identification_error;
        }
    }
#ifdef CONFIG_TUNER_DIB0090_DEBUG
    dbgpHal("state->identity->version = 0x%04x\n", identity->version);
    dbgpHal("state->identity->product = 0x%04x\n", identity->product);
    dbgpHal("state->identity->p1g     = 0x%04x\n", identity->p1g);
    dbgpHal("state->identity->in_soc  = 0x%04x\n", identity->in_soc);
#endif

    return DIB_RETURN_SUCCESS;

identification_error:
    return DIB_RETURN_ERROR;
}

int dib0090_identify(struct dibFrontend *fe)
{
    struct dib0090_state *state = fe->tuner_priv;
    struct dib0090_identity *identity = &state->identity;

    return dib0090_identify_common(fe, identity);
}

int dib0090_identify_fw(struct dibFrontend *fe)
{
    struct dib0090_fw_state *state = fe->tuner_priv;
    struct dib0090_identity *identity = &state->identity;

    return dib0090_identify_common(fe, identity);
}

#define HARD_RESET(state) do {  if (cfg->reset) {  if (cfg->sleep) cfg->sleep(fe, 0); DibMSleep(10);  cfg->reset(fe, 1); DibMSleep(10);  cfg->reset(fe, 0); DibMSleep(10);  }  } while (0)

void dib0090_reset_digital(struct dibFrontend *fe, const struct dib0090_config *cfg)
{
    struct dibDataBusClient *client = tuner_get_data_bus_client(fe);
    uint16_t PllCfg;
    uint16_t v;
    int i;

	 HARD_RESET(state);


    data_bus_client_write16(client, TUNER_EN, EN_PLL | EN_CRYSTAL);

    if(!cfg->in_soc) {
        data_bus_client_write16(client, SLEEP_EN, EN_DIGCLK | EN_PLL | EN_CRYSTAL); /* PLL, DIG_CLK and CRYSTAL remain */
        /* adcClkOutRatio=8->7, release reset */
        data_bus_client_write16(client, DIG_CFG_3, ((cfg->io.adc_clock_ratio-1) << 11) | (0 << 10) | (1 << 9) | (1 << 8) | (cfg->data_tx_drv << 4) | cfg->ls_cfg_pad_drv);

        v = (0 << 15) | ((!cfg->analog_output) << 14) | (1 << 9) | (0 << 8) | (cfg->clkouttobamse << 4) | (0 << 2) | (0);
        if (cfg->clkoutdrive != 0)
            v |= cfg->clkoutdrive << 5;
        else
            v |= 7 << 5;

        /* if chip is in Nautilus 1.1-fix put drive to maximum */
#if 0
        if (cfg->fast_if_drv != 0)
            v |= cfg->fast_if_drv << 10;
        else
#endif
            v |= 2 << 10;
        data_bus_client_write16(client, DIG_CFG, v);

        /* Read Pll current config **/
        PllCfg = data_bus_client_read16(client, PLL_CFG);

        /** Reconfigure PLL if current setting is different from default setting **/
        if ((PllCfg & 0x1FFF) != ((cfg->io.pll_range << 12) | (cfg->io.pll_loopdiv << 6) | (cfg->io.pll_prediv)) && (!cfg->in_soc) && !cfg->io.pll_bypass) {

            /* Set Bypass mode */
            PllCfg |=  (1<<15);
            data_bus_client_write16(client, PLL_CFG, PllCfg);

            /* Set Reset Pll */
            PllCfg &=  ~(1<<13);
            data_bus_client_write16(client, PLL_CFG, PllCfg);

            /*** Set new Pll configuration in bypass and reset state ***/
            PllCfg =  (1 << 15) | (0 << 13) | (cfg->io.pll_range << 12) | (cfg->io.pll_loopdiv << 6) | (cfg->io.pll_prediv);
            data_bus_client_write16(client, PLL_CFG, PllCfg);

            /* Remove Reset Pll */
            PllCfg |=  (1<<13);
            data_bus_client_write16(client, PLL_CFG, PllCfg);

            /*** Wait for PLL lock ***/
            i = 100;
            do {
                v = !!(data_bus_client_read16(client, DIG_CFG_RO) & 0x800);
                if (v)
                    break;
            } while (--i);

            if (i == 0) {
                dbgpHal("Pll: Unable to lock Pll\n");
                return;
            }

            /* Finally Remove Bypass mode */
            PllCfg &=  ~(1<<15);
            data_bus_client_write16(client, PLL_CFG, PllCfg);
        }

        if(cfg->io.pll_bypass) {
            PllCfg |=  (cfg->io.pll_bypass << 15);
            data_bus_client_write16(client, PLL_CFG, PllCfg);
        }
    }
}

#ifdef BUILD_LEGACY_DIB_ADAPTER
static int dib0090_fw_reset(struct dibFrontend *fe)
{
   struct dib0090_fw_state *state = fe->tuner_priv;
   dib0090_reset_digital(fe, state->cfg);
   return dib0090_identify_fw(fe);
}

static int dib0090_fw_release(struct dibFrontend *tuner)
{
    struct dib0090_fw_state *st = tuner->tuner_priv;
    MemFree(st,sizeof(struct dib0090_fw_state));
    return DIB_RETURN_SUCCESS;
}

static const struct dibTunerInfo dib0090_fw_info = {
	INFO_TEXT("DiBcom DiB0090 (Krosus) Firmware"),   // name
	TUNER_CAN_VHF | TUNER_CAN_UHF | TUNER_CAN_LBAND | TUNER_CAN_SBAND, // caps

	{
        dib0090_fw_reset,   // reset
		NULL, // wakeup
		NULL, // sleep

		NULL, // tune_digital

		NULL,
		NULL,

        dib0090_fw_release, // release
	}
};

struct dibFrontend * dib0090_reset_register(struct dibFrontend *fe, struct dibDataBusHost *data, const struct dib0090_config *cfg)
{
	struct dib0090_fw_state *st = MemAlloc(sizeof(struct dib0090_fw_state));
	if (st == NULL)
		return NULL;
	DibZeroMemory(st,sizeof(struct dib0090_fw_state));

    frontend_register_tuner(fe, &st->info, &dib0090_fw_info, st);

    data_bus_client_init(tuner_get_data_bus_client(fe), &data_bus_client_template_8b_16b, data);

    st->cfg = cfg;

    return fe;
}
#endif
