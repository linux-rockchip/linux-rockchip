/* basic test application for showing the use of the demod/tuner-driver for the TFE7090P_PVR4 based on four DiB7090P */
#include "sample.h"

#include <adapter/frontend_tune.h>
#include <adapter/sip.h>
#include <demod/dib7000p.h>
#include <sip/dibx090p.h>
#include <tuner/dib0090.h>
#include "monitor/monitor.h"

static int dib7090p_update_lna(struct dibFrontend *fe, uint16_t agc_global);

static const struct dibx090p_config tfe7090p_pvr_layout_config[4] = {
    {
        1,      // output_mpeg2_in_188_bytes
        dib7090p_update_lna,   // update_lna
        DIBX090P_GPIO_DEFAULT_DIRECTIONS - ( (1 << 4) ),
        DIBX090P_GPIO_DEFAULT_VALUES     + ( (1 << 4) ),
        DIBX090P_GPIO_DEFAULT_PWM_POS,     // gpio_pwm_pos
        50,    // dib0070_freq_offset_khz_uhf
        70,   // dib0070_freq_offset_khz_vhf
        12000,  // clock_khz
        0,      // diversity delay
        NULL,   // dib0090_wbd_table
        0,      // osc_buffer_state
        4,      // clock_pad_drive
        1,      // tuner_gating
        0,      // invert_iq
        0,      // use_high_level_adjustment
        0,      // force_crystal_mode 0 means auto mode; 1 means force cristal <24MHz
        0x2d98, // drives
        1,      // div_cfg
        1,      // clkouttobamse
        0,      // clkoutdrive
        0,      // refclksel
        0,      // fref_clock_ratio
        0,      // forward_erronous_mpeg_packet
        0,      // invert_spectrum
    },{
        1,      // output_mpeg2_in_188_bytes
        dib7090p_update_lna,   // update_lna
        DIBX090P_GPIO_DEFAULT_DIRECTIONS - ( (1 << 4) ),
        DIBX090P_GPIO_DEFAULT_VALUES     + ( (1 << 4) ),
        DIBX090P_GPIO_DEFAULT_PWM_POS,     // gpio_pwm_pos
        -50,    // dib0070_freq_offset_khz_uhf
        -70,    // dib0070_freq_offset_khz_vhf
        12000,  // clock_khz
        0,      // diversity delay
        NULL,   // dib0090_wbd_table
        0,      // osc_buffer_state
        2,      // clock_pad_drive
        1,      // tuner_gating
        0,      // invert_iq
        0,      // use_high_level_adjustment   /* WHEN TO USE ? */
        0,      // force_crystal_mode 0 means auto mode; 1 means force cristal <24MHz
        0x2d98, // drives
        1,      // div_cfg
        1,      // clkouttobamse
        0,      // clkoutdrive
        0,      // refclksel
        0,      // fref_clock_ratio
        0,      // forward_erronous_mpeg_packet
        0,      // invert_spectrum
    },{
        1,      // output_mpeg2_in_188_bytes
        dib7090p_update_lna,   // update_lna
        DIBX090P_GPIO_DEFAULT_DIRECTIONS - ( (1 << 4) ),
        DIBX090P_GPIO_DEFAULT_VALUES     + ( (1 << 4) ),
        DIBX090P_GPIO_DEFAULT_PWM_POS,     // gpio_pwm_pos
        50,     // dib0070_freq_offset_khz_uhf
        70,     // dib0070_freq_offset_khz_vhf
        12000,  // clock_khz
        0,      // diversity delay
        NULL,   // dib0090_wbd_table
        0,      // osc_buffer_state
        2,      // clock_pad_drive
        1,      // tuner_gating
        0,      // invert_iq
        0,      // use_high_level_adjustment   /* WHEN TO USE ? */
        0,      // force_crystal_mode 0 means auto mode; 1 means force cristal <24MHz
        0x2d98, // drives
        1,      // div_cfg
        1,      // clkouttobamse
        0,      // clkoutdrive
        0,      // refclksel
        0,      // fref_clock_ratio
        0,      // forward_erronous_mpeg_packet
        0,      // invert_spectrum
    },{
        1,      // output_mpeg2_in_188_bytes
        dib7090p_update_lna,   // update_lna
        DIBX090P_GPIO_DEFAULT_DIRECTIONS - ( (1 << 4) ),
        DIBX090P_GPIO_DEFAULT_VALUES     + ( (1 << 4) ),
        DIBX090P_GPIO_DEFAULT_PWM_POS,     // gpio_pwm_pos
        -50,    // dib0070_freq_offset_khz_uhf
        -70,    // dib0070_freq_offset_khz_vhf
        12000,  // clock_khz
        0,      // diversity delay
        NULL,   // dib0090_wbd_table
        0,      // osc_buffer_state
        2,      // clock_pad_drive
        1,      // tuner_gating
        0,      // invert_iq
        0,      // use_high_level_adjustment   /* WHEN TO USE ? */
        0,      // force_crystal_mode 0 means auto mode; 1 means force cristal <24MHz
        0x2d98, // drives
        1,      // div_cfg
        1,      // clkouttobamse
        0,      // clkoutdrive
        0,      // refclksel
        0,      // fref_clock_ratio
        0,      // forward_erronous_mpeg_packet
        0,      // invert_spectrum
    }
};

static int dib7090p_update_lna(struct dibFrontend *fe, uint16_t agc_global)
{
    return 0;
}

int main (void)
{
    // default I2C implementation is based on parallel port but user can connect its
    // own I2C driver using host_i2c_interface_attach();
    // implementation is done in sample/interface/host.c
    //struct dibDataBusHost *i2c = host_i2c_interface_attach(NULL);
	struct dibDataBusHost *i2c = open_spp_i2c();
	struct dibFrontend fe[4];
	struct dibChannel ch[4];
	struct dibDemodMonitor mon[4];

	DibZeroMemory(mon, sizeof(mon));

	if (i2c == NULL)
		return 1;

	frontend_init(&fe[0]); /* initializing the frontend-structure */
	frontend_set_id(&fe[0], 0); /* assign an absolute ID to the frontend */
	frontend_set_description(&fe[0], "DVB-T/H Single/Master");

	frontend_init(&fe[1]); /* initializing the frontend-structure */
	frontend_set_id(&fe[1], 1); /* assign an absolute ID to the frontend */
	frontend_set_description(&fe[1], "DVB-T/H Slave/PVR#1");

	frontend_init(&fe[2]); /* initializing the frontend-structure */
	frontend_set_id(&fe[2], 2); /* assign an absolute ID to the frontend */
	frontend_set_description(&fe[2], "DVB-T/H Slave/PVR#2");

	frontend_init(&fe[3]); /* initializing the frontend-structure */
	frontend_set_id(&fe[3], 3); /* assign an absolute ID to the frontend */
	frontend_set_description(&fe[3], "DVB-T/H Slave/PVR#3");

    if ( dib7090p_sip_register(&fe[0], i2c, 0x90, &tfe7090p_pvr_layout_config[0]) == NULL)
        return DIB_RETURN_ERROR;
    if ( dib7090p_sip_register(&fe[1], i2c, 0x92, &tfe7090p_pvr_layout_config[1]) == NULL)
        return DIB_RETURN_ERROR;
    if ( dib7090p_sip_register(&fe[2], i2c, 0x94, &tfe7090p_pvr_layout_config[2]) == NULL)
        return DIB_RETURN_ERROR;
    if ( dib7090p_sip_register(&fe[3], i2c, 0x96, &tfe7090p_pvr_layout_config[3]) == NULL)
        return DIB_RETURN_ERROR;

    if (dib7000p_i2c_enumeration(i2c, 1, 0x10, 0x90) == DIB_RETURN_ERROR) {
        DibDbgPrint("-E-  DiB7090P: enumeration of the master chip failed.\n");
        return DIB_RETURN_ERROR;
    }

    /* reset GPIO */
    data_bus_client_write16(demod_get_data_bus_client(&fe[0]), 1032, 0xffff);

    /* reset slave */
    dib7000p_set_gpio(&fe[0], 1, 0, 1);
    dib7000p_set_gpio(&fe[0], 1, 0, 0);
    dib7000p_set_gpio(&fe[0], 1, 0, 1);

    if(dib7000p_i2c_enumeration(i2c, 1, 0x10, 0x92) == DIB_RETURN_ERROR) {
        DibDbgPrint("-E-  DiB7090P: enumeration of the slave1 chip failed.\n");
        return DIB_RETURN_ERROR;
    }

    /* reset GPIO */
    data_bus_client_write16(demod_get_data_bus_client(&fe[1]), 1032, 0xffff);

    /* reset slave */
    dib7000p_set_gpio(&fe[1], 1, 0, 1);
    dib7000p_set_gpio(&fe[1], 1, 0, 0);
    dib7000p_set_gpio(&fe[1], 1, 0, 1);

    if(dib7000p_i2c_enumeration(i2c, 1, 0x10, 0x94) == DIB_RETURN_ERROR) {
        DibDbgPrint("-E-  DiB7090P: enumeration of the slave2 chip failed.\n");
        return DIB_RETURN_ERROR;
    }

    /* reset GPIO */
    data_bus_client_write16(demod_get_data_bus_client(&fe[2]), 1032, 0xffff);

    /* reset slave */
    dib7000p_set_gpio(&fe[2], 1, 0, 1);
    dib7000p_set_gpio(&fe[2], 1, 0, 0);
    dib7000p_set_gpio(&fe[2], 1, 0, 1);

    if(dib7000p_i2c_enumeration(i2c, 1, 0x10, 0x96) == DIB_RETURN_ERROR) {
        DibDbgPrint("-E-  DiB7090P: enumeration of the slave2 chip failed.\n");
        return DIB_RETURN_ERROR;
    }

    frontend_reset(&fe[0]);
    frontend_reset(&fe[1]);
    frontend_reset(&fe[2]);
    frontend_reset(&fe[3]);

    INIT_CHANNEL(&ch[0], STANDARD_DVBT);
    ch[0].RF_kHz = 474000;
    ch[0].bandwidth_kHz = 8000;
    INIT_CHANNEL(&ch[1], STANDARD_DVBT);
    ch[1].RF_kHz = 490000;
    ch[1].bandwidth_kHz = 8000;
    INIT_CHANNEL(&ch[2], STANDARD_DVBT);
    ch[2].RF_kHz = 498000;
    ch[2].bandwidth_kHz = 8000;
    INIT_CHANNEL(&ch[3], STANDARD_DVBT);
    ch[3].RF_kHz = 506000;
    ch[3].bandwidth_kHz = 8000;

    tune_diversity(&fe[0], 1, &ch[0]);
    tune_diversity(&fe[1], 1, &ch[1]);
    tune_diversity(&fe[2], 1, &ch[2]);
    tune_diversity(&fe[3], 1, &ch[3]);

    DibDbgPrint("-I-  Tuning done <enter>\n");
    getchar();

    while (1) {
#ifdef CONFIG_DEMOD_DIB7000P_SMART_SFN_CORRECTION
        /* update the FFT window for pathological SFN topology (only for guard interval 1/4) */
        if ((data_bus_client_read16(demod_get_data_bus_client(&fe[0]), 463) & 0x3) == 3)
            dib7000p_update_FFT_window(&fe[0], 2);
        if ((data_bus_client_read16(demod_get_data_bus_client(&fe[1]), 463) & 0x3) == 3)
            dib7000p_update_FFT_window(&fe[1], 2);
        if ((data_bus_client_read16(demod_get_data_bus_client(&fe[2]), 463) & 0x3) == 3)
            dib7000p_update_FFT_window(&fe[2], 2);
        if ((data_bus_client_read16(demod_get_data_bus_client(&fe[3]), 463) & 0x3) == 3)
            dib7000p_update_FFT_window(&fe[3], 2);
#endif
        demod_get_monitoring(&fe[0], &mon[0]);
        demod_get_monitoring(&fe[1], &mon[1]);
        demod_get_monitoring(&fe[2], &mon[2]);
        demod_get_monitoring(&fe[3], &mon[3]);
        dib7000_print_monitor(mon, NULL, 0 ,4);
        usleep(100000);
    }

	frontend_unregister_components(&fe[3]);
	frontend_unregister_components(&fe[2]);
	frontend_unregister_components(&fe[1]);
	frontend_unregister_components(&fe[0]);

    //host_i2c_release(i2c);
	close_spp_i2c();

	return 0;
}

