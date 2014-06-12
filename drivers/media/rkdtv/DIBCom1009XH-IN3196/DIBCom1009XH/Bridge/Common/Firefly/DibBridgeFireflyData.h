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
* @file "DibBridgeFireflyData.h"
* @brief Firefly specific bridge functionnality.
*
***************************************************************************************************/
#ifndef DIB_BRIDGE_FIREFLY_DATA_H
#define DIB_BRIDGE_FIREFLY_DATA_H

#include "DibBridgeConfig.h" /* Must be first include of all SDK files - Defines compilation options */
#include "DibBridge.h"

#if (DIB_MPEFEC_DATA == 1)
void      DibBridgeFireflyGetCpt(struct DibBridgeContext *pContext, uint16_t * pDataMsg);
uint32_t  DibBridgeFireflyAssembleSlice(struct DibBridgeContext *pContext, struct DibBridgeDmaFlags *pFlags, uint32_t offs, uint32_t len);
DIBSTATUS DibBridgeFireflyHbmProfiler(struct DibBridgeContext *pContext, uint8_t idx, uint8_t page, uint8_t LastFrag);
DIBSTATUS DibBridgeFireflySendAck(struct DibBridgeContext *pContext, struct DibBridgeDmaFlags *pFlags, uint8_t failed);
#else
#define   DibBridgeFireflyGetCpt(pContext, pDataMsg)
#define   DibBridgeFireflyHbmProfiler(pContext, idx, page, LastFrag)   DIBSTATUS_SUCCESS
uint32_t  DibBridgeFireflyAssembleSlice(struct DibBridgeContext *pContext, struct DibBridgeDmaFlags *pFlags, uint32_t offs, uint32_t len)   {return 0;}
DIBSTATUS DibBridgeFireflySendAck(struct DibBridgeContext *pContext, struct DibBridgeDmaFlags *pFlags, uint8_t failed) { return DIBSTATUS_SUCCESS;}
#endif

#endif
