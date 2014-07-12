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

#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,33)
#include <generated/autoconf.h>
#else
#include <linux/autoconf.h>
#endif
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>
//#include <linux/smp_lock.h>
#include <linux/completion.h>
#include <asm/uaccess.h>
#include <linux/usb.h>
#include <linux/kthread.h>

#include <linux/vmalloc.h>

#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/ethtool.h>

#include <linux/time.h>
#include <linux/types.h>
#include <linux/unistd.h>
#include <linux/ctype.h>
#include <asm/delay.h>

#include <linux/version.h>
#include "DibStatus.h"   	//my add


#define LINUX_PTR_CAST   unsigned long

/*****************************************************************************/
/***                         Events                                        ***/
/*****************************************************************************/
typedef struct semaphore DIB_EVENT;

/*------------------------------------------------------------
           DibAllocateEvent( DIB_EVENT *Event )
-------------------------------------------------------------*/
static __inline void DibAllocateEvent(DIB_EVENT *Event)
{

}

/*------------------------------------------------------------
           DibDeAllocateEvent( DIB_EVENT *Event )
-------------------------------------------------------------*/
static __inline void DibDeAllocateEvent(DIB_EVENT *Event)
{
	
}

/*------------------------------------------------------------
      DibInitNotificationEvent    (DIB_EVENT *Event)
-------------------------------------------------------------*/
static __inline void DibInitNotificationEvent(DIB_EVENT *Event)
{
  sema_init((struct semaphore *) Event, 0);
  //printk("*****DibInitNotificationEvent******\n");
}

/*------------------------------------------------------------
           DibAllocateEvent( DIB_EVENT *Event )
-------------------------------------------------------------*/
static __inline void DibSetEvent(DIB_EVENT *Event)
{
  up((struct semaphore *) Event);
 // printk("*****DibSetEvent******\n");
}

/*------------------------------------------------------------
           DibResetEvent( DIB_EVENT *Event )
-------------------------------------------------------------*/
#define DibResetEvent(a)

/*------------------------------------------------------------
      uint8_t DibWaitForEvent( DIB_LOCK *Lock, uint32_t timeout )
-------------------------------------------------------------*/
static __inline uint8_t DibWaitForEvent(DIB_EVENT *Event, uint32_t timeout)
{
#if 1
  //down((struct semaphore *) Event);
  int rc = down_timeout((struct semaphore *)Event, (unsigned long)msecs_to_jiffies(timeout));  
#else
  int rc = down_interruptible((struct semaphore *) Event);

  if(rc != 0){
     //printk(CRB "DibWaitForEvent failed" CRA);
    return DIBSTATUS_ERROR;
  }
#endif

  return (uint8_t)rc;
}

/*****************************************************************************/
/***                         Events                                        ***/
/*****************************************************************************/

typedef struct semaphore DIB_LOCK;

/*------------------------------------------------------------
             DibInitLock    (DIB_LOCK *Lock)
-------------------------------------------------------------*/
static __inline void DibInitLock(DIB_LOCK *Lock)
{
  sema_init((struct semaphore *) Lock, 1);
}

/*------------------------------------------------------------
             DibReleaseLock (DIB_LOCK *Lock)
-------------------------------------------------------------*/
static __inline void DibReleaseLock(DIB_LOCK *Lock)
{
  up((struct semaphore *)Lock);
}

/*------------------------------------------------------------
             DibAcquireLock (DIB_LOCK *Lock)
-------------------------------------------------------------*/
static __inline void DibAcquireLock(DIB_LOCK *Lock)
{
#if 1
  down((struct semaphore *) Lock);
#else
  int rc=down_interruptible((struct semaphore *) Lock);
  if(rc != 0){
   printk(CRB "DibAcquireLock failed" CRA);
  }
#endif
} 

/*------------------------------------------------------------
             DibAllocateLock (DIB_LOCK *Lock)
-------------------------------------------------------------*/
static __inline void DibAllocateLock(DIB_LOCK *Lock)
{
}

static __inline int32_t DibGetLockValue(DIB_LOCK *Lock)
{
#if(LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26))
    return ((struct semaphore *)Lock)->count;
#else
   return ((struct semaphore *)Lock)->count.counter;
#endif
}

/*------------------------------------------------------------
             DibDeAllocateLock (DIB_LOCK *Lock)
-------------------------------------------------------------*/
static __inline void DibDeAllocateLock(DIB_LOCK *Lock)
{
}

/*****************************************************************************/
/***                         MEMORY                                        ***/
/*****************************************************************************/
#define DibMemAlloc(Size)                       kmalloc(Size,GFP_KERNEL)
#define DibMemFree(chunk, Size)                 kfree(chunk)
#define DibMoveMemory                           memmove
#define DibZeroMemory(_chunk, _size)            memset( (char *)_chunk, 0, _size)
#define DibSetMemory(_chunk, _val, _size)       memset( (char *)_chunk, _val, _size)

/*****************************************************************************/
/***                         DELAY                                         ***/
/*****************************************************************************/
#define DibMSleep(x)       msleep(x)

#endif
