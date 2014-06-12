#ifndef INTERFACE_LINUX_I2C_H
#define INTERFACE_LINUX_I2C_H

#include "../adapter/busdescriptor.h"

#ifdef __cplusplus
extern "C" {
#endif

struct dibDataBusHost * host_i2c_interface_attach(void *priv);
void host_i2c_release(struct dibDataBusHost *);


struct dibDataBusHost* rkdroid_open_i2c(void);
void rkdroid_close_i2c(void);

#ifdef __cplusplus
}
#endif

#endif
