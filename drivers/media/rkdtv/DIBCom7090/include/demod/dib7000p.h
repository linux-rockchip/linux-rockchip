#ifndef DEMOD_DIB7000P_H
#define DEMOD_DIB7000P_H

#include "dibx000_common.h"

#ifdef __cplusplus
extern "C" {
#endif

struct dibDataBusHost;
struct dibDataBusClient;
struct dibFrontend;
struct dibDemodMonitor;

//! Configuration of one DiB7000P demodulator
/**
 * This struct is used to configure the DiB7000-demod-driver to make it handle an IC on a specific board correctly.
 */
struct dib7000p_config {

/**
 * indicates if the demod shall output the MPEG2-TS as 188 or 204 bytes
 */
	uint8_t output_mpeg2_in_188_bytes;

/**
 * use hostbus for diversity out or standard diversity out (or nothing in single mode)
 */
	uint8_t hostbus_diversity;

/**
 * tell the demod if its tuner is baseband or not
 */
	uint8_t tuner_is_baseband;

/**
 * callback of the demodulator to set up an LNA.
 * @param demod is the demod-context
 * @param agc_global is the value n_agc_global - it is used for the implementation to know if the LNA-status can be changed or not.
 * @return you have to return 1 when the LNA-status has changed otherwise 0 (when 1 is returned the demod restarts its AGC)
 */
	int (*update_lna) (struct dibFrontend *demod, uint16_t agc_global);

/**
 * number of AGC config given below
 */
	uint8_t agc_config_count;
/**
 * AGC parameters @see struct dibx000_agc_config
 */
	const struct dibx000_agc_config *agc;
/**
 * Bandwidth parameters @see struct dibx000_bandwidth_parameter
 */
	const struct dibx000_bandwidth_config *bw;

/**
 * monitoring information
 */
	int lna_gain_step;

/**
 * monitoring information
 */
	int tun_max_gain;

/**
 * monitoring information
 */
	int tun_ref_level;

/**
 * Initial GPIO setup
 */
#define DIB7000P_GPIO_DEFAULT_DIRECTIONS 0xffff

/**
 * Initial GPIO directions
 */
	uint16_t gpio_dir;

#define DIB7000P_GPIO_DEFAULT_VALUES     0x0000
/**
 * Initial GPIO values
 */
	uint16_t gpio_val;

#define DIB7000P_GPIO_PWM_POS0(v)        ((v & 0xf) << 12)
#define DIB7000P_GPIO_PWM_POS1(v)        ((v & 0xf) << 8 )
#define DIB7000P_GPIO_PWM_POS2(v)        ((v & 0xf) << 4 )
#define DIB7000P_GPIO_PWM_POS3(v)         (v & 0xf)
#define DIB7000P_GPIO_DEFAULT_PWM_POS    0xffff
/**
 * Initial GPIO PWM Positions
 */
	uint16_t gpio_pwm_pos;

/**
 * set the frequency divider for PWM
 */
    uint16_t pwm_freq_div;

/**
 * a callback called, before and after an AGC restart of the demod,
 * @before indicates if the call is before or after
 */
	int (*agc_control) (struct dibFrontend *, uint8_t before);

/**
 * use software spurious protectiong
 */
	uint8_t spur_protect : 1;

/**
 * disable sample and hold in ADC configuration
 */
    uint8_t disable_sample_and_hold : 1;

/**
 * enable the current mirror in the ADC configuration
 */
    uint8_t enable_current_mirror : 1;
    uint16_t diversity_delay;
    uint8_t forward_erronous_mpeg_packet; /* if 1 the erronous mpeg packet are forwared; if 0, the erronous mpeg packet are discarded */
    uint8_t invert_spectrum;
};

#define DEFAULT_DIB7000P_I2C_ADDRESS 18


extern struct dibFrontend * dib7000p_register(struct dibFrontend *fe, struct dibDataBusHost *host, uint8_t i2c_addr, const struct dib7000p_config *cfg);
extern int dib7000p_i2c_enumeration(struct dibDataBusHost *host, int no_of_demods, uint8_t default_addr, uint8_t first_addr);

extern void dib7000p_close_gate(struct dibFrontend *fe);

//! get the struct dibDataBusHost for a specific demod
/**
 * This function gives you access to the different I2C-Interfaces (and methods) of the DiB7000 by returning the appropriate pointer to a struct dibDataBusHost.
 * \param demod specifies the demod to use as I2C-master
 * \param intf specifies the interface
 * \param gating indicates if the I2C-Adapter is just opening the gate instead of using the I2C-master for transmitting the I2C-message. (Works only, if the demod itself is on an I2C-bus).
 * \return reference to the appropriate I2C-Adapter or NULL in case of an error
 */
extern struct dibDataBusHost * dib7000p_get_i2c_master(struct dibFrontend *demod, enum dibx000_i2c_interface intf, int gating);

//! GPIOs of the DiB7000
/**
 * Controlling the GPIOs of the DiB7000-demodulator.
 * @param demod demodulator data
 * @param num number of GPIO to control (if in doubt do not touch at least GPIO1,2 and GPIO3,4)
 * @param dir GPIO director 0 = OUT, 1 = IN
 * @param val GPIO value 1 or 0
 */
extern int dib7000p_set_gpio(struct dibFrontend *demod, uint8_t num, uint8_t dir, uint8_t val);

//! GPIOs of the DiB7000
/**
 * Reading the GPIOs of the DiB7000-demodulator.
 * @param demod demodulator data
 * @param num number of GPIO to read
 */
extern uint8_t dib7000p_get_gpio(struct dibFrontend *demod, uint8_t num);


//! GPIOs of the DiB7000
/**
 * Reading the direction of the DiB7000-demodulator GPIOs.
 * @param demod demodulator data
 * @param num number of GPIO direction to read
 */
extern uint8_t dib7000p_get_gpio_dir(struct dibFrontend *demod, uint8_t num);

//! enabled the VBG voltage of the ADC
/**
 * On some boards it is necessary to enabled the VBG voltage of the ADC, while
 * the demod is powered off, to improve reception quality of the Analog-Chip.
 */
extern int dib7000p_enable_vbg_voltage(struct dibFrontend *demod);

//! detect the dib7000p on an i2c-bus
extern int dib7000pc_detection(struct dibDataBusHost *i2c_adap);

extern int dib7000p_set_acquisition_mode(struct dibFrontend *demod, uint8_t agc_disable);

//! Enable or disable the SFN workaround
/**
 * In some place in the world the broadcasters do not use the same
 * cell IDs for different transmitters in a SFN. If you experience
 * problem in such places you need to enable the workaround by calling
 * this function one time
 */
extern int dib7000p_bad_sfn_workaround(struct dibFrontend *demod, uint8_t enable);

extern void dib7000p_set_agc_range(struct dibDemod *demod, uint8_t agc, uint16_t min, uint16_t max);
extern void dib7000p_get_agc_values(struct dibDemod *demod, uint16_t *agc_global, uint16_t *agc1, uint16_t *agc2, uint16_t *wbd);
extern void dib7000p_get_sad_data(struct dibFrontend *fe, uint16_t *sad0, uint16_t *sad1, uint16_t *sad2, uint16_t *sad3);
extern void dib7000p_restart_agc(struct dibFrontend *fe);

//! Attaching one or more DiB7000P-demodulators using I2C-bus
/**
 * This function is only provided for backward compatibility. Please use dib7000p_register.
 */
extern int dib7000p_attach(struct dibDataBusHost *i2c_adap, int no_of_demods, uint8_t default_addr, uint8_t do_i2c_enum, const struct dib7000p_config cfg[], struct dibFrontend *demod[]);

extern int dib7000p_update_pll(struct dibFrontend *fe, struct dibx000_bandwidth_config * bw);
extern int dib7000p_agc_startup(struct dibFrontend *, struct dibChannel *ch);
extern int32_t dib7000p_get_adc_power(struct dibFrontend *fe);
extern int dib7000p_set_agc1_min(struct dibFrontend *fe, uint16_t v);
#ifdef CONFIG_DEMOD_DIB7000P_SMART_SFN_CORRECTION
extern int dib7000p_update_FFT_window(struct dibFrontend *fe, uint8_t nb_tps_window);
#endif

#ifdef __cplusplus
}
#endif

#endif
