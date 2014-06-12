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

#if (DIB_FIG_DATA == 1)

DIBSTATUS DibDriverDragonflyAddItemFig(struct DibDriverContext * pContext, union DibFilters * pFilterDesc, FILTER_HDL FilterHdl, uint32_t ItemHdl, struct MsgCreateItem *msg)
{
   struct DibDriverFigServiceCtx * pFigService;
   struct DibDriverItem * pItem;

   pFigService = (struct DibDriverFigServiceCtx *)DibMemAlloc(sizeof(struct DibDriverFigServiceCtx));

   if(pFigService == NULL)
   {
      DIB_DEBUG(SOFT_IF_ERR, (CRB "IntDriverAddPid: no memory for FIC service" CRA));
      return DIBSTATUS_ERROR;
   }

   DibZeroMemory(pFigService, sizeof(struct DibDriverFigServiceCtx));
   pItem = &pContext->ItemInfo[ItemHdl];
   pItem->ItemCtx.pFigService = pFigService;

   pContext->FilterInfo[FilterHdl].NbActivePids++;

   return DIBSTATUS_SUCCESS;
}

DIBSTATUS DibDriverDragonflyRemoveItemFig(struct DibDriverContext * pContext, struct DibDriverItem * pItem)
{
   if(pItem->ItemCtx.pFigService)
      DibMemFree(pItem->ItemCtx.pFigService, sizeof(struct DibDriverFigServiceCtx));

   pItem->ItemCtx.pFigService = NULL;

   return DIBSTATUS_SUCCESS;
}

void DibDriverDragonflyProcessMsgMonitorFic(struct DibDriverContext * pContext, uint32_t * Data)
{
   struct MsgFicMonitor Msg;
   ELEM_HDL             ItemHdl;
   FILTER_HDL           FilterHdl;

   MsgFicMonitorUnpack(&pContext->RxSerialBuf, &Msg);

   ItemHdl  = Msg.ItemHdl;
   FilterHdl = pContext->ItemInfo[ItemHdl].ParentFilter;

   /* warning: we can receive that message after filter deletion */
   if(pContext->FilterInfo[FilterHdl].TypeFilter == eFIG)
   {
      pContext->ItemInfo[ItemHdl].ItemCtx.pFigService->FicMonitoring.CorrectCnt   = Msg.AvalaiblePackets;
      pContext->ItemInfo[ItemHdl].ItemCtx.pFigService->FicMonitoring.ErrCnt       = Msg.ErrorPackets;
      DIB_DEBUG(MSG_LOG,(CRB "MSG_FIC_MONITOR: FilterHdl %d received %d avalaible packets and %d failed packet" CRA, FilterHdl, Msg.AvalaiblePackets, Msg.ErrorPackets));
   }
}
#endif

#if (DIB_TDMB_DATA == 1)

DIBSTATUS DibDriverDragonflyAddItemTdmb(struct DibDriverContext * pContext, union DibFilters * pFilterDesc, FILTER_HDL FilterHdl, uint32_t ItemHdl, struct MsgCreateItem *msg)
{
   struct DibDriverTdmbServiceCtx * pTdmbService;
   struct DibDriverItem * pItem;
   uint16_t Pid = 8192;

   msg->Param.Tdmb.Pid     = Pid;
   msg->Param.Tdmb.SubCh   = pFilterDesc->Tdmb.SubCh;

   pTdmbService = (struct DibDriverTdmbServiceCtx *)DibMemAlloc(sizeof(struct DibDriverTdmbServiceCtx));
   pItem = &pContext->ItemInfo[ItemHdl];
   pItem->ItemCtx.pTdmbService = pTdmbService;

   if(pTdmbService == NULL)
   {
      DIB_DEBUG(SOFT_IF_ERR, (CRB "IntDriverAddPid: no memory for TDMB service" CRA));
      return DIBSTATUS_ERROR;
   }

   pTdmbService->SubId = pFilterDesc->Tdmb.SubCh;

   /*pContext->FilterInfo[FilterHdl].NbActivePids++;*/

   return DIBSTATUS_SUCCESS;
}

DIBSTATUS DibDriverDragonflyRemoveItemTdmb(struct DibDriverContext * pContext, struct DibDriverItem * pItem)
{
   if(pItem->ItemCtx.pTdmbService)
      DibMemFree(pItem->ItemCtx.pTdmbService, sizeof(struct DibDriverTdmbServiceCtx));

   pItem->ItemCtx.pTdmbService = NULL;

   return DIBSTATUS_SUCCESS;
}
#endif

#if (DIB_DAB_DATA == 1)

DIBSTATUS DibDriverDragonflyAddItemDabAudio(struct DibDriverContext * pContext, union DibFilters * pFilterDesc, FILTER_HDL FilterHdl, uint32_t ItemHdl, struct MsgCreateItem *msg)
{
   struct DibDriverDabAudioServiceCtx * pDabAudioService;
   struct DibDriverItem * pItem;

   msg->Param.Dab.SubCh    = pFilterDesc->Dab.SubCh;
   msg->Param.Dab.Type     = pFilterDesc->Dab.Type;
   msg->Param.Dab.Fec      = pFilterDesc->Dab.UseFec;

   pDabAudioService = (struct DibDriverDabAudioServiceCtx *)DibMemAlloc(sizeof(struct DibDriverDabAudioServiceCtx));

   if(pDabAudioService == NULL)
   {
      DIB_DEBUG(SOFT_IF_ERR, (CRB "IntDriverAddPid: no memory for DAB audio service" CRA));
      return DIBSTATUS_ERROR;
   }

   DibZeroMemory(pDabAudioService, sizeof(struct DibDriverDabAudioServiceCtx));

   pItem = &pContext->ItemInfo[ItemHdl];
   pItem->ItemCtx.pDabAudioService = pDabAudioService;

   pDabAudioService->SubId = pFilterDesc->Dab.SubCh;

   pContext->FilterInfo[FilterHdl].NbActivePids++;

   return DIBSTATUS_SUCCESS;
}

DIBSTATUS DibDriverDragonflyRemoveItemDabAudio(struct DibDriverContext * pContext, struct DibDriverItem * pItem)
{
   if(pItem->ItemCtx.pDabAudioService)
      DibMemFree(pItem->ItemCtx.pDabAudioService, sizeof(struct DibDriverDabAudioServiceCtx));

   pItem->ItemCtx.pDabAudioService = NULL;

   return DIBSTATUS_SUCCESS;
   
}

void DibDriverDragonflyProcessMsgMonitorDabPlus(struct DibDriverContext * pContext, uint32_t * Data)
{
   struct MsgMscPlusMonitor               Msg;
   struct DibDriverDabAudioServiceCtx *   pDabPlus;
   ELEM_HDL                               ItemHdl;
   FILTER_HDL                             FilterId;

   MsgMscPlusMonitorUnpack(&pContext->RxSerialBuf, &Msg);
   ItemHdl = Msg.ItemHdl;
   FilterId = pContext->ItemInfo[ItemHdl].ParentFilter;

   /* warning: we can receive that message after filter deletion */
   if(pContext->FilterInfo[FilterId].TypeFilter == eDAB)
   {
      pDabPlus = pContext->ItemInfo[ItemHdl].ItemCtx.pDabAudioService;
      pDabPlus->DabMonitoring.TotalPackets      = Msg.AvalaibleSuperFrame;
      pDabPlus->DabMonitoring.Error             = Msg.FailedsuperFrame;
      pDabPlus->DabMonitoring.CorrectedPackets  = Msg.CorrectedSuperFrame;
      pDabPlus->DabMonitoring.CountinuityError  = 0;
      DIB_DEBUG(MSG_LOG, (CRB "MSG_MSCPLUS_MONITOR: FilterId %d : G / C / F / S     %u / %u / %u / %u" CRA, FilterId,
            Msg.AvalaibleSuperFrame, Msg.CorrectedSuperFrame, Msg.FailedsuperFrame, Msg.NbFrameWoSynchro));
   }
}
#endif

#if (DIB_DABPACKET_DATA == 1)

DIBSTATUS DibDriverDragonflyAddItemDabPacket(struct DibDriverContext * pContext, union DibFilters * pFilterDesc, FILTER_HDL FilterHdl, uint32_t ItemHdl, struct MsgCreateItem * msg)
{
   struct DibDriverDabPacketServiceCtx * pDabPacketService;
   struct DibDriverItem * pItem;

   msg->Param.DabPacket.SubCh          = pFilterDesc->DabPacket.SubCh;
   msg->Param.DabPacket.Address        = pFilterDesc->DabPacket.Address;
   msg->Param.DabPacket.Fec            = pFilterDesc->DabPacket.UseFec;
   msg->Param.DabPacket.DataGroup      = pFilterDesc->DabPacket.UseDataGroup;

   pDabPacketService = (struct DibDriverDabPacketServiceCtx *)DibMemAlloc(sizeof(struct DibDriverDabPacketServiceCtx));

   if(pDabPacketService == NULL)
   {
      DIB_DEBUG(SOFT_IF_ERR, (CRB "IntDriverAddPid: no memory for Dab Packet service" CRA));
      return DIBSTATUS_ERROR;
   }

   DibZeroMemory(pDabPacketService, sizeof(struct DibDriverDabPacketServiceCtx));

   pItem = &pContext->ItemInfo[ItemHdl];
   pItem->ItemCtx.pDabPacketService = pDabPacketService;

   pDabPacketService->SubId = pFilterDesc->DabPacket.SubCh;

   pContext->FilterInfo[FilterHdl].NbActivePids++;

   return DIBSTATUS_SUCCESS;
}

DIBSTATUS DibDriverDragonflyRemoveItemDabPacket(struct DibDriverContext * pContext, struct DibDriverItem * pItem)
{
   if(pItem->ItemCtx.pDabPacketService)
      DibMemFree(pItem->ItemCtx.pDabPacketService, sizeof(struct DibDriverDabPacketServiceCtx));

   pItem->ItemCtx.pDabPacketService = NULL;

   return DIBSTATUS_SUCCESS;
   
}

void DibDriverDragonflyProcessMsgMonitorMscPacket(struct DibDriverContext *pContext, uint32_t * Data)
{
   struct MsgMscPacketMonitor Msg;
   struct DibDriverDabPacketServiceCtx * pDabPack;
   ELEM_HDL                   ItemHdl;
   FILTER_HDL                 FilterId;

   MsgMscPacketMonitorUnpack(&pContext->RxSerialBuf, &Msg);

   ItemHdl     = Msg.ItemHdl;
   FilterId    = pContext->ItemInfo[ItemHdl].ParentFilter;

   /* warning: we can receive that message after filter deletion */
   if(pContext->FilterInfo[FilterId].TypeFilter == eDABPACKET)
   {
      pDabPack = pContext->ItemInfo[ItemHdl].ItemCtx.pDabPacketService;
      pDabPack->DabPacketMonitoring.MscPacketGood    = Msg.MscPacketsAvail;
      pDabPack->DabPacketMonitoring.MscPacketsError  = Msg.MscPacketsError;
      pDabPack->DabPacketMonitoring.DataGroupGood    = Msg.DataGroupAvail;
      pDabPack->DabPacketMonitoring.DataGroupError   = Msg.DataGroupError;
      DIB_DEBUG(MSG_LOG,(CRB "MSG_MSCPACKET_MONITOR: FilterId %d" CRA, FilterId));
   }
}
#endif


#endif /* USE_DRAGONFLY */
