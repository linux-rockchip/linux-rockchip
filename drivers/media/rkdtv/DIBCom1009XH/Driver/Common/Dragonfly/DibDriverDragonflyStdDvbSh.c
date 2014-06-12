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
* @file "DibDriverDragonflyStdDvbSh.c"
* @brief Driver Interface.
*
***************************************************************************************************/
#include "DibDriverConfig.h" /* Must be first include of all SDK files - Defines compilation options */

#if (USE_DRAGONFLY == 1)


#include "DibDriverCommon.h"
#include "DibDriverMessages.h"
#include "DibDriver.h"

#include "DibDriverDragonflyRegisters.h"   /* host-dragonfly register interface. Found in bridge/common        */
#include "DibDriverDragonflyMessages.h"    /* specific dragonlfy messages formatting. Found in bridge/common   */
#include "DibDriverDragonfly.h"

#if (DIB_DVBSH_STD == 1)

void DibDriverDragonflyProcessMsgMonitDemodDvbSh(struct DibDriverContext * pContext, struct MsgAckGetSignalMonit * pMsg, struct DibTotalDemodMonit * pTotalMonitInfo)
{
   struct DibDvbDemodMonit * pDemodMonit;
   struct SignalMonitoring * pMon = &pMsg->Mon;

   pDemodMonit                                  = &pTotalMonitInfo->DemodMonit[pMsg->DemodId].Dvbsh.Dvb;
   pDemodMonit->Timeout                         = (pMon->MonitStatus == DIB_MONIT_TIMEOUT) ? 1 : 0;
   pDemodMonit->AgcPower                        = pMon->AdcPower;
   pDemodMonit->AgcGlobal                       = pMon->AgcGlobal;
   pDemodMonit->RfTotalGain                     = (pDemodMonit->AgcGlobal * pContext->Hd.BoardConfig->DefAgcDynamicsDb) / (uint32_t) (1 << 16);
   pDemodMonit->WgnFp                           = pMon->EqualNoise;
   pDemodMonit->SigFp                           = pMon->EqualSignal;
   pDemodMonit->CombWgnFp                       = pMon->CombNoise;
   pDemodMonit->CombSigFp                       = pMon->CombSignal;
   pDemodMonit->TimingOffset                    = pMon->TimingOffset;
   pDemodMonit->FrequencyOffset                 = pMon->FrequencyOffset;
   pDemodMonit->VAgcRf                          = pMon->AgcRf;
   pDemodMonit->VAgcBB                          = pMon->AgcBb;
   pDemodMonit->AgcWbd                          = pMon->AgcWbd;
   pDemodMonit->AgcSplitOffset                  = pMon->AgcSplitOffset;
   pDemodMonit->LnaStatus                       = 0;
   pDemodMonit->Locks                           = pMon->Locks;
   pDemodMonit->TransmitterId                   = pMon->TransmitterId;

   pTotalMonitInfo->DemodMonit[pMsg->DemodId].Dvbsh.Per = pMon->ChanDec.Dvbsh.Per;
   pDemodMonit->SignalQuality                           = pMon->ChanDec.Dvbsh.Quality;
   pDemodMonit->SignalQualityFast                       = pMon->ChanDec.Dvbsh.FastQuality;
}

uint8_t DibDriverDragonflyGetTimeoutDvbSh(struct DibTotalDemodMonit *pTotalMonitInfo, uint32_t TmpDemId)
{
   return pTotalMonitInfo->DemodMonit[TmpDemId].Dvbsh.Dvb.Timeout;
}

DIBSTATUS DibDriverDragonflySetMsgTuneDvbSh(struct DibTuneChan * pTc, struct MsgCreateChannel *pMsg)
{
   /* common part with dvb-t */
   pMsg->Desc.Param.Dvbsh.Dvb.Fft           = pTc->Std.Dvbsh.Dvb.Nfft;
   pMsg->Desc.Param.Dvbsh.Dvb.NativeIntlv   = pTc->Std.Dvbsh.Dvb.IntlvNative;
   pMsg->Desc.Param.Dvbsh.Dvb.InvSpect      = pTc->InvSpec;
   pMsg->Desc.Param.Dvbsh.Dvb.Constellation = pTc->Std.Dvbsh.Dvb.Nqam;

   /* speDesc.cific dvb-sh part */
   pMsg->Desc.Param.Dvbsh.TurboLateTaps     = pTc->Std.Dvbsh.NbLateTaps;
   pMsg->Desc.Param.Dvbsh.TurboNonLateIncr  = pTc->Std.Dvbsh.NonLateIncr;
   pMsg->Desc.Param.Dvbsh.TurboCommonMult   = pTc->Std.Dvbsh.CommonMult;
   pMsg->Desc.Param.Dvbsh.TurboNbSlices     = pTc->Std.Dvbsh.NbSlices;
   pMsg->Desc.Param.Dvbsh.TurboSliceDist    = pTc->Std.Dvbsh.SliceDistance;
   pMsg->Desc.Param.Dvbsh.CodeRateHp        = pTc->Std.Dvbsh.Dvb.VitCodeRateHp;
   pMsg->Desc.Param.Dvbsh.CodeRateLp        = pTc->Std.Dvbsh.Dvb.VitCodeRateLp;
   pMsg->Desc.Param.Dvbsh.Alpha             = pTc->Std.Dvbsh.Dvb.VitAlpha;
   pMsg->Desc.Param.Dvbsh.SelectHp          = pTc->Std.Dvbsh.Dvb.VitSelectHp;
   pMsg->Desc.Param.Dvbsh.Hierach           = pTc->Std.Dvbsh.Dvb.VitHrch;

   return DIBSTATUS_SUCCESS;
}


DIBSTATUS DibDriverDragonflyGetChannelInfoDvbSh(struct DibDriverContext * pContext, struct MsgUpdateChannelIndication * pMsg, uint16_t ChHdl)
{
   return DIBSTATUS_SUCCESS;
}

#endif

#endif /* USE_DRAGONFLY */
