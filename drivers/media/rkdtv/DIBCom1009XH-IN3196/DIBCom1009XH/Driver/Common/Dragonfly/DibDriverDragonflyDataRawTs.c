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
* @file "DibDriverDragonflyDataRawTs.c"
* @brief Dragonfly Driver specific RawTs Data handler
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
#include "DibMsgMac.h"

#if (DIB_RAWTS_DATA == 1) || (DIB_TDMB_DATA == 1) 

/****************************************************************************
 * Firmware send rawts monitoring total packets information
 ****************************************************************************/
void DibDriverDragonflyProcessMsgMonitorRawTs(struct DibDriverContext *pContext, uint32_t * Data)
{
   struct MsgRawTSMonitor Msg;
   uint32_t Total, Errors, Disc;
   ELEM_HDL ItemHdl;
   FILTER_HDL FilterId ;

   MsgRawTSMonitorUnpack(&pContext->RxSerialBuf, &Msg);
   Total    = Msg.TotalPackets;
   Errors   = Msg.ErrorPackets;
   Disc     = Msg.CCPackets;
   ItemHdl  = Msg.ItemHdl;
   FilterId = pContext->ItemInfo[ItemHdl].ParentFilter;

   /* warning: we can receive that message after filter deletion */
   if((pContext->FilterInfo[FilterId].TypeFilter == eTS) || (pContext->FilterInfo[FilterId].TypeFilter == eTDMB))
   {
      pContext->FilterInfo[FilterId].DvbTMonitoring.TotalPackets = Total;

      /* Embedded CC counter means something only in Pid filtering mode */
      if((pContext->ItemInfo[ItemHdl].Config.Ts.Pid & DIB_ALL_PIDS) != DIB_ALL_PIDS)
      {
         pContext->FilterInfo[FilterId].DvbTMonitoring.CcFailCnt    = Disc;
      }
      pContext->FilterInfo[FilterId].DvbTMonitoring.ErrCnt       = Errors;
      DIB_DEBUG(MSG_LOG, (CRB "MSG_RAWTS_MONITOR: FilterId %d received %d packets, with %d errors and %d disc" CRA, FilterId, Total, Errors, Disc));
   }
}
#endif

#if (DIB_RAWTS_DATA == 1)
DIBSTATUS DibDriverDragonflyAddItemRawTs(struct DibDriverContext * pContext, union DibFilters * pFilterDesc, ELEM_HDL * pElemHdl, uint32_t ItemHdl, struct MsgCreateItem * msg)
{
   msg->Param.Ts.Pid = pFilterDesc->Ts.Pid;
   return DIBSTATUS_SUCCESS;
}
#endif


#endif /* USE_DRAGONFLY */
