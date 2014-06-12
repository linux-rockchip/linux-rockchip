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
* @brief Driver User to Kernel Bridge Target functionality.
*
***************************************************************************************************/
#ifndef DIB_DRIVER_TARGET_H
#define DIB_DRIVER_TARGET_H

#include "DibDriverConfig.h" /* Must be first include of all SDK files - Defines compilation options */
#include "DibBridgeTargetCommon.h"

/* Setup Irq Polarity */
#define IRQ_POLARITY    IRQ_ACTIVE_HIGH

/* Private SIOC flags for IOCTL */
#define SIOC_ENABLE_HS          1
#define SIOC_ENABLE_IRQ         2

/* Specify we have Specfic Target Context */
#define DIB_DRIVER_TARGET_CTX   1
struct DibDriverTargetCtx
{
   struct DibBridgeContext *pBridgeContext;
};

#endif
