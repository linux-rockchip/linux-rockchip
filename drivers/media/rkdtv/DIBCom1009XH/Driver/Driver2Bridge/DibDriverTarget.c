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
* @file "DibDriverTarget.c"
* @brief Driver User to Kernel Bridge Target functionality.
*
***************************************************************************************************/
#include "DibDriverConfig.h" /* Must be first include of all SDK files - Defines compilation options */
#include "DibDriverTargetTypes.h"
#include "DibDriverTargetDebug.h"
#include "DibDriverTarget.h"

/* Include order is important for merge Mode */
#include "DibBridgeCommon.h" 
#include "DibDriverConstants.h"
#include "DibBridge.h"
#include "DibBridgeTarget.h"
#include "DibBridgeData.h"
#include "DibBridgeMailboxHandler.h"
#include "DibDriverCommon.h"
#include "DibDriverPacketDemux.h"
#include "DibDriverData.h"
#include "DibDriverDowncalls.h"
#include "DibDriverMessages.h"

/* This file makes a link between the high level and the low level driver */

/*******************************************************************************
*  Init the bridge from the driver
******************************************************************************/
DIBSTATUS DibDriverTargetInit(struct DibDriverContext *pContext)
{
   DIBSTATUS status = DIBSTATUS_ERROR;
   uint32_t Config[12];

   DIB_ASSERT(pContext->DriverTargetCtx.pBridgeContext == NULL);

   /* Allocate Bridge Context Associated to Driver Context */
   pContext->DriverTargetCtx.pBridgeContext = (struct DibBridgeContext *) DibMemAlloc(sizeof(struct DibBridgeContext));

   if(pContext->DriverTargetCtx.pBridgeContext == NULL)
      return DIBSTATUS_INSUFFICIENT_MEMORY;

   memset(pContext->DriverTargetCtx.pBridgeContext, 0, sizeof(struct DibBridgeContext));

#if (USE_DRAGONFLY == 1)
   if((pContext->DibChip == DIB_VOYAGER) || (pContext->DibChip == DIB_NAUTILUS))
   {
      Config[0]  = pContext->DragonflyRegisters.JedecAddr;
      Config[1]  = pContext->DragonflyRegisters.JedecValue;
      Config[2]  = pContext->DragonflyRegisters.MacMbxSize;
      Config[3]  = pContext->DragonflyRegisters.MacMbxStart;
      Config[4]  = pContext->DragonflyRegisters.MacMbxEnd;
      Config[5]  = pContext->DragonflyRegisters.HostMbxSize;
      Config[6]  = pContext->DragonflyRegisters.HostMbxStart;
      Config[7]  = pContext->DragonflyRegisters.HostMbxEnd;
      Config[8]  = pContext->DragonflyRegisters.HostMbxRdPtrReg;
      Config[9]  = pContext->DragonflyRegisters.HostMbxWrPtrReg;
      Config[10] = pContext->DragonflyRegisters.MacMbxRdPtrReg;
      Config[11] = pContext->DragonflyRegisters.MacMbxWrPtrReg;
   }
   else
#endif
      DibSetMemory(Config, 0, sizeof(Config));

   status = DibBridgeInit(pContext->DriverTargetCtx.pBridgeContext, pContext->BoardHdl, pContext->DibChip, Config, DEFAULT_DEBUG_MASK);

   if(status != DIBSTATUS_SUCCESS)
   {
      DibMemFree(pContext->DriverTargetCtx.pBridgeContext, sizeof(struct DibBridgeContext));
      pContext->DriverTargetCtx.pBridgeContext = NULL;
   }
   else
   {
   /* Keep reference to driver to fwd data up*/
   pContext->DriverTargetCtx.pBridgeContext->BridgeTargetCtx.pDriverContext = pContext;
   }

   return status;
}

/*******************************************************************************
*  Inform bridge that init is completed
******************************************************************************/
DIBSTATUS DibD2BPostInit(struct DibDriverContext * pContext, uint32_t flags)
{
   struct DibBridgeContext *pBridgeContext = pContext->DriverTargetCtx.pBridgeContext;

   DIB_ASSERT(pBridgeContext);

   return DibBridgePostInit(pBridgeContext, flags);
}

/****************************************************************************
* DeInit the bridge from the driver
****************************************************************************/
DIBSTATUS DibDriverTargetDeinit(struct DibDriverContext * pContext)
{
   DIBSTATUS                status         = DIBSTATUS_ERROR;
   struct DibBridgeContext *pBridgeContext = pContext->DriverTargetCtx.pBridgeContext;

   DIB_ASSERT(pBridgeContext);

   status = DibBridgeDeinit(pBridgeContext);

   /* Free allocated bridge context */
   DibMemFree(pBridgeContext, sizeof(struct DibBridgeContext));
   pContext->DriverTargetCtx.pBridgeContext = NULL;

   return status;
}


#if (mSDK==0)
/*******************************************************************************
*  Read Data from the bridge
******************************************************************************/
DIBSTATUS DibDriverTargetDataRead(struct DibDriverContext *pContext, uint8_t *Buffer, uint32_t *BytesRead, FILTER_HDL FilterHdl)
{
   uint8_t                 *ReadBuf        = NULL;
   FILTER_HDL               FilterIdx      = FilterHdl;
   struct DibBridgeContext *pBridgeContext = pContext->DriverTargetCtx.pBridgeContext;

   DIB_ASSERT(pBridgeContext);
   DIB_ASSERT(Buffer);
   DIB_ASSERT(BytesRead);
   
   if(DibBridgeGetRawBufferFull(pBridgeContext, &ReadBuf, &FilterIdx) != DIBSTATUS_SUCCESS)
   {
      /* should never happens */
      DibDriverTargetLogFunction(CRB "DibDriverTargetDataRead Error filter %d instead of %d" CRA, FilterIdx, FilterHdl);
      return DIBSTATUS_ERROR;
   }
      
   DibMoveMemory(Buffer, ReadBuf, (*BytesRead));

   /* for mpe processing, the buffer is freed later, after having forwarded the data */
   if(pContext->FilterInfo[FilterHdl].TypeFilter != eMPEFEC)
   {
      DibBridgeNotifyRawbufferRead(pBridgeContext, ReadBuf, (*BytesRead));
   }

   if((*BytesRead) == 0)
      return DIBSTATUS_ERROR;
   else
      return DIBSTATUS_SUCCESS;
}

/*******************************************************************************
 *  Flush data from bridge
 ******************************************************************************/
DIBSTATUS DibDriverTargetDataFlush(struct DibDriverContext *pContext, FILTER_HDL FilterHdl, uint32_t Size)
{   
   int32_t                  ret            = DIBSTATUS_SUCCESS;
   uint8_t                 *ReadBuf        = NULL;
   FILTER_HDL               FilterIdx      = FilterHdl;
   struct DibBridgeContext *pBridgeContext = pContext->DriverTargetCtx.pBridgeContext;

   DIB_ASSERT(pBridgeContext);

   if(DibBridgeGetRawBufferFull(pBridgeContext, &ReadBuf, &FilterIdx) != DIBSTATUS_SUCCESS)
   {
      DibDriverTargetLogFunction(CRB "DibDriverTargetDataFlush Error filter %d instead of %d" CRA, FilterIdx, FilterHdl);
      return DIBSTATUS_ERROR;
   }

   /* for mpe processing, the buffer is freed later, after having forwarded the data */
   if(pContext->FilterInfo[FilterHdl].TypeFilter != eMPEFEC)
   {
      DibBridgeNotifyRawbufferRead(pBridgeContext, ReadBuf, Size);
   }

   return ret;
}
#endif

/*******************************************************************************
* Read a register from the chip
******************************************************************************/
DIBSTATUS DibD2BReadReg(struct DibDriverContext *pContext, uint32_t Addr, uint8_t ReadMode, uint32_t Cnt, uint8_t *pBuf)
{
   struct DibBridgeContext *pBridgeContext = pContext->DriverTargetCtx.pBridgeContext;

   DIB_ASSERT(pBridgeContext);
   DIB_ASSERT(pBuf);

   return DibBridgeRead(pBridgeContext, Addr, ReadMode, pBuf, Cnt);
}

/*******************************************************************************
*  Write a register to the chip
******************************************************************************/
DIBSTATUS DibD2BWriteReg(struct DibDriverContext *pContext, uint32_t Addr, uint8_t WriteMode, uint32_t Cnt, uint8_t *pBuf)
{
   struct DibBridgeContext *pBridgeContext = pContext->DriverTargetCtx.pBridgeContext;

   DIB_ASSERT(pBridgeContext);
   DIB_ASSERT(pBuf);

   return DibBridgeWrite(pBridgeContext, Addr, WriteMode, pBuf, Cnt);
}


/*******************************************************************************
*  Send a msg to the chip
******************************************************************************/
DIBSTATUS DibD2BSendMsg(struct DibDriverContext *pContext, uint32_t Cnt, uint32_t *pBuf)
{
   struct DibBridgeContext *pBridgeContext = pContext->DriverTargetCtx.pBridgeContext;

   DIB_ASSERT(pBridgeContext);
   DIB_ASSERT(pBuf);

   return DibBridgeSendMsg(pBridgeContext, pBuf, Cnt);
}

/*******************************************************************************
*  Send a msg to the chip
******************************************************************************/
DIBSTATUS DibD2BTransfertBuf(struct DibDriverContext *pContext, struct DibDriverDmaCtx *pDma)
{
   struct DibBridgeDmaCtx   DmaCtx;
   struct DibBridgeContext *pBridgeContext = pContext->DriverTargetCtx.pBridgeContext;

   DIB_ASSERT(pBridgeContext);

   memcpy(&DmaCtx, pDma, sizeof(struct DibDriverDmaCtx));
   memset(&DmaCtx.DmaFlags, 0, sizeof(struct DibBridgeDmaFlags));

   return DibBridgeTransfertBuf(pBridgeContext, &DmaCtx);
}


/*******************************************************************************
*  Send a msg from the bridge to the driver
******************************************************************************/
void DibB2DFwdMsg(struct DibBridgeContext *pContext, uint32_t Cnt, uint16_t *pBuf)
{
   DIB_ASSERT(pContext);
   DIB_ASSERT(pContext->BridgeTargetCtx.pDriverContext);
   DIB_ASSERT(pBuf);

   DibDriverProcessMessage(pContext->BridgeTargetCtx.pDriverContext, Cnt, pBuf);
}

#if (mSDK==0)
/*******************************************************************************
*  Send a SIPSI section from the bridge to the driver
******************************************************************************/
void DibB2DFwdSiPsiBuf(struct DibBridgeContext *pContext, uint32_t Cnt, uint8_t *pBuf)
{
   DIB_ASSERT(pContext);
   DIB_ASSERT(pContext->BridgeTargetCtx.pDriverContext);
   DIB_ASSERT(pBuf);

   DibDriverProcessSiPsiBuffer(pContext->BridgeTargetCtx.pDriverContext, Cnt, pBuf);
}

/*******************************************************************************
 *  Send Raw Data from the bridge to the driver
 ******************************************************************************/
void DibB2DFwdRawData(struct DibBridgeContext *pContext, uint32_t Size, ELEM_HDL ItemIndex, uint8_t FirstFrag, uint8_t LastFrag)
{
   struct DibDriverDataInfoUp dataInfo;

   DIB_ASSERT(pContext);
   DIB_ASSERT(pContext->BridgeTargetCtx.pDriverContext);

   dataInfo.Length    = Size;
   dataInfo.ItemIndex = ItemIndex;
   dataInfo.Flags     = (FirstFrag << 1) | LastFrag;

   DibDriverRawDataRead(pContext->BridgeTargetCtx.pDriverContext, dataInfo);
  
   DIB_DEBUG(PACKET_LOG, (CRB "DibB2DFwdRawData Length %d" CRA, Size));
}

void DibB2DFwdEvents(struct DibBridgeContext *pContext, enum DibEvent Event, union DibEventConfig * pEventConfig)
{
   struct DibDriverEventInfoUp eventInfo;

   DIB_ASSERT(pContext);
   DIB_ASSERT(pContext->BridgeTargetCtx.pDriverContext);
   
   eventInfo.EventType = Event;
   DibMoveMemory((char *)&(eventInfo.EventConfig), pEventConfig, sizeof(union DibEventConfig));
   
   DibDriverPostEvent(pContext->BridgeTargetCtx.pDriverContext, &eventInfo);
   
   DIB_DEBUG(PACKET_LOG, (CRB "DibB2DFwdEvents Event %d" CRA, Event));
}
/*******************************************************************************
 *  Drive to Bridge information concerning the Data path from down to up
 ******************************************************************************/
DIBSTATUS DibD2BInformBridge(struct DibDriverContext *pContext, enum DibBridgeInfoType BridgeInfoType, union DibInformBridge * pInfoBridge)
{
   struct DibBridgeContext *pBridgeContext = pContext->DriverTargetCtx.pBridgeContext;

   DIB_ASSERT(pBridgeContext);

   return DibBridgeSetInfo(pBridgeContext, BridgeInfoType, pInfoBridge);
}
#endif

/*******************************************************************************
 *  Disable IRQ processing
 ******************************************************************************/
void DibDriverTargetDisableIrqProcessing(struct DibDriverContext *pContext)
{
   struct DibBridgeContext *pBridgeContext = pContext->DriverTargetCtx.pBridgeContext;

   DIB_ASSERT(pBridgeContext);

   DibBridgeTargetDisableIRQ(pBridgeContext);
}

/*******************************************************************************
 *  Enable IRQ processing
 ******************************************************************************/
void DibDriverTargetEnableIrqProcessing(struct DibDriverContext *pContext)
{
   struct DibBridgeContext *pBridgeContext = pContext->DriverTargetCtx.pBridgeContext;

   DIB_ASSERT(pBridgeContext);

   DibBridgeTargetEnableIrq(pBridgeContext);
}
