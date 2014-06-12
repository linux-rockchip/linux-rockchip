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
* @file "DibDriverDragonflyDataCmmbSvc.c"
* @brief Dragonfly Driver specific Cmmb Service handler.
*******************************************************************************/
#include "DibDriverConfig.h" /* Must be first include of all SDK files - Defines compilation options */

#if (USE_DRAGONFLY == 1)


#include "DibDriverCommon.h"
#include "DibDriverRegisterIf.h"
#include "DibDriverMessages.h"
#include "DibDriver.h"

#include "DibDriverDragonflyRegisters.h"   /* host-dragonfly register interface. Found in bridge/common        */
#include "DibDriverDragonflyMessages.h"    /* specific dragonlfy messages formatting. Found in bridge/common   */
#include "DibDriverDragonfly.h"

#if (DIB_CMMB_DATA == 1)

/**
 * DibDriverDragonflyProcessMsgMonitorCmmbMf
 */
void DibDriverDragonflyProcessMsgMonitorCmmbMf(struct DibDriverContext *pContext, uint32_t *Data)
{
   struct MsgCmmbMfMonitor MsgIn;
   struct DibDriverCmmbServiceCtx *pCmmb;

   MsgCmmbMfMonitorUnpack(&pContext->RxSerialBuf, &MsgIn);

   /* Case item was removed. */
   if(MsgIn.ItemId == DIB_UNSET)
      return;

   DIB_ASSERT(MsgIn.ItemId < DIB_MAX_NB_ITEMS);

   /* CMMB service context. */
   pCmmb = pContext->ItemInfo[MsgIn.ItemId].ItemCtx.pCmmbService;
   if(pCmmb != NULL)
   {
      /* Multiplex frame monitoring. */
      pCmmb->CmmbMonitoring.TotalMf          = MsgIn.TotalMf;
      pCmmb->CmmbMonitoring.ErrorMfBeforeRs  = MsgIn.ErrorMfBeforeRs;
      pCmmb->CmmbMonitoring.ErrorMfAfterRs   = MsgIn.ErrorMfAfterRs;
   }
}

/**
 * DibDriverDragonflyProcessMsgMonitorCmmbRsm
 */
void DibDriverDragonflyProcessMsgMonitorCmmbRsm(struct DibDriverContext *pContext, uint32_t *Data)
{
   struct DibDriverCmmbServiceCtx *pCmmb;
   struct MsgCmmbRsmMonitor MsgIn;

    /* Item handler. */
   DIB_ASSERT(Data);

   MsgCmmbRsmMonitorUnpack(&pContext->RxSerialBuf, &MsgIn);

   /* Case item was removed. */
   if(MsgIn.ItemId == DIB_UNSET)
      return;

   DIB_ASSERT(MsgIn.ItemId < DIB_MAX_NB_ITEMS);

   /* CMMB service context. */
   pCmmb = pContext->ItemInfo[MsgIn.ItemId].ItemCtx.pCmmbService;
   if(pCmmb != NULL)
   {
      /* LDPC blocks. */
      pCmmb->CmmbMonitoring.TotalLdpcBlocks         = MsgIn.TotalLdpcBlocks;
      pCmmb->CmmbMonitoring.ErrorLdpcBlocks         = MsgIn.ErrorLdpcBlocks;

      /* RS matrices. */
      pCmmb->CmmbMonitoring.TotalRsMatrices         = MsgIn.TotalRsMatrices;
      pCmmb->CmmbMonitoring.ErrorRsMatricesBeforeRs = MsgIn.ErrorRsMatricesBeforeRs;
      pCmmb->CmmbMonitoring.ErrorRsMatricesAfterRs  = MsgIn.ErrorRsMatricesAfterRs;
   }
}

/**
 * DibDriverDragonflyAddItemCmmbSvc
 */
DIBSTATUS DibDriverDragonflyAddItemCmmbSvc(struct DibDriverContext *pContext,
                                           union DibFilters *pFilterDesc,
                                           FILTER_HDL FilterHdl,
                                           uint32_t ItemHdl,
                                           struct MsgCreateItem * msg)
{
   struct DibDriverCmmbServiceCtx *pCmmbService;
   struct DibDriverItem *pItem;
   uint8_t *pMultiplexFrame = 0;

   if (pContext->NbCmmbSvc >= CMMB_SVC_NB_MAX)
   {
      DIB_DEBUG(SOFT_IF_ERR, (CRB "DibDriverDragonflyAddItemCmmbSvc: Too many Services required" CRA));
      return DIBSTATUS_RESOURCES;
   }

   /* Set message to firmware content. */
   msg->Param.CmmbSvc.Mode  = pFilterDesc->CmmbSvc.Mode;
   msg->Param.CmmbSvc.MfId  = pFilterDesc->CmmbSvc.MfId;
   msg->Param.CmmbSvc.SvcId = pFilterDesc->CmmbSvc.SvcId;

   /* Memory allocation for Cmmb service context. */
   pCmmbService = (struct DibDriverCmmbServiceCtx*)DibMemAlloc(sizeof(struct DibDriverCmmbServiceCtx));
   pItem = &pContext->ItemInfo[ItemHdl];
   pItem->ItemCtx.pCmmbService = pCmmbService;
   if(pCmmbService == NULL)
   {
      DIB_DEBUG(SOFT_IF_ERR, (CRB "DibDriverDragonflyAddItemCmmbSvc: no memory for CMMB service" CRA));
      return DIBSTATUS_ERROR;
   }
   DibZeroMemory(&pItem->ItemCtx.pCmmbService->CmmbMonitoring, sizeof(struct CmmbMonit));
   pItem->ItemCtx.pCmmbService->Size = 0;

   /* Memory allocation for Cmmb service data (reconstructed multiplex frames). */
   pMultiplexFrame = (uint8_t*)DibMemAlloc(CMMB_MAX_MF_LENGTH);
   if(pMultiplexFrame == NULL)
   {
      DIB_DEBUG(SOFT_IF_ERR, (CRB "DibDriverDragonflyAddItemCmmbSvc: no memory for CMMB multiplex frame buffer" CRA));
      return DIBSTATUS_RESOURCES;
   }
   pItem->ItemCtx.pCmmbService->MultiplexFrame = pMultiplexFrame;
   DibZeroMemory(pItem->ItemCtx.pCmmbService->MultiplexFrame, CMMB_MAX_MF_LENGTH);
   pContext->NbCmmbSvc++;

   return DIBSTATUS_SUCCESS;
}

/**
 * DibDriverDragonflyRemoveItemCmmbSvc
 */
DIBSTATUS DibDriverDragonflyRemoveItemCmmbSvc(struct DibDriverContext *pContext, struct DibDriverItem *pItem)
{
   /* Display for debug. */
   DIB_DEBUG(MSG_LOG, (CRB "DibDriverDragonflyRemoveItemCmmbSvc (ParentFilter = %d): Mode = %d, MfId = %d, SvcId = %d" CRA,
             pItem->ParentFilter,
             pItem->Config.CmmbSvc.Mode, pItem->Config.CmmbSvc.MfId, pItem->Config.CmmbSvc.SvcId));

   /* Free memory for Cmmb service data (reconstructed multiplex frame). */
   DIB_ASSERT(pItem->ItemCtx.pCmmbService->MultiplexFrame != NULL);
   DibMemFree(pItem->ItemCtx.pCmmbService->MultiplexFrame, CMMB_MAX_MF_LENGTH);
   pItem->ItemCtx.pCmmbService->MultiplexFrame = NULL;

   /* Free Cmmb service context. */
   DIB_ASSERT(pItem->ItemCtx.pCmmbService != NULL);
   DibMemFree(pItem->ItemCtx.pCmmbService, sizeof(struct DibDriverCmmbServiceCtx));
   pItem->ItemCtx.pCmmbService = NULL;
   pContext->NbCmmbSvc--;

   return DIBSTATUS_SUCCESS;
}

#endif /* DIB_CMMB_DATA */


#endif /* USE_DRAGONFLY */
