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
* @file "DibBridgeTargetTypes.h"
* @brief Target Specific.
*
***************************************************************************************************/
#ifndef DIB_BRIDGE_TARGET_TYPES_H
#define DIB_BRIDGE_TARGET_TYPES_H

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 500
#endif

#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <sys/time.h>
#include "DibExtDefines.h"

#define LINUX_PTR_CAST   unsigned long

/*****************************************************************************/
/***                         Events                                        ***/
/*****************************************************************************/
typedef sem_t DIB_EVENT;

/**
 * DibAllocateEvent( DIB_EVENT *Event )
 */
static __inline void DibAllocateEvent(DIB_EVENT * Event)
{
}

/**
 * DibDeAllocateEvent( DIB_EVENT *Event )
 */
static __inline void DibDeAllocateEvent(DIB_EVENT * Event)
{
}

/**
 * DibInitNotificationEvent(DIB_EVENT *Event)
 */
static __inline void DibInitNotificationEvent(DIB_EVENT * Event)
{
  sem_init((sem_t *) Event, 0, 0);
}

/*
 * DibAllocateEvent( DIB_EVENT *Event )
 */
static __inline void DibSetEvent(DIB_EVENT * Event)
{
  sem_post((sem_t *) Event);
}

/**
 * DibResetEvent( DIB_EVENT *Event )
 */
#define DibResetEvent( a )

/**
 * DibWaitForEvent( DIB_LOCK *Lock, uint32_t timeout )
 */
static __inline uint8_t DibWaitForEvent(DIB_EVENT * Event, uint32_t timeout)
{
  sem_wait((sem_t *) Event);
  return 0;
}

/*****************************************************************************/
/***                         Events                                        ***/
/*****************************************************************************/

typedef sem_t DIB_LOCK;

/**
 * DibInitLock(DIB_LOCK *Lock)
 */
static __inline void DibInitLock(DIB_LOCK * Lock)
{
  sem_init((sem_t *) Lock, 0, 1);
}

/**
 * DibReleaseLock (DIB_LOCK *Lock)
 */
static __inline void DibReleaseLock(DIB_LOCK * Lock)
{
  sem_post((sem_t *) Lock);
}

/**
 * DibAcquireLock (DIB_LOCK *Lock)
 */
static __inline void DibAcquireLock(DIB_LOCK * Lock)
{
  sem_wait((sem_t *) Lock);
}

/**
 * DibAllocateLock (DIB_LOCK *Lock)
 */
static __inline void DibAllocateLock(DIB_LOCK * Lock)
{
}

/**
 * DibDeAllocateLock (DIB_LOCK *Lock)
 */
static __inline void DibDeAllocateLock(DIB_LOCK * Lock)
{
}

/*****************************************************************************/
/***                         MEMORY                                        ***/
/*****************************************************************************/
#define DibMemAlloc                             malloc
#define DibMemFree(chunk, Size)                 free(chunk)
#define DibMoveMemory                           memmove
#define DibZeroMemory                           bzero
#define DibSetMemory(_chunk, _val, _size)       memset( (char *)_chunk, _val, _size)

/*****************************************************************************/
/***                         DELAY                                         ***/
/*****************************************************************************/
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

#endif
