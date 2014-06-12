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
* @file "DibDriverRegisterIf.c"
* @brief Internal Access functions.
*
***************************************************************************************************/
#include "DibDriverConfig.h" /* Must be first include of all SDK files - Defines compilation options */
#include "DibDriverConstants.h"
#include "DibDriverTargetTypes.h"
#include "DibDriverCommon.h"
#include "DibDriverTargetDebug.h"
#include "DibDriverDowncalls.h"
#include "DibDriverRegisterIf.h"
#include "DibDriverMessages.h"

/**
 * Read Data 
 * @param pContext  pointer to the context
 * @param Addr      Address to read from
 * @param NbTx      
 * @param Nb        
 * @param pBuf      
 */
DIBSTATUS DibDriverReadI2C(struct DibDriverContext *pContext, uint16_t Addr, uint8_t NbTx, uint32_t Nb, uint8_t *pBuf)
{
   DIBSTATUS ret = DIBSTATUS_ERROR;

   if(Nb > I2C_BRIDGE_FRAME_SIZE)
      return DIBSTATUS_INVALID_PARAMETER;

   DIB_DEBUG(MSG_LOG, (CRB "Reading on I2C with NbTx = %d" CRA, NbTx));

   DibAcquireLock(&pContext->BridgeRWLock);

   /*  Clear any remaining Event from a previous timeout */
   DibResetEvent(&pContext->BridgeRWEvent);

   ret = DibDriverI2CRead(pContext, (uint8_t) (Addr & 0xff), pBuf, NbTx, pBuf, (uint8_t) Nb);

   if(ret != DIBSTATUS_SUCCESS) 
   {
      DibReleaseLock(&pContext->BridgeRWLock);
      return ret;
   }

   DIB_DEBUG(MSG_LOG, (CRB "  DibDriverReadI2C: waiting for return Event " CRA));

   ret = DibWaitForEvent(&pContext->BridgeRWEvent, 1000);

   if(ret == DIBSTATUS_SUCCESS) 
   {
      DIB_DEBUG(MSG_LOG, (CRB "  DibDriverReadI2C: received Event " CRA));
      DibMoveMemory(pBuf, pContext->ChipContext.I2CBridgeDataRead, Nb);
      DibResetEvent(&pContext->BridgeRWEvent);
   }
   else 
   {
      DIB_DEBUG(MSG_LOG, (CRB "  DibDriverReadI2C: did not receive Event " CRA));
   }

   DibReleaseLock(&pContext->BridgeRWLock);
   return ret;
}

/**
 * Write Data 
 * @param pContext  pointer to the context
 * @param Addr      Address to write to
 * @param Nb        
 * @param pBuf      
 */
DIBSTATUS DibDriverWriteI2C(struct DibDriverContext *pContext, uint16_t Addr, uint32_t Nb, uint8_t *pBuf)
{
   DIBSTATUS ret = DIBSTATUS_ERROR;

   DibAcquireLock(&pContext->BridgeRWLock);

   /*  Clear any remaining Event from a previous timeout */
   DibResetEvent(&pContext->BridgeRWEvent);

   DIB_DEBUG(MSG_LOG, (CRB "  DibDriverWriteI2C: waiting for return Event " CRA));

   ret = DibDriverI2CWrite(pContext, (uint8_t) (Addr & 0xff), pBuf, (uint8_t) Nb);

   if(ret != DIBSTATUS_SUCCESS) 
   {
      DibReleaseLock(&pContext->BridgeRWLock);
      return ret;
   }

   ret = DibWaitForEvent(&pContext->BridgeRWEvent, 1000);        /*  1s */

   if(ret == DIBSTATUS_SUCCESS) 
   {
      DibResetEvent(&pContext->BridgeRWEvent);
   }
   else 
   {
      DIB_DEBUG(MSG_ERR, (CRB "DibDriverWriteI2C: Event timeout" CRA));
   }

   DibReleaseLock(&pContext->BridgeRWLock);

   return ret;
}

/**
 * Read Data from internal bus
 * @param pContext  pointer to the context
 * @param Addr      Address to read from
 * @param pData            
 */
DIBSTATUS DibDriverReadInternalBus(struct DibDriverContext *pContext, uint32_t Addr, uint32_t *pData)
{
   DIBSTATUS ret = DIBSTATUS_ERROR;

   DibAcquireLock(&pContext->BridgeRWLock);

   /*  Clear any remaining Event from a previous timeout */
   DibResetEvent(&pContext->BridgeRWEvent);

   if((ret = DibDriverApbRead(pContext, Addr)) != DIBSTATUS_SUCCESS) 
   {
      DibReleaseLock(&pContext->BridgeRWLock);
      return ret;
   }

   DIB_DEBUG(MSG_LOG, (CRB "  DibDriverReadInternalBus: waiting for return Event " CRA));
   ret = DibWaitForEvent(&pContext->BridgeRWEvent, 1000);        /* 1s */

   if(ret == DIBSTATUS_SUCCESS) 
   {
      DibResetEvent(&pContext->BridgeRWEvent);
      *pData = pContext->ChipContext.APBBridgeDataRead;
   }
   else 
   {
      DIB_DEBUG(MSG_ERR, (CRB "DibDriverReadInternalBus: Event timeout" CRA));
   }

   DibReleaseLock(&pContext->BridgeRWLock);

   return ret;
}

/**
 * Write Data to internal bus
 * @param pContext  pointer to the context
 * @param Addr      Address to write to
 * @param Data            
 */
DIBSTATUS DibDriverWriteInternalBus(struct DibDriverContext *pContext, uint32_t Addr, uint32_t Data)
{
   DIBSTATUS ret = DIBSTATUS_ERROR;

   DibAcquireLock(&pContext->BridgeRWLock);

   /*  Clear any remaining Event from a previous timeout */
   DibResetEvent(&pContext->BridgeRWEvent);

   if((ret = DibDriverApbWrite(pContext, Addr, Data)) != DIBSTATUS_SUCCESS) 
   {
      DIB_DEBUG(MSG_ERR, (CRB "DibDriverApbWrite returned an error" CRA));
      DibReleaseLock(&pContext->BridgeRWLock);
      return ret;
   }

   DIB_DEBUG(MSG_LOG, (CRB "  DibDriverWriteInternalBus: waiting for return Event " CRA));

   ret = DibWaitForEvent(&pContext->BridgeRWEvent, 1000);        /*  1s */

   DibReleaseLock(&pContext->BridgeRWLock);

   if(ret == DIBSTATUS_SUCCESS) 
   {
      DibResetEvent(&pContext->BridgeRWEvent);
      return DIBSTATUS_SUCCESS;
   }
   else if(ret == DIBSTATUS_TIMEOUT) 
   {
      return DIBSTATUS_TIMEOUT;
   }
   else 
   {
      return DIBSTATUS_ERROR;
   }
}

/****************************************************************************
*
****************************************************************************/
DIBSTATUS DibDriverReadDirectBus(struct DibDriverContext *pContext, uint32_t Addr, uint32_t *pData)
{
   DIBSTATUS ret = DIBSTATUS_SUCCESS;

   DibAcquireLock(&pContext->BridgeRWLock);

   if(pContext->DibChip == DIB_FIREFLY)
   {
      uint16_t Value;

      DibDriverReadReg16(pContext, Addr, &Value);
      *pData = Value;
   }
   else if((pContext->DibChip == DIB_VOYAGER) || (pContext->DibChip == DIB_NAUTILUS))
   {
      DibDriverReadReg32(pContext, Addr, pData);
   }

   DibReleaseLock(&pContext->BridgeRWLock);

   return ret;
}

/****************************************************************************
*
****************************************************************************/
DIBSTATUS DibDriverWriteDirectBus(struct DibDriverContext *pContext, uint32_t Addr, uint32_t Data)
{
   DIBSTATUS ret = DIBSTATUS_SUCCESS;

   DibAcquireLock(&pContext->BridgeRWLock);

   if(pContext->DibChip == DIB_FIREFLY)
   {
      uint16_t Value = (uint16_t)Data;
      DibDriverWriteReg16(pContext, Addr, Value);
   }
   else if((pContext->DibChip == DIB_VOYAGER) || (pContext->DibChip == DIB_NAUTILUS))
   {
      DibDriverWriteReg32(pContext, Addr, Data);
   }

   DibReleaseLock(&pContext->BridgeRWLock);

   return ret;
}

DIBSTATUS DibDriverWriteReg8(struct DibDriverContext *pContext, uint32_t Addr, uint8_t Value)
{
   return DibD2BWriteReg(pContext, Addr, DIBBRIGDE_BIT_MODE_8, 1, &Value);
}

DIBSTATUS DibDriverWriteReg16(struct DibDriverContext *pContext, uint32_t Addr, uint16_t Value)
{
   uint8_t txbuffer[2];

   txbuffer[0] = (Value >> 8) & 0xFF;
   txbuffer[1] = (Value) & 0xFF;
   return DibD2BWriteReg(pContext, Addr, DIBBRIGDE_BIT_MODE_16, 2, txbuffer);
}

DIBSTATUS DibDriverWriteReg32(struct DibDriverContext *pContext, uint32_t Addr, uint32_t Value)
{
   uint8_t txbuffer[4];

   txbuffer[0] = (Value >> 24) & 0xFF;
   txbuffer[1] = (Value >> 16) & 0xFF;
   txbuffer[2] = (Value >> 8)  & 0xFF;
   txbuffer[3] = (Value) & 0xFF;
   return DibD2BWriteReg(pContext, Addr, DIBBRIGDE_BIT_MODE_32, 4, txbuffer);
}

DIBSTATUS DibDriverReadReg16(struct DibDriverContext *pContext, uint32_t Addr, uint16_t * Value)
{
   uint8_t rxbuffer[2];
   DIBSTATUS ret = DIBSTATUS_ERROR;

   ret    = DibD2BReadReg(pContext, Addr, DIBBRIGDE_BIT_MODE_16, 2, rxbuffer);
   *Value = (((rxbuffer[0] << 8) & 0xff00) | (rxbuffer[1] & 0xff)) & 0xffff;
   return ret;
}

DIBSTATUS DibDriverReadReg8(struct DibDriverContext *pContext, uint32_t Addr, uint8_t * Value)
{
   DIBSTATUS ret = DIBSTATUS_ERROR;

   ret = DibD2BReadReg(pContext, Addr, DIBBRIGDE_BIT_MODE_8, 1, Value);
   return ret;
}


DIBSTATUS DibDriverReadReg32(struct DibDriverContext *pContext, uint32_t Addr, uint32_t *Value)
{
   uint32_t ByteMode = DIBBRIGDE_BIT_MODE_32;
   uint8_t rxbuffer[4];
   DIBSTATUS ret = DIBSTATUS_ERROR;

#if ENG3_COMPATIBILITY == 1
   /* Force 32 bit read register access to be 8 bit ones due to sh interleaver conflict with voyager full-mask chipsets */
   if(pContext->DibChip == DIB_VOYAGER)
      ByteMode = DIBBRIGDE_BIT_MODE_8;
#endif

   ret    = DibD2BReadReg(pContext, Addr, ByteMode, 4, rxbuffer);
   *Value = ((rxbuffer[0] << 24) & 0xFF000000) |
            ((rxbuffer[1] << 16) & 0xFF0000)   |
            ((rxbuffer[2] << 8)  & 0xFF00)     |
            ((rxbuffer[3])       & 0xFF);
   return ret;
}
