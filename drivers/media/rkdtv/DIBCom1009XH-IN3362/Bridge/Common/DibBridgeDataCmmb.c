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

/*******************************************************************************
* @file "DibBridgeDataCmmb.c"
* @brief Cmmb Data Handling.
*******************************************************************************/
#include "DibBridgeConfig.h" /* Must be first include of all SDK files - Defines compilation options */
#include "DibBridgeTargetDebug.h"
#include "DibBridgeCommon.h"
#include "DibBridgeTarget.h"
#include "DibBridgeMailboxHandler.h"
#include "DibBridgePayloadCheckers.h"
#include "DibBridgeTestIf.h"
#include "DibBridge.h"
#include "DibBridgeData.h"

#if (DIB_CMMB_DATA == 1)

/**
 * DibBridgeInitCmmbCtx
 */
void DibBridgeInitCmmbCtx(struct DibBridgeContext *pContext)
{
   uint32_t i;
   for(i = 0; i < DIB_MAX_NB_ITEMS; i++)
   {
      pContext->ItSvc[i].MultiplexFrame = NULL;
      pContext->ItSvc[i].Size           = 0;
      pContext->ItSvc[i].PhyFrameId     = 0xff; /* (0:59) count set by Firmware. */
      pContext->ItSvc[i].WrOffset       = 0;
   }
}

/**
 * IntBridgeAllocMultiplexFrame
 */
DIBSTATUS IntBridgeAllocMultiplexFrame(struct DibBridgeContext *pContext, ELEM_HDL ItemHdl, uint32_t Size, uint8_t PhyFrameId)
{
   DibBridgeAllocateRawBuffer(pContext, &pContext->ItSvc[ItemHdl].MultiplexFrame, Size);
   if(pContext->ItSvc[ItemHdl].MultiplexFrame != NULL)
   {
      pContext->ItSvc[ItemHdl].Size       = Size;
      pContext->ItSvc[ItemHdl].PhyFrameId = PhyFrameId;
      pContext->ItSvc[ItemHdl].WrOffset   = 0;
      return DIBSTATUS_SUCCESS;
   }
   return DIBSTATUS_RESOURCES;
}

/**
 * IntBridgeFlushMultiplexFrame
 */
void IntBridgeFlushMultiplexFrame(struct DibBridgeContext *pContext, ELEM_HDL ItemHdl)
{
   if(pContext->ItSvc[ItemHdl].MultiplexFrame != NULL)
   {
      DibBridgeDesallocateRawBuffer(pContext, pContext->ItSvc[ItemHdl].MultiplexFrame, pContext->ItSvc[ItemHdl].Size);
   }
   pContext->ItSvc[ItemHdl].MultiplexFrame = NULL;
   pContext->ItSvc[ItemHdl].Size           = 0;
   pContext->ItSvc[ItemHdl].PhyFrameId     = 0xff;
   pContext->ItSvc[ItemHdl].WrOffset       = 0;
}

/**
 * IntBridgeCheckMultiplexFrameDesc
 */
DIBSTATUS IntBridgeCheckMultiplexFrameDesc(struct DibBridgeContext *pContext, ELEM_HDL ItemHdl, uint8_t PhyFrameId, uint8_t BegOfMf, uint32_t AddrInMf)
{
   DIBSTATUS Status = DIBSTATUS_SUCCESS;

   /* Check physical frame identifier. */
   if(pContext->ItSvc[ItemHdl].PhyFrameId == 0xff)
   {
      /* No current MF. RS matrix fragment must be beginning of new MF. */
      if(BegOfMf != 1)
      {
         DIB_DEBUG(CMMB_ERR, (CRB "*** Error: Beginning of MF %d missing" CRA, PhyFrameId));
         Status = DIBSTATUS_ERROR;
      }
   }
   else if(PhyFrameId != pContext->ItSvc[ItemHdl].PhyFrameId)
   {
      /* An MF is currently reconstructing. Must be same frame identifier. */
      DIB_DEBUG(CMMB_ERR, (CRB "*** Error: End of MF %d missing (new MF %d)" CRA, pContext->ItSvc[ItemHdl].PhyFrameId, PhyFrameId));
      Status = DIBSTATUS_ERROR;
   }
   else if(AddrInMf != pContext->ItSvc[ItemHdl].WrOffset)
   {
      /* Mismatch in between columns received so far and incoming columns. */
      DIB_DEBUG(CMMB_ERR, (CRB "*** Error: Middle of MF %d missing (%d I/O %d)" CRA, PhyFrameId, AddrInMf, pContext->ItSvc[ItemHdl].WrOffset));
      Status = DIBSTATUS_ERROR;
   }

   return Status;
}

/**
 * Case of whole MF corrected to be sent to driver directly
 */
void DibBridgeCmmbMfDone(struct DibBridgeContext *pContext, enum DibDataMode DataMode, struct DibBridgeDmaCtx *pDmaCtx)
{
   uint32_t NbRows;
   uint32_t DataSize;
   ELEM_HDL idx;


   /* Number of rows = 0 if Mf 0 */
   NbRows = pDmaCtx->ChipBaseMin;

   /* Service context index. */
   idx =  pDmaCtx->DmaFlags.ItemHdl;
   DIB_ASSERT(idx < DIB_MAX_NB_ITEMS);

   DataSize = ( NbRows == 0)? 8640 : (pDmaCtx->DmaLen - 4);

   /* Display for debug. */
   DIB_DEBUG(CMMB_LOG, (CRB "DibBridgeCmmbMfDone (): ItemHdl = %d DataSize =%d" CRA, idx,  DataSize));


#if (DIB_CHECK_CMMB_DATA == 1)
      /* Check multiplex frame payload. */
   DibBridgeCheckCmmbMultiplexFrame(pContext, pDmaCtx->pHostAddr, DataSize, idx);
   DibBridgeForwardCheckStats(pContext, idx);
#endif /* DIB_CHECK_CMMB_DATA */

   if(DataMode == eCLBACK)
   {
      if(pContext->DataInfoUp[pContext->NextFreeWrBuff].pBuff == NULL)
      {
         /* Use buffer. */
         pContext->DataInfoUp[pContext->NextFreeWrBuff].pBuff = pDmaCtx->pHostAddr;
         pContext->DataInfoUp[pContext->NextFreeWrBuff].FilterIndex = pContext->ItSvc[pDmaCtx->DmaFlags.ItemHdl].FilterParent;
         /* Set next free buffer in pool. */
         pContext->NextFreeWrBuff = (pContext->NextFreeWrBuff + 1)%(MAX_Q_BUFF);

         /* Display for debug. */
         DIB_DEBUG(CMMB_LOG, (CRB "NextFreeWrBuff : %d" CRA, pContext->NextFreeWrBuff));

         /* Forward data to driver. */
         DibB2DFwdRawData(pContext, DataSize , idx, pDmaCtx->DmaFlags.FirstFrag, pDmaCtx->DmaFlags.LastFrag);
      }
      else
      {
         DIB_DEBUG(CMMB_ERR, (CRB "DataInfoUp Overflow: Free Buffer" CRA));
         DibBridgeDesallocateRawBuffer(pContext, pDmaCtx->pHostAddr, DataSize);
      }
   }
   else
   {
      if (pContext->IpCallback.DataCmmb != NULL)
      {
         pContext->IpCallback.DataCmmb(pContext, idx, pDmaCtx->pHostAddr, pDmaCtx->DmaLen);
      }
      DibBridgeDesallocateRawBuffer(pContext, pDmaCtx->pHostAddr, pDmaCtx->DmaLen);
   }
}

/**
 * RS Matrix is in Host memory and Firmware has been acknowledged.
 * Memory copy: rebuilds Multiplex Frame(s) from RS Matrix.
 * Note that:
 * - There is no alignment in between RS matrices and multiplex frames.
 * - One RS matrix may or may not carry begin or end of multiplex frame.
 * - One RS matrix may carry up to eight multiplex frames from eight successive
 *   broadcast channel frames (eight seconds, one time slot per broadcast
 *   channel frame and eight time slot per RS matrix).
 * - One RS matrix may straddle several multiplex frames from consecutive
 *   physical layer frames.
 */
void DibBridgeCmmbRsmDone(struct DibBridgeContext *pContext, enum DibDataMode DataMode, struct DibBridgeDmaCtx *pDmaCtx)
{
   uint32_t Length;
   uint32_t NbRows;
   uint32_t NbCols;
   uint32_t FragDataSize;
   uint32_t  MfSize;
   uint8_t NbMfFragments;
   ELEM_HDL idx;
   uint32_t MfDescriptor;
   uint32_t TailAddress;
   uint8_t NbMfCols;
   uint8_t PhyFrameId;
   uint8_t BegOfMf;
   uint8_t EndOfMf;
   uint16_t MfColIndex;
   uint32_t RdOffset = 0;
   DIBSTATUS Status;
   uint8_t i;
   DIB_ASSERT(pDmaCtx->DmaLen > 0);
   DIB_ASSERT(pDmaCtx->pHostAddr);


   /* Number of rows and cols of RS matrix. */
   NbRows = pDmaCtx->ChipBaseMin;
   NbCols = pDmaCtx->ChipBaseMax;

   /* if MF complete not simple RSmatrix sent or NbRows = 0 in case of Mf0*/
   if ((pDmaCtx->DmaFlags.BlockType == 1) ||( NbRows == 0))
   {
      DibBridgeCmmbMfDone( pContext, DataMode, pDmaCtx);
      return;
   }

   /* Service context index. */
   idx = pDmaCtx->DmaFlags.ItemHdl;
   DIB_ASSERT(idx < DIB_MAX_NB_ITEMS);

   /* Total length of RS matrix (payload + descriptors). */
   Length = pDmaCtx->DmaLen;


   /* Number of multiplex frames straddled by RS matrix. */
   NbMfFragments = (uint8_t)((Length - (NbRows * NbCols)) / 4);

   /* Number of 4092-byte blocks needed to store a whole Multiplex Frame removing its ratio of RS columns */
   MfSize = (uint32_t)((pDmaCtx->DmaFlags.FrameId * 4092 * NbCols) / 240) ;

   /* Display for debug. */
   DIB_DEBUG(CMMB_LOG, (CRB "DibBridgeCmmbRsmDone (%d x %d): ItemHdl = %d, PayloadSize = %d, NbMfFragments = %d, MfAllocated = %d" CRA,
         NbRows, NbCols, idx, NbRows * NbCols, NbMfFragments, MfSize));


   /* Parse MF descriptors: one 32-bit word for each MF. */
   for(i = 0; i < NbMfFragments; i++)
   {
      /* 32-bit descriptors inserted at end of matrix payload. */
      TailAddress = NbRows * NbCols + 4 * i;
      MfDescriptor = pDmaCtx->pHostAddr[TailAddress]     << 24 |
               pDmaCtx->pHostAddr[TailAddress + 1] << 16 |
               pDmaCtx->pHostAddr[TailAddress + 2] <<  8 |
               pDmaCtx->pHostAddr[TailAddress + 3];

      /* Number of data columns for MF. */
      NbMfCols = (uint8_t)((MfDescriptor >> 24) & 0xff);

      /* Physical frame identifier of MF. */
      PhyFrameId = (uint8_t)((MfDescriptor >> 16) & 0x3f);

      /* End of MF flag. */
      EndOfMf = (uint8_t)((MfDescriptor >> 15) & 0x1);

      /* Destination (column index) of group of columns from RS matrix to MF. */
      MfColIndex = (uint16_t)(MfDescriptor & 0x7fff);
      BegOfMf = 0;
      if(MfColIndex == 0)
         BegOfMf = 1;

      FragDataSize = NbMfCols * NbRows;

      /* Display for debug. */
      DIB_DEBUG(CMMB_LOG, (CRB "  [%d]: PHY%02d, IDX=%03d, NBC=%03d, EOMF=%d" CRA, i, PhyFrameId, MfColIndex, NbMfCols, EndOfMf));

      /* Check MF descriptor. */
      Status = IntBridgeCheckMultiplexFrameDesc(pContext, idx, PhyFrameId, BegOfMf, MfColIndex * NbRows);
      if(Status != DIBSTATUS_SUCCESS)
      {
         /* Flush current MF (if any). */
         IntBridgeFlushMultiplexFrame(pContext, idx);

         /* Skip if not begin of multiplex frame. */
         if(BegOfMf == 0)
         {
            RdOffset += FragDataSize;
            continue;
         }
      }

      /* Begin of multiplex frame. */
      if(BegOfMf == 1)
      {
         /* Allocate memory for multiplex frame. */
         Status = IntBridgeAllocMultiplexFrame(pContext, idx, MfSize, PhyFrameId);
         if(Status != DIBSTATUS_SUCCESS)
         {
            /* Display error message and skip multiplex frame. */
            DIB_DEBUG(CMMB_ERR, (CRB "*** Error: MF Memory Allocation Failure" CRA));
            RdOffset += FragDataSize;
            continue;
         }
      }

      if(FragDataSize > 0)
      {
         /* Move payload data columns. */
         DibMoveMemory(pContext->ItSvc[idx].MultiplexFrame + pContext->ItSvc[idx].WrOffset, pDmaCtx->pHostAddr + RdOffset, FragDataSize);

         /* Update counts. */
         pContext->ItSvc[idx].WrOffset += FragDataSize;
         RdOffset += FragDataSize;
      }

      /* End of multiplex frame. */
      if(EndOfMf == 1)
      {
         if(pContext->ItSvc[idx].WrOffset == 0)
         {
            DIB_DEBUG(CMMB_ERR, (CRB "Empty multiplex frame" CRA));
            IntBridgeFlushMultiplexFrame(pContext, idx);
         }
         else
         {
#if (DIB_CHECK_CMMB_DATA == 1)
         /* Check multiplex frame payload. */
            DibBridgeCheckCmmbMultiplexFrame(pContext, pContext->ItSvc[idx].MultiplexFrame, pContext->ItSvc[idx].WrOffset, idx);
            DibBridgeForwardCheckStats(pContext, idx);
#endif /* DIB_CHECK_CMMB_DATA */

            if(DataMode == eCLBACK)
            {
               if(pContext->DataInfoUp[pContext->NextFreeWrBuff].pBuff != NULL)
               {
                  /* No free buffer in pool. */
                  DIB_DEBUG(CMMB_ERR, (CRB "DataInfoUp Overflow: Free Buffer" CRA));

                  /* Free multiplex frame buffer (discard data). */
                  IntBridgeFlushMultiplexFrame(pContext, idx);
               }
               else
               {
                  /* Use buffer. */
                  pContext->DataInfoUp[pContext->NextFreeWrBuff].pBuff = pContext->ItSvc[idx].MultiplexFrame;
                  pContext->DataInfoUp[pContext->NextFreeWrBuff].FilterIndex = pContext->ItSvc[pDmaCtx->DmaFlags.ItemHdl].FilterParent;

                  /* Set next free buffer in pool. */
                  pContext->NextFreeWrBuff = (pContext->NextFreeWrBuff + 1)%(MAX_Q_BUFF);

                  /* Display for debug. */
                  DIB_DEBUG(CMMB_LOG, (CRB "NextFreeWrBuff : %d" CRA, pContext->NextFreeWrBuff));

                  /* Forward data to driver. */
                  DibB2DFwdRawData(pContext, pContext->ItSvc[idx].WrOffset, idx, pDmaCtx->DmaFlags.FirstFrag, pDmaCtx->DmaFlags.LastFrag);

                  /* Clear multiplex frame buffer descriptor (memory will free after Driver retrieves data). */
                  pContext->ItSvc[idx].MultiplexFrame = NULL;
                  pContext->ItSvc[idx].Size = 0;
                  pContext->ItSvc[idx].PhyFrameId = 0xff;
                  pContext->ItSvc[idx].WrOffset = 0;
               }
            }
            else
            {
               /* Free multiplex frame buffer (discard data). */
               IntBridgeFlushMultiplexFrame(pContext, idx);
            }
         }
      }
   }

   /* Free memory allocated for RS matrix. */
   DibBridgeDesallocateRawBuffer(pContext, pDmaCtx->pHostAddr, pDmaCtx->DmaLen);
}

#endif /* DIB_CMMB_DATA */
