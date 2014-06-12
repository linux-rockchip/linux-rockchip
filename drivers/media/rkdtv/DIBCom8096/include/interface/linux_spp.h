#ifndef INTERFACE_LINUX_SPP_H
#define INTERFACE_LINUX_SPP_H

#include <adapter/busdescriptor.h>

#ifdef __cplusplus
extern "C" {
#endif

struct dibSPPDescriptor * linux_spp_open(void);
void linux_spp_close(struct dibSPPDescriptor *desc);

#ifdef __cplusplus
}
#endif

#endif
