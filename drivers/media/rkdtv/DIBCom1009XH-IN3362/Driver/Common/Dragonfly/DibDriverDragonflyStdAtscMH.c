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
* @file "DibDriverDragonflyStdAtscMH.c"
* @brief AtscMH Standard Handling.
*******************************************************************************/
#include "DibDriverConfig.h" /* Must be first include of all SDK files - Defines compilation options */

#if (USE_DRAGONFLY == 1)


#include "DibDriverCommon.h"
#include "DibDriverMessages.h"
#include "DibDriver.h"

#include "DibDriverDragonflyRegisters.h"   /* host-dragonfly register interface. Found in bridge/common        */
#include "DibDriverDragonflyMessages.h"    /* specific dragonlfy messages formatting. Found in bridge/common   */
#include "DibDriverDragonfly.h"

#if (DIB_ATSCMH_STD == 1)
/**
 * Extract demod monitoring info from incoming message and
 * copy it to corresponding structure. Same As ATSC version !
 */


uint8_t DibDriverDragonflyGetTimeoutAtscMH(struct DibTotalDemodMonit *pTotalMonitInfo, uint32_t TmpDemId)
{
  /*return pTotalMonitInfo->DemodMonit[TmpDemId].AtscMH.Dvb.Timeout;*/
}

/**
 * Fill in outgoing message with tuning channel parameters.
 */
DIBSTATUS DibDriverDragonflySetMsgTuneAtscMH(struct DibTuneChan *pTc, struct MsgCreateChannel *pMsg)
{
   /* no support of TsArray with know parameters */
   DibSetMemory(&pMsg->Desc.Param, -1, sizeof(pMsg->Desc.Param));
   return DIBSTATUS_SUCCESS;
}

/**
 * Parse incoming message acknowledging tuning channel creation.
 * Extract and copy channel parameters to Driver context.
 */
DIBSTATUS DibDriverDragonflyGetChannelInfoAtscMH(struct DibDriverContext *pContext, struct MsgUpdateChannelIndication * pMsg,
                                              uint16_t ChHdl)
{
#if 0
   /* Destination structure. */
   struct DibAtscMHTuneChan *pTcAtscMH;

   /* Destination is different if TuneMonit. */
   if(pContext->ChannelInfo[ChHdl].Temporary == eDIB_FALSE)
   {
      pTcAtscMH = &(pContext->ChannelInfo[ChHdl].ChannelDescriptor.Std.AtscMH);
   }
   else
   {
      pTcAtscMH = &(pContext->ChannelInfo[ChHdl].MonitContext.TuneMonit.ChannelDescriptor.Std.AtscMH);
   }
#endif
   /* TODO Parse message and fill in structure. */
   return DIBSTATUS_SUCCESS;
}

#endif /* DIB_ATSCMH_STD */

#endif /* USE_DRAGONFLY */
