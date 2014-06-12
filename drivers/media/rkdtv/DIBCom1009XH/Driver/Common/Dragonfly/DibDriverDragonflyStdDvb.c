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
* @file "DibDriverDragonflyStdDvb.c"
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

#if (DIB_DVB_STD == 1)

void DibDriverDragonflyProcessMsgMonitDemodDvb(struct DibDriverContext * pContext, struct MsgAckGetSignalMonit * pMsg, struct DibTotalDemodMonit * pTotalMonitInfo)
{
   struct DibDvbDemodMonit * pDemodMonit;
   struct SignalMonitoring * pMon = &pMsg->Mon;

   pDemodMonit = &pTotalMonitInfo->DemodMonit[pMsg->DemodId].Dvb;
   pDemodMonit->Timeout           = (pMon->MonitStatus == DIB_MONIT_TIMEOUT) ? 1 : 0;
   pDemodMonit->AgcPower          = pMon->AdcPower;
   pDemodMonit->AgcGlobal         = pMon->AgcGlobal;
   pDemodMonit->RfTotalGain       = (pDemodMonit->AgcGlobal * pContext->Hd.BoardConfig->DefAgcDynamicsDb) / (uint32_t) (1 << 16);
   pDemodMonit->WgnFp             = pMon->EqualNoise;
   pDemodMonit->SigFp             = pMon->EqualSignal;
   pDemodMonit->CombWgnFp         = pMon->CombNoise;
   pDemodMonit->CombSigFp         = pMon->CombSignal;
   pDemodMonit->TimingOffset      = pMon->TimingOffset;
   pDemodMonit->FrequencyOffset   = pMon->FrequencyOffset;
   pDemodMonit->VAgcRf            = pMon->AgcRf;
   pDemodMonit->VAgcBB            = pMon->AgcBb;
   pDemodMonit->AgcWbd            = pMon->AgcWbd;
   pDemodMonit->AgcSplitOffset    = pMon->AgcSplitOffset;
   pDemodMonit->LnaStatus         = 0;
   pDemodMonit->Locks             = pMon->Locks;
   pDemodMonit->FecBerRs          = pMon->ChanDec.Dvbt.Ber;
   pDemodMonit->FecMpfailCnt      = pMon->ChanDec.Dvbt.Per;
   pDemodMonit->SignalQuality     = pMon->ChanDec.Dvbt.Quality;
   pDemodMonit->SignalQualityFast = pMon->ChanDec.Dvbt.FastQuality;
   pDemodMonit->TransmitterId     = pMon->TransmitterId;
}

uint8_t DibDriverDragonflyGetTimeoutDvb(struct DibTotalDemodMonit *pTotalMonitInfo, uint32_t TmpDemId)
{
   return pTotalMonitInfo->DemodMonit[TmpDemId].Dvb.Timeout;
}

void DibDriverDragonflyCheckMonitDvb(struct DibDriverContext * pContext, struct DibTotalDemodMonit * pTotalMonitInf, CHANNEL_HDL ChannelHdl)
{
   uint8_t NbDemod;
   uint8_t i;
   uint8_t ValidChanDecMonit = 0;
   struct DibDvbDemodMonit * pDemodMonit;
   NbDemod = pContext->ChannelInfo[ChannelHdl].pStream->NbConnFrontends;

   for (i=0;i<NbDemod;i++)
   {
      pDemodMonit = &pTotalMonitInf->DemodMonit[i].Dvb;
      if (DIB_GET_COFF_LOCK(pDemodMonit->Locks) == 1)
        ValidChanDecMonit = 1;
   }

   if (ValidChanDecMonit == 0)
   {
     pDemodMonit = &pTotalMonitInf->DemodMonit[0].Dvb;
     pDemodMonit->Locks             &= ~0x01C0;
   }
}

DIBSTATUS DibDriverDragonflySetMsgTuneDvb(struct DibTuneChan * pTc, struct MsgCreateChannel *pMsg)
{
   /* common part with dvb-sh */
   pMsg->Desc.Param.Dvbt.Dvb.Fft           = pTc->Std.Dvb.Nfft;
   pMsg->Desc.Param.Dvbt.Dvb.Guard         = pTc->Std.Dvb.Guard;
   pMsg->Desc.Param.Dvbt.Dvb.NativeIntlv   = pTc->Std.Dvb.IntlvNative;
   pMsg->Desc.Param.Dvbt.Dvb.InvSpect      = pTc->InvSpec;
   pMsg->Desc.Param.Dvbt.Dvb.Constellation = pTc->Std.Dvb.Nqam;

   /* dvb-t specific */
   pMsg->Desc.Param.Dvbt.ViterbiSelectHp   = pTc->Std.Dvb.VitSelectHp;
   pMsg->Desc.Param.Dvbt.ViterbiCodeRateHp = pTc->Std.Dvb.VitCodeRateHp;
   pMsg->Desc.Param.Dvbt.ViterbiCodeRateLp = pTc->Std.Dvb.VitCodeRateLp;
   pMsg->Desc.Param.Dvbt.ViterbiAlpha      = pTc->Std.Dvb.VitAlpha;
   pMsg->Desc.Param.Dvbt.ViterbiHierach    = pTc->Std.Dvb.VitHrch;

   return DIBSTATUS_SUCCESS;
}


DIBSTATUS DibDriverDragonflyGetChannelInfoDvb(struct DibDriverContext * pContext, struct MsgUpdateChannelIndication * pMsg, uint16_t ChHdl)
{
   struct DibTuneChan    * pTCh;
   struct DibDvbTuneChan * pTcDvb;
   struct DVBTChannelParameters * pParam = &pMsg->Desc.Param.Dvbt;

   /* do not store scanning information at the same place if TuneMonit */
   if(pContext->ChannelInfo[ChHdl].Temporary == eDIB_FALSE)
      pTCh = &(pContext->ChannelInfo[ChHdl].ChannelDescriptor);
   else
      pTCh = &(pContext->ChannelInfo[ChHdl].MonitContext.TuneMonit.ChannelDescriptor);

   pTcDvb = &(pTCh->Std.Dvb);

   pTcDvb->Nfft              = pParam->Dvb.Fft;
   pTcDvb->Guard             = pParam->Dvb.Guard;
   pTcDvb->Nqam              = pParam->Dvb.Constellation;
   pTcDvb->IntlvNative       = pParam->Dvb.NativeIntlv;
   pTCh->InvSpec             = pParam->Dvb.InvSpect;

   pTcDvb->VitHrch           = pParam->ViterbiHierach;
   pTcDvb->VitSelectHp       = pParam->ViterbiSelectHp;
   pTcDvb->VitAlpha          = pParam->ViterbiAlpha;
   pTcDvb->VitCodeRateHp     = pParam->ViterbiCodeRateHp;
   pTcDvb->VitCodeRateLp     = pParam->ViterbiCodeRateLp;

   return DIBSTATUS_SUCCESS;
}
#endif
#endif /* USE_DRAGONFLY */
