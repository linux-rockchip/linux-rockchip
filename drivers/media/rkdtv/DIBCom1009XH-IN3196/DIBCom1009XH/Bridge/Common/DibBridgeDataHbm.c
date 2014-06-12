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
* @file "DibBridgeHbm.c"
* @brief Hbm Data Handling.
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

#if ((DIB_MPEFEC_DATA == 1) || (DIB_MPEIFEC_DATA == 1))

/****************************************************************************
 * Switch reception Buffer to correction Buffer
 ****************************************************************************/
void DibBridgeSetMpeBufCor(struct DibBridgeContext *pContext, uint8_t ItemHdl)
{
  pContext->ItSvc[ItemHdl].MpeBufCor  = pContext->ItSvc[ItemHdl].MpeBufRx;
  pContext->ItSvc[ItemHdl].MpeBufRx   = NULL;
  pContext->ItSvc[ItemHdl].MpeRxState = BUF_RDY;
  pContext->ItSvc[ItemHdl].BufCorLen  = pContext->ItSvc[ItemHdl].DataLenRx;
  pContext->ItSvc[ItemHdl].SkipR      = 0;
}

/* ------------------------------------------------------------------------ */
DIBSTATUS DibBridgeSliceDone(struct DibBridgeContext *pContext, struct DibBridgeDmaCtx * pDmaCtx)
{
   uint8_t *src, *dst;
   uint16_t offs;
   uint16_t SkipR;
   uint8_t  ItemHdl, i;
   uint32_t len;
   DIBSTATUS ret = DIBSTATUS_SUCCESS;

   ItemHdl = pDmaCtx->DmaFlags.ItemHdl;

   DIB_ASSERT(ItemHdl < DIB_MAX_NB_ITEMS);
   DIB_ASSERT(pContext->ItSvc[ItemHdl].FilterParent != DIB_UNSET);
   DIB_ASSERT(pContext->SliceBuf);

   /* service was flushed */
   if(pContext->ItSvc[ItemHdl].MpeBufCor == NULL)
   {
      DIB_DEBUG(MPEBUF_ERR, (CRB "Get Slice Failed - no associated buffer. " CRA));
      return DIBSTATUS_ERROR;
   }

   offs  = pContext->ItSvc[ItemHdl].FecOffset;
   src   = pContext->SliceBuf;

   /* Slice Len stored in all cases - only get data size - RS is not needed */
   len   = pContext->ItSvc[ItemHdl].SliceLen;
   dst   = pContext->ItSvc[ItemHdl].MpeBufCor + offs;
   SkipR = pContext->ItSvc[ItemHdl].SkipR;

   /* Acknowledge the RISC the retrieval */
   if(pContext->DibChip == DIB_FIREFLY)
      ret = DibBridgeSendAck(pContext, &pDmaCtx->DmaFlags, 0);

   /* Copy corrected Data back to Buffer */
   i = pContext->ItSvc[ItemHdl].SkipC;
   
   while(i) 
   {
      DibMoveMemory(dst, src, len);
      src += len;
      dst += SkipR;
      i--;
   }

   return ret;
}

/****************************************************************************
 * Grabs received errorneous frame and makes a slice out of it
 * to be sent for FEC treatment
 *
 * @param[in] pContext bridge context structure
 * @param[in] ItemHdl  item enumeration
 * @param[in] offs     line Offset from beginning. Max is 1024 lines.
 * @param[in] len      number of lines per slice. Max is 32*256=8Ko
 * @param[in] faddr    target address inside the RISC
 * @param[in] index    index of the concerned table
 * @return DIBDMA
 *
 ****************************************************************************/
DIBDMA DibBridgeAssembleSlice(struct DibBridgeContext *pContext, uint32_t offs, uint32_t len, uint32_t faddr, uint32_t index, struct DibBridgeDmaFlags * pFlags)
{
   struct DibBridgeDmaCtx * pDmaCtx = &pContext->DmaCtx;
   uint8_t ItemHdl = pFlags->ItemHdl;
   uint32_t NbCols;

   DIB_ASSERT(ItemHdl < DIB_MAX_NB_ITEMS);
   DIB_ASSERT(offs < 1024);
   DIB_ASSERT(len <= HBM_SLICE_LINES);

   /* Messages are process in order, so when Buffer slice is requested previous correction is done */
   if(pContext->ItSvc[ItemHdl].MpeBufCor == NULL) 
   {
      if((index != 0) && (pContext->DibChip != DIB_FIREFLY))
      {
         pContext->ItSvc[ItemHdl].SkipR      = 0;
      }
      else
      {
         DibBridgeSetMpeBufCor(pContext, ItemHdl);
         DIB_DEBUG(MPEBUF_LOG, (CRB "SLICE: RX to Correction Svc= %d" CRA, ItemHdl));
      }
   }

   /* Coherency check */
   if((index != 0) && (pContext->DibChip != DIB_FIREFLY))
   {
      if((offs == 0) && (pContext->ItSvc[ItemHdl].FecOffset))
      {
         DibBridgeSendAck(pContext, &pDmaCtx->DmaFlags, 1);

         DIB_DEBUG(MPEBUF_ERR, (CRB "Correction desynch on Table %d" CRA, index-1));
         return DIB_NO_DMA;
      }
   }

   /** Fill now the DmaFlags of the DmaCtx, cause it is needed to send the ack (if required) */
   memcpy(&pDmaCtx->DmaFlags, pFlags, sizeof(struct DibBridgeDmaFlags));

   /* fail check */
   if(pContext->ItSvc[ItemHdl].MpeBufCor == NULL) 
   {
      DibBridgeSendAck(pContext, &pDmaCtx->DmaFlags, 1);

      DIB_DEBUG(MPEBUF_ERR, (CRB "No FEC Buffer avalaible (item= %d, index %d)" CRA, ItemHdl, index));
      return DIB_NO_DMA;
   }

   /** Assemble the slice from MpeBufCor to SliceBuf, and set SkipR and SKipC if not already set */
   NbCols = DibBridgeAssembleSliceSpec(pContext, pFlags, offs, len);

   pDmaCtx->Dir         = DIBBRIDGE_DMA_WRITE;
   pDmaCtx->ChipAddr    = faddr;
   pDmaCtx->ChipBaseMin = 0;
   pDmaCtx->ChipBaseMax = 0;

   pDmaCtx->DmaLen      = NbCols * len;

   pDmaCtx->pHostAddr   = pContext->SliceBuf;

   pContext->ItSvc[ItemHdl].FecOffset  = (uint16_t)offs;

   DibBridgeSetupDma(pContext, pDmaCtx);

   return DibBridgeRequestDma(pContext, pDmaCtx);
}

/****************************************************************************
 * Gets corrected slice from RISC and places it back at correct place
 * to the MPE frame Buffer
 *
 * @param[in] pContext bridge context structure
 * @param[in] ItemHdl  item enumeration
 * @param[in] offs     line Offset from beginning. Max is 1024 lines.
 * @param[in] len      number of lines per slice. Max is 32*256=8Ko
 * @param[in] faddr    target address inside the RISC
 * @param[in] index    index of the correction table   
 * @return  DIBDMA
 *
 ****************************************************************************/
DIBDMA DibBridgeGetSlice(struct DibBridgeContext *pContext, uint32_t offs, uint32_t len, uint32_t faddr, uint32_t index, struct DibBridgeDmaFlags *pFlags)
{
   struct DibBridgeDmaCtx * pDmaCtx = &pContext->DmaCtx;

   DIB_ASSERT(pFlags->ItemHdl < DIB_MAX_NB_ITEMS);
   DIB_ASSERT(offs < 1024);
   DIB_ASSERT(len <= HBM_SLICE_LINES);

   DIB_DEBUG(MPEBUF_LOG, (CRB "Get Slice len %d addr %x item %d" CRA, 256 * len, faddr,pFlags->ItemHdl));
   memcpy(&pDmaCtx->DmaFlags, pFlags, sizeof(struct DibBridgeDmaFlags));

   pDmaCtx->Dir         = DIBBRIDGE_DMA_READ;
   pDmaCtx->ChipAddr    = faddr;
   pDmaCtx->ChipBaseMin = 0;
   pDmaCtx->ChipBaseMax = 0;

   /* Get only corrected data size - RS is not needed. */
   pDmaCtx->DmaLen = index*len;

   if(pContext->ItSvc[pFlags->ItemHdl].SliceLen == 0)
      pContext->ItSvc[pFlags->ItemHdl].SliceLen = len;

   pDmaCtx->pHostAddr   = pContext->SliceBuf;

   DibBridgeSetupDma(pContext, pDmaCtx);

   return DibBridgeRequestDma(pContext, pDmaCtx);
}


#endif
