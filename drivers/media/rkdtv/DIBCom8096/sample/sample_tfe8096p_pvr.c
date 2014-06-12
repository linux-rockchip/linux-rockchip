/* basic test application for showing the use of the demod/tuner-driver for the TFE8096P_PVR based on two DiB8096P */
#ifdef DIB0700_INTERFACE
#include "sample_dib0700.h"
#else
#include "sample.h"
#endif

#include <adapter/frontend_tune.h>
#include <adapter/sip.h>
#include <demod/dib8000p.h>
#include <sip/dibx090p.h>
#include <tuner/dib0090.h>
#include "monitor/monitor.h"

static int dib8096p_update_lna(struct dibFrontend *fe, uint16_t agc_global);

static const struct dibx090p_config tfe8096p_pvr_config[2] = {
    {
        1,      // output_mpeg2_in_188_bytes
        dib8096p_update_lna,   // update_lna
        DIBX090P_GPIO_DEFAULT_DIRECTIONS - ( (1 << 4) ),
        DIBX090P_GPIO_DEFAULT_VALUES     + ( (1 << 4) ),
        DIBX090P_GPIO_DEFAULT_PWM_POS,     // gpio_pwm_pos
        -143, //freq_offset_khz_uhf;
        -143, //freq_offset_khz_vhf;
        12000,  // clock_khz
        0,      // diversity delay
        NULL,   // dib0090_wbd_table
        0, 0, 0, 0,
        0, // use high level adjustment
    },{
        1,      // output_mpeg2_in_188_bytes
        dib8096p_update_lna,   // update_lna
        DIBX090P_GPIO_DEFAULT_DIRECTIONS - ( (1 << 4) ),
        DIBX090P_GPIO_DEFAULT_VALUES     + ( (1 << 4) ),
        DIBX090P_GPIO_DEFAULT_PWM_POS,     // gpio_pwm_pos
        -143, //freq_offset_khz_uhf;
        -143, //freq_offset_khz_vhf;
        12000,  // clock_khz
        0,      // diversity delay
        NULL,   // dib0090_wbd_table
        0, 0, 0, 0,
        0, // use high level adjustment
    }
};

static int dib8096p_update_lna(struct dibFrontend *fe, uint16_t agc_global)
{
    dbgpl(NULL, "update lna, agc_global=%d", agc_global);

    if ((tfe8096p_pvr_config[0].use_high_level_adjustment & 1) && (agc_global < 16000))
        demod_set_gpio(fe, 5, 0, 0);

    return 0;
}

/* for the TFE8096P-PVR board, if tuning only the second frontend, need to enable MPEG mux of the first frontend */
struct dibFrontend *fe_master;
int (*tfe8096p_pvr_set_output_mode)(struct dibFrontend *fe, int mode);
static int tfe8096p_pvr_layout_set_output_mode(struct dibFrontend *fe, int mode)
{
    dbgpl(NULL, "overide set output mode: fe%i mode=%i", fe->id, mode);

    if(mode != OUTPUT_MODE_DIVERSITY) {
        tfe8096p_pvr_set_output_mode(fe, mode);
        dibx090p_setDibTxMux(fe, MPEG_ON_DIBTX);
        dibx090p_setHostBusMux(fe, DIBTX_ON_HOSTBUS);

        dibx090p_set_output_mode(fe_master, mode);
        dibx090p_set_diversity_in(fe_master, 0);
    } else
        tfe8096p_pvr_set_output_mode(fe, mode);

    return DIB_RETURN_SUCCESS;
}

int main (void)
{
#ifndef DIB0700_INTERFACE
    // default I2C implementation is based on parallel port but user can connect its
    // own I2C driver using host_i2c_interface_attach();
    // implementation is done in sample/interface/host.c
    //struct dibDataBusHost *i2c = host_i2c_interface_attach(NULL);
	struct dibDataBusHost *i2c = open_spp_i2c();
#else
	struct dibDataBusHost *i2c = open_dib0700_i2c(0, 1);
#endif
	struct dibDataBusHost *slave_data;
	struct dibFrontend fe[2];
	struct dibChannel ch[2];
	struct dibDemodMonitor mon[2];

	DibZeroMemory(mon, sizeof(mon));

	if (i2c == NULL)
		return 1;

	frontend_init(&fe[0]); /* initializing the frontend-structure */
	frontend_set_id(&fe[0], 0); /* assign an absolute ID to the frontend */
	frontend_set_description(&fe[0], "ISDB-T Single/Master");

	frontend_init(&fe[1]); /* initializing the frontend-structure */
	frontend_set_id(&fe[1], 1); /* assign an absolute ID to the frontend */
	frontend_set_description(&fe[1], "ISDB-T Slave/PVR");

    if ( dib8090p_sip_register(&fe[0], i2c, 0x90, &tfe8096p_pvr_config[0]) == NULL)
        return DIB_RETURN_ERROR;
    slave_data = dib8000p_get_i2c_master(&fe[0], DIBX000_I2C_INTERFACE_GPIO_6_7, 1);
    if ( dib8090p_sip_register(&fe[1], slave_data, 0x92, &tfe8096p_pvr_config[1]) == NULL)
        return DIB_RETURN_ERROR;

    /* override set output mode */
    fe_master = &fe[0];
    tfe8096p_pvr_set_output_mode = fe[1].demod_info->ops.set_output_mode;
    fe[1].demod_info->ops.set_output_mode = tfe8096p_pvr_layout_set_output_mode;

    if (dib8000p_i2c_enumeration(i2c, 1, DIBX090P_DEFAULT_I2C_ADDRESS, 0x90) == DIB_RETURN_ERROR) {
        DibDbgPrint("-E-  DiB8096P: enumeration of the master chip failed.\n");
        return DIB_RETURN_ERROR;
    }

    /* reset GPIO */
    data_bus_client_write16(demod_get_data_bus_client(&fe[0]), 1032, 0xffff);

    /* reset slave */
    data_bus_client_write16(demod_get_data_bus_client(&fe[0]), 1032, 0xffff);
    demod_set_gpio(&fe[0], 8, 0, 1);
    demod_set_gpio(&fe[0], 8, 0, 0);
    demod_set_gpio(&fe[0], 8, 0, 1);

    if(dib8000p_i2c_enumeration(slave_data, 1, 0x10, 0x92) == DIB_RETURN_ERROR) {
        DibDbgPrint("-E-  DiB8096P: enumeration of the master chip failed.\n");
        return DIB_RETURN_ERROR;
    }

    frontend_reset(&fe[0]);
    frontend_reset(&fe[1]);

    INIT_CHANNEL(&ch[0], STANDARD_ISDBT);
    ch[0].RF_kHz = 737142;
    INIT_CHANNEL(&ch[1], STANDARD_ISDBT);
    ch[1].RF_kHz = 665142;

    tune_diversity(&fe[0], 1, &ch[0]);
    tune_diversity(&fe[1], 1, &ch[1]);
#ifdef DIB0700_INTERFACE
    set_streaming_dib0700(&fe[0], &ch[0], 1, 1);
    set_streaming_dib0700(&fe[1], &ch[1], 0, 1);
#endif

    DibDbgPrint("-I-  Tuning done <enter>\n");
    getchar();

    while (1) {
        demod_get_monitoring(&fe[0], &mon[0]);
        demod_get_monitoring(&fe[1], &mon[1]);
        dib7000_print_monitor(mon, NULL, 0 ,2);
        usleep(100000);
    }

#ifdef DIB0700_INTERFACE
    set_streaming_dib0700(&fe[0], &ch[0], 1, 0);
    set_streaming_dib0700(&fe[1], &ch[1], 0, 0);
#endif
	frontend_unregister_components(&fe[0]);
	frontend_unregister_components(&fe[1]);

#ifndef DIB0700_INTERFACE
    //host_i2c_release(i2c);
	close_spp_i2c();
#else
	close_dib0700_i2c();
#endif

	return 0;
}

