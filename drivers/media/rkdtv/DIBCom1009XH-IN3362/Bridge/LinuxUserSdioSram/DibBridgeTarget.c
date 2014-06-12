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
#include <pthread.h>
#include "DibBridgeCommon.h"
#include "DibBridgeTargetDebug.h"
#include "DibBridgeTarget.h"
#include "DibBridgeTargetCommon.h"
#include "DibBridge.h"
#if (BUILD_SDK == 1)
#include "DibBridgeMailboxHandler.h"
#endif


#if (INTERRUPT_MODE == USE_POLLING)
static void *DibBridgeTargetIrqPollingThread(void *Data);

/* Global var for thread init/termination synchronisation */
volatile int32_t pthreadStartStopSynchro = 1;
#endif


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
   return (uint8_t *) DibMemAlloc(Size);
}

/****************************************************************************
* Freeing allocated Buffer 
****************************************************************************/
void DibBridgeTargetFreeBuf(uint8_t *pBuf, uint32_t Size)
{
   if(pBuf != NULL) 
   {
      DibMemFree(pBuf, Size);
      pBuf = NULL;
   }
}

/****************************************************************************
* Setup the interrupt handler
****************************************************************************/
void DibBridgeTargetSetupIrqHandler(struct DibBridgeContext *pContext)
{
   /* Start IRQ thread */
   pthreadStartStopSynchro = 1;

   if ( pthread_attr_init(&pContext->BridgeTargetCtx.IrqAttr) != 0 )
   {
      DIB_DEBUG(PORT_ERR,(CRB "failed pthread_attr_init ():" CRA));
   }

   pthread_create(&pContext->BridgeTargetCtx.IrqThread, &pContext->BridgeTargetCtx.IrqAttr, DibBridgeTargetIrqPollingThread, pContext);

   DIB_DEBUG(PORT_LOG,(CRB "HighPrioThread = %u" CRA,*(uint32_t *)&pContext->BridgeTargetCtx.IrqThread));

   while(pthreadStartStopSynchro)
      DibMSleep(1);
}

/****************************************************************************
 * Remove and clean-up the interrupt handler
 ****************************************************************************/
void DibBridgeTargetRemoveIrqHandler(struct DibBridgeContext *pContext)
{
   /* Stop the IRQ thread */
   if(pthreadStartStopSynchro == 0)
   {
      pthreadStartStopSynchro = 1;

      while(pthreadStartStopSynchro)
         DibMSleep(1);
   }

   /* remove the ressources allocated for thread management */
   pthread_join(pContext->BridgeTargetCtx.IrqThread, NULL);
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
void DibBridgeTargetMpeCallback(struct DibBridgeContext *pContext, uint8_t *ip, uint32_t size, uint8_t Svc)
{

}

/****************************************************************************
* DibBridgeTargetRawTsCallback : to be implemented according to the target 
****************************************************************************/
void DibBridgeTargetRawTsCallback(struct DibBridgeContext *pContext, uint8_t *pBuf, uint32_t Size)
{

}


/**
* Thread to process chip interrupt requests
*/
static void *DibBridgeTargetIrqPollingThread(void *Data)
{
   struct DibBridgeContext *pContext = (struct DibBridgeContext *) Data;
   DIBDMA                   DmaStat;
   DIBSTATUS                Status   = DIBSTATUS_SUCCESS;

   DIB_DEBUG(PORT_LOG, (CRB "dibbridge Irq: Launching Irq polling thread" CRA));

   DibAcquireLock(&pContext->BridgeTargetCtx.IrqLock);

   /* pthread has started */
   pthreadStartStopSynchro = 0;

   for(;;) 
   {
      DibMSleep(1);
      DibBridgeTargetDisableIRQ(pContext);
   
      /* Check if we are requested to terminate */
      if(pthreadStartStopSynchro) 
      {
         /* Enable IRQ processing */
         DibBridgeTargetEnableIrq(pContext);
         DIB_DEBUG(PORT_LOG, (CRB "IRQ thread terminating" CRA));
         break;
      }

      DmaStat = DIB_NO_DMA;
      while(DmaStat != DIB_NO_IRQ) 
      {
         /* Process IRQ */
         DmaStat = DibBridgeProcessIrq(pContext);

         /* Synch or asynch, it is done now! */
         if(DmaStat == DIB_DMA_DONE) 
            Status = DibBridgeProcessDma(pContext, &pContext->DmaCtx);

         if(Status != DIBSTATUS_SUCCESS)
            DIB_DEBUG(PORT_ERR, (CRB "Process Dma Failed." CRA));

         if(DmaStat == DIB_DEV_FAILED)
         {
            pthreadStartStopSynchro = 1; /* Thread will be killed */
            DibBridgeTargetEnableIrq(pContext);
            DIB_DEBUG(PORT_ERR, (CRB "Device stopped responding or Dma Failed." CRA));
            goto End;
         }
      }

      /* Enable IRQ processing */
      DibBridgeTargetEnableIrq(pContext);
   }

   /* pthread has terminated */
   pthreadStartStopSynchro = 0;

End:
   return 0;
}

/**
 * IntBridgeGetTimeInMs()
 * @param  void
 * @return the current time in milliseconds
 */
uint32_t IntBridgeGetTimeInMs(void)
{
   struct timeval Time;
   gettimeofday(&Time, NULL);
   return ((Time.tv_sec * 1000) + (Time.tv_usec / 1000));
}







