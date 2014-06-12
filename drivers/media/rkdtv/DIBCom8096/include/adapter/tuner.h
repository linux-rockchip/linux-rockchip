#ifndef ADAPTER_TUNER_H
#define ADAPTER_TUNER_H

#include "frontend.h"
#include "databus.h"

#ifdef __cplusplus
extern "C" {
#endif

struct dibTunerMonitor;

#define TUNER_CAN_VHF   0x001
#define TUNER_CAN_UHF   0x002
#define TUNER_CAN_LBAND 0x004
#define TUNER_CAN_SBAND     0x008
#define TUNER_CAN_FAST_TUNE 0x010

#define TUNER_MAX_USE_COUNT 3

//! generic description of a Tuner
/**
 * This struct describes a tuner in generic way. Each tuner-driver has to implement one.
 */
struct dibTunerInfo {
	const char name[MAX_INFO_NAME_LENGTH]; /*! Keeps a human-readable name as a string. */
	int caps; /*! Defines the capabilities. */

	struct { /*! Generic operations each tuner has to provide */
        int (*reset) (struct dibFrontend *);
		int (*wakeup) (struct dibFrontend *); /*! Called after sleep and before each tune - to put the tuner into a defined state */
		int (*sleep) (struct dibFrontend *); /*! Put the tuner to sleep per software. Note that the power consumption is still higher than doing it by hardware. Temporary low-power-mode during runtime. */
		int (*tune_digital) (struct dibFrontend *, struct dibChannel *); /*! Tune the tuner to a channel */
		int (*tune_analog) (struct dibFrontend *, struct dibAnalogChannel *); /*! Tune the tuner in analog mode */
		int (*generic_monitoring) (struct dibFrontend *, uint32_t type, uint8_t *buf, uint32_t size, uint32_t offset); /*! Get monitoring information of the tuner. */
		int (*release) (struct dibFrontend *); /*! The tuner has to implement a function where it does his cleanup such an free private memory etc. . This is the field to fill for that. */
	} ops;

	int use_count; /*! This field is used for shared tuners not to use them in different configurations : add 2 for digital 1 for each analog (internal use only) */
    struct dibDataBusClient data_bus_client; /*! structure to get access to the registers of the tuner */

    uint32_t callback_time;
};

#define tuner_get_name(fe) fe->tuner_info->name

/* stable API for dibTunerOps - use this instead of the function pointers directly */
#define FE_TUNER_FUNC_CHECK(obj, func)  FE_FUNC_CHECK(obj, tuner_info, func)
#define FE_TUNER_FUNC_CALL(obj, func)   FE_FUNC_CALL(obj,  tuner_info, func)

extern int tuner_reset(struct dibFrontend *);
extern int tuner_init(struct dibFrontend *);
#define tuner_wakeup tuner_init

extern int tuner_sleep(struct dibFrontend *);

extern int tuner_tune(struct dibFrontend *fe, struct dibChannel *ch);

extern int tuner_tune_fast(struct dibFrontend *fe, struct dibChannel *ch);

#ifndef NO_DVBCHANNEL_BACKWARD_COMPAT
extern int tuner_tune_digital(struct dibFrontend *, struct dibDVBChannel *);
#endif

/* setting the bandwidth of a tuner is optional */
#define tuner_set_bandwidth_ex(f, c) DIB_RETURN_SUCCESS
#define tuner_set_bandwidth(f, c)    DIB_RETURN_SUCCESS

extern int tuner_tune_analog(struct dibFrontend *, struct dibAnalogChannel *);

extern int tuner_get_frequency(struct dibFrontend *);

void tuner_release(struct dibFrontend *);

extern struct dibDataBusClient * tuner_get_data_bus_client(struct dibFrontend *);
#define tuner_get_register_access(t) tuner_get_data_bus_client(t)

extern int tuner_try_inc_use_count(struct dibFrontend *, int);
extern int tuner_inc_use_count(struct dibFrontend *, int);
extern int tuner_dec_use_count(struct dibFrontend *, int);

extern int tuner_use_count(struct dibFrontend *);

#ifdef __cplusplus
}
#endif

#endif
