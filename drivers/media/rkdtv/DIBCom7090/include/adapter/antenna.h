#ifndef ADAPTER_ANTENNA_H
#define ADAPTER_ANTENNA_H

#include "common.h"
#include "databus.h"

#ifdef __cplusplus
extern "C" {
#endif

struct dibAntInfo {
    const char name[MAX_INFO_NAME_LENGTH];

    struct {
        int (*reset)    (struct dibFrontend *);
        void (*release) (struct dibFrontend *);
	void (*update)	(struct dibFrontend *, uint16_t );
    } ops;

    struct dibDataBusClient data_bus_client;

    void *priv;
};

#define FE_ANT_FUNC_CHECK(obj, func)  FE_FUNC_CHECK(obj, antenna_info, func)
#define FE_ANT_FUNC_CALL(obj, func)   FE_FUNC_CALL(obj,  antenna_info, func)

extern struct dibDataBusClient * antenna_get_data_bus_client(struct dibFrontend *);

extern int antenna_reset(struct dibFrontend *fe);
extern void antenna_release(struct dibFrontend *fe);
extern void antenna_update(struct dibFrontend *fe, uint16_t);

extern const char * antenna_get_name(struct dibFrontend *fe);

#ifdef __cplusplus
}
#endif

#endif
