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
#include <semaphore.h>

#if (DIBINTERF_PHY == DIBINTERF_PHY_I2C)
#include "I2c.h"

/* address to use for single boards */
#define DEVICE_ADDRESS 0x80
/* address to use for diversity boards */
/* #define DEVICE_ADDRESS 0x7F */

struct DibBridgeContext *pLocalContext;

/****************************************************************************
* DibBridgeTargetInit
****************************************************************************/
DIBSTATUS DibBridgeTargetInit(struct DibBridgeContext *pContext)
{
   DIBSTATUS rc = DIBSTATUS_SUCCESS;
   uint8_t usrAddr;

   /*** Initialize Mutex to avoid concurrent access ***/

   DibAllocateLock(&pContext->DibTargetAccessLock);
   DibInitLock(&pContext->DibTargetAccessLock);

   DibAllocateLock(&pContext->BridgeTargetCtx.IrqLock);
   DibInitLock(&pContext->BridgeTargetCtx.IrqLock);

   /*** Configure Driver for I2C interface ***/
   DibBridgeSetHostIfMode(pContext, eI2C);

   /*** Use a global variable to call in DibWaitForEvent ***/
   pLocalContext = pContext;

   /*** Initialize I2C by Parallel port **i*/
   I2CInit();

   /*** Set user i2c address
    * LSBit is the "use_user_i2c_address" flag (1=user, 0=default)
    * In default mode, the address the chip responds to depends on the DIB_START_IN pin
    */
   usrAddr = DEVICE_ADDRESS;
   DibBridgeTargetWrite(pContext, 0x8000b02c, DIBBRIDGE_BIT_MODE_8, 1, &usrAddr);

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
   uint32_t   status, nb_lack, device_address, curk;
   uint32_t  new_addr;
   uint32_t   k, n;

   /* In word Mode the write count has to be even */
   if((ByteMode == DIBBRIDGE_BIT_MODE_16) && (Cnt & 1)) 
   {
      DIB_DEBUG(PORT_LOG, (CRB "-E- DibBridgeTargetWrite: Odd byte count" CRA));
      return DIBSTATUS_INVALID_PARAMETER;
   }

   /* Get semaphore to avoid concurrent access */
   DibAcquireLock(&pContext->DibTargetAccessLock);

   /* Add here the Code */
   device_address =  DEVICE_ADDRESS;
   status         = 0;
   nb_lack        = Cnt;
   new_addr       = Addr;
   curk           = 0;

   /**************************************************************/
   /********* 16 bits Architecture Dibcom Chips: *****************/
   /**************************************************************/
   /********* Dib7070, Dib7078, Dib9080, Dib9090 *****************/
   /**************************************************************/
   if(DibBridgeGetArch(pContext) == DIBBRIDGE_BIT_MODE_16)
   {

     while(nb_lack != 0)
     {
        pContext->BridgeTargetCtx.DataBuffer[0] = (new_addr>>8) & 0xFF;
        pContext->BridgeTargetCtx.DataBuffer[1] = (new_addr   ) & 0xFF;
  
        if(nb_lack>I2C_MAX_RDWR_SIZE)
        {
           n = I2C_MAX_RDWR_SIZE;
        }
        else
        {
           n = nb_lack;
        }
  
          nb_lack -= n; 
  
        pContext->BridgeTargetCtx.DataBuffer[0] |= 0x10;
  
        if(ByteMode == DIBBRIDGE_BIT_MODE_8) 
        {
             pContext->BridgeTargetCtx.DataBuffer[0] |= 0x20;
        }
  
        for(k=0 ; k<n ; k++)
          {
           pContext->BridgeTargetCtx.DataBuffer[k+2] = pBuf[k+curk];
        }
        curk += n;
    
        status |= I2CWrite(device_address, pContext->BridgeTargetCtx.DataBuffer, 2+n, ByteMode );
     }
    
   }
   /**************************************************************/
   /********* 32 bits Architecture Dibcom Chips: *****************/
   /**************************************************************/
   /******************* Dib29098, Dib10098 ***********************/
   /**************************************************************/
   else if(DibBridgeGetArch(pContext) == DIBBRIDGE_BIT_MODE_32)
   {
     while(nb_lack != 0)
     {
        pContext->BridgeTargetCtx.DataBuffer[1] =  (new_addr >> 24 ) & 0xFF;
        pContext->BridgeTargetCtx.DataBuffer[2] =  (new_addr >> 16 ) & 0xFF;
        pContext->BridgeTargetCtx.DataBuffer[3] =  (new_addr >> 8 ) & 0xFF;
        pContext->BridgeTargetCtx.DataBuffer[4] =  (new_addr      ) & 0xFF;
        pContext->BridgeTargetCtx.DataBuffer[0] =  0x20 | (ByteMode << 3);  

        if(nb_lack>I2C_MAX_RDWR_SIZE)
        {
           n = I2C_MAX_RDWR_SIZE;
        }
        else
        {
           n = nb_lack;
        }
  
        nb_lack -= n; 
  
        for(k=0 ; k<n ; k++)
        {
           pContext->BridgeTargetCtx.DataBuffer[k+5] = pBuf[k+curk];
        }
        curk += n;
    
        status |= I2CWrite(device_address, pContext->BridgeTargetCtx.DataBuffer, 5+n, ByteMode );

        new_addr = DibBridgeIncrementFormattedAddress(pContext, new_addr, I2C_MAX_RDWR_SIZE);
     }
   }

   if(status) 
   {
      DIB_DEBUG(PORT_LOG, (CRB "-E- DibBridgeTargetWrite: Error I2C" CRA));
      rc = DIBSTATUS_ERROR;
   }

   /*** Release  Semaphore ***/
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
   DIBSTATUS rc = DIBSTATUS_SUCCESS;
   int32_t status, nb_lack, device_address, curk;
   uint32_t new_addr;
   uint32_t k , n ;
   uint8_t txbuffer[5]  ;

   /* In word Mode the byte count has to be even */
   if((ByteMode == DIBBRIDGE_BIT_MODE_16) && ((Cnt & 1) != 0)) 
   {
      DIB_DEBUG(PORT_LOG, (CRB "-E- DibBridgeTargetRead: Odd byte count" CRA));
      return DIBSTATUS_INVALID_PARAMETER;
   }

   /* Get semaphore to avoid concurrent access */
   DibAcquireLock(&pContext->DibTargetAccessLock);

   /* Add here the code */
   status         = 0;
   nb_lack        = Cnt;
   device_address = DEVICE_ADDRESS;
   new_addr       = Addr;
   curk           = 0;

   /**************************************************************/
   /********* 16 bits Architecture Dibcom Chips: *****************/
   /**************************************************************/
   /********* Dib7070, Dib7078, Dib9080, Dib9090 *****************/
   /**************************************************************/
   if(DibBridgeGetArch(pContext) == DIBBRIDGE_BIT_MODE_16)
   {
     while(nb_lack != 0)
     {
       txbuffer[0] =  (new_addr >> 8 ) & 0xFF;
       txbuffer[1] =  (new_addr      ) & 0xFF;

       if(nb_lack > I2C_MAX_RDWR_SIZE)
          n = I2C_MAX_RDWR_SIZE;
       else
          n = nb_lack;
 
       nb_lack -= n;
    
       txbuffer[0] |= 0x10;

       if(ByteMode == DIBBRIDGE_BIT_MODE_8)
       {
          txbuffer[0] |= 0x20;
       }
  
       status |= I2CRead( device_address, txbuffer, 2, pContext->BridgeTargetCtx.DataBuffer, n, ByteMode); 
  
       for(k = 0; k < n; k++)
       {
          pBuf[curk+k] =  pContext->BridgeTargetCtx.DataBuffer[k];
       }
  
       curk += n;
     }
   }
   /**************************************************************/
   /********* 32 bits Architecture Dibcom Chips: *****************/
   /**************************************************************/
   /******************* Dib29098, Dib10098 ***********************/
   /**************************************************************/
   else if(DibBridgeGetArch(pContext) == DIBBRIDGE_BIT_MODE_32)
   {
     while(nb_lack != 0)
     {
        txbuffer[1] =  (new_addr >> 24 ) & 0xFF;
        txbuffer[2] =  (new_addr >> 16 ) & 0xFF;
        txbuffer[3] =  (new_addr >> 8 ) & 0xFF;
        txbuffer[4] =  (new_addr      ) & 0xFF;
        txbuffer[0] =  0x20 | (ByteMode << 3);  

  
        if(nb_lack > I2C_MAX_RDWR_SIZE) 
           n = I2C_MAX_RDWR_SIZE;
        else 
           n = nb_lack;
        
        nb_lack -= n;
  
        status |= I2CRead( device_address, txbuffer, 5, pContext->BridgeTargetCtx.DataBuffer, n, ByteMode); 

        for(k = 0; k < n; k++)
        {
           pBuf[curk+k] =  pContext->BridgeTargetCtx.DataBuffer[k];
        }
        curk += n;

        new_addr = DibBridgeIncrementFormattedAddress(pContext, new_addr, I2C_MAX_RDWR_SIZE);

     }
   }

   if(status) 
   {
      DIB_DEBUG(PORT_LOG, (CRB "-E- DibBridgeTargetRead: Error I2C" CRA));
      rc = DIBSTATUS_ERROR;
   }

   /* Release semaphore */
   DibReleaseLock(&pContext->DibTargetAccessLock);
   return rc;
}

#endif
