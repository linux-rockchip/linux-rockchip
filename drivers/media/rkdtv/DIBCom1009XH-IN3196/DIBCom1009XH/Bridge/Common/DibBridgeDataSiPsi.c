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
* @file "DibBridgeSiPSi.c"
* @brief SiPsi Data Handling.
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

#if (DIB_SIPSI_DATA == 1)

/* ------------------------------------------------------------------------ */
void DibBridgeSiPsiDone(struct DibBridgeContext *pContext, struct DibBridgeDmaCtx * pDmaCtx)
{
   DIB_ASSERT(pDmaCtx->DmaLen > 0);
   DIB_ASSERT(pDmaCtx->pHostAddr);

   DibB2DFwdSiPsiBuf(pContext, pDmaCtx->DmaLen, pDmaCtx->pHostAddr);
}

/**
 * Transfert Sipsi request from chip memory to host memory
 * @param min: min base address in the firmware (circular buffer)
 * @param max: max base address in the firmware (circular buffer)
 * @param faddr: firwmare address where to find the sipsis
 * @return : The status of the dma transfert if occured.
 */
DIBDMA DibBridgeGetSipsi(struct DibBridgeContext *pContext, uint32_t min, uint32_t max, uint32_t faddr, uint32_t len, struct DibBridgeDmaFlags * pFlags)
{
   struct DibBridgeDmaCtx * pDmaCtx = &pContext->DmaCtx;

   memcpy(&pDmaCtx->DmaFlags, pFlags, sizeof(struct DibBridgeDmaFlags));

   pDmaCtx->Dir         = DIBBRIDGE_DMA_READ;
   pDmaCtx->ChipAddr    = faddr;
   pDmaCtx->ChipBaseMin = min;
   pDmaCtx->ChipBaseMax = max;
   pDmaCtx->DmaLen      = len;
   pDmaCtx->pHostAddr   = pContext->SiPsiBuf;

   DibBridgeSetupDma(pContext, pDmaCtx);

   return DibBridgeRequestDma(pContext, pDmaCtx);
}

#endif
