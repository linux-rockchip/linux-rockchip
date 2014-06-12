#ifndef ADAPTER_FRONTEND_TUNE_H
#define ADAPTER_FRONTEND_TUNE_H

#include "frontend.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void frontend_tune_diversity_sync(struct dibFrontend fe[],int num, struct dibChannel *ch);
extern void frontend_tune_diversity_async(struct dibFrontend fe[], int count, struct dibChannel *ch);

#if defined(CONFIG_NO_TUNE_ASYNC) && !defined(NO_SYNC_INTERFACE)
#define tune_diversity frontend_tune_diversity_sync
#else
#define tune_diversity frontend_tune_diversity_async
#endif

#ifdef __cplusplus
}
#endif

#endif

