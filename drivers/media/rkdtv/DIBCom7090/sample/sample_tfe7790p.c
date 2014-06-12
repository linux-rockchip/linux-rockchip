/* basic test application for showing the use of the demod/tuner-driver for the TFE7790P based on a single DiB7790P */
#include "sample_dib0700.h"

#include <adapter/frontend_tune.h>
#include <adapter/sip.h>
#include <demod/dib7000p.h>
#include <sip/dibx090p.h>
#include <tuner/dib0090.h>
#include "monitor/monitor.h"

static int dib7790p_update_lna(struct dibFrontend *fe, uint16_t agc_global);

static const struct dibx090p_config tfe7790p_layout_config = {
    1,
    dib7790p_update_lna, // update_lna

    DIBX090P_GPIO_DEFAULT_DIRECTIONS,
    DIBX090P_GPIO_DEFAULT_VALUES,
    DIBX090P_GPIO_DEFAULT_PWM_POS,

    0,     // dib0090_freq_offset_khz_uhf
    0,     // dib0090_freq_offset_khz_vhf

    12000, // clock_khz
    0,      // diversity delay
    NULL,   // dib0090_wbd_table
    0,
    4,
    0,
    0,
    0,
    1,      // crystal mode
    0x2d98, // dib8k_drives
    0,
    1, //clkouttobamse
    0,
    0,
    0,
};

static int dib7790p_update_lna(struct dibFrontend *fe, uint16_t agc_global)
{
    uint16_t agc1, agc2,wbd, wbd_target,wbd_offset,threshold_agc1;
    int16_t  wbd_delta;

    if ((fe->current_rf)<400000)
        threshold_agc1 = 25000;
    else
        threshold_agc1 = 30000;

    wbd_target = (dib0090_get_wbd_target(fe, fe->current_rf)*8+1)/2;
    wbd_offset = dib0090_get_wbd_offset(fe);
    dib7000p_get_agc_values(fe, NULL, &agc1, &agc2, &wbd);
    wbd_delta =(int16_t)wbd - (((int16_t)wbd_offset+10)*4) ;

    dbgpl(NULL, "update lna, agc_global=%d agc1=%d agc2=%d\n", agc_global, agc1, agc2);
    dbgpl(NULL, "update lna, wbd=%d wbd target=%d wbd offset=%d wbd delta=%d\n", wbd, wbd_target, wbd_offset, wbd_delta);

    if ((agc1 < threshold_agc1) && (wbd_delta > 0)) {
        //dbgpl(NULL, "agc global low (%d) and wbd high (%d) ==> set the switch to 0 \n", agc_global, wbd);
        dib0090_set_switch(fe, 1, 1, 1);
        dib0090_set_vga(fe, 0);
        dib0090_update_rframp_7090(fe, 0);
        dib0090_update_tuning_table_7090(fe, 0);
    } else {
        //dbgpl(NULL, "wbd low (%d) ==> set the vga to 1\n ", wbd);
        dib0090_set_vga(fe, 1);
        dib0090_update_rframp_7090(fe, 1);
        dib0090_update_tuning_table_7090(fe, 1);
        dib0090_set_switch(fe, 0, 0, 0);
    }

    if ((tfe7790p_layout_config.use_high_level_adjustment & 1) && (agc_global < 16000))
        demod_set_gpio(fe, 8, 0, 0);

    return 0;
}

int main (void)
{
	struct dibDataBusHost *i2c = open_dib0700_i2c(1, 1);
	struct dibFrontend fe;
	struct dibChannel ch;
	struct dibDemodMonitor mon;

	DibZeroMemory(&mon, sizeof(mon));

	if (i2c == NULL)
		return 1;

	frontend_init(&fe); /* initializing the frontend-structure */
	frontend_set_id(&fe, 0); /* assign an absolute ID to the frontend */
	frontend_set_description(&fe, "DVB-T Single/Master");

    if ( dib7090e_sip_register(&fe, i2c, 0x10, &tfe7790p_layout_config) == NULL)
        return DIB_RETURN_ERROR;

    if (dib7000p_i2c_enumeration(i2c, 1, 0x10, 0x10) != DIB_RETURN_SUCCESS) {
        DibDbgPrint("-E-  Can not find dib7090\n");
        goto error;
    }

    frontend_reset(&fe);

    dib7000p_set_gpio(&fe, 8, 0, 1);
    DibDbgPrint("GPIO 8 ON \n");

    INIT_CHANNEL(&ch, STANDARD_DVBT);
    ch.RF_kHz = 474000;
    ch.bandwidth_kHz = 8000;

    tune_diversity(&fe, 1, &ch);
    set_streaming_dib0700(&fe, &ch, 1, 1);

    DibDbgPrint("-I-  Tuning done <enter>\n");
    getchar();

    while (1) {
        /* after enumerating on the same i2c-bus, the i2c-addresses of the bus will be 0x80 for the diversity master and 0x82 for the slave */
        demod_get_monitoring(&fe, &mon);
        dib7000_print_monitor(&mon, NULL, 0 ,1);
        usleep(100000);
    }

error:
    set_streaming_dib0700(&fe, &ch, 1, 0);
	frontend_unregister_components(&fe);

    close_dib0700_i2c();

	return 0;
}

