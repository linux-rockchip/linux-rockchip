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
* @file "DibDriverDataRawTs.c"
* @brief Driver RawTs data handling.
*
***************************************************************************************************/
#include "DibDriverConfig.h" /* Must be first include of all SDK files - Defines compilation options */
#include "DibDriverCommon.h"
#include "DibDriver.h"
#include "DibDriverMessages.h"

#if (DIB_DAB_DATA == 1)
DIBSTATUS DibDriverGetMonitoringDabAudio(struct DibDriverContext * pContext, ELEM_HDL ItemHdl, union DibDataMonit * pDataMonit, uint8_t ClearMonit)
{
   struct DibDabMonitoring             * pDabMonit = (struct DibDabMonitoring *) pDataMonit;
   struct DibDriverDabAudioServiceCtx  * pService;   
   FILTER_HDL                          FilterHdl  = pContext->ItemInfo[ItemHdl].ParentFilter;

   DIB_ASSERT(pDabMonit);
   DIB_ASSERT(ItemHdl < DIB_MAX_NB_ITEMS);
   DIB_ASSERT(pContext->FilterInfo[FilterHdl].TypeFilter == eDAB);
   DIB_ASSERT(((ClearMonit == eDIB_TRUE) || (ClearMonit == eDIB_FALSE)));
   
   DIB_DEBUG(SOFT_IF_LOG, (CRB "DibDriverGetMonitoringDabAudio" CRA));

#if (LOG_APPLICATION_TO_DRIVER_CALLS_TO_FILE == 1)
   {
      DibLogToFile_Time(gLogFile, gLogFileStartTimeSec, gLogFileStartTimeMsec);
      DibLogToFile_Mesg(gLogFile, CRB "DibDriverGetMonitoringDabAudio" CRA);
   }
#endif

   DibDriverTargetDisableIrqProcessing(pContext);

   pService = pContext->ItemInfo[ItemHdl].ItemCtx.pDabAudioService;

   if(pDabMonit)
   {
      /*** Initialize DAB Monitoring structure ***/
      DibZeroMemory(pDabMonit, sizeof(struct DibDabMonitoring));

      pDabMonit->NbPid                    = 0;
      pDabMonit->ChannelIndex             = pContext->FilterInfo[pContext->ItemInfo[ItemHdl].ParentFilter].ParentChannel;
      pDabMonit->FilterIndex              = FilterHdl;
      pDabMonit->FilterType               = pContext->FilterInfo[FilterHdl].TypeFilter;
      if(pService)
      {
         pDabMonit->CountinuityError         = pService->DabMonitoring.CountinuityError;
         pDabMonit->Error                    = pService->DabMonitoring.Error;
         pDabMonit->TotalPackets             = pService->DabMonitoring.TotalPackets;
         pDabMonit->CorrPackets              = pService->DabMonitoring.CorrectedPackets;
         /* Protected access to shared ressource, clear error and cc counts */
         if(ClearMonit == eDIB_TRUE)
            DibZeroMemory(&pService->DabMonitoring, sizeof(pService->DabMonitoring));
      }    
   }
   DibDriverTargetEnableIrqProcessing(pContext);

   /* Send Message to clear internal monitoring info is sent separatly to avoid dead locks */
   if((pService) && (ClearMonit == eDIB_TRUE))
      DibDriverClearMonit(pContext, ItemHdl);

   return DIBSTATUS_SUCCESS;   
}

DIBSTATUS DibDriverCheckParamDabAudio(struct DibDriverContext * pContext, union DibFilters * pFilterDesc)
{
   DIBSTATUS Status = DIBSTATUS_INVALID_PARAMETER;

   if(pFilterDesc->Dab.SubCh >= DIB_DAB_MAX_NB_CHANNELS)
      goto End;
   if(pFilterDesc->Dab.Type >= DIB_MSC_MAX_NB_TYPE)
      goto End;
   if((pFilterDesc->Dab.UseFec != 1) && (pFilterDesc->Dab.UseFec != 0))
      goto End;
   if((pFilterDesc->Dab.Type == DIB_MSC_STREAM_AUDIO) && (pFilterDesc->Dab.UseFec != 0))
      goto End;
   
    Status = DIBSTATUS_SUCCESS;

End:
   return Status;   
}
#endif

#if (DIB_DABPACKET_DATA == 1)
DIBSTATUS DibDriverGetMonitoringDabPacket(struct DibDriverContext * pContext, ELEM_HDL ItemHdl, union DibDataMonit * pDataMonit, uint8_t ClearMonit)
{
   struct DibDabPacketMonitoring * pMonit = (struct DibDabPacketMonitoring *) pDataMonit;
   FILTER_HDL                FilterHdl  = pContext->ItemInfo[ItemHdl].ParentFilter;
   struct DibDriverDabPacketServiceCtx  * pService;  

   DIB_ASSERT(pMonit);
   DIB_ASSERT(ItemHdl < DIB_MAX_NB_ITEMS);
   DIB_ASSERT(pContext->FilterInfo[FilterHdl].TypeFilter == eDABPACKET);
   DIB_ASSERT(((ClearMonit == eDIB_TRUE) || (ClearMonit == eDIB_FALSE)));


   DibDriverTargetDisableIrqProcessing(pContext);

   pService = pContext->ItemInfo[ItemHdl].ItemCtx.pDabPacketService;

   if(pMonit)
   {
      /*** Initialize DAB Monitoring structure ***/
      DibZeroMemory(pMonit, sizeof(struct DibDabPacketMonitoring));

      pMonit->ChannelIndex             = pContext->FilterInfo[pContext->ItemInfo[ItemHdl].ParentFilter].ParentChannel;
      pMonit->FilterIndex              = FilterHdl;
      if(pService)
      {
         pMonit->MscPacketGood         = pService->DabPacketMonitoring.MscPacketGood;
         pMonit->MscPacketsError       = pService->DabPacketMonitoring.MscPacketsError;
         pMonit->DataGroupGood         = pService->DabPacketMonitoring.DataGroupGood;
         pMonit->DataGroupError        = pService->DabPacketMonitoring.DataGroupError;
         /* Protected access to shared ressource, clear error */
         if(ClearMonit == eDIB_TRUE)
            DibZeroMemory(&pService->DabPacketMonitoring, sizeof(pService->DabPacketMonitoring));
      }    
   }
   DibDriverTargetEnableIrqProcessing(pContext);

   /* Send Message to clear internal monitoring info is sent separatly to avoid dead locks */
   if((pService) && (ClearMonit == eDIB_TRUE))
      DibDriverClearMonit(pContext, ItemHdl);

   DIB_DEBUG(SOFT_IF_LOG, (CRB "DibDriverGetDabPacketMonitoring %d" CRA, pMonit->FilterIndex));
   return DIBSTATUS_SUCCESS;   
}

DIBSTATUS DibDriverCheckParamDabPacket(struct DibDriverContext * pContext, union DibFilters * pFilterDesc)
{
   DIBSTATUS Status = DIBSTATUS_INVALID_PARAMETER;

   if(pFilterDesc->DabPacket.SubCh >= DIB_DAB_MAX_NB_CHANNELS)
      goto End;
   if(pFilterDesc->DabPacket.Address >= DIB_DABPACKET_MAX_NB_ADDRESS)
      goto End;
   if((pFilterDesc->DabPacket.UseFec != 1) && (pFilterDesc->DabPacket.UseFec != 0))
      goto End;
   if((pFilterDesc->DabPacket.UseDataGroup != 1) && (pFilterDesc->DabPacket.UseDataGroup != 0))
      goto End;
   
    Status = DIBSTATUS_SUCCESS;

End:
   return Status;   
}
#endif

#if (DIB_TDMB_DATA == 1)
DIBSTATUS DibDriverGetMonitoringTdmb(struct DibDriverContext * pContext, ELEM_HDL ItemHdl, union DibDataMonit * pDataMonit, uint8_t ClearMonit)
{
   struct DibDabMonitoring *pDabMonit = (struct DibDabMonitoring *) pDataMonit;
   FILTER_HDL                FilterHdl  = pContext->ItemInfo[ItemHdl].ParentFilter;
   struct DibDriverFilter   *pFilter    = &pContext->FilterInfo[FilterHdl];

   DIB_ASSERT(pDabMonit);
   DIB_ASSERT(ItemHdl < DIB_MAX_NB_ITEMS);
   DIB_ASSERT(pContext->FilterInfo[FilterHdl].TypeFilter == eTDMB);
   DIB_ASSERT(((ClearMonit == eDIB_TRUE) || (ClearMonit == eDIB_FALSE)));

   DIB_DEBUG(SOFT_IF_LOG, (CRB "DibDriverGetTdmbMonitoring %d %u" CRA, FilterHdl, pFilter->DvbTMonitoring.TotalPackets));

   DibDriverTargetDisableIrqProcessing(pContext);

   /* Set external DVB-T/TDMB monitoring info */
   pDabMonit->FilterIndex              = FilterHdl;
   pDabMonit->FilterType               = pContext->FilterInfo[FilterHdl].TypeFilter;
   pDabMonit->NbPid                    = pFilter->NbActivePids;
   pDabMonit->ChannelIndex             = pFilter->ParentChannel;
   /* checker of the bridge activated */
   if(pFilter->DvbTMonitoring.CheckTotalPackets)
      pDabMonit->TotalPackets             = pFilter->DvbTMonitoring.CheckTotalPackets;
   else
      pDabMonit->TotalPackets             = pFilter->DvbTMonitoring.TotalPackets;

   pDabMonit->CountinuityError         = pFilter->DvbTMonitoring.CheckCcFailCnt;
   pDabMonit->Error                    = pFilter->DvbTMonitoring.ErrCnt;
 
   if(ClearMonit == eDIB_TRUE)
   {
      /* Clear Driver monitoring info */
      DibZeroMemory(&pFilter->DvbTMonitoring, sizeof(struct DvbTMonit));
   }

   DibDriverTargetEnableIrqProcessing(pContext);

   if(ClearMonit == eDIB_TRUE)
   {
      /* Clear Firmware and Bridge monitoring info */
      DibDriverClearMonit(pContext, ItemHdl);
   }

   return DIBSTATUS_SUCCESS;
}

DIBSTATUS DibDriverCheckParamTdmb(struct DibDriverContext * pContext, union DibFilters * pFilterDesc)
{
   DIBSTATUS Status = DIBSTATUS_INVALID_PARAMETER;

   if(pFilterDesc->Tdmb.SubCh >= DIB_DAB_MAX_NB_CHANNELS)
      goto End;
   
    Status = DIBSTATUS_SUCCESS;

End:
   return Status;      
}
#endif

#if (DIB_FIG_DATA == 1)
DIBSTATUS DibDriverGetMonitoringFic(struct DibDriverContext *pContext, ELEM_HDL ItemHdl, union DibDataMonit *pDataMonit, uint8_t ClearMonit)
{
   struct DibFicMonitoring * pMonit = (struct DibFicMonitoring *) pDataMonit;
   FILTER_HDL                FilterHdl  = pContext->ItemInfo[ItemHdl].ParentFilter;
   struct DibDriverFigServiceCtx  * pService;  

   DIB_ASSERT(pMonit);
   DIB_ASSERT(ItemHdl < DIB_MAX_NB_ITEMS);
   DIB_ASSERT(pContext->FilterInfo[FilterHdl].TypeFilter == eFIG);
   DIB_ASSERT(((ClearMonit == eDIB_TRUE) || (ClearMonit == eDIB_FALSE)));


   DibDriverTargetDisableIrqProcessing(pContext);

   pService = pContext->ItemInfo[ItemHdl].ItemCtx.pFigService;

   if(pMonit)
   {
      /*** Initialize DAB Monitoring structure ***/
      DibZeroMemory(pMonit, sizeof(struct DibFicMonitoring));

      pMonit->ChannelIndex             = pContext->FilterInfo[pContext->ItemInfo[ItemHdl].ParentFilter].ParentChannel;
      pMonit->FilterIndex              = FilterHdl;
      if(pService)
      {
         pMonit->CorrectCnt            = pService->FicMonitoring.CorrectCnt;
         pMonit->ErrCnt                = pService->FicMonitoring.ErrCnt;
         /* Protected access to shared ressource, clear error and cc counts */
         if(ClearMonit == eDIB_TRUE)
            DibZeroMemory(&pService->FicMonitoring, sizeof(pService->FicMonitoring));
      }    
   }
   DibDriverTargetEnableIrqProcessing(pContext);

   /* Send Message to clear internal monitoring info is sent separatly to avoid dead locks */
   if((pService) && (ClearMonit == eDIB_TRUE))
      DibDriverClearMonit(pContext, ItemHdl);

   DIB_DEBUG(SOFT_IF_LOG, (CRB "DibDriverGetFicMonitoring %d" CRA, pMonit->FilterIndex));
   return DIBSTATUS_SUCCESS;   
}
#endif
