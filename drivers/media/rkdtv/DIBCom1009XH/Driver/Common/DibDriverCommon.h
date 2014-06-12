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
* @file "DibDriverCommon.h"
* @brief Gpio related structures.
*
***************************************************************************************************/
#ifndef DIB_DRIVER_COMMON_H
#define DIB_DRIVER_COMMON_H

#include "DibDriverConfig.h" /* Must be first include of all SDK files - Defines compilation options */
#include "DibDriverConstants.h"
#include "DibDriverTargetDebug.h"
#include "DibDriverTargetTypes.h"
#include "DibDriverIf.h"               /* This will include demod and context structures */
#include "DibDriverConfig.h"

/*************************************************************/
/*** Access Type                                          ****/
/*************************************************************/
/* Dma Access Types */
#define DIBDRIVER_DMA_WRITE         0
#define DIBDRIVER_DMA_READ          1


/*************************************************************/
/*** Debug Mode                                           ****/
/*************************************************************/
#ifndef DIB_BRIDGE_COMMON_H
/* Optimised Mode Selected */
#if (DEBUG_MODE == NO_DEBUG)

/* No logs */
#define DIB_DEBUG(_MASK_, _MSG_)

/* Remove Asserts */
#ifdef DIB_ASSERT
#undef DIB_ASSERT
#define DIB_ASSERT(foo)
#endif

#else
/* Debug Verbose Selected */
#define DIB_DEBUG(_MASK_, _MSG_) if(pContext->DriverDebugMask & _MASK_) { DibDriverTargetLogFunction _MSG_ ; }
/* Leave assert Implementation to Target */
#endif
#endif

#endif
