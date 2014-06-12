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
* @file "DibBridgeMailboxHandler.c"
* @brief Bridge Mailbox handling.
*
***************************************************************************************************/
#include "DibBridgeConfig.h" /* Must be first include of all SDK files - Defines compilation options */
#include "DibBridgeTargetDebug.h"
#include "DibBridgeCommon.h"
#include "DibBridge.h"
#include "DibBridgeTarget.h"
#include "DibBridgeRawTs.h"
#include "DibBridgeMailboxHandler.h"
#include "DibBridgeData.h"
#include "DibDriverDragonflyStd.h" // by infospace, dckim


#if (DRIVER_AND_BRIDGE_MERGED == 0)
#include "DibBridgeTargetModule.h"
#endif /*DRIVER_AND_BRIDGE_MERGED */

#if (SUPPORT_CAV == 1)
#include "DibBridgeCav.h"
#endif

/**
 * directly call the low data msg handler.
 * @param[in] min: Field Info 1 of IN_DATA_MSG. Most of the time it is a lower limit in a circular buffer of the firmware
 * @param[in] max: Field Info 2 of IN_DATA_MSG. Most of the time it is a higher limit in a circular buffer of the firmware
 * @param[in] add: Field Info 3 of IN_DATA_MSG. Most of the time it is the add in the firmware, src of the dma transfert
 * @param[in] len: Field Info 4 of IN_DATA_MSG. Most of the time it is the len in lines (256 bytes) or bytes to transfert
 * @param[in] flags: the flags (svc, type, first_frag, last_frag, prefetch..) needed when the dma transfert is finished.
 * @return: the DMA state: no DMA (DIB_NO_DMA), done (DIB_DMA_DONE) or pending (DIB_DMA_PENDING).
 */
#if (mSDK==0)
DIBDMA IntBridgeDeferDataMsg(struct DibBridgeContext *pContext, uint32_t min, uint32_t max, uint32_t add, uint32_t len, struct DibBridgeDmaFlags *pFlags)
{
   return DibBridgeLowDataMsgHandlerCommon(pContext, min, max, add, len, pFlags);
}

/**
 * Host Dma transfer is now finished, so we can process the information to upper layer
 * and eventually acknowledge the firmware.
 * @param[in] pContext; the bridge context structure.
 */
DIBSTATUS DibBridgeProcessDmaCommon(struct DibBridgeContext *pContext, struct DibBridgeDmaCtx * pDmaCtx)
{
   DIBSTATUS Status = DIBSTATUS_SUCCESS;
   ELEM_HDL ItemHdl = pDmaCtx->DmaFlags.ItemHdl; /**< This is an ItemHandler */
   DIB_ASSERT(ItemHdl < DIB_MAX_NB_ITEMS);
   DIB_ASSERT(pDmaCtx->DmaFlags.Type <= FORMAT_MSCPACKET);

   if(pDmaCtx->DmaFlags.Prefetch == 0)
   {
      switch (pDmaCtx->DmaFlags.Type) 
      {
         case FORMAT_SIPSI:
            DibBridgeSiPsiDone(pContext, pDmaCtx);
            break;

         case FORMAT_CMMB_RSM:
            DIB_ASSERT(pContext->ItSvc[ItemHdl].FilterParent != DIB_UNSET);
            DibBridgeCmmbRsmDone(pContext, pContext->FilterInfo[pContext->ItSvc[ItemHdl].FilterParent].DataMode, pDmaCtx);
            break;

         case FORMAT_PES_VIDEO:
         case FORMAT_PES_AUDIO:
         case FORMAT_PES_OTHER:
         case FORMAT_PCR:
         case FORMAT_RAWTS:
         case FORMAT_MSCPACKET:
         case FORMAT_MSCPLUS:
         case FORMAT_FIG:
         case FORMAT_MSC:
            /* check item cause the bridge can be still not informed that a new item is created whearas a block is ready!! */
            DIB_ASSERT(pContext->ItSvc[ItemHdl].FilterParent != DIB_UNSET);
            DibBridgeOtherDataDone(pContext, pContext->FilterInfo[pContext->ItSvc[ItemHdl].FilterParent].DataMode, pDmaCtx);
            break;

         case FORMAT_MPE:
         case FORMAT_MPE_CORR:
         case FORMAT_LAST_FRG:
         case FORMAT_FRG:
            DIB_ASSERT(pContext->ItSvc[ItemHdl].FilterParent != DIB_UNSET);
            DibBridgeMpeDataDone(pContext, pDmaCtx);
            break;

         case FORMAT_SH_FRG_DATA:
         case FORMAT_SH_FRG_IFEC:
         case FORMAT_SH_PATTERN_DATA:
         case FORMAT_SH_PATTERN_IFEC:
         case FORMAT_SH_FILL_DATA:
         case FORMAT_SH_FILL_IFEC:
         case FORMAT_SH_ALLOC:     
            /*DibBridgeShDataDone(pContext, pDmaCtx);*/
            break;

         case FORMAT_FLUSH_SVC:
            /* only forward Data to the IP Stack if it is not prefecth Data */
            break;

         case FORMAT_SLICE_REQ:
            if(pContext->DibChip == DIB_FIREFLY)
               DibBridgeSendAck(pContext, &pDmaCtx->DmaFlags, 0);
            break;

         case FORMAT_SLICE_DONE:
            Status = DibBridgeSliceDone(pContext, pDmaCtx);
            break;

#if (DEMOD_TRACE == 1)
         case FORMAT_DTRACE:
            DibB2DFwdDemodTrace(pContext, DTRACE_BUFF_SIZE, pContext->DemodTraceBuf);
            break;
#endif

#if (SUPPORT_CAV == 1)
         case FORMAT_CAV1:
            IntBridgeCav1Done(pContext, pDmaCtx);
            break;

         case FORMAT_CAV2:
            IntBridgeCav2Done(pContext, pDmaCtx);
            break;
#endif
#if 0 // by infospace, dckim : Default = Enable
         case FORMAT_ATSCMH_FIC:
         case FORMAT_ATSCMH_RSM:
            IntBridgeAtscmhDone(pContext, pDmaCtx);
            break;
#endif
         default:
            DIB_DEBUG(MAILBOX_ERR, (CRB "Oops! Unknown DMA action %d!" CRA, pDmaCtx->DmaFlags.Type));
            return DIBSTATUS_ERROR;
      }
   }

   /** We clear DmaCtx to indicate that it is cleaned */
   memset(pDmaCtx, 0, sizeof(struct DibBridgeDmaCtx));      /* DMA is done now */

   return Status;
}

/**
 * Handler for IN_MSG_DATA message.
 * Parses and processes the most prioritary messages, and passes the others
 * to the upper layer. Start host DMA transfert if needed.
 * @param[in] min: Field Info 1 of IN_DATA_MSG. Most of the time it is a lower limit in a circular buffer of the firmware
 * @param[in] max: Field Info 2 of IN_DATA_MSG. Most of the time it is a higher limit in a circular buffer of the firmware
 * @param[in] add: Field Info 3 of IN_DATA_MSG. Most of the time it is the add in the firmware, src of the dma transfert
 * @param[in] len: Field Info 4 of IN_DATA_MSG. Most of the time it is the len in lines (256 bytes) or bytes to transfert
 * @param[in] flags: the flags (svc, type, first_frag, last_frag, prefetch..) needed when the dma transfert is finished.
 * @return: the DMA state: no DMA (DIB_NO_DMA), done (DIB_DMA_DONE) or pending (DIB_DMA_PENDING).
 */
DIBDMA DibBridgeHighDataMsgHandlerCommon(struct DibBridgeContext *pContext, uint32_t min, uint32_t max, uint32_t add, uint32_t len, struct DibBridgeDmaFlags *pFlags)
{
   DIBDMA   DmaStat;

   DIB_ASSERT((pFlags->LastFrag == eDIB_TRUE) || (pFlags->LastFrag == eDIB_FALSE));

   /* Data notification */
   DmaStat = DIB_NO_DMA;

   /* Sanity checks */
   DIB_ASSERT(pFlags->ItemHdl < DIB_MAX_NB_ITEMS);
   if(pFlags->ItemHdl >= DIB_MAX_NB_ITEMS) 
   {
      DIB_DEBUG(MAILBOX_ERR, (CRB "DMA transfer request illegal Item number %d!" CRA, pFlags->ItemHdl));
      return DIB_NO_DMA;
   }

   DIB_ASSERT(pFlags->NbRows <= 4);

   switch (pFlags->Type) 
   {
      /* -------------------------------------------------------------------------------- */
      case FORMAT_SLICE_REQ:
      case FORMAT_SLICE_DONE:
      case FORMAT_SIPSI:
      case FORMAT_FIG:
      case FORMAT_MSC:
      case FORMAT_RAWTS:
      case FORMAT_MPE_CORR:
      case FORMAT_MSCPACKET:
      case FORMAT_MSCPLUS:
         DmaStat = IntBridgeDeferDataMsg(pContext, min, max, add, len, pFlags);
         break;

         /* -------------------------------------------------------------------------------- */
      case FORMAT_MPE:
      case FORMAT_FRG:
      case FORMAT_LAST_FRG:
      case FORMAT_FLUSH_SVC:
      case FORMAT_PES_VIDEO:
      case FORMAT_PES_AUDIO:
      case FORMAT_PES_OTHER:
      case FORMAT_PCR:
      case FORMAT_SH_FRG_DATA:
      case FORMAT_SH_FRG_IFEC:
      case FORMAT_SH_PATTERN_DATA:
      case FORMAT_SH_PATTERN_IFEC:
      case FORMAT_SH_FILL_DATA:
      case FORMAT_SH_FILL_IFEC:
      case FORMAT_SH_ALLOC:
      case FORMAT_CMMB_RSM:
#if (SUPPORT_CAV == 1)
      case FORMAT_CAV1:
      case FORMAT_CAV2:
#endif
      case FORMAT_ATSCMH_FIC:
      case FORMAT_ATSCMH_RSM:
         DmaStat = DibBridgeLowDataMsgHandlerCommon(pContext, min, max, add, len, pFlags);
         break;

         /* -------------------------------------------------------------------------------- */
      default:
         DIB_DEBUG(MAILBOX_ERR, (CRB "High: Received unknown Type for Data message: %d" CRA, pFlags->Type));
         break;
   }

   return DmaStat;      /* Tells do we have a pending DMA or not */
}

/**
 * Handler for IN_MSG_DATA message.
 * Parses and processes the most prioritary messages, and passes the others
 * to the upper layer. Start host DMA transfert if needed.
 * @param[in] min: Field Info 1 of IN_DATA_MSG. Most of the time it is a lower limit in a circular buffer of the firmware
 * @param[in] max: Field Info 2 of IN_DATA_MSG. Most of the time it is a higher limit in a circular buffer of the firmware
 * @param[in] add: Field Info 3 of IN_DATA_MSG. Most of the time it is the add in the firmware, src of the dma transfert
 * @param[in] len: Field Info 4 of IN_DATA_MSG. Most of the time it is the len in lines (256 bytes) or bytes to transfert
 * @param[in] flags: the flags (svc, type, first_frag, last_frag, prefetch..) needed when the dma transfert is finished.
 * @return: the DMA state: no DMA (DIB_NO_DMA), done (DIB_DMA_DONE) or pending (DIB_DMA_PENDING).
 */
DIBDMA DibBridgeLowDataMsgHandlerCommon(struct DibBridgeContext *pContext, uint32_t min, uint32_t max, uint32_t add, uint32_t len, struct DibBridgeDmaFlags *pFlags)
{
   DIBDMA   DmaStat;

   DIB_ASSERT((pFlags->LastFrag == eDIB_TRUE) || (pFlags->LastFrag == eDIB_FALSE));

   /* Data notification */
   DmaStat = DIB_NO_DMA;

   DIB_DEBUG(MAILBOX_LOG, (CRB "MSG IN DATA:  Type=%d item=%d min=0x%08x max=0x%08x add=0x%08x len=%d" CRA, pFlags->Type, pFlags->ItemHdl, min, max, add, len));

   /* Sanity checks */
   DIB_ASSERT(pFlags->ItemHdl < DIB_MAX_NB_ITEMS);
   if(pFlags->ItemHdl >= DIB_MAX_NB_ITEMS) 
   {
      DIB_DEBUG(MAILBOX_ERR, (CRB "DMA transfer request illegal Item number %d!" CRA, pFlags->ItemHdl));
      return DIB_NO_DMA;
   }

   DIB_ASSERT(pFlags->NbRows <= 4);

   switch (pFlags->Type) 
   {
      /* -------------------------------------------------------------------------------- */
      case FORMAT_FLUSH_SVC:
         DibBridgeFlushService(pContext, pFlags->ItemHdl, (uint8_t) min, (uint8_t) max, pFlags->Prefetch);
         break;

      case FORMAT_SLICE_REQ:
         if (pContext->SliceBuf == NULL) 
         {
            /*** FEC slice Buffer allocation*/
            pContext->SliceBuf = DibBridgeTargetAllocBuf(HBM_SLICE_SIZE);

            if(pContext->SliceBuf == 0) 
            {
               union DibEventConfig EventConfig;
               EventConfig.DataLostEvent.ItemHdl = pFlags->ItemHdl;
               DIB_DEBUG(PORT_ERR, (CRB "Unable to allocate slice Buffer" CRA));

               /* signal error to firmware flush cor Buffer*/
               DibBridgeSendAck(pContext, pFlags, 1);
               DibB2DFwdEvents(pContext, eDATA_LOST, &EventConfig);
               break;
            }
         }
         DmaStat = DibBridgeAssembleSlice(pContext, min, max, add, len, pFlags);

         break;

         /* -------------------------------------------------------------------------------- */
      case FORMAT_SLICE_DONE:
         DmaStat = DibBridgeGetSlice(pContext, min, max, add, len, pFlags);

         break;

         /* -------------------------------------------------------------------------------- */
      case FORMAT_SIPSI:
         DmaStat = DibBridgeGetSipsi(pContext, min, max, add, len, pFlags);
         break;

         /* -------------------------------------------------------------------------------- */
      case FORMAT_RAWTS:
      case FORMAT_FIG:
      case FORMAT_MSCPACKET:
      case FORMAT_MSCPLUS:
      case FORMAT_MSC:
      case FORMAT_PES_VIDEO:
      case FORMAT_PES_AUDIO:
      case FORMAT_PES_OTHER:
      case FORMAT_PCR:
      case FORMAT_CMMB_RSM:
#if (SUPPORT_CAV == 1)
      case FORMAT_CAV1:
      case FORMAT_CAV2:
#endif
      case FORMAT_ATSCMH_FIC:
      case FORMAT_ATSCMH_RSM:
         DmaStat = DibBridgeGetRawData(pContext, min, max, add, len, pFlags);
         break;

      case FORMAT_SH_FRG_DATA:
      case FORMAT_SH_FRG_IFEC:
         DmaStat = DibBridgeGetShFragment(pContext, min, max, 0, add, len, pFlags, 1);
         /*DibMSleep(1);*/
         break;

      case FORMAT_SH_PATTERN_DATA:
      case FORMAT_SH_PATTERN_IFEC:
         pContext->DmaCtx.DmaFlags = *pFlags;
         pContext->DmaCtx.DmaLen   = len;
         DmaStat = DibBridgeFillShPattern(pContext, min, max, add, len, pFlags);
         break;

      case FORMAT_SH_FILL_DATA:
      case FORMAT_SH_FILL_IFEC:
         pContext->DmaCtx.DmaFlags = *pFlags;
         pContext->DmaCtx.DmaLen   = len;
         DmaStat = DibBridgeFillShData(pContext, min, max, add, len, pFlags);
         break;

      case FORMAT_SH_ALLOC:
         pContext->DmaCtx.DmaFlags = *pFlags;
         pContext->DmaCtx.DmaLen   = len;
         DmaStat = DibBridgeAllocMemory(pContext, min, max, add, len, pFlags);
         break;
         
         /* -------------------------------------------------------------------------------- */
      case FORMAT_MPE:
      case FORMAT_FRG:
      case FORMAT_LAST_FRG:
         DmaStat = DibBridgeGetFragment(pContext, min, max, add, len, pFlags);
         break;

      case FORMAT_MPE_CORR:
         /** As no dma is required only set the pFlags and Size and complete dma */
         pContext->DmaCtx.DmaFlags = *pFlags;
         pContext->DmaCtx.DmaLen   = len;
         /** ProcessDmaDone after this function */
         DmaStat = DIB_DMA_DONE;
         break;

         /* -------------------------------------------------------------------------------- */
      default:
         DIB_DEBUG(MAILBOX_ERR, (CRB "Low: Received unknown Type for Data message: %d" CRA, pFlags->Type));
         break;
   }

   return DmaStat;      /* Tells do we have a pending DMA or not */
}
#endif /* mSDK == 0 */

/**
 * Called from bridge context to send a message comming from the driver to the firmware.
 * Messages sent directly from the bridge to the firmware are directly sent using 
 * DibBridgeSendMsgSpec.
 * @param[in] pContext: The bridge context structure
 * @param[in] Data: The buffer to send.
 * @param[in] len: The length in bytes of the message.
 * @return: DIBSTATUS_XXX
 */
DIBSTATUS DibBridgeSendMsg(struct DibBridgeContext *pContext, uint32_t *Data, uint32_t len)
{
   DIBSTATUS ret = DIBSTATUS_ERROR;

   DibBridgeTargetDisableIRQ(pContext);

   DIB_DEBUG(MAILBOX_LOG, (CRB "MSG OUT sent" CRA));

   ret = DibBridgeSendMsgSpec(pContext, Data, len);

   DibBridgeTargetEnableIrq(pContext);

   return ret;
}

/*******************************************************************************
*  Send a msg to the chip
******************************************************************************/
DIBSTATUS DibBridgeTransfertBuf(struct DibBridgeContext *pContext, struct DibBridgeDmaCtx * pDmaCtx)
{
   DIBSTATUS ret;

   DibBridgeTargetDisableIRQ(pContext);
   DibBridgeSetupDma(pContext, pDmaCtx);

   ret = DibBridgeTargetRequestDma(pContext, pDmaCtx);

   DibBridgeTargetEnableIrq(pContext);

   return (ret == DIB_DMA_DONE) ? DIBSTATUS_SUCCESS : DIBSTATUS_ERROR;
}

#if (DRIVER_AND_BRIDGE_MERGED == 0)
/****************************************************************************
 * Bridge 2 Driver msg handling function. Also used to transmit SIPSI fragment
 * buffers to the Driver.
 ****************************************************************************/
void DibBridgeInitMsgQueue(struct DibBridgeContext *pContext)
{
   DibAllocateLock(&pContext->MsgQueueLock);
   DibInitLock(&pContext->MsgQueueLock);

   DibAllocateEvent(&pContext->MsgQueueEvent);
   DibInitNotificationEvent(&pContext->MsgQueueEvent);

   pContext->MsgQueueTail        = 0;
   pContext->MsgQueueHead        = 0;
   pContext->MsgQueueInitialized = 1;
}

/****************************************************************************
 * Warn the BridgeGetMsgQueue function that the MsgQueue is terminating.
 * Then the MsgQueue will be DeInited on close sys call.
 ****************************************************************************/
void DibBridgeTerminateMsgQueue(struct DibBridgeContext *pContext)
{
   pContext->MsgQueueInitialized = 0;
   DibSetEvent(&pContext->MsgQueueEvent);
   DIB_DEBUG(PORT_LOG, (CRB "DibBridgeTerminateMsgQueue!" CRA));
}

void DibBridgeDeinitMsgQueue(struct DibBridgeContext *pContext)
{
   /** msg queue lock and event are freed uppon close */
   DibDeAllocateLock(&pContext->MsgQueueLock);
   DibDeAllocateEvent(&pContext->MsgQueueEvent);
   DIB_DEBUG(PORT_LOG, (CRB "DibBridgeDeinitMsgQueue!" CRA));
}

/**
 * Retrieve a msg from the Bridge 2 Driver msg queue, and copy it to the input buffer using a specific target copy function;
 */
DIBSTATUS DibBridgeGetBufFromMsgQueue(struct DibBridgeContext *pContext, uint16_t *pType, uint16_t *pSize, uint8_t *pBuf)
{
   struct DibBridgeMsgQueue *pMsg;
   uint32_t  MsgQueueAvail,  MsgSize;
   uint16_t  Nb;
   uint8_t  *pQueueAddr;
   DIBSTATUS rc = DIBSTATUS_ERROR;

   /** proper initialisation */
   *pSize = 0;

check_msg:

   /** safely access to the queue */
   DibAcquireLock(&pContext->MsgQueueLock);

   DIB_ASSERT(pContext->MsgQueueHead < UPMESSAGE_MAILBOX_SIZE);
   DIB_ASSERT(pContext->MsgQueueTail < UPMESSAGE_MAILBOX_SIZE);

   DIB_ASSERT((pContext->MsgQueueHead & 3) == 0);
   DIB_ASSERT((pContext->MsgQueueTail & 3) == 0);

   /** Check if message queue is empty. Warning: Bridge can be exiting, and several events can have been sent
    *  by the low level thread. If queue is empty, check first for message queue terminaison, otherwise wait for an event. */
   if(pContext->MsgQueueHead == pContext->MsgQueueTail) 
   {
      DibReleaseLock(&pContext->MsgQueueLock);

      /* Check for bridge terminaison */
      if(!pContext->MsgQueueInitialized) 
      {
         DIB_DEBUG(PORT_ERR, (CRB "DibBridgeGetBufFromMsgQueue: msg queue closed, exiting driver!" CRA));

         DibBridgeDeinitMsgQueue(pContext);

         return DIBSTATUS_ERROR;
      }

      /** wait for a message event */
      DIB_DEBUG(PORT_LOG, (CRB "DibBridgeGetBufFromMsgQueue: WaitForEvent...!" CRA));
      rc = DibWaitForEvent(&pContext->MsgQueueEvent, -1);
      DIB_DEBUG(PORT_LOG, (CRB "DibBridgeGetBufFromMsgQueue: ...WakeUp!" CRA));
      if(rc != DIBSTATUS_SUCCESS)
      {
         /* this should never occur, even when exiting sdk */
         DIB_DEBUG(PORT_ERR, (CRB "DibWaitForEvent failed !" CRA));
         return rc;
      }

      goto check_msg;
   }
   else
   {
      pQueueAddr = ((uint8_t *)pContext->MsgQueue + pContext->MsgQueueTail);

      DIB_DEBUG(PORT_LOG, (CRB "DibBridgeGetBufFromMsgQueue: ...Read one message!" CRA));
      /** extract message header */
      pMsg = (struct DibBridgeMsgQueue *)pQueueAddr;
      *pType = pMsg->Type;
      *pSize = Nb = pMsg->Size;
      MsgSize = DIB_BRIDGE_MESSAGE_SIZE(Nb);
      pQueueAddr += sizeof(struct DibBridgeMsgQueue);
      pContext->MsgQueueTail += sizeof(struct DibBridgeMsgQueue);

      /** wrap if needed after message header copy */
      if(pContext->MsgQueueTail >= UPMESSAGE_MAILBOX_SIZE)
      {
         pQueueAddr = (uint8_t *)pContext->MsgQueue;
         pContext->MsgQueueTail = 0;
      }

      /** check if a wrap is needed during buffer copy */
      if(Nb > 0)
      {
         /** get the number of free byte until the end of the buffer */
         MsgQueueAvail = UPMESSAGE_MAILBOX_SIZE - pContext->MsgQueueTail;

         DIB_ASSERT(MsgQueueAvail >= 4);

         if(Nb > MsgQueueAvail)
         {
            /** copy the first part */
            DibBridgeTargetCopyToUser(pBuf, pQueueAddr, MsgQueueAvail);

            /** copy the second part */
            pQueueAddr = (uint8_t *)pContext->MsgQueue;
            DibBridgeTargetCopyToUser(pBuf + MsgQueueAvail, pQueueAddr, Nb - MsgQueueAvail);
         }
         else
         {
            /** copy the whole block */
            DibBridgeTargetCopyToUser(pBuf, pQueueAddr, Nb);
         }
      }

      pContext->MsgQueueTail = (pContext->MsgQueueTail + MsgSize - sizeof(struct DibBridgeMsgQueue)) % (UPMESSAGE_MAILBOX_SIZE);

      /* Type if flagged with last bit set to 1 if queue becomes empty */
      if(pContext->MsgQueueHead == pContext->MsgQueueTail) *pType |= UPMESSAGE_BIT_EMPTY;

      DibReleaseLock(&pContext->MsgQueueLock);
   }

   return DIBSTATUS_SUCCESS;
}

DIBSTATUS DibBridgeAddBufferToMessageQueue(struct DibBridgeContext *pContext, uint16_t Type, uint32_t Nb, uint8_t *pBuf)
{
   struct DibBridgeMsgQueue * pMsg;
   uint8_t *pQueueAddr;
   uint32_t MsgQueueFree;
   uint32_t MsgSize;
   uint32_t OldHead;
   uint32_t WasEmpty;

   /* No init done */
   if(!pContext->MsgQueueInitialized) 
   {
      DIB_DEBUG(PORT_ERR, (CRB "can't add msg to msg queue : no init done !" CRA));
      return DIBSTATUS_ERROR;
   }

   DIB_ASSERT(pBuf);
   DIB_ASSERT(Nb <= UPMESSAGE_MAX_SIZE);
   DIB_ASSERT(Nb <= 0xFFFF);   /* as typecasted in unt16_t */

   /** safely access to MsgQueue */
   DibAcquireLock(&pContext->MsgQueueLock);

   DIB_ASSERT(pContext->MsgQueueHead < UPMESSAGE_MAILBOX_SIZE);
   DIB_ASSERT(pContext->MsgQueueTail < UPMESSAGE_MAILBOX_SIZE);

   DIB_ASSERT((pContext->MsgQueueHead & 3) == 0);
   DIB_ASSERT((pContext->MsgQueueTail & 3) == 0);

   MsgQueueFree = (pContext->MsgQueueTail - pContext->MsgQueueHead + UPMESSAGE_MAILBOX_SIZE - 1) % (UPMESSAGE_MAILBOX_SIZE);
   WasEmpty = (pContext->MsgQueueTail == pContext->MsgQueueHead) ? 1 : 0 ;

   /** the message contain the buffer. be carefull with message size. */
   MsgSize = DIB_BRIDGE_MESSAGE_SIZE(Nb);

   DIB_DEBUG(PORT_LOG, (CRB "IntBridgeAddBufferToMessageQueue: MsgQueueFree=%d, MsgSize=%d" CRA,MsgQueueFree, MsgSize));

   DIB_ASSERT((MsgSize & 3) == 0);

   /** The number of free bytes must stay stricktly greater than 0 after adding the message to avoid pointer beeing equal */
   if( MsgSize >= MsgQueueFree ) 
   {
      DIB_DEBUG(PORT_ERR, (CRB "QUEUE FULL (free bytes = %d), discard msg %d of size %d %d" CRA, MsgQueueFree, Type, MsgSize, Nb));
      DibReleaseLock(&pContext->MsgQueueLock);
      return DIBSTATUS_RESOURCES;
   }

   /** If we have a non prio message, check that we still have UPMESSAGE_MAILBOX_MIN_SIZE for our priority messages */
   if(((MsgQueueFree - MsgSize) < UPMESSAGE_MAILBOX_MIN_SIZE) && (Type != UPMESSAGE_TYPE_MSG))
   {
      DIB_DEBUG(PORT_ERR, (CRB "QUEUE CRITICAL SIZE (free bytes = %d), discard msg %d of size %d %d" CRA, MsgQueueFree, Type, MsgSize, Nb));
      DibReleaseLock(&pContext->MsgQueueLock);
      return DIBSTATUS_RESOURCES;
   }

   pQueueAddr = ((uint8_t *)pContext->MsgQueue + pContext->MsgQueueHead);

   /** we always have the place to put at least the message header into the circular buffer */
   pMsg       = (struct DibBridgeMsgQueue *)pQueueAddr;
   pMsg->Size = (uint16_t) Nb;
   pMsg->Type = Type;
   pQueueAddr += sizeof(struct DibBridgeMsgQueue);
   pContext->MsgQueueHead += sizeof(struct DibBridgeMsgQueue);

   /** wrap if needed after message header copy */
   if(pContext->MsgQueueHead >= UPMESSAGE_MAILBOX_SIZE)
   {
      pQueueAddr = (uint8_t *)pContext->MsgQueue;
      pContext->MsgQueueHead = 0;
   }

   /** check if a wrap is needed during buffer copy */
   if(Nb > 0)
   {
      /** get the number of free byte until the end of the buffer */
      MsgQueueFree = UPMESSAGE_MAILBOX_SIZE - pContext->MsgQueueHead;

      DIB_ASSERT(MsgQueueFree >= 4);

      if(Nb > MsgQueueFree)
      {
         /** copy the first part */
         DibMoveMemory(pQueueAddr, pBuf, MsgQueueFree);

         /** copy the second part */
         pQueueAddr = (uint8_t *)pContext->MsgQueue;
         DibMoveMemory(pQueueAddr, pBuf + MsgQueueFree, Nb - MsgQueueFree);
      }
      else
      {
         /** copy the buffer in one block */
         DibMoveMemory(pQueueAddr, pBuf, Nb);
      }
   }

   /** move MsgQueueHead with MsgSize, not Nb! */
   OldHead = pContext->MsgQueueHead;
   pContext->MsgQueueHead = (pContext->MsgQueueHead + MsgSize - sizeof(struct DibBridgeMsgQueue)) % (UPMESSAGE_MAILBOX_SIZE);

   DIB_ASSERT(pContext->MsgQueueHead != pContext->MsgQueueTail);

   DIB_DEBUG(RAWTS_LOG, (CRB "IntBridgeAddBufferToMessageQueue Add Msg (Size: %d Type: %d)" CRA, Nb, Type));    

   /** warn the user if queue was empty */
   if(WasEmpty)
   {
      DibSetEvent(&pContext->MsgQueueEvent);
   }

   DibReleaseLock(&pContext->MsgQueueLock);

   return DIBSTATUS_SUCCESS;
}

#endif /*DRIVER_AND_BRIDGE_MERGED */
