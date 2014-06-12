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

#if (DIB_CTTB_STD == 1)

void DibDriverDragonflyProcessMsgMonitDemodCttb(struct DibDriverContext * pContext, struct MsgAckGetSignalMonit * pMsg, struct DibTotalDemodMonit * pTotalMonitInfo)
{
   struct DibCttbDemodMonit * pDemodMonit;
   struct SignalMonitoring * pMon = &pMsg->Mon;

   pDemodMonit = &pTotalMonitInfo->DemodMonit[pMsg->DemodId].Cttb;
   pDemodMonit->Dvb.Timeout            = (pMon->MonitStatus == DIB_MONIT_TIMEOUT) ? 1 : 0;
   pDemodMonit->Dvb.Locks              = pMon->Locks;
   pDemodMonit->LdpcSyndrome           = pMon->ChanDec.Cttb.LdpcSyndrome;
   pDemodMonit->LdpcSyndromeFirstIter  = pMon->ChanDec.Cttb.LdpcSyndromeFirstIter;
   pDemodMonit->LdpcNbIter             = pMon->ChanDec.Cttb.LdpcNbIter;
   pDemodMonit->LdpcFlags              = pMon->ChanDec.Cttb.LdpcFlags;
   pDemodMonit->Per                    = pMon->ChanDec.Cttb.Per;


   pDemodMonit->Dvb.AgcPower           = pMon->AdcPower;
   pDemodMonit->Dvb.AgcGlobal          = pMon->AgcGlobal;
   pDemodMonit->Dvb.RfTotalGain        = (pDemodMonit->Dvb.AgcGlobal * pContext->Hd.BoardConfig->DefAgcDynamicsDb) / (uint32_t) (1 << 16);
   pDemodMonit->Dvb.WgnFp              = pMon->EqualNoise;
   pDemodMonit->Dvb.SigFp              = pMon->EqualSignal;
   pDemodMonit->Dvb.CombWgnFp          = pMon->CombNoise;
   pDemodMonit->Dvb.CombSigFp          = pMon->CombSignal;
   pDemodMonit->Dvb.TimingOffset       = pMon->TimingOffset;
   pDemodMonit->Dvb.FrequencyOffset    = pMon->FrequencyOffset;
   pDemodMonit->Dvb.VAgcRf             = pMon->AgcRf;
   pDemodMonit->Dvb.VAgcBB             = pMon->AgcBb;
   pDemodMonit->Dvb.AgcWbd             = pMon->AgcWbd;
   pDemodMonit->Dvb.AgcSplitOffset     = pMon->AgcSplitOffset;
   pDemodMonit->Dvb.LnaStatus          = 0;
   pDemodMonit->Dvb.FecBerRs           = 0;
   pDemodMonit->Dvb.FecMpfailCnt       = 0;
   pDemodMonit->Dvb.SignalQuality      = pMon->ChanDec.Cttb.Quality;
   pDemodMonit->Dvb.SignalQualityFast  = pMon->ChanDec.Cttb.FastQuality;
   pDemodMonit->Dvb.TransmitterId      = pMon->TransmitterId;
}

uint8_t DibDriverDragonflyGetTimeoutCttb(struct DibTotalDemodMonit * pTotalMonitInfo, uint32_t TmpDemId)
{
   return pTotalMonitInfo->DemodMonit[TmpDemId].Cttb.Dvb.Timeout;
}

void DibDriverDragonflyCheckMonitCttb(struct DibDriverContext * pContext, struct DibTotalDemodMonit * pTotalMonitInf, CHANNEL_HDL ChannelHdl)
{
   uint8_t NbDemod;
   uint8_t i;
   uint8_t ValidChanDecMonit = 0;
   struct DibCttbDemodMonit * pDemodMonit;
   NbDemod = pContext->ChannelInfo[ChannelHdl].pStream->NbConnFrontends;

   for (i=0;i<NbDemod;i++)
   {
      pDemodMonit = &pTotalMonitInf->DemodMonit[i].Cttb;
      if (DIB_GET_COFF_LOCK(pDemodMonit->Dvb.Locks) == 1)
        ValidChanDecMonit = 1;
   }

   if (ValidChanDecMonit == 0)
   {
     pDemodMonit = &pTotalMonitInf->DemodMonit[0].Cttb;
     pDemodMonit->Per               = 0xFFFFFFFF;
   }

}


DIBSTATUS DibDriverDragonflySetMsgTuneCttb(struct DibTuneChan * pTc, struct MsgCreateChannel * pMsg)
{

   pMsg->Desc.Param.Cttb.Fft                      = pTc->Std.Cttb.Nfft;
   pMsg->Desc.Param.Cttb.Guard                    = pTc->Std.Cttb.Guard;
   pMsg->Desc.Param.Cttb.Constellation            = pTc->Std.Cttb.Nqam;
   pMsg->Desc.Param.Cttb.LdpcCodeRate             = pTc->Std.Cttb.LdpcCodeRate;
   pMsg->Desc.Param.Cttb.Intlv                    = pTc->Std.Cttb.TimeIntlv;

   return DIBSTATUS_SUCCESS;
}

DIBSTATUS DibDriverDragonflyGetChannelInfoCttb(struct DibDriverContext * pContext, struct MsgUpdateChannelIndication * pMsg, uint16_t ChHdl)
{
   struct DibTuneChan    * pTCh;
   struct DibCttbTuneChan * pTcCttb;
   struct CTTBChannelParameters * pParam = &pMsg->Desc.Param.Cttb;

   /* do not store scanning information at the same place if TuneMonit */
   if(pContext->ChannelInfo[ChHdl].Temporary == eDIB_FALSE)
      pTCh = &(pContext->ChannelInfo[ChHdl].ChannelDescriptor);
   else
      pTCh = &(pContext->ChannelInfo[ChHdl].MonitContext.TuneMonit.ChannelDescriptor);

   pTcCttb = &pTCh->Std.Cttb;

   pTcCttb->Nfft                       = pParam->Fft;
   pTcCttb->Guard                      = pParam->Guard;
   pTcCttb->Nqam                       = pParam->Constellation;
   pTcCttb->LdpcCodeRate               = pParam->LdpcCodeRate;
   pTcCttb->TimeIntlv                  = pParam->Intlv;

   return DIBSTATUS_SUCCESS;
}

#endif

#endif /* USE_DRAGONFLY */
