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
* @file "DibDriverFireflyDataMpeFec.c"
* @brief Firefly Driver specific Mpe Fec Data handler
*
***************************************************************************************************/
#include "DibDriverConfig.h" /* Must be first include of all SDK files - Defines compilation options */

#if (USE_FIREFLY == 1)


#include "DibDriverCommon.h"
#include "DibDriverRegisterIf.h"
#include "DibDriverMessages.h"
#include "DibDriver.h"
#include "DibDriverFirefly.h"
#include "DibDriverFireflyMessages.h"


#if (DIB_MPEFEC_DATA == 1)

/****************************************************************************
 * Firmware send mpe fec frame information.
 ****************************************************************************/
void DibDriverFireflyProcessMsgMonitorMpeFec(struct DibDriverContext *pContext, uint16_t * Data)
{
   uint32_t total  = Data[0] | ((uint32_t) (Data[1] & 0xffff) << 16);
   uint32_t faulty = Data[2] | ((uint32_t) (Data[3] & 0xffff) << 16);
   uint32_t failed = Data[4] | ((uint32_t) (Data[5] & 0xffff) << 16);

   uint32_t beg = Data[6] | ((uint32_t) (Data[7] & 0xffff) << 16);
   uint32_t end = Data[8] | ((uint32_t) (Data[9] & 0xffff) << 16);

   uint16_t min = Data[10];
   uint16_t max = Data[11];
   uint16_t mes = Data[12];

   uint8_t  padding      = Data[13] & 0xff;
   uint8_t  puncturing   = (Data[13] >> 8) & 0xff;
   uint8_t  power_up     = Data[14] & 0xff;
   uint8_t  SvcNb        = (Data[14] >> 8) & ALL_SVC_MSK;
   int32_t  BurstTimeMes = end - beg;
   uint32_t ItemHdl      = pContext->FireflyService[SvcNb].ItemIndex;
#if (DEBUG_MODE == DEBUG_PRINT)
   uint32_t missed       = (Data[15] | ((uint32_t) (Data[16] & 0xffff) << 16));
#endif

   struct DibDriverMpeServiceCtx *pMpe;


   DIB_ASSERT(Data);
   DIB_ASSERT(SvcNb < DIB_MAX_NB_SERVICES);

   /* Item has been removed since last call*/
   if(ItemHdl == DIB_UNSET)
      return;

   DIB_ASSERT(ItemHdl < DIB_MAX_NB_ITEMS);

   pMpe = pContext->ItemInfo[ItemHdl].ItemCtx.pMpeService;

   if(BurstTimeMes < 0)
      BurstTimeMes += 1 << 16;

   DIB_DEBUG(MSG_LOG,
      (CRB "-IN_MSG_MPE_MONITOR: N_SVC=%d, NTF_DUR=%d, MEAS_DUR=%d, PWUP=%d, BOF=%d, EOF=%d, MEAS_DLTA_T=%d" CRA,
      SvcNb, (max - min) * 10, BurstTimeMes / 10, power_up,
      ((uint32_t) beg) / ((uint32_t) 10), ((uint32_t) end) / ((uint32_t) 10), mes / 10));

   DIB_DEBUG(MSG_LOG, (CRB "   PAD=%d PUNC=%d Tot/Err/Fail/Miss %d/%d/%d/%d" CRA,
      padding, puncturing, total, faulty, failed, missed));

   if(pMpe != NULL)
   {
      pMpe->DvbHMonitoring.DeltaTStream         = max * 10;
      pMpe->DvbHMonitoring.DeltaTDetected       = mes / 10;
      pMpe->DvbHMonitoring.BurstTimeStream      = (max - min) * 10;
      pMpe->DvbHMonitoring.BurstTimeDetected    = BurstTimeMes / 10;
      pMpe->DvbHMonitoring.PowerupTime          = power_up;
      pMpe->DvbHMonitoring.TotalTables          = total;
      pMpe->DvbHMonitoring.ErrorTablesBeforeFec = faulty;
      pMpe->DvbHMonitoring.ErrorTablesAfterFec  = failed;
      pMpe->DvbHMonitoring.MpePadding           = padding;
      pMpe->DvbHMonitoring.MpePuncturing        = puncturing;

      /* mpemnt - add mpe Table Size*/
      DIB_DEBUG(MPE_MNT_LOG,
            (CRB "  TS (%d): MPE (%4d)  Frames   (%4d): T/C/F/M/C/I: %d / %d / %d / %d / %d / %d " CRA,
            pContext->FireflyService[SvcNb].TsChannelNumber, pMpe->RsNumRows, pContext->FireflyService[SvcNb].ValidPid,
            total, faulty, failed, missed, pMpe->DvbHMonitoring.CcFailCnt, pMpe->DvbHMonitoring.ErrCnt));
   }
}

/****************************************************************************
*
****************************************************************************/
void DibDriverFireflyProcessMsgHbmProf(struct DibDriverContext *pContext, uint16_t * Data)
{
#if (DEBUG_MODE == DEBUG_PRINT)
   uint32_t HbmMaxTime = Data[0] | ((uint32_t) (Data[1] & 0xffff) << 16); /* max time to serve IRQ/retrieve frag */
   uint8_t  HbmMinFree = (uint8_t)((Data[2] >> 8)&0xFF) ; /* Min numbers of free HBM blocks */
   uint8_t  HbmCorrupt = (uint8_t)((Data[2])&0xFF); /* Nb of HBM blocks corrupted */
   uint8_t  SvcNb      = (uint8_t)((Data[3]  & ALL_SVC_MSK)&0xFF);

   DIB_ASSERT(Data);

   DIB_DEBUG(MPE_HBM_PROF, (CRB "  TS (%d): PID (%4d)  HBM Time / Blocks / Corrupts : %8d / %2d / %2d" CRA,
                           pContext->FireflyService[SvcNb].TsChannelNumber,
                           pContext->FireflyService[SvcNb].ValidPid,
                           HbmMaxTime,
                           HbmMinFree,
                           HbmCorrupt));
#endif
}


DIBSTATUS DibDriverFireflyAddItemMpeFec(struct DibDriverContext *pContext, FILTER_HDL FilterHdl, union DibFilters *pFilterDesc, struct DibAddPidInfo *PidInfo)
{
   PidInfo->PidTarget                                    = eTARGET_MPE;
   PidInfo->Pid                                          = pFilterDesc->MpeFec.Pid;
   PidInfo->Specific.MpeSpecific.NbRows                  = pFilterDesc->MpeFec.NbRows;
   PidInfo->Specific.MpeSpecific.MaxBurstDuration        = pFilterDesc->MpeFec.MaxBurstDuration;
   PidInfo->Specific.MpeSpecific.HasFec                  = pFilterDesc->MpeFec.NbRows != 0 ? 1 : 0;
   PidInfo->Specific.MpeSpecific.OnlyIfPrioritaryChannel = pContext->ChannelInfo[pContext->FilterInfo[FilterHdl].ParentChannel].IsDataChannel;
   PidInfo->Specific.MpeSpecific.Prefetch                = pFilterDesc->MpeFec.Prefetch;

   return DIBSTATUS_SUCCESS;
}


DIBSTATUS DibDriverFireflyAddPidMpeFec(struct DibDriverContext *pContext, struct DibDriverItem *pItem, FILTER_HDL FilterHdl, uint8_t *format, uint8_t SvcIdx, enum DibPidMode *PidMode, struct DibAddPidInfo *pPidInfo)
{
   struct   DibDriverMpeServiceCtx *p_ms;

   DIB_ASSERT((pPidInfo->PidTarget != eTARGET_MPE) || (pPidInfo->Specific.MpeSpecific.NbRows == 0)   ||
		    (pPidInfo->Specific.MpeSpecific.NbRows == 256) || (pPidInfo->Specific.MpeSpecific.NbRows == 512) ||
		    (pPidInfo->Specific.MpeSpecific.NbRows == 768) || (pPidInfo->Specific.MpeSpecific.NbRows == 1024));

   DIB_ASSERT((pPidInfo->PidTarget != eTARGET_MPE) || (pPidInfo->Specific.MpeSpecific.NbRows != 0) || (pPidInfo->Specific.MpeSpecific.HasFec == 0));

   *format = FORMAT_MPE;

   p_ms               = (struct DibDriverMpeServiceCtx *) DibMemAlloc(sizeof(struct DibDriverMpeServiceCtx));
   pItem->ItemCtx.pMpeService = p_ms;

   if(p_ms == NULL)
   {
      DIB_DEBUG(SOFT_IF_ERR, (CRB "IntDriverAddPid: no memory for MPE service" CRA));
      return DIBSTATUS_ERROR;
   }

   p_ms->RsNumRows               = pPidInfo->Specific.MpeSpecific.NbRows;
   p_ms->Pid                     = SvcIdx;
   p_ms->OnlyIfPrioritaryChannel = pPidInfo->Specific.MpeSpecific.OnlyIfPrioritaryChannel;

   if(pPidInfo->Specific.MpeSpecific.HasFec == 0)
   {
      p_ms->RsNumRows                       = 0;
      pPidInfo->Specific.MpeSpecific.NbRows = 0;
   }

   pContext->FireflyService[SvcIdx].TsChannelNumber = pPidInfo->TsChannelNumber;
   p_ms->MaxBurstDuration = pPidInfo->Specific.MpeSpecific.MaxBurstDuration;

   DibZeroMemory(&p_ms->DvbHMonitoring, sizeof(struct DvbHMonit));

   pContext->FilterInfo[FilterHdl].NbActivePids++;

   *PidMode = pPidInfo->Specific.MpeSpecific.Prefetch;

   return DIBSTATUS_SUCCESS;
}

DIBSTATUS DibDriverFireflyDelPidMpeFec(struct DibDriverContext *pContext, struct DibDriverItem *pItem, ELEM_HDL elemHdl)
{
   DIB_ASSERT(pItem->ItemCtx.pMpeService != NULL);
   DibMemFree(pItem->ItemCtx.pMpeService, sizeof(struct DibDriverMpeServiceCtx));
   pItem->ItemCtx.pMpeService = NULL;

   return DIBSTATUS_SUCCESS;
}

/**
* Runtime parameter changing. This function can do the following updates:
* - Turn on/off a PID to/from prefetch mode
* @param[in] pContext: driver context
* @param[in ] pPidInfo: Directive to set or unset prefetch for a certain PID.
*/
DIBSTATUS DibDriverFireflySetPidMode(struct DibDriverContext *pContext, struct DibPidModeCfg *pPidInfo)
{
   struct DibDriverItem *pItem;
   uint8_t               TsChannel;
   enum DibDataType      DataType;
   CHANNEL_HDL           ChannelHdl;
   uint16_t              Pid;

   DIB_ASSERT(pPidInfo);
   DIB_ASSERT((pPidInfo->Mode == ePREFETCH) || (pPidInfo->Mode == eACTIVE));
   DIB_ASSERT(pPidInfo->ElemHdl < DIB_MAX_NB_ITEMS);

   pItem = &pContext->ItemInfo[pPidInfo->ElemHdl];

   DIB_ASSERT(pItem->ParentFilter < DIB_MAX_NB_FILTERS);

   DataType  = pContext->FilterInfo[pItem->ParentFilter].TypeFilter;

   /** check this filter really exists */
   DIB_ASSERT(DataType == eMPEFEC);

   ChannelHdl = pContext->FilterInfo[pItem->ParentFilter].ParentChannel;
   DIB_ASSERT(ChannelHdl < DIB_MAX_NB_CHANNELS);

   /* Find the TS channel Number */
   for(TsChannel = 0; TsChannel < DIB_NB_PERSISTENT_TS_CHANNELS; TsChannel++)
   {
      if(pContext->FireflyChannel[TsChannel] == ChannelHdl)
         break;
   }

   DIB_ASSERT(TsChannel < DIB_NB_PERSISTENT_TS_CHANNELS);

   Pid = pItem->Config.MpeFec.Pid;

   DIB_ASSERT(pItem->ServId < DIB_MAX_NB_SERVICES);

   pItem->Config.MpeFec.Prefetch = pPidInfo->Mode;
   DIB_DEBUG(SOFT_IF_LOG, (CRB "DibDriverSetPidMode: changing service %d Mode" CRA, pItem->ServId));

   return DibDriverFireflySetCfgSvc(pContext, TsChannel, REQ_MOD_PID, Pid, pItem->ServId, 0, 0, pPidInfo->Mode);
}

#endif


#endif /* USE_FIREFLY */
