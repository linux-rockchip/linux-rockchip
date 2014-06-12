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
* @file "DibBridgeRawTs.c"
* @brief RawTs buffer Handling.
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

#if (mSDK == 0)
/****************************************************************************
 * DibBridgeAllocateRawBuffer
 ****************************************************************************/
DIBSTATUS DibBridgeAllocateRawBuffer(struct DibBridgeContext *pContext, uint8_t** Buffer, uint32_t size)
{
   uint8_t status = DIBSTATUS_SUCCESS;

   *Buffer = (uint8_t *)DibMemAlloc(size);

   if ((*Buffer) == NULL) 
   {
      DIB_DEBUG(RAWTS_ERR, (CRB "RAWTS Allocate Buffer Fail" CRA));
      status = DIBSTATUS_ERROR;
   }
      
   DIB_DEBUG(RAWTS_LOG, (CRB "RAWTS Allocate Buffer Sucessfull" CRA));

   return (status);
}

/****************************************************************************
 * DibBridgeDesallocateRawBuffer
 ****************************************************************************/
DIBSTATUS DibBridgeDesallocateRawBuffer(struct DibBridgeContext *pContext, uint8_t *buffer, uint32_t size)
{
   DibMemFree(buffer, size);
   DIB_DEBUG(RAWTS_LOG, (CRB "RAWTS Free Buffer" CRA));
   return DIBSTATUS_SUCCESS;
}

/****************************************************************************
 * DibBridgeNotifyRawbufferWrite
 ****************************************************************************/
DIBSTATUS DibBridgeNotifyRawbufferWrite(struct DibBridgeContext *pContext, uint8_t *buffer, uint32_t len, uint8_t Item)
{
  DIB_DEBUG(RAWTS_LOG, (CRB "RAWTS Callback Push Mode" CRA));
  if (pContext->IpCallback.DataMpegTs != NULL)
  {
     pContext->IpCallback.DataMpegTs(pContext, Item, buffer, len);
  }
  return DibBridgeDesallocateRawBuffer(pContext, buffer,len);
}

/****************************************************************************
 * DibBridgeGetRawBufferFull
 ****************************************************************************/
DIBSTATUS DibBridgeGetRawBufferFull(struct DibBridgeContext *pContext, uint8_t** Buffer, FILTER_HDL * pFilterHdl)
{
   *Buffer = pContext->DataInfoUp[pContext->NextFreeRdBuff].pBuff;
    
   if(pFilterHdl)
   {
      if(*pFilterHdl != pContext->DataInfoUp[pContext->NextFreeRdBuff].FilterIndex)
      {
         DIB_DEBUG(RAWTS_ERR, (CRB "Error DibBridgeGetRawBufferFull Filter %d %d" CRA, *pFilterHdl, pContext->DataInfoUp[pContext->NextFreeRdBuff].FilterIndex));
         *Buffer = 0;
         *pFilterHdl = pContext->DataInfoUp[pContext->NextFreeRdBuff].FilterIndex;
         return DIBSTATUS_ERROR;
      }
   }
   
   pContext->DataInfoUp[pContext->NextFreeRdBuff].pBuff = NULL;
   pContext->DataInfoUp[pContext->NextFreeRdBuff].FilterIndex = DIB_UNSET;

   pContext->NextFreeRdBuff = (pContext->NextFreeRdBuff + 1)%(MAX_Q_BUFF); 

   DIB_DEBUG(RAWTS_LOG, (CRB "Read Raw Buffer from Queue (NextFreeRdBuff= %d)" CRA, pContext->NextFreeRdBuff));

   DIB_ASSERT(*Buffer);
   if((*Buffer) == 0)
   {
      DIB_DEBUG(RAWTS_ERR, (CRB "Error DibBridgeGetRawBufferFull" CRA));
      return DIBSTATUS_ERROR;
   }

   return DIBSTATUS_SUCCESS;
}

/****************************************************************************
 * DibBridgeNotifyRawbufferRead
 ****************************************************************************/
DIBSTATUS DibBridgeNotifyRawbufferRead(struct DibBridgeContext *pContext, uint8_t *buffer, uint32_t size)
{
   DibMemFree(buffer, size);
   DIB_DEBUG(RAWTS_LOG, (CRB "RAWTS Buffer Read => Free" CRA));
   return DIBSTATUS_SUCCESS;
}
#endif /*(mSDK == 0)*/
