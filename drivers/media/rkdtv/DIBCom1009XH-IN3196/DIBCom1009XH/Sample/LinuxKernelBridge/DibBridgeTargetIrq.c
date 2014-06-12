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
* @file "DibBridgeTargetIrq.c"
* @brief Target specific implementation.
*
***************************************************************************************************/
#include "DibBridgeCommon.h"
#include "DibBridgeMailboxHandler.h"
#include "DibBridgeTarget.h"
#include "DibBridgeTargetIrq.h"
#include "DibBridge.h"

#include <linux/version.h>
#include <linux/workqueue.h>

/********************************************************* 
 *                                                       *
 * USE SCHEDULE WORK TO HANDLE THE BOTOM HALF OF THE IRQ *
 * THIS IS FOR LINUX 2.6 with BRIDGE_MODE == MODE_MULTIL-*
 * -EVEL_IRQ || MODE_MULTILEVEL_POLLING                  *
 * SUPPORTED In KERNEL MODE ONLY.                        *
 *                                                       *
 *********************************************************/

volatile int32_t pthreadStartStopSynchro = 0;

/**
 *  Thread to process chip interrupt requests
 */
#if(LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,20))
void DibBridgeTargetIrqPollingThread(struct work_struct *work)
{
   struct DibBridgeContext *pContext = container_of(work, struct DibBridgeContext, BridgeTargetCtx.DibWork.work);
#else
void DibBridgeTargetIrqPollingThread(void *work)
{
   struct DibBridgeContext *pContext = (struct DibBridgeContext *) work;
#endif
   DIBDMA                   DmaStat = DIB_NO_DMA;
   DIBSTATUS                Status  = DIBSTATUS_SUCCESS;

   DibAcquireLock(&pContext->BridgeTargetCtx.IrqLock);  

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
         DIB_DEBUG(PORT_ERR, (CRB "Device stopped responding" CRA));
         goto End;
      }

   }

#if (INTERRUPT_MODE == USE_POLLING)
   if(pthreadStartStopSynchro)
       queue_delayed_work(pContext->BridgeTargetCtx.DibWorkQueue, &pContext->BridgeTargetCtx.DibWork, HZ/100);
#endif

   /* Enable IRQ processing */
   DibBridgeTargetEnableIrq(pContext);

End:
   /* Device Stopped responding */
   if(DmaStat == DIB_DEV_FAILED) 
   {
      DibBridgeTargetEnableIrq(pContext);
      DibBridgeTargetStopIrqPollThread(pContext);
   }
}

/**
 *
 */
int32_t DibBridgeTargetStartIrqPollThread(struct DibBridgeContext *pContext)
{
  /* Alloc and aquire the synchro lock */
  DibInitLock(&pContext->BridgeTargetCtx.IrqLock);    
  DibAcquireLock(&pContext->BridgeTargetCtx.IrqLock);  

  pContext->BridgeTargetCtx.DibWorkQueue = create_singlethread_workqueue("DibIrqWorkQueue");
  pthreadStartStopSynchro = 1;

#if(LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,20))
  INIT_DELAYED_WORK(&pContext->BridgeTargetCtx.DibWork, DibBridgeTargetIrqPollingThread);
#else
  INIT_WORK(&pContext->BridgeTargetCtx.DibWork, DibBridgeTargetIrqPollingThread, pContext);
#endif

#if (INTERRUPT_MODE == USE_POLLING)
  queue_delayed_work(pContext->BridgeTargetCtx.DibWorkQueue, &pContext->BridgeTargetCtx.DibWork, HZ/100);
#endif
   return 0;
}

/**
 *
 */
int32_t DibBridgeTargetStopIrqPollThread(struct DibBridgeContext *pContext)
{

   DibAcquireLock(&pContext->BridgeTargetCtx.IrqLock);  
   
   pthreadStartStopSynchro = 0;

   DibReleaseLock(&pContext->BridgeTargetCtx.IrqLock);  

   cancel_delayed_work(&pContext->BridgeTargetCtx.DibWork);

   //flush_workqueue(pContext->BridgeTargetCtx.DibWorkQueue);
   destroy_workqueue(pContext->BridgeTargetCtx.DibWorkQueue);
   DibDeAllocateLock(&pContext->BridgeTargetCtx.IrqLock);
   return 0;
}

/**
 *
 */
void DibBridgeTargetIrqWakeupThread(struct DibBridgeContext *pContext)
{
#if(LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,20))
   queue_work(pContext->BridgeTargetCtx.DibWorkQueue, &pContext->BridgeTargetCtx.DibWork.work);
#else
   queue_work(pContext->BridgeTargetCtx.DibWorkQueue, &pContext->BridgeTargetCtx.DibWork);
#endif
}
