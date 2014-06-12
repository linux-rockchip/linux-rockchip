#ifndef DIB0090_PRIV_H
#define DIB0090_PRIV_H

#define KROSUS_PLL_LOCKED   0x800
#define KROSUS              0x2

/* Use those defines to identify SOC version */
#define SOC               0x02
#define SOC_7090_P1G_11R1 0x82
#define SOC_7090_P1G_21R1 0x8a
#define SOC_8090_P1G_11R1 0x86
#define SOC_8090_P1G_21R1 0x8e

/* else use thos ones to check */
#define P1A_B      0x0
#define P1C	   0x1
#define P1D_E_F    0x3
#define P1G	   0x7
#define P1G_21R2   0xf

#define MP001 0x1 /* Single 9090/8096 */
#define MP005 0x4 /* Single Sband */
#define MP008 0x6 /* Dual diversity VHF-UHF-LBAND */
#define MP009 0x7 /* Dual diversity 29098 CBAND-UHF-LBAND-SBAND */

#define EFUSE

struct dib0090_identity
{
    uint8_t version;
    uint8_t product;
    uint8_t p1g;
    uint8_t in_soc;
};


#define EN_LNA0      0x8000
#define EN_LNA1      0x4000
#define EN_LNA2      0x2000
#define EN_LNA3      0x1000
#define EN_MIX0      0x0800
#define EN_MIX1      0x0400
#define EN_MIX2      0x0200
#define EN_MIX3      0x0100
#define EN_IQADC     0x0040
#define EN_PLL       0x0020
#define EN_TX        0x0010
#define EN_BB        0x0008
#define EN_LO        0x0004
#define EN_BIAS      0x0001

#define EN_IQANA     0x0002
#define EN_DIGCLK    0x0080 /* not in the TUNER_EN reg, only in SLEEP_EN */
#define EN_CRYSTAL   0x0002

#define EN_UHF		 0x22E9
#define EN_VHF		 0x44E9
#define EN_LBD		 0x11E9
#define EN_SBD		 0x44E9
#define EN_CAB		 0x88E9

// Define the Mixer settings
#define MIX_S		0x485a
#define V2I_S           0x0240
#define LOAD_S          0x87d0

// Calibration defines
#define      DC_CAL 0x1
#define     WBD_CAL 0x2
#define    TEMP_CAL 0x4
#define CAPTRIM_CAL 0x8

struct dc_calibration;

struct dib0090_state {
    struct dibTunerInfo info;
    struct dibFrontend *fe;
    struct dibFrontend *twin;

    const struct dib0090_config *config;

    uint16_t wbd_offset;
    int16_t wbd_target; /* in dB */

    int16_t rf_gain_limit;  /* take-over-point: where to split between bb and rf gain */
    int16_t current_gain; /* keeps the currently programmed gain */
    uint8_t agc_step;     /* new binary search */

    uint16_t gain[2]; /* for channel monitoring */

    const uint16_t *rf_ramp;
    const uint16_t *bb_ramp;

    /* for the software AGC ramps */
    uint16_t bb_1_def;
    uint16_t rf_lt_def;
    uint16_t gain_reg[4];

#ifdef CONFIG_TUNER_DIB0090_CAPTRIM_MEMORY
#define MEM_DEPTH 8
    struct {
        uint32_t freq;
        uint8_t cap;
    } memory[MEM_DEPTH];
    uint8_t memory_index;
#endif

    /* for the captrim/dc-offset/temperature search */
    int8_t step;
    int16_t adc_diff;
    int16_t min_adc_diff;

    int8_t captrim;
    int8_t fcaptrim;

    uint16_t wbdmux, bias;

    const struct dc_calibration *dc;
    uint16_t bb4, bb6, bb7;

    const struct dib0090_tuning *current_tune_table_index;
    const struct dib0090_pll *current_pll_table_index;

    const struct dib0090_wbd_slope *current_wbd_table;

    int (*tune)(struct dibFrontend *, struct dibChannel *);

#ifdef CONFIG_TUNER_DIB0090_P1B_SUPPORT
    uint16_t bsearch_step;
    uint8_t rf_ramp_slope;
    int16_t max_rfgain; /* in 10th dB - depending on the tuned band */
    int16_t max_bbgain;
#endif

    uint8_t tuner_is_tuned;
    uint8_t agc_freeze;
    uint8_t current_band;

    int16_t temperature;
    int16_t vdd;

    uint8_t calibrate;

    uint32_t rest;
    uint32_t current_rf;
    uint32_t rf_request;
    uint8_t force_max_gain;
    uint8_t current_standard;
    struct dib0090_identity identity;
    uint8_t wbd_calibration_gain;
    int32_t adc_target;
    uint16_t gain_alpha;
    int8_t ptat_ref_offset;
};

struct dc_calibration {
    uint8_t addr;
    uint8_t offset;
    uint8_t pga;
    uint16_t bb1;
    uint8_t i;
};

struct dib0090_tuning {
    uint32_t max_freq; /* for every frequency less than or equal to that field: this information is correct */
    uint8_t switch_trim;
    uint8_t lna_tune;
    uint16_t lna_bias; //Force to lay a 16bits due to the cable band
    uint16_t v2i;
    uint16_t mix;
    uint16_t load;
    uint16_t tuner_enable;
};

struct dib0090_pll {
    uint32_t max_freq; /* for every frequency less than or equal to that field: this information is correct */
    uint8_t vco_band;
    uint8_t hfdiv_code;
    uint8_t hfdiv;
    uint8_t topresc;
};

struct slope
{
    int16_t range;
    int16_t slope;
};

extern int dib0090_p1b_register(struct dibFrontend *fe);
extern int dib0090_identify(struct dibFrontend *fe);
extern void dib0090_reset_digital(struct dibFrontend *fe, const struct dib0090_config *cfg);
extern uint32_t dib0090_get_slow_adc_val(struct dib0090_state *state);
extern int dib0090_sleep(struct dibFrontend *fe);
extern int dib0090_gain_control(struct dibFrontend *fe, struct dibChannel *ch);
extern void dib0090_set_EFUSE(struct dib0090_state *state);
extern void dib0090_set_boost(struct dib0090_state *state, int onoff);

/* Calibration functions */
extern int dib0090_dc_offset_calibration(struct dib0090_state *state, enum frontend_tune_state *tune_state);
extern int dib0090_wbd_calibration(struct dib0090_state *state, enum frontend_tune_state *tune_state);
extern int dib0090_get_temperature(struct dib0090_state *state, enum frontend_tune_state *tune_state);
extern int dib0090_captrim_search(struct dib0090_state *state, enum frontend_tune_state *tune_state);

/* IO functions*/
extern void dib0090_write_reg(struct dib0090_state *st, uint32_t r, uint16_t v);
extern uint16_t dib0090_read_reg(struct dib0090_state *st, uint32_t reg);
#ifdef CONFIG_DIB0090_USE_PWM_AGC
extern void dib0090_write_regs(struct dib0090_state *state, uint8_t r, const uint16_t *b, uint8_t c);
#endif
#define BUILD_LEGACY_DIB_ADAPTER
#ifdef BUILD_LEGACY_DIB_ADAPTER
#define dbgpHal(...) dbgp("DIB0090: " __VA_ARGS__)
#endif


extern const struct dibDebugObject dib0090_dbg;

#endif
