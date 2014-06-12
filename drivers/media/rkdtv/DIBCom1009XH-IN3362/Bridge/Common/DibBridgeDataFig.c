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
* @file "DibBridgeFig.c"
* @brief Fig data Handling.
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

#if (DIB_FIG_DATA == 1)

void DibBridgeInitDabCtx(struct DibBridgeContext * pContext)
{
   uint32_t ii;
   for(ii = 0; ii < DIB_MAX_NB_ITEMS; ii++) 
   {
      pContext->ItSvc[ii].FilterParent = DIB_UNSET;
   }  
}

/* ------------------------------------------------------------------------ */
void DibBridgeFigDone(struct DibBridgeContext *pContext, struct DibBridgeDmaCtx * pDmaCtx)
{
#if (DIB_CHECK_FIG_DATA == 2)
   static uint8_t FirstTime = 1;
   
   if(FirstTime)
   {
      DibBridgeCheckFigInit(pDmaCtx->pHostAddr, pDmaCtx->DmaLen);
      FirstTime = 0;
   }

   DibBridgeCheckFigData(pContext, pDmaCtx->pHostAddr, pDmaCtx->DmaLen);
   DibBridgeDesallocateRawBuffer(pContext, pDmaCtx->pHostAddr, pDmaCtx->DmaLen);
#endif

}


#endif
