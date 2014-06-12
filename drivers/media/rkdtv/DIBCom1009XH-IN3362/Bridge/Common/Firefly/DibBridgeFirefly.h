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
* @file "DibBridgeFirefly.h"
* @brief Firefly specific bridge functionnality.
*
***************************************************************************************************/
#ifndef DIB_BRIDGE_FIREFLY_H
#define DIB_BRIDGE_FIREFLY_H

#include "DibBridgeConfig.h" /* Must be first include of all SDK files - Defines compilation options */
#include "DibBridge.h"

extern void DibBridgeFireflyRegisterIf(struct DibBridgeContext *pContext);
DIBSTATUS DibBridgeFireflyGetService(struct DibBridgeContext *pContext, uint8_t ItemHdl, uint8_t *pSvcNb);
DIBSTATUS DibBridgeFireflySendMsg(struct DibBridgeContext *pContext, uint32_t * short_data, uint32_t len);

#endif
