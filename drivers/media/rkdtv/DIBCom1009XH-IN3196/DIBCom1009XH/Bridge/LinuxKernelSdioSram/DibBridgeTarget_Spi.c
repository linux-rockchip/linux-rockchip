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
#include "DibBridgeCommon.h"
#include "DibBridgeTargetDebug.h"
#include "DibBridgeTarget.h"
#include "DibBridgeTargetCommon.h"
#include "DibBridge.h"
#include "DibBridgeMailboxHandler.h"


#if (DIBINTERF_PHY == DIBINTERF_PHY_SPI)

#include "DibSpi.h"

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

  /*** Configure Driver for SPI interface ***/
   DibBridgeSetHostIfMode(pContext, eSDIO);

   /*** Init Dib SPI protocol and SPI bus ***/
   DibSpiInit(pContext);

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

   /*** DeInit Dib SPI protocol and SPI bus ***/
   DibSpiDeInit(pContext);

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

   while(Cnt > 0)
   {
     if(Cnt > DIBSPI_MAX_WR_SIZE)
     {
       DibSpiWrite(pContext,Addr,pBuf,DIBSPI_MAX_WR_SIZE);
       Cnt -= DIBSPI_MAX_WR_SIZE;
       pBuf += DIBSPI_MAX_WR_SIZE;
     }
     else
     {
       DibSpiWrite(pContext,Addr,pBuf,Cnt);
       Cnt = 0;
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

   while(Cnt > 0) 
   {
    
     if(Cnt > DIBSPI_MAX_RD_SIZE)
     {
       DibSpiRead(pContext,Addr,pBuf,DIBSPI_MAX_RD_SIZE);
       Cnt -= DIBSPI_MAX_RD_SIZE;
       pBuf += DIBSPI_MAX_RD_SIZE;
     }
     else
     { 
       DibSpiRead(pContext,Addr,pBuf,Cnt);
       Cnt = 0;
     }
   }

   /* Release semaphore */
   DibReleaseLock(&pContext->DibTargetAccessLock);
   return DIBSTATUS_SUCCESS;
}

#endif
