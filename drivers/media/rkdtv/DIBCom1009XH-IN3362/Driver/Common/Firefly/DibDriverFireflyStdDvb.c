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
* @file "DibDriverFireflyStdDvb.c"
* @brief Driver Interface.
*
***************************************************************************************************/
#include "DibDriverConfig.h" /* Must be first include of all SDK files - Defines compilation options */

#if (USE_FIREFLY == 1)


#include "DibDriverCommon.h"
#include "DibDriver.h"
#include "DibDriverMessages.h"
#include "DibDriverFireflyMessages.h"

#if (DIB_DVB_STD == 1)

/**
 * IntDriverFireflyParseTpsRegisters
 *
 * register address: 478, 479, 480, 481, 482, 487, 535
 * TpsRegisters[] idx.: 0  , 1  , 2  , 3  , 4  , 5  , 6
 */
void IntDriverFireflyParseTpsRegisters(struct DibDriverContext *pContext, uint16_t *pTpsRegisters, struct DibDvbTuneChan *pStc)
{
   struct DibTpsInfo *pSti = &(pStc->TpsInfo);

   DIB_ASSERT(pTpsRegisters);
   DIB_ASSERT(pStc);
   
   pSti->TpsDecVar         = pTpsRegisters[0];
   pSti->TpsDec            = (uint8_t) ((pTpsRegisters[1] & (1 << 9)) >> 9);
   pSti->TpsSymbolNum      = (uint8_t) ((pTpsRegisters[1] & 0x1fc) >> 2);
   pSti->TpsFrameNum       = (uint8_t) (pTpsRegisters[1] & 3);
   pSti->TpsNqam           = (uint8_t) ((pTpsRegisters[2] & 0xc000) >> 14);
   pSti->TpsHrch           = (uint8_t) ((pTpsRegisters[2] & 0x2000) >> 13);
   pSti->TpsVitAlpha       = (uint8_t) ((pTpsRegisters[2] & 0x1c00) >> 10);
   pSti->TpsNfft           = (uint8_t) ((pTpsRegisters[2] & 0x300) >> 8);
   pSti->TpsCodeRateHp     = (uint8_t) ((pTpsRegisters[2] & 0xe0) >> 5);
   pSti->TpsCodeRateLp     = (uint8_t) ((pTpsRegisters[2] & 0x1c) >> 2);
   pSti->TpsGuard          = (uint8_t) (pTpsRegisters[2] & 3);
   pSti->TpsNativeIntlv    = (uint8_t) ((pTpsRegisters[3] & (1 << 5)) >> 5);
   pSti->TpsDvbH           = (uint8_t) ((pTpsRegisters[3] & (1 << 4)) >> 4);
   pSti->TpsTimeSlicingLp  = (uint8_t) ((pTpsRegisters[3] & (1 << 3)) >> 3);
   pSti->TpsTimeSlicingHp  = (uint8_t) ((pTpsRegisters[3] & (1 << 2)) >> 2);
   pSti->TpsMpeFecLp       = (uint8_t) ((pTpsRegisters[3] & (1 << 1)) >> 1);
   pSti->TpsMpeFecHp       = (uint8_t) ((pTpsRegisters[3] & (1 << 0)) >> 0);
   pSti->TpsCellId         = pTpsRegisters[4];
   pSti->TpsLen            = (uint8_t) ((pTpsRegisters[5] & 0xfc0) >> 6);
   pSti->TpsReserved       = (uint8_t) (pTpsRegisters[5] & 0x3f);
   pSti->TpsDecLock        = (uint8_t) ((pTpsRegisters[6] & (1 << 5)) >> 5);
   pSti->TpsSyncLock       = (uint8_t) ((pTpsRegisters[6] & (1 << 4)) >> 4);
   pSti->TpsDataLock       = (uint8_t) ((pTpsRegisters[6] & (1 << 3)) >> 3);
   pSti->TpsBchUncor       = (uint8_t) ((pTpsRegisters[6] & (1 << 2)) >> 2);
   pSti->TpsCellIdLock     = (uint8_t) ((pTpsRegisters[6] & (1 << 1)) >> 1);

   pStc->Nfft              = (enum DibFFTSize)           pSti->TpsNfft;
   pStc->Nqam              = (enum DibConstellation)     pSti->TpsNqam;
   pStc->Guard             = (enum DibGuardIntervalSize) pSti->TpsGuard;
   pStc->VitCodeRateHp     = (enum DibVitCoderate)       pSti->TpsCodeRateHp;
   pStc->VitCodeRateLp     = (enum DibVitCoderate)       pSti->TpsCodeRateLp;
   pStc->VitAlpha          = (enum DibVitAlpha)          pSti->TpsVitAlpha;
   pStc->VitHrch           = (enum DibVitHrch)           pSti->TpsHrch;
   pStc->IntlvNative       = (enum DibIntlvNative)       pSti->TpsNativeIntlv;

   DIB_DEBUG(MSG_LOG, (CRB "  IntDriverFireflyParseTpsRegisters" CRA));
   DIB_DEBUG(MSG_LOG, (CRB "  \tTpsDec           = %u" CRA, pSti->TpsDec));
   DIB_DEBUG(MSG_LOG, (CRB "  \tTpsSymbolNum     = %u" CRA, pSti->TpsSymbolNum));
   DIB_DEBUG(MSG_LOG, (CRB "  \tTpsFrameNum      = %u" CRA, pSti->TpsFrameNum));
   DIB_DEBUG(MSG_LOG, (CRB "  \tTpsNqam          = %u" CRA, pSti->TpsNqam));
   DIB_DEBUG(MSG_LOG, (CRB "  \tTpsHrch          = %u" CRA, pSti->TpsHrch));
   DIB_DEBUG(MSG_LOG, (CRB "  \tTpsVitAlpha      = %u" CRA, pSti->TpsVitAlpha));
   DIB_DEBUG(MSG_LOG, (CRB "  \tTpsNfft          = %u" CRA, pSti->TpsNfft));
   DIB_DEBUG(MSG_LOG, (CRB "  \tTpsCodeRateHp    = %u" CRA, pSti->TpsCodeRateHp));
   DIB_DEBUG(MSG_LOG, (CRB "  \tTpsCodeRateLp    = %u" CRA, pSti->TpsCodeRateLp));
   DIB_DEBUG(MSG_LOG, (CRB "  \tTpsGuard         = %u" CRA, pSti->TpsGuard));
   DIB_DEBUG(MSG_LOG, (CRB "  \tTpsNativeIntlv   = %u" CRA, pSti->TpsNativeIntlv));
   DIB_DEBUG(MSG_LOG, (CRB "  \tTpsDvbH          = %u" CRA, pSti->TpsDvbH));
   DIB_DEBUG(MSG_LOG, (CRB "  \tTpsTimeSlicingHp = %u" CRA, pSti->TpsTimeSlicingHp));
   DIB_DEBUG(MSG_LOG, (CRB "  \tTpsTimeSlicingLp = %u" CRA, pSti->TpsTimeSlicingLp));
   DIB_DEBUG(MSG_LOG, (CRB "  \tTpsMpeFecHp      = %u" CRA, pSti->TpsMpeFecHp));
   DIB_DEBUG(MSG_LOG, (CRB "  \tTpsMpeFecLp      = %u" CRA, pSti->TpsMpeFecLp));
   DIB_DEBUG(MSG_LOG, (CRB "  \tTpsCellId        = %u" CRA, pSti->TpsCellId));
   DIB_DEBUG(MSG_LOG, (CRB "  \tTpsLen           = %u" CRA, pSti->TpsLen));
   DIB_DEBUG(MSG_LOG, (CRB "  \tTpsReserved      = %u" CRA, pSti->TpsReserved));
   DIB_DEBUG(MSG_LOG, (CRB "  \tTpsDecLock       = %u" CRA, pSti->TpsDecLock));
   DIB_DEBUG(MSG_LOG, (CRB "  \tTpsSyncLock      = %u" CRA, pSti->TpsSyncLock));
   DIB_DEBUG(MSG_LOG, (CRB "  \tTpsDataLock      = %u" CRA, pSti->TpsDataLock));
   DIB_DEBUG(MSG_LOG, (CRB "  \tTpsBchUncor      = %u" CRA, pSti->TpsBchUncor));
   DIB_DEBUG(MSG_LOG, (CRB "  \tTpsCellIdLock    = %u" CRA, pSti->TpsCellIdLock));
}

DIBSTATUS DibDriverFireflyGetChannelInfoDvb(struct DibDriverContext * pContext, uint8_t ChannelIdx, uint8_t ScanStatus, uint16_t *Data, uint8_t TuneMonit)
{
   /* Either Data (0:1) or SiPsi (2) channel. */
   struct DibTuneChan    *pTc    = &(pContext->ChannelInfo[ChannelIdx].ChannelDescriptor);
   struct DibDvbTuneChan *pTcDvb = &(pContext->ChannelInfo[ChannelIdx].ChannelDescriptor.Std.Dvb);

   enum DibBool   ScanFlag    = (enum DibBool)((Data[0] >> 4) & 0xf);
   uint8_t        InvSpec     = (Data[0] >> 12) & 0x0f;

   DIB_DEBUG(MSG_LOG, (CRB "  End of scan message InvSpec=%d, scan_flag=%d" CRA, InvSpec, ScanFlag));

   if(TuneMonit)
   {
      pTc    = &(pContext->ChannelInfo[ChannelIdx].MonitContext.TuneMonit.ChannelDescriptor);
      pTcDvb = &(pContext->ChannelInfo[ChannelIdx].MonitContext.TuneMonit.ChannelDescriptor.Std.Dvb);
   }

   DIB_ASSERT(pTcDvb);
   DIB_ASSERT(pTc);

   pTc->InvSpec    = (enum DibSpectrumInv)InvSpec;

   /** the channel handler is important in callback mode */
   pTc->ChannelHdl = ChannelIdx;
   pTc->ScanStatus = ScanStatus;

   /* Gets TPS if relevant. */
   if(ScanStatus == DIB_SCAN_SUCCESS)
   {
      if(ScanFlag == eDIB_TRUE)
         IntDriverFireflyParseTpsRegisters(pContext, Data + 1, pTcDvb);
   }
   else
   {
      /* No Lock, the TPS have no sense */
      DibZeroMemory(&pTcDvb->TpsInfo, sizeof(struct DibTpsInfo));
   }

   return DIBSTATUS_SUCCESS;
}


/****************************************************************************
* IntDriverFireflyScanChannel
*
* field name          field Length (bits)    word index (weight)
* ----------          -------------------    -------------------
* no. of words        8                      0 (lsb)
* mess. ID            8                      0 (msb)
*
* RFkHz (ll)          8                      1 (lsb)
* RFkHz (l)           8                      1 (msb)
*
* RFkHz (h)           8                      2 (lsb)
* RFkHz (hh)          8                      2 (msb)
*
* Bw (l)              8                      3 (lsb)
* Bw (h)              8                      3 (msb)
*
* nfft                8                      4 (lsb)
* guard               8                      4 (msb)
*
* InvSpec             8                      5 (lsb)
* TS channel no.      8                      5 (msb)
****************************************************************************/
DIBSTATUS DibDriverFireflyScanChannelDvb(struct DibDriverContext *pContext, uint8_t tsChNum, struct DibTuneChan *pTc, enum DibDemodType Type)
{
   uint16_t buf[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
   uint8_t  Nb      = 10;


   struct DibDvbTuneChan *pTcDvb = &(pTc->Std.Dvb);

   DIB_ASSERT(pTc);
   DIB_ASSERT(tsChNum < DIB_MAX_NB_TS_CHANNELS);
   DIB_ASSERT((Type == eSTANDARD_DVB) || (Type == (eSTANDARD_DVB | eFAST)));

   buf[0] = (OUT_MSG_SCAN_CHANNEL << 8) + Nb;
   buf[1] = (pTc->RFkHz & 0xff) | (pTc->RFkHz & 0xff00);
   buf[2] = ((pTc->RFkHz >> 16) & 0xff) | ((pTc->RFkHz >> 16) & 0xff00);
   buf[3] = (pTc->Bw & 0xff) | (pTc->Bw & 0xff00);
   buf[4] = (pTcDvb->Nfft & 0xff) | ((pTcDvb->Guard & 0xff) << 8);
   buf[5] = (pTc->InvSpec & 0x0f) | ((pTcDvb->VitSelectHp & 0x0f) << 4) | ((tsChNum & 0xff) << 8);
   buf[6] = (pTcDvb->IntlvNative & 0xff) | ((pTcDvb->VitHrch & 0xff) << 8);
   buf[7] = (pTcDvb->VitAlpha & 0xff) | ((pTcDvb->VitCodeRateHp & 0xff) << 8);
   buf[8] = (pTcDvb->VitCodeRateLp & 0xff) | ((pTcDvb->Nqam & 0xff) << 8);
   buf[9] = (uint8_t) Type; /* SDK must match ff adapter.h */

   DIB_DEBUG(MSG_LOG,
            ("  IntDriverFireflyScanChannel: n_ts_channel=%d: RFkHz=%d, Bw=%d, nqam=%d, nfft=%d, guard=%d, InvSpec=%d, ",
            tsChNum, pTc->RFkHz, pTc->Bw, pTcDvb->Nqam, pTcDvb->Nfft, pTcDvb->Guard, pTc->InvSpec));

   DIB_DEBUG(MSG_LOG,
            (CRB "VitSelectHp=%d, VitHrch=%d, VitAlpha=%d, VitCodeRateHp=%d, VitCodeRateLp=%d" CRA,
            pTcDvb->VitSelectHp, pTcDvb->VitHrch, pTcDvb->VitAlpha, pTcDvb->VitCodeRateHp, pTcDvb->VitCodeRateLp));

   return DibDriverSendMessage(pContext, (uint32_t*) buf, (uint32_t) (Nb << 1));

}

/**
* DibDriverFrontendRfTotalGain
*/
uint32_t DibDriverFireflyFrontendRfTotalGain(uint8_t LnaStatus, uint32_t AgcGlobal, struct DibDriverContext *pContext)
{
   int32_t  LnaGaindBArray[3] = { 0, 0, 0 };
   uint32_t LnaGaindB         = (1 << 16) * LnaGaindBArray[LnaStatus];
   uint32_t AgcGaindB         = AgcGlobal * (pContext->Hd.BoardConfig->DefAgcDynamicsDb);

   DIB_ASSERT(LnaStatus < 3);

   return ((AgcGaindB + LnaGaindB) / (uint32_t) (1 << 16));
}

void DibDriverFireflyProcessMsgMonitDemodDvb(struct DibDriverContext * pContext, uint16_t *Data, struct DibTotalDemodMonit * pTotalMonitInfo)
{
   uint8_t   TsChannelNumber  = (uint8_t) Data[0] & 0x03;
   uint8_t   DemId            = (((uint8_t) Data[0]) >> 2) & 0x03;
   uint8_t   Timeout          =  (Data[0] >> 4) & 0x01;
   uint8_t   Aborted          = ((Data[0] >> 4) & 0x02) >> 1;
   uint8_t   Failed           = ((Data[0] >> 4) & 0x04) >> 2;
   uint16_t  TmpPow, TmpExp;
   uint32_t  TmpFp;
   uint32_t  CopyDemInfo;
   uint8_t   ChannelIdx      = pContext->FireflyChannel[TsChannelNumber];

   struct DibDvbDemodMonit   *pMonitInfo;

   if(Timeout)
      pTotalMonitInfo->MonitStatus = DIB_MONIT_TIMEOUT;
   else if(Aborted)
      pTotalMonitInfo->MonitStatus = DIB_MONIT_ABORTED;
   else if(Failed)
      pTotalMonitInfo->MonitStatus = DIB_MONIT_INVALID_PARAM;
   else
      pTotalMonitInfo->MonitStatus = DIB_MONIT_RELIABLE;


   pTotalMonitInfo->Type = eSTANDARD_DVB;

   pMonitInfo = &(pTotalMonitInfo->DemodMonit[DemId].Dvb);
   DIB_ASSERT(pMonitInfo != NULL);

   /* see if the information for this dem_id is wanted by the user */
   if(((TsChannelNumber == 3) &&
       ((pContext->ChannelInfo[ChannelIdx].MonitContext.TuneMonit.ChannelDescriptor.ScanStatus == DIB_SCAN_FAILURE) ||
        (pContext->ChannelInfo[ChannelIdx].MonitContext.TuneMonit.ChannelDescriptor.ScanStatus == DIB_SCAN_ABORTED) ||
        (pContext->ChannelInfo[ChannelIdx].MonitContext.TuneMonit.ChannelDescriptor.ScanStatus == DIB_SCAN_INVALID_PARAM))) ||
        (DemId >= pContext->ChannelInfo[ChannelIdx].NbDemodsUser))
   {
      /* This monit info should be not set if scan of TuneMonit as failed
      OR This demod info are not wanted by the user */
      CopyDemInfo=0;
      memset(pMonitInfo, 0, sizeof(struct DibDvbDemodMonit));
   }
   else
   {
      /* This demod info is wanted by the user */
      CopyDemInfo=1;
   }

   if(CopyDemInfo)
   {
      pMonitInfo->Timeout        = Timeout;
      pMonitInfo->LnaStatus      = (Data[0] >> 8) & 0xff;
      pMonitInfo->AgcPower       = ((uint32_t) (Data[1] & 0x000f)) << 16 | (uint32_t) (Data[2]); /* [384] | [385]*/
      pMonitInfo->VAgcRf         = Data[3] & 0xffff;        /* [388] */
      pMonitInfo->VAgcBB         = Data[4] & 0xffff;        /* [389] */
      pMonitInfo->AgcGlobal      = (uint32_t) (Data[5]);    /* [390] */
      pMonitInfo->RfTotalGain    = DibDriverFireflyFrontendRfTotalGain(pMonitInfo->LnaStatus, pMonitInfo->AgcGlobal, pContext);
      pMonitInfo->AgcSplitOffset = Data[6] & 0xff;          /* [392] */
      pMonitInfo->AgcWbd         = Data[7] & 0xfff;         /* [393] */

      /*  Noise.  */
      TmpPow = ((Data[8] & 0x0ff0) >> 4) & 0xff;            /* [505] */
      TmpExp = (((Data[8] & 0x000f) << 2) | ((Data[9] & 0xc000) >> 14)) & 0x3f;        /* [505] | [506] */

      if(TmpExp & 0x0020)
         TmpExp |= 0xc0;

      TmpFp   = (uint8_t) (TmpPow);
      TmpFp <<= (int8_t) (TmpExp) + 16;

      pMonitInfo->WgnFp = (uint32_t) (TmpFp);

      /*  Signal.  */
      TmpPow = ((Data[9] & 0x3fc0) >> 6) & 0xff;   /* [506] */
      TmpExp = Data[9] & 0x3f;                     /* [506] */

      if(TmpExp & 0x0020)
         TmpExp |= 0xc0;

      TmpFp = (uint8_t) (TmpPow);
      TmpFp <<= (int8_t) (TmpExp) + 16;

      pMonitInfo->SigFp = (uint32_t) (TmpFp);

      /*  pMonitInfo->Diversity = Data[10]; */
      pMonitInfo->FecBerRs          = (((uint32_t) (Data[11] & 0x001f)) << 16) | ((uint32_t) Data[12]); /* [526] | [527] */
      pMonitInfo->FecMpfailCnt      = Data[13];      /* [532] */
      pMonitInfo->Locks             = Data[14];
      pMonitInfo->SignalQuality     = (uint8_t) (Data[15] & 0x00FF);        /* Signal quality. */
      pMonitInfo->SignalQualityFast = (uint8_t) ((Data[15] & 0xFF00) >> 8); /* Signal quality. */

      DIB_DEBUG(MSG_LOG, (CRB "  ActiveDemod=%d DemId=%d SFQ=%d" CRA,
               pContext->ChannelInfo[ChannelIdx].ChannelMonit.NbDemods, DemId, pMonitInfo->SignalQualityFast));
   }

}
#endif


#endif /* USE_FIREFLY */
