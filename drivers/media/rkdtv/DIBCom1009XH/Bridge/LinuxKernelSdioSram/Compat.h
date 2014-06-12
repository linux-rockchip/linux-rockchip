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
* @file "Compat.h"
* @brief .
*
***************************************************************************************************/
#ifndef _COMPAT_H_
#define _COMPAT_H_

#include <linux/version.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,14)
#define kzalloc(Size, flags)              \
({                                        \
void *__ret = kmalloc(Size, flags);       \
if (__ret)                                \
memset(__ret, 0, Size);                   \
   __ret;                                 \
})
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,13)
# define class class_simple
# define class_device_create(a, b, c, d, e, f...)  class_simple_device_add(a, c, d, e, f)
# define class_device_destroy(a, b)                class_simple_device_remove(b)
# define class_create(a, b)                        class_simple_create(a, b)
# define class_destroy(a)                          class_simple_destroy(a)
#else
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,15)
# define class_device_create(a, b, c, d, e, f...) class_device_create(a, c, d, e, f)
#endif
#endif

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,15)

#define DEFINE_MUTEX(a)             DECLARE_MUTEX(a)
#define mutex_lock_interruptible(a) down_interruptible(a)
#define mutex_unlock(a)             up(a)
#define mutex_lock(a)               down(a)
#define mutex_init(a)               init_MUTEX(a)
#define mutex_trylock(a)            down_trylock(a)
#define mutex                       semaphore
#endif

#endif
