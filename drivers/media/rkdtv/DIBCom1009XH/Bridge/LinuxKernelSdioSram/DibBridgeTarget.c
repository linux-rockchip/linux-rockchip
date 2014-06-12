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
* @file "DibBridgeTarget.c"
* @brief Target Specific.
*
***************************************************************************************************/

#include "DibBridgeCommon.h"
#include "DibBridgeTargetDebug.h"
#include "DibBridgeTarget.h"
#include "DibBridgeTargetIrq.h"
#include "DibBridgeTargetMpe.h"
#include "DibBridgeTargetNet.h"
#include "DibBridge.h"

/****************************************************************************
 * IntBridgeTargetGetNbPages
 ****************************************************************************/
uint32_t IntBridgeTargetGetNbPages(uint32_t Size)
{
   uint32_t NbPages  = 0;
   uint32_t NbBlocks = (uint32_t) (Size/4096);

   while(NbBlocks > 1)
   {
      NbPages++;
      NbBlocks = NbBlocks >> 1;
   }

   NbBlocks = (uint32_t) (Size/4096);

   if(Size%4096)
      NbBlocks++;

   if(~(1 << NbPages) & NbBlocks)
      NbPages++;

   return NbPages;
}


/*-----------------------------------------------------------------------
  |  DibBridgeTargetRequestDma
  -------------------------------------------------------------------------
  |  This callback function is called by the driver when a MPE Frame is
  |  available in the chip memory and has to be read by the HOST.
  |  In Host Buffering Mode, it is used as well to write Data from
  |  the HOST to the chip memory
  |
  |  This Function does not need to be modified in No Specific DMA
  |  Engine is used to read/write Data to/from the chip. In this case,
  |  The normal read/write function is used as described below
  |
  -------------------------------------------------------------------------
 ****************************************************************************
 * @param: pContext: bridge context structure
 * @param: pDmaCtx: a context that describes the dma transfer
 ****************************************************************************/
DIBDMA DibBridgeTargetRequestDma(struct DibBridgeContext *pContext, struct DibBridgeDmaCtx * pDmaCtx)
{
   DIBSTATUS Status = DIBSTATUS_SUCCESS;

   if(pDmaCtx->DmaSize > MAX_MPE_FRAME_SIZE) 
   {
      DIB_DEBUG(DMA_ERR, (CRB "Will NOT request a %d byte DMA. There is something wrong" CRA, pDmaCtx->DmaSize));
      DIB_DEBUG(DMA_ERR, (CRB "" CRA));
      return DIB_NO_DMA;
   }
   /*
   DIB_DEBUG(PORT_LOG, (CRB "TARGET %s : Chip %x Host %p Len %d" CRA, (pDmaCtx->Dir == DIBBRIDGE_DMA_READ) ? "READ" : "WRITE", pDmaCtx->ChipAddr, pDmaCtx->pHostAddr, pDmaCtx->DmaSize));
   DIB_DEBUG(PORT_LOG, (CRB "" CRA));
   */
   /***************************************************************/
   /* HERE, the DMA is now set up before calling this function    */
   /***************************************************************/

   /***************  DMA ACCESS ***********************************/
   if(pDmaCtx->DmaSize) 
   {
      if(pDmaCtx->Dir == DIBBRIDGE_DMA_READ)
         Status = DibBridgeTargetRead(pContext, pDmaCtx->FmtChipAddr, pDmaCtx->Mode, pDmaCtx->DmaSize, pDmaCtx->pHostAddr);
      else
         Status = DibBridgeTargetWrite(pContext, pDmaCtx->FmtChipAddr, pDmaCtx->Mode, pDmaCtx->DmaSize, pDmaCtx->pHostAddr);
   }

   if(Status != DIBSTATUS_SUCCESS)
   {
      DIB_DEBUG(DMA_ERR, (CRB "DibBridgeTargetRead/Write Failed %d Addr %x, Mode %d, Size %d" CRA, Status, pDmaCtx->FmtChipAddr, pDmaCtx->Mode, pDmaCtx->DmaSize));
      return DIB_DEV_FAILED;
   }
   else
   {
      return DIB_DMA_DONE;
   }
}

/****************************************************************************
* Allocate requested Buffer Size
****************************************************************************/
uint8_t *DibBridgeTargetAllocBuf(uint32_t Size)
{
   return (uint8_t *) __get_free_pages(GFP_DMA | GFP_KERNEL, IntBridgeTargetGetNbPages(Size));
}

/****************************************************************************
* Freeing allocated Buffer 
****************************************************************************/
void DibBridgeTargetFreeBuf(uint8_t *pBuf, uint32_t Size)
{
   if(pBuf != NULL) 
   {
      free_pages((unsigned long int) pBuf, IntBridgeTargetGetNbPages(Size));
      pBuf = NULL;
   }
}

/****************************************************************************
* Setup the interrupt handler
****************************************************************************/
void DibBridgeTargetSetupIrqHandler(struct DibBridgeContext *pContext)
{
   /* Start IRQ thread */
   DibBridgeTargetStartIrqPollThread(pContext);
}

/****************************************************************************
 * Remove and clean-up the interrupt handler
 ****************************************************************************/
void DibBridgeTargetRemoveIrqHandler(struct DibBridgeContext *pContext)
{
   /* Stop the IRQ thread */
   DibBridgeTargetStopIrqPollThread(pContext);
}

/****************************************************************************
 * DibBridgedisable_IRQ : to be implemented according to the target 
 ****************************************************************************/
void DibBridgeTargetDisableIRQ(struct DibBridgeContext *pContext)
{
   DibAcquireLock(&pContext->BridgeTargetCtx.IrqLock);
}

/****************************************************************************
 * DibBridgeEnableIrq : to be implemented according to the target 
 ****************************************************************************/
void DibBridgeTargetEnableIrq(struct DibBridgeContext *pContext)
{
   DibReleaseLock(&pContext->BridgeTargetCtx.IrqLock);
}


/****************************************************************************
* DibBridgeTargetMpeCallback : to be implemented according to the target 
****************************************************************************/
void DibBridgeTargetMpeCallback(struct DibBridgeContext *pContext, uint8_t *pBuf, uint32_t Size, uint8_t Svc)
{
   DibBridgeTargetParseMpeFrame(pContext, pBuf, Size);
}

/****************************************************************************
* DibBridgeTargetRawTsCallback : to be implemented according to the target 
****************************************************************************/
void DibBridgeTargetRawTsCallback(struct DibBridgeContext *pContext, uint8_t *pBuf, uint32_t Size)
{
   DibBridgeTargetNetSubmitRawTsPacket(pBuf, Size);
}


/**
 * IntBridgeGetTimeInMs()
 * @param  void
 * @return the current time in milliseconds
 */
uint32_t IntBridgeGetTimeInMs(void)
{
   return (uint32_t)((jiffies * 1000) / HZ);
}







