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

#include "DibBridgeConfig.h" /* Must be first include of all SDK files - Defines compilation options */

#include "DibBridgeTargetDebug.h"
#include "DibBridgeCommon.h"
#include "DibBridgeTarget.h"
#include "DibBridgeMailboxHandler.h"
#include "DibBridgeTestIf.h"
#include "DibBridgeData.h"
#include "DibBridge.h"

#if (USE_FIREFLY == 1)
#include "DibBridgeFirefly.h"
#endif
#if (USE_DRAGONFLY == 1)
#include "DibBridgeDragonfly.h"
#endif

void IntBridgeAtscmhDone(struct DibBridgeContext *pContext, struct DibBridgeDmaCtx * pDmaCtx)
{
   ELEM_HDL Item  = pDmaCtx->DmaFlags.ItemHdl;
   uint8_t Type = pDmaCtx->DmaFlags.BlockType;
   static uint32_t Iteration = 0, MaxSize = 0;
   uint8_t i;
   uint8_t* chkbuf=pDmaCtx->pHostAddr;
   FILE *DataFileOut;

   DataFileOut = fopen("atscmh.dat", "ab");
   fwrite(chkbuf, sizeof(uint8_t), pDmaCtx->DmaLen, DataFileOut);
   fclose(DataFileOut);

   Iteration++;

   DibBridgeDesallocateRawBuffer(pContext, pDmaCtx->pHostAddr, pDmaCtx->DmaLen);
}
