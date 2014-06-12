/****************************************************************************
 *
 *      Copyright (c) DiBcom SA.  All rights reserved.
 *
 *      THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 *      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 *      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 *      PURPOSE.
 *
 ****************************************************************************/

/**************************************************************************************************
* @file "DibDriverTargetTypes.h"
* @brief Linux User to Kernel Bridge Target types.
*
***************************************************************************************************/
#ifndef DIB_DRIVER_TARGET_TYPES_H
#define DIB_DRIVER_TARGET_TYPES_H

#include "DibDriverConfig.h" /* Must be first include of all SDK files - Defines compilation options */

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 600
#ifndef __USE_XOPEN2K
#define __USE_XOPEN2K
#endif
#endif

#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <errno.h>

#include "DibDriverConstants.h"

/* #define DibDbgPrint  DbgPrint */
/* #define DibErrPrint  DbgPrint */

#include "DibStatus.h"
#include "DibExtDefines.h"

typedef sem_t DIB_LOCK;

/*  Emulation of a kernel driver */
typedef DIB_LOCK DIB_MUTEX;

#define DibWaitForMutex   DibAcquireLock
#define DibReleaseMutex   DibReleaseLock
#define DibInitMutex      DibInitLock

/*  Events */
typedef sem_t DIB_EVENT;

static __inline void DibAllocateEvent(DIB_EVENT * Event)
{
}

static __inline void DibDeAllocateEvent(DIB_EVENT * Event)
{
}

static __inline void DibInitNotificationEvent(DIB_EVENT * Lock)
{
   sem_init((sem_t *) Lock, 0, 0);
}

static __inline void DibSetEvent(DIB_EVENT * Lock)
{
   sem_post((sem_t *) Lock);
}

#define DibResetEvent( a )

static __inline uint8_t DibWaitForEvent(DIB_LOCK * Lock, uint32_t timeout)
{
    struct timeval now;
    struct timespec deadline;
    int rc;

    gettimeofday(&now, NULL);

    deadline.tv_sec = now.tv_sec + timeout/1000;
    deadline.tv_nsec = (now.tv_usec * 1000) + ((timeout%1000) * 1000000);
    deadline.tv_sec += (deadline.tv_nsec / 1000000000); 
    deadline.tv_nsec %= 1000000000;
   rc = sem_timedwait((sem_t *) Lock, &deadline) ;
   if(rc != 0) {
       /* If TimeOut propagate error to API */
       rc = DIBSTATUS_TIMEOUT;
   }

   return rc;
}

/*  Locks */
static __inline void DibInitLock(DIB_LOCK * Lock)
{
   sem_init((sem_t *) Lock, 0, 1);
}

static __inline void DibReleaseLock(DIB_LOCK * Lock)
{
   sem_post((sem_t *) Lock);
}

static __inline void DibAcquireLock(DIB_LOCK * Lock)
{
   while(sem_wait((sem_t *) Lock) == -1);
}

static __inline void DibAllocateLock(DIB_LOCK * Lock)
{
}

static __inline void DibDeAllocateLock(DIB_LOCK * Lock)
{
}

#define DibMemAlloc              malloc
#define DibMemFree(chunk, Size)  free(chunk)

#define DibMoveMemory            memmove
#define DibZeroMemory            bzero
#define DibSetMemory(_chunk, _val, _size)       memset( (char *)_chunk, _val, _size)

/*  Sleep */
/**
 * DibMSleep (uint32_t ms)
 */
static __inline void DibMSleep(uint32_t timeout_ms)
{
   struct timeval tv;
   tv.tv_sec = timeout_ms / 1000;
   tv.tv_usec = (timeout_ms % 1000) * 1000;
   select(0, NULL, NULL, NULL, &tv);
}

/*  Trace Handling */
#define DibWriteRawTrace( format... )  fprintf(trace_file,format)
#define DibFlushRawTrace()             fflush( trace_file);

#define DibWriteTrace                  DbgPrint
#define DibFlushTrace()

#endif
