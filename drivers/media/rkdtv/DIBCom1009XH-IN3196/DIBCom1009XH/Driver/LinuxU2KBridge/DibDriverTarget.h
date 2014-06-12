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
* @file "DibDriverTarget.h"
* @brief Linux User to Kernel Bridge Target functionality.
*
***************************************************************************************************/
#ifndef DIB_DRIVER_TARGET_H
#define DIB_DRIVER_TARGET_H

#include "DibDriverConfig.h" /* Must be first include of all SDK files - Defines compilation options */
#include "../../Sample/LinuxKernelBridge/DibBridgeTargetModule.h"

#define HOOK_SRAM

/* Setup Irq Polarity */
#define IRQ_POLARITY    IRQ_ACTIVE_LOW 

/* Specify we have a Specfic Target Context */
#define DIB_DRIVER_TARGET_CTX    1


/* That's the target dependent part of the DIB_DEVICE structure. */
/* Other parts of the structure should be kept in ports, while this one will be changed */
/* to put target/OS dependent information in it. */

#include <pthread.h>

struct DibDriverTargetCtx
{
   DIB_LOCK    IrqLock;
   int         BridgeFd;
   pthread_t   IrqThread;
   DIB_EVENT   IrqReadDone;
};

#endif
