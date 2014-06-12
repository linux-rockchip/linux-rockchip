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
* @file "DibDriverDragonflyStdAtsc.c"
* @brief Atsc Standard Handling.
*******************************************************************************/
#include "DibDriverConfig.h" /* Must be first include of all SDK files - Defines compilation options */

#if (USE_DRAGONFLY == 1)


#include "DibDriverCommon.h"
#include "DibDriverMessages.h"
#include "DibDriver.h"

#include "DibDriverDragonflyRegisters.h"   /* host-dragonfly register interface. Found in bridge/common        */
#include "DibDriverDragonflyMessages.h"    /* specific dragonlfy messages formatting. Found in bridge/common   */
#include "DibDriverDragonfly.h"

#if (DIB_ATSC_STD == 1)

/**
 * Extract CMMB demod monitoring info from incoming message and
 * copy it to corresponding structure.
 */

void DibDriverDragonflyProcessMsgMonitDemodAtsc(struct DibDriverContext *pContext, struct MsgAckGetSignalMonit * pMsg,
                                               struct DibTotalDemodMonit *pTotalMonitInfo)
{

   /* Destination: CMMB demod monitoring structure. */
   struct DibAtscDemodMonit *pDemodMonit;
   struct SignalMonitoring * pMon = &pMsg->Mon;


   pDemodMonit = (struct DibAtscDemodMonit *)&(pTotalMonitInfo->DemodMonit[pMsg->DemodId].Atsc);

   /*pTotalMonitInfo->DemodMonit[pMsg->DemodId].Atsc = pMon->ChanDec.Atsc.Per;*/
   pDemodMonit->Dvb.FecBerRs                               = pMon->ChanDec.Atsc.Ber;
   pDemodMonit->Dvb.FecMpfailCnt                           = pMon->ChanDec.Atsc.Per >> 16;
   pDemodMonit->Dvb.Timeout                                = (pMon->MonitStatus == DIB_MONIT_TIMEOUT) ? 1 : 0;
   pDemodMonit->Dvb.AgcPower                               = pMon->AdcPower;
   pDemodMonit->Dvb.AgcGlobal                              = pMon->AgcGlobal;
   pDemodMonit->Dvb.RfTotalGain                            = 0;
   pDemodMonit->Dvb.WgnFp                                  = pMon->EqualNoise;
   pDemodMonit->Dvb.SigFp                                  = pMon->EqualSignal;
   pDemodMonit->Dvb.SysNoiseFp                             = pMon->SystemNoise;
   pDemodMonit->Dvb.CombWgnFp                              = pMon->CombNoise;
   pDemodMonit->Dvb.CombSigFp                              = pMon->CombSignal;
   pDemodMonit->Dvb.TimingOffset                           = pMon->TimingOffset;
   pDemodMonit->Dvb.FrequencyOffset                        = pMon->FrequencyOffset;
   pDemodMonit->Dvb.VAgcRf                                 = pMon->AgcRf;
   pDemodMonit->Dvb.VAgcBB                                 = pMon->AgcBb;
   pDemodMonit->Dvb.AgcWbd                                 = pMon->AgcWbd;
   pDemodMonit->Dvb.AgcSplitOffset                         = pMon->AgcSplitOffset;
   pDemodMonit->Dvb.SignalQuality                          = 100; /*pMon->ChanDec.Atsc.Quality;*/
   pDemodMonit->Dvb.SignalQualityFast                      = 100; /*pMon->ChanDec.Atsc.FastQuality;*/
   pDemodMonit->Dvb.LnaStatus                              = 0;
   pDemodMonit->Dvb.Locks                                  = pMon->Locks;
   pDemodMonit->Dvb.TransmitterId                          = pMon->TransmitterId;
}

uint8_t DibDriverDragonflyGetTimeoutAtsc(struct DibTotalDemodMonit *pTotalMonitInfo, uint32_t TmpDemId)
{
   return pTotalMonitInfo->DemodMonit[TmpDemId].Atsc.Dvb.Timeout;
}

/**
 * Fill in outgoing message with tuning channel parameters.
 */
DIBSTATUS DibDriverDragonflySetMsgTuneAtsc(struct DibTuneChan *pTc, struct MsgCreateChannel *pMsg)
{
   pMsg->Desc.Param.Atsc.dummy = -1;
   return DIBSTATUS_SUCCESS;
}

/**
 * Parse incoming message acknowledging tuning channel creation.
 * Extract and copy channel parameters to Driver context.
 */
DIBSTATUS DibDriverDragonflyGetChannelInfoAtsc(struct DibDriverContext *pContext, struct MsgUpdateChannelIndication * pMsg,
                                              uint16_t ChHdl)
{
#if 0
   /* Destination structure. */
   struct DibAtscTuneChan *pTcAtsc;

   /* Destination is different if TuneMonit. */
   if(pContext->ChannelInfo[ChHdl].Temporary == eDIB_FALSE)
   {
      pTcAtsc = &(pContext->ChannelInfo[ChHdl].ChannelDescriptor.Std.Atsc);
   }
   else
   {
      pTcAtsc = &(pContext->ChannelInfo[ChHdl].MonitContext.TuneMonit.ChannelDescriptor.Std.Atsc);
   }
#endif
   /* TODO Parse message and fill in structure. */
   return DIBSTATUS_SUCCESS;
}

#endif /* DIB_ATSC_STD */

#endif /* USE_DRAGONFLY */
