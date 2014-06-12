#ifndef DIBTYPES_H
#define DIBTYPES_H

#include <wdm.h>

#include <windef.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

typedef ULONG  uint32_t;
typedef  LONG   int32_t;
typedef USHORT uint16_t;
typedef  SHORT  int16_t;
typedef UCHAR   uint8_t;
typedef  CHAR    int8_t;

#define DibMoveMemory(dest, src, size) RtlMoveMemory(dest, src, size)
#define DibZeroMemory(addr, size)      RtlZeroMemory(addr, size)
#define DibSetMemory(addr, fill, size) RtlFillMemory(addr, size, fill)



extern uint32_t systime(void);


/*  Common mutex functions */
typedef	KMUTEX	DIB_LOCK;
#define DibAcquireLock(lock) KeWaitForMutexObject(lock, Executive, KernelMode, FALSE, NULL)
#define DibReleaseLock(lock) KeReleaseMutex(lock, FALSE)
#define DibFreeLock(lock)    //do { lock = NULL; } while (0)
#define DibInitLock(lock)    KeInitializeMutex(lock, 0)
#define DibAllocateLock(lock)
#define DibDeAllocateLock(lock)

/*  Comon event */
typedef	KEVENT	DIB_EVENT;
#define DibInitNotificationEvent(event) KeInitializeEvent(event, NotificationEvent, FALSE)
#define DibSetEvent(event)              KeSetEvent(event, 0, FALSE )
#define DibResetEvent(event)            KeResetEvent(event)
extern uint8_t DibWaitForEvent(DIB_EVENT *event, int32_t timeout_milli);

#define MemAlloc(size)     ExAllocatePoolWithTag(NonPagedPool, size, 'cBiD')
#define MemFree(addr,size) ExFreePool(addr)
extern void DibMSleep(uint32_t delay_ms);
#define DibUSleep(d) DibMSleep(d  / 1000 + 1)

#define DIB_CHANNEL_CHANGE enum dibChannelChange

#define VIDEO_CONFIG  struct dibVideoConfig
#define PVIDEO_CONFIG struct dibVideoConfig *
#define VIDEO_TRANSITIONS  struct dibVideoTransitions
#define PVIDEO_TRANSITIONS struct dibVideoTransitions *
#define VIDEO_FRAMING struct dibVideoFraming
#define PVIDEO_FRAMING struct dibVideoFraming *

#define DIB_AUDIO_STANDARD enum dibAudioStandard
#define DIB_AUDIO_INPUT    enum dibAudioInput

#define DIB_PICTURE_PROPERTY enum dibPictureProperty
#define DIB_VIDEO_INPUT enum dibVideoInput

#if DBG
#else
#define DEBUG_FUNCTIONS_PROTOTYPED_BY_PLATFORM

static __inline void debug_printf(const char *fmt, ...) {}
static __inline void debug_printf_line(const struct dibDebugObject *dbg, const char *fmt, ...) {}
#endif

#define __BYTE_ORDER __LITTLE_ENDIAN
static __inline uint16_t bswap_16(uint16_t v)
{
	uint16_t w=0x1;
	/* little endian */
	if (*(uint8_t *)&w == 1)
		return (((v>>8)&0xff) | ((v&0xff)<<8));
	/* big endian */
	return v;
}

static __inline uint32_t bswap_32(uint32_t v)
{
	uint16_t w=0x1;
	/* little endian */
	if (*(uint8_t *)&w == 1)
		return ((v&0xff000000)>>24) | ((v&0x00ff0000)>>8) | ((v&0x0000ff00)<<8) | ((v&0x000000ff)<<24);
	/* big endian */
	return v;
}

#endif
