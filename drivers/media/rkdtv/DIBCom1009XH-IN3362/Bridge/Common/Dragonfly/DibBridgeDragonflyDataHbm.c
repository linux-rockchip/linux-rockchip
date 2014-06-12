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
* @file "DibBridgeDragonflyDataHbm.c"
* @brief Dragonfly sprecific bridge functionality.
*
***************************************************************************************************/
#include "DibBridgeConfig.h" /* Must be first include of all SDK files - Defines compilation options */

#if (USE_DRAGONFLY == 1)

#include "DibBridgeTargetDebug.h"
#include "DibBridgeCommon.h"
#include "DibBridgeTarget.h"
#include "DibBridgeMailboxHandler.h"
#include "DibBridgeTestIf.h"
#include "DibBridge.h"
#include "DibBridgeDragonflyRegisters.h"
#include "DibBridgeDragonflyTest.h"
#include "DibBridgeDragonfly.h"
#include "DibBridgeDragonflyData.h"

#if (DIB_HBM_DATA == 1)

uint32_t DibBridgeDragonflyAssembleSlice(struct DibBridgeContext *pContext, struct DibBridgeDmaFlags *pFlags, uint32_t offs, uint32_t len)
{
   uint8_t *Src, *Dst;
   uint32_t  i;
   ELEM_HDL ItemHdl = pFlags->ItemHdl;

   Src         = pContext->ItSvc[ItemHdl].MpeBufCor + offs;
   Dst         = pContext->SliceBuf;

   if(pContext->ItSvc[pFlags->ItemHdl].SkipR == 0)
   {
      pContext->ItSvc[pFlags->ItemHdl].SkipR = pFlags->NbRows << 8;                                                                  /* NbSrcRows */
      pContext->ItSvc[pFlags->ItemHdl].SkipC = (uint8_t) (pContext->ItSvc[pFlags->ItemHdl].BufCorLen / pContext->ItSvc[pFlags->ItemHdl].SkipR);  /* NbSrcCols */
      DIB_DEBUG(RAWTS_LOG, (CRB "FIRST_ASS: Size=%d SkipR=%d SkipC=%d" CRA, pContext->ItSvc[pFlags->ItemHdl].BufCorLen, pContext->ItSvc[pFlags->ItemHdl].SkipR, pContext->ItSvc[pFlags->ItemHdl].SkipC));
   }

   DIB_DEBUG(RAWTS_LOG, (CRB "ASS : i %d type %d tbl %d tbl %d off %d src %p dst %p SrcRows %d SrcCols %d" CRA,
                         pFlags->ItemHdl, pFlags->Type, pFlags->BlockId, pFlags->FrameId, offs, Src, Dst, pContext->ItSvc[ItemHdl].SkipR, pContext->ItSvc[ItemHdl].SkipC));

   /* Build slice Buffer with faulty Data */
   i = 0;

   while(i < pContext->ItSvc[ItemHdl].SkipC) 
   {
      /* Payload always exists */
      DibMoveMemory(Dst, Src, len);
      Src += pContext->ItSvc[ItemHdl].SkipR;
      Dst += len;
      i++;
   }

   /*DisplaySliceBuf(pContext->ItSvc[ItemHdl].MpeBufCor, pContext->ItSvc[ItemHdl].SkipR, pContext->ItSvc[ItemHdl].SkipC);*/
   /*DisplaySliceBuf(pContext->SliceBuf, 32, 7);*/

   /* return the exact number of columns to transfer (compressed mode) */
   return pContext->ItSvc[ItemHdl].SkipC;
}

#endif

#endif /* USE_DRAGONFLY */
