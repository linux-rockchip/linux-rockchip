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
* @file "DibDriverDragonflyDataMpeIFec.c"
* @brief Dragonfly Driver specific Mpe IFec Data handler
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

#if (DIB_MPEIFEC_DATA == 1)

/****************************************************************************
 * Firmware send mpe i-fec frame information.
 ****************************************************************************/
void DibDriverDragonflyProcessMsgMonitorMpeIFec(struct DibDriverContext *pContext, uint32_t * Data)
{
   struct MsgMpeIfecMonitor Msg;
   int32_t  BurstTimeMes;
   uint8_t  ItemHdl;
   struct DibDriverMpeIFecServiceCtx * pMpeIFec;

   MsgMpeIfecMonitorUnpack(&pContext->RxSerialBuf, &Msg);

   BurstTimeMes = Msg.BurstEndDate - Msg.BurstStartDate;
   ItemHdl = Msg.ItemHdl;

   DIB_ASSERT(Data);

#if (DEBUG_MODE == DEBUG_PRINT)

   DIB_DEBUG(MSG_LOG,(CRB "ItemIndex = %d" CRA, ItemHdl));
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

   pMpeIFec = pContext->ItemInfo[ItemHdl].ItemCtx.pMpeIFecService;

   if(BurstTimeMes < 0)
      BurstTimeMes += 1 << 16;

   /* mpemnt */
   if(pMpeIFec != NULL)
   {
/*      DIB_DEBUG(MPE_MNT_LOG,(CRB "PID (%4d) Rows (%4d) B/S/D/C/R %2d / %2d / %2d / %3d / %2d  " CRA, */
      DIB_DEBUG(MPE_MNT_LOG,(CRB "PID (%4d) Rows (%4d) Burst (%d) Table (%d)" CRA, pMpeIFec->Pid, pMpeIFec->RsNumRows, Msg.BurstIndex, Msg.TableIndex));

      pMpeIFec->DvbSHMonitoring.AdtTotalTables     = Msg.NbADTTables;
      pMpeIFec->DvbSHMonitoring.AdtTablesCorrected = Msg.NbADTTablesCorrected;
      pMpeIFec->DvbSHMonitoring.AdtTablesFailed    = Msg.NbADTTablesFailed;
      pMpeIFec->DvbSHMonitoring.AdtTablesGood      = Msg.NbADTTablesGood;

      DIB_DEBUG(MPE_MNT_LOG, (CRB " ADT INFO : T/G/C/F: %d / %d / %d / %d " CRA, Msg.NbADTTables, Msg.NbADTTablesGood, Msg.NbADTTablesCorrected, Msg.NbADTTablesFailed));
      
      /** ADST (ie. burst) monitoring */
      pMpeIFec->DvbSHMonitoring.AdstTotalTables    = Msg.NbADSTTables         ;
      pMpeIFec->DvbSHMonitoring.AdstTablesLost     = Msg.NbADSTTablesLost     ;
      pMpeIFec->DvbSHMonitoring.AdstTablesCorrected= Msg.NbADSTTablesCorrected;
      pMpeIFec->DvbSHMonitoring.AdstTablesFailed   = Msg.NbADSTTablesFailed   ;
      pMpeIFec->DvbSHMonitoring.AdstTablesGood     = Msg.NbADSTTablesGood     ;

      pMpeIFec->DvbSHMonitoring.DeltaTStream       = Msg.DeltatMax * 10;
      pMpeIFec->DvbSHMonitoring.DeltaTDetected     = Msg.FrameDuration / 10;
      pMpeIFec->DvbSHMonitoring.BurstStream        = (Msg.DeltatMax - Msg.DeltatMin) * 10;
      pMpeIFec->DvbSHMonitoring.BurstDetected      = BurstTimeMes / 10;
      pMpeIFec->DvbSHMonitoring.PowerUpTime        = Msg.PowerUpDuration;

      DIB_DEBUG(MPE_MNT_LOG, (CRB " ADST INFO : T/G/C/F/L: %d / %d / %d / %d / %d " CRA, Msg.NbADSTTables, Msg.NbADSTTablesGood, Msg.NbADSTTablesCorrected, Msg.NbADSTTablesFailed, Msg.NbADSTTablesLost));
   }
}

DIBSTATUS DibDriverDragonflyAddItemMpeIFec(struct DibDriverContext *pContext, union DibFilters *pFilterDesc, FILTER_HDL FilterHdl, uint32_t ItemHdl, struct MsgCreateItem * msg)
{
   struct DibDriverMpeIFecServiceCtx *pMpeIFecService;
   struct DibDriverItem              *pItem;

   msg->Param.Mpeifec.Pid                 = pFilterDesc->MpeIFec.Pid;
   msg->Param.Mpeifec.MaxBurstDuration    = pFilterDesc->MpeIFec.MaxBurstDuration;
   msg->Param.Mpeifec.Prefetch            = pFilterDesc->MpeIFec.Prefetch;
   msg->Param.Mpeifec.B                   = pFilterDesc->MpeIFec.EncodParal;
   msg->Param.Mpeifec.C                   = pFilterDesc->MpeIFec.NbAdstColumns;
   msg->Param.Mpeifec.S                   = pFilterDesc->MpeIFec.SpreadingFactor;
   msg->Param.Mpeifec.R                   = pFilterDesc->MpeIFec.MaxIFecSect;
   msg->Param.Mpeifec.D                   = pFilterDesc->MpeIFec.SendingDelay;

   pMpeIFecService = (struct DibDriverMpeIFecServiceCtx *) DibMemAlloc(sizeof(struct DibDriverMpeIFecServiceCtx));
   pItem = &pContext->ItemInfo[ItemHdl];
   pItem->ItemCtx.pMpeIFecService = pMpeIFecService;

   if(pMpeIFecService == NULL)
   {
      DIB_DEBUG(SOFT_IF_ERR, (CRB "IntDriverAddPid: no memory for MPE-IFEC service" CRA));
      return DIBSTATUS_ERROR;
   }

   pMpeIFecService->RsNumRows               = pFilterDesc->MpeIFec.NbRows;
   pMpeIFecService->Pid                     = pFilterDesc->MpeIFec.Pid;
   pMpeIFecService->OnlyIfPrioritaryChannel = pContext->ChannelInfo[pContext->FilterInfo[FilterHdl].ParentChannel].IsDataChannel;
   pMpeIFecService->MaxBurstDuration        = pFilterDesc->MpeIFec.MaxBurstDuration;

   /* TBD_IF */

   DibZeroMemory(&pMpeIFecService->DvbSHMonitoring, sizeof(struct DvbSHMonit));

   pContext->FilterInfo[FilterHdl].NbActivePids++;

   return DIBSTATUS_SUCCESS;
}

DIBSTATUS DibDriverDragonflyRemoveItemMpeIFec(struct DibDriverContext *pContext, struct DibDriverItem *pItem)
{
   if(pItem->ItemCtx.pMpeIFecService)
      DibMemFree(pItem->ItemCtx.pMpeIFecService, sizeof(struct DibDriverMpeIFecServiceCtx));

   pItem->ItemCtx.pMpeIFecService = NULL;

   return DIBSTATUS_SUCCESS;
}
#endif


#endif /* USE_DRAGONFLY */
