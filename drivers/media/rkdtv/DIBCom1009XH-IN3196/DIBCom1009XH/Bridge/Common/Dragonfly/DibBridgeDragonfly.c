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
* @file "DibBridgeDragonfly.c"
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
#include "DibBridgeData.h"

#if (DRIVER_AND_BRIDGE_MERGED == 0)
#include "DibBridgeTargetModule.h"
#endif /*DRIVER_AND_BRIDGE_MERGED */

#if (DIB_CHECK_DATA == 1)
#include "DibBridgePayloadCheckers.h"
static void DibBridgeDragonflyClearCheckStats(struct DibBridgeContext *pContext, uint32_t * RxData);
#endif

#define MAC_IRQ      (1 << 1)
#define IRQ_POL_MSK  (1 << 4)

void IntBridgeGetCpt(struct DibBridgeContext *pContext, uint16_t * Data);

/* Bridge 2 Driver message handling function prototype */
void DibB2DFwdMsg(struct DibBridgeContext *pContext, uint32_t Nb, uint16_t * buf);

static uint32_t DibBridgeDragonflyReceiveMsg(struct DibBridgeContext *pContext, uint32_t * Data);
static DIBDMA DibBridgeDragonflyMsgHandler(struct DibBridgeContext *pContext, struct MsgHeader * pHeader, uint32_t * RxData);
static uint32_t DibBridgeDragonflyFormatAddress(struct DibBridgeContext *pContext, uint32_t Addr, uint8_t ByteMode);

/**
 *  Retreive the number of free bytes in the HOST mailbox
 */
static __inline uint32_t IntBridgeDragonflyMailboxSpace(uint32_t rdptr, uint32_t wrptr, uint32_t Size)
{
   uint32_t free;

   if(rdptr == wrptr)
      free = Size;
   else if(rdptr > wrptr)
      free = (rdptr-wrptr);
   else
      free = Size-(wrptr - rdptr);

   DIB_ASSERT(free >= 4);
   free -= 4;

   return free;
}

/**
 *  Retreive the number of available bytes in the HOST mailbox
 */
static __inline uint32_t IntBridgeDragonflyMailboxBytes(uint32_t rdptr, uint32_t wrptr, uint32_t Size)
{
   uint32_t nbbytes;

   if(rdptr == wrptr)
      nbbytes = 0;
   else if(wrptr > rdptr)
      nbbytes = (wrptr-rdptr);
   else
      nbbytes = Size-(rdptr - wrptr);

   return nbbytes;
}


/****************************************************************************
 * Setup chip memory controller
 ****************************************************************************/
static DIBSTATUS DibBridgeDragonflySetupDma(struct DibBridgeContext *pContext, struct DibBridgeDmaCtx * pDmaCtx)
{
   DIBSTATUS Status = DIBSTATUS_SUCCESS;
   unsigned char Disable32bitDma = 0;

#if (HIGH_SPEED_DMA == 1)

  /*** Voyager Host issue is supported => Disable 32 bits DMA when the chip used is Voyager ***/
#if (ENG3_COMPATIBILITY == 1)
   if (pContext->DibChip == DIB_VOYAGER)
   {
     Disable32bitDma = 1;
   }
#else
  /*** Voyager Host issue is not supported => 32 bits DMA can be used ***/
   Disable32bitDma = 0;
#endif

#else
   /*** HIGH_SPEED_DMA not supported ***/
   Disable32bitDma = 1;
#endif

   /*** Use 32 bits transfer. Check alignment ***/
   if (Disable32bitDma == 0)
   {
     uint32_t NbBytes, j;
     pDmaCtx->Mode = DIBBRIDGE_BIT_MODE_32;
     pDmaCtx->DmaSize = pDmaCtx->DmaLen;
  
     /*--------------  Software management of alignement issues -----------------------*/
  
     /* particular case where only one 32 bit word is involved */
     if((pDmaCtx->ChipAddr & 0xFFFFFFFC) == (((pDmaCtx->ChipAddr+pDmaCtx->DmaSize-1) & 0xFFFFFFFC)))
     {
        for(j=0; j<pDmaCtx->DmaSize; j++)
        {
           if(pDmaCtx->Dir == DIBBRIDGE_DMA_READ)
                Status = DibBridgeReadReg8(pContext, pDmaCtx->ChipAddr + j, pDmaCtx->pHostAddr + j);
           else
              Status = DibBridgeWriteReg8(pContext, pDmaCtx->ChipAddr + j, pDmaCtx->pHostAddr[j]);

           if(Status != DIBSTATUS_SUCCESS)
           {
              DIB_DEBUG(PORT_ERR, (CRB "DibBridgeDragonflySetupDma Failed Idx%d Nb %d " CRA, j, pDmaCtx->DmaSize));
              return Status;
           }
        }
        pDmaCtx->ChipAddr += pDmaCtx->DmaSize;
        pDmaCtx->pHostAddr += pDmaCtx->DmaSize;
        pDmaCtx->DmaSize = 0;
     }
     /* general case: alignement issues are at the beginning and at the end */
     else
     {
        /* beginning */
        if(pDmaCtx->ChipAddr & 3)
        {
             NbBytes = 4 - (pDmaCtx->ChipAddr & 3);
           for(j=0; j<NbBytes; j++)
           {
              if(pDmaCtx->Dir == DIBBRIDGE_DMA_READ)
                 Status = DibBridgeReadReg8(pContext, pDmaCtx->ChipAddr + j, pDmaCtx->pHostAddr + j);
              else
                 Status = DibBridgeWriteReg8(pContext, pDmaCtx->ChipAddr + j, pDmaCtx->pHostAddr[j]);
  
              if(Status != DIBSTATUS_SUCCESS)
              {
                 DIB_DEBUG(PORT_ERR, (CRB "DibBridgeDragonflySetupDma Failed Idx%d Nb %d " CRA, j, NbBytes));
                 return Status;
              }
  
           }
           pDmaCtx->DmaSize -= NbBytes;
           pDmaCtx->ChipAddr += NbBytes;
           pDmaCtx->pHostAddr += NbBytes;
        }
  
        /* at the end */
        if((pDmaCtx->ChipAddr+pDmaCtx->DmaSize) & 3)
        {
           NbBytes = ((pDmaCtx->ChipAddr + pDmaCtx->DmaSize) & 3);
           /* do not transfert these NbBytes in the main transfert */
           pDmaCtx->DmaSize -= NbBytes;
           for(j=0; j<NbBytes; j++)
           {
              if(pDmaCtx->Dir == DIBBRIDGE_DMA_READ)
                 Status = DibBridgeReadReg8(pContext, pDmaCtx->ChipAddr + pDmaCtx->DmaSize + j, pDmaCtx->pHostAddr + pDmaCtx->DmaSize + j);
              else
                 Status = DibBridgeWriteReg8(pContext, pDmaCtx->ChipAddr + pDmaCtx->DmaSize + j, pDmaCtx->pHostAddr[j + pDmaCtx->DmaSize]);
  
              if(Status != DIBSTATUS_SUCCESS)
              {
                 DIB_DEBUG(PORT_ERR, (CRB "DibBridgeDragonflySetupDma Failed Idx%d Nb %d " CRA, j, NbBytes));
                 return Status;
              }
           }
        }
     }
   }
   else
   {
     /*** Force 8 bits transfers ***/
     pDmaCtx->Mode    = DIBBRIDGE_BIT_MODE_8;
     pDmaCtx->DmaSize = pDmaCtx->DmaLen;
   }

   /** Compute formatted chip address */
   pDmaCtx->FmtChipAddr = DibBridgeDragonflyFormatAddress(pContext, pDmaCtx->ChipAddr, pDmaCtx->Mode);

   return Status;
}



/****************************************************************************
 * Really start target dma. Swap buffer if really need.
 * ############ WARNING: host buffer will be modified #######################
 ****************************************************************************/
DIBDMA DibBridgeDragonflyRequestDma(struct DibBridgeContext *pContext, struct DibBridgeDmaCtx * pDmaCtx)
{
   DIBDMA rc;

   rc = DibBridgeTargetRequestDma(pContext, pDmaCtx);
   return rc;
}

/**
* Sends a message to SPARC
*  Warning!!!!!! the message MUST be a set of uint32_t or int32_t, and the use of
* bit Mask is forbidden cause behave differently on little and big endian arch
* @param pContext: bridge context
* param Data: aligned 32 bits Data pointer. Reference the whole message
* param Size: number of bytes of the message
* WARNING: the Data buffer can be swapped by this function and should not be used after this call!!!
*/
static DIBSTATUS DibBridgeDragonflySendMsg(struct DibBridgeContext *pContext, uint32_t * Data, uint32_t Size)
{
   uint8_t  Status = DIBSTATUS_ERROR;
   int32_t  MaxRetries = DIB_BRIDGE_MAX_MAILBOX_TRY;
   uint32_t Rdptr;
   uint32_t Wrptr;
   uint32_t Free;

   DIB_ASSERT((Size & 3) == 0);

   DIB_ASSERT((Data));
   DIB_ASSERT(Size >= 4);

   DIB_DEBUG(MAILBOX_LOG, (CRB "+SendMsg() Request=> Msg : %x, Size %d" CRA, *Data, Size));

   /* Check if there is space in msgbox */
   if((Status = DibBridgeReadReg32(pContext, pContext->DragonflyRegisters.MacMbxWrPtrReg, &Wrptr)) != DIBSTATUS_SUCCESS)
      goto End;

   DIB_ASSERT((Wrptr & 3) == 0);
   /* Ensure we have enought place in the mailbow to post this message */
   while(MaxRetries > 0)
   {
      /* Get MAC read pointer (implemented as follower) */
      if((Status = DibBridgeRead32Reg32(pContext, pContext->DragonflyRegisters.MacMbxRdPtrReg, &Rdptr)) != DIBSTATUS_SUCCESS)
         goto End;

      DIB_ASSERT((Rdptr & 3) == 0);
      DIB_ASSERT((Rdptr >= pContext->DragonflyRegisters.MacMbxStart && Rdptr < pContext->DragonflyRegisters.MacMbxEnd));

      /* Do not allow to write last byte, this is to avoid overflow when rd==wr msg box is empty */
      Free = IntBridgeDragonflyMailboxSpace(Rdptr, Wrptr, pContext->DragonflyRegisters.MacMbxSize);

      /* get the number of 32 bits words available */
      if(Size > Free)
      {
         DibMSleep(1);
         MaxRetries--;
      }
      else
      {
         /* break successfully the loop */
         MaxRetries=-1;
      }
   }

   if(MaxRetries < 0)
   {
      struct MsgHeader MsgIn;
      DIB_DEBUG(MAILBOX_LOG, (CRB "SendMsg() %d bytes available in msg box." CRA, Free));

      SerialBufInit(&pContext->RxSerialBuf, Data, 32);
      MsgHeaderUnpack(&pContext->RxSerialBuf, &MsgIn);

      if(Wrptr + Size > pContext->DragonflyRegisters.MacMbxEnd)
      {
         uint32_t len;
         /* Transfer must be done in two step */
         len = pContext->DragonflyRegisters.MacMbxEnd - Wrptr;
         if((Status = DibBridgeWriteBuffer32(pContext, Wrptr, Data, len)) != DIBSTATUS_SUCCESS)
            goto End;
         if((Status = DibBridgeWriteBuffer32(pContext, pContext->DragonflyRegisters.MacMbxStart, Data + (len >> 2), Size - len)) != DIBSTATUS_SUCCESS)
            goto End;
         Wrptr = pContext->DragonflyRegisters.MacMbxStart + Size - len;
      }
      else
      {
         /* Transfer can be done in a single step */
         if((Status = DibBridgeWriteBuffer32(pContext, Wrptr, Data, Size)) != DIBSTATUS_SUCCESS)
            goto End;
         Wrptr += Size;
      }

      if(Wrptr == pContext->DragonflyRegisters.MacMbxEnd)
         Wrptr = pContext->DragonflyRegisters.MacMbxStart;

      /* Update rd pointer (this trigger an irq in the firmware) */
      Status = DibBridgeWriteReg32(pContext, pContext->DragonflyRegisters.MacMbxWrPtrReg, Wrptr);

      if (MsgIn.MsgId == OUT_MSG_UDIBADAPTER_CFG)
          DibMSleep(50);
   }
   else
   {
      DIB_DEBUG(MAILBOX_ERR, (CRB "-SendMsg() Failed Msg box full" CRA));
      Status = DIBSTATUS_ERROR;
   }

   DIB_DEBUG(MAILBOX_LOG, (CRB "-SendMsg()" CRA));

End:
   return Status;
}

/****************************************************************************
 * Clear HW interrupt at host interface level
 ****************************************************************************/
static __inline DIBSTATUS IntBridgeDragonflyClearHostIrq(struct DibBridgeContext *pContext)
{
   DIBSTATUS status = DIBSTATUS_SUCCESS;

#if CLEAR_HOST_IRQ_MODE == CLEAR_BY_MESSAGE
   /* Workaround for concurrent access to apb and demod */
   struct MsgHeader MsgOut;

   DIB_DEBUG(MAILBOX_LOG, (CRB "DibBridgeDragonflySendAck" CRA));

   /* Message header */
   MsgOut.Type      = MSG_TYPE_MAC;
   MsgOut.MsgId     = OUT_MSG_CLEAR_HOST_IRQ;
   MsgOut.MsgSize   = GetWords(MsgHeaderBits, 32);
   MsgOut.ChipId    = MASTER_IDENT;

   MsgHeaderPackInit(&MsgOut, &pContext->TxSerialBuf);

   status = DibBridgeDragonflySendMsg(pContext, pContext->TxBuffer, MsgOut.MsgSize * 4);
#endif
#if CLEAR_HOST_IRQ_MODE == CLEAR_BY_REGISTER
   uint32_t tmp;
   status = DibBridgeReadReg32(pContext, REG_HIF_INT_STAT, &tmp);   /* Clear HW IRQ */
#endif

   return status;
}

/****************************************************************************
 * There was an interrupt. Let's check the necessary action
 ****************************************************************************/
static DIBDMA DibBridgeDragonflyProcessIrq(struct DibBridgeContext *pContext)
{
   DIBDMA DmaStatus = DIB_NO_IRQ;
   struct MsgHeader MsgIn;
   uint32_t * RxData;

   if(pContext->RxCnt == 0)
   {
#if (INTERRUPT_MODE != USE_POLLING)
      /* clear hardware interrrupt in anycase since we received interrupt */
      if(IntBridgeDragonflyClearHostIrq(pContext) != DIBSTATUS_SUCCESS)
         return DIB_DEV_FAILED; /* Device failed to respond */
#endif

      DIB_ASSERT(pContext->HostBuffer);
      pContext->RxOffset = 0;

      /* Read N messages if possible */
      pContext->RxCnt = DibBridgeDragonflyReceiveMsg(pContext, pContext->HostBuffer);
   }

   /* Process the N messages */
   if(pContext->RxCnt >  0)
   {
      RxData = &pContext->HostBuffer[pContext->RxOffset];
      SerialBufInit(&pContext->RxSerialBuf, RxData, 32);
      MsgHeaderUnpack(&pContext->RxSerialBuf, &MsgIn);
      SerialBufRestart(&pContext->RxSerialBuf);

      if((pContext->RxCnt < (MsgIn.MsgSize>>2)) || (MsgIn.MsgSize > pContext->DragonflyRegisters.HostMbxSize))
      {
         DIB_DEBUG(MAILBOX_ERR, (CRB "+RecvMsg() => ERROR: MsgSize %d" CRA, MsgIn.MsgSize));
         DIB_DEBUG(MAILBOX_ERR,(CRB "NbBytes received %d" CRA,pContext->RxCnt));
         pContext->RxCnt = 0;
      }
      else
      {
         DmaStatus = DibBridgeDragonflyMsgHandler(pContext, &MsgIn, RxData);
         pContext->RxOffset += MsgIn.MsgSize;
         pContext->RxCnt -= (MsgIn.MsgSize << 2);
      }
   }

   return DmaStatus;
}


/**
* Reads ONE message from one of the risc
* @param pContext: bridge context
* @param Data: Buffer owning the message, header included
* @param nb_words: number of 32 bit words available in the mailbox
* @return number of 32 bit words of the message
*/
static uint32_t DibBridgeDragonflyReceiveMsg(struct DibBridgeContext *pContext, uint32_t * Data)
{
   uint32_t NbBytes;
   uint32_t rdptr;
   uint32_t wrptr;

   DIBSTATUS Status = DIBSTATUS_SUCCESS;

   /* Check if there is space in msgbox */
   if((Status = DibBridgeReadReg32(pContext, pContext->DragonflyRegisters.HostMbxRdPtrReg, &rdptr) != DIBSTATUS_SUCCESS))
      goto End;

   if((Status = DibBridgeRead32Reg32(pContext, pContext->DragonflyRegisters.HostMbxWrPtrReg, &wrptr) != DIBSTATUS_SUCCESS))
      goto End;

   //DIB_ASSERT((wrptr & 3) == 0);

   NbBytes = IntBridgeDragonflyMailboxBytes(rdptr, wrptr, pContext->DragonflyRegisters.HostMbxSize);
   //DIB_ASSERT((NbBytes & 3) == 0);

   if(NbBytes > 0)
   {
      uint32_t len = 0;
      DIB_DEBUG(IRQ_LOG, (CRB "IRQ HOST, NbBytes in mailbox = %d" CRA, NbBytes));

      if(rdptr + NbBytes > pContext->DragonflyRegisters.HostMbxEnd)
      {
         /* The mailbox must be read in two parts */
         len = pContext->DragonflyRegisters.HostMbxEnd - rdptr;

         if((Status = DibBridgeReadBuffer32(pContext, rdptr, Data, len)) != DIBSTATUS_SUCCESS)
            goto End;
         if((Status = DibBridgeReadBuffer32(pContext, pContext->DragonflyRegisters.HostMbxStart, Data + (len / 4), NbBytes - len)) != DIBSTATUS_SUCCESS)
            goto End;
      }
      else
      {
         /* The mailbox can be read in one pass */
         if((Status = DibBridgeReadBuffer32(pContext, rdptr, Data, NbBytes)) != DIBSTATUS_SUCCESS)
            goto End;
      }

      if((Status = DibBridgeWriteReg32(pContext, pContext->DragonflyRegisters.HostMbxRdPtrReg, wrptr) != DIBSTATUS_SUCCESS))
         goto End;
   }

End:
   if(Status == DIBSTATUS_SUCCESS)
      return NbBytes;
   else
      return 0;
}

/****************************************************************************
 * Parses and processes the most prioritary messages, and passes the others
 * to the upper layer. Returns the DMA state: no DMA. done or pending.
 ****************************************************************************/
#if (mSDK==0)
static DIBDMA DibBridgeDragonflyDataMsgHandler(struct DibBridgeContext * pContext)
{
   struct DibBridgeDmaFlags flags;
   struct MsgData MsgIn;
   DIBDMA                   DmaStat;

   MsgDataUnpack(&pContext->RxSerialBuf, &MsgIn);

/*
   struct timeval Time;
   gettimeofday(&Time, NULL);
*/
   flags.Type     = MSG_DATA_TYPE(MsgIn.Format);
   flags.ItemHdl  = MSG_DATA_ITEM_INDEX(MsgIn.Format);
   flags.BlockId  = MSG_DATA_BLOCK_ID(MsgIn.Format);
   flags.BlockType= MSG_DATA_BLOCK_TYPE(MsgIn.Format);
   flags.FirstFrag= MSG_DATA_FIRST_FRAG(MsgIn.Format);
   flags.LastFrag = MSG_DATA_LAST_FRAG(MsgIn.Format);
   flags.NbRows   = MSG_DATA_NB_ROWS(MsgIn.Format);
   flags.FrameId  = MSG_DATA_FRAME_ID(MsgIn.Format);

   if(flags.Type == FORMAT_MPE || flags.Type == FORMAT_LAST_FRG || flags.Type == FORMAT_FLUSH_SVC)
   {
      flags.Prefetch = MSG_DATA_BLOCK_TYPE(MsgIn.Format);
   }
   else
   {
      flags.Prefetch = 0;
   }

   DIB_DEBUG(RAWTS_LOG, (CRB "Min %d Max %d Addr %08x Len %d Rows %d FLAGS : s %d t %d b %d ff %d lf %d frm %d fw %08x" CRA,
                         MsgIn.Min, MsgIn.Max, MsgIn.Add, MsgIn.Len, flags.NbRows,
                         flags.ItemHdl, flags.Type, flags.BlockId, flags.FirstFrag, flags.LastFrag, flags.FrameId, MsgIn.Min));

/*
   DIB_DEBUG(RAWTS_LOG, (CRB "%d : %d IN_MSG_DATA Type %d len %u" CRA, (int)Time.tv_sec, (int)Time.tv_usec, flags.Type, MsgIn.Len));
   DIB_DEBUG(RAWTS_ERR, (CRB "%d : %f IN_MSG_DATA Type %d len %u" CRA, (int)Time.tv_sec, (float)((float)Time.tv_usec/1000.0f), flags.Type, MsgIn.Len));*/
/* DIB_DEBUG(RAWTS_LOG, (CRB CRB "" CRA CRA));
   DIB_DEBUG(RAWTS_LOG, (CRB "---" CRA));*/
   DmaStat = DibBridgeHighDataMsgHandlerCommon(pContext, MsgIn.Min, MsgIn.Max, MsgIn.Add, MsgIn.Len, &flags);

   if(DmaStat == DIB_NO_DMA)
   {
      DIB_DEBUG(MAILBOX_LOG, (CRB "Spec: Received unknown Type for Data message: %d" CRA, flags.Type));
   }

   return DmaStat;      /* Tells do we have a pending DMA or not */
}
#endif

static DIBDMA DibBridgeDragonflyMsgHandler(struct DibBridgeContext *pContext, struct MsgHeader * pHeader, uint32_t * RxData)
{
   if(pHeader->MsgSize > 0)
   {
      DIB_DEBUG(MAILBOX_LOG, (CRB "+RecvMsg() => Msg : id %d, Size %d" CRA, pHeader->MsgId, pHeader->MsgSize));

      /* ------------------------------------------------------------------------------------ */
      /* Now we have one message, let's check the Type of it */
      DIB_DEBUG(IRQ_LOG, (CRB "IRQ: MSG %d, Size %d" CRA, pHeader->MsgId, pHeader->MsgSize));

      /* ------------------------------------------------------------------------------------ */
      /* It can be either Data (0), CPT (1) or a message to passed up (>= 2) */
      /* ------------------------------------------------------------------------------------ */
#if (mSDK == 0)

      if(pHeader->MsgId == IN_MSG_DATA && pHeader->Type == MSG_TYPE_MAC)
      {
         /* This is Data message */
         return DibBridgeDragonflyDataMsgHandler(pContext);
      }

      /* ------------------------------------------------------------------------------------ */
      /*This is an Info message */
      else if(pHeader->MsgId == IN_MSG_FRAME_INFO && pHeader->Type == MSG_TYPE_MAC)
      {
         return DibBridgeDragonflyInfoMsgHandler(pContext, RxData, pHeader->MsgSize);
      }

      else if(pHeader->MsgId == IN_MSG_CTL_MONIT && pHeader->Type == MSG_TYPE_MAC)
      {
#if (DIB_CHECK_DATA == 1)
         /* Clear Bridge checker statistics */
         DibBridgeDragonflyClearCheckStats(pContext, RxData);
#endif
      }
      else
      {
         /* flush buffers after item removal */
         if(pHeader->MsgId == IN_MSG_ACK_FREE_ITEM && pHeader->Type == MSG_TYPE_MAC)
            DibBridgeFreeUnusedMpeBuffer(pContext);

         /* Other: the whole message will be passed up */
         DIB_DEBUG(MAILBOX_LOG, (CRB "MSG IN (%d) forwarded " CRA, pHeader->MsgId));
         DibB2DFwdMsg(pContext, (pHeader->MsgSize << 2) /*in bytes*/, (uint16_t*)RxData);
      }
#else
      DIB_DEBUG(MAILBOX_LOG, (CRB "MSG IN (%d) forwarded " CRA, pHeader->MsgId));
      DibB2DFwdMsg(pContext, (pHeader->MsgSize << 2) /*in bytes*/, (uint16_t*)RxData);
#endif
   }


   /* ------------------------------------------------------------------------------------ */
   return DIB_NO_DMA;
}

/****************************************************************************
* Checks message coming from the RISC and acts appropriately
****************************************************************************/
static DIBSTATUS DibBridgeDragonflySendAck(struct DibBridgeContext *pContext, struct DibBridgeDmaFlags *pFlags, uint8_t failed)
{
   struct MsgAckData MsgOut;

   DIB_DEBUG(MAILBOX_LOG, (CRB "" CRA));
   DIB_DEBUG(MAILBOX_LOG, (CRB "DibBridgeDragonflySendAck" CRA));

   /* Message header */
   MsgOut.Head.Type      = MSG_TYPE_MAC;
   MsgOut.Head.MsgId     = (failed > 1) ? 1 : 0; /* DF1 - RESET  ;  DF0 - ACK */
   MsgOut.Head.MsgSize   = GetWords(MsgAckDataBits, 32);
   MsgOut.Head.ChipId    = MASTER_IDENT;

   MsgOut.Status         = failed;
   /*pContext->FecOffset is not used for dragonfly based chipset */
   MsgOut.Format         = SET_DATA_FORMAT(pFlags->ItemHdl,
                                           pFlags->Type,
                                           pFlags->FirstFrag,
                                           pFlags->LastFrag,
                                           pFlags->NbRows,
                                           pFlags->BlockType,
                                           pFlags->BlockId,
                                           pFlags->FrameId);

   MsgAckDataPackInit(&MsgOut, &pContext->TxSerialBuf);

   return DibBridgeDragonflySendMsg(pContext, pContext->TxBuffer, MsgOut.Head.MsgSize * 4);
}

/******************************************************************************
 * Dma if finished, acknowledge the firmware and do the job
 ******************************************************************************/
#if (mSDK == 0)
static DIBSTATUS DibBridgeDragonflyProcessDma(struct DibBridgeContext *pContext, struct DibBridgeDmaCtx * pDmaCtx)
{
   DIBSTATUS ret = DIBSTATUS_ERROR;

   /* every Data message need to be acknowledged */
   ret = DibBridgeDragonflySendAck(pContext, &pDmaCtx->DmaFlags, 0);

   if(ret == DIBSTATUS_SUCCESS)
   {
      /* process dma independantly of the architecture */
      ret = DibBridgeProcessDmaCommon(pContext, pDmaCtx);
   }

   return ret;
}
#endif


/******************************************************************************
 * 32 bit address formating for all dragonfly based chipsets
 ******************************************************************************/
static uint32_t DibBridgeDragonflyFormatAddress(struct DibBridgeContext *pContext, uint32_t Addr, uint8_t ByteMode)
{
   switch(pContext->HostIfMode)
   {
      case eSRAM:
         return DF_ADDR_TO_SRAM(Addr, ByteMode, 1, 0);

      case eSDIO:
         return DF_ADDR_TO_SDIO(Addr, 1);

      case eSPI:
         return DF_ADDR_TO_SPI(Addr, ByteMode, 1);

 	 default:
         return Addr;
   }
}


/******************************************************************************
 * 16 bit access to non demod apb address is not working on voyager chipset
 ******************************************************************************/
static DIBSTATUS IntBridgeVoyagerWrite16Even(struct DibBridgeContext *pContext, uint32_t Address, uint8_t *b, uint32_t len)
{
   uint8_t  wa[4] = { 0 };
   uint32_t i, FormattedAddr;
   DIBSTATUS  ret = DIBSTATUS_SUCCESS;

   for (i = 0; i < len; i += 2)
   {
      FormattedAddr = DibBridgeDragonflyFormatAddress(pContext, Address + i, DIBBRIDGE_BIT_MODE_32);

      wa[0] = b[i];
      wa[1] = b[i + 1];

      if((ret = DibBridgeTargetWrite(pContext, FormattedAddr, DIBBRIDGE_BIT_MODE_32, 4, wa) != DIBSTATUS_SUCCESS))
         break;
   }

   return ret;
}

static DIBSTATUS IntBridgeVoyagerRead16Even(struct DibBridgeContext *pContext, uint32_t Address, uint8_t *b, uint32_t len)
{
   uint8_t  wa[4] = { 0 };
   uint32_t i, FormattedAddress;
   DIBSTATUS  ret = DIBSTATUS_SUCCESS;

   for (i = 0; i < len; i += 2)
   {
      FormattedAddress = DibBridgeDragonflyFormatAddress(pContext, Address + i, DIBBRIDGE_BIT_MODE_32);

      if((ret = DibBridgeTargetRead(pContext, FormattedAddress, DIBBRIDGE_BIT_MODE_32, 4, wa) != DIBSTATUS_SUCCESS))
         break;

      b[i]   = wa[0];
      b[i+1] = wa[1];
   }

   return ret;
}

static DIBSTATUS DibBridgeVoyager1PreFormat(struct DibBridgeContext *pContext, uint8_t ByteMode, uint32_t * Addr, uint8_t IsWriteAccess, uint8_t * Buf, uint32_t Cnt)
{
   if(DIB29000_APB_EVEN_ADDR(*Addr, ByteMode))
   {
      if(IsWriteAccess)
         return IntBridgeVoyagerWrite16Even(pContext, *Addr, Buf, Cnt);
      else
         return IntBridgeVoyagerRead16Even(pContext, *Addr, Buf, Cnt);
   }

   /* address formating */
   *Addr = DibBridgeDragonflyFormatAddress(pContext, *Addr, ByteMode);

   return DIBSTATUS_CONTINUE;
}

static DIBSTATUS DibBridgeVoyager1PostFormat(struct DibBridgeContext *pContext, uint8_t ByteMode, uint32_t * Addr, uint8_t IsWriteAccess, uint8_t * Buf, uint32_t Cnt)
{
   return DIBSTATUS_SUCCESS;
}

static DIBSTATUS DibBridgeNautilus1PreFormat(struct DibBridgeContext *pContext, uint8_t ByteMode, uint32_t * Addr, uint8_t IsWriteAccess, uint8_t * Buf, uint32_t Cnt)
{
   /* address formating */
   *Addr = DibBridgeDragonflyFormatAddress(pContext, *Addr, ByteMode);

   return DIBSTATUS_CONTINUE;
}

static DIBSTATUS DibBridgeNautilus1PostFormat(struct DibBridgeContext *pContext, uint8_t ByteMode, uint32_t * Addr, uint8_t IsWriteAccess, uint8_t * Buf, uint32_t Cnt)
{
   return DIBSTATUS_SUCCESS;
}

static uint32_t DibBridgeDragonflyIncrementFormattedAddress(struct DibBridgeContext *pContext, uint32_t InFmtAddr, int32_t Offset)
{
   uint32_t OutFmtAddr = 0, ByteMode, Addr;

   switch(pContext->HostIfMode)
   {
      case eSRAM:
         Addr = DF_SRAM_TO_ADDR(InFmtAddr);
         Addr += Offset;
         ByteMode = (InFmtAddr & 0x06000000) >> 25;
         OutFmtAddr = DF_ADDR_TO_SRAM(Addr, ByteMode, 1, 0);
         break;

      case eSDIO:
         Addr = DF_SDIO_TO_ADDR(InFmtAddr);
         Addr += Offset;
         OutFmtAddr = DF_ADDR_TO_SDIO(Addr, 1);
         break;

      case eI2C:
         OutFmtAddr = InFmtAddr+Offset;
         break;

      case eSPI:
         Addr = DF_SPI_TO_ADDR(InFmtAddr);
         Addr += Offset;
         ByteMode = (InFmtAddr & 0x30000000) >> 28;
         OutFmtAddr = DF_ADDR_TO_SPI(Addr, ByteMode, 1);
         break;

      default:
         break;
   }
   return OutFmtAddr;
}

/******************************************************************************
* Assemble the slice from MpeBufCor to SliceBuf, and set SkipR and SKipC if not already set
******************************************************************************/

/*
void DisplaySliceBuf(uint8_t *pSliceBuf, uint32_t NbRows, uint32_t NbCols)
{
   uint32_t i,j;
   printf(CRB "" CRA);
   for(i=0; i<NbRows; i++)
   {
      for(j=0; j<NbCols; j++)
      {
         printf("%02x ",pSliceBuf[i+j*NbRows]);
      }
      printf(CRB "" CRA);
   }
   printf(CRB "" CRA);
}
*/


/******************************************************************************
 * return the bus architecture (32, 16 or 8 bits)
 ******************************************************************************/
static uint8_t DibBridgeDragonflyGetArchi(struct DibBridgeContext *pContext)
{
   return DIBBRIDGE_BIT_MODE_32;
}

/******************************************************************************
 * clean checker statistics
 ******************************************************************************/
#if (DIB_CHECK_DATA == 1)
static void DibBridgeDragonflyClearCheckStats(struct DibBridgeContext *pContext, uint32_t * RxData)
{
   enum DibDataType  FilterType;
   struct MsgCtrlMonit Msg;
   ELEM_HDL ItemHdl;
   FILTER_HDL FilterHdl;

   MsgCtrlMonitUnpack(&pContext->RxSerialBuf, &Msg);

   ItemHdl = Msg.ItemId;

   /* When ClearMonit message, clear Bridge monitoring info */
   if(Msg.Cmd == 1)
   {
      FilterHdl = pContext->ItSvc[ItemHdl].FilterParent;
      FilterType = pContext->FilterInfo[ItemHdl].DataType;

      /* DVB-H: Clear IP and RTP checker data */
      if((FilterType == eMPEFEC) || (FilterType == eMPEIFEC))
      {
         pContext->ItSvc[ItemHdl].CcFailCnt     = 0;
         pContext->ItSvc[ItemHdl].ErrCnt        = 0;
         pContext->ItSvc[ItemHdl].CurCc         = 0xffff;
      }
      if((FilterType == eDAB))
      {
         pContext->ItSvc[ItemHdl].CcFailCnt     = 0;
         pContext->ItSvc[ItemHdl].ErrCnt        = 0;
         pContext->ItSvc[ItemHdl].CurCc         = 0;
         pContext->ItSvc[ItemHdl].DataLenRx     = 0;
         pContext->ItSvc[ItemHdl].NbMaxFrames   = 0;
      }
#if DIB_CHECK_CMMB_DATA == 1
      else if(FilterType == eCMMBSVC)
      {
         pContext->ItSvc[ItemHdl].CcFailCnt     = 0;
         pContext->ItSvc[ItemHdl].ErrCnt        = 0;
         pContext->ItSvc[ItemHdl].CurCc         = 0xffff;
      }
#endif
#if DIB_CHECK_RAWTS_DATA == 1
      /* DVB-T: Clear RAWTS checker data */
      else if (FilterType == eTS)
      {
         DibSetMemory(&pContext->FilterInfo[FilterHdl].CheckRawTs, 0, sizeof(struct CheckRawTs));
      }
#endif

      DIB_DEBUG(MAILBOX_LOG, (CRB "Clear checker stats for Item %d" CRA, ItemHdl));
   }
}

/** Build a message for driver to summarize ip checking */
/* XXX this should not go in the official release */
static void DibBridgeDragonflyForwardCheckStats(struct DibBridgeContext *pContext, ELEM_HDL Item)
{
   struct MsgChecker Msg;

   FILTER_HDL Filter = pContext->ItSvc[Item].FilterParent;

   /* Message header */
   Msg.Head.Type      = MSG_TYPE_MAC;
   Msg.Head.MsgId     = IN_MSG_CHECKER;
   Msg.Head.MsgSize   = GetWords(MsgCheckerBits, 32);
   Msg.Head.ChipId    = HOST_IDENT;

   Msg.ItemId         = Item;

#if DIB_CHECK_RAWTS_DATA == 1
   if((pContext->FilterInfo[Filter].DataType == eTS) || (pContext->FilterInfo[Filter].DataType == eTDMB))
   {
      Msg.Total         = pContext->FilterInfo[Filter].CheckRawTs.TotalNbPackets;
      Msg.CcFailCnt     = pContext->FilterInfo[Filter].CheckRawTs.DiscontinuitiesCount;
      Msg.ErrCnt        = pContext->FilterInfo[Filter].CheckRawTs.CorruptedPacketsCount;
   }
   else
#endif
#if DIB_CHECK_CMMB_DATA == 1
   if(pContext->FilterInfo[Filter].DataType == eCMMBSVC)
   {
      Msg.CcFailCnt     = pContext->ItSvc[Item].CcFailCnt;
      Msg.ErrCnt        = pContext->ItSvc[Item].ErrCnt;
   }
   else
#endif
#if DIB_CHECK_MSC_DATA == 1
   if(pContext->FilterInfo[Filter].DataType == eDAB)
   {
#if DIB_DAB_DATA == 1
      Msg.Total         = pContext->ItSvc[Item].NbMaxFrames;
#endif
      Msg.CcFailCnt     = pContext->ItSvc[Item].CcFailCnt;
      Msg.ErrCnt        = pContext->ItSvc[Item].ErrCnt;
   }
   else
#endif
   if ((pContext->FilterInfo[Filter].DataType == eMPEFEC) || (pContext->FilterInfo[Filter].DataType == eMPEIFEC))
   {
      Msg.CcFailCnt     = pContext->ItSvc[Item].CcFailCnt;
      Msg.ErrCnt        = pContext->ItSvc[Item].ErrCnt;
   }
   else
   {
      DIB_DEBUG(MAILBOX_ERR, (CRB "DibBridgeDragonflyForwardCheckStats : unsupported data type" CRA));
      return;
   }

   MsgCheckerPackInit(&Msg, &pContext->TxSerialBuf);

   DibB2DFwdMsg(pContext, Msg.Head.MsgSize * 4, (uint16_t *)pContext->TxBuffer);
}
#endif

/**
 * associate svc to item. Nothing to do cause we have no idea of what is a svc
 * @param pContext   pointer to the bridge context
 * @param svc        firefly's service (only useful in firefly's case)
 * @param item       item's number concerned
 */
static void DibBridgeDragonflySetService(struct DibBridgeContext *pContext, uint8_t Svc, ELEM_HDL ItemHdl, FILTER_HDL FilterHdl, enum DibDataType DataType, enum DibDataMode DataMode)
{
}

/**
 * Indicate to the firmware that buffer reception is aborted due to buffer overflow or memory consideration
 * @param pContext: bridge context
 * @param SvcNb: service number that failed
 */
static DIBSTATUS DibBridgeDragonflySignalBufFail(struct DibBridgeContext *pContext, struct DibBridgeDmaFlags * pFlags, uint8_t Flush)
{
   return DibBridgeDragonflySendAck(pContext, pFlags, (1+Flush));
}

#if (DIB_BRIDGE_HBM_PROFILER == 1)
/**
 * Send profiler info to the SPARC
 */
static DIBSTATUS DibBridgeDragonflyHbmProfiler(struct DibBridgeContext *pContext, uint8_t idx, uint8_t page, uint8_t LastFrag)
{
   return DIBSTATUS_ERROR;
}
#endif

/******************************************************************************
 * Configure or reconfigure SDIO endianess
 ******************************************************************************/
void DibBridgeDragonflyConfigureSdioEndianness(struct DibBridgeContext *pContext)
{
   uint32_t Jedec32;
   uint16_t Jedec16;
   uint32_t InvJedec;

   InvJedec = pContext->DragonflyRegisters.JedecValue;
   DibBridgeSwap32((uint8_t*)&InvJedec, 4);

   /* toggle sdio endianess if default configuration is not good */
   DibBridgeReadReg32(pContext, pContext->DragonflyRegisters.JedecAddr, &Jedec32);
   DibBridgeReadReg16(pContext, pContext->DragonflyRegisters.JedecAddr, &Jedec16);
   if((Jedec32 == InvJedec) || (Jedec16 == InvJedec >> 16))
   {
      DibBridgeWriteReg32(pContext, REG_HIF_SDIO_IRQ_EN, 0x0A000000);
   }
}

/******************************************************************************
 * Init nautilus software specific
 ******************************************************************************/
DIBSTATUS DibBridgeDragonflyChipsetInit(struct DibBridgeContext *pContext)
{
   pContext->RxCnt = 0;
   pContext->RxOffset = 0;
   if((pContext->HostBuffer = (uint32_t *)DibBridgeTargetAllocBuf(pContext->DragonflyRegisters.HostMbxSize)) == 0)
   {
      return DIBSTATUS_ERROR;
   }
   DibBridgeDragonflyConfigureSdioEndianness(pContext);
/*
   printf("DibBridgeDragonflyChipsetInit:\n");
   printf("  JedecAddr        = %08x\n", pContext->DragonflyRegisters.JedecAddr   );
   printf("  JedecValue       = %08x\n", pContext->DragonflyRegisters.JedecValue    );
   printf("  MAC_MBOX_SIZE    = %d\n"  , pContext->DragonflyRegisters.MacMbxSize   );
   printf("  MAC_MBOX_END     = %08x\n", pContext->DragonflyRegisters.MacMbxEnd    );
   printf("  MAC_MBOX_START   = %08x\n", pContext->DragonflyRegisters.MacMbxStart  );
   printf("  HOST_MBOX_SIZE   = %d\n"  , pContext->DragonflyRegisters.HostMbxSize  );
   printf("  HOST_MBOX_END    = %08x\n", pContext->DragonflyRegisters.HostMbxEnd   );
   printf("  HOST_MBOX_START  = %08x\n", pContext->DragonflyRegisters.HostMbxStart );
   printf("  HOST_MBOX_RD_PTR = %08x\n", pContext->DragonflyRegisters.HostMbxRdPtrReg);
   printf("  HOST_MBOX_WR_PTR = %08x\n", pContext->DragonflyRegisters.HostMbxWrPtrReg);
   printf("  MAC_MBOX_RD_PTR  = %08x\n", pContext->DragonflyRegisters.MacMbxRdPtrReg );
   printf("  MAC_MBOX_WR_PTR  = %08x\n", pContext->DragonflyRegisters.MacMbxWrPtrReg );
*/
   return DIBSTATUS_SUCCESS;
}

/******************************************************************************
 * Deinit dragonfly and voyager software specific
 ******************************************************************************/
void DibBridgeDragonflyChipsetDeinit(struct DibBridgeContext *pContext)
{
   DibBridgeTargetFreeBuf((uint8_t *)pContext->HostBuffer, pContext->DragonflyRegisters.HostMbxSize);
}

/******************************************************************************
 * Specific output message formating for dragonfly
 ******************************************************************************/
void DibBridgeDragonflyRegisterIf(struct DibBridgeContext *pContext, uint32_t * Config)
{
   /* upack dragonflu based register config */
   pContext->DragonflyRegisters.JedecAddr       = Config[0];
   pContext->DragonflyRegisters.JedecValue      = Config[1];
   pContext->DragonflyRegisters.MacMbxSize      = Config[2];
   pContext->DragonflyRegisters.MacMbxStart     = Config[3];
   pContext->DragonflyRegisters.MacMbxEnd       = Config[4];
   pContext->DragonflyRegisters.HostMbxSize     = Config[5];
   pContext->DragonflyRegisters.HostMbxStart    = Config[6];
   pContext->DragonflyRegisters.HostMbxEnd      = Config[7];
   pContext->DragonflyRegisters.HostMbxRdPtrReg = Config[8];
   pContext->DragonflyRegisters.HostMbxWrPtrReg = Config[9];
   pContext->DragonflyRegisters.MacMbxRdPtrReg  = Config[10];
   pContext->DragonflyRegisters.MacMbxWrPtrReg  = Config[11];

   /* specific architecture functions */
   switch(pContext->DibChip)
   {
      case DIB_VOYAGER:
         pContext->BridgeChipOps.PreFormat           = DibBridgeVoyager1PreFormat;
         pContext->BridgeChipOps.PostFormat          = DibBridgeVoyager1PostFormat;
#if ((DIB_BRIDGE_TESTIF_PREINIT == 1) || (DIB_BRIDGE_TESTIF_POSTINIT == 1))
         pContext->BridgeChipOps.TestRegister        = IntBridgeVoyager1TestRegister;
         pContext->BridgeChipOps.TestExternalRam     = IntBridgeVoyager1TestExternalRam;
#endif
         break;
      case DIB_NAUTILUS:
         pContext->BridgeChipOps.PreFormat           = DibBridgeNautilus1PreFormat;
         pContext->BridgeChipOps.PostFormat          = DibBridgeNautilus1PostFormat;
#if ((DIB_BRIDGE_TESTIF_PREINIT == 1) || (DIB_BRIDGE_TESTIF_POSTINIT == 1))
         pContext->BridgeChipOps.TestRegister        = IntBridgeNautilus1TestRegister;
         pContext->BridgeChipOps.TestExternalRam     = IntBridgeNautilus1TestExternalRam;
#endif
         break;
      default:
         break;
   }
   pContext->BridgeChipOps.SendMsg                   = DibBridgeDragonflySendMsg;
   pContext->BridgeChipOps.AssembleSlice             = DibBridgeDragonflyAssembleSlice;
   pContext->BridgeChipOps.SendAck                   = DibBridgeDragonflySendAck;
   pContext->BridgeChipOps.ProcessIrq                = DibBridgeDragonflyProcessIrq;
#if (mSDK == 0)
   pContext->BridgeChipOps.ProcessDma                = DibBridgeDragonflyProcessDma;
#else
   pContext->BridgeChipOps.ProcessDma                = NULL;
#endif
   pContext->BridgeChipOps.SetupDma                  = DibBridgeDragonflySetupDma;
   pContext->BridgeChipOps.RequestDma                = DibBridgeDragonflyRequestDma;
   pContext->BridgeChipOps.GetArch                   = DibBridgeDragonflyGetArchi;
   pContext->BridgeChipOps.IncrementFormattedAddress = DibBridgeDragonflyIncrementFormattedAddress;
   pContext->BridgeChipOps.SignalBufFail             = DibBridgeDragonflySignalBufFail;
   pContext->BridgeChipOps.ChipsetInit               = DibBridgeDragonflyChipsetInit;
   pContext->BridgeChipOps.ChipsetDeinit             = DibBridgeDragonflyChipsetDeinit;

#if (DIB_BRIDGE_HBM_PROFILER == 1)
   pContext->BridgeChipOps.HbmProfiler               = DibBridgeDragonflyHbmProfiler;
#endif

#if (DIB_CHECK_DATA == 1)
   pContext->BridgeChipOps.ClearCheckStats           = DibBridgeDragonflyClearCheckStats;
   pContext->BridgeChipOps.ForwardCheckStats         = DibBridgeDragonflyForwardCheckStats;
#endif

#if ((DIB_BRIDGE_TESTIF_PREINIT == 1) || (DIB_BRIDGE_TESTIF_POSTINIT == 1))
   pContext->BridgeChipOps.TestBasicRead             = IntBridgeDragonflyTestBasicRead;
   pContext->BridgeChipOps.TestInternalRam           = IntBridgeDragonflyTestInternalRam;
   pContext->BridgeChipOps.GetRamAddr                = DibBridgeDragonflyGetRamAddr;
#endif

   pContext->BridgeChipOps.SetService                = DibBridgeDragonflySetService;
}

#endif /* USE_DRAGONFLY */
