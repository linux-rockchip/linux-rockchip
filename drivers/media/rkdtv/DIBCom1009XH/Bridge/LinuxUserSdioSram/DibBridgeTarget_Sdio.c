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

#if (DIBINTERF_PHY == DIBINTERF_PHY_SDIO)

#include "Sdio.h"

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

  /*** Configure Driver for SDIO interface ***/
   DibBridgeSetHostIfMode(pContext, eSDIO);

   /*** Initialize SDIO Controller ***/
   SdioInit(pContext);

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
DIBSTATUS DibBridgeTargetWrite(struct DibBridgeContext *pContext, uint32_t FmtAddr, uint8_t ByteMode, uint32_t Cnt, uint8_t *pBuf)
{
   DIBSTATUS rc = DIBSTATUS_SUCCESS;
   SDIO_CMD sdiocmd;

   /* In word Mode the write count has to be even */
   if((ByteMode == DIBBRIDGE_BIT_MODE_16) && (Cnt & 1)) 
   {
      DIB_DEBUG(PORT_LOG, (CRB "-E- DibBridgeTargetWrite: Odd byte count" CRA));
      return DIBSTATUS_INVALID_PARAMETER;
   }

   /* Get semaphore to avoid concurrent access */
   DibAcquireLock(&pContext->DibTargetAccessLock);

   /* For 32 bit architectures, we need to write high 16 bytes before transfer */
   if(DibBridgeGetArch(pContext) == DIBBRIDGE_BIT_MODE_32)
   {
      /* Use CMD52 to configure address for 32 bits platforms. Note, cmd53 is not working here */
      sdiocmd.cmd    = SDIO_CMD_52;     
      sdiocmd.addr   = 0x10000 | ((FmtAddr>>16) & 0xFFFF);
      sdiocmd.fct    = 1;
      sdiocmd.buffer = &ByteMode;
      sdiocmd.rw      = SDIO_CMD_WRITE; 
      SdioCmd52(pContext,&sdiocmd);
   }

   while(Cnt > 0) 
   {
      if(Cnt > SDIO_MAX_WR_SIZE) 
      {
          /*** Prepare the SDIO Command ****/
          sdiocmd.cmd		= SDIO_CMD_53;     
          sdiocmd.addr		= FmtAddr & 0x0000FFFF;         
          sdiocmd.rw		= SDIO_CMD_WRITE;  
          sdiocmd.buffer	= pBuf;            
          sdiocmd.size		= SDIO_MAX_WR_SIZE;   
          sdiocmd.fct   	= 1;
          sdiocmd.block   	= 0;
          sdiocmd.block_size	= 0;

          /*** Perform the SDIO transfer ****/
          SdioCmd53(pContext,&sdiocmd);
   	  FmtAddr = DibBridgeIncrementFormattedAddress(pContext, FmtAddr, SDIO_MAX_WR_SIZE);

          Cnt  -= SDIO_MAX_WR_SIZE;
          pBuf += SDIO_MAX_WR_SIZE;
       }
       else 
       {
          /*** Prepare the SDIO Command ****/
          sdiocmd.cmd		= SDIO_CMD_53;
          sdiocmd.addr		= FmtAddr & 0x0000FFFF;         
          sdiocmd.rw		= SDIO_CMD_WRITE;
          sdiocmd.buffer	= pBuf;
          sdiocmd.size		= Cnt;
          sdiocmd.fct   	= 1;
          sdiocmd.block   	= 0;
          sdiocmd.block_size	= 0;

          /*** Perform the SDIO transfer ****/
          SdioCmd53(pContext,&sdiocmd);
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
DIBSTATUS DibBridgeTargetRead(struct DibBridgeContext *pContext, uint32_t FmtAddr, uint8_t ByteMode, uint32_t Cnt, uint8_t * pBuf)
{
   SDIO_CMD sdiocmd;
   /* In word Mode the byte count has to be even */
   if((ByteMode != DIBBRIDGE_BIT_MODE_8) && ((Cnt & 1) != 0)) 
   {
      DIB_DEBUG(PORT_LOG, (CRB "-E- DibBridgeTargetRead: Odd byte count" CRA));
      return DIBSTATUS_INVALID_PARAMETER;
   }

   /* Get semaphore to avoid concurrent access */
   DibAcquireLock(&pContext->DibTargetAccessLock);

   /* For 32 bit architectures, we need to write high 16 bytes before transfer */
   if(DibBridgeGetArch(pContext) == DIBBRIDGE_BIT_MODE_32)
   {
      /* Use CMD52 to configure address for 32 bits platforms. Note, cmd53 is not working here */
      sdiocmd.cmd    = SDIO_CMD_52;     
      sdiocmd.addr   = 0x10000 | ((FmtAddr>>16) & 0xFFFF);
      sdiocmd.fct    = 1;
      sdiocmd.buffer = &ByteMode;
      sdiocmd.rw     = SDIO_CMD_WRITE; 
      SdioCmd52(pContext,&sdiocmd);
   }

   while(Cnt > 0) 
   {
      if(Cnt > SDIO_MAX_RD_SIZE) 
      {
          /*** Prepare the SDIO Command ****/
          sdiocmd.cmd		= SDIO_CMD_53;     
          sdiocmd.addr		= FmtAddr  & 0x0000FFFF;
          sdiocmd.rw		= SDIO_CMD_READ;   
          sdiocmd.buffer	= pBuf;            
          sdiocmd.size		= SDIO_MAX_RD_SIZE;   
          sdiocmd.fct   	= 1;
          sdiocmd.block   	= 0;
          sdiocmd.block_size	= 0;

          /*** Perform the SDIO transfer ****/
          SdioCmd53(pContext,&sdiocmd);
   	  FmtAddr = DibBridgeIncrementFormattedAddress(pContext, FmtAddr, SDIO_MAX_RD_SIZE);

          Cnt  -= SDIO_MAX_RD_SIZE;
          pBuf += SDIO_MAX_RD_SIZE;
       }
       else 
       {
          /*** Prepare the SDIO Command ****/
          sdiocmd.cmd		= SDIO_CMD_53;
          sdiocmd.addr		= FmtAddr & 0x0000FFFF;
          sdiocmd.rw		= SDIO_CMD_READ;
          sdiocmd.buffer	= pBuf;
          sdiocmd.size		= Cnt;
          sdiocmd.fct   	= 1;
          sdiocmd.block   	= 0;
          sdiocmd.block_size	= 0;

          /*** Perform the SDIO transfer ****/
          SdioCmd53(pContext,&sdiocmd);
          Cnt = 0;
       }
   }

   /* Release semaphore */
   DibReleaseLock(&pContext->DibTargetAccessLock);
   return DIBSTATUS_SUCCESS;
}

#endif /*  */
