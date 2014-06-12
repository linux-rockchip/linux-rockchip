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
* @file "DibDriverDataCmmb.c"
* @brief Cmmb Data Handling.
*******************************************************************************/
#include "DibDriverConfig.h" /* Must be first include of all SDK files - Defines compilation options */
#include "DibDriverCommon.h"
#include "DibDriver.h"
#include "DibDriverMessages.h"
#include "DibDriverIntDbg.h"

#if (DIB_CMMB_DATA == 1)

/**
 * Get Cmmb Data monitoring
 */
DIBSTATUS DibDriverGetMonitoringCmmb(struct DibDriverContext *pContext, ELEM_HDL ItemHdl, union DibDataMonit *pDataMonit, uint8_t ClearMonit)
{
   struct DibCmmbMonitoring       *pCmmbMonit = (struct DibCmmbMonitoring*)pDataMonit;
   struct DibDriverCmmbServiceCtx *pCmmbService;
   FILTER_HDL                     FilterHdl   = pContext->ItemInfo[ItemHdl].ParentFilter;

   DIB_ASSERT(pCmmbMonit);
   DIB_ASSERT(ItemHdl < DIB_MAX_NB_ITEMS);
   DIB_ASSERT(pContext->FilterInfo[FilterHdl].TypeFilter == eCMMBSVC);
   DIB_ASSERT(((ClearMonit == eDIB_TRUE) || (ClearMonit == eDIB_FALSE)));

   DIB_DEBUG(SOFT_IF_LOG, (CRB "DibDriverGetMonitoringCmmb" CRA));

#if (LOG_APPLICATION_TO_DRIVER_CALLS_TO_FILE == 1)
   {
      DibLogToFile_Time(gLogFile, gLogFileStartTimeSec, gLogFileStartTimeMsec);
      DibLogToFile_Mesg(gLogFile, CRB "DibDriverGetMonitoringCmmb" CRA);
   }
#endif

   DibDriverTargetDisableIrqProcessing(pContext);

   pCmmbService = pContext->ItemInfo[ItemHdl].ItemCtx.pCmmbService;

   if(pCmmbService)
   {
      if(pCmmbMonit)
      {
         /* Clear CMMB monitoring structure. */
         DibZeroMemory(pCmmbMonit, sizeof(struct DibCmmbMonitoring));

         /* Service identification. */
         pCmmbMonit->MfId                    = (uint8_t)pContext->ItemInfo[ItemHdl].Config.CmmbSvc.MfId;
         pCmmbMonit->ChannelIndex            = pContext->FilterInfo[pContext->ItemInfo[ItemHdl].ParentFilter].ParentChannel;
         pCmmbMonit->FilterIndex             = FilterHdl;

         /* LDPC blocks for latest RS matrix. */
         pCmmbMonit->TotalLdpcBlocks         = pCmmbService->CmmbMonitoring.TotalLdpcBlocks;
         pCmmbMonit->ErrorLdpcBlocks         = pCmmbService->CmmbMonitoring.ErrorLdpcBlocks;

         /* RS matrices statistics. */
         pCmmbMonit->TotalRsMatrices         = pCmmbService->CmmbMonitoring.TotalRsMatrices;
         pCmmbMonit->ErrorRsMatricesBeforeRs = pCmmbService->CmmbMonitoring.ErrorRsMatricesBeforeRs; 
         pCmmbMonit->ErrorRsMatricesAfterRs  = pCmmbService->CmmbMonitoring.ErrorRsMatricesAfterRs;

         /* Multiplex frame */
         pCmmbMonit->TotalMf                 = pCmmbService->CmmbMonitoring.TotalMf;
         pCmmbMonit->ErrorMfBeforeRs         = pCmmbService->CmmbMonitoring.ErrorMfBeforeRs; 
         pCmmbMonit->ErrorMfAfterRs          = pCmmbService->CmmbMonitoring.ErrorMfAfterRs;

         /* LDPC block error rate. */
         if(pCmmbMonit->TotalLdpcBlocks)
            pCmmbMonit->Bler = (uint32_t)((pCmmbMonit->ErrorLdpcBlocks * 1000) / pCmmbMonit->TotalLdpcBlocks);
         else
            pCmmbMonit->Bler = 0;

         /* RS matrix error rate. */
         if(pCmmbMonit->TotalRsMatrices)
            pCmmbMonit->Rsmer = (uint32_t)((pCmmbMonit->ErrorRsMatricesAfterRs * 1000) / pCmmbMonit->TotalRsMatrices);
         else
            pCmmbMonit->Rsmer = 0;

         /* Payload Checker statistics */
         pCmmbMonit->ErrorMf                = pCmmbService->CmmbMonitoring.ErrorMf;
         pCmmbMonit->DiscountMf             = pCmmbService->CmmbMonitoring.DiscountMf;
      }

      /* Protected access to shared ressource, clear. */
      if(ClearMonit == eDIB_TRUE)
         DibZeroMemory(&pCmmbService->CmmbMonitoring, sizeof(pCmmbService->CmmbMonitoring));
   }
   DibDriverTargetEnableIrqProcessing(pContext);

   /* Send message to clear internal monitoring info separately to avoid dead locks. */
   if((pCmmbService) && (ClearMonit == eDIB_TRUE))
      DibDriverClearMonit(pContext, ItemHdl);

   return DIBSTATUS_SUCCESS;
}

/**
* Initialization of context for channel decoder emulated by Host.
*/
void DibInitChannelDecoder(struct DibDriverContext *pContext)
{
#if DIB_INTERNAL_DEBUG == 1
   IntDriverReflexCmmbInit(pContext);
#endif
}

#endif /* DIB_CMMB_DATA */
