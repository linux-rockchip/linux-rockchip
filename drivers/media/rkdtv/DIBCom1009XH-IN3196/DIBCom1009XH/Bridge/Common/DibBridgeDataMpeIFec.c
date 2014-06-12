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
* @file "DibBridgeMpeIfec.c"
* @brief MPE IFEC Data Handling.
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

#if (DIB_MPEIFEC_DATA == 1)

void DibBridgeInitMpeIFecCtx(struct DibBridgeContext *pContext)
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

      pContext->ItSvc[i].GlobalSize = 0;
      memset(pContext->ItSvc[i].EncodeAdd, 0, sizeof(pContext->ItSvc[i].EncodeAdd));
      memset(pContext->ItSvc[i].MpeBurst, 0, sizeof(pContext->ItSvc[i].MpeBurst));
      memset(pContext->ItSvc[i].EncodLenRx, 0, sizeof(pContext->ItSvc[i].EncodLenRx));      /* 32 Bits */
      memset(pContext->ItSvc[i].RecordMsg, 0, sizeof(pContext->ItSvc[i].RecordMsg));
      pContext->ItSvc[i].FreeRecordMsg = 0;      

      pContext->ItSvc[i].ProcBuf[0] = 0;
      pContext->ItSvc[i].ProcBuf[1] = 0;
   }
}


void DibBridgeShDataDone(struct DibBridgeContext * pContext, struct DibBridgeDmaFlags * pFlags, uint32_t TableIdx)
{
   ELEM_HDL                 idx     = pFlags->ItemHdl;
   uint8_t                  BufIdx  = 0;

   DIB_ASSERT(idx < DIB_MAX_NB_ITEMS);

   BufIdx = pContext->ItSvc[idx].MpeRxState;
   DIB_ASSERT((BufIdx == 0) || (BufIdx == 1));

   /* Already have a Cor buffer */
   if(pContext->ItSvc[idx].MpeBufCor)
   {
      /* Already have a queued buffer */
      if(pContext->ItSvc[idx].MpeBufRx)
      {
         /* Send a Flush Message to firmware */ 
         DibBridgeSignalBufFail(pContext, pFlags, 1);
         DIB_DEBUG(MPEBUF_ERR, (CRB "RX:COR: Host is too slow cannot bufferise data for Table %d." CRA, TableIdx));
         goto End;
      }
      else
      {
         /* Next to be processed buffer */
         DIB_DEBUG(MPEBUF_LOG, (CRB "RX SECOND : Storing Table %d in Buffer %d" CRA, TableIdx, BufIdx));
         pContext->ItSvc[idx].MpeBufRx = pContext->ItSvc[idx].ProcBuf[BufIdx];
      }
   }
   else
   {
      /* Currently Processed Buffer */
      DIB_DEBUG(MPEBUF_LOG, (CRB "COR FIRST : Storing Table %d in Buffer %d" CRA, TableIdx, BufIdx));
      pContext->ItSvc[idx].MpeBufCor = pContext->ItSvc[pFlags->ItemHdl].ProcBuf[BufIdx];
   }

   /* Copy Buffer Content and update index */
   DibMoveMemory(pContext->ItSvc[pFlags->ItemHdl].ProcBuf[BufIdx], pContext->ItSvc[idx].EncodeAdd[TableIdx][0], pContext->ItSvc[idx].BufCorLen);
   DibSetMemory(pContext->ItSvc[idx].EncodeAdd[TableIdx][0], 0x47, pContext->ItSvc[idx].BufCorLen);
   pContext->ItSvc[idx].MpeRxState = ((pContext->ItSvc[idx].MpeRxState+1) % 2);

End:
   return;
}

void DibBridgeFreeMpeIFecBuffers(struct DibBridgeContext *pContext) 
{
   uint8_t Idx = 0;
   uint8_t Tab = 0;

   for(Idx = 0; Idx < DIB_MAX_NB_ITEMS; Idx++)
   {
      for(Tab = 0; Tab < 64; Tab++)
      {
         if(pContext->ItSvc[Idx].EncodeAdd[Tab][0])
         {
            DibBridgeTargetFreeBuf(pContext->ItSvc[Idx].MpeBurst[Tab], pContext->ItSvc[Idx].DataLenRx);
            pContext->ItSvc[Idx].MpeBurst[Tab] = NULL;
         }
         else
         {
            /* If Ptr is null then no more on this Item */
            break;
         }
      }
   }
}

DIBDMA DibBridgeAllocMemory(struct DibBridgeContext * pContext, uint32_t min, uint32_t max, uint32_t faddr, uint32_t len, struct DibBridgeDmaFlags * pFlags)
{
   uint32_t NbRows = (pFlags->NbRows << 8);
   uint32_t DataSize = NbRows * min;
   uint32_t GlobalSize = NbRows * max;
  /* uint32_t NbMpeTables = faddr;*/
   uint32_t m = 0;
   
   /** Allocation of correction tables */
   DIB_DEBUG(PORT_LOG, (CRB "\tAlloc NbRows : %d min %d max %d from 0 to %d alloc %d" CRA, NbRows, min, max, len-1, pFlags->BlockType));

   for(m = 0; m < len; m++)
   {
      pContext->ItSvc[pFlags->ItemHdl].EncodLenRx[m][0] = 0;
      pContext->ItSvc[pFlags->ItemHdl].EncodLenRx[m][1] = 0;
      if(pFlags->BlockType)
      {
         pContext->ItSvc[pFlags->ItemHdl].EncodeAdd[m][0] = DibBridgeTargetAllocBuf(GlobalSize);
         if(pContext->ItSvc[pFlags->ItemHdl].EncodeAdd[m][0] == 0)
         {
            goto error;
         }
         pContext->ItSvc[pFlags->ItemHdl].EncodeAdd[m][1] = pContext->ItSvc[pFlags->ItemHdl].EncodeAdd[m][0] + DataSize;

         DibSetMemory(pContext->ItSvc[pFlags->ItemHdl].EncodeAdd[m][0], 0x47, GlobalSize);

         pContext->ItSvc[pFlags->ItemHdl].GlobalSize= GlobalSize;
         pContext->ItSvc[pFlags->ItemHdl].DataLenRx = GlobalSize;
         pContext->ItSvc[pFlags->ItemHdl].BufCorLen = GlobalSize;
         pContext->ItSvc[pFlags->ItemHdl].SkipR = 0;
                  
      }
      else
      {
         DibBridgeTargetFreeBuf(pContext->ItSvc[pFlags->ItemHdl].EncodeAdd[m][0], pContext->ItSvc[pFlags->ItemHdl].DataLenRx);
         pContext->ItSvc[pFlags->ItemHdl].EncodeAdd[m][0] = 0;
         pContext->ItSvc[pFlags->ItemHdl].EncodeAdd[m][1] = 0; /* ? */
         
      }
   }

   if(pFlags->BlockType)
   {
      memset(pContext->ItSvc[pFlags->ItemHdl].RecordMsg, 0, sizeof(pContext->ItSvc[pFlags->ItemHdl].RecordMsg));
      pContext->ItSvc[pFlags->ItemHdl].ProcBuf[0] = DibBridgeTargetAllocBuf(GlobalSize);
      pContext->ItSvc[pFlags->ItemHdl].ProcBuf[1] = DibBridgeTargetAllocBuf(GlobalSize);

      if((pContext->ItSvc[pFlags->ItemHdl].ProcBuf[0] == 0) || (pContext->ItSvc[pFlags->ItemHdl].ProcBuf[1] == 0))
         goto error;
   }
   else
   {
      DibBridgeTargetFreeBuf(pContext->ItSvc[pFlags->ItemHdl].ProcBuf[0], pContext->ItSvc[pFlags->ItemHdl].BufCorLen);
      DibBridgeTargetFreeBuf(pContext->ItSvc[pFlags->ItemHdl].ProcBuf[1], pContext->ItSvc[pFlags->ItemHdl].BufCorLen);
      pContext->ItSvc[pFlags->ItemHdl].ProcBuf[0] = 0;
      pContext->ItSvc[pFlags->ItemHdl].ProcBuf[1] = 0;
      pContext->ItSvc[pFlags->ItemHdl].MpeBufRx  = NULL;
      pContext->ItSvc[pFlags->ItemHdl].MpeBufCor  = NULL;
   }

   /** Allocation of Mpe bursts */
   for(m = 0; m < /*NbMpeTables*/len; m++)
   {
      if(pFlags->BlockType)
      {         
         pContext->ItSvc[pFlags->ItemHdl].MpeBurst[m] = DibBridgeTargetAllocBuf(DataSize);
         if(pContext->ItSvc[pFlags->ItemHdl].MpeBurst[m] == 0)
         {
            goto error;
         }
      }
      else
      {
         DibBridgeTargetFreeBuf(pContext->ItSvc[pFlags->ItemHdl].MpeBurst[m], DataSize);
         pContext->ItSvc[pFlags->ItemHdl].MpeBurst[m] = NULL;
      }      
   }
   if(pFlags->BlockType)
   {
      return DIB_DMA_DONE;
   }
   else
   {
      pContext->ItSvc[pFlags->ItemHdl].MpeBufRx        = NULL;
      pContext->ItSvc[pFlags->ItemHdl].MpeBufCor       = NULL;
      pContext->ItSvc[pFlags->ItemHdl].MpeRxState      = BUF_RDY;
      pContext->ItSvc[pFlags->ItemHdl].BufCorLen       = 0;
      pContext->ItSvc[pFlags->ItemHdl].SliceLen        = 0;
      
      return DIB_NO_DMA;
   }
error:
   for(m = 0; m < len; m++)
   {
      if(pContext->ItSvc[pFlags->ItemHdl].EncodeAdd[m][0] != 0)
      {
         DibBridgeTargetFreeBuf(pContext->ItSvc[pFlags->ItemHdl].EncodeAdd[m][0], pContext->ItSvc[pFlags->ItemHdl].DataLenRx);         
         pContext->ItSvc[pFlags->ItemHdl].EncodeAdd[m][0] = 0;
      }
      else
      {
         break;
      }
   }
   for(m = 0; m < /*NbMpeTables*/len; m++)
   {
      if(pContext->ItSvc[pFlags->ItemHdl].MpeBurst[m] != 0)
      {
         DibBridgeTargetFreeBuf(pContext->ItSvc[pFlags->ItemHdl].MpeBurst[m], DataSize);
         pContext->ItSvc[pFlags->ItemHdl].MpeBurst[m] = 0;
      }
      else
      {
         break;
      }
   }

   if(pContext->ItSvc[pFlags->ItemHdl].ProcBuf[0])
   {
      DibBridgeTargetFreeBuf(pContext->ItSvc[pFlags->ItemHdl].ProcBuf[0], pContext->ItSvc[pFlags->ItemHdl].BufCorLen);
      pContext->ItSvc[pFlags->ItemHdl].ProcBuf[0] = 0;
   }

   if(pContext->ItSvc[pFlags->ItemHdl].ProcBuf[1])
   {
      DibBridgeTargetFreeBuf(pContext->ItSvc[pFlags->ItemHdl].ProcBuf[1], pContext->ItSvc[pFlags->ItemHdl].BufCorLen);
      pContext->ItSvc[pFlags->ItemHdl].ProcBuf[1] = 0;
   }

   if(pFlags->BlockType)
      DibBridgeSignalBufFail(pContext, pFlags, 0);

   return DIB_NO_DMA;
}

DIBDMA DibBridgeGetShFragment(struct DibBridgeContext * pContext, uint32_t Min, uint32_t Shift, uint8_t * IntAdd, uint32_t ExtAdd, uint32_t len, struct DibBridgeDmaFlags * pFlags, uint32_t External)
{
   uint32_t Begin  = (Min & 0xFFFF0000) >> 16; /* FirstCol if k < B */
   uint32_t NbRows = (pFlags->NbRows << 8);
   struct DibBridgeDmaCtx *pDmaCtx = &pContext->DmaCtx;
   DIBDMA DmaStat = DIB_DMA_DONE;
   uint16_t m = (uint16_t) (Min & 0xFFFF); /* EncodIdx */
   uint32_t Length   = 0;
   uint8_t  Mode     = 0;
   uint32_t Delta    = 0;
   uint32_t PaddSize = 0;
   uint32_t ErrSize = 0;
   uint32_t DmaSize  = 0;
   volatile uint32_t ChipAddr = 0;
   
   if(pFlags->Type == FORMAT_SH_FRG_DATA)
   {
      PaddSize = Shift;
      ErrSize = (len&0xFFFF0000) | (Begin&0xFFFF);
   }
   DIB_ASSERT(((IntAdd == 0) && (External)) || ((ExtAdd == 0) && (!External)));
   
   /* Allocation of buffers should have been done before */
   if(pContext->ItSvc[pFlags->ItemHdl].EncodeAdd[0][0] == 0)
   {
      return DIB_NO_DMA;
   }
   if((m & 0x7F) == 0x7F)
   {
      return DIB_NO_DMA;
   }
   pDmaCtx->Dir         = DIBBRIDGE_DMA_READ;
   pDmaCtx->ChipAddr    = ExtAdd;
   pDmaCtx->ChipBaseMin = 0;
   pDmaCtx->ChipBaseMax = 0;
   
/** Direct or recorded transfert ? */
   
   memcpy(&pDmaCtx->DmaFlags, pFlags, sizeof(struct DibBridgeDmaFlags));
   if((m >> 7)&0x1)
   {
      uint8_t FreeRecordMsg = pContext->ItSvc[pFlags->ItemHdl].FreeRecordMsg;
      if(FreeRecordMsg >= 96)
      {
         return DIB_NO_DMA;
      }

      pContext->ItSvc[pFlags->ItemHdl].RecordMsg[FreeRecordMsg].Index = (m & 0x7F);
      pDmaCtx->DmaLen = len;
      
      /* First step : DMA to a local buffer */
      pContext->ItSvc[pFlags->ItemHdl].RecordMsg[FreeRecordMsg].Add = DibBridgeTargetAllocBuf(len);
      pDmaCtx->pHostAddr = pContext->ItSvc[pFlags->ItemHdl].RecordMsg[FreeRecordMsg].Add;
      
      DibBridgeSetupDma(pContext, pDmaCtx);
      DmaStat = DibBridgeRequestDma(pContext, pDmaCtx);
      
      /* Second step : prepare the future dma */
      if(DmaStat == DIB_DMA_DONE)
      {
         pContext->ItSvc[pFlags->ItemHdl].RecordMsg[FreeRecordMsg].Begin = Begin;
         pContext->ItSvc[pFlags->ItemHdl].RecordMsg[FreeRecordMsg].Shift = Shift;
         pContext->ItSvc[pFlags->ItemHdl].RecordMsg[FreeRecordMsg].Len = len;
         memcpy(&pContext->ItSvc[pFlags->ItemHdl].RecordMsg[FreeRecordMsg].DmaFlags, pFlags, sizeof(struct DibBridgeDmaFlags));
         pContext->ItSvc[pFlags->ItemHdl].FreeRecordMsg++;
      }
      else
      {
         DibBridgeTargetFreeBuf(pContext->ItSvc[pFlags->ItemHdl].RecordMsg[FreeRecordMsg].Add, len);
         pContext->ItSvc[pFlags->ItemHdl].RecordMsg[FreeRecordMsg].Add = NULL;
      }
      
      return DmaStat;
   }
   
   m = m&0x1F;
   len = len&0xFFFF;
 
   Mode = (pFlags->Type == FORMAT_SH_FRG_DATA) ? 0 : 1;
 
   /* Begin of mini buffer */
   if(pFlags->FirstFrag)
   {
      if(pFlags->Type == FORMAT_SH_FRG_DATA)
      {
         pContext->ItSvc[pFlags->ItemHdl].EncodLenRx[m][Mode] = 0;
      }
      else
      {
         pContext->ItSvc[pFlags->ItemHdl].EncodLenRx[m][Mode] = Begin*NbRows;
      }
   }

   if(pFlags->Type == FORMAT_SH_FRG_DATA)
   {
      Delta = len;
   }
   else
   {
      Delta = pContext->ItSvc[pFlags->ItemHdl].EncodLenRx[m][Mode]%NbRows;
      Delta = ((Delta == 0) ? NbRows : (NbRows - Delta));
   }     

   pDmaCtx->pHostAddr = pContext->ItSvc[pFlags->ItemHdl].EncodeAdd[m][Mode] + pContext->ItSvc[pFlags->ItemHdl].EncodLenRx[m][Mode];
   
   Length = len;

   if(pFlags->Type == FORMAT_SH_FRG_DATA)
      Shift = Length;

   pDmaCtx->DmaLen = (Delta < Length) ? Delta : Length;

   while(Length > 0)
   {
      /* Due to byte aligment */
      ChipAddr = pDmaCtx->ChipAddr;
      DmaSize  = pDmaCtx->DmaLen;

      if(External)
      {
         if(DibBridgeSetupDma(pContext, pDmaCtx) != DIBSTATUS_SUCCESS)
         {
            DIB_DEBUG(DMA_ERR, (CRB "DibBridgeSetupDma Failed size %d, addr %x, Addr %x)" CRA, DmaSize, ChipAddr, pDmaCtx->ChipAddr));
            return DIB_DEV_FAILED;
         }
         
         DmaStat = DibBridgeRequestDma(pContext, pDmaCtx);
         pDmaCtx->ChipAddr = ChipAddr + DmaSize; /* First update was done in case of alignment */

         if(DmaStat != DIB_DMA_DONE)
         {          
            DIB_DEBUG(DMA_ERR, (CRB "DibBridgeRequestDma Failed (stat= %d, size %d %d %d, addr %x)" CRA, DmaStat, DmaSize,pDmaCtx->DmaLen, pDmaCtx->DmaSize, ChipAddr));
            return DmaStat;
         }
      }
      else
      {
         /** internal : not a dma */
         DibMoveMemory(pDmaCtx->pHostAddr, IntAdd, pDmaCtx->DmaLen);
         IntAdd += pDmaCtx->DmaLen;
         DmaStat = DIB_NO_DMA;
      }

      pContext->ItSvc[pFlags->ItemHdl].EncodLenRx[m][Mode] += (DmaSize); /* pDmaCtx->DmaLen may have been changed by alignment */
      
      if((pFlags->Type == FORMAT_SH_FRG_IFEC) && (((pContext->ItSvc[pFlags->ItemHdl].EncodLenRx[m][Mode])%NbRows) == 0))
      {
         pContext->ItSvc[pFlags->ItemHdl].EncodLenRx[m][Mode] += (Shift);
      }

      pDmaCtx->pHostAddr = pContext->ItSvc[pFlags->ItemHdl].EncodeAdd[m][Mode] + pContext->ItSvc[pFlags->ItemHdl].EncodLenRx[m][Mode];
      Length -= DmaSize; /* pDmaCtx->DmaLen may have been changed by alignment */

      if(pFlags->Type == FORMAT_SH_FRG_IFEC)
      {
         /* Calculate new dma lenght */
         pDmaCtx->DmaLen = (Length < NbRows) ? Length : NbRows;
      }
      else
      {
         DIB_ASSERT(Length == 0);
      }
   }
   
   if(ErrSize != 0)
   {
      uint8_t * Dst = pContext->ItSvc[pFlags->ItemHdl].EncodeAdd[m][Mode] + pContext->ItSvc[pFlags->ItemHdl].EncodLenRx[m][Mode];

      DibSetMemory(Dst, 0x47, ErrSize);
      pContext->ItSvc[pFlags->ItemHdl].EncodLenRx[m][Mode] += ErrSize;
   }

   if(PaddSize != 0)
   {
      uint8_t * Dst = pContext->ItSvc[pFlags->ItemHdl].EncodeAdd[m][Mode] + pContext->ItSvc[pFlags->ItemHdl].EncodLenRx[m][Mode];

      DibZeroMemory(Dst, PaddSize);
      pContext->ItSvc[pFlags->ItemHdl].EncodLenRx[m][Mode] += PaddSize;
   }

   if(pFlags->LastFrag)
      DibBridgeShDataDone(pContext, pFlags, m);

   return DmaStat;
}

DIBDMA DibBridgeFillShData(struct DibBridgeContext * pContext, uint32_t Begin, uint32_t Shift, uint32_t NbCol, uint32_t Table, struct DibBridgeDmaFlags * pFlags)
{
   uint8_t  Mode, Inc;
   uint16_t m        = pFlags->FrameId;
   uint32_t NbRows   = (pFlags->NbRows << 8);
   uint16_t IdxCol   = (uint16_t) (NbCol&0xFFFF);
   uint16_t Pattern  = (uint16_t) ((NbCol >> 16)&0xFF);
   uint16_t EncodMax = (uint16_t) ((NbCol >> 24)&0xFF);
   uint16_t First    = (uint16_t) (Table&0xFFFF);
   uint16_t Last     = (uint16_t) ((Table >> 16)&0xFFFF);
   uint8_t * Dst;
   uint8_t count = 0;
   
   /* Allocation of buffers should have been done before */
   if(pContext->ItSvc[pFlags->ItemHdl].EncodeAdd[0][0] == 0)
   {
      return DIB_NO_DMA;
   }
   
   DIB_ASSERT((pFlags->Type == FORMAT_SH_FILL_DATA) || (pFlags->Type == FORMAT_SH_FILL_IFEC));
   
   if(pFlags->Type == FORMAT_SH_FILL_DATA)
   {
      Inc = 1; 
      Mode = 0;
   }
   else
   {
      Inc = -1;
      Mode = 1;
   }   
   while(IdxCol)
   {
      if(count == Begin)
         pContext->ItSvc[pFlags->ItemHdl].EncodLenRx[m][Mode] = 0;

      Dst = pContext->ItSvc[pFlags->ItemHdl].EncodeAdd[m][Mode] + pContext->ItSvc[pFlags->ItemHdl].EncodLenRx[m][Mode];
      
      DibSetMemory(Dst, Pattern, NbRows);
      pContext->ItSvc[pFlags->ItemHdl].EncodLenRx[m][Mode] += (NbRows*Shift); 
      IdxCol --;
      m += Inc;
      if(m == (Last + 1))
         m = First;    
      if(m == EncodMax)
         m = 0;
      count++;
   }
         
   return DIB_NO_DMA;
}

DIBDMA DibBridgeFillShPattern(struct DibBridgeContext * pContext, uint32_t Begin, uint32_t Shift, uint32_t FailedSize, uint32_t CorrectSize, struct DibBridgeDmaFlags * pFlags)
{
   uint8_t m = pFlags->FrameId;
   uint8_t * Dst;
   int32_t CopyLen;
   uint8_t Mode;
   uint8_t Val = 0x47;
   uint32_t NbRows = (pFlags->NbRows << 8);
   int32_t Length = 0;
   
   /* Allocation of buffers should have been done before */
   if(pContext->ItSvc[pFlags->ItemHdl].EncodeAdd[0][0] == 0)
   {
      return DIB_NO_DMA;
   }
   
   DIB_ASSERT((pFlags->Type == FORMAT_SH_PATTERN_DATA) || (pFlags->Type == FORMAT_SH_PATTERN_IFEC));
   
   if(pFlags->Type == FORMAT_SH_PATTERN_DATA)
   {
      Mode = 0;
      CopyLen = FailedSize;      
   }
   else
   {
      Mode = 1;
      CopyLen = FailedSize > NbRows ? NbRows : FailedSize;
   }
   
   /* Begin of mini buffer */
   if(pFlags->FirstFrag)
   {
      pContext->ItSvc[pFlags->ItemHdl].EncodLenRx[m][Mode] = Begin*NbRows;
   }
   Dst = pContext->ItSvc[pFlags->ItemHdl].EncodeAdd[m][Mode] + pContext->ItSvc[pFlags->ItemHdl].EncodLenRx[m][Mode];
   
   Length = FailedSize;    

   if(Shift == 0)
      Shift = Length;
   
   while(Length > 0)
   {
      DibSetMemory(Dst, Val, CopyLen);
      pContext->ItSvc[pFlags->ItemHdl].EncodLenRx[m][Mode] += (Shift); 
      Dst = pContext->ItSvc[pFlags->ItemHdl].EncodeAdd[m][Mode] + pContext->ItSvc[pFlags->ItemHdl].EncodLenRx[m][Mode];
      Length -= CopyLen;
      CopyLen = (Length < CopyLen ? Length : CopyLen);
   }

   if(pFlags->Type == FORMAT_SH_PATTERN_DATA)
   {
      CopyLen = CorrectSize;  
      Shift = 0;
   }
   else
   {
      CopyLen = CorrectSize > NbRows ? NbRows : CorrectSize;
   }
   
   Length = CorrectSize;   

   if(Shift == 0)
      Shift = Length;

   while(Length > 0)
   {
      DibZeroMemory(Dst, CopyLen);
      pContext->ItSvc[pFlags->ItemHdl].EncodLenRx[m][Mode] += (Shift); 
      Dst = pContext->ItSvc[pFlags->ItemHdl].EncodeAdd[m][Mode] + pContext->ItSvc[pFlags->ItemHdl].EncodLenRx[m][Mode];
      Length -= CopyLen;
      CopyLen = (Length < CopyLen ? Length : CopyLen);
   }

   if(pFlags->LastFrag)
      DibBridgeShDataDone(pContext, pFlags, m);
         
   return DIB_NO_DMA;
}

#endif
