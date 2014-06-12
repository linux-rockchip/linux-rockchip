#ifndef SIP_DIBX090P_PRIV_H
#define SIP_DIBX090P_PRIV_H

//#define BIST_TEST

#ifdef BIST_TEST
#define BIST_NOP_CMD   0
#define BIST_TYPE_STDALGO 1
#define BIST_DUMP_CMD  1
#define BIST_GO_ON_CMD 2
#define BIST_STOP_CMD  3

#define BIST_ALGO_4N 1
#define BIST_ALGO_MARCH10N 2
#define BIST_ALGO_MARCH14N 3
#define BIST_ALGO_SCAN8N 4
#define BIST_ALGO_MARCH26N 5
#define BIST_ALGO_MARCH22N 6
#define BIST_ALGO_MARCH12N 0

#define BIST_TYPE_MANUAL 0
#define BIST_TYPE_STDALGO 1
#endif

struct dibx090p_best_adc {
    uint32_t timf;        /* New Timf */
    uint32_t pll_loopdiv; /* New prediv */
    uint32_t pll_prediv;  /* New loopdiv */
};

struct dibx090p_adc {
    uint32_t freq;        /* RF freq MHz */
    uint32_t timf;        /* New Timf */
    uint32_t pll_loopdiv; /* New prediv */
    uint32_t pll_prediv;  /* New loopdiv */
};

struct dibx090p_state {
    struct dibSIPInfo info;
    struct dib0090_config dib0090_cfg;
    const struct dibx090p_config *cfg;
    struct dibFrontend *fe;

    struct dibDataBusHost tuner_access;
    uint8_t slow_adc_state;
    uint16_t tuner_enable;

    int (*set_diversity_in) (struct dibFrontend *fe, int onoff);
    int (*set_output_mode) (struct dibFrontend *fe, int onoff);

    uint16_t version;
    uint8_t input_mode_mpeg;
};

extern void dibx090p_release(struct dibFrontend *fe);
extern int dibx090p_get_dc_power(struct dibFrontend *fe, uint8_t IQ);
extern int dibx090p_agc_restart(struct dibFrontend *fe, uint8_t restart);
extern int dibx090p_tuner_sleep(struct dibFrontend *fe, int onoff);
extern int dibx090p_update_lna(struct dibFrontend *fe, uint16_t agc_global);

#ifdef BIST_TEST
extern int dibx090p_test_bist(struct dibFrontend *fe, unsigned int cmdCfg, unsigned int targetMem, unsigned int algoType, unsigned int algoSeq, unsigned int addrCfg, unsigned int dataCfg)
#endif

#endif
