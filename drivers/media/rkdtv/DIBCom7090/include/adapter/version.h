#ifndef ADAPTER_VERSION_H
#define ADAPTER_VERSION_H

#include <adapter/common.h>

#include "scmref.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DIB_ADAPTER_VERSION_(maj, min, rel) ( ((maj) << 24) + ((min) << 12) + (rel) )

#define DIB_ADAPTER_VERSION DIB_ADAPTER_VERSION_(2,4,35)
#define DIB_ADAPTER_VERSION_EXTENSION ".2"

#define DIB_ADAPTER_VERSION_RELEASE ((DIB_ADAPTER_VERSION      ) & 0xfff)
#define DIB_ADAPTER_VERSION_MINOR   ((DIB_ADAPTER_VERSION >> 12) & 0xfff)
#define DIB_ADAPTER_VERSION_MAJOR   ((DIB_ADAPTER_VERSION >> 24) &  0xff)

extern const uint32_t dib_adapter_binary_version;
#define DIB_ADAPTER_BINARY_VERSION_RELEASE ((dib_adapter_binary_version      ) & 0xfff)
#define DIB_ADAPTER_BINARY_VERSION_MINOR   ((dib_adapter_binary_version >> 12) & 0xfff)
#define DIB_ADAPTER_BINARY_VERSION_MAJOR   ((dib_adapter_binary_version >> 24) &  0xff)
extern const char *DIB_ADAPTER_BINARY_VERSION_EXTENSION;
extern const char *DIB_ADAPTER_BINARY_VERSION_SCMREF;


#ifdef __cplusplus
}
#endif

#endif
