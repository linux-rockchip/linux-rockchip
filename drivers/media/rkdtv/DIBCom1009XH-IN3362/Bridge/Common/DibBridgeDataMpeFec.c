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
* @file "DibBridgeMpeFec.c"
* @brief MpeFec Data Handling.
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

#if (DIB_MPEFEC_DATA == 1)

#if (DIB_BRIDGE_HBM_PROFILER == 1)
#define DibBridgeHbmProfiler(pContext, idx, page, LastFrag)  pContext->BridgeChipOps.HbmProfiler(pContext, idx, page, LastFrag)
#else
#define DibBridgeHbmProfiler(pContext, idx, page, LastFrag)
#endif


/****************************************************************************
* freeing mpe Buffer
****************************************************************************/
void DibBridgeFreeMpeBuffer(struct DibBridgeContext *pContext, uint8_t Index)
{
   DibBridgeTargetFreeBuf(pContext->MpeBuffers[Index].Data, pContext->MpeBuffers[Index].Size);
   pContext->MpeBuffers[Index].Data   = NULL;
   pContext->MpeBuffers[Index].Size   = 0;
   pContext->MpeBuffers[Index].ServId = DIB_UNSET;
}

/**
 * getting a mpe Buffer from pContext->MpeBuffer pool. The mpe buffer management is:
 * - If a buffer with the required \a Size is found, take it.
 * - Else If one entry in the pool is free, allocate the buffer with the required size.
 * - Else If the Biggest unused buffer of the pool is enought to contain \a Size bytes, take it.
 * - Else Free the Smallest unsed buffer and replace the entry with an allocated buffer of size \a Size.
 * @param[in] pContext: The bridge context structure.
 * @param[in] Size: The required buffer size.
 * @param[in] Item: The Item Handler to link with the allocated buffer (for future removal of
 * all buffers of a same item).
 * @return: The Buffer address, NULL if no free space in the pool.
 */
uint8_t *IntBridgeGetMpeBuffer(struct DibBridgeContext *pContext, uint32_t Size, uint8_t Item)
{
   uint8_t  i;
   uint8_t  free_idx     = 0xFF;
   uint8_t  min_size_idx = 0xFF;
   uint8_t  max_size_idx = 0xFF;
   uint32_t max_size     = 0;
   uint32_t min_size     = 0xFFFFFFFF;
   uint8_t *pBuf         = NULL;

   DIB_ASSERT(Size != 0);

   for(i = 0; i < MAX_MPE_BUFFER; i++) 
   {
      if(pContext->MpeBuffers[i].ServId == DIB_UNSET) 
      {
         /* Found an allocated Buffer of appropriate Size */
         if(Size == pContext->MpeBuffers[i].Size) 
         {
            DIB_DEBUG(MPEBUF_LOG, (CRB "GETBUF: Reuse Buffer %d for item %d, Size %d" CRA, i, Item, Size));
            pBuf                           = pContext->MpeBuffers[i].Data;
            pContext->MpeBuffers[i].ServId = Item;
            break;
         }

         if(pContext->MpeBuffers[i].Size < min_size) 
         {
            min_size     = pContext->MpeBuffers[i].Size;
            min_size_idx = i;
         }

         if(pContext->MpeBuffers[i].Size > max_size) 
         {
            max_size     = pContext->MpeBuffers[i].Size;
            max_size_idx = i;
         }

         if((pContext->MpeBuffers[i].Data == NULL) && (pContext->MpeBuffers[i].Size == 0) && (free_idx == 0xFF)) 
         {
            /* found a free spot, as Buffer are allocated consecutively once we hit a free spot, 
            all others should be free */
            free_idx = i;
            break;
         }
      }
   }

   if(pBuf == NULL) 
   {
      /* First allocate new memory */
      if(free_idx != 0xFF) 
      {
         pBuf = DibBridgeTargetAllocBuf(Size);

         if(pBuf) 
         {
            DIB_DEBUG(MPEBUF_LOG, (CRB "GETBUF: Alloc Buffer %d for item %d, Size %d" CRA, free_idx, Item, Size));

            pContext->MpeBuffers[free_idx].Data   = pBuf;
            pContext->MpeBuffers[free_idx].Size   = Size;
            pContext->MpeBuffers[free_idx].ServId = Item;
         }
         else 
         {
            DIB_DEBUG(MPEBUF_ERR, (CRB "Failed to allocate Buffer item %d Size %d" CRA, Item, Size));
         }
      }
      else if(Size < max_size) 
      {
         /* Give out a bigger Buffer */
         DIB_DEBUG(MPEBUF_LOG, (CRB "GETBUF: Use Bigger Buffer %d for item %d, Size %d" CRA, max_size_idx, Item, pContext->MpeBuffers[max_size_idx].Size));

         pBuf                                      = pContext->MpeBuffers[max_size_idx].Data;
         pContext->MpeBuffers[max_size_idx].ServId = Item;
      }
      else if(min_size_idx != 0xFF) 
      {
         /* Free current Buffer and allocate a bigger one*/
         DibBridgeFreeMpeBuffer(pContext, min_size_idx);
         DIB_DEBUG(MPEBUF_LOG, (CRB "GETBUF: Free Buffer %d for item %d, Size= %d" CRA, min_size_idx, Item,pContext->MpeBuffers[min_size_idx].Size));

         pBuf = DibBridgeTargetAllocBuf(Size);

         if(pBuf) 
         {
            DIB_DEBUG(MPEBUF_LOG, (CRB "GETBUF: ReAlloc Buffer %d for item %d, new Size= %d" CRA, free_idx, Item, Size));

            pContext->MpeBuffers[min_size_idx].Data   = pBuf;
            pContext->MpeBuffers[min_size_idx].Size   = Size;
            pContext->MpeBuffers[min_size_idx].ServId = Item;
         }
         else 
         {
            DIB_DEBUG(MPEBUF_ERR, (CRB "Failed to allocate Buffer item %d Size %d" CRA, Item, Size));

            pContext->MpeBuffers[min_size_idx].Data   = NULL;
            pContext->MpeBuffers[min_size_idx].Size   = 0;
            pContext->MpeBuffers[min_size_idx].ServId = DIB_UNSET;
         }
      }
   }

   return pBuf;
}

/****************************************************************************
* Reuse mpe Buffer
****************************************************************************/
void IntBridgeFreeMpeBuffer(struct DibBridgeContext *pContext, uint8_t *pBuf)
{
   uint32_t i;
   DIB_ASSERT(pBuf);

   for(i = 0; i < MAX_MPE_BUFFER; i++) 
   {
      if(pContext->MpeBuffers[i].Data == pBuf) 
      {
         DIB_DEBUG(MPEBUF_LOG, (CRB "FREEBUF: Free Buffer %d for item %d" CRA, i, pContext->MpeBuffers[i].ServId));
         pContext->MpeBuffers[i].ServId = DIB_UNSET;
         break;
      }
   }
   DIB_ASSERT(i < MAX_MPE_BUFFER);
}



/****************************************************************************
* freeing any unsued mpe Buffer
****************************************************************************/
void DibBridgeFreeUnusedMpeBuffer(struct DibBridgeContext *pContext)
{
#if (DIB_FREE_UNUSED_MPE_BUF == 1)
   uint32_t i = 0; 

   for(i = 0; i < MAX_MPE_BUFFER; i++) 
   {
      if((pContext->MpeBuffers[i].Data != NULL) && (pContext->MpeBuffers[i].ServId == DIB_UNSET))
         DibBridgeFreeMpeBuffer(pContext, i);
   }
#endif
}

/* ------------------------------------------------------------------------ */
void DibBridgeMpeDataDone(struct DibBridgeContext *pContext, struct DibBridgeDmaCtx * pDmaCtx)
{
   uint8_t  Type,  i;
   ELEM_HDL idx     = DIB_UNSET;
   uint8_t *pBuf[2] = {0, 0};
   uint32_t Size[2] = {0, 0};

   idx  = pDmaCtx->DmaFlags.ItemHdl;
   Type = pDmaCtx->DmaFlags.Type;

   DIB_ASSERT(idx < DIB_MAX_NB_ITEMS);
   DIB_ASSERT((Type >= FORMAT_MPE) && (Type <= FORMAT_SLICE_DONE));

   /* Just do the DMA for now */
   if(Type == FORMAT_FRG) 
   {
      pContext->ItSvc[idx].MpeFrag += pDmaCtx->DmaLen;
   }
   /* Frame is finished, act as appropriate! */
   else 
   {
      /* check if the completion is for the currently received Data or pending correction */
      if(pContext->ItSvc[idx].MpeBufCor) 
      {
         if((Type == FORMAT_MPE_CORR) || (Type == FORMAT_MPE))
         {
            if(Type == FORMAT_MPE)
               pContext->ItSvc[idx].MpeRxState = BUF_COMPLETE;

            pBuf[0] = pContext->ItSvc[idx].MpeBufCor;
            Size[0] = pDmaCtx->DmaLen;

            /* correction done */
            pContext->ItSvc[idx].MpeBufCor = NULL;
            DIB_DEBUG(MPEBUF_LOG, (CRB "DONE: Send Correction Buf for item %d" CRA, idx));

            /* second Buffer completed */
            if(pContext->ItSvc[idx].MpeRxState == BUF_COMPLETE) 
            {
               pBuf[1] = pContext->ItSvc[idx].MpeBufRx;
               Size[1] = pContext->ItSvc[idx].DataLenRx;

               pContext->ItSvc[idx].MpeBufRx   = NULL;
               pContext->ItSvc[idx].MpeRxState = BUF_RDY;
               DIB_DEBUG(MPEBUF_LOG, (CRB "DONE: Send Pending RX Buf for item %d" CRA, idx));
            }
         }
         else /* FORMAT_LAST_FRAG */
         {
            /* Flag Buffer as complete but do not overwrite */
            pContext->ItSvc[idx].MpeRxState = BUF_COMPLETE;
         }
      }
      else if(pContext->ItSvc[idx].MpeBufRx) 
      {
         pBuf[0] = pContext->ItSvc[idx].MpeBufRx;
         Size[0] = pContext->ItSvc[idx].DataLenRx;

         pContext->ItSvc[idx].MpeBufRx   = NULL;
         pContext->ItSvc[idx].MpeRxState = BUF_RDY;

         DIB_DEBUG(MPEBUF_LOG, (CRB "DONE: Send RX Buf for item %d" CRA, idx));
      }

      for(i = 0; i < 2; i++) 
      {
         if(pBuf[i]) 
         {
            DibBridgeForwardData(pContext, pBuf[i], Size[i], idx);

            /* Return Buffer to pool */
            IntBridgeFreeMpeBuffer(pContext, pBuf[i]);
         }
      }
   }
}

/**
 * Flush MPE of a service if in prefetch mode
 * @param ItemHdl: The 
 * @param FlushCor: Flush correction buffer
 * @param FlushRx: Flush pending mpe buffer
 */
void DibBridgeFlushService(struct DibBridgeContext *pContext, ELEM_HDL ItemHdl, uint8_t FlushCor, uint8_t FlushRx, uint8_t Prefetch)
{
   DIB_ASSERT(pContext->ItSvc[ItemHdl].FilterParent < DIB_MAX_NB_FILTERS);
   /* Always flush correction Buffer, in case of Prefetch if we have to correct Data it will be in the rx Buffer */
   if (pContext->ItSvc[ItemHdl].MpeBufCor && FlushCor) 
   {
      IntBridgeFreeMpeBuffer(pContext, pContext->ItSvc[ItemHdl].MpeBufCor);
      pContext->ItSvc[ItemHdl].MpeBufCor   = NULL;
      DIB_DEBUG(MPEBUF_LOG, (CRB "FLUSH: Free Correction Buffer item= %d" CRA,ItemHdl));
   }

   /* Forward already prefetched Data (and ok) to the ip stack */
   if((FlushRx == 0) && (pContext->ItSvc[ItemHdl].MpeBufRx) && (pContext->ItSvc[ItemHdl].MpeRxState == BUF_COMPLETE)) 
   {
      DIB_DEBUG(MPEBUF_LOG, (CRB "FLUSH: Send PREFETCHED Buffer item= %d" CRA,ItemHdl));
      DibBridgeForwardData(pContext, pContext->ItSvc[ItemHdl].MpeBufRx, pContext->ItSvc[ItemHdl].DataLenRx, ItemHdl);

      FlushRx = 1;
   }

   if(Prefetch) 
   {
      if(FlushRx && (pContext->ItSvc[ItemHdl].MpeRxState == BUF_RECEPTION))
         FlushRx = 0;
   }

   /* do not flush reception Buffer if min is set to 1 - Prefetch specific */
   if (pContext->ItSvc[ItemHdl].MpeBufRx && FlushRx) 
   {
      DIB_DEBUG(MPEBUF_LOG, (CRB "FLUSH: Discard RX Buffer item= %d" CRA, ItemHdl));
      IntBridgeFreeMpeBuffer(pContext, pContext->ItSvc[ItemHdl].MpeBufRx);
      pContext->ItSvc[ItemHdl].MpeBufRx   = NULL;
   }

   if(FlushRx) 
      pContext->ItSvc[ItemHdl].MpeRxState = BUF_RDY;
}

/**
 * Transfert a fragment from chip memory to host memory
 * @param min: min base address in the firmware (circular buffer)
 * @param max: max base address in the firmware (circular buffer)
 * @param faddr: firwmare address where to find the sipsis
 * @param len: length of the fragment
 * @param pFlags: data flags and other indicators.
 * @return : The status of the dma transfert if occured.
 */
DIBDMA DibBridgeGetFragment(struct DibBridgeContext *pContext, uint32_t min, uint32_t max, uint32_t faddr, uint32_t len, struct DibBridgeDmaFlags * pFlags)
{
   struct DibBridgeDmaCtx *pDmaCtx = &pContext->DmaCtx;
   DIBDMA                  DmaStat = DIB_NO_DMA;

#if (DIB_BRIDGE_HBM_PROFILER == 1)
   uint32_t SavedMin = min;
#endif

   uint32_t BufSize = MAX_MPE_FRAME_SIZE; /* Start with maximum buffer size */

   memcpy(&pDmaCtx->DmaFlags, pFlags, sizeof(struct DibBridgeDmaFlags));

   /* allocate minimum required buffer size */
   if(pFlags->NbRows != 0)
   {
      /* Service table size */
      BufSize = pFlags->NbRows*256;

      /* Number of columns to store */
      if(pFlags->Type == FORMAT_MPE)
         BufSize *= 191; /* No FEC */
      else
         BufSize *= 255;
   }

   pDmaCtx->Dir         = DIBBRIDGE_DMA_READ;
   pDmaCtx->ChipAddr    = faddr;
   pDmaCtx->ChipBaseMin = min;
   pDmaCtx->ChipBaseMax = max;
   pDmaCtx->DmaLen      = len;
   pDmaCtx->pHostAddr   = 0;
   
   /* We are receiving a new frame/fragment */
   if(pFlags->FirstFrag || (pFlags->Type == FORMAT_MPE)) 
   {
      if(pContext->ItSvc[pFlags->ItemHdl].FilterParent >= DIB_MAX_NB_FILTERS)
      {
         DIB_DEBUG(MPEBUF_ERR,(CRB "MPE buffer with removed service %d" CRA,pFlags->ItemHdl));
         DibBridgeSignalBufFail(pContext, pFlags, 0);
         return DIB_NO_DMA;
      }
      /* Check if there is already a complete Buffer waiting to be corrected on the service
         allow double buffering of Data on this service */
      if(pContext->ItSvc[pFlags->ItemHdl].MpeRxState == BUF_CORR_PENDING) 
      {
         /* There is space to store the current Buffer correction */
         if(pContext->ItSvc[pFlags->ItemHdl].MpeBufCor == NULL) 
         {
            DibBridgeSetMpeBufCor(pContext, pFlags->ItemHdl);

            DIB_DEBUG(MPEBUF_LOG, (CRB "FRAG: Move Rx to Cor item= %d" CRA,pFlags->ItemHdl));
         }
         else 
         {
            union DibEventConfig EventConfig;
            EventConfig.DataLostEvent.ItemHdl = pFlags->ItemHdl;
            DIB_DEBUG(MAILBOX_ERR, (CRB "Cannot bufferise service %d (FirstFrag=%d)" CRA, pFlags->ItemHdl,pFlags->FirstFrag));

            /* signal to firmware we cannot bufferise Data */
            DibBridgeSignalBufFail(pContext, pFlags, 0);
            DibB2DFwdEvents(pContext, eDATA_LOST, &EventConfig);

            return DIB_NO_DMA;
         }
      }

      /* Get Buffer to store Data */
      if(pContext->ItSvc[pFlags->ItemHdl].MpeBufRx == NULL) 
      {
         DIB_DEBUG(MPEBUF_LOG, (CRB "Initializing service %d" CRA, pFlags->ItemHdl));

         pContext->ItSvc[pFlags->ItemHdl].MpeBufRx = IntBridgeGetMpeBuffer(pContext, BufSize, pFlags->ItemHdl);
         /* ((pFlags->Type == FORMAT_MPE) ? MAX_MPE_NO_FEC_SIZE : ((pFlags->NbRows == 0) ? MAX_MPE_FRAME_SIZE : ((pFlags->NbRows*256)*256))), pFlags->ItemHdl); */

         if(pContext->ItSvc[pFlags->ItemHdl].MpeBufRx == NULL) 
         {
            union DibEventConfig EventConfig;
            EventConfig.DataLostEvent.ItemHdl = pFlags->ItemHdl;
            DIB_DEBUG(MAILBOX_ERR, (CRB "Could not allocate reception Buffer for service %d" CRA, pFlags->ItemHdl));

            /* signal to firmware we cannot bufferise Data */
            DibBridgeSignalBufFail(pContext, pFlags, 0);
            DibB2DFwdEvents(pContext, eDATA_LOST, &EventConfig);

            return DIB_NO_DMA;
         }

         pContext->ItSvc[pFlags->ItemHdl].MpeRxState = BUF_RECEPTION;
      }
      else 
      {
         union DibEventConfig EventConfig;
         EventConfig.DataLostEvent.ItemHdl = pFlags->ItemHdl;
         /* Both buffers are full signal to firmware we cannot bufferise Data */
         DibBridgeSignalBufFail(pContext, pFlags, 0);
         DibB2DFwdEvents(pContext, eDATA_LOST, &EventConfig);

         DIB_DEBUG(MPEBUF_ERR,(CRB "Data Lost Event with item %d" CRA,pFlags->ItemHdl));
         return DIB_NO_DMA;
      }
   }

   /* We have already signaled overflow on service */
   if((pContext->ItSvc[pFlags->ItemHdl].MpeRxState != BUF_RECEPTION) || (pContext->ItSvc[pFlags->ItemHdl].MpeBufRx == NULL))
   {
      DIB_DEBUG(MPEBUF_ERR,(CRB "Overflow with item %d" CRA,pFlags->ItemHdl));
      DibBridgeSignalBufFail(pContext, pFlags, 0);
      return DIB_NO_DMA;
   }

   /* Last fragment of Buffer, will require correction */
   if(pFlags->LastFrag)
      pContext->ItSvc[pFlags->ItemHdl].MpeRxState = BUF_CORR_PENDING;

   /* Complete frame is always fetched to the start of the Buffer */
   if(pFlags->Type == FORMAT_MPE) 
   {
      pDmaCtx->pHostAddr = pContext->ItSvc[pFlags->ItemHdl].MpeBufRx;
      pContext->ItSvc[pFlags->ItemHdl].DataLenRx = 0;
   }
   /* Find out the destination of a fragment */
   else 
   {
      if(pFlags->FirstFrag)         /* This is the first fragment, initialize.. */
      {
         pContext->ItSvc[pFlags->ItemHdl].MpeFrag = pContext->ItSvc[pFlags->ItemHdl].MpeBufRx;
         pContext->ItSvc[pFlags->ItemHdl].DataLenRx = 0;
      }

      pDmaCtx->pHostAddr = pContext->ItSvc[pFlags->ItemHdl].MpeFrag;
   }

#if (USE_DRAGONFLY == 1)
   if((pContext->DibChip == DIB_VOYAGER) || (pContext->DibChip == DIB_NAUTILUS))
   {
      pContext->ItSvc[pFlags->ItemHdl].DataLenRx += len;
   }
   else
#endif
   {
      pContext->ItSvc[pFlags->ItemHdl].DataLenRx = len;

      /* Last fragment! Subtract the amount of already received Data. */
      if(pFlags->Type == FORMAT_LAST_FRG)
         pDmaCtx->DmaLen -= (pContext->ItSvc[pFlags->ItemHdl].MpeFrag - pContext->ItSvc[pFlags->ItemHdl].MpeBufRx);
   }

   /* We have receive the entire Buffer and it requires no correction, but if the service is in Prefetch it will not
      forwarded to the IP stack, but it needs to be tag as ready. Prefetch is only signaled on complete Buffer. */
   if(pFlags->Prefetch)
      pContext->ItSvc[pFlags->ItemHdl].MpeRxState = BUF_COMPLETE;

   /** For the last fragment, min and max MUST be set to zero, otherwise it doesn't work. */
   if((pFlags->Type == FORMAT_LAST_FRG) || pFlags->LastFrag) 
      pDmaCtx->ChipBaseMax = pDmaCtx->ChipBaseMin = 0;

   if(pDmaCtx->DmaLen > 0)
   {
      /* Graceful recovery in case of overflow */
      if((uint32_t) ((pDmaCtx->pHostAddr+pDmaCtx->DmaLen) - pContext->ItSvc[pFlags->ItemHdl].MpeBufRx) > BufSize) 
      {
         union DibEventConfig EventConfig;
         EventConfig.DataLostEvent.ItemHdl = pFlags->ItemHdl;

         DIB_DEBUG(MAILBOX_ERR, (CRB "" CRA));
         DIB_DEBUG(MAILBOX_ERR, (CRB "Buffer data overflow, was not resetted properly or service contains more data than specified (%d)" CRA, pFlags->ItemHdl));

         /* signal to firmware we cannot bufferise Data */
         DibBridgeSignalBufFail(pContext, pFlags, 0);
         DibB2DFwdEvents(pContext, eDATA_LOST, &EventConfig);

         DmaStat = DIB_NO_DMA;
      }
      else
      {
         DibBridgeSetupDma(pContext, pDmaCtx);

         DmaStat = DibBridgeRequestDma(pContext, pDmaCtx);
      }
   }
   else
   {
      DmaStat = DIB_DMA_DONE;
   }

#if (DIB_BRIDGE_HBM_PROFILER == 1)
   /* HBM Profiler */
   DibBridgeHbmProfiler(pContext, pFlags->ItemHdl, (faddr-SavedMin)/4096, (pFlags->Type == FORMAT_LAST_FRG) || pFlags->LastFrag);
#endif

#if (TEST_TRANSFERT == 1)
   DibBridgeTestDmaTransfert(pContext, faddr, (uint32_t *)pDmaCtx->pHostAddr, pDmaCtx->DmaLen);
#endif
   return DmaStat;
}

void DibBridgeInitMpeFecCtx(struct DibBridgeContext *pContext)
{
   uint32_t i;
   for(i = 0; i < DIB_MAX_NB_ITEMS; i++) 
   {
      pContext->ItSvc[i].MpeBufRx        = NULL;
      pContext->ItSvc[i].MpeBufCor       = NULL;
      pContext->ItSvc[i].MpeRxState      = BUF_RDY;
      pContext->ItSvc[i].BufCorLen       = 0;
      pContext->ItSvc[i].FilterParent    = DIB_UNSET;
      pContext->ItSvc[i].SliceLen        = 0;
   }
}

void DibBridgeInitMpeFecBuffers(struct DibBridgeContext *pContext)
{
   uint32_t i;
   for(i = 0; i < MAX_MPE_BUFFER; i++) 
   {
      pContext->MpeBuffers[i].Data    = NULL;
      pContext->MpeBuffers[i].ServId  = DIB_UNSET;
      pContext->MpeBuffers[i].Size    = 0;
   }
}


void DibBridgeFreeMpeFecBuffers(struct DibBridgeContext *pContext)
{
   uint8_t i;
   for(i = 0; i < MAX_MPE_BUFFER; i++) 
   {
      if(pContext->MpeBuffers[i].Data != NULL)
         DibBridgeFreeMpeBuffer(pContext, i);
   }
}
#endif
