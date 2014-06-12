#ifndef DIBTYPES_H
#define DIBTYPES_H

#include <Windows.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdarg.h>

/*#if DBG
static __inline void DibDbgPrint(   const char *format,... )
{
    va_list marker;
    char buffer[1024];

    va_start( marker, format );     /* Initialize variable arguments. */

/*    _vsnprintf(buffer, 1024, format, marker);
    OutputDebugString(buffer);

    va_end(marker);
}
#else
#define DibDbgPrint
#endif
*/


typedef double REAL;
typedef UCHAR uint8_t;
typedef ULONG uint32_t;
typedef LONG int32_t;
typedef  USHORT uint16_t;
typedef   SHORT int16_t;
//typedef  UINT uint32_t;
//typedef   INT  int32_t;
typedef char int8_t;

#define DIB_EVENT void

#define DibMoveMemory(dest,src,size) memcpy(dest,src,size)
#define DibZeroMemory(addr,size)     memset(addr,0,size)

//#define	DibMSleep	Sleep
extern void DibUSleep(uint32_t);
extern void DibMSleep(uint32_t);
extern uint32_t systime(void);
#define MemAlloc(size)     malloc(size)
#define MemFree(addr,size) free(addr)

static __inline void DibSetMemory( void* addr, UCHAR fill, size_t size) {
    FillMemory(addr, size, fill);
}
typedef	HANDLE DIB_LOCK;

static __inline void DibAcquireLock( DIB_LOCK * lock ) {
	WaitForSingleObject(*lock, INFINITE);
}

static __inline void DibReleaseLock( DIB_LOCK * lock ) {
	ReleaseMutex(*lock);
}

static __inline void DibInitLock( DIB_LOCK * lock ) {
    *lock = CreateMutex(NULL, FALSE, NULL);
}

static __inline void DibFreeLock( DIB_LOCK * lock ) {
	CloseHandle(*lock);
}

#define DibAllocateLock(lock)
#define DibDeAllocateLock(lock)

extern uint8_t DibWaitForEvent(DIB_EVENT *event, int32_t timeout_milli);

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
