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


#if (DIB_MPEIFEC_DATA == 1)

/**
 * Get MpeIFec Data monitoring
 * @param[in] pContext: driver context
 * @param[in] ItemHdl: filter item handler.
 * @param[out] pDVBMonit: DVB-SH monitoring information of the corresponding item
 * @param[in] ClearMonit: Reset internal monitoring information after retreiving them.
 */
DIBSTATUS DibDriverGetMonitoringMpeIFec(struct DibDriverContext *pContext, ELEM_HDL ItemHdl, union DibDataMonit *pDataMonit, uint8_t ClearMonit)
{
   struct DibDvbSHMonitoring         *pDvbSHMonit = (struct DibDvbSHMonitoring *) pDataMonit;
   struct DibDriverMpeIFecServiceCtx *pMpeIFecService;     /* Points to MPE service context if target MPE. pMpeService_CONTEXT*/
   FILTER_HDL                         FilterHdl  = pContext->ItemInfo[ItemHdl].ParentFilter;

   DIB_ASSERT(pDvbSHMonit);
   DIB_ASSERT(ItemHdl < DIB_MAX_NB_ITEMS);
   DIB_ASSERT(pContext->FilterInfo[FilterHdl].TypeFilter == eMPEIFEC);
   DIB_ASSERT(((ClearMonit == eDIB_TRUE) || (ClearMonit == eDIB_FALSE)));
   
   DIB_DEBUG(SOFT_IF_LOG, (CRB "DibDriverGetDVBSHMonitoring" CRA));

#if (LOG_APPLICATION_TO_DRIVER_CALLS_TO_FILE == 1)
   {
      DibLogToFile_Time(gLogFile, gLogFileStartTimeSec, gLogFileStartTimeMsec);
      DibLogToFile_Mesg(gLogFile, CRB "DibDriverGetDVBSHMonitoring" CRA);
   }
#endif

   DibDriverTargetDisableIrqProcessing(pContext);

   pMpeIFecService = pContext->ItemInfo[ItemHdl].ItemCtx.pMpeIFecService;

   if(pMpeIFecService)
   {
      if(pDvbSHMonit)
      {
         /*** Initialize DVBH Monitoring structure ***/
         DibZeroMemory(pDvbSHMonit, sizeof(struct DibDvbSHMonitoring));

         pDvbSHMonit->Pid                    = pContext->ItemInfo[ItemHdl].Config.MpeIFec.Pid;
         pDvbSHMonit->ChannelIndex           = pContext->FilterInfo[pContext->ItemInfo[ItemHdl].ParentFilter].ParentChannel;
         pDvbSHMonit->FilterIndex            = FilterHdl;

         pDvbSHMonit->NbRows                 = pContext->ItemInfo[ItemHdl].Config.MpeIFec.NbRows;
         pDvbSHMonit->EncodParal             = pContext->ItemInfo[ItemHdl].Config.MpeIFec.EncodParal;
         pDvbSHMonit->SpreadingFactor        = pContext->ItemInfo[ItemHdl].Config.MpeIFec.SpreadingFactor;
         pDvbSHMonit->SendingDelay           = pContext->ItemInfo[ItemHdl].Config.MpeIFec.SendingDelay;
         pDvbSHMonit->NbAdstColumns          = pContext->ItemInfo[ItemHdl].Config.MpeIFec.NbAdstColumns;
         pDvbSHMonit->MaxIFecSect            = pContext->ItemInfo[ItemHdl].Config.MpeIFec.MaxIFecSect;

         pDvbSHMonit->AdstTotalTables        = pMpeIFecService->DvbSHMonitoring.AdstTotalTables;    
         pDvbSHMonit->AdstTablesLost         = pMpeIFecService->DvbSHMonitoring.AdstTablesLost;     
         pDvbSHMonit->AdstTablesCorrected    = pMpeIFecService->DvbSHMonitoring.AdstTablesCorrected;
         pDvbSHMonit->AdstTablesFailed       = pMpeIFecService->DvbSHMonitoring.AdstTablesFailed;   
         pDvbSHMonit->AdstTablesGood         = pMpeIFecService->DvbSHMonitoring.AdstTablesGood;

         pDvbSHMonit->AdtTotalTables         = pMpeIFecService->DvbSHMonitoring.AdtTotalTables;     
         pDvbSHMonit->AdtTablesCorrected     = pMpeIFecService->DvbSHMonitoring.AdtTablesCorrected; 
         pDvbSHMonit->AdtTablesFailed        = pMpeIFecService->DvbSHMonitoring.AdtTablesFailed;    
         pDvbSHMonit->AdtTablesGood          = pMpeIFecService->DvbSHMonitoring.AdtTablesGood;      

         pDvbSHMonit->DeltaTStream           = pMpeIFecService->DvbSHMonitoring.DeltaTStream;
         pDvbSHMonit->DeltaTDetected         = pMpeIFecService->DvbSHMonitoring.DeltaTDetected;
         pDvbSHMonit->BurstStream            = pMpeIFecService->DvbSHMonitoring.BurstStream;
         pDvbSHMonit->BurstDetected          = pMpeIFecService->DvbSHMonitoring.BurstDetected;
         pDvbSHMonit->PowerUpTime            = pMpeIFecService->DvbSHMonitoring.PowerUpTime; 

         pDvbSHMonit->CountinuityError       = pMpeIFecService->DvbSHMonitoring.CcFailCnt;
         pDvbSHMonit->Error                  = pMpeIFecService->DvbSHMonitoring.ErrCnt;
      }

      /* Protected access to shared ressource, clear error and cc counts */
      if(ClearMonit)
         DibZeroMemory(&pMpeIFecService->DvbSHMonitoring, sizeof(pMpeIFecService->DvbSHMonitoring));
   }

   DibDriverTargetEnableIrqProcessing(pContext);

   /* Send Message to clear internal monitoring info is sent separatly to avoid dead locks */
   if((pMpeIFecService) && (ClearMonit == eDIB_TRUE))
      DibDriverClearMonit(pContext, ItemHdl);

   return DIBSTATUS_SUCCESS;
}

/**
 * Check MpeIFec Parameters
 * @param[in]  pContext: driver context
 * @param[in]  pFilterDesc: filter descripter handler.
 * @param[out] Pid: Pointer to output Pid for filter coherency check
 */
DIBSTATUS DibDriverCheckParamMpeIFec(struct DibDriverContext *pContext, union DibFilters *pFilterDesc, uint32_t *Pid)
{
   DIBSTATUS Status = DIBSTATUS_INVALID_PARAMETER;

   if((pFilterDesc->MpeIFec.NbRows != eNBROWS_256) && (pFilterDesc->MpeIFec.NbRows != eNBROWS_512) && 
      (pFilterDesc->MpeIFec.NbRows != eNBROWS_768) && (pFilterDesc->MpeIFec.NbRows != eNBROWS_1024))
      goto End;

   if(pFilterDesc->MpeIFec.Pid >= DIB_POSITIVE_PIDS)
      goto End;

   if((pFilterDesc->MpeIFec.Prefetch != ePREFETCH) && (pFilterDesc->MpeIFec.Prefetch != eACTIVE))
      goto End;

   /* (B) is between 1-191 */
   if((pFilterDesc->MpeIFec.EncodParal < 1)  || (pFilterDesc->MpeIFec.EncodParal > 191))
      goto End;

   /* (C) is between 1-191 */
   if((pFilterDesc->MpeIFec.NbAdstColumns < 1) || (pFilterDesc->MpeIFec.NbAdstColumns > 191))
      goto End;

/*       Given Limited Range 0-255 (D) is automatically valid. */
 /*  if((pFilterDesc->MpeIFec.SendingDelay != 0))  Only Support D=0 for now 
      goto End;*/

   /* (S) is between 1-64 */
   if((pFilterDesc->MpeIFec.SpreadingFactor < 1) || (pFilterDesc->MpeIFec.SpreadingFactor > 64))
      goto End;

   /* (R) is between 1-64 */
   if((pFilterDesc->MpeIFec.MaxIFecSect < 1) || (pFilterDesc->MpeIFec.MaxIFecSect > 64))
      goto End;

   if(pFilterDesc->MpeIFec.MaxBurstDuration == 0)
      goto End;

   *Pid    = pFilterDesc->MpeIFec.Pid;
    Status = DIBSTATUS_SUCCESS;

End:
   return Status;
}
#endif
