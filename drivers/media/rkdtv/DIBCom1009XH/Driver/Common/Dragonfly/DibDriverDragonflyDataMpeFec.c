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
* @file "DibDriverDragonflyDataMpeFec.c"
* @brief Dragonfly Driver specific Mpe Fec Data handler
*
***************************************************************************************************/
#include "DibDriverConfig.h" /* Must be first include of all SDK files - Defines compilation options */

#if (USE_DRAGONFLY == 1)


#include "DibDriverCommon.h"
#include "DibDriverRegisterIf.h"
#include "DibDriverMessages.h"
#include "DibDriver.h"

#include "DibDriverDragonflyRegisters.h"   /* host-dragonfly register interface. Found in bridge/common        */
#include "DibDriverDragonflyMessages.h"    /* specific dragonlfy messages formatting. Found in bridge/common   */
#include "DibDriverDragonfly.h"

#if (DIB_MPEFEC_DATA == 1)

/****************************************************************************
 * Firmware send mpe fec frame information.
 ****************************************************************************/
void DibDriverDragonflyProcessMsgMonitorMpeFec(struct DibDriverContext * pContext, uint32_t * Data)
{
   struct MsgMpeMonitor Msg;
   struct DibDriverMpeServiceCtx * pMpe;
   int32_t  BurstTimeMes;
   uint8_t  ItemHdl;

   DIB_ASSERT(Data);

   MsgMpeMonitorUnpack(&pContext->RxSerialBuf, &Msg);
   BurstTimeMes = Msg.BurstEndDate - Msg.BurstStartDate;
   ItemHdl = Msg.ItemHdl;

#if (DEBUG_MODE == DEBUG_PRINT)

   DIB_DEBUG(MSG_LOG,(CRB "MpeMonitor message :" CRA));
   DIB_DEBUG(MSG_LOG,(CRB "NbRxBurst       = %d" CRA, Msg.NbRxBurst));
   DIB_DEBUG(MSG_LOG,(CRB "Faulty          = %d" CRA, Msg.NbErrTableBeforeFec));
   DIB_DEBUG(MSG_LOG,(CRB "Failed          = %d" CRA, Msg.NbErrTableAfterFec));
   DIB_DEBUG(MSG_LOG,(CRB "NbErrBeforeFec  = %d" CRA, Msg.NbErrBeforeFec));
   DIB_DEBUG(MSG_LOG,(CRB "NbErrAfterFec   = %d" CRA, Msg.NbErrAfterFec));
   DIB_DEBUG(MSG_LOG,(CRB "BurstStartDate  = %d" CRA, Msg.BurstStartDate));
   DIB_DEBUG(MSG_LOG,(CRB "BurstEndDate    = %d" CRA, Msg.BurstEndDate));
   DIB_DEBUG(MSG_LOG,(CRB "Delta_t_min     = %d" CRA, Msg.DeltatMin));
   DIB_DEBUG(MSG_LOG,(CRB "Delta_t_max     = %d" CRA, Msg.DeltatMax));
   DIB_DEBUG(MSG_LOG,(CRB "Pad=%d, Punc    = %d" CRA, Msg.Padding, Msg.Puncturing));
   DIB_DEBUG(MSG_LOG,(CRB "ItemIndex       = %d, FilterIndex = %d" CRA, ItemHdl, Msg.FilterId));

   switch(Msg.FrameStatus)
   {
   case 0:
      DIB_DEBUG(MSG_LOG,(CRB "FrameStatus     = DEF_FRAME_EMPTY" CRA));   break;
   case 1:
      DIB_DEBUG(MSG_LOG,(CRB "FrameStatus     = DEF_FRAME_INCOMPLETE" CRA));   break;
   case 2:
      DIB_DEBUG(MSG_LOG,(CRB "FrameStatus     = DEF_FRAME_FAILED" CRA));   break;
   case 3:
      DIB_DEBUG(MSG_LOG,(CRB "FrameStatus     = DEF_FRAME_COMPLETE" CRA));   break;
   case 4:
      DIB_DEBUG(MSG_LOG,(CRB "FrameStatus     = DEF_FRAME_FAULTY" CRA));   break;
   case 5:
      DIB_DEBUG(MSG_LOG,(CRB "FrameStatus     = DEF_FRAME_RECOVERED" CRA));   break;
   case 6:
      DIB_DEBUG(MSG_LOG,(CRB "FrameStatus     = DEF_FRAME_MISSED" CRA));   break;
   default:
      break;
   }
#endif

   /* Item has been removed since last call*/
   if(ItemHdl == DIB_UNSET)
      return;

   DIB_ASSERT(ItemHdl < DIB_MAX_NB_ITEMS);

   pMpe = pContext->ItemInfo[ItemHdl].ItemCtx.pMpeService;

   if(BurstTimeMes < 0)
      BurstTimeMes += 1 << 16;

   if(pMpe != NULL)
   {
      pMpe->DvbHMonitoring.DeltaTStream         = Msg.DeltatMax * 10;
      pMpe->DvbHMonitoring.DeltaTDetected       = Msg.FrameDuration / 10;
      pMpe->DvbHMonitoring.BurstTimeStream      = (Msg.DeltatMax - Msg.DeltatMin) * 10;
      pMpe->DvbHMonitoring.BurstTimeDetected    = BurstTimeMes / 10;
      pMpe->DvbHMonitoring.PowerupTime          = Msg.PowerUpDuration;
      pMpe->DvbHMonitoring.TotalTables          = Msg.NbRxBurst;
      pMpe->DvbHMonitoring.ErrorTablesBeforeFec = Msg.NbErrTableBeforeFec;
      pMpe->DvbHMonitoring.ErrorTablesAfterFec  = Msg.NbErrTableAfterFec;
      pMpe->DvbHMonitoring.MpePadding           = Msg.Padding;
      pMpe->DvbHMonitoring.MpePuncturing        = Msg.Puncturing;

      /* Todo : add mpe Table Missed */
      DIB_DEBUG(MPE_MNT_LOG,
            (CRB "  TS (%d): MPE (%4d)  Frames   (%4d): T/C/F/M/C/I: %d / %d / %d / %d / %d / %d " CRA,
            pContext->FilterInfo[pContext->ItemInfo[ItemHdl].ParentFilter].ParentChannel, pMpe->RsNumRows, pMpe->Pid,
            Msg.NbRxBurst, Msg.NbErrTableBeforeFec, Msg.NbErrTableAfterFec, Msg.NbTableMissed, pMpe->DvbHMonitoring.CcFailCnt, pMpe->DvbHMonitoring.ErrCnt));
   }
}

DIBSTATUS DibDriverDragonflyAddItemMpeFec(struct DibDriverContext *pContext, union DibFilters *pFilterDesc, FILTER_HDL FilterHdl, uint32_t ItemHdl, struct MsgCreateItem * msg)
{
   struct DibDriverMpeServiceCtx *pMpeService;     /* Points to MPE service context if target MPE. MPE_SERVICE_CONTEXT*/
   struct DibDriverItem          *pItem;

   msg->Param.Mpefec.Pid = pFilterDesc->MpeFec.Pid;
   msg->Param.Mpefec.NbRows = pFilterDesc->MpeFec.NbRows;
   msg->Param.Mpefec.MaxBurstDuration = pFilterDesc->MpeFec.MaxBurstDuration;
   msg->Param.Mpefec.Prefetch = pFilterDesc->MpeFec.Prefetch;

   pMpeService = (struct DibDriverMpeServiceCtx *)DibMemAlloc(sizeof(struct DibDriverMpeServiceCtx));
   pItem = &pContext->ItemInfo[ItemHdl];
   pItem->ItemCtx.pMpeService = pMpeService;

   if(pMpeService == NULL)
   {
      DIB_DEBUG(SOFT_IF_ERR, (CRB "IntDriverAddPid: no memory for MPE service" CRA));
      return DIBSTATUS_ERROR;
   }

   pMpeService->RsNumRows               = pFilterDesc->MpeFec.NbRows;
   pMpeService->Pid                     = pFilterDesc->MpeFec.Pid;
   pMpeService->OnlyIfPrioritaryChannel = pContext->ChannelInfo[pContext->FilterInfo[FilterHdl].ParentChannel].IsDataChannel;
   pMpeService->MaxBurstDuration        = pFilterDesc->MpeFec.MaxBurstDuration;

   DibZeroMemory(&pMpeService->DvbHMonitoring, sizeof(struct DvbHMonit));

   pContext->FilterInfo[FilterHdl].NbActivePids++;

   return DIBSTATUS_SUCCESS;
}

DIBSTATUS DibDriverDragonflyRemoveItemMpeFec(struct DibDriverContext *pContext, struct DibDriverItem *pItem)
{
   if(pItem->ItemCtx.pMpeService)
      DibMemFree(pItem->ItemCtx.pMpeService, sizeof(struct DibDriverMpeServiceCtx));

   pItem->ItemCtx.pMpeService = NULL;

   return DIBSTATUS_SUCCESS;
}
#endif


#endif /* USE_DRAGONFLY */
