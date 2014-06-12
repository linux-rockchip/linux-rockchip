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
* @file "DibDriverDragonflyStdDab.c"
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

#if (DIB_DAB_STD == 1)

DIBSTATUS DibDriverDragonflySetMsgTuneDab(struct DibTuneChan *pTc, struct MsgCreateChannel *pMsg)
{
   /* no support of Fic with know parameters */
   DibSetMemory(&pMsg->Desc.Param, -1, sizeof(pMsg->Desc.Param));
   return DIBSTATUS_SUCCESS;
}

DIBSTATUS DibDriverDragonflyGetChannelInfoDab(struct DibDriverContext * pContext, struct MsgUpdateChannelIndication * pMsg, uint16_t ChHdl)
{
   struct DibDabTuneChan * pDab;

   /* do not store scanning information at the same place if TuneMonit */
   if(pContext->ChannelInfo[ChHdl].Temporary == eDIB_FALSE)
      pDab = &(pContext->ChannelInfo[ChHdl].ChannelDescriptor.Std.Dab);
   else
      pDab = &(pContext->ChannelInfo[ChHdl].MonitContext.TuneMonit.ChannelDescriptor.Std.Dab);

   pDab->TMode       = pMsg->Desc.Param.Dab.TMode;
   pDab->NbActiveSub = pMsg->Desc.Param.Dab.NbActiveSubCh;

   return DIBSTATUS_SUCCESS;
}


void DibDriverDragonflyProcessMsgMonitDemodDab(struct DibDriverContext * pContext, struct MsgAckGetSignalMonit * pMsg, struct DibTotalDemodMonit * pTotalMonitInfo)
{
   struct DibDvbDemodMonit * pDemodMonit;
   struct SignalMonitoring * pMon = &pMsg->Mon;

   pDemodMonit = &pTotalMonitInfo->DemodMonit[pMsg->DemodId].Dab.Dvb;

   pDemodMonit->FecBerRs                               = pMon->ChanDec.Dab.Ber;
   pDemodMonit->FecMpfailCnt                           = pMon->ChanDec.Dab.Per;
   pDemodMonit->Timeout                                = (pMon->MonitStatus == DIB_MONIT_TIMEOUT) ? 1 : 0;
   pDemodMonit->AgcPower                               = pMon->AdcPower;
   pDemodMonit->AgcGlobal                              = pMon->AgcGlobal;
   pDemodMonit->RfTotalGain                            = (pDemodMonit->AgcGlobal * pContext->Hd.BoardConfig->DefAgcDynamicsDb) / (uint32_t) (1 << 16);
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
   pDemodMonit->SignalQuality                          = pMon->ChanDec.Dab.Quality;
   pDemodMonit->SignalQualityFast                      = pMon->ChanDec.Dab.FastQuality;
   pDemodMonit->LnaStatus                              = 0;
   pDemodMonit->Locks                                  = pMon->Locks;
   pDemodMonit->TransmitterId                          = pMon->TransmitterId;
   pTotalMonitInfo->DemodMonit[pMsg->DemodId].Dab.DmbLockFrame = (pMon->Locks >> 8) & 1;
   pTotalMonitInfo->DemodMonit[pMsg->DemodId].Dab.DmbBer = pMon->ChanDec.Dab.TdmbBer;
}

uint8_t DibDriverDragonflyGetTimeoutDab(struct DibTotalDemodMonit * pTotalMonitInfo, uint32_t TmpDemId)
{
   return pTotalMonitInfo->DemodMonit[TmpDemId].Dab.Dvb.Timeout;
}

void DibDriverDragonflyCheckMonitDab(struct DibDriverContext * pContext, struct DibTotalDemodMonit * pTotalMonitInf, CHANNEL_HDL ChannelHdl)
{
   uint8_t NbDemod;
   uint8_t i;
   uint8_t ValidChanDecMonit = 0;
   struct DibDabDemodMonit * pDabDemodMonit;
   NbDemod = pContext->ChannelInfo[ChannelHdl].pStream->NbConnFrontends;

   for (i=0;i<NbDemod;i++)
   {
      pDabDemodMonit = &pTotalMonitInf->DemodMonit[i].Dab;
      if (DIB_GET_DAB_LOCK_DAB_COFF(pDabDemodMonit->Dvb.Locks) == 1)
        ValidChanDecMonit = 1;
   }

   if (ValidChanDecMonit == 0)
   {
     pDabDemodMonit = &pTotalMonitInf->DemodMonit[0].Dab;
     pDabDemodMonit->Dvb.Locks &= ~0x01C0;
     pDabDemodMonit->DmbLockFrame = 0;
   }
}

#endif

#endif /* USE_DRAGONFLY */
