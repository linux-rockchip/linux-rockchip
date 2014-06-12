/* basic test application for showing the use of the demod/tuner-driver for the TFE8096P based on a single DiB8096P */
#include "sample_dib0700.h"

#include <adapter/frontend_tune.h>
#include <adapter/sip.h>
#include <demod/dib8000p.h>
#include <sip/dibx090p.h>
#include <tuner/dib0090.h>
#include "monitor/monitor.h"

static int tfe8096p_update_lna(struct dibFrontend *fe, uint16_t agc_global);

static const struct dibx090p_config tfe8096p_layout_config = {
    1,
    tfe8096p_update_lna, // update_lna
    DIBX090P_GPIO_DEFAULT_DIRECTIONS,
    DIBX090P_GPIO_DEFAULT_VALUES,
    DIBX090P_GPIO_DEFAULT_PWM_POS,
    -143, //freq_offset_khz_uhf;
    -143, //freq_offset_khz_vhf;
    12000,  // clock_khz
    48,   //diversity_delay
    NULL,   // dib0090_wbd_table
    0, 0, 0, 0,
    0, // use high level adjustment
};

static int tfe8096p_update_lna(struct dibFrontend *fe, uint16_t agc_global)
{
    dbgpl(NULL, "update lna, agc_global=%d", agc_global);
    if ((tfe8096p_layout_config.use_high_level_adjustment & 1) && (agc_global < 16000))
        demod_set_gpio(fe, 5, 0, 0);

    return 0;
}

int main (void)
{
    // default I2C implementation is based on parallel port but user can connect its
    // own I2C driver using host_i2c_interface_attach();
    // implementation is done in sample/interface/host.c
    //struct dibDataBusHost *i2c = host_i2c_interface_attach(NULL);
	//struct dibDataBusHost *i2c = open_spp_i2c();
	struct dibDataBusHost *i2c = open_dib0700_i2c(0, 1);
	struct dibFrontend fe;
	struct dibChannel ch;
	struct dibDemodMonitor mon;

	DibZeroMemory(&mon, sizeof(mon));

	if (i2c == NULL)
		return 1;

	frontend_init(&fe); /* initializing the frontend-structure */
	frontend_set_id(&fe, 0); /* assign an absolute ID to the frontend */
	frontend_set_description(&fe, "ISDB-T Single/Master");

    if (dib8090p_sip_register(&fe, i2c, 0x10, &tfe8096p_layout_config) == NULL)
        return DIB_RETURN_ERROR;

    if (dib8000p_i2c_enumeration(i2c, 1, DIBX090P_DEFAULT_I2C_ADDRESS, 0x10) != DIB_RETURN_SUCCESS) {
        DibDbgPrint("-E-  Can not find dib8096p\n");
        goto error;
    }

    frontend_reset(&fe);

    demod_set_gpio(&fe, 8, 0, 1);
    DibDbgPrint("GPIO 8 ON \n");

    INIT_CHANNEL(&ch, STANDARD_ISDBT);
    ch.RF_kHz = 737142;

    tune_diversity(&fe, 1, &ch);
    set_streaming_dib0700(&fe, &ch, 0, 1);

    DibDbgPrint("-I-  Tuning done <enter>\n");
    getchar();

    while (1) {
        /* after enumerating on the same i2c-bus, the i2c-addresses of the bus will be 0x80 for the diversity master and 0x82 for the slave */
        demod_get_monitoring(&fe, &mon);
        dib7000_print_monitor(&mon, NULL, 0 ,1);
        usleep(100000);
    }

error:
    set_streaming_dib0700(&fe, &ch, 0, 0);
	frontend_unregister_components(&fe);

    //host_i2c_release(i2c);
	//close_spp_i2c();
    close_dib0700_i2c();

	return 0;
}

