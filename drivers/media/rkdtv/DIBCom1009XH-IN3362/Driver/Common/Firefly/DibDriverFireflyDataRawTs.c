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
* @file "DibDriverFireflyDataRawTs.c"
* @brief Firefly Driver specific RawTs Data handler
*
***************************************************************************************************/
#include "DibDriverConfig.h" /* Must be first include of all SDK files - Defines compilation options */

#if (USE_FIREFLY == 1)


#include "DibDriverCommon.h"
#include "DibDriverRegisterIf.h"
#include "DibDriverMessages.h"
#include "DibDriver.h"
#include "DibDriverFirefly.h"

#if (DIB_RAWTS_DATA == 1)

/****************************************************************************
 * Firmware send rawts monitoring total packets information
 ****************************************************************************/
void DibDriverFireflyProcessMsgMonitorRawTs(struct DibDriverContext *pContext, uint16_t * Data)
{
   ELEM_HDL ItemHdl = pContext->FireflyService[ Data[0] ].ItemIndex;
   uint32_t NbPackets = (uint32_t)((Data[2] << 16) | Data[1]);
   uint32_t NbDiscount = (uint32_t)((Data[4] << 16) | Data[3]);
   FILTER_HDL FilterId = pContext->ItemInfo[ItemHdl].ParentFilter;

   DIB_ASSERT(Data);

   /* warning: we can receive that message after filter deletion */
   if(pContext->FilterInfo[FilterId].TypeFilter == eTS)
   {
      pContext->FilterInfo[FilterId].DvbTMonitoring.TotalPackets = NbPackets;

      /* Embedded CC counter means something only in Pid filtering mode */
      if((pContext->ItemInfo[ItemHdl].Config.Ts.Pid & DIB_ALL_PIDS) != DIB_ALL_PIDS)
      {
         pContext->FilterInfo[FilterId].DvbTMonitoring.CcFailCnt = NbDiscount;
      }
      pContext->FilterInfo[FilterId].DvbTMonitoring.ErrCnt = 0; /* NO TEI on firefly */

      DIB_DEBUG(MSG_LOG,(CRB "MSG_RAWTS_MONITOR: FilterId %d received %d packets, with %d errors and %d disc" CRA, FilterId, NbPackets, 0, NbDiscount));
   }
}

DIBSTATUS DibDriverFireflyAddItemRawTs(struct DibDriverContext *pContext, union DibFilters *pFilterDesc, struct DibAddPidInfo *PidInfo)
{
   PidInfo->PidTarget = eTARGET_RAW_TS;
   PidInfo->Pid       = pFilterDesc->Ts.Pid;

   return DIBSTATUS_SUCCESS;
}

DIBSTATUS DibDriverFireflyAddPidRawTs(struct DibDriverContext *pContext, struct DibDriverItem *pItem, FILTER_HDL FilterHdl, uint8_t *format, uint8_t *isWatch, enum DibPidMode *PidMode)
{
   /* For Raw TS */
   pContext->FilterInfo[FilterHdl].NbActivePids++;
   DIB_ASSERT(pContext->FilterInfo[FilterHdl].NbActivePids <= DIB_MAX_NB_SERVICES);
   DIB_DEBUG(SOFT_IF_LOG, (CRB "DibDriverif_PidFilter: Active Raw TS PIDs: %d" CRA,
         pContext->FilterInfo[FilterHdl].NbActivePids));

   *format = FORMAT_RAWTS;
      
   return DIBSTATUS_SUCCESS;
}
#endif


#endif /* USE_FIREFLY */
