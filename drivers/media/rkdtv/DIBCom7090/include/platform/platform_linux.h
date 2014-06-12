#ifndef DIBTYPES_H
#define DIBTYPES_H

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <stdint.h>
#include <endian.h>

#include <byteswap.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MemAlloc	malloc
#define	MemFree( chunk, size )	free(chunk)

#ifndef DibMemAlloc
#define DibMemAlloc MemAlloc
#define DibMemFree  MemFree
#define DibSetMemory memset
#endif

#define DibMoveMemory memmove
#define DibZeroMemory(a, n) memset(a, 0, n)
// Sleep

#if 1
#define DibMSleep(v) usleep((v)*1000)
#define DibUSleep(v) usleep(v)
#else

extern void busy_wait(uint32_t usec);
#define DibMSleep(v) busy_wait(v*1000)
#define DibUSleep(v) busy_wait(v)
#endif

#define DIB_LOCK pthread_mutex_t

#define DibInitLock(lock)    pthread_mutex_init(lock, NULL)
#define DibFreeLock(lock)    pthread_mutex_destroy(lock)
#define DibAcquireLock(lock) pthread_mutex_lock(lock)
#define DibReleaseLock(lock) pthread_mutex_unlock(lock)
#define DibAllocateLock(lock)
#define DibDeAllocateLock(lock)

extern uint32_t systime(void);

#ifndef CONFIG_DEBUG
 #define DEBUG_FUNCTIONS_PROTOTYPED_BY_PLATFORM
 #define debug_printf_line(first, args...)
 #define debug_printf(args...)
 #define DEBUG_OBJECT(name, type, text)
 #define DEBUG_OBJECT_STATIC(name, type, text)
#endif

#ifdef DIBCOM_TESTING
struct dibI2CAccess;
extern void debug_i2c_write(struct dibI2CAccess *msg);
#endif

#ifdef __cplusplus
}
#endif

#endif
