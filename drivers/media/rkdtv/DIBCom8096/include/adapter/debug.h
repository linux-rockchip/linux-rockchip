#ifndef ADAPTER_DEBUG_H
#define ADAPTER_DEBUG_H

#include "../platform/platform.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DEBUG_ADAPTER 0x0001
#define DEBUG_DEMOD   0x0002
#define DEBUG_TUNER   0x0004
#define DEBUG_VIDEO   0x0008
#define DEBUG_AUDIO   0x0010
#define DEBUG_ANALOG  0x0018
#define DEBUG_BUS     0x0020
#define DEBUG_SIP     0x0040
#define DEBUG_LAYOUT  0x0080
#define DEBUG_BOARD   0x0100
#define DEBUG_HAL     0x0120

#define DEBUG_MASK    0xffff

#define DEBUG_PREFIX_MAX_LEN 50
struct dibDebugObject {
	uint32_t mask;
	char     prefix[DEBUG_PREFIX_MAX_LEN];
};

#ifndef DEBUG_OBJECT
#define DEBUG_OBJECT(name, type, text) \
    const struct dibDebugObject name = { \
        type, \
        text \
    };
#endif

#ifndef DEBUG_OBJECT_STATIC
#define DEBUG_OBJECT_STATIC(name, type, text) static DEBUG_OBJECT(name, type, text)
#endif

//extern void debug_object_init(struct dibDebugObject *obj, char *prefix, uint32_t mask);

#ifndef DEBUG_FUNCTIONS_PROTOTYPED_BY_PLATFORM
/* to be implemented by the platform */

//! Print a line of debugging information
/**
 * This function is called from inside the code to ask for printing
 * one line of debugging. The first argument gives information about
 * the context the debugging message was emitted.
 * \param dbg context-information about the source of this debug-message.
 * \param fmt format string
 * \param ... arguments
 */
extern void debug_printf_line(const struct dibDebugObject *dbg, const char *fmt, ...);

//! Print some characters of debugging information
/**
 * This function is called from inside the code to ask for printing
 * some characters which not necessarily need to be a line. There is
 * not information about the context of this debugging message. It is
 * used to print related information which is spread of several lines
 * of code.
 * \param fmt format string
 * \param ... arguments
 */
extern void debug_printf(const char *fmt, ...);
#endif

#define dbgpl    debug_printf_line

#define dbgp     debug_printf

#define dbg_bufdump(b,l) do { uint32_t _i = 0; for (_i = 0; _i < (l); _i++) dbgp("%02x ",b[_i]); } while (0)


/* backward compat */
#define DibDbgPrint dbgp

#define fstart do { dbgp("++ %s - %d\n", __FUNCTION__, __LINE__ ); } while (0)
#define fend   do { dbgp("-- %s - %d\n", __FUNCTION__, __LINE__ ); } while (0)

#ifdef DIBCOM_TESTING
#define dbg_testing(x) printf("%s - %s\n", __FUNCTION__, x)
#else
#define dbg_testing(x)
#endif

#ifdef __cplusplus
}
#endif

#endif
