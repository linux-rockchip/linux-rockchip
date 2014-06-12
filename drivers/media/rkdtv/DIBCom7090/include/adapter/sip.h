#ifndef ADAPTER_SIP_H
#define ADAPTER_SIP_H

#include "frontend.h"

#ifdef __cplusplus
extern "C" {
#endif

/* now a SIP contains some extra potion added to a frontend (using its components to store the component) */
struct dibSIPInfo {
	const char name[MAX_INFO_NAME_LENGTH];

	struct {
		void (*release) (struct dibFrontend *);
	} ops;

    void *priv;
};

#define sip_release(s) \
   do { if ((s) != NULL && (s)->sip != NULL && (s)->sip->ops.release != NULL) (s)->sip->ops.release(s); } while (0)

#ifdef __cplusplus
}
#endif

#endif
