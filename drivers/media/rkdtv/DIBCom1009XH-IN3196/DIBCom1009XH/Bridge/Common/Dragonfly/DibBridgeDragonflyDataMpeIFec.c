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
* @file "DibBridgeDragonflyDataMpeIFec.c"
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
#include "DibBridgeData.h"

#if (DIB_MPEIFEC_DATA == 1)
uint8_t IntBridgeDragonflyGetEncoderTableIndex(struct DibBridgeDmaFlags DmaFlags, uint8_t Index, uint32_t * pTab, uint8_t Size)
{
   uint8_t Type = DmaFlags.Type;
   uint8_t ii = 0;
   
   for(ii = 0; ii < Size; ii++)
   {
      if((Type == GET_MSG_TYPE(pTab[ii])) && (Index == GET_MSG_INDEX(pTab[ii])))
      {
         break;
      }
   }
   DIB_ASSERT(ii < Size);
   return GET_MSG_TABLE(pTab[ii]);
}

DIBDMA DibBridgeDragonflyMoveData(struct DibBridgeContext * pContext, uint32_t * Data, uint32_t Size, uint8_t Move)
{
   DIBDMA DmaStat = DIB_NO_DMA;

   uint8_t  ItemHdl = (uint8_t) ((Data[1] >> 8)&0xFF);
   uint8_t FreeRecordMsg = pContext->ItSvc[ItemHdl].FreeRecordMsg, pointer = 0;
   uint8_t EncodIdx;
   struct DibBridgeRecordDmaFlags * pRecordMsg = &pContext->ItSvc[ItemHdl].RecordMsg[0];

   /** the bufferised messages have to be taken in the correct order and to be moved in the correct table */
   while(pointer != FreeRecordMsg)
   {
      if(Move)
      {
         EncodIdx = IntBridgeDragonflyGetEncoderTableIndex(pRecordMsg->DmaFlags, pRecordMsg->Index, &Data[2], Size - 2);

         DmaStat = DibBridgeGetShFragment(pContext, (pRecordMsg->Begin << 16) | EncodIdx, pRecordMsg->Shift, pRecordMsg->Add, 0, pRecordMsg->Len, &pRecordMsg->DmaFlags, 0);
      }
      DibBridgeTargetFreeBuf(pRecordMsg->Add, pRecordMsg->Len);
      pointer++;
      pRecordMsg++;
   }
   pContext->ItSvc[ItemHdl].FreeRecordMsg = 0;

   return DmaStat;
}

void DibBridgeDragonflyReconstructBurst(struct DibBridgeContext * pContext, uint32_t * Data, uint32_t Size)
{
   uint8_t  ItemHdl    = (uint8_t) ((Data[1] >> 8)&0xFF);
   uint16_t NbRows     = (uint16_t) (((Data[1] >> 20)&0x7) << 8);
   uint8_t  TableIdx   = (uint8_t) ((Data[1] >> 24)&0xFF);
   uint8_t  BurstValid = (uint8_t) ((Data[1] >> 23)&0x1);
   uint8_t  NotReset   = (uint8_t) ((Data[1] >> 17)&0x1);
   uint8_t  NbCol, BurstIdx, Shift, Begin, Valid, jj, ii;
   uint8_t *AdtSrc, *AdstDest;
   uint32_t GlobalSize = 0;

   Size -= 2;

   AdtSrc = pContext->ItSvc[ItemHdl].MpeBufCor;

   if(AdtSrc == 0)
   {
      DIB_DEBUG(MPEBUF_ERR, (CRB "No buffer to reconstruct Table %d" CRA, TableIdx));
      return;
   }

   DIB_DEBUG(MPEBUF_LOG,(CRB "SH : ADT SRC %d NbRows %d burst valid %d" CRA, TableIdx, NbRows, BurstValid));
   
   for(ii = 0; ii < Size; ii++)
   {
      jj = 0;
      Valid = (uint8_t) ((Data[2 + ii] >> 31)&0x1);
      NbCol = (uint8_t) ((Data[2 + ii] >> 8)&0xFF);
      Shift = (uint8_t) ((Data[2 + ii] >> 16)&0xFF);
      Begin = (uint8_t) ((Data[2 + ii] >> 24)&0x7F);
      BurstIdx = (uint8_t) (Data[2 + ii]&0xFF);

      AdstDest = pContext->ItSvc[ItemHdl].MpeBurst[BurstIdx] + (Begin * NbRows);
     
      while(jj < NbCol)
      {
         if(Valid)
         {
            DibMoveMemory(AdstDest, AdtSrc, NbRows);
         }
         AdtSrc += NbRows; 
         AdstDest += (Shift * NbRows);
         GlobalSize += NbRows;
         jj++;
      }
   }

   if(BurstValid)
   {
      if(pContext->ItSvc[ItemHdl].MpeBurst[Data[2]&0xFF] != 0)
      {
         DIB_DEBUG(MPEBUF_LOG,(CRB "SH : ADST %d forward" CRA, Data[2]&0xFF));
         DibBridgeForwardData(pContext, pContext->ItSvc[ItemHdl].MpeBurst[Data[2]&0xFF], GlobalSize, ItemHdl);
         DibZeroMemory(pContext->ItSvc[ItemHdl].MpeBurst[Data[2]&0xFF], GlobalSize);
      }      
   }
   else
   {
      DIB_DEBUG(MPEBUF_LOG,(CRB "SH : ADST failed" CRA));
   }
     
   if(!NotReset)
   {
      DibZeroMemory(pContext->ItSvc[ItemHdl].MpeBurst[Data[2]&0xFF], GlobalSize);
   }

   /* Manage Processing buffers */
   if(pContext->ItSvc[ItemHdl].MpeBufRx)
   {
      DIB_DEBUG(MPEBUF_LOG,(CRB "COR : Done processing buffer for Table %d" CRA, TableIdx));
      DIB_DEBUG(MPEBUF_LOG,(CRB "RX TO COR: Processing second buffer" CRA));
      pContext->ItSvc[ItemHdl].MpeBufCor = pContext->ItSvc[ItemHdl].MpeBufRx;
      pContext->ItSvc[ItemHdl].MpeBufRx  = NULL;
   }
   else
   {
      DIB_DEBUG(MPEBUF_LOG,(CRB "COR : Done processing buffer for Table %d" CRA, TableIdx));
      pContext->ItSvc[ItemHdl].MpeBufCor = NULL;
   }

   /* Indicate Correction is done (in all cases )*/
   pContext->ItSvc[ItemHdl].FecOffset = 0;
}

void DibBridgeDragonflyResetTable(struct DibBridgeContext * pContext, uint32_t * Data)
{
   uint8_t ItemHdl      = (uint8_t) ((Data[1] >> 8)&0xFF);
   uint16_t NbRows      = (uint16_t) (((Data[1] >> 16)&0xFF) << 8);
   uint8_t NbTbIdxMax   = (uint8_t) ((Data[1] >> 24)&0xFF);
   uint8_t ii           = 0;
   
   if((Data[1]&0x1) == 1)
   {
      for(ii = 0; ii < NbTbIdxMax; ii++)
      {
         pContext->ItSvc[ItemHdl].EncodLenRx[ii][0] = 0;
         pContext->ItSvc[ItemHdl].EncodLenRx[ii][1] = 0;
         DibSetMemory(pContext->ItSvc[ItemHdl].EncodeAdd[ii][0], 0x47, pContext->ItSvc[ItemHdl].DataLenRx);
      }      
   }
   else
   {
      uint8_t TableIdx    = (uint8_t) ((Data[2] >> 24)&0xFF);
      uint8_t NbTable     = (uint8_t) ((Data[2] >> 16)&0xFF); 
      uint16_t NbCol      = (uint8_t) ((Data[2] >> 8)&0xFF);
      uint8_t AddOneCol   = (uint8_t) (Data[2]&0xFF);
      uint32_t ColTotal   = 0;
      for(ii = 0; ii < NbTable; ii++)
      {
         if(ii == AddOneCol)
            NbCol++;
         ColTotal += NbCol;
         pContext->ItSvc[ItemHdl].EncodLenRx[TableIdx][0] = ColTotal*NbRows;

         if(TableIdx == 0)
            TableIdx = NbTbIdxMax;
         TableIdx--;
      }
   }
}

DIBDMA DibBridgeDragonflyInfoMsgHandler(struct DibBridgeContext * pContext, uint32_t * Data, uint32_t Size)
{
   DIBDMA DmaStat = DIB_NO_DMA;
   if((Data[1]&0x10) != 0)
   {
      DmaStat = DibBridgeDragonflyMoveData(pContext, Data, Size, (uint8_t) (Data[1]&0x1));
   }
   else
   {
      if((Data[1]&0x20) != 0)
      {
         DibBridgeDragonflyReconstructBurst(pContext, Data, Size);
      }
      else
      {
         if((Data[1]&0x40) != 0)
         {
            DibBridgeDragonflyResetTable(pContext, Data);
         }
      }
   }

   return DmaStat;
}

#endif

#endif /* USE_DRAGONFLY */
