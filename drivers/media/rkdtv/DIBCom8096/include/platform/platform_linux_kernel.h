#ifndef DIBTYPES_H
#define DIBTYPES_H

#include <asm/types.h>
#include <linux/version.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/delay.h>


//#include <endian.h>
#define uint32_t u32
#define  int32_t s32
#define uint16_t u16
#define  int16_t s16
#define  uint8_t u8
#define   int8_t s8
#if 0
#define uint32_t unsigned int 
#define  int32_t int
#define uint16_t unsigned short
#define  int16_t short
#define  uint8_t unsigned char
#define   int8_t char
#endif
#define MemAlloc(size)       kmalloc(size, GFP_KERNEL)
#define	MemFree(chunk, size) kfree(chunk)

#define DibMemAlloc MemAlloc
#define DibMemFree  MemFree

#define DibMoveMemory memmove
#define DibZeroMemory(pointer, size) memset(pointer, 0, size)
#define DibSetMemory  memset
// Sleep
#define DibMSleep(v) msleep(v)
#define DibUSleep(v) msleep(v)

extern uint32_t systime(void);

typedef int DIB_LOCK;

#define DibInitLock(lock)
#define DibFreeLock(lock)
#define DibAcquireLock(lock)
#define DibReleaseLock(lock)
#define DibAllocateLock(lock)
#define DibDeAllocateLock(lock)


typedef int DIB_EVENT;
#define DibAllocateEvent(event)
#define DibDeAllocateEvent(event)
#define DibInitNotificationEvent(event)
#define DibSetEvent(event)
#define DibResetEvent(a)
#define DibWaitForEvent(event, timeout)

#endif
