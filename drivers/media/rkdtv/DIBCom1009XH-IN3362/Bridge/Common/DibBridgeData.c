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

#if (DRIVER_AND_BRIDGE_MERGED == 0)
#include "DibBridgeTargetModule.h"
#endif /*DRIVER_AND_BRIDGE_MERGED */

#if ((DIB_CHECK_IP_DATA == 1)  || (DIB_CHECK_RTP_DATA == 1) || (DIB_CHECK_RAWTS_DATA == 1) || \
     (DIB_CHECK_PES_DATA == 1) || (DIB_CHECK_PCR_DATA == 1) || (DIB_CHECK_CAV_DATA == 1)   || \
     (DIB_CHECK_FIG_DATA == 1) || (DIB_CHECK_MSC_DATA == 1))
#include "DibBridgePayloadCheckers.h"
#endif

#if (SUPPORT_CAV == 1)
#include "DibBridgeCav.h"
#endif

#if (mSDK == 0)
void DibBridgeOtherDataDone(struct DibBridgeContext * pContext, enum DibDataMode DataMode, struct DibBridgeDmaCtx * pDmaCtx)
{
   uint8_t rc;

#if DIB_CHECK_RAWTS_DATA == 1
   if(pDmaCtx->DmaFlags.Type == FORMAT_RAWTS)
   {
      DibBridgeCheckRawTsData(pContext, pDmaCtx->pHostAddr, pDmaCtx->DmaLen, pDmaCtx->DmaFlags.ItemHdl);
      DibBridgeForwardCheckStats(pContext, pDmaCtx->DmaFlags.ItemHdl);
   }
#endif
#if DIB_CHECK_MSC_DATA
   if(pDmaCtx->DmaFlags.Type == FORMAT_MSC)
   {
      DibBridgeMscDone(pContext, pDmaCtx, pDmaCtx->DmaFlags.ItemHdl);
      DibBridgeForwardCheckStats(pContext, pDmaCtx->DmaFlags.ItemHdl);
   }
#endif

   if(DataMode == eCLBACK)
   {
     if(pContext->DataInfoUp[pContext->NextFreeWrBuff].pBuff != NULL)
     {
       DIB_DEBUG(RAWTS_ERR, (CRB "DataInfoUp Overflow: Free Buffer" CRA));
       DibBridgeDesallocateRawBuffer(pContext, pDmaCtx->pHostAddr, pDmaCtx->DmaLen);
     }
     else
     {
        pContext->DataInfoUp[pContext->NextFreeWrBuff].pBuff = pDmaCtx->pHostAddr;
        pContext->DataInfoUp[pContext->NextFreeWrBuff].FilterIndex = pContext->ItSvc[pDmaCtx->DmaFlags.ItemHdl].FilterParent; 

        pContext->NextFreeWrBuff = (pContext->NextFreeWrBuff + 1)%(MAX_Q_BUFF);

        DIB_DEBUG(RAWTS_LOG, (CRB "NextFreeWrBuff : %d" CRA, pContext->NextFreeWrBuff));
        DibB2DFwdRawData(pContext, pDmaCtx->DmaLen, pDmaCtx->DmaFlags.ItemHdl, pDmaCtx->DmaFlags.FirstFrag, pDmaCtx->DmaFlags.LastFrag);
     }
   }
   else
   {
      switch(pDmaCtx->DmaFlags.Type)
      {
       case FORMAT_RAWTS:
          rc = DibBridgeNotifyRawbufferWrite(pContext,pDmaCtx->pHostAddr,pDmaCtx->DmaLen, pDmaCtx->DmaFlags.ItemHdl);
          DIB_ASSERT(rc == DIBSTATUS_SUCCESS);
          break;
       case FORMAT_FIG:
          DibBridgeFigDone(pContext, pDmaCtx);
          break;
       case FORMAT_MSC:
          DibBridgeMscDone(pContext, pDmaCtx, pDmaCtx->DmaFlags.ItemHdl);
          break;
      case FORMAT_MSCPACKET:
          break;
      case FORMAT_MSCPLUS:
          DibBridgeMscPlusDone(pContext, pDmaCtx, pDmaCtx->DmaFlags.ItemHdl);
          break;
      case FORMAT_PES_VIDEO:
      case FORMAT_PES_AUDIO:
      case FORMAT_PES_OTHER:
          DibBridgePesDone(pContext, pDmaCtx);
          break;
      case FORMAT_PCR:
          DibBridgePcrDone(pContext, pDmaCtx);
          break;
       default:
          DIB_DEBUG(RAWTS_ERR, (CRB "IntBridgeOtherDataDone : FORMAT %d not managed" CRA, pDmaCtx->DmaFlags.Type));
         break;
      }
   }
}

/* ------------------------------------------------------------------------ */
void DibBridgeForwardData(struct DibBridgeContext *pContext, uint8_t *pBuf, uint32_t Size, ELEM_HDL Item)
{
   enum DibDataMode DataMode = pContext->FilterInfo[pContext->ItSvc[Item].FilterParent].DataMode;

   DIB_ASSERT((DataMode == eIP) || (DataMode == eCLBACK));
   DIB_ASSERT(pBuf);
   
#if (DIB_CHECK_RTP_DATA == 1)
   DibBridgeCheckRtpData(pContext, pBuf, Size, Item);
   DibBridgeForwardCheckStats(pContext, Item);
#endif

#if (DIB_CHECK_IP_DATA == 1)
   DibBridgeCheckIpData(pContext, pBuf, Size, Item);
   DibBridgeForwardCheckStats(pContext, Item);
#endif /* DIB_CHECK_IP_DATA */

   if(DataMode == eIP)
   {
      if (pContext->IpCallback.DataMpe != NULL)
      {
         pContext->IpCallback.DataMpe(pContext, Item, pBuf, Size);
      }
   }
   else
   {
      DIB_ASSERT(pContext->DataInfoUp[pContext->NextFreeWrBuff].pBuff == NULL);
      pContext->DataInfoUp[pContext->NextFreeWrBuff].pBuff = pBuf; 
      pContext->DataInfoUp[pContext->NextFreeWrBuff].FilterIndex = pContext->ItSvc[Item].FilterParent; 
      
      pContext->NextFreeWrBuff = (pContext->NextFreeWrBuff + 1)%(MAX_Q_BUFF); 
      
      DIB_DEBUG(RAWTS_LOG, (CRB "Add Raw Buffer in Queue (NextFreeWrBuff= %d)" CRA, pContext->NextFreeWrBuff));
      
      DibB2DFwdRawData(pContext, Size, Item, 0, 0);
   }
}

/**
 * Transfert Raw Data (FIG, MSC, TS or PES) from on chip memory to bridge.
 * @param min: min base address in the firmware (circular buffer)
 * @param max: max base address in the firmware (circular buffer)
 * @param faddr: firwmare address where to find the sipsis
 * @return : The status of the dma transfert if occured.
 */
DIBDMA DibBridgeGetRawData(struct DibBridgeContext *pContext, uint32_t min, uint32_t max, uint32_t faddr, uint32_t len, struct DibBridgeDmaFlags * pFlags)
{
   struct DibBridgeDmaCtx * pDmaCtx = &pContext->DmaCtx;

   memcpy(&pDmaCtx->DmaFlags, pFlags, sizeof(struct DibBridgeDmaFlags));

   pDmaCtx->Dir         = DIBBRIDGE_DMA_READ;
   pDmaCtx->ChipAddr    = faddr;
   pDmaCtx->ChipBaseMin = min; /* NbRows */
   pDmaCtx->ChipBaseMax = max; /* K */
   pDmaCtx->DmaLen      = len;

#if (DIB_CMMB_DATA == 1)
   /* Data block contains RS matrix + descriptor (8 x 32-bit words max). */
   /* DIB_ASSERT(len <= (CMMB_RSM_MAX_SIZE + 32)); */
#else
   DIB_ASSERT(len <= RAWTS_BUFFER_SIZE);
#endif /* DIB_CMMB_DATA */

   DibBridgeAllocateRawBuffer(pContext, &pDmaCtx->pHostAddr, pDmaCtx->DmaLen);

   if(pDmaCtx->pHostAddr != NULL) 
   {
      pContext->DmaCtx = *pDmaCtx;

      DibBridgeSetupDma(pContext, pDmaCtx);

      return DibBridgeRequestDma(pContext, pDmaCtx);
   }
   else 
   {
      return DIB_NO_DMA;
   }
}

#if (DRIVER_AND_BRIDGE_MERGED == 0)
void DibB2DFwdSiPsiBuf(struct DibBridgeContext *pContext, uint32_t Nb, uint8_t *pBuf)
{
   DIBSTATUS rc;
   rc = DibBridgeAddBufferToMessageQueue(pContext, UPMESSAGE_TYPE_SIPSI, Nb, pBuf);
   DIB_ASSERT(rc == DIBSTATUS_SUCCESS);
}

#if (DEMOD_TRACE == 1)
void DibB2DFwdDemodTrace(struct DibBridgeContext *pContext, uint32_t Nb, uint8_t *pBuf)
{
   DIBSTATUS rc;
   rc = DibBridgeAddBufferToMessageQueue(pContext, UPMESSAGE_TYPE_DTRACE, Nb, pBuf);
   DIB_ASSERT(rc == DIBSTATUS_SUCCESS);
}
#endif

void DibB2DFwdMsg(struct DibBridgeContext *pContext, uint32_t Nb, uint16_t *pBuf)
{
   DIBSTATUS rc;
   rc = DibBridgeAddBufferToMessageQueue(pContext, UPMESSAGE_TYPE_MSG, Nb, (uint8_t *)pBuf);
   DIB_ASSERT(rc == DIBSTATUS_SUCCESS);
}

void DibB2DFwdRawData(struct DibBridgeContext *pContext, uint32_t Size, ELEM_HDL ItemIndex, uint8_t FirstFrag, uint8_t LastFrag)
{
   struct DibDataInfoUp DataInfoUp;
   DIBSTATUS rc;

   DataInfoUp.Length    = Size;
   DataInfoUp.ItemIndex = ItemIndex;
   DataInfoUp.Flags = (FirstFrag << 1) | LastFrag;

   rc = DibBridgeAddBufferToMessageQueue(pContext, UPMESSAGE_TYPE_DATA, sizeof(struct DibDataInfoUp), (uint8_t *)(&DataInfoUp));
   DIB_ASSERT(rc == DIBSTATUS_SUCCESS);
}

void DibB2DFwdEvents(struct DibBridgeContext *pContext, enum DibEvent Event, union DibEventConfig * pEventConfig)
{
   struct DibEventInfoUp EventInfoUp;
   DIBSTATUS rc;

   EventInfoUp.EventType = Event;
   DibMoveMemory(&(EventInfoUp.EventConfig), pEventConfig, sizeof(union DibEventConfig));

   rc = DibBridgeAddBufferToMessageQueue(pContext, UPMESSAGE_TYPE_EVENT, sizeof(struct DibEventInfoUp), (uint8_t *)(&EventInfoUp));
   DIB_ASSERT(rc == DIBSTATUS_SUCCESS);
}

#endif /*DRIVER_AND_BRIDGE_MERGED */
#endif /* mSDK == 0*/
