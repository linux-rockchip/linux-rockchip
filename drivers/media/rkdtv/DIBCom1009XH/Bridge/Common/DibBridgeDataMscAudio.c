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
* @file "DibBridgeDataMscAudio.c"
* @brief Tdmb Handling.
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
#include "DibBridgeData.h"

#if(DIB_DAB_DATA == 1)

void DibBridgeMscDone(struct DibBridgeContext *pContext, struct DibBridgeDmaCtx * pDmaCtx, ELEM_HDL Item)
{
#if (DIB_CHECK_MSC_DATA == 1)
#if (DIB_CHECK_DATA_IN_FILE == 1)
   static uint8_t FirstTime = 15;

   if(FirstTime > 0)
   {
      if(FirstTime == 5)
         DibBridgeCheckMscInit(pContext, pDmaCtx->pHostAddr, pDmaCtx->DmaLen, Item);
      FirstTime--;
   }
   else
   {
      DibBridgeCheckMscData(pContext, pDmaCtx->pHostAddr, pDmaCtx->DmaLen, Item);
   }
#else

   if(pContext->ItSvc[Item].DataLenRx == 0)
   {
      if(DibBridgeCheckMscInit(pContext, pDmaCtx->pHostAddr, pDmaCtx->DmaLen, Item) == 0)
         pContext->ItSvc[Item].DataLenRx = 0;
   }
   if(pContext->ItSvc[Item].DataLenRx != 0)
   {
      if(DibBridgeCheckMscData(pContext, pDmaCtx->pHostAddr, pDmaCtx->DmaLen, Item) == 0)
         pContext->ItSvc[Item].DataLenRx = 0;
   }   
#endif
#endif

   if (pContext->IpCallback.DataDab != NULL)
   {
      pContext->IpCallback.DataDab(pContext, Item, pDmaCtx->pHostAddr, pDmaCtx->DmaLen);
   }
}

void DibBridgeMscPlusDone(struct DibBridgeContext *pContext, struct DibBridgeDmaCtx * pDmaCtx, ELEM_HDL Item)
{
   if (pContext->IpCallback.DataDabPlus != NULL)
   {
      pContext->IpCallback.DataDabPlus(pContext, Item, pDmaCtx->pHostAddr, pDmaCtx->DmaLen);
   }
}
#endif
