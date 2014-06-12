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
* @file "DibBridgePes.c"
* @brief Pes data Handling.
*
***************************************************************************************************/
#include "DibBridgeConfig.h" /* Must be first include of all SDK files - Defines compilation options */
#include "DibBridgeTargetDebug.h"
#include "DibBridgeCommon.h"
#include "DibBridgeTarget.h"
#include "DibBridgeMailboxHandler.h"
#include "DibBridgePayloadCheckers.h"
#include "DibBridgeTestIf.h"
#include "DibBridge.h"
#include "DibBridgeData.h"

#if (DIB_PES_DATA == 1)

void DibBridgePesDone(struct DibBridgeContext *pContext, struct DibBridgeDmaCtx * pDmaCtx)
{
#if (DIB_CHECK_PES_DATA == 1)
    DibBridgeCheckPesData(pContext, pDmaCtx->pHostAddr, pDmaCtx);
#endif
   DibBridgeDesallocateRawBuffer(pContext, pDmaCtx->pHostAddr, pDmaCtx->DmaLen);
}

void DibBridgePcrDone(struct DibBridgeContext *pContext, struct DibBridgeDmaCtx * pDmaCtx)
{
#if (DIB_CHECK_PCR_DATA == 1)
    DibBridgeCheckPcrData(pContext, pDmaCtx->pHostAddr, pDmaCtx->DmaLen);
#endif
   DibBridgeDesallocateRawBuffer(pContext, pDmaCtx->pHostAddr, pDmaCtx->DmaLen);
}


#endif
