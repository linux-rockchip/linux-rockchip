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
* @file "DibDriverDragonflyStdCmmb.c"
* @brief Cmmb Standard Handling.
*******************************************************************************/
#include "DibDriverConfig.h" /* Must be first include of all SDK files - Defines compilation options */

#if (USE_DRAGONFLY == 1)


#include "DibDriverCommon.h"
#include "DibDriverMessages.h"
#include "DibDriver.h"

#include "DibDriverDragonflyRegisters.h"   /* host-dragonfly register interface. Found in bridge/common        */
#include "DibDriverDragonflyMessages.h"    /* specific dragonlfy messages formatting. Found in bridge/common   */
#include "DibDriverDragonfly.h"

#if (DIB_CMMB_STD == 1)

/**
 * Extract CMMB demod monitoring info from incoming message and
 * copy it to corresponding structure.
 */
void DibDriverDragonflyProcessMsgMonitDemodCmmb(struct DibDriverContext *pContext, struct MsgAckGetSignalMonit * pMsg,
                                               struct DibTotalDemodMonit *pTotalMonitInfo)
{
   /* Destination: CMMB demod monitoring structure. */
   struct DibDvbDemodMonit *pDemodMonit;
   struct SignalMonitoring * pMon = &pMsg->Mon;

   pDemodMonit = &pTotalMonitInfo->DemodMonit[pMsg->DemodId].Cmmb.Dvb;

   pTotalMonitInfo->DemodMonit[pMsg->DemodId].Cmmb.Per = pMon->ChanDec.Cmmb.Per;
   pDemodMonit->Timeout                                = (pMon->MonitStatus == DIB_MONIT_TIMEOUT) ? 1 : 0;
   pDemodMonit->AgcPower                               = pMon->AdcPower;
   pDemodMonit->AgcGlobal                              = pMon->AgcGlobal;
   pDemodMonit->RfTotalGain                            = 0;
   pDemodMonit->WgnFp                                  = pMon->EqualNoise;
   pDemodMonit->SigFp                                  = pMon->EqualSignal;
   pDemodMonit->SysNoiseFp                             = pMon->SystemNoise;
   pDemodMonit->CombWgnFp                              = pMon->CombNoise;
   pDemodMonit->CombSigFp                              = pMon->CombSignal;
   pDemodMonit->TimingOffset                           = pMon->TimingOffset;
   pDemodMonit->FrequencyOffset                        = pMon->FrequencyOffset;
   pDemodMonit->VAgcRf                                 = pMon->AgcRf;
   pDemodMonit->VAgcBB                                 = pMon->AgcBb;
   pDemodMonit->AgcWbd                                 = pMon->AgcWbd;
   pDemodMonit->AgcSplitOffset                         = pMon->AgcSplitOffset;
   pDemodMonit->SignalQuality                          = 100;
   pDemodMonit->SignalQualityFast                      = 100;
   pDemodMonit->LnaStatus                              = 0;
   pDemodMonit->Locks                                  = pMon->Locks;
   pDemodMonit->TransmitterId                          = pMon->TransmitterId;
}

uint8_t DibDriverDragonflyGetTimeoutCmmb(struct DibTotalDemodMonit *pTotalMonitInfo, uint32_t TmpDemId)
{
   return pTotalMonitInfo->DemodMonit[TmpDemId].Cmmb.Dvb.Timeout;
}

/**
 * Fill in outgoing message with tuning channel parameters.
 */
DIBSTATUS DibDriverDragonflySetMsgTuneCmmb(struct DibTuneChan *pTc, struct MsgCreateChannel *pMsg)
{
   /* no support of TsArray with know parameters */
   DibSetMemory(&pMsg->Desc.Param, -1, sizeof(pMsg->Desc.Param));
   return DIBSTATUS_SUCCESS;
}

/**
 * Parse incoming message acknowledging tuning channel creation.
 * Extract and copy channel parameters to Driver context.
 */
DIBSTATUS DibDriverDragonflyGetChannelInfoCmmb(struct DibDriverContext *pContext, struct MsgUpdateChannelIndication * pMsg,
                                              uint16_t ChHdl)
{
   /* Destination structure. */
   struct DibCmmbTuneChan *pTcCmmb;

   /* Destination is different if TuneMonit. */
   if(pContext->ChannelInfo[ChHdl].Temporary == eDIB_FALSE)
   {
      pTcCmmb = &(pContext->ChannelInfo[ChHdl].ChannelDescriptor.Std.Cmmb);
   }
   else
   {
      pTcCmmb = &(pContext->ChannelInfo[ChHdl].MonitContext.TuneMonit.ChannelDescriptor.Std.Cmmb);
   }

   /* TODO Parse message and fill in structure. */
   return DIBSTATUS_SUCCESS;
}

#endif /* DIB_CMMB_STD */

#endif /* USE_DRAGONFLY */
