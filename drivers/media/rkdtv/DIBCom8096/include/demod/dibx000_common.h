#ifndef DIBX000_COMMON_H
#define DIBX000_COMMON_H

#include "../adapter/common.h"

//! I2C interface enum describes the different I2C interfaces of the DiBX000-demods
enum dibx000_i2c_interface {
	DIBX000_I2C_INTERFACE_TUNER      = 0, //!< Selects the INT_TUNER interface
	DIBX000_I2C_INTERFACE_GPIO_1_2   = 1, //!< Selects the GP1 and GP2 interface only on 7000
	DIBX000_I2C_INTERFACE_GPIO_3_4   = 2, //!< Selects the GP3 and GP4 interface only on 7000
	DIBX000_I2C_INTERFACE_GPIO_0_1   = 3,  //!< Selects the GP0 and GP1 interface only on 7000
        DIBX000_I2C_INTERFACE_GPIO_8_9   = 4,  //!< Selects the GP8 and GP9
        DIBX000_I2C_INTERFACE_GPIO_10_11 = 5,  //!< Selects the GP10 and GP11
        DIBX000_I2C_INTERFACE_GPIO_12_13 = 6,  //!< Selects the GP12 and GP13
        DIBX000_I2C_INTERFACE_GPIO_6_7   = 3   //!< Selects the GP6 and GP7
};

enum dibx000_0wire_interface {
	DIBx000_0WIRE_8      = 0, //Gpio 8 on the 0 wire
	DIBx000_0WIRE_14     = 1, //Gpio 14 on the 0 Wire 
	DIBx000_1WIRE_8      = 2, //Gpio 8 on the 1 wire
	DIBx000_1WIRE_14     = 3, //Gpio 14 on the 1 Wire
};

struct dibx000_agc_config {
	/* defines the capabilities of this AGC-setting - using the BAND_-defines*/
	uint8_t  band_caps;

	/* 7000PC/MC: P_agc_use_sd_mod1, P_agc_use_sd_mod2 +
	 * 7000x:     P_agc_freq_pwm_div, P_agc_inv_pwm1, P_agc_inv_pwm2,
	 *            P_agc_inh_dc_rv_est, P_agc_time_est, P_agc_freeze, P_agc_nb_est, P_agc_write */
	/* 3000MC/P:  P_agc_rst_accu_conv, P_agc_freq_pwm_div, P_agc_inv_pwm_hf,
	 *            P_agc_inv_pwm_if, P_agc_inv_pwm_ext, P_agc_win_length */
	uint16_t setup;

	/* 7000x: P_agc_inv_gain */
	uint16_t inv_gain;
	/* 7000x: P_agc_time_stabiliz */
	uint16_t time_stabiliz;

	/* 7000x: P_agc_alpha_level, P_agc_thlock */
	uint8_t  alpha_level;
	uint16_t thlock;

	/* 7000MC/PC: P_agc_wbd_inv, P_sad_wbd_opp */
	uint8_t  wbd_inv;
	/* 7000x:  P_agc_wbd_ref */
	uint16_t wbd_ref;
	/* 7000MC/PC: P_agc_wbd_sel, P_sad_wbd_sel */
	uint8_t wbd_sel;
	/* 7000x:  P_agc_wbd_alpha */
	uint8_t wbd_alpha;

	uint16_t agc1_max;
	uint16_t agc1_min;
	uint16_t agc2_max;
	uint16_t agc2_min;

	uint8_t agc1_pt1;
	uint8_t agc1_pt2;
	/* 7000PC/MC: pt3 */
	uint8_t agc1_pt3;

	uint8_t agc1_slope1;
	uint8_t agc1_slope2;

	uint8_t agc2_pt1;
	uint8_t agc2_pt2;

	uint8_t agc2_slope1;
	uint8_t agc2_slope2;

	uint8_t alpha_mant;
	uint8_t alpha_exp;

	uint8_t beta_mant;
	uint8_t beta_exp;

/**
 * tells the demod to use AGC softsplit and not the wideband-detector, the caller has to fill @see agc_split with useful values then.
 */
	uint8_t perform_agc_softsplit;

//! AGC Split thresholds
/**
 * Defines agc-split thresholds and limits of a board when AGC-Split is used.
 */
	struct {
		uint16_t min;       // AGC_SPLIT_MIN
		uint16_t max;       // AGC_SPLIT_MAX
		uint16_t min_thres; // AGC_GLOBAL_SPLIT_MIN
		uint16_t max_thres; // AGC_GLOBAL_SPLIT_MAX
	} split;
};

struct dibx000_bandwidth_config {
	uint32_t   internal;  /* internal clock in kHz */
	uint32_t   sampling;  /* sampling frequency in kHz */

	/* pll_cfg */
	uint8_t pll_prediv;
	uint8_t pll_ratio;
	uint8_t pll_range;
	uint8_t pll_reset;
	uint8_t pll_bypass;

/* TODO think about adding :<bitsize> */
	uint8_t enable_refdiv;
	uint8_t bypclk_div;
	uint8_t IO_CLK_en_core;
	uint8_t ADClkSrc;
	uint8_t modulo;

	uint16_t sad_cfg;

	uint32_t ifreq;
	uint32_t timf;
	uint32_t xtal_hz;
    uint8_t refclk;
    uint8_t divclk;
};

enum dibx000_adc_states {
	DIBX000_SLOW_ADC_ON = 0,
	DIBX000_SLOW_ADC_OFF,
	DIBX000_ADC_ON,
	DIBX000_ADC_OFF,
	DIBX000_VBG_ENABLE,
	DIBX000_VBG_DISABLE,
};

#endif
