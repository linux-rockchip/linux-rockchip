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
* @file "SdioSkeleton.c"
* @brief Target Specific.
*
***************************************************************************************************/
#include <pthread.h>
#include "DibBridgeCommon.h"
#include "DibBridgeTargetDebug.h"
#include "DibBridgeTarget.h"
#include "DibBridgeTargetCommon.h"
#include "DibBridge.h"
#include "DibBridgeMailboxHandler.h"

#if (DIBINTERF_PHY == DIBINTERF_PHY_SRAM)

#include "Sram.h"

/****************************************************************************
* DibBridgeTargetInit
****************************************************************************/
DIBSTATUS DibBridgeTargetInit(struct DibBridgeContext *pContext)
{
   DIBSTATUS rc = DIBSTATUS_SUCCESS;

   /*** Initialize Mutex to avoid concurrent access ***/
   DibAllocateLock(&pContext->DibTargetAccessLock);
   DibInitLock(&pContext->DibTargetAccessLock);

   DibAllocateLock(&pContext->BridgeTargetCtx.IrqLock);
   DibInitLock(&pContext->BridgeTargetCtx.IrqLock);

   /*** Configure Driver for SRAM interface ***/
   DibBridgeSetHostIfMode(pContext, eSRAM);

   /*** Initialize SRAM Controller ***/
   SramInit(pContext);

   return rc;
}

/*-----------------------------------------------------------------------
|  Post Initialize Physical Interface
-----------------------------------------------------------------------*/
DIBSTATUS DibBridgeTargetSetHighSpeed(struct DibBridgeContext *pContext)
{
   /* target specific postinit : high-speed */
   return DIBSTATUS_SUCCESS;
}

/****************************************************************************
* DibBridgephy_deinit
****************************************************************************/
DIBSTATUS DibBridgeTargetDeinit(struct DibBridgeContext *pContext)
{
   DibDeAllocateLock(&pContext->DibTargetAccessLock);
   DibDeAllocateLock(&pContext->BridgeTargetCtx.IrqLock);
   return DIBSTATUS_SUCCESS;
}

/****************************************************************************
* Write Data to the chip
* ----------------------------------------------------------------------
* Parameters:
* -----------
* struct DibBridgeContext *pContext
* uint16_t      Addr:          Address to write
* uint8_t       ByteMode:      Transfer Mode
*                 0: 16 bits wide register write
*                 1: 8 bits wide FIFO write
* uint32_t       Cnt:      Number of Bytes
* uint8_t      *pBuf:      Data Buffer
* ----------------------------------------------------------------------
* Return Code:
* -----------
*           DIBSTATUS_SUCCESS
*           DIBSTATUS_ERROR
****************************************************************************/
DIBSTATUS DibBridgeTargetWrite(struct DibBridgeContext *pContext, uint32_t Addr, uint8_t ByteMode, uint32_t Cnt, uint8_t *pBuf)
{
   DIBSTATUS rc = DIBSTATUS_SUCCESS;

   /* In word Mode the write count has to be even */
   if((ByteMode == DIBBRIDGE_BIT_MODE_16) && (Cnt & 1)) 
   {
      DIB_DEBUG(PORT_LOG, (CRB "-E- DibBridgeTargetWrite: Odd byte count" CRA));
      return DIBSTATUS_INVALID_PARAMETER;
   }

   /* Get semaphore to avoid concurrent access */
   DibAcquireLock(&pContext->DibTargetAccessLock);

   /**************************************************************/
   /********* 16 bits Architecture Dibcom Chips: *****************/
   /**************************************************************/
   /********* Dib7070, Dib7078, Dib9080, Dib9090 *****************/
   /**************************************************************/
   if(DibBridgeGetArch(pContext) == DIBBRIDGE_BIT_MODE_16)
   {
      /* write address on the bus */
      SramWrite(pContext,DIB16_ADDR_MSB, Addr >> 8);
      SramWrite(pContext,DIB16_ADDR_LSB, Addr & 0xff);

      /* write the data on the bus */
      if(ByteMode == DIBBRIDGE_BIT_MODE_8)
      {
         while(Cnt)
         {
           Cnt--;
           SramWrite(pContext,DIB16_DATA_LSB, *(pBuf++));
         }
      }
      else
      {
         Cnt >>= 1;        /* number of WORDS */
         while(Cnt)
         {
            Cnt--;
            SramWrite(pContext,DIB16_DATA_MSB, *(pBuf++));
            SramWrite(pContext,DIB16_DATA_LSB, *(pBuf++));
         }
      }
    }
   /**************************************************************/
   /********* 32 bits Architecture Dibcom Chips: *****************/
   /**************************************************************/
   /******************* Dib29098, Dib10098 ***********************/
   /**************************************************************/
    else if(DibBridgeGetArch(pContext) == DIBBRIDGE_BIT_MODE_32)
    {
       /* write address on the bus */
       SramWrite(pContext,DIB32_ADDR_MSB, (Addr >> 24) & 0xff);
       SramWrite(pContext,DIB32_ADDR_mSB, (Addr >> 16) & 0xff);
       SramWrite(pContext,DIB32_ADDR_lSB, (Addr >> 8) & 0xff);
       SramWrite(pContext,DIB32_ADDR_LSB, Addr & 0xff);

       /* write the data on the bus */
       if(ByteMode == DIBBRIDGE_BIT_MODE_8)
       {
          while(Cnt)
          {
             Cnt--;
             SramWrite(pContext,DIB32_DATA_LSB, *(pBuf++));
          }
       }
       else if(ByteMode == DIBBRIDGE_BIT_MODE_16)
       {
          Cnt >>= 1;        /* number of HALFWORDS */
          while(Cnt)
          {
             Cnt--;
             SramWrite(pContext,DIB32_DATA_lSB, *(pBuf++));
             SramWrite(pContext,DIB32_DATA_LSB, *(pBuf++));
          }
       }
       else
       {
          Cnt >>= 2;        /* number of WORDS */
          while(Cnt)
          {
             Cnt--;
             SramWrite(pContext,DIB32_DATA_MSB, *(pBuf++));
             SramWrite(pContext,DIB32_DATA_mSB, *(pBuf++));
             SramWrite(pContext,DIB32_DATA_lSB, *(pBuf++));
             SramWrite(pContext,DIB32_DATA_LSB, *(pBuf++));
          }
       }
    }

   /*** Release Dib Interf Semaphore ***/
   DibReleaseLock(&pContext->DibTargetAccessLock);
   return rc;
}

/****************************************************************************
* Read Data from the chip
-* ----------------------------------------------------------------------
* Parameters:
* -----------
* struct DibBridgeContext *pContext
* uint16_t      Addr:          Address to Read
* uint8_t       ByteMode:      Transfer Mode
*                 0: 16 bits wide register read
*                 1: 8 bits wide FIFO read
* uint32_t        Cnt:            Number of Bytes
* uint8_t       *pBuf:          Data Buffer
-* ----------------------------------------------------------------------
* Return Code:
* -----------
*           DIBSTATUS_SUCCESS
*           DIBSTATUS_ERROR
****************************************************************************/
DIBSTATUS DibBridgeTargetRead(struct DibBridgeContext *pContext, uint32_t Addr, uint8_t ByteMode, uint32_t Cnt, uint8_t * pBuf)
{
   /* In word Mode the byte count has to be even */
   if((ByteMode != DIBBRIDGE_BIT_MODE_8) && ((Cnt & 1) != 0)) 
   {
      DIB_DEBUG(PORT_LOG, (CRB "-E- DibBridgeTargetRead: Odd byte count" CRA));
      return DIBSTATUS_INVALID_PARAMETER;
   }

   /* Get semaphore to avoid concurrent access */
   DibAcquireLock(&pContext->DibTargetAccessLock);

   /**************************************************************/
   /********* 16 bits Architecture Dibcom Chips: *****************/
   /**************************************************************/
   /********* Dib7070, Dib7078, Dib9080, Dib9090 *****************/
   /**************************************************************/
   if(DibBridgeGetArch(pContext) == DIBBRIDGE_BIT_MODE_16)
   {
      /* write address on the bus */
      SramWrite(pContext,DIB16_ADDR_MSB, Addr >> 8);
      SramWrite(pContext,DIB16_ADDR_LSB, Addr & 0xff);

      /* write the data on the bus */
      if(ByteMode == DIBBRIDGE_BIT_MODE_8)
      {
         while(Cnt)
         {
            Cnt--;
            *(pBuf++) = SramRead(pContext,DIB16_DATA_LSB);
         }
      }
      else
      {
         Cnt >>= 1;        /* number of WORDS */
         while(Cnt)
         {
            Cnt--;
            *(pBuf++) = SramRead(pContext,DIB16_DATA_MSB);
            *(pBuf++) = SramRead(pContext,DIB16_DATA_LSB);
         }
      }
   }
   /**************************************************************/
   /********* 32 bits Architecture Dibcom Chips: *****************/
   /**************************************************************/
   /******************* Dib29098, Dib10098 ***********************/
   /**************************************************************/
   else if(DibBridgeGetArch(pContext) == DIBBRIDGE_BIT_MODE_32)
   {
       /* write address on the bus */
       SramWrite(pContext,DIB32_ADDR_MSB, (Addr >> 24) & 0xff);
       SramWrite(pContext,DIB32_ADDR_mSB, (Addr >> 16) & 0xff);
       SramWrite(pContext,DIB32_ADDR_lSB, (Addr >> 8) & 0xff);
       SramWrite(pContext,DIB32_ADDR_LSB, Addr & 0xff);

       /* write the data on the bus */
       if(ByteMode == DIBBRIDGE_BIT_MODE_8)
       {
          while(Cnt)
          {
             Cnt--;
             *(pBuf++) = SramRead(pContext,DIB32_DATA_LSB);
          }
       }
       else if(ByteMode == DIBBRIDGE_BIT_MODE_16)
       {
          Cnt >>= 1;        /* number of HALFWORDS */
          while(Cnt)
          {
             Cnt--;
             *(pBuf++) = SramRead(pContext,DIB32_DATA_lSB);
             *(pBuf++) = SramRead(pContext,DIB32_DATA_LSB);
          }
       }
       else
       {
          Cnt >>= 2;        /* number of WORDS */
          while(Cnt)
          {
             Cnt--;
             *(pBuf++) = SramRead(pContext,DIB32_DATA_MSB);
             *(pBuf++) = SramRead(pContext,DIB32_DATA_mSB);
             *(pBuf++) = SramRead(pContext,DIB32_DATA_lSB);
             *(pBuf++) = SramRead(pContext,DIB32_DATA_LSB);
          }
       }
    }


   /* Release semaphore */
   DibReleaseLock(&pContext->DibTargetAccessLock);
   return DIBSTATUS_SUCCESS;
}

#endif
