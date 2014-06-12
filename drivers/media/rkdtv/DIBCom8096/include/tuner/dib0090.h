/* Krosus Registers */
#define CTRL_BB_1 1
#define CTRL_BB_2 2
#define CTRL_BB_3 3
#define CTRL_BB_4 4
#define CTRL_BB_5 5
#define CTRL_BB_6 6
#define CTRL_BB_7 7
#define CTRL_RXRF1 8
#define CTRL_RXRF2 9
#define CTRL_RXRF3 10
#define CTRL_RF_SW 11
#define CTRL_RF_V2I 12
#define CTRL_RF_MIX  13
#define CTRL_RF_LOAD 14
#define CTRL_RF_LT 15
#define CTRL_WBDMUX 16
#define CTRL_TX 17
#define IQ_ADC_CTRL 18
#define CTRL_BIAS 19
#define CTRL_CRYSTAL 20
#define CTRL_LO_1 21
#define CTRL_LO_2 22
#define CTRL_LO_3 23
#define CTRL_LO_4 24
#define CTRL_LO_5 25
#define DIG_CFG_RO 26
#define SLEEP_EN 27
#define CTRL_LO_6 28
#define ADCVAL 29
#define ADCCLK 30
#define VGA_MODE 31
#define DIG_CFG_3 32
#define PLL_CFG 33
#define CALIBRATE 34
#define DIG_CFG 35
#define TUNER_EN 36
#define EFUSE_1 37
#define EFUSE_2 38
#define EFUSE_3 39
#define EFUSE_4 40
#define EFUSE_CTRL 41
#define RF_RAMP1 42
#define RF_RAMP2 43
#define RF_RAMP3 44
#define RF_RAMP4 45
#define RF_RAMP5 46
#define RF_RAMP6 47
#define RF_RAMP7 48
#define RF_RAMP8 49
#define RF_RAMP9 50
#define BB_RAMP1 51
#define BB_RAMP2 52
#define BB_RAMP3 53
#define BB_RAMP4 54
#define BB_RAMP5 55
#define BB_RAMP6 56
#define BB_RAMP7 57
#define CURRGAINBB 58
#define CURRGAINRF 59
#define PWM1_REG 60
#define PWM2_REG 61
#define GAIN4_1 62
#define GAIN4_2 63

//Available for P1G
#define CTRL_LO_7 17
#define CAPTRIM_CFG 64

#ifndef TUNER_DIB0090_H
#define TUNER_DIB0090_H

#ifdef __cplusplus
extern "C" {
#endif

struct dibFrontend;
struct dibDataBusHost;

struct dib0090_io_config {
    uint32_t clock_khz;

    uint8_t pll_bypass  : 1;
    uint8_t pll_range   : 1;
    uint8_t pll_prediv  : 6;
    uint8_t pll_loopdiv : 6;

    uint8_t adc_clock_ratio; /* valid is 8, 7 ,6 */
    uint16_t pll_int_loop_filt; // internal loop filt value. If not fill in , default is 8165
};

struct dib0090_wbd_slope {
    uint16_t max_freq; /* for every frequency less than or equal to that field: this information is correct */
    uint16_t slope_cold;
    uint16_t offset_cold;
    uint16_t slope_hot;
    uint16_t offset_hot;
    uint8_t  wbd_gain;
};

struct dib0090_low_if_offset_table {
    int std;
    uint32_t RF_freq;
    int32_t offset_khz;
};

struct dib0090_config {
    struct dib0090_io_config io;
    /* tuner pins controlled externally */
    int (*reset) (struct dibFrontend *, int);
    int (*sleep) (struct dibFrontend *, int);

    /*  offset in kHz */
    int freq_offset_khz_uhf;
    int freq_offset_khz_vhf;

    int (*get_adc_power) (struct dibFrontend *);

    uint8_t clkouttobamse : 1; /* activate or deactivate clock output */
    uint8_t analog_output;

    uint8_t i2c_address;

    uint16_t _unused0; /* was wbd-value - now unused */
    uint16_t force_cband_input;
    uint8_t use_pwm_agc;

    uint8_t clkoutdrive;

    uint8_t fref_clock_ratio;
    uint8_t cap_value;

    const struct dib0090_wbd_slope *wbd;

    uint8_t ls_cfg_pad_drv;
    uint8_t data_tx_drv;
    int (*get_dc_power) (struct dibFrontend *, uint8_t IQ);
    const struct dib0090_low_if_offset_table *low_if;

    uint8_t in_soc;  /* 7090 and 8096P, and ... */
    uint8_t adc_target_apply_3dB; /* VOYAGER and NAUTILUS anafe AGC power is IQ power, Each ADCI and Q are then working 3dB bellow expected level: this bit to one add 3dB to the ADC target */
    uint8_t is_nautilus;
    uint8_t is_dib7090e;
    uint8_t force_crystal_mode;
    uint8_t invert_spectrum;
};

extern struct dibFrontend * dib0090_firmware_register(struct dibFrontend *fe, struct dibDataBusHost *data, const struct dib0090_config *cfg);
extern struct dibFrontend * dib0090_register(struct dibFrontend *, struct dibDataBusHost *, const struct dib0090_config *);
extern struct dibFrontend * dib0090_reset_register(struct dibFrontend *, struct dibDataBusHost *, const struct dib0090_config *);

extern void dib0090_twin_setup(struct dibFrontend *master, struct dibFrontend *slave);
extern uint8_t dib0090_is_SH_capable(struct dibFrontend *fe);

extern int dib0090_gain_control(struct dibFrontend *fe, struct dibChannel *ch);
extern int dib0090_get_digital_clk_out(struct dibFrontend *fe);
extern void dib0090_set_captrim(struct dibFrontend *fe);

extern void dib0090_get_current_gain(struct dibFrontend *fe, uint16_t *rf, uint16_t *bb, uint16_t *rf_gain_limit, uint16_t *rflt);
extern void dib0090_pwm_gain_reset(struct dibFrontend *fe, struct dibChannel *ch);
extern void dib0090_set_dc_servo(struct dibFrontend *fe, uint8_t DC_servo_cutoff);

extern int16_t dib0090_get_wbd_value(struct dibFrontend *);
extern uint16_t dib0090_get_wbd_offset(struct dibFrontend * fe);
extern uint16_t dib0090_get_wbd_target(struct dibFrontend * fe, uint32_t rt_khz);

extern int dib0090_set_wbd_table(struct dibFrontend *fe, const struct dib0090_wbd_slope *wbd);
extern void dib0090_force_wbd_target(struct dibFrontend *fe, uint16_t forced_wbd_offset);

#ifdef CONFIG_DIB0090_USE_PWM_AGC
extern void dib0090_dcc_freq(struct dibFrontend *fe,uint8_t fast);
#endif

extern int dib0090_gain_control(struct dibFrontend *fe, struct dibChannel *ch);

extern int dib0090_set_switch(struct dibFrontend *fe, uint8_t sw1, uint8_t sw2, uint8_t sw3);
extern int dib0090_set_vga(struct dibFrontend *fe, uint8_t onoff);
extern int dib0090_set_cg(struct dibFrontend *fe);
extern int dib0090_update_rframp_7090(struct dibFrontend *fe, uint8_t cfg_sensitivity);
extern int dib0090_update_tuning_table_7090(struct dibFrontend *fe, uint8_t cfg_sensitivity);

#ifdef __cplusplus
}
#endif

#endif

/* Krosus Registers */
#define CTRL_BB_1 1
#define CTRL_BB_2 2
#define CTRL_BB_3 3
#define CTRL_BB_4 4
#define CTRL_BB_5 5
#define CTRL_BB_6 6
#define CTRL_BB_7 7
#define CTRL_RXRF1 8
#define CTRL_RXRF2 9
#define CTRL_RXRF3 10
#define CTRL_RF_SW 11
#define CTRL_RF_V2I 12
#define CTRL_RF_MIX  13
#define CTRL_RF_LOAD 14
#define CTRL_RF_LT 15
#define CTRL_WBDMUX 16
#define CTRL_TX 17
#define IQ_ADC_CTRL 18
#define CTRL_BIAS 19
#define CTRL_CRYSTAL 20
#define CTRL_LO_1 21
#define CTRL_LO_2 22
#define CTRL_LO_3 23
#define CTRL_LO_4 24
#define CTRL_LO_5 25
#define DIG_CFG_RO 26
#define SLEEP_EN 27
#define CTRL_LO_6 28
#define ADCVAL 29
#define ADCCLK 30
#define VGA_MODE 31
#define DIG_CFG_3 32
#define PLL_CFG 33
#define CALIBRATE 34
#define DIG_CFG 35
#define TUNER_EN 36
#define EFUSE_1 37
#define EFUSE_2 38
#define EFUSE_3 39
#define EFUSE_4 40
#define EFUSE_CTRL 41
#define RF_RAMP1 42
#define RF_RAMP2 43
#define RF_RAMP3 44
#define RF_RAMP4 45
#define RF_RAMP5 46
#define RF_RAMP6 47
#define RF_RAMP7 48
#define RF_RAMP8 49
#define RF_RAMP9 50
#define BB_RAMP1 51
#define BB_RAMP2 52
#define BB_RAMP3 53
#define BB_RAMP4 54
#define BB_RAMP5 55
#define BB_RAMP6 56
#define BB_RAMP7 57
#define CURRGAINBB 58
#define CURRGAINRF 59
#define PWM1_REG 60
#define PWM2_REG 61
#define GAIN4_1 62
#define GAIN4_2 63
#define DUMP 99

//Available for P1G
#define CTRL_LO_7 17
#define CAPTRIM_CFG 64

