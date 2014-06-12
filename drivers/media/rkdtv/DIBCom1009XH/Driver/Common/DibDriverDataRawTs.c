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


#if (DIB_RAWTS_DATA == 1) || (DIB_TDMB_DATA == 1)

/**
 * Get RawTs Data monitoring, returns the statistics of the whole filter
 * @param[in] pContext: driver context
 * @param[in] ItemHdl: filter item handler.
 * @param[out] pDvbTMonit: DVB monitoring information of the corresponding item
 * @param[in] ClearMonit: Reset internal monitoring information after retreiving them.
 */
DIBSTATUS DibDriverGetMonitoringRawTs(struct DibDriverContext *pContext, ELEM_HDL ItemHdl, union DibDataMonit *pDataMonit, uint8_t ClearMonit)
{
   struct DibDvbTMonitoring *pDvbTMonit = (struct DibDvbTMonitoring *) pDataMonit;
   FILTER_HDL                FilterHdl  = pContext->ItemInfo[ItemHdl].ParentFilter;
   struct DibDriverFilter   *pFilter    = &pContext->FilterInfo[FilterHdl];

   DIB_ASSERT(pDvbTMonit);
   DIB_ASSERT(ItemHdl < DIB_MAX_NB_ITEMS);
   DIB_ASSERT(pContext->FilterInfo[FilterHdl].TypeFilter == eTS);
   DIB_ASSERT(((ClearMonit == eDIB_TRUE) || (ClearMonit == eDIB_FALSE)));

   DIB_DEBUG(SOFT_IF_LOG, (CRB "DibDriverGetDVBTMonitoring %d %u" CRA, FilterHdl, pFilter->DvbTMonitoring.TotalPackets));

   DibDriverTargetDisableIrqProcessing(pContext);

   /* Set external DVB-T/TDMB monitoring info */
   pDvbTMonit->FilterIndex              = FilterHdl;
   pDvbTMonit->NbPid                    = pFilter->NbActivePids;
   pDvbTMonit->ChannelIndex             = pFilter->ParentChannel;
   pDvbTMonit->TotalPackets             = pFilter->DvbTMonitoring.TotalPackets;
   pDvbTMonit->CountinuityError         = pFilter->DvbTMonitoring.CcFailCnt;
   pDvbTMonit->Error                    = pFilter->DvbTMonitoring.ErrCnt;
   pDvbTMonit->CheckTotalPackets        = pFilter->DvbTMonitoring.CheckTotalPackets;
   pDvbTMonit->CheckCountinuityError    = pFilter->DvbTMonitoring.CheckCcFailCnt;
   pDvbTMonit->CheckError               = pFilter->DvbTMonitoring.CheckErrCnt;
   
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


DIBSTATUS DibDriverCheckParamRawTs(struct DibDriverContext *pContext, union DibFilters *pFilterDesc, uint32_t *Pid)
{
   DIBSTATUS Status = DIBSTATUS_INVALID_PARAMETER;

   if ((pFilterDesc->Ts.Pid < DIB_MAX_NB_PIDS) || (pFilterDesc->Ts.Pid == DIB_ALL_PIDS) || (pFilterDesc->Ts.Pid == DIB_POSITIVE_PIDS))
   {
     *Pid    = pFilterDesc->Ts.Pid;
      Status = DIBSTATUS_SUCCESS;
   }

   return Status;
}
#endif
