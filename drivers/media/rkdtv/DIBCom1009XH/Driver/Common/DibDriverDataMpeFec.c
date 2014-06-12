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
* @file "DibDriverIf.c"
* @brief Driver Interface.
*
***************************************************************************************************/
#include "DibDriverConfig.h" /* Must be first include of all SDK files - Defines compilation options */
#include "DibDriverCommon.h"
#include "DibDriver.h"
#include "DibDriverMessages.h"

#if (DIB_MPEFEC_DATA == 1)

/**
 * Get MpeFec Data monitoring
 * @param[in] pContext: driver context
 * @param[in] ItemHdl: filter item handler.
 * @param[out] pDVBMonit: DVB monitoring information of the corresponding item
 * @param[in] ClearMonit: Reset internal monitoring information after retreiving them.
 */
DIBSTATUS DibDriverGetMonitoringMpeFec(struct DibDriverContext *pContext, ELEM_HDL ItemHdl, union DibDataMonit *pDataMonit, uint8_t ClearMonit)
{
   struct DibDvbHMonitoring      *pDvbHMonit = (struct DibDvbHMonitoring *) pDataMonit;
   struct DibDriverMpeServiceCtx *pMpeService;     /* Points to MPE service context if target MPE. pMpeService_CONTEXT*/
   FILTER_HDL                     FilterHdl  = pContext->ItemInfo[ItemHdl].ParentFilter;

   DIB_ASSERT(pDvbHMonit);
   DIB_ASSERT(ItemHdl < DIB_MAX_NB_ITEMS);
   DIB_ASSERT(pContext->FilterInfo[FilterHdl].TypeFilter == eMPEFEC);
   DIB_ASSERT(((ClearMonit == eDIB_TRUE) || (ClearMonit == eDIB_FALSE)));
   
   DIB_DEBUG(SOFT_IF_LOG, (CRB "DibDriverGetDVBHMonitoring" CRA));

#if (LOG_APPLICATION_TO_DRIVER_CALLS_TO_FILE == 1)
   {
      DibLogToFile_Time(gLogFile, gLogFileStartTimeSec, gLogFileStartTimeMsec);
      DibLogToFile_Mesg(gLogFile, CRB "DibDriverGetDVBHMonitoring" CRA);
   }
#endif

   DibDriverTargetDisableIrqProcessing(pContext);

   pMpeService = pContext->ItemInfo[ItemHdl].ItemCtx.pMpeService;

   if(pMpeService)
   {
      if(pDvbHMonit)
      {
         /*** Initialize DVBH Monitoring structure ***/
         DibZeroMemory(pDvbHMonit, sizeof(struct DibDvbHMonitoring));

         pDvbHMonit->Pid                      = pContext->ItemInfo[ItemHdl].Config.MpeFec.Pid;
         pDvbHMonit->ChannelIndex             = pContext->FilterInfo[pContext->ItemInfo[ItemHdl].ParentFilter].ParentChannel;
         pDvbHMonit->FilterIndex              = FilterHdl;
         pDvbHMonit->MpePadding               = pMpeService->DvbHMonitoring.MpePadding;
         pDvbHMonit->MpePuncturing            = pMpeService->DvbHMonitoring.MpePuncturing;
         pDvbHMonit->TotalTables              = pMpeService->DvbHMonitoring.TotalTables;
         pDvbHMonit->ErrorTablesBeforeFec     = pMpeService->DvbHMonitoring.ErrorTablesBeforeFec; 
         pDvbHMonit->ErrorTablesAfterFec      = pMpeService->DvbHMonitoring.ErrorTablesAfterFec;
         pDvbHMonit->TableSize                = pMpeService->RsNumRows;
         pDvbHMonit->DeltaTStream             = pMpeService->DvbHMonitoring.DeltaTStream;
         pDvbHMonit->DeltaTDetected           = pMpeService->DvbHMonitoring.DeltaTDetected;
         pDvbHMonit->BurstStream              = pMpeService->DvbHMonitoring.BurstTimeStream;
         pDvbHMonit->BurstDetected            = pMpeService->DvbHMonitoring.BurstTimeDetected;
         pDvbHMonit->PowerUpTime              = pMpeService->DvbHMonitoring.PowerupTime; 
         pDvbHMonit->CountinuityError         = pMpeService->DvbHMonitoring.CcFailCnt;
         pDvbHMonit->Error                    = pMpeService->DvbHMonitoring.ErrCnt;

         if(pDvbHMonit->TotalTables)
            pDvbHMonit->Mfer = (uint32_t) ((pMpeService->DvbHMonitoring.ErrorTablesAfterFec * 1000) / pMpeService->DvbHMonitoring.TotalTables);
         else
            pDvbHMonit->Mfer = 0;
      }

      /* Protected access to shared ressource, clear error and cc counts */
      if(ClearMonit == eDIB_TRUE)
         DibZeroMemory(&pMpeService->DvbHMonitoring, sizeof(pMpeService->DvbHMonitoring));
   }
   DibDriverTargetEnableIrqProcessing(pContext);

   /* Send Message to clear internal monitoring info is sent separatly to avoid dead locks */
   if((pMpeService) && (ClearMonit == eDIB_TRUE))
      DibDriverClearMonit(pContext, ItemHdl);

   return DIBSTATUS_SUCCESS;
}

/**
 * Check MpeFec Parameters
 * @param[in]  pContext: driver context
 * @param[in]  pFilterDesc: filter descripter handler.
 * @param[out] Pid: Pointer to output Pid for filter coherency check
 */
DIBSTATUS DibDriverCheckParamMpeFec(struct DibDriverContext *pContext, union DibFilters *pFilterDesc, uint32_t *Pid)
{
   DIBSTATUS Status = DIBSTATUS_INVALID_PARAMETER;

   if((pFilterDesc->MpeFec.NbRows != eNBROWS_0) && (pFilterDesc->MpeFec.NbRows != eNBROWS_256)   &&
      (pFilterDesc->MpeFec.NbRows != eNBROWS_512) && (pFilterDesc->MpeFec.NbRows != eNBROWS_768) &&
      (pFilterDesc->MpeFec.NbRows != eNBROWS_1024))
      goto End;

   if(pFilterDesc->MpeFec.Pid >= DIB_POSITIVE_PIDS)
      goto End;

   if((pFilterDesc->MpeFec.Prefetch != ePREFETCH) && (pFilterDesc->MpeFec.Prefetch != eACTIVE))
      goto End;
         
   *Pid   = pFilterDesc->MpeFec.Pid;
   Status = DIBSTATUS_SUCCESS;

End:
   return Status;
}

DIBSTATUS DibDriverGetPidMode(struct DibDriverContext *pContext, union DibParamConfig *pParamConfig)
{
   DIBSTATUS Status = DIBSTATUS_INVALID_PARAMETER;
   uint32_t ElemHdl = pParamConfig->PidMode.ElemHdl;
               
#if (LOG_APPLICATION_TO_DRIVER_CALLS_TO_FILE == 1)
   DibLogToFileTime(gLogFile, gLogFileStartTimeSec, gLogFileStartTimeMsec);
   DibLogToFileMesg(gLogFile, CRB "DibDriverGetConfig: Item %d PidMode" CRA, pParamConfig->PidMode.ElemHdl);
#endif
   /* Check if valid */
   if((ElemHdl >= DIB_MAX_NB_ITEMS) || (pContext->ItemInfo[ElemHdl].ParentFilter == DIB_UNSET))
      goto End;

   /* Pid mode is only valid for Mpe or Mpe-IFec services */
   if((pContext->FilterInfo[(pContext->ItemInfo[ElemHdl].ParentFilter)].TypeFilter != eMPEFEC) &&
      (pContext->FilterInfo[(pContext->ItemInfo[ElemHdl].ParentFilter)].TypeFilter != eMPEIFEC))
      goto End;

   pParamConfig->PidMode.Mode = pContext->ItemInfo[ElemHdl].Config.MpeFec.Prefetch;

   Status = DIBSTATUS_SUCCESS;

End:
   return Status;
}


#endif
